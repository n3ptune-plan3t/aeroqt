#pragma once
#include <QColor>
#include <QJsonObject>
#include <QPalette>
#include <QString>

// Theme = JSON tokens. Colors: "#rrggbb", "#aarrggbb", "@role" (palette role),
// or a mix "@a+@b:0.3" (30% of b into a). Palette-driven => KDE color schemes just work.
class Theme {
public:
    bool load(const QString &path);
    bool loadDefault();
    static QString userPath();           // $AEROQT_THEME or ~/.config/aeroqt/theme.json
    QString name() const { return m_name; }

    qreal num(const QString &key, qreal def) const;                        // "button.radius"
    QColor color(const QString &key, const QPalette &pal, const QColor &def) const;

    static QColor mix(const QColor &a, const QColor &b, qreal t);
    // Whichever of Text / HighlightedText contrasts better with `bg` (text over tinted glass).
    static QColor contrastText(const QColor &bg, const QPalette &pal, QPalette::ColorGroup g);
    static QColor parseColor(const QString &s, const QPalette &pal, const QColor &def);

private:
    QJsonValue lookup(const QString &key) const;
    QJsonObject m_root;
    QString m_name = QStringLiteral("none");
};
