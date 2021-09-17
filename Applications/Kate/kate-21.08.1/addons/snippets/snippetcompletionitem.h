/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SNIPPETCOMPLETIONITEM_H
#define SNIPPETCOMPLETIONITEM_H

/// TODO: push this into kdevplatform/language/codecompletion so language plugins can reuse it's functionality

#include <QString>
#include <QVariant>

class Snippet;
class SnippetRepository;
class QModelIndex;

namespace KTextEditor
{
class View;
class Range;
class CodeCompletionModel;
}

class SnippetCompletionItem
{
public:
    SnippetCompletionItem(Snippet *snippet, SnippetRepository *repo);
    ~SnippetCompletionItem();

    void execute(KTextEditor::View *view, const KTextEditor::Range &word);
    QVariant data(const QModelIndex &index, int role, const KTextEditor::CodeCompletionModel *model) const;

private:
    // we copy since the snippet itself can be deleted at any time
    QString m_name;
    QString m_snippet;
    SnippetRepository *m_repo;
};

#endif // SNIPPETCOMPLETIONITEM_H
