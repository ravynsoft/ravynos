/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2018 Gregor Mi <codestruct@posteo.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_TAB_SWITCHER_FILES_MODEL_H
#define KTEXTEDITOR_TAB_SWITCHER_FILES_MODEL_H

#include <QAbstractTableModel>
#include <QIcon>
#include <QString>

namespace KTextEditor
{
class Document;
}

namespace detail
{
/**
 * Represents one item in the table view of the tab switcher.
 */
class FilenameListItem
{
public:
    FilenameListItem(KTextEditor::Document *doc);

    KTextEditor::Document *document;
    QIcon icon() const;
    QString documentName() const;
    QString fullPath() const;

    /**
     * calculated from documentName and fullPath
     */
    QString displayPathPrefix;
};
using FilenameList = std::vector<FilenameListItem>;

class TabswitcherFilesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TabswitcherFilesModel(QObject *parent = nullptr);
    ~TabswitcherFilesModel() override = default;
    bool insertDocument(int row, KTextEditor::Document *document);
    bool removeDocument(KTextEditor::Document *document);

    /**
     * Clears all data from the model
     */
    void clear();

    /**
     * NOTE: The returned pointer will become invalid as soon as the underlying vector changes.
     */
    KTextEditor::Document *item(int row) const;

    /**
     * Move the document to row 0.
     */
    void raiseDocument(KTextEditor::Document *document);

    /*
     * Use this method to update all items.
     * This is typically needed when a document name changes, since then the prefix paths change,
     * so all items need an update.
     */
    void updateItems();

    /**
     * Reimplemented to return the column count of top-level items.
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Reimplemented to return the top-level row count.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Returns the data for the requested model index.
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * Reimplemented to remove the specified rows.
     * The paret is always ignored since this is a table model.
     */
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    FilenameList data_;
};

QString longestCommonPrefix(std::vector<QString> const &strs);

}

#endif // KTEXTEDITOR_TAB_SWITCHER_FILES_MODEL_H
