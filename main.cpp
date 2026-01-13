#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QObject>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Use the path created by qt_add_qml_module
    const QUrl url(QStringLiteral("qrc:/qt/qml/SensorApp/main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}