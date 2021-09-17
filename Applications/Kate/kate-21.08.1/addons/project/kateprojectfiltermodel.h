/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *  SPDX-FileCopyrightText: 2021 Waqar Ahmed        <waqar.17a@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATEPROJECTFILTERMODEL_H
#define KATEPROJECTFILTERMODEL_H

#include <QDebug>
#include <QSortFilterProxyModel>

#include <kfts_fuzzy_match.h>

class KateProjectFilterProxyModel : public QSortFilterProxyModel
{
public:
    KateProjectFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
    }

    void setFilterString(const QString &string)
    {
        m_pattern = string;
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (m_pattern.isEmpty()) {
            return true;
        }

        int score = 0; // unused intentionally
        QString file = sourceModel()->index(sourceRow, 0, sourceParent).data().toString();
        return kfts::fuzzy_match(m_pattern, file, score);
    }

private:
    QString m_pattern;
};

#endif // KATEPROJECTFILTERMODEL_H
