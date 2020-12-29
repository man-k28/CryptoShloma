import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import CryptoShloma.Common 1.0
import BaseElements 1.0

Dialog {
    id: contentDialog
    padding: 10
    spacing: 10
    dim: true
    Material.accent: Material.color(Material.Green)
    clip: true
    modal: true
    standardButtons: Dialog.Save | Dialog.Close
    onAccepted: {
        SettingsManager.set(SettingsManager.TradeBaseSumm, SettingsManager.General, tfTradeSummBaseCurrency.text)
        SettingsManager.set(SettingsManager.MinSpread, SettingsManager.General, tfMinSpread.text)
        SettingsManager.set(SettingsManager.MinProfit, SettingsManager.General, tfMinProfit.value)
        SettingsManager.set(SettingsManager.MinBaseVolume, SettingsManager.General, tfMinBaseVolume.value)
        SettingsManager.set(SettingsManager.MinCoinPrice, SettingsManager.General, tfMinCoinPrice.text)
        SettingsManager.set(SettingsManager.TradeDifference, SettingsManager.General, tfTradeDifference.text)
        SettingsManager.set(SettingsManager.HanukkahTable, SettingsManager.Hanukkah, hanukkahModel.model.toJsonString())
        SettingsManager.set(SettingsManager.HanukkahMaxSteps, SettingsManager.Hanukkah, tfHanukkahMaxSteps.value)
        SettingsManager.set(SettingsManager.BinanceEnabled, SettingsManager.Binance, cbBinanceEnabled.checked)
        SettingsManager.set(SettingsManager.BinanceAPIKey, SettingsManager.Binance, tfBinanceApiKey.text)
        SettingsManager.set(SettingsManager.BinanceSecretKey, SettingsManager.Binance, tfBinanceSecretKey.text)
        ExchangePool.refreshModels()
        CryptoShloma.reloadSettings()
    }

    GroupBox {
        id: gbApi
        title: qsTr("API")
        spacing: 0
        padding: 10
        anchors.left: parent.left
        Column {
            spacing: 0
            padding: 0

            Row {
                spacing: 10
                padding: 0
                property bool binanceEnabled: (SettingsManager.get(SettingsManager.BinanceEnabled, SettingsManager.Binance) === 'true')
                CheckBox {
                    width: 120
                    id: cbBinanceEnabled
                    checked: parent.binanceEnabled
                    text: qsTr("Binance")
                }

                PasswordField {
                    id: tfBinanceApiKey
                    placeholderText: qsTr("API Key")
                    enabled: cbBinanceEnabled.checked
                    text: SettingsManager.get(SettingsManager.BinanceAPIKey,SettingsManager.Binance)
                    ToolTip.text: qsTr("Binance API Key")
                }

                PasswordField {
                    id: tfBinanceSecretKey
                    placeholderText: qsTr("Secret Key")
                    enabled: cbBinanceEnabled.checked
                    text: SettingsManager.get(SettingsManager.BinanceSecretKey,SettingsManager.Binance)
                    ToolTip.text: qsTr("Binance Key")
                }
            }
        }
    }

    GroupBox {
        id: gbHanukkah
        anchors.left: gbApi.right
        anchors.leftMargin: 10
        anchors.right: gbSpreder.left
        anchors.rightMargin: 10
        anchors.bottom: gbSpreder.bottom
        anchors.top: parent.top
        spacing: 0
        padding: 10
        title: qsTr("Hanukkah")

        Column {
            spacing: 0
            padding: 0
            anchors.fill: parent
            Grid {
                spacing: 10
                padding: 0
                columns: 2
                rows: 1
                verticalItemAlignment: Grid.AlignVCenter
                id: wrapperMaxSteps
                height: lbMaxSteps.height + tfHanukkahMaxSteps.height + parent.spacing
                width: lbMaxSteps.width + tfHanukkahMaxSteps.width + parent.spacing

                Label {
                    id: lbMaxSteps
                    text: qsTr("Max Steps")
                }

                SpinBox {
                    id: tfHanukkahMaxSteps
                    spacing: 0
                    editable: true
                    from: 0
                    to: 100
                    value: SettingsManager.get(SettingsManager.HanukkahMaxSteps, SettingsManager.Hanukkah)
                    ToolTip.timeout: 2000
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Max Steps")
                }
            }

            HanukkahTable {
                id: hanukkahModel
                width: parent.width
                height: parent.height - wrapperMaxSteps.height - parent.spacing
            }
        }
    }

    GroupBox {
        id: gbSpreder
        anchors.right: parent.right
        spacing: 0
        padding: 10
        title: qsTr("Main")
        Grid {
            id: grid
            spacing: 10
            padding: 0
            columns: 2
            rows: 8
            verticalItemAlignment: Grid.AlignVCenter

            Label {
                text: qsTr("Trade base summ")
            }

            TextField {
                id: tfTradeSummBaseCurrency
                focus: true
                selectByMouse: true
                placeholderText: qsTr("Trade base summ")
                text: SettingsManager.get(SettingsManager.TradeBaseSumm)
            }

            Label {
                text: qsTr("Min spread")
            }

            TextField {
                id: tfMinSpread
                focus: true
                selectByMouse: true
                placeholderText: qsTr("Min spread")
                text: SettingsManager.get(SettingsManager.MinSpread)
            }

            Label {
                text: qsTr("Min profit %")
            }

            SpinBox {
                id: tfMinProfit
                value: SettingsManager.get(SettingsManager.MinProfit)
                editable: true
                from: 0
                to: 10
                ToolTip.timeout: 2000
                ToolTip.visible: hovered
                ToolTip.text: qsTr("0 - for only fee included")
            }

            Label {
                text: qsTr("Min base volume")
            }

            SpinBox {
                id: tfMinBaseVolume
                value: SettingsManager.get(SettingsManager.MinBaseVolume)
                editable: true
                from: 1
                to: 10000
            }

            Label {
                text: qsTr("Min coin price")
            }

            TextField {
                id: tfMinCoinPrice
                focus: true
                selectByMouse: true
                placeholderText: qsTr("Min coin price")
                text: SettingsManager.get(SettingsManager.MinCoinPrice)
            }

            Label {
                text: qsTr("Trade difference")
            }

            TextField {
                id: tfTradeDifference
                focus: true
                selectByMouse: true
                placeholderText: qsTr("Trade difference")
                text: SettingsManager.get(SettingsManager.TradeDifference)
            }

            Label {
                text: qsTr("Debug mode")
            }

            Switch {
                //TODO: сделать включение режима отладки
                id: tmDebug
                focus: true
            }

            Label {
                text: qsTr("Emulation")
            }

            Switch {
                //BUG: сделать перезагрузку эмуляции
                id: tmbEmulation
                focus: true
                checked: SettingsManager.get(SettingsManager.EnableTestData) === 'true'
                onClicked: {
                    SettingsManager.set(SettingsManager.EnableTestData, SettingsManager.General, checked)
                    CryptoShloma.restartEmulation()
                    CryptoShloma.reloadSettings();
                }
            }
        }
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:800;width:1400}
}
 ##^##*/
