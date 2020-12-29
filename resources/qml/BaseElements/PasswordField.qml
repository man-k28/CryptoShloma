import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

TextField {
    focus: true
    echoMode: TextInput.Password
    selectByMouse: true
    ToolTip.timeout: 3000
    ToolTip.visible: hovered
}
