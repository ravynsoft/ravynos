/*
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents


Item {
    id: root

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.fullRepresentation: Item {
        Layout.minimumWidth: 32 * units.devicePixelRatio
        Layout.minimumHeight: 24 * units.devicePixelRatio
        anchors.fill: parent
        PlasmaCore.IconItem {
            id: menuIcon
            source: plasmoid.file('','icons/2x.png')
            width: 20 * units.devicePixelRatio
            height: 20 * units.devicePixelRatio
            anchors.centerIn: parent
            smooth: true
        }
        ColorOverlay {
            anchors.fill: menuIcon
            source: menuIcon
            color: "black"
        }
        MouseArea {
            id: mouseArea
            anchors.fill: parent
    
            onClicked: {
                plasmoid.nativeInterface.openMenu(0,20 * units.devicePixelRatio)
            }
        }
    }
}

