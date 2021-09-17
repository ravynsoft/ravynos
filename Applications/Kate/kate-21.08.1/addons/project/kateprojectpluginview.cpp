/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectpluginview.h"
#include "fileutil.h"
#include "gitwidget.h"
#include "kateproject.h"
#include "kateprojectinfoview.h"
#include "kateprojectinfoviewindex.h"
#include "kateprojectplugin.h"
#include "kateprojectview.h"

#include <KTextEditor/Command>
#include <ktexteditor/application.h>
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>

#include <KAboutData>
#include <KActionCollection>
#include <KActionMenu>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KStringHandler>
#include <KXMLGUIFactory>

#include <QAction>
#include <QDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMenu>
#include <QTimer>
#include <QVBoxLayout>

#define PROJECTCLOSEICON "window-close"

K_PLUGIN_FACTORY_WITH_JSON(KateProjectPluginFactory, "kateprojectplugin.json", registerPlugin<KateProjectPlugin>();)

KateProjectPluginView::KateProjectPluginView(KateProjectPlugin *plugin, KTextEditor::MainWindow *mainWin)
    : QObject(mainWin)
    , m_plugin(plugin)
    , m_mainWindow(mainWin)
    , m_toolView(nullptr)
    , m_toolInfoView(nullptr)
    , m_toolMultiView(nullptr)
    , m_lookupAction(nullptr)
    , m_gotoSymbolAction(nullptr)
    , m_gotoSymbolActionAppMenu(nullptr)
{
    KXMLGUIClient::setComponentName(QStringLiteral("kateproject"), i18n("Kate Project Manager"));
    setXMLFile(QStringLiteral("ui.rc"));

    /**
     * create toolviews
     */
    m_toolView = m_mainWindow->createToolView(m_plugin,
                                              QStringLiteral("kateproject"),
                                              KTextEditor::MainWindow::Left,
                                              QIcon::fromTheme(QStringLiteral("project-open")),
                                              i18n("Projects"));
    m_gitToolView.reset(m_mainWindow->createToolView(m_plugin,
                                                     QStringLiteral("kateprojectgit"),
                                                     KTextEditor::MainWindow::Left,
                                                     QIcon(QStringLiteral(":/icons/icons/sc-apps-git.svg")),
                                                     i18n("Git")));
    m_toolInfoView = m_mainWindow->createToolView(m_plugin,
                                                  QStringLiteral("kateprojectinfo"),
                                                  KTextEditor::MainWindow::Bottom,
                                                  QIcon::fromTheme(QStringLiteral("view-choose")),
                                                  i18n("Current Project"));

    /**
     * create the combo + buttons for the toolViews + stacked widgets
     */
    m_projectsCombo = new QComboBox(m_toolView);
    m_projectsCombo->setFrame(false);
    m_reloadButton = new QToolButton(m_toolView);
    m_reloadButton->setAutoRaise(true);
    m_reloadButton->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    m_closeProjectButton = new QToolButton(m_toolView);
    m_closeProjectButton->setAutoRaise(true);
    m_closeProjectButton->setIcon(QIcon::fromTheme(QStringLiteral(PROJECTCLOSEICON)));
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->addWidget(m_projectsCombo);
    layout->addWidget(m_reloadButton);
    layout->addWidget(m_closeProjectButton);
    m_toolView->layout()->addItem(layout);
    m_toolView->layout()->setSpacing(0);

    m_projectsComboGit = new QComboBox(m_gitToolView.get());
    m_projectsComboGit->setFrame(false);
    m_gitStatusRefreshButton = new QToolButton(m_gitToolView.get());
    m_gitStatusRefreshButton->setAutoRaise(true);
    m_gitStatusRefreshButton->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->addWidget(m_projectsComboGit);
    layout->addWidget(m_gitStatusRefreshButton);
    m_gitToolView->layout()->addItem(layout);
    m_gitToolView->layout()->setSpacing(0);

    m_stackedProjectViews = new QStackedWidget(m_toolView);
    m_stackedProjectInfoViews = new QStackedWidget(m_toolInfoView);
    m_stackedgitViews = new QStackedWidget(m_gitToolView.get());

    connect(m_projectsCombo,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            m_projectsComboGit,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged));
    connect(m_projectsComboGit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_projectsCombo->setCurrentIndex(index);
    });
    connect(m_projectsCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &KateProjectPluginView::slotCurrentChanged);
    connect(m_reloadButton, &QToolButton::clicked, this, &KateProjectPluginView::slotProjectReload);

    connect(m_closeProjectButton, &QToolButton::clicked, this, &KateProjectPluginView::slotProjectAboutToClose);
    connect(m_plugin, &KateProjectPlugin::pluginViewProjectClosing, this, &KateProjectPluginView::slotProjectClose);

    connect(m_gitStatusRefreshButton, &QToolButton::clicked, this, [this] {
        if (auto widget = m_stackedgitViews->currentWidget()) {
            qobject_cast<GitWidget *>(widget)->getStatus();
        }
    });
    /**
     * create views for all already existing projects
     * will create toolviews on demand!
     */
    const auto projectList = m_plugin->projects();
    for (KateProject *project : projectList) {
        viewForProject(project);
    }
    // If the list of projects is empty we do not want to restore the tool view from the last session, BUG: 432296
    if (projectList.isEmpty()) {
        // We have to call this in the next iteration of the event loop, after the session is restored
        QTimer::singleShot(0, this, [this]() {
            m_mainWindow->hideToolView(m_toolView);
            m_mainWindow->hideToolView(m_gitToolView.get());
            m_mainWindow->hideToolView(m_toolInfoView);
            if (m_toolMultiView) {
                m_mainWindow->hideToolView(m_toolMultiView);
            }
        });
    }

    /**
     * connect to important signals, e.g. for auto project view creation
     */
    connect(m_plugin, &KateProjectPlugin::projectCreated, this, &KateProjectPluginView::viewForProject);
    connect(m_plugin, &KateProjectPlugin::configUpdated, this, &KateProjectPluginView::slotConfigUpdated);
    connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &KateProjectPluginView::slotViewChanged);
    connect(m_mainWindow, &KTextEditor::MainWindow::viewCreated, this, &KateProjectPluginView::slotViewCreated);

    /**
     * connect for all already existing views
     */
    const auto views = m_mainWindow->views();
    for (KTextEditor::View *view : views) {
        slotViewCreated(view);
    }

    /**
     * trigger once view change, to highlight right document
     */
    slotViewChanged();

    /**
     * back + forward
     */
    auto a = actionCollection()->addAction(QStringLiteral("projects_open_project"), this, SLOT(openDirectoryOrProject()));
    a->setText(i18n("Open Folder..."));

    a = actionCollection()->addAction(QStringLiteral("projects_todos"), this, SLOT(showProjectTodos()));
    a->setText(i18n("Project TODOs"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("korg-todo")));

    a = actionCollection()->addAction(KStandardAction::Back, QStringLiteral("projects_prev_project"), this, SLOT(slotProjectPrev()));
    actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Left));

    a = actionCollection()->addAction(KStandardAction::Forward, QStringLiteral("projects_next_project"), this, SLOT(slotProjectNext()));
    actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Right));

    a = actionCollection()->addAction(QStringLiteral("projects_goto_index"), this, SLOT(slotProjectIndex()));
    a->setText(i18n("Lookup"));
    actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::ALT | Qt::Key_1));
    a = actionCollection()->addAction(QStringLiteral("projects_close"), this, SLOT(slotProjectAboutToClose()));
    a->setText(i18n("Close Project"));
    a->setIcon(QIcon::fromTheme(QStringLiteral(PROJECTCLOSEICON)));
    m_gotoSymbolActionAppMenu = a = actionCollection()->addAction(KStandardAction::Goto, QStringLiteral("projects_goto_symbol"), this, SLOT(slotGotoSymbol()));

    // popup menu
    auto popup = new KActionMenu(i18n("Project"), this);
    actionCollection()->addAction(QStringLiteral("popup_project"), popup);

    m_lookupAction = popup->menu()->addAction(i18n("Lookup: %1", QString()), this, &KateProjectPluginView::slotProjectIndex);
    m_gotoSymbolAction = popup->menu()->addAction(i18n("Goto: %1", QString()), this, &KateProjectPluginView::slotGotoSymbol);

    connect(popup->menu(), &QMenu::aboutToShow, this, &KateProjectPluginView::slotContextMenuAboutToShow);

    connect(m_mainWindow, &KTextEditor::MainWindow::unhandledShortcutOverride, this, &KateProjectPluginView::handleEsc);

    // update git status on toolview visibility change
    connect(m_gitToolView.get(), SIGNAL(toolVisibleChanged(bool)), this, SLOT(slotUpdateStatus(bool)));

    /**
     * add us to gui
     */
    m_mainWindow->guiFactory()->addClient(this);

    /**
     * align to current config
     */
    slotConfigUpdated();
}

