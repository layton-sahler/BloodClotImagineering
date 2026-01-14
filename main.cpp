#include <QQmlApplicationEngine>
#include <QObject>
#include <QSystemTrayIcon>
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication::setApplicationName("BloodClotSensorApp");
    QApplication::setOrganizationName("Imagineering");
    
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Use the path created by qt_add_qml_module
    const QUrl url(QStringLiteral("qrc:/qt/qml/SensorApp/main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(QIcon(":/assets/icon.png"), &app);
    trayIcon->show();
    trayIcon->showMessage("SensorApp Active", "We will notify you of sensor changes.", QSystemTrayIcon::Information, 3000);
    return app.exec();
}