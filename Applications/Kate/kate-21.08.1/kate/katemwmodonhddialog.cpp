/*
    SPDX-License-Identifier: LGPL-2.0-only

    ---
    SPDX-FileCopyrightText: 2004 Anders Lund <anders@alweb.dk>
*/

#include "katemwmodonhddialog.h"

#include "kateapp.h"
#include "katedocmanager.h"
#include "katemainwindow.h"

#include <KIO/JobUiDelegate>
#include <KIO/OpenUrlJob>

#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>

#include <QDir>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class KateDocItem : public QTreeWidgetItem
{
public:
    KateDocItem(KTextEditor::Document *doc, const QString &status, QTreeWidget *tw)
        : QTreeWidgetItem(tw)
        , document(doc)
    {
        setText(0, doc->url().toString());
        setText(1, status);
        if (!doc->isModified()) {
            setCheckState(0, Qt::Checked);
        } else {
            setCheckState(0, Qt::Unchecked);
        }
    }
    ~KateDocItem() override
    {
    }

    KTextEditor::Document *document;
};

KateMwModOnHdDialog::KateMwModOnHdDialog(DocVector docs, QWidget *parent, const char *name)
    : QDialog(parent)
    , m_proc(nullptr)
    , m_diffFile(nullptr)
    , m_blockAddDocument(false)
{
    setWindowTitle(i18n("Documents Modified on Disk"));
    setObjectName(QString::fromLatin1(name));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // Message
    QHBoxLayout *hb = new QHBoxLayout;
    mainLayout->addLayout(hb);

    // dialog text
    QLabel *icon = new QLabel(this);
    hb->addWidget(icon);
    icon->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-warning")).pixmap(style()->pixelMetric(QStyle::PM_LargeIconSize, nullptr, this)));

    QLabel *t = new QLabel(i18n("<qt>The documents listed below have changed on disk.<p>Select one "
                                "or more at once, and press an action button until the list is empty.</p></qt>"),
                           this);
    hb->addWidget(t);
    hb->setStretchFactor(t, 1000);

    // Document list
    twDocuments = new QTreeWidget(this);
    mainLayout->addWidget(twDocuments);
    QStringList header;
    header << i18n("Filename") << i18n("Status on Disk");
    twDocuments->setHeaderLabels(header);
    twDocuments->setSelectionMode(QAbstractItemView::SingleSelection);
    twDocuments->setRootIsDecorated(false);

    m_stateTexts << QString() << i18n("Modified") << i18n("Created") << i18n("Deleted");
    for (auto &doc : qAsConst(docs)) {
        new KateDocItem(doc, m_stateTexts[static_cast<uint>(KateApp::self()->documentManager()->documentInfo(doc)->modifiedOnDiscReason)], twDocuments);
    }
    twDocuments->header()->setStretchLastSection(false);
    twDocuments->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    twDocuments->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    connect(twDocuments, &QTreeWidget::currentItemChanged, this, &KateMwModOnHdDialog::slotSelectionChanged);
    connect(twDocuments, &QTreeWidget::itemChanged, this, &KateMwModOnHdDialog::slotCheckedFilesChanged);

    // Diff line
    hb = new QHBoxLayout;
    mainLayout->addLayout(hb);

    btnDiff = new QPushButton(QIcon::fromTheme(QStringLiteral("document-preview")), i18n("&View Difference"), this);
    btnDiff->setWhatsThis(
        i18n("Calculates the difference between the editor contents and the disk "
             "file for the selected document, and shows the difference with the "
             "default application. Requires diff(1)."));
    hb->addWidget(btnDiff);
    connect(btnDiff, &QPushButton::clicked, this, &KateMwModOnHdDialog::slotDiff);

    // Dialog buttons
    dlgButtons = new QDialogButtonBox(this);
    mainLayout->addWidget(dlgButtons);

    QPushButton *ignoreButton = new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-warning")), i18n("&Ignore Changes"));
    ignoreButton->setToolTip(i18n("Remove modified flag from selected documents"));
    dlgButtons->addButton(ignoreButton, QDialogButtonBox::RejectRole);
    connect(ignoreButton, &QPushButton::clicked, this, &KateMwModOnHdDialog::slotIgnore);

    QPushButton *overwriteButton = new QPushButton;
    KGuiItem::assign(overwriteButton, KStandardGuiItem::overwrite());
    overwriteButton->setToolTip(i18n("Overwrite selected documents, discarding disk changes"));
    dlgButtons->addButton(overwriteButton, QDialogButtonBox::DestructiveRole);
    connect(overwriteButton, &QPushButton::clicked, this, &KateMwModOnHdDialog::slotOverwrite);

    QPushButton *reloadButton = new QPushButton(QIcon::fromTheme(QStringLiteral("view-refresh")), i18n("&Reload"));
    reloadButton->setDefault(true);
    reloadButton->setToolTip(i18n("Reload selected documents from disk"));
    dlgButtons->addButton(reloadButton, QDialogButtonBox::DestructiveRole);
    connect(reloadButton, &QPushButton::clicked, this, &KateMwModOnHdDialog::slotReload);

    // butons will only be enabled when items are checked. see slotCheckedFilesChanged()
    dlgButtons->setEnabled(false);
    slotCheckedFilesChanged(nullptr, 0);

    slotSelectionChanged(nullptr, nullptr);
}

