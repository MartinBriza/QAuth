import QtQuick 1.1
import QAuth 1.0

Rectangle {
    width: 480
    height: 360
    color: "#EEEEEE"

    QAuth {
        id: auth
        verbose: true

        request {
            finishAutomatically: true
        }

        onError: {
            requestError.text = message
        }
        onFinished: {
            status.text = "Finished"
        }

        Component.onCompleted: {
            auth.start()
        }
    }

    Component {
        id: promptDelegate
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            Rectangle {
                anchors.centerIn: parent
                width: parent.width - 6
                height: parent.height - 6
                Text {
                    id: promptMessage
                    text: message
                }
                TextInput {
                    anchors.right: parent.right
                    id: promptInput
                    width: parent.width - promptMessage.width - 8
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
    }

    Row {
        id: statusRect
        anchors.horizontalCenter: parent.horizontalCenter
        Text {
            id: statusText
            text: "Status: "
        }
        Text {
            id: status
            text: "Authenticating"
        }
    }
    Text {
        id: requestInfo
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: statusRect.bottom
        text: auth.request.info
    }
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: requestInfo.bottom
        id: requestError
    }

    Rectangle {
        color: "#DDDDDD"
        anchors.top: requestError.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 20
        height: parent.height - y + parent.y
        ListView {
            id: promptList
            delegate: promptDelegate
            model: auth.request.prompts
            width: parent.width
        }
    }

}