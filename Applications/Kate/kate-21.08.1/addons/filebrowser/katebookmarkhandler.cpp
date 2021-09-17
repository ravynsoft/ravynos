/* This file is part of the KDE project
   Copyright (C) xxxx KFile Authors
   SPDX-FileCopyrightText: 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
   SPDX-FileCopyrightText: 2009 Dominik Haumann <dhaumann kde org>
   SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "katebookmarkhandler.h"
#include "katefilebrowser.h"

#include <KActionCollection>
#include <KDirOperator>

#include <QMenu>
#include <QStandardPaths>

KateBookmarkHandler::KateBookmarkHandler(KateFileBrowser *parent, QMenu *kpopupmenu)
    : QObject(parent)
    , mParent(parent)
    , m_menu(kpopupmenu)
{
    setObjectName(QStringLiteral("KateBookmarkHandler"));
    if (!m_menu) {
        m_menu = new QMenu(parent);
    }

    QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kate/fsbookmarks.xml"));
    if (file.isEmpty()) {
        file = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kate/fsbookmarks.xml");
    }

    KBookmarkManager *manager = KBookmarkManager::managerForFile(file, QStringLiteral("kate"));
    manager->setUpdate(true);

    m_bookmarkMenu = new KBookmarkMenu(manager, this, m_menu);

    KActionCollection *ac = parent->actionCollection();
    if (QAction *addBookmarkAction = m_bookmarkMenu->addBookmarkAction()) {
        ac->addAction(addBookmarkAction->objectName(), addBookmarkAction);
    }
    if (QAction *newBookmarkFolderAction = m_bookmarkMenu->newBookmarkFolderAction()) {
        ac->addAction(newBookmarkFolderAction->objectName(), newBookmarkFolderAction);
    }
    if (QAction *editBookmarksAction = m_bookmarkMenu->editBookmarksAction()) {
        ac->addAction(editBookmarksAction->objectName(), editBookmarksAction);
    }
}

KateBookmarkHandler::~KateBookmarkHandler()
{
    delete m_bookmarkMenu;
}

QUrl KateBookmarkHandler::currentUrl() const
{
    return mParent->dirOperator()->url();
}

QString KateBookmarkHandler::currentTitle() const
{
    return currentUrl().url();
}

void KateBookmarkHandler::openBookmark(const KBookmark &bm, Qt::MouseButtons, Qt::KeyboardModifiers)
{
    Q_EMIT openUrl(bm.url().url());
}

// kate: space-indent on; indent-width 2; replace-tabs on;
