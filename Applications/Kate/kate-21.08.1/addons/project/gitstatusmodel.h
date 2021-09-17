/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef GITSTATUSMODEL_H
#define GITSTATUSMODEL_H

#include <QAbstractItemModel>

#include "git/gitstatus.h"

class GitStatusModel : public QAbstractItemModel
{
public:
    explicit GitStatusModel(QObject *parent);

    enum ItemType { NodeStage = 0, NodeChanges, NodeConflict, NodeUntrack, NodeFile };
    enum Role { TreeItemType = Qt::UserRole + 1, FileNameRole };

public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void addItems(GitUtils::GitParsedStatus status, bool numStat);
    QVector<int> emptyRows();

    const QVector<GitUtils::StatusItem> &untrackedFiles() const
    {
        return m_nodes[3];
    }

    const QVector<GitUtils::StatusItem> &stagedFiles() const
    {
        return m_nodes[0];
    }

    const QVector<GitUtils::StatusItem> &changedFiles() const
    {
        return m_nodes[1];
    }

    QModelIndex getModelIndex(ItemType type) const
    {
        return createIndex(type, 0, 0xFFFFFFFF);
    }

private:
    QVector<GitUtils::StatusItem> m_nodes[4];
    bool m_showNumStat = false;
};

#endif // GITSTATUSMODEL_H
