import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

ToolButton {
    padding: 0
    spacing: 0
    width: 14
    height: 14
    icon.color: Material.color(Material.Green)
    antialiasing: true
    hoverEnabled: true
    ToolTip.timeout: 2000
    ToolTip.visible: hovered
}
