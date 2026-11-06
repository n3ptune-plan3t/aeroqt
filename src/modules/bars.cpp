#include "glass.h"
#include "module.h"
#include <QStyleOptionSizeGrip>
#include <QStyleOptionToolBar>

// Toolbars + status bar.
//  - CE_ToolBar                        : glass bar; the border line sits on the side facing the content
//                                        (Top area -> bottom edge, Bottom -> top, Left -> right, Right -> left)
//  - PE_IndicatorToolBarHandle         : three engraved dots (State_Horizontal = bar is horizontal)
//  - PE_IndicatorToolBarSeparator      : engraved line
//  - PE_PanelStatusBar / FrameStatusBarItem : glass bar with top border / no item frames
//  - CE_SizeGrip                       : diagonal dot triangle, mirrored per corner (RTL passes BottomLeft)
// Toolbar extension button is NOT a style primitive: QToolBar shows a QToolButton with
// SP_ToolBarHorizontal/VerticalExtensionButton icon (sized by PM_ToolBarExtensionExtent).
// The button slab is already Aero via buttons.cpp; the icon is Aero via AeroStyle::standardPixmap.
namespace {
struct Bars : Module {
    const char *name() const override { return "bars"; }

    int pixelMetric(QStyle::PixelMetric pm, const QStyleOption *, const QWidget *, const Theme &t) const override
    {
        switch (pm) {
        case QStyle::PM_ToolBarHandleExtent:    return int(t.num("toolbar.handle", 8));
        case QStyle::PM_ToolBarSeparatorExtent: return int(t.num("toolbar.separator", 8));
        case QStyle::PM_ToolBarExtensionExtent: return int(t.num("toolbar.extension", 22));
        default: return -1;
        }
    }

    // Gradient bar with the border on `edge`.
    static void bar(QPainter *p, const QRect &r, Qt::Edge edge, const QColor &base, const QColor &border)
    {
        p->save();
        switch (edge) {
        case Qt::BottomEdge:
            Glass::drawBar(p, r, base, border);
            break;
        case Qt::TopEdge:                                        // mirror vertically
            p->translate(0, r.top() + r.bottom() + 1);
            p->scale(1, -1);
            Glass::drawBar(p, r, base, border);
            break;
        default: {                                               // side bars: horizontal gradient
            QLinearGradient g(r.topLeft(), r.topRight());
            const bool onRight = edge == Qt::RightEdge;
            g.setColorAt(onRight ? 0.0 : 1.0, base.darker(103));
            g.setColorAt(onRight ? 1.0 : 0.0, base.lighter(108));
            p->fillRect(r, g);
            p->setPen(border);
            const qreal x = onRight ? r.right() + 0.5 : r.left() + 0.5;
            p->drawLine(QPointF(x, r.top()), QPointF(x, r.bottom() + 1));
        }
        }
        p->restore();
    }

    static QColor barBase(const QPalette &pal, const Theme &t) { return t.color("toolbar.base", pal, pal.window().color()); }
    static QColor barBorder(const QPalette &pal, const Theme &t)
    {
        return t.color("toolbar.border", pal, Theme::mix(pal.window().color(), pal.dark().color(), 0.45));
    }

    bool drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *o, QPainter *p,
                       const QWidget *, const Theme &t) const override
    {
        const QPalette &pal = o->palette;
        switch (pe) {
        case QStyle::PE_IndicatorToolBarHandle:
            Glass::drawHandleDots(p, o->rect, (o->state & QStyle::State_Horizontal) ? Qt::Vertical : Qt::Horizontal,
                                  t.color("splitter.dots", pal, Theme::mix(pal.window().color(), pal.dark().color(), 0.55)));
            return true;
        case QStyle::PE_IndicatorToolBarSeparator:
            Glass::drawSeparator(p, o->rect, (o->state & QStyle::State_Horizontal) ? Qt::Vertical : Qt::Horizontal,
                                 t.color("menu.separator", pal, Theme::mix(pal.window().color(), pal.dark().color(), 0.35)),
                                 Glass::edgeLight(pal.window().color()));
            return true;
        case QStyle::PE_PanelStatusBar:
            bar(p, o->rect, Qt::TopEdge, t.color("statusbar.base", pal, pal.window().color()),
                t.color("statusbar.border", pal, barBorder(pal, t)));
            return true;
        case QStyle::PE_FrameStatusBarItem:
            return true;                                          // flat: no sunken frame around items
        default: return false;
        }
    }

    bool drawControl(QStyle::ControlElement ce, const QStyleOption *o, QPainter *p,
                     const QWidget *, const Theme &t) const override
    {
        const QPalette &pal = o->palette;
        if (ce == QStyle::CE_ToolBar) {
            const auto *tb = qstyleoption_cast<const QStyleOptionToolBar *>(o);
            Qt::Edge edge = Qt::BottomEdge;
            if (tb) {
                switch (tb->toolBarArea) {
                case Qt::BottomToolBarArea: edge = Qt::TopEdge; break;
                case Qt::LeftToolBarArea:   edge = Qt::RightEdge; break;
                case Qt::RightToolBarArea:  edge = Qt::LeftEdge; break;
                default: break;
                }
            }
            bar(p, o->rect, edge, barBase(pal, t), barBorder(pal, t));
            return true;
        }
        if (ce == QStyle::CE_SizeGrip) {
            const auto *g = qstyleoption_cast<const QStyleOptionSizeGrip *>(o);
            if (!g) return false;
            const bool right = g->corner == Qt::BottomRightCorner || g->corner == Qt::TopRightCorner;
            const bool bottom = g->corner == Qt::BottomRightCorner || g->corner == Qt::BottomLeftCorner;
            const int sx = right ? -1 : 1, sy = bottom ? -1 : 1;
            const QPointF a(right ? o->rect.right() - 2.5 : o->rect.left() + 3.5,
                            bottom ? o->rect.bottom() - 2.5 : o->rect.top() + 3.5);
            const QColor dark = t.color("splitter.dots", pal, Theme::mix(pal.window().color(), pal.dark().color(), 0.55));
            p->save();
            p->setRenderHint(QPainter::Antialiasing);
            p->setPen(Qt::NoPen);
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j <= i; ++j) {                    // (0,0) (1,0) (0,1) (2,0) (1,1) (0,2)
                    const QPointF c(a.x() + sx * 4 * (i - j), a.y() + sy * 4 * j);
                    p->setBrush(QColor::fromRgbF(1, 1, 1, 0.75));
                    p->drawEllipse(c + QPointF(0.7, 0.7), 1.1, 1.1);
                    p->setBrush(dark);
                    p->drawEllipse(c, 1.1, 1.1);
                }
            p->restore();
            return true;
        }
        return false;
    }
};
}

void registerBars(QVector<Module *> &v) { v.append(new Bars); }
