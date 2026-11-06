#include "aerostyle.h"
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
#include <QPixmap>

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
    connect(&m_tick, &QTimer::timeout, this, &AeroStyle::tick);
    m_theme.loadDefault();
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &AeroStyle::reloadTheme);
    reloadTheme();
}

AeroStyle::~AeroStyle() { qDeleteAll(m_modules); }

void AeroStyle::reloadTheme()
{
    const QString user = Theme::userPath();
    if (QFileInfo::exists(user)) {
        if (m_theme.load(user)) {
            if (m_debug) qInfo() << "[aeroqt] loaded theme" << m_theme.name() << "from" << user;
        } else qWarning() << "[aeroqt] bad theme json:" << user;
        if (!m_watcher.files().contains(user)) m_watcher.addPath(user);  // re-arm after atomic saves
    }
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
    if (!m_animOn) { m_hover.clear(); m_phase.clear(); m_tick.stop(); }
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
        it = m_hover.insert(w, {});
        connect(w, &QObject::destroyed, this, &AeroStyle::forget);
    }
    Hot &h = (*it)[id];                 // new parts start at 0 and fade toward the target
    if (h.target != tgt) { h.target = tgt; kick(); }
    return h.v;
}

qreal AeroStyle::phase(const QWidget *w, int periodMs)
{
    if (!m_animOn || !w || periodMs <= 0) return 0.5;
    const qint64 now = m_clock.elapsed();
    if (!m_phase.contains(w)) connect(w, &QObject::destroyed, this, &AeroStyle::forget);
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

void AeroStyle::unpolish(QWidget *w) { QProxyStyle::unpolish(w); }

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
        QPixmap pm(16, 16);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        const QColor c = (opt && opt->state & QStyle::State_Enabled)
                             ? opt->palette.buttonText().color()
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
