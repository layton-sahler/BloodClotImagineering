#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>      // Required for setContextProperty
#include <QSystemTrayIcon>
#include <QIcon>
#include "notifmanager.h" // Ensure this matches your filename

int main(int argc, char *argv[]) {
    // 1. App Setup
    QApplication::setApplicationName("BloodClotSensorApp");
    QApplication::setOrganizationName("SciLabImagineering");
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // 2. Instantiate and Register your Manager FIRST
    // We do this BEFORE engine.load so QML knows what "notifManager" is immediately
    NotifManager myNotifManager; 
    engine.rootContext()->setContextProperty("NotifManager", &myNotifManager);

    // 3. Load QML
    const QUrl url(QStringLiteral("qrc:/qt/qml/SensorApp/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    // 4. System Tray Setup (Kept exactly as you had it)
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(QIcon(":/assets/icon.png"), &app);
    trayIcon->show();
    trayIcon->showMessage("SensorApp Active", "We will notify you of sensor changes.", 
                          QSystemTrayIcon::Information, 3000);

    return app.exec();
}