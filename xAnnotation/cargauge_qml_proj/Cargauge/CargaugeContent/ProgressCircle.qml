import QtQuick
import QtQuick.Controls

import QtQuick.Studio.Components 1.0
import QtQuick.Studio.DesignEffects


Item {
    id: root
    width: 450
    height: 450

    property color outerCicleColor: "#414546"
    property color innerCicleColor: "#5ca8ba"
    property int outerCircleWidth: 60
    property int innerCircleWidth: 40
    property real startAngle: -61.3
    property real endAngle: 240
    property real percentage:20.3     // [0.0-100.00]

    ArcItem {
        id: outer_arc
        width: parent.width
        height: parent.height
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        rotation: -90
        z: 1
        fillColor: "#00ffffff"
        strokeColor: outerCicleColor
        end: endAngle
        begin: startAngle
        strokeWidth: outerCircleWidth
    }

    ArcItem {
        id: inner_arc
        width: parent.width- (outerCircleWidth-innerCircleWidth)
        height: parent.height-(outerCircleWidth-innerCircleWidth)
        anchors.verticalCenter: parent.verticalCenter
        strokeColor: "#218e80"
        anchors.horizontalCenter: parent.horizontalCenter
        z: 1
        fillColor: "#00ffffff"
        rotation: -90
        end: startAngle+ percentage*(endAngle- startAngle)/100.0
        begin: startAngle
        strokeWidth: innerCircleWidth
    }

}
