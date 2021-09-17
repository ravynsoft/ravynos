/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>

    SPDX-License-Identifier: MIT
*/

#include "lspclientcompletion.h"
#include "lspclientplugin.h"

#include "lspclient_debug.h"

#include <KTextEditor/Cursor>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <QIcon>
#include <QUrl>

#include <algorithm>
#include <utility>

// clang-format off
#define RETURN_CACHED_ICON(name) \
    { \
        static QIcon icon(QIcon::fromTheme(QStringLiteral(name))); \
        return icon; \
    }
// clang-format on

static QIcon kind_icon(LSPCompletionItemKind kind)
{
    switch (kind) {
    case LSPCompletionItemKind::Method:
    case LSPCompletionItemKind::Function:
    case LSPCompletionItemKind::Constructor:
        RETURN_CACHED_ICON("code-function")
    case LSPCompletionItemKind::Variable:
        RETURN_CACHED_ICON("code-variable")
    case LSPCompletionItemKind::Class:
    case LSPCompletionItemKind::Interface:
    case LSPCompletionItemKind::Struct:
        RETURN_CACHED_ICON("code-class");
    case LSPCompletionItemKind::Module:
        RETURN_CACHED_ICON("code-block");
    case LSPCompletionItemKind::Field:
    case LSPCompletionItemKind::Property:
        // align with symbolview
        RETURN_CACHED_ICON("code-variable");
    case LSPCompletionItemKind::Enum:
    case LSPCompletionItemKind::EnumMember:
        RETURN_CACHED_ICON("enum");
    default:
        break;
    }
    return QIcon();
}

static KTextEditor::CodeCompletionModel::CompletionProperty kind_property(LSPCompletionItemKind kind)
{
    using CompletionProperty = KTextEditor::CodeCompletionModel::CompletionProperty;
    auto p = CompletionProperty::NoProperty;

    switch (kind) {
    case LSPCompletionItemKind::Method:
    case LSPCompletionItemKind::Function:
    case LSPCompletionItemKind::Constructor:
        p = CompletionProperty::Function;
        break;
    case LSPCompletionItemKind::Variable:
        p = CompletionProperty::Variable;
        break;
    case LSPCompletionItemKind::Class:
    case LSPCompletionItemKind::Interface:
        p = CompletionProperty::Class;
        break;
    case LSPCompletionItemKind::Struct:
        p = CompletionProperty::Class;
        break;
    case LSPCompletionItemKind::Module:
        p = CompletionProperty::Namespace;
        break;
    case LSPCompletionItemKind::Enum:
    case LSPCompletionItemKind::EnumMember:
        p = CompletionProperty::Enum;
        break;
    default:
        break;
    }
    return p;
}

struct LSPClientCompletionItem : public LSPCompletionItem {
    int argumentHintDepth = 0;
    QString prefix;
    QString postfix;
    int start = 0;
    int len = 0;

    LSPClientCompletionItem(const LSPCompletionItem &item)
        : LSPCompletionItem(item)
    {
        // transform for later display
        // sigh, remove (leading) whitespace (looking at clangd here)
        // could skip the [] if empty detail, but it is a handy watermark anyway ;-)
        label = QString(label.simplified() + QLatin1String(" [") + detail.simplified() + QStringLiteral("]"));
    }

    LSPClientCompletionItem(const LSPSignatureInformation &sig, int activeParameter, const QString &_sortText)
    {
        argumentHintDepth = 1;
        documentation = sig.documentation;
        label = sig.label;
        sortText = _sortText;

        // transform into prefix, name, suffix if active
        if (activeParameter >= 0 && activeParameter < sig.parameters.length()) {
            const auto &param = sig.parameters.at(activeParameter);
            if (param.start >= 0 && param.start < label.length() && param.end >= 0 && param.end < label.length() && param.start < param.end) {
                start = param.start;
                len = param.end - param.start;
                prefix = label.mid(0, param.start);
                postfix = label.mid(param.end);
                label = label.mid(param.start, param.end - param.start);
            }
        }
    }
};

static bool compare_match(const LSPCompletionItem &a, const LSPCompletionItem &b)
{
    return a.sortText < b.sortText;
}

class LSPClientCompletionImpl : public LSPClientCompletion
{
    Q_OBJECT

    typedef LSPClientCompletionImpl self_type;

    QSharedPointer<LSPClientServerManager> m_manager;
    QSharedPointer<LSPClientServer> m_server;
    bool m_selectedDocumentation = false;
    bool m_signatureHelp = true;
    bool m_complParens = true;

