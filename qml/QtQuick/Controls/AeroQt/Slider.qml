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
        border.color: control.palette.dark
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
        color: control.palette.button
        border.color: control.palette.dark
        Rectangle {
            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
            height: parent.height * 0.48; radius: 9
            color: Qt.rgba(1, 1, 1, 0.30)
        }
    }
}
