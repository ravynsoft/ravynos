/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>

    SPDX-License-Identifier: MIT
*/

#include "lspclientserver.h"

#include "lspclient_debug.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#include <utility>

// good/bad old school; allows easier concatenate
#define CONTENT_LENGTH "Content-Length"

static const QString MEMBER_ID = QStringLiteral("id");
static const QString MEMBER_METHOD = QStringLiteral("method");
static const QString MEMBER_ERROR = QStringLiteral("error");
static const QString MEMBER_CODE = QStringLiteral("code");
static const QString MEMBER_MESSAGE = QStringLiteral("message");
static const QString MEMBER_PARAMS = QStringLiteral("params");
static const QString MEMBER_RESULT = QStringLiteral("result");
static const QString MEMBER_URI = QStringLiteral("uri");
static const QString MEMBER_VERSION = QStringLiteral("version");
static const QString MEMBER_START = QStringLiteral("start");
static const QString MEMBER_END = QStringLiteral("end");
static const QString MEMBER_POSITION = QStringLiteral("position");
static const QString MEMBER_LOCATION = QStringLiteral("location");
static const QString MEMBER_RANGE = QStringLiteral("range");
static const QString MEMBER_LINE = QStringLiteral("line");
static const QString MEMBER_CHARACTER = QStringLiteral("character");
static const QString MEMBER_KIND = QStringLiteral("kind");
static const QString MEMBER_TEXT = QStringLiteral("text");
static const QString MEMBER_LANGID = QStringLiteral("languageId");
static const QString MEMBER_LABEL = QStringLiteral("label");
static const QString MEMBER_DOCUMENTATION = QStringLiteral("documentation");
static const QString MEMBER_DETAIL = QStringLiteral("detail");
static const QString MEMBER_COMMAND = QStringLiteral("command");
static const QString MEMBER_EDIT = QStringLiteral("edit");
static const QString MEMBER_TITLE = QStringLiteral("title");
static const QString MEMBER_ARGUMENTS = QStringLiteral("arguments");
static const QString MEMBER_DIAGNOSTICS = QStringLiteral("diagnostics");
static const QString MEMBER_TARGET_URI = QStringLiteral("targetUri");
static const QString MEMBER_TARGET_RANGE = QStringLiteral("targetRange");
static const QString MEMBER_TARGET_SELECTION_RANGE = QStringLiteral("targetSelectionRange");
static const QString MEMBER_PREVIOUS_RESULT_ID = QStringLiteral("previousResultId");
static const QString MEMBER_QUERY = QStringLiteral("query");

// message construction helpers
static QJsonObject to_json(const LSPPosition &pos)
{
    return QJsonObject{{MEMBER_LINE, pos.line()}, {MEMBER_CHARACTER, pos.column()}};
}

static QJsonObject to_json(const LSPRange &range)
{
    return QJsonObject{{MEMBER_START, to_json(range.start())}, {MEMBER_END, to_json(range.end())}};
}

static QJsonValue to_json(const LSPLocation &location)
{
    if (location.uri.isValid()) {
        return QJsonObject{{MEMBER_URI, location.uri.toString()}, {MEMBER_RANGE, to_json(location.range)}};
    }
    return QJsonValue();
}

static QJsonValue to_json(const LSPDiagnosticRelatedInformation &related)
{
    auto loc = to_json(related.location);
    if (loc.isObject()) {
        return QJsonObject{{MEMBER_LOCATION, to_json(related.location)}, {MEMBER_MESSAGE, related.message}};
    }
    return QJsonValue();
}

static QJsonObject to_json(const LSPDiagnostic &diagnostic)
{
    // required
    auto result = QJsonObject();
    result[MEMBER_RANGE] = to_json(diagnostic.range);
    result[MEMBER_MESSAGE] = diagnostic.message;
    // optional
    if (!diagnostic.code.isEmpty()) {
        result[QStringLiteral("code")] = diagnostic.code;
    }
    if (diagnostic.severity != LSPDiagnosticSeverity::Unknown) {
        result[QStringLiteral("severity")] = static_cast<int>(diagnostic.severity);
    }
    if (!diagnostic.source.isEmpty()) {
        result[QStringLiteral("source")] = diagnostic.source;
    }
    QJsonArray relatedInfo;
    for (const auto &vrelated : diagnostic.relatedInformation) {
        auto related = to_json(vrelated);
        if (related.isObject()) {
            relatedInfo.push_back(related);
        }
    }
    result[QStringLiteral("relatedInformation")] = relatedInfo;
    return result;
}

static QJsonArray to_json(const QList<LSPTextDocumentContentChangeEvent> &changes)
{
    QJsonArray result;
    for (const auto &change : changes) {
        result.push_back(QJsonObject{{MEMBER_RANGE, to_json(change.range)}, {MEMBER_TEXT, change.text}});
    }
    return result;
}

static QJsonObject versionedTextDocumentIdentifier(const QUrl &document, int version = -1)
{
    QJsonObject map{{MEMBER_URI, document.toString()}};
    if (version >= 0) {
        map[MEMBER_VERSION] = version;
    }
    return map;
}

static QJsonObject textDocumentItem(const QUrl &document, const QString &lang, const QString &text, int version)
{
    auto map = versionedTextDocumentIdentifier(document, version);
    map[MEMBER_TEXT] = text;
    map[MEMBER_LANGID] = lang;
    return map;
}

static QJsonObject textDocumentParams(const QJsonObject &m)
{
    return QJsonObject{{QStringLiteral("textDocument"), m}};
}

static QJsonObject textDocumentParams(const QUrl &document, int version = -1)
{
    return textDocumentParams(versionedTextDocumentIdentifier(document, version));
}

static QJsonObject textDocumentPositionParams(const QUrl &document, LSPPosition pos)
{
    auto params = textDocumentParams(document);
    params[MEMBER_POSITION] = to_json(pos);
    return params;
}

static QJsonObject referenceParams(const QUrl &document, LSPPosition pos, bool decl)
{
    auto params = textDocumentPositionParams(document, pos);
    params[QStringLiteral("context")] = QJsonObject{{QStringLiteral("includeDeclaration"), decl}};
    return params;
}

static QJsonObject formattingOptions(const LSPFormattingOptions &_options)
{
    auto options = _options.extra;
    options[QStringLiteral("tabSize")] = _options.tabSize;
    options[QStringLiteral("insertSpaces")] = _options.insertSpaces;
    return options;
}

static QJsonObject documentRangeFormattingParams(const QUrl &document, const LSPRange *range, const LSPFormattingOptions &_options)
{
    auto params = textDocumentParams(document);
    if (range) {
        params[MEMBER_RANGE] = to_json(*range);
    }
    params[QStringLiteral("options")] = formattingOptions(_options);
    return params;
}

static QJsonObject documentOnTypeFormattingParams(const QUrl &document, const LSPPosition &pos, const QChar &lastChar, const LSPFormattingOptions &_options)
{
    auto params = textDocumentPositionParams(document, pos);
    params[QStringLiteral("ch")] = QString(lastChar);
    params[QStringLiteral("options")] = formattingOptions(_options);
    return params;
}

static QJsonObject renameParams(const QUrl &document, const LSPPosition &pos, const QString &newName)
{
    auto params = textDocumentPositionParams(document, pos);
    params[QStringLiteral("newName")] = newName;
    return params;
}

static QJsonObject codeActionParams(const QUrl &document, const LSPRange &range, const QList<QString> &kinds, const QList<LSPDiagnostic> &diagnostics)
{
    auto params = textDocumentParams(document);
    params[MEMBER_RANGE] = to_json(range);
    QJsonObject context;
    QJsonArray diags;
    for (const auto &diagnostic : diagnostics) {
        diags.push_back(to_json(diagnostic));
    }
    context[MEMBER_DIAGNOSTICS] = diags;
    if (kinds.length()) {
        context[QStringLiteral("only")] = QJsonArray::fromStringList(kinds);
    }
    params[QStringLiteral("context")] = context;
    return params;
}

static QJsonObject executeCommandParams(const QString &command, const QJsonValue &args)
{
    return QJsonObject{{MEMBER_COMMAND, command}, {MEMBER_ARGUMENTS, args}};
}

