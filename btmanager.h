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
    QML_ELEMENT
    Q_PROPERTY(QStringList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
    explicit BluetoothManager(QObject *parent = nullptr) : QObject(parent) {
        QBluetoothLocalDevice localDevice;
        if (localDevice.isValid()) {
            qDebug() << "Local Adapter:" << localDevice.name() << localDevice.address().toString();
        } else {
            qDebug() << "ERROR: No local Bluetooth adapter found!";
        }

        connect(&m_agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, 
                this, &BluetoothManager::addDevice);
        
        // Ensure UI updates if state changes unexpectedly
        connect(this, &BluetoothManager::connectedChanged, [](){ qDebug() << "UI Sync: Connection state updated."; });
    }

    // Destructor
    ~BluetoothManager() {
        if (m_controller) {
            m_controller->disconnectFromDevice();
        }
        m_agent.stop();
    }

    bool isConnected() const { 
        return m_controller && m_controller->state() == QLowEnergyController::ConnectedState; 
    }

    Q_INVOKABLE void startScan() {
        m_deviceList.clear();
        m_foundDevices.clear();
        emit devicesChanged();
        m_agent.stop();
        m_agent.start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
        qDebug() << "Scanning...";
    }

    Q_INVOKABLE void connectToDevice(int index) {
        m_agent.stop(); 
        if (index < 0 || index >= m_foundDevices.count()) return;

        if (m_controller) {
            m_controller->disconnectFromDevice();
            m_controller->deleteLater();
        }

        m_controller = QLowEnergyController::createCentral(m_foundDevices[index], this);
        m_controller->setRemoteAddressType(QLowEnergyController::RandomAddress);

        // Link established
        connect(m_controller, &QLowEnergyController::connected, this, [this](){
            qDebug() << "CONNECTED! Starting service discovery...";
            emit connectedChanged(); // Update QML visibility
            m_controller->discoverServices(); 
        });

        // Link broken
        connect(m_controller, &QLowEnergyController::disconnected, this, [this](){
            qDebug() << "Disconnected from device.";
            emit connectedChanged(); // Update QML visibility
        });

        // Service discovery finished
        connect(m_controller, &QLowEnergyController::discoveryFinished, this, [this](){
            qDebug() << "Discovery Finished. Searching for Sensor Service...";
            
            // 1. Create the Service Object
            QLowEnergyService *service = m_controller->createServiceObject(
                QBluetoothUuid(QString("4fafc201-1fb5-459e-8fcc-c5c9c331914b")), this);

            if (service) {
                // 4. Finally, connect the data signal to THIS specific characteristic
                connect(service, &QLowEnergyService::characteristicChanged, 
                        this, [this](const QLowEnergyCharacteristic &c, const QByteArray &value){
                    
                    if (c.uuid() == QBluetoothUuid(QString("beb5483e-36e1-4688-b7f5-ea07361b26a8"))) {
                        QString rawString = QString::fromUtf8(value);
                        QStringList dataParts = rawString.split(',');

                        if (dataParts.size() >= 5) {
                            // Convert to float for internal logic/filtering if needed
                            bool ok;
                            float ecgVal = dataParts[0].toFloat(&ok);

                            if (ok) {
                                // Determine if it's a valid voltage or a status code
                                if (ecgVal <= 1.01f && ecgVal >= 0.99f) {
                                    qDebug() << "STATUS: Leads are DETACHED";
                                } else if (ecgVal == 0.0f) {
                                    qDebug() << "STATUS: Sensor Muted/Saturated";
                                } else {
                                    // Valid floating point waveform data
                                    // qDebug() << "ECG Voltage:" << ecgVal << "V";
                                }
                            }

                            // Emit the full list of strings so the QML/Chart can plot them
                            emit dataReceived(dataParts);
                            
                            // Debugging output
                            qDebug() << "Parsed -> ECG:" << dataParts[0] << "V | Stretch:" << dataParts[1];
                        }
                    }
                });
                service->discoverDetails();
            } else {
                qDebug() << "ERROR: Sensor Service UUID not found.";
            }
        });

        connect(m_controller, &QLowEnergyController::errorOccurred, this, [this](QLowEnergyController::Error error){
            qDebug() << "Bluetooth Error:" << error;
            emit connectedChanged();
        });

        qDebug() << "Attempting to connect to" << m_foundDevices[index].name();
        m_controller->connectToDevice();
    }

    QStringList devices() const { return m_deviceList; }

signals:
    void devicesChanged();
    void connectedChanged();
    void dataReceived(QStringList dataparts);

private slots:
    void addDevice(const QBluetoothDeviceInfo &info) {
        QString name = info.name();
        if (name.isEmpty()) name = "Unknown Device";

        for (const auto &existing : m_foundDevices) {
            if (existing.address() == info.address()) return;
        }

        m_deviceList.append(name + " [" + info.address().toString() + "]");
        m_foundDevices.append(info);
        emit devicesChanged();
    }

private:
    QLowEnergyController *m_controller = nullptr;
    QBluetoothDeviceDiscoveryAgent m_agent;
    QStringList m_deviceList;
    QList<QBluetoothDeviceInfo> m_foundDevices;
};

#endif