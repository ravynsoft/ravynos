/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *  SPDX-FileCopyrightText: 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectcompletion.h"
#include "kateproject.h"
#include "kateprojectplugin.h"

#include <KLocalizedString>

#include <QIcon>

KateProjectCompletion::KateProjectCompletion(KateProjectPlugin *plugin)
    : KTextEditor::CodeCompletionModel(nullptr)
    , m_plugin(plugin)
{
}

KateProjectCompletion::~KateProjectCompletion()
{
}

void KateProjectCompletion::saveMatches(KTextEditor::View *view, const KTextEditor::Range &range)
{
    m_matches.clear();
    allMatches(m_matches, view, range);
}

QVariant KateProjectCompletion::data(const QModelIndex &index, int role) const
{
    if (role == InheritanceDepth) {
        return 10010; // Very high value, so the word-completion group and items are shown behind any other groups/items if there is multiple
    }

    if (!index.parent().isValid()) {
        // It is the group header
        switch (role) {
        case Qt::DisplayRole:
            return i18n("Project Completion");
        case GroupRole:
            return Qt::DisplayRole;
        }
    }

    if (index.column() == KTextEditor::CodeCompletionModel::Name && role == Qt::DisplayRole) {
        return m_matches.item(index.row())->data(Qt::DisplayRole);
    }

    if (index.column() == KTextEditor::CodeCompletionModel::Icon && role == Qt::DecorationRole) {
        static QIcon icon(QIcon::fromTheme(QStringLiteral("insert-text")).pixmap(QSize(16, 16)));
        return icon;
    }

    return QVariant();
}

QModelIndex KateProjectCompletion::parent(const QModelIndex &index) const
{
    if (index.internalId()) {
        return createIndex(0, 0, quintptr(0));
    } else {
        return QModelIndex();
    }
}

QModelIndex KateProjectCompletion::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (row == 0) {
            return createIndex(row, column, quintptr(0));
        } else {
            return QModelIndex();
        }

    } else if (parent.parent().isValid()) {
        return QModelIndex();
    }

    if (row < 0 || row >= m_matches.rowCount() || column < 0 || column >= ColumnCount) {
        return QModelIndex();
    }

    return createIndex(row, column, 1);
}

int KateProjectCompletion::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid() && !(m_matches.rowCount() == 0)) {
        return 1; // One root node to define the custom group
    } else if (parent.parent().isValid()) {
        return 0; // Completion-items have no children
    } else {
        return m_matches.rowCount();
    }
}

bool KateProjectCompletion::shouldStartCompletion(KTextEditor::View *view, const QString &insertedText, bool userInsertion, const KTextEditor::Cursor &position)
{
    if (!userInsertion) {
        return false;
    }
    if (insertedText.isEmpty()) {
        return false;
    }

    QString text = view->document()->line(position.line()).left(position.column());

    uint check = 3; // v->config()->wordCompletionMinimalWordLength();

    if (check <= 0) {
        return true;
    }
    int start = text.length();
    int end = text.length() - check;
    if (end < 0) {
        return false;
    }
    for (int i = start - 1; i >= end; i--) {
        QChar c = text.at(i);
        if (!(c.isLetter() || (c.isNumber()) || c == QLatin1Char('_'))) {
            return false;
        }
    }

    return true;
}

bool KateProjectCompletion::shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::Range &range, const QString &currentCompletion)
{
    if (m_automatic) {
        if (currentCompletion.length() < 3 /*v->config()->wordCompletionMinimalWordLength()*/) {
            return true;
        }
    }

    return CodeCompletionModelControllerInterface::shouldAbortCompletion(view, range, currentCompletion);
}

void KateProjectCompletion::completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range, InvocationType it)
{
    /**
     * auto invoke...
     */
    m_automatic = false;
    if (it == AutomaticInvocation) {
        m_automatic = true;

        if (range.columnWidth() >= 3 /*v->config()->wordCompletionMinimalWordLength()*/) {
            saveMatches(view, range);
        } else {
            m_matches.clear();
        }

        // done here...
        return;
    }

    // normal case ;)
    saveMatches(view, range);
}

// Scan throughout the entire document for possible completions,
// ignoring any dublets
void KateProjectCompletion::allMatches(QStandardItemModel &model, KTextEditor::View *view, const KTextEditor::Range &range) const
{
    /**
     * get project scope for this document, else fail
     */
    QList<KateProject *> projects;
    if (m_plugin->multiProjectCompletion()) {
        projects = m_plugin->projects();
    } else {
        auto project = m_plugin->projectForDocument(view->document());
        if (project) {
            projects.push_back(project);
        }
    }

    /**
     * let project index fill the completion for this document
     */
    for (const auto &project : projects) {
        if (project->projectIndex()) {
            project->projectIndex()->findMatches(model, view->document()->text(range), KateProjectIndex::CompletionMatches);
        }
    }
}

KTextEditor::CodeCompletionModelControllerInterface::MatchReaction KateProjectCompletion::matchingItem(const QModelIndex & /*matched*/)
{
    return HideListIfAutomaticInvocation;
}

// Return the range containing the word left of the cursor
KTextEditor::Range KateProjectCompletion::completionRange(KTextEditor::View *view, const KTextEditor::Cursor &position)
{
    int line = position.line();
    int col = position.column();

    KTextEditor::Document *doc = view->document();
    while (col > 0) {
        QChar c = (doc->characterAt(KTextEditor::Cursor(line, col - 1)));
        if (c.isLetterOrNumber() || c.isMark() || c == QLatin1Char('_')) {
            col--;
            continue;
        }

        break;
    }

    return KTextEditor::Range(KTextEditor::Cursor(line, col), position);
}
