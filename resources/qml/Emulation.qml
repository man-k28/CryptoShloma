import QtQuick.Dialogs 1.3
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

Popup {
    property alias dialog: fileDialog
    width: 250
    height: 70
    Material.theme: Material.Dark
    modal: false

    FileDialog {
        id: fileDialog
        title: qsTr("Please choose a file")
        folder: shortcuts.home
        nameFilters: [qsTr("Comma-Separated Values (*.csv)")]
        onAccepted: {
            var exchange = pageRepeater.itemAt(mainWrapper.currentIndex)
            if (exchange.item !== undefined && exchange.status === Loader.Ready)
                exchange.item.api.emulator.loadMarketOrders(fileDialog.fileUrl)
        }
    }

    Column {
        anchors.fill: parent
        ToolButton {
            text: qsTr("Load market orders")
            onClicked: fileDialog.open()
        }
    }
}
