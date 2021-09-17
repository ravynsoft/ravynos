/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "gitstatus.h"

#include <KLocalizedString>
#include <QByteArray>
#include <QList>

GitUtils::GitParsedStatus GitUtils::parseStatus(const QByteArray &raw)
{
    QVector<GitUtils::StatusItem> untracked;
    QVector<GitUtils::StatusItem> unmerge;
    QVector<GitUtils::StatusItem> staged;
    QVector<GitUtils::StatusItem> changed;

    const QList<QByteArray> rawList = raw.split(0x00);
    for (const auto &r : rawList) {
        if (r.isEmpty() || r.length() < 3) {
            continue;
        }

        char x = r.at(0);
        char y = r.at(1);
        uint16_t xy = (((uint16_t)x) << 8) | y;
        using namespace GitUtils;

        switch (xy) {
        case StatusXY::QQ:
            untracked.append({r.mid(3), GitStatus::Untracked, 'U', 0, 0});
            break;
        case StatusXY::II:
            untracked.append({r.mid(3), GitStatus::Ignored, 'I', 0, 0});
            break;

        case StatusXY::DD:
            unmerge.append({r.mid(3), GitStatus::Unmerge_BothDeleted, x, 0, 0});
            break;
        case StatusXY::AU:
            unmerge.append({r.mid(3), GitStatus::Unmerge_AddedByUs, x, 0, 0});
            break;
        case StatusXY::UD:
            unmerge.append({r.mid(3), GitStatus::Unmerge_DeletedByThem, x, 0, 0});
            break;
        case StatusXY::UA:
            unmerge.append({r.mid(3), GitStatus::Unmerge_AddedByThem, x, 0, 0});
            break;
        case StatusXY::DU:
            unmerge.append({r.mid(3), GitStatus::Unmerge_DeletedByUs, x, 0, 0});
            break;
        case StatusXY::AA:
            unmerge.append({r.mid(3), GitStatus::Unmerge_BothAdded, x, 0, 0});
            break;
        case StatusXY::UU:
            unmerge.append({r.mid(3), GitStatus::Unmerge_BothModified, x, 0, 0});
            break;
        }

        switch (x) {
        case 'M':
            staged.append({r.mid(3), GitStatus::Index_Modified, x, 0, 0});
            break;
        case 'A':
            staged.append({r.mid(3), GitStatus::Index_Added, x, 0, 0});
            break;
        case 'D':
            staged.append({r.mid(3), GitStatus::Index_Deleted, x, 0, 0});
            break;
        case 'R':
            staged.append({r.mid(3), GitStatus::Index_Renamed, x, 0, 0});
            break;
        case 'C':
            staged.append({r.mid(3), GitStatus::Index_Copied, x, 0, 0});
            break;
        }

        switch (y) {
        case 'M':
            changed.append({r.mid(3), GitStatus::WorkingTree_Modified, y, 0, 0});
            break;
        case 'D':
            changed.append({r.mid(3), GitStatus::WorkingTree_Deleted, y, 0, 0});
            break;
        case 'A':
            changed.append({r.mid(3), GitStatus::WorkingTree_IntentToAdd, y, 0, 0});
            break;
        }
    }

    return {untracked, unmerge, staged, changed};
}

QString GitUtils::statusString(GitUtils::GitStatus s)
{
    switch (s) {
    case WorkingTree_Modified:
    case Index_Modified:
        return i18n(" ‣ Modified");
    case Untracked:
        return i18n(" ‣ Untracked");
    case Index_Renamed:
        return i18n(" ‣ Renamed");
    case Index_Deleted:
    case WorkingTree_Deleted:
        return i18n(" ‣ Deleted");
    case Index_Added:
    case WorkingTree_IntentToAdd:
        return i18n(" ‣ Added");
    case Index_Copied:
        return i18n(" ‣ Copied");
    case Ignored:
        return i18n(" ‣ Ignored");
    case Unmerge_AddedByThem:
    case Unmerge_AddedByUs:
    case Unmerge_BothAdded:
    case Unmerge_BothDeleted:
    case Unmerge_BothModified:
    case Unmerge_DeletedByThem:
    case Unmerge_DeletedByUs:
        return i18n(" ‣ Conflicted");
    }
    return QString();
}

static bool getNum(const QByteArray &numBytes, int *num)
{
    bool res = false;
    *num = numBytes.toInt(&res);
    return res;
}

static void addNumStat(QVector<GitUtils::StatusItem> &items, int add, int sub, const QByteArray &file)
{
    // look in modified first, then staged
    auto item = std::find_if(items.begin(), items.end(), [&file](const GitUtils::StatusItem &si) {
        return si.file == file;
    });
    if (item != items.end()) {
        item->linesAdded = add;
        item->linesRemoved = sub;
        return;
    }
}

void GitUtils::parseDiffNumStat(QVector<GitUtils::StatusItem> &items, const QByteArray &raw)
{
    const auto lines = raw.split(0x00);
    for (const auto &line : lines) {
        // format: 12(adds)\t10(subs)\tFileName
        const auto cols = line.split('\t');
        if (cols.length() < 3) {
            continue;
        }

        int add = 0;
        if (!getNum(cols.at(0), &add)) {
            continue;
        }
        int sub = 0;
        if (!getNum(cols.at(1), &sub)) {
            continue;
        }

        const auto file = cols.at(2);
        addNumStat(items, add, sub, file);
    }
}

QVector<GitUtils::StatusItem> GitUtils::parseDiffNameStatus(const QByteArray &raw)
{
    const auto lines = raw.split('\n');
    QVector<GitUtils::StatusItem> out;
    out.reserve(lines.size());
    for (const auto &l : lines) {
        const auto cols = l.split('\t');
        if (cols.size() < 2) {
            continue;
        }

        GitUtils::StatusItem i;
        i.statusChar = cols[0][0];

        i.file = cols[1];
        out.append(i);
    }
    return out;
}
