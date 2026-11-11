import QtQuick 1.1
import com.nokia.symbian 1.1
import QtMobility.feedback 1.1

Page {
    id: categoryPage

    tools: toolBarLayout

    Rectangle {
        anchors.fill: parent
        color: window.bgColor
    }

    Rectangle {
        id: categoryHeader
        width: parent.width
        height: categoryHeaderText.height + 11
        color: window.bgColor

        Text {
            id: categoryHeaderText
            text: qsTr("Categories") + l.emptyString
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

    ListView {
        id: categoryList
        anchors { top: categoryHeader.bottom; bottom: parent.bottom; left: parent.left; right: parent.right; }
        clip: true

        model: l.categories()

        delegate: Item {
            id: delegate
            property bool display: modelData !== ""
            width: parent.width
            height: display ? appTitle.height + 18 : 0
            visible: display

            Rectangle {
                anchors.fill: parent
                color: mouseArea.pressed ? window.highlightColor : (l.isHeadCategory(index)) ? window.lineColor : window.bgColor

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent

                    onPressed: tf.play()
                    onReleased: tf.play()
                    onClicked: {
                        window.pageStack.push (Qt.resolvedUrl("CategoryListPage.qml"), {categoryId : index} )
                    }
                }
            }

            Text {
                id: appTitle
                text: modelData
                width: parent.width - 20
                wrapMode: TextEdit.Wrap
                font.pixelSize: 25
                color: mouseArea.pressed ? window.bgColor : window.textColor
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 10
            }

            Rectangle {
                id: border2
                height: 1
                width: parent.width
                color: window.lineColor
                visible: (index === 0 || (index === (model.count-1))) ? false : true
            }
        }
    }

    ThemeEffect {
        id: tf
        effect: ThemeEffect.BasicButton;
    }
}
