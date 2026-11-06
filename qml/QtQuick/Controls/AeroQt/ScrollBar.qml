import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.ScrollBar {
    id: control
    implicitWidth: orientation === Qt.Horizontal ? 12 : 12
    implicitHeight: orientation === Qt.Horizontal ? 12 : 12
    background: Rectangle {
        color: control.palette.button
        opacity: 0.6
    }
    contentItem: Rectangle {
        implicitWidth: control.orientation === Qt.Horizontal ? 40 : 12
        implicitHeight: control.orientation === Qt.Horizontal ? 12 : 40
        radius: 4
        color: control.palette.button
        border.color: control.palette.dark
        Rectangle {
            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
            height: control.orientation === Qt.Horizontal ? parent.height * 0.48 : 0
            radius: 4
            color: Qt.rgba(1, 1, 1, 0.28)
        }
    }
}
