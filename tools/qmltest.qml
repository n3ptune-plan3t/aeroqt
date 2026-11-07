// Smoke + behaviour test for the AeroQt QQC2 style (run by aeroqt-qmltest).
// failures() returns the list of problems; empty = pass.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: win
    visible: true
    width: 420; height: 720

    ColumnLayout {
        anchors.fill: parent
        Button { id: btn; text: "Button" }
        Button { text: "Checked"; checkable: true; checked: true }
        Button { text: "Disabled"; enabled: false }
        CheckBox { text: "CheckBox"; checked: true }
        RadioButton { text: "Radio"; checked: true }
        Switch { text: "Switch"; checked: true }
        ComboBox { id: combo; model: ["One", "Two", "Three"] }
        ComboBox { id: editCombo; editable: true; model: ["Alpha", "Beta"] }
        TextField { id: field; placeholderText: "hint" }
        TextArea { id: area; placeholderText: "area hint"; Layout.preferredHeight: 50 }
        SpinBox { id: spin; from: 0; to: 10; value: 3 }
        Slider { value: 0.4 }
        ProgressBar { value: 0.6 }
        BusyIndicator { running: true }
        TabBar { id: tabs; TabButton { text: "A" } TabButton { text: "B" } }
        ToolBar { Label { text: "toolbar" } }
        ScrollView { Layout.preferredHeight: 40; Layout.fillWidth: true; Label { text: "scroll\n\n\n\n\n" } }
    }

    function placeholderOf(c, t) {
        for (var i = 0; i < c.children.length; ++i)
            if (c.children[i].text === t) return c.children[i]
        return null
    }

    function failures() {
        var f = []
        combo.popup.open()
        spin.increase(); spin.increase()

        // ComboBox must show its current text (editText is empty when not editable)
        if (combo.contentItem.text !== "One") f.push("combo shows '" + combo.contentItem.text + "', expected 'One'")
        // ... and its dropdown must list the items in a non-empty popup (needs a delegate + height)
        if (combo.popup.contentItem.count !== 3) f.push("combo popup lists " + combo.popup.contentItem.count + " items, expected 3")
        if (!(combo.popup.height > 0)) f.push("combo popup has no height")
        if (editCombo.contentItem.text !== "Alpha") f.push("editable combo shows '" + editCombo.contentItem.text + "'")

        // SpinBox: steps and shows the value in an editable TextInput
        if (spin.value !== 5) f.push("spin value " + spin.value + ", expected 5")
        if (spin.contentItem.text !== "5") f.push("spin shows '" + spin.contentItem.text + "'")
        if (!spin.up.indicator || !spin.down.indicator) f.push("spin has no up/down indicators")

        // Placeholders only show if the style supplies a PlaceholderText item
        var p1 = placeholderOf(field, "hint"), p2 = placeholderOf(area, "area hint")
        if (!p1 || !p1.visible) f.push("TextField placeholder not shown")
        if (!p2 || !p2.visible) f.push("TextArea placeholder not shown")

        // TabBar needs a styled TabButton
        if (tabs.count !== 2 || !(tabs.itemAt(0).width > 0)) f.push("tab buttons not laid out")
        if (!tabs.itemAt(0).checked) f.push("first tab not current")
        return f
    }
}
