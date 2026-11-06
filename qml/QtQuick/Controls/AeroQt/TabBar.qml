import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.TabBar {
    id: control
    implicitHeight: 32
    background: Rectangle {
        color: control.palette.window
        border.color: control.palette.dark
    }
}
