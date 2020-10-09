import QtQuick 1.1
import com.nokia.meego 1.1

Page {
    id: root

    property string passwordHint: ""

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
                subtitle: qsTr("Your have Two-Step Verification enabled,\nso your account is protected with an\nadditional password")
                width: parent.width
            }

            Item {
                width: parent.width
                height: 20
            }
            Label {
                text: qsTr("Your Password")
            }

            TextField {
                id: password
                width: parent.width
                echoMode: TextInput.Password
                placeholderText: qsTr("Password")
            }

            Label {
                font.pixelSize: 24
                text: qsTr("Password hint: <i>%1</i>").arg(passwordHint)
                visible: passwordHint !== ""
            }

            Item {
                height: 20
                width: parent.width
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Next")
                width: parent.width / 2
                onClicked: setPassword()
            }

        }
    }


    function setPassword() {
        if (password.text.length > 0) {
            actor.checkPassword(password.text)
            pageStack.busy = true
            password.text = ""
        } else {
            showBanner("Password field can't be empty")
        }
    }

    tools: ToolBarLayout {
        id: passwordTools
        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: pageStack.pop()
        }
    }
}
