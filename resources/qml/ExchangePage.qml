import QtQuick 2.12
import QtQuick.Controls 2.12
import ModelViews 1.0

Page {
    property alias balanceModel : bv.model
    property alias volumeMarketModel : vmv.viewModel
    property alias spreadMarketModel : smv.viewModel
    property alias tradingPairsModel : tpv.viewModel
    property alias openOrdersModel : oov.viewModel
    property alias buyOrdersModel : marketOrdersView.buyModel
    property alias sellOrdersModel : marketOrdersView.sellModel
    property alias tradeHistoryModel : thv.model
    property alias tradeHistorySelectedPairsModel : thspv.viewModel
    property var api
    property var marketOrdersModel

    ExchangePageSection {
        id: gbSpreadMarketsView
        anchors.right: gbVolumeMarketsView.left
        anchors.rightMargin: 10
        title: qsTr("Spread markets")
        width: 475
        height: 230

        SpreadMarketsView {
            id: smv
            anchors.fill: parent
        }
    }

    ExchangePageSection {
        id: gbVolumeMarketsView
        anchors.right: parent.right
        anchors.rightMargin: 5
        title: qsTr("Volume markets")
        width: 475
        height: 230

        VolumeMarketsView {
            id: vmv
            anchors.fill: parent
        }
    }

    ExchangePageSection {
        id: gbTradingPairsView
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.top: gbVolumeMarketsView.bottom
        anchors.topMargin: 10
        title: qsTr("Trading pairs")
        width: 625
        height: 200

        TradingPairsView {
            id: tpv
            anchors.fill: parent
        }
    }

    OrdersView {
        id: marketOrdersView
        width: 510
        height: 230
        buyModel: buyOrdersModel
        sellModel: sellOrdersModel
        anchors.left: parent.left
        anchors.leftMargin: 5
    }

    ExchangePageSection {
        id: gbOpenOrdersView
        anchors.top: marketOrdersView.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 5
        title: qsTr("Open orders")
        width: 642
        height: 200

        OpenOrdersView {
            id: oov
            anchors.fill: parent
        }
    }

    ExchangePageSection {
        id: gbTradeHistoryView
        anchors.top: gbOpenOrdersView.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 5
        title: qsTr("Trade history")
        width: 606
        height: 270

        TradingHistoryView {
            id: thv
            anchors.fill: parent
        }
    }

    ExchangePageSection {
        id: gbTradeHistorySelectedPairView
        anchors.top: gbOpenOrdersView.bottom
        anchors.topMargin: 10
        anchors.left: gbTradeHistoryView.right
        anchors.leftMargin: 25
        title: qsTr("Trade history for pair")
        width: 606
        height: 270

        TradingHistorySelectedPairView {
            id: thspv
            anchors.fill: parent
        }
    }

    ExchangePageSection {
        id: gbBalancesView
        anchors.top: gbTradingPairsView.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 5
        title: qsTr("Balances")
        width: 300
        height: 270

        BalanceView {
            id: bv
            anchors.fill: parent
        }
    }
}
