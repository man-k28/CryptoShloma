import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import CryptoShloma.Gui 1.0
import CryptoShloma.Common 1.0
import CryptoShloma.Core 1.0
import CommonLibs 1.0
import BaseElements 1.0

ListViewStyled {
    id: tableView
    anchors.fill: parent
    cacheBuffer: 50
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
                text: qsTr("Fee")
                width: 70
            }

            HeaderLabel {
                text: qsTr("Created")
                width: 100
            }

            HeaderLabel {
                text: qsTr("Closed")
                width: 100
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
                    text: model.order.market.currency.symbol + "/" + model.order.market.baseCurrency.symbol
                    width: 70
                }
                EmptyLabel {
                    text: model.order.type === Order.Buy ? qsTr("Buy") : qsTr("Sell");
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
                    text: CommonLibs.toDouble(model.order.fee);
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
            }

            Separator {}
        }
    }
}
