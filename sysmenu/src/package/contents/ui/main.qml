import QtQuick 2.15
import QtQuick.Layouts 1.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents


Item {
    id: root

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.fullRepresentation: Item {
        Layout.minimumWidth: 20 * devicePixelRatio

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
            }

            PlasmaComponents.MenuItem {
                id: storeItem
                text: "App Store"
                enabled: true
            }

            PlasmaComponents.MenuItem {
                id: spacerItem2
                separator: true
                enabled: true
            }

            PlasmaComponents.MenuItem {
                id: recentItem
                text: "Recent Items       >"
                enabled: true
            }

            PlasmaComponents.MenuItem {
                id: spacerItem3
                separator: true
                enabled: true
            }

            PlasmaComponents.MenuItem {
                id: forceQuitItem
                text: "Force Quit"
                enabled: true
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
            }

            PlasmaComponents.MenuItem {
                id: restartItem
                text: "Restart..."
                enabled: true
            }

            PlasmaComponents.MenuItem {
                id: shutDownItem
                text: "Shut Down..."
                enabled: true
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
            }

            PlasmaComponents.MenuItem {
                id: logoutItem
                text: "Log out"
                enabled: true
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

