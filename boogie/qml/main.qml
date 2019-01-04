import QtQuick 2.10
import QtQuick.Window 2.2
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

ApplicationWindow {
    id: window
    width: 540
    height: 960
    visible: true

    Material.theme: Material.Dark
    Material.accent: Material.Lime
    Material.primary: Material.LightGreen
    Material.foreground: Material.LightGreen

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: FirstPage {}
    }
}


