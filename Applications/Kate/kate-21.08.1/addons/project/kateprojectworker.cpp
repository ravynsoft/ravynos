/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectworker.h"
#include "kateprojectitem.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QThread>
#include <QTime>
#include <QtConcurrent>

#include <algorithm>

KateProjectWorker::KateProjectWorker(const QString &baseDir, const QString &indexDir, const QVariantMap &projectMap, bool force)
    : m_baseDir(baseDir)
    , m_indexDir(indexDir)
    , m_projectMap(projectMap)
    , m_force(force)
{
    Q_ASSERT(!m_baseDir.isEmpty());
}

void KateProjectWorker::run()
{
    /**
     * Create dummy top level parent item and empty map inside shared pointers
     * then load the project recursively
     */
    KateProjectSharedQStandardItem topLevel(new QStandardItem());
    KateProjectSharedQHashStringItem file2Item(new QHash<QString, KateProjectItem *>());
    loadProject(topLevel.data(), m_projectMap, file2Item.data(), m_baseDir);

    /**
     * sort the stuff once recursively, this is a LOT faster than once sorting the list
     * as we have normally not all stuff in on level of directory
     */
    topLevel->sortChildren(0);

    /**
     * decide if we need to create an index
     * if we need to do so, we will need to create a copy of the file list for later use
     * before this was default on, which is dangerous for large repositories, e.g. out-of-memory or out-of-disk
     * if specified in project map; use that setting, otherwise fall back to global setting
     */
    bool indexEnabled = !m_indexDir.isEmpty();
    const QVariantMap ctagsMap = m_projectMap[QStringLiteral("ctags")].toMap();
    auto indexValue = ctagsMap[QStringLiteral("enable")];
    if (!indexValue.isNull()) {
        indexEnabled = indexValue.toBool();
    }

    /**
     * create some local backup of some data we need for further processing!
     * this is expensive, therefore only really do this if required!
     */
    QStringList files;
    if (indexEnabled) {
        files = file2Item->keys();
    }

    /**
     * hand out our model item & mapping to the main thread
     * that will let Kate already show the project, even before index processing starts
     */
    Q_EMIT loadDone(topLevel, file2Item);

    /**
     * without indexing, we are even done with all stuff here
     */
    if (!indexEnabled) {
        Q_EMIT loadIndexDone(KateProjectSharedProjectIndex());
        return;
    }

    /**
     * create new index, this will do the loading in the constructor
     * wrap it into shared pointer for transfer to main thread
     */
    KateProjectSharedProjectIndex index(new KateProjectIndex(m_baseDir, m_indexDir, files, ctagsMap, m_force));
    Q_EMIT loadIndexDone(index);
}

void KateProjectWorker::loadProject(QStandardItem *parent, const QVariantMap &project, QHash<QString, KateProjectItem *> *file2Item, const QString &baseDir)
{
    /**
     * recurse to sub-projects FIRST
     */
    const QVariantList subGroups = project[QStringLiteral("projects")].toList();
    for (const QVariant &subGroupVariant : subGroups) {
        /**
         * convert to map and get name, else skip
         */
        const QVariantMap subProject = subGroupVariant.toMap();
        const QString keyName = QStringLiteral("name");
        if (subProject[keyName].toString().isEmpty()) {
            continue;
        }

        /**
         * recurse
         */
        QStandardItem *subProjectItem = new KateProjectItem(KateProjectItem::Project, subProject[keyName].toString());
        loadProject(subProjectItem, subProject, file2Item, baseDir);
        parent->appendRow(subProjectItem);
    }

    /**
     * load all specified files
     */
    const QString keyFiles = QStringLiteral("files");
    const QVariantList files = project[keyFiles].toList();
    for (const QVariant &fileVariant : files) {
        loadFilesEntry(parent, fileVariant.toMap(), file2Item, baseDir);
    }
}

/**
 * small helper to construct directory parent items
 * @param dir2Item map for path => item
 * @param path current path we need item for
 * @return correct parent item for given path, will reuse existing ones
 */
