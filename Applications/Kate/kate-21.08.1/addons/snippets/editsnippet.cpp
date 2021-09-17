/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
 *  SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "editsnippet.h"

#include "ui_editsnippet.h"

#include "snippet.h"
#include "snippetrepository.h"
#include "snippetstore.h"

#include <KHelpClient>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageWidget>

#include <QAction>
#include <QPushButton>
#include <QToolButton>
#include <QWhatsThis>

#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

KTextEditor::View *createView(QWidget *tabWidget)
{
    auto document = KTextEditor::Editor::instance()->createDocument(tabWidget);
    auto view = document->createView(tabWidget);

    view->action("file_save")->setEnabled(false);
    tabWidget->layout()->addWidget(view);
    view->setStatusBarEnabled(false);
    return view;
}

EditSnippet::EditSnippet(SnippetRepository *repository, Snippet *snippet, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::EditSnippetBase)
    , m_repo(repository)
    , m_snippet(snippet)
    , m_topBoxModified(false)
{
    Q_ASSERT(m_repo);
    m_ui->setupUi(this);

    connect(this, &QDialog::accepted, this, &EditSnippet::save);

    m_okButton = m_ui->buttons->button(QDialogButtonBox::Ok);
    KGuiItem::assign(m_okButton, KStandardGuiItem::ok());
    m_ui->buttons->addButton(m_okButton, QDialogButtonBox::AcceptRole);
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);

    auto cancelButton = m_ui->buttons->button(QDialogButtonBox::Cancel);
    KGuiItem::assign(cancelButton, KStandardGuiItem::cancel());
    m_ui->buttons->addButton(cancelButton, QDialogButtonBox::RejectRole);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    m_snippetView = createView(m_ui->snippetTab);
    if (!m_repo->fileTypes().isEmpty()) {
        m_snippetView->document()->setMode(m_repo->fileTypes().first());
    }

    m_scriptsView = createView(m_ui->scriptTab);
    m_scriptsView->document()->setMode(QStringLiteral("JavaScript"));
    m_scriptsView->document()->setText(m_repo->script());
    m_scriptsView->document()->setModified(false);

    // view for testing the snippet
    m_testView = createView(m_ui->testWidget);
    // splitter default size ratio
    m_ui->splitter->setSizes(QList<int>() << 400 << 150);
    connect(m_ui->dotest_button, &QPushButton::clicked, this, &EditSnippet::test);

    // modified notification stuff
    connect(m_ui->snippetNameEdit, &QLineEdit::textEdited, this, &EditSnippet::topBoxModified);
    connect(m_ui->snippetNameEdit, &QLineEdit::textEdited, this, &EditSnippet::validate);
    connect(m_ui->snippetShortcut, &KKeySequenceWidget::keySequenceChanged, this, &EditSnippet::topBoxModified);
    connect(m_snippetView->document(), &KTextEditor::Document::textChanged, this, &EditSnippet::validate);

    auto showHelp = [](const QString &text) {
        QWhatsThis::showText(QCursor::pos(), text);
    };
    connect(m_ui->snippetLabel, &QLabel::linkActivated, showHelp);
    connect(m_ui->scriptLabel, &QLabel::linkActivated, showHelp);

    // if we edit a snippet, add all existing data
    if (m_snippet) {
        setWindowTitle(i18n("Edit Snippet %1 in %2", m_snippet->text(), m_repo->text()));

        m_snippetView->document()->setText(m_snippet->snippet());
        m_ui->snippetNameEdit->setText(m_snippet->text());
        m_ui->snippetShortcut->setKeySequence(m_snippet->action()->shortcut());

        // unset modified flags
        m_snippetView->document()->setModified(false);
        m_topBoxModified = false;
    } else {
        setWindowTitle(i18n("Create New Snippet in Repository %1", m_repo->text()));
    }

    m_ui->messageWidget->hide();
    validate();

    m_ui->snippetNameEdit->setFocus();
    setTabOrder(m_ui->snippetNameEdit, m_snippetView);

    QSize initSize = sizeHint();
    initSize.setHeight(initSize.height() + 200);
}

void EditSnippet::test()
{
    m_testView->document()->clear();
    m_testView->insertTemplate(KTextEditor::Cursor(0, 0), m_snippetView->document()->text(), m_scriptsView->document()->text());
    m_testView->setFocus();
}

EditSnippet::~EditSnippet()
{
    delete m_ui;
}

void EditSnippet::setSnippetText(const QString &text)
{
    m_snippetView->document()->setText(text);
    validate();
}

void EditSnippet::validate()
{
    const QString &name = m_ui->snippetNameEdit->text();
    bool valid = !name.isEmpty() && !m_snippetView->document()->isEmpty();
    // make sure the snippetname includes no spaces
    if (name.contains(QLatin1Char(' ')) || name.contains(QLatin1Char('\t'))) {
        m_ui->messageWidget->setText(i18n("Snippet name cannot contain spaces"));
        m_ui->messageWidget->animatedShow();
        valid = false;
    } else {
        // hide message widget if snippet does not include spaces
        m_ui->messageWidget->animatedHide();
    }
    if (valid) {
        m_ui->messageWidget->hide();
    }
    m_okButton->setEnabled(valid);
}

void EditSnippet::save()
{
    Q_ASSERT(!m_ui->snippetNameEdit->text().isEmpty());

    if (!m_snippet) {
        // save as new snippet
        m_snippet = new Snippet();
        m_snippet->action(); // ensure that the snippet's QAction is created before it is added to a widget by the rowsInserted() signal
        m_repo->appendRow(m_snippet);
    }
    m_snippet->setSnippet(m_snippetView->document()->text());
    m_snippetView->document()->setModified(false);
    m_snippet->setText(m_ui->snippetNameEdit->text());
    m_snippet->action()->setShortcut(m_ui->snippetShortcut->keySequence());
    m_repo->setScript(m_scriptsView->document()->text());
    m_scriptsView->document()->setModified(false);
    m_topBoxModified = false;
    m_repo->save();

    setWindowTitle(i18n("Edit Snippet %1 in %2", m_snippet->text(), m_repo->text()));
}

void EditSnippet::reject()
{
    if (m_topBoxModified || m_snippetView->document()->isModified() || m_scriptsView->document()->isModified()) {
        int ret = KMessageBox::warningContinueCancel(qApp->activeWindow(),
                                                     i18n("The snippet contains unsaved changes. Do you want to continue and lose all changes?"),
                                                     i18n("Warning - Unsaved Changes"));
        if (ret == KMessageBox::Cancel) {
            return;
        }
    }
    QDialog::reject();
}

void EditSnippet::topBoxModified()
{
    m_topBoxModified = true;
}
