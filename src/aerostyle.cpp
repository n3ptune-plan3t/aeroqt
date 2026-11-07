#include "aerostyle.h"
#include "glass.h"
#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QDial>
#include <QFileInfo>
#include <QScrollBar>
#include <QSlider>
#include <QCalendarWidget>
#include <QSplitter>
#include <QTabBar>
#include <QAbstractScrollArea>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QPixmap>
#include <QScreen>

AeroStyle::AeroStyle() : QProxyStyle(QStringLiteral("fusion"))
{
    m_debug = qEnvironmentVariableIsSet("AEROQT_DEBUG");
    registerButtons(m_modules);
    registerInputs(m_modules);
    registerProgress(m_modules);
    registerScrollbar(m_modules);
    registerSlider(m_modules);
    registerSpinCombo(m_modules);
    registerTabs(m_modules);
    registerMenus(m_modules);
    registerFrames(m_modules);
    registerItemViews(m_modules);
    registerDocks(m_modules);
    registerBars(m_modules);
    registerMisc(m_modules);
    registerTitleBar(m_modules);
    for (Module *m : m_modules) { m->self = this; m->anim = this; }
    m_clock.start();
    m_tick.setInterval(16);
    // Coarse timer lets the OS coalesce wakeups (lightweight: fewer precise
    // timer fires than Qt's default); still smooth at 16-33ms frame times.
    m_tick.setTimerType(Qt::CoarseTimer);
    connect(&m_tick, &QTimer::timeout, this, &AeroStyle::tick);
    m_theme.loadDefault();
    m_reloadTimer.setSingleShot(true);
    m_reloadTimer.setInterval(60);
    connect(&m_reloadTimer, &QTimer::timeout, this, &AeroStyle::reloadTheme);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &AeroStyle::scheduleReload);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString &) {
        // Only the override file matters; ignore unrelated churn in ~/.config/aeroqt.
        const QString user = Theme::userPath();
        if (QFileInfo::exists(user) != m_userThemeLoaded || !m_watcher.files().contains(user))
            scheduleReload();
    });
    reloadTheme();
}

AeroStyle::~AeroStyle()
{
    // Pixmaps must not outlive the QGuiApplication (they would be destroyed from
    // a static destructor otherwise); a second AeroStyle just re-fills the cache.
    Glass::clearCache();
    qDeleteAll(m_modules);
}

void AeroStyle::scheduleReload()
{
    // Editors fire several change events per save (truncate, write, rename);
    // coalesce them into one reload.
    m_reloadTimer.start();
}

void AeroStyle::armWatcher()
{
    // Watch the file (content edits) AND its directory (file created / replaced by an
    // atomic rename / deleted) so a theme.json added after login hot-reloads too.
    const QString user = Theme::userPath();
    const QString dir = QFileInfo(user).absolutePath();
    if (QFileInfo::exists(user) && !m_watcher.files().contains(user)) m_watcher.addPath(user);
    if (QDir(dir).exists() && !m_watcher.directories().contains(dir)) m_watcher.addPath(dir);
}

void AeroStyle::reloadTheme()
{
    Glass::clearCache();
    const QString user = Theme::userPath();
    if (QFileInfo::exists(user)) {
        if (m_theme.load(user)) {
            if (m_debug) qInfo() << "[aeroqt] loaded theme" << m_theme.name() << "from" << user;
        } else {
            qWarning() << "[aeroqt] bad theme json, keeping the previous theme:" << user;
        }
    } else if (m_userThemeLoaded) {
        // The override was removed: fall back to the selected built-in instead of
        // silently keeping the stale override.
        m_theme.loadDefault();
        if (m_debug) qInfo() << "[aeroqt] user theme removed, back to" << m_theme.name();
    }
    m_userThemeLoaded = QFileInfo::exists(user);
    armWatcher();
    applyAnimSettings();
    if (qApp)
        for (QWidget *w : QApplication::allWidgets()) w->update();
}

