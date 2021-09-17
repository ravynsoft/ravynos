/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_PROJECT_INFO_VIEW_INDEX_H
#define KATE_PROJECT_INFO_VIEW_INDEX_H

#include <QLineEdit>
#include <QTreeView>
#include <QWidget>

class KateProjectPluginView;
class KMessageWidget;
class KateProject;
class QStandardItemModel;

/**
 * Class representing a view of a project.
 * A tree like view of project content.
 */
class KateProjectInfoViewIndex : public QWidget
{
    Q_OBJECT

public:
    /**
     * construct project info view for given project
     * @param pluginView our plugin view
     * @param project project this view is for
     */
    KateProjectInfoViewIndex(KateProjectPluginView *pluginView, KateProject *project, QWidget *parent = nullptr);

    /**
     * deconstruct info view
     */
    ~KateProjectInfoViewIndex() override;

    /**
     * our project.
     * @return project
     */
    KateProject *project() const
    {
        return m_project;
    }

private Q_SLOTS:
    /**
     * Called if text in lineedit changes, then we need to search
     * @param text new text
     */
    void slotTextChanged(const QString &text);

    /**
     * item got clicked, do stuff, like open document
     * @param index model index of clicked item
     */
    void slotClicked(const QModelIndex &index);

    /**
     * called whenever the index of the project was updated. Here,
     * it's used to show a warning, if ctags is not installed.
     */
    void indexAvailable();

    /**
     * called to enable or disable widgets
     * @param enable
     */
    void enableWidgets(bool enable);

    /**
     * called if goto symbol is requested
     * @param text target symbol
     * @param number of results
     */
    void slotGotoSymbol(const QString &text, int &results);

private:
    /**
     * our plugin view
     */
    KateProjectPluginView *m_pluginView;

    /**
     * our project
     */
    KateProject *m_project;

    /**
     * information widget showing a warning about missing ctags.
     */
    KMessageWidget *m_messageWidget;

    /**
     * line edit which allows to search index
     */
    QLineEdit *m_lineEdit;

    /**
     * tree view for results
     */
    QTreeView *m_treeView;

    /**
     * standard item model for results
     */
    QStandardItemModel *m_model;
};

#endif
