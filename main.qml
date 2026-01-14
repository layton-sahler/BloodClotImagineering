import QtQuick
import QtQuick.Window
import QtQuick.Controls
import SensorApp 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: "Sensor App (C++ Backend)"
    color: "#f0f0f0" // Set window background to light grey to see white buttons

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // --- SCAN BUTTON ---
        Button {
            text: "Start Scan"
            contentItem: Text {
                text: parent.text
                font: parent.font
                color: "black"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                implicitWidth: 200
                implicitHeight: 40
                color: parent.down ? "#e0e0e0" : "white" 
                border.color: "black"
                border.width: 1
                radius: 4
            }
            onClicked: btManager.startScan()
        }

        // --- DEVICE LIST ---
        ListView {
            width: parent.width
            height: 300 // Reduced height to fit the other button
            clip: true
            model: btManager.devices
            delegate: ItemDelegate {
                width: parent.width
                text: modelData
                onClicked: {
                    console.log("Connecting to index:", index)
                    btManager.connectToDevice(index) 
                }
            }
        }

        // --- PERMISSIONS BUTTON ---
        Button {
            text: "Enable Bluetooth / Test Notification"
            contentItem: Text {
                text: parent.text
                font: parent.font
                color: "black"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                implicitWidth: 200
                implicitHeight: 40
                color: parent.down ? "#e0e0e0" : "white"
                border.color: "black"
                border.width: 1
                radius: 4
            }
            onClicked: {
                btManager.requestBluetoothEnable()
            }
        }
    }

    BluetoothManager {
        id: btManager
    }
}