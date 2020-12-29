import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import CryptoShloma.Gui 1.0
import CryptoShloma.Common 1.0
import CommonLibs 1.0
import BaseElements 1.0

ListViewStyled {
    header: ListViewHeaderStyled {
        property var view: ListView.view
        width: view.width
        Row {
            spacing: 1
            HeaderLabel {
                text: qsTr("Price")
                width: 70
            }

            HeaderLabel {
                text: qsTr("Volume")
                width: 110
            }

            HeaderLabel {
                text: qsTr("Total")
                width: 80
            }
        }
    }

    delegate: ListViewDelegateStyled {
        property var view: ListView.view
        property int row: index
        Column {
            Row {
                spacing: 1
                EmptyLabel {
                    text: CommonLibs.toDouble(model.order.price);
                    width: 70
                }
                EmptyLabel {
                    text: CommonLibs.toDouble(model.order.currencyVolume);
                    width: 100
                }
                EmptyLabel {
                    text: CommonLibs.toDouble(model.order.baseVolume);
                    width: 80
                }
            }

            Separator {}
        }
    }
}
