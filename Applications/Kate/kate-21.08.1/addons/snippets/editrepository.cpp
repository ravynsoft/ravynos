/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
 *  SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *  SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "editrepository.h"
#include "snippetrepository.h"
#include "snippetstore.h"

#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>

#include <KLocalizedString>
#include <KStandardGuiItem>
#include <KUser>

#include <QDialogButtonBox>
#include <QPushButton>

EditRepository::EditRepository(SnippetRepository *repository, QWidget *parent)
    : QDialog(parent)
    , Ui::EditRepositoryBase()
    , m_repo(repository)
{
    setupUi(this);

    connect(repoNameEdit, &KLineEdit::textEdited, this, &EditRepository::validate);
    connect(this, &QDialog::accepted, this, &EditRepository::save);

    auto ok = buttonBox->button(QDialogButtonBox::Ok);
    KGuiItem::assign(ok, KStandardGuiItem::ok());
    connect(ok, &QPushButton::clicked, this, &EditRepository::accept);

    auto cancel = buttonBox->button(QDialogButtonBox::Cancel);
    KGuiItem::assign(cancel, KStandardGuiItem::cancel());
    connect(cancel, &QPushButton::clicked, this, &EditRepository::reject);

    // fill list of available modes
    QSharedPointer<KTextEditor::Document> document(KTextEditor::Editor::instance()->createDocument(nullptr));
    repoFileTypesList->addItems(document->highlightingModes());
    repoFileTypesList->sortItems();
    repoFileTypesList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(repoFileTypesList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &EditRepository::updateFileTypes);

    // add default licenses
    repoLicenseEdit->addItems(QStringList() << QStringLiteral("BSD") << QStringLiteral("Artistic") << QStringLiteral("LGPL v2+") << QStringLiteral("LGPL v3+"));
    repoLicenseEdit->setEditable(true);

    // if we edit a repo, add all existing data
    if (m_repo) {
        repoNameEdit->setText(m_repo->text());
        repoAuthorsEdit->setText(m_repo->authors());
        repoNamespaceEdit->setText(m_repo->completionNamespace());
        if (!m_repo->license().isEmpty()) {
            int index = repoLicenseEdit->findText(m_repo->license());
            if (index == -1) {
                repoLicenseEdit->addItem(m_repo->license());
                repoLicenseEdit->model()->sort(0);
                index = repoLicenseEdit->findText(m_repo->license());
            }
            repoLicenseEdit->setCurrentIndex(index);
        }
        const auto fileTypes = m_repo->fileTypes();
        for (const QString &type : fileTypes) {
            const auto items = repoFileTypesList->findItems(type, Qt::MatchExactly);
            for (QListWidgetItem *item : items) {
                item->setSelected(true);
            }
        }

        setWindowTitle(i18n("Edit Snippet Repository %1", m_repo->text()));
    } else {
        setWindowTitle(i18n("Create New Snippet Repository"));
        KUser user;
        repoAuthorsEdit->setText(user.property(KUser::FullName).toString());
    }

    validate();
    updateFileTypes();
    repoNameEdit->setFocus();
}

void EditRepository::validate()
{
    bool valid = !repoNameEdit->text().isEmpty() && !repoNameEdit->text().contains(QLatin1Char('/'));
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

void EditRepository::save()
{
    Q_ASSERT(!repoNameEdit->text().isEmpty());
    if (!m_repo) {
        // save as new repo
        m_repo = SnippetRepository::createRepoFromName(repoNameEdit->text());
    }
    m_repo->setText(repoNameEdit->text());
    m_repo->setAuthors(repoAuthorsEdit->text());
    m_repo->setLicense(repoLicenseEdit->currentText());
    m_repo->setCompletionNamespace(repoNamespaceEdit->text());

    QStringList types;
    const auto selectedItems = repoFileTypesList->selectedItems();
    for (QListWidgetItem *item : selectedItems) {
        types << item->text();
    }
    m_repo->setFileTypes(types);
    m_repo->save();

    setWindowTitle(i18n("Edit Snippet Repository %1", m_repo->text()));
}

void EditRepository::updateFileTypes()
{
    QStringList types;
    const auto selectedItems = repoFileTypesList->selectedItems();
    for (QListWidgetItem *item : selectedItems) {
        types << item->text();
    }
    if (types.isEmpty()) {
        repoFileTypesListLabel->setText(i18n("<i>leave empty for general purpose snippets</i>"));
    } else {
        repoFileTypesListLabel->setText(types.join(QLatin1String(", ")));
    }
}
