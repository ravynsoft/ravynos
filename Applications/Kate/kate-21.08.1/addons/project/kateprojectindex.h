/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_PROJECT_INDEX_H
#define KATE_PROJECT_INDEX_H

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <QStandardItemModel>
#include <QStringList>
#include <QTemporaryFile>

/**
 * ctags reading
 */
#include "ctags/readtags.h"

/**
 * Class representing the index of a project.
 * This includes knowledge from ctags and Co.
 * Allows you to search for stuff and to get some useful auto-completion.
 * Is created in Worker thread in the background, then passed to project in
 * the main thread for usage.
 */
class KateProjectIndex
{
public:
    /**
     * construct new index for given files
     * @param files files to index
     * @param ctagsMap ctags section for extra options
     */
    KateProjectIndex(const QString &baseDir, const QString &indexDir, const QStringList &files, const QVariantMap &ctagsMap, bool force);

    /**
     * deconstruct project
     */
    ~KateProjectIndex();

    /**
     * Which kind of match items should be created in the passed model
     * of the findMatches function?
     */
    enum MatchType {
        /**
         * Completion matches, containing only name and same name only once
         */
        CompletionMatches,

        /**
         * Find matches, containing name, kind, file, line, ...
         */
        FindMatches
    };

    /**
     * Fill in completion matches for given view/range.
     * Uses e.g. ctags index.
     * @param model model to fill with matches
     * @param searchWord word to search for
     * @param type type of matches
     * @param options ctags find options (use default if -1)
     */
    void findMatches(QStandardItemModel &model, const QString &searchWord, MatchType type, int options = -1);

    /**
     * Check if running ctags was successful. This can be used
     * as indicator whether ctags is installed or not.
     * @return true if a valid index exists, otherwise false
     */
    bool isValid() const
    {
        return m_ctagsIndexHandle;
    }

private:
    /**
     * Load ctags tags.
     * @param files files to index
     * @param ctagsMap ctags section for extra options
     */
    void loadCtags(const QStringList &files, const QVariantMap &ctagsMap, bool force);

    /**
     * Open ctags tags.
     */
    void openCtags();

private:
    /**
     * ctags index file
     */
    QScopedPointer<QFile> m_ctagsIndexFile;

    /**
     * handle to ctags file for querying, if possible
     */
    tagFile *m_ctagsIndexHandle;
};

#endif
