#include "glass.h"
#include "module.h"
#include <QSlider>

// Slider: thin inset groove, accent-filled "progress" part, round glass handle, ticks.
namespace {
struct Slider : Module {
    const char *name() const override { return "slider"; }

    int pixelMetric(QStyle::PixelMetric pm, const QStyleOption *, const QWidget *, const Theme &t) const override
    {
        switch (pm) {
        case QStyle::PM_SliderLength:    return int(t.num("slider.handleLength", 18));
        case QStyle::PM_SliderThickness: return int(t.num("slider.handleThickness", 18)) + 4;
        default: return -1;
        }
    }

    bool drawComplex(QStyle::ComplexControl cc, const QStyleOptionComplex *o, QPainter *p,
                     const QWidget *w, const Theme &t) const override
    {
        if (cc != QStyle::CC_Slider) return false;
        const auto *sl = qstyleoption_cast<const QStyleOptionSlider *>(o);
        if (!sl) return false;

        const QPalette &pal = o->palette;
        const bool horiz = sl->orientation == Qt::Horizontal;
        const bool enabled = sl->state & QStyle::State_Enabled;
        const QColor accent = t.color("accent", pal, pal.highlight().color());
        const QColor border = t.color("slider.border", pal, pal.dark().color());
        const QRect handle = self->subControlRect(QStyle::CC_Slider, sl, QStyle::SC_SliderHandle, w);

        // ---- groove (centered thin bar, ignoring the tick space Fusion reserves) ----
        QRect gr = self->subControlRect(QStyle::CC_Slider, sl, QStyle::SC_SliderGroove, w);
        const int gw = int(t.num("slider.grooveWidth", 6));
        QRectF groove = horiz ? QRectF(gr.left(), handle.center().y() - gw / 2.0 + 0.5, gr.width(), gw)
                              : QRectF(handle.center().x() - gw / 2.0 + 0.5, gr.top(), gw, gr.height());
        if (sl->subControls & QStyle::SC_SliderGroove) {
            Glass::drawInset(p, groove, gw / 2.0, t.color("slider.groove", pal, pal.button().color()),
                             t.color("input.border", pal, pal.dark().color()));
            // filled part from the minimum end up to the handle centre
            if (enabled) {
                const QPointF c = QRectF(handle).center();
                const bool far = sl->upsideDown;            // min is at right (horiz) / bottom (vert)
                QRectF f = groove.adjusted(1, 1, -1, -1);
                if (horiz) { far ? f.setLeft(c.x()) : f.setRight(c.x()); }
                else       { far ? f.setTop(c.y())  : f.setBottom(c.y()); }
                if (f.width() > 1 && f.height() > 1) {
                    Glass::Spec s;
                    s.base = t.color("slider.fill", pal, accent);
                    s.border = s.base.darker(140);
                    s.glow = s.base.lighter(150);
                    s.radius = gw / 2.0 - 1;
                    s.gloss = t.num("gloss", 0.55) * 0.8;
                    s.axis = horiz ? Qt::Vertical : Qt::Horizontal;
                    Glass::drawSlab(p, f, s);
                }
            }
        }

        // ---- ticks ----
        if ((sl->subControls & QStyle::SC_SliderTickmarks) && sl->tickPosition != QSlider::NoTicks) {
            const int interval = sl->tickInterval > 0 ? sl->tickInterval : sl->pageStep;
            const int len = horiz ? handle.width() : handle.height();
            const int span = (horiz ? gr.width() : gr.height()) - len;
            const int start = (horiz ? gr.left() : gr.top()) + len / 2;
            if (interval > 0 && span > 0) {
                p->save();
                QColor tc = t.color("slider.tick", pal, pal.dark().color());
                tc.setAlphaF(enabled ? 0.8 : 0.4);
                p->setPen(tc);
                const int tl = 4;
                for (int v = sl->minimum; v <= sl->maximum; v += interval) {
                    const int pos = start + QStyle::sliderPositionFromValue(sl->minimum, sl->maximum, v, span, sl->upsideDown);
                    if (horiz) {
                        if (sl->tickPosition & 1) p->drawLine(pos, int(groove.top()) - 3 - tl, pos, int(groove.top()) - 3);        // above
                        if (sl->tickPosition & 2) p->drawLine(pos, int(groove.bottom()) + 3, pos, int(groove.bottom()) + 3 + tl); // below
                    } else {
                        if (sl->tickPosition & 1) p->drawLine(int(groove.left()) - 3 - tl, pos, int(groove.left()) - 3, pos);    // left
                        if (sl->tickPosition & 2) p->drawLine(int(groove.right()) + 3, pos, int(groove.right()) + 3 + tl, pos);  // right
                    }
                    if (interval <= 0) break;
                }
                p->restore();
            }
        }

        // ---- handle ----
        if (sl->subControls & QStyle::SC_SliderHandle) {
            const bool down = (sl->activeSubControls & QStyle::SC_SliderHandle) && (sl->state & QStyle::State_Sunken);
            const bool hov = (sl->activeSubControls & QStyle::SC_SliderHandle) && (sl->state & QStyle::State_MouseOver);
            Glass::Spec s;
            s.base = t.color("slider.handle", pal, pal.button().color());
            if (down) s.base = Theme::mix(s.base, accent, 0.35);
            s.border = border;
            s.glow = accent;
            // keep the handle a squarish round slab, centred in the rect Fusion gave us
            const qreal side = qMin(handle.width(), handle.height());
            QRectF hr(0, 0, horiz ? handle.width() : side, horiz ? side : handle.height());
            hr.moveCenter(QRectF(handle).center());
            s.radius = qMin(hr.width(), hr.height()) * 0.45;
            s.gloss = t.num("gloss", 0.55);
            s.pressed = down; s.hovered = hov; s.enabled = enabled;
            s.hoverT = fade(w, 0, hov); if (!down) s.hoverTint = 0.15;
            Glass::drawSlab(p, hr, s);
            if ((sl->state & QStyle::State_HasFocus) && (sl->state & QStyle::State_KeyboardFocusChange))
                Glass::drawFocusGlow(p, hr, s.radius, accent);
        }
        return true;
    }
};
}

void registerSlider(QVector<Module *> &v) { v.append(new Slider); }
