/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
 *  SPDX-FileCopyrightText: 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_PROJECT_COMPLETION_H
#define KATE_PROJECT_COMPLETION_H

#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>
#include <ktexteditor/view.h>

#include <QStandardItemModel>

/**
 * Project wide completion support.
 */
class KateProjectCompletion : public KTextEditor::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface
{
    Q_OBJECT

    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)

public:
    /**
     * Construct project completion.
     * @param plugin our plugin
     */
    KateProjectCompletion(class KateProjectPlugin *plugin);

    /**
     * Deconstruct project completion.
     */
    ~KateProjectCompletion() override;

    /**
     * This function is responsible to generating / updating the list of current
     * completions. The default implementation does nothing.
     *
     * When implementing this function, remember to call setRowCount() (or implement
     * rowCount()), and to generate the appropriate change notifications (for instance
     * by calling QAbstractItemModel::reset()).
     * @param view The view to generate completions for
     * @param range The range of text to generate completions for
     * */
    void completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range, InvocationType invocationType) override;

    bool shouldStartCompletion(KTextEditor::View *view, const QString &insertedText, bool userInsertion, const KTextEditor::Cursor &position) override;
    bool shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::Range &range, const QString &currentCompletion) override;

    void saveMatches(KTextEditor::View *view, const KTextEditor::Range &range);

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    MatchReaction matchingItem(const QModelIndex &matched) override;

    KTextEditor::Range completionRange(KTextEditor::View *view, const KTextEditor::Cursor &position) override;

    void allMatches(QStandardItemModel &model, KTextEditor::View *view, const KTextEditor::Range &range) const;

private:
    /**
     * our plugin view
     */
    KateProjectPlugin *m_plugin;

    /**
     * model with matching data
     */
    QStandardItemModel m_matches;

    /**
     * automatic invocation?
     */
    bool m_automatic = false;
};

#endif
