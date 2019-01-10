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
	}

	FileDialog {
		id: fileDialog
		visible: false
		title: "Izaberi sliku"
		folder: shortcuts.home
		nameFilters: ["Images files (*.jpg, *png)"]
		onAccepted: {
			var path = fileDialog.fileUrl.toString()
			console.log(path)
			Client.sendPicture(inConversationWith, path)
			Client.addMsgToBuffer(Client.username(), inConversationWith, path, "image")
			messageModel.append({image: true,message: path, index: 1})
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
			}
		}
		onShowMsgForGroup: {
			if(grId == groupId){
				if(Client.username() === msgFrom){
                    messageModel.append({message: msgFrom + ":\n" + msg, index : 1, image: false})
				}
				else {
                    messageModel.append({message: msgFrom + ":\n" + msg, index : 0, image: false})
				}
			}
		}

		onShowPicture: {
			console.log(path)
			if(inConversationWith === msgFrom) {
				messageModel.append({message: path, index : 0, image: true})
			} else if(Client.username() === msgFrom) {
				messageModel.append({message: path, index : 1, image: true})
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
					Keys.onReturnPressed: {
						if(grId == -1){
							if(messageField.text.trim() !== "") {
								Client.sendMsgData(inConversationWith, messageField.text.trim())
								Client.addMsgToBuffer(Client.username(), inConversationWith, messageField.text.trim(), "text")
								messageModel.append({message: messageField.text.trim(), index : 1, image : false})
							}
						}else {
							Client.sendGroupMsgData(grId, messageField.text.trim())
							messageModel.append({message: messageField.text.trim(), index: 1, image: false})
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
								Client.addMsgToBuffer(Client.username(), inConversationWith, messageField.text.trim(), "text")
								messageModel.append({message: messageField.text.trim(), index : 1, image: false})
							}
						}else {
							Client.sendGroupMsgData(grId, messageField.text.trim())
							messageModel.append({message: messageField.text.trim(), index: 1, image:false})
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
