#include "glass.h"
#include "module.h"
#include <QAbstractSpinBox>

// SpinBox + ComboBox.
//  - frame  : same inset as QLineEdit (tokens input.*) so they sit next to line edits seamlessly
//  - buttons: small glass slabs (tokens spin.* / combo.*)
//  - non-editable combo: full glass button (like a push button) + arrow
// Geometry comes from Fusion; we only override the arrow-button widths.
namespace {
struct SpinCombo : Module {
    const char *name() const override { return "spincombo"; }

    static QColor arrowColor(const QPalette &pal, bool enabled)
    {
        const QColor c = pal.color(enabled ? QPalette::Active : QPalette::Disabled, QPalette::ButtonText);
        return Theme::mix(c, pal.button().color(), 0.25);
    }
    // One glass column; `hot` (optional) is overlaid with an accent tint for hover/press.
    void column(QPainter *p, const QRectF &col, const QRectF &hot, bool pressed, const QColor &accent,
                const QPalette &pal, const Theme &t, bool enabled, qreal hoverT) const
    {
        Glass::Spec s;
        s.base = t.color("spin.button", pal, pal.button().color());
        s.border = t.color("input.border", pal, pal.dark().color());
        s.glow = accent;
        s.radius = t.num("spin.radius", 3);
        s.gloss = t.num("gloss", 0.55) * 0.8;
        s.enabled = enabled; s.hovered = hoverT > 0.5; s.hoverT = hoverT;
        Glass::drawSlab(p, col, s);
        if (!hot.isNull() && enabled) {
            p->save();
            p->setRenderHint(QPainter::Antialiasing);
            QColor a = accent; a.setAlphaF(pressed ? 0.45 : 0.22);
            p->setPen(Qt::NoPen); p->setBrush(a);
            p->setClipRect(col.adjusted(1, 1, -1, -1));
            p->drawRect(hot);
            p->restore();
        }
    }

    bool drawComplex(QStyle::ComplexControl cc, const QStyleOptionComplex *o, QPainter *p,
                     const QWidget *w, const Theme &t) const override
    {
        if (cc == QStyle::CC_SpinBox) return spin(o, p, w, t);
        if (cc == QStyle::CC_ComboBox) return combo(o, p, w, t);
        return false;
    }

    // ---------------------------------------------------------------- spin
    bool spin(const QStyleOptionComplex *o, QPainter *p, const QWidget *w, const Theme &t) const
    {
        const auto *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(o);
        if (!sb) return false;
        const QPalette &pal = o->palette;
        const bool enabled = sb->state & QStyle::State_Enabled;
        const QColor accent = t.color("accent", pal, pal.highlight().color());
        const qreal r = t.num("input.radius", 5);

        if (sb->frame) {
            Glass::drawInset(p, o->rect, r, t.color("input.base", pal, pal.base().color()),
                             t.color("input.border", pal, pal.dark().color()));
            if (sb->state & QStyle::State_HasFocus)
                Glass::drawFocusGlow(p, o->rect, r, t.color("input.focus", pal, accent));
        }
        if (sb->buttonSymbols == QAbstractSpinBox::NoButtons) return true;

        const QRect up = self->subControlRect(QStyle::CC_SpinBox, sb, QStyle::SC_SpinBoxUp, w);
        const QRect dn = self->subControlRect(QStyle::CC_SpinBox, sb, QStyle::SC_SpinBoxDown, w);
        if (up.isEmpty() || dn.isEmpty()) return true;
        const QRectF col = QRectF(up.united(dn)).adjusted(0, 1, -1, -1);
        const auto act = sb->activeSubControls;
        const bool pressed = sb->state & QStyle::State_Sunken;
        const bool hov = sb->state & QStyle::State_MouseOver;
        QRectF hot;
        if (act & QStyle::SC_SpinBoxUp) hot = QRectF(up);
        else if (act & QStyle::SC_SpinBoxDown) hot = QRectF(dn);
        column(p, col, hot, pressed, accent, pal, t, enabled, fade(w, 0, hov && enabled));
        // divider
        p->save();
        QColor sep = pal.dark().color(); sep.setAlphaF(0.35);
        p->setPen(sep);
        const qreal y = (up.bottom() + dn.top()) / 2.0 + 0.5;
        p->drawLine(QPointF(col.left() + 1, y), QPointF(col.right() - 1, y));
        p->restore();
        const bool canUp = enabled && (sb->stepEnabled & QAbstractSpinBox::StepUpEnabled);
        const bool canDn = enabled && (sb->stepEnabled & QAbstractSpinBox::StepDownEnabled);
        Glass::drawArrow(p, up, Qt::UpArrow, arrowColor(pal, canUp), canUp);
        Glass::drawArrow(p, dn, Qt::DownArrow, arrowColor(pal, canDn), canDn);
        return true;
    }

