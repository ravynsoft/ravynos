/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef LSP_SEMANTIC_HIGHLIGHTING_H
#define LSP_SEMANTIC_HIGHLIGHTING_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>

#include <KTextEditor/MovingRange>

#include <memory>
#include <unordered_map>
#include <vector>

namespace KTextEditor
{
class View;
class Document;
}

class SemanticTokensLegend;
class LSPClientServerManager;
struct LSPSemanticTokensDelta;

class SemanticHighlighter : public QObject
{
    Q_OBJECT
public:
    SemanticHighlighter(QSharedPointer<LSPClientServerManager> serverManager, QObject *parent = nullptr);

    void doSemanticHighlighting(KTextEditor::View *v);

private:
    void doSemanticHighlighting_impl(KTextEditor::View *v);

    void semanticHighlightRange(KTextEditor::View *view, const KTextEditor::Cursor &);

    QString previousResultIdForDoc(KTextEditor::Document *doc) const;

    /**
     * Unregister a doc from highlighter and remove all its associated moving ranges and tokens
     */
    Q_SLOT void remove(KTextEditor::Document *doc);

    void processTokens(const LSPSemanticTokensDelta &tokens, KTextEditor::View *view, const SemanticTokensLegend *legend);

    /**
     * Does the actual highlighting
     */
    void highlight(KTextEditor::View *view, const SemanticTokensLegend *legend);

    /**
     * Insert new incoming tokens @p data for doc with @p url
     */
    void insert(KTextEditor::Document *doc, const QString &resultId, const std::vector<uint32_t> &data);

    /**
     * Handle SemanticTokensEdits
     */
    void update(KTextEditor::Document *doc, const QString &resultId, uint32_t start, uint32_t deleteCount, const std::vector<uint32_t> &data);

    /**
     * A simple struct which holds the tokens recieved by server +
     * moving ranges that were created to highlight those tokens
     */
    struct TokensData {
        std::vector<uint32_t> tokens;
        std::vector<std::unique_ptr<KTextEditor::MovingRange>> movingRanges;
    };

    /**
     * token types specified in server caps. Uncomment for debugging
     */
    //     QVector<QString> m_types;

    /**
     * Doc => result-id mapping
     */
    std::unordered_map<KTextEditor::Document *, QString> m_docResultId;

    /**
     * semantic token and moving range mapping for doc
     */
    std::unordered_map<KTextEditor::Document *, TokensData> m_docSemanticInfo;

    QSharedPointer<LSPClientServerManager> m_serverManager;

    QTimer m_requestTimer;
    QPointer<KTextEditor::View> m_currentView;
};
#endif
