import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.BusyIndicator {
    id: control
    implicitWidth: 32
    implicitHeight: 32
    contentItem: Item {
        Repeater {
            model: 8
            delegate: Rectangle {
                width: 4; height: 4; radius: 2
                color: Qt.alpha(control.palette.highlight, 0.3 + 0.7 * (1 - (index / 8)))
                Component.onCompleted: {
                    const a = index / 8 * Math.PI * 2;
                    x = parent.width / 2 + Math.cos(a) * 12 - width / 2;
                    y = parent.height / 2 + Math.sin(a) * 12 - height / 2;
                }
            }
        }
        RotationAnimator on rotation {
            from: 0; to: 360; duration: 1200; loops: Animation.Infinite
        }
    }
}
