import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.ToolBar {
    id: control
    implicitHeight: 36
    background: Rectangle {
        color: control.palette.window
        border.color: control.palette.dark
        Rectangle {
            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
            height: parent.height * 0.48
            color: Qt.rgba(1, 1, 1, 0.25)
        }
    }
}
