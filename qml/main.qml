import QtQuick 1.1
import com.nokia.meego 1.1

PageStackWindow {
    id: appWindow

    initialPage: mainPage

    MainPage {
        id: mainPage
    }

    ToolBarLayout {
        id: commonTools
        visible: false
        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: Qt.quit()
        }
    }

    ToolBarLayout {
        id: editTools
        visible: false
        ToolButton {
            text: qsTr("Save")
            enabled: mainPage.checkFilled()
            onClicked: mainPage.saveAccount()
        }
        ToolButton {
            text: qsTr("Cancel")
            onClicked: Qt.quit()
        }
        ToolIcon {
            platformIconId: "toolbar-view-menu"
            anchors.right: (parent === undefined) ? undefined : parent.right
            onClicked: (myMenu.status === DialogStatus.Closed) ? myMenu.open() : myMenu.close()
        }
    }

    Menu {
        id: myMenu
        visualParent: pageStack
        MenuLayout {
            MenuItem { text: qsTr("Delete account"); onClicked: delconfirm.open() }
        }
    }

    QueryDialog {
        id: delconfirm
        titleText: qsTr("Delete account")
        message: qsTr("Are you sure to delete account")
        acceptButtonText: qsTr("Ok")
        rejectButtonText: qsTr("Cancel")
        onAccepted: actor.deleteAccount()
    }
}
