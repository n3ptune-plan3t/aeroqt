#include "glass.h"
#include "module.h"
#include <QStyleOptionDockWidget>

// Splitter handles, QMainWindow dock separators, dock title bars, floating dock frame.
//  - CE_Splitter + PE_IndicatorDockWidgetResizeHandle share one painter: transparent at rest,
//    accent wash + engraved dots on hover, stronger when dragged.
//    State_Horizontal means the handle is a vertical strip (widgets side by side).
//  - CE_DockWidgetTitle : glass bar + elided title (vertical title bars fall back to Fusion).
namespace {
struct Docks : Module {
    const char *name() const override { return "docks"; }

    int pixelMetric(QStyle::PixelMetric pm, const QStyleOption *, const QWidget *, const Theme &t) const override
    {
        return pm == QStyle::PM_SplitterWidth ? int(t.num("splitter.width", 6)) : -1;
    }

    static void handle(const QStyleOption *o, QPainter *p, const Theme &t)
    {
        const QPalette &pal = o->palette;
        const bool hover = o->state & QStyle::State_MouseOver;
        const bool down = o->state & QStyle::State_Sunken;
        const bool vertStrip = o->state & QStyle::State_Horizontal;
        if (hover || down) {
            QColor c = t.color("accent", pal, pal.highlight().color());
            c.setAlphaF(down ? 0.35 : 0.18);
            p->fillRect(o->rect, c);
        }
        Glass::drawHandleDots(p, o->rect, vertStrip ? Qt::Vertical : Qt::Horizontal,
                              t.color("splitter.dots", pal, Theme::mix(pal.window().color(), pal.dark().color(), 0.55)));
    }

    bool drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *o, QPainter *p,
                       const QWidget *, const Theme &t) const override
    {
        switch (pe) {
        case QStyle::PE_IndicatorDockWidgetResizeHandle:
            handle(o, p, t);
            return true;
        case QStyle::PE_FrameDockWidget: {                               // floating dock border only
            const QPalette &pal = o->palette;
            p->save();
            p->setPen(t.color("dock.border", pal, pal.dark().color()));
            p->setBrush(Qt::NoBrush);
            p->drawRect(QRectF(o->rect).adjusted(0.5, 0.5, -0.5, -0.5));
            p->restore();
            return true;
        }
        default: return false;
        }
    }

    bool drawControl(QStyle::ControlElement ce, const QStyleOption *o, QPainter *p,
                     const QWidget *w, const Theme &t) const override
    {
        if (ce == QStyle::CE_Splitter) { handle(o, p, t); return true; }
        if (ce != QStyle::CE_DockWidgetTitle) return false;
        const auto *dw = qstyleoption_cast<const QStyleOptionDockWidget *>(o);
        if (!dw || dw->verticalTitleBar) return false;
        const QPalette &pal = dw->palette;
        const bool enabled = dw->state & QStyle::State_Enabled;

        Glass::drawBar(p, dw->rect, t.color("dock.base", pal, pal.window().color()),
                       t.color("dock.border", pal, Theme::mix(pal.window().color(), pal.dark().color(), 0.45)));
        const QRect tr = self->subElementRect(QStyle::SE_DockWidgetTitleBarText, dw, w);
        const QString title = dw->fontMetrics.elidedText(dw->title, Qt::ElideRight, tr.width());
        self->drawItemText(p, tr, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, pal, enabled, title,
                           QPalette::WindowText);
        return true;
    }
};
}

void registerDocks(QVector<Module *> &v) { v.append(new Docks); }
