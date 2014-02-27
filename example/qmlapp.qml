import QtQuick 1.1
import QAuth 1.0

Rectangle {
    width: 480
    height: 200
    color: "#EEEEEE"

    QAuth {
        id: auth
        verbose: true
        user: "test1"
        onRequestChanged: {
            requestInfo.text = request.info
            promptList.model = request.prompts
            request.finishAutomatically = true
        }
        onError: {
            requestError.text = message
        }
        Component.onCompleted: {
            auth.start()
        }
    }

    Component {
        id: promptDelegate
        Item {
            width: parent.width
            Text {
                id: promptMessage
                text: message
            }
            TextInput {
                id: promptInput
                anchors.left: promptMessage.right
                width: parent.width / 2
                echoMode: hidden ? TextInput.Password : TextInput.Normal
                onAccepted: {
                    response = text
                }
                Rectangle {
                    z: -1
                    anchors.fill: parent
                    color: "#FFFFFF"
                }
            }
        }
    }

    Text {
        id: requestInfoText
        text: "Current request info: "
    }
    Text {
        anchors.left: requestInfoText.right
        id: requestInfo
    }
    Text {
        anchors.top: requestInfoText.bottom
        id: requestErrorText
        text: "Current request error: "
    }
    Text {
        anchors.left: requestErrorText.right
        anchors.top: requestErrorText.top
        id: requestError
    }
    Text {
        anchors.top: requestErrorText.bottom
        id: statusText
        text: "Status: "
    }
    Text {
        anchors.left: statusText.right
        anchors.top: statusText.top
        id: status
        text: "Authenticating"
    }

    ListView {
        id: promptList
        delegate: promptDelegate
        width: parent.width - 8
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: statusText.bottom
    }

}