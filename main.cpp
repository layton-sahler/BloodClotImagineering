#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "btmanager.h"
#include "notifmanager.h"
#include <QQuickStyle>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[]) {
    QQuickStyle::setStyle("Basic");
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    NotifManager myNotif; 
    engine.rootContext()->setContextProperty("NotifManager", &myNotif);

    // This path is now locked by your RESOURCE_PREFIX in CMake
    const QUrl url(u"qrc:/qt/qml/SensorApp/main.qml"_s);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}