/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "gitstatusmodel.h"

#include <KColorScheme>
#include <QDebug>
#include <QFileInfo>
#include <QFont>
#include <QIcon>
#include <QMimeDatabase>

#include <KLocalizedString>

static constexpr int Staged = 0;
static constexpr int Changed = 1;
static constexpr int Conflict = 2;
static constexpr int Untrack = 3;
static constexpr quintptr Root = 0xFFFFFFFF;

GitStatusModel::GitStatusModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    // setup root rows
    beginInsertRows(QModelIndex(), 0, 3);
    endInsertRows();
}

QModelIndex GitStatusModel::index(int row, int column, const QModelIndex &parent) const
{
    auto rootIndex = Root;
    if (parent.isValid()) {
        if (parent.internalId() == Root) {
            rootIndex = parent.row();
        }
    }
    return createIndex(row, column, rootIndex);
}

QModelIndex GitStatusModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    return createIndex(child.internalId(), 0, Root);
}

int GitStatusModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return 4;
    }

    if (parent.internalId() == Root) {
        if (parent.row() < 0 || parent.row() > 3) {
            return 0;
        }

        return m_nodes[parent.row()].size();
    }
    return 0;
}

int GitStatusModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant GitStatusModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const int row = index.row();

    if (index.internalId() == Root) {
        if (role == Qt::DisplayRole) {
            if (index.column() == 1) {
                return QString::number(m_nodes[row].count());
            } else {
                if (row == Staged) {
                    return i18n("Staged");
                } else if (row == Untrack) {
                    return i18n("Untracked");
                } else if (row == Conflict) {
                    return i18n("Conflict");
                } else if (row == Changed) {
                    return i18n("Modified");
                } else {
                    Q_UNREACHABLE();
                }
            }
        } else if (role == Qt::FontRole) {
            QFont bold;
            bold.setBold(true);
            return bold;
        } else if (role == Role::TreeItemType) {
            return NodeStage + row;
        } else if (role == Qt::TextAlignmentRole) {
            if (index.column() == 0) {
                return (int)(Qt::AlignLeft | Qt::AlignVCenter);
            } else {
                return (int)(Qt::AlignRight | Qt::AlignVCenter);
            }
        }
    } else {
        int rootIndex = index.internalId();
        if (rootIndex < 0 || rootIndex > 3) {
            return QVariant();
        }

        if (role == Qt::DisplayRole) {
            if (index.column() == 0) {
                const auto filename = QFileInfo(QString::fromUtf8(m_nodes[rootIndex].at(row).file)).fileName();
                if (filename.isEmpty()) {
                    return m_nodes[rootIndex].at(row).file;
                }
                return filename;
            } else {
                if (!m_showNumStat) {
                    return QString(QLatin1Char(m_nodes[rootIndex].at(row).statusChar));
                }
                int a = m_nodes[rootIndex].at(row).linesAdded;
                int r = m_nodes[rootIndex].at(row).linesRemoved;
                auto add = QString::number(a);
                auto sub = QString::number(r);
                QString statusChar(QLatin1Char(m_nodes[rootIndex].at(row).statusChar));
                QString ret = QStringLiteral("+") + add + QStringLiteral(" -") + sub + QStringLiteral(" ") + statusChar;
                return ret;
            }
        } else if (role == FileNameRole) {
            return m_nodes[rootIndex].at(row).file;
        } else if (role == Qt::DecorationRole) {
            if (index.column() == 0) {
                const QString file = QString::fromUtf8(m_nodes[rootIndex].at(row).file);
                return QIcon::fromTheme(QMimeDatabase().mimeTypeForFile(file, QMimeDatabase::MatchExtension).iconName());
            }
        } else if (role == Role::TreeItemType) {
            return ItemType::NodeFile;
        } else if (role == Qt::ToolTipRole) {
            return QString(QString::fromUtf8(m_nodes[rootIndex].at(row).file) + GitUtils::statusString(m_nodes[rootIndex].at(row).status));
        } else if (role == Qt::TextAlignmentRole) {
            if (index.column() == 0) {
                return (int)(Qt::AlignLeft | Qt::AlignVCenter);
            } else {
                return (int)(Qt::AlignRight | Qt::AlignVCenter);
            }
        } else if (role == Qt::ForegroundRole) {
            if (index.column() == 1 && rootIndex > 0) {
                return KColorScheme().foreground(KColorScheme::NegativeText).color();
            } else if (index.column() == 1 && rootIndex == 0) {
                return KColorScheme().foreground(KColorScheme::PositiveText).color();
            }
        }
    }

    return {};
}
void GitStatusModel::addItems(GitUtils::GitParsedStatus status, bool numStat)
{
    beginResetModel();
    m_nodes[Staged] = std::move(status.staged);
    m_nodes[Changed] = std::move(status.changed);
    m_nodes[Conflict] = std::move(status.unmerge);
    m_nodes[Untrack] = std::move(status.untracked);
    m_showNumStat = numStat;
    endResetModel();
}

QVector<int> GitStatusModel::emptyRows()
{
    QVector<int> empty;
    for (int i = 0; i < 4; ++i) {
        if (m_nodes[i].isEmpty()) {
            empty.append(i);
        }
    }
    return empty;
}
