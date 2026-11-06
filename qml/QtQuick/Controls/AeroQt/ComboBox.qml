import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.ComboBox {
    id: control
    implicitWidth: 160
    implicitHeight: 30
    leftPadding: 8; rightPadding: 28; topPadding: 4; bottomPadding: 4
    background: Rectangle {
        radius: 6
        color: control.palette.button
        border.color: control.palette.dark
        border.width: 1
        Rectangle {
            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
            height: parent.height * 0.48; radius: 6
            color: Qt.rgba(1, 1, 1, 0.28)
        }
    }
    contentItem: Text {
        text: control.editText
        font: control.font
        color: control.palette.buttonText
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    indicator: Text {
        x: control.width - width - 8
        y: (control.height - height) / 2
        text: "▾"
        color: control.palette.buttonText
    }
    popup: Popup {
        y: control.height + 2
        width: control.width
        contentItem: ListView {
            model: control.delegateModel
            clip: true
            ScrollBar.vertical: ScrollBar { }
        }
        background: Rectangle {
            radius: 4
            color: control.palette.base
            border.color: control.palette.dark
        }
    }
}
