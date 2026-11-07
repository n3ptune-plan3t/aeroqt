// aeroqt-qmltest : loads every control of the AeroQt QQC2 style headless and checks behaviour.
//   QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software ./aeroqt-qmltest <qml-import-dir> <test.qml>
// Fails on any QML warning (missing import, unknown property...) or any failure reported by
// tools/qmltest.qml. Built only when Qt6 Quick + QuickControls2 are found (see CMakeLists.txt).
#include <QEventLoop>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickStyle>
#include <QTimer>
#include <QUrl>
#include <cstdio>

int main(int argc, char **argv)
{
    if (argc < 3) { std::fprintf(stderr, "usage: %s <qml-import-dir> <test.qml>\n", argv[0]); return 2; }
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.addImportPath(QString::fromLocal8Bit(argv[1]));
    QQuickStyle::setStyle(QStringLiteral("AeroQt"));

    int problems = 0;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [&](const QList<QQmlError> &l) {
        for (const QQmlError &e : l) { std::printf("QML warning: %s\n", qPrintable(e.toString())); ++problems; }
    });
    engine.load(QUrl::fromLocalFile(QString::fromLocal8Bit(argv[2])));
    if (engine.rootObjects().isEmpty()) { std::printf("FAIL: could not load %s with style AeroQt\n", argv[2]); return 1; }

    QEventLoop loop;                                   // let the window polish/layout first
    QTimer::singleShot(250, &loop, &QEventLoop::quit);
    loop.exec();

    QVariant ret;
    QMetaObject::invokeMethod(engine.rootObjects().first(), "failures", Q_RETURN_ARG(QVariant, ret));
    QTimer::singleShot(100, &loop, &QEventLoop::quit); // popups open asynchronously; re-check settles
    loop.exec();
    for (const QVariant &v : ret.toList()) { std::printf("FAIL: %s\n", qPrintable(v.toString())); ++problems; }

    if (problems) { std::printf("%d problem(s)\n", problems); return 1; }
    std::printf("AeroQt QML style OK\n");
    return 0;
}
