/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectindex.h"

#include <QDir>
#include <QProcess>

/**
 * include ctags reading
 */
#include "ctags/readtags.c"

KateProjectIndex::KateProjectIndex(const QString &baseDir, const QString &indexDir, const QStringList &files, const QVariantMap &ctagsMap, bool force)
    : m_ctagsIndexHandle(nullptr)
{
    // allow project to override and specify a (re-usable) indexfile
    // otherwise fall-back to a temporary file if nothing specified
    auto ctagsFile = ctagsMap.value(QStringLiteral("index_file"));
    if (ctagsFile.userType() == QMetaType::QString) {
        auto path = ctagsFile.toString();
        if (!QDir::isAbsolutePath(path)) {
            path = QDir(baseDir).absoluteFilePath(path);
        }
        m_ctagsIndexFile.reset(new QFile(path));
    } else {
        // indexDir is typically QDir::tempPath() or otherwise specified in configuration
        m_ctagsIndexFile.reset(new QTemporaryFile(indexDir + QStringLiteral("/kate.project.ctags")));
    }

    /**
     * load ctags
     */
    loadCtags(files, ctagsMap, force);
}

KateProjectIndex::~KateProjectIndex()
{
    /**
     * delete ctags handle if any
     */
    if (m_ctagsIndexHandle) {
        tagsClose(m_ctagsIndexHandle);
        m_ctagsIndexHandle = nullptr;
    }
}

void KateProjectIndex::loadCtags(const QStringList &files, const QVariantMap &ctagsMap, bool force)
{
    /**
     * only overwrite existing index upon reload
     * (a temporary index file will never exist)
     */
    if (m_ctagsIndexFile->exists() && !force) {
        openCtags();
        return;
    }

    /**
     * create temporary file
     * if not possible, fail
     */
    if (!m_ctagsIndexFile->open(QIODevice::ReadWrite)) {
        return;
    }

    /**
     * close file again, other process will use it
     */
    m_ctagsIndexFile->close();

    /**
     * try to run ctags for all files in this project
     * output to our ctags index file
     */
    QProcess ctags;
    QStringList args;
    args << QStringLiteral("-L") << QStringLiteral("-") << QStringLiteral("-f") << m_ctagsIndexFile->fileName() << QStringLiteral("--fields=+K+n");
    const QString keyOptions = QStringLiteral("options");
    for (const QVariant &optVariant : ctagsMap[keyOptions].toList()) {
        args << optVariant.toString();
    }
    ctags.start(QStringLiteral("ctags"), args);
    if (!ctags.waitForStarted()) {
        return;
    }

    /**
     * write files list and close write channel
     */
    ctags.write(files.join(QLatin1Char('\n')).toLocal8Bit());
    ctags.closeWriteChannel();

    /**
     * wait for done
     */
    if (!ctags.waitForFinished(-1)) {
        return;
    }

    openCtags();
}

void KateProjectIndex::openCtags()
{
    /**
     * file not openable, bad
     */
    if (!m_ctagsIndexFile->open(QIODevice::ReadOnly)) {
        return;
    }

    /**
     * get size
     */
    qint64 size = m_ctagsIndexFile->size();

    /**
     * close again
     */
    m_ctagsIndexFile->close();

    /**
     * empty file, bad
     */
    if (!size) {
        return;
    }

    /**
     * close current
     */
    if (m_ctagsIndexHandle) {
        tagsClose(m_ctagsIndexHandle);
        m_ctagsIndexHandle = nullptr;
    }

    /**
     * try to open ctags file
     */
    tagFileInfo info;
    memset(&info, 0, sizeof(tagFileInfo));
    m_ctagsIndexHandle = tagsOpen(m_ctagsIndexFile->fileName().toLocal8Bit().constData(), &info);
}

void KateProjectIndex::findMatches(QStandardItemModel &model, const QString &searchWord, MatchType type, int options)
{
    /**
     * abort if no ctags index
     */
    if (!m_ctagsIndexHandle) {
        return;
    }

    /**
     * word to complete
     * abort if empty
     */
    QByteArray word = searchWord.toLocal8Bit();
    if (word.isEmpty()) {
        return;
    }

    /**
     * try to search entry
     * fail if none found
     */
    tagEntry entry;
    if (options == -1) {
        options = TAG_PARTIALMATCH | TAG_OBSERVECASE;
    }
    if (tagsFind(m_ctagsIndexHandle, &entry, word.constData(), options) != TagSuccess) {
        return;
    }

    /**
     * set to show words only once for completion matches
     */
    QSet<QString> guard;

    /**
     * loop over all found tags
     * first one is filled by above find, others by find next
     */
    do {
        /**
         * skip if no name
         */
        if (!entry.name) {
            continue;
        }

        /**
         * get name
         */
        QString name(QString::fromLocal8Bit(entry.name));

        /**
         * construct right items
         */
        switch (type) {
        case CompletionMatches:
            /**
             * add new completion item, if new name
             */
            if (!guard.contains(name)) {
                model.appendRow(new QStandardItem(name));
                guard.insert(name);
            }
            break;

        case FindMatches:
            /**
             * add new find item, contains of multiple columns
             */
            QList<QStandardItem *> items;
            items << new QStandardItem(name);
            items << new QStandardItem(entry.kind ? QString::fromLocal8Bit(entry.kind) : QString());
            items << new QStandardItem(entry.file ? QString::fromLocal8Bit(entry.file) : QString());
            items << new QStandardItem(QString::number(entry.address.lineNumber));
            model.appendRow(items);
            break;
        }
    } while (tagsFindNext(m_ctagsIndexHandle, &entry) == TagSuccess);
}
