/* This file is part of the KDE project
   Copyright (C) xxxx KFile Authors
   SPDX-FileCopyrightText: 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
   SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
   SPDX-FileCopyrightText: 2009 Dominik Haumann <dhaumann kde org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KATE_BOOKMARK_HANDLER_H
#define KATE_BOOKMARK_HANDLER_H

#include <KBookmarkManager>
#include <KBookmarkMenu>

class KateFileBrowser;
class QMenu;

class KateBookmarkHandler : public QObject, public KBookmarkOwner
{
    Q_OBJECT

public:
    explicit KateBookmarkHandler(KateFileBrowser *parent, QMenu *kpopupmenu = nullptr);
    ~KateBookmarkHandler() override;

    // KBookmarkOwner interface:
    QUrl currentUrl() const override;
    QString currentTitle() const override;

    QMenu *menu() const
    {
        return m_menu;
    }
    void openBookmark(const KBookmark &, Qt::MouseButtons, Qt::KeyboardModifiers) override;

Q_SIGNALS:
    void openUrl(const QString &url);

private:
    KateFileBrowser *mParent;
    QMenu *m_menu;
    KBookmarkMenu *m_bookmarkMenu;
};

#endif // KATE_BOOKMARK_HANDLER_H
// kate: space-indent on; indent-width 2; replace-tabs on;
