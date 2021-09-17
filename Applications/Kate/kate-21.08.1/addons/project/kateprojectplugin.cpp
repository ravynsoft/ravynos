/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectplugin.h"

#include "kateproject.h"
#include "kateprojectconfigpage.h"
#include "kateprojectpluginview.h"

#include <ktexteditor/application.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QCoreApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>
#include <QTime>

#include <vector>

#ifdef HAVE_CTERMID
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace
{
const QString ProjectFileName = QStringLiteral(".kateproject");
const QString GitFolderName = QStringLiteral(".git");
const QString SubversionFolderName = QStringLiteral(".svn");
const QString MercurialFolderName = QStringLiteral(".hg");

const QString GitConfig = QStringLiteral("git");
const QString SubversionConfig = QStringLiteral("subversion");
const QString MercurialConfig = QStringLiteral("mercurial");

const QStringList DefaultConfig = QStringList() << GitConfig << SubversionConfig << MercurialConfig;
}

KateProjectPlugin::KateProjectPlugin(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
    , m_completion(this)
{
    qRegisterMetaType<KateProjectSharedQStandardItem>("KateProjectSharedQStandardItem");
    qRegisterMetaType<KateProjectSharedQHashStringItem>("KateProjectSharedQHashStringItem");
    qRegisterMetaType<KateProjectSharedProjectIndex>("KateProjectSharedProjectIndex");

    connect(KTextEditor::Editor::instance()->application(), &KTextEditor::Application::documentCreated, this, &KateProjectPlugin::slotDocumentCreated);
    connect(&m_fileWatcher, &QFileSystemWatcher::directoryChanged, this, &KateProjectPlugin::slotDirectoryChanged);

    // read configuration prior to cwd project setup below
    readConfig();
    QStringList args = qApp->arguments();
    bool projectSpecified = false;
    args.removeFirst(); // The first argument is the executable name
    for (const QString &arg : qAsConst(args)) {
        QFileInfo info(arg);
        if (info.isDir()) {
            projectForDir(info.absoluteFilePath(), true);
            projectSpecified = true;
        }
    }

#ifdef HAVE_CTERMID
    /**
     * open project for our current working directory, if this kate has a terminal
     * https://stackoverflow.com/questions/1312922/detect-if-stdin-is-a-terminal-or-pipe-in-c-c-qt
     */
    char tty[L_ctermid + 1] = {0};
    ctermid(tty);
    int fd = ::open(tty, O_RDONLY);

    if (fd >= 0) {
        if (!projectSpecified) {
            projectForDir(QDir::current());
        }
        ::close(fd);
    }
#endif

    for (auto document : KTextEditor::Editor::instance()->application()->documents()) {
        slotDocumentCreated(document);
    }

    registerVariables();
}

KateProjectPlugin::~KateProjectPlugin()
{
    unregisterVariables();

    for (KateProject *project : m_projects) {
        m_fileWatcher.removePath(QFileInfo(project->fileName()).canonicalPath());
        delete project;
    }
    m_projects.clear();
}

QObject *KateProjectPlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    return new KateProjectPluginView(this, mainWindow);
}

int KateProjectPlugin::configPages() const
{
    return 1;
}

KTextEditor::ConfigPage *KateProjectPlugin::configPage(int number, QWidget *parent)
{
    if (number != 0) {
        return nullptr;
    }
    return new KateProjectConfigPage(parent, this);
}

KateProject *KateProjectPlugin::createProjectForFileName(const QString &fileName)
{
    KateProject *project = new KateProject(m_threadPool, this);
    if (!project->loadFromFile(fileName)) {
        delete project;
        return nullptr;
    }

    m_projects.append(project);
    m_fileWatcher.addPath(QFileInfo(fileName).canonicalPath());
    Q_EMIT projectCreated(project);
    return project;
}

