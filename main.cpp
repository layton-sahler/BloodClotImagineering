#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QObject>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Use the modern Qt 6 URI (standard for qt_add_qml_module)
    // If this fails, try the shorter one: u"qrc:/SensorApp/main.qml"_s
    const QUrl url(u"qrc:/qt/qml/SensorApp/main.qml"_s);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}