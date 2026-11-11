import QtQuick 1.1
import com.nokia.symbian 1.1
import com.nokia.extras 1.1
import QtMobility.feedback 1.1
import Wunderland 1.0

PageStackWindow {
    id: window
    initialPage: MainPage {}
    showStatusBar: true
    showToolBar: true

    property int appId: -1 // the app to open when the update list has loaded
    property int failedAppId: -1

    // Theme. The pages use these colour roles, never colour names. The
    // Symbian components (toolbar, dialogs, switches) keep their default
    // dark style; only ones drawn on bgColor follow the theme
    // (platformInverted: !window.darkTheme).
    property bool darkTheme: true
    property color bgColor: darkTheme ? "black" : "#FCFCFC"
    property color bgTransparentColor: darkTheme ? "#00000000" : "#00FFFFFF" // for gradients into bgColor
    property color textColor: darkTheme ? "#FCFCFC" : "black"
    property color subtleColor: darkTheme ? "lightgrey" : "grey" // secondary text
    property color lineColor: darkTheme ? "grey" : "lightgrey" // separators, shaded rows
    property color highlightColor: "#4791D9" // pressed items, update marks; text on it uses bgColor

    property bool aasFeed: true

    Component.onCompleted: {
        // Not bindings: these settings apply after a restart
        darkTheme = settings.darkTheme
        aasFeed = settings.aasFeed

        appCatalog.loadMainList()

        if (launchAppId !== -1) {
            // Open app in AppList
            appCatalog.loadUpdateList(launchAppId)
            appId = launchAppId
        }
    }

    Connections {
        target: appCatalog
        onAppListUpdate: {
            console.debug("appListUpdate QML")
            window.pageStack.push(Qt.resolvedUrl("UpdatePage.qml"))
        }
        onLoadingError: {
            // The app to open (launchAppId, subscriber) could not be loaded
            if (listId == AppCatalog.UpdateList && appId !== -1) {
                failedAppId = appId
                appId = -1
                openAppBanner.text = qsTr("The app could not be opened.") + " " + loadingErrorText(error) + " " + qsTr("Tap to retry.")
                openAppBanner.open()
            }
        }
        onUpdateListChanged: {
            if (appId !== -1) {
                appId = -1
                window.pageStack.push (Qt.resolvedUrl("DetailPage.qml"), { "detailItem": appCatalog._updateList.get(0) })
            }
        }
    }

    Connections {
        target: subscriber
        onAppListIdReceived: {
            if (appListId !== -1) {
                appCatalog.loadUpdateList(appListId)
                appId = appListId
            }
        }
    }

    ToolBarLayout {
        id: toolBarLayout
        ToolButton {
            flat: true
            iconSource: "toolbar-back"
            onClicked: window.pageStack.depth <= 1 ? closeApp() : window.pageStack.pop()
        }

        ToolButton {
            flat: true
            iconSource: "toolbar-home"
            onClicked: if (window.pageStack.depth > 1) window.pageStack.pop(null)
        }

        ToolButton {
            flat: true
            iconSource: "toolbar-list"
            onClicked: window.pageStack.push( Qt.resolvedUrl("CategoryPage.qml") )
        }

        ToolButton {
            flat: true
            iconSource: "toolbar-search"
            onClicked: window.pageStack.push( Qt.resolvedUrl("SearchPage.qml") )
        }

        ToolButton {
            flat: true
            iconSource: "images/contacts.svg"
            onClicked: window.pageStack.push ( Qt.resolvedUrl("PersonalPage.qml") )

            Image {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenterOffset: -parent.height/2
                anchors.horizontalCenterOffset: parent.width/4
                width: parent.width/2
                height: parent.height/2
                source: "images/updateicon.png"
                visible: (updateTracker.count > 0)

                Text {
                    anchors.centerIn: parent
                    color: "#FCFCFC"
                    font.pixelSize: 16
                    text: updateTracker.count
                }
            }
        }
    }

    InfoBanner {
        id: openAppBanner
        interactive: true
        timeout: 6000
        onClicked: {
            close()
            appId = failedAppId
            appCatalog.retry(AppCatalog.UpdateList)
        }
    }

    // Text for a StoreApi error code (appCatalog.loadingError)
    function loadingErrorText(error) {
        if (error == StoreApi.NetworkError)
            return qsTr("Could not connect to the Wunderland Store. Please check your internet connection.")
        if (error == StoreApi.TimeoutError)
            return qsTr("The Wunderland Store did not answer in time.")
        if (error == StoreApi.HttpError)
            return qsTr("The Wunderland Store is not available right now.")
        return qsTr("The Wunderland Store sent an invalid response.")
    }

    // Text for a Downloader error code (installManager/update.downloadError),
    // except NoSpaceError (1), which needs the download size. The DetailPage
    // context keeps the existing translations of the texts that came from there.
    function downloadErrorText(code) {
        if (code == 0) return qsTranslate("DetailPage", "A connection error appeared while trying to start the download.")
        if (code == 2) return qsTr("The download could not be saved on your device.")
        if (code == 3) return qsTr("The app was downloaded, but the installer could not be started.")
        return qsTranslate("DetailPage", "An unknown error appeared while trying to start the download.")
    }

    function closeApp() {
        if (installManager.state != InstallManager.Idle) closeDialog.open()
        else Qt.quit()
    }

    DialogComp {
        id: closeDialog
        text: qsTr("You are currently downloading an app. If you will now close Wunderland, the download will be aborted and has to be restarted next time.")
        buttonTexts: [qsTr("Close"), qsTr("Cancel")]
        titleText: "Note"

        onButtonClicked: {
            if (index == 0) {
                // Close app
                Qt.quit()
            } else {
                closeDialog.close()
            }
        }
    }

    ThemeEffect {
        id: tf
        effect: ThemeEffect.BasicButton;
    }
}
