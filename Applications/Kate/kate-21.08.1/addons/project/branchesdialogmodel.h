/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KATEQUICKOPENMODEL_H
#define KATEQUICKOPENMODEL_H

#include <QAbstractTableModel>
#include <QVariant>
#include <QVector>

#include "git/gitutils.h"

class BranchesDialogModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Role { FuzzyScore = Qt::UserRole + 1, OriginalSorting, CheckoutName, RefType, Creator, ItemTypeRole };
    enum ItemType { BranchItem, CreateBranch, CreateBranchFrom };

    explicit BranchesDialogModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    void refresh(const QVector<GitUtils::Branch> &branches, bool checkingOut = false);
    void clear();
    void clearBranchCreationItems();

    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        if (!index.isValid()) {
            return false;
        }
        if (role == Role::FuzzyScore) {
            auto row = index.row();
            m_modelEntries[row].score = value.toInt();
        }
        return QAbstractTableModel::setData(index, value, role);
    }

private:
    struct Branch {
        QString name;
        QString remote;
        GitUtils::RefType refType;
        int score;
        int dateSort;
        ItemType itemType;
    };

    QVector<BranchesDialogModel::Branch> m_modelEntries;
};

#endif
