#include "glass.h"
#include "module.h"
#include <QIcon>
#include <QMenuBar>
#include <QPainterPath>
#include <QStyleOptionMenuItem>
#include <QWidget>
#include <QWindow>

// Menus + menu bar.
//  popup  : PE_PanelMenu (fill) + PE_FrameMenu (border) + CE_MenuItem (fully ours) + CT_MenuItem size
//  menubar: PE_PanelMenuBar (glass bar) + CE_MenuBarItem (glass slab when active)
// CE_MenuItem layout (LTR; mirrored with visualRect for RTL):
//   | pad | icon/check column | gap | text ...... shortcut | arrow area | pad |
// QMenu adds tabWidth (widest shortcut) to the column width itself, so sizeFromContents must NOT.
// CE_MenuTearoff: engraved line + dots at the top of a tear-off menu.
namespace {
constexpr int kPad = 6;     // outer horizontal padding
constexpr int kGap = 6;     // between icon column and text
constexpr int kArrow = 14;  // submenu arrow area (reserved on every item so columns line up)
constexpr int kIcon = 16;

using MI = QStyleOptionMenuItem;

int iconColumn(const MI *mi)
{
    return qMax(mi->maxIconWidth, mi->menuHasCheckableItems ? kIcon : 0);
}


struct Menus : Module {
    const char *name() const override { return "menus"; }

    // ------------------------------------------------------------ metrics
    int pixelMetric(QStyle::PixelMetric pm, const QStyleOption *, const QWidget *, const Theme &t) const override
    {
        switch (pm) {
        case QStyle::PM_MenuPanelWidth:      return 1;
        case QStyle::PM_MenuHMargin:         return int(t.num("menu.hMargin", 2));
        case QStyle::PM_MenuVMargin:         return int(t.num("menu.vMargin", 3));
        case QStyle::PM_MenuBarPanelWidth:   return 0;
        case QStyle::PM_MenuBarHMargin:      return 2;
        case QStyle::PM_MenuBarVMargin:      return 1;
        case QStyle::PM_MenuBarItemSpacing:  return 1;
        default: return -1;
        }
    }

    QSize sizeFromContents(QStyle::ContentsType ct, const QStyleOption *o, const QSize &sz,
                           const QWidget *, const Theme &t) const override
    {
        if (ct != QStyle::CT_MenuItem) return QSize();
        const auto *mi = qstyleoption_cast<const MI *>(o);
        if (!mi) return QSize();
        if (mi->menuItemType == MI::Separator) {
            if (mi->text.isEmpty()) return QSize(10, int(t.num("menu.separatorHeight", 8)));
            QFont f = mi->font;                                   // section header: QMenu passes 2x2, text width is ours
            f.setBold(true);
            const QFontMetrics fm(f);
            return QSize(qMax(sz.width(), fm.horizontalAdvance(mi->text) + 2 * kPad + 24), fm.height() + 8);
        }
        if (mi->menuItemType != MI::Normal && mi->menuItemType != MI::DefaultItem && mi->menuItemType != MI::SubMenu)
            return QSize();
        const int col = iconColumn(mi);
        int w = sz.width() + 2 * kPad + kArrow + (col ? col + kGap : 0);
        if (mi->text.contains(QLatin1Char('\t'))) w += 24;      // text <-> shortcut gap
        const int h = qMax(sz.height() + 6, int(t.num("menu.itemHeight", 24)));
        return QSize(w, h);
    }

    // ------------------------------------------------------------ primitives
    bool drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *o, QPainter *p,
                       const QWidget *, const Theme &t) const override
    {
        const QPalette &pal = o->palette;
        switch (pe) {
        case QStyle::PE_PanelMenu:
            p->fillRect(o->rect, t.color("menu.base", pal, Theme::mix(pal.base().color(), pal.window().color(), 0.2)));
            return true;
        case QStyle::PE_FrameMenu: {
            p->save();
            p->setPen(t.color("menu.border", pal, pal.dark().color()));
            p->setBrush(Qt::NoBrush);
            p->drawRect(QRectF(o->rect).adjusted(0.5, 0.5, -0.5, -0.5));
            p->restore();
            return true;
        }
        case QStyle::PE_PanelMenuBar:
            paintBar(o->rect, pal, p, t);
            return true;
        default: return false;
        }
    }

