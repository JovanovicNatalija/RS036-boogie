import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.1

Page {
    id: root

    property string inConversationWith
    property string from
    property int index : 0

    header: ToolBar {
        ToolButton {
            text: qsTr("Back")
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            onClicked: root.StackView.view.pop()
        }
        Label {
            id: pageTitle
            text: inConversationWith
            font.pixelSize: 20
            anchors.centerIn: parent
        }
    }

    ListModel {
        id: messageModel
    }

    Connections {
        target: Client
        onShowMsg: {
            if(inConversationWith === msgFrom)
                messageModel.append({message: msg, index : 0})
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
                    placeholderText: qsTr("Compose message")
                    wrapMode: TextArea.Wrap
                }

                Button {
                    id: confirmButton
                    text: qsTr("Confirm")
                    enabled: messageField.length > 0
                    onClicked: {
						Client.sendMsgData(from, inConversationWith, messageField.text)
                        messageModel.append({message: messageField.text, index : 1});
                        messageField.clear()
                    }
                }
            }
        }
    }
}
