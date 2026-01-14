import QtQuick
import QtQuick.Window
import QtQuick.Controls
import SensorApp 1.0

Window {
    id: rootWindow
    width: 640
    height: 480
    visible: true
    title: "Bloodclot Imagineering"
    color: "#ffffff"

    // --- C++ MANAGERS ---
    BluetoothManager {
        id: btManager
    }

    // --- MAIN UI LAYOUT ---
    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Button {
            text: "Start Scan"
            background: Rectangle {
                implicitWidth: 200; implicitHeight: 40
                color: "Black"
                border.color: "black"; radius: 4
            }
            onClicked: btManager.startScan()
        }

        ListView {
            width: parent.width
            height: 250
            clip: true
            model: btManager.devices
            delegate: ItemDelegate {
                width: parent.width
                text: modelData
                onClicked: btManager.connectToDevice(index)
            }
        }

        Button {
            text: "Enable Bluetooth / Test Notification"
            background: Rectangle {
                implicitWidth: 200; implicitHeight: 40
                color: "Black"
                border.color: "black"; radius: 4
            }
            onClicked: btManager.requestBluetoothEnable()
        }
    }

    // --- NOTIFICATION UI (Placed last so it stays on top) ---
    Rectangle {
        id: notificationPopup
        z: 100 // Ensures it stays above the ListView
        visible: false
        width: 300
        height: 50
        color: "#333333" // Dark background
        border.color: "orange"
        border.width: 2
        radius: 8
        anchors.horizontalCenter: parent.horizontalCenter
        y: -height // Initial position (hidden)

        // Custom properties so your C++ signal can set the text
        property alias titleText: notifyTitle.text
        property alias bodyText: notifyBody.text

        Column {
            anchors.centerIn: parent
            Text { id: notifyTitle; color: "orange"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
            Text { id: notifyBody; color: "white"; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter }
        }

        // Animations
        NumberAnimation on y { id: slideDown; to: 20; duration: 500; easing.type: Easing.OutBack }
        NumberAnimation on y { id: slideUp; to: -notificationPopup.height; duration: 500; easing.type: Easing.InBack }
    }

    Connections {
        target: NotifManager // Must match the name in your setContextProperty in main.cpp
        function onNewNotification(title, message) {
            notificationPopup.titleText = title
            notificationPopup.bodyText = message
            notificationPopup.visible = true
            slideDown.start()
            hideTimer.start()
        }
    }

    Timer {
        id: hideTimer
        interval: 3000
        onTriggered: slideUp.start()
    }
}