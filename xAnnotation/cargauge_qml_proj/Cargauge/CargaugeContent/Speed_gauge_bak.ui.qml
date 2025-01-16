

/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Design Studio.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
import QtQuick 2.8
import QtQuick.Timeline 1.0
import QtQuick.Studio.Components 1.0
import QtQuick.Studio.DesignEffects

Item {
    id: speed_dial_195_151
    width: 650
    height: 600
    opacity: 1
    visible: true
    rotation: 0
    z: 1
    property alias kphDisplay: kph_number_195_91.text
    property alias kplDisplay: kpl_number_195_93.text
    property alias kphFrame: kphTimeline.currentFrame
    x: 0

    Rectangle {
        color: "#000000"
        anchors.fill: parent
        anchors.leftMargin: -27
        anchors.rightMargin: 27
        anchors.topMargin: -39
        anchors.bottomMargin: 0

    }

    // background: black
    Image {
        id: speed_dial_195_151Asset
        x: 161
        y: 153
        visible: true
        source: "assets/speed_dial_195_151.png"
    }

    Item {
        id: speed_numbers_195_116
        x: 0
        y: 0
        anchors.fill: parent
        Text {
            id: kph_195_95
            x: 151
            y: 523
            color: "#FFFFFF"
            text: "0"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_96
            x: 81
            y: 484
            color: "#FFFFFF"
            text: "10"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_97
            x: 29
            y: 421
            color: "#FFFFFF"
            text: "20"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_98
            x: 2
            y: 357
            color: "#FFFFFF"
            text: "30"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_99
            x: -11
            y: 280
            color: "#FFFFFF"
            text: "40"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_100
            x: -2
            y: 204
            color: "#FFFFFF"
            text: "50"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_101
            x: 16
            y: 135
            color: "#CCCCCC"
            text: "60"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_102
            x: 66
            y: 70
            color: "#FFFFFF"
            text: "70"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_103
            x: 125
            y: 15
            color: "#FFFFFF"
            text: "80"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_104
            x: 197
            y: -14
            color: "#FFFFFF"
            text: "90"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_105
            x: 274
            y: -23
            color: "#FFFFFF"
            text: "100"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_106
            x: 364
            y: -11
            color: "#FFFFFF"
            text: "110"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_107
            x: 435
            y: 13
            color: "#FFFFFF"
            text: "120"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_108
            x: 495
            y: 62
            color: "#FFFFFF"
            text: "130"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_109
            x: 539
            y: 127
            color: "#FFFFFF"
            text: "140"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_110
            x: 562
            y: 196
            color: "#FFFFFF"
            text: "150"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_111
            x: 575
            y: 269
            color: "#FFFFFF"
            text: "160"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_112
            x: 569
            y: 352
            color: "#FFFFFF"
            text: "170"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_113
            x: 538
            y: 424
            color: "#FFFFFF"
            text: "180"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_114
            x: 494
            y: 485
            color: "#FFFFFF"
            text: "190"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }

        Text {
            id: kph_195_115
            x: 426
            y: 524
            color: "#FFFFFF"
            text: "200"
            font.weight: Font.Thin
            font.pixelSize: 29
            font.family: "Exo 2"
        }
    }

    Item {
        id: speedometer_dots_195_141
        x: 54
        y: 45
        CustomDot {
            id: dot_0_195_140
            x: 127
            y: 457
        }

        CustomDot {
            id: dot_10_195_125
            x: 73
            y: 422
        }

        CustomDot {
            id: dot_20_195_129
            x: 31
            y: 366
        }

        CustomDot {
            id: dot_30_195_132
            x: 8
            y: 310
        }

        CustomDot {
            id: dot_40_195_121
            x: -2
            y: 248
        }

        CustomDot {
            id: dot_50_195_135
            x: 6
            y: 182
        }

        CustomDot {
            id: dot_60_195_138
            x: 28
            y: 123
        }

        CustomDot {
            id: dot_70_195_123
            x: 68
            y: 72
        }

        CustomDot {
            id: dot_80_195_127
            x: 117
            y: 32
        }

        CustomDot {
            id: dot_90_195_131
            x: 176
            y: 8
        }

        CustomDot {
            id: dot_100_195_120
            x: 238
            y: 0
        }

        CustomDot {
            id: dot_110_195_134
            x: 303
            y: 8
        }

        CustomDot {
            id: dot_120_195_139
            x: 361
            y: 32
        }

        CustomDot {
            id: dot_130_195_126
            x: 412
            y: 70
        }

        CustomDot {
            id: dot_140_195_130
            x: 450
            y: 120
        }

        CustomDot {
            id: dot_150_195_133
            x: 474
            y: 178
        }

        CustomDot {
            id: dot_160_195_122
            x: 482
            y: 242
        }

        CustomDot {
            id: dot_170_195_136
            x: 475
            y: 307
        }

        CustomDot {
            id: dot_180_195_137
            x: 451
            y: 363
        }

        CustomDot {
            id: dot_190_195_124
            x: 412
            y: 415
        }

        CustomDot {
            id: dot_200_195_128
            x: 361
            y: 454
        }
    }

    Image {
        id: sppedometer_outer_ring_195_86
        x: 41
        y: 34
        source: "assets/sppedometer_outer_ring_195_86.png"
    }

    Image {
        id: speedometer_track_bg_195_87
        x: 69
        y: 61
        source: "assets/speedometer_track_bg_195_87.png"
    }

    Image {
        id: speedometer_needle_ring_195_90
        x: 153
        y: 145
        source: "assets/speedometer_needle_ring_195_90.png"
    }

    Text {
        id: kpl_number_195_93
        x: 192
        y: 315
        width: 219
        height: 57
        color: "#FFFFFF"
        text: "15.5"
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 42
        font.family: "Cherry"
    }

    Text {
        id: kpl_readout_195_118
        x: 277
        y: 371
        color: "#FFFFFF"
        text: "KPL"
        font.weight: Font.ExtraLight
        font.pixelSize: 32
        font.family: "IBM Plex Mono"
    }

    Text {
        id: kph_number_195_91
        x: 187
        y: 201
        width: 230
        height: 98
        color: "#FFFFFF"
        text: "140"
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 72
        font.family: "Cherry"
    }

    Text {
        id: kph_readout_195_117
        x: 277
        y: 173
        color: "#FFFFFF"
        text: "KPH"
        font.weight: Font.ExtraLight
        font.pixelSize: 30
        font.family: "IBM Plex Mono"
    }

    ArcItem {
        id: arc
        x: 76
        y: 71
        width: 445
        height: 442
        fillColor: "#00ffffff"
        rotation: -90
        strokeColor: "#5ca8ba"
        end: 200
        begin: -61.3
        strokeWidth: 45
    }

    Item {
        id: item1
        x: 200
        y: 191
        width: 200
        height: 200

        Image {
            id: speedometer_needle_195_142
            x: 224
            y: -23
            source: "assets/speedometer_needle_195_142.png"
        }
    }
}
