/*
    SPDX-FileCopyrightText: 2013 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FolderFilesList_h
#define FolderFilesList_h

#include <QRegularExpression>
#include <QStringList>
#include <QThread>
#include <QVector>

class FolderFilesList : public QThread
{
    Q_OBJECT

public:
    FolderFilesList(QObject *parent = nullptr);
    ~FolderFilesList() override;

    void run() override;

    void generateList(const QString &folder, bool recursive, bool hidden, bool symlinks, const QString &types, const QString &excludes);

    void terminateSearch();

    QStringList fileList();

Q_SIGNALS:
    void searching(const QString &path);
    void fileListReady();

private:
    struct DirectoryWithResults {
        QString directory;
        QStringList newDirectories;
        QStringList newFiles;
    };

    void checkNextItem(DirectoryWithResults &handleOnFolder) const;

private:
    QString m_folder;
    QStringList m_files;
    bool m_cancelSearch = false;

    bool m_recursive = false;
    bool m_hidden = false;
    bool m_symlinks = false;
    QStringList m_types;
    QVector<QRegularExpression> m_excludes;
};

#endif
