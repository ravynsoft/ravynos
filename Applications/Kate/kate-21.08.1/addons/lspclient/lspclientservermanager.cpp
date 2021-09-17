/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>

    SPDX-License-Identifier: MIT
*/

/* see plugins.docbook lspclient-configuration
 * for client configuration documentation
 */

#include "lspclientservermanager.h"

#include "lspclient_debug.h"

#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/MainWindow>
#include <KTextEditor/MovingInterface>
#include <KTextEditor/View>

#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QThread>
#include <QTime>
#include <QTimer>

#include <json_utils.h>

// sadly no common header for plugins to include this from
// unless we do come up with such one
typedef QMap<QString, QString> QStringMap;
Q_DECLARE_METATYPE(QStringMap)

// helper to find a proper root dir for the given document & file name that indicate the root dir
static QString rootForDocumentAndRootIndicationFileName(KTextEditor::Document *document, const QString &rootIndicationFileName)
{
    // search only feasible if document is local file
    if (!document->url().isLocalFile()) {
        return QString();
    }

    // search root upwards
    QDir dir(QFileInfo(document->url().toLocalFile()).absolutePath());
    QSet<QString> seenDirectories;
    while (!seenDirectories.contains(dir.absolutePath())) {
        // update guard
        seenDirectories.insert(dir.absolutePath());

        // the file that indicates the root dir is there => all fine
        if (dir.exists(rootIndicationFileName)) {
            return dir.absolutePath();
        }

        // else: cd up, if possible or abort
        if (!dir.cdUp()) {
            break;
        }
    }

    // no root found, bad luck
    return QString();
}

#include <memory>

// helper guard to handle revision (un)lock
struct RevisionGuard {
    QPointer<KTextEditor::Document> m_doc;
    KTextEditor::MovingInterface *m_movingInterface = nullptr;
    qint64 m_revision = -1;

    RevisionGuard(KTextEditor::Document *doc = nullptr)
        : m_doc(doc)
        , m_movingInterface(qobject_cast<KTextEditor::MovingInterface *>(doc))
    {
        Q_ASSERT(m_movingInterface);
        m_revision = m_movingInterface->revision();
        m_movingInterface->lockRevision(m_revision);
    }

    // really only need/allow this one (out of 5)
    RevisionGuard(RevisionGuard &&other)
        : RevisionGuard(nullptr)
    {
        std::swap(m_doc, other.m_doc);
        std::swap(m_movingInterface, other.m_movingInterface);
        std::swap(m_revision, other.m_revision);
    }

    void release()
    {
        m_movingInterface = nullptr;
        m_revision = -1;
    }

    ~RevisionGuard()
    {
        // NOTE: hopefully the revision is still valid at this time
        if (m_doc && m_movingInterface && m_revision >= 0) {
            m_movingInterface->unlockRevision(m_revision);
        }
    }
};

class LSPClientRevisionSnapshotImpl : public LSPClientRevisionSnapshot
{
    Q_OBJECT

    typedef LSPClientRevisionSnapshotImpl self_type;

    // std::map has more relaxed constraints on value_type
    std::map<QUrl, RevisionGuard> m_guards;

    Q_SLOT
    void clearRevisions(KTextEditor::Document *doc)
    {
        for (auto &item : m_guards) {
            if (item.second.m_doc == doc) {
                item.second.release();
            }
        }
    }

public:
    void add(KTextEditor::Document *doc)
    {
        Q_ASSERT(doc);

        // make sure revision is cleared when needed and no longer used (to unlock or otherwise)
        // see e.g. implementation in katetexthistory.cpp and assert's in place there
        // clang-format off
        auto conn = connect(doc, SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)), this, SLOT(clearRevisions(KTextEditor::Document*)));
        Q_ASSERT(conn);
        conn = connect(doc, SIGNAL(aboutToDeleteMovingInterfaceContent(KTextEditor::Document*)), this, SLOT(clearRevisions(KTextEditor::Document*)));
        Q_ASSERT(conn);
        // clang-format on
        m_guards.emplace(doc->url(), doc);
    }

    void find(const QUrl &url, KTextEditor::MovingInterface *&miface, qint64 &revision) const override
    {
        auto it = m_guards.find(url);
        if (it != m_guards.end()) {
            miface = it->second.m_movingInterface;
            revision = it->second.m_revision;
        } else {
            miface = nullptr;
            revision = -1;
        }
    }
};

