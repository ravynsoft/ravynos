/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008-2014 Dominik Haumann <dhaumann kde org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "btfileindexer.h"
#include "btdatabase.h"

#include <QDebug>
#include <QDir>

BtFileIndexer::BtFileIndexer(KateBtDatabase *database)
    : cancelAsap(false)
    , db(database)
{
}

BtFileIndexer::~BtFileIndexer()
{
}

void BtFileIndexer::setSearchPaths(const QStringList &urls)
{
    searchPaths.clear();
    for (const QString &url : urls) {
        if (!searchPaths.contains(url)) {
            searchPaths += url;
        }
    }
}

void BtFileIndexer::setFilter(const QStringList &fileFilter)
{
    filter = fileFilter;
    qDebug() << filter;
}

void BtFileIndexer::run()
{
    if (filter.empty()) {
        qDebug() << "Filter is empty. Aborting.";
        return;
    }

    cancelAsap = false;
    for (int i = 0; i < searchPaths.size(); ++i) {
        if (cancelAsap) {
            break;
        }
        indexFiles(searchPaths[i]);
    }
    qDebug() << QStringLiteral("Backtrace file database contains %1 files").arg(db->size());
}

void BtFileIndexer::cancel()
{
    cancelAsap = true;
}

void BtFileIndexer::indexFiles(const QString &url)
{
    QDir dir(url);
    if (!dir.exists()) {
        return;
    }

    QStringList files = dir.entryList(filter, QDir::Files | QDir::NoSymLinks | QDir::Readable | QDir::NoDotAndDotDot | QDir::CaseSensitive);
    db->add(url, files);

    QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoSymLinks | QDir::Readable | QDir::NoDotAndDotDot | QDir::CaseSensitive);
    for (int i = 0; i < subdirs.size(); ++i) {
        if (cancelAsap) {
            break;
        }
        indexFiles(url + QLatin1Char('/') + subdirs[i]);
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