KateProjectPluginView::~KateProjectPluginView()
{
    /**
     * cleanup for all views
     */
    for (QObject *view : qAsConst(m_textViews)) {
        KTextEditor::CodeCompletionInterface *cci = qobject_cast<KTextEditor::CodeCompletionInterface *>(view);
        if (cci) {
            cci->unregisterCompletionModel(m_plugin->completion());
        }
    }

    /**
     * cu toolviews
     */
    delete m_toolView;
    m_toolView = nullptr;
    delete m_toolInfoView;
    m_toolInfoView = nullptr;
    delete m_toolMultiView;
    m_toolMultiView = nullptr;

    /**
     * cu gui client
     */
    m_mainWindow->guiFactory()->removeClient(this);
}

void KateProjectPluginView::slotConfigUpdated()
{
    if (!m_plugin->multiProjectGoto()) {
        delete m_toolMultiView;
        m_toolMultiView = nullptr;
    } else if (!m_toolMultiView) {
        m_toolMultiView = m_mainWindow->createToolView(m_plugin,
                                                       QStringLiteral("kateprojectmulti"),
                                                       KTextEditor::MainWindow::Bottom,
                                                       QIcon::fromTheme(QStringLiteral("view-choose")),
                                                       i18n("Projects Index"));
        auto gotoindex = new KateProjectInfoViewIndex(this, nullptr, m_toolMultiView);
        m_toolMultiView->layout()->addWidget(gotoindex);
    }

    // update action state
    m_gotoSymbolActionAppMenu->setEnabled(m_toolMultiView);
    m_gotoSymbolAction->setEnabled(m_toolMultiView);
}

