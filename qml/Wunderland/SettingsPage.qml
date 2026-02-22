import QtQuick 1.1
import com.nokia.symbian 1.1
import com.nokia.extras 1.1
import QtMobility.feedback 1.1

Page {
    id: settingsPage
    tools: toolBarLayout

    Rectangle {
        id: background
        color: window.white
        anchors.fill: parent
    }

    Rectangle {
        id: settingsHeader
        width: parent.width
        height: settingsHeaderText.height + 11
        color: window.white

        Text {
            id: settingsHeaderText
            text: qsTr("Settings") + l.emptyString
            width: parent.width - 20
            wrapMode: TextEdit.Wrap
            font.pixelSize: 18
            color: window.black
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
        }

        Rectangle {
            id: border
            height: 1
            width: parent.width
            color: window.black
            anchors.bottom: parent.bottom
        }
    }

    Flickable {
        id: flickArea
        height: parent.height// - aboutHeader.height - basicInfo.height
        width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: settingsHeader.bottom
        contentWidth: parent.width
        contentHeight: settingsItem.height
        flickableDirection: Flickable.VerticalFlick
        clip: true

        Rectangle {
            id: settingsItem
            width: parent.width
            height: openInBrowserItem.height + darkThemeItem.height + aasFeedItem.height + aboutItem.height + 11 + 6
            color: window.white

            

            Item {
                id: openInBrowserItem
                anchors.top: parent.top
                anchors.topMargin: 5
                width: parent.width
                height: 50

                Text {
                    text: qsTr("Open links in phone browser") + ":" + l.emptyString
                    color: window.black
                    font.pixelSize: 20
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: openInBrowserSwitch.left
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    wrapMode: Text.WordWrap
                }

                Switch {
                    id: openInBrowserSwitch
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter

                    checked: news.getOpenInBrowser()

                    onClicked: {
                        news.setOpenInBrowser(checked)
                    }
                }
            }

            Rectangle {
                id: border1
                height: 1
                width: parent.width
                color: window.lightgrey
                anchors.top: openInBrowserItem.bottom
                anchors.topMargin: 5
            }

            Item {
                id: darkThemeItem
                anchors.top: openInBrowserItem.bottom
                anchors.topMargin: 10
                width: parent.width
                height: 50

                Text {
                    text: qsTr("Dark theme") + ":" + l.emptyString
                    color: window.black
                    font.pixelSize: 20
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: darkThemeSwitch.left
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    wrapMode: Text.WordWrap
                }

                Switch {
                    id: darkThemeSwitch
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter

                    checked: settings.darkTheme()

                    onClicked: {
                        settings.setDarkTheme(checked)
                        infoBanner.open()
                    }
                }
            }

            Rectangle {
                id: border2
                height: 1
                width: parent.width
                color: window.lightgrey
                anchors.top: darkThemeItem.bottom
                anchors.topMargin: 5
            }

            Item {
                id: aasFeedItem
                anchors.top: darkThemeItem.bottom
                anchors.topMargin: 10
                width: parent.width
                height: 50

                Text {
                    text: qsTr("Show latest news from Wunderland Store") + ":" + l.emptyString
                    color: window.black
                    font.pixelSize: 20
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: aasFeedSwitch.left
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    wrapMode: Text.WordWrap
                }

                Switch {
                    id: aasFeedSwitch
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter

                    checked: settings.aasFeed()

                    onClicked: {
                        settings.setAasFeed(checked)
                        infoBanner.open()
                    }
                }
            }

            Rectangle {
                id: border6
                height: 1
                width: parent.width
                color: window.lightgrey
                anchors.top: aasFeedItem.bottom
                anchors.topMargin: 5
            }

            
        }
    }

    InfoBanner {
        id: infoBanner
        text: qsTr("This setting change will take effect after restarting Wunderland.")
    }


    ThemeEffect {
        id: tf
        effect: ThemeEffect.BasicButton;
    }
}
