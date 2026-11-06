#include "glass.h"
#include "module.h"
#include <QPainterPath>
#include <QStyleOptionProgressBar>

namespace {
struct Progress : Module {
    const char *name() const override { return "progress"; }

    // Soft white band sweeping along the fill (direction `dir`: unit vector in device space).
    static void shine(QPainter *p, const QRectF &fill, qreal radius, bool horiz, bool reverse, qreal u)
    {
        const qreal len = horiz ? fill.width() : fill.height();
        const qreal bw = qMax<qreal>(24, len * 0.25);
        const qreal v = u * 1.5;                              // sweep during the first 2/3 of the period, then rest
        if (v >= 1.0 || len < 8) return;
        const qreal pos = -bw + (len + bw) * (reverse ? 1.0 - v : v);
        p->save();
        QPainterPath clip;
        clip.addRoundedRect(fill, radius, radius);
        p->setClipPath(clip, Qt::IntersectClip);
        QLinearGradient g = horiz ? QLinearGradient(fill.left() + pos, 0, fill.left() + pos + bw, 0)
                                  : QLinearGradient(0, fill.bottom() - pos, 0, fill.bottom() - pos - bw);
        g.setColorAt(0.0, QColor::fromRgbF(1, 1, 1, 0.0));
        g.setColorAt(0.5, QColor::fromRgbF(1, 1, 1, 0.38));
        g.setColorAt(1.0, QColor::fromRgbF(1, 1, 1, 0.0));
        p->setPen(Qt::NoPen);
        p->setBrush(g);
        p->drawRect(fill);
        p->restore();
    }

    bool drawControl(QStyle::ControlElement ce, const QStyleOption *o, QPainter *p,
                     const QWidget *w, const Theme &t) const override
    {
        const auto *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(o);
        if (!pb) return false;
        const QPalette &pal = o->palette;
        const qreal rad = t.num("progress.radius", 5);

        if (ce == QStyle::CE_ProgressBarGroove) {
            Glass::drawInset(p, o->rect, rad, t.color("progress.groove", pal, pal.button().color()),
                             t.color("input.border", pal, pal.dark().color()));
            return true;
        }
        if (ce == QStyle::CE_ProgressBarContents) {
            const bool horiz = pb->state & QStyle::State_Horizontal;
            QRectF r = QRectF(o->rect);
            const int span = pb->maximum - pb->minimum;
            const bool busy = span == 0;                            // QProgressBar busy indicator (min == max == 0)
            const bool animate = !anim || anim->enabled();
            const qreal ratio = span > 0 ? qreal(pb->progress - pb->minimum) / span : 1.0;
            if (ratio <= 0) return true;
            if (busy) {                                             // marquee: a glass segment runs through the groove
                const qreal len = horiz ? r.width() : r.height();
                const qreal seg = qMax<qreal>(28, len * 0.30);
                const qreal u = anim ? anim->phase(w, int(t.num("progress.marqueeMs", 1500))) : 0.5;
                qreal pos = -seg + (len + seg) * (animate ? u : 0.5 - seg / (2 * (len + seg)));
                if (pb->invertedAppearance) pos = len - pos - seg;
                QRectF sr = horiz ? QRectF(r.left() + pos, r.top(), seg, r.height())
                                  : QRectF(r.left(), r.bottom() - pos - seg, r.width(), seg);
                Glass::Spec s;
                s.base = t.color("progress.fill", pal, QColor("#4cc94c"));
                s.glow = t.color("progress.glow", pal, QColor("#b6ffb0"));
                s.border = s.base.darker(150);
                s.radius = rad;
                s.gloss = t.num("gloss", 0.55);
                s.enabled = pb->state & QStyle::State_Enabled;
                p->save();
                QPainterPath clip;
                clip.addRoundedRect(r, rad, rad);
                p->setClipPath(clip, Qt::IntersectClip);
                Glass::drawSlab(p, sr, s);
                p->restore();
                return true;
            }
            if (horiz) {
                const qreal w = r.width() * ratio;
                r = pb->invertedAppearance ? QRectF(r.right() - w, r.top(), w, r.height())
                                           : QRectF(r.left(), r.top(), w, r.height());
            } else {
                const qreal h = r.height() * ratio;
                r = pb->invertedAppearance ? QRectF(r.left(), r.top(), r.width(), h)
                                           : QRectF(r.left(), r.bottom() - h, r.width(), h);
            }
            Glass::Spec s;
            s.base = t.color("progress.fill", pal, QColor("#4cc94c"));
            s.glow = t.color("progress.glow", pal, QColor("#b6ffb0"));
            s.border = s.base.darker(150);
            s.radius = rad;
            s.gloss = t.num("gloss", 0.55);
            s.enabled = pb->state & QStyle::State_Enabled;
            Glass::drawSlab(p, r, s);
            if (anim && anim->enabled() && t.num("progress.shine", 1) > 0.5 && pb->progress < pb->maximum
                && (pb->state & QStyle::State_Enabled))
                shine(p, r, rad, horiz, pb->invertedAppearance, anim->phase(w, int(t.num("progress.shineMs", 2600))));
            return true;
        }
        return false;
    }
};
}

void registerProgress(QVector<Module *> &v) { v.append(new Progress); }
