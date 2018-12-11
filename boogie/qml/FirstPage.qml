import QtQuick 2.6
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1
import QtQuick.Controls 2.4
import QtQuick.Window 2.2

Page {
    id: root
    property string server_ip_address

    signal connectToServer(string ip)

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

                Row {
                    Label{
                        id: ipLabel
                        text: qsTr("Server ip address:")
                        font.pixelSize: 14
                    }

                    TextField {
                        id: ipField
                        placeholderText: qsTr("127.0.0.1")
                        selectByMouse: true
                    }
                }

                Row {
                    Label{
                        id: usernameLabel
                        text: qsTr("username:")
                        font.pixelSize: 14

                    }

                    TextField {
                        id: usernameField
                        anchors.topMargin: 20
                        placeholderText: qsTr("username")
                        selectByMouse: true

                    }
                }
                Row {
                    Label{
                        id: passwordLabel
                        text: qsTr("password:")
                        font.pixelSize: 14
                    }
                    TextField {
                        id: passwordField
                        echoMode: TextInput.Password
                        placeholderText: qsTr("*********")
                        selectByMouse: true
                    }
                }

                Button {
                    id: sendButton
                    text: qsTr("Confirm")
                    enabled: ipField.length > 0 && usernameField.length > 0 && passwordField.length > 0
                    onClicked: {
                        root.StackView.view.push("qrc:/qml/ContactPage.qml", { username: usernameField.text })

                        client.connectToServer(ipField.text)
                        client.sendAuthData(usernameField.text, passwordField.text)
                    }
                }
            }
        }
    }
}
