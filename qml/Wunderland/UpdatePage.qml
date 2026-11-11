import QtQuick 1.1
import com.nokia.symbian 1.1
import com.nokia.extras 1.1

Page {
    id: updatePage

    onStatusChanged: {
        if (status == PageStatus.Activating) {
            update.loadUpdateData()
            window.showToolbar = false
        }
    }

    Rectangle {
        id: background
        color: window.bgColor
        anchors.fill: parent
    }

    Rectangle {
        id: updateHeader
        width: parent.width
        height: updateHeaderText.height + 11
        color: window.bgColor

        Text {
            id: updateHeaderText
            text: qsTr("Update")
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

    Flickable {
        id: flickable
        width: parent.width - 20
        anchors.left: parent.left
        anchors.leftMargin: 10
        contentWidth: parent.width - 20
        contentHeight: updateItem.height
        anchors.top: updateHeader.bottom
        anchors.bottom: parent.bottom
        flickableDirection: Flickable.VerticalFlick
        clip: true
        interactive: contentHeight > parent.height ? true : false

        Item {
            id: updateItem
            width: parent.width
            height: text.height + download.height + cancel.height + 60
            anchors.top: parent.top
            anchors.topMargin: (updateItem.height <= updatePage.height) ? (updatePage.height-updateItem.height)/2 : 0

            TextEdit {
                id: text
                readOnly: true
                wrapMode: TextEdit.Wrap
                width: (parent.width/6)*5
                anchors.top: parent.top
                anchors.topMargin: 15
                anchors.horizontalCenter: parent.horizontalCenter
                color: window.textColor

                text: ""
            }

            Label {
                id: downloadProgress
                text: "Downloading (0%)"
                font.pixelSize: 22
                color: window.textColor
                visible: false
                anchors.centerIn: download
            }

            Button {
                id: download
                anchors.top: text.bottom
                anchors.topMargin: 15
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width/2

                text: "Download"

                visible: !downloadProgress.visible
                onClicked: {
                    // First: a failed start (no space) hides it again right away
                    downloadProgress.visible = true
                    update.startDownload()
                }
            }

            Button {
                id: cancel
                anchors.top: download.bottom
                anchors.topMargin: 15
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width/2

                text: qsTr("Cancel")

                onClicked: {
                    Qt.quit()
                }
            }
        }
    }

    Connections {
        target: update

        onLoadingDone: {
            if (loadResult) {
                text.text = message
            } else {
                text.text = qsTr("There was an error getting more data about the update. Please check your internet connection.")
            }
        }

        onProgressChanged: downloadProgress.text = "Downloading (" + progress + "%)"
        onDownloadError: {
            // Show the Download button again
            downloadProgress.visible = false
            if (code == 1)
                errorBanner.text = qsTr("There is not enough space available on your device to download the update.")
            else
                errorBanner.text = window.downloadErrorText(code)
            errorBanner.open()
        }
    }

    InfoBanner {
        id: errorBanner
    }

    ToolBarLayout {
        id: toolBarLayoutUpdate
        ToolButton {
            flat: true
            iconSource: "toolbar-back"
            onClicked: Qt.quit()
        }
    }
}