    QVector<QChar> m_triggersCompletion;
    QVector<QChar> m_triggersSignature;
    bool m_triggerSignature = false;

    QList<LSPClientCompletionItem> m_matches;
    LSPClientServer::RequestHandle m_handle, m_handleSig;

public:
    LSPClientCompletionImpl(QSharedPointer<LSPClientServerManager> manager)
        : LSPClientCompletion(nullptr)
        , m_manager(std::move(manager))
        , m_server(nullptr)
    {
    }

    void setServer(QSharedPointer<LSPClientServer> server) override
    {
        m_server = server;
        if (m_server) {
            const auto &caps = m_server->capabilities();
            m_triggersCompletion = caps.completionProvider.triggerCharacters;
            m_triggersSignature = caps.signatureHelpProvider.triggerCharacters;
        } else {
            m_triggersCompletion.clear();
            m_triggersSignature.clear();
        }
    }

    void setSelectedDocumentation(bool s) override
    {
        m_selectedDocumentation = s;
    }

    void setSignatureHelp(bool s) override
    {
        m_signatureHelp = s;
    }

    void setCompleteParens(bool s) override
    {
        m_complParens = s;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid() || index.row() >= m_matches.size()) {
            return QVariant();
        }

        const auto &match = m_matches.at(index.row());

