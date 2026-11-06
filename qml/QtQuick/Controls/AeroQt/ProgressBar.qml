import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.ProgressBar {
    id: control
    implicitWidth: 160
    implicitHeight: 16
    background: Rectangle {
        radius: 5
        color: control.palette.button
        border.color: control.palette.dark
    }
    contentItem: Item {
        Rectangle {
            width: parent.width * control.visualPosition
            height: parent.height
            radius: 5
            color: "#4cc94c"
            Rectangle {
                anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
                height: parent.height * 0.48; radius: 5
                color: Qt.rgba(1, 1, 1, 0.35)
            }
        }
    }
}
