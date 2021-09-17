/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "lspsemantichighlighting.h"
#include "lspclientprotocol.h"
#include "lspclientservermanager.h"
#include "semantic_tokens_legend.h"

#include <KTextEditor/MovingInterface>
#include <KTextEditor/MovingRange>
#include <KTextEditor/View>

SemanticHighlighter::SemanticHighlighter(QSharedPointer<LSPClientServerManager> serverManager, QObject *parent)
    : QObject(parent)
    , m_serverManager(std::move(serverManager))
{
    m_requestTimer.setInterval(2000);
    m_requestTimer.setSingleShot(true);
    m_requestTimer.connect(&m_requestTimer, &QTimer::timeout, this, [this]() {
        doSemanticHighlighting_impl(m_currentView);
    });
}

static KTextEditor::Range getCurrentViewLinesRange(KTextEditor::View *view)
{
    Q_ASSERT(view);

    auto doc = view->document();
    auto first = view->firstDisplayedLine();
    auto last = view->lastDisplayedLine();
    auto lastLineLen = doc->line(last).size();
    return KTextEditor::Range(first, 0, last, lastLineLen);
}

void SemanticHighlighter::doSemanticHighlighting(KTextEditor::View *view)
{
    // start the timer
    // We dont send the request directly because then there can be too many requests
    // which leads to too much load on the server and client/server getting out of sync.
    //
    // This strategy can be problematic if the user keeps typing, but in reality that is
    // unlikely to happen I think
    m_currentView = view;
    m_requestTimer.start();
}

void SemanticHighlighter::doSemanticHighlighting_impl(KTextEditor::View *view)
{
    if (!view) {
        return;
    }

    auto server = m_serverManager->findServer(view);
    if (!server) {
        return;
    }

    const auto &caps = server->capabilities();
    const bool serverSupportsSemHighlighting = caps.semanticTokenProvider.full || caps.semanticTokenProvider.fullDelta || caps.semanticTokenProvider.range;
    if (!serverSupportsSemHighlighting) {
        return;
    }

    auto doc = view->document();
    if (m_docResultId.count(doc) == 0) {
        connect(doc,
                SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document *)),
                this,
                SLOT(remove(KTextEditor::Document *)),
                Qt::UniqueConnection);
        connect(doc, SIGNAL(aboutToDeleteMovingInterfaceContent(KTextEditor::Document *)), this, SLOT(remove(KTextEditor::Document *)), Qt::UniqueConnection);
    }

    if (caps.semanticTokenProvider.range) {
        connect(view, &KTextEditor::View::verticalScrollPositionChanged, this, &SemanticHighlighter::semanticHighlightRange, Qt::UniqueConnection);
    }

    //  m_semHighlightingManager.setTypes(server->capabilities().semanticTokenProvider.types);

    QPointer<KTextEditor::View> v = view;
    auto h = [this, v, server](const LSPSemanticTokensDelta &st) {
        if (v && server) {
            const auto legend = &server->capabilities().semanticTokenProvider.legend;
            processTokens(st, v, legend);
        }
    };

    if (caps.semanticTokenProvider.range) {
        server->documentSemanticTokensRange(doc->url(), getCurrentViewLinesRange(view), this, h);
    } else if (caps.semanticTokenProvider.fullDelta) {
        auto prevResultId = previousResultIdForDoc(doc);
        server->documentSemanticTokensFullDelta(doc->url(), prevResultId, this, h);
    } else {
        server->documentSemanticTokensFull(doc->url(), QString(), this, h);
    }
}

void SemanticHighlighter::semanticHighlightRange(KTextEditor::View *view, const KTextEditor::Cursor &)
{
    doSemanticHighlighting(view);
}

QString SemanticHighlighter::previousResultIdForDoc(KTextEditor::Document *doc) const
{
    auto it = m_docResultId.find(doc);
    if (it != m_docResultId.end()) {
        return it->second;
    }
    return QString();
}

void SemanticHighlighter::processTokens(const LSPSemanticTokensDelta &tokens, KTextEditor::View *view, const SemanticTokensLegend *legend)
{
    Q_ASSERT(view);

    for (const auto &semTokenEdit : tokens.edits) {
        update(view->document(), tokens.resultId, semTokenEdit.start, semTokenEdit.deleteCount, semTokenEdit.data);
    }

    if (!tokens.data.empty()) {
        insert(view->document(), tokens.resultId, tokens.data);
    }
    highlight(view, legend);
}