QPair<KateProjectView *, KateProjectInfoView *> KateProjectPluginView::viewForProject(KateProject *project)
{
    /**
     * needs valid project
     */
    Q_ASSERT(project);

    /**
     * existing view?
     */
    if (m_project2View.contains(project)) {
        return m_project2View.value(project);
    }

    /**
     * create new views
     */
    KateProjectView *view = new KateProjectView(this, project, m_mainWindow);
    KateProjectInfoView *infoView = new KateProjectInfoView(this, project);
    GitWidget *gitView = new GitWidget(project, m_mainWindow, this);

    /**
     * attach to toolboxes
     * first the views, then the combo, that triggers signals
     */
    m_stackedProjectViews->addWidget(view);
    m_stackedProjectInfoViews->addWidget(infoView);
    m_stackedgitViews->addWidget(gitView);
    m_projectsCombo->addItem(QIcon::fromTheme(QStringLiteral("project-open")), project->name(), project->fileName());
    m_projectsComboGit->addItem(QIcon::fromTheme(QStringLiteral("project-open")), project->name(), project->fileName());

    /*
     * inform onward
     */
    Q_EMIT pluginProjectAdded(project->baseDir(), project->name());

    /**
     * remember and return it
     */
    return (m_project2View[project] = QPair<KateProjectView *, KateProjectInfoView *>(view, infoView));
}

QString KateProjectPluginView::projectFileName() const
{
    QWidget *active = m_stackedProjectViews->currentWidget();
    if (!active) {
        return QString();
    }

    return static_cast<KateProjectView *>(active)->project()->fileName();
}

QString KateProjectPluginView::projectName() const
{
    QWidget *active = m_stackedProjectViews->currentWidget();
    if (!active) {
        return QString();
    }

    return static_cast<KateProjectView *>(active)->project()->name();
}

QString KateProjectPluginView::projectBaseDir() const
{
    QWidget *active = m_stackedProjectViews->currentWidget();
    if (!active) {
        return QString();
    }

    return static_cast<KateProjectView *>(active)->project()->baseDir();
}

