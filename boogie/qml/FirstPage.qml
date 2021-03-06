import QtQuick 2.11
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1
import QtQuick.Controls 2.4
import QtQuick.Window 2.2

Page {
    id: root

    property string serverIpAddress

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

				GridLayout{
                    id: glMainLayout
                    columns: 2

                    Connections {
                        target: Client
                        onBadPass: {
                            usernameField.clear()
                            passwordField.clear()
                            root.StackView.view.pop();
							msgLabel.text = "Loša šifra!"
                        }

                        onAlreadyLogIn: {
                            usernameField.clear()
                            passwordField.clear()
                            root.StackView.view.pop();
							msgLabel.text = "Već ste ulogovani na drugom računaru!"
                        }
                    }

					Label{
						id: ipLabel
                        text: qsTr("Adresa servera:")
						font.pixelSize: 14
					}
					TextField {
						id: ipField
						text: "localhost"
						readOnly: true
						selectByMouse: true
					}

					Label{
						id:portLabel
						text: qsTr("Port servera: ")
						font.pixelSize: 14
					}
					TextField {
						id: portField
						text: qsTr("10000")
						selectByMouse: true
					}

					Label{
						id: usernameLabel
						text: qsTr("Korisničko ime:")
						font.pixelSize: 14
					}
					TextField {
						id: usernameField
						anchors.topMargin: 20
						placeholderText: qsTr("Korisničko ime")
						selectByMouse: true
					}

					Label{
						id: passwordLabel
						text: qsTr("Šifra:")
						font.pixelSize: 14
					}
					TextField {
						id: passwordField
						echoMode: TextInput.Password
						placeholderText: qsTr("*********")
						selectByMouse: true
					}

				}

                Label{
                    id: msgLabel
                    font.pixelSize: 14
                    color: "red"
                }

                Keys.onReturnPressed: {

					if( ipField.length > 0 &&
						usernameField.length > 0 &&
						portField.length > 0 &&
						parseInt(portField.text) > 1024 &&
						passwordField.length > 0) {

						root.StackView.view.push(
							"qrc:/qml/ContactPage.qml",
							{ username: usernameField.text })
						Client.connectToServer(ipField.text,parseInt(portField.text))
						Client.sendAuthData(usernameField.text, passwordField.text)
						Client.readFromXml()
                    }

                }

				Button {
					id: sendButton
                    text: qsTr("Potvrdi")
					enabled: ipField.length > 0 &&
                             usernameField.length > 0 &&
							 portField.length > 0 &&
							 parseInt(portField.text) > 1024 &&
                             passwordField.length > 0
					anchors.horizontalCenter: parent.horizontalCenter
					onClicked: {
                        stackView.push(
									"qrc:/qml/ContactPage.qml",
									{ username: usernameField.text })
                        if(msgLabel.text === "") {
							Client.connectToServer(ipField.text, parseInt(portField.text))
                        }
                        Client.sendAuthData(usernameField.text, passwordField.text)
                        Client.readFromXml()
                    }
                }
            }
        }

    }


}