// ------------------------------------------------------------------ animation engine
void AeroStyle::applyAnimSettings()
{
    m_animOn = !qEnvironmentVariableIsSet("AEROQT_NO_ANIM") && m_theme.num("anim.enabled", 1) > 0.5;
    m_inMs  = qMax(1, int(m_theme.num("anim.hoverIn", 110)));
    m_outMs = qMax(1, int(m_theme.num("anim.hoverOut", 240)));
    // Lightweight tick (Fusion draws instantly; Breeze/Klassy animate, but
    // 60Hz CoarseTimer wakeups are the main CPU cost on low-end laptops).
    // aero-lite sets anim.frameMs=33 (~30fps) via theme; AEROQT_PERF_LIGHT
    // forces it for any theme (matches Klassy-style "reduce effects" presets).
    const bool perfLight = qEnvironmentVariableIsSet("AEROQT_PERF_LIGHT");
    int frameMs = qBound(8, int(m_theme.num("anim.frameMs", 16)), 100);
    if (perfLight) frameMs = qMax(frameMs, 33);
    if (m_tick.interval() != frameMs) m_tick.setInterval(frameMs);
    // Perf multipliers scale/skip glass gradient passes (see Glass::setPerf).
    qreal glossMul = m_theme.num("perf.gloss", 1);
    qreal glowMul = m_theme.num("perf.glow", 1);
    if (perfLight) { glossMul = qMin(glossMul, 0.3); glowMul = qMin(glowMul, 0.3); }
    Glass::setPerf(glossMul, glowMul);
    if (!m_animOn) { m_hover.clear(); m_phase.clear(); m_tick.stop(); }
}

int AeroStyle::trackedParts() const
{
    int n = 0;
    for (auto it = m_hover.constBegin(); it != m_hover.constEnd(); ++it) n += it->size();
    return n;
}

void AeroStyle::kick()
{
    if (m_tick.isActive()) return;
    m_last = m_clock.elapsed();
    m_tick.start();
}

void AeroStyle::forget(QObject *o)
{
    m_hover.remove(o);
    m_phase.remove(o);
}

qreal AeroStyle::hover(const QWidget *w, quintptr id, bool on)
{
    const qreal tgt = on ? 1.0 : 0.0;
    if (!m_animOn || !w) return tgt;
    auto it = m_hover.find(w);
    if (it == m_hover.end()) {
        // Almost every painted part is cold. Do not allocate state (or connect a signal)
        // for something that is not being hovered and has no fade in flight.
        if (!on) return 0.0;
        it = m_hover.insert(w, {});
        // UniqueConnection: entries are erased when they come to rest, so the same long-lived
        // widget gets here again on every later hover; without it each hover would add
        // another destroyed() connection.
        connect(w, &QObject::destroyed, this, &AeroStyle::forget, Qt::UniqueConnection);
    }
    auto hi = it->find(id);
    if (hi == it->end()) {
        if (!on) return 0.0;
        hi = it->insert(id, {});         // new parts start at 0 and fade toward the target
    }
    Hot &h = hi.value();
    if (h.target != tgt) { h.target = tgt; kick(); }
    return h.v;
}

qreal AeroStyle::phase(const QWidget *w, int periodMs)
{
    if (!m_animOn || !w || periodMs <= 0) return 0.5;
    const qint64 now = m_clock.elapsed();
    if (!m_phase.contains(w)) connect(w, &QObject::destroyed, this, &AeroStyle::forget, Qt::UniqueConnection);
    m_phase[w] = now;
    kick();
    return qreal(now % periodMs) / periodMs;
}

static void repaintWidget(const QObject *o)
{
    auto *w = qobject_cast<QWidget *>(const_cast<QObject *>(o));
    if (!w) return;
    if (auto *sa = qobject_cast<QAbstractScrollArea *>(w)) sa->viewport()->update();   // item views paint in the viewport
    w->update();
}

