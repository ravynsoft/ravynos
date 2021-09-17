/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kateapp.h"

#include "kateviewmanager.h"

#include <KConfigGroup>
#include <KConfigGui>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStartupInfo>
#include <KWindowInfo>

#ifdef WITH_KUSERFEEDBACK
#include <KUserFeedback/ApplicationVersionSource>
#include <KUserFeedback/PlatformInfoSource>
#include <KUserFeedback/QtVersionSource>
#include <KUserFeedback/ScreenInfoSource>
#include <KUserFeedback/StartCountSource>
#include <KUserFeedback/UsageTimeSource>
#endif

#include <QApplication>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QFileInfo>
#include <QFileOpenEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QTextCodec>
#include <QUrlQuery>

#include <urlinfo.h>

/**
 * singleton instance pointer
 */
static KateApp *appSelf = Q_NULLPTR;

Q_LOGGING_CATEGORY(LOG_KATE, "kate", QtWarningMsg)

KateApp::KateApp(const QCommandLineParser &args)
    : m_args(args)
    , m_wrapper(appSelf = this)
    , m_docManager(this)
    , m_adaptor(this)
    , m_pluginManager(this)
    , m_sessionManager(this)
    , m_stashManager(this)
{
    /**
     * re-route some signals to application wrapper
     */
    connect(&m_docManager, &KateDocManager::documentCreated, &m_wrapper, &KTextEditor::Application::documentCreated);
    connect(&m_docManager, &KateDocManager::documentWillBeDeleted, &m_wrapper, &KTextEditor::Application::documentWillBeDeleted);
    connect(&m_docManager, &KateDocManager::documentDeleted, &m_wrapper, &KTextEditor::Application::documentDeleted);

    /**
     * handle mac os x like file open request via event filter
     */
    qApp->installEventFilter(this);

#ifdef WITH_KUSERFEEDBACK
    /**
     * defaults, inspired by plasma
     */
    m_userFeedbackProvider.setProductIdentifier(QStringLiteral("org.kde.kate"));
    m_userFeedbackProvider.setFeedbackServer(QUrl(QStringLiteral("https://telemetry.kde.org/")));
    m_userFeedbackProvider.setSubmissionInterval(7);
    m_userFeedbackProvider.setApplicationStartsUntilEncouragement(5);
    m_userFeedbackProvider.setEncouragementDelay(30);

    /**
     * add some feedback providers
     */

    // software version info
    m_userFeedbackProvider.addDataSource(new KUserFeedback::ApplicationVersionSource);
    m_userFeedbackProvider.addDataSource(new KUserFeedback::QtVersionSource);

    // info about the machine
    m_userFeedbackProvider.addDataSource(new KUserFeedback::PlatformInfoSource);
    m_userFeedbackProvider.addDataSource(new KUserFeedback::ScreenInfoSource);

    // usage info
    m_userFeedbackProvider.addDataSource(new KUserFeedback::StartCountSource);
    m_userFeedbackProvider.addDataSource(new KUserFeedback::UsageTimeSource);
#endif
}

KateApp::~KateApp()
{
    /**
     * unregister from dbus before we get unusable...
     */
    if (QDBusConnection::sessionBus().interface()) {
        m_adaptor.emitExiting();
        QDBusConnection::sessionBus().unregisterObject(QStringLiteral("/MainApplication"));
    }

    /**
     * delete all main windows before the document manager & co. die
     */
    while (!m_mainWindows.isEmpty()) {
        // mainwindow itself calls KateApp::removeMainWindow(this)
        delete m_mainWindows[0];
    }
}

KateApp *KateApp::self()
{
    return appSelf;
}

bool KateApp::init()
{
    // set KATE_PID for use in child processes
    qputenv("KATE_PID", QStringLiteral("%1").arg(QCoreApplication::applicationPid()).toLatin1().constData());

    // handle restore different
    if (qApp->isSessionRestored()) {
        restoreKate();
    } else {
        // let us handle our command line args and co ;)
        // we can exit here if session chooser decides
        if (!startupKate()) {
            // session chooser told to exit kate
            return false;
        }
    }

    return true;
}