// helper class to sync document changes to LSP server
class LSPClientServerManagerImpl : public LSPClientServerManager
{
    Q_OBJECT

    typedef LSPClientServerManagerImpl self_type;

    struct ServerInfo {
        QSharedPointer<LSPClientServer> server;
        // config specified server url
        QString url;
        QTime started;
        int failcount = 0;
        // pending settings to be submitted
        QJsonValue settings;
        // use of workspace folders allowed
        bool useWorkspace = false;
    };

    struct DocumentInfo {
        QSharedPointer<LSPClientServer> server;
        KTextEditor::MovingInterface *movingInterface;
        QUrl url;
        qint64 version;
        bool open : 1;
        bool modified : 1;
        // used for incremental update (if non-empty)
        QList<LSPTextDocumentContentChangeEvent> changes;
    };

    LSPClientPlugin *m_plugin;
    KTextEditor::MainWindow *m_mainWindow;
    // merged default and user config
    QJsonObject m_serverConfig;
    // root -> (mode -> server)
    QMap<QUrl, QMap<QString, ServerInfo>> m_servers;
    QHash<KTextEditor::Document *, DocumentInfo> m_docs;
    bool m_incrementalSync = false;

    // highlightingModeRegex => language id
    std::vector<std::pair<QRegularExpression, QString>> m_highlightingModeRegexToLanguageId;
    // cache of highlighting mode => language id, to avoid massive regex matching
    QHash<QString, QString> m_highlightingModeToLanguageIdCache;
    // whether to pass the language id (key) to server when opening document
    // most either do not care about the id, or can find out themselves
    // (and might get confused if we pass a not so accurate one)
    QHash<QString, bool> m_documentLanguageId;

    typedef QVector<QSharedPointer<LSPClientServer>> ServerList;

public:
    LSPClientServerManagerImpl(LSPClientPlugin *plugin, KTextEditor::MainWindow *mainWin)
        : m_plugin(plugin)
        , m_mainWindow(mainWin)
    {
        connect(plugin, &LSPClientPlugin::update, this, &self_type::updateServerConfig);
        QTimer::singleShot(100, this, &self_type::updateServerConfig);

        // stay tuned on project situation
        QObject *projectView = projectPluginView();
        if (projectView) {
            // clang-format off
            connect(projectView,
                    SIGNAL(pluginProjectAdded(QString,QString)),
                    this,
                    SLOT(onProjectAdded(QString,QString)));
            connect(projectView,
                    SIGNAL(pluginProjectRemoved(QString,QString)),
                    this,
                    SLOT(onProjectRemoved(QString,QString)));
            // clang-format on
        }
    }

    ~LSPClientServerManagerImpl() override
    {
        // stop everything as we go down
        // several stages;
        // stage 1; request shutdown of all servers (in parallel)
        // (give that some time)
        // stage 2; send TERM
        // stage 3; send KILL

        // stage 1

        /* some msleep are used below which is somewhat BAD as it blocks/hangs
         * the mainloop, however there is not much alternative:
         * + running an inner mainloop leads to event processing,
         *   which could trigger an unexpected sequence of 'events'
         *   such as (re)loading plugin that is currently still unloading
         *   (consider scenario of fast-clicking enable/disable of LSP plugin)
         * + could reduce or forego the sleep, but that increases chances
         *   on an unclean shutdown of LSP server, which may or may not
         *   be able to handle that properly (so let's try and be a polite
         *   client and try to avoid that to some degree)
         * So we are left with a minor sleep compromise ...
         */

        int count = 0;
        for (const auto &el : m_servers) {
            for (const auto &si : el) {
                auto &s = si.server;
                if (!s) {
                    continue;
                }
                disconnect(s.data(), nullptr, this, nullptr);
                if (s->state() != LSPClientServer::State::None) {
                    ++count;
                    s->stop(-1, -1);
                }
            }
        }
        if (count) {
            QThread::msleep(500);
        } else {
            return;
        }

        // stage 2 and 3
        count = 0;
        for (count = 0; count < 2; ++count) {
            bool wait = false;
            for (const auto &el : m_servers) {
                for (const auto &si : el) {
                    auto &s = si.server;
                    if (!s) {
                        continue;
                    }
                    wait = true;
                    s->stop(count == 0 ? 1 : -1, count == 0 ? -1 : 1);
                }
            }
            if (wait && count == 0) {
                QThread::msleep(100);
            }
        }
    }

