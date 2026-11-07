#include "glass.h"
#include <QCache>
#include <QHash>
#include <QtGlobal>
#include <cstring>
#include <QLinearGradient>
#include <QPaintDevice>
#include <QPainterPath>
#include <QPixmap>
#include <QRadialGradient>

namespace Glass {

// Qt HiDPI keeps a device-independent coordinate system for Widgets/Quick, but
// hairline alignment still depends on the *physical* pixel grid
// (https://doc.qt.io/qt-6/highdpi.html): a 1px border is 1 physical px, i.e.
// 0.5 / devicePixelRatio logical units. Snap with the actual DPR so borders
// stay crisp and not double-width on Retina (QPaintDevice::devicePixelRatioF).
static qreal snap(QPainter *p)
{
    qreal dpr = 1.0;
    if (QPaintDevice *dev = p ? p->device() : nullptr) {
        dpr = dev->devicePixelRatioF();
        if (!qIsFinite(dpr) || dpr <= 0) dpr = 1.0;
    }
    return 0.5 / dpr;
}

static QPainterPath rr(const QRectF &r, qreal rad)
{
    QPainterPath p;
    p.addRoundedRect(r, rad, rad);
    return p;
}

// Lightweight knobs: gloss/glow pass multipliers from the theme ("perf.gloss"
// / "perf.glow", 1 = full Aero, 0 = skip that gradient pass entirely).
static qreal g_glossMul = 1.0;
static qreal g_glowMul = 1.0;
void setPerf(qreal glossMul, qreal glowMul)
{
    g_glossMul = qBound<qreal>(0.0, glossMul, 1.0);
    g_glowMul = qBound<qreal>(0.0, glowMul, 1.0);
    clearCache();
}

// Settled-state slab pixmap cache. Keyed by a POD of the physical size, DPR
// and every spec field that changes the pixels (no per-paint string
// building), so a theme reload, palette change or DPR change naturally
// misses and re-renders. QCache gives real LRU eviction and a byte budget
// instead of a fixed entry count, so one huge slab cannot evict a whole
// window's worth of small buttons.
struct SlabKey {
    quint32 v[14];
    bool operator==(const SlabKey &o) const { return std::memcmp(v, o.v, sizeof v) == 0; }
};
static inline size_t qHash(const SlabKey &k, size_t seed = 0) noexcept
{
    return qHashBits(k.v, sizeof k.v, seed);
}

static constexpr int kCacheBudgetKB = 6144;           // total pixmap budget (~6 MB)
static QCache<SlabKey, QPixmap> g_slabs(kCacheBudgetKB);

void clearCache() { g_slabs.clear(); }

static SlabKey makeKey(int pw, int ph, qreal dpr, const Spec &s, qreal hv)
{
    SlabKey k;
    std::memset(&k, 0, sizeof k);
    int i = 0;
    k.v[i++] = quint32(pw);
    k.v[i++] = quint32(ph);
    k.v[i++] = quint32(qRound(dpr * 100));
    k.v[i++] = quint32(qRound(s.radius * 100));
    k.v[i++] = s.base.rgba();
    k.v[i++] = s.border.rgba();
    k.v[i++] = s.glow.rgba();
    k.v[i++] = quint32(qRound(s.gloss * 100));
    k.v[i++] = quint32(qRound(hv * 100));
    k.v[i++] = quint32(qRound(s.hoverTint * 100));
    k.v[i++] = (s.pressed ? 1u : 0u) | (s.enabled ? 2u : 0u) | (s.axis == Qt::Horizontal ? 4u : 0u);
    return k;
}

// Direct painter (3 gradients). Called for animated hover frames (hoverT
// mid-transition) and as the cache miss path; the cached wrapper below is
// the hot path for settled slabs.
static void drawSlabDirect(QPainter *p, const QRectF &rectIn, const Spec &s)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    QRectF rect = rectIn;
    if (s.axis == Qt::Horizontal) {
        // Draw as a normal (top->bottom) slab in a rotated frame: local top == left edge.
        p->translate(rectIn.center());
        p->rotate(-90);
        rect = QRectF(-rectIn.height() / 2, -rectIn.width() / 2, rectIn.height(), rectIn.width());
    }
    const qreal sn = snap(p);
    const QRectF r = rect.adjusted(sn, sn, -sn, -sn);
    const QPainterPath body = rr(r, s.radius);

