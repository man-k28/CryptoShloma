import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import BaseElements 1.0
import CryptoShloma.Common 1.0

Item {
    property alias model: hanukkahModel
    ListModel {
        id: hanukkahModel

        Component.onCompleted: {
            updateModel()
        }

        function updateModel() {
            var array = JSON.parse(SettingsManager.get(SettingsManager.HanukkahTable, SettingsManager.Hanukkah));
            hanukkahModel.clear()
            for(var i in array)
                hanukkahModel.append(array[i])
        }

        function toJsonArray() {
            var data = [];
            for ( var i = 0; i < count; ++i )
                data.push(get(i))
            return data
        }

        function toJsonString() {
            return JSON.stringify(toJsonArray())
        }
    }

    Column {
        anchors.fill: parent

        Row {
            padding: 0
            spacing: 10
            id: buttonGroup
            TableToolButton {
                id: btnAdd
                width: 50
                height: 30
                checked: true
                text: qsTr("Add")
                ToolTip.visible: false
                onClicked: {
                    hanukkahModel.append({"step":0, "growth": 0})
                }
            }

            TableToolButton {
                id: btnRemove
                width: 70
                height: 30
                text: qsTr("Remove")
                ToolTip.visible: false
                onClicked: {
                    hanukkahModel.remove(tableView.currentIndex)
                }
            }
        }

        ListViewStyled {
            spacing: 0
            id: tableView
            width: parent.width
            height: parent.height - buttonGroup.height
            model: hanukkahModel
            header: ListViewHeaderStyled {
                width: tableView.width
                Row {
                    spacing: 10
                    padding: 0
                    HeaderLabel {
                        text: qsTr("№")
                        width: 20
                    }

                    HeaderLabel {
                        text: qsTr("Step %")
                        width: 50
                    }
                    HeaderLabel {
                        text: qsTr("Growth %")
                        width: 50
                    }
                }
            }
            delegate: Row {
                id: delegateRow
                property int row: index
                spacing: 10
                padding: 0
                Label {
                    text: index + 1
                    width: 20
                    opacity: 0.6
                    leftPadding: 5
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            tableView.currentIndex = delegateRow.row
                        }
                    }
                }

                TextInput {
                  text: model.step
                  width: 50
                  color: "#fff"
                  horizontalAlignment: Text.AlignHCenter
                  onEditingFinished: model.step = Number(text)
                }

                TextInput {
                    text: model.growth
                    width: 50
                    color: "#fff"
                    horizontalAlignment: Text.AlignHCenter
                    onEditingFinished: model.growth = Number(text)
                }
            }
            highlight: Rectangle { color: Material.color(Material.LightBlue); opacity: 0.3 }
            highlightFollowsCurrentItem: true
        }
    }
}


