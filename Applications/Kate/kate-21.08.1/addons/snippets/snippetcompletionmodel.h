/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SNIPPETCOMPLETIONMODEL_H
#define SNIPPETCOMPLETIONMODEL_H

#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>

#include <QPointer>

namespace KTextEditor
{
class View;
}

class SnippetCompletionItem;

class SnippetCompletionModel : public KTextEditor::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)

public:
    SnippetCompletionModel();
    ~SnippetCompletionModel() override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    void completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range, KTextEditor::CodeCompletionModel::InvocationType invocationType) override;
    void executeCompletionItem(KTextEditor::View *view, const KTextEditor::Range &word, const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    KTextEditor::Range completionRange(KTextEditor::View *view, const KTextEditor::Cursor &position) override;
    bool shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::Range &range, const QString &currentCompletion) override;

private:
    void initData(KTextEditor::View *view);
    QList<SnippetCompletionItem *> m_snippets;
};

#endif
