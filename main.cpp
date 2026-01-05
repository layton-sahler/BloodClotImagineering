#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QObject>

using namespace Qt::StringLiterals; // Add this line!

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    
    // Change _qs to _s and update the path to the new Policy default
    const QUrl url(u"qrc:/qt/qml/SensorApp/main.qml"_s); 

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}