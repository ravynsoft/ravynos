/*
    SPDX-FileCopyrightText: 2020 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "gotosymboltreeview.h"

#include <KTextEditor/Cursor>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>
#include <QHeaderView>

GotoSymbolTreeView::GotoSymbolTreeView(KTextEditor::MainWindow *mainWindow, QWidget *parent)
    : QTreeView(parent)
    , m_mainWindow(mainWindow)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setTextElideMode(Qt::ElideRight);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setHeaderHidden(true);
    setRootIsDecorated(false);
}

int GotoSymbolTreeView::sizeHintWidth() const
{
    return sizeHintForColumn(0);
}

void GotoSymbolTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (globalMode) {
        return QTreeView::currentChanged(current, previous);
    }

    int line = current.data(Qt::UserRole).toInt();
    KTextEditor::Cursor c(--line, 0);
    if (c.isValid()) {
        auto view = m_mainWindow->activeView();
        if (view) {
            view->setCursorPosition(c);
        }
    }

    return QTreeView::currentChanged(current, previous);
}
