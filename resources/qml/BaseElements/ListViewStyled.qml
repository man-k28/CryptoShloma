import QtQuick 2.12
import QtQuick.Controls 2.12

ListView {
    property bool busyEnabled: true
    BusyIndicatorStyled {
        id: indicator
    }

    onCountChanged: {
        indicator.running = (count > 0 ? false : true) && busyEnabled
    }

    clip: true
    contentWidth: headerItem.width
    flickableDirection: Flickable.VerticalFlick
    ScrollIndicator.vertical: ScrollIndicator { }
//    focus: true
    headerPositioning: ListView.OverlayHeader
    boundsBehavior: Flickable.StopAtBounds
}
