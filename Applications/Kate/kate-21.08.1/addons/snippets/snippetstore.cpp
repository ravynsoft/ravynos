/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
 *  SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "snippetstore.h"

#include "katesnippetglobal.h"
#include "snippetrepository.h"

#include <QDir>
#include <QStandardPaths>

#include <KSharedConfig>

#include <ktexteditor/editor.h>

Q_DECLARE_METATYPE(KSharedConfig::Ptr)

SnippetStore *SnippetStore::m_self = nullptr;

SnippetStore::SnippetStore(KateSnippetGlobal *plugin)
    : m_plugin(plugin)
{
    m_self = this;

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, //
                                                       QStringLiteral("ktexteditor_snippets/data"),
                                                       QStandardPaths::LocateDirectory)
        << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("ktexteditor_snippets/ghns"), QStandardPaths::LocateDirectory);

    QStringList files;
    for (const QString &dir : dirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.xml"));
        for (const QString &file : fileNames) {
            files.append(dir + QLatin1Char('/') + file);
        }
    }

    for (const QString &file : qAsConst(files)) {
        SnippetRepository *repo = new SnippetRepository(file);
        appendRow(repo);
    }
}

SnippetStore::~SnippetStore()
{
    invisibleRootItem()->removeRows(0, invisibleRootItem()->rowCount());
    m_self = nullptr;
}

void SnippetStore::init(KateSnippetGlobal *plugin)
{
    Q_ASSERT(!SnippetStore::self());
    new SnippetStore(plugin);
}

SnippetStore *SnippetStore::self()
{
    return m_self;
}

Qt::ItemFlags SnippetStore::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    if (!index.parent().isValid()) {
        flags |= Qt::ItemIsUserCheckable;
    }
    return flags;
}

KConfigGroup SnippetStore::getConfig()
{
    return KSharedConfig::openConfig()->group("Snippets");
}

bool SnippetStore::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && value.toString().isEmpty()) {
        // don't allow empty names
        return false;
    }
    if (value == data(index, role)) {
        // if unchanged, avoid saving
        return true;
    }
    const bool success = QStandardItemModel::setData(index, value, role);
    if (!success || role != Qt::EditRole) {
        return success;
    }

    // when we edited something, save the repository

    QStandardItem *repoItem = nullptr;
    if (index.parent().isValid()) {
        repoItem = itemFromIndex(index.parent());
    } else {
        repoItem = itemFromIndex(index);
    }

    SnippetRepository *repo = dynamic_cast<SnippetRepository *>(repoItem);
    if (repo) {
        repo->save();
    }
    return true;
}

SnippetRepository *SnippetStore::repositoryForFile(const QString &file)
{
    for (int i = 0; i < rowCount(); ++i) {
        if (SnippetRepository *repo = dynamic_cast<SnippetRepository *>(item(i))) {
            if (repo->file() == file) {
                return repo;
            }
        }
    }
    return nullptr;
}
