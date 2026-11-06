#include "glass.h"
#include "module.h"
#include <QPainterPath>
#include <QRadialGradient>

namespace {
struct Inputs : Module {
    const char *name() const override { return "inputs"; }

    bool drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *o, QPainter *p,
                       const QWidget *w, const Theme &t) const override
    {
        const QPalette &pal = o->palette;
        const auto st = o->state;
        const QColor accent = t.color("accent", pal, pal.highlight().color());

        switch (pe) {
        case QStyle::PE_PanelLineEdit: {
            const auto *f = qstyleoption_cast<const QStyleOptionFrame *>(o);
            const qreal r = t.num("input.radius", 5);
            const QColor fill = t.color("input.base", pal, pal.base().color());
            const QColor border = t.color("input.border", pal, pal.dark().color());
            Glass::drawInset(p, o->rect, r, fill, (f && f->lineWidth == 0) ? Qt::transparent : border);
            if (st & QStyle::State_HasFocus)
                Glass::drawFocusGlow(p, o->rect, r, t.color("input.focus", pal, accent));
            return true;
        }
        case QStyle::PE_IndicatorCheckBox:
        case QStyle::PE_IndicatorRadioButton: {
            const bool radio = pe == QStyle::PE_IndicatorRadioButton;
            const bool on = st & QStyle::State_On;
            const bool tri = st & QStyle::State_NoChange;
            Glass::Spec s;
            s.base = t.color("check.base", pal, pal.base().color());
            if (st & QStyle::State_Sunken) s.base = Theme::mix(s.base, accent, 0.3);
            s.border = t.color("input.border", pal, pal.dark().color());
            s.glow = accent;
            s.radius = radio ? o->rect.width() / 2.0 : t.num("check.radius", 3);
            s.gloss = t.num("gloss", 0.55) * 0.8;
            s.hovered = st & QStyle::State_MouseOver;
            s.hoverT = fade(w, 0, (st & QStyle::State_MouseOver) && (st & QStyle::State_Enabled));
            s.enabled = st & QStyle::State_Enabled;
            Glass::drawSlab(p, o->rect, s);

            const QColor mark = t.color("check.mark", pal, accent);
            p->save();
            p->setRenderHint(QPainter::Antialiasing);
            const QRectF r = QRectF(o->rect).adjusted(0.5, 0.5, -0.5, -0.5);
            if (radio && on) {
                const QRectF d = r.adjusted(r.width() * .28, r.height() * .28, -r.width() * .28, -r.height() * .28);
                QRadialGradient g(d.center() - QPointF(d.width() * .15, d.height() * .2), d.width());
                g.setColorAt(0, mark.lighter(170));
                g.setColorAt(1, mark.darker(120));
                p->setPen(Qt::NoPen);
                p->setBrush(g);
                p->drawEllipse(d);
            } else if (!radio && (on || tri)) {
                p->setBrush(Qt::NoBrush);
                p->setPen(QPen(mark.darker(115), qMax<qreal>(1.6, r.width() * 0.16), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                if (tri) p->drawLine(r.center() + QPointF(-r.width() * .25, 0), r.center() + QPointF(r.width() * .25, 0));
                else {
                    QPainterPath c;
                    c.moveTo(r.left() + r.width() * .24, r.top() + r.height() * .54);
                    c.lineTo(r.left() + r.width() * .43, r.top() + r.height() * .72);
                    c.lineTo(r.left() + r.width() * .78, r.top() + r.height() * .28);
                    p->drawPath(c);
                }
            }
            p->restore();
            return true;
        }
        default: return false;
        }
    }
};
}

void registerInputs(QVector<Module *> &v) { v.append(new Inputs); }
