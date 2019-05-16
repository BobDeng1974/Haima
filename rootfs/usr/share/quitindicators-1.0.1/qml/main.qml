import QtQuick 2.0
import "QUItIndicators"

Image {
    id: root

    property string loadedDemo: ""

    width: 1280
    height: 720
    source: "images/background.jpg"

    onLoadedDemoChanged: {
        if (loadedDemo == "" || loadedDemo == "Wall" || loadedDemo == "Huge") {
            contentArea.interactive = false;
        } else {
            contentArea.interactive = true;
        }

        if (loadedDemo == "") {
            loader.source = "";
            descriptionText.text = "";
        } else {
            loader.source = "examples/" + loadedDemo + ".qml";
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 15
        text: "QUIt Indicators"
        font.pixelSize: 32
        color: "#ffffff"
    }
    Text {
        id: descriptionText
        anchors.horizontalCenter: parent.horizontalCenter
        y: 55
        font.pixelSize: 16
        color: "#b0b0b0"
    }

    Image {
        width: parent.width - 180
        x: 90
        y: 85
        z: 10
        source: "images/small_dot.png"
        fillMode: Image.TileHorizontally
        opacity: 0.1
    }

    ListModel {
        id: examplesModel
        ListElement {
            name: "Gallery"
            title: "Indicators gallery"
        }
        ListElement {
            name: "Huge"
            title: "Huge ProgressIndicator"
        }
        ListElement {
            name: "BusyStressTest"
            title: "BusyIndicator - Stress test"
        }
        ListElement {
            name: "ProgressStressTest"
            title: "ProgressIndicator - Stress test"
        }
        ListElement {
            name: "Wall"
            title: "Picture Wall demo"
        }
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 140
        width: 320
        spacing: 16
        opacity: 1.0 - contentArea.opacity
        Repeater {
            model: examplesModel
            Button {
                text: model.title
                onClicked: {
                    loadedDemo = model.name
                    descriptionText.text = model.title
                }
            }
        }
    }

    Flickable {
        id: contentArea
        anchors.fill: parent
        anchors.topMargin: 100
        opacity: loadedDemo != ""
        contentHeight: loader.height
        visible: loadedDemo != ""
        clip: true
        Behavior on opacity {
            NumberAnimation {
                duration: 500
                easing.type: Easing.InOutQuad
            }
        }

        Loader {
            id: loader
            width: parent.width - 128
            x: 64
        }
    }

    Image {
        source: "images/back.png"
        opacity: loadedDemo != ""
        Behavior on opacity {
            NumberAnimation {
                duration: 400
            }
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                loadedDemo = "";
            }
        }
    }
}
