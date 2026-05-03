import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import SensorApp
import QtCharts
import QtQuick.Shapes

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 640
    height: 480
    title: "VENOSENSE"

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
                            appStack.push(dashboard)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: dashboard
        Page {
            header: ToolBar {
                implicitHeight: 80
                background: Rectangle {
                    color: "#c2c2c2"
                }
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    Button {
                        text: "Disconnect"
                        onClicked: appStack.pop()
                    }
                    Label {
                        text: "Dashboard"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 20
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 15
                Rectangle {
                    id: heartrateButton
                    Layout.fillWidth: true
                    height: 100
                    radius: 10
                    
                    color: hrHandler.hovered ? "#e0e0e0" : "#2ecc71" 
                    Behavior on color { ColorAnimation { duration: 200 } }

                    HoverHandler {
                        id: hrHandler
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "HEARTRATE MONITOR"
                        color: "white"
                        font.bold: true
                        font.pixelSize: 18
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            appStack.push(hrmonitor) 
                        }
                    }
                }

                Rectangle {
                    id: accelbutton
                    Layout.fillWidth: true
                    height: 100
                    radius: 10
                    
                    color: haHandler.hovered ? "#e0e0e0" : "#2ecc71" 
                    Behavior on color { ColorAnimation { duration: 200 } }

                    HoverHandler {
                        id: haHandler
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "ACCELEROMETER"
                        color: "white"
                        font.bold: true
                        font.pixelSize: 18
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            appStack.push(accelerometer) 
                        }
                    }
                }

                Rectangle {
                    id: stretchbutton
                    Layout.fillWidth: true
                    height: 100
                    radius: 10
                    
                    color: hsHandler.hovered ? "#e0e0e0" : "#2ecc71"
                    Behavior on color { ColorAnimation { duration: 200 } }

                    HoverHandler {
                        id: hsHandler
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "STRETCH SENSOR"
                        color: "white"
                        font.bold: true
                        font.pixelSize: 18
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            appStack.push(stretchsensor) 
                        }
                    }
                }
                Item { Layout.fillHeight: true }
                Button {
                    text: "Demo Notification"
                    onClicked: NotifManager.createInternalAlert("Warning: DEMO NOTIFICATION", "Detected increase in swelling and decrease in mobility. Movement is recommended.")
                }
                Item { Layout.fillHeight: true }
                Button {
                    text: "Mystery button..."
                    onClicked: NotifManager.createInternalAlert("HAPPY 20th BIRTHDAY TEJO!!!", "Congratulations! The person who pressed this button shall recieve 1x free Wow! Tikka meal (on Layton)")
                }
            }
        }
    }

    Component {
        id: stretchsensor
        Page {
            id: stretchPage
            property int stretch: 0
            property int tickCounter: 0

            // This is where we handle the data coming from C++
            Connections {
                target: btManager
                function onMetricsUpdated(stretch, bpm, pulse, moving, resting, connected, immobility) {
                    stretchPage.stretch = stretch;
                    
                    // Add the new point (0 or 1)
                    stretchSeries.append(stretchPage.tickCounter, stretch);
                    
                    // Auto-scroll the X-Axis
                    if (stretchPage.tickCounter > xAxis.max) {
                        xAxis.min++;
                        xAxis.max++;
                    }
                    
                    stretchPage.tickCounter++;
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                // 2. Added a big Status Box since 0/1 data is hard to see on a graph alone
                Rectangle {
                    Layout.fillWidth: true
                    height: 80
                    radius: 10
                    color: stretchPage.stretch === 1 ? "#00FF41" : "#333333"
                    border.color: "#666666"
                    
                    Label {
                        anchors.centerIn: parent
                        text: stretchPage.stretch === 1 ? "STRETCH DETECTED" : "NO STRETCH"
                        color: stretchPage.stretch === 1 ? "black" : "white"
                        font.pixelSize: 22
                        font.bold: true
                    }
                }

                ChartView {
                    id: chartView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    theme: ChartView.ChartThemeDark
                    antialiasing: true
                    legend.visible: false

                    ValueAxis { 
                        id: xAxis
                        min: 0
                        max: 50 // Shorter window makes 0/1 data look "bigger"
                        tickCount: 6 
                    }

                    ValueAxis { 
                        id: yAxis
                        min: -0.2 // Buffer below 0
                        max: 1.2  // Buffer above 1
                        tickCount: 2
                        labelFormat: "%d"
                        titleText: "Status (0=Off, 1=On)"
                    }

                    LineSeries {
                        id: stretchSeries
                        name: "Stretch"
                        color: "#00FF41"
                        width: 3
                        axisX: xAxis
                        axisY: yAxis
                    }
                }
            }
        }
    }
    Component {
        id: accelerometer
        Page {
            id: accelPage
            property double restingScore: 0.0
            property bool isMoving: false
            property double immobilityScore: 0.0

            Connections {
                target: btManager
                function onMetricsUpdated(stretch, bpm, pulse, moving, resting, connected, immobility) {
                    accelPage.restingScore = resting;
                    immobilityScore = immobility;
                    accelPage.isMoving = moving;
                }
            }

            header: ToolBar {
                implicitHeight: 80
                background: Rectangle {
                    color: "#c2c2c2"
                }
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    Button {
                        text: "Back"
                        onClicked: appStack.pop()
                    }
                    Label {
                        text: "Resting Score"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 20
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 30

                Item {
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredWidth: 300
                    Layout.preferredHeight: 300

                    id: stabilityWidget
                    property real value: restingScore
                    property real maxValue: 100.0
                    property color strokeColor: restingScore > 70 ? "#27ae60" : "#e67e22"
                    property color backgroundColor: "#333333" // Lighter grey so you can see the "track"
                    property int strokeWidth: 15

                    property real angle: (value / maxValue) * 360

                    Shape {
                        anchors.fill: parent
                        layer.enabled: true
                        layer.samples: 4

                        // Background Track
                        ShapePath {
                            strokeColor: stabilityWidget.backgroundColor
                            fillColor: "transparent"
                            strokeWidth: stabilityWidget.strokeWidth
                            capStyle: ShapePath.RoundCap
                            PathAngleArc {
                                centerX: 150; centerY: 150
                                radiusX: 130; radiusY: 130
                                startAngle: 0; sweepAngle: 360
                            }
                        }

                        // Progress Bar
                        ShapePath {
                            strokeColor: stabilityWidget.strokeColor
                            fillColor: "transparent"
                            strokeWidth: stabilityWidget.strokeWidth
                            capStyle: ShapePath.RoundCap
                            PathAngleArc {
                                centerX: 150; centerY: 150
                                radiusX: 130; radiusY: 130
                                startAngle: -90
                                sweepAngle: stabilityWidget.angle

                                Behavior on sweepAngle {
                                    NumberAnimation { duration: 600; easing.type: Easing.OutBack }
                                }
                            }
                        }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 5
                        Text {
                            text: Math.round(restingScore) + "%"
                            color: "black" // Changed to black for visibility on white bg, or "white" if themed
                            font.pixelSize: 48; font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: "STABILITY"
                            color: "gray"
                            font.pixelSize: 14; font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
                
                Text {
                    text: restingScore < 50 ? "Heavy Movement Detected" : "Device is Still"
                    Layout.alignment: Qt.AlignHCenter
                    font.italic: true
                    color: "#666"
                }

                Item { Layout.fillHeight: true }

                Item {
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredWidth: 300
                    Layout.preferredHeight: 300

                    id: immobilityWidget
                    property real value: immobilityScore
                    property real maxValue: 100.0
                    property color strokeColor: immobilityScore > 70 ? "#e67e22" : "#27ae60"
                    property color backgroundColor: "#333333"
                    property int strokeWidth: 15

                    property real angle: (value / maxValue) * 360

                    Shape {
                        anchors.fill: parent
                        layer.enabled: true
                        layer.samples: 4

                        ShapePath {
                            strokeColor: immobilityWidget.backgroundColor
                            fillColor: "transparent"
                            strokeWidth: immobilityWidget.strokeWidth
                            capStyle: ShapePath.RoundCap
                            PathAngleArc {
                                centerX: 150; centerY: 150
                                radiusX: 130; radiusY: 130
                                startAngle: 0; sweepAngle: 360
                            }
                        }

                        ShapePath {
                            strokeColor: immobilityWidget.strokeColor
                            fillColor: "transparent"
                            strokeWidth: immobilityWidget.strokeWidth
                            capStyle: ShapePath.RoundCap
                            PathAngleArc {
                                centerX: 150; centerY: 150
                                radiusX: 130; radiusY: 130
                                startAngle: -90
                                sweepAngle: immobilityWidget.angle

                                Behavior on sweepAngle {
                                    NumberAnimation { duration: 600; easing.type: Easing.OutBack }
                                }
                            }
                        }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 5
                        Text {
                            text: Math.round(immobilityScore) + "%"
                            color: "black"
                            font.pixelSize: 48; font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: "IMMOBILITY"
                            color: "gray"
                            font.pixelSize: 14; font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
                
                Text {
                    text: immobilityScore < 50 ? "Healthy Venous Return" : "Leg Movement Suggested"
                    Layout.alignment: Qt.AlignHCenter
                    font.italic: true
                    color: "#666"
                }
                Item { Layout.fillHeight: true }
            }
        }
    }

    Component {
        id: hrmonitor
        Page {
            property int countdownSeconds: 30

            Popup {
                id: timerPopup
                anchors.centerIn: parent
                width: parent.width * 0.8; height: 400
                modal: true; focus: true
                visible: true // Opens automatically when page loads
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                
                background: Rectangle { color: "#27ae60"; radius: 15 }

                Timer {
                    id: calibrationTimer
                    interval: 1000
                    running: timerPopup.opened
                    repeat: true
                    onTriggered: {
                        if (countdownSeconds > 0) {
                            countdownSeconds--;
                        } else {
                            timerPopup.close();
                            calibrationTimer.stop();
                        }
                    }
                }

                contentItem: ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 30

                    Text {
                        text: "Calibrating..."
                        color: "white"
                        font.pixelSize: 32
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: countdownSeconds + "s"
                        color: "white"
                        font.pixelSize: 64
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: "Please stay completely still."
                        color: "white"
                        font.pixelSize: 18
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Button {
                        text: countdownSeconds > 0 ? "Wait for Calibration..." : "Continue"
                        enabled: countdownSeconds === 0
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: timerPopup.close()
                    }
                }

                onOpened: countdownSeconds = 30
            }

            header: ToolBar {
                implicitHeight: 80
                background: Rectangle {
                    color: "#c2c2c2"
                }
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    Button {
                        text: "Back"
                        onClicked: appStack.pop()
                    }
                    Label {
                        text: "Heartrate Monitor"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        opacity: isMoving ? 0.5 : 1.0
                    }
                    Label {
                        id: bpmLabel
                        text: "0 BPM"
                        font.bold: true
                        color: isMoving ? "gray" : "#ff4d4d"
                    }
                }
            }

            property int sampleCount: 0
            property int maxVisibleSamples: 100
            property bool isMoving: false
            property double restingScore: 100.0 
            property bool isConnected: false

            ColumnLayout {
                anchors.fill: parent

                ChartView {
                    id: chartView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    theme: ChartView.ChartThemeDark
                    antialiasing: true
                    legend.visible: false

                    ValueAxis { id: xAxis; min: 0; max: 100; tickCount: 5 }
                    ValueAxis { 
                        id: yAxis; 
                        min: -0.1; max: 1.2;
                        titleText: "Pulse Trigger" 
                    }

                    LineSeries {
                        id: pulseSeries
                        name: "Heartbeat"
                        color: isMoving ? "#444" : "#00FF41"
                        width: 3 
                        axisX: xAxis
                        axisY: yAxis
                    }
                }
                
                Rectangle {
                    Layout.fillWidth: true
                    height: 30
                    color: "#1a1a1a"
                    Rectangle {
                        width: parent.width * (restingScore / 100.0)
                        height: parent.height
                        color: restingScore > 70 ? "#27ae60" : "#f39c12"
                        Behavior on width { NumberAnimation { duration: 300 } }
                    }
                    Text {
                        anchors.centerIn: parent
                        text: "Stability: " + Math.round(restingScore) + "%"
                        color: "white"
                        font.pixelSize: 10
                    }
                }

                Rectangle {
                    id: statusIndicator
                    Layout.fillWidth: true
                    height: 40
                    color: isMoving ? "#e67e22" : (isConnected ? "#27ae60" : "#c0392b")
                    
                    Text {
                        anchors.centerIn: parent
                        text: isMoving ? "MOTION DETECTED - HOLD STILL" : 
                            (isConnected ? "SYSTEM STABLE" : "DISCONNECTED")
                        color: "white"
                        font.bold: true
                    }
                }
            }
            
            Connections {
                target: btManager
                function onMetricsUpdated(stretch, bpm, pulse, moving, resting, connected) {
                    isMoving = moving;
                    restingScore = resting;
                    isConnected = connected;

                    pulseSeries.append(sampleCount, pulse);
                    
                    bpmLabel.text = Math.round(bpm) + " BPM";

                    if (sampleCount > maxVisibleSamples) {
                        xAxis.min++;
                        xAxis.max++;
                        if (pulseSeries.count > maxVisibleSamples + 5) {
                            pulseSeries.remove(0);
                        }
                    }
                    sampleCount++;
                }
            }
        }
    }

    Rectangle {
        id: notificationPopup
        z: 100
        visible: false
        width: 600; height: 120
        color: "#000000"; border.color: "red"; border.width: 2; radius: 8
        anchors.horizontalCenter: parent.horizontalCenter
        y: -height
        property alias titleText: notifyTitle.text
        property alias bodyText: notifyBody.text
        Column {
            anchors.centerIn: parent
            Text { id: notifyTitle; color: "red"; font.bold: true }
            Text { id: notifyBody; color: "white"; font.pixelSize: 12 }
        }
        NumberAnimation on y { id: slideDown; to: 20; duration: 500; easing.type: Easing.OutBack }
        NumberAnimation on y { id: slideUp; to: -notificationPopup.height; duration: 500; easing.type: Easing.InBack }
    }

    Connections {
        target: typeof NotifManager !== "undefined" ? NotifManager : null 
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