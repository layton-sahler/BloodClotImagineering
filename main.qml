import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import SensorApp
import QtCharts

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 640
    height: 480
    title: "Bloodclot Imagineering"

    BluetoothManager {
        id: btManager
    }

    StackView {
        id: appStack
        anchors.fill: parent
        initialItem: scanPage 
    }

    Component {
        id: scanPage
        Page {
            id: scanView
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Layout.alignment: Qt.AlignTop

                    Button {
                        id: scanButton
                        onClicked: btManager.startScan()
                        contentItem: Text {
                            text: "Start Scan"
                            font.pixelSize: 20; color: "white"
                            horizontalAlignment: Text.AlignHCenter
                        }
                        background: Rectangle {
                            implicitWidth: 200; implicitHeight: 40
                            color: scanButton.pressed ? "#333333" : "black"
                            radius: 4
                        }
                    }
                    
                    Button {
                        Layout.preferredHeight: 40
                        text: "Enable Bluetooth"
                        onClicked: btManager.requestBluetoothEnable()
                    }
                }
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: btManager.devices
                    delegate: ItemDelegate {
                        width: ListView.view.width
                        text: modelData
                        onClicked: btManager.connectToDevice(index)
                    }
                }
            }

            Connections {
                target: btManager
                function onConnectedChanged() {
                    if(btManager.connected) {
                        statusPopup.open()
                    }
                }
            }

            Popup {
                id: statusPopup
                anchors.centerIn: parent
                width: 300; height: 200
                modal: true; focus: true
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                background: Rectangle { color: "#2ecc71"; radius: 10 }

                contentItem: Column {
                    spacing: 20; anchors.centerIn: parent
                    Text { text: "Connection Success!"; color: "white"; font.bold: true; font.pixelSize: 18 }
                    Button {
                        text: "Continue to App"
                        onClicked: {
                            statusPopup.close()
                            appStack.push(dashboardPage)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: dashboardPage
        Page {
            header: ToolBar {
                RowLayout {
                    anchors.fill: parent
                    Button {
                        text: "Disconnect"
                        onClicked: appStack.pop()
                    }
                    Label {
                        text: "Sensor Dashboard"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
            property int sampleCount: 0
            property int maxVisibleSamples: 100

            ColumnLayout {
                anchors.fill: parent

                ChartView {
                    id: chartView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    theme: ChartView.ChartThemeDark

                    // 1. Define Axes as standalone objects
                    ValueAxis { 
                        id: xAxis
                        min: 0
                        max: maxVisibleSamples 
                    }

                    ValueAxis {
                        id: axisY
                        min: 0.0
                        max: 3.3
                        tickCount: 5
                        labelFormat: "%.2f V" // Shows 2 decimal places on the axis
                    }

                    LineSeries {
                        id: ecgSeries
                        name: "ECG"
                        color: "#00FF41"
                        width: 3 
                        visible: true 
                        opacity: 1.0
                        axisX: xAxis
                        axisY: yAxis
                    }

                    // 2. Attach them ONLY here
                    Component.onCompleted: {
                        ecgSeries.attachAxis(xAxis);
                        ecgSeries.attachAxis(yAxis);
                    }
                }
                
                Button {
                    text: "Back"
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: appStack.pop()
                }
            }
            
            Connections {
                target: btManager
                function onDataReceived(ecg, stretch, accX, accY, accZ) {
                    let x = parseFloat(sampleCount);
                    let y = parseFloat(ecg);

                    if (!isNaN(x) && !isNaN(y)) {
                        ecgSeries.append(x, y);
                    } else {
                        console.log("Invalid data received:", ecg);
                    }                    
                    
                    let ecgValue = Number(ecg)
                                        
                    if (sampleCount > maxVisibleSamples) {
                        xAxis.min++
                        xAxis.max++
                        
                        if (ecgSeries.count > maxVisibleSamples + 20) {
                            ecgSeries.remove(0)
                        }
                    }

                    // Auto-scale Y-axis so the wave doesn't go off-screen
                    if (ecgValue > yAxis.max) yAxis.max = ecgValue + 100
                    if (ecgValue < yAxis.min) yAxis.min = ecgValue - 100
                    
                    sampleCount++
                }
            }
        }
    }

    // --- GLOBAL ELEMENTS ---

    Rectangle {
        id: notificationPopup
        z: 100
        visible: false
        width: 300; height: 60
        color: "#333333"; border.color: "orange"; border.width: 2; radius: 8
        anchors.horizontalCenter: parent.horizontalCenter
        y: -height

        property alias titleText: notifyTitle.text
        property alias bodyText: notifyBody.text

        Column {
            anchors.centerIn: parent
            Text { id: notifyTitle; color: "orange"; font.bold: true }
            Text { id: notifyBody; color: "white"; font.pixelSize: 12 }
        }

        NumberAnimation on y { id: slideDown; to: 20; duration: 500; easing.type: Easing.OutBack }
        NumberAnimation on y { id: slideUp; to: -notificationPopup.height; duration: 500; easing.type: Easing.InBack }
    }

    Connections {
        target: NotifManager 
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

    Connections {
        target: btManager
        function onConnectedChanged() {
            if (!btManager.connected && appStack.depth > 1) {
                appStack.pop(null) 
            }
        }
    }
}