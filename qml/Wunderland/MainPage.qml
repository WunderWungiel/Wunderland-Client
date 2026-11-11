import QtQuick 1.1
import com.nokia.symbian 1.1
import QtMobility.feedback 1.1

Page {
    id: mainPage
    tools: toolBarLayoutMain

    Rectangle {
        id: appListItem
        width: parent.width
        height: window.aasFeed ? parent.height/2 : parent.height
        color: window.bgColor

        Rectangle {
            id: appListHeader
            width: parent.width
            height: appListHeaderText.height + 11
            color: window.bgColor

            Text {
                id: appListHeaderText
                text: qsTr("Latest apps and games")
                width: parent.width - 20
                wrapMode: TextEdit.Wrap
                font.pixelSize: 18
                color: window.textColor
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 10
            }

            Rectangle {
                id: border
                height: 1
                width: parent.width
                color: window.textColor
                anchors.bottom: parent.bottom
            }
        }

        AppListView {
            id: appList
            anchors { top: appListHeader.bottom; bottom: parent.bottom; left: parent.left; right: parent.right; }

            listModel: appCatalog._mainList

            onRefreshEvent: {
                appCatalog.loadMainList()
            }
        }

        Connections {
            target: appCatalog
            onMainListChanged: {
                console.debug("mainListChanged QML")
                appList.stopLoading()
            }
        }
    }

    Rectangle {
        id: border2
        height: 2
        width: parent.width
        color: window.textColor
        anchors.top: appListItem.bottom
        anchors.topMargin: -1
    }

    Rectangle {
        id: newsItem
        width: parent.width
        height: window.aasFeed ? parent.height/2 : 0
        anchors.top: border2.bottom
        color: window.bgColor
        visible: window.aasFeed

        Rectangle {
            id: newsHeader
            width: parent.width
            height: newsHeaderText.height + 11
            color: window.bgColor

            Text {
                id: newsHeaderText
                text: qsTr("Latest news from Wunderland Store")
                width: parent.width - 20
                wrapMode: TextEdit.Wrap
                font.pixelSize: 18
                color: window.textColor
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 10
            }

            Rectangle {
                id: border3
                height: 1
                width: parent.width
                color: window.textColor
                anchors.bottom: parent.bottom
            }
        }

        NewsElement {
            id: newsElement
            anchors { top: newsHeader.bottom; bottom: parent.bottom; left: parent.left; right: parent.right; }
            topMargin: mainPage.height/2 + newsHeader.height
        }
    }

    ToolBarLayout {
        id: toolBarLayoutMain
        ToolButton {
            flat: true
            iconSource: window.pageStack.depth <= 1 ? "images/close_stop.svg" : "toolbar-back"
            onClicked: {
                if (newsElement.browserStatus) {
                    // Browser open, close browser
                    newsElement.closeBrowser()
                } else {
                    // Back
                    window.pageStack.depth <= 1 ? window.closeApp() : window.pageStack.pop()
                }
            }
        }

        ToolButton {
            flat: true
            iconSource: "toolbar-home"
            visible: !newsElement.browserStatus
            enabled: !newsElement.browserStatus
            onClicked: if (window.pageStack.depth > 1) window.pageStack.pop(null)
        }

        ToolButton {
            flat: true
            iconSource: "toolbar-list"
            visible: !newsElement.browserStatus
            enabled: !newsElement.browserStatus
            onClicked: window.pageStack.push( Qt.resolvedUrl("CategoryPage.qml") )
        }

        ToolButton {
            flat: true
            iconSource: "toolbar-search"
            visible: !newsElement.browserStatus
            enabled: !newsElement.browserStatus
            onClicked: window.pageStack.push( Qt.resolvedUrl("SearchPage.qml") )
        }

        ToolButton {
            flat: true
            iconSource: "images/contacts.svg"
            visible: !newsElement.browserStatus
            enabled: !newsElement.browserStatus
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
                    color: "#FFFFFF"
                    font.pixelSize: 16
                    text: updateTracker.count
                }
            }
        }
    }

    ThemeEffect {
        id: tf
        effect: ThemeEffect.BasicButton;
    }
}
