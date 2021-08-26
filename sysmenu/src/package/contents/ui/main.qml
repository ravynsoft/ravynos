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
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents


Item {
    id: root

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.fullRepresentation: Item {
        Layout.minimumWidth: 20 * units.devicePixelRatio

	PlasmaCore.DataSource {
            id: pmEngine
            engine: "powermanagement"
            connectedSources: ["PowerDevil", "Sleep States"]

            onSourceAdded: {
                disconnectSource(source);
                connectSource(source);
            }
            onSourceRemoved: {
                disconnectSource(source);
            }

            function performOperation(what) {
                var service = serviceForSource("PowerDevil")
                var operation = service.operationDescription(what)
                service.startOperationCall(operation)
            }
        }

        PlasmaCore.IconItem {
            id: menuIcon
            source: plasmoid.file('','icons/tree.svg')
            width: 16 * units.devicePixelRatio
            height: 16 * units.devicePixelRatio
            anchors.right: parent.right
            smooth: true
        }

        PlasmaComponents.Menu {
            id: menu
            visualParent: root.parent
            placement: PlasmaCore.Types.BottomPosedLeftAlignedPopup

            PlasmaComponents.MenuItem {
                id: aboutItem
                text: "About this computer"
                enabled: true
                onClicked: {
                    plasmoid.nativeInterface.aboutThisComputer()
                }
            }

            PlasmaComponents.MenuItem {
                id: spacerItem1
                separator: true
                enabled: true
            }

            PlasmaComponents.MenuItem {
                id: prefItem
                text: "System Preferences"
                enabled: true
                onClicked: {
                    plasmoid.nativeInterface.systemPreferences()
                }
            }

            PlasmaComponents.MenuItem {
                id: storeItem
                text: "App Store"
                enabled: false
            }

            PlasmaComponents.MenuItem {
                id: spacerItem2
                separator: true
                enabled: true
            }

            PlasmaComponents.MenuItem {
                id: recentItem
                text: "Recent Items         >"
                enabled: false
            }

            PlasmaComponents.MenuItem {
                id: spacerItem3
                separator: true
                enabled: true
            }

            PlasmaComponents.MenuItem {
                id: forceQuitItem
                text: "Force Quit"
                enabled: false
            }

            PlasmaComponents.MenuItem {
                id: spacerItem4
                separator: true
                enabled: true
            }

            PlasmaComponents.MenuItem {
                id: sleepItem
                text: "Sleep"
                enabled: true
                onClicked: plasmoid.nativeInterface.suspend()
            }

            PlasmaComponents.MenuItem {
                id: restartItem
                text: "Restart..."
                enabled: true
                onClicked: plasmoid.nativeInterface.requestLogout(1,1,3)
                // 1 1 3 = reboot with confirmation
            }

            PlasmaComponents.MenuItem {
                id: shutDownItem
                text: "Shut Down..."
                enabled: true
                onClicked: pmEngine.performOperation("requestShutDown")
            }

            PlasmaComponents.MenuItem {
                id: spacerItem5
                separator: true
                enabled: true
            }

            PlasmaComponents.MenuItem {
                id: lockItem
                text: "Lock Screen"
                enabled: true
                onClicked: pmEngine.performOperation("lockScreen")
            }

            PlasmaComponents.MenuItem {
                id: logoutItem
                text: "Log out"
                enabled: true
                onClicked: plasmoid.nativeInterface.requestLogout(1,3,3)
                // 1,3,3 = logout with confirmation. 0,3,3 = logout now
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
    
            onClicked: {
                menu.open(0,20)
            }
        }
    }
}

