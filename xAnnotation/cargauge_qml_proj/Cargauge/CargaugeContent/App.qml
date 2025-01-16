// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Timeline 1.0

import Cargauge

Item {
    id: root
    width: 700
    height: 300

    property int rpm_percent: 10
    property int speed_percent: 14

    visible: true
    // title: "Cargauge"

    Rectangle {
        id: rectangle
        width: parent.width
        height: parent.height
        color: "#241f1f"

        Speed_gauge {
            id: speed_gauge
            x: 207
            y: -150
            scale: 0.5
            percentage: root.speed_percent

            SequentialAnimation {
                id: anims
                running: true
                loops: -1
                NumberAnimation {
                    target: speed_gauge
                    property: "percentage"
                    running: true
                    from: 0
                    to: 100
                    duration: 2000
                }
                NumberAnimation {
                    target: speed_gauge
                    property: "percentage"
                    running: true
                    from: 100
                    to: 0
                    duration: 2000
                }
            }

        }

        Rpm_gauge {
            id: rpm_gauge
            scale: 0.5
            x: -150
            y: -140

            percentage: root.rpm_percent

            SequentialAnimation {
                id: anims_rpm
                running: true
                loops: -1
                NumberAnimation {
                    target: rpm_gauge
                    property: "percentage"
                    running: true
                    from: 0
                    to: 25.50
                    duration: 2000
                }
                NumberAnimation {
                    target: rpm_gauge
                    property: "percentage"
                    running: true
                    from: 25.50
                    to: 0
                    duration: 2000
                }
            }

        }
    }

}