QStandardItem *KateProjectWorker::directoryParent(const QDir &base, QHash<QString, QStandardItem *> &dir2Item, QString path)
{
    /**
     * throw away simple /
     */
    if (path == QLatin1String("/")) {
        path = QString();
    }

    /**
     * quick check: dir already seen?
     */
    const auto existingIt = dir2Item.find(path);
    if (existingIt != dir2Item.end()) {
        return existingIt.value();
    }

    /**
     * else: construct recursively
     */
    const int slashIndex = path.lastIndexOf(QLatin1Char('/'));

    /**
     * no slash?
     * simple, no recursion, append new item toplevel
     */
    if (slashIndex < 0) {
        const auto item = new KateProjectItem(KateProjectItem::Directory, path);
        item->setData(base.absoluteFilePath(path), Qt::UserRole);
        dir2Item[path] = item;
        dir2Item[QString()]->appendRow(item);
        return item;
    }

    /**
     * else, split and recurse
     */
    const QString leftPart = path.left(slashIndex);
    const QString rightPart = path.right(path.size() - (slashIndex + 1));

    /**
     * special handling if / with nothing on one side are found
     */
    if (leftPart.isEmpty() || rightPart.isEmpty()) {
        return directoryParent(base, dir2Item, leftPart.isEmpty() ? rightPart : leftPart);
    }

    /**
     * else: recurse on left side
     */
    const auto item = new KateProjectItem(KateProjectItem::Directory, rightPart);
    item->setData(base.absoluteFilePath(path), Qt::UserRole);
    dir2Item[path] = item;
    directoryParent(base, dir2Item, leftPart)->appendRow(item);
    return item;
}

void KateProjectWorker::loadFilesEntry(QStandardItem *parent,
                                       const QVariantMap &filesEntry,
                                       QHash<QString, KateProjectItem *> *file2Item,
                                       const QString &baseDir)
{
    QDir dir(baseDir);
    if (!dir.cd(filesEntry[QStringLiteral("directory")].toString())) {
        return;
    }

    /**
     * handle linked projects, if any
     * one can reference other projects by specifying the path to them
     */
    QStringList linkedProjects = filesEntry[QStringLiteral("projects")].toStringList();
    if (!linkedProjects.empty()) {
        /**
         * ensure project files are made absolute in respect to correct base dir
         */
        for (auto &project : linkedProjects) {
            project = dir.absoluteFilePath(project);
        }

        /**
         * users might have specified duplicates, this can't happen for the other ways
         */
        linkedProjects.removeDuplicates();

        /**
         * filter out all directories that have no .kateproject inside!
         */
        linkedProjects.erase(std::remove_if(linkedProjects.begin(),
                                            linkedProjects.end(),
                                            [](const QString &item) {
                                                const QFileInfo projectFile(item + QLatin1String("/.kateproject"));
                                                return !projectFile.exists() || !projectFile.isFile();
                                            }),
                             linkedProjects.end());

        /**
         * we sort the projects, below we require that we walk them in order:
         * lala
         * lala/test
         * mow
         * mow/test2
         */
        std::sort(linkedProjects.begin(), linkedProjects.end());

        /**
         * now add our projects to the current item parent
         * later the tree view will e.g. allow to jump to the sub-projects
         */
        QHash<QString, QStandardItem *> dir2Item;
        dir2Item[QString()] = parent;
        for (const auto &filePath : linkedProjects) {
            /**
             * cheap file name computation
             * we do this A LOT, QFileInfo is very expensive just for this operation
             */
            const int slashIndex = filePath.lastIndexOf(QLatin1Char('/'));
            const QString fileName = (slashIndex < 0) ? filePath : filePath.mid(slashIndex + 1);
            const QString filePathName = (slashIndex < 0) ? QString() : filePath.left(slashIndex);

            /**
             * construct the item with right directory prefix
             * already hang in directories in tree
             */
            KateProjectItem *fileItem = new KateProjectItem(KateProjectItem::LinkedProject, fileName);
            fileItem->setData(filePath, Qt::UserRole);

            /**
             * projects are directories, register them, we walk in order over the projects
             * even if the nest, toplevel ones would have been done before!
             */
            dir2Item[dir.relativeFilePath(filePath)] = fileItem;

            // get the directory's relative path to the base directory
            QString dirRelPath = dir.relativeFilePath(filePathName);
            // if the relative path is ".", clean it up
            if (dirRelPath == QLatin1Char('.')) {
                dirRelPath = QString();
            }

            // put in our item to the right directory parent
            directoryParent(dir, dir2Item, dirRelPath)->appendRow(fileItem);
        }

        /**
         * files with linked projects will ignore all other stuff inside
         */
        return;
    }

    /**
     * get list of files for this directory, might query the VCS
     */
    QVector<QString> files = findFiles(dir, filesEntry);

    /**
     * sort out non-files
     * even for git, that just reports non-directories, we need to filter out e.g. sym-links to directories
     * we use map, not filter, less locking!
     * we invalidate not matching stuff by clearing the string and skip that then below in processing
     */
    const QString dirPath = dir.path() + QLatin1Char('/');
    QtConcurrent::blockingMap(files, [dirPath](QString &item) {
        if (!QFileInfo(dirPath + item).isFile()) {
            item.clear();
        }
    });

    /**
     * construct paths first in tree and items in a map
     */
    QHash<QString, QStandardItem *> dir2Item;
    dir2Item[QString()] = parent;
    file2Item->reserve(files.size()); // perhaps a bit too much, as we emptied stuff above, but who cares!
    for (const QString &filePath : qAsConst(files)) {
        // skip empty files, that is filtered stuff
        if (filePath.isEmpty()) {
            continue;
        }

        /**
         * cheap file name computation
         * we do this A LOT, QFileInfo is very expensive just for this operation
         */
        const int slashIndex = filePath.lastIndexOf(QLatin1Char('/'));
        const QString fileName = (slashIndex < 0) ? filePath : filePath.mid(slashIndex + 1);
        const QString filePathName = (slashIndex < 0) ? QString() : filePath.left(slashIndex);
        const QString fullFilePath = dirPath + filePath;

        /**
         * construct the item with right directory prefix
         * already hang in directories in tree
         */
        KateProjectItem *fileItem = new KateProjectItem(KateProjectItem::File, fileName);
        fileItem->setData(fullFilePath, Qt::UserRole);
        (*file2Item)[fullFilePath] = fileItem;

        // put in our item to the right directory parent
        directoryParent(dir, dir2Item, filePathName)->appendRow(fileItem);
    }
}

