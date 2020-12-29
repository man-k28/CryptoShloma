import QtQuick 2.12
import QtQuick.Controls 2.12
import BaseElements 1.0

GroupBox {
    spacing: 0
    padding: 0
    label: EmptyLabel {
        x: parent.leftPadding
        width: parent.availableWidth
        text: parent.title
        horizontalAlignment: Text.AlignLeft
        elide: Text.ElideLeft
    }
    background: Rectangle {
        color: "transparent"
        border.width: 0
    }
}
