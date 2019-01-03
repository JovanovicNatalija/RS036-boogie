import QtQuick 2.6
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Page {
    id: root

    header: ToolBar {
        Label {
            text: qsTr("Kontakti")
            font.pixelSize: 20
            anchors.centerIn: parent
        }
    }

    ListModel{
        id: contactModel
    }

    Connections {
        target: Client
        onBadPass: {
			root.StackView.view.pop();
        }

        onClearContacts: {
            contactModel.clear()
        }

        onShowContacts: {
            contactModel.append({con:contact, index: online})
        }

    }

    ListView{
        width: parent.width
        id: listView
        anchors.fill: parent
        topMargin: 48
        leftMargin: 48
        bottomMargin: 48
        rightMargin: 48
        spacing: 20
        model: contactModel
        header: RowLayout {
            id: glMainLayout
            width: parent.width
            Label{
               id: addContact
               text: qsTr("Novi kontakt:")
               font.pixelSize: 15
            }
            TextField {
                id: newContactField
                placeholderText: qsTr("Korisnicko ime")
                selectByMouse: true

            }
            Button {
                id: addContactButton
                text: qsTr("Dodaj novi kontakt")
                enabled: newContactField.length > 0
                onClicked: {
                    Client.checkNewContact(newContactField.text)
                    newContactField.clear()
                }
            }
        }

        delegate: ItemDelegate {
            Rectangle {
                 width: 10
                 height: width
                 color: { index ? "green" : "red" }
                 border.color: "black"
                 border.width: 1
                 radius: width*0.5
                 anchors.verticalCenter: parent.verticalCenter
            }
            text: model.con
            width: listView.width - listView.leftMargin - listView.rightMargin
            onClicked: {
                root.StackView.view.push("qrc:/qml/ConversationPage.qml", { inConversationWith: model.con })
            }
        }
    }
}
