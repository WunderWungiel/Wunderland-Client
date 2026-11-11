import QtQuick 1.1
import com.nokia.symbian 1.1

CommonDialog {
    id: updateDialog
    height: content.height

    buttonTexts: ["Ok"]

    content:
    ListView {
        id: listView
        width: parent.width - 20
        anchors.left: parent.left
        anchors.leftMargin: 10
        contentWidth: parent.width - 20
        height: updateDialog.platformContentMaximumHeight
        flickableDirection: Flickable.VerticalFlick
        clip: true
        interactive: true

        model: l.languageCodes

        delegate: Item {
            width: parent.width
            height: checkBox.height
            Text {
                text: l.languageNames[index]
                color: platformStyle.colorNormalLight
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
            }

            CheckBox {
                id: checkBox
                text: ""
                anchors.right: parent.right
                anchors.rightMargin: 10
                platformInverted: true
                checked: l.isSelected(modelData)
                onClicked: {
                    if (checked) l.addLanguage(modelData)
                    else l.removeLanguage(modelData)
                }
            }
        }
    }
}
