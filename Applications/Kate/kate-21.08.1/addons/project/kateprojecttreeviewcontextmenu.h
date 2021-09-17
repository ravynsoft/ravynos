/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2013 Dominik Haumann <dhaumann.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_PROJECT_TREE_VIEW_CONTEXT_MENU_H
#define KATE_PROJECT_TREE_VIEW_CONTEXT_MENU_H

#include <QObject>
#include <QPoint>
#include <QString>

class QWidget;
class QModelIndex;
class KateProjectViewTree;

class KateProjectTreeViewContextMenu : public QObject
{
    Q_OBJECT
public:
    /**
     * our project.
     * @return project
     */
    void exec(const QString &filename, const QModelIndex &index, const QPoint &pos, KateProjectViewTree *parent);

    /**
     * emits on clicking Menu->Show File History
     */
    Q_SIGNAL void showFileHistory(const QString &file);
};

#endif