KateMwModOnHdDialog::~KateMwModOnHdDialog()
{
    KateMainWindow::unsetModifiedOnDiscDialogIfIf(this);

    if (m_proc) {
        m_proc->kill();
        m_proc->waitForFinished();
        delete m_proc;
        m_proc = Q_NULLPTR;
    }

    if (m_diffFile) {
        m_diffFile->setAutoRemove(true);
        delete m_diffFile;
        m_diffFile = Q_NULLPTR;
    }
}

void KateMwModOnHdDialog::slotIgnore()
{
    handleSelected(Ignore);
}

void KateMwModOnHdDialog::slotOverwrite()
{
    handleSelected(Overwrite);
}

void KateMwModOnHdDialog::slotReload()
{
    handleSelected(Reload);
}

void KateMwModOnHdDialog::handleSelected(int action)
{
    // don't alter the treewidget via addDocument, we modify it here!
    m_blockAddDocument = true;

    // collect all items we can remove
    QList<QTreeWidgetItem *> itemsToDelete;
    for (QTreeWidgetItemIterator it(twDocuments); *it; ++it) {
        KateDocItem *item = static_cast<KateDocItem *>(*it);
        if (item->checkState(0) == Qt::Checked) {
            KTextEditor::ModificationInterface::ModifiedOnDiskReason reason =
                KateApp::self()->documentManager()->documentInfo(item->document)->modifiedOnDiscReason;
            bool success = true;

            if (KTextEditor::ModificationInterface *iface = qobject_cast<KTextEditor::ModificationInterface *>(item->document)) {
                iface->setModifiedOnDisk(KTextEditor::ModificationInterface::OnDiskUnmodified);
            }

            switch (action) {
            case Overwrite:
                success = item->document->save();
                if (!success) {
                    KMessageBox::sorry(this, i18n("Could not save the document \n'%1'", item->document->url().toString()));
                }
                break;

            case Reload:
                item->document->documentReload();
                break;

            default:
                break;
            }

            if (success) {
                itemsToDelete.append(item);
            } else {
                if (KTextEditor::ModificationInterface *iface = qobject_cast<KTextEditor::ModificationInterface *>(item->document)) {
                    iface->setModifiedOnDisk(reason);
                }
            }
        }
    }

    // remove the marked items, addDocument is blocked, this is save!
    for (int i = 0; i < itemsToDelete.count(); ++i) {
        delete itemsToDelete[i];
    }

    // any documents left unhandled?
    if (!twDocuments->topLevelItemCount()) {
        accept();
    }

    // update the dialog buttons
    slotCheckedFilesChanged(nullptr, 0);

    // allow addDocument again
    m_blockAddDocument = false;
}

void KateMwModOnHdDialog::slotSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
    KateDocItem *currentDocItem = static_cast<KateDocItem *>(current);
    // set the diff button enabled
    btnDiff->setEnabled(currentDocItem
                        && KateApp::self()->documentManager()->documentInfo(currentDocItem->document)->modifiedOnDiscReason
                            != KTextEditor::ModificationInterface::OnDiskDeleted);
}

void KateMwModOnHdDialog::slotCheckedFilesChanged(QTreeWidgetItem *, int column)
{
    if (column != 0) {
        // we only need to react when the checkbox (in column 0) changes
        return;
    }

    for (QTreeWidgetItemIterator it(twDocuments); *it; ++it) {
        KateDocItem *item = static_cast<KateDocItem *>(*it);
        if (item->checkState(0) == Qt::Checked) {
            // at least 1 item is checked so we enable the buttons
            dlgButtons->setEnabled(true);
            return;
        }
    }

    // no itmes checked, disable all buttons
    dlgButtons->setEnabled(false);
}

