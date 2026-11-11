import QtQuick 1.1
import com.nokia.symbian 1.1
import com.nokia.extras 1.1
import QtMobility.feedback 1.1
import Wunderland 1.0

Page {
    id: detailPage

    tools: toolBarLayout

    // The app, as AppListModel.get() returns it
    property variant detailItem: -1

    // Download size for display ("1.5 MB", "300 KB"), "" if unknown
    property string sizeText: detailItem["version"] !== "" ? formatSize(parseInt(detailItem["downloadsize"])) : ""

    Rectangle {
        id: background
        color: window.bgColor
        anchors.fill: parent
    }

    Flickable {
        id: flickArea
        height: parent.height
        width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        contentWidth: parent.width
        contentHeight: appLinks.y + appLinks.height
        flickableDirection: Flickable.VerticalFlick
        clip: true

        Label {
            id: appName
            text: detailItem["name"]
            width: parent.width - 20
            wrapMode: TextEdit.Wrap
            font.pixelSize: 30
            color: window.textColor
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.left: parent.left
            anchors.leftMargin: 10
        }

        Rectangle {
            id: border
            height: 1
            width: parent.width
            color: window.lineColor
            anchors.top: appName.bottom
            anchors.topMargin: 10
        }

        Image {
            id: appIcon
            height: 80
            width: 80
            asynchronous: true
            source: detailItem["icon"]
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.top: border.bottom
            anchors.topMargin: 10
        }

        Item {
            height: appDeveloper.height + appCategory.height + appVersion.height
            anchors.left: appIcon.right
            anchors.leftMargin: 5
            anchors.verticalCenter: appIcon.verticalCenter

            Label {
                id: appDeveloper
                text: detailItem["developer"]
                wrapMode: TextEdit.Wrap
                font.pixelSize: 20
                color: window.subtleColor
                anchors.top: parent.top
            }

            Label {
                id: appCategory
                text: l.categoryName(detailItem["category"])
                wrapMode: TextEdit.Wrap
                font.pixelSize: 20
                color: window.subtleColor
                anchors.top: appDeveloper.bottom
                anchors.topMargin: 0
            }

            Label {
                id: appVersion
                text: detailItem["version"] + (detailPage.sizeText !== "" ? " - " + detailPage.sizeText : "")
                wrapMode: TextEdit.Wrap
                font.pixelSize: 20
                color: window.subtleColor
                anchors.top: appCategory.bottom
                anchors.topMargin: 0
            }
        }

        Label {
            id: appPrice
            text: (detailItem["price"] > 0) ? (detailItem["price"] + " €") : qsTr("Free")
            font.pixelSize: 24
            color: window.textColor
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: appButton.verticalCenter
        }

        Item {
            id: downloadProgress
            anchors.top: appIcon.bottom
            anchors.topMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            width: parent.width - 20
            visible: appButton.buttonAction == InstallManager.Downloading
            height: cancelButton.height + progressBar.height + 10

            Button {
                id: cancelButton
                text: qsTr("Cancel")
                anchors.top: parent.top
                anchors.right: parent.right
                width: (detailPage.width/5) * 2
                onClicked: {
                    if (installManager.cancelDownload()) {
                        downloadProgress.visible = false;
                        infoBanner.text = qsTr("Download cancelled.")
                        infoBanner.open()
                    } else {
                        infoBanner.text = qsTr("Download could not be cancelled.")
                        infoBanner.open()
                    }
                }
            }

            ProgressBar {
                id: progressBar
                width: parent.width
                anchors.top: cancelButton.bottom
                anchors.topMargin: 10
                anchors.right: parent.right
                value: installManager.progress
                maximumValue: 100.0
            }
        }

        Button {
            id: appButton
            property int buttonAction: installManager.buttonAction(detailItem)
            text: buttonText(buttonAction)
            anchors.top: appIcon.bottom
            anchors.topMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            width: (detailPage.width/5) * 2
            visible: !downloadProgress.visible
            onClicked: {
                if (buttonAction == InstallManager.Launch) installManager.buttonPressed(detailItem, InstallManager.Launch)
                // Download/update; buttonPressed returns if displaying downloadProgress
                else downloadProgress.visible = installManager.buttonPressed(detailItem, buttonAction)
            }
        }

        Rectangle {
            id: border2
            height: 1
            width: parent.width
            color: window.lineColor
            anchors.top: downloadProgress.visible ? downloadProgress.bottom : appButton.bottom
            anchors.topMargin: 10
        }

        Item {
            id: appDescription
            width: parent.width
            height: (appDescriptionLabel.paintedHeight>(detailPage.height/3)) ? (detailPage.height/3) : appDescriptionLabel.height
            anchors.top: border2.bottom
            anchors.topMargin: 10

            property bool opened: !(appDescriptionLabel.paintedHeight>(detailPage.height/3))

            Text {
                id: appDescriptionLabel
                text: ((appButton.buttonAction == InstallManager.Update) && (detailItem["changelog"] !== "")) ? detailItem["changelog"] + "\n\n" + detailItem["description"] : detailItem["description"]
                width: parent.width - 20
                wrapMode: TextEdit.Wrap
                font.pixelSize: 18
                color: window.textColor
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: 10
            }

            Rectangle {
                id: appDescriptionRect
                width: parent.width
                height: parent.height/3

                anchors.bottom: parent.bottom
                anchors.bottomMargin: -10
                visible: !appDescription.opened

                gradient: Gradient {
                    GradientStop { position: 0.0; color: window.bgTransparentColor }
                    GradientStop { position: 1.0; color: window.bgColor }
                }

                MouseArea {
                    id: appDescriptionMouseArea
                    anchors.fill: parent

                    onPressed: if (!appDescription.opened || appDescriptionLabel.paintedHeight>(detailPage.height/3)) tf.play()
                    onReleased: if (!appDescription.opened || appDescriptionLabel.paintedHeight>(detailPage.height/3)) tf.play()

                    onClicked: {
                        if (!appDescription.opened) {
                            // Open label content
                            appDescription.height = appDescriptionLabel.paintedHeight
                            appDescription.opened = true
                        } else if (appDescriptionLabel.paintedHeight>(detailPage.height/3)) {
                            appDescription.height = (detailPage.height/3)
                            appDescription.opened = false
                        }
                    }
                }
            }
        }

        Rectangle {
            id: appDescriptionLayer
            visible: !appDescription.opened
            width: parent.width
            height: appDescriptionLabel.paintedHeight
            color: window.bgColor
            anchors.top: appDescription.bottom
            anchors.topMargin: 10
        }

        Rectangle {
            id: border3
            height: 1
            width: parent.width
            color: window.lineColor
            anchors.top: appDescription.bottom
            anchors.topMargin: 10
        }

        Rectangle {
            id: contactsHeader
            width: parent.width
            height: contactsText.height + 11
            anchors.top: border3.bottom
            color: window.bgColor

            Text {
                id: contactsText
                text: qsTr("Contact the developer")
                width: parent.width - 20
                wrapMode: TextEdit.Wrap
                font.pixelSize: 18
                color: window.textColor
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 10
            }

            Rectangle {
                id: border8
                height: 1
                width: parent.width
                color: window.lineColor
                anchors.bottom: parent.bottom
            }
        }

        Rectangle {
            id: appContactLinks
            width: parent.width
            height: parent.width/4
            anchors.top: contactsHeader.bottom
            color: window.bgColor

            Row {
                anchors.fill: parent

                ContactButton {
                    width: parent.width/4
                    height: parent.height
                    source: "images/twitter.png"
                    active: detailItem["twitter"] !== ""
                    onClicked: Qt.openUrlExternally("https://twitter.com/" + detailItem["twitter"])
                }

                ContactButton {
                    width: parent.width/4
                    height: parent.height
                    source: "images/facebook.png"
                    active: detailItem["facebook"] !== ""
                    onClicked: Qt.openUrlExternally(detailItem["facebook"])
                }

                ContactButton {
                    width: parent.width/4
                    height: parent.height
                    source: "images/mail.png"
                    active: detailItem["mail"] !== ""
                    onClicked: Qt.openUrlExternally("mailto:" + detailItem["mail"] + "?subject=" + detailItem["name"] + " on Wunderland")
                }

                ContactButton {
                    width: parent.width/4
                    height: parent.height
                    source: "images/web.png"
                    active: detailItem["website"] !== ""
                    onClicked: Qt.openUrlExternally(detailItem["website"])
                }
            }
        }

        Rectangle {
            id: border4
            height: 1
            width: parent.width
            color: window.lineColor
            anchors.top: appContactLinks.bottom
        }

        Rectangle {
            id: appLinks
            width: parent.width
            height: appLinkImages.height + appLinkDonate.height + appLinkAAS.height
            anchors.top: border4.bottom
            color: window.bgColor

            AppLink {
                id: appLinkImages
                width: parent.width
                text: qsTr("Screenshots")
                show: (detailItem["image1"] !== "" || detailItem["image2"] !== "" || detailItem["image3"] !== "" || detailItem["image4"] !== "" || detailItem["image5"] !== "")
                visible: show
                anchors.top: parent.top
                onClicked: detailPage.pageStack.push(Qt.resolvedUrl("ImagePage.qml"), {image1: detailItem["image1"], image2: detailItem["image2"], image3: detailItem["image3"], image4: detailItem["image4"], image5: detailItem["image5"]})
            }

            Rectangle {
                id: border5
                height: 1
                width: parent.width
                color: window.lineColor
                anchors.top: appLinkImages.bottom
                visible: appLinkImages.visible
            }

            AppLink {
                id: appLinkDonate
                width: parent.width
                text: qsTr("Donate")
                show: (detailItem["donation"] !== "")
                visible: show
                anchors.top: border5.bottom
                onClicked: symbianFunctions.openUrlInDefaultBrowser(detailItem["donation"])
            }

            Rectangle {
                id: border6
                height: 1
                width: parent.width
                color: window.lineColor
                anchors.top: appLinkDonate.bottom
                visible: appLinkDonate.visible
            }

            AppLink {
                id: appLinkAAS
                width: parent.width
                text: qsTr("Read on All About Symbian")
                show: (detailItem["aas"] !== "")
                visible: show
                anchors.top: border6.bottom
                onClicked: symbianFunctions.openUrlInDefaultBrowser(detailItem["aas"])
            }

            Rectangle {
                id: border7
                height: 1
                width: parent.width
                color: window.lineColor
                anchors.top: appLinkAAS.bottom
                visible: appLinkAAS.visible
            }
        }
    }

    ScrollDecorator {
        flickableItem: flickArea
    }

    InfoBanner {
        id: infoBanner
        text: ""
    }

    Connections {
        target: installManager
        onInstallServerFinished: {
            if (statusCode === 0) {
                // Successful installation
                updateButton()
            } else {
                infoBanner.text = qsTr("Installation failed or was aborted.")
                infoBanner.open()
                updateButton()
            }
        }
        onDownloadError: {
            if (code == 0) {
                infoBanner.text = qsTr("A connection error appeared while trying to start the download.")
                infoBanner.open()
                updateButton()
            } else if (code == 1) {
                infoBanner.text = qsTr("There is not enough space available on your device to download this app. At least %1 is required.").arg(detailPage.sizeText)
                infoBanner.open()
                updateButton()
            } else {
                infoBanner.text = qsTr("An unknown error appeared while trying to start the download.")
                infoBanner.open()
                updateButton()
            }
        }
    }

    function buttonText(action) {
        if (action == InstallManager.Download) return qsTr("Download")
        if (action == InstallManager.Update) return qsTr("Update")
        return qsTr("Launch")
    }

    function formatSize(bytes) {
        if (!(bytes > 0)) return ""
        if (bytes > 1000000) return (bytes / 1000000.0).toFixed(1) + " MB"
        return (bytes / 1000.0).toFixed() + " KB"
    }

    function updateButton() {
        appButton.buttonAction = installManager.buttonAction(detailItem)
        appButton.text = buttonText(appButton.buttonAction)
        downloadProgress.visible = false
    }

    ThemeEffect {
        id: tf
        effect: ThemeEffect.BasicButton
    }
}
