/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>

    SPDX-License-Identifier: MIT
*/

#ifndef LSPCLIENTSERVER_H
#define LSPCLIENTSERVER_H

#include "lspclientprotocol.h"

#include <QJsonValue>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QUrl>
#include <QVector>

#include <functional>
#include <optional>

namespace utils
{
// template helper
// function bind helpers
template<typename R, typename T, typename Tp, typename... Args>
inline std::function<R(Args...)> mem_fun(R (T::*pm)(Args...), Tp object)
{
    return [object, pm](Args... args) {
        return (object->*pm)(std::forward<Args>(args)...);
    };
}

template<typename R, typename T, typename Tp, typename... Args>
inline std::function<R(Args...)> mem_fun(R (T::*pm)(Args...) const, Tp object)
{
    return [object, pm](Args... args) {
        return (object->*pm)(std::forward<Args>(args)...);
    };
}

// prevent argument deduction
template<typename T>
struct identity {
    typedef T type;
};

} // namespace utils

static const int TIMEOUT_SHUTDOWN = 200;

template<typename T>
using ReplyHandler = std::function<void(const T &)>;

using ErrorReplyHandler = ReplyHandler<LSPResponseError>;
using DocumentSymbolsReplyHandler = ReplyHandler<QList<LSPSymbolInformation>>;
using DocumentDefinitionReplyHandler = ReplyHandler<QList<LSPLocation>>;
using DocumentHighlightReplyHandler = ReplyHandler<QList<LSPDocumentHighlight>>;
using DocumentHoverReplyHandler = ReplyHandler<LSPHover>;
using DocumentCompletionReplyHandler = ReplyHandler<QList<LSPCompletionItem>>;
using SignatureHelpReplyHandler = ReplyHandler<LSPSignatureHelp>;
using FormattingReplyHandler = ReplyHandler<QList<LSPTextEdit>>;
using CodeActionReplyHandler = ReplyHandler<QList<LSPCodeAction>>;
using WorkspaceEditReplyHandler = ReplyHandler<LSPWorkspaceEdit>;
using ApplyEditReplyHandler = ReplyHandler<LSPApplyWorkspaceEditResponse>;
using WorkspaceFoldersReplyHandler = ReplyHandler<QList<LSPWorkspaceFolder>>;
using SwitchSourceHeaderHandler = ReplyHandler<QString>;
using SemanticTokensDeltaReplyHandler = ReplyHandler<LSPSemanticTokensDelta>;
using WorkspaceSymbolsReplyHandler = ReplyHandler<std::vector<LSPSymbolInformation>>;

class LSPClientPlugin;

class LSPClientServer : public QObject
{
    Q_OBJECT

public:
    enum class State { None, Started, Running, Shutdown };

    class LSPClientServerPrivate;
    class RequestHandle
    {
        friend class LSPClientServerPrivate;
        QPointer<LSPClientServer> m_server;
        int m_id = -1;

    public:
        RequestHandle &cancel()
        {
            if (m_server) {
                m_server->cancel(m_id);
            }
            return *this;
        }
    };

    using FoldersType = std::optional<QList<LSPWorkspaceFolder>>;
    LSPClientServer(const QStringList &server,
                    const QUrl &root,
                    const QString &langId = QString(),
                    const QJsonValue &init = QJsonValue(),
                    const FoldersType &folders = std::nullopt);
    ~LSPClientServer() override;

    // server management
    // request start
    bool start();
    // request shutdown/stop
    // if to_xxx >= 0 -> send signal if not exit'ed after timeout
    void stop(int to_term_ms, int to_kill_ms);
    int cancel(int id);

    // properties
    const QStringList &cmdline() const;
    const QUrl &root() const;
    const QString &langId() const;
    State state() const;
    Q_SIGNAL void stateChanged(LSPClientServer *server);

    const LSPServerCapabilities &capabilities() const;

