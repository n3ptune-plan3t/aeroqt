#include "glass.h"
#include "module.h"
#include <QStyleOptionTitleBar>

// QMdiSubWindow title bar (CC_TitleBar): glass bar, elided label, glass buttons.
// QStyleOptionTitleBar members are: text, icon, titleBarFlags, titleBarState (+ subControls).
// Buttons: slab delegated to buttons module (self->drawPrimitive(PE_PanelButtonTool)),
// glyph drawn here. Geometry from Fusion via self->subControlRect.
// Height is PM_TitleBarHeight (there is no CT_TitleBarHeight in QStyle).
// QCalendarWidget / QWizard / QProgressDialog need no painter of their own: calendar and
// wizard pages are QFrames (PE_Frame focus glow in frames.cpp), their buttons are tool/push
// buttons, and QProgressDialog is a progress bar + button - all already Aero.
namespace {
struct TitleBar : Module {
    const char *name() const override { return "titlebar"; }

    int pixelMetric(QStyle::PixelMetric pm, const QStyleOption *, const QWidget *, const Theme &t) const override
    {
        return pm == QStyle::PM_TitleBarHeight ? int(t.num("titlebar.height", 22)) : -1;
    }

    bool drawComplex(QStyle::ComplexControl cc, const QStyleOptionComplex *o, QPainter *p,
                     const QWidget *w, const Theme &t) const override
    {
        if (cc != QStyle::CC_TitleBar) return false;
        const auto *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(o);
        if (!tb) return false;
        const QPalette &pal = tb->palette;
        const bool enabled = tb->state & QStyle::State_Enabled;
        const bool active = tb->state & QStyle::State_Active;
        const QColor accent = t.color("accent", pal, pal.highlight().color());

        // ---- bar background (geometry from Fusion via self->subControlRect) ----
        Glass::Spec s;
        s.base = active ? t.color("titlebar.active", pal, Theme::mix(pal.button().color(), accent, 0.20))
                        : t.color("titlebar.base", pal, Theme::mix(pal.window().color(), pal.dark().color(), 0.15));
        s.border = t.color("titlebar.border", pal, Theme::mix(pal.window().color(), pal.dark().color(), 0.45));
        s.glow = accent;
        s.radius = t.num("titlebar.radius", 4);
        s.gloss = t.num("gloss", 0.55);
        s.enabled = enabled;
        s.hoverT = fade(w, 0, (tb->state & QStyle::State_MouseOver) && enabled);
        Glass::drawSlab(p, QRectF(tb->rect).adjusted(0, 0, 0, -1), s);

        // ---- label ----
        if (tb->subControls & QStyle::SC_TitleBarLabel) {
            const QRect lab = self->subControlRect(QStyle::CC_TitleBar, tb, QStyle::SC_TitleBarLabel, w);
            if (!lab.isNull()) {
                int flags = Qt::AlignLeft | Qt::AlignVCenter | Qt::ElideRight | Qt::TextSingleLine | Qt::TextShowMnemonic;
                if (!self->styleHint(QStyle::SH_UnderlineShortcut, tb, w)) flags |= Qt::TextHideMnemonic;
                self->drawItemText(p, lab, flags, pal, enabled, tb->text, QPalette::WindowText);
            }
        }

        // ---- buttons: slab delegated to buttons module, glyph drawn here ----
        struct Btn { QStyle::SubControl sc; const char *glyph; };
        static const Btn btns[] = {{QStyle::SC_TitleBarSysMenu, "|="},
                                   {QStyle::SC_TitleBarMinButton, "-"},
                                   {QStyle::SC_TitleBarMaxButton, "[]"},
                                   {QStyle::SC_TitleBarNormalButton, "[]"},
                                   {QStyle::SC_TitleBarShadeButton, "^"},
                                   {QStyle::SC_TitleBarUnshadeButton, "v"},
                                   {QStyle::SC_TitleBarContextHelpButton, "?"},
                                   {QStyle::SC_TitleBarCloseButton, "x"}};
        for (const Btn &b : btns) {
            if (!(tb->subControls & b.sc)) continue;
            const QRect r = self->subControlRect(QStyle::CC_TitleBar, tb, b.sc, w);
            if (r.isNull()) continue;
            QStyleOptionButton btn;
            btn.QStyleOption::operator=(*tb);
            btn.rect = r;
            btn.state = tb->state;
            btn.text.clear();
            self->drawPrimitive(QStyle::PE_PanelButtonTool, &btn, p, w);
            p->save();
            p->setPen(pal.color(enabled ? QPalette::Active : QPalette::Disabled, QPalette::ButtonText));
            p->drawText(r, Qt::AlignCenter, QLatin1String(b.glyph));
            p->restore();
        }
        return true;
    }
};
}

void registerTitleBar(QVector<Module *> &v) { v.append(new TitleBar); }
