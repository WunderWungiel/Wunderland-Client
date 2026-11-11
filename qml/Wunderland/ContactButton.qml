import QtQuick 1.1
import QtMobility.feedback 1.1

// One square contact button on the detail page (Twitter, Facebook, mail, website).
// Greyed out and inactive when the app has no such link.
Rectangle {
    id: contactButton

    signal clicked()
    property alias source: icon.source
    property bool active: true

    color: (mouseArea.pressed && active) ? window.highlightColor : window.bgColor

    Image {
        id: icon
        anchors.centerIn: parent
        asynchronous: true
        opacity: contactButton.active ? 1 : 0.3
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onPressed: tf.play()
        onReleased: tf.play()
        onClicked: if (contactButton.active) contactButton.clicked()
    }

    ThemeEffect {
        id: tf
        effect: ThemeEffect.BasicButton
    }
}
