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
#include "DataProcessing.h"

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
        
        connect(this, &BluetoothManager::connectedChanged, [](){ qDebug() << "UI Sync: Connection state updated."; });
    }

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

        connect(m_controller, &QLowEnergyController::connected, this, [this](){
            qDebug() << "CONNECTED! Starting service discovery...";
            emit connectedChanged();
            m_controller->discoverServices(); 
        });

        connect(m_controller, &QLowEnergyController::disconnected, this, [this](){
            qDebug() << "Disconnected from device.";
            emit connectedChanged();
        });

        connect(m_controller, &QLowEnergyController::discoveryFinished, this, [this](){
            qDebug() << "Discovery Finished. Searching for Sensor Service...";
            QLowEnergyService *service = m_controller->createServiceObject(
                QBluetoothUuid(QString("4fafc201-1fb5-459e-8fcc-c5c9c331914b")), this);

            if (service) {
                connect(service, &QLowEnergyService::stateChanged, this, [this, service](QLowEnergyService::ServiceState state) {
                    
                    if (state == QLowEnergyService::RemoteServiceDiscovered) {
                        qDebug() << "SUCCESS: Service details discovered. Looking for Characteristic...";

                        QLowEnergyCharacteristic sensorChar = service->characteristic(
                            QBluetoothUuid(QString("beb5483e-36e1-4688-b7f5-ea07361b26a8")));

                        if (sensorChar.isValid()) {
                            qDebug() << "Characteristic found! Subscribing...";

                            QLowEnergyDescriptor notification = sensorChar.descriptor(
                                QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
                            if (notification.isValid()) {
                                service->writeDescriptor(notification, QByteArray::fromHex("0100"));
                            }

                            connect(service, &QLowEnergyService::characteristicChanged, 
                                this, [this](const QLowEnergyCharacteristic &c, const QByteArray &value){
                                
                            if (c.uuid() == QBluetoothUuid(QString("beb5483e-36e1-4688-b7f5-ea07361b26a8"))) {
                                QString rawString = QString::fromUtf8(value);
                                QStringList dataParts = rawString.split(',');

                                qDebug() << "New Packet Received:" << rawString;

                                if (dataParts.size() < 6) return; 

                                SensorMetrics metrics = m_processor.processRawPacket(dataParts, true);

                                emit metricsUpdated(metrics.stretchValue, 
                                                    metrics.bpm, 
                                                    metrics.pulse,
                                                    metrics.isMoving,
                                                    metrics.restingScore,
                                                    metrics.isConnected,
                                                    metrics.immobility
                                                    );
                            }
                        });
                        } else {
                            qDebug() << "ERROR: Characteristic UUID not found in service!";
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
    void metricsUpdated(double stretch, double bpm, double pulse, bool isMoving, double restingScore, bool connected, double immobility);
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
    DataProcessing m_processor;
    QLowEnergyController *m_controller = nullptr;
    QBluetoothDeviceDiscoveryAgent m_agent;
    QStringList m_deviceList;
    QList<QBluetoothDeviceInfo> m_foundDevices;
};

#endif