    // ---------------------------------------------------------------- combo
    bool combo(const QStyleOptionComplex *o, QPainter *p, const QWidget *w, const Theme &t) const
    {
        const auto *cb = qstyleoption_cast<const QStyleOptionComboBox *>(o);
        if (!cb) return false;
        const QPalette &pal = o->palette;
        const auto st = cb->state;
        const bool enabled = st & QStyle::State_Enabled;
        const QColor accent = t.color("accent", pal, pal.highlight().color());
        const QRect arrowRect = self->subControlRect(QStyle::CC_ComboBox, cb, QStyle::SC_ComboBoxArrow, w);
        const bool popupOpen = st & QStyle::State_On;
        const bool arrowDown = popupOpen || ((cb->activeSubControls & QStyle::SC_ComboBoxArrow) && (st & QStyle::State_Sunken));
        const bool hov = st & QStyle::State_MouseOver;

        if (cb->editable) {
            const qreal r = t.num("input.radius", 5);
            if (cb->frame) {
                Glass::drawInset(p, o->rect, r, t.color("input.base", pal, pal.base().color()),
                                 t.color("input.border", pal, pal.dark().color()));
                if (st & QStyle::State_HasFocus)
                    Glass::drawFocusGlow(p, o->rect, r, t.color("input.focus", pal, accent));
            }
            column(p, QRectF(arrowRect).adjusted(0, 1, -1, -1), arrowDown || hov ? QRectF(arrowRect) : QRectF(),
                   arrowDown, accent, pal, t, enabled, fade(w, 0, hov && enabled));
        } else {
            // whole control = glass button
            const bool down = popupOpen || (st & QStyle::State_Sunken);
            QColor base = t.color("button.base", pal, pal.button().color());
            if (down) base = Theme::mix(base, accent, 0.35);
            Glass::Spec s;
            s.base = base;
            s.border = t.color("button.border", pal, pal.dark().color());
            s.glow = t.color("button.glow", pal, accent);
            s.radius = t.num("button.radius", 6);
            s.gloss = t.num("gloss", 0.55);
            s.pressed = down; s.hovered = hov; s.enabled = enabled;
            s.hoverT = fade(w, 0, hov && enabled); if (!down) s.hoverTint = 0.15;
            Glass::drawSlab(p, o->rect, s);
            // subtle separator in front of the arrow
            p->save();
            QColor sep = pal.dark().color(); sep.setAlphaF(0.25);
            p->setPen(sep);
            const qreal x = arrowRect.left() + 0.5;
            p->drawLine(QPointF(x, o->rect.top() + 5), QPointF(x, o->rect.bottom() - 5));
            p->restore();
            if ((st & QStyle::State_HasFocus) && (st & QStyle::State_KeyboardFocusChange))
                Glass::drawFocusGlow(p, o->rect, s.radius, accent);
        }
        Glass::drawArrow(p, arrowRect, Qt::DownArrow, arrowColor(pal, enabled), enabled);
        return true;
    }
};
}

void registerSpinCombo(QVector<Module *> &v) { v.append(new SpinCombo); }
