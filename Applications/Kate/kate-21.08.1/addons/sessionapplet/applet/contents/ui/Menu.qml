/********************************************************************
This file is part of the KDE project.

SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>

Based on the clipboard applet:
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
import QtQuick 2.0
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents // for Highlight

PlasmaExtras.ScrollArea {
    id: menu
    property alias view: menuListView
    property alias model: menuListView.model
    signal itemSelected(string uuid)
    signal remove(string uuid)
    signal newSession(string sessionName)

    ListView {
        id: menuListView
        focus: true

        boundsBehavior: Flickable.StopAtBounds
        interactive: contentHeight > height
        highlight: PlasmaComponents.Highlight {
            anchors.bottomMargin: -listMargins.bottom
            y: 1
        }
        highlightMoveDuration: 0
        highlightResizeDuration: 0
        currentIndex: -1

        delegate: KateSessionsItemDelegate {
            width: menuListView.width

            onItemSelected: menu.itemSelected(uuid)
            onRemove: menu.remove(uuid)
            onNewSession:menu.newSession(sessionname)
        }
    }
}
