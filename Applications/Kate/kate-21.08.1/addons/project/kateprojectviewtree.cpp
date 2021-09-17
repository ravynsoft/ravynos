/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectviewtree.h"
#include "kateproject.h"
#include "kateprojectfiltermodel.h"
#include "kateprojectpluginview.h"
#include "kateprojecttreeviewcontextmenu.h"

#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>

#include <QContextMenuEvent>
#include <QDir>

#include <KLocalizedString>

KateProjectViewTree::KateProjectViewTree(KateProjectPluginView *pluginView, KateProject *project)
    : m_pluginView(pluginView)
    , m_project(project)
{
    /**
     * default style
     */
    setHeaderHidden(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAllColumnsShowFocus(true);

    /**
     * attach view => project
     * do this once, model is stable for whole project life time
     * kill selection model
     * create sort proxy model
     */
    QItemSelectionModel *m = selectionModel();

    KateProjectFilterProxyModel *sortModel = new KateProjectFilterProxyModel(this);

    // sortModel->setFilterRole(SortFilterRole);
    // sortModel->setSortRole(SortFilterRole);
    sortModel->setRecursiveFilteringEnabled(true);
    sortModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    sortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    sortModel->setSourceModel(m_project->model());
    setModel(sortModel);
    delete m;

    /**
     * connect needed signals
     * we use activated + clicked as we want "always" single click activation + keyboard focus / enter working
     */
    connect(this, &KateProjectViewTree::activated, this, &KateProjectViewTree::slotClicked);
    connect(this, &KateProjectViewTree::clicked, this, &KateProjectViewTree::slotClicked);
    connect(m_project, &KateProject::modelChanged, this, &KateProjectViewTree::slotModelChanged);

    /**
     * trigger once some slots
     */
    slotModelChanged();
}

KateProjectViewTree::~KateProjectViewTree()
{
}

void KateProjectViewTree::selectFile(const QString &file)
{
    /**
     * get item if any
     */
    QStandardItem *item = m_project->itemForFile(file);
    if (!item) {
        return;
    }

    /**
     * select it
     */
    QModelIndex index = static_cast<QSortFilterProxyModel *>(model())->mapFromSource(m_project->model()->indexFromItem(item));
    scrollTo(index, QAbstractItemView::EnsureVisible);
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::Clear | QItemSelectionModel::Select);
}

void KateProjectViewTree::openSelectedDocument()
{
    /**
     * anything selected?
     */
    QModelIndexList selecteStuff = selectedIndexes();
    if (selecteStuff.isEmpty()) {
        return;
    }

    /**
     * we only handle files here!
     */
    if (selecteStuff[0].data(KateProjectItem::TypeRole).toInt() != KateProjectItem::File) {
        return;
    }

    /**
     * open document for first element, if possible
     */
    QString filePath = selecteStuff[0].data(Qt::UserRole).toString();
    if (!filePath.isEmpty()) {
        m_pluginView->mainWindow()->openUrl(QUrl::fromLocalFile(filePath));
    }
}

void KateProjectViewTree::addFile(const QModelIndex &idx, const QString &fileName)
{
    auto proxyModel = static_cast<QSortFilterProxyModel *>(model());
    auto index = proxyModel->mapToSource(idx);
    auto item = m_project->model()->itemFromIndex(index);

    const QString fullFileName = index.data(Qt::UserRole).toString() + QLatin1Char('/') + fileName;

    /**
     * Create an actual file on disk
     */
    QFile f(fullFileName);
    bool created = f.open(QIODevice::WriteOnly);
    if (!created) {
        QVariantMap genericMessage;
        genericMessage.insert(QStringLiteral("type"), QStringLiteral("Error"));
        genericMessage.insert(QStringLiteral("category"), i18n("Project"));
        genericMessage.insert(QStringLiteral("categoryIcon"), QIcon::fromTheme(QStringLiteral("document-new")));
        genericMessage.insert(QStringLiteral("text"), i18n("Failed to create file: %1, Error: %2", fileName, f.errorString()));
        Q_EMIT m_pluginView->message(genericMessage);
        return;
    }

    KateProjectItem *i = new KateProjectItem(KateProjectItem::File, fileName);
    i->setData(fullFileName, Qt::UserRole);
    item->appendRow(i);
    m_project->addFile(fullFileName, i);
    item->sortChildren(0);
}