    // map (highlight)mode to lsp languageId
    QString languageId(const QString &mode)
    {
        // query cache first
        const auto cacheIt = m_highlightingModeToLanguageIdCache.find(mode);
        if (cacheIt != m_highlightingModeToLanguageIdCache.end()) {
            return cacheIt.value();
        }

        // match via regexes + cache result
        for (auto it : m_highlightingModeRegexToLanguageId) {
            if (it.first.match(mode).hasMatch()) {
                m_highlightingModeToLanguageIdCache[mode] = it.second;
                return it.second;
            }
        }

        // else: we have no matching server!
        m_highlightingModeToLanguageIdCache[mode] = QString();
        return QString();
    }

    QObject *projectPluginView()
    {
        return m_mainWindow->pluginView(QStringLiteral("kateprojectplugin"));
    }

    QString documentLanguageId(const QString mode)
    {
        auto langId = languageId(mode);
        const auto it = m_documentLanguageId.find(langId);
        // FIXME ?? perhaps use default false
        // most servers can find out much better on their own
        // (though it would actually have to be confirmed as such)
        bool useId = true;
        if (it != m_documentLanguageId.end()) {
            useId = it.value();
        }

        return useId ? langId : QString();
    }

    void setIncrementalSync(bool inc) override
    {
        m_incrementalSync = inc;
    }

    QSharedPointer<LSPClientServer> findServer(KTextEditor::View *view, bool updatedoc = true) override
    {
        if (!view) {
            return nullptr;
        }

        auto document = view->document();
        if (!document || document->url().isEmpty()) {
            return nullptr;
        }

        auto it = m_docs.find(document);
        auto server = it != m_docs.end() ? it->server : nullptr;
        if (!server) {
            if ((server = _findServer(view, document))) {
                trackDocument(document, server);
            }
        }

        if (server && updatedoc) {
            update(server.data(), false);
        }
        return server;
    }

    // restart a specific server or all servers if server == nullptr
    void restart(LSPClientServer *server) override
    {
        ServerList servers;
        // find entry for server(s) and move out
        for (auto &m : m_servers) {
            for (auto it = m.begin(); it != m.end();) {
                if (!server || it->server.data() == server) {
                    servers.push_back(it->server);
                    it = m.erase(it);
                } else {
                    ++it;
                }
            }
        }
        restart(servers, server == nullptr);
    }

    qint64 revision(KTextEditor::Document *doc) override
    {
        auto it = m_docs.find(doc);
        return it != m_docs.end() ? it->version : -1;
    }

    LSPClientRevisionSnapshot *snapshot(LSPClientServer *server) override
    {
        auto result = new LSPClientRevisionSnapshotImpl;
        for (auto it = m_docs.begin(); it != m_docs.end(); ++it) {
            if (it->server == server) {
                // sync server to latest revision that will be recorded
                update(it.key(), false);
                result->add(it.key());
            }
        }
        return result;
    }

private:
    void showMessage(const QString &msg, KTextEditor::Message::MessageType level)
    {
        // inform interested view(er) which will decide how/where to show
        Q_EMIT LSPClientServerManager::showMessage(level, msg);
    }

    // caller ensures that servers are no longer present in m_servers
    void restart(const ServerList &servers, bool reload = false)
    {
        // close docs
        for (const auto &server : servers) {
            // controlling server here, so disable usual state tracking response
            disconnect(server.data(), nullptr, this, nullptr);
            for (auto it = m_docs.begin(); it != m_docs.end();) {
                auto &item = it.value();
                if (item.server == server) {
                    // no need to close if server not in proper state
                    if (server->state() != LSPClientServer::State::Running) {
                        item.open = false;
                    }
                    it = _close(it, true);
                } else {
                    ++it;
                }
            }
        }

        // helper captures servers
        auto stopservers = [servers](int t, int k) {
            for (const auto &server : servers) {
                server->stop(t, k);
            }
        };

        // trigger server shutdown now
        stopservers(-1, -1);

        // initiate delayed stages (TERM and KILL)
        // async, so give a bit more time
        QTimer::singleShot(2 * TIMEOUT_SHUTDOWN, this, [stopservers]() {
            stopservers(1, -1);
        });
        QTimer::singleShot(4 * TIMEOUT_SHUTDOWN, this, [stopservers]() {
            stopservers(-1, 1);
        });

        // as for the start part
        // trigger interested parties, which will again request a server as needed
        // let's delay this; less chance for server instances to trip over each other
        QTimer::singleShot(6 * TIMEOUT_SHUTDOWN, this, [this, reload]() {
            // this may be a good time to refresh server config
            if (reload) {
                // will also trigger as mentioned above
                updateServerConfig();
            } else {
                Q_EMIT serverChanged();
            }
        });
    }

