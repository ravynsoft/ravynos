/*
SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

SPDX-License-Identifier: LGPL-2.0-only
*/

#include "cachedsqlquerymodel.h"

#include <QDebug>

CachedSqlQueryModel::CachedSqlQueryModel(QObject *parent, int cacheCapacity)
    : QSqlQueryModel(parent)
    , cache(cacheCapacity)
{
}

QVariant CachedSqlQueryModel::data(const QModelIndex &item, int role) const
{
    if (!item.isValid()) {
        return QVariant();
    }

    if (role == Qt::EditRole) {
        return QSqlQueryModel::data(item, role);
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    const int row = item.row();

    return record(row).value(item.column());
}

QSqlRecord CachedSqlQueryModel::record(int row) const
{
    // if cache capacity is set to 0, don't use cache
    if (cacheCapacity() == 0) {
        return QSqlQueryModel::record(row);
    }

    const int lookAhead = cacheCapacity() / 5;
    const int halfLookAhead = lookAhead / 2;

    if (row > cache.lastIndex()) {
        if (row - cache.lastIndex() > lookAhead) {
            cacheRecords(row - halfLookAhead, qMin(rowCount(), row + halfLookAhead));
        } else {
            int until = qMin(rowCount(), cache.lastIndex() + lookAhead);

            while (cache.lastIndex() < until) {
                cache.append(QSqlQueryModel::record(cache.lastIndex() + 1));
            }
        }
    } else if (row < cache.firstIndex()) {
        if (cache.firstIndex() - row > lookAhead) {
            cacheRecords(qMax(0, row - halfLookAhead), row + halfLookAhead);
        } else {
            int until = qMax(0, cache.firstIndex() - lookAhead);

            while (cache.firstIndex() > until) {
                cache.prepend(QSqlQueryModel::record(cache.firstIndex() - 1));
            }
        }
    }

    return cache.at(row);
}

void CachedSqlQueryModel::clear()
{
    clearCache();

    QSqlQueryModel::clear();
}

void CachedSqlQueryModel::cacheRecords(int from, int to) const
{
    qDebug() << "caching records from" << from << "to" << to;

    for (int i = from; i <= to; ++i) {
        cache.insert(i, QSqlQueryModel::record(i));
    }
}

void CachedSqlQueryModel::clearCache()
{
    cache.clear();
}

int CachedSqlQueryModel::cacheCapacity() const
{
    return cache.capacity();
}

void CachedSqlQueryModel::setCacheCapacity(int capacity)
{
    qDebug() << "cache capacity set to" << capacity;

    cache.setCapacity(capacity);
}

void CachedSqlQueryModel::queryChange()
{
    clearCache();
}
