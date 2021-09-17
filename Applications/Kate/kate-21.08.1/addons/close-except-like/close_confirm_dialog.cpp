/**
 * \file
 *
 * \brief Class \c kate::CloseConfirmDialog (implementation)
 *
 * SPDX-FileCopyrightText: 2012 Alex Turbov <i.zaufi@gmail.com>
 *
 * \date Sun Jun 24 16:29:13 MSK 2012 -- Initial design
 */
/*
 * KateCloseExceptPlugin is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * KateCloseExceptPlugin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Project specific includes
#include "close_confirm_dialog.h"

// Standard includes
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString> /// \todo Where is \c i18n() defined?
#include <KSharedConfig>
#include <QBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStyle>

#include <cassert>

namespace kate
{
namespace
{
class KateDocItem : public QTreeWidgetItem
{
public:
    KateDocItem(KTextEditor::Document *doc, QTreeWidget *tw)
        : QTreeWidgetItem(tw)
        , document(doc)
    {
        setText(0, doc->documentName());
        setText(1, doc->url().toString());
        setCheckState(0, Qt::Checked);
    }
    KTextEditor::Document *document;
};
} // anonymous namespace

CloseConfirmDialog::CloseConfirmDialog(QList<KTextEditor::Document *> &docs, KToggleAction *show_confirmation_action, QWidget *const parent)
    : QDialog(parent)
    , m_docs(docs)
{
    assert("Documents container expected to be non empty" && !docs.isEmpty());
    setupUi(this);

    setWindowTitle(i18nc("@title:window", "Close files confirmation"));
    setModal(true);
    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    icon->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-warning")).pixmap(style()->pixelMetric(QStyle::PM_LargeIconSize, nullptr, this)));

    text->setText(i18nc("@label:listbox", "You are about to close the following documents:"));

    QStringList headers;
    headers << i18nc("@title:column", "Document") << i18nc("@title:column", "Location");
    m_docs_tree->setHeaderLabels(headers);
    m_docs_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_docs_tree->setRootIsDecorated(false);

    for (auto &doc : qAsConst(m_docs)) {
        new KateDocItem(doc, m_docs_tree);
    }
    m_docs_tree->header()->setStretchLastSection(false);
    m_docs_tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_docs_tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    m_dont_ask_again->setText(i18nc("option:check", "Do not ask again"));
    // NOTE If we are here, it means that 'Show Confirmation' action is enabled,
    // so not needed to read config...
    assert("Sanity check" && show_confirmation_action->isChecked());
    m_dont_ask_again->setCheckState(Qt::Unchecked);
    connect(m_dont_ask_again, &QCheckBox::toggled, show_confirmation_action, &KToggleAction::toggle);

    // Update documents list according checkboxes
    connect(this, &CloseConfirmDialog::accepted, this, &CloseConfirmDialog::updateDocsList);

    KConfigGroup gcg(KSharedConfig::openConfig(), "kate-close-except-like-CloseConfirmationDialog");
    KWindowConfig::restoreWindowSize(windowHandle(), gcg); // restore dialog geometry from config
}

CloseConfirmDialog::~CloseConfirmDialog()
{
    KConfigGroup gcg(KSharedConfig::openConfig(), "kate-close-except-like-CloseConfirmationDialog");
    KWindowConfig::saveWindowSize(windowHandle(), gcg); // write dialog geometry to config
    gcg.sync();
}

/**
 * Going to remove unchecked files from the given documents list
 */
void CloseConfirmDialog::updateDocsList()
{
    for (QTreeWidgetItemIterator it(m_docs_tree, QTreeWidgetItemIterator::NotChecked); *it; ++it) {
        KateDocItem *item = static_cast<KateDocItem *>(*it);
        m_docs.removeAll(item->document);
        qDebug() << "do not close the file " << item->document->url().toString();
    }
}

} // namespace kate
