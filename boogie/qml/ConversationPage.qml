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
            var msg = fileDialog.fileUrl.toString()
            console.log(msg)
            Client.sendPicture(msg, inConversationWith)
            messageModel.append({image: true, message: msg, index: 1})
        }
    }

    ListModel {
        id: messageModel
    }

    Connections {
        target: Client
        onShowMsg: {
            if(inConversationWith === msgFrom) {
                messageModel.append({message: msg, index : 0, image: false})
            } else if(Client.getUsername() === msgFrom) {
                messageModel.append({message: msg, index : 1, image: false})
            }
        }

        onShowPicture: {
            console.log(picturePath)
            messageModel.append({image: true, message: picturePath, index: 0})
        }
    }



    ColumnLayout {
        anchors.fill: parent

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: pane.leftPadding
            spacing: 20

            model: messageModel

            delegate: Loader {
                anchors.right: {
                if(index) parent.right
                }

                sourceComponent: image ? imageMsg : textMsg

                Component {
                    id: textMsg
                    Rectangle {
                        width: lblMsg.width + 2*lblMsg.anchors.margins
                        height: lblMsg.height + 2*lblMsg.anchors.margins
                        radius: 10
                        color: index ? "#4F7942" : "#808080"

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
                }



                Component {
                    id: imageMsg
                    Image {
                        width: sourceSize.width * 0.3
                        height: sourceSize.height * 0.3
                        source: model.message
                        fillMode: Image.PreserveAspectFit
                    }
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
                            messageModel.append({message: messageField.text.trim(), index : 1, image: false});
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
                        messageModel.append({message: messageField.text, index : 1, image: false});
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
