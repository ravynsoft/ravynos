/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2014 Dominik Haumann <dhaumann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_TABSWITCHER_TREEVIEW_H
#define KTEXTEDITOR_TABSWITCHER_TREEVIEW_H

#include <QTreeView>

/**
 * TODO: see screenshots https://phabricator.kde.org/D16054:
 *       some paths are truncated on the right side. Why?
 */
class TabSwitcherTreeView : public QTreeView
{
    Q_OBJECT

public:
    /**
     * Default constructor
     */
    TabSwitcherTreeView();

    /**
     * Sum of the widths of both columns
     */
    int sizeHintWidth() const;

    void resizeColumnsToContents();

Q_SIGNALS:
    /**
     * This signal is emitted whenever use activates an item through
     * the list view.
     * @note @p selectionIndex is a model index of the selectionModel()
     *       and not of the QListView's model itself.
     */
    void itemActivated(const QModelIndex &selectionIndex);

protected:
    /**
     * Reimplemented for tracking the CTRL key modifier.
     */
    void keyReleaseEvent(QKeyEvent *event) override;

    /**
     * Reimplemented for tracking the ESCAPE key.
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * Reimplemented for adjusting the column widths to fit the contents
     */
    void showEvent(QShowEvent *event) override;
};

#endif // KTEXTEDITOR_TABSWITCHER_TREEVIEW_H