static QJsonObject applyWorkspaceEditResponse(const LSPApplyWorkspaceEditResponse &response)
{
    return QJsonObject{{QStringLiteral("applied"), response.applied}, {QStringLiteral("failureReason"), response.failureReason}};
}

static QJsonObject workspaceFolder(const LSPWorkspaceFolder &response)
{
    return QJsonObject{{MEMBER_URI, response.uri.toString()}, {QStringLiteral("name"), response.name}};
}

static QJsonObject changeConfigurationParams(const QJsonValue &settings)
{
    return QJsonObject{{QStringLiteral("settings"), settings}};
}

static QJsonArray to_json(const QList<LSPWorkspaceFolder> &l)
{
    QJsonArray result;
    for (const auto &e : l) {
        result.push_back(workspaceFolder(e));
    }
    return result;
}

static QJsonObject changeWorkspaceFoldersParams(const QList<LSPWorkspaceFolder> &added, const QList<LSPWorkspaceFolder> &removed)
{
    QJsonObject event;
    event[QStringLiteral("added")] = to_json(added);
    event[QStringLiteral("removed")] = to_json(removed);
    return QJsonObject{{QStringLiteral("event"), event}};
}

static void from_json(QVector<QChar> &trigger, const QJsonValue &json)
{
    for (const auto &t : json.toArray()) {
        auto st = t.toString();
        if (st.length()) {
            trigger.push_back(st.at(0));
        }
    }
}

static void from_json(LSPCompletionOptions &options, const QJsonValue &json)
{
    if (json.isObject()) {
        auto ob = json.toObject();
        options.provider = true;
        options.resolveProvider = ob.value(QStringLiteral("resolveProvider")).toBool();
        from_json(options.triggerCharacters, ob.value(QStringLiteral("triggerCharacters")));
    }
}

static void from_json(LSPSignatureHelpOptions &options, const QJsonValue &json)
{
    if (json.isObject()) {
        auto ob = json.toObject();
        options.provider = true;
        from_json(options.triggerCharacters, ob.value(QStringLiteral("triggerCharacters")));
    }
}

static void from_json(LSPDocumentOnTypeFormattingOptions &options, const QJsonValue &json)
{
    if (json.isObject()) {
        auto ob = json.toObject();
        options.provider = true;
        from_json(options.triggerCharacters, ob.value(QStringLiteral("moreTriggerCharacter")));
        auto trigger = ob.value(QStringLiteral("firstTriggerCharacter")).toString();
        if (trigger.size()) {
            options.triggerCharacters.insert(0, trigger.at(0));
        }
    }
}

static void from_json(LSPWorkspaceFoldersServerCapabilities &options, const QJsonValue &json)
{
    if (json.isObject()) {
        auto ob = json.toObject();
        options.supported = ob.value(QStringLiteral("supported")).toBool();
        auto notify = ob.value(QStringLiteral("changeNotifications"));
        options.changeNotifications = notify.isString() ? notify.toString().size() > 0 : notify.toBool();
    }
}

static void from_json(LSPSemanticTokensOptions &options, const QJsonObject &json)
{
    if (json.isEmpty()) {
        return;
    }

    if (json.value(QStringLiteral("full")).isObject()) {
        auto full = json.value(QStringLiteral("full")).toObject();
        options.fullDelta = full.value(QStringLiteral("delta")).toBool();
    } else {
        options.full = json.value(QStringLiteral("full")).toBool();
    }
    options.range = json.value(QStringLiteral("range")).toBool();

    const auto legend = json.value(QStringLiteral("legend")).toObject();
    const auto tokenTypes = legend.value(QStringLiteral("tokenTypes")).toArray();

    std::vector<QString> types;
    types.reserve(tokenTypes.size());
    std::transform(tokenTypes.cbegin(), tokenTypes.cend(), std::back_inserter(types), [](const QJsonValue &jv) {
        return jv.toString();
    });
    //     options.types = QVector<QString>(types.begin(), types.end());
    options.legend.initialize(types);

    // Disabled

    //     const auto tokenMods = legend.value(QStringLiteral("tokenModifiers")).toArray();
    //     std::vector<QString> modifiers;
    //     modifiers.reserve(tokenMods.size());
    //     std::transform(tokenMods.cbegin(), tokenMods.cend(), std::back_inserter(modifiers), [](const QJsonValue &jv) {
    //         return jv.toString();
    //     });
}

static void from_json(LSPServerCapabilities &caps, const QJsonObject &json)
{
    // in older protocol versions a support option is simply a boolean
    // in newer version it may be an object instead;
    // it should not be sent unless such support is announced, but let's handle it anyway
    // so consider an object there as a (good?) sign that the server is suitably capable
    auto toBoolOrObject = [](const QJsonValue &value) {
        return value.toBool() || value.isObject();
    };

    auto sync = json.value(QStringLiteral("textDocumentSync"));
    caps.textDocumentSync = static_cast<LSPDocumentSyncKind>(
        (sync.isObject() ? sync.toObject().value(QStringLiteral("change")) : sync).toInt(static_cast<int>(LSPDocumentSyncKind::None)));
    caps.hoverProvider = toBoolOrObject(json.value(QStringLiteral("hoverProvider")));
    from_json(caps.completionProvider, json.value(QStringLiteral("completionProvider")));
    from_json(caps.signatureHelpProvider, json.value(QStringLiteral("signatureHelpProvider")));
    caps.definitionProvider = toBoolOrObject(json.value(QStringLiteral("definitionProvider")));
    caps.declarationProvider = toBoolOrObject(json.value(QStringLiteral("declarationProvider")));
    caps.typeDefinitionProvider = toBoolOrObject(json.value(QStringLiteral("typeDefinitionProvider")));
    caps.referencesProvider = toBoolOrObject(json.value(QStringLiteral("referencesProvider")));
    caps.implementationProvider = toBoolOrObject(json.value(QStringLiteral("implementationProvider")));
    caps.documentSymbolProvider = toBoolOrObject(json.value(QStringLiteral("documentSymbolProvider")));
    caps.documentHighlightProvider = toBoolOrObject(json.value(QStringLiteral("documentHighlightProvider")));
    caps.documentFormattingProvider = toBoolOrObject(json.value(QStringLiteral("documentFormattingProvider")));
    caps.documentRangeFormattingProvider = toBoolOrObject(json.value(QStringLiteral("documentRangeFormattingProvider")));
    from_json(caps.documentOnTypeFormattingProvider, json.value(QStringLiteral("documentOnTypeFormattingProvider")));
    caps.renameProvider = toBoolOrObject(json.value(QStringLiteral("renameProvider")));
    auto codeActionProvider = json.value(QStringLiteral("codeActionProvider"));
    caps.codeActionProvider = codeActionProvider.toBool() || codeActionProvider.isObject();
    from_json(caps.semanticTokenProvider, json.value(QStringLiteral("semanticTokensProvider")).toObject());
    auto workspace = json.value(QStringLiteral("workspace")).toObject();
    from_json(caps.workspaceFolders, workspace.value(QStringLiteral("workspaceFolders")));
}

// follow suit; as performed in kate docmanager
// normalize at this stage/layer to avoid surprises elsewhere
// sadly this is not a single QUrl method as one might hope ...
static QUrl normalizeUrl(const QUrl &url)
{
    // Resolve symbolic links for local files (done anyway in KTextEditor)
    if (url.isLocalFile()) {
        QString normalizedUrl = QFileInfo(url.toLocalFile()).canonicalFilePath();
        if (!normalizedUrl.isEmpty()) {
            return QUrl::fromLocalFile(normalizedUrl);
        }
    }

    // else: cleanup only the .. stuff
    return url.adjusted(QUrl::NormalizePathSegments);
}

static LSPResponseError parseResponseError(const QJsonValue &v)
{
    LSPResponseError ret;
    if (v.isObject()) {
        const auto &vm = v.toObject();
        ret.code = LSPErrorCode(vm.value(MEMBER_CODE).toInt());
        ret.message = vm.value(MEMBER_MESSAGE).toString();
        ret.data = vm.value(QStringLiteral("data"));
    }
    return ret;
}

