import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import CryptoShloma.Gui 1.0
import CryptoShloma.Common 1.0
import CommonLibs 1.0
import BaseElements 1.0

Item {
    property alias viewModel: tableView.model
    anchors.fill: parent

    TradePairPopup {
        id: spreadMarketsPopup
        x: width
        y: 0
        button.onClicked: {
            close()
            if ( maxSteps > table.count )
                return;
            var metaConfig = JSON;
            metaConfig.hannukahTable = table.toJsonArray()
            metaConfig.maxSteps = maxSteps
            market.metaConfig = metaConfig
            market.percentageProfit = profit
            market.tradeSumm = tradeSumm
            model.isTradable = true
            api.controller.appendMarket(market.id, market.userConfig)
            tradeHistorySelectedPairsModel.setFilterMarketId(market.id)
            marketOrdersModel.setFilterMarketId(market.id)
            marketOrdersView.title = market.currency.symbol
        }
        onOpened: {
            refresh()
        }
    }

    Column {
        anchors.fill: parent
        spacing: 0
        padding: 0

        SearchField {
            id: search
            width: parent.width
            font.pixelSize: 12
            focus: true
            onTextChanged : {
                viewModel.setFilterFixedString(text)
            }
        }

        ListViewStyled {
            id: tableView
            width: parent.width
            height: parent.height - search.height
            header: ListViewHeaderStyled {
                width: tableView.width
                height: 20
                Column {
                    spacing: 0
                    padding: 0
                    height: parent.height

                    Row {
                        spacing: 1
                        padding: 0
                        HeaderLabel {
                            text: qsTr("Symbol")
                            width: 80
                            topPadding: 0
                        }

                        HeaderLabel {
                            text: qsTr("AskPrice")
                            width: 90
                            topPadding: 0
                        }

                        HeaderLabel {
                            text: qsTr("BidPrice")
                            width: 90
                            topPadding: 0
                        }

                        HeaderLabel {
                            text: qsTr("BaseVolume")
                            width: 120
                            topPadding: 0
                        }

                        HeaderLabel {
                            text: qsTr("Spread")
                            width: 70
                            topPadding: 0
                        }

                        HeaderLabel {
                            width: 14
                            topPadding: 0
                        }
                    }
                }
            }

            delegate: ListViewDelegateStyled {
                property int row: index
                property var view: ListView.view

                Column {
                    Row {
                        id: rowWrapper
                        property string label: model.market.label
                        spacing: 1
                        EmptyLabel {
                            text: rowWrapper.label
                            width: 80
                        }
                        EmptyLabel {
                            text: CommonLibs.toDouble(model.market.askPrice)
                            width: 90
                        }
                        EmptyLabel {
                            text: CommonLibs.toDouble(model.market.bidPrice)
                            width: 90
                        }
                        EmptyLabel {
                            text: CommonLibs.toDouble(model.market.baseVolume)
                            width: 120
                        }
                        EmptyLabel {
                            text: CommonLibs.toDouble(model.market.spread)
                            width: 70
                        }

                        TableToolButton {
                            icon.source: "qrc:/ui/icons/resources/ui_icons/add.svg"
                            ToolTip.text: qsTr("Start %1").arg(rowWrapper.label)
                            onClicked: {
                                spreadMarketsPopup.market = model.market
                                spreadMarketsPopup.model = model
                                spreadMarketsPopup.button.text = qsTr("Start %1").arg(rowWrapper.label)
                                spreadMarketsPopup.open()
                            }
                        }
                    }

                    Separator {}
                }
            }
        }
    }
}
