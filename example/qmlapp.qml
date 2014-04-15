import QtQuick ${COMPONENTS_VERSION}
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

        onRequestChanged: {
            if (request.prompts.length == 0)
                request.done()
        }
        onInfo: {
            requestInfo.text = message
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
    }
    Text {
        id: requestError
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: requestInfo.bottom
    }

    Rectangle {
        color: "#DDDDDD"
        anchors.top: requestError.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 20
        height: parent.height - y
        ListView {
            anchors.fill: parent
            id: promptList
            delegate: Row {
                Text {
                    id: promptMessage
                    text: message
                }
                TextInput {
                    id: promptInput
                    width: 150
                    echoMode: hidden ? TextInput.Password : TextInput.Normal
                    onAccepted: {
                        response = text
                    }
                    Rectangle {
                        z: -1
                        anchors.fill: promptInput
                        color: "white"
                    }
                }
            }
            model: auth.request.prompts
        }
    }

}