void KateApp::restoreKate()
{
    KConfig *sessionConfig = KConfigGui::sessionConfig();

    // activate again correct session!!!
    QString lastSession(sessionConfig->group("General").readEntry("Last Session", QString()));
    sessionManager()->activateSession(lastSession, false, false);

    // plugins
    KateApp::self()->pluginManager()->loadConfig(sessionConfig);

    // restore the files we need
    m_docManager.restoreDocumentList(sessionConfig);

    // restore all windows ;)
    for (int n = 1; KMainWindow::canBeRestored(n); n++) {
        newMainWindow(sessionConfig, QString::number(n));
    }

    // oh, no mainwindow, create one, should not happen, but make sure ;)
    if (mainWindowsCount() == 0) {
        newMainWindow();
    }
}

bool KateApp::startupKate()
{
    // user specified session to open
    if (m_args.isSet(QStringLiteral("start"))) {
        sessionManager()->activateSession(m_args.value(QStringLiteral("start")), false);
    } else if (m_args.isSet(QStringLiteral("startanon"))) {
        sessionManager()->activateAnonymousSession();
    } else if (!m_args.isSet(QStringLiteral("stdin")) && (m_args.positionalArguments().count() == 0)) { // only start session if no files specified
        // let the user choose session if possible
        if (!sessionManager()->chooseSession()) {
            // we will exit kate now, notify the rest of the world we are done
            KStartupInfo::appStarted(KStartupInfo::startupId());
            return false;
        }
    } else {
        sessionManager()->activateAnonymousSession();
    }

    // oh, no mainwindow, create one, should not happen, but make sure ;)
    if (mainWindowsCount() == 0) {
        newMainWindow();
    }

    // notify about start
    QWidget *win = activeKateMainWindow();
    win->setAttribute(Qt::WA_NativeWindow, true);
    KStartupInfo::setNewStartupId(win->windowHandle(), KStartupInfo::startupId());

    QTextCodec *codec = m_args.isSet(QStringLiteral("encoding")) ? QTextCodec::codecForName(m_args.value(QStringLiteral("encoding")).toUtf8()) : nullptr;
    bool tempfileSet = m_args.isSet(QStringLiteral("tempfile"));

    KTextEditor::Document *doc = nullptr;
    const QString codec_name = codec ? QString::fromLatin1(codec->name()) : QString();

    for (const auto &positionalArgument : m_args.positionalArguments()) {
        UrlInfo info(positionalArgument);

        // this file is no local dir, open it, else warn
        bool noDir = !info.url.isLocalFile() || !QFileInfo(info.url.toLocalFile()).isDir();

        if (noDir) {
            doc = openDocUrl(info.url, codec_name, tempfileSet);
            if (info.cursor.isValid()) {
                setCursor(info.cursor.line(), info.cursor.column());
            } else if (hasCursorInArgs()) {
                setCursorFromArgs(activeMainWindow()->activeView());
            }
        } else if (!KateApp::self()->pluginManager()->plugin(QStringLiteral("kateprojectplugin"))) {
            KMessageBox::sorry(activeKateMainWindow(), i18n("Folders can only be opened when the projects plugin is enabled"));
        }
    }

    // handle stdin input
    if (m_args.isSet(QStringLiteral("stdin"))) {
        QTextStream input(stdin, QIODevice::ReadOnly);

        // set chosen codec
        if (codec) {
            input.setCodec(codec);
        }

        QString line;
        QString text;

        do {
            line = input.readLine();
            text.append(line + QLatin1Char('\n'));
        } while (!line.isNull());

        openInput(text, codec_name);
    } else if (doc) {
        activeKateMainWindow()->viewManager()->activateView(doc);
    }

    return true;
}

void KateApp::shutdownKate(KateMainWindow *win)
{
    if (!win->queryClose_internal()) {
        return;
    }

    sessionManager()->saveActiveSession(true);

    /**
     * all main windows will be cleaned up
     * in the KateApp destructor after the event
     * loop is left
     */
    QApplication::quit();
}