static LSPMarkupContent parseMarkupContent(const QJsonValue &v)
{
    LSPMarkupContent ret;
    if (v.isObject()) {
        const auto &vm = v.toObject();
        ret.value = vm.value(QStringLiteral("value")).toString();
        auto kind = vm.value(MEMBER_KIND).toString();
        if (kind == QLatin1String("plaintext")) {
            ret.kind = LSPMarkupKind::PlainText;
        } else if (kind == QLatin1String("markdown")) {
            ret.kind = LSPMarkupKind::MarkDown;
        }
    } else if (v.isString()) {
        ret.kind = LSPMarkupKind::PlainText;
        ret.value = v.toString();
    }
    return ret;
}

static LSPPosition parsePosition(const QJsonObject &m)
{
    auto line = m.value(MEMBER_LINE).toInt(-1);
    auto column = m.value(MEMBER_CHARACTER).toInt(-1);
    return {line, column};
}

static bool isPositionValid(const LSPPosition &pos)
{
    return pos.isValid();
}

static LSPRange parseRange(const QJsonObject &range)
{
    auto startpos = parsePosition(range.value(MEMBER_START).toObject());
    auto endpos = parsePosition(range.value(MEMBER_END).toObject());
    return {startpos, endpos};
}

static LSPLocation parseLocation(const QJsonObject &loc)
{
    auto uri = normalizeUrl(QUrl(loc.value(MEMBER_URI).toString()));
    auto range = parseRange(loc.value(MEMBER_RANGE).toObject());
    return {QUrl(uri), range};
}

static LSPLocation parseLocationLink(const QJsonObject &loc)
{
    auto uri = normalizeUrl(QUrl(loc.value(MEMBER_TARGET_URI).toString()));
    // both should be present, selection contained by the other
    // so let's preferentially pick the smallest one
    auto vrange = loc.value(MEMBER_TARGET_SELECTION_RANGE);
    if (vrange.isUndefined()) {
        vrange = loc.value(MEMBER_TARGET_RANGE);
    }
    auto range = parseRange(vrange.toObject());
    return {QUrl(uri), range};
}

static LSPDocumentHighlight parseDocumentHighlight(const QJsonValue &result)
{
    auto hover = result.toObject();
    auto range = parseRange(hover.value(MEMBER_RANGE).toObject());
    // default is DocumentHighlightKind.Text
    auto kind = static_cast<LSPDocumentHighlightKind>(hover.value(MEMBER_KIND).toInt(static_cast<int>(LSPDocumentHighlightKind::Text)));
    return {range, kind};
}

static QList<LSPDocumentHighlight> parseDocumentHighlightList(const QJsonValue &result)
{
    QList<LSPDocumentHighlight> ret;
    // could be array
    if (result.isArray()) {
        for (const auto &def : result.toArray()) {
            ret.push_back(parseDocumentHighlight(def));
        }
    } else if (result.isObject()) {
        // or a single value
        ret.push_back(parseDocumentHighlight(result));
    }
    return ret;
}

static LSPMarkupContent parseHoverContentElement(const QJsonValue &contents)
{
    LSPMarkupContent result;
    if (contents.isString()) {
        result.value = contents.toString();
    } else {
        // should be object, pretend so
        auto cont = contents.toObject();
        auto text = cont.value(QStringLiteral("value")).toString();
        if (text.isEmpty()) {
            // nothing to lose, try markdown
            result = parseMarkupContent(contents);
        } else {
            result.value = text;
        }
    }
    if (result.value.length()) {
        result.kind = LSPMarkupKind::PlainText;
    }
    return result;
}

static LSPHover parseHover(const QJsonValue &result)
{
    LSPHover ret;
    auto hover = result.toObject();
    // normalize content which can be of many forms
    ret.range = parseRange(hover.value(MEMBER_RANGE).toObject());
    auto contents = hover.value(QStringLiteral("contents"));

    // support the deprecated MarkedString[] variant, used by e.g. Rust rls
    if (contents.isArray()) {
        for (const auto &c : contents.toArray()) {
            ret.contents.push_back(parseHoverContentElement(c));
        }
    } else {
        ret.contents.push_back(parseHoverContentElement(contents));
    }

    return ret;
}

static QList<LSPSymbolInformation> parseDocumentSymbols(const QJsonValue &result)
{
    // the reply could be old SymbolInformation[] or new (hierarchical) DocumentSymbol[]
    // try to parse it adaptively in any case
    // if new style, hierarchy is specified clearly in reply
    // if old style, it is assumed the values enter linearly, that is;
    // * a parent/container is listed before its children
    // * if a name is defined/declared several times and then used as a parent,
    //   then we try to find such a parent whose range contains current range
    //   (otherwise fall back to using the last instance as a parent)

    QList<LSPSymbolInformation> ret;
    QMultiMap<QString, LSPSymbolInformation *> index;

    std::function<void(const QJsonObject &symbol, LSPSymbolInformation *parent)> parseSymbol = [&](const QJsonObject &symbol, LSPSymbolInformation *parent) {
        const auto &location = symbol.value(MEMBER_LOCATION).toObject();
        const auto &mrange = symbol.contains(MEMBER_RANGE) ? symbol.value(MEMBER_RANGE) : location.value(MEMBER_RANGE);
        auto range = parseRange(mrange.toObject());
        // if flat list, try to find parent by name
        if (!parent) {
            auto container = symbol.value(QStringLiteral("containerName")).toString();
            auto it = index.find(container);
            // default to last inserted
            if (it != index.end()) {
                parent = it.value();
            }
            // but prefer a containing range
            while (it != index.end() && it.key() == container) {
                if (it.value()->range.contains(range)) {
                    parent = it.value();
                    break;
                }
                ++it;
            }
        }
        auto list = parent ? &parent->children : &ret;
        if (isPositionValid(range.start()) && isPositionValid(range.end())) {
            auto name = symbol.value(QStringLiteral("name")).toString();
            auto kind = static_cast<LSPSymbolKind>(symbol.value(MEMBER_KIND).toInt());
            auto detail = symbol.value(MEMBER_DETAIL).toString();
            list->push_back({name, kind, range, detail});
            index.insert(name, &list->back());
            // proceed recursively
            for (const auto &child : symbol.value(QStringLiteral("children")).toArray()) {
                parseSymbol(child.toObject(), &list->back());
            }
        }
    };

    for (const auto &info : result.toArray()) {
        parseSymbol(info.toObject(), nullptr);
    }
    return ret;
}

static QList<LSPLocation> parseDocumentLocation(const QJsonValue &result)
{
    QList<LSPLocation> ret;
    // could be array
    if (result.isArray()) {
        for (const auto &def : result.toArray()) {
            const auto &ob = def.toObject();
            ret.push_back(parseLocation(ob));
            // bogus server might have sent LocationLink[] instead
            // let's try to handle it, but not announce in capabilities
            if (ret.back().uri.isEmpty()) {
                ret.back() = parseLocationLink(ob);
            }
        }
    } else if (result.isObject()) {
        // or a single value
        ret.push_back(parseLocation(result.toObject()));
    }
    return ret;
}

static QList<LSPCompletionItem> parseDocumentCompletion(const QJsonValue &result)
{
    QList<LSPCompletionItem> ret;
    QJsonArray items = result.toArray();
    // might be CompletionList
    if (items.empty()) {
        items = result.toObject().value(QStringLiteral("items")).toArray();
    }

    //     auto parseTextEdit = [](const QJsonObject &obj) -> LSPTextEdit {
    //         auto newText = obj.value(QStringLiteral("newText")).toString();
    //         auto range = parseRange(obj.value(QStringLiteral("range")).toObject());
    //         return LSPTextEdit{range, newText};
    //     };

    for (const auto &vitem : items) {
        const auto &item = vitem.toObject();
        auto label = item.value(MEMBER_LABEL).toString();
        auto detail = item.value(MEMBER_DETAIL).toString();
        auto doc = parseMarkupContent(item.value(MEMBER_DOCUMENTATION));
        auto sortText = item.value(QStringLiteral("sortText")).toString();
        if (sortText.isEmpty()) {
            sortText = label;
        }
        auto insertText = item.value(QStringLiteral("insertText")).toString();
        if (insertText.isEmpty()) {
            insertText = label;
        }
        auto kind = static_cast<LSPCompletionItemKind>(item.value(MEMBER_KIND).toInt());
        //         auto textEdit = parseTextEdit(item.value(QStringLiteral("textEdit")).toObject());
        ret.push_back({label, kind, detail, doc, sortText, insertText /*, textEdit*/});
    }
    return ret;
}

