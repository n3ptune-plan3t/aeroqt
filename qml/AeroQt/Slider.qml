import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.Slider {
    id: control
    implicitWidth: 160
    implicitHeight: 24
    background: Rectangle {
        x: control.leftPadding
        y: (control.height - height) / 2
        width: control.availableWidth
        height: 6
        radius: 3
        color: control.palette.button
        opacity: control.enabled ? 1 : 0.6
        border.color: control.visualFocus ? control.palette.highlight
                        : control.palette.dark
        Rectangle {
            width: control.availableWidth * control.visualPosition
            height: parent.height
            radius: 3
            color: control.palette.highlight
        }
    }
    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * control.availableWidth - width / 2
        y: (control.height - height) / 2
        implicitWidth: 18; implicitHeight: 18
        radius: 9
        color: control.pressed ? Qt.darker(control.palette.button, 1.06)
                : control.hovered ? Qt.lighter(control.palette.button, 1.08)
                : control.palette.button
        opacity: control.enabled ? 1 : 0.6
        border.color: control.visualFocus ? control.palette.highlight
                        : control.palette.dark
        border.width: control.visualFocus ? 2 : 1
        Rectangle {
            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
            height: parent.height * 0.48; radius: 9
            color: Qt.rgba(1, 1, 1, control.pressed ? 0.10 : 0.30)
            visible: control.enabled && !control.pressed
        }
    }
}