    const qreal hv = s.hoverT >= 0 ? s.hoverT : (s.hovered ? 1.0 : 0.0);
    QColor base = s.base;
    if (hv > 0 && s.hoverTint > 0) {                       // fade the accent tint in with the hover factor
        const qreal k = qBound<qreal>(0, s.hoverTint * hv, 1);
        base = QColor::fromRgbF(base.redF() + (s.glow.redF() - base.redF()) * k,
                                base.greenF() + (s.glow.greenF() - base.greenF()) * k,
                                base.blueF() + (s.glow.blueF() - base.blueF()) * k, base.alphaF());
    }
    if (!s.enabled) {
        // Desaturate toward the grey with the *same* luminance instead of
        // dropping HSV saturation: HSV keeps the value, which shifts warm/
        // cool greys (and a grey hueF() < 0 maps to red). 30% color + 70%
        // luminance-grey stays legible in both light and dark schemes.
        const qreal lum = 0.299 * base.redF() + 0.587 * base.greenF() + 0.114 * base.blueF();
        base = QColor::fromRgbF(base.redF() * 0.3 + lum * 0.7,
                                base.greenF() * 0.3 + lum * 0.7,
                                base.blueF() * 0.3 + lum * 0.7, base.alphaF());
    }

    // 1. body: Aero "split" gradient (light top half, deeper bottom half)
    QLinearGradient g(r.topLeft(), r.bottomLeft());
    const int k = s.pressed ? 90 : 118;
    g.setColorAt(0.00, base.lighter(k));
    g.setColorAt(0.48, base);
    g.setColorAt(0.52, base.darker(s.pressed ? 112 : 104));
    g.setColorAt(1.00, base.lighter(s.pressed ? 100 : 112));
    p->setPen(Qt::NoPen);
    p->setBrush(g);
    p->drawPath(body);

    // 2. bottom glow (reflected light; skipped in lite perf mode)
    if (s.enabled && g_glowMul > 0.001) {
        QColor gl = s.glow;
        gl.setAlphaF((0.45 + 0.30 * hv) * (s.pressed ? 0.6 : 1.0) * g_glowMul);
        QRadialGradient rg(r.center().x(), r.bottom(), r.width() * 0.55);
        rg.setColorAt(0, gl);
        gl.setAlpha(0);
        rg.setColorAt(1, gl);
        p->save();
        p->setClipPath(body, Qt::IntersectClip);   // keep any caller clip (tabs clip the slab)
        p->setBrush(rg);
        p->drawRect(r);
        p->restore();
    }

    // 3. glossy upper-half highlight (skipped in lite perf mode)
    if (!s.pressed && s.enabled && s.gloss * g_glossMul > 0.001) {
        QRectF top(r.left() + 1, r.top() + 1, r.width() - 2, r.height() * 0.48);
        QLinearGradient hg(top.topLeft(), top.bottomLeft());
        hg.setColorAt(0, QColor::fromRgbF(1, 1, 1, 0.85 * s.gloss * g_glossMul));
        hg.setColorAt(1, QColor::fromRgbF(1, 1, 1, 0.12 * s.gloss * g_glossMul));
        p->setBrush(hg);
        p->drawPath(rr(top, qMax<qreal>(0, s.radius - 1)));
    }

    // 4. inner white keyline + outer border. Keyline strength follows
    // edgeLight(base) (0.7 light / 0.1 dark) so the engraved white line stays
    // soft on dark themes instead of a fixed 0.55 white (see edgeLight note).
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(QColor::fromRgbF(1, 1, 1, (s.pressed ? 0.25 : 0.55) * edgeLight(base) / 0.7), 1));
    p->drawPath(rr(r.adjusted(1, 1, -1, -1), qMax<qreal>(0, s.radius - 1)));
    QColor b = s.border;
    if (!s.enabled) b.setAlphaF(0.5);
    p->setPen(QPen(b, 1));
    p->drawPath(body);
    p->restore();
}

