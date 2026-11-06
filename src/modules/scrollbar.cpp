#include "glass.h"
#include "module.h"

// Scrollbar: inset track, glass thumb with grip lines, glass arrow buttons.
// Geometry (which rects exist) comes from the base style via self->subControlRect;
// we only own the extent/min-thumb metrics and the painting.
namespace {
struct Scrollbar : Module {
    const char *name() const override { return "scrollbar"; }

    int pixelMetric(QStyle::PixelMetric pm, const QStyleOption *, const QWidget *, const Theme &t) const override
    {
        switch (pm) {
        case QStyle::PM_ScrollBarExtent:        return int(t.num("scrollbar.extent", 17));
        case QStyle::PM_ScrollBarSliderMin:     return int(t.num("scrollbar.minThumb", 28));
        default: return -1;
        }
    }

    bool drawComplex(QStyle::ComplexControl cc, const QStyleOptionComplex *o, QPainter *p,
                     const QWidget *w, const Theme &t) const override
    {
        if (cc != QStyle::CC_ScrollBar) return false;
        const auto *sb = qstyleoption_cast<const QStyleOptionSlider *>(o);
        if (!sb) return false;

        const QPalette &pal = o->palette;
        const bool horiz = sb->orientation == Qt::Horizontal;
        const bool enabled = sb->state & QStyle::State_Enabled;
        const qreal rad = t.num("scrollbar.radius", 4);
        const QColor accent = t.color("accent", pal, pal.highlight().color());
        const QColor border = t.color("scrollbar.border", pal, pal.dark().color());

        // track
        Glass::drawInset(p, o->rect, rad, t.color("scrollbar.groove", pal, pal.button().color()),
                         t.color("input.border", pal, pal.dark().color()));

        auto pressedOn = [&](QStyle::SubControl sc) { return (sb->activeSubControls & sc) && (sb->state & QStyle::State_Sunken); };
        auto hoverOn = [&](QStyle::SubControl sc) { return (sb->activeSubControls & sc) && (sb->state & QStyle::State_MouseOver); };

        // arrow buttons
        auto button = [&](QStyle::SubControl sc, Qt::ArrowType arrow) {
            const QRect r = self->subControlRect(QStyle::CC_ScrollBar, sb, sc, w);
            if (!r.isValid() || r.isEmpty()) return;
            const bool down = pressedOn(sc), hov = hoverOn(sc);
            Glass::Spec s;
            s.base = t.color("scrollbar.thumb", pal, pal.button().color());
            if (down) s.base = Theme::mix(s.base, accent, 0.35);
            s.border = border;
            s.glow = accent;
            s.radius = rad;
            s.gloss = t.num("gloss", 0.55) * 0.8;
            s.pressed = down; s.hovered = hov; s.enabled = enabled;
            s.hoverT = fade(w, sc, hov); if (!down) s.hoverTint = 0.15;
            s.axis = horiz ? Qt::Vertical : Qt::Horizontal;
            Glass::drawSlab(p, QRectF(r).adjusted(1, 1, -1, -1), s);
            Glass::drawArrow(p, r, arrow, Theme::mix(pal.color(enabled ? QPalette::Active : QPalette::Disabled, QPalette::ButtonText), pal.button().color(), 0.25), enabled);
        };
        const bool rtl = horiz && sb->direction == Qt::RightToLeft;
        button(QStyle::SC_ScrollBarSubLine, horiz ? (rtl ? Qt::RightArrow : Qt::LeftArrow) : Qt::UpArrow);
        button(QStyle::SC_ScrollBarAddLine, horiz ? (rtl ? Qt::LeftArrow : Qt::RightArrow) : Qt::DownArrow);

        // thumb
        const QRect tr = self->subControlRect(QStyle::CC_ScrollBar, sb, QStyle::SC_ScrollBarSlider, w);
        if (enabled && tr.isValid() && !tr.isEmpty()) {
            const bool down = pressedOn(QStyle::SC_ScrollBarSlider);
            const bool hov = hoverOn(QStyle::SC_ScrollBarSlider);
            Glass::Spec s;
            s.base = t.color("scrollbar.thumb", pal, pal.button().color());
            if (down) s.base = Theme::mix(s.base, accent, 0.35);
            s.border = border;
            s.glow = accent;
            s.radius = rad;
            s.gloss = t.num("gloss", 0.55);
            s.pressed = down; s.hovered = hov; s.enabled = true;
            s.hoverT = fade(w, QStyle::SC_ScrollBarSlider, hov); if (!down) s.hoverTint = 0.15;
            // gradient runs across the thumb's short side
            s.axis = horiz ? Qt::Vertical : Qt::Horizontal;
            const QRectF slab = QRectF(tr).adjusted(horiz ? 0 : 1, horiz ? 1 : 0, horiz ? 0 : -1, horiz ? -1 : 0);
            Glass::drawSlab(p, slab, s);
            if ((horiz ? tr.width() : tr.height()) >= 36)
                Glass::drawGrip(p, slab, horiz ? Qt::Vertical : Qt::Horizontal,
                                t.color("scrollbar.grip", pal, pal.dark().color()));
        }
        return true;
    }
};
}

void registerScrollbar(QVector<Module *> &v) { v.append(new Scrollbar); }
