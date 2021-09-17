/***************************************************************************
 *   SPDX-FileCopyrightText: 2004 Jens Dagerbo *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#include "tags.h"
#include <stdio.h>

namespace ctags
{
#include "readtags.h"
}

#include "ctagskinds.h"

QString Tags::_tagsfile;

Tags::TagEntry::TagEntry()
{
}

Tags::TagEntry::TagEntry(const QString &tag, const QString &type, const QString &file, const QString &pattern)
    : tag(tag)
    , type(type)
    , file(file)
    , pattern(pattern)
{
}

bool Tags::hasTag(const QString &tag)
{
    ctags::tagFileInfo info;
    ctags::tagFile *file = ctags::tagsOpen(_tagsfile.toLocal8Bit().constData(), &info);
    ctags::tagEntry entry;

    bool found = (ctags::tagsFind(file, &entry, tag.toLocal8Bit().constData(), TAG_FULLMATCH | TAG_OBSERVECASE) == ctags::TagSuccess);

    ctags::tagsClose(file);

    return found;
}

bool Tags::hasTag(const QString &fileName, const QString &tag)
{
    setTagsFile(fileName);
    ctags::tagFileInfo info;
    ctags::tagFile *file = ctags::tagsOpen(_tagsfile.toLocal8Bit().constData(), &info);
    ctags::tagEntry entry;

    bool found = (ctags::tagsFind(file, &entry, tag.toLocal8Bit().constData(), TAG_FULLMATCH | TAG_OBSERVECASE) == ctags::TagSuccess);

    ctags::tagsClose(file);

    return found;
}

unsigned int Tags::numberOfMatches(const QString &tagpart, bool partial)
{
    unsigned int n = 0;

    if (tagpart.isEmpty()) {
        return 0;
    }

    ctags::tagFileInfo info;
    ctags::tagFile *file = ctags::tagsOpen(_tagsfile.toLocal8Bit().constData(), &info);
    ctags::tagEntry entry;

    QByteArray tagpartBArray = tagpart.toLocal8Bit(); // for holding the char *
    if (ctags::tagsFind(file, &entry, tagpartBArray.data(), TAG_OBSERVECASE | (partial ? TAG_PARTIALMATCH : TAG_FULLMATCH)) == ctags::TagSuccess) {
        do {
            n++;
        } while (ctags::tagsFindNext(file, &entry) == ctags::TagSuccess);
    }

    ctags::tagsClose(file);

    return n;
}

Tags::TagList Tags::getPartialMatchesNoi8n(const QString &tagFile, const QString &tagpart)
{
    setTagsFile(tagFile);

    auto getExtension = [](const QString &fileUrl) -> QStringRef {
        int dotPos = fileUrl.lastIndexOf(QLatin1Char('.'));
        if (dotPos > -1) {
            return fileUrl.midRef(dotPos + 1);
        }
        return QStringRef();
    };

    Tags::TagList list;

    if (tagpart.isEmpty()) {
        return list;
    }

    ctags::tagFileInfo info;
    ctags::tagFile *file = ctags::tagsOpen(_tagsfile.toLocal8Bit().constData(), &info);
    ctags::tagEntry entry;

    QByteArray tagpartBArray = tagpart.toLocal8Bit(); // for holding the char *
    if (ctags::tagsFind(file, &entry, tagpartBArray.data(), TAG_OBSERVECASE | TAG_PARTIALMATCH) == ctags::TagSuccess) {
        do {
            QString file = QString::fromLocal8Bit(entry.file);
            QString type(CTagsKinds::findKindNoi18n(entry.kind, getExtension(file)));

            if (type.isEmpty() && file.endsWith(QLatin1String("Makefile"))) {
                type = QStringLiteral("macro");
            }

            list << TagEntry(QString::fromLocal8Bit(entry.name), type, file, QString::fromLocal8Bit(entry.address.pattern));
        } while (ctags::tagsFindNext(file, &entry) == ctags::TagSuccess);
    }

    ctags::tagsClose(file);

    return list;
}

Tags::TagList Tags::getMatches(const QString &tagpart, bool partial, const QStringList &types)
{
    Tags::TagList list;

    if (tagpart.isEmpty()) {
        return list;
    }

    ctags::tagFileInfo info;
    ctags::tagFile *file = ctags::tagsOpen(_tagsfile.toLocal8Bit().constData(), &info);
    ctags::tagEntry entry;

    QByteArray tagpartBArray = tagpart.toLocal8Bit(); // for holding the char *
    if (ctags::tagsFind(file, &entry, tagpartBArray.data(), TAG_OBSERVECASE | (partial ? TAG_PARTIALMATCH : TAG_FULLMATCH)) == ctags::TagSuccess) {
        do {
            QString file = QString::fromLocal8Bit(entry.file);
            QString type(CTagsKinds::findKind(entry.kind, file.section(QLatin1Char('.'), -1)));

            if (type.isEmpty() && file.endsWith(QLatin1String("Makefile"))) {
                type = QStringLiteral("macro");
            }
            if (types.isEmpty() || types.contains(QString::fromLocal8Bit(entry.kind))) {
                list << TagEntry(QString::fromLocal8Bit(entry.name), type, file, QString::fromLocal8Bit(entry.address.pattern));
            }
        } while (ctags::tagsFindNext(file, &entry) == ctags::TagSuccess);
    }

    ctags::tagsClose(file);

    return list;
}

void Tags::setTagsFile(const QString &file)
{
    _tagsfile = file;
}

QString Tags::getTagsFile()
{
    return _tagsfile;
}

unsigned int Tags::numberOfPartialMatches(const QString &tagpart)
{
    return numberOfMatches(tagpart, true);
}

unsigned int Tags::numberOfExactMatches(const QString &tagpart)
{
    return numberOfMatches(tagpart, false);
}

Tags::TagList Tags::getPartialMatches(const QString &tagpart)
{
    return getMatches(tagpart, true);
}

Tags::TagList Tags::getExactMatches(const QString &tag)
{
    return getMatches(tag, false);
}

Tags::TagList Tags::getPartialMatches(const QString &file, const QString &tagpart)
{
    setTagsFile(file);
    return getMatches(tagpart, true);
}

Tags::TagList Tags::getExactMatches(const QString &file, const QString &tag)
{
    setTagsFile(file);
    return getMatches(tag, false);
}

Tags::TagList Tags::getMatches(const QString &file, const QString &tagpart, bool partial, const QStringList &types)
{
    setTagsFile(file);
    return getMatches(tagpart, partial, types);
}

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs on;
