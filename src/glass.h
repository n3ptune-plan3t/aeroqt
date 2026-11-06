#pragma once
#include <QColor>
#include <QPainter>
#include <QRectF>
#include <Qt>

// The Frutiger Aero primitive: glossy glass slab. All modules paint through this,
// so restyling the whole look = editing glass.cpp (or the theme tokens).
namespace Glass {
struct Spec {
    QColor base;                 // body tint
    QColor border;               // outer 1px line
    QColor glow;                 // bottom reflected light
    qreal  radius  = 6;
    qreal  gloss   = 0.55;       // 0..1 strength of the upper-half highlight
    bool   pressed = false;
    bool   hovered = false;
    // Smoothed hover 0..1 from the Animator (<0 = not animated: use `hovered`). Drives the glow strength
    // and, with `hoverTint`, a tint of the body toward `glow` (replaces ad-hoc Theme::mix(base, accent) at call sites).
    qreal  hoverT    = -1;
    qreal  hoverTint = 0;        // max fraction of `glow` mixed into `base` at hoverT == 1
    bool   enabled = true;
    // Gradient axis: Vertical = light top / deep bottom (buttons, horizontal bars);
    // Horizontal = light left / deep right (thumbs of vertical scrollbars).
    Qt::Orientation axis = Qt::Vertical;
};
// Lightweight controls (Fusion is palette-native and draws ~flat fills with
// no per-frame SVG/QSS work; Kvantum pays QtSvg per widget, Klassy/Breeze add
// KF deps). AeroQt keeps native QPainter + palette (no Svg/KF/QSS), but still
// recomputing 3 gradients per slab was the remaining hot cost. Two knobs
// close the gap (see aero-lite preset): perf multipliers scale/skip the
// gloss+glow passes, and a bounded pixmap cache renders a settled slab
// (hoverT at rest) once and blits it thereafter (like Klassy/Breeze caching
// the static state). setPerf(1,1)+cache is the default full-Aero look.
void setPerf(qreal glossMul, qreal glowMul);
void clearCache();
void drawSlab(QPainter *p, const QRectF &rect, const Spec &s);
// Calm raised panel (tab pane, menu popups): soft top light, keyline, border.
void drawPanel(QPainter *p, const QRectF &rect, qreal radius, const QColor &fill, const QColor &border);
// Full-width bar background (menu bar): split Aero gradient + bottom border line.
void drawBar(QPainter *p, const QRectF &rect, const QColor &base, const QColor &border);
// Engraved 1px line + white highlight (menu separators).
// `hiAlpha`: strength of the white highlight line (use edgeLight(background): soft on dark themes).
void drawSeparator(QPainter *p, const QRectF &rect, Qt::Orientation lineAxis, const QColor &dark, qreal hiAlpha = 0.7);
qreal edgeLight(const QColor &bg);   // 0.7 on light backgrounds, 0.1 on dark ones
// Three engraved dots along `lineAxis` (splitter / dock separators; fits a 6px strip).
void drawHandleDots(QPainter *p, const QRectF &rect, Qt::Orientation lineAxis, const QColor &dark);
void drawInset(QPainter *p, const QRectF &rect, qreal radius, const QColor &fill, const QColor &border);
// Small filled triangle (spin/combo/scrollbar arrows). Size is derived from `rect`.
void drawArrow(QPainter *p, const QRectF &rect, Qt::ArrowType dir, const QColor &c, bool enabled = true);
// Three short grip lines centered in `rect` (scrollbar thumbs, splitters). `lineAxis`
// = direction the lines run along.
void drawGrip(QPainter *p, const QRectF &rect, Qt::Orientation lineAxis, const QColor &c);
void drawFocusGlow(QPainter *p, const QRectF &rect, qreal radius, const QColor &c);
}
