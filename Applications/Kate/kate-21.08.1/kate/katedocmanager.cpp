/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2007 Flavio Castelli <flavio.castelli@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "katedocmanager.h"

#include "kateapp.h"
#include "katedebug.h"
#include "katemainwindow.h"
#include "katesavemodifieddialog.h"
#include "kateviewmanager.h"

#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>

#include <KColorScheme>
#include <KConfigGroup>
#include <KIO/DeleteJob>
#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QFileDialog>
#include <QProgressDialog>
#include <QTextCodec>
#include <QTimer>

KateDocManager::KateDocManager(QObject *parent)
    : QObject(parent)
    , m_metaInfos(QStringLiteral("katemetainfos"), KConfig::NoGlobals)
    , m_saveMetaInfos(true)
    , m_daysMetaInfos(0)
{
    // set our application wrapper
    KTextEditor::Editor::instance()->setApplication(KateApp::self()->wrapper());

    // create one doc, we always have at least one around!
    createDoc();
}

KateDocManager::~KateDocManager()
{
    // write metainfos?
    if (m_saveMetaInfos) {
        // saving meta-infos when file is saved is not enough, we need to do it once more at the end
        saveMetaInfos(m_docList);

        // purge saved filesessions
        if (m_daysMetaInfos > 0) {
            const QStringList groups = m_metaInfos.groupList();
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            QDateTime def(QDate(1970, 1, 1));
#else
            QDateTime def(QDate(1970, 1, 1).startOfDay());
#endif
            for (const auto &group : groups) {
                QDateTime last = m_metaInfos.group(group).readEntry("Time", def);
                if (last.daysTo(QDateTime::currentDateTimeUtc()) > m_daysMetaInfos) {
                    m_metaInfos.deleteGroup(group);
                }
            }
        }
    }
}

