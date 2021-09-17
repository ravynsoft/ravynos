/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_PROJECT_VIEW_TREE_H
#define KATE_PROJECT_VIEW_TREE_H

#include <QTreeView>

class KateProjectPluginView;
class KateProject;

/**
 * A tree like view of project content.
 */
class KateProjectViewTree : public QTreeView
{
    Q_OBJECT

public:
    /**
     * construct project view for given project
     * @param pluginView our plugin view
     * @param project project this view is for
     */
    KateProjectViewTree(KateProjectPluginView *pluginView, KateProject *project);

    /**
     * deconstruct project
     */
    ~KateProjectViewTree() override;

    /**
     * our project.
     * @return project
     */
    KateProject *project() const
    {
        return m_project;
    }

    /**
     * Select given file in the view.
     * @param file select this file in the view, will be shown if invisible
     */
    void selectFile(const QString &file);

    /**
     * Open the selected document, if any.
     */
    void openSelectedDocument();

    /**
     * Add a new file
     */
    void addFile(const QModelIndex &idx, const QString &fileName);

    /**
     * Add a new directory
     */
    void addDirectory(const QModelIndex &idx, const QString &name);
    
    /**
     * remove a file, the function isn't closing document before removing'
     */
    void removeFile(const QModelIndex &idx, const QString &fullFilePath);

    /**
     * Open project terminal at location dirPath
     */
    void openTerminal(const QString &dirPath);

private Q_SLOTS:
    /**
     * item got clicked, do stuff, like open document
     * @param index model index of clicked item
     */
    void slotClicked(const QModelIndex &index);

    /**
     * Triggered on model changes.
     * This includes the files list, itemForFile mapping!
     */
    void slotModelChanged();

protected:
    /**
     * Create matching context menu.
     * @param event context menu event
     */
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    /**
     * our plugin view
     */
    KateProjectPluginView *m_pluginView;

    /**
     * our project
     */
    KateProject *m_project;

Q_SIGNALS:
    void showFileHistory(const QString &file);
};

#endif