QVariantMap KateProjectPluginView::projectMap() const
{
    QWidget *active = m_stackedProjectViews->currentWidget();
    if (!active) {
        return QVariantMap();
    }

    return static_cast<KateProjectView *>(active)->project()->projectMap();
}

QStringList KateProjectPluginView::projectFiles() const
{
    KateProjectView *active = static_cast<KateProjectView *>(m_stackedProjectViews->currentWidget());
    if (!active) {
        return QStringList();
    }

    return active->project()->files();
}

QString KateProjectPluginView::allProjectsCommonBaseDir() const
{
    auto projects = m_plugin->projects();

    if (projects.empty()) {
        return QString();
    }

    if (projects.size() == 1) {
        return projects[0]->baseDir();
    }

    QString commonParent1 = FileUtil::commonParent(projects[0]->baseDir(), projects[1]->baseDir());

    for (int i = 2; i < projects.size(); i++) {
        commonParent1 = FileUtil::commonParent(commonParent1, projects[i]->baseDir());
    }

    return commonParent1;
}

QStringList KateProjectPluginView::allProjectsFiles() const
{
    QStringList fileList;

    const auto projectList = m_plugin->projects();
    for (auto project : projectList) {
        fileList.append(project->files());
    }

    return fileList;
}

QMap<QString, QString> KateProjectPluginView::allProjects() const
{
    QMap<QString, QString> projectMap;

    const auto projectList = m_plugin->projects();
    for (auto project : projectList) {
        projectMap[project->baseDir()] = project->name();
    }
    return projectMap;
}

void KateProjectPluginView::slotViewChanged()
{
    /**
     * get active view
     */
    KTextEditor::View *activeView = m_mainWindow->activeView();

    /**
     * update pointer, maybe disconnect before
     */
    if (m_activeTextEditorView) {
        m_activeTextEditorView->document()->disconnect(this);
    }
    m_activeTextEditorView = activeView;

    /**
     * no current active view, return
     */
    if (!m_activeTextEditorView) {
        return;
    }

    /**
     * connect to url changed, for auto load
     */
    connect(m_activeTextEditorView->document(), &KTextEditor::Document::documentUrlChanged, this, &KateProjectPluginView::slotDocumentUrlChanged);

    /**
     * trigger slot once
     */
    slotDocumentUrlChanged(m_activeTextEditorView->document());
}

void KateProjectPluginView::slotCurrentChanged(int index)
{
    // trigger change of stacked widgets
    m_stackedProjectViews->setCurrentIndex(index);
    m_stackedProjectInfoViews->setCurrentIndex(index);
    m_stackedgitViews->setCurrentIndex(index);

    {
        const QSignalBlocker blocker(m_projectsComboGit);
        m_projectsComboGit->setCurrentIndex(index);
    }

    // update focus proxy + open currently selected document
    if (QWidget *current = m_stackedProjectViews->currentWidget()) {
        m_stackedProjectViews->setFocusProxy(current);
        static_cast<KateProjectView *>(current)->openSelectedDocument();
    }

    // update focus proxy
    if (QWidget *current = m_stackedProjectInfoViews->currentWidget()) {
        m_stackedProjectInfoViews->setFocusProxy(current);
    }

    // update git focus proxy + update status
    if (QWidget *current = m_stackedgitViews->currentWidget()) {
        m_stackedgitViews->setFocusProxy(current);
        static_cast<GitWidget *>(current)->getStatus();
    }

    // project file name might have changed
    Q_EMIT projectFileNameChanged();
    Q_EMIT projectMapChanged();
}

void KateProjectPluginView::slotDocumentUrlChanged(KTextEditor::Document *document)
{
    /**
     * abort if empty url or no local path
     */
    if (document->url().isEmpty() || !document->url().isLocalFile()) {
        return;
    }

    /**
     * search matching project
     */
    KateProject *project = m_plugin->projectForUrl(document->url());
    if (!project) {
        return;
    }

    /**
     * select the file FIRST
     */
    m_project2View.value(project).first->selectFile(document->url().toLocalFile());

    /**
     * get active project view and switch it, if it is for a different project
     * do this AFTER file selection
     */
    KateProjectView *active = static_cast<KateProjectView *>(m_stackedProjectViews->currentWidget());
    if (active != m_project2View.value(project).first) {
        int index = m_projectsCombo->findData(project->fileName());
        if (index >= 0) {
            m_projectsCombo->setCurrentIndex(index);
        }
    }
}