        if (role == Qt::DisplayRole) {
            if (index.column() == KTextEditor::CodeCompletionModel::Name) {
                return match.label;
            } else if (index.column() == KTextEditor::CodeCompletionModel::Prefix) {
                return match.prefix;
            } else if (index.column() == KTextEditor::CodeCompletionModel::Postfix) {
                return match.postfix;
            }
        } else if (role == Qt::DecorationRole && index.column() == KTextEditor::CodeCompletionModel::Icon) {
            return kind_icon(match.kind);
        } else if (role == KTextEditor::CodeCompletionModel::CompletionRole) {
            return kind_property(match.kind);
        } else if (role == KTextEditor::CodeCompletionModel::ArgumentHintDepth) {
            return match.argumentHintDepth;
        } else if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) {
            // (ab)use depth to indicate sort order
            return index.row();
        } else if (role == KTextEditor::CodeCompletionModel::IsExpandable) {
            return !match.documentation.value.isEmpty();
        } else if (role == KTextEditor::CodeCompletionModel::ExpandingWidget && !match.documentation.value.isEmpty()) {
            // probably plaintext, but let's show markdown as-is for now
            // FIXME better presentation of markdown
            return match.documentation.value;
        } else if (role == KTextEditor::CodeCompletionModel::ItemSelected && !match.argumentHintDepth && !match.documentation.value.isEmpty()
                   && m_selectedDocumentation) {
            return match.documentation.value;
        } else if (role == KTextEditor::CodeCompletionModel::CustomHighlight && match.argumentHintDepth > 0) {
            if (index.column() != Name || match.len == 0)
                return {};
            QTextCharFormat boldFormat;
            boldFormat.setFontWeight(QFont::Bold);
            const QList<QVariant> highlighting{
                QVariant(0),
                QVariant(match.len),
                boldFormat,
            };
            return highlighting;
        } else if (role == CodeCompletionModel::HighlightingMethod && match.argumentHintDepth > 0) {
            return QVariant(HighlightMethod::CustomHighlighting);
        }

        return QVariant();
    }

    bool shouldStartCompletion(KTextEditor::View *view, const QString &insertedText, bool userInsertion, const KTextEditor::Cursor &position) override
    {
        qCInfo(LSPCLIENT) << "should start " << userInsertion << insertedText;

        if (!userInsertion || !m_server || insertedText.isEmpty()) {
            return false;
        }

        // covers most already ...
        bool complete = CodeCompletionModelControllerInterface::shouldStartCompletion(view, insertedText, userInsertion, position);
        QChar lastChar = insertedText.at(insertedText.count() - 1);

        m_triggerSignature = false;
        complete = complete || m_triggersCompletion.contains(lastChar);
        if (m_triggersSignature.contains(lastChar)) {
            complete = true;
            m_triggerSignature = true;
        }

        return complete;
    }

    void completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range, InvocationType it) override
    {
        Q_UNUSED(it)

        qCInfo(LSPCLIENT) << "completion invoked" << m_server;

        // maybe use WaitForReset ??
        // but more complex and already looks good anyway
        auto handler = [this](const QList<LSPCompletionItem> & compl ) {
            beginResetModel();
            qCInfo(LSPCLIENT) << "adding completions " << compl .size();
            // purge all existing completion items
            m_matches.erase(std::remove_if(m_matches.begin(),
                                           m_matches.end(),
                                           [](const LSPClientCompletionItem &ci) {
                                               return ci.argumentHintDepth == 0;
                                           }),
                            m_matches.end());
            for (const auto &item : compl ) {
                m_matches.push_back(item);
            }
            std::stable_sort(m_matches.begin(), m_matches.end(), compare_match);
            setRowCount(m_matches.size());
            endResetModel();
        };

        auto sigHandler = [this](const LSPSignatureHelp &sig) {
            beginResetModel();
            qCInfo(LSPCLIENT) << "adding signatures " << sig.signatures.size();
            int index = 0;
            m_matches.erase(std::remove_if(m_matches.begin(),
                                           m_matches.end(),
                                           [](const LSPClientCompletionItem &ci) {
                                               return ci.argumentHintDepth == 1;
                                           }),
                            m_matches.end());
            for (const auto &item : sig.signatures) {
                int sortIndex = 10 + index;
                int active = -1;
                if (index == sig.activeSignature) {
                    sortIndex = 0;
                    active = sig.activeParameter;
                }
                // trick active first, others after that
                m_matches.push_back({item, active, QString(QStringLiteral("%1").arg(sortIndex, 3, 10))});
                ++index;
            }
            std::stable_sort(m_matches.begin(), m_matches.end(), compare_match);
            setRowCount(m_matches.size());
            endResetModel();
        };

        beginResetModel();
        m_matches.clear();
        auto document = view->document();
        if (m_server && document) {
            // the default range is determined based on a reasonable identifier (word)
            // which is generally fine and nice, but let's pass actual cursor position
            // (which may be within this typical range)
            auto position = view->cursorPosition();
            auto cursor = qMax(range.start(), qMin(range.end(), position));
            m_manager->update(document, false);
            if (!m_triggerSignature) {
                m_handle = m_server->documentCompletion(document->url(), {cursor.line(), cursor.column()}, this, handler);
            }
            if (m_signatureHelp) {
                m_handleSig = m_server->signatureHelp(document->url(), {cursor.line(), cursor.column()}, this, sigHandler);
            }
        }
        setRowCount(m_matches.size());
        endResetModel();
    }

    /**
     * @brief return next char *after* the range
     */
    static QChar peekNextChar(KTextEditor::Document *doc, const KTextEditor::Range &range)
    {
        return doc->characterAt(KTextEditor::Cursor(range.end().line(), range.end().column()));
    }

    static bool isFunctionKind(LSPCompletionItemKind k)
    {
        return k == LSPCompletionItemKind::Function || k == LSPCompletionItemKind::Method;
    }

    void executeCompletionItem(KTextEditor::View *view, const KTextEditor::Range &word, const QModelIndex &index) const override
    {
        if (index.row() >= m_matches.size()) {
            return;
        }

        QChar next = peekNextChar(view->document(), word);

        QString matching = m_matches.at(index.row()).insertText;
        // if there is already a '"' or >, remove it, this happens with #include "xx.h"
        if ((next == QLatin1Char('"') && matching.endsWith(QLatin1Char('"'))) || (next == QLatin1Char('>') && matching.endsWith(QLatin1Char('>')))) {
            matching.chop(1);
        }

        const LSPCompletionItemKind kind = m_matches.at(index.row()).kind;
        // Is this a function?
        // add parentheses if function and guestimated meaningful for language in question
        // this covers at least the common cases such as clangd, python, etc
        // also no need to add one if the next char is already
        bool addParens = m_complParens && next != QLatin1Char('(') && isFunctionKind(kind) && m_triggersSignature.contains(QLatin1Char('('));
        if (addParens) {
            matching += QStringLiteral("()");
        }

        view->document()->replaceText(word, matching);

        if (addParens) {
            // place the cursor in between (|)
            view->setCursorPosition({view->cursorPosition().line(), view->cursorPosition().column() - 1});
        }
    }

    void aborted(KTextEditor::View *view) override
    {
        Q_UNUSED(view);
        beginResetModel();
        m_matches.clear();
        m_handle.cancel();
        m_handleSig.cancel();
        m_triggerSignature = false;
        endResetModel();
    }
};

LSPClientCompletion *LSPClientCompletion::new_(QSharedPointer<LSPClientServerManager> manager)
{
    return new LSPClientCompletionImpl(std::move(manager));
}

#include "lspclientcompletion.moc"
