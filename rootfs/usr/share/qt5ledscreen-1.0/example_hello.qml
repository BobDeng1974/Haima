import QtQuick 2.0
import "ledscreencomponent"

Rectangle {
    width: sourceArea.width * ledScreen.ledSize
    height: 180
    color: "#000000"

    Item {
        id: sourceArea
        width: 44
        height: 10
        Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 11
            font.family: "verdana"
            font.bold: true
            smooth: false
            text: "HELLO!"
        }
    }

    LedScreen {
        id: ledScreen
        anchors.fill: parent
        sourceItem: sourceArea
        ledSize: 18
        ledColor: "#ff8800"
    }
}