    void onStateChanged(LSPClientServer *server)
    {
        if (server->state() == LSPClientServer::State::Running) {
            // send settings if pending
            ServerInfo *info = nullptr;
            for (auto &m : m_servers) {
                for (auto &si : m) {
                    if (si.server.data() == server) {
                        info = &si;
                        break;
                    }
                }
            }
            if (info && !info->settings.isUndefined()) {
                server->didChangeConfiguration(info->settings);
            }
            // provide initial workspace folder situation
            // this is done here because the folder notification pre-dates
            // the workspaceFolders property in 'initialize'
            // there is also no way to know whether the server supports that
            // and in fact some servers do "support workspace folders" (e.g. notification)
            // but do not know about the 'initialize' property
            // so, in summary, the notification is used here rather than the property
            const auto &caps = server->capabilities();
            if (caps.workspaceFolders.changeNotifications && info && info->useWorkspace) {
                if (auto folders = currentWorkspaceFolders(); !folders.isEmpty()) {
                    server->didChangeWorkspaceFolders(folders, {});
                }
            }
            // clear for normal operation
            Q_EMIT serverChanged();
        } else if (server->state() == LSPClientServer::State::None) {
            // went down
            // find server info to see how bad this is
            // if this is an occasional termination/crash ... ok then
            // if this happens quickly (bad/missing server, wrong cmdline/config), then no restart
            QSharedPointer<LSPClientServer> sserver;
            QString url;
            bool retry = true;
            for (auto &m : m_servers) {
                for (auto &si : m) {
                    if (si.server.data() == server) {
                        url = si.url;
                        if (si.started.secsTo(QTime::currentTime()) < 60) {
                            ++si.failcount;
                        }
                        // clear the entry, which will be re-filled if needed
                        // otherwise, leave it in place as a dead mark not to re-create one in _findServer
                        if (si.failcount < 2) {
                            std::swap(sserver, si.server);
                        } else {
                            sserver = si.server;
                            retry = false;
                        }
                    }
                }
            }
            auto action = retry ? i18n("Restarting") : i18n("NOT Restarting");
            showMessage(i18n("Server terminated unexpectedly ... %1 [%2] [homepage: %3] ", action, server->cmdline().join(QLatin1Char(' ')), url),
                        KTextEditor::Message::Warning);
            if (sserver) {
                // sserver might still be in m_servers
                // but since it died already bringing it down will have no (ill) effect
                restart({sserver});
            }
        }
    }

