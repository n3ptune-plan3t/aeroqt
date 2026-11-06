#include "glass.h"
#include "module.h"
#include <QAbstractItemModel>
#include <QStyleOptionHeader>
#include <QTableView>
#include <QStyleOptionViewItem>

// Item views + headers.
//  - PE_PanelItemViewItem / PE_PanelItemViewRow : glass selection + hover slab, per item.
//      Corners are rounded only at the outer ends of a row of cells (viewItemPosition), so a
//      selected table row reads as ONE slab. Inactive window => greyed selection.
//  - CE_ItemViewItem : selected items get a palette whose HighlightedText contrasts with OUR light
//      glass (the stock white HighlightedText would vanish on it). Done by re-entering the style
//      once with a modified option; `busy` makes this module step aside on the re-entry so
//      Fusion draws the actual content.
//  - CE_HeaderSection : glass header cell with engraved separator.
// PE_IndicatorHeaderArrow is left to Fusion (Qt's SortUp/SortDown naming is inverted vs. intuition).
namespace {
thread_local bool busy = false;

struct Reentry {
    Reentry() { busy = true; }
    ~Reentry() { busy = false; }
};

QColor selectionBase(const QPalette &pal, bool active, const Theme &t)
{
    return active ? t.color("view.selected", pal, Theme::mix(pal.base().color(), pal.highlight().color(), 0.45))
                  : t.color("view.inactive", pal, Theme::mix(pal.base().color(), pal.dark().color(), 0.22));
}

struct ItemViews : Module {
    const char *name() const override { return "itemviews"; }

    // QTableView never sets viewItemPosition: derive it from the column so a selected row is one slab.
    static QStyleOptionViewItem::ViewItemPosition positionOf(const QStyleOptionViewItem *vo, const QWidget *w)
    {
        using P = QStyleOptionViewItem;
        if (vo->viewItemPosition != P::Invalid || !qobject_cast<const QTableView *>(w) || !vo->index.isValid())
            return vo->viewItemPosition;
        const int cols = vo->index.model()->columnCount(vo->index.parent());
        const int c = vo->index.column();
        return cols <= 1 ? P::OnlyOne : c == 0 ? P::Beginning : c == cols - 1 ? P::End : P::Middle;
    }

    bool drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *o, QPainter *p,
                       const QWidget *w, const Theme &t) const override
    {
        if (pe != QStyle::PE_PanelItemViewItem && pe != QStyle::PE_PanelItemViewRow) return false;
        const auto *vo = qstyleoption_cast<const QStyleOptionViewItem *>(o);
        if (!vo) return false;
        const QPalette &pal = vo->palette;
        const QRect r = vo->rect;
        const auto st = vo->state;

        if (vo->backgroundBrush.style() != Qt::NoBrush) {
            p->save();
            p->setBrushOrigin(r.topLeft());
            p->fillRect(r, vo->backgroundBrush);
            p->restore();
        } else if (vo->features & QStyleOptionViewItem::Alternate) {
            p->fillRect(r, pal.alternateBase());
        }

        if (pe == QStyle::PE_PanelItemViewRow) return true;    // rows keep their background; items draw the slabs
        const bool sel = st & QStyle::State_Selected;
        const bool hover = st & QStyle::State_MouseOver;
        if (!(st & QStyle::State_Enabled) && !sel) return true;
        qreal hoverFade = 1.0;                                      // selection is instant, plain hover fades
        if (!sel) {
            const QModelIndex ix = vo->index;
            const quintptr id = ix.isValid() ? (quintptr(ix.row()) * 1000003u) ^ (quintptr(ix.column()) * 10007u) ^ quintptr(ix.internalId())
                                             : (quintptr(r.x() & 0xffff) << 16) | quintptr(r.y() & 0xffff);
            hoverFade = fade(w, id, hover);
            if (hoverFade <= 0.002) return true;
        }

        const bool active = st & QStyle::State_Active;
        const QColor accent = t.color("accent", pal, pal.highlight().color());
        Glass::Spec s;
        if (sel) {
            s.base = selectionBase(pal, active, t);
            if (hover && active) s.base = Theme::mix(s.base, accent, 0.15);
        } else {
            s.base = t.color("view.hover", pal, Theme::mix(pal.base().color(), accent, 0.18));
        }
        s.border = sel ? Theme::mix(active ? accent : pal.mid().color(), pal.dark().color(), 0.25)
                       : Theme::mix(s.base, accent, 0.45);
        s.glow = (sel && !active) ? s.base : accent;      // unfocused view: calm slab, no accent glow
        const qreal rad = t.num("view.radius", 3);
        s.radius = rad;
        s.gloss = t.num("gloss", 0.55) * (sel ? 0.8 : 0.45);
        s.hovered = hover;
        s.enabled = st & QStyle::State_Enabled;
        s.axis = Qt::Vertical;

        // which ends of the slab are rounded
        using VP = QStyleOptionViewItem::ViewItemPosition;
        VP pos = positionOf(vo, w);
        const bool left = pos != QStyleOptionViewItem::Middle && pos != QStyleOptionViewItem::End;
        const bool right = pos != QStyleOptionViewItem::Middle && pos != QStyleOptionViewItem::Beginning;
        const QRectF vis = QRectF(r).adjusted(left ? 1 : 0, 1, right ? -1 : 0, -1);
        const qreal ext = 2 * rad + 2;
        const QRectF slab = vis.adjusted(left ? 0 : -ext, 0, right ? 0 : ext, 0);   // square ends fall outside the clip

        p->save();
        p->setClipRect(vis);
        p->setOpacity(p->opacity() * hoverFade);
        Glass::drawSlab(p, slab, s);
        p->restore();
        return true;
    }

