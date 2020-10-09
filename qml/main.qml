import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1

PageStackWindow {
    id: appWindow

    initialPage: actor.getAuthorizationState() === "Ready" ? editExistingView : mainPage

    MainPage { id: mainPage }
    EditExistingView { id: editExistingView }

    ToolBarLayout {
        id: commonTools
        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: Qt.quit()
        }
    }

    Connections {
        target: actor
        onCodeRequested: {
            pageStack.busy = false
            pageStack.push(Qt.resolvedUrl("CodeView.qml"), {
                               phoneNumber: phoneNumber,
                               type: type,
                               nextType: nextType,
                               timeout: timeout * 1000})

            actor.setValue("phoneNumber", phoneNumber)
        }

        onPasswordRequested: {
            pageStack.busy = false
            pageStack.push(Qt.resolvedUrl("PasswordView.qml"), { passwordHint: passwordHint })
        }

        onRegistrationRequested: {
            pageStack.busy = false
            pageStack.push(Qt.resolvedUrl("RegistrationView.qml"), { text: text })
        }

        onError: {
            pageStack.busy = false
            showBanner(errorString)
        }
    }

    Rectangle {
        z: 2
        anchors.fill: parent
        visible: pageStack.busy
        color: "#70000000"

        BusyIndicator  {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            running: true
            platformStyle: BusyIndicatorStyle { size: "large" }
        }
    }

    InfoBanner {
        id: banner
        y: 36 /* StatusBar height */ + 8
    }

    function showBanner(text) {
        banner.text = actor.getLocalizedString(text)
        banner.show()
    }

    Component.onCompleted: {
        var parameters = {
            api_id: 142713,
            api_hash: "9e9e687a70150c6436afe3a2b6bfd7d7",
            useFileDatabase: true,
            useChatInfoDatabase: true,
            useMessageDatabase: true,
            useSecretChats: true,
            enableStorageOptimizer: true
        }

        if (actor.getAuthorizationState() === "WaitTdlibParameters") {
            actor.setTdlibParameters(parameters)
            actor.setValue("parameters", parameters)
        }
    }

}
