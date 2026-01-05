#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    
    // This matches the URI "SensorApp" set in your CMakeLists.txt
    const QUrl url(u"qrc:/SensorApp/main.qml"_qs);

    // This connection tells you EXACTLY if the loading failed
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QUEUED_CONNECTION);

    engine.load(url);

    return app.exec();
}