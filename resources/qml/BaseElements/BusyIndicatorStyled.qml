import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

BusyIndicator {
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter
    id: indicator

    contentItem: Item {
        id: item
        x: parent.width / 2 - 32
        y: parent.height / 2 - 32
        implicitWidth: 64
        implicitHeight: 64
        width: 64
        height: 64
        opacity: indicator.running ? 1 : 0

        Behavior on opacity {
            OpacityAnimator {
                duration: 250
            }
        }

        RotationAnimator {
            target: item
            running: indicator.visible && indicator.running
            from: 0
            to: 360
            loops: Animation.Infinite
            duration: 1250
        }

        Repeater {
            id: repeater
            model: 6

            Rectangle {
                x: item.width / 2 - width / 2
                y: item.height / 2 - height / 2
                implicitWidth: 10
                implicitHeight: 10
                radius: 5
                color: Material.color(Material.Green)
                transform: [
                    Translate {
                        y: -Math.min(item.width, item.height) * 0.5 + 5
                    },
                    Rotation {
                        angle: index / repeater.count * 360
                        origin.x: 5
                        origin.y: 5
                    }
                ]
            }
        }
    }
}