KateProject *KateProjectPlugin::projectForDir(QDir dir, bool userSpecified)
{
    /**
     * Save dir to create a project from directory if nothing works
     */
    const QDir originalDir = dir;

    /**
     * search project file upwards
     * with recursion guard
     * do this first for all level and only after this fails try to invent projects
     * otherwise one e.g. invents projects for .kateproject tree structures with sub .git clones
     */
    QSet<QString> seenDirectories;
    std::vector<QString> directoryStack;
    while (!seenDirectories.contains(dir.absolutePath())) {
        // update guard
        seenDirectories.insert(dir.absolutePath());

        // remember directory for later project creation based on other criteria
        directoryStack.push_back(dir.absolutePath());

        // check for project and load it if found
        const QString canonicalPath = dir.canonicalPath();
        const QString canonicalFileName = dir.filePath(ProjectFileName);
        for (KateProject *project : m_projects) {
            if (project->baseDir() == canonicalPath || project->fileName() == canonicalFileName) {
                return project;
            }
        }

        // project file found => done
        if (dir.exists(ProjectFileName)) {
            return createProjectForFileName(canonicalFileName);
        }

        // else: cd up, if possible or abort
        if (!dir.cdUp()) {
            break;
        }
    }

    /**
     * if we arrive here, we found no .kateproject
     * => we want to invent a project based on e.g. version control system info
     */
    for (const QString &dir : directoryStack) {
        // try to invent project based on version control stuff
        KateProject *project = nullptr;
        if ((project = detectGit(dir)) || (project = detectSubversion(dir)) || (project = detectMercurial(dir))) {
            return project;
        }
    }

    /**
     * Version control not found? Load the directory as project
     */
    if (userSpecified) {
        return createProjectForDirectory(originalDir);
    }

    /**
     * Give up
     */
    return nullptr;
}

bool KateProjectPlugin::closeProject(KateProject *project)
{
    QVector<KTextEditor::Document *> projectDocuments;
    for (auto doc : KTextEditor::Editor::instance()->application()->documents())
        if (QUrl(project->baseDir()).isParentOf(doc->url().adjusted(QUrl::RemoveScheme)))
            projectDocuments.push_back(doc);

    // if we have some documents open for this project, ask if we want to close, else just do it
    if (!projectDocuments.isEmpty()) {
        QWidget *window = KTextEditor::Editor::instance()->application()->activeMainWindow()->window();
        const QString title = i18n("Confirm project closing: %1", project->name());
        const QString text = i18n("Do you want to close the project %1 and the related %2 open documents?", project->name(), projectDocuments.size());
        if (QMessageBox::Yes != QMessageBox::question(window, title, text, QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes)) {
            return false;
        }

        for (auto doc : projectDocuments)
            KTextEditor::Editor::instance()->application()->closeDocument(doc);
    }

    Q_EMIT pluginViewProjectClosing(project);
    if (m_projects.removeOne(project)) {
        m_fileWatcher.removePath(QFileInfo(project->fileName()).canonicalPath());
        delete project;
        return true;
    }

    return false;
}

KateProject *KateProjectPlugin::projectForUrl(const QUrl &url)
{
    if (url.isEmpty() || !url.isLocalFile()) {
        return nullptr;
    }

    return projectForDir(QFileInfo(url.toLocalFile()).absoluteDir());
}

void KateProjectPlugin::slotDocumentCreated(KTextEditor::Document *document)
{
    connect(document, &KTextEditor::Document::documentUrlChanged, this, &KateProjectPlugin::slotDocumentUrlChanged);
    connect(document, &KTextEditor::Document::destroyed, this, &KateProjectPlugin::slotDocumentDestroyed);

    slotDocumentUrlChanged(document);
}

void KateProjectPlugin::slotDocumentDestroyed(QObject *document)
{
    if (KateProject *project = m_document2Project.value(document)) {
        project->unregisterDocument(static_cast<KTextEditor::Document *>(document));
    }

    m_document2Project.remove(document);
}

void KateProjectPlugin::slotDocumentUrlChanged(KTextEditor::Document *document)
{
    KateProject *project = projectForUrl(document->url());

    if (KateProject *project = m_document2Project.value(document)) {
        project->unregisterDocument(document);
    }

    if (!project) {
        m_document2Project.remove(document);
    } else {
        m_document2Project[document] = project;
    }

    if (KateProject *project = m_document2Project.value(document)) {
        project->registerDocument(document);
    }
}

