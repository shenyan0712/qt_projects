

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
    id: speed_gauge
    width: 658
    height: 600

    property real percentage: 31.21

    Image {
        id: speed_number
        anchors.verticalCenter: parent.verticalCenter
        source: "assets/speedgauge_number.png"
        anchors.horizontalCenter: parent.horizontalCenter
    }

    ProgressCircle {
        x: 96
        y: 98
        percentage: parent.percentage
    }

    Image {
        id: speedometer_needle_ring_195_90
        x: 175
        y: 176
        source: "assets/speedometer_needle_ring_195_90.png"
        rotation: 0
    }
    Image {
        id: centor_dial
        x: 184
        y: 184
        source: "assets/speed_dial_195_151.png"
    }

    //指针
    Item {
        id: item1
        x: 221
        y: 221
        width: 200
        height: 200
        z: 1
        rotation: -209.7 + parent.percentage * (92.4 + 209.7) / 100
        // rotation: 92.4
        Image {
            id: speedometer_needle_195_142
            x: 224
            y: -23
            source: "assets/speedometer_needle_195_142.png"
        }
    }

    Text {
        id: kph_readout_195_117
        x: 293
        y: 333
        color: "#FFFFFF"
        text: "KPH"
        font.weight: Font.ExtraLight
        font.pixelSize: 30
        font.family: "IBM Plex Mono"
    }
    // 速度数字显示
    Text {
        id: kph_number_195_91
        x: 206
        y: 241
        width: 230
        height: 98
        color: "#FFFFFF"
        text: (parent.percentage * 200.0 / 100.0).toFixed(1).toString()
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 72
        font.family: "Cherry"
    }
}
