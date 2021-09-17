/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008-2014 Dominik Haumann <dhaumann kde org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef BTFILEINDEXER_H
#define BTFILEINDEXER_H

#include <QString>
#include <QStringList>
#include <QThread>

class KateBtDatabase;

class BtFileIndexer : public QThread
{
    Q_OBJECT
public:
    BtFileIndexer(KateBtDatabase *db);
    ~BtFileIndexer() override;
    void setSearchPaths(const QStringList &urls);

    void setFilter(const QStringList &filter);

    void cancel();

protected:
    void run() override;
    void indexFiles(const QString &url);

private:
    bool cancelAsap;
    QStringList searchPaths;
    QStringList filter;

    KateBtDatabase *db;
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
