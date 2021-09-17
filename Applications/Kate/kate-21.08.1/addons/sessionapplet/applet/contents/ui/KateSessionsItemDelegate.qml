/********************************************************************
This file is part of the KDE project.

SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>

Based on the clipboard applet:
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.plasma.core 2.0 as PlasmaCore


PlasmaComponents.ListItem {
    id: menuItem

    signal itemSelected(string uuid)
    signal newSession(string sessionname)
    signal remove(string uuid)
    
    property bool showInput: false

    height: Math.max(Math.max(label.height, toolButtonsLayout.implicitHeight), sessionnameditlayout.implicitHeight) + 2 * PlasmaCore.Units.smallSpacing

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            if (TypeRole!=1)
                menuItem.itemSelected(UuidRole);
            else {
                showInput=true; 
            }
        }
        onEntered: menuListView.currentIndex = index
        onExited: menuListView.currentIndex = -1

        Item {
            id: label
            height: childrenRect.height
            anchors {
                left: parent.left
                leftMargin: PlasmaCore.Units.smallSpacing
                right: parent.right
                verticalCenter: parent.verticalCenter
            }

            PlasmaComponents3.Label {
                height: implicitHeight
                anchors {
                    left: parent.left
                    right: parent.right
                    rightMargin: PlasmaCore.Units.gridUnit * 2
                    leftMargin: PlasmaCore.Units.iconSizes.small + PlasmaCore.Units.smallSpacing * 2
                    verticalCenter: parent.verticalCenter
                }
                maximumLineCount: 1
                text: DisplayRole.trim()
                visible: !showInput // TypeRole: 0: Text, 1: Image, 2: Url
                elide: Text.ElideRight
                wrapMode: Text.Wrap
            }

            PlasmaCore.IconItem {
                width: PlasmaCore.Units.iconSizes.small
                height: width
                anchors.verticalCenter: parent.verticalCenter
                source: DecorationRole
                enabled: true
                visible: true
            }
        }

        RowLayout {
                id:sessionnameditlayout
                visible:showInput
                height: implicitHeight
                anchors {
                    left: parent.left
                    right: parent.right
                    rightMargin: 0
                    leftMargin: PlasmaCore.Units.iconSizes.small + PlasmaCore.Units.smallSpacing * 2
                    verticalCenter: parent.verticalCenter
                }

                PlasmaComponents3.TextField {
                    id: sessionname
                    placeholderText: i18n("Session name")
                    clearButtonShown: true
                    Layout.fillWidth: true
                }

                PlasmaComponents3.ToolButton {
                    icon.name: "dialog-ok"
                    enabled: sessionname.text.replace(/^\s+|\s+$/g, '').length>0
                    onClicked: {menuItem.newSession(sessionname.text.replace(/^\s+|\s+$/g, '')); showInput=false;}

                    PlasmaComponents3.ToolTip {
                        text: i18n("Create new session and start Kate")
                    }
                }

                PlasmaComponents3.ToolButton {
                    icon.name: "dialog-cancel"
                    onClicked: showInput=false

                    PlasmaComponents3.ToolTip {
                        text: i18n("Cancel session creation")
                    }
                }
        }
        
        RowLayout {
            id: toolButtonsLayout
            anchors {
                right: label.right
                verticalCenter: parent.verticalCenter
            }

            PlasmaComponents3.ToolButton {
                icon.name: "edit-delete"
                onClicked: menuItem.remove(UuidRole)

                PlasmaComponents3.ToolTip {
                    text: i18n("Delete session")
                }
            }

            Component.onCompleted: {
                toolButtonsLayout.visible = Qt.binding(function () { return (TypeRole==2) && (menuListView.currentIndex == index); });
            }
        }
    }
}
