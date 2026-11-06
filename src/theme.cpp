#include "theme.h"
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QStandardPaths>

static bool readJson(const QString &path, QJsonObject &out)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QJsonParseError err;
    const auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return false;
    out = doc.object();
    return true;
}

QString Theme::userPath()
{
    const QByteArray env = qgetenv("AEROQT_THEME");
    if (!env.isEmpty()) return QString::fromLocal8Bit(env);
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
           + QStringLiteral("/aeroqt/theme.json");
}

// Built-in themes are complete files in themes/ (:/aeroqt/*.json).
// Select one with $AEROQT_THEME_NAME=flat-dark|win7-classic (default frutiger-aero).
static QString builtinPath()
{
    const QByteArray n = qgetenv("AEROQT_THEME_NAME");
    if (n == "flat-dark") return QStringLiteral(":/aeroqt/flat-dark.json");
    if (n == "win7-classic") return QStringLiteral(":/aeroqt/win7-classic.json");
    return QStringLiteral(":/aeroqt/frutiger-aero.json");
}

bool Theme::loadDefault() { return load(builtinPath()); }

bool Theme::load(const QString &path)
{
    QJsonObject o;
    if (!readJson(path, o)) return false;
    // Built-in themes load as-is; user themes only override what they set (deep-merge over the selected built-in).
    if (!path.startsWith(QStringLiteral(":/aeroqt/"))) {
        QJsonObject base;
        readJson(builtinPath(), base);
        for (auto it = o.begin(); it != o.end(); ++it) {
            if (it->isObject() && base.value(it.key()).isObject()) {
                QJsonObject merged = base.value(it.key()).toObject();
                const QJsonObject over = it->toObject();
                for (auto j = over.begin(); j != over.end(); ++j) merged[j.key()] = j.value();
                base[it.key()] = merged;
            } else base[it.key()] = it.value();
        }
        o = base;
    }
    m_root = o;
    m_name = o.value(QStringLiteral("name")).toString(QStringLiteral("unnamed"));
    return true;
}

QJsonValue Theme::lookup(const QString &key) const
{
    QJsonValue v = m_root;
    for (const QString &part : key.split(QLatin1Char('.'))) {
        if (!v.isObject()) return QJsonValue::Undefined;
        v = v.toObject().value(part);
    }
    return v;
}

qreal Theme::num(const QString &key, qreal def) const
{
    const QJsonValue v = lookup(key);
    return v.isDouble() ? v.toDouble() : def;
}

QColor Theme::mix(const QColor &a, const QColor &b, qreal t)
{
    return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * t,
                            a.greenF() + (b.greenF() - a.greenF()) * t,
                            a.blueF() + (b.blueF() - a.blueF()) * t,
                            a.alphaF() + (b.alphaF() - a.alphaF()) * t);
}

QColor Theme::contrastText(const QColor &bg, const QPalette &pal, QPalette::ColorGroup g)
{
    const QColor a = pal.color(g, QPalette::Text), b = pal.color(g, QPalette::HighlightedText);
    return qAbs(a.lightnessF() - bg.lightnessF()) >= qAbs(b.lightnessF() - bg.lightnessF()) ? a : b;
}

static QColor roleColor(const QString &r, const QPalette &p, const QColor &def)
{
    struct { const char *n; QPalette::ColorRole role; } map[] = {
        {"window", QPalette::Window}, {"windowText", QPalette::WindowText},
        {"base", QPalette::Base}, {"text", QPalette::Text},
        {"button", QPalette::Button}, {"buttonText", QPalette::ButtonText},
        {"highlight", QPalette::Highlight}, {"highlightedText", QPalette::HighlightedText},
        {"light", QPalette::Light}, {"mid", QPalette::Mid}, {"dark", QPalette::Dark},
        {"shadow", QPalette::Shadow},
    };
    for (const auto &m : map)
        if (r == QLatin1String(m.n)) return p.color(m.role);
    return def;
}

QColor Theme::parseColor(const QString &s, const QPalette &pal, const QColor &def)
{
    auto one = [&](QString t) {
        t = t.trimmed();
        return t.startsWith(QLatin1Char('@')) ? roleColor(t.mid(1), pal, def) : QColor(t);
    };
    const int plus = s.indexOf(QLatin1Char('+'));
    if (plus < 0) { const QColor c = one(s); return c.isValid() ? c : def; }
    const QString rhs = s.mid(plus + 1);
    const int colon = rhs.lastIndexOf(QLatin1Char(':'));
    const qreal t = colon >= 0 ? rhs.mid(colon + 1).toDouble() : 0.5;
    return mix(one(s.left(plus)), one(colon >= 0 ? rhs.left(colon) : rhs), t);
}

QColor Theme::color(const QString &key, const QPalette &pal, const QColor &def) const
{
    const QJsonValue v = lookup(key);
    return v.isString() ? parseColor(v.toString(), pal, def) : def;
}