    QSharedPointer<LSPClientServer> _findServer(KTextEditor::View *view, KTextEditor::Document *document)
    {
        // compute the LSP standardized language id, none found => no change
        auto langId = languageId(document->highlightingMode());
        if (langId.isEmpty()) {
            return nullptr;
        }

        QObject *projectView = projectPluginView();
        // preserve raw QString value so it can be used and tested that way below
        const auto projectBase = projectView ? projectView->property("projectBaseDir").toString() : QString();
        const auto &projectMap = projectView ? projectView->property("projectMap").toMap() : QVariantMap();

        // merge with project specific
        auto projectConfig = QJsonDocument::fromVariant(projectMap).object().value(QStringLiteral("lspclient")).toObject();
        auto serverConfig = json::merge(m_serverConfig, projectConfig);

        // locate server config
        QJsonValue config;
        QSet<QString> used;
        // reduce langId
        auto realLangId = langId;
        while (true) {
            qCInfo(LSPCLIENT) << "language id " << langId;
            used << langId;
            config = serverConfig.value(QStringLiteral("servers")).toObject().value(langId);
            if (config.isObject()) {
                const auto &base = config.toObject().value(QStringLiteral("use")).toString();
                // basic cycle detection
                if (!base.isEmpty() && !used.contains(base)) {
                    langId = base;
                    continue;
                }
            }
            break;
        }

        if (!config.isObject()) {
            return nullptr;
        }

        // merge global settings
        serverConfig = json::merge(serverConfig.value(QStringLiteral("global")).toObject(), config.toObject());

        // used for variable substitution in the sequl
        // NOTE that also covers a form of environment substitution using %{ENV:XYZ}
        auto editor = KTextEditor::Editor::instance();

        std::optional<QString> rootpath;
        const auto rootv = serverConfig.value(QStringLiteral("root"));
        if (rootv.isString()) {
            auto sroot = rootv.toString();
            editor->expandText(sroot, view, sroot);
            if (QDir::isAbsolutePath(sroot)) {
                rootpath = sroot;
            } else if (!projectBase.isEmpty()) {
                rootpath = QDir(projectBase).absoluteFilePath(sroot);
            } else if (sroot.isEmpty()) {
                // empty root; so we are convinced the server can handle null rootUri
                rootpath = QString();
            } else if (const auto url = document->url(); url.isValid() && url.isLocalFile()) {
                // likewise, but use safer traditional approach and specify rootUri
                rootpath = QDir(QFileInfo(url.toLocalFile()).absolutePath()).absoluteFilePath(sroot);
            }
        }

        /**
         * no explicit set root dir? search for a matching root based on some name filters
         * this is required for some LSP servers like rls that don't handle that on their own like
         * clangd does
         */
        if (!rootpath) {
            const auto fileNamesForDetection = serverConfig.value(QStringLiteral("rootIndicationFileNames"));
            if (fileNamesForDetection.isArray()) {
                // we try each file name alternative in the listed order
                // this allows to have preferences
                for (auto name : fileNamesForDetection.toArray()) {
                    if (name.isString()) {
                        auto root = rootForDocumentAndRootIndicationFileName(document, name.toString());
                        if (!root.isEmpty()) {
                            rootpath = root;
                            break;
                        }
                    }
                }
            }
        }

        // just in case ... ensure normalized result
        if (rootpath && !rootpath->isEmpty()) {
            auto cpath = QFileInfo(*rootpath).canonicalFilePath();
            if (!cpath.isEmpty()) {
                rootpath = cpath;
            }
        }

        // is it actually safe/reasonable to use workspaces?
        // in practice, (at this time) servers do do not quite consider or support all that
        // so in that regard workspace folders represents a bit of "spec endulgance"
        // (along with quite some other aspects for that matter)
        //
        // if a server was/is able to handle a "generic root",
        //   let's assume it is safe to consider workspace folders if it explicitly claims such support
        // if, however, an explicit root was/is necessary,
        //   let's assume not safe
        // in either case, let configuration explicitly specify this
        bool useWorkspace = serverConfig.value(QStringLiteral("useWorkspace")).toBool(!rootpath ? true : false);

        // last fallback: home directory
        if (!rootpath) {
            rootpath = QDir::homePath();
        }

        auto root = rootpath && !rootpath->isEmpty() ? QUrl::fromLocalFile(*rootpath) : QUrl();
        auto &serverinfo = m_servers[root][langId];
        auto &server = serverinfo.server;

        // maybe there is a server with other root that is workspace capable
        if (!server && useWorkspace) {
            for (const auto &l : m_servers) {
                // for (auto it = l.begin(); it != l.end(); ++it) {
                auto it = l.find(langId);
                if (it != l.end()) {
                    if (auto oserver = it->server) {
                        const auto &caps = oserver->capabilities();
                        if (caps.workspaceFolders.supported && caps.workspaceFolders.changeNotifications && it->useWorkspace) {
                            // so this server can handle workspace folders and should know about project root
                            server = oserver;
                            break;
                        }
                    }
                }
            }
        }

        if (!server) {
            QStringList cmdline;

            // choose debug command line for debug mode, fallback to command
            auto vcmdline = serverConfig.value(m_plugin->m_debugMode ? QStringLiteral("commandDebug") : QStringLiteral("command"));
            if (vcmdline.isUndefined()) {
                vcmdline = serverConfig.value(QStringLiteral("command"));
            }

            auto scmdline = vcmdline.toString();
            if (!scmdline.isEmpty()) {
                cmdline = scmdline.split(QLatin1Char(' '));
            } else {
                for (const auto &c : vcmdline.toArray()) {
                    cmdline.push_back(c.toString());
                }
            }

            // some more expansion and substitution
            // unlikely to be used here, but anyway
            for (auto &e : cmdline) {
                editor->expandText(e, view, e);
            }

            if (cmdline.length() > 0) {
                // optionally search in supplied path(s)
                auto vpath = serverConfig.value(QStringLiteral("path")).toArray();
                if (vpath.size() > 0) {
                    auto cmd = QStandardPaths::findExecutable(cmdline[0]);
                    if (cmd.isEmpty()) {
                        // collect and expand in case home dir or other (environment) variable reference is used
                        QStringList path;
                        for (const auto &e : vpath) {
                            auto p = e.toString();
                            editor->expandText(p, view, p);
                            path.push_back(p);
                        }
                        cmd = QStandardPaths::findExecutable(cmdline[0], path);
                        if (!cmd.isEmpty()) {
                            cmdline[0] = cmd;
                        }
                    }
                }
                // an empty list is always passed here (or null)
                // the initial list is provided/updated using notification after start
                // since that is what a server is more aware of
                // and should support if it declares workspace folder capable
                // (as opposed to the new initialization property)
                LSPClientServer::FoldersType folders;
                if (useWorkspace) {
                    folders = QList<LSPWorkspaceFolder>();
                }
                server.reset(new LSPClientServer(cmdline, root, realLangId, serverConfig.value(QStringLiteral("initializationOptions")), folders));
                connect(server.data(), &LSPClientServer::stateChanged, this, &self_type::onStateChanged, Qt::UniqueConnection);
                if (!server->start()) {
                    QString errorMessage = i18n("Failed to start server: %1", cmdline.join(QLatin1Char(' ')));
                    const auto url = serverConfig.value(QStringLiteral("url")).toString();
                    if (!url.isEmpty()) {
                        errorMessage += QStringLiteral("\n") + i18n("Please check your PATH for the binary");
                        errorMessage += QStringLiteral("\n") + i18n("See also %1 for installation or details", url);
                    }
                    showMessage(errorMessage, KTextEditor::Message::Error);
                } else {
                    showMessage(i18n("Started server %2: %1", cmdline.join(QLatin1Char(' ')), serverDescription(server.data())),
                                KTextEditor::Message::Positive);
                    using namespace std::placeholders;
                    server->connect(server.data(), &LSPClientServer::logMessage, this, std::bind(&self_type::onMessage, this, true, _1));
                    server->connect(server.data(), &LSPClientServer::showMessage, this, std::bind(&self_type::onMessage, this, false, _1));
                    server->connect(server.data(), &LSPClientServer::workspaceFolders, this, &self_type::onWorkspaceFolders, Qt::UniqueConnection);
                }
                serverinfo.settings = serverConfig.value(QStringLiteral("settings"));
                serverinfo.started = QTime::currentTime();
                serverinfo.url = serverConfig.value(QStringLiteral("url")).toString();
                // leave failcount as-is
                serverinfo.useWorkspace = useWorkspace;
            }
        }
        return (server && server->state() == LSPClientServer::State::Running) ? server : nullptr;
    }

