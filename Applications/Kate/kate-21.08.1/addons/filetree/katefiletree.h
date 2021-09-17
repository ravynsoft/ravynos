/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2010 Thomas Fjellstrom <thomas@fjellstrom.ca>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_FILETREE_H
#define KATE_FILETREE_H

#include <QIcon>
#include <QTreeView>
#include <QUrl>

namespace KTextEditor
{
class Document;
}

class QActionGroup;

class KateFileTree : public QTreeView
{
    Q_OBJECT

public:
    KateFileTree(QWidget *parent);
    ~KateFileTree() override;

    void setModel(QAbstractItemModel *model) override;
    void setShowCloseButton(bool show);

public Q_SLOTS:
    void slotDocumentClose();
    void slotExpandRecursive();
    void slotCollapseRecursive();
    void slotDocumentCloseOther();
    void slotDocumentReload();
    void slotOpenContainingFolder();
    void slotCopyFilename();
    void slotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void slotDocumentFirst();
    void slotDocumentLast();
    void slotDocumentNext();
    void slotDocumentPrev();
    void slotPrintDocument();
    void slotPrintDocumentPreview();
    void slotResetHistory();
    void slotDocumentDelete();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

Q_SIGNALS:
    void closeDocument(KTextEditor::Document *);
    void activateDocument(KTextEditor::Document *);

    void openDocument(const QUrl &);

    void viewModeChanged(bool treeMode);
    void sortRoleChanged(int);

private Q_SLOTS:
    void mouseClicked(const QModelIndex &index);

    void slotTreeMode();
    void slotListMode();

    void slotSortName();
    void slotSortPath();
    void slotSortOpeningOrder();
    void slotFixOpenWithMenu();
    void slotOpenWithMenuAction(QAction *a);

    void slotRenameFile();

private:
    QAction *setupOption(QActionGroup *group, const QIcon &, const QString &, const QString &, const char *slot, bool checked = false);

private:
    QAction *m_filelistCloseDocument;
    QAction *m_filelistExpandRecursive;
    QAction *m_filelistCollapseRecursive;
    QAction *m_filelistCloseOtherDocument;
    QAction *m_filelistReloadDocument;
    QAction *m_filelistOpenContainingFolder;
    QAction *m_filelistCopyFilename;
    QAction *m_filelistRenameFile;
    QAction *m_filelistPrintDocument;
    QAction *m_filelistPrintDocumentPreview;
    QAction *m_filelistDeleteDocument;

    QAction *m_treeModeAction;
    QAction *m_listModeAction;

    QAction *m_sortByFile;
    QAction *m_sortByPath;
    QAction *m_sortByOpeningOrder;
    QAction *m_resetHistory;

    QPersistentModelIndex m_previouslySelected;
    QPersistentModelIndex m_indexContextMenu;

    bool m_hasCloseButton;
};

#endif // KATE_FILETREE_H
