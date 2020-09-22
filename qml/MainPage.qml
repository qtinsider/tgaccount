import QtQuick 1.1
import com.nokia.meego 1.1

Page {
    id: mainPage
    anchors.fill: parent

    Component.onCompleted: {
        if (initType === 1) {
            editTools.visible = true
            tools = editTools
        } else {
            commonTools.visible = true
            tools = commonTools
        }
    }

    function checkFilled() {
        return (displayname.text.length > 0 && username.text.length > 0 && password.text.length > 0);
    }

    function createAccount() {
        createfade.visible = true;
        actor.createAccount(displayname.text,
                            username.text,
                            password.text,
                            showicon.checked);
    }

    function saveAccount() {
        actor.saveAccount(displayname.text,
                          username.text,
                          password.text,
                          server.text,
                          port.text,
                          encryption.checked,
                          showicon.checked,
                          title.checked);
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: 5
        contentWidth: width
        contentHeight: contentcolumn.height
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: contentcolumn
            width: parent.width
            spacing: 10

            Title {
                id: title
                image: "image://theme/%1".arg(qsTrId("qtn_%1_icon".arg(xservice)))
                title: qsTr("Telegram account")
                subtitle: qsTr("Different, Handy, Powerful")
                width: parent.width
                showcheckbox: initType === 1
                checked: enabledvalue
            }

            LabeledTextField {
                id: displayname
                width: parent.width
                label: qsTr("qtn_display_name")
                placeholderText: qsTr("qtn_set_display_name")
                text: displaynametext
            }

            LabeledTextField {
                id: username
                width: parent.width
                inputMethodHints: unamenumeric ? Qt.ImhDigitsOnly : 0
                label: qsTr("Username")
                placeholderText: qsTr("Set your username")
                text: usernametext
            }

            LabeledTextField {
                id: password
                width: parent.width
                echoMode: TextInput.Password
                label: qsTr("Password")
                placeholderText: qsTr("Set your password")
                text: passwordtext
            }

            LabeledTextField {
                id: server
                width: parent.width
                label: qsTr("Server")
                placeholderText: qsTr("Server")
                visible: (initType === 1) ? serverset : false
                clearbutton: true
                text: servertext
            }

            LabeledTextField {
                id: port
                width: parent.width
                inputMethodHints: Qt.ImhDigitsOnly
                label: qsTr("Port")
                placeholderText: qsTr("Port")
                clearbutton: true
                visible: portinactive ? false : (initType === 1)
                text: porttext
            }

            LabeledSwitch {
                id: encryption
                width: parent.width
                text: qsTr("Enable encryption")
                visible: (initType === 1) ? encryptionset : false
                checked: encryptionvalue
            }

            LabeledSwitch {
                id: showicon
                width: parent.width
                text: qsTr("qtn_desktop_icon")
                checked: showiconvalue
            }

            ButtonElement {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Create account")
                visible: initType !== 1
                width: parent.width / 2
                enabled: checkFilled()
                onClicked: createAccount()
            }

            Item {
                height: 120 - title.extraheight
                width: parent.width
                visible: extratextvisible
            }


            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 24
                visible: extratextvisible
                text: "<a style='color:#40a0ff' href='%1'>%2</a>".arg(qsTrId("qtn_%1_extra_link".arg(xservice))).arg(qsTrId("qtn_%1_extra_text".arg(xservice)))
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
    }

    Rectangle {
        id: createfade
        anchors.fill: parent
        color: "black"
        visible: onloadbusy

        Column {
            anchors.centerIn: parent
            width: parent.width
            spacing: 30

            BusyIndicator {
                anchors.horizontalCenter: parent.horizontalCenter
                platformStyle: BusyIndicatorStyle { size: "large" }
                running: parent.visible
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                text: qsTr("Creating")
                visible: !onloadbusy
            }
        }
    }
}
