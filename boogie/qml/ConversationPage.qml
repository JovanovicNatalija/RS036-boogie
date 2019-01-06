import QtQuick 2.10
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1
import QtQuick.Controls 2.4
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.3

Page {
    id: root

    property string inConversationWith
    property int index : 0

    header: ToolBar {
        ToolButton {
            property bool shown: false
            text: qsTr("Prikazi prethodne poruke")
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            enabled: !shown
            onClicked: {
                shown = true
                messageModel.clear()
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
        nameFilters: ["Images files (*.jpg, *png)"]
        onAccepted: {
            Client.sendPicture(fileDialog.fileUrl, inConversationWith)
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
				width: lblMsg.width + 2*lblMsg.anchors.margins
				height: lblMsg.height + 2*lblMsg.anchors.margins
                radius: 10
                color: index ? "#4F7942" : "#808080"
                anchors.right: {
                    if(index) parent.right
                }

                Label {
                    id: lblMsg
                    text: model.message
					//used to get text width in pixels
					TextMetrics {
						id: textMetrics
						font: lblMsg.font
						text: lblMsg.text
					}
					width: textMetrics.boundingRect.width < 500
							? textMetrics.boundingRect.width : 500
                    color: "black"
					wrapMode: width == 500 ? Text.WordWrap : Text.NoWrap
                    anchors.centerIn: parent
                    anchors.margins: 10
                }

            }

            onCountChanged: {
                var newIndex = count - 1
                positionViewAtEnd()
                currentIndex = newIndex
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
					wrapMode: Text.WordWrap
                    Keys.onPressed: {
                        if(event.key === Qt.Key_Return ){
                            Client.sendMsgData(inConversationWith, messageField.text.trim())
							Client.addMsgToBuffer(Client.getUsername(), inConversationWith, messageField.text.trim())
							messageModel.append({message: messageField.text.trim(), index : 1});
                            messageField.clear()
							//so that textArea wouldnt read return key too
							event.accepted = true
                        }
                    }
                }


                Button {
                    id: confirmButton
                    text: qsTr("Posalji")
                    enabled: messageField.length > 0
                    onClicked: {
                        Client.sendMsgData(inConversationWith, messageField.text)
						Client.addMsgToBuffer(Client.getUsername(), inConversationWith, messageField.text)
						messageModel.append({message: messageField.text, index : 1});
                        messageField.clear()
                    }
                }
               Button {
                   id: picButton
                   text: qsTr("Izaberi sliku")
                   onClicked: {
                       fileDialog.open()
                   }
               }
            }
        }
    }
}
