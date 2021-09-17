/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KATE_SNIPPET_GLOBAL_H__
#define __KATE_SNIPPET_GLOBAL_H__

#include <QPointer>
#include <QVariant>

#include <KTextEditor/View>

class SnippetCompletionModel;
class Snippet;

/**
 * This is the main class of KDevelop's snippet plugin.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class KateSnippetGlobal : public QObject
{
    Q_OBJECT

public:
    KateSnippetGlobal(QObject *parent, const QVariantList &args = QVariantList());
    ~KateSnippetGlobal() override;

    /**
     * Inserts the given @p snippet into the currently active view.
     * If the current active view is not inherited from KTextEditor::View
     * nothing will happen.
     */
    void insertSnippet(Snippet *snippet);

    static KateSnippetGlobal *self()
    {
        return s_self;
    }

    /**
     * Code completion model.
     * @return code completion model for snippets
     */
    SnippetCompletionModel *completionModel()
    {
        return m_model.data();
    }

public Q_SLOTS:
    /**
     * Create snippet for given view, e.g. by using the selection
     * @param view view to create snippet for
     */
    void createSnippet(KTextEditor::View *view);

    void insertSnippetFromActionData();

private:
    static KateSnippetGlobal *s_self;
    QScopedPointer<SnippetCompletionModel> m_model;
    QPointer<KTextEditor::View> m_activeViewForDialog;
};

#endif
