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
	property int grId

	Component.onCompleted: {
		Client.displayOnConvPage(inConversationWith)
        Client.displayOnConvPage(grId.toString())
	}

	header: ToolBar {
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

        MouseArea {
            id: notification
            width: parent.width / 3
            height: parent.height
            Rectangle {
                id: recNotification
                width: parent.width
                height: parent.height
                color: "grey"
                visible: false
                radius: 15
                Label {
                    id:labelNotification
                    anchors.centerIn: parent
                }
            }
            anchors.right: parent.right
            onClicked: {
                labelNotification.text = ""
                recNotification.visible = false
                notification.enabled = false
            }
        }
	}

	FileDialog {
		id: fileDialog
		visible: false
		title: "Izaberi sliku"
		folder: shortcuts.home
        nameFilters: ["Images files (*.jpg *png *.jpeg)"]
		onAccepted: {
			var path = fileDialog.fileUrl.toString()
            if(grId == -1) {
                Client.sendPicture(inConversationWith, path)
                Client.addMsgToBuffer(false, Client.username(), inConversationWith, path, "image")
                messageModel.append({image: true, message: path, index: 1})
            } else {
                Client.sendGroupPictureData(grId, path)
                Client.addMsgToBuffer(true, Client.username(), grId.toString(), path, "image")
                messageModel.append({index: 1, message: path, image: true})
            }
		}
	}

	ListModel {
		id: messageModel
	}

	Connections {
		target: Client
		onShowMsg: {
			console.log(msg)
			if(inConversationWith === msgFrom) {
				messageModel.append({message: msg, index : 0, image: false})
			} else if(Client.username() === msgFrom) {
				messageModel.append({message: msg, index : 1, image: false})
            } else {
                labelNotification.text = msgFrom + " : " + msg
                recNotification.visible = true
                notification.enabled = true
                Client.unreadMsg(msgFrom)
            }

		}
		onShowMsgForGroup: {
			if(grId == groupId){
               if(Client.username() === msgFrom) {
                    messageModel.append({message: msg, index : 1, image: false})
                } else {
                    messageModel.append({message: msgFrom + ":\n" + msg, index : 0, image: false})
                    // TODO: POPRAVI OVO DA SE SALJE I IME GRUPE
                    // labelNotification.text = msgFrom + " : " + msg
                    // recNotification.visible = true
                    // notification.enabled = true
                    // Client.unreadMsg(msgFrom)
                }
			}
		}

		onShowPicture: {
			if(inConversationWith === msgFrom) {
                messageModel.append({from: "", message: path, index : 0, image: true})
			} else if(Client.username() === msgFrom) {
                messageModel.append({from: "", message: path, index : 1, image: true})
            } else {
                labelNotification.text = msgFrom + " : " + slika
                recNotification.visible = true
                notification.enabled = true
                Client.unreadMsg(msgFrom)
            }
		}

        onShowPictureForGroup: {
            if(grId == groupId) {
                if(Client.username() === msgFrom) {
                    messageModel.append({from: "", message: path, index : 1, image: true})
                } else {
                    messageModel.append({from: msgFrom + ":", message: path, index : 0, image: true})
                    console.log(msgFrom)
                    // TODO!
                    // labelNotification.text = msgFrom + " : " + slika
                    // recNotification.visible = true
                    // notification.enabled = true
                    // Client.unreadMsg(msgFrom)
                }
            }
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
                    Rectangle {
                        color: "#808080"
                        width: image.width
                        height: image.height + (fromLabel.text == "" ? 0 : fromLabel.height + 10)
                        Label {
                            color: "black"
                            id: fromLabel
                            text: model.from
                        }

                        Image {
                            id:image
                            width: sourceSize.width * 0.3
                            height: sourceSize.height * 0.3
                            source: model.message
                            fillMode: Image.PreserveAspectFit
                            anchors.bottom: parent.bottom
                        }
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
					Keys.onReturnPressed: {
						if(grId == -1){
							if(messageField.text.trim() !== "") {
								Client.sendMsgData(inConversationWith, messageField.text.trim())
                                Client.addMsgToBuffer(false, Client.username(), inConversationWith, messageField.text.trim(), "text")
								messageModel.append({message: messageField.text.trim(), index : 1, image : false})
							}
						}else {
                            if(messageField.text.trim() !== "") {
                                Client.sendGroupMsgData(grId, messageField.text.trim())
                                Client.addMsgToBuffer(true, Client.username(), grId.toString(), messageField.text.trim(), "text")
                                messageModel.append({message: messageField.text.trim(), index: 1, image: false})
                            }
                        }

						messageField.clear()
					}
				}


				Button {
					id: confirmButton
					text: qsTr("Pošalji")
					enabled: messageField.length > 0
					onClicked: {
						if(grId == -1){
							if(messageField.text.trim() !== "") {
								Client.sendMsgData(inConversationWith, messageField.text.trim())
                                Client.addMsgToBuffer(falsem, Client.username(), inConversationWith, messageField.text.trim(), "text")
								messageModel.append({message: messageField.text.trim(), index : 1, image: false})
							}
						}else {
                            if(messageField.text.trim() !== "") {
                                Client.sendGroupMsgData(grId, messageField.text.trim())
                                Client.addMsgToBuffer(true, Client.username(), grId.toString(), messageField.text.trim(), "text")
                                messageModel.append({message: messageField.text.trim(), index: 1, image:false})
                             }
						}

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
