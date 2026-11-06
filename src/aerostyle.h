#pragma once
#include <QElapsedTimer>
#include <QFileSystemWatcher>
#include <QHash>
#include <QProxyStyle>
#include <QTimer>
#include <QVector>
#include "module.h"

class AeroStyle : public QProxyStyle, public Animator {
    Q_OBJECT
public:
    AeroStyle();
    ~AeroStyle() override;

    void polish(QWidget *w) override;
    void unpolish(QWidget *w) override;
    using QProxyStyle::polish;
    using QProxyStyle::unpolish;

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                       const QWidget *w) const override;
    void drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                     const QWidget *w) const override;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *w) const override;
    int pixelMetric(PixelMetric pm, const QStyleOption *opt, const QWidget *w) const override;
    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &sz,
                           const QWidget *w) const override;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                         const QWidget *w) const override;
    QRect subElementRect(SubElement se, const QStyleOption *opt, const QWidget *w) const override;
    int styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *w,
                  QStyleHintReturn *ret) const override;
    QPixmap standardPixmap(StandardPixmap sp, const QStyleOption *opt = nullptr) const override;

    const Theme &theme() const { return m_theme; }

    // Animator
    qreal hover(const QWidget *w, quintptr id, bool on) override;
    qreal phase(const QWidget *w, int periodMs) override;
    bool enabled() const override { return m_animOn; }

private slots:
    void reloadTheme();
    void tick();
    void forget(QObject *o);

private:
    Theme m_theme;
    QVector<Module *> m_modules;
    QFileSystemWatcher m_watcher;
    bool m_debug = false;

    // ---- animation state (see Animator) ----
    struct Hot { qreal v = 0, target = 0; };
    QHash<const QObject *, QHash<quintptr, Hot>> m_hover;
    QHash<const QObject *, qint64> m_phase;        // widget -> last time phase() was asked (ms)
    QTimer m_tick;
    QElapsedTimer m_clock;
    qint64 m_last = 0;
    bool m_animOn = true;
    int m_inMs = 110, m_outMs = 240;
    void kick();
    void applyAnimSettings();
};