KatePluginManager *KateApp::pluginManager()
{
    return &m_pluginManager;
}

KateDocManager *KateApp::documentManager()
{
    return &m_docManager;
}

KateSessionManager *KateApp::sessionManager()
{
    return &m_sessionManager;
}

KateStashManager *KateApp::stashManager()
{
    return &m_stashManager;
}

bool KateApp::openUrl(const QUrl &url, const QString &encoding, bool isTempFile)
{
    return openDocUrl(url, encoding, isTempFile);
}

bool KateApp::isOnActivity(const QString &activity)
{
    for (const auto &window : m_mainWindows) {
        const KWindowInfo info(window->winId(), {}, NET::WM2Activities);
        const auto activities = info.activities();
        // handle special case of "on all activities"
        if (activities.isEmpty() || activities.contains(activity)) {
            return true;
        }
    }

    return false;
}

KTextEditor::Document *KateApp::openDocUrl(const QUrl &url, const QString &encoding, bool isTempFile)
{
    KateMainWindow *mainWindow = activeKateMainWindow();

    if (!mainWindow) {
        return nullptr;
    }

    QTextCodec *codec = encoding.isEmpty() ? nullptr : QTextCodec::codecForName(encoding.toLatin1());

    // this file is no local dir, open it, else warn
    bool noDir = !url.isLocalFile() || !QFileInfo(url.toLocalFile()).isDir();

    KTextEditor::Document *doc = nullptr;

    if (noDir) {
        KateDocumentInfo docInfo;
        docInfo.doPostLoadOperations = !url.isLocalFile() && (hasCursorInArgs() || url.hasQuery());
        // open a normal file
        if (codec) {
            doc = mainWindow->viewManager()->openUrl(url, QString::fromLatin1(codec->name()), true, isTempFile, docInfo);
        } else {
            doc = mainWindow->viewManager()->openUrl(url, QString(), true, isTempFile, docInfo);
        }
    } else {
        KMessageBox::sorry(mainWindow, i18n("The file '%1' could not be opened: it is not a normal file, it is a folder.", url.url()));
    }

    return doc;
}

void KateApp::setCursorFromArgs(KTextEditor::View *view)
{
    int line = 0;
    int column = 0;
    bool nav = false;

    if (!view && !(view = activeKateMainWindow()->activeView())) {
        return;
    }

    if (m_args.isSet(QStringLiteral("line"))) {
        line = m_args.value(QStringLiteral("line")).toInt() - 1;
        nav = true;
    }

    if (m_args.isSet(QStringLiteral("column"))) {
        column = m_args.value(QStringLiteral("column")).toInt() - 1;
        nav = true;
    }

    if (nav) {
        view->setCursorPosition(KTextEditor::Cursor(line, column));
        activeKateMainWindow()->setAutoSaveSettings();
    }
}

void KateApp::setCursorFromQueryString(KTextEditor::View *view)
{
    int line = 0;
    int column = 0;
    bool nav = false;

    if (!view && !(view = activeKateMainWindow()->activeView())) {
        return;
    }

    QUrlQuery urlQuery(view->document()->url());
    QString lineStr = urlQuery.queryItemValue(QStringLiteral("line"));
    QString columnStr = urlQuery.queryItemValue(QStringLiteral("column"));

    if (!lineStr.isEmpty()) {
        line = lineStr.toInt();
        line > 0 && line--;
        nav = true;
    }

    if (!columnStr.isEmpty()) {
        column = columnStr.toInt();
        column > 0 && column--;
        nav = true;
    }

    if (nav) {
        view->setCursorPosition(KTextEditor::Cursor(line, column));
        activeKateMainWindow()->setAutoSaveSettings();
    }
}

bool KateApp::setCursor(int line, int column)
{
    KateMainWindow *mainWindow = activeKateMainWindow();

    if (!mainWindow) {
        return false;
    }

    if (auto v = mainWindow->viewManager()->activeView()) {
        v->removeSelection();
        v->setCursorPosition(KTextEditor::Cursor(line, column));
    }

    return true;
}

