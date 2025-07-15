import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: "OpenRGB 设备列表"

    property alias deviceModel: deviceListView.model

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        // 标题栏
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#2c3e50"
            radius: 5

            Text {
                anchors.centerIn: parent
                text: "OpenRGB 设备管理器"
                color: "white"
                font.pixelSize: 20
                font.bold: true
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
                Text {
                    anchors.centerIn: parent
                    visible: deviceListView.count === 0
                    text: "未检测到任何设备\n请确保设备已正确连接"
                    color: "#7f8c8d"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
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
                    text: "设备数量: " + (deviceListView.count || 0)
                    color: "white"
                    font.pixelSize: 12
                }

                Item {
                    Layout.fillWidth: true
                }

                Text {
                    text: "OpenRGB 自定义界面"
                    color: "#bdc3c7"
                    font.pixelSize: 10
                }
            }
        }
    }
}
