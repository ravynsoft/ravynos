/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "dataoutputview.h"

#include <QAction>

#include <QCursor>
#include <QMenu>

DataOutputView::DataOutputView(QWidget *parent)
    : QTableView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &DataOutputView::customContextMenuRequested, this, &DataOutputView::slotCustomContextMenuRequested);
}

void DataOutputView::slotCustomContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);

    QMenu menu;

    menu.addActions(actions());

    menu.exec(QCursor::pos());
}
