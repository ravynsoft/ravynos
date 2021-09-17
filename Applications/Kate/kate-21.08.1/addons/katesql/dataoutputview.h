/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef DATAOUTPUTVIEW_H
#define DATAOUTPUTVIEW_H

#include <QTableView>

class DataOutputView : public QTableView
{
    Q_OBJECT

public:
    DataOutputView(QWidget *parent = nullptr);

private Q_SLOTS:
    void slotCustomContextMenuRequested(const QPoint &pos);
};

#endif // DATAOUTPUTVIEW_H
