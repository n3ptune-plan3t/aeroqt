#include "glass.h"
#include "module.h"
#include <QStyleOptionTab>
#include <QStyleOptionTabBarBase>
#include <QStyleOptionTabWidgetFrame>

// Tabs: glass tab shapes + calm pane.
//  - CE_TabBarTabShape  : glass slab, outer corners rounded, selected tab merges into the pane
//  - PE_FrameTabWidget  : pane panel
//  - PE_FrameTabBarBase : baseline under a standalone tab bar (gap below selected tab)
// All four tab orientations are painted in a "north" frame (outer side = top) via toNorth().
// Geometry (label rect, sizes) stays with Fusion; we only set the overlap metrics.
namespace {
enum Side { N, S, W, E };

Side sideOf(QTabBar::Shape s)
{
    switch (s) {
    case QTabBar::RoundedSouth: case QTabBar::TriangularSouth: return S;
    case QTabBar::RoundedWest:  case QTabBar::TriangularWest:  return W;
    case QTabBar::RoundedEast:  case QTabBar::TriangularEast:  return E;
    default: return N;
    }
}

// Maps the painter so `r` is drawn as a north tab (outer edge = y 0, pane side = y max).
// Returns the local rect (0,0,lw,lh).
QRectF toNorth(QPainter *p, const QRect &r, Side s)
{
    switch (s) {
    case N: p->translate(r.left(), r.top()); return QRectF(0, 0, r.width(), r.height());
    case S: p->translate(r.left(), r.bottom() + 1); p->scale(1, -1); return QRectF(0, 0, r.width(), r.height());
    case W: p->translate(r.left(), r.bottom() + 1); p->rotate(-90); return QRectF(0, 0, r.height(), r.width());
    case E: p->translate(r.right() + 1, r.top()); p->rotate(90); return QRectF(0, 0, r.height(), r.width());
    }
    return QRectF();
}

struct Tabs : Module {
    const char *name() const override { return "tabs"; }

    int pixelMetric(QStyle::PixelMetric pm, const QStyleOption *, const QWidget *, const Theme &) const override
    {
        switch (pm) {
        case QStyle::PM_TabBarBaseOverlap: return 1;   // pane border runs under the tab bar's last row
        case QStyle::PM_TabBarTabOverlap:  return 0;   // we leave a 1px gap ourselves
        default: return -1;
        }
    }

    static QColor paneColor(const QPalette &pal, const Theme &t)
    {
        return t.color("tab.pane", pal, Theme::mix(pal.window().color(), pal.base().color(), 0.5));
    }

    bool drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *o, QPainter *p,
                       const QWidget *, const Theme &t) const override
    {
        const QPalette &pal = o->palette;
        const QColor border = t.color("tab.border", pal, pal.dark().color());
        switch (pe) {
        case QStyle::PE_FrameTabWidget:
            Glass::drawPanel(p, o->rect, t.num("tab.paneRadius", 3), paneColor(pal, t), border);
            return true;
        case QStyle::PE_FrameTabBarBase: {
            const auto *b = qstyleoption_cast<const QStyleOptionTabBarBase *>(o);
            if (!b) return false;
            const QRect bar = b->tabBarRect.isNull() ? o->rect : b->tabBarRect;
            const QRect sel = b->selectedTabRect;
            p->save();
            p->setPen(border);
            // line on the pane side, interrupted under the selected tab
            auto seg = [&](bool horiz, int fixed, int from, int to, int gapFrom, int gapTo) {
                auto line = [&](int a, int c) { if (c > a) horiz ? p->drawLine(a, fixed, c, fixed) : p->drawLine(fixed, a, fixed, c); };
                if (sel.isNull()) line(from, to);
                else { line(from, qMin(to, gapFrom)); line(qMax(from, gapTo), to); }
            };
            const QRect r = o->rect;
            switch (sideOf(b->shape)) {
            case N: seg(true, bar.bottom(), r.left(), r.right() + 1, sel.left(), sel.right() + 1); break;
            case S: seg(true, bar.top(), r.left(), r.right() + 1, sel.left(), sel.right() + 1); break;
            case W: seg(false, bar.right(), r.top(), r.bottom() + 1, sel.top(), sel.bottom() + 1); break;
            case E: seg(false, bar.left(), r.top(), r.bottom() + 1, sel.top(), sel.bottom() + 1); break;
            }
            p->restore();
            return true;
        }
        default: return false;
        }
    }

    bool drawControl(QStyle::ControlElement ce, const QStyleOption *o, QPainter *p,
                     const QWidget *w, const Theme &t) const override
    {
        if (ce != QStyle::CE_TabBarTabShape) return false;
        const auto *tab = qstyleoption_cast<const QStyleOptionTab *>(o);
        if (!tab) return false;

        const QPalette &pal = o->palette;
        const auto st = o->state;
        const bool selected = st & QStyle::State_Selected;
        const bool hover = st & QStyle::State_MouseOver;
        const bool enabled = st & QStyle::State_Enabled;
        const QColor accent = t.color("accent", pal, pal.highlight().color());
        const qreal rad = t.num("tab.radius", 5);

        Glass::Spec s;
        s.base = selected ? t.color("tab.selected", pal, Theme::mix(pal.button().color(), pal.highlight().color(), 0.10))
                          : t.color("tab.base", pal, Theme::mix(pal.button().color(), pal.dark().color(), 0.10));
        s.border = t.color("tab.border", pal, pal.dark().color());
        s.glow = accent;
        s.radius = rad;
        s.gloss = t.num("gloss", 0.55) * (selected ? 1.0 : 0.7);
        s.hovered = hover;
        // one fade per tab: its rect position is a stable id within the tab bar
        s.hoverT = fade(w, (quintptr(o->rect.x() & 0xffff) << 16) | quintptr(o->rect.y() & 0xffff), hover && enabled && !selected);
        if (!selected) s.hoverTint = 0.18;
        s.enabled = enabled;

        p->save();
        const Side side = sideOf(tab->shape);
        const QRectF R = toNorth(p, o->rect, side);
        // unselected tabs leave the pane border row and a 1px gap visible; selected one covers them
        const QRectF vis(R.left(), R.top(), R.width() - (selected ? 0 : 1), R.height() - (selected ? 0 : 1));
        p->setClipRect(vis);
        // slab is taller than the clip so the pane-side corners/border fall outside it
        s.axis = Qt::Vertical;
        Glass::drawSlab(p, QRectF(vis.left(), vis.top(), vis.width(), vis.height() + 2 * rad + 2), s);

        if (selected) {   // merge with the pane: pane-colored strip over the last two rows
            p->setClipping(false);
            p->setRenderHint(QPainter::Antialiasing, false);
            p->fillRect(QRectF(1, R.height() - 2, R.width() - 2, 2), paneColor(pal, t));
        }
        p->restore();
        return true;
    }
};
}

void registerTabs(QVector<Module *> &v) { v.append(new Tabs); }