static LSPSignatureInformation parseSignatureInformation(const QJsonObject &json)
{
    LSPSignatureInformation info;

    info.label = json.value(MEMBER_LABEL).toString();
    info.documentation = parseMarkupContent(json.value(MEMBER_DOCUMENTATION));
    for (const auto &rpar : json.value(QStringLiteral("parameters")).toArray()) {
        auto par = rpar.toObject();
        auto label = par.value(MEMBER_LABEL);
        int begin = -1, end = -1;
        if (label.isArray()) {
            auto range = label.toArray();
            if (range.size() == 2) {
                begin = range.at(0).toInt(-1);
                end = range.at(1).toInt(-1);
                if (begin > info.label.length()) {
                    begin = -1;
                }
                if (end > info.label.length()) {
                    end = -1;
                }
            }
        } else {
            auto sub = label.toString();
            if (sub.length()) {
                begin = info.label.indexOf(sub);
                if (begin >= 0) {
                    end = begin + sub.length();
                }
            }
        }
        info.parameters.push_back({begin, end});
    }
    return info;
}

static LSPSignatureHelp parseSignatureHelp(const QJsonValue &result)
{
    LSPSignatureHelp ret;
    QJsonObject sig = result.toObject();

    for (const auto &info : sig.value(QStringLiteral("signatures")).toArray()) {
        ret.signatures.push_back(parseSignatureInformation(info.toObject()));
    }
    ret.activeSignature = sig.value(QStringLiteral("activeSignature")).toInt(0);
    ret.activeParameter = sig.value(QStringLiteral("activeParameter")).toInt(0);
    ret.activeSignature = qMin(qMax(ret.activeSignature, 0), ret.signatures.size());
    ret.activeParameter = qMax(ret.activeParameter, 0);
    if (!ret.signatures.isEmpty()) {
        ret.activeParameter = qMin(ret.activeParameter, ret.signatures.at(ret.activeSignature).parameters.size());
    }
    return ret;
}

static QString parseClangdSwitchSourceHeader(const QJsonValue &result)
{
    return result.toString();
}

static QList<LSPTextEdit> parseTextEdit(const QJsonValue &result)
{
    QList<LSPTextEdit> ret;
    for (const auto &redit : result.toArray()) {
        auto edit = redit.toObject();
        auto text = edit.value(QStringLiteral("newText")).toString();
        auto range = parseRange(edit.value(MEMBER_RANGE).toObject());
        ret.push_back({range, text});
    }
    return ret;
}

static LSPWorkspaceEdit parseWorkSpaceEdit(const QJsonValue &result)
{
    QHash<QUrl, QList<LSPTextEdit>> ret;
    auto changes = result.toObject().value(QStringLiteral("changes")).toObject();
    for (auto it = changes.begin(); it != changes.end(); ++it) {
        ret.insert(normalizeUrl(QUrl(it.key())), parseTextEdit(it.value()));
    }
    return {ret};
}

static LSPCommand parseCommand(const QJsonObject &result)
{
    auto title = result.value(MEMBER_TITLE).toString();
    auto command = result.value(MEMBER_COMMAND).toString();
    auto args = result.value(MEMBER_ARGUMENTS).toArray();
    return {title, command, args};
}

static QList<LSPDiagnostic> parseDiagnostics(const QJsonArray &result)
{
    QList<LSPDiagnostic> ret;
    for (const auto &vdiag : result) {
        auto diag = vdiag.toObject();
        auto range = parseRange(diag.value(MEMBER_RANGE).toObject());
        auto severity = static_cast<LSPDiagnosticSeverity>(diag.value(QStringLiteral("severity")).toInt());
        auto code = diag.value(QStringLiteral("code")).toString();
        auto source = diag.value(QStringLiteral("source")).toString();
        auto message = diag.value(MEMBER_MESSAGE).toString();
        auto relatedInfo = diag.value(QStringLiteral("relatedInformation")).toArray();
        QList<LSPDiagnosticRelatedInformation> relatedInfoList;
        for (const auto &vrelated : relatedInfo) {
            auto related = vrelated.toObject();
            auto relLocation = parseLocation(related.value(MEMBER_LOCATION).toObject());
            auto relMessage = related.value(MEMBER_MESSAGE).toString();
            relatedInfoList.push_back({relLocation, relMessage});
        }
        ret.push_back({range, severity, code, source, message, relatedInfoList});
    }
    return ret;
}

static QList<LSPCodeAction> parseCodeAction(const QJsonValue &result)
{
    QList<LSPCodeAction> ret;
    for (const auto &vaction : result.toArray()) {
        auto action = vaction.toObject();
        // entry could be Command or CodeAction
        if (!action.value(MEMBER_COMMAND).isString()) {
            // CodeAction
            auto title = action.value(MEMBER_TITLE).toString();
            auto kind = action.value(MEMBER_KIND).toString();
            auto command = parseCommand(action.value(MEMBER_COMMAND).toObject());
            auto edit = parseWorkSpaceEdit(action.value(MEMBER_EDIT));
            auto diagnostics = parseDiagnostics(action.value(MEMBER_DIAGNOSTICS).toArray());
            ret.push_back({title, kind, diagnostics, edit, command});
        } else {
            // Command
            auto command = parseCommand(action);
            ret.push_back({command.title, QString(), {}, {}, command});
        }
    }
    return ret;
}

static QJsonArray supportedSemanticTokenTypes()
{
    return QJsonArray({QStringLiteral("namespace"), QStringLiteral("type"),     QStringLiteral("class"),         QStringLiteral("enum"),
                       QStringLiteral("interface"), QStringLiteral("struct"),   QStringLiteral("typeParameter"), QStringLiteral("parameter"),
                       QStringLiteral("variable"),  QStringLiteral("property"), QStringLiteral("enumMember"),    QStringLiteral("event"),
                       QStringLiteral("function"),  QStringLiteral("method"),   QStringLiteral("macro"),         QStringLiteral("keyword"),
                       QStringLiteral("modifier"),  QStringLiteral("comment"),  QStringLiteral("string"),        QStringLiteral("number"),
                       QStringLiteral("regexp"),    QStringLiteral("operator")});
}

/**
 * Used for both delta and full
 */
static LSPSemanticTokensDelta parseSemanticTokensDelta(const QJsonValue &result)
{
    LSPSemanticTokensDelta ret;
    auto json = result.toObject();
    ret.resultId = json.value(QStringLiteral("resultId")).toString();

    auto edits = json.value(QStringLiteral("edits")).toArray();

    for (const auto &edit_jsonValue : edits) {
        if (!edit_jsonValue.isObject()) {
            continue;
        }

        auto edit = edit_jsonValue.toObject();

        LSPSemanticTokensEdit e;
        e.start = edit.value(QStringLiteral("start")).toInt();
        e.deleteCount = edit.value(QStringLiteral("deleteCount")).toInt();

        auto data = edit.value(QStringLiteral("data")).toArray();
        e.data.reserve(data.size());
        std::transform(data.cbegin(), data.cend(), std::back_inserter(e.data), [](const QJsonValue &jv) {
            return jv.toInt();
        });

        ret.edits.push_back(e);
    }

    auto data = json.value(QStringLiteral("data")).toArray();
    ret.data.reserve(data.size());
    std::transform(data.cbegin(), data.cend(), std::back_inserter(ret.data), [](const QJsonValue &jv) {
        return jv.toInt();
    });

    return ret;
}

static LSPPublishDiagnosticsParams parseDiagnostics(const QJsonObject &result)
{
    LSPPublishDiagnosticsParams ret;

    ret.uri = normalizeUrl(QUrl(result.value(MEMBER_URI).toString()));
    ret.diagnostics = parseDiagnostics(result.value(MEMBER_DIAGNOSTICS).toArray());
    return ret;
}

static LSPApplyWorkspaceEditParams parseApplyWorkspaceEditParams(const QJsonObject &result)
{
    LSPApplyWorkspaceEditParams ret;

    ret.label = result.value(MEMBER_LABEL).toString();
    ret.edit = parseWorkSpaceEdit(result.value(MEMBER_EDIT));
    return ret;
}

static LSPShowMessageParams parseMessage(const QJsonObject &result)
{
    LSPShowMessageParams ret;

    ret.type = static_cast<LSPMessageType>(result.value(QStringLiteral("type")).toInt());
    ret.message = result.value(MEMBER_MESSAGE).toString();
    return ret;
}

