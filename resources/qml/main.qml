import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
//import Qt.labs.platform 1.1

ApplicationWindow {
    id: applicationWindow
    visible: true
    minimumHeight: 830
    minimumWidth: 1550
    title: qsTr("CryptoShloma v.%1").arg(CryptoShloma.softwareVersion)
    Material.theme: Material.Dark

    header: TopBar{ id: bar }

    SwipeView {
        id: mainWrapper
        spacing: 0
        padding: 0
        interactive: false
        currentIndex: bar.currentIndex
        anchors.fill: parent

        Repeater {
            id: pageRepeater
            model: ExchangePool
            Loader {
                active: SwipeView.isCurrentItem
                sourceComponent: ExchangePage {
                    spreadMarketModel: model.exchange.spreadMarketsProxyModel
                    volumeMarketModel: model.exchange.volumeMarketsProxyModel
                    tradingPairsModel: model.exchange.tradingPairsProxyModel
                    buyOrdersModel: model.exchange.buyOrdersProxyModel
                    sellOrdersModel: model.exchange.sellOrdersProxyModel
                    openOrdersModel: model.exchange.openOrdersProxyModel
                    tradeHistoryModel: model.exchange.tradeHistoryProxyModel
                    tradeHistorySelectedPairsModel: model.exchange.tradeHistorySelectedPairProxyModel
                    balanceModel: model.exchange.balancesProxyModel
                    api: model.exchange.api
                    marketOrdersModel: model.exchange.marketOrdersProxyModel
                }
            }
        }
    }

    SettingsItem {
        id: settingsWindow
        width: parent.width
        height: parent.height
    }

    Emulation {
        id: emulatorWindow
    }

//    SystemTrayIcon {
//        visible: true
//        iconSource: "qrc:/ui/icons/resources/ui_icons/rabbi.svg"

////        onActivated: {
////            parent.show()
////            parent.raise()
////            parent.requestActivate()
////        }

//        menu: Menu {
////                MenuItem {
////                    text: qsTr("Торговать")
////                    checkable: true
////                    iconSource: "qrc:/ui/icons/resources/ui_icons/play-button.svg"
////                }
//                MenuItem {
//                    text: qsTr("Settings")
//                    iconSource: "qrc:/ui/icons/resources/ui_icons/settings.svg"
//                    onTriggered: settingsWindow.open()
//                }
//                MenuSeparator{}
//                MenuItem {
//                    text: qsTr("Quit")
//                    iconSource: "qrc:/ui/icons/resources/ui_icons/power.svg"
//                    onTriggered: Qt.quit()
//                }
//        }
////        onMessageClicked: console.log("Message clicked")
//        Component.onCompleted: showMessage("Cryptoshloma", "Let's make cryptomoney, man! :)")
//    }
}


