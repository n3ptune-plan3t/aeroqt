#pragma once
#include <QColor>
#include <QHash>
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
    // Parsed color token ("@a+@b:0.3" split once, reused per paint). Palette
    // resolution still happens per call (the palette itself differs per
    // widget / color group), but the string work is cached: Fusion reads
    // QPalette directly with zero string parsing, so this cache is what
    // keeps our per-paint cost at that level (vs Kvantum's per-frame SVG
    // / QtSvg work or QSS selector matching per widget).
    struct ParsedColor {
        bool mix = false;
        QString a, b;
        qreal t = 0.5;
    };
    QJsonValue lookup(const QString &key) const;
    QColor resolveParsed(const ParsedColor &pc, const QPalette &pal, const QColor &def) const;
    static ParsedColor splitColorString(const QString &s);
    QJsonObject m_root;
    QString m_name = QStringLiteral("none");
    mutable QHash<QString, QJsonValue> m_lookupCache;
    mutable QHash<QString, ParsedColor> m_colorCache;
};
