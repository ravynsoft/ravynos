/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2010 Thomas Fjellstrom <thomas@fjellstrom.ca>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_FILETREEPROXYMODEL_H
#define KATE_FILETREEPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace KTextEditor
{
class Document;
}

class KateFileTreeProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    KateFileTreeProxyModel(QObject *p = nullptr);
    QModelIndex docIndex(const KTextEditor::Document *) const;
    bool isDir(const QModelIndex &i) const;
    void setSourceModel(QAbstractItemModel *model) override;

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif /* KATE_FILETREEPROXYMODEL_H */
