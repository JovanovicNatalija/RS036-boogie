import QtQuick 2.6
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1
import QtQuick.Controls 2.4
import QtQuick.Window 2.2
import QtQuick 2.11
import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.1

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

					Label{
						id: ipLabel
                        text: qsTr("Adresa servera:")
						font.pixelSize: 14
					}
					TextField {
						id: ipField
						placeholderText: qsTr("127.0.0.1")
						selectByMouse: true
					}

					Label{
						id: usernameLabel
                        text: qsTr("Korisnicko ime:")
						font.pixelSize: 14
					}
					TextField {
						id: usernameField
						anchors.topMargin: 20
                        placeholderText: qsTr("Korisnicko ime")
						selectByMouse: true
					}

					Label{
						id: passwordLabel
                        text: qsTr("Sifra:")
						font.pixelSize: 14
					}
					TextField {
						id: passwordField
						echoMode: TextInput.Password
						placeholderText: qsTr("*********")
						selectByMouse: true
					}
				}
                Keys.onPressed: {
                    if(event.key === Qt.Key_Return ){
                        root.StackView.view.push(
                                    "qrc:/qml/ContactPage.qml",
                                    { username: usernameField.text })
                        Client.connectToServer(usernameField.text, ipField.text)
                        Client.sendAuthData(passwordField.text)
                        Client.readFromXml()
                    }
                }

				Button {
					id: sendButton
                    text: qsTr("Potvrdi")
					enabled: ipField.length > 0 &&
                             usernameField.length > 0 &&
                             passwordField.length > 0
					anchors.horizontalCenter: parent.horizontalCenter
					onClicked: {
						root.StackView.view.push(
									"qrc:/qml/ContactPage.qml",
									{ username: usernameField.text })
                        Client.connectToServer(usernameField.text, ipField.text)
                        Client.sendAuthData(passwordField.text)
                        Client.readFromXml()

                    }

                }

            }
        }
    }
}
