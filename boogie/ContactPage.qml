import QtQuick 2.6
import QtQuick.Controls 2.1

Page {
    property string username
    id: root
    header: ToolBar {
        Label {
            text: qsTr("Contacts")
            font.pixelSize: 20
            anchors.centerIn: parent
        }
    }

    ListView{
        id: listView
        anchors.fill: parent
        topMargin: 48
        leftMargin: 48
        bottomMargin: 48
        rightMargin: 48
        spacing: 20
        model: ["Natalija Jovanovic", "Vuk Novakovic", "Nikola Milovanovic"]
        delegate: ItemDelegate {
            text: modelData
            width: listView.width - listView.leftMargin - listView.rightMargin
            onClicked: {
                root.StackView.view.push("qrc:/ConversationPage.qml", { inConversationWith: modelData, from: username })
            }
        }

    }
}