    void updateServerConfig()
    {
        // default configuration, compiled into plugin resource, reading can't fail
        QFile defaultConfigFile(QStringLiteral(":/lspclient/settings.json"));
        defaultConfigFile.open(QIODevice::ReadOnly);
        Q_ASSERT(defaultConfigFile.isOpen());
        m_serverConfig = QJsonDocument::fromJson(defaultConfigFile.readAll()).object();

        // consider specified configuration if existing
        const auto configPath = m_plugin->configPath().toLocalFile();
        if (!configPath.isEmpty() && QFile::exists(configPath)) {
            QFile f(configPath);
            if (f.open(QIODevice::ReadOnly)) {
                const auto data = f.readAll();
                if (!data.isEmpty()) {
                    QJsonParseError error{};
                    auto json = QJsonDocument::fromJson(data, &error);
                    if (error.error == QJsonParseError::NoError) {
                        if (json.isObject()) {
                            m_serverConfig = json::merge(m_serverConfig, json.object());
                        } else {
                            showMessage(i18n("Failed to parse server configuration '%1': no JSON object", configPath), KTextEditor::Message::Error);
                        }
                    } else {
                        showMessage(i18n("Failed to parse server configuration '%1': %2", configPath, error.errorString()), KTextEditor::Message::Error);
                    }
                }
            } else {
                showMessage(i18n("Failed to read server configuration: %1", configPath), KTextEditor::Message::Error);
            }
        }

        // build regex of highlightingMode => language id
        m_highlightingModeRegexToLanguageId.clear();
        m_highlightingModeToLanguageIdCache.clear();
        const auto servers = m_serverConfig.value(QLatin1String("servers")).toObject();
        for (auto it = servers.begin(); it != servers.end(); ++it) {
            // get highlighting mode regex for this server, if not set, fallback to just the name
            const auto &server = it.value().toObject();
            QString highlightingModeRegex = server.value(QLatin1String("highlightingModeRegex")).toString();
            if (highlightingModeRegex.isEmpty()) {
                highlightingModeRegex = it.key();
            }
            m_highlightingModeRegexToLanguageId.emplace_back(QRegularExpression(highlightingModeRegex, QRegularExpression::CaseInsensitiveOption), it.key());
            // should we use the languageId in didOpen
            auto docLanguageId = server.value(QLatin1String("documentLanguageId"));
            if (docLanguageId.isBool()) {
                m_documentLanguageId[it.key()] = docLanguageId.toBool();
            }
        }

        // we could (but do not) perform restartAll here;
        // for now let's leave that up to user
        // but maybe we do have a server now where not before, so let's signal
        Q_EMIT serverChanged();
    }

