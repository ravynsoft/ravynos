/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>

    SPDX-License-Identifier: MIT
*/

#ifndef LSPCLIENTSERVERMANAGER_H
#define LSPCLIENTSERVERMANAGER_H

#include "lspclientplugin.h"
#include "lspclientserver.h"

#include <KTextEditor/Message>

#include <QSharedPointer>

namespace KTextEditor
{
class MainWindow;
class Document;
class View;
class MovingInterface;
}

class LSPClientRevisionSnapshot;

/*
 * A helper class that manages LSP servers in relation to a KTextDocument.
 * That is, spin up a server for a document when so requested, and then
 * monitor when the server is up (or goes down) and the document (for changes).
 * Those changes may then be synced to the server when so requested (e.g. prior
 * to another component performing an LSP request for a document).
 * So, other than managing servers, it also manages the document-server
 * relationship (and document), what's in a name ...
 */
class LSPClientServerManager : public QObject
{
    Q_OBJECT

public:
    // factory method; private implementation by interface
    static QSharedPointer<LSPClientServerManager> new_(LSPClientPlugin *plugin, KTextEditor::MainWindow *mainWin);

    virtual QSharedPointer<LSPClientServer> findServer(KTextEditor::View *view, bool updatedoc = true) = 0;

    virtual void update(KTextEditor::Document *doc, bool force) = 0;

    virtual void restart(LSPClientServer *server) = 0;

    virtual void setIncrementalSync(bool inc) = 0;

    // latest sync'ed revision of doc (-1 if N/A)
    virtual qint64 revision(KTextEditor::Document *doc) = 0;

    // lock all relevant documents' current revision and sync that to server
    // locks are released when returned snapshot is delete'd
    virtual LSPClientRevisionSnapshot *snapshot(LSPClientServer *server) = 0;

    // helper method providing descriptive label for a server
    static QString serverDescription(LSPClientServer *server)
    {
        if (server) {
            auto root = server->root().toLocalFile();
            return QStringLiteral("%1@%2").arg(server->langId(), root);
        } else {
            return {};
        }
    }

public:
Q_SIGNALS:
    void serverChanged();
    void showMessage(KTextEditor::Message::MessageType level, const QString &msg);
    // proxy server signals in case those are emitted very early
    void serverShowMessage(LSPClientServer *server, const LSPShowMessageParams &);
    void serverLogMessage(LSPClientServer *server, const LSPShowMessageParams &);
};

class LSPClientRevisionSnapshot : public QObject
{
    Q_OBJECT

public:
    // find a locked revision for url in snapshot
    virtual void find(const QUrl &url, KTextEditor::MovingInterface *&miface, qint64 &revision) const = 0;
};

#endif
