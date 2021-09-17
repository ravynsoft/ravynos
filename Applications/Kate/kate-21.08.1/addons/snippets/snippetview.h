/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
 *  SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *  SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SNIPPETVIEW_H
#define SNIPPETVIEW_H

#include <KTextEditor/MainWindow>

#include "ui_snippetview.h"

class QStandardItem;
class KateSnippetGlobal;
class QAction;
class QSortFilterProxyModel;

namespace KTextEditor
{
}

/**
 * This class gets embedded into the right tool view by the KateSnippetGlobal.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 * @author Milian Wolff <mail@milianw.de>
 */
class SnippetView : public QWidget, public Ui::SnippetViewBase
{
    Q_OBJECT

public:
    explicit SnippetView(KateSnippetGlobal *plugin, KTextEditor::MainWindow *mainWindow, QWidget *parent = nullptr);

public:
    void setupActionsForWindow(QWidget *widget);

private Q_SLOTS:
    /**
     * Opens the "Add Repository" dialog.
     */
    void slotAddRepo();

    /**
     * Opens the "Edit repository" dialog.
     */
    void slotEditRepo();

    /**
     * Removes the selected repository from the disk.
     */
    void slotRemoveRepo();

    /**
     * Insert the selected snippet into the current file
     */
    void slotSnippetClicked(const QModelIndex &index);

    /**
     * Open the edit dialog for the selected snippet
     */
    void slotEditSnippet();

    /**
     * Removes the selected snippet from the tree and the filesystem
     */
    void slotRemoveSnippet();

    /**
     * Creates a new snippet and open the edit dialog for it
     */
    void slotAddSnippet();

    /**
     * Slot to get hot new stuff.
     */
    void slotGHNS();

    /**
     * Slot to put the selected snippet to GHNS
     */
    void slotSnippetToGHNS();

    void contextMenu(const QPoint &pos);
    /// disables or enables available actions based on the currently selected item
    void validateActions();

    /// insert snippet on double click
    bool eventFilter(QObject *, QEvent *) override;

private:
    QStandardItem *currentItem();

    KateSnippetGlobal *m_plugin;
    QSortFilterProxyModel *m_proxy;

    QAction *m_addRepoAction;
    QAction *m_removeRepoAction;
    QAction *m_editRepoAction;
    QAction *m_addSnippetAction;
    QAction *m_removeSnippetAction;
    QAction *m_editSnippetAction;
    QAction *m_getNewStuffAction;
    QAction *m_putNewStuffAction;
};

#endif
