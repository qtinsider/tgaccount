import QtQuick 1.1
import com.nokia.meego 1.1

Page {
    id: root
    tools: commonTools

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
                width: parent.width
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("You can only have one Telegram account for now")
                font.pixelSize: 36
                color: "#0088cc"
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
                visible: initType === 0
            }

            Label{
                width:parent.width
                text: qsTr("<style type=text/css> b { color: #808000 } </style>This Telegram account is connected to <b>%1</b>").arg(currentPhoneNumber)
                font.pixelSize: 36
                color: "#0088cc"
                horizontalAlignment: Text.AlignHCenter
                visible: initType === 1
            }

            Item {
                height: 60
                width: parent.width
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("<style type=text/css> b { color: #ff3b28 } </style> <b>Delete account</b>")
                width: parent.width / 2
                visible: initType === 1
                onClicked: {
                    confirmDialog.open()
                }
            }
        }
    }

    QueryDialog {
        id: confirmDialog
        titleText: qsTrId("Delete account")
        message: qsTrId("Are you to delete your Telegram account")
        acceptButtonText: qsTrId("Accept")
        rejectButtonText: qsTrId("Reject")
        onAccepted: actor.deleteAccount()
    }
}
