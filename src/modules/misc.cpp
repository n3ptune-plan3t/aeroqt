#include "glass.h"
#include "module.h"
#include <QPainterPath>
#include <QStyleOptionRubberBand>
#include <QStyleOptionSlider>
#include <QStyleOptionToolBox>
#include <QtMath>

// Odds and ends that each need only one painter:
//  - PE_IndicatorBranch : Win7-style tree expanders (hollow arrow closed, solid corner triangle open),
//                         no dotted connector lines
//  - CC_Dial            : glass knob in an inset ring, accent handle dot, optional tick marks
//  - CE_ToolBoxTabShape : glass slab per QToolBox page header
//  - CE_RubberBand      : translucent accent selection rectangle (SH_RubberBand_Mask = none)
namespace {
struct Misc : Module {
    const char *name() const override { return "misc"; }

    int styleHint(QStyle::StyleHint sh, const QStyleOption *, const QWidget *, QStyleHintReturn *,
                  const Theme &) const override
    {
        return sh == QStyle::SH_RubberBand_Mask ? 0 : Unhandled;   // we blend with alpha, no mask
    }

    // ------------------------------------------------------------ primitives
    bool drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *o, QPainter *p,
                       const QWidget *, const Theme &t) const override
    {
        if (pe != QStyle::PE_IndicatorBranch) return false;
        const auto st = o->state;
        if (!(st & QStyle::State_Children)) return true;           // no connector lines
        const QPalette &pal = o->palette;
        const bool open = st & QStyle::State_Open;
        const bool hover = st & QStyle::State_MouseOver;
        const bool rtl = o->direction == Qt::RightToLeft;
        const QColor accent = t.color("accent", pal, pal.highlight().color());
        const QPointF c = QRectF(o->rect).center();
        const qreal s = 3.6;
        const qreal dx = rtl ? -1 : 1;                              // mirror for RTL

        QPolygonF poly;
        if (open)   // solid triangle in the lower "corner" (SE, or SW in RTL)
            poly << QPointF(c.x() + dx * s, c.y() - s) << QPointF(c.x() + dx * s, c.y() + s) << QPointF(c.x() - dx * s, c.y() + s);
        else        // hollow arrow pointing at the content
            poly << QPointF(c.x() - dx * s * 0.6, c.y() - s) << QPointF(c.x() - dx * s * 0.6, c.y() + s)
                 << QPointF(c.x() + dx * s * 0.8, c.y());

        const QColor ink = Theme::mix(pal.text().color(), pal.base().color(), 0.45);
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        if (open) {
            p->setPen(Qt::NoPen);
            p->setBrush(hover ? accent : ink);
        } else {
            p->setPen(QPen(hover ? accent : Theme::mix(ink, pal.base().color(), 0.2), 1.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            p->setBrush(hover ? Theme::mix(pal.base().color(), accent, 0.25) : pal.base().color());
        }
        p->drawPolygon(poly);
        p->restore();
        return true;
    }

    // ------------------------------------------------------------ controls
    bool drawControl(QStyle::ControlElement ce, const QStyleOption *o, QPainter *p,
                     const QWidget *w, const Theme &t) const override
    {
        const QPalette &pal = o->palette;
        const QColor accent = t.color("accent", pal, pal.highlight().color());
        if (ce == QStyle::CE_RubberBand) {
            if (!qstyleoption_cast<const QStyleOptionRubberBand *>(o)) return false;
            QColor fill = accent;
            fill.setAlpha(60);
            p->save();
            p->setPen(accent);
            p->setBrush(fill);
            p->drawRect(QRectF(o->rect).adjusted(0.5, 0.5, -0.5, -0.5));
            p->restore();
            return true;
        }
        if (ce == QStyle::CE_ToolBoxTabShape) {
            const auto *tb = qstyleoption_cast<const QStyleOptionToolBox *>(o);
            if (!tb) return false;
            const auto st = o->state;
            const bool selected = st & QStyle::State_Selected;
            Glass::Spec s;
            s.base = selected ? t.color("tab.selected", pal, Theme::mix(pal.button().color(), accent, 0.18))
                              : t.color("tab.base", pal, Theme::mix(pal.button().color(), pal.dark().color(), 0.10));
            s.border = t.color("tab.border", pal, pal.dark().color());
            s.glow = accent;
            s.radius = 3;
            s.gloss = t.num("gloss", 0.55);
            s.hovered = st & QStyle::State_MouseOver;
            s.hoverT = fade(w, quintptr(o->rect.y() & 0xffff), (st & QStyle::State_MouseOver) && (st & QStyle::State_Enabled));
            s.pressed = st & QStyle::State_Sunken;
            s.enabled = st & QStyle::State_Enabled;
            Glass::drawSlab(p, QRectF(o->rect).adjusted(0, 0, 0, -1), s);
            return true;
        }
        return false;
    }

    // ------------------------------------------------------------ dial
    bool drawComplex(QStyle::ComplexControl cc, const QStyleOptionComplex *o, QPainter *p,
                     const QWidget *w, const Theme &t) const override
    {
        if (cc != QStyle::CC_Dial) return false;
        const auto *d = qstyleoption_cast<const QStyleOptionSlider *>(o);
        if (!d) return false;
        const QPalette &pal = d->palette;
        const auto st = d->state;
        const bool enabled = st & QStyle::State_Enabled;
        const QColor accent = t.color("accent", pal, pal.highlight().color());

        const qreal side = qMin(d->rect.width(), d->rect.height()) - 2;
        if (side < 12) return true;
        const QPointF c = QRectF(d->rect).center();
        const bool ticks = (d->subControls & QStyle::SC_DialTickmarks) && d->tickInterval > 0;
        const qreal ringD = ticks ? side - 10 : side;
        const QRectF ring(c.x() - ringD / 2, c.y() - ringD / 2, ringD, ringD);
        const QRectF knob = ring.adjusted(4, 4, -4, -4);

        const qreal range = d->maximum - d->minimum;
        const qreal progress = range > 0 ? (d->sliderPosition - d->minimum) / range : 0;
        const bool wrap = d->dialWrapping;
        const qreal angle = wrap ? M_PI * 1.5 - progress * 2 * M_PI : (8 * M_PI - progress * 10 * M_PI) / 6;

        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        if (ticks) {                                               // notches just outside the ring
            const int n = qMin<int>(200, range / d->tickInterval);
            p->setPen(QPen(Theme::mix(pal.window().color(), pal.dark().color(), 0.6), 1.2, Qt::SolidLine, Qt::RoundCap));
            for (int i = 0; i <= n; ++i) {
                const qreal pr = n ? qreal(i) / n : 0;
                const qreal a = wrap ? M_PI * 1.5 - pr * 2 * M_PI : (8 * M_PI - pr * 10 * M_PI) / 6;
                const qreal r1 = ringD / 2 + 2, r2 = ringD / 2 + 5;
                p->drawLine(QPointF(c.x() + r1 * qCos(a), c.y() - r1 * qSin(a)),
                            QPointF(c.x() + r2 * qCos(a), c.y() - r2 * qSin(a)));
            }
        }
        Glass::drawInset(p, ring, ringD / 2, Theme::mix(pal.window().color(), pal.dark().color(), 0.12),
                         t.color("input.border", pal, pal.dark().color()));

        Glass::Spec s;
        s.base = t.color("button.base", pal, pal.button().color());
        s.border = t.color("button.border", pal, pal.dark().color());
        s.glow = accent;
        s.radius = knob.width() / 2;
        s.gloss = t.num("gloss", 0.55);
        s.hovered = st & QStyle::State_MouseOver;
        s.hoverT = fade(w, 0, (st & QStyle::State_MouseOver) && enabled);
        s.pressed = st & QStyle::State_Sunken;
        s.enabled = enabled;
        Glass::drawSlab(p, knob, s);

        // handle dot
        const qreal hr = knob.width() / 2 * 0.68, dotR = qMax<qreal>(2.5, knob.width() * 0.07);
        const QPointF h(c.x() + hr * qCos(angle), c.y() - hr * qSin(angle));
        p->setPen(Qt::NoPen);
        p->setBrush(QColor::fromRgbF(1, 1, 1, 0.8));
        p->drawEllipse(h + QPointF(0.6, 0.9), dotR, dotR);
        p->setBrush(enabled ? Theme::mix(accent, pal.dark().color(), 0.35) : pal.color(QPalette::Disabled, QPalette::Mid));
        p->drawEllipse(h, dotR, dotR);

        if (st & QStyle::State_HasFocus) {
            QColor f = accent;
            f.setAlphaF(0.7);
            p->setPen(QPen(f, 1.4, Qt::DotLine));
            p->setBrush(Qt::NoBrush);
            p->drawEllipse(ring.adjusted(-1, -1, 1, 1));
        }
        p->restore();
        return true;
    }
};
}

void registerMisc(QVector<Module *> &v) { v.append(new Misc); }