QVector<QString> KateProjectWorker::findFiles(const QDir &dir, const QVariantMap &filesEntry)
{
    /**
     * shall we collect files recursively or not?
     */
    const bool recursive = !filesEntry.contains(QLatin1String("recursive")) || filesEntry[QStringLiteral("recursive")].toBool();

    /**
     * try the different version control systems first
     */

    if (filesEntry[QStringLiteral("git")].toBool()) {
        return filesFromGit(dir, recursive);
    }

    if (filesEntry[QStringLiteral("svn")].toBool()) {
        return filesFromSubversion(dir, recursive);
    }

    if (filesEntry[QStringLiteral("hg")].toBool()) {
        return filesFromMercurial(dir, recursive);
    }

    if (filesEntry[QStringLiteral("darcs")].toBool()) {
        return filesFromDarcs(dir, recursive);
    }

    /**
     * if we arrive here, we have some manual specification of files, no VCS
     */

    /**
     * try explicit list of stuff
     */
    QStringList userGivenFilesList = filesEntry[QStringLiteral("list")].toStringList();
    if (!userGivenFilesList.empty()) {
        /**
         * make the files absolute relative to current dir
         * all code later requires this and the filesFrom... routines do this, too, internally
         * even without this, the tree views will show them, but opening them will create new elements!
         */
        for (auto &file : userGivenFilesList) {
            file = dir.absoluteFilePath(file);
        }

        /**
         * users might have specified duplicates, this can't happen for the other ways
         */
        userGivenFilesList.removeDuplicates();
        return userGivenFilesList.toVector();
    }

    /**
     * if nothing found for that, try to use filters to scan the directory
     * here we only get files
     */
    return filesFromDirectory(dir, recursive, filesEntry[QStringLiteral("filters")].toStringList());
}

