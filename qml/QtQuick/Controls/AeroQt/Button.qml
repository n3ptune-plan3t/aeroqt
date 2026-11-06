import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.Button {
    id: control
    implicitWidth: Math.max(background ? background.implicitWidth : 0, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0, contentItem.implicitHeight + topPadding + bottomPadding)
    leftPadding: 12
    rightPadding: 12
    topPadding: 6
    bottomPadding: 6

    background: Rectangle {
        implicitWidth: 80
        implicitHeight: 30
        radius: 6
        color: control.palette.button
        opacity: control.enabled ? 1 : 0.6
        border.color: control.palette.dark
        border.width: 1
        Rectangle { // gloss: white top half
            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
            height: parent.height * 0.48
            radius: 6
            color: Qt.rgba(1, 1, 1, control.pressed ? 0.10 : 0.28)
            visible: control.enabled && !control.pressed
        }
        Rectangle { // bottom glow
            anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
            height: parent.height * 0.35
            radius: 6
            color: Qt.alpha(control.palette.highlight, control.hovered ? 0.35 : 0.18)
        }
    }
    contentItem: Text {
        text: control.text
        font: control.font
        color: control.palette.buttonText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