void KateProjectPlugin::slotDirectoryChanged(const QString &path)
{
    QString fileName = QDir(path).filePath(ProjectFileName);
    for (KateProject *project : m_projects) {
        if (project->fileName() == fileName) {
            QDateTime lastModified = QFileInfo(fileName).lastModified();
            if (project->fileLastModified().isNull() || (lastModified > project->fileLastModified())) {
                project->reload();
            }
            break;
        }
    }
}

KateProject *KateProjectPlugin::detectGit(const QDir &dir)
{
    // allow .git as dir and file (file for git worktree stuff, https://git-scm.com/docs/git-worktree)
    if (m_autoGit && dir.exists(GitFolderName)) {
        return createProjectForRepository(QStringLiteral("git"), dir);
    }

    return nullptr;
}

KateProject *KateProjectPlugin::detectSubversion(const QDir &dir)
{
    if (m_autoSubversion && dir.exists(SubversionFolderName) && QFileInfo(dir, SubversionFolderName).isDir()) {
        return createProjectForRepository(QStringLiteral("svn"), dir);
    }

    return nullptr;
}

KateProject *KateProjectPlugin::detectMercurial(const QDir &dir)
{
    if (m_autoMercurial && dir.exists(MercurialFolderName) && QFileInfo(dir, MercurialFolderName).isDir()) {
        return createProjectForRepository(QStringLiteral("hg"), dir);
    }

    return nullptr;
}

KateProject *KateProjectPlugin::createProjectForRepository(const QString &type, const QDir &dir)
{
    QVariantMap cnf, files;
    files[type] = 1;
    cnf[QStringLiteral("name")] = dir.dirName();
    cnf[QStringLiteral("files")] = (QVariantList() << files);

    KateProject *project = new KateProject(m_threadPool, this);
    project->loadFromData(cnf, dir.canonicalPath());

    m_projects.append(project);

    Q_EMIT projectCreated(project);
    return project;
}

KateProject *KateProjectPlugin::createProjectForDirectory(const QDir &dir)
{
    QVariantMap cnf, files;
    files[QStringLiteral("directory")] = QStringLiteral("./");
    cnf[QStringLiteral("name")] = dir.dirName();
    cnf[QStringLiteral("files")] = (QVariantList() << files);

    KateProject *project = new KateProject(m_threadPool, this);
    project->loadFromData(cnf, dir.canonicalPath());

    m_projects.append(project);

    Q_EMIT projectCreated(project);
    return project;
}

void KateProjectPlugin::setAutoRepository(bool onGit, bool onSubversion, bool onMercurial)
{
    m_autoGit = onGit;
    m_autoSubversion = onSubversion;
    m_autoMercurial = onMercurial;
    writeConfig();
}

bool KateProjectPlugin::autoGit() const
{
    return m_autoGit;
}

bool KateProjectPlugin::autoSubversion() const
{
    return m_autoSubversion;
}

bool KateProjectPlugin::autoMercurial() const
{
    return m_autoMercurial;
}

void KateProjectPlugin::setIndex(bool enabled, const QUrl &directory)
{
    m_indexEnabled = enabled;
    m_indexDirectory = directory;
    writeConfig();
}

bool KateProjectPlugin::getIndexEnabled() const
{
    return m_indexEnabled;
}

QUrl KateProjectPlugin::getIndexDirectory() const
{
    return m_indexDirectory;
}

bool KateProjectPlugin::multiProjectCompletion() const
{
    return m_multiProjectCompletion;
}

bool KateProjectPlugin::multiProjectGoto() const
{
    return m_multiProjectGoto;
}

void KateProjectPlugin::setGitStatusShowNumStat(bool show)
{
    m_gitNumStat = show;
    writeConfig();
}

bool KateProjectPlugin::showGitStatusWithNumStat()
{
    return m_gitNumStat;
}

void KateProjectPlugin::setSingleClickAction(ClickAction cb)
{
    m_singleClickAction = cb;
    writeConfig();
}

ClickAction KateProjectPlugin::singleClickAcion()
{
    return m_singleClickAction;
}

void KateProjectPlugin::setDoubleClickAction(ClickAction cb)
{
    m_doubleClickAction = cb;
    writeConfig();
}

ClickAction KateProjectPlugin::doubleClickAcion()
{
    return m_doubleClickAction;
}