void KateProjectPluginView::switchToProject(const QDir &dir)
{
    /**
     * search matching project
     */
    KateProject *project = m_plugin->projectForDir(dir);
    if (!project) {
        return;
    }

    /**
     * get active project view and switch it, if it is for a different project
     * do this AFTER file selection
     */
    KateProjectView *active = static_cast<KateProjectView *>(m_stackedProjectViews->currentWidget());
    if (active != m_project2View.value(project).first) {
        int index = m_projectsCombo->findData(project->fileName());
        if (index >= 0) {
            m_projectsCombo->setCurrentIndex(index);
        }
    }
}

void KateProjectPluginView::slotViewCreated(KTextEditor::View *view)
{
    /**
     * connect to destroyed
     */
    connect(view, &KTextEditor::View::destroyed, this, &KateProjectPluginView::slotViewDestroyed);

    /**
     * add completion model if possible
     */
    KTextEditor::CodeCompletionInterface *cci = qobject_cast<KTextEditor::CodeCompletionInterface *>(view);
    if (cci) {
        cci->registerCompletionModel(m_plugin->completion());
    }

    /**
     * remember for this view we need to cleanup!
     */
    m_textViews.insert(view);
}

void KateProjectPluginView::slotViewDestroyed(QObject *view)
{
    /**
     * remove remembered views for which we need to cleanup on exit!
     */
    m_textViews.remove(view);
}

void KateProjectPluginView::slotProjectPrev()
{
    if (!m_projectsCombo->count()) {
        return;
    }

    if (m_projectsCombo->currentIndex() == 0) {
        m_projectsCombo->setCurrentIndex(m_projectsCombo->count() - 1);
    } else {
        m_projectsCombo->setCurrentIndex(m_projectsCombo->currentIndex() - 1);
    }
}

void KateProjectPluginView::slotProjectNext()
{
    if (!m_projectsCombo->count()) {
        return;
    }

    if (m_projectsCombo->currentIndex() + 1 == m_projectsCombo->count()) {
        m_projectsCombo->setCurrentIndex(0);
    } else {
        m_projectsCombo->setCurrentIndex(m_projectsCombo->currentIndex() + 1);
    }
}

void KateProjectPluginView::slotProjectReload()
{
    /**
     * force reload if any active project
     */
    if (QWidget *current = m_stackedProjectViews->currentWidget()) {
        static_cast<KateProjectView *>(current)->project()->reload(true);
    }
    /**
     * Refresh git status
     */
    if (auto widget = m_stackedgitViews->currentWidget()) {
        qobject_cast<GitWidget *>(widget)->getStatus();
    }
}

void KateProjectPluginView::slotProjectAboutToClose()
{
    if (QWidget *current = m_stackedProjectViews->currentWidget()) {
        m_plugin->closeProject(static_cast<KateProjectView *>(current)->project());
    }
}

void KateProjectPluginView::slotProjectClose(KateProject *project)
{
    const int index = m_plugin->projects().indexOf(project);
    m_project2View.erase(m_project2View.find(project));

    QWidget *stackedProjectViewsWidget = m_stackedProjectViews->widget(index);
    m_stackedProjectViews->removeWidget(stackedProjectViewsWidget);
    delete stackedProjectViewsWidget;

    QWidget *stackedProjectInfoViewsWidget = m_stackedProjectInfoViews->widget(index);
    m_stackedProjectInfoViews->removeWidget(stackedProjectInfoViewsWidget);
    delete stackedProjectInfoViewsWidget;

    QWidget *stackedgitViewsWidget = m_stackedgitViews->widget(index);
    m_stackedgitViews->removeWidget(stackedgitViewsWidget);
    delete stackedgitViewsWidget;

    m_projectsCombo->removeItem(index);
    m_projectsComboGit->removeItem(index);

    // inform onward
    Q_EMIT pluginProjectRemoved(project->baseDir(), project->name());
}

