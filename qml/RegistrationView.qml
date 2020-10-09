import QtQuick 1.1
import com.nokia.meego 1.1

Page {
    id: root

    property string text: ""

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
                subtitle: qsTr("Enter your name")
                width: parent.width
            }

            Item {
                width: parent.width
                height: 20
            }

            Label {
                text: qsTr("Your Name")
            }

            Column {
                spacing: 15
                width: parent.width

                TextField {
                    id: first_name
                    width: parent.width
                    inputMethodHints: Qt.ImhNoPredictiveText
                    placeholderText: qsTr("First name (required)")
                }
                TextField {
                    id: last_name
                    width: parent.width
                    inputMethodHints: Qt.ImhNoPredictiveText
                    placeholderText: qsTr("Last name (optional)")
                }
            }


            Item {
                height: 20
                width: parent.width
            }

            Label {
                width: parent.width
                text: qsTr("<style type=text/css> a { text-decoration: underline; color: #0088cc } </style>By signing up,<br>you agree to the <a href='http://www.telegram.com'>Terms of Service.</a>")
                font.pixelSize: 24
                onLinkActivated: terms.open()
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Done")
                width: parent.width / 2
                onClicked: registerUser()
            }

        }
    }

    function registerUser() {
        if (first_name.text.length > 0) {
            actor.registerUser(first_name.text, last_name.text)
            pageStack.busy = true
        } else {
            showBanner("Text field can't be empty")
        }
    }

    tools: ToolBarLayout {
        id: registrationTools
        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: pageStack.pop()
        }
    }


    QueryDialog{
        id: terms
        titleText: qsTr("Terms of Service")
        message: text
        acceptButtonText: qsTr("OK")
    }
}
