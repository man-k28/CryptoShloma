import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import CryptoShloma.Gui 1.0
import CryptoShloma.Common 1.0
import CommonLibs 1.0
import BaseElements 1.0

ListViewStyled {
    id: tableView
    anchors.fill: parent
    header: ListViewHeaderStyled {
        width: tableView.width
        Row {
            spacing: 1

            HeaderLabel {
                text: qsTr("Symbol")
                width: 70
            }

            HeaderLabel {
                text: qsTr("Total")
                width: 70
            }

            HeaderLabel {
                text: qsTr("Available")
                width: 70
            }

            HeaderLabel {
                text: qsTr("HeldForTrades")
                width: 80
            }
        }
    }

    delegate: ListViewDelegateStyled {
        property int row: index
        property var view: ListView.view
        Column {
            Row {
                spacing: 1
                EmptyLabel {
                    text: model.balance.currency.symbol;
                    width: 70
                }
                EmptyLabel {
                    text: CommonLibs.toDouble(model.balance.total);
                    width: 70
                }
                EmptyLabel {
                    text: CommonLibs.toDouble(model.balance.available);
                    width: 70
                }
                EmptyLabel {
                    text: CommonLibs.toDouble(model.balance.heldForTrades);
                    width: 80
                }

//                ToolButton {
//                    padding: 0
//                    spacing: 0
//                    width: 14
//                    height: 14
//                    icon.color: Material.color(Material.Green)
//                    icon.source: "qrc:/ui/icons/resources/ui_icons/garbage.svg"
//                    antialiasing: true
//                    ToolTip.timeout: 2000
//                    ToolTip.visible: hovered
//                    hoverEnabled: true
//                    ToolTip.text: qsTr("Sell %1").arg(model.balance.currency.symbol)
//                    onClicked: {
//                        api.dropCoin(model.balance.currency.symbol)
//                    }
//                    Component.onCompleted: {
//                        if ( model.balance.currency.symbol === "BTC")
//                            visible = false
//                    }
//                }
            }

            Separator {}
        }
    }
}
