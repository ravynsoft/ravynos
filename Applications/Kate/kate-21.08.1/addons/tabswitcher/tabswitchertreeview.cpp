/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2014 Dominik Haumann <dhaumann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tabswitchertreeview.h"
#include "tabswitcher.h"

#include <QDebug>
#include <QKeyEvent>

TabSwitcherTreeView::TabSwitcherTreeView()
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    // setUniformItemSizes(true);
    setTextElideMode(Qt::ElideMiddle);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setHeaderHidden(true);
    setRootIsDecorated(false);
}

int TabSwitcherTreeView::sizeHintWidth() const
{
    return sizeHintForColumn(0) + sizeHintForColumn(1);
}

void TabSwitcherTreeView::resizeColumnsToContents()
{
    resizeColumnToContents(0);
    resizeColumnToContents(1);
}

void TabSwitcherTreeView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) {
        Q_EMIT itemActivated(selectionModel()->currentIndex());
        event->accept();
        hide();
    } else {
        QTreeView::keyReleaseEvent(event);
    }
}

void TabSwitcherTreeView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        event->accept();
        hide();
    } else {
        QTreeView::keyPressEvent(event);
    }
}

void TabSwitcherTreeView::showEvent(QShowEvent *event)
{
    resizeColumnsToContents();
    QTreeView::showEvent(event);
}
