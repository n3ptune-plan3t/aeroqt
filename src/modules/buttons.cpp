#include "glass.h"
#include "module.h"

namespace {
struct Buttons : Module {
    const char *name() const override { return "buttons"; }

    bool drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *o, QPainter *p,
                       const QWidget *w, const Theme &t) const override
    {
        if (pe != QStyle::PE_PanelButtonCommand && pe != QStyle::PE_PanelButtonTool
            && pe != QStyle::PE_PanelButtonBevel)
            return false;

        const auto st = o->state;
        const bool down = st & (QStyle::State_Sunken | QStyle::State_On);
        const bool hover = st & QStyle::State_MouseOver;
        const bool enabled = st & QStyle::State_Enabled;

        // flat tool buttons only show glass on interaction (fading in/out with the hover factor)
        const qreal hv = fade(w, 0, hover && enabled);
        const bool flat = pe == QStyle::PE_PanelButtonTool && (st & QStyle::State_AutoRaise) && !down;
        const bool kbFocus = (st & QStyle::State_HasFocus) && (st & QStyle::State_KeyboardFocusChange);

        const QPalette &pal = o->palette;
        const QColor accent = t.color("accent", pal, pal.highlight().color());
        if (flat && hv <= 0.002) {
            // An idle flat tool button paints no slab, but a keyboard-focused one must
            // still show where focus is (focus must never depend on hover).
            if (kbFocus) Glass::drawFocusGlow(p, o->rect, t.num("button.radius", 6), accent);
            return true;
        }
        QColor base = t.color("button.base", pal, pal.button().color());
        if (const auto *b = qstyleoption_cast<const QStyleOptionButton *>(o))
            if (b->features & QStyleOptionButton::DefaultButton) base = Theme::mix(base, accent, 0.25);
        if (down) base = Theme::mix(base, accent, 0.35);

        Glass::Spec s;
        s.base = base;
        s.border = t.color("button.border", pal, pal.dark().color());
        s.glow = t.color("button.glow", pal, accent);
        s.radius = t.num("button.radius", 6);
        s.gloss = t.num("gloss", 0.55);
        s.pressed = down;
        s.hovered = hover;
        s.hoverT = hv;
        if (!down) s.hoverTint = 0.15;
        s.enabled = enabled;
        if (flat) { p->save(); p->setOpacity(p->opacity() * hv); }
        Glass::drawSlab(p, o->rect, s);
        if (flat) p->restore();
        if (kbFocus) Glass::drawFocusGlow(p, o->rect, s.radius, accent);
        return true;
    }
};
}

void registerButtons(QVector<Module *> &v) { v.append(new Buttons); }
