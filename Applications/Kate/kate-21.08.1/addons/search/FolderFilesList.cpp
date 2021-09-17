/*
    SPDX-FileCopyrightText: 2013 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "FolderFilesList.h"

#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfoList>
#include <QtConcurrent>

#include <unordered_set>
#include <vector>

FolderFilesList::FolderFilesList(QObject *parent)
    : QThread(parent)
{
    // ensure we have a proper thread name during e.g. perf profiling
    setObjectName(QStringLiteral("FolderFilesList"));
}

FolderFilesList::~FolderFilesList()
{
    m_cancelSearch = true;
    wait();
}

void FolderFilesList::run()
{
    m_files.clear();

    /**
     * iterative algorithm, in each round, we put in X directories to traverse
     * we will get as output X times: new directories + found files
     */
    std::vector<DirectoryWithResults> directoriesWithResults{DirectoryWithResults{m_folder, QStringList(), QStringList()}};
    std::unordered_set<QString> directoryGuard{m_folder};
    QElapsedTimer time;
    time.start();
    while (!directoriesWithResults.empty()) {
        /**
         * all 100 ms => inform about progress
         */
        if (time.elapsed() > 100) {
            time.restart();
            Q_EMIT searching(directoriesWithResults[0].directory);
        }

        /**
         * map the stuff blocking, we are in a background thread, easiest way
         * this will just span ideal thread count many things while this thread more or less sleeps
         * we call the checkNextItem member function, that one must be careful to not do evil things ;=)
         */
        QtConcurrent::blockingMap(directoriesWithResults, [this](DirectoryWithResults &item) {
            checkNextItem(item);
        });

        /**
         * collect the results to create new worklist for next round
         */
        std::vector<DirectoryWithResults> nextRound;
        for (const auto &result : directoriesWithResults) {
            /**
             * one new item for the next round for each new directory
             */
            for (const auto &newDirectory : result.newDirectories) {
                if (directoryGuard.insert(newDirectory).second) {
                    nextRound.push_back(DirectoryWithResults{newDirectory, QStringList(), QStringList()});
                }
            }

            /**
             * just append found files
             */
            m_files << result.newFiles;
        }

        /**
         * let's get next round going
         */
        directoriesWithResults = nextRound;
    }

    if (m_cancelSearch) {
        m_files.clear();
    }
    Q_EMIT fileListReady();
}

void FolderFilesList::generateList(const QString &folder, bool recursive, bool hidden, bool symlinks, const QString &types, const QString &excludes)
{
    m_cancelSearch = false;
    m_folder = folder;
    if (!m_folder.endsWith(QLatin1Char('/'))) {
        m_folder += QLatin1Char('/');
    }
    m_recursive = recursive;
    m_hidden = hidden;
    m_symlinks = symlinks;

    m_types.clear();
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const auto typesList = types.split(QLatin1Char(','), QString::SkipEmptyParts);
#else
    const auto typesList = types.split(QLatin1Char(','), Qt::SkipEmptyParts);
#endif
    for (const QString &type : typesList) {
        m_types << type.trimmed();
    }
    if (m_types.isEmpty()) {
        m_types << QStringLiteral("*");
    }

    QStringList tmpExcludes = excludes.split(QLatin1Char(','));
    m_excludes.clear();
    for (int i = 0; i < tmpExcludes.size(); i++) {
        m_excludes << QRegularExpression(QRegularExpression::wildcardToRegularExpression(tmpExcludes[i].trimmed()));
    }

    start();
}

void FolderFilesList::terminateSearch()
{
    m_cancelSearch = true;
    wait();
    m_files.clear();
}

QStringList FolderFilesList::fileList()
{
    if (m_cancelSearch) {
        m_files.clear();
    }
    return m_files;
}

void FolderFilesList::checkNextItem(DirectoryWithResults &handleOnFolder) const
{
    /**
     * IMPORTANT: this member function is called by MULTIPLE THREADS
     * => it is const, it shall only modify handleOnFolder
     */

    if (m_cancelSearch) {
        return;
    }

    QDir currentDir(handleOnFolder.directory);
    if (!currentDir.isReadable()) {
        // qDebug() << currentDir.absolutePath() << "Not readable";
        return;
    }

    QDir::Filters filter = QDir::Files | QDir::NoDotAndDotDot | QDir::Readable;
    if (m_hidden) {
        filter |= QDir::Hidden;
    }
    if (m_recursive) {
        filter |= QDir::AllDirs;
    }
    if (!m_symlinks) {
        filter |= QDir::NoSymLinks;
    }

    const QFileInfoList entries = currentDir.entryInfoList(m_types, filter, QDir::Name | QDir::LocaleAware);
    for (const auto &entry : entries) {
        const QString absFilePath = entry.absoluteFilePath();
        bool skip{false};
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        const QStringList pathSplit = absFilePath.split(QLatin1Char('/'), QString::SkipEmptyParts);
#else
        const QStringList pathSplit = absFilePath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
#endif
        for (const auto &regex : m_excludes) {
            for (const auto &part : pathSplit) {
                QRegularExpressionMatch match = regex.match(part);
                if (match.hasMatch()) {
                    skip = true;
                    break;
                }
            }
        }
        if (skip) {
            continue;
        }

        if (entry.isDir()) {
            handleOnFolder.newDirectories.append(absFilePath);
        }

        if (entry.isFile()) {
            handleOnFolder.newFiles.append(absFilePath);
        }
    }
}
