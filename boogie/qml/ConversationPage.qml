import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.1
import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1
import QtQuick.Controls 2.4
import QtQuick.Window 2.2
import QtQuick 2.11
import QtQuick 2.2
import QtQuick.Dialogs 1.3

Page {
    id: root

    property string inConversationWith
    property int index : 0

    header: ToolBar {
        ToolButton {
            text: qsTr("Prikazi prethodne poruke")
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            onClicked: {
                Client.displayOnConvPage(inConversationWith)
            }
        }

        ToolButton {
            text: qsTr("Nazad")
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            onClicked: {
                root.StackView.view.pop()
            }
        }

        Label {
            id: pageTitle
            text: inConversationWith
            font.pixelSize: 20
            anchors.centerIn: parent
        }
    }

    FileDialog {
        id: fileDialog
        visible: false
        title: "Izaberi sliku"
        folder: shortcuts.home
        onAccepted: {
            Client.sendPicture(fileDialog.fileUrls)
        }
    }

    ListModel {
        id: messageModel
    }

    Connections {
        target: Client
        onShowMsg: {
            if(inConversationWith === msgFrom) {
                messageModel.append({message: msg, index : 0})
            } else if(Client.getUsername() === msgFrom) {
                messageModel.append({message: msg, index : 1})
            }
        }
    }



    ColumnLayout {
        anchors.fill: parent


        ListView {

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: pane.leftPadding

            spacing: 12

            model: messageModel

            delegate: Rectangle {
                width: lblMsg.width
                height: 40
                color: index ? "rosybrown" : "violet"
                //postavljamo poruke koje smo mi poslali desno, a one koje
                //smo primili ostavljamo levo
                anchors.right: {
                    if(index) parent.right
                }

                Label {
                    id: lblMsg
                    text: model.message
                    anchors.centerIn: parent
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }

        Pane {
            id: pane
            Layout.fillWidth: true

            RowLayout {
                width: parent.width

                TextArea  {
                    id: messageField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Poruka")
                    wrapMode: TextArea.Wrap
                    Keys.onPressed: {
                        if(event.key === Qt.Key_Return ){
                            Client.sendMsgData(inConversationWith, messageField.text)
                            Client.addMsgToBuffer(Client.getUsername(), inConversationWith, Client.splitMessage(messageField.text))
                            messageModel.append({message: Client.splitMessage(messageField.text), index : 1});
                            messageField.clear()
                        }
                    }
                }


                Button {
                    id: confirmButton
                    text: qsTr("Posalji")
                    enabled: messageField.length > 0
                    onClicked: {
                        Client.sendMsgData(inConversationWith, messageField.text)
                        Client.addMsgToBuffer(Client.getUsername(), inConversationWith, Client.splitMessage(messageField.text))
                        messageModel.append({message: Client.splitMessage(messageField.text), index : 1});
                        messageField.clear()
                    }
                }
               Button {
                   id: picButton
                   text: qsTr("Picture")
                   onClicked: {
                       fileDialog.open()
                   }
               }
            }
        }
    }
}
