import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import CryptoShloma.Gui 1.0
import CryptoShloma.Common 1.0
import CommonLibs 1.0
import BaseElements 1.0

Item {
    property alias viewModel: tableView.model

    EmptyLabel {
        id: helpIndicator
        text: qsTr("No tading coins")
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
        highlight: Rectangle { color: Material.color(Material.LightBlue); opacity: 0.3 }
        highlightFollowsCurrentItem: true
        header: ListViewHeaderStyled {
            width: tableView.width
            Row {
                spacing: 1
                HeaderLabel {
                    text: qsTr("Symbol")
                    width: 70
                }

                HeaderLabel {
                    text: qsTr("Volume")
                    width: 110
                }

                HeaderLabel {
                    text: qsTr("Spread")
                    width: 70
                }

                HeaderLabel {
                    text: qsTr("Trend")
                    width: 70
                }

                HeaderLabel {
                    text: qsTr("MinTrend")
                    width: 70
                }

                HeaderLabel {
                    text: qsTr("Status")
                    width: 70
                }

//                HeaderLabel {
//                    text: qsTr("PauseMode")
//                    width: 70
//                }

//                HeaderLabel {
//                    text: qsTr("IsVolumeTrading")
//                    width: 80
//                }

                HeaderLabel {
                    text: qsTr("LastRateUpdateTime")
                    width: 110
                }

                HeaderLabel {
                    width: 14
                }

                HeaderLabel {
                    width: 14
                }

                HeaderLabel {
                    width: 14
                }
            }
        }

        TradePairPopup {
            id: tradingPairsPopup
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
            }
        }

        delegate: ListViewDelegateStyled {
            property int row: index
            property var view: ListView.view
            Column {
                MouseArea {
                    id: area
                    height: childrenRect.height
                    width: childrenRect.width
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    Row {
                        spacing: 1
                        EmptyLabel {
                            text: model.market.label
                            width: 70
                        }
                        EmptyLabel {
                            text: CommonLibs.toDouble(model.market.baseVolume);
                            width: 110
                        }
                        EmptyLabel {
                            text: CommonLibs.toDouble(model.market.spread);
                            width: 70
                        }

                        EmptyLabel {
                            property real trendRate: Number(model.market.currentTrendRate).toFixed(2)
                            text: qsTr("%1 %").arg(trendRate);
                            width: 70
                            color: trendRate < 0 ? Material.color(Material.Red) : Material.color(Material.Green)
                        }

                        EmptyLabel {
                            text: qsTr("%1 %").arg(Number(model.market.minTrend).toFixed(2));
                            width: 70
                        }

                        EmptyLabel {
                            id: lbStatus
                            width: 70

                            states: [
                                State {
                                    when: model.market.status == 1 //TODO: register to metaType
                                    PropertyChanges {
                                        target: lbStatus
                                        text: qsTr("Ok")
                                    }
                                },
                                State {
                                    when: model.market.status == 2 //TODO: register to metaType
                                    PropertyChanges {
                                        target: lbStatus
                                        text: qsTr("Closed")
                                    }
                                },
                                State {
                                    when: model.market.status == 3 //TODO: register to metaType
                                    PropertyChanges {
                                        target: lbStatus
                                        text: qsTr("Paused")
                                    }
                                }
                            ]

                        }

//                        EmptyLabel {
//                            text: model.market.pauseMode
//                            width: 70
//                        }

//                        EmptyLabel {
//                            text: model.market.isVolumeTrading
//                            width: 80
//                        }

                        EmptyLabel {
                            text: Qt.formatDateTime(model.market.lastRateUpdateTime, "yyyy-MM-dd hh:mm:ss");
                            width: 110
                        }

                        TableToolButton {
                            id: pauseButton

                            onClicked: {
                                model.market.pauseMode = !model.market.pauseMode
                                api.controller.updateMarketConfig(model.market.id, model.market.userConfig)
                            }
                            states: [
                                State {
                                    when: model.market.pauseMode === false
                                    PropertyChanges {
                                        target: pauseButton
                                        icon.source: "qrc:/ui/icons/resources/ui_icons/pause.svg"
                                        icon.color: Material.color(Material.Red)
                                        ToolTip.text: qsTr("Pause %1").arg(model.market.label)
                                    }
                                },
                                State {
                                    when: model.market.pauseMode === true
                                    PropertyChanges {
                                        target: pauseButton
                                        icon.source: "qrc:/ui/icons/resources/ui_icons/play-button.svg"
                                        icon.color: Material.color(Material.Green)
                                        ToolTip.text: qsTr("Continue %1").arg(model.market.label)
                                    }
                                }
                            ]
                        }

                        TableToolButton {
                            icon.source: "qrc:/ui/icons/resources/ui_icons/settings-1.svg"
                            enabled: false //TODO: dynamic settings
                            hoverEnabled: false
                            icon.color: Material.color(Material.Grey)
                            ToolTip.text: qsTr("Settings %1").arg(model.market.label)
                            onClicked: {
                                tradingPairsPopup.model = model
                                tradingPairsPopup.market = model.market
                                tradingPairsPopup.profit = model.market.percentageProfit
                                tradingPairsPopup.maxSteps = model.market.metaConfig.maxSteps
                                tradingPairsPopup.button.text = qsTr("Save %1").arg(model.market.label)
                                tradingPairsPopup.open()
                            }
                        }

                        TableToolButton {
                            property var pairId: model.market.id
                            icon.source: "qrc:/ui/icons/resources/ui_icons/cancel-1.svg"
                            ToolTip.text: qsTr("Stop %1").arg(model.market.label)
                            onClicked: {
                                model.market.isVolumeTrading = false
                                model.isTradable = false
                                api.controller.removeMarket(pairId)
                                tradeHistorySelectedPairsModel.setFilterMarketId()
                                marketOrdersModel.setFilterMarketId()
                                marketOrdersView.title = ""
                            }
                        }
                    }

                    cursorShape: Qt.PointingHandCursor

                    onClicked: {
                        if ( mouse.button === Qt.LeftButton) {
                            view.currentIndex = row
                            tradeHistorySelectedPairsModel.setFilterMarketId(model.market.id)
                            marketOrdersModel.setFilterMarketId(model.market.id)
                            marketOrdersView.title = model.market.label
                        } /*else if ( mouse.button === Qt.RightButton ) { //FIXME: вернуть, когда будет динамическое изменение настроек пары
                            tradingPairsPopup.open()
                        }*/
                    }
                }

                Separator {}
            }
        }
    }
}
