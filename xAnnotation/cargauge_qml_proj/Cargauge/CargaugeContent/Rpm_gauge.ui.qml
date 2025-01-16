

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls

import QtQuick.Studio.Components 1.0
import QtQuick.Studio.DesignEffects

Item {
    id: rpm_gauge
    width: 600
    height: 600

    property real percentage: 15.21

    property real startAngle: 120 // 90+30
    property real endAngle: 420 // 360+ (90-30)
    property real numOfDot: 11

    property color outerCicleColor: "#414546"
    property color innerCicleColor: "#5ca8ba"
    property int outerCircleWidth: 60
    property int innerCircleWidth: 40
    property real dotRadius: 242

    Repeater {
        opacity: 0.4
        model: numOfDot

        // angleSpace= (endAngle-startAngle)//*numOfDot*/
        CustomDot {
            x: parent.width / 2 + dotRadius
               * Math.cos(startAngle * Math.PI / 180
                          + index * (endAngle - startAngle) * (Math.PI / 180) / (numOfDot - 1))
            y: parent.height / 2 + dotRadius
               * Math.sin(startAngle * Math.PI / 180
                          + index * (endAngle - startAngle) * (Math.PI / 180) / (numOfDot - 1))

            // radius: 350
            color: "white"
        }
    }

    ArcItem {
        property real circleRadius: 257

        id: thin_circle
        x: (parent.width - 2 * circleRadius) / 2 + 4
        y: (parent.height - 2 * circleRadius) / 2 + 3
        width: 2 * circleRadius
        height: 2 * circleRadius
        rotation: 90
        strokeColor: "#ffffff"
        fillColor: "#00ffffff"
        strokeWidth: 4
        begin: 120 + 0
        end: 420 - 0
    }

    ProgressCircle {
        percentage: parent.percentage
        startAngle: -60
        endAngle: 240
        x: (parent.width - width) / 2 + 5
        y: (parent.height - height) / 2 + 5
    }

    Image {
        id: speedometer_needle_ring_195_90
        x: 158
        y: 159
        source: "assets/speedometer_needle_ring_195_90.png"
        rotation: 0
    }
    Image {
        id: centor_dial
        x: 167
        y: 168
        source: "assets/speed_dial_195_151.png"
    }

    Text {
        id: kph_readout_195_117
        x: 276
        y: 327
        color: "#FFFFFF"
        text: "RPM"
        font.weight: Font.ExtraLight
        font.pixelSize: 30
        font.family: "IBM Plex Mono"
    }
    // 转速数字显示
    Text {
        id: kph_number_195_91
        x: 189
        y: 223
        width: 230
        height: 98
        color: "#FFFFFF"
        text: (parent.percentage * 10000 / 100.0).toFixed(0).toString()
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 72
        font.family: "Cherry"
    }
    // 转速指针
    Item {
        id: item22
        x: 204
        y: 205
        width: 200
        height: 200
        z: 1
        rotation: -208 + parent.percentage * (92 + 208) / 100
        // rotation: -208
        Image {
            id: speedometer_needle_195_142
            x: 224
            y: -23
            source: "assets/speedometer_needle_195_142.png"
        }
    }

    // 数字显示
    Text {
        x: 148
        y: 532
        text: "0"
        color: "white"
        font.weight: Font.Thin
        font.pixelSize: 29
        font.family: "Exo 2"
    }

    Text {
        x: 44
        y: 431
        color: "#ffffff"
        text: "1"
        font.pixelSize: 29
        font.weight: Font.Thin
        font.family: "Exo 2"
    }

    Text {
        x: 10
        y: 286
        color: "#ffffff"
        text: "2"
        font.pixelSize: 29
        font.weight: Font.Thin
        font.family: "Exo 2"
    }

    Text {
        x: 42
        y: 132
        color: "#ffffff"
        text: "3"
        font.pixelSize: 29
        font.weight: Font.Thin
        font.family: "Exo 2"
    }

    Text {
        x: 147
        y: 34
        color: "#ffffff"
        text: "4"
        font.pixelSize: 29
        font.weight: Font.Thin
        font.family: "Exo 2"
    }

    Text {
        x: 296
        y: -8
        color: "#ffffff"
        text: "5"
        font.pixelSize: 29
        font.weight: Font.Thin
        font.family: "Exo 2"
    }

    Text {
        x: 446
        y: 32
        color: "#ffffff"
        text: "6"
        font.pixelSize: 29
        font.weight: Font.Thin
        font.family: "Exo 2"
    }

    Text {
        x: 542
        y: 130
        color: "#ffffff"
        text: "7"
        font.pixelSize: 29
        z: 1
        font.weight: Font.Thin
        font.family: "Exo 2"
    }

    Text {
        x: 581
        y: 286
        color: "#ffffff"
        text: "8"
        font.pixelSize: 29
        font.weight: Font.Thin
        font.family: "Exo 2"
    }

    Text {
        x: 542
        y: 435
        color: "#ffffff"
        text: "9"
        font.pixelSize: 29
        font.weight: Font.Thin
        font.family: "Exo 2"
    }
    Text {
        x: 427
        y: 532
        color: "#ffffff"
        text: "10"
        font.pixelSize: 29
        font.weight: Font.Thin
        font.family: "Exo 2"
    }
}