void drawSlab(QPainter *p, const QRectF &rect, const Spec &s)
{
    // Cache only settled slabs (hoverT at rest): mid-transition frames are
    // few (hoverIn 110ms / hoverOut 240ms) and must vary continuously.
    const qreal hv = s.hoverT >= 0 ? s.hoverT : (s.hovered ? 1.0 : 0.0);
    const bool settled = hv <= 0.01 || hv >= 0.99;
    if (!p || !settled || rect.width() < 4 || rect.height() < 4
        || rect.width() > 2048 || rect.height() > 2048) {
        drawSlabDirect(p, rect, s);
        return;
    }
    qreal dpr = p->device() ? p->device()->devicePixelRatioF() : 1.0;
    if (!qIsFinite(dpr) || dpr <= 0) dpr = 1.0;

    // The pixmap's *physical* size is the logical size times the DPR; its
    // logical size (physical / dpr) is what we blit and what we paint into.
    const int pw = qMax(1, qRound(rect.width() * dpr));
    const int ph = qMax(1, qRound(rect.height() * dpr));
    const QRectF local(0, 0, pw / dpr, ph / dpr);

    const SlabKey key = makeKey(pw, ph, dpr, s, hv);
    if (const QPixmap *hit = g_slabs.object(key)) {
        p->drawPixmap(rect.topLeft(), *hit);        // caller's clip (tabs) still applies
        return;
    }
    const int costKB = qMax(1, pw * ph * 4 / 1024);
    if (costKB > kCacheBudgetKB / 8) {              // too big to be worth keeping
        drawSlabDirect(p, rect, s);
        return;
    }
    auto *pm = new QPixmap(pw, ph);
    if (pm->isNull()) { delete pm; drawSlabDirect(p, rect, s); return; }
    pm->setDevicePixelRatio(dpr);
    pm->fill(Qt::transparent);
    {
        QPainter mp(pm);
        drawSlabDirect(&mp, local, s);
    }
    p->drawPixmap(rect.topLeft(), *pm);
    g_slabs.insert(key, pm, costKB);                // QCache takes ownership
}

void drawInset(QPainter *p, const QRectF &rect, qreal radius, const QColor &fill, const QColor &border)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    const qreal sn = snap(p);
    const QRectF r = rect.adjusted(sn, sn, -sn, -sn);
    p->setPen(Qt::NoPen);
    p->setBrush(fill);
    p->drawPath(rr(r, radius));
    // soft inner top shadow
    QLinearGradient sh(r.topLeft(), r.topLeft() + QPointF(0, 5));
    sh.setColorAt(0, QColor(0, 0, 0, 38));
    sh.setColorAt(1, QColor(0, 0, 0, 0));
    p->save();
    p->setClipPath(rr(r, radius), Qt::IntersectClip);
    p->setBrush(sh);
    p->drawRect(r);
    p->restore();
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(border, 1));
    p->drawPath(rr(r, radius));
    p->restore();
}

void drawPanel(QPainter *p, const QRectF &rect, qreal radius, const QColor &fill, const QColor &border)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    const qreal sn = snap(p);
    const QRectF r = rect.adjusted(sn, sn, -sn, -sn);
    QLinearGradient g(r.topLeft(), r.bottomLeft());
    g.setColorAt(0, fill.lighter(106));
    g.setColorAt(1, fill);
    p->setPen(Qt::NoPen);
    p->setBrush(g);
    p->drawPath(rr(r, radius));
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(QColor::fromRgbF(1, 1, 1, 0.45), 1));          // inner keyline
    p->drawPath(rr(r.adjusted(1, 1, -1, -1), qMax<qreal>(0, radius - 1)));
    p->setPen(QPen(border, 1));
    p->drawPath(rr(r, radius));
    p->restore();
}

void drawBar(QPainter *p, const QRectF &rect, const QColor &base, const QColor &border)
{
    p->save();
    QLinearGradient g(rect.topLeft(), rect.bottomLeft());
    g.setColorAt(0.00, base.lighter(112));
    g.setColorAt(0.48, base.lighter(103));
    g.setColorAt(0.52, base);
    g.setColorAt(1.00, base.darker(104));
    p->fillRect(rect, g);
    p->setPen(border);
    p->drawLine(QPointF(rect.left(), rect.bottom() - 0.5), QPointF(rect.right(), rect.bottom() - 0.5));
    p->restore();
}

qreal edgeLight(const QColor &bg) { return bg.lightnessF() < 0.4 ? 0.10 : 0.70; }