static std::vector<LSPSymbolInformation> parseWorkspaceSymbols(const QJsonValue &result)
{
    auto res = result.toArray();

    std::vector<LSPSymbolInformation> symbols;
    symbols.reserve(res.size());

    std::transform(res.cbegin(), res.cend(), std::back_inserter(symbols), [](const QJsonValue &jv) {
        auto symbol = jv.toObject();

        LSPSymbolInformation symInfo;

        const auto location = symbol.value(MEMBER_LOCATION).toObject();
        const auto mrange = symbol.contains(MEMBER_RANGE) ? symbol.value(MEMBER_RANGE) : location.value(MEMBER_RANGE);

        auto containerName = symbol.value(QStringLiteral("containerName")).toString();
        if (!containerName.isEmpty()) {
            containerName.append(QStringLiteral("::"));
        }
        symInfo.name = containerName + symbol.value(QStringLiteral("name")).toString();
        symInfo.kind = (LSPSymbolKind)symbol.value(MEMBER_KIND).toInt();
        symInfo.range = parseRange(mrange.toObject());
        symInfo.url = QUrl(location.value(MEMBER_URI).toString());
        symInfo.score = symbol.value(QStringLiteral("score")).toDouble();
        symInfo.tags = (LSPSymbolTag)symbol.value(QStringLiteral("tags")).toInt();
        return symInfo;
    });

    std::sort(symbols.begin(), symbols.end(), [](const LSPSymbolInformation &l, const LSPSymbolInformation &r) {
        return l.score > r.score;
    });

    return symbols;
}

using GenericReplyType = QJsonValue;
using GenericReplyHandler = ReplyHandler<GenericReplyType>;

class LSPClientServer::LSPClientServerPrivate
{
    typedef LSPClientServerPrivate self_type;

    LSPClientServer *q;
    // server cmd line
    QStringList m_server;
    // workspace root to pass along
    QUrl m_root;
    // language id
    QString m_langId;
    // user provided init
    QJsonValue m_init;
    // optional workspace folders
    FoldersType m_folders;
    // server process
    QProcess m_sproc;
    // server declared capabilities
    LSPServerCapabilities m_capabilities;
    // server state
    State m_state = State::None;
    // last msg id
    int m_id = 0;
    // receive buffer
    QByteArray m_receive;
    // registered reply handlers
    // (result handler, error result handler)
    QHash<int, std::pair<GenericReplyHandler, GenericReplyHandler>> m_handlers;
    // pending request responses
    static constexpr int MAX_REQUESTS = 5;
    QVector<int> m_requests{MAX_REQUESTS + 1};

public:
    LSPClientServerPrivate(LSPClientServer *_q,
                           const QStringList &server,
                           const QUrl &root,
                           const QString &langId,
                           const QJsonValue &init,
                           const FoldersType &folders)
        : q(_q)
        , m_server(server)
        , m_root(root)
        , m_langId(langId)
        , m_init(init)
        , m_folders(folders)
    {
        // setup async reading
        QObject::connect(&m_sproc, &QProcess::readyRead, utils::mem_fun(&self_type::read, this));
        QObject::connect(&m_sproc, &QProcess::stateChanged, utils::mem_fun(&self_type::onStateChanged, this));
    }

    ~LSPClientServerPrivate()
    {
        stop(TIMEOUT_SHUTDOWN, TIMEOUT_SHUTDOWN);
    }

    const QStringList &cmdline() const
    {
        return m_server;
    }

    const QUrl &root() const
    {
        return m_root;
    }

    const QString &langId() const
    {
        return m_langId;
    }

    State state()
    {
        return m_state;
    }

    const LSPServerCapabilities &capabilities()
    {
        return m_capabilities;
    }

    int cancel(int reqid)
    {
        if (m_handlers.remove(reqid) > 0) {
            auto params = QJsonObject{{MEMBER_ID, reqid}};
            write(init_request(QStringLiteral("$/cancelRequest"), params));
        }
        return -1;
    }

private:
    void setState(State s)
    {
        if (m_state != s) {
            m_state = s;
            Q_EMIT q->stateChanged(q);
        }
    }

    RequestHandle write(const QJsonObject &msg, const GenericReplyHandler &h = nullptr, const GenericReplyHandler &eh = nullptr, const int *id = nullptr)
    {
        RequestHandle ret;
        ret.m_server = q;

        if (!running()) {
            return ret;
        }

        auto ob = msg;
        ob.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
        // notification == no handler
        if (h) {
            ob.insert(MEMBER_ID, ++m_id);
            ret.m_id = m_id;
            m_handlers[m_id] = {h, eh};
        } else if (id) {
            ob.insert(MEMBER_ID, *id);
        }

        QJsonDocument json(ob);
        auto sjson = json.toJson();

        qCInfo(LSPCLIENT) << "calling" << msg[MEMBER_METHOD].toString();
        qCDebug(LSPCLIENT) << "sending message:\n" << QString::fromUtf8(sjson);
        // some simple parsers expect length header first
        auto hdr = QStringLiteral(CONTENT_LENGTH ": %1\r\n").arg(sjson.length());
        // write is async, so no blocking wait occurs here
        m_sproc.write(hdr.toLatin1());
        m_sproc.write("\r\n");
        m_sproc.write(sjson);

        return ret;
    }

    RequestHandle send(const QJsonObject &msg, const GenericReplyHandler &h = nullptr, const GenericReplyHandler &eh = nullptr)
    {
        if (m_state == State::Running) {
            return write(msg, h, eh);
        } else {
            qCWarning(LSPCLIENT) << "send for non-running server";
        }
        return RequestHandle();
    }

    void read()
    {
        // accumulate in buffer
        m_receive.append(m_sproc.readAllStandardOutput());

        // try to get one (or more) message
        QByteArray &buffer = m_receive;

        while (true) {
            qCDebug(LSPCLIENT) << "buffer size" << buffer.length();
            auto header = QByteArray(CONTENT_LENGTH ":");
            int index = buffer.indexOf(header);
            if (index < 0) {
                // avoid collecting junk
                if (buffer.length() > 1 << 20) {
                    buffer.clear();
                }
                break;
            }
            index += header.length();
            int endindex = buffer.indexOf("\r\n", index);
            auto msgstart = buffer.indexOf("\r\n\r\n", index);
            if (endindex < 0 || msgstart < 0) {
                break;
            }
            msgstart += 4;
            bool ok = false;
            auto length = buffer.mid(index, endindex - index).toInt(&ok, 10);
            // FIXME perhaps detect if no reply for some time
            // then again possibly better left to user to restart in such case
            if (!ok) {
                qCWarning(LSPCLIENT) << "invalid " CONTENT_LENGTH;
                // flush and try to carry on to some next header
                buffer.remove(0, msgstart);
                continue;
            }
            // sanity check to avoid extensive buffering
            if (length > 1 << 29) {
                qCWarning(LSPCLIENT) << "excessive size";
                buffer.clear();
                continue;
            }
            if (msgstart + length > buffer.length()) {
                break;
            }
            // now onto payload
            auto payload = buffer.mid(msgstart, length);
            buffer.remove(0, msgstart + length);
            qCInfo(LSPCLIENT) << "got message payload size " << length;
            qCDebug(LSPCLIENT) << "message payload:\n" << payload;
            QJsonParseError error{};
            auto msg = QJsonDocument::fromJson(payload, &error);
            if (error.error != QJsonParseError::NoError || !msg.isObject()) {
                qCWarning(LSPCLIENT) << "invalid response payload";
                continue;
            }
            auto result = msg.object();
            // check if it is the expected result
            int msgid = -1;
            if (result.contains(MEMBER_ID)) {
                // allow id to be returned as a string value, happens e.g. for Perl LSP server
                const auto idValue = result[MEMBER_ID];
                if (idValue.isString()) {
                    msgid = idValue.toString().toInt();
                } else {
                    msgid = idValue.toInt();
                }
            } else {
                processNotification(result);
                continue;
            }
            // could be request
            if (result.contains(MEMBER_METHOD)) {
                processRequest(result);
                continue;
            }

            // a valid reply; what to do with it now
            auto it = m_handlers.find(msgid);
            if (it != m_handlers.end()) {
                // copy handler to local storage
                const auto handler = *it;

                // remove handler from our set, do this pre handler execution to avoid races
                m_handlers.erase(it);

                // run handler, might e.g. trigger some new LSP actions for this server
                // process and provide error if caller interested,
                // otherwise reply will resolve to 'empty' response
                auto &h = handler.first;
                auto &eh = handler.second;
                if (result.contains(MEMBER_ERROR) && eh) {
                    eh(result.value(MEMBER_ERROR));
                } else {
                    h(result.value(MEMBER_RESULT));
                }
            } else {
                // could have been canceled
                qCDebug(LSPCLIENT) << "unexpected reply id" << msgid;
            }
        }
    }

