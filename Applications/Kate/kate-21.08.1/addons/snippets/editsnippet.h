/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
 *  SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef EDITSNIPPET_H
#define EDITSNIPPET_H

#include <QDialog>

namespace KTextEditor
{
class View;
}

class SnippetRepository;
class Snippet;

namespace Ui
{
class EditSnippetBase;
}

/**
 * This dialog is used to create/edit snippets in a given repository.
 *
 * @author Milian Wolff <mail@milianw.de>
 */
class EditSnippet : public QDialog
{
    Q_OBJECT

public:
    /// @p snippet set to 0 when you want to create a new snippet.
    explicit EditSnippet(SnippetRepository *repo, Snippet *snippet, QWidget *parent = nullptr);
    ~EditSnippet() override;

    void setSnippetText(const QString &text);

    void reject() override;

private:
    Ui::EditSnippetBase *m_ui;
    SnippetRepository *m_repo;
    Snippet *m_snippet;
    KTextEditor::View *m_snippetView;
    KTextEditor::View *m_scriptsView;
    KTextEditor::View *m_testView;
    bool m_topBoxModified;
    QPushButton *m_okButton;

private Q_SLOTS:
    void test();
    void save();
    void validate();
    void topBoxModified();
};

#endif
