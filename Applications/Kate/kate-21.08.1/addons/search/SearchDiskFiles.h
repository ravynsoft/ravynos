/*
    SPDX-FileCopyrightText: 2011-21 Kåre Särs <kare.sars@iki.fi>
    SPDX-FileCopyrightText: 2021 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SearchDiskFiles_h
#define SearchDiskFiles_h

// Qt
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QRegularExpression>
#include <QRunnable>
#include <QStringList>

// std
#include <atomic>

// locals
#include "MatchModel.h"

class QString;
class QUrl;
class QFile;

/**
 * Thread-safe worklist to feed the SearchDiskFiles runnables.
 */
class SearchDiskFilesWorkList
{
public:
    /**
     * Default constructor => nothing to be done
     */
    SearchDiskFilesWorkList() = default;

    /**
     * Any workers running?
     * @return any worker running?
     */
    bool isRunning()
    {
        QMutexLocker lock(&m_mutex);
        return m_currentRunningRunnables > 0;
    }

    /**
     * Search canceled?
     * @return canceled?
     */
    bool isCanceled()
    {
        return m_canceled;
    }

    /**
     * Init the search, shall only be done if not running.
     * @param files files to search
     * @param numberOfWorkers number of workers we will spawn
     */
    void init(const QStringList &files, int numberOfWorkers)
    {
        /**
         * ensure sane initial state: last search is done!
         * should hold even if canceled, the last markOnRunnableAsDone clears all fields!
         */
        QMutexLocker lock(&m_mutex);
        Q_ASSERT(m_currentRunningRunnables == 0);
        Q_ASSERT(m_filesToSearch.isEmpty());
        Q_ASSERT(m_filesToSearchIndex == 0);

        /**
         * we shall not be called without any work!
         */
        Q_ASSERT(!files.isEmpty());
        Q_ASSERT(numberOfWorkers > 0);

        /**
         * init work
         */
        m_currentRunningRunnables = numberOfWorkers;
        m_filesToSearch = files;
        m_filesToSearchIndex = 0;
        m_canceled = false;
    }

    /**
     * Get one file to search if still some there.
     * Will return empty string if no further work (or canceled)
     * @return file to search next or empty string
     */
    QString nextFileToSearch()
    {
        QMutexLocker lock(&m_mutex);
        if (m_filesToSearchIndex >= m_filesToSearch.size()) {
            return QString();
        }

        // else return file, shall not be empty and advance one file
        const auto file = m_filesToSearch.at(m_filesToSearchIndex);
        Q_ASSERT(!file.isEmpty());
        ++m_filesToSearchIndex;
        return file;
    }

    /**
     * Mark one runnable as done.
     */
    void markOnRunnableAsDone()
    {
        QMutexLocker lock(&m_mutex);
        Q_ASSERT(m_currentRunningRunnables > 0);
        --m_currentRunningRunnables;

        // if we are done, cleanup
        if (m_currentRunningRunnables == 0) {
            m_filesToSearch.clear();
            m_filesToSearchIndex = 0;
        }
    }

    /**
     * Cancel the work.
     */
    void cancel()
    {
        QMutexLocker lock(&m_mutex);
        m_canceled = true;
        m_filesToSearch.clear();
        m_filesToSearchIndex = 0;
    }

private:
    /**
     * non-recursive mutex to lock internals, only locked a very short time
     */
    QMutex m_mutex;

    /**
     * current number of still active runnables, if == 0 => nothing running
     */
    int m_currentRunningRunnables{0}; // guarded by m_mutex

    /**
     * worklist => files to search in on the disk
     */
    QStringList m_filesToSearch; // guarded by m_mutex

    /**
     * current index into the worklist => next file to search
     * we don't do modify the stringlist, we just move the index
     */
    int m_filesToSearchIndex{0}; // guarded by m_mutex

    /**
     * was the search canceled?
     */
    std::atomic_bool m_canceled{false};
};

class SearchDiskFiles : public QObject, public QRunnable
{
    Q_OBJECT

public:
    SearchDiskFiles(SearchDiskFilesWorkList &worklist, const QRegularExpression &regexp, const bool includeBinaryFiles);

    void run() override;

Q_SIGNALS:
    void matchesFound(const QUrl &url, const QVector<KateSearchMatch> &searchMatches);

private:
    QVector<KateSearchMatch> searchSingleLineRegExp(QFile &file);
    QVector<KateSearchMatch> searchMultiLineRegExp(QFile &file);

private:
    SearchDiskFilesWorkList &m_worklist;
    const QRegularExpression m_regExp;
    bool m_includeBinaryFiles = false;
};

#endif