    // ------------------------------------------------------------ controls
    bool drawControl(QStyle::ControlElement ce, const QStyleOption *o, QPainter *p,
                     const QWidget *w, const Theme &t) const override
    {
        switch (ce) {
        case QStyle::CE_MenuBarEmptyArea: paintBar(o->rect, o->palette, p, t); return true;
        case QStyle::CE_MenuBarItem:      return menuBarItem(o, p, w, t);
        case QStyle::CE_MenuItem:         return menuItem(o, p, w, t);
        case QStyle::CE_MenuTearoff: {                        // grip strip at the top of a tear-off menu
            const QPalette &pal = o->palette;
            const QColor dark = t.color("menu.separator", pal, Theme::mix(pal.base().color(), pal.dark().color(), 0.35));
            p->fillRect(o->rect, t.color("menu.base", pal, Theme::mix(pal.base().color(), pal.window().color(), 0.2)));
            Glass::drawSeparator(p, QRectF(o->rect), Qt::Horizontal, dark, Glass::edgeLight(pal.base().color()));
            Glass::drawHandleDots(p, QRect(o->rect.left() + o->rect.width() / 2 - 8, o->rect.top(), 16, o->rect.height()),
                                  Qt::Horizontal, dark);
            return true;
        }
        case QStyle::CE_MenuScroller: {                       // arrow strip at the ends of a long menu
            const QPalette &pal = o->palette;
            p->fillRect(o->rect, t.color("menu.base", pal, Theme::mix(pal.base().color(), pal.window().color(), 0.2)));
            Glass::drawArrow(p, o->rect, (o->state & QStyle::State_DownArrow) ? Qt::DownArrow : Qt::UpArrow,
                             pal.color(QPalette::Active, QPalette::Text), o->state & QStyle::State_Enabled);
            return true;
        }
        default: return false;
        }
    }

    // Qt paints a QMenuBar as items + empty area only (no panel), so both paint the bar themselves.
    static void paintBar(const QRect &r, const QPalette &pal, QPainter *p, const Theme &t)
    {
        Glass::drawBar(p, r, t.color("menubar.base", pal, pal.window().color()),
                       t.color("menubar.border", pal, Theme::mix(pal.window().color(), pal.dark().color(), 0.45)));
    }

    // Item highlight shared by popup + bar.
    static void highlight(QPainter *p, const QRectF &r, const QPalette &pal, const Theme &t, bool pressed, QColor *outBase,
                          qreal alpha = 1.0)
    {
        const QColor accent = t.color("accent", pal, pal.highlight().color());
        Glass::Spec s;
        s.base = t.color("menu.item", pal, Theme::mix(pal.base().color(), accent, 0.45));
        if (pressed) s.base = Theme::mix(s.base, accent, 0.25);
        s.border = t.color("menu.itemBorder", pal, Theme::mix(accent, pal.dark().color(), 0.25));
        s.glow = accent;
        s.radius = t.num("menu.itemRadius", 3);
        s.gloss = t.num("gloss", 0.55) * 0.9;
        s.pressed = pressed;
        s.hovered = true;
        if (outBase) *outBase = s.base;
        p->save();
        p->setOpacity(p->opacity() * alpha);
        Glass::drawSlab(p, r, s);
        p->restore();
    }

