/***************************************************************************
 *   SPDX-FileCopyrightText: 2004 Jens Dagerbo *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef TAGS_H
#define TAGS_H

#include <QString>
#include <QStringList>
#include <QVector>

class Tags
{
public:
    struct TagEntry {
        TagEntry();
        TagEntry(const QString &tag, const QString &type, const QString &file, const QString &pattern);

        QString tag;
        QString type;
        QString file;
        QString pattern;
    };

    typedef QVector<TagEntry> TagList;

    /**
     *    Method to set the tag database filename
     * @param file the tag database filename
     */
    static void setTagsFile(const QString &file);

    static QString getTagsFile();

    /**
     *    Method to check if the tag database contains a specific tag
     * @param tag Tag to look up
     * @return returns true if tag database contains 'tag'
     */
    static bool hasTag(const QString &tag);
    static bool hasTag(const QString &fileName, const QString &tag);

    static unsigned int numberOfPartialMatches(const QString &tagpart);
    static unsigned int numberOfExactMatches(const QString &tag);
    static unsigned int numberOfMatches(const QString &tagpart, bool partial);

    static TagList getPartialMatches(const QString &tagpart);
    static TagList getExactMatches(const QString &tag);
    static TagList getMatches(const QString &tagpart, bool partial, const QStringList &types = QStringList());

    static TagList getPartialMatches(const QString &file, const QString &tagpart);
    static TagList getExactMatches(const QString &file, const QString &tag);
    static TagList getMatches(const QString &file, const QString &tagpart, bool partial, const QStringList &types = QStringList());
    static TagList getPartialMatchesNoi8n(const QString &tagFile, const QString &tagpart);

private:
    static QString _tagsfile;
};

#endif

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
