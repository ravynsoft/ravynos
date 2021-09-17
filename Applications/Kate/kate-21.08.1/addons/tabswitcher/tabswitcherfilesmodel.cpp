/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2018 Gregor Mi <codestruct@posteo.org>
   SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tabswitcherfilesmodel.h"

#include <QBrush>
#include <QDebug>
#include <QFileInfo>
#include <QMimeDatabase>

#include <KTextEditor/Document>

#include <algorithm>

namespace detail
{
FilenameListItem::FilenameListItem(KTextEditor::Document *doc)
    : document(doc)
{
}

QIcon FilenameListItem::icon() const
{
    return QIcon::fromTheme(QMimeDatabase().mimeTypeForUrl(document->url()).iconName());
}

QString FilenameListItem::documentName() const
{
    return document->documentName();
}

QString FilenameListItem::fullPath() const
{
    return document->url().toLocalFile();
}

/**
 * Note that if strs contains the empty string, the result will be ""
 */
QString longestCommonPrefix(std::vector<QString> const &strs)
{
    if (strs.empty()) {
        return QString();
    }

    if (strs.size() == 1) {
        return strs.front();
    }

    // get the min length
    auto it = std::min_element(strs.begin(), strs.end(), [](const QString &lhs, const QString &rhs) {
        return lhs.size() < rhs.size();
    });
    const int n = it->size();

    for (int pos = 0; pos < n; pos++) { // check each character
        for (size_t i = 1; i < strs.size(); i++) {
            if (strs[i][pos] != strs[i - 1][pos]) { // we found a mis-match
                // reverse search to find path separator
                const int sepIndex = strs.front().leftRef(pos).lastIndexOf(QLatin1Char('/'));
                if (sepIndex >= 0) {
                    pos = sepIndex + 1;
                }
                return strs.front().left(pos);
            }
        }
    }
    // prefix is n-length
    return strs.front().left(n);
}

void post_process(FilenameList &data)
{
    // collect non-empty paths
    std::vector<QString> paths;
    for (const auto &item : data) {
        const auto path = item.fullPath();
        if (!path.isEmpty()) {
            paths.push_back(path);
        }
    }

    const QString prefix = longestCommonPrefix(paths);
    int prefix_length = prefix.length();
    if (prefix_length == 1) { // if there is only the "/" at the beginning, then keep it
        prefix_length = 0;
    }

    for (auto &item : data) {
        // Note that item.documentName can contain additional characters - e.g. "README.md (2)" -
        // so we cannot use that and have to parse the base filename by other means:
        const QString basename = QFileInfo(item.fullPath()).fileName(); // e.g. "archive.tar.gz"

        // cut prefix (left side) and cut document name (plus slash) on the right side
        int len = item.fullPath().length() - prefix_length - basename.length() - 1;
        if (len > 0) { // only assign in case item.fullPath() is not empty
            // "PREFIXPATH/REMAININGPATH/BASENAME" --> "REMAININGPATH"
            item.displayPathPrefix = item.fullPath().mid(prefix_length, len);
        }
    }
}
}

detail::TabswitcherFilesModel::TabswitcherFilesModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

bool detail::TabswitcherFilesModel::insertDocument(int row, KTextEditor::Document *document)
{
    beginInsertRows(QModelIndex(), row, row);
    data_.insert(data_.begin() + row, FilenameListItem(document));
    endInsertRows();

    // update all other items, since the common prefix path may have changed
    updateItems();

    return true;
}

bool detail::TabswitcherFilesModel::removeDocument(KTextEditor::Document *document)
{
    auto it = std::find_if(data_.begin(), data_.end(), [document](FilenameListItem &item) {
        return item.document == document;
    });
    if (it == data_.end()) {
        return false;
    }

    const int row = std::distance(data_.begin(), it);
    removeRow(row);

    return true;
}

bool detail::TabswitcherFilesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    if (row < 0 || row + count > rowCount()) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    data_.erase(data_.begin() + row, data_.begin() + row + count);
    endRemoveRows();

    // update all other items, since the common prefix path may have changed
    updateItems();

    return true;
}

void detail::TabswitcherFilesModel::clear()
{
    if (!data_.empty()) {
        beginResetModel();
        data_.clear();
        endResetModel();
    }
}

void detail::TabswitcherFilesModel::raiseDocument(KTextEditor::Document *document)
{
    // skip row 0, since row 0 is already correct
    for (int row = 1; row < rowCount(); ++row) {
        if (data_[row].document == document) {
            beginMoveRows(QModelIndex(), row, row, QModelIndex(), 0);
            std::rotate(data_.begin(), data_.begin() + row, data_.begin() + row + 1);
            endMoveRows();
            break;
        }
    }
}

KTextEditor::Document *detail::TabswitcherFilesModel::item(int row) const
{
    return data_[row].document;
}

void detail::TabswitcherFilesModel::updateItems()
{
    post_process(data_);
    Q_EMIT dataChanged(createIndex(0, 0), createIndex(data_.size() - 1, 1), {});
}

int detail::TabswitcherFilesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

int detail::TabswitcherFilesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return data_.size();
}

QVariant detail::TabswitcherFilesModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        const auto &row = data_[index.row()];
        if (index.column() == 0) {
            return row.displayPathPrefix;
        } else {
            return row.documentName();
        }
    } else if (role == Qt::DecorationRole) {
        if (index.column() == 1) {
            const auto &row = data_[index.row()];
            return row.icon();
        }
    } else if (role == Qt::ToolTipRole) {
        const auto &row = data_[index.row()];
        return row.fullPath();
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == 0) {
            return Qt::AlignRight + Qt::AlignVCenter;
        } else {
            return Qt::AlignVCenter;
        }
    } else if (role == Qt::ForegroundRole) {
        if (index.column() == 0) {
            return QBrush(Qt::darkGray);
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