void drawSeparator(QPainter *p, const QRectF &rect, Qt::Orientation lineAxis, const QColor &dark, qreal hiAlpha)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing, false);
    const QPointF m = rect.center();
    const int x = int(m.x()), y = int(m.y());
    p->setPen(dark);
    if (lineAxis == Qt::Horizontal) {
        p->drawLine(int(rect.left()), y, int(rect.right()), y);
        p->setPen(QColor::fromRgbF(1, 1, 1, hiAlpha));
        p->drawLine(int(rect.left()), y + 1, int(rect.right()), y + 1);
    } else {
        p->drawLine(x, int(rect.top()), x, int(rect.bottom()));
        p->setPen(QColor::fromRgbF(1, 1, 1, hiAlpha));
        p->drawLine(x + 1, int(rect.top()), x + 1, int(rect.bottom()));
    }
    p->restore();
}

void drawHandleDots(QPainter *p, const QRectF &rect, Qt::Orientation lineAxis, const QColor &dark)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    const QPointF m = rect.center();
    for (int i = -1; i <= 1; ++i) {
        const QPointF c = lineAxis == Qt::Vertical ? QPointF(m.x(), m.y() + i * 4) : QPointF(m.x() + i * 4, m.y());
        p->setBrush(QColor::fromRgbF(1, 1, 1, 0.75));
        p->drawEllipse(c + QPointF(0.7, 0.7), 1.1, 1.1);
        p->setBrush(dark);
        p->drawEllipse(c, 1.1, 1.1);
    }
    p->restore();
}

void drawArrow(QPainter *p, const QRectF &rect, Qt::ArrowType dir, const QColor &c, bool enabled)
{
    const qreal s = qBound<qreal>(2.5, qMin(rect.width(), rect.height()) * 0.2, 4.5);   // half-extent
    const QPointF m = rect.center();
    QPolygonF poly;
    switch (dir) {
    case Qt::UpArrow:    poly << QPointF(m.x() - s, m.y() + s * .5) << QPointF(m.x() + s, m.y() + s * .5) << QPointF(m.x(), m.y() - s * .6); break;
    case Qt::DownArrow:  poly << QPointF(m.x() - s, m.y() - s * .5) << QPointF(m.x() + s, m.y() - s * .5) << QPointF(m.x(), m.y() + s * .6); break;
    case Qt::LeftArrow:  poly << QPointF(m.x() + s * .5, m.y() - s) << QPointF(m.x() + s * .5, m.y() + s) << QPointF(m.x() - s * .6, m.y()); break;
    case Qt::RightArrow: poly << QPointF(m.x() - s * .5, m.y() - s) << QPointF(m.x() - s * .5, m.y() + s) << QPointF(m.x() + s * .6, m.y()); break;
    default: return;
    }
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    QColor fill = c;
    if (!enabled) fill.setAlphaF(0.4);
    p->setPen(QPen(fill, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p->setBrush(fill);
    p->drawPolygon(poly);
    p->restore();
}

void drawGrip(QPainter *p, const QRectF &rect, Qt::Orientation lineAxis, const QColor &c)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing, false);
    const QPointF m = rect.center();
    const qreal len = qMin(qMax(rect.width(), rect.height()) * 0.5, 8.0);
    QColor light = QColor::fromRgbF(1, 1, 1, 0.7);
    for (int i = -1; i <= 1; ++i) {
        const qreal o = i * 3;
        QLineF dark = lineAxis == Qt::Horizontal ? QLineF(m.x() - len / 2, m.y() + o, m.x() + len / 2, m.y() + o)
                                                 : QLineF(m.x() + o, m.y() - len / 2, m.x() + o, m.y() + len / 2);
        p->setPen(c);
        p->drawLine(dark.translated(0, 0));
        p->setPen(light);
        p->drawLine(lineAxis == Qt::Horizontal ? dark.translated(0, 1) : dark.translated(1, 0));
    }
    p->restore();
}

void drawFocusGlow(QPainter *p, const QRectF &rect, qreal radius, const QColor &c)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setBrush(Qt::NoBrush);
    const qreal sn = snap(p);
    for (int i = 2; i >= 0; --i) {
        QColor a = c;
        a.setAlphaF(0.18 + 0.22 * (2 - i) / 2.0);
        p->setPen(QPen(a, 1));
        const qreal off = sn + (2 - i);
        p->drawPath(rr(rect.adjusted(off, off, -off, -off), qMax<qreal>(0, radius - (2 - i))));
    }
    p->restore();
}

} // namespace Glass