    void trackDocument(KTextEditor::Document *doc, const QSharedPointer<LSPClientServer> &server)
    {
        auto it = m_docs.find(doc);
        if (it == m_docs.end()) {
            KTextEditor::MovingInterface *miface = qobject_cast<KTextEditor::MovingInterface *>(doc);
            it = m_docs.insert(doc, {server, miface, doc->url(), 0, false, false, {}});
            // track document
            connect(doc, &KTextEditor::Document::documentUrlChanged, this, &self_type::untrack, Qt::UniqueConnection);
            connect(doc, &KTextEditor::Document::highlightingModeChanged, this, &self_type::untrack, Qt::UniqueConnection);
            connect(doc, &KTextEditor::Document::aboutToClose, this, &self_type::untrack, Qt::UniqueConnection);
            connect(doc, &KTextEditor::Document::destroyed, this, &self_type::untrack, Qt::UniqueConnection);
            connect(doc, &KTextEditor::Document::textChanged, this, &self_type::onTextChanged, Qt::UniqueConnection);
            // in case of incremental change
            connect(doc, &KTextEditor::Document::textInserted, this, &self_type::onTextInserted, Qt::UniqueConnection);
            connect(doc, &KTextEditor::Document::textRemoved, this, &self_type::onTextRemoved, Qt::UniqueConnection);
            connect(doc, &KTextEditor::Document::lineWrapped, this, &self_type::onLineWrapped, Qt::UniqueConnection);
            connect(doc, &KTextEditor::Document::lineUnwrapped, this, &self_type::onLineUnwrapped, Qt::UniqueConnection);
        } else {
            it->server = server;
        }
    }

    decltype(m_docs)::iterator _close(decltype(m_docs)::iterator it, bool remove)
    {
        if (it != m_docs.end()) {
            if (it->open) {
                // release server side (use url as registered with)
                (it->server)->didClose(it->url);
                it->open = false;
            }
            if (remove) {
                disconnect(it.key(), nullptr, this, nullptr);
                it = m_docs.erase(it);
            }
        }
        return it;
    }

    void _close(KTextEditor::Document *doc, bool remove)
    {
        auto it = m_docs.find(doc);
        if (it != m_docs.end()) {
            _close(it, remove);
        }
    }

    void untrack(QObject *doc)
    {
        _close(qobject_cast<KTextEditor::Document *>(doc), true);
        Q_EMIT serverChanged();
    }

    void close(KTextEditor::Document *doc)
    {
        _close(doc, false);
    }

    void update(const decltype(m_docs)::iterator &it, bool force)
    {
        auto doc = it.key();
        if (it != m_docs.end() && it->server) {
            it->version = it->movingInterface->revision();

            if (!m_incrementalSync) {
                it->changes.clear();
            }
            if (it->open) {
                if (it->modified || force) {
                    (it->server)->didChange(it->url, it->version, (it->changes.empty()) ? doc->text() : QString(), it->changes);
                }
            } else {
                (it->server)->didOpen(it->url, it->version, documentLanguageId(doc->highlightingMode()), doc->text());
                it->open = true;
            }
            it->modified = false;
            it->changes.clear();
        }
    }

    void update(KTextEditor::Document *doc, bool force) override
    {
        update(m_docs.find(doc), force);
    }

    void update(LSPClientServer *server, bool force)
    {
        for (auto it = m_docs.begin(); it != m_docs.end(); ++it) {
            if (it->server == server) {
                update(it, force);
            }
        }
    }

