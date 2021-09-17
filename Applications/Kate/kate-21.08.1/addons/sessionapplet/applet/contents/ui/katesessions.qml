/********************************************************************
This file is part of the KDE project.

SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>

Based on the clipboard applet:
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: main
    width: (Plasmoid.formFactor==PlasmaCore.Types.Planar)? PlasmaCore.Units.gridUnit * 14 : undefined
    height: (Plasmoid.formFactor==PlasmaCore.Types.Planar)? PlasmaCore.Units.gridUnit * 16: undefined
    
    Plasmoid.switchWidth: PlasmaCore.Units.gridUnit * 11
    Plasmoid.switchHeight: PlasmaCore.Units.gridUnit * 11
    Plasmoid.status: PlasmaCore.Types.ActiveStatus
    Plasmoid.toolTipMainText: i18n("Kate Sessions")
    Plasmoid.icon: "kate"


    Component.onCompleted: {
        plasmoid.removeAction("configure");
    }

    PlasmaCore.DataSource {
        id: sessionsSource
        property bool editing: false;
        engine: "org.kde.plasma.katesessions"
        connectedSources: "katesessions"
        function serviceAction(uuid, op) {
            const service = sessionsSource.serviceForSource(uuid);
            var operation = service.operationDescription(op);
            return service.startOperationCall(operation);
        }
        function newSession(sessionName) {
            const service = sessionsSource.serviceForSource("");
            var operation = service.operationDescription("newSession");
            operation.sessionName = sessionName;
            return service.startOperationCall(operation);
        }
        
    }

    property var searchHeader: PlasmaExtras.PlasmoidHeading {
        PlasmaComponents3.TextField {
            id: filter
            placeholderText: i18n("Search…")
            clearButtonShown: true
            anchors.fill: parent
        }
    }

    Plasmoid.fullRepresentation: PlasmaComponents3.Page {

        id: dialogItem
        Layout.minimumWidth: PlasmaCore.Units.gridUnit * 12
        Layout.minimumHeight: PlasmaCore.Units.gridUnit * 12

        focus: true
        header: searchHeader

        property alias listMargins: listItemSvg.margins

        PlasmaCore.FrameSvgItem {
            id : listItemSvg
            imagePath: "widgets/listitem"
            prefix: "normal"
            visible: false
        }

        Keys.onPressed: {
            switch(event.key) {
                case Qt.Key_Up: {
                    sessionsMenu.view.decrementCurrentIndex();
                    event.accepted = true;
                    break;
                }
                case Qt.Key_Down: {
                    sessionsMenu.view.incrementCurrentIndex();
                    event.accepted = true;
                    break;
                }
                case Qt.Key_Enter:
                case Qt.Key_Return: {
                    if (sessionsMenu.view.currentIndex >= 0) {
                        const uuid = sessionsMenu.model.get(sessionsMenu.view.currentIndex).UuidRole
                        if (uuid) {
                            sessionsSource.serviceAction(uuid, "invoke")
                            sessionsMenu.view.currentIndex = 0
                        }
                    }
                    break;
                }
                case Qt.Key_Escape: {
                    if (filter.text == "") {
                        plasmoid.expanded = false;
                    } else {
                        filter.text = "";
                    }
                    event.accepted = true;
                    break;
                }
                default: { // forward key to filter
                    // filter.text += event.text wil break if the key is backspace
                    if (event.key == Qt.Key_Backspace && filter.text == "") {
                        return;
                    }
                    if (event.text != "" && !filter.activeFocus) {
                        sessionsMenu.view.currentIndex = -1
                        if (event.text == "v" && event.modifiers & Qt.ControlModifier) {
                            filter.paste();
                        } else {
                            filter.text = "";
                            filter.text += event.text;
                        }
                        filter.forceActiveFocus();
                        event.accepted = true;
                    }
                }
            }
        }
        ColumnLayout {
            anchors.fill: parent
            Menu {
                id: sessionsMenu
                model: PlasmaCore.SortFilterModel {
                    sourceModel: sessionsSource.models.katesessions
                    filterRole: "DisplayRole"
                    filterRegExp: filter.text
                }
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: PlasmaCore.Units.smallSpacing
                onItemSelected: {
                    sessionsSource.serviceAction(uuid, "invoke")
                    plasmoid.expanded = false;
                }
                onRemove: sessionsSource.serviceAction(uuid, "remove")
                onNewSession: sessionsSource.newSession(sessionName)
            }
        }
    }
}