QVector<QString> KateProjectWorker::filesFromGit(const QDir &dir, bool recursive)
{
    /**
     * query files via ls-files and make them absolute afterwards
     */

    /**
     * git ls-files -z results a bytearray where each entry is \0-terminated.
     * NOTE: Without -z, Umlauts such as "Der Bäcker/Das Brötchen.txt" do not work (#389415)
     *
     * use --recurse-submodules, there since git 2.11 (released 2016)
     * our own submodules handling code leads to file duplicates
     */
    const QStringList lsFilesArgs{QStringLiteral("ls-files"), QStringLiteral("-z"), QStringLiteral("--recurse-submodules"), QStringLiteral(".")};

    /**
     * ls-files untracked
     */
    const QStringList lsFilesUntrackedArgs{QStringLiteral("ls-files"),
                                           QStringLiteral("-z"),
                                           QStringLiteral("--others"),
                                           QStringLiteral("--exclude-standard"),
                                           QStringLiteral(".")};

    // ls-files + ls-files untracked
    return gitFiles(dir, recursive, lsFilesArgs) << gitFiles(dir, recursive, lsFilesUntrackedArgs);
}

QVector<QString> KateProjectWorker::gitFiles(const QDir &dir, bool recursive, const QStringList &args)
{
    QProcess git;
    git.setWorkingDirectory(dir.absolutePath());
    git.start(QStringLiteral("git"), args, QProcess::ReadOnly);
    QVector<QString> files;
    if (!git.waitForStarted() || !git.waitForFinished(-1)) {
        return files;
    }

    const QString dirAbsoloutePath = dir.absolutePath() + QLatin1Char('/');

    const QList<QByteArray> byteArrayList = git.readAllStandardOutput().split('\0');
    files.reserve(byteArrayList.size());
    for (const QByteArray &byteArray : byteArrayList) {
        if (byteArray.isEmpty()) {
            continue;
        }
        if (!recursive && (byteArray.indexOf('/') != -1)) {
            continue;
        }
        files.append(QString::fromUtf8(byteArray));
    }
    return files;
}

