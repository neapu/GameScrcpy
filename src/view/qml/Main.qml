import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    color: "#ffffff"

    MessageDialog {
        id: errorDialog
        title: "打开设备失败"
        text: ""
    }


    Rectangle {
        id: buttonsContainer
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        height: 40

        color: "transparent"

        Button {
            id: addRemoteDeviceButton
            text: "Add Remote Device"
            icon.source: "qrc:/icons/add.svg"
            display: AbstractButton.TextBesideIcon
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.verticalCenter: parent.verticalCenter

            background: Rectangle {
                color: addRemoteDeviceButton.down ? "#d0d0d0" : (addRemoteDeviceButton.hovered ? "#eeeeee" : "transparent")
                border.color: addRemoteDeviceButton.down || addRemoteDeviceButton.hovered ? "#bdbdbd" : "transparent"
                border.width: 1
                radius: 4
            }

            onClicked: {
                console.log("Add Remote Device button clicked")
            }
        }

        Button {
            id: updateDeviceListButton
            text: "Update Device List"
            icon.source: "qrc:/icons/refresh.svg"
            display: AbstractButton.TextBesideIcon
            anchors.left: addRemoteDeviceButton.right
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter

            background: Rectangle {
                color: updateDeviceListButton.down ? "#d0d0d0" : (updateDeviceListButton.hovered ? "#eeeeee" : "transparent")
                border.color: updateDeviceListButton.down || updateDeviceListButton.hovered ? "#bdbdbd" : "transparent"
                border.width: 1
                radius: 4
            }

            onClicked: {
                qmlAdapter.updateDeviceList();
            }
        }

        Button {
            id: settingsButton
            text: "Settings"
            icon.source: "qrc:/icons/settings.svg"
            display: AbstractButton.TextBesideIcon
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.verticalCenter: parent.verticalCenter

            background: Rectangle {
                color: settingsButton.down ? "#d0d0d0" : (settingsButton.hovered ? "#eeeeee" : "transparent")
                border.color: settingsButton.down || settingsButton.hovered ? "#bdbdbd" : "transparent"
                border.width: 1
                radius: 4
            }

            onClicked: {
                console.log("Settings button clicked")
            }
        }
    }

    Rectangle {
        anchors.top: buttonsContainer.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        height: 1
        color: "#666666"
    }

    Rectangle {
        id: deviceListHeader
        anchors.top: buttonsContainer.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.right: parent.right
        anchors.rightMargin: 20
        height: 30

        color: "transparent"

        Text {
            text: "Name"
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
        }

        Text {
            text: "Serial"
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 150
        }

        Text {
            text: "Android Version"
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 300
        }

        Text {
            text: "Status"
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 10
        }
    }

    Rectangle {
        id: deviceListContainer
        anchors.top: deviceListHeader.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10

        color: "transparent"

        ScrollView {
            anchors.fill: parent

            ListView {
                id: deviceListView
                width: parent.width
                height: parent.height
                spacing: 5
                model: deviceModel

                delegate: Rectangle {
                    width: deviceListView.width
                    height: 40
                    color: mouseArea.containsMouse ? "#ababab" : "transparent"
                    radius: 6

                    Text {
                        text: name
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                    }

                    Text {
                        text: serial
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 150
                    }

                    Text {
                        text: androidVersion
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 300
                    }

                    Text {
                        text: status
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true

                        onDoubleClicked: {
                            const err = qmlAdapter.openDevice(serial)
                            if (err && err.length > 0) {
                                errorDialog.text = err
                                errorDialog.open()
                            }
                        }
                    }
                }
            }
        }
    }
}
