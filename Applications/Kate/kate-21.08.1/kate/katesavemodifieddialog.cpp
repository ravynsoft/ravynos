/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2004 Joseph Wenninger <jowenn@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "katesavemodifieddialog.h"

#include "katedebug.h"

#include <KGuiItem>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>

#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

class AbstractKateSaveModifiedDialogCheckListItem : public QTreeWidgetItem
{
public:
    AbstractKateSaveModifiedDialogCheckListItem(const QString &title, const QString &url)
    {
        setFlags(flags() | Qt::ItemIsUserCheckable);
        setText(0, title);
        setText(1, url);
        setCheckState(0, Qt::Checked);
        setState(InitialState);
    }
    ~AbstractKateSaveModifiedDialogCheckListItem() override
    {
    }
    virtual bool synchronousSave(QWidget *dialogParent) = 0;
    enum STATE { InitialState, SaveOKState, SaveFailedState };
    STATE state() const
    {
        return m_state;
    }
    void setState(enum STATE state)
    {
        m_state = state;
        switch (state) {
        case InitialState:
            setIcon(0, QIcon());
            break;
        case SaveOKState:
            setIcon(0, QIcon::fromTheme(QStringLiteral("dialog-ok")));
            // QStringLiteral("ok") icon should probably be QStringLiteral("dialog-success"), but we don't have that icon in KDE 4.0
            break;
        case SaveFailedState:
            setIcon(0, QIcon::fromTheme(QStringLiteral("dialog-error")));
            break;
        }
    }

private:
    STATE m_state = InitialState;
};

class KateSaveModifiedDocumentCheckListItem : public AbstractKateSaveModifiedDialogCheckListItem
{
public:
    KateSaveModifiedDocumentCheckListItem(KTextEditor::Document *document)
        : AbstractKateSaveModifiedDialogCheckListItem(document->documentName(), document->url().toString())
    {
        m_document = document;
    }
    ~KateSaveModifiedDocumentCheckListItem() override
    {
    }
    bool synchronousSave(QWidget *dialogParent) override
    {
        if (m_document->url().isEmpty()) {
            const QUrl url = QFileDialog::getSaveFileUrl(dialogParent, i18n("Save As (%1)", m_document->documentName()));
            if (!url.isEmpty()) {
                if (!m_document->saveAs(url)) {
                    setState(SaveFailedState);
                    setText(1, m_document->url().toString());
                    return false;
                } else {
                    bool sc = m_document->waitSaveComplete();
                    setText(1, m_document->url().toString());
                    if (!sc) {
                        setState(SaveFailedState);
                        return false;
                    } else {
                        setState(SaveOKState);
                        return true;
                    }
                }
            } else {
                // setState(SaveFailedState);
                return false;
            }
        } else {
            // document has an existing location
            if (!m_document->save()) {
                setState(SaveFailedState);
                setText(1, m_document->url().toString());
                return false;
            } else {
                bool sc = m_document->waitSaveComplete();
                setText(1, m_document->url().toString());
                if (!sc) {
                    setState(SaveFailedState);
                    return false;
                } else {
                    setState(SaveOKState);
                    return true;
                }
            }
        }

        return false;
    }

private:
    KTextEditor::Document *m_document;
};

