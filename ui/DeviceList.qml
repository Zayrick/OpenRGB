import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: "OpenRGB 设备列表"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        // 标题栏
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#2c3e50"
            radius: 5

            RowLayout {
                anchors.fill: parent
                anchors.margins: 15

                Text {
                    Layout.fillWidth: true
                    text: "OpenRGB 设备管理器"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                }

                // 加载指示器
                BusyIndicator {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    visible: deviceModel.loading
                    running: deviceModel.loading

                    ToolTip.visible: loadingMouseArea.containsMouse
                    ToolTip.text: "正在检测设备..."

                    MouseArea {
                        id: loadingMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                    }
                }
            }
        }

        // 设备列表
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ListView {
                id: deviceListView
                anchors.fill: parent
                spacing: 5
                model: deviceModel

                delegate: Rectangle {
                    width: deviceListView.width
                    height: 80
                    color: "#ecf0f1"
                    radius: 5
                    border.color: "#bdc3c7"
                    border.width: 1

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: parent.color = "#d5dbdb"
                        onExited: parent.color = "#ecf0f1"
                        onClicked: {
                            console.log("选中设备:", model.name)
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10

                        // 设备图标
                        Rectangle {
                            Layout.preferredWidth: 50
                            Layout.preferredHeight: 50
                            color: "#3498db"
                            radius: 25

                            Text {
                                anchors.centerIn: parent
                                text: model.name ? model.name.charAt(0).toUpperCase() : "?"
                                color: "white"
                                font.pixelSize: 20
                                font.bold: true
                            }
                        }

                        // 设备信息
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                text: model.name || "未知设备"
                                font.pixelSize: 16
                                font.bold: true
                                color: "#2c3e50"
                            }

                            Text {
                                text: "类型: " + (model.type || "未知")
                                font.pixelSize: 12
                                color: "#7f8c8d"
                            }

                            Text {
                                text: "位置: " + (model.location || "未知")
                                font.pixelSize: 12
                                color: "#7f8c8d"
                            }
                        }

                        // 状态指示器
                        Rectangle {
                            Layout.preferredWidth: 20
                            Layout.preferredHeight: 20
                            radius: 10
                            color: model.connected ? "#27ae60" : "#e74c3c"

                            ToolTip.visible: statusMouseArea.containsMouse
                            ToolTip.text: model.connected ? "已连接" : "未连接"

                            MouseArea {
                                id: statusMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                            }
                        }
                    }
                }

                // 空状态显示
                Column {
                    anchors.centerIn: parent
                    visible: deviceListView.count === 0 && !deviceModel.loading
                    spacing: 10

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "未检测到任何设备"
                        color: "#7f8c8d"
                        font.pixelSize: 18
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "请确保设备已正确连接并重新启动应用"
                        color: "#95a5a6"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                // 加载状态显示
                Column {
                    anchors.centerIn: parent
                    visible: deviceListView.count === 0 && deviceModel.loading
                    spacing: 15

                    BusyIndicator {
                        anchors.horizontalCenter: parent.horizontalCenter
                        running: deviceModel.loading
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: deviceModel.progressText || "正在检测设备，请稍候..."
                        color: "#3498db"
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                    }

                    // 进度条
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 300
                        height: 20
                        color: "#ecf0f1"
                        radius: 10
                        border.color: "#bdc3c7"
                        border.width: 1

                        Rectangle {
                            width: parent.width * (deviceModel.progress / 100.0)
                            height: parent.height
                            color: "#3498db"
                            radius: 10

                            Behavior on width {
                                NumberAnimation {
                                    duration: 200
                                    easing.type: Easing.OutCubic
                                }
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: deviceModel.progress + "%"
                            color: "#2c3e50"
                            font.pixelSize: 12
                            font.bold: true
                        }
                    }
                }
            }
        }

        // 底部状态栏
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "#34495e"
            radius: 5

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10

                Text {
                    text: deviceModel.loading ?
                        (deviceModel.progressText || "检测中...") + " (" + deviceModel.progress + "%)" :
                        "设备数量: " + (deviceListView.count || 0)
                    color: deviceModel.loading ? "#3498db" : "white"
                    font.pixelSize: 12
                }

                Item {
                    Layout.fillWidth: true
                }

                Text {
                    text: "OpenRGB 自定义界面 - 事件驱动版"
                    color: "#bdc3c7"
                    font.pixelSize: 10
                }
            }
        }
    }
}
