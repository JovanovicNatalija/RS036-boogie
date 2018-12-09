import QtQuick 2.6
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1


Page {
    id: root
    property string server_ip_address
    signal connectToServer(string ip);
    onConnectToServer:{
        console.log("poslata ip adresa " + ip )
    }

    header: ToolBar {
        Label {
            text: qsTr("Boogie")
            font.pixelSize: 20
            anchors.centerIn: parent
        }
    }

    ColumnLayout {
        anchors.centerIn: parent

        Pane {
            id: pane
            Layout.fillWidth: true

            Column {
                width: parent.width

                TextArea {
                    id: ipField
                    placeholderText: qsTr("Server ip address")
                    selectByMouse: true
                    selectByKeyboard: true
                }

                Button {
                    id: sendButton
                    text: qsTr("Confirm")
                    enabled: ipField.length > 0
                    onClicked: {
                        root.StackView.view.push("qrc:/ContactPage.qml");
                        root.connectToServer(ipField.text);
                    }
                }
            }
        }
    }
}
