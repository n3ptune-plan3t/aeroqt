#pragma once
#include <climits>
#include <QPainter>
#include <QSize>
#include <QStyle>
#include <QStyleOption>
#include <QVector>
#include "theme.h"

// Animation service, implemented once by AeroStyle (modules never own timers).
//   hover(w, id, on): smoothed 0..1 factor per (widget, part). `id` tells parts of one widget apart
//                     (tab rect, scrollbar subcontrol, item index...). New entries fade in from 0.
//   phase(w, ms)    : 0..1 position in a looping animation; calling it keeps `w` repainting at ~60 fps
//                     until it stops being asked (progress marquee / shine).
// With animations off (AEROQT_NO_ANIM=1 or theme anim.enabled = 0) hover() returns the target at once.
struct Animator {
    virtual ~Animator() = default;
    virtual qreal hover(const QWidget *w, quintptr id, bool on) = 0;
    virtual qreal phase(const QWidget *w, int periodMs) = 0;
    virtual bool enabled() const = 0;
};

// A Module owns the painting + geometry of one family of widgets.
//   draw*  : return true = "handled", false = fall through to the next module,
//            then to the base style (Fusion).
//   geometry hooks use sentinels for "not handled":
//            pixelMetric -> -1, sizeFromContents -> invalid QSize,
//            subControlRect/subElementRect -> null QRect, styleHint -> Module::Unhandled
// `self` is the owning AeroStyle (set once at construction). Modules call
// self->subControlRect(...) etc. so they get the *final* geometry (own override
// or Fusion's) without knowing which module provided it.
// To add a widget: new file in src/modules/, subclass Module, register it in
// AeroStyle::AeroStyle(). To debug: AEROQT_DEBUG=1 prints who handled what.
struct Module {
    static constexpr int Unhandled = INT_MIN;
    QStyle *self = nullptr;
    Animator *anim = nullptr;   // set with `self`; may be null in tests

    qreal fade(const QWidget *w, quintptr id, bool on) const
    {
        return (anim && w) ? anim->hover(w, id, on) : (on ? 1.0 : 0.0);
    }

    virtual ~Module() = default;
    virtual const char *name() const = 0;

    // ---- painting ----
    virtual bool drawPrimitive(QStyle::PrimitiveElement, const QStyleOption *, QPainter *,
                               const QWidget *, const Theme &) const { return false; }
    virtual bool drawControl(QStyle::ControlElement, const QStyleOption *, QPainter *,
                             const QWidget *, const Theme &) const { return false; }
    virtual bool drawComplex(QStyle::ComplexControl, const QStyleOptionComplex *, QPainter *,
                             const QWidget *, const Theme &) const { return false; }

    // ---- geometry ----
    virtual int pixelMetric(QStyle::PixelMetric, const QStyleOption *, const QWidget *,
                            const Theme &) const { return -1; }
    virtual QSize sizeFromContents(QStyle::ContentsType, const QStyleOption *, const QSize &,
                                   const QWidget *, const Theme &) const { return QSize(); }
    virtual QRect subControlRect(QStyle::ComplexControl, const QStyleOptionComplex *,
                                 QStyle::SubControl, const QWidget *, const Theme &) const { return QRect(); }
    virtual QRect subElementRect(QStyle::SubElement, const QStyleOption *, const QWidget *,
                                 const Theme &) const { return QRect(); }
    virtual int styleHint(QStyle::StyleHint, const QStyleOption *, const QWidget *,
                          QStyleHintReturn *, const Theme &) const { return Unhandled; }
};

// One register function per module file (call order = dispatch priority).
void registerButtons(QVector<Module *> &);
void registerInputs(QVector<Module *> &);
void registerProgress(QVector<Module *> &);
void registerScrollbar(QVector<Module *> &);
void registerSlider(QVector<Module *> &);
void registerSpinCombo(QVector<Module *> &);
void registerTabs(QVector<Module *> &);
void registerMenus(QVector<Module *> &);
void registerFrames(QVector<Module *> &);
void registerItemViews(QVector<Module *> &);
void registerDocks(QVector<Module *> &);
void registerBars(QVector<Module *> &);
void registerMisc(QVector<Module *> &);
void registerTitleBar(QVector<Module *> &);
