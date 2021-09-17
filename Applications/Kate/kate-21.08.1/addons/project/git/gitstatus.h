/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef GITSTATUS_H
#define GITSTATUS_H

#include <QString>
#include <QVector>

namespace GitUtils
{
enum GitStatus {
    Unmerge_BothDeleted,
    Unmerge_AddedByUs,
    Unmerge_DeletedByThem,
    Unmerge_AddedByThem,
    Unmerge_DeletedByUs,
    Unmerge_BothAdded,
    Unmerge_BothModified,

    Index_Modified,
    Index_Added,
    Index_Deleted,
    Index_Renamed,
    Index_Copied,

    WorkingTree_Modified,
    WorkingTree_Deleted,
    WorkingTree_IntentToAdd,

    Untracked,
    Ignored
};

enum StatusXY {
    DD = 0x4444,
    AU = 0x4155,
    UD = 0x5544,
    UA = 0x5541,
    DU = 0x4455,
    AA = 0x4141,
    UU = 0x5555,

    //??
    QQ = 0x3f3f,
    //!!
    II = 0x2121,
};

struct StatusItem {
    QByteArray file;
    GitStatus status;
    char statusChar;
    int linesAdded;
    int linesRemoved;
};

struct GitParsedStatus {
    QVector<StatusItem> untracked;
    QVector<StatusItem> unmerge;
    QVector<StatusItem> staged;
    QVector<StatusItem> changed;
};

GitParsedStatus parseStatus(const QByteArray &raw);

void parseDiffNumStat(QVector<GitUtils::StatusItem> &items, const QByteArray &raw);

QVector<StatusItem> parseDiffNameStatus(const QByteArray &raw);

QString statusString(GitStatus s);
}

#endif // GITSTATUS_H