KTextEditor::Document *KateDocManager::createDoc(const KateDocumentInfo &docInfo)
{
    KTextEditor::Document *doc = KTextEditor::Editor::instance()->createDocument(this);

    // turn of the editorpart's own modification dialog, we have our own one, too!
    const KConfigGroup generalGroup(KSharedConfig::openConfig(), "General");
    bool ownModNotification = generalGroup.readEntry("Modified Notification", false);
    if (qobject_cast<KTextEditor::ModificationInterface *>(doc)) {
        qobject_cast<KTextEditor::ModificationInterface *>(doc)->setModifiedOnDiskWarning(!ownModNotification);
    }

    m_docList.push_back(doc);
    m_docInfos.emplace(doc, docInfo);

    // connect internal signals...
    connect(doc, &KTextEditor::Document::modifiedChanged, this, &KateDocManager::slotModChanged1);
    // clang-format off
    connect(doc,
            SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
            this,
            SLOT(slotModifiedOnDisc(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));
    // clang-format on

    // we have a new document, show it the world
    Q_EMIT documentCreated(doc);
    Q_EMIT documentCreatedViewManager(doc);

    // return our new document
    return doc;
}

KateDocumentInfo *KateDocManager::documentInfo(KTextEditor::Document *doc)
{
    auto it = m_docInfos.find(doc);
    if (it != m_docInfos.end()) {
        return &it->second;
    }
    return nullptr;
}

static QUrl normalizeUrl(const QUrl &url)
{
    // Resolve symbolic links for local files (done anyway in KTextEditor)
    if (url.isLocalFile()) {
        QString normalizedUrl = QFileInfo(url.toLocalFile()).canonicalFilePath();
        if (!normalizedUrl.isEmpty()) {
            return QUrl::fromLocalFile(normalizedUrl);
        }
    }

    // else: cleanup only the .. stuff
    return url.adjusted(QUrl::NormalizePathSegments);
}

KTextEditor::Document *KateDocManager::findDocument(const QUrl &url) const
{
    const QUrl u(normalizeUrl(url));
    for (KTextEditor::Document *it : m_docList) {
        if (it->url() == u) {
            return it;
        }
    }
    return nullptr;
}

std::vector<KTextEditor::Document *>
KateDocManager::openUrls(const QList<QUrl> &urls, const QString &encoding, bool isTempFile, const KateDocumentInfo &docInfo)
{
    std::vector<KTextEditor::Document *> docs;
    docs.reserve(urls.size());
    for (const QUrl &url : urls) {
        docs.push_back(openUrl(url, encoding, isTempFile, docInfo));
    }
    return docs;
}

KTextEditor::Document *KateDocManager::openUrl(const QUrl &url, const QString &encoding, bool isTempFile, const KateDocumentInfo &docInfo)
{
    // special handling: if only one unmodified empty buffer in the list,
    // keep this buffer in mind to close it after opening the new url
    KTextEditor::Document *untitledDoc = nullptr;
    if ((documentList().size() == 1) && (!documentList().at(0)->isModified() && documentList().at(0)->url().isEmpty())) {
        untitledDoc = documentList().front();
    }

    //
    // create new document
    //
    const QUrl u(normalizeUrl(url));
    KTextEditor::Document *doc = nullptr;

    // always new document if url is empty...
    if (!u.isEmpty()) {
        doc = findDocument(u);
    }

    if (!doc) {
        if (untitledDoc) {
            QTimer::singleShot(0, untitledDoc, [this, untitledDoc] {
                closeDocument(untitledDoc);
            });
        }
        doc = createDoc(docInfo);

        if (!encoding.isEmpty()) {
            doc->setEncoding(encoding);
        }

        if (!u.isEmpty()) {
            doc->openUrl(u);
            loadMetaInfos(doc, u);
        }
    }

    //
    // if needed, register as temporary file
    //
    if (isTempFile && u.isLocalFile()) {
        QFileInfo fi(u.toLocalFile());
        if (fi.exists()) {
            m_tempFiles[doc] = std::make_pair(u, fi.lastModified());
            qCDebug(LOG_KATE) << "temporary file will be deleted after use unless modified: " << u;
        }
    }

    return doc;
}

bool KateDocManager::closeDocuments(const QList<KTextEditor::Document *> &documents, bool closeUrl)
{
    if (documents.empty()) {
        return false;
    }

    saveMetaInfos(documents);

    Q_EMIT aboutToDeleteDocuments(QList<KTextEditor::Document *>{documents.begin(), documents.end()});

    int last = 0;
    bool success = true;
    for (KTextEditor::Document *doc : documents) {
        if (closeUrl && !doc->closeUrl()) {
            success = false; // get out on first error
            break;
        }

        if (closeUrl) {
            auto it = m_tempFiles.find(doc);
            if (it != m_tempFiles.end()) {
                const auto [url, lastMod] = it->second;
                QFileInfo fi(url.toLocalFile());
                if (fi.lastModified() <= lastMod
                    || KMessageBox::questionYesNo(KateApp::self()->activeKateMainWindow(),
                                                  i18n("The supposedly temporary file %1 has been modified. "
                                                       "Do you want to delete it anyway?",
                                                       url.url(QUrl::PreferLocalFile)),
                                                  i18n("Delete File?"))
                        == KMessageBox::Yes) {
                    KIO::del(url, KIO::HideProgressInfo);
                    qCDebug(LOG_KATE) << "Deleted temporary file " << url;
                    m_tempFiles.erase(it);
                } else {
                    m_tempFiles.erase(it);
                    qCDebug(LOG_KATE) << "The supposedly temporary file " << url.url() << " have been modified since loaded, and has not been deleted.";
                }
            }
        }

        KateApp::self()->emitDocumentClosed(QString::number(reinterpret_cast<qptrdiff>(doc)));

        // document will be deleted, soon
        Q_EMIT documentWillBeDeleted(doc);

        // really delete the document and its infos
        m_docInfos.erase(doc);
        delete m_docList.takeAt(m_docList.indexOf(doc));

        // document is gone, emit our signals
        Q_EMIT documentDeleted(doc);

        last++;
    }

    /**
     * never ever empty the whole document list
     * do this before documentsDeleted is emitted, to have no flicker
     */
    if (m_docList.empty()) {
        createDoc();
    }

    Q_EMIT documentsDeleted(QList<KTextEditor::Document *>{documents.begin() + last, documents.end()});

    return success;
}

bool KateDocManager::closeDocument(KTextEditor::Document *doc, bool closeUrl)
{
    if (!doc) {
        return false;
    }

    return closeDocuments({doc}, closeUrl);
}

bool KateDocManager::closeDocumentList(const QList<KTextEditor::Document *> &documents)
{
    std::vector<KTextEditor::Document *> modifiedDocuments;
    for (KTextEditor::Document *document : documents) {
        if (document->isModified()) {
            modifiedDocuments.push_back(document);
        }
    }

    if (!modifiedDocuments.empty() && !KateSaveModifiedDialog::queryClose(nullptr, modifiedDocuments)) {
        return false;
    }

    return closeDocuments(documents, false); // Do not show save/discard dialog
}

bool KateDocManager::closeAllDocuments(bool closeUrl)
{
    /**
     * just close all documents
     */
    return closeDocuments(m_docList, closeUrl);
}

bool KateDocManager::closeOtherDocuments(KTextEditor::Document *doc)
{
    /**
     * close all documents beside the passed one
     */
    QList<KTextEditor::Document *> documents;
    documents.reserve(m_docList.size() - 1);
    for (auto document : m_docList) {
        if (document != doc) {
            documents.push_back(document);
        }
    }

    return closeDocuments(documents);
}

/**
 * Find all modified documents.
 * @return Return the list of all modified documents.
 */
std::vector<KTextEditor::Document *> KateDocManager::modifiedDocumentList()
{
    std::vector<KTextEditor::Document *> modified;
    std::copy_if(m_docList.begin(), m_docList.end(), std::back_inserter(modified), [](KTextEditor::Document *doc) {
        return doc->isModified();
    });
    return modified;
}

bool KateDocManager::queryCloseDocuments(KateMainWindow *w)
{
    const auto docCount = m_docList.size();
    for (KTextEditor::Document *doc : m_docList) {
        if (doc->url().isEmpty() && doc->isModified()) {
            int msgres = KMessageBox::warningYesNoCancel(w,
                                                         i18n("<p>The document '%1' has been modified, but not saved.</p>"
                                                              "<p>Do you want to save your changes or discard them?</p>",
                                                              doc->documentName()),
                                                         i18n("Close Document"),
                                                         KStandardGuiItem::save(),
                                                         KStandardGuiItem::discard());

            if (msgres == KMessageBox::Cancel) {
                return false;
            }

            if (msgres == KMessageBox::Yes) {
                const QUrl url = QFileDialog::getSaveFileUrl(w, i18n("Save As"));
                if (!url.isEmpty()) {
                    if (!doc->saveAs(url)) {
                        return false;
                    }
                } else {
                    return false;
                }
            }
        } else {
            if (!doc->queryClose()) {
                return false;
            }
        }
    }

    // document count changed while queryClose, abort and notify user
    if (m_docList.size() > docCount) {
        KMessageBox::information(w, i18n("New file opened while trying to close Kate, closing aborted."), i18n("Closing Aborted"));
        return false;
    }

    return true;
}

void KateDocManager::saveAll()
{
    for (KTextEditor::Document *doc : m_docList) {
        if (doc->isModified()) {
            doc->documentSave();
        }
    }
}

void KateDocManager::saveSelected(const QList<KTextEditor::Document *> &docList)
{
    for (KTextEditor::Document *doc : docList) {
        if (doc->isModified()) {
            doc->documentSave();
        }
    }
}

void KateDocManager::reloadAll()
{
    // reload all docs that are NOT modified on disk
    for (KTextEditor::Document *doc : m_docList) {
        if (!documentInfo(doc)->modifiedOnDisc) {
            doc->documentReload();
        }
    }

    // take care of all documents that ARE modified on disk
    KateApp::self()->activeKateMainWindow()->showModOnDiskPrompt(KateMainWindow::PromptAll);
}

void KateDocManager::closeOrphaned()
{
    QList<KTextEditor::Document *> documents;

    for (KTextEditor::Document *doc : m_docList) {
        KateDocumentInfo *info = documentInfo(doc);
        if (info && !info->openSuccess) {
            documents.push_back(doc);
        }
    }

    closeDocuments(documents);
}

void KateDocManager::saveDocumentList(KConfig *config)
{
    KConfigGroup openDocGroup(config, "Open Documents");

    openDocGroup.writeEntry("Count", (int)m_docList.size());

    int i = 0;
    for (KTextEditor::Document *doc : m_docList) {
        const QString entryName = QStringLiteral("Document %1").arg(i);
        KConfigGroup cg(config, entryName);
        doc->writeSessionConfig(cg);

        i++;
    }
}

void KateDocManager::restoreDocumentList(KConfig *config)
{
    KConfigGroup openDocGroup(config, "Open Documents");
    unsigned int count = openDocGroup.readEntry("Count", 0);

    if (count == 0) {
        return;
    }

    QProgressDialog progress;
    progress.setWindowTitle(i18n("Starting Up"));
    progress.setLabelText(i18n("Reopening files from the last session..."));
    progress.setModal(true);
    progress.setCancelButton(nullptr);
    progress.setRange(0, count);

    for (unsigned int i = 0; i < count; i++) {
        KConfigGroup cg(config, QStringLiteral("Document %1").arg(i));
        KTextEditor::Document *doc = nullptr;

        if (i == 0) {
            doc = m_docList.front();
        } else {
            doc = createDoc();
        }

        connect(doc, SIGNAL(completed()), this, SLOT(documentOpened()));
        connect(doc, &KParts::ReadOnlyPart::canceled, this, &KateDocManager::documentOpened);

        doc->readSessionConfig(cg);

        KateApp::self()->stashManager()->popDocument(doc, cg);

        progress.setValue(i);
    }
}

void KateDocManager::slotModifiedOnDisc(KTextEditor::Document *doc, bool b, KTextEditor::ModificationInterface::ModifiedOnDiskReason reason)
{
    auto it = m_docInfos.find(doc);
    if (it != m_docInfos.end()) {
        it->second.modifiedOnDisc = b;
        it->second.modifiedOnDiscReason = reason;
        slotModChanged1(doc);
    }
}

/**
 * Load file's meta-information if the checksum didn't change since last time.
 */
bool KateDocManager::loadMetaInfos(KTextEditor::Document *doc, const QUrl &url)
{
    if (!m_saveMetaInfos) {
        return false;
    }

    if (!m_metaInfos.hasGroup(url.toDisplayString())) {
        return false;
    }

    const QByteArray checksum = doc->checksum().toHex();
    bool ok = true;
    if (!checksum.isEmpty()) {
        KConfigGroup urlGroup(&m_metaInfos, url.toDisplayString());
        const QString old_checksum = urlGroup.readEntry("Checksum");

        if (QString::fromLatin1(checksum) == old_checksum) {
            QSet<QString> flags;
            if (documentInfo(doc)->openedByUser) {
                flags << QStringLiteral("SkipEncoding");
            }
            flags << QStringLiteral("SkipUrl");
            doc->readSessionConfig(urlGroup, flags);
        } else {
            urlGroup.deleteGroup();
            ok = false;
        }

        m_metaInfos.sync();
    }

    return ok && doc->url() == url;
}

/**
 * Save file's meta-information if doc is in 'unmodified' state
 */

void KateDocManager::saveMetaInfos(const QList<KTextEditor::Document *> &documents)
{
    /**
     * skip work if no meta infos wanted
     */
    if (!m_saveMetaInfos) {
        return;
    }

    /**
     * store meta info for all non-modified documents which have some checksum
     */
    const QDateTime now = QDateTime::currentDateTimeUtc();
    for (KTextEditor::Document *doc : documents) {
        /**
         * skip modified docs
         */
        if (doc->isModified()) {
            continue;
        }

        const QByteArray checksum = doc->checksum().toHex();
        if (!checksum.isEmpty()) {
            /**
             * write the group with checksum and time
             */
            KConfigGroup urlGroup(&m_metaInfos, doc->url().toString());
            urlGroup.writeEntry("Checksum", QString::fromLatin1(checksum));
            urlGroup.writeEntry("Time", now);

            /**
             * write document session config
             */
            doc->writeSessionConfig(urlGroup);
        }
    }

    /**
     * sync to not loose data
     */
    m_metaInfos.sync();
}

void KateDocManager::slotModChanged(KTextEditor::Document *doc)
{
    saveMetaInfos({doc});
}

void KateDocManager::slotModChanged1(KTextEditor::Document *doc)
{
    // clang-format off
    QMetaObject::invokeMethod(KateApp::self()->activeKateMainWindow(), "queueModifiedOnDisc", Qt::QueuedConnection, Q_ARG(KTextEditor::Document*,doc));
    // clang-format on
}

void KateDocManager::documentOpened()
{
    KColorScheme colors(QPalette::Active);

    KTextEditor::Document *doc = qobject_cast<KTextEditor::Document *>(sender());
    if (!doc) {
        return; // should never happen, but who knows
    }
    disconnect(doc, SIGNAL(completed()), this, SLOT(documentOpened()));
    disconnect(doc, &KParts::ReadOnlyPart::canceled, this, &KateDocManager::documentOpened);

    // Only set "no success" when doc is empty to avoid close of files
    // with other trouble when do closeOrphaned()
    if (doc->openingError() && doc->isEmpty()) {
        KateDocumentInfo *info = documentInfo(doc);
        if (info) {
            info->openSuccess = false;
        }
    }
}