    void onTextChanged(KTextEditor::Document *doc)
    {
        auto it = m_docs.find(doc);
        if (it != m_docs.end()) {
            it->modified = true;
        }
    }

    DocumentInfo *getDocumentInfo(KTextEditor::Document *doc)
    {
        if (!m_incrementalSync) {
            return nullptr;
        }

        auto it = m_docs.find(doc);
        if (it != m_docs.end() && it->server) {
            const auto &caps = it->server->capabilities();
            if (caps.textDocumentSync == LSPDocumentSyncKind::Incremental) {
                return &(*it);
            }
        }
        return nullptr;
    }

    void onTextInserted(KTextEditor::Document *doc, const KTextEditor::Cursor &position, const QString &text)
    {
        auto info = getDocumentInfo(doc);
        if (info) {
            info->changes.push_back({LSPRange{position, position}, text});
        }
    }

    void onTextRemoved(KTextEditor::Document *doc, const KTextEditor::Range &range, const QString &text)
    {
        (void)text;
        auto info = getDocumentInfo(doc);
        if (info) {
            info->changes.push_back({range, QString()});
        }
    }

    void onLineWrapped(KTextEditor::Document *doc, const KTextEditor::Cursor &position)
    {
        // so a 'newline' has been inserted at position
        // could have been UNIX style or other kind, let's ask the document
        auto text = doc->text({position, {position.line() + 1, 0}});
        onTextInserted(doc, position, text);
    }

    void onLineUnwrapped(KTextEditor::Document *doc, int line)
    {
        // lines line-1 and line got replaced by current content of line-1
        Q_ASSERT(line > 0);
        auto info = getDocumentInfo(doc);
        if (info) {
            LSPRange oldrange{{line - 1, 0}, {line + 1, 0}};
            LSPRange newrange{{line - 1, 0}, {line, 0}};
            auto text = doc->text(newrange);
            info->changes.push_back({oldrange, text});
        }
    }

    void onMessage(bool isLog, const LSPLogMessageParams &params)
    {
        // determine server description
        auto server = dynamic_cast<LSPClientServer *>(sender());
        auto message = params.message;
        if (isLog) {
            Q_EMIT serverLogMessage(server, params);
        } else {
            Q_EMIT serverShowMessage(server, params);
        }
    }

    QList<LSPWorkspaceFolder> currentWorkspaceFolders()
    {
        QList<LSPWorkspaceFolder> folders;
        auto projectView = projectPluginView();
        if (projectView) {
            auto projectsMap = projectView->property("allProjects").value<QStringMap>();
            for (auto it = projectsMap.begin(); it != projectsMap.end(); ++it) {
                folders.push_back(workspaceFolder(it.key(), it.value()));
            }
        }
        return folders;
    }

    static LSPWorkspaceFolder workspaceFolder(const QString &baseDir, const QString &name)
    {
        return {QUrl::fromLocalFile(baseDir), name};
    }

    void updateWorkspace(bool added, const QString &baseDir, const QString &name)
    {
        qCInfo(LSPCLIENT) << "update workspace" << added << baseDir << name;
        for (const auto &u : m_servers) {
            for (const auto &si : u) {
                if (auto server = si.server) {
                    const auto &caps = server->capabilities();
                    if (caps.workspaceFolders.changeNotifications && si.useWorkspace) {
                        auto wsfolder = workspaceFolder(baseDir, name);
                        QList<LSPWorkspaceFolder> l{wsfolder}, empty;
                        server->didChangeWorkspaceFolders(added ? l : empty, added ? empty : l);
                    }
                }
            }
        }
    }

    Q_SLOT void onProjectAdded(QString baseDir, QString name)
    {
        updateWorkspace(true, baseDir, name);
    }

    Q_SLOT void onProjectRemoved(QString baseDir, QString name)
    {
        updateWorkspace(false, baseDir, name);
    }

    void onWorkspaceFolders(const WorkspaceFoldersReplyHandler &h, bool &handled)
    {
        if (handled) {
            return;
        }

        auto folders = currentWorkspaceFolders();
        h(folders);

        handled = true;
    }
};

QSharedPointer<LSPClientServerManager> LSPClientServerManager::new_(LSPClientPlugin *plugin, KTextEditor::MainWindow *mainWin)
{
    return QSharedPointer<LSPClientServerManager>(new LSPClientServerManagerImpl(plugin, mainWin));
}

#include "lspclientservermanager.moc"