void AeroStyle::tick()
{
    const qint64 now = m_clock.elapsed();
    const qreal dt = qreal(now - m_last);
    m_last = now;
    bool active = false;
    QVector<const QObject *> dirty;

    for (auto wi = m_hover.begin(); wi != m_hover.end();) {
        bool moved = false;
        for (auto ai = wi->begin(); ai != wi->end();) {
            Hot &h = ai.value();
            if (h.v != h.target) {
                const qreal step = dt / (h.target > h.v ? m_inMs : m_outMs);
                h.v = h.target > h.v ? qMin(h.target, h.v + step) : qMax(h.target, h.v - step);
                moved = active = true;
                ++ai;
            } else if (h.v == 0) {
                ai = wi->erase(ai);                         // at rest and cold: forget it
            } else {
                ++ai;
            }
        }
        if (moved) dirty.append(wi.key());
        wi = wi->isEmpty() ? m_hover.erase(wi) : std::next(wi);
    }
    for (auto pi = m_phase.begin(); pi != m_phase.end();) {
        if (now - pi.value() > 250) { pi = m_phase.erase(pi); continue; }   // nobody asked lately: stop
        auto *w = qobject_cast<const QWidget *>(pi.key());
        if (w && w->isVisible()) { dirty.append(pi.key()); active = true; }
        ++pi;
    }
    for (const QObject *o : std::as_const(dirty)) repaintWidget(o);
    if (!active) m_tick.stop();
}

void AeroStyle::polish(QWidget *w)
{
    QProxyStyle::polish(w);
    if (qobject_cast<QAbstractButton *>(w) || qobject_cast<QComboBox *>(w)
        || qobject_cast<QAbstractSpinBox *>(w) || qobject_cast<QScrollBar *>(w)
        || qobject_cast<QTabBar *>(w) || qobject_cast<QSlider *>(w) || qobject_cast<QDial *>(w)
        || qobject_cast<QCalendarWidget *>(w))
        w->setAttribute(Qt::WA_Hover, true);
    if (qobject_cast<QSplitterHandle *>(w))
        w->setAttribute(Qt::WA_Hover, true);
    if (auto *v = qobject_cast<QAbstractItemView *>(w))              // per-item hover slab
        v->viewport()->setAttribute(Qt::WA_Hover, true);
}

void AeroStyle::unpolish(QWidget *w)
{
    forget(w);
    QProxyStyle::unpolish(w);
}

#define DISPATCH(call)                                                                       \
    for (const Module *m : m_modules)                                                        \
        if (m->call) {                                                                       \
            if (m_debug) qDebug("[aeroqt] %-9s handled %s=%d", m->name(), #call, int(el));   \
            return;                                                                          \
        }

void AeroStyle::drawPrimitive(PrimitiveElement el, const QStyleOption *o, QPainter *p, const QWidget *w) const
{
    DISPATCH(drawPrimitive(el, o, p, w, m_theme))
    if (m_debug) qDebug("[aeroqt] fallback   drawPrimitive=%d", int(el));
    QProxyStyle::drawPrimitive(el, o, p, w);
}

void AeroStyle::drawControl(ControlElement el, const QStyleOption *o, QPainter *p, const QWidget *w) const
{
    DISPATCH(drawControl(el, o, p, w, m_theme))
    if (m_debug) qDebug("[aeroqt] fallback   drawControl=%d", int(el));
    QProxyStyle::drawControl(el, o, p, w);
}

void AeroStyle::drawComplexControl(ComplexControl el, const QStyleOptionComplex *o, QPainter *p, const QWidget *w) const
{
    DISPATCH(drawComplex(el, o, p, w, m_theme))
    if (m_debug) qDebug("[aeroqt] fallback   drawComplex=%d", int(el));
    QProxyStyle::drawComplexControl(el, o, p, w);
}

int AeroStyle::pixelMetric(PixelMetric pm, const QStyleOption *o, const QWidget *w) const
{
    for (const Module *m : m_modules) {
        const int v = m->pixelMetric(pm, o, w, m_theme);
        if (v >= 0) return v;
    }
    return QProxyStyle::pixelMetric(pm, o, w);
}

QSize AeroStyle::sizeFromContents(ContentsType ct, const QStyleOption *o, const QSize &sz, const QWidget *w) const
{
    for (const Module *m : m_modules) {
        const QSize v = m->sizeFromContents(ct, o, sz, w, m_theme);
        if (v.isValid()) return v;
    }
    return QProxyStyle::sizeFromContents(ct, o, sz, w);
}

QRect AeroStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *o, SubControl sc, const QWidget *w) const
{
    for (const Module *m : m_modules) {
        const QRect v = m->subControlRect(cc, o, sc, w, m_theme);
        if (!v.isNull()) return v;
    }
    return QProxyStyle::subControlRect(cc, o, sc, w);
}