    static QJsonObject init_error(const LSPErrorCode code, const QString &msg)
    {
        return QJsonObject{{MEMBER_ERROR, QJsonObject{{MEMBER_CODE, static_cast<int>(code)}, {MEMBER_MESSAGE, msg}}}};
    }

    static QJsonObject init_request(const QString &method, const QJsonObject &params = QJsonObject())
    {
        return QJsonObject{{MEMBER_METHOD, method}, {MEMBER_PARAMS, params}};
    }

    static QJsonObject init_response(const QJsonValue &result = QJsonValue())
    {
        return QJsonObject{{MEMBER_RESULT, result}};
    }

    bool running()
    {
        return m_sproc.state() == QProcess::Running;
    }

    void onStateChanged(QProcess::ProcessState nstate)
    {
        if (nstate == QProcess::NotRunning) {
            setState(State::None);
        }
    }

    void shutdown()
    {
        if (m_state == State::Running) {
            qCInfo(LSPCLIENT) << "shutting down" << m_server;
            // cancel all pending
            m_handlers.clear();
            // shutdown sequence
            send(init_request(QStringLiteral("shutdown")));
            // maybe we will get/see reply on the above, maybe not
            // but not important or useful either way
            send(init_request(QStringLiteral("exit")));
            // no longer fit for regular use
            setState(State::Shutdown);
        }
    }

    void onInitializeReply(const QJsonValue &value)
    {
        // only parse parts that we use later on
        from_json(m_capabilities, value.toObject().value(QStringLiteral("capabilities")).toObject());
        // finish init
        initialized();
    }

    void initialize()
    {
        // clang-format off
        QJsonObject codeAction{{QStringLiteral("codeActionLiteralSupport"),
                                QJsonObject{{QStringLiteral("codeActionKind"), QJsonObject{{QStringLiteral("valueSet"), QJsonArray()}}}}}};

        QJsonObject semanticTokens{{QStringLiteral("requests"),
                                        QJsonObject{
                                            {QStringLiteral("range"), true},
                                            {QStringLiteral("full"), QJsonObject{{QStringLiteral("delta"), true}}}
                                       }
                                  },
                                  {QStringLiteral("tokenTypes"), supportedSemanticTokenTypes()},
                                  {QStringLiteral("tokenModifiers"), QJsonArray()},
                                  {QStringLiteral("formats"), QJsonArray({QStringLiteral("relative")})},
        };
        QJsonObject capabilities{{QStringLiteral("textDocument"),
                                        QJsonObject{
                                            {QStringLiteral("documentSymbol"), QJsonObject{{QStringLiteral("hierarchicalDocumentSymbolSupport"), true}} },
                                            {QStringLiteral("publishDiagnostics"), QJsonObject{{QStringLiteral("relatedInformation"), true}}},
                                            {QStringLiteral("codeAction"), codeAction},
                                            {QStringLiteral("semanticTokens"), semanticTokens}
                                        }
                                }};
        // only declare workspace support if folders so specified
        if (m_folders) {
            capabilities[QStringLiteral("workspace")] = QJsonObject{{QStringLiteral("workspaceFolders"), true}};
        }
        // NOTE a typical server does not use root all that much,
        // other than for some corner case (in) requests
        QJsonObject params{{QStringLiteral("processId"), QCoreApplication::applicationPid()},
                           {QStringLiteral("rootPath"), m_root.isValid() ? m_root.toLocalFile() : QJsonValue()},
                           {QStringLiteral("rootUri"), m_root.isValid() ? m_root.toString() : QJsonValue()},
                           {QStringLiteral("capabilities"), capabilities},
                           {QStringLiteral("initializationOptions"), m_init}};
        // only add new style workspaces init if so specified
        if (m_folders) {
            params[QStringLiteral("workspaceFolders")] = to_json(*m_folders);
        }
        //
        write(init_request(QStringLiteral("initialize"), params), utils::mem_fun(&self_type::onInitializeReply, this));
        // clang-format on
    }

    void initialized()
    {
        write(init_request(QStringLiteral("initialized")));
        setState(State::Running);
    }

public:
    bool start()
    {
        if (m_state != State::None) {
            return true;
        }

        auto program = m_server.front();
        auto args = m_server;
        args.pop_front();
        qCInfo(LSPCLIENT) << "starting" << m_server << "with root" << m_root;

        // start LSP server in project root
        m_sproc.setWorkingDirectory(m_root.toLocalFile());

        // at least we see some errors somewhere then
        m_sproc.setProcessChannelMode(QProcess::ForwardedErrorChannel);
        m_sproc.setReadChannel(QProcess::QProcess::StandardOutput);
        m_sproc.start(program, args);
        const bool result = m_sproc.waitForStarted();
        if (result) {
            setState(State::Started);
            // perform initial handshake
            initialize();
        }
        return result;
    }

    void stop(int to_term, int to_kill)
    {
        if (running()) {
            shutdown();
            if ((to_term >= 0) && !m_sproc.waitForFinished(to_term)) {
                m_sproc.terminate();
            }
            if ((to_kill >= 0) && !m_sproc.waitForFinished(to_kill)) {
                m_sproc.kill();
            }
        }
    }

    RequestHandle documentSymbols(const QUrl &document, const GenericReplyHandler &h, const GenericReplyHandler &eh)
    {
        auto params = textDocumentParams(document);
        return send(init_request(QStringLiteral("textDocument/documentSymbol"), params), h, eh);
    }

    RequestHandle documentDefinition(const QUrl &document, const LSPPosition &pos, const GenericReplyHandler &h)
    {
        auto params = textDocumentPositionParams(document, pos);
        return send(init_request(QStringLiteral("textDocument/definition"), params), h);
    }

    RequestHandle documentDeclaration(const QUrl &document, const LSPPosition &pos, const GenericReplyHandler &h)
    {
        auto params = textDocumentPositionParams(document, pos);
        return send(init_request(QStringLiteral("textDocument/declaration"), params), h);
    }

    RequestHandle documentTypeDefinition(const QUrl &document, const LSPPosition &pos, const GenericReplyHandler &h)
    {
        auto params = textDocumentPositionParams(document, pos);
        return send(init_request(QStringLiteral("textDocument/typeDefinition"), params), h);
    }

    RequestHandle documentImplementation(const QUrl &document, const LSPPosition &pos, const GenericReplyHandler &h)
    {
        auto params = textDocumentPositionParams(document, pos);
        return send(init_request(QStringLiteral("textDocument/implementation"), params), h);
    }

    RequestHandle documentHover(const QUrl &document, const LSPPosition &pos, const GenericReplyHandler &h)
    {
        auto params = textDocumentPositionParams(document, pos);
        return send(init_request(QStringLiteral("textDocument/hover"), params), h);
    }

    RequestHandle documentHighlight(const QUrl &document, const LSPPosition &pos, const GenericReplyHandler &h)
    {
        auto params = textDocumentPositionParams(document, pos);
        return send(init_request(QStringLiteral("textDocument/documentHighlight"), params), h);
    }

    RequestHandle documentReferences(const QUrl &document, const LSPPosition &pos, bool decl, const GenericReplyHandler &h)
    {
        auto params = referenceParams(document, pos, decl);
        return send(init_request(QStringLiteral("textDocument/references"), params), h);
    }

    RequestHandle documentCompletion(const QUrl &document, const LSPPosition &pos, const GenericReplyHandler &h)
    {
        auto params = textDocumentPositionParams(document, pos);
        return send(init_request(QStringLiteral("textDocument/completion"), params), h);
    }

    RequestHandle signatureHelp(const QUrl &document, const LSPPosition &pos, const GenericReplyHandler &h)
    {
        auto params = textDocumentPositionParams(document, pos);
        return send(init_request(QStringLiteral("textDocument/signatureHelp"), params), h);
    }

