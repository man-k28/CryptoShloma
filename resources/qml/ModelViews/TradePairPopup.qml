import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import CryptoShloma.Common 1.0
import BaseElements 1.0
import "../"

Popup {
    property alias button: start
    property alias profit: sbMinProfit.value
    property alias tradeSumm: tfTradeSumm.text
    property alias maxSteps: sbHanukkahMaxSteps.value
    property alias table: hanukkahTable.model
    property var market
    property var model

    function refresh() {
        table.updateModel()
        sbMinProfit.value = SettingsManager.get(SettingsManager.MinProfit)
        sbHanukkahMaxSteps.value = SettingsManager.get(SettingsManager.HanukkahMaxSteps, SettingsManager.Hanukkah)
        tfTradeSumm.text = SettingsManager.get(SettingsManager.TradeBaseSumm)
    }

    id: popup
    width: 170
    height: 340
    modal: false
    focus: false
    visible: false
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    Column {
        spacing: 5
        padding: 0
        Grid {
            id: grid
            spacing: 2
            columns: 1
            rows: 4
            verticalItemAlignment: Grid.AlignVCenter
            horizontalItemAlignment: Grid.AlignHCenter
            Row {
                height: lbProfit.height + sbMinProfit.height + parent.spacing
                width: lbProfit.width + sbMinProfit.width + parent.spacing

                EmptyLabel {
                    id: lbProfit
                    width: 30
                    text: qsTr("Profit")
                }

                SpinBox {
                    id: sbMinProfit
                    width: 50
                    height: 20
                    spacing: 0
                    font.pixelSize: 10
                    editable: true
                    from: 0
                    to: 100
                    value: SettingsManager.get(SettingsManager.MinProfit)
                    ToolTip.timeout: 2000
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("0 - for only fee included")
                }
            }

            Row {
                height: lbMaxSteps.height + sbHanukkahMaxSteps.height + parent.spacing
                width: lbMaxSteps.width + sbHanukkahMaxSteps.width + parent.spacing

                EmptyLabel {
                    id: lbMaxSteps
                    width: 50
                    text: qsTr("Max Steps")
                }

                SpinBox {
                    id: sbHanukkahMaxSteps
                    width: 50
                    height: 20
                    spacing: 0
                    font.pixelSize: 10
                    editable: true
                    from: 0
                    to: 100
                    value: SettingsManager.get(SettingsManager.HanukkahMaxSteps, SettingsManager.Hanukkah)
                    ToolTip.timeout: 2000
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Max Steps")
                }
            }

            Row {
                height: lbTradeSumm.height + tfTradeSumm.height + parent.spacing
                width: lbTradeSumm.width + tfTradeSumm.width + parent.spacing

                EmptyLabel {
                    id: lbTradeSumm
                    width: 70
                    height: 40
                    text: qsTr("Trade Summ")
                }

                TextField {
                    id: tfTradeSumm
                    width: 50
                    height: 40
                    font.pixelSize: 10
                    focus: true
                    selectByMouse: true
                    placeholderText: qsTr("Trade base summ")
                    text: SettingsManager.get(SettingsManager.TradeBaseSumm)
                }
            }

            HanukkahTable {
                id: hanukkahTable
                width: 145
                height: 130
            }
        }
        ToolButton {
            id: start
            height: 20
        }
    }
}
