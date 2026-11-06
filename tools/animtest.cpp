// aeroqt-animtest : headless check of the animation engine (hover fade, progress marquee).
//   QT_QPA_PLATFORM=offscreen ./aeroqt-animtest            -> expects animations ON
//   AEROQT_NO_ANIM=1 QT_QPA_PLATFORM=offscreen ./aeroqt-animtest   -> expects them OFF
// Exit code 0 = all checks passed.
#include "aerostyle.h"
#include <QApplication>
#include <QEventLoop>
#include <QImage>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QStyleOptionButton>
#include <QStyleOptionProgressBar>
#include <QTimer>
#include <cstdio>

static void wait(int ms)
{
    QEventLoop l;
    QTimer::singleShot(ms, &l, &QEventLoop::quit);
    l.exec();
}

static QImage drawButton(QPushButton *b, bool hover)
{
    QImage img(100, 30, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    QStyleOptionButton o;
    o.initFrom(b);
    o.rect = img.rect();
    o.state = QStyle::State_Enabled | (hover ? QStyle::State_MouseOver : QStyle::State_None);
    b->style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &o, &p, b);
    return img;
}

static QImage drawBusy(QProgressBar *pb)
{
    QImage img(200, 16, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    QStyleOptionProgressBar o;
    o.initFrom(pb);
    o.rect = img.rect();
    o.minimum = o.maximum = o.progress = 0;
    pb->style()->drawControl(QStyle::CE_ProgressBarContents, &o, &p, pb);
    return img;
}

static long dist(const QImage &a, const QImage &b)
{
    long d = 0;
    for (int y = 0; y < a.height(); ++y)
        for (int x = 0; x < a.width(); ++x) {
            const QRgb p = a.pixel(x, y), q = b.pixel(x, y);
            d += qAbs(qRed(p) - qRed(q)) + qAbs(qGreen(p) - qGreen(q)) + qAbs(qBlue(p) - qBlue(q));
        }
    return d;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    auto *style = new AeroStyle;
    app.setStyle(style);
    const bool expectAnim = !qEnvironmentVariableIsSet("AEROQT_NO_ANIM");
    int fails = 0;
    auto check = [&](bool ok, const char *what) { std::printf("%s  %s\n", ok ? "ok  " : "FAIL", what); if (!ok) ++fails; };

    check(style->enabled() == expectAnim, "style->enabled() matches AEROQT_NO_ANIM");

    QPushButton b("x");
    const QImage base = drawButton(&b, false);
    const long d0 = dist(drawButton(&b, true), base);
    wait(50);
    const long d1 = dist(drawButton(&b, true), base);
    wait(300);
    const long d2 = dist(drawButton(&b, true), base);
    std::printf("hover-in  distances: t0=%ld t50=%ld t350=%ld\n", d0, d1, d2);
    check(d2 > 0, "hovered button differs from rest state");
    if (expectAnim) {
        check(d0 < d1 && d1 < d2, "hover fades in monotonically (t0 < t50 < t350)");
        check(d0 < d2 / 4, "first hovered paint starts near the rest state");
    } else {
        check(d0 == d2, "animations off: hover is instant");
    }
    const long e0 = dist(drawButton(&b, false), base);
    wait(100);
    const long e1 = dist(drawButton(&b, false), base);
    wait(400);
    const long e2 = dist(drawButton(&b, false), base);
    std::printf("hover-out distances: t0=%ld t100=%ld t500=%ld\n", e0, e1, e2);
    check(e2 == 0, "hover fades fully out (back to rest state)");
    if (expectAnim) check(e0 > e1 && e1 > 0, "hover fades out gradually (t0 > t100 > 0)");

    QProgressBar pb;
    pb.setRange(0, 0);
    const QImage m0 = drawBusy(&pb);
    wait(400);
    const QImage m1 = drawBusy(&pb);
    const long md = dist(m0, m1);
    std::printf("marquee distance over 400ms: %ld\n", md);
    check(expectAnim ? md > 0 : md == 0, expectAnim ? "indeterminate marquee moves" : "animations off: marquee is static");
    std::printf(fails ? "\n%d check(s) FAILED\n" : "\nall checks passed\n", fails);
    return fails ? 1 : 0;
}
