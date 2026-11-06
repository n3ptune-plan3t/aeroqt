import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.Switch {
    id: control
    implicitWidth: Math.max(background ? background.implicitWidth : 0, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0, contentItem.implicitHeight + topPadding + bottomPadding)
    leftPadding: indicator.width + spacing
    spacing: 6
    indicator: Rectangle {
        implicitWidth: 40; implicitHeight: 20
        radius: 10
        x: control.leftPadding - width - control.spacing
        y: (control.height - height) / 2
        color: control.checked ? control.palette.highlight : control.palette.button
        border.color: control.palette.dark
        Rectangle {
            width: 16; height: 16; radius: 8
            x: control.checked ? parent.width - width - 2 : 2
            y: 2
            color: control.palette.button
            border.color: control.palette.dark
        }
    }
    contentItem: Text {
        text: control.text
        font: control.font
        color: control.palette.text
        verticalAlignment: Text.AlignVCenter
    }
}
