#include "glass.h"
#include "module.h"
#include <QStyleOptionFrame>
#include <QStyleOptionGroupBox>

// Frames: group box, generic QFrame panel, tooltip, keyboard focus rectangle.
//  - CC_GroupBox        : glass panel + title + checkbox (geometry from Fusion)
//  - PE_Frame           : 1px border for scroll areas / StyledPanel frames (no fill, no rounding:
//                         the viewport inside would clip rounded corners)
//  - PE_PanelTipLabel   : square tooltip (a tip window has no translucency => no rounded corners)
//  - PE_FrameFocusRect  : soft rounded accent outline (check/radio labels, item views)
namespace {
struct Frames : Module {
    const char *name() const override { return "frames"; }

    bool drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *o, QPainter *p,
                       const QWidget *, const Theme &t) const override
    {
        const QPalette &pal = o->palette;
        switch (pe) {
        case QStyle::PE_Frame: {
            if (!qstyleoption_cast<const QStyleOptionFrame *>(o)) return false;
            const QColor border = t.color("input.border", pal, pal.dark().color());
            p->save();
            const QRectF r = QRectF(o->rect).adjusted(0.5, 0.5, -0.5, -0.5);
            if (o->state & QStyle::State_Sunken) {               // faint inner top shadow
                p->setPen(QColor(0, 0, 0, 28));
                p->drawLine(QPointF(r.left() + 1, r.top() + 1.5), QPointF(r.right() - 1, r.top() + 1.5));
            }
            p->setPen(border);
            p->setBrush(Qt::NoBrush);
            p->drawRect(r);
            p->restore();
            // QTextEdit / QAbstractScrollArea / calendar / wizard pages: focus glow
            // (PE_Frame is how QFrame-based widgets ask for their border).
            if (o->state & QStyle::State_HasFocus)
                Glass::drawFocusGlow(p, o->rect, t.num("input.radius", 5),
                                     t.color("input.focus", pal, t.color("accent", pal, pal.highlight().color())));
            return true;
        }
        case QStyle::PE_PanelTipLabel: {
            const QColor fill = pal.toolTipBase().color();
            Glass::drawPanel(p, o->rect, 0, fill, Theme::mix(pal.toolTipText().color(), fill, 0.6));
            return true;
        }
        case QStyle::PE_FrameFocusRect: {
            const QColor accent = t.color("accent", pal, pal.highlight().color());
            QColor c = accent;
            c.setAlphaF(0.75);
            p->save();
            p->setRenderHint(QPainter::Antialiasing);
            p->setBrush(Qt::NoBrush);
            p->setPen(QPen(c, 1, Qt::DotLine));
            p->drawRoundedRect(QRectF(o->rect).adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);
            p->restore();
            return true;
        }
        default: return false;
        }
    }

    bool drawComplex(QStyle::ComplexControl cc, const QStyleOptionComplex *o, QPainter *p,
                     const QWidget *w, const Theme &t) const override
    {
        if (cc != QStyle::CC_GroupBox) return false;
        const auto *gb = qstyleoption_cast<const QStyleOptionGroupBox *>(o);
        if (!gb) return false;
        const QPalette &pal = o->palette;
        const bool enabled = gb->state & QStyle::State_Enabled;

        if (gb->subControls & QStyle::SC_GroupBoxFrame) {
            const QRect frame = self->subControlRect(QStyle::CC_GroupBox, gb, QStyle::SC_GroupBoxFrame, w);
            const QColor border = t.color("group.border", pal, pal.dark().color());
            if (gb->features & QStyleOptionFrame::Flat) {
                Glass::drawSeparator(p, QRectF(frame.left(), frame.top(), frame.width(), 2), Qt::Horizontal, border,
                                     Glass::edgeLight(pal.window().color()));
            } else {
                Glass::drawPanel(p, frame, t.num("group.radius", 5),
                                 t.color("group.base", pal, Theme::mix(pal.window().color(), pal.base().color(), 0.3)),
                                 border);
            }
        }
        if (gb->subControls & QStyle::SC_GroupBoxLabel) {
            const QRect label = self->subControlRect(QStyle::CC_GroupBox, gb, QStyle::SC_GroupBoxLabel, w);
            int flags = Qt::TextShowMnemonic | Qt::AlignLeft | Qt::AlignVCenter;   // label rect already hugs the text
            if (!self->styleHint(QStyle::SH_UnderlineShortcut, gb, w)) flags |= Qt::TextHideMnemonic;
            p->save();
            if (gb->textColor.isValid()) p->setPen(gb->textColor);
            self->drawItemText(p, label, flags, pal, enabled, gb->text,
                               gb->textColor.isValid() ? QPalette::NoRole : QPalette::WindowText);
            p->restore();
        }
        if (gb->subControls & QStyle::SC_GroupBoxCheckBox) {
            QStyleOptionButton box;
            box.QStyleOption::operator=(*gb);
            box.rect = self->subControlRect(QStyle::CC_GroupBox, gb, QStyle::SC_GroupBoxCheckBox, w);
            self->drawPrimitive(QStyle::PE_IndicatorCheckBox, &box, p, w);
        }
        return true;
    }
};
}

void registerFrames(QVector<Module *> &v) { v.append(new Frames); }
