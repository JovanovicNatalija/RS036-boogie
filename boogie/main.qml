import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.4

ApplicationWindow {
    id: window
    width: 540
    height: 960
    visible: true

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: FirstPage {}
    }
}

