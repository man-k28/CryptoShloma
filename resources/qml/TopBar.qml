import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import CryptoShloma.Gui 1.0
import CryptoShloma.Common 1.0
import BaseElements 1.0

ToolBar {
    Material.background: "#303030"
    property alias currentIndex: bar.currentIndex
    TabBar {
        width: parent.width - rightMenu.width
        id: bar
        Repeater {
            model: ExchangePool

            delegate: TabBarIcon {
                id: icon
                icon.color: model.exchange.api.isOnline ? Material.color(Material.Green) : Material.color(Material.Red)
                states: [
                    State {
                        when: model.exchange.api.type == ExchangeType.Binance
                        PropertyChanges {
                            target: icon
                            icon.source: "qrc:/ui/exchange_logos/resources/exchange_logos/binance.svg"
                            ToolTip.text: qsTr("Binance")
                        }
                    }
                ]
            }
        }
    }

    Row {
        id: rightMenu
        anchors.right: parent.right

        ToolBarIcon {
            property bool isEnabled: SettingsManager.get(SettingsManager.EnableTestData) === 'true'
            icon.source: "qrc:/ui/icons/resources/ui_icons/speedometer.svg"
            icon.color: Material.color(Material.Green)
            ToolTip.text: qsTr("Open emulator")
            onClicked: {
                emulatorWindow.dialog.open()
            }
            visible: isEnabled
        }

        ToolBarIcon {
            icon.source: "qrc:/ui/icons/resources/ui_icons/big-database.svg"
            icon.color: Crud.isOpen ? Material.color(Material.Green) : Material.color(Material.Red)
            ToolTip.text: qsTr("Database")
        }

        ToolBarIcon {
            icon.source: "qrc:/ui/icons/resources/ui_icons/settings.svg"
            icon.color: Material.color(Material.Green)
            ToolTip.text: qsTr("Settings")
            onClicked: settingsWindow.open()
        }
    }
}