    RequestHandle clangdSwitchSourceHeader(const QUrl &document, const GenericReplyHandler &h)
    {
        auto params = QJsonObject{{MEMBER_URI, document.toString()}};
        return send(init_request(QStringLiteral("textDocument/switchSourceHeader"), params), h);
    }

    RequestHandle documentFormatting(const QUrl &document, const LSPFormattingOptions &options, const GenericReplyHandler &h)
    {
        auto params = documentRangeFormattingParams(document, nullptr, options);
        return send(init_request(QStringLiteral("textDocument/formatting"), params), h);
    }

    RequestHandle documentRangeFormatting(const QUrl &document, const LSPRange &range, const LSPFormattingOptions &options, const GenericReplyHandler &h)
    {
        auto params = documentRangeFormattingParams(document, &range, options);
        return send(init_request(QStringLiteral("textDocument/rangeFormatting"), params), h);
    }

    RequestHandle
    documentOnTypeFormatting(const QUrl &document, const LSPPosition &pos, QChar lastChar, const LSPFormattingOptions &options, const GenericReplyHandler &h)
    {
        auto params = documentOnTypeFormattingParams(document, pos, lastChar, options);
        return send(init_request(QStringLiteral("textDocument/onTypeFormatting"), params), h);
    }

    RequestHandle documentRename(const QUrl &document, const LSPPosition &pos, const QString &newName, const GenericReplyHandler &h)
    {
        auto params = renameParams(document, pos, newName);
        return send(init_request(QStringLiteral("textDocument/rename"), params), h);
    }

    RequestHandle
    documentCodeAction(const QUrl &document, const LSPRange &range, const QList<QString> &kinds, QList<LSPDiagnostic> diagnostics, const GenericReplyHandler &h)
    {
        auto params = codeActionParams(document, range, kinds, std::move(diagnostics));
        return send(init_request(QStringLiteral("textDocument/codeAction"), params), h);
    }

    RequestHandle documentSemanticTokensFull(const QUrl &document, bool delta, const QString requestId, const LSPRange &range, const GenericReplyHandler &h)
    {
        auto params = textDocumentParams(document);
        // Delta
        if (delta && !requestId.isEmpty()) {
            params[MEMBER_PREVIOUS_RESULT_ID] = requestId;
            return send(init_request(QStringLiteral("textDocument/semanticTokens/full/delta"), params), h);
        }
        // Range
        if (range.isValid()) {
            params[MEMBER_RANGE] = to_json(range);
            return send(init_request(QStringLiteral("textDocument/semanticTokens/range"), params), h);
        }

        return send(init_request(QStringLiteral("textDocument/semanticTokens/full"), params), h);
    }

    void executeCommand(const QString &command, const QJsonValue &args)
    {
        auto params = executeCommandParams(command, args);
        // Pass an empty lambda as reply handler because executeCommand is a Request, but we ignore the result
        send(init_request(QStringLiteral("workspace/executeCommand"), params), [](const auto &) {});
    }

    void didOpen(const QUrl &document, int version, const QString &langId, const QString &text)
    {
        auto params = textDocumentParams(textDocumentItem(document, langId, text, version));
        send(init_request(QStringLiteral("textDocument/didOpen"), params));
    }

    void didChange(const QUrl &document, int version, const QString &text, const QList<LSPTextDocumentContentChangeEvent> &changes)
    {
        Q_ASSERT(text.isEmpty() || changes.empty());
        auto params = textDocumentParams(document, version);
        params[QStringLiteral("contentChanges")] = text.size() ? QJsonArray{QJsonObject{{MEMBER_TEXT, text}}} : to_json(changes);
        send(init_request(QStringLiteral("textDocument/didChange"), params));
    }

    void didSave(const QUrl &document, const QString &text)
    {
        auto params = textDocumentParams(document);
        params[QStringLiteral("text")] = text;
        send(init_request(QStringLiteral("textDocument/didSave"), params));
    }

    void didClose(const QUrl &document)
    {
        auto params = textDocumentParams(document);
        send(init_request(QStringLiteral("textDocument/didClose"), params));
    }

    void didChangeConfiguration(const QJsonValue &settings)
    {
        auto params = changeConfigurationParams(settings);
        send(init_request(QStringLiteral("workspace/didChangeConfiguration"), params));
    }

    void didChangeWorkspaceFolders(const QList<LSPWorkspaceFolder> &added, const QList<LSPWorkspaceFolder> &removed)
    {
        auto params = changeWorkspaceFoldersParams(added, removed);
        send(init_request(QStringLiteral("workspace/didChangeWorkspaceFolders"), params));
    }

    void workspaceSymbol(const QString &symbol, const GenericReplyHandler &h)
    {
        auto params = QJsonObject{{MEMBER_QUERY, symbol}};
        send(init_request(QStringLiteral("workspace/symbol"), params), h);
    }

    void processNotification(const QJsonObject &msg)
    {
        auto method = msg[MEMBER_METHOD].toString();
        if (method == QLatin1String("textDocument/publishDiagnostics")) {
            Q_EMIT q->publishDiagnostics(parseDiagnostics(msg[MEMBER_PARAMS].toObject()));
        } else if (method == QLatin1String("window/showMessage")) {
            Q_EMIT q->showMessage(parseMessage(msg[MEMBER_PARAMS].toObject()));
        } else if (method == QLatin1String("window/logMessage")) {
            Q_EMIT q->logMessage(parseMessage(msg[MEMBER_PARAMS].toObject()));
        } else {
            qCWarning(LSPCLIENT) << "discarding notification" << method;
        }
    }

    GenericReplyHandler prepareResponse(int msgid)
    {
        // allow limited number of outstanding requests
        auto ctx = QPointer<LSPClientServer>(q);
        m_requests.push_back(msgid);
        if (m_requests.size() > MAX_REQUESTS) {
            m_requests.pop_front();
        }
        auto h = [ctx, this, msgid](const GenericReplyType &response) {
            if (!ctx) {
                return;
            }
            auto index = m_requests.indexOf(msgid);
            if (index >= 0) {
                m_requests.remove(index);
                write(init_response(response), nullptr, nullptr, &msgid);
            } else {
                qCWarning(LSPCLIENT) << "discarding response" << msgid;
            }
        };
        return h;
    }

    template<typename ReplyType>
    static ReplyHandler<ReplyType> responseHandler(const GenericReplyHandler &h,
                                                   typename utils::identity<std::function<GenericReplyType(const ReplyType &)>>::type c)
    {
        return [h, c](const ReplyType &m) {
            h(c(m));
        };
    }

    // pretty rare and limited use, but anyway
    void processRequest(const QJsonObject &msg)
    {
        auto method = msg[MEMBER_METHOD].toString();
        auto msgid = msg[MEMBER_ID].toInt();
        auto params = msg[MEMBER_PARAMS];
        bool handled = false;
        if (method == QLatin1String("workspace/applyEdit")) {
            auto h = responseHandler<LSPApplyWorkspaceEditResponse>(prepareResponse(msgid), applyWorkspaceEditResponse);
            Q_EMIT q->applyEdit(parseApplyWorkspaceEditParams(params.toObject()), h, handled);
        } else if (method == QLatin1String("workspace/workspaceFolders")) {
            // helper to convert from array to value
            auto workspaceFolders = [](auto &&p) {
                return to_json(p);
            };
            auto h = responseHandler<QList<LSPWorkspaceFolder>>(prepareResponse(msgid), workspaceFolders);
            Q_EMIT q->workspaceFolders(h, handled);
        } else {
            write(init_error(LSPErrorCode::MethodNotFound, method), nullptr, nullptr, &msgid);
            qCWarning(LSPCLIENT) << "discarding request" << method;
        }
    }
};

// generic convert handler
// sprinkle some connection-like context safety
// not so likely relevant/needed due to typical sequence of events,
// but in case the latter would be changed in surprising ways ...
template<typename ReplyType>
static GenericReplyHandler
make_handler(const ReplyHandler<ReplyType> &h, const QObject *context, typename utils::identity<std::function<ReplyType(const GenericReplyType &)>>::type c)
{
    // empty provided handler leads to empty handler
    if (!h || !c) {
        return nullptr;
    }

    QPointer<const QObject> ctx(context);
    return [ctx, h, c](const GenericReplyType &m) {
        if (ctx) {
            h(c(m));
        }
    };
}

