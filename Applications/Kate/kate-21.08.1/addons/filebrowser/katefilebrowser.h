/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>
   SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
   SPDX-FileCopyrightText: 2009 Dominik Haumann <dhaumann kde org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KATE_FILEBROWSER_H
#define KATE_FILEBROWSER_H

#include <ktexteditor/mainwindow.h>

#include <KFile>

#include <QMenu>
#include <QUrl>
#include <QWidget>

#include "katefilebrowseropenwithmenu.h"

class KateBookmarkHandler;
class KActionCollection;
class KDirOperator;
class KFileItem;
class KHistoryComboBox;
class KToolBar;
class KConfigGroup;
class KUrlNavigator;

class QAbstractItemView;
class QAction;

/*
    The kate file selector presents a directory view, in which the default action is
    to open the activated file.
    Additionally, a toolbar for managing the kdiroperator widget + sync that to
    the directory of the current file is available, as well as a filter widget
    allowing to filter the displayed files using a name filter.
*/

class KateFileBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit KateFileBrowser(KTextEditor::MainWindow *mainWindow = nullptr, QWidget *parent = nullptr);
    ~KateFileBrowser() override;

    void readSessionConfig(const KConfigGroup &config);
    void writeSessionConfig(KConfigGroup &config);

    void setupToolbar();
    void setView(KFile::FileView);
    KDirOperator *dirOperator()
    {
        return m_dirOperator;
    }

    KActionCollection *actionCollection()
    {
        return m_actionCollection;
    }

public Q_SLOTS:
    void slotFilterChange(const QString &);
    void setDir(const QUrl &);
    void setDir(const QString &url)
    {
        setDir(QUrl(url));
    }
    void selectorViewChanged(QAbstractItemView *);

private Q_SLOTS:
    void fileSelected(const KFileItem & /*file*/);
    void updateDirOperator(const QUrl &u);
    void updateUrlNavigator(const QUrl &u);
    void setActiveDocumentDir();
    void autoSyncFolder();
    void contextMenuAboutToShow(const KFileItem &item, QMenu *menu);
    void fixOpenWithMenu();
    void openWithMenuAction(QAction *a);

protected:
    QUrl activeDocumentUrl();
    void openSelectedFiles();
    void setupActions();

public:
    KTextEditor::MainWindow *mainWindow()
    {
        return m_mainWindow;
    }

private:
    KToolBar *m_toolbar;
    KActionCollection *m_actionCollection;
    KateBookmarkHandler *m_bookmarkHandler = nullptr;
    KUrlNavigator *m_urlNavigator;
    KDirOperator *m_dirOperator;
    KHistoryComboBox *m_filter;
    QAction *m_autoSyncFolder = nullptr;
    KateFileBrowserOpenWithMenu *m_openWithMenu = nullptr;

    KTextEditor::MainWindow *m_mainWindow;
};

#endif // KATE_FILEBROWSER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
