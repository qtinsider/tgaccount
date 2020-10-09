import QtQuick 1.1
import com.nokia.meego 1.1

Page {
    id: root

    property string phoneNumber: ""
    property variant type
    property variant nextType: null
    property int timeout: 0

    anchors.fill: parent

    Flickable {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        width:  parent.width
        height: parent.height
        contentHeight: 600

        Column {
            width: parent.width
            spacing: 10

            Title {
                id: title
                image: "telegram-logo.png"
                title: qsTr("Telegram account")
                subtitle: qsTr("Phone verification")
                width: parent.width
            }

            Item { height: 20; width: parent.width }

            Label {
                text: qsTr("Enter code")
            }

            Column {
                width: parent.width
                spacing: 20

                TextField {
                    id: code
                    width: parent.width
                    inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
                    placeholderText: qsTr("Code")
                }

                Label {
                    font.pixelSize: 24
                    text: (type.type !== "Telegram") ? actor.getLocalizedString(type.type).arg(phoneNumber) : actor.getLocalizedString(type.type)
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Next")
                    width: parent.width / 2
                    onClicked: {
                        if (code.text.length > 0) {
                            pageStack.busy = true
                            actor.checkCode(code.text)
                            code.text = ""
                        } else {
                            showBanner("Code field can't be empty")
                        }
                    }
                }

                Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: 24
                    font.underline: true
                    color: "#0088cc"
                    visible: nextType.type === "SMS"
                    text: qsTr("Send code via SMS")

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            pageStack.busy = true
                            actor.resendCode()
                        }
                    }
                }

                Row {
                    id: codeTextItem
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 4
                    Label {
                        horizontalAlignment: Text.AlignHCenter
                        font.pointSize: 24
                        text: qsTr("Telegram will call you in ")
                    }
                    Label {
                        id: codeTimeText
                        horizontalAlignment: Text.AlignHCenter
                        font.pointSize: 24
                        color: "#ffcc00"
                    }
                    visible: nextType.type === "Call"
                }

                Timer {
                    id: codeExpireTimer
                    interval: 1000
                    repeat: true
                    onTriggered: {
                        timeout = timeout - 1000
                        codeTimeText.text = actor.formatTime(timeout / 1000)
                        if(timeout === 0) {
                            codeExpireTimer.stop()
                            pageStack.busy = true
                            codeTextItem.visible = false
                            actor.resendCode()
                        }
                    }
                }
            }

        }
    }

    onTimeoutChanged: {
        codeExpireTimer.start()
        codeTimeText.text = actor.formatTime(timeout / 1000)
    }

    tools: ToolBarLayout {
        id: codeTools
        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: pageStack.pop()
        }
    }
}