QVector<QString> KateProjectWorker::filesFromMercurial(const QDir &dir, bool recursive)
{
    QVector<QString> files;

    QProcess hg;
    hg.setWorkingDirectory(dir.absolutePath());
    QStringList args;
    args << QStringLiteral("manifest") << QStringLiteral(".");
    hg.start(QStringLiteral("hg"), args, QProcess::ReadOnly);
    if (!hg.waitForStarted() || !hg.waitForFinished(-1)) {
        return files;
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const QStringList relFiles =
        QString::fromLocal8Bit(hg.readAllStandardOutput()).split(QRegularExpression(QStringLiteral("[\n\r]")), QString::SkipEmptyParts);
#else
    const QStringList relFiles = QString::fromLocal8Bit(hg.readAllStandardOutput()).split(QRegularExpression(QStringLiteral("[\n\r]")), Qt::SkipEmptyParts);
#endif

    files.reserve(relFiles.size());
    for (const QString &relFile : relFiles) {
        if (!recursive && (relFile.indexOf(QLatin1Char('/')) != -1)) {
            continue;
        }

        files.append(relFile);
    }

    return files;
}

QVector<QString> KateProjectWorker::filesFromSubversion(const QDir &dir, bool recursive)
{
    QVector<QString> files;

    QProcess svn;
    svn.setWorkingDirectory(dir.absolutePath());
    QStringList args;
    args << QStringLiteral("status") << QStringLiteral("--verbose") << QStringLiteral(".");
    if (recursive) {
        args << QStringLiteral("--depth=infinity");
    } else {
        args << QStringLiteral("--depth=files");
    }
    svn.start(QStringLiteral("svn"), args, QProcess::ReadOnly);
    if (!svn.waitForStarted() || !svn.waitForFinished(-1)) {
        return files;
    }

    /**
     * get output and split up into lines
     */
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const QStringList lines = QString::fromLocal8Bit(svn.readAllStandardOutput()).split(QRegularExpression(QStringLiteral("[\n\r]")), QString::SkipEmptyParts);
#else
    const QStringList lines = QString::fromLocal8Bit(svn.readAllStandardOutput()).split(QRegularExpression(QStringLiteral("[\n\r]")), Qt::SkipEmptyParts);
#endif

    /**
     * remove start of line that is no filename, sort out unknown and ignore
     */
    bool first = true;
    int prefixLength = -1;

    files.reserve(lines.size());
    for (const QString &line : lines) {
        /**
         * get length of stuff to cut
         */
        if (first) {
            /**
             * try to find ., else fail
             */
            prefixLength = line.lastIndexOf(QLatin1Char('.'));
            if (prefixLength < 0) {
                break;
            }

            /**
             * skip first
             */
            first = false;
            continue;
        }

        /**
         * get file, if not unknown or ignored
         * prepend directory path
         */
        if ((line.size() > prefixLength) && line[0] != QLatin1Char('?') && line[0] != QLatin1Char('I')) {
            files.append(line.right(line.size() - prefixLength));
        }
    }

    return files;
}

QVector<QString> KateProjectWorker::filesFromDarcs(const QDir &dir, bool recursive)
{
    QVector<QString> files;

    const QString cmd = QStringLiteral("darcs");
    QString root;

    {
        QProcess darcs;
        darcs.setWorkingDirectory(dir.absolutePath());
        QStringList args;
        args << QStringLiteral("list") << QStringLiteral("repo");

        darcs.start(cmd, args, QProcess::ReadOnly);

        if (!darcs.waitForStarted() || !darcs.waitForFinished(-1)) {
            return files;
        }

        auto str = QString::fromLocal8Bit(darcs.readAllStandardOutput());
        QRegularExpression exp(QStringLiteral("Root: ([^\\n\\r]*)"));
        auto match = exp.match(str);

        if (!match.hasMatch()) {
            return files;
        }

        root = match.captured(1);
    }

    QStringList relFiles;
    {
        QProcess darcs;
        QStringList args;
        darcs.setWorkingDirectory(dir.absolutePath());
        args << QStringLiteral("list") << QStringLiteral("files") << QStringLiteral("--no-directories") << QStringLiteral("--pending");

        darcs.start(cmd, args, QProcess::ReadOnly);

        if (!darcs.waitForStarted() || !darcs.waitForFinished(-1)) {
            return files;
        }

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        relFiles = QString::fromLocal8Bit(darcs.readAllStandardOutput()).split(QRegularExpression(QStringLiteral("[\n\r]")), QString::SkipEmptyParts);
#else
        relFiles = QString::fromLocal8Bit(darcs.readAllStandardOutput()).split(QRegularExpression(QStringLiteral("[\n\r]")), Qt::SkipEmptyParts);
#endif
    }

    files.reserve(relFiles.size());
    for (const QString &relFile : relFiles) {
        const QString path = dir.relativeFilePath(root + QLatin1String("/") + relFile);

        if ((!recursive && (relFile.indexOf(QLatin1Char('/')) != -1)) || (recursive && (relFile.indexOf(QLatin1String("..")) == 0))) {
            continue;
        }

        files.append(path);
    }

    return files;
}

QVector<QString> KateProjectWorker::filesFromDirectory(const QDir &_dir, bool recursive, const QStringList &filters)
{
    /**
     * setup our filters, we only want files!
     */
    QDir dir(_dir);
    dir.setFilter(QDir::Files);
    if (!filters.isEmpty()) {
        dir.setNameFilters(filters);
    }

    /**
     * construct flags for iterator
     */
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags;
    if (recursive) {
        flags = flags | QDirIterator::Subdirectories;
    }

    /**
     * create iterator and collect all files
     */
    QVector<QString> files;
    QDirIterator dirIterator(dir, flags);
    const QString dirPath = dir.path() + QLatin1Char('/');
    while (dirIterator.hasNext()) {
        dirIterator.next();
        // make it relative path
        files.append(dirIterator.filePath().remove(dirPath));
    }
    return files;
}
