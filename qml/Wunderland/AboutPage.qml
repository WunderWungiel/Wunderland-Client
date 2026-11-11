import QtQuick 1.1
import com.nokia.symbian 1.1
import com.nokia.extras 1.1

Page {
    id: aboutPage
    tools: toolBarLayout

    /*onStatusChanged: {
        if (status == PageStatus.Activating) {
            aboutHeader.headerShown();
        }
    }*/

    onHeightChanged: {
        if (aboutPage.height > aboutPage.width) {
            icon.height = aboutPage.width/4
            icon.width = aboutPage.width/4
        } else {
            icon.height = aboutPage.height/4
            icon.width = aboutPage.height/4
        }
    }

    Rectangle {
        id: bg
        anchors.fill: parent
        color: window.bgColor
    }

    Rectangle {
        id: aboutHeader
        width: parent.width
        height: aboutHeaderText.height + 11
        anchors.top: parent.top
        color: window.bgColor

        Text {
            id: aboutHeaderText
            text: qsTr("About Wunderland")
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
        id: basicInfo
        anchors.top: aboutHeader.bottom
        anchors.left: parent.left
        color: window.lineColor
        width: parent.width
        height: icon.height

        Image {
            id: icon
            height: aboutPage.width/4
            width: aboutPage.width/4
            source: "images/about-icon.svg"
            anchors.top: parent.top
            smooth: true
        }

        TextEdit {
            id: author
            readOnly: true
            wrapMode: TextEdit.Wrap
            width: parent.width - icon.width - 20
            anchors.left: icon.right
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter

            color: window.textColor
            font.pixelSize: 20

            text: "Wunderland " + appVersion + "<br>
Wunder Wungiel<br>
Copyright (C) 2026"
        }
    }

    Rectangle {
        id: changelogItem
        anchors.top: basicInfo.bottom
        height: changelogLabel.height + 34
        color: (changelogMouseArea.pressed) ? window.highlightColor : window.bgColor
        width: parent.width

        Text {
            id: changelogLabel
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("Changelog")
            font.pixelSize: 20

            color: (changelogMouseArea.pressed) ? window.bgColor : window.textColor
        }

        MouseArea {
            id: changelogMouseArea
            anchors.fill: parent

            onPressed: tf.play()
            onReleased: tf.play()
            onClicked: {
                if (update.changelog == "") {
                    loading.start()
                    update.loadUpdateData(true)
                } else {
                    changelogDialog.open()
                }
            }
        }
    }

    Rectangle {
        id: border7
        height: 1
        width: parent.width
        color: window.lineColor
        anchors.top: changelogItem.bottom
    }

    Flickable {
        id: flickArea
        height: parent.height - aboutHeader.height - basicInfo.height - changelogItem.height
        width: parent.width - 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: changelogItem.bottom
        contentWidth: aboutText.width
        contentHeight: aboutText.height + 10
        flickableDirection: Flickable.VerticalFlick
        clip: true

        TextEdit {
            id: aboutText
            readOnly: true
            wrapMode: TextEdit.Wrap
            width: flickArea.width
            anchors.top: parent.top
            anchors.topMargin: 5

            color: window.textColor
            font.pixelSize: 19

            text: "Web: http://wunderwungiel.pl<br><br>
Based on AppList by Sebastian (schumi1331). Thank you!<br><br>
- Anand Bibek: Pull to refresh<br>
- Bhavin: Beta testing<br>
- Fabian (@huellif): Symbian C++ and beta testing<br>
- Kush: Beta testing<br>
- István: Beta testing- Kush: Beta testing<br>
- Lily: Beta testing<br>
- Lyubomir (@batezippi): Beta testing<br>
- Magissia: Hosting<br>
- Microsoft: Idea how a good Store web UI may look like<br>
- Moritz (@MoritzJT): Beta testing<br>
- Nokia: The Symbian platform till now and my 808 PureView<br>
- Oleg (@aquarius_rus): SISContents<br>
- pisarz1958 and Motaz: Homescreen widget tutorial and dll improvements<br>
- Simone Grandi: Belle icons from his theme \"Evolve+ FP2 Edition\"<br>
- Slava; Ilya and Anton: SVGB decoder and the Qt port SVGB2SVG<br>
- Stephen Elop: Killing a beautiful software and operating system<br>
- Steve Litchfield: His curated App Store, feedback, promoting and beta testing<br>
- All these guys in the Qt forums, stackoverflow and the web<br>
- All developers and companies who developed or are developing for Symbian<br>
- You for reading this and using Wunderland"
        }
    }

    ScrollDecorator {
        flickableItem: flickArea
    }

    Loading {
        id: loading
        anchors.fill: parent
        text: qsTr("Loading") + "..."
    }

    InfoBanner {
        id: banner
        text: qsTr("Failed to load changelog history.")
    }

    Connections {
        target: update
        onChangelogChanged: {
            loading.stop()
            if (update.changelog !== "") changelogDialog.open()
            else banner.open()
        }
    }

    DialogComp {
        id: changelogDialog
        text: update.changelog
        buttonTexts: ["Close"]
        titleText: "Changelog"
        displayCheckBox: false
    }
}