bool KateApp::hasCursorInArgs()
{
    return m_args.isSet(QStringLiteral("line")) || m_args.isSet(QStringLiteral("column"));
}

bool KateApp::openInput(const QString &text, const QString &encoding)
{
    activeKateMainWindow()->viewManager()->openUrl(QUrl(), encoding, true);

    if (!activeKateMainWindow()->viewManager()->activeView()) {
        return false;
    }

    KTextEditor::Document *doc = activeKateMainWindow()->viewManager()->activeView()->document();

    if (!doc) {
        return false;
    }

    return doc->setText(text);
}

KateMainWindow *KateApp::newMainWindow(KConfig *sconfig_, const QString &sgroup_)
{
    KConfig *sconfig = sconfig_ ? sconfig_ : KSharedConfig::openConfig().data();
    QString sgroup = !sgroup_.isEmpty() ? sgroup_ : QStringLiteral("MainWindow0");

    KateMainWindow *mainWindow = new KateMainWindow(sconfig, sgroup);
    mainWindow->show();

    return mainWindow;
}

void KateApp::addMainWindow(KateMainWindow *mainWindow)
{
    m_mainWindows.push_back(mainWindow);
}

void KateApp::removeMainWindow(KateMainWindow *mainWindow)
{
    m_mainWindows.removeAll(mainWindow);
}

KateMainWindow *KateApp::activeKateMainWindow()
{
    if (m_mainWindows.isEmpty()) {
        return nullptr;
    }

    int n = m_mainWindows.indexOf(static_cast<KateMainWindow *>((static_cast<QApplication *>(QCoreApplication::instance())->activeWindow())));

    if (n < 0) {
        n = 0;
    }

    return m_mainWindows[n];
}

int KateApp::mainWindowsCount() const
{
    return m_mainWindows.size();
}

int KateApp::mainWindowID(KateMainWindow *window)
{
    return m_mainWindows.indexOf(window);
}

KateMainWindow *KateApp::mainWindow(int n)
{
    if (n < m_mainWindows.size()) {
        return m_mainWindows[n];
    }

    return nullptr;
}

void KateApp::emitDocumentClosed(const QString &token)
{
    m_adaptor.emitDocumentClosed(token);
}

KTextEditor::Plugin *KateApp::plugin(const QString &name)
{
    return m_pluginManager.plugin(name);
}

bool KateApp::eventFilter(QObject *obj, QEvent *event)
{
    /**
     * handle mac os like file open
     */
    if (event->type() == QEvent::FileOpen) {
        /**
         * try to open and activate the new document, like we would do for stuff
         * opened via dbus
         */
        QFileOpenEvent *foe = static_cast<QFileOpenEvent *>(event);
        KTextEditor::Document *doc = openDocUrl(foe->url(), QString(), false);
        if (doc && activeKateMainWindow()) {
            activeKateMainWindow()->viewManager()->activateView(doc);
        }
        return true;
    }

    /**
     * else: pass over to default implementation
     */
    return QObject::eventFilter(obj, event);
}

void KateApp::remoteMessageReceived(const QString &message, QObject *)
{
    /**
     * try to parse message, ignore if no object
     */
    const QJsonDocument jsonMessage = QJsonDocument::fromJson(message.toUtf8());
    if (!jsonMessage.isObject()) {
        return;
    }

    /**
     * open all passed urls
     */
    const QJsonArray urls = jsonMessage.object().value(QLatin1String("urls")).toArray();
    for (QJsonValue urlObject : urls) {
        /**
         * get url meta data
         */
        const QUrl url = urlObject.toObject().value(QLatin1String("url")).toVariant().toUrl();
        const int line = urlObject.toObject().value(QLatin1String("line")).toVariant().toInt();
        const int column = urlObject.toObject().value(QLatin1String("column")).toVariant().toInt();

        /**
         * open file + set line/column if requested
         */
        openUrl(url, QString(), false);
        if (line >= 0 && column >= 0) {
            setCursor(line, column);
        }
    }

    // try to activate current window
    m_adaptor.activate();
}