// ### the code below is slightly modified from kdelibs/kate/part/katedialogs,
// class KateModOnHdPrompt.
void KateMwModOnHdDialog::slotDiff()
{
    if (!btnDiff->isEnabled()) { // diff button already pressed, proc not finished yet
        return;
    }

    if (!twDocuments->currentItem()) {
        return;
    }

    KTextEditor::Document *doc = (static_cast<KateDocItem *>(twDocuments->currentItem()))->document;

    // don't try to diff a deleted file
    if (KateApp::self()->documentManager()->documentInfo(doc)->modifiedOnDiscReason == KTextEditor::ModificationInterface::OnDiskDeleted) {
        return;
    }

    if (m_diffFile) {
        return;
    }

    m_diffFile = new QTemporaryFile();
    m_diffFile->open();

    // Start a KProcess that creates a diff
    m_proc = new KProcess(this);
    m_proc->setOutputChannelMode(KProcess::MergedChannels);
    *m_proc << QStringLiteral("diff") << QStringLiteral("-ub") << QStringLiteral("-") << doc->url().toLocalFile();
    connect(m_proc, &KProcess::readyRead, this, &KateMwModOnHdDialog::slotDataAvailable);
    connect(m_proc, static_cast<void (KProcess::*)(int, QProcess::ExitStatus)>(&KProcess::finished), this, &KateMwModOnHdDialog::slotPDone);

    setCursor(Qt::WaitCursor);
    btnDiff->setEnabled(false);

    m_proc->start();

    QTextStream ts(m_proc);
    int lastln = doc->lines() - 1;
    for (int l = 0; l < lastln; ++l) {
        ts << doc->line(l) << QLatin1Char('\n');
    }
    ts << doc->line(lastln);
    ts.flush();
    m_proc->closeWriteChannel();
}

void KateMwModOnHdDialog::slotDataAvailable()
{
    m_diffFile->write(m_proc->readAll());
}

void KateMwModOnHdDialog::slotPDone()
{
    setCursor(Qt::ArrowCursor);
    slotSelectionChanged(twDocuments->currentItem(), nullptr);

    const QProcess::ExitStatus es = m_proc->exitStatus();
    delete m_proc;
    m_proc = nullptr;

    if (es != QProcess::NormalExit) {
        KMessageBox::sorry(this,
                           i18n("The diff command failed. Please make sure that "
                                "diff(1) is installed and in your PATH."),
                           i18n("Error Creating Diff"));
        delete m_diffFile;
        m_diffFile = nullptr;
        return;
    }

    if (m_diffFile->size() == 0) {
        KMessageBox::information(this, i18n("Ignoring amount of white space changed, the files are identical."), i18n("Diff Output"));
        delete m_diffFile;
        m_diffFile = nullptr;
        return;
    }

    m_diffFile->setAutoRemove(false);
    QUrl url = QUrl::fromLocalFile(QDir::tempPath() + QLatin1Char('/') + m_diffFile->fileName());
    delete m_diffFile;
    m_diffFile = nullptr;

    Q_EMIT requestOpenDiffDocument(url);
}

void KateMwModOnHdDialog::addDocument(KTextEditor::Document *doc)
{
    // guard this e.g. during handleSelected
    if (m_blockAddDocument) {
        return;
    }

    for (QTreeWidgetItemIterator it(twDocuments); *it; ++it) {
        KateDocItem *item = static_cast<KateDocItem *>(*it);
        if (item->document == doc) {
            delete item;
            break;
        }
    }
    uint reason = static_cast<uint>(KateApp::self()->documentManager()->documentInfo(doc)->modifiedOnDiscReason);
    if (reason) {
        new KateDocItem(doc, m_stateTexts[reason], twDocuments);
    }

    if (!twDocuments->topLevelItemCount()) {
        accept();
    }
}

void KateMwModOnHdDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == 0) {
        if (event->key() == Qt::Key_Escape) {
            event->accept();
            return;
        }
    }
    QDialog::keyPressEvent(event);
}

void KateMwModOnHdDialog::closeEvent(QCloseEvent *e)
{
    if (!twDocuments->topLevelItemCount()) {
        QDialog::closeEvent(e);
    } else {
        e->ignore();
    }
}
