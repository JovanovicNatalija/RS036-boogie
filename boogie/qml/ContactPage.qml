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

        onClearContacts: {
            contactModel.clear()
            contactModelForGroups.clear()
        }

        onShowContacts: {
            contactModel.append({con:contact, index: online, grId: groupId})
            contactModelForGroups.append({con:contact, index: online, grId : groupId})
        }
        onShowGroups: {
            contactModel.append({con:contact, index:online, grId : groupId})
        }

    }

    ListView{
        width: parent.width
        layoutDirection: Qt.RightToLeft
        cacheBuffer: 316
        id: listView
        anchors.fill: parent
        topMargin: 48
        leftMargin: 48
        bottomMargin: 48
        rightMargin: 48
        spacing: 20
        model: contactModel
        header: ColumnLayout {
            id: glMainLayout
            width: parent.width
            RowLayout{
                Label{
                   id: addContact
                   text: qsTr("Novi kontakt:")
                   font.pixelSize: 15
                }
                TextField {
                    id: newContactField
					placeholderText: qsTr("Korisničko ime")
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

                Label{
                    id: msgLabel
                    font.pixelSize: 14
                    color: "red"
                }

                Connections {
                    target: Client
                    onBadContact: {
                        msgLabel.text = msg
                    }
                }

                Keys.onReturnPressed: {
                    Client.checkNewContact(newContactField.text)
                    newContactField.clear()
                }
            }
            RowLayout{
                Button {
                    id: createGroupButton
                    text: qsTr("Napravi novu grupu")
                    onClicked: groupPopup.open()

                }

            }
        }

        delegate: ItemDelegate {
            Connections {
                target: Client
                onUnreadMsg: {
                    if(username === model.con) {
                        if(numOfUnreadMessages === "") {
                            numOfUnreadMessages.text = 1;
                        } else {
                            numOfUnreadMessages.text = (Number(numOfUnreadMessages.text) + 1).toString()
                        }
                    }
                }

                onShowMsg: {
                    if(stackView.depth === 2) {
                        if(msgFrom === model.con) {
                            if(numOfUnreadMessages === "") {
                                numOfUnreadMessages.text = 1;
                            } else {
                                numOfUnreadMessages.text = (Number(numOfUnreadMessages.text) + 1).toString()
                            }
                        }
                    }
                }

                onShowPicture: {
                    if(stackView.depth === 2) {
                        if(msgFrom === model.con) {
                            if(numOfUnreadMessages === "") {
                                numOfUnreadMessages.text = 1;
                            } else {
                                numOfUnreadMessages.text = (Number(numOfUnreadMessages.text) + 1).toString()
                            }
                        }
                    }
                }
            }

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
            Rectangle {
                visible: numOfUnreadMessages.text != ""
                width: numOfUnreadMessages.width + 4
                height: 15
                color: "#BADBAD"
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                radius: 30
                Label {
                    id: numOfUnreadMessages
                    color: "black"
                    anchors.margins: 2
                    anchors.centerIn: parent
                }
            }

            onClicked: {
                numOfUnreadMessages.text = ""
                root.StackView.view.push("qrc:/qml/ConversationPage.qml", { inConversationWith: model.con, grId: grId})
            }
        }
    }
    Popup{
        id: groupPopup
        x: 100
        y: 100
        width: 400
        height: 500
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        onClosed: {
            Client.clearGroupSet()

        }
        ListModel{
            id: contactModelForGroups
        }
        ListView{
            width: parent.width
            layoutDirection: Qt.RightToLeft
            cacheBuffer: 316
            id: listViewGroup
            anchors.fill: parent
            topMargin: 48
            leftMargin: 48
            bottomMargin: 48
            rightMargin: 48
            spacing: 20
            model: contactModelForGroups
            header: RowLayout {
                TextField {
                    id: groupNameField
                    placeholderText: qsTr("Naziv grupe")
                    selectByMouse: true
                    onVisibleChanged: groupNameField.clear()
            }
                Button {
                    id: confirmGroupButton
                    text: qsTr("Napravi grupu")
                    enabled: groupNameField.length > 0
                    onClicked: {
                        Client.sendGroupInfos(groupNameField.text)
                        groupNameField.clear()
                        groupPopup.close()
                    }
                }
                Keys.onReturnPressed: {
                    Client.sendGroupInfos(groupNameField.text)
                    groupNameField.clear()
                    groupPopup.close()
                }
            }
            delegate: ItemDelegate {
                CheckBox {
                    id: a
                    checked: false
                    text: model.con
                    width: listView.width - listView.leftMargin - listView.rightMargin
                    onClicked: {
                    }
                    onCheckedChanged: {
                        if(a.checked){
                            Client.addContactToGroupSet(a.text)
                        }
                        else
                            Client.removeContactFromGroupSet(a.text)
                    }
                }
            }
        }
    }
}