void KateProjectViewTree::addDirectory(const QModelIndex &idx, const QString &name)
{
    auto proxyModel = static_cast<QSortFilterProxyModel *>(model());
    auto index = proxyModel->mapToSource(idx);
    auto item = m_project->model()->itemFromIndex(index);
    const QString fullDirName = index.data(Qt::UserRole).toString() + QLatin1Char('/') + name;

    QDir dir(index.data(Qt::UserRole).toString());
    if (!dir.mkdir(name)) {
        QVariantMap genericMessage;
        genericMessage.insert(QStringLiteral("type"), QStringLiteral("Error"));
        genericMessage.insert(QStringLiteral("category"), i18n("Project"));
        genericMessage.insert(QStringLiteral("categoryIcon"), QIcon::fromTheme(QStringLiteral("folder-new")));
        genericMessage.insert(QStringLiteral("text"), i18n("Failed to create dir: %1", name));
        Q_EMIT m_pluginView->message(genericMessage);
        return;
    }

    KateProjectItem *i = new KateProjectItem(KateProjectItem::Directory, name);
    i->setData(fullDirName, Qt::UserRole);
    item->appendRow(i);
    item->sortChildren(0);
}

void KateProjectViewTree::removeFile(const QModelIndex& idx, const QString& fullFilePath)
{
    auto proxyModel = static_cast<QSortFilterProxyModel *>(model());
    auto index = proxyModel->mapToSource(idx);
    auto item = m_project->model()->itemFromIndex(index);
    QStandardItem* parent = item->parent();

    /**
     * Delete file
     */
    QFile file(fullFilePath);
    if(file.remove())//.moveToTrash()
    {
        if(parent != nullptr)
        {
            parent->removeRow(item->row());
            parent->sortChildren(0);
        }
        else
        {
            m_project->model()->removeRow(item->row());
            m_project->model()->sort(0);
        }
        m_project->removeFile(fullFilePath);
    }
}

void KateProjectViewTree::openTerminal(const QString &dirPath)
{
    m_pluginView->openTerminal(dirPath, m_project);
}

void KateProjectViewTree::slotClicked(const QModelIndex &index)
{
    /**
     * open document, if any usable user data
     */
    const QString filePath = index.data(Qt::UserRole).toString();
    if (!filePath.isEmpty()) {
        /**
         * normal file? => just trigger open of it
         */
        if (index.data(KateProjectItem::TypeRole).toInt() == KateProjectItem::File) {
            m_pluginView->mainWindow()->openUrl(QUrl::fromLocalFile(filePath));
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::Clear | QItemSelectionModel::Select);
            return;
        }

        /**
         * linked project? => switch the current active project
         */
        if (index.data(KateProjectItem::TypeRole).toInt() == KateProjectItem::LinkedProject) {
            m_pluginView->switchToProject(QDir(filePath));
            return;
        }
    }
}

void KateProjectViewTree::slotModelChanged()
{
    /**
     * model was updated
     * perhaps we need to highlight again new file
     */
    KTextEditor::View *activeView = m_pluginView->mainWindow()->activeView();
    if (activeView && activeView->document()->url().isLocalFile()) {
        selectFile(activeView->document()->url().toLocalFile());
    }
}

void KateProjectViewTree::contextMenuEvent(QContextMenuEvent *event)
{
    /**
     * get path file path or don't do anything
     */
    QModelIndex index = selectionModel()->currentIndex();
    QString filePath = index.data(Qt::UserRole).toString();
    if (filePath.isEmpty()) {
        QTreeView::contextMenuEvent(event);
        return;
    }

    KateProjectTreeViewContextMenu menu;
    connect(&menu, &KateProjectTreeViewContextMenu::showFileHistory, this, &KateProjectViewTree::showFileHistory);
    menu.exec(filePath, index, viewport()->mapToGlobal(event->pos()), this);

    event->accept();
}