    bool drawControl(QStyle::ControlElement ce, const QStyleOption *o, QPainter *p,
                     const QWidget *w, const Theme &t) const override
    {
        if (ce == QStyle::CE_ItemViewItem) {
            if (busy) return false;                                   // re-entry: Fusion draws the content
            const auto *vo = qstyleoption_cast<const QStyleOptionViewItem *>(o);
            if (!vo || !(vo->state & QStyle::State_Selected)) return false;
            QStyleOptionViewItem copy(*vo);
            const bool active = vo->state & QStyle::State_Active;
            const QColor base = selectionBase(vo->palette, active, t);
            for (QPalette::ColorGroup g : {QPalette::Active, QPalette::Inactive})
                copy.palette.setColor(g, QPalette::HighlightedText,
                                      Theme::contrastText(base, vo->palette, active ? QPalette::Active : QPalette::Inactive));
            Reentry guard;
            self->drawControl(ce, &copy, p, w);
            return true;
        }
        if (ce == QStyle::CE_HeaderSection) return headerSection(o, p, w, t);
        return false;
    }

    bool headerSection(const QStyleOption *o, QPainter *p, const QWidget *w, const Theme &t) const
    {
        const auto *h = qstyleoption_cast<const QStyleOptionHeader *>(o);
        if (!h) return false;
        const QPalette &pal = h->palette;
        const QRect r = h->rect;
        const bool horiz = h->orientation == Qt::Horizontal;
        const QColor accent = t.color("accent", pal, pal.highlight().color());
        QColor base = t.color("header.base", pal, pal.button().color());
        const bool hov = h->state & QStyle::State_MouseOver;
        const qreal hv = fade(w, (quintptr(r.x() & 0xffff) << 16) | quintptr(r.y() & 0xffff), hov && !(h->state & QStyle::State_Sunken));
        if (h->state & QStyle::State_Sunken)  base = Theme::mix(base, accent, 0.30);
        else if (hv > 0)                      base = Theme::mix(base, accent, 0.18 * hv);
        else if (h->state & QStyle::State_On) base = Theme::mix(base, accent, 0.12);
        const QColor border = t.color("header.border", pal, Theme::mix(pal.dark().color(), pal.button().color(), 0.5));

        p->save();
        if (horiz) {
            Glass::drawBar(p, r, base, border);
        } else {                                                       // row header: horizontal gradient
            QLinearGradient g(r.topLeft(), r.topRight());
            g.setColorAt(0, base.lighter(110));
            g.setColorAt(1, base);
            p->fillRect(r, g);
            p->setPen(border);
            p->drawLine(r.right(), r.top(), r.right(), r.bottom());
        }
        // engraved separator at the trailing edge
        p->setRenderHint(QPainter::Antialiasing, false);
        const QColor hi = QColor::fromRgbF(1, 1, 1, Glass::edgeLight(base));
        if (horiz) {
            p->setPen(border);
            p->drawLine(r.right(), r.top() + 4, r.right(), r.bottom() - 4);
            p->setPen(hi);
            p->drawLine(r.right() - 1, r.top() + 4, r.right() - 1, r.bottom() - 4);
        } else {
            p->setPen(border);
            p->drawLine(r.left() + 4, r.bottom(), r.right() - 4, r.bottom());
            p->setPen(hi);
            p->drawLine(r.left() + 4, r.bottom() - 1, r.right() - 4, r.bottom() - 1);
        }
        p->restore();
        return true;
    }
};
}

void registerItemViews(QVector<Module *> &v) { v.append(new ItemViews); }
