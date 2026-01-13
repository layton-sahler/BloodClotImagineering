import QtQuick
import QtQuick.Window
import QtQuick.Controls
import SensorApp 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: "Sensor App (C++ Backend)"

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Button {
            text: "Start Scan"
            onClicked: btManager.startScan()
        }

        ListView {
            width: parent.width
            height: 400
            clip: true
            model: btManager.devices
            
            // The delegate must stay inside the ListView
                delegate: ItemDelegate {
                width: parent.width
                text: modelData
                onClicked: {
                    console.log("Connecting to index:", index)
                    // 'index' is a built-in property of the ListView delegate
                    btManager.connectToDevice(index) 
                }
            }
        }
    }
    BluetoothManager {
        id: btManager
    }
}