void KateProjectPlugin::setMultiProject(bool completion, bool gotoSymbol)
{
    m_multiProjectCompletion = completion;
    m_multiProjectGoto = gotoSymbol;
    writeConfig();
}

void KateProjectPlugin::readConfig()
{
    KConfigGroup config(KSharedConfig::openConfig(), "project");

    const QStringList autorepository = config.readEntry("autorepository", DefaultConfig);
    m_autoGit = autorepository.contains(GitConfig);
    m_autoSubversion = autorepository.contains(SubversionConfig);
    m_autoMercurial = autorepository.contains(MercurialConfig);

    m_indexEnabled = config.readEntry("index", false);
    m_indexDirectory = config.readEntry("indexDirectory", QUrl());

    m_multiProjectCompletion = config.readEntry("multiProjectCompletion", false);
    m_multiProjectGoto = config.readEntry("multiProjectCompletion", false);

    m_gitNumStat = config.readEntry("gitStatusNumStat", true);
    m_singleClickAction = (ClickAction)config.readEntry("gitStatusSingleClick", (int)ClickAction::ShowDiff);
    m_doubleClickAction = (ClickAction)config.readEntry("gitStatusDoubleClick", (int)ClickAction::StageUnstage);

    Q_EMIT configUpdated();
}

void KateProjectPlugin::writeConfig()
{
    KConfigGroup config(KSharedConfig::openConfig(), "project");
    QStringList repos;

    if (m_autoGit) {
        repos << GitConfig;
    }

    if (m_autoSubversion) {
        repos << SubversionConfig;
    }

    if (m_autoMercurial) {
        repos << MercurialConfig;
    }

    config.writeEntry("autorepository", repos);

    config.writeEntry("index", m_indexEnabled);
    config.writeEntry("indexDirectory", m_indexDirectory);

    config.writeEntry("multiProjectCompletion", m_multiProjectCompletion);
    config.writeEntry("multiProjectGoto", m_multiProjectGoto);

    config.writeEntry("gitStatusNumStat", m_gitNumStat);
    config.writeEntry("gitStatusSingleClick", (int)m_singleClickAction);
    config.writeEntry("gitStatusDoubleClick", (int)m_doubleClickAction);

    Q_EMIT configUpdated();
}

static KateProjectPlugin *findProjectPlugin()
{
    auto plugin = KTextEditor::Editor::instance()->application()->plugin(QStringLiteral("kateprojectplugin"));
    return qobject_cast<KateProjectPlugin *>(plugin);
}

void KateProjectPlugin::registerVariables()
{
    auto editor = KTextEditor::Editor::instance();
    editor->registerVariableMatch(QStringLiteral("Project:Path"),
                                  i18n("Full path to current project excluding the file name."),
                                  [](const QStringView &, KTextEditor::View *view) {
                                      if (!view) {
                                          return QString();
                                      }
                                      auto projectPlugin = findProjectPlugin();
                                      if (!projectPlugin) {
                                          return QString();
                                      }
                                      auto kateProject = findProjectPlugin()->projectForUrl(view->document()->url());
                                      if (!kateProject) {
                                          return QString();
                                      }
                                      return QDir(kateProject->baseDir()).absolutePath();
                                  });

    editor->registerVariableMatch(QStringLiteral("Project:NativePath"),
                                  i18n("Full path to current project excluding the file name, with native path separator (backslash on Windows)."),
                                  [](const QStringView &, KTextEditor::View *view) {
                                      if (!view) {
                                          return QString();
                                      }
                                      auto projectPlugin = findProjectPlugin();
                                      if (!projectPlugin) {
                                          return QString();
                                      }
                                      auto kateProject = findProjectPlugin()->projectForUrl(view->document()->url());
                                      if (!kateProject) {
                                          return QString();
                                      }
                                      return QDir::toNativeSeparators(QDir(kateProject->baseDir()).absolutePath());
                                  });
}

void KateProjectPlugin::unregisterVariables()
{
    auto editor = KTextEditor::Editor::instance();
    editor->unregisterVariableMatch(QStringLiteral("Project:Path"));
    editor->unregisterVariableMatch(QStringLiteral("Project:NativePath"));
}