    // language
    RequestHandle documentSymbols(const QUrl &document, const QObject *context, const DocumentSymbolsReplyHandler &h, const ErrorReplyHandler &eh = nullptr);
    RequestHandle documentDefinition(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentDefinitionReplyHandler &h);
    RequestHandle documentDeclaration(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentDefinitionReplyHandler &h);
    RequestHandle documentTypeDefinition(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentDefinitionReplyHandler &h);
    RequestHandle documentImplementation(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentDefinitionReplyHandler &h);
    RequestHandle documentHighlight(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentHighlightReplyHandler &h);
    RequestHandle documentHover(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentHoverReplyHandler &h);
    RequestHandle documentReferences(const QUrl &document, const LSPPosition &pos, bool decl, const QObject *context, const DocumentDefinitionReplyHandler &h);
    RequestHandle documentCompletion(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentCompletionReplyHandler &h);
    RequestHandle signatureHelp(const QUrl &document, const LSPPosition &pos, const QObject *context, const SignatureHelpReplyHandler &h);

    // clangd specific
    RequestHandle clangdSwitchSourceHeader(const QUrl &document, const QObject *context, const SwitchSourceHeaderHandler &h);

    RequestHandle documentFormatting(const QUrl &document, const LSPFormattingOptions &options, const QObject *context, const FormattingReplyHandler &h);
    RequestHandle documentRangeFormatting(const QUrl &document,
                                          const LSPRange &range,
                                          const LSPFormattingOptions &options,
                                          const QObject *context,
                                          const FormattingReplyHandler &h);
    RequestHandle documentOnTypeFormatting(const QUrl &document,
                                           const LSPPosition &pos,
                                           QChar lastChar,
                                           const LSPFormattingOptions &options,
                                           const QObject *context,
                                           const FormattingReplyHandler &h);
    RequestHandle
    documentRename(const QUrl &document, const LSPPosition &pos, const QString &newName, const QObject *context, const WorkspaceEditReplyHandler &h);

    RequestHandle documentCodeAction(const QUrl &document,
                                     const LSPRange &range,
                                     const QList<QString> &kinds,
                                     QList<LSPDiagnostic> diagnostics,
                                     const QObject *context,
                                     const CodeActionReplyHandler &h);

    RequestHandle documentSemanticTokensFull(const QUrl &document, const QString requestId, const QObject *context, const SemanticTokensDeltaReplyHandler &h);

    RequestHandle
    documentSemanticTokensFullDelta(const QUrl &document, const QString requestId, const QObject *context, const SemanticTokensDeltaReplyHandler &h);

    RequestHandle documentSemanticTokensRange(const QUrl &document, const LSPRange &range, const QObject *context, const SemanticTokensDeltaReplyHandler &h);

    void executeCommand(const QString &command, const QJsonValue &args);

    // sync
    void didOpen(const QUrl &document, int version, const QString &langId, const QString &text);
    // only 1 of text or changes should be non-empty and is considered
    void didChange(const QUrl &document, int version, const QString &text, const QList<LSPTextDocumentContentChangeEvent> &changes = {});
    void didSave(const QUrl &document, const QString &text);
    void didClose(const QUrl &document);

    // workspace
    void didChangeConfiguration(const QJsonValue &settings);
    void didChangeWorkspaceFolders(const QList<LSPWorkspaceFolder> &added, const QList<LSPWorkspaceFolder> &removed);
    void workspaceSymbol(const QString &symbol, const QObject *context, const WorkspaceSymbolsReplyHandler &h);

    // notification = signal
Q_SIGNALS:
    void showMessage(const LSPShowMessageParams &);
    void logMessage(const LSPLogMessageParams &);
    void publishDiagnostics(const LSPPublishDiagnosticsParams &);

    // request = signal
    void applyEdit(const LSPApplyWorkspaceEditParams &req, const ApplyEditReplyHandler &h, bool &handled);
    void workspaceFolders(const WorkspaceFoldersReplyHandler &h, bool &handled);

private:
    // pimpl data holder
    LSPClientServerPrivate *const d;
};

#endif
