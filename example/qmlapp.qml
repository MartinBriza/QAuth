import QtQuick 1.1
import QAuth 1.0

Rectangle {
    width: 320
    height: 200
    color: "#EEEEEE"

    QAuth {
        id: auth
        verbose: true
        user: "test1"
        onRequest: {
            requestInfo.text = request.info
            promptList.model = request.prompts
        }
    }

    Component {
        id: promptDelegate
        Item {
            Text {
                id: promptMessage
                text: message
            }
            Rectangle {
                color: "#FFFFFF"
                anchors.left: promptMessage.right
                TextInput {
                    id: promptInput
                    text: "OFC"
                    width: 40
                    focus: true
                }
            }
        }
    }

    Grid {
        id: infoGrid
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 8
        spacing: 4
        columns: 2

        Text {
            text: "User:"
        }
        Text {
            id: userName
            text: auth.user
        }

        Text {
            text: "Current request info:"
        }
        Text {
            id: requestInfo
        }

    }

    ListView {
        id: promptList
        delegate: promptDelegate
        width: parent.width - 8
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: infoGrid.bottom
        spacing: 4
    }

    Component.onCompleted : auth.start()
}