    bool menuBarItem(const QStyleOption *o, QPainter *p, const QWidget *w, const Theme &t) const
    {
        const auto *mi = qstyleoption_cast<const MI *>(o);
        if (!mi) return false;
        const bool enabled = mi->state & QStyle::State_Enabled;
        const bool active = enabled && (mi->state & (QStyle::State_Selected | QStyle::State_Sunken));
        QPalette pal = mi->palette;
        // bar slice behind the item; gradient spans the whole bar so items and empty area line up
        paintBar(qobject_cast<const QMenuBar *>(w) ? w->rect() : mi->rect, pal, p, t);
        const QPalette::ColorGroup g = enabled ? QPalette::Active : QPalette::Disabled;
        QColor txt = pal.color(g, QPalette::ButtonText);
        const qreal hv = fade(w, quintptr(mi->rect.left()), active);
        if (hv > 0.002) {
            QColor base;
            highlight(p, QRectF(mi->rect).adjusted(0, 1, 0, -1), pal, t, mi->state & QStyle::State_Sunken, &base, hv);
            if (active) txt = Theme::contrastText(base, pal, g);
        }
        pal.setColor(QPalette::ButtonText, txt);
        int flags = Qt::AlignCenter | Qt::TextDontClip | Qt::TextSingleLine | Qt::TextShowMnemonic;
        if (!self->styleHint(QStyle::SH_UnderlineShortcut, mi, w)) flags |= Qt::TextHideMnemonic;
        self->drawItemText(p, mi->rect, flags, pal, enabled, mi->text, QPalette::ButtonText);
        return true;
    }

