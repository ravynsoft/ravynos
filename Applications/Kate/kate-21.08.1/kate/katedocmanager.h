/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __KATE_DOCMANAGER_H__
#define __KATE_DOCMANAGER_H__

#include <ktexteditor/document.h>
#include <ktexteditor/modificationinterface.h>

#include <QDateTime>
#include <QList>
#include <QObject>

#include <KConfig>

#include <unordered_map>

class KateMainWindow;

class KateDocumentInfo
{
public:
    enum CustomRoles { RestoreOpeningFailedRole };

public:
    KateDocumentInfo() = default;

    bool modifiedOnDisc = false;
    KTextEditor::ModificationInterface::ModifiedOnDiskReason modifiedOnDiscReason = KTextEditor::ModificationInterface::OnDiskUnmodified;

    bool openedByUser = false;
    bool openSuccess = true;
    bool doPostLoadOperations = false;
};

class KateDocManager : public QObject
{
    Q_OBJECT

public:
    KateDocManager(QObject *parent);
    ~KateDocManager() override;

    KTextEditor::Document *createDoc(const KateDocumentInfo &docInfo = KateDocumentInfo());

    KateDocumentInfo *documentInfo(KTextEditor::Document *doc);

    /** Returns the documentNumber of the doc with url URL or -1 if no such doc is found */
    KTextEditor::Document *findDocument(const QUrl &url) const;

    const QList<KTextEditor::Document *> &documentList() const
    {
        return m_docList;
    }

    KTextEditor::Document *
    openUrl(const QUrl &, const QString &encoding = QString(), bool isTempFile = false, const KateDocumentInfo &docInfo = KateDocumentInfo());

    std::vector<KTextEditor::Document *>
    openUrls(const QList<QUrl> &, const QString &encoding = QString(), bool isTempFile = false, const KateDocumentInfo &docInfo = KateDocumentInfo());

    bool closeDocument(KTextEditor::Document *, bool closeUrl = true);
    bool closeDocuments(const QList<KTextEditor::Document *> &documents, bool closeUrl = true);
    bool closeDocumentList(const QList<KTextEditor::Document *> &documents);
    bool closeAllDocuments(bool closeUrl = true);
    bool closeOtherDocuments(KTextEditor::Document *);

    std::vector<KTextEditor::Document *> modifiedDocumentList();
    bool queryCloseDocuments(KateMainWindow *w);

    void saveDocumentList(KConfig *config);
    void restoreDocumentList(KConfig *config);

    inline bool getSaveMetaInfos()
    {
        return m_saveMetaInfos;
    }
    inline void setSaveMetaInfos(bool b)
    {
        m_saveMetaInfos = b;
    }

    inline int getDaysMetaInfos()
    {
        return m_daysMetaInfos;
    }
    inline void setDaysMetaInfos(int i)
    {
        m_daysMetaInfos = i;
    }

public Q_SLOTS:
    /**
     * saves all documents that has at least one view.
     * documents with no views are ignored :P
     */
    void saveAll();

    /**
     * reloads all documents that has at least one view.
     * documents with no views are ignored :P
     */
    void reloadAll();

    /**
     * close all documents, which could not be reopened
     */
    void closeOrphaned();

    /**
     * save selected documents from the File List
     */
    void saveSelected(const QList<KTextEditor::Document *> &);

Q_SIGNALS:
    /**
     * This signal is emitted when the \p document was created.
     */
    void documentCreated(KTextEditor::Document *document);

    /**
     * This signal is emitted when the \p document was created.
     * This is emitted after the initial documentCreated for internal use in view manager
     */
    void documentCreatedViewManager(KTextEditor::Document *document);

    /**
     * This signal is emitted before a \p document which should be closed is deleted
     * The document is still accessible and usable, but it will be deleted
     * after this signal was send.
     *
     * @param document document that will be deleted
     */
    void documentWillBeDeleted(KTextEditor::Document *document);

    /**
     * This signal is emitted when the \p document has been deleted.
     *
     *  Warning !!! DO NOT ACCESS THE DATA REFERENCED BY THE POINTER, IT IS ALREADY INVALID
     *  Use the pointer only to remove mappings in hash or maps
     */
    void documentDeleted(KTextEditor::Document *document);

    /**
     * This signal is emitted before the documents batch is going to be deleted
     *
     * note that the batch can be interrupted in the middle and only some
     * of the documents may be actually deleted. See documentsDeleted() signal.
     */
    void aboutToDeleteDocuments(const QList<KTextEditor::Document *> &);

    /**
     * This signal is emitted after the documents batch was deleted
     *
     * This is the batch closing signal for aboutToDeleteDocuments
     * @param documents the documents that weren't deleted after all
     */
    void documentsDeleted(const QList<KTextEditor::Document *> &documents);

private Q_SLOTS:
    void slotModifiedOnDisc(KTextEditor::Document *doc, bool b, KTextEditor::ModificationInterface::ModifiedOnDiskReason reason);
    void slotModChanged(KTextEditor::Document *doc);
    void slotModChanged1(KTextEditor::Document *doc);

private:
    bool loadMetaInfos(KTextEditor::Document *doc, const QUrl &url);
    void saveMetaInfos(const QList<KTextEditor::Document *> &docs);

    QList<KTextEditor::Document *> m_docList;
    std::unordered_map<KTextEditor::Document *, KateDocumentInfo> m_docInfos;

    KConfig m_metaInfos;
    bool m_saveMetaInfos;
    int m_daysMetaInfos;

    typedef std::pair<QUrl, QDateTime> TPair;
    std::unordered_map<KTextEditor::Document *, TPair> m_tempFiles;

private Q_SLOTS:
    void documentOpened();
};

#endif