void SemanticHighlighter::remove(KTextEditor::Document *doc)
{
    m_docResultId.erase(doc);
    m_docSemanticInfo.erase(doc);
}

void SemanticHighlighter::insert(KTextEditor::Document *doc, const QString &resultId, const std::vector<uint32_t> &data)
{
    m_docResultId[doc] = resultId;
    TokensData &tokensData = m_docSemanticInfo[doc];
    tokensData.tokens = data;
}

/**
 * Handle semantic tokens edits
 */
void SemanticHighlighter::update(KTextEditor::Document *doc, const QString &resultId, uint32_t start, uint32_t deleteCount, const std::vector<uint32_t> &data)
{
    auto toks = m_docSemanticInfo.find(doc);
    if (toks == m_docSemanticInfo.end()) {
        return;
    }

    auto &existingTokens = toks->second.tokens;

    // replace
    if (deleteCount > 0) {
        existingTokens.erase(existingTokens.begin() + start, existingTokens.begin() + start + deleteCount);
    }
    existingTokens.insert(existingTokens.begin() + start, data.begin(), data.end());

    //     Update result Id
    m_docResultId[doc] = resultId;
}

void SemanticHighlighter::highlight(KTextEditor::View *view, const SemanticTokensLegend *legend)
{
    Q_ASSERT(legend);

    if (!view || !legend) {
        return;
    }
    auto doc = view->document();
    auto miface = qobject_cast<KTextEditor::MovingInterface *>(doc);

    TokensData &semanticData = m_docSemanticInfo[doc];
    auto &movingRanges = semanticData.movingRanges;
    auto &data = semanticData.tokens;

    if (data.size() % 5 != 0) {
        qWarning() << "Bad data for doc: " << doc->url() << " skipping";
        return;
    }

    uint32_t currentLine = 0;
    uint32_t start = 0;

    size_t reusedRanges = 0;
    size_t newRanges = 0;
    size_t existingMovingRangesCount = movingRanges.size();

    for (size_t i = 0; i < data.size(); i += 5) {
        auto deltaLine = data.at(i);
        auto deltaStart = data.at(i + 1);
        auto len = data.at(i + 2);
        auto type = data.at(i + 3);
        auto mod = data.at(i + 4);
        (void)mod;

        currentLine += deltaLine;

        if (deltaLine == 0) {
            start += deltaStart;
        } else {
            start = deltaStart;
        }

        // QString text = doc->line(currentLine);
        // text = text.mid(start, len);

        auto attribute = legend->attributeForTokenType(type);
        if (!attribute) {
            continue;
        }

        KTextEditor::Range r(currentLine, start, currentLine, start + len);

        // Check if we have a moving ranges already available in the cache
        if (reusedRanges < existingMovingRangesCount) {
            auto &range = movingRanges[reusedRanges];
            if (!range) {
                range.reset(miface->newMovingRange(r));
            }
            reusedRanges++;
            // clear attribute first so that we block some of the notifyAboutRangeChange stuff!
            range->setAttribute(KTextEditor::Attribute::Ptr(nullptr));
            range->setZDepth(-90000.0);
            range->setRange(r);
            range->setAttribute(attribute);
            continue;
        }

        std::unique_ptr<KTextEditor::MovingRange> mr(miface->newMovingRange(r));
        mr->setZDepth(-90000.0);
        mr->setAttribute(attribute);
        movingRanges.push_back(std::move(mr));
        newRanges++;

        // std::cout << "Token: " << text.toStdString() << " => " << m_types.at(type).toStdString() << ", Line: {" << currentLine << ", " << deltaLine
        //         << "}\n";
    }

    /**
     * Invalid all extra ranges
     */
    const auto totalUtilizedRanges = reusedRanges + newRanges;
    if (totalUtilizedRanges < movingRanges.size()) {
        std::for_each(movingRanges.begin() + totalUtilizedRanges, movingRanges.end(), [](const std::unique_ptr<KTextEditor::MovingRange> &mr) {
            mr->setRange(KTextEditor::Range::invalid());
        });
    }
}
