import QtQuick 1.1
import com.nokia.symbian 1.1
import Wunderland 1.0
import QtMobility.feedback 1.1

Page {
    id: personalPage
    tools: toolBarLayout

    property bool listLoaded: false

    onStatusChanged: {
        if (status == PageStatus.Activating) {
            if (!listLoaded) {
                personalList.startLoading()
                appCatalog.loadPersonalList(0, true)
                listLoaded = true
            }
        }
    }

    Rectangle {
        id: aboutItem
        width: parent.width
        height: settingsHeader.height + settingsItem.height + helpItem.height + 1 //11*2
        color: window.bgColor

        Rectangle {
            id: settingsHeader
            width: parent.width
            height: settingsHeaderText.height + 11
            color: window.bgColor

            Text {
                id: settingsHeaderText
                text: qsTr("Settings / About Wunderland")
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

        Rectangle {
            id: settingsItem
            anchors.top: settingsHeader.bottom
            height: settingsLabel.height + 34
            color: (settingsMouseArea.pressed) ? window.highlightColor : window.bgColor
            width: parent.width

            Text {
                id: settingsLabel
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                wrapMode: Text.WordWrap

                text: qsTr("Settings")
                font.pixelSize: 20

                color: (settingsMouseArea.pressed) ? window.bgColor : window.textColor
            }

            MouseArea {
                id: settingsMouseArea
                anchors.fill: parent

                onPressed: tf.play()
                onReleased: tf.play()
                onClicked: personalPage.pageStack.push( Qt.resolvedUrl("SettingsPage.qml") )
            }
        }

        Rectangle {
            id: border7
            height: 1
            width: parent.width
            color: window.lineColor
            anchors.top: settingsItem.bottom
        }

        Rectangle {
            id: helpItem
            anchors.top: border7.bottom
            height: label.height + 34
            color: (mouseArea.pressed) ? window.highlightColor : window.bgColor
            width: parent.width

            Text {
                id: label
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                wrapMode: Text.WordWrap

                text: qsTr("Help & info about Wunderland")
                font.pixelSize: 20

                color: (mouseArea.pressed) ? window.bgColor : window.textColor
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent

                onPressed: tf.play()
                onReleased: tf.play()
                onClicked: personalPage.pageStack.push( Qt.resolvedUrl("AboutPage.qml") )
            }
        }
    }

    Rectangle {
        id: border2
        height: 2
        width: parent.width
        color: window.textColor
        anchors.top: aboutItem.bottom
        anchors.topMargin: -1
    }

    Rectangle {
        id: personalListItem
        width: parent.width
        height: parent.height - aboutItem.height
        anchors.top: border2.bottom
        color: window.bgColor

        Rectangle {
            id: personalListHeader
            width: parent.width
            height: personalListHeaderText.height + 11
            color: window.bgColor

            Text {
                id: personalListHeaderText
                text: qsTr("My apps")
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

        Rectangle {
            id: updateItem
            anchors.top: personalListHeader.bottom
            height: (updateTracker.count > 0) ? updateLabel.height + 34 : 0
            color: (updateMouseArea.pressed) ? window.bgColor : window.highlightColor
            width: parent.width
            visible: (updateTracker.count > 0)

            Text {
                id: updateLabel
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                wrapMode: Text.WordWrap

                text: qsTr("Updates available")
                font.pixelSize: 20

                color: (updateMouseArea.pressed) ? window.textColor : "#FCFCFC"
            }

            Rectangle {
                id: border4
                height: 1
                width: parent.width
                color: window.lineColor
                anchors.bottom: parent.bottom
            }

            MouseArea {
                id: updateMouseArea
                anchors.fill: parent

                onPressed: tf.play()
                onReleased: tf.play()
                onClicked: personalPage.pageStack.push( Qt.resolvedUrl("UpdatesPage.qml") )
            }
        }

        AppListView {
            id: personalList
            anchors { top: updateItem.bottom; bottom: parent.bottom; left: parent.left; right: parent.right; }

            listId: AppCatalog.PersonalList
            listModel: appCatalog.personalList

            onAtYEndChanged: {
                if (atYEnd && personalList.count > 0 && personalList.count%25 == 0 && !personalList.currentlyLoading) {
                    personalList.currentlyLoading = true
                    appCatalog.loadPersonalList(personalList.count, false)
                }
            }

            allowPullToRefresh: false
        }
    }

    Connections {
        target: appCatalog
        onPersonalListChanged: {
            console.debug("personalListChanged QML")
            personalList.stopLoading()
        }
    }

    ThemeEffect {
        id: tf
        effect: ThemeEffect.BasicButton;
    }
}
