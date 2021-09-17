/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008-2014 Dominik Haumann <dhaumann kde org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "btdatabase.h"

#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>

void KateBtDatabase::loadFromFile(const QString &url)
{
    QFile file(url);
    if (file.open(QIODevice::ReadOnly)) {
        QMutexLocker locker(&mutex);
        QDataStream ds(&file);
        ds >> db;
    }
    //     qDebug() << "Number of entries in the backtrace database" << url << ":" << db.size();
}

void KateBtDatabase::saveToFile(const QString &url) const
{
    QFile file(url);
    if (file.open(QIODevice::WriteOnly)) {
        QMutexLocker locker(&mutex);
        QDataStream ds(&file);
        ds << db;
    }
    //     qDebug() << "Saved backtrace database to" << url;
}

QString KateBtDatabase::value(const QString &key)
{
    // key is either of the form "foo/bar.txt" or only "bar.txt"
    QString file = key;
    QStringList sl = key.split(QLatin1Char('/'));
    if (sl.size() > 1) {
        file = sl[1];
    }

    QMutexLocker locker(&mutex);
    if (db.contains(file)) {
        const QStringList &sl = db.value(file);
        for (int i = 0; i < sl.size(); ++i) {
            if (sl[i].indexOf(key) != -1) {
                return sl[i];
            }
        }
        // try to use the first one
        if (!sl.empty()) {
            return sl[0];
        }
    }

    return QString();
}

void KateBtDatabase::add(const QString &folder, const QStringList &files)
{
    QMutexLocker locker(&mutex);
    for (const QString &file : files) {
        QStringList &sl = db[file];
        QString entry = QDir::fromNativeSeparators(folder + QLatin1Char('/') + file);
        if (!sl.contains(entry)) {
            sl << entry;
        }
    }
}

int KateBtDatabase::size() const
{
    QMutexLocker locker(&mutex);
    return db.size();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
