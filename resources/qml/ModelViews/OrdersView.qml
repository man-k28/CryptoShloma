import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import BaseElements 1.0

Item {
    property alias buyModel: buyView.model
    property alias sellModel: sellView.model
    property alias title: title.text
    EmptyLabel {
        id: helpIndicator
        text: qsTr("Please, select trading coin")
        visible: false
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        color: Material.color(Material.Grey)
    }

    id: itemView
    EmptyLabel {
        id: title
        text: ""
        anchors.horizontalCenter: parent.horizontalCenter
    }
    Row {
        spacing: 15

        GroupBox {
            spacing: 0
            padding: 0
            title: qsTr("Sell orders")
            label: EmptyLabel {
                x: parent.leftPadding
                width: parent.availableWidth
                text: parent.title
                elide: Text.ElideRight
            }
            background: Rectangle {
                color: "transparent"
                border.width: 0
            }
            Column {
                OrdersListView {
                    onCountChanged: {
                        helpIndicator.visible = (count > 0 ? false : true)
                    }
                    busyEnabled: false
                    id: sellView
                    height: 205
                    width: 255
                }
            }
        }

        GroupBox {
            spacing: 0
            padding: 0
            title: qsTr("Buy orders")
            label: EmptyLabel {
                x: parent.rightPadding
                width: parent.availableWidth
                text: parent.title
                horizontalAlignment: Text.AlignRight
                elide: Text.ElideRight
            }
            background: Rectangle {
                color: "transparent"
                border.width: 0
            }
            Column {
                OrdersListView {
                    onCountChanged: {
                        helpIndicator.visible = (count > 0 ? false : true)
                    }
                    busyEnabled: false
                    id: buyView
                    height: 205
                    width: 255
                }
            }
        }
    }
}
