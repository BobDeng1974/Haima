import QtQuick 2.0

Item {
    id: root

    property alias text: textItem.text
    signal clicked

    width: 320
    height: 64

    Rectangle {
        anchors.fill: parent
        border.color: "#000000"
        border.width: 2
        color: mouseArea.pressed && mouseArea.containsMouse ? "#ffffff" : "#d0d0d0"
    }

    Image {
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 8
        source: "images/back.png"
        rotation: 180
        width: 48
        height: 48
    }

    Text {
        id: textItem
        color: "#000000"
        font.pixelSize: 16
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: -24
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: root.clicked();
    }
}
