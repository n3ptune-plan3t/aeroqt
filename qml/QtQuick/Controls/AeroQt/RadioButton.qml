import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.RadioButton {
    id: control
    implicitWidth: Math.max(background ? background.implicitWidth : 0, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0, contentItem.implicitHeight + topPadding + bottomPadding)
    leftPadding: indicator.width + spacing
    rightPadding: 6
    spacing: 6
    indicator: Rectangle {
        implicitWidth: 18; implicitHeight: 18
        radius: 9
        x: control.leftPadding - width - control.spacing
        y: (control.height - height) / 2
        color: control.palette.base
        border.color: control.palette.dark
        border.width: 1
        Rectangle {
            anchors.centerIn: parent
            width: 9; height: 9; radius: 4.5
            color: control.palette.highlight
            visible: control.checked
        }
    }
    contentItem: Text {
        text: control.text
        font: control.font
        color: control.palette.text
        verticalAlignment: Text.AlignVCenter
    }
}
