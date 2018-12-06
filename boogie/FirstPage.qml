import QtQuick 2.6
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1

Page {
    id: root
    property string server_ip_address

    header: ToolBar {
        Label {

            text: qsTr("Boogie")
            font.pixelSize: 20
            anchors.centerIn: parent
        }
    }
    ColumnLayout {
        anchors.fill: parent
        Pane {
            id: pane
            Layout.fillWidth: true

            Column {
                width: parent.width

                TextArea {
                    id: ipField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Server ip address")
                    wrapMode: TextArea.Wrap
                }

                Button {
                    id: sendButton
                    text: qsTr("Confirm")
                    enabled: ipField.length > 0
                    onClicked: root.StackView.view.push("qrc:/ContactPage.qml")
                }
            }
        }
    }
}
