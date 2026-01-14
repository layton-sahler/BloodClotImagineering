#ifndef BTMANAGER_H
#define BTMANAGER_H
#include <QStyle>
#include <QApplication>
#include <QtWidgets/QSystemTrayIcon>
#include <QtBluetooth/QLowEnergyService>
#include <QtBluetooth/QLowEnergyCharacteristic>
#include <QtBluetooth/QLowEnergyDescriptor>
#include <QtGui/QIcon>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QtQml/qqmlregistration.h>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QBluetoothLocalDevice>

class BluetoothManager : public QObject {
    Q_OBJECT
    QML_ELEMENT // This handles the registration automatically
    Q_PROPERTY(QStringList devices READ devices NOTIFY devicesChanged)

public:
    explicit BluetoothManager(QObject *parent = nullptr) : QObject(parent) {
    // Add this to check local adapter state
    QBluetoothLocalDevice localDevice;
    if (localDevice.isValid()) {
        qDebug() << "Local Adapter Name:" << localDevice.name();
        qDebug() << "Local Adapter Address:" << localDevice.address().toString();
    } else {
        qDebug() << "ERROR: No local Bluetooth adapter found!";
    }

    connect(&m_agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, 
            this, &BluetoothManager::addDevice);
}

    Q_INVOKABLE void startScan() {
        m_deviceList.clear();
        m_foundDevices.clear();
        emit devicesChanged();
        m_agent.start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
        qDebug() << "Scanning...";
    }
    
    Q_INVOKABLE void requestBluetoothEnable() {
        QBluetoothLocalDevice localDevice;
        
        // Create the tray icon if it doesn't exist, or use a member variable
        static QSystemTrayIcon *tray = new QSystemTrayIcon(this);
        
        // Windows requires an icon to show a message!
        // This uses a standard system icon so you don't have to provide a file yet.
        tray->setIcon(qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation));
        tray->show(); 

        if (localDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
            qDebug() << "Bluetooth is off. Directing user and sending toast...";
            
            // 1. Send the Notification
            tray->showMessage("Bluetooth Required", 
                            "Please turn on Bluetooth to connect your sensor.", 
                            QSystemTrayIcon::Warning, 3000);
                            
            // 2. Open Settings
            QDesktopServices::openUrl(QUrl("ms-settings:bluetooth"));
        } else {
            tray->showMessage("SensorApp", "Bluetooth is active!", QSystemTrayIcon::Information, 2000);
        }
    }

    Q_INVOKABLE void connectToDevice(int index) {
        if (index < 0 || index >= m_foundDevices.count()) return;
        
        if (m_controller) {
            m_controller->disconnectFromDevice();
            m_controller->deleteLater();
        }

        m_controller = QLowEnergyController::createCentral(m_foundDevices[index], this);
        connect(m_controller, &QLowEnergyController::connected, this, [](){
            qDebug() << "Connected!";
        });
        m_controller->connectToDevice();
    }

    QStringList devices() const { return m_deviceList; }

signals:
    void devicesChanged();

private slots:
    void addDevice(const QBluetoothDeviceInfo &info) {
        QString label = info.name() + " (" + info.address().toString() + ")";
        if (!m_deviceList.contains(label)) {
            m_deviceList.append(label);
            m_foundDevices.append(info);
            emit devicesChanged();
            qDebug() << "Found:" << label;
        }
    }

private:
    QLowEnergyController *m_controller = nullptr;
    QBluetoothDeviceDiscoveryAgent m_agent;
    QStringList m_deviceList;
    QList<QBluetoothDeviceInfo> m_foundDevices;
};

#endif