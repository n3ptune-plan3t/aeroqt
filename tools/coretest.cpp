// aeroqt-coretest : headless regression tests for the non-visual core.
//   QT_QPA_PLATFORM=offscreen ./aeroqt-coretest        (also run by ctest as "core")
// Covers: theme color/token parsing, user-override deep merge, Glass slab cache at
// DPR 1 and 2 (the cached pixmap must have the same size as an uncached slab),
// and animator bookkeeping (cold parts allocate nothing, no connection pile-up).
#include "aerostyle.h"
#include "glass.h"
#include "theme.h"
#include <QApplication>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
#include <cstdio>

static int g_fail = 0;
#define CHECK(cond)                                                                  \
    do {                                                                             \
        if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); ++g_fail; } \
    } while (0)

static void wait(int ms)
{
    QEventLoop l;
    QTimer::singleShot(ms, &l, &QEventLoop::quit);
    l.exec();
}

static bool near(const QColor &a, const QColor &b, int tol = 1)
{
    return qAbs(a.red() - b.red()) <= tol && qAbs(a.green() - b.green()) <= tol
           && qAbs(a.blue() - b.blue()) <= tol && qAbs(a.alpha() - b.alpha()) <= tol;
}

// ------------------------------------------------------------------ theme
static void testTheme()
{
    QPalette pal;
    pal.setColor(QPalette::Highlight, QColor(0, 0, 200));
    pal.setColor(QPalette::Button, QColor(100, 100, 100));
    pal.setColor(QPalette::Light, QColor(255, 255, 255));

    const QColor def(1, 2, 3);
    CHECK(near(Theme::parseColor("#ff0000", pal, def), QColor(255, 0, 0)));
    CHECK(near(Theme::parseColor("@highlight", pal, def), QColor(0, 0, 200)));
    CHECK(near(Theme::parseColor("@nope", pal, def), def));                      // unknown role -> default
    CHECK(near(Theme::parseColor("@button+@light:0.5", pal, def), QColor(177, 177, 177), 2));
    CHECK(near(Theme::parseColor("@button+@light:9", pal, def), QColor(255, 255, 255)));   // weight clamps to 1
    CHECK(near(Theme::parseColor("@button+@light:-3", pal, def), QColor(100, 100, 100))); // ... and to 0

    Theme t;
    CHECK(t.loadDefault());
    CHECK(t.name() == "Frutiger Aero");
    CHECK(t.num("button.radius", -1) == 6);
    CHECK(t.num("no.such.key", 42) == 42);
    // Walking *through* a non-object must be "undefined", not that value (was: returned the string).
    CHECK(t.num("name.radius", 7) == 7);
    CHECK(near(t.color("name.base", pal, def), def));
    CHECK(near(t.color("accent", pal, def), QColor(0, 0, 200)));                 // "@highlight"
    // cached path gives the same answer
    CHECK(near(t.color("accent", pal, def), QColor(0, 0, 200)));
}

static void testThemeMerge()
{
    QTemporaryDir dir;
    CHECK(dir.isValid());
    const QString path = dir.filePath("theme.json");
    QFile f(path);
    CHECK(f.open(QIODevice::WriteOnly));
    // Override one nested leaf; everything else must still come from the built-in.
    f.write(R"({"button":{"radius":13},"extra":{"deep":{"leaf":5}}})");
    f.close();

    Theme t;
    CHECK(t.load(path));
    CHECK(t.num("button.radius", -1) == 13);                                    // overridden
    CHECK(t.num("button.radius", -1) != 6);
    CHECK(t.color("button.base", QPalette(), QColor()).isValid());                // sibling survived
    CHECK(t.num("input.radius", -1) == 5);                                       // other groups survived
    CHECK(t.num("extra.deep.leaf", -1) == 5);                                    // new nested keys appear

    // Garbage must be rejected (the caller keeps the previous theme).
    QFile bad(dir.filePath("bad.json"));
    CHECK(bad.open(QIODevice::WriteOnly));
    bad.write("{ not json");
    bad.close();
    CHECK(!t.load(dir.filePath("bad.json")));
    CHECK(t.num("button.radius", -1) == 13);                                    // untouched by the failed load
}

// ------------------------------------------------------------------ glass cache / HiDPI
static QImage renderSlab(qreal dpr, const QRectF &r, qreal hoverT)
{
    QImage img(QSize(qRound(120 * dpr), qRound(40 * dpr)), QImage::Format_ARGB32_Premultiplied);
    img.setDevicePixelRatio(dpr);
    img.fill(Qt::white);
    QPainter p(&img);
    Glass::Spec s;
    s.base = QColor("#e2eef8");
    s.border = QColor("#5a6e82");
    s.glow = QColor("#2896e6");
    s.hoverT = hoverT;
    Glass::drawSlab(&p, r, s);
    return img;
}

// Rightmost / bottom-most non-white device pixel of the slab.
static QPoint extent(const QImage &img)
{
    int mx = -1, my = -1;
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            if (QColor(img.pixel(x, y)) != QColor(Qt::white)) { mx = qMax(mx, x); my = qMax(my, y); }
    return {mx, my};
}

static void testGlassDpr()
{
    for (qreal dpr : {1.0, 1.5, 2.0, 3.0}) {
        Glass::clearCache();
        const QRectF r(10, 5, 100, 30);
        // hoverT 0 = settled -> goes through the cache; 0.5 = mid-transition -> painted directly.
        const QImage miss = renderSlab(dpr, r, 0.0);     // fills the cache
        const QImage hit = renderSlab(dpr, r, 0.0);      // served from the cache
        const QImage direct = renderSlab(dpr, r, 0.5);   // never cached

        const QPoint em = extent(miss), ed = extent(direct);
        // A 100x30 logical slab at (10,5) ends at logical (110,35): within a physical pixel of that.
        CHECK(qAbs(em.x() - qRound(110 * dpr)) <= 2);
        CHECK(qAbs(em.y() - qRound(35 * dpr)) <= 2);
        // cached and uncached slabs must cover the same area (this was half size at DPR 2)
        CHECK(qAbs(em.x() - ed.x()) <= 1 && qAbs(em.y() - ed.y()) <= 1);
        // cache hit is pixel-identical to the miss that created it
        CHECK(miss == hit);
        if (miss != hit) std::printf("  (dpr %g)\n", dpr);
    }
    // Pixel check at DPR 2: a point well inside the slab but beyond half its size must be painted.
    Glass::clearCache();
    const QImage img = renderSlab(2.0, QRectF(10, 5, 100, 30), 0.0);
    CHECK(QColor(img.pixel(2 * 100, 2 * 20)) != QColor(Qt::white));
}

// ------------------------------------------------------------------ animator
struct ProbeButton : QPushButton {
    int destroyedReceivers() const { return receivers(SIGNAL(destroyed(QObject *))); }
};

static void testAnimator()
{
    AeroStyle style;
    ProbeButton b;
    b.show();

    // Cold parts allocate nothing.
    for (int i = 0; i < 200; ++i) CHECK(style.hover(&b, quintptr(i), false) == 0.0);
    CHECK(style.trackedParts() == 0);

    const int baseline = b.destroyedReceivers();
    // hover -> fade in -> unhover -> come to rest -> repeat. Each cycle re-creates the
    // entry; the destroyed() connection must not be added again every time.
    for (int cycle = 0; cycle < 6; ++cycle) {
        style.hover(&b, 1, true);
        wait(160);                                       // > hoverIn
        CHECK(style.hover(&b, 1, true) > 0.95);
        style.hover(&b, 1, false);
        wait(330);                                       // > hoverOut + one tick to erase it
        CHECK(style.hover(&b, 1, false) == 0.0);
    }
    CHECK(style.trackedParts() == 0);
    CHECK(b.destroyedReceivers() <= baseline + 1);
    if (b.destroyedReceivers() > baseline + 1)
        std::printf("  destroyed() receivers: baseline %d, now %d\n", baseline, b.destroyedReceivers());
}

int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", qEnvironmentVariableIsSet("QT_QPA_PLATFORM") ? qgetenv("QT_QPA_PLATFORM") : "offscreen");
    qunsetenv("AEROQT_NO_ANIM");              // the animator checks need animations on
    qunsetenv("AEROQT_THEME");
    qputenv("AEROQT_THEME", QByteArray("/nonexistent/aeroqt-test-theme.json"));
    QApplication app(argc, argv);

    testTheme();
    testThemeMerge();
    testGlassDpr();
    testAnimator();

    if (g_fail) { std::printf("%d check(s) failed\n", g_fail); return 1; }
    std::printf("all core checks passed\n");
    return 0;
}