QString KateProjectPluginView::currentWord() const
{
    KTextEditor::View *kv = m_activeTextEditorView;
    if (!kv) {
        return QString();
    }

    if (kv->selection() && kv->selectionRange().onSingleLine()) {
        return kv->selectionText();
    }

    return kv->document()->wordAt(kv->cursorPosition());
}

void KateProjectPluginView::slotProjectIndex()
{
    const QString word = currentWord();
    if (!word.isEmpty()) {
        auto tabView = qobject_cast<QTabWidget *>(m_stackedProjectInfoViews->currentWidget());
        if (tabView) {
            if (auto codeIndex = tabView->findChild<KateProjectInfoViewIndex *>()) {
                tabView->setCurrentWidget(codeIndex);
            }
        }
        m_mainWindow->showToolView(m_toolInfoView);
        Q_EMIT projectLookupWord(word);
    }
}

void KateProjectPluginView::slotGotoSymbol()
{
    if (!m_toolMultiView) {
        return;
    }

    const QString word = currentWord();
    if (!word.isEmpty()) {
        int results = 0;
        Q_EMIT gotoSymbol(word, results);
        if (results > 1) {
            m_mainWindow->showToolView(m_toolMultiView);
        }
    }
}

void KateProjectPluginView::slotContextMenuAboutToShow()
{
    const QString word = currentWord();
    if (word.isEmpty()) {
        return;
    }

    const QString squeezed = KStringHandler::csqueeze(word, 30);
    m_lookupAction->setText(i18n("Lookup: %1", squeezed));
    m_gotoSymbolAction->setText(i18n("Goto: %1", squeezed));
}

void KateProjectPluginView::handleEsc(QEvent *e)
{
    if (!m_mainWindow) {
        return;
    }

    QKeyEvent *k = static_cast<QKeyEvent *>(e);
    if (k->key() == Qt::Key_Escape && k->modifiers() == Qt::NoModifier) {
        const auto infoView = qobject_cast<const KateProjectInfoView *>(m_stackedProjectInfoViews->currentWidget());
        if (m_toolInfoView->isVisible() && (!infoView || !infoView->ignoreEsc())) {
            m_mainWindow->hideToolView(m_toolInfoView);
        }
    }
}

void KateProjectPluginView::slotUpdateStatus(bool visible)
{
    if (!visible) {
        return;
    }

    if (auto widget = m_stackedgitViews->currentWidget()) {
        static_cast<GitWidget *>(widget)->getStatus();
    }
}

void KateProjectPluginView::openDirectoryOrProject()
{
    const QString dir = QFileDialog::getExistingDirectory(nullptr, i18n("Choose a directory"), QDir::currentPath());
    auto project = m_plugin->projectForDir(dir, true);
    // switch to this project
    if (project) {
        int index = m_projectsCombo->findData(project->fileName());
        if (index >= 0) {
            m_projectsCombo->setCurrentIndex(index);
        }
    }
}

void KateProjectPluginView::showProjectTodos()
{
    KTextEditor::Command *pgrep = KTextEditor::Editor::instance()->queryCommand(QStringLiteral("pgrep"));
    if (!pgrep) {
        return;
    }
    QString msg;
    pgrep->exec(nullptr, QStringLiteral("preg (TODO|FIXME)\\b"), msg);
}

void KateProjectPluginView::showDiffInFixedView(const QByteArray &contents)
{
    if (!m_fixedView.view) {
        m_fixedView.view = mainWindow()->openUrl(QUrl());
        m_fixedView.defaultMenu = m_fixedView.view->contextMenu();
    }

    m_fixedView.view->document()->setText(QString::fromUtf8(contents));
    m_fixedView.view->document()->setHighlightingMode(QStringLiteral("Diff"));
    /** We don't want save dialog on close */
    m_fixedView.view->document()->setModified(false);
    m_fixedView.view->setCursorPosition({0, 0});
    m_fixedView.restoreMenu();
    /** Activate this view */
    m_mainWindow->activateView(m_fixedView.view->document());
}

void KateProjectPluginView::openTerminal(const QString &dirPath, KateProject *project)
{
    m_mainWindow->showToolView(m_toolInfoView);

    if (m_project2View.contains(project)) {
        m_project2View.find(project)->second->resetTerminal(dirPath);
    }
}

#include "kateprojectpluginview.moc"