LSPClientServer::LSPClientServer(const QStringList &server, const QUrl &root, const QString &langId, const QJsonValue &init, const FoldersType &folders)
    : d(new LSPClientServerPrivate(this, server, root, langId, init, folders))
{
}

LSPClientServer::~LSPClientServer()
{
    delete d;
}

const QStringList &LSPClientServer::cmdline() const
{
    return d->cmdline();
}

const QUrl &LSPClientServer::root() const
{
    return d->root();
}

const QString &LSPClientServer::langId() const
{
    return d->langId();
}

LSPClientServer::State LSPClientServer::state() const
{
    return d->state();
}

const LSPServerCapabilities &LSPClientServer::capabilities() const
{
    return d->capabilities();
}

bool LSPClientServer::start()
{
    return d->start();
}

void LSPClientServer::stop(int to_t, int to_k)
{
    return d->stop(to_t, to_k);
}

int LSPClientServer::cancel(int reqid)
{
    return d->cancel(reqid);
}

LSPClientServer::RequestHandle
LSPClientServer::documentSymbols(const QUrl &document, const QObject *context, const DocumentSymbolsReplyHandler &h, const ErrorReplyHandler &eh)
{
    return d->documentSymbols(document, make_handler(h, context, parseDocumentSymbols), make_handler(eh, context, parseResponseError));
}

LSPClientServer::RequestHandle
LSPClientServer::documentDefinition(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentDefinitionReplyHandler &h)
{
    return d->documentDefinition(document, pos, make_handler(h, context, parseDocumentLocation));
}

LSPClientServer::RequestHandle
LSPClientServer::documentImplementation(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentDefinitionReplyHandler &h)
{
    return d->documentImplementation(document, pos, make_handler(h, context, parseDocumentLocation));
}

LSPClientServer::RequestHandle
LSPClientServer::documentDeclaration(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentDefinitionReplyHandler &h)
{
    return d->documentDeclaration(document, pos, make_handler(h, context, parseDocumentLocation));
}

LSPClientServer::RequestHandle
LSPClientServer::documentTypeDefinition(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentDefinitionReplyHandler &h)
{
    return d->documentTypeDefinition(document, pos, make_handler(h, context, parseDocumentLocation));
}

LSPClientServer::RequestHandle
LSPClientServer::documentHover(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentHoverReplyHandler &h)
{
    return d->documentHover(document, pos, make_handler(h, context, parseHover));
}

LSPClientServer::RequestHandle
LSPClientServer::documentHighlight(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentHighlightReplyHandler &h)
{
    return d->documentHighlight(document, pos, make_handler(h, context, parseDocumentHighlightList));
}

LSPClientServer::RequestHandle
LSPClientServer::documentReferences(const QUrl &document, const LSPPosition &pos, bool decl, const QObject *context, const DocumentDefinitionReplyHandler &h)
{
    return d->documentReferences(document, pos, decl, make_handler(h, context, parseDocumentLocation));
}

LSPClientServer::RequestHandle
LSPClientServer::documentCompletion(const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentCompletionReplyHandler &h)
{
    return d->documentCompletion(document, pos, make_handler(h, context, parseDocumentCompletion));
}

LSPClientServer::RequestHandle
LSPClientServer::signatureHelp(const QUrl &document, const LSPPosition &pos, const QObject *context, const SignatureHelpReplyHandler &h)
{
    return d->signatureHelp(document, pos, make_handler(h, context, parseSignatureHelp));
}

LSPClientServer::RequestHandle LSPClientServer::clangdSwitchSourceHeader(const QUrl &document, const QObject *context, const SwitchSourceHeaderHandler &h)
{
    return d->clangdSwitchSourceHeader(document, make_handler(h, context, parseClangdSwitchSourceHeader));
}

LSPClientServer::RequestHandle
LSPClientServer::documentFormatting(const QUrl &document, const LSPFormattingOptions &options, const QObject *context, const FormattingReplyHandler &h)
{
    return d->documentFormatting(document, options, make_handler(h, context, parseTextEdit));
}

LSPClientServer::RequestHandle LSPClientServer::documentRangeFormatting(const QUrl &document,
                                                                        const LSPRange &range,
                                                                        const LSPFormattingOptions &options,
                                                                        const QObject *context,
                                                                        const FormattingReplyHandler &h)
{
    return d->documentRangeFormatting(document, range, options, make_handler(h, context, parseTextEdit));
}

LSPClientServer::RequestHandle LSPClientServer::documentOnTypeFormatting(const QUrl &document,
                                                                         const LSPPosition &pos,
                                                                         const QChar lastChar,
                                                                         const LSPFormattingOptions &options,
                                                                         const QObject *context,
                                                                         const FormattingReplyHandler &h)
{
    return d->documentOnTypeFormatting(document, pos, lastChar, options, make_handler(h, context, parseTextEdit));
}

LSPClientServer::RequestHandle LSPClientServer::documentRename(const QUrl &document,
                                                               const LSPPosition &pos,
                                                               const QString &newName,
                                                               const QObject *context,
                                                               const WorkspaceEditReplyHandler &h)
{
    return d->documentRename(document, pos, newName, make_handler(h, context, parseWorkSpaceEdit));
}

LSPClientServer::RequestHandle LSPClientServer::documentCodeAction(const QUrl &document,
                                                                   const LSPRange &range,
                                                                   const QList<QString> &kinds,
                                                                   QList<LSPDiagnostic> diagnostics,
                                                                   const QObject *context,
                                                                   const CodeActionReplyHandler &h)
{
    return d->documentCodeAction(document, range, kinds, std::move(diagnostics), make_handler(h, context, parseCodeAction));
}

LSPClientServer::RequestHandle
LSPClientServer::documentSemanticTokensFull(const QUrl &document, const QString requestId, const QObject *context, const SemanticTokensDeltaReplyHandler &h)
{
    auto invalidRange = KTextEditor::Range::invalid();
    return d->documentSemanticTokensFull(document, /* delta = */ false, requestId, invalidRange, make_handler(h, context, parseSemanticTokensDelta));
}

LSPClientServer::RequestHandle LSPClientServer::documentSemanticTokensFullDelta(const QUrl &document,
                                                                                const QString requestId,
                                                                                const QObject *context,
                                                                                const SemanticTokensDeltaReplyHandler &h)
{
    auto invalidRange = KTextEditor::Range::invalid();
    return d->documentSemanticTokensFull(document, /* delta = */ true, requestId, invalidRange, make_handler(h, context, parseSemanticTokensDelta));
}

LSPClientServer::RequestHandle
LSPClientServer::documentSemanticTokensRange(const QUrl &document, const LSPRange &range, const QObject *context, const SemanticTokensDeltaReplyHandler &h)
{
    return d->documentSemanticTokensFull(document, /* delta = */ false, QString(), range, make_handler(h, context, parseSemanticTokensDelta));
}

void LSPClientServer::executeCommand(const QString &command, const QJsonValue &args)
{
    return d->executeCommand(command, args);
}

void LSPClientServer::didOpen(const QUrl &document, int version, const QString &langId, const QString &text)
{
    return d->didOpen(document, version, langId, text);
}

void LSPClientServer::didChange(const QUrl &document, int version, const QString &text, const QList<LSPTextDocumentContentChangeEvent> &changes)
{
    return d->didChange(document, version, text, changes);
}

void LSPClientServer::didSave(const QUrl &document, const QString &text)
{
    return d->didSave(document, text);
}

void LSPClientServer::didClose(const QUrl &document)
{
    return d->didClose(document);
}

void LSPClientServer::didChangeConfiguration(const QJsonValue &settings)
{
    return d->didChangeConfiguration(settings);
}

void LSPClientServer::didChangeWorkspaceFolders(const QList<LSPWorkspaceFolder> &added, const QList<LSPWorkspaceFolder> &removed)
{
    return d->didChangeWorkspaceFolders(added, removed);
}

void LSPClientServer::workspaceSymbol(const QString &symbol, const QObject *context, const WorkspaceSymbolsReplyHandler &h)
{
    return d->workspaceSymbol(symbol, make_handler(h, context, parseWorkspaceSymbols));
}