    bool menuItem(const QStyleOption *o, QPainter *p, const QWidget *w, const Theme &t) const
    {
        const auto *mi = qstyleoption_cast<const MI *>(o);
        if (!mi) return false;
        const QPalette &pal = mi->palette;
        const QRect r = mi->rect;
        const Qt::LayoutDirection dir = mi->direction;
        auto vis = [&](const QRect &x) { return QStyle::visualRect(dir, r, x); };

        if (mi->menuItemType == MI::Separator) {
            if (!mi->text.isEmpty()) {                           // section header: dim bold text + trailing line
                QFont f = mi->font;
                f.setBold(true);
                const int tw = QFontMetrics(f).horizontalAdvance(mi->text);
                const QColor dim = Theme::mix(pal.color(QPalette::Active, QPalette::Text), pal.base().color(), 0.35);
                p->save();
                p->setFont(f);
                p->setPen(dim);
                const QRect tr(r.left() + kPad, r.top(), tw + 2, r.height());
                p->drawText(vis(tr), Qt::AlignVCenter | int(QStyle::visualAlignment(dir, Qt::AlignLeft)) | Qt::TextSingleLine, mi->text);
                p->restore();
                const int lx = tr.right() + 8;
                if (lx < r.right() - kPad)
                    Glass::drawSeparator(p, QRectF(vis(QRect(lx, r.top(), r.right() - kPad - lx, r.height()))), Qt::Horizontal,
                                         t.color("menu.separator", pal, Theme::mix(pal.base().color(), pal.dark().color(), 0.35)),
                                         Glass::edgeLight(pal.base().color()));
                return true;
            }
            const QColor c = t.color("menu.separator", pal, Theme::mix(pal.base().color(), pal.dark().color(), 0.35));
            Glass::drawSeparator(p, QRectF(r.left() + kPad, r.top(), r.width() - 2 * kPad, r.height() - 1),
                                 Qt::Horizontal, c, Glass::edgeLight(pal.base().color()));
            return true;
        }
        if (mi->menuItemType != MI::Normal && mi->menuItemType != MI::DefaultItem && mi->menuItemType != MI::SubMenu)
            return false;

        const bool enabled = mi->state & QStyle::State_Enabled;
        const bool sel = enabled && (mi->state & QStyle::State_Selected);
        const QPalette::ColorGroup g = enabled ? QPalette::Active : QPalette::Disabled;
        QColor txt = pal.color(g, QPalette::Text);
        const qreal hv = fade(w, quintptr(r.top()), sel);        // highlight glides in/out as the selection moves
        if (hv > 0.002) {
            QColor base;
            highlight(p, QRectF(r).adjusted(2, 1, -2, -1), pal, t, mi->state & QStyle::State_Sunken, &base, hv);
            if (sel) txt = Theme::contrastText(base, pal, g);
        }

        // ---- layout (LTR) ----
        const int col = iconColumn(mi);
        const int textLeft = r.left() + kPad + (col ? col + kGap : 0);
        const QRect iconR(r.left() + kPad, r.top(), col, r.height());
        const QRect arrowR(r.right() - kPad - kArrow + 1, r.top(), kArrow, r.height());
        const int scRight = arrowR.left() - 2;

        QString text = mi->text, shortcut;
        const int tab = text.indexOf(QLatin1Char('\t'));
        if (tab >= 0) { shortcut = text.mid(tab + 1); text = text.left(tab); }

        // ---- icon / check ----
        const bool checkable = mi->checkType != MI::NotCheckable;
        const bool checked = checkable && mi->checked;
        if (col) {
            const QRect ic = vis(iconR);
            const QRect box = QStyle::alignedRect(dir, Qt::AlignCenter, QSize(kIcon + 2, kIcon + 2), ic);
            if (checked && !mi->icon.isNull())
                Glass::drawInset(p, box, 3, Theme::mix(pal.base().color(), pal.highlight().color(), 0.3),
                                 t.color("input.border", pal, pal.dark().color()));
            if (!mi->icon.isNull()) {
                const QIcon::Mode mode = !enabled ? QIcon::Disabled : (sel ? QIcon::Active : QIcon::Normal);
                const QPixmap pm = mi->icon.pixmap(QSize(kIcon, kIcon), w ? w->devicePixelRatioF() : 1.0, mode,
                                                   checked ? QIcon::On : QIcon::Off);
                const QSize ls = pm.size() / pm.devicePixelRatio();
                p->drawPixmap(QStyle::alignedRect(dir, Qt::AlignCenter, ls, ic).topLeft(), pm);
            } else if (checked) {
                p->save();
                p->setRenderHint(QPainter::Antialiasing);
                const QRectF b = QRectF(box).adjusted(2, 2, -2, -2);
                const QColor mark = enabled ? txt : pal.color(QPalette::Disabled, QPalette::Text);
                if (mi->checkType == MI::Exclusive) {
                    p->setPen(Qt::NoPen);
                    p->setBrush(mark);
                    p->drawEllipse(b.center(), b.width() * 0.22, b.width() * 0.22);
                } else {
                    p->setBrush(Qt::NoBrush);
                    p->setPen(QPen(mark, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    QPainterPath c;
                    c.moveTo(b.left() + b.width() * .15, b.top() + b.height() * .52);
                    c.lineTo(b.left() + b.width() * .42, b.top() + b.height() * .78);
                    c.lineTo(b.left() + b.width() * .88, b.top() + b.height() * .22);
                    p->drawPath(c);
                }
                p->restore();
            }
        }

        // ---- text + shortcut ----
        p->save();
        QFont f = mi->font;
        if (mi->menuItemType == MI::DefaultItem) f.setBold(true);
        p->setFont(f);
        p->setPen(txt);
        int flags = Qt::TextDontClip | Qt::TextSingleLine | Qt::TextShowMnemonic | Qt::AlignVCenter;
        if (!self->styleHint(QStyle::SH_UnderlineShortcut, mi, w)) flags |= Qt::TextHideMnemonic;
        const QRect textR(textLeft, r.top(), scRight - textLeft + 1, r.height());
        p->drawText(vis(textR), flags | int(QStyle::visualAlignment(dir, Qt::AlignLeft)), text);
        if (!shortcut.isEmpty()) {
            QColor sc = txt;
            if (!sel) sc = Theme::mix(txt, pal.base().color(), 0.35);
            p->setPen(sc);
            p->drawText(vis(textR), flags | int(QStyle::visualAlignment(dir, Qt::AlignRight)), shortcut);
        }
        p->restore();

        // ---- submenu arrow ----
        if (mi->menuItemType == MI::SubMenu)
            Glass::drawArrow(p, vis(arrowR), dir == Qt::RightToLeft ? Qt::LeftArrow : Qt::RightArrow, txt, enabled);
        return true;
    }
};
}

void registerMenus(QVector<Module *> &v) { v.append(new Menus); }
