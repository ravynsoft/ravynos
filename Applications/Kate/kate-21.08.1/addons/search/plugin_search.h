/*
    SPDX-FileCopyrightText: 2011-21 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _PLUGIN_SEARCH_H_
#define _PLUGIN_SEARCH_H_

#include <KTextEditor/Command>
#include <KTextEditor/Message>
#include <KTextEditor/Plugin>
#include <QAction>
#include <ktexteditor/application.h>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/sessionconfiginterface.h>

#include <QThreadPool>
#include <QTimer>
#include <QTreeView>

#include <KXMLGUIClient>

#include "ui_results.h"
#include "ui_search.h"

#include "FolderFilesList.h"
#include "MatchModel.h"
#include "SearchDiskFiles.h"
#include "search_open_files.h"

class KateSearchCommand;
class QPoint;
namespace KTextEditor
{
class MovingRange;
class MovingInterface;
}

class Results : public QWidget, public Ui::Results
{
    Q_OBJECT
public:
    Results(QWidget *parent = nullptr);
    int matches = 0;
    QRegularExpression regExp;
    bool useRegExp = false;
    bool matchCase = false;
    QString replaceStr;
    int searchPlaceIndex = 0;
    QString treeRootText;
    MatchModel matchModel;

Q_SIGNALS:
    void colorsChanged();
};

// This class keeps the focus inside the S&R plugin when pressing tab/shift+tab by overriding focusNextPrevChild()
class ContainerWidget : public QWidget
{
    Q_OBJECT
public:
    ContainerWidget(QWidget *parent)
        : QWidget(parent)
    {
    }

Q_SIGNALS:
    void nextFocus(QWidget *currentWidget, bool *found, bool next);

protected:
    bool focusNextPrevChild(bool next) override;
};

class KatePluginSearch : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit KatePluginSearch(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());
    ~KatePluginSearch() override;

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

private:
    KateSearchCommand *m_searchCommand = nullptr;
};

class KatePluginSearchView : public QObject, public KXMLGUIClient, public KTextEditor::SessionConfigInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::SessionConfigInterface)

public:
    KatePluginSearchView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainWindow, KTextEditor::Application *application);
    ~KatePluginSearchView() override;

    void readSessionConfig(const KConfigGroup &config) override;
    void writeSessionConfig(KConfigGroup &config) override;

public Q_SLOTS:
    void stopClicked();
    void startSearch();
    void setSearchString(const QString &pattern);
    void navigateFolderUp();
    void setCurrentFolder();
    void setSearchPlace(int place);
    void goToNextMatch();
    void goToPreviousMatch();
    void setRegexMode(bool enabled);
    void setCaseInsensitive(bool enabled);
    void setExpandResults(bool enabled);

private:
    enum CopyResultType { AllExpanded, All };

private Q_SLOTS:
    void openSearchView();
    void handleEsc(QEvent *e);
    void nextFocus(QWidget *currentWidget, bool *found, bool next);

    void addTab();
    void tabCloseRequested(int index);
    void toggleOptions(bool show);

    void searchContextMenu(const QPoint &pos);
    void replaceContextMenu(const QPoint &pos);

    void searchPlaceChanged();
    void startSearchWhileTyping();

    void folderFileListChanged();

    void matchesFound(const QUrl &url, const QVector<KateSearchMatch> &searchMatches);

    void addRangeAndMark(KTextEditor::Document *doc, const KateSearchMatch &match, KTextEditor::Attribute::Ptr attr, KTextEditor::MovingInterface *miface);

    void searchDone();
    void searchWhileTypingDone();
    void indicateMatch(bool hasMatch);

    void itemSelected(const QModelIndex &item);

    void clearMarksAndRanges();
    void clearDocMarksAndRanges(KTextEditor::Document *doc);

    void replaceSingleMatch();
    void replaceChecked();

    void replaceDone();

    void updateCheckState(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void updateMatchMarks();

    void syncModelRanges();

    void resultTabChanged(int index);

    void expandResults();

    /**
     * keep track if the project plugin is alive and if the project file did change
     */
    void slotPluginViewCreated(const QString &name, QObject *pluginView);
    void slotPluginViewDeleted(const QString &name, QObject *pluginView);
    void slotProjectFileNameChanged();

    void copySearchToClipboard(CopyResultType type);
    void customResMenuRequested(const QPoint &pos);

Q_SIGNALS:
    void searchBusy(bool busy);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    QStringList filterFiles(const QStringList &fileList) const;
    void startDiskFileSearch(const QStringList &fileList, const QRegularExpression &reg, bool includeBinaryFiles);
    void cancelDiskFileSearch();
    bool searchingDiskFiles();

    void updateViewColors();

    void onResize(const QSize &size);

    Ui::SearchDialog m_ui{};
    QWidget *m_toolView;
    KTextEditor::Application *m_kateApp;
    SearchOpenFiles m_searchOpenFiles;
    FolderFilesList m_folderFilesList;

    /**
     * worklist for runnables, must survive thread pool below!
     */
    SearchDiskFilesWorkList m_worklistForDiskFiles;

    /**
     * threadpool for multi-threaded disk search
     */
    QThreadPool m_searchDiskFilePool;

    QTimer m_diskSearchDoneTimer;
    QTimer m_updateCheckedStateTimer;
    QAction *m_matchCase = nullptr;
    QAction *m_useRegExp = nullptr;
    Results *m_curResults = nullptr;
    bool m_searchJustOpened = false;
    int m_projectSearchPlaceIndex = 0;
    bool m_isSearchAsYouType = false;
    bool m_isVerticalLayout = false;
    QString m_resultBaseDir;
    QVector<KTextEditor::MovingRange *> m_matchRanges;
    QTimer m_changeTimer;
    QPointer<KTextEditor::Message> m_infoMessage;
    QColor m_replaceHighlightColor;
    KTextEditor::Attribute::Ptr m_resultAttr;

    /**
     * current project plugin view, if any
     */
    QObject *m_projectPluginView = nullptr;

    /**
     * our main window
     */
    KTextEditor::MainWindow *m_mainWindow;
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
