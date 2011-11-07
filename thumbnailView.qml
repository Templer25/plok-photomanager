import QtQuick 1.1

Item {
    width: 800
    height: 600

    Component {
        id: thumbnailDelegate
        Item {
            width: 150
            height: 130

            Column {
                spacing: 5
                Image {
                    width: 150
                    height: 100
                    source: path
                    anchors.horizontalCenter: parent.horizontalCenter

                    asynchronous: true
                    sourceSize.width: 150
                    sourceSize.height: 100
                }

                Text {
                    text: name
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.bold: true
                }
            }

            MouseArea {
                id: thumbnail_mousearea
                anchors.fill: parent
                onClicked: {
                    thumbnailView.currentIndex = index

                }
            }
        }
    }

    GridView {
        id: thumbnailView
        cacheBuffer: 400

        anchors.fill: parent
        flow: GridView.LeftToRight

        cellHeight: 130
        cellWidth: 160

        maximumFlickVelocity: 2000
        flickDeceleration: 2000

//        delegate: Item {

//            height: 100

//            Column {
//                spacing: 5
//                Image {
//                    width: 150
//                    height: 100
//                    source: path
//                    anchors.horizontalCenter: parent.horizontalCenter

//                    asynchronous: true
//                    sourceSize.width: 150
//                    sourceSize.height: 100
//                }

//                Text {
//                    text: name
//                    anchors.horizontalCenter: parent.horizontalCenter
//                    font.bold: true
//                }
//            }
//        }

        model: thumbnailViewModel
        delegate: thumbnailDelegate

        highlight: Rectangle { color: "lightsteelblue"; radius: 5 }

        onCurrentIndexChanged: mainWindow.currentIndexChanged (currentIndex)
    }

    focus: true
}
