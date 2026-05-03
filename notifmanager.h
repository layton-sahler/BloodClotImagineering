#ifndef NOTIFMANAGER_H
#define NOTIFMANAGER_H

#include <QObject>
#include <QString>

class NotifManager : public QObject {
    Q_OBJECT
public:
    explicit NotifManager(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void createInternalAlert(const QString &title, const QString &message) {
        emit newNotification(title, message);
    }

signals:
    void newNotification(QString title, QString message);
};
#endif