KateSaveModifiedDialog::KateSaveModifiedDialog(QWidget *parent, const std::vector<KTextEditor::Document *> &documents)
    : QDialog(parent)
{
    setWindowTitle(i18n("Save Documents"));
    setObjectName(QStringLiteral("KateSaveModifiedDialog"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // label
    QLabel *lbl = new QLabel(i18n("<qt>The following documents have been modified. Do you want to save them before closing?</qt>"), this);
    mainLayout->addWidget(lbl);

    // main view
    m_list = new QTreeWidget(this);
    mainLayout->addWidget(m_list);
    m_list->setColumnCount(2);
    m_list->setHeaderLabels(QStringList() << i18n("Documents") << i18n("Location"));
    m_list->setRootIsDecorated(true);

    for (KTextEditor::Document *doc : documents) {
        m_list->addTopLevelItem(new KateSaveModifiedDocumentCheckListItem(doc));
    }
    m_list->resizeColumnToContents(0);

    connect(m_list, &QTreeWidget::itemChanged, this, &KateSaveModifiedDialog::slotItemActivated);

    QPushButton *selectAllButton = new QPushButton(i18n("Se&lect All"), this);
    mainLayout->addWidget(selectAllButton);
    connect(selectAllButton, &QPushButton::clicked, this, &KateSaveModifiedDialog::slotSelectAll);

    // dialog buttons
    QDialogButtonBox *buttons = new QDialogButtonBox(this);
    mainLayout->addWidget(buttons);

    m_saveButton = new QPushButton;
    KGuiItem::assign(m_saveButton, KStandardGuiItem::save());
    buttons->addButton(m_saveButton, QDialogButtonBox::YesRole);
    connect(m_saveButton, &QPushButton::clicked, this, &KateSaveModifiedDialog::slotSaveSelected);

    QPushButton *discardButton = new QPushButton;
    KGuiItem::assign(discardButton, KStandardGuiItem::discard());
    buttons->addButton(discardButton, QDialogButtonBox::NoRole);
    connect(discardButton, &QPushButton::clicked, this, &KateSaveModifiedDialog::slotDoNotSave);

    QPushButton *cancelButton = new QPushButton;
    KGuiItem::assign(cancelButton, KStandardGuiItem::cancel());
    cancelButton->setDefault(true);
    buttons->addButton(cancelButton, QDialogButtonBox::RejectRole);
    connect(cancelButton, &QPushButton::clicked, this, &KateSaveModifiedDialog::reject);
    cancelButton->setFocus();
}

KateSaveModifiedDialog::~KateSaveModifiedDialog()
{
}

void KateSaveModifiedDialog::slotItemActivated(QTreeWidgetItem *, int)
{
    bool enableSaveButton = false;

    for (int i = 0; i < m_list->topLevelItemCount(); ++i) {
        if (m_list->topLevelItem(i)->checkState(0) == Qt::Checked) {
            enableSaveButton = true;
            break;
        }
    }

    m_saveButton->setEnabled(enableSaveButton);
}

void KateSaveModifiedDialog::slotSelectAll()
{
    for (int i = 0; i < m_list->topLevelItemCount(); ++i) {
        m_list->topLevelItem(i)->setCheckState(0, Qt::Checked);
    }

    m_saveButton->setEnabled(true);
}

void KateSaveModifiedDialog::slotSaveSelected()
{
    if (doSave()) {
        done(QDialog::Accepted);
    }
}

void KateSaveModifiedDialog::slotDoNotSave()
{
    done(QDialog::Accepted);
}

bool KateSaveModifiedDialog::doSave()
{
    for (int i = 0; i < m_list->topLevelItemCount(); ++i) {
        AbstractKateSaveModifiedDialogCheckListItem *cit = static_cast<AbstractKateSaveModifiedDialogCheckListItem *>(m_list->topLevelItem(i));

        if (cit->checkState(0) == Qt::Checked && (cit->state() != AbstractKateSaveModifiedDialogCheckListItem::SaveOKState)) {
            if (!cit->synchronousSave(this /*perhaps that should be the kate mainwindow*/)) {
                if (cit->state() == AbstractKateSaveModifiedDialogCheckListItem::SaveFailedState) {
                    KMessageBox::sorry(this, i18n("Data you requested to be saved could not be written. Please choose how you want to proceed."));
                }
                return false;
            }
        } else if ((cit->checkState(0) != Qt::Checked) && (cit->state() == AbstractKateSaveModifiedDialogCheckListItem::SaveFailedState)) {
            cit->setState(AbstractKateSaveModifiedDialogCheckListItem::InitialState);
        }
    }

    return true;
}

bool KateSaveModifiedDialog::queryClose(QWidget *parent, const std::vector<KTextEditor::Document *> &documents)
{
    KateSaveModifiedDialog d(parent, documents);
    return (d.exec() != QDialog::Rejected);
}
