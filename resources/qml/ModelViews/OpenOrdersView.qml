import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import CryptoShloma.Gui 1.0
import CryptoShloma.Common 1.0
import CryptoShloma.Core 1.0
import CommonLibs 1.0
import BaseElements 1.0

Item {
    property alias viewModel: tableView.model

    EmptyLabel {
        id: helpIndicator
        text: qsTr("No open orders")
        visible: false
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        color: Material.color(Material.Grey)
    }

    ListViewStyled {
        onCountChanged: {
            helpIndicator.visible = (count > 0 ? false : true)
        }
        id: tableView
        anchors.fill: parent
        busyEnabled: false
        header: ListViewHeaderStyled {
            width: tableView.width
            Row {
                spacing: 1
                HeaderLabel {
                    text: qsTr("Symbol")
                    width: 70
                }

                HeaderLabel {
                    text: qsTr("Type")
                    width: 50
                }

                HeaderLabel {
                    text: qsTr("Rate")
                    width: 70
                }

                HeaderLabel {
                    text: qsTr("Amount")
                    width: 70
                }

                HeaderLabel {
                    text: qsTr("Total")
                    width: 70
                }

                HeaderLabel {
                    text: qsTr("Remaining")
                    width: 70
                }

                HeaderLabel {
                    text: qsTr("Timestamp")
                    width: 100
                }

                HeaderLabel {
                    text: qsTr("*Updated")
                    width: 100
                }
            }
        }

        delegate: ListViewDelegateStyled {
            property int row: index
            property var view: ListView.view
            Column {
                MouseArea {
                    height: childrenRect.height
                    width: childrenRect.width
                    acceptedButtons: Qt.RightButton
                    Row {
                        spacing: 1
                        EmptyLabel {
                            text: model.order.market.currency.symbol + "/" + model.order.market.baseCurrency.symbol
                            width: 70
                        }
                        EmptyLabel {
                            text: model.order.type === Order.Buy ? qsTr("Buy") : qsTr("Sell")
                            width: 50
                        }
                        EmptyLabel {
                            text: CommonLibs.toDouble(model.order.price);
                            width: 70
                        }
                        EmptyLabel {
                            text: CommonLibs.toDouble(model.order.currencyVolume);
                            width: 70
                        }
                        EmptyLabel {
                            text: CommonLibs.toDouble(model.order.baseVolume);
                            width: 70
                        }
                        EmptyLabel {
                            text: CommonLibs.toDouble(model.order.remaining);
                            width: 70
                        }
                        EmptyLabel {
                            text: Qt.formatDateTime(model.order.timestamp, "yyyy-MM-dd hh:mm:ss");
                            width: 100
                        }
                        EmptyLabel {
                            text: Qt.formatDateTime(model.order.updatedTimestamp, "yyyy-MM-dd hh:mm:ss");
                            width: 100
                        }
                        ToolButton {
                            padding: 0
                            spacing: 0
                            width: 14
                            height: 14
                            icon.color: Material.color(Material.Green)
                            icon.source: "qrc:/ui/icons/resources/ui_icons/garbage.svg"
                            antialiasing: true
                            ToolTip.timeout: 2000
                            ToolTip.visible: hovered
                            hoverEnabled: true
                            ToolTip.text: qsTr("Cancel %1").arg(model.order.market.currency.symbol)
                            onClicked: {
                                api.controller.cancelOrder(model.order.market.id, model.order.orderId)
                            }
                        }
                        ToolButton {
                            property bool isEnabled: SettingsManager.get(SettingsManager.EnableTestData) === 'true' && !emulatorWindow.visible
                            padding: 0
                            spacing: 0
                            width: 14
                            height: 14
                            icon.color: Material.color(Material.Green)
                            icon.source: "qrc:/ui/icons/resources/ui_icons/bag.svg"
                            antialiasing: true
                            ToolTip.timeout: 2000
                            ToolTip.visible: hovered
                            hoverEnabled: true
                            visible: isEnabled
                            ToolTip.text: qsTr("Sell %1").arg(model.order.market.currency.symbol)
                            onClicked: {
                                api.emulator.sellOrder(model.order.market.id,
                                                       model.order.orderId,
                                                       model.order.price,
                                                       model.order.baseVolume,
                                                       model.order.currencyVolume)
                            }
                        }
                    }
                    cursorShape: Qt.PointingHandCursor
                    Popup {
                        id: popup
                        width: 120
                        height: 60
                        modal: false
                        focus: false
                        visible: false
                        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                        x: parent.mouseX - width
                        y: parent.mouseY

                        Column {
                            spacing: 5
                            padding: 0
                            EmptyLabel {text: qsTr("Cancel order?")}
                            Row {
                                ToolButton {
                                    height: 20
                                    text: qsTr("Yes")
                                    onClicked: {
                                        popup.close()
                                        api.controller.cancelOrder(model.order.market.marketId, model.order.orderId)
                                    }
                                }
                                ToolButton {
                                    height: 20
                                    text: qsTr("No")
                                    onClicked: {
                                        popup.close()
                                    }
                                }
                            }
                        }
                    }
                    onClicked: {
                        if ( mouse.button === Qt.RightButton )
                            popup.open()
                    }
                }
                Separator {}
            }
        }
    }
}