QRect AeroStyle::subElementRect(SubElement se, const QStyleOption *o, const QWidget *w) const
{
    for (const Module *m : m_modules) {
        const QRect v = m->subElementRect(se, o, w, m_theme);
        if (!v.isNull()) return v;
    }
    return QProxyStyle::subElementRect(se, o, w);
}

int AeroStyle::styleHint(StyleHint sh, const QStyleOption *o, const QWidget *w, QStyleHintReturn *ret) const
{
    for (const Module *m : m_modules) {
        const int v = m->styleHint(sh, o, w, ret, m_theme);
        if (v != Module::Unhandled) return v;
    }
    return QProxyStyle::styleHint(sh, o, w, ret);
}

QPixmap AeroStyle::standardPixmap(StandardPixmap sp, const QStyleOption *opt, const QWidget *w) const
{
    // Toolbar extension button icon (QToolBar shows a QToolButton with this pixmap;
    // there is no PE_IndicatorToolBarExtension primitive in QStyle).
    if (sp == SP_ToolBarHorizontalExtensionButton || sp == SP_ToolBarVerticalExtensionButton) {
        const bool horiz = sp == SP_ToolBarHorizontalExtensionButton;
        // HiDPI: paint at logical size but back the buffer with devicePixelRatio
        // physical pixels (https://doc.qt.io/qt-6/highdpi.html "Drawing High
        // Resolution Versions of Pixmaps"), so the chevron stays sharp on
        // Retina. Disabled color comes from the Disabled palette group, not a
        // literal grey, so it stays correct in both light and dark schemes.
        qreal dpr = 1.0;
        if (w && w->devicePixelRatioF() > 0) dpr = w->devicePixelRatioF();
        else if (QGuiApplication::primaryScreen() && QGuiApplication::primaryScreen()->devicePixelRatio() > 0)
            dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
        QPixmap pm(16, 16);
        pm.setDevicePixelRatio(dpr);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        const QColor c = opt
            ? (opt->state & QStyle::State_Enabled
                   ? opt->palette.buttonText().color()
                   : opt->palette.color(QPalette::Disabled, QPalette::ButtonText))
            : QColor(120, 120, 120);
        p.setPen(Qt::NoPen);
        p.setBrush(c);
        for (int i = 0; i < 2; ++i) {           // double chevron along the bar axis
            const int off = i * 5;
            if (horiz) {
                const int x = 2 + off;
                p.drawPolygon(QPolygonF{QPointF(x, 4), QPointF(x, 12), QPointF(x + 4, 8)});
            } else {
                const int y = 2 + off;
                p.drawPolygon(QPolygonF{QPointF(4, y), QPointF(12, y), QPointF(8, y + 4)});
            }
        }
        return pm;
    }
    (void)w;
    return QProxyStyle::standardPixmap(sp, opt, w);
}
