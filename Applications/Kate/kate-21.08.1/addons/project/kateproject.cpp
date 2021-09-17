/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateproject.h"
#include "kateprojectplugin.h"
#include "kateprojectworker.h"

#include <KLocalizedString>

#include <ktexteditor/document.h>

#include <json_utils.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QPlainTextDocumentLayout>
#include <utility>

KateProject::KateProject(QThreadPool &threadPool, KateProjectPlugin *plugin)
    : m_notesDocument(nullptr)
    , m_untrackedDocumentsRoot(nullptr)
    , m_threadPool(threadPool)
    , m_plugin(plugin)
{
}

KateProject::~KateProject()
{
    saveNotesDocument();
}

bool KateProject::loadFromFile(const QString &fileName)
{
    /**
     * bail out if already fileName set!
     */
    if (!m_fileName.isEmpty()) {
        return false;
    }

    /**
     * set new filename and base directory
     */
    m_fileName = fileName;
    m_baseDir = QFileInfo(m_fileName).canonicalPath();

    /**
     * trigger reload
     */
    return reload();
}

bool KateProject::reload(bool force)
{
    QVariantMap map = readProjectFile();

    if (map.isEmpty()) {
        m_fileLastModified = QDateTime();
    } else {
        m_fileLastModified = QFileInfo(m_fileName).lastModified();
        m_globalProject = map;
    }

    return load(m_globalProject, force);
}

void KateProject::renameFile(const QString &newName, const QString &oldName)
{
    auto it = m_file2Item->find(oldName);
    if (it == m_file2Item->end()) {
        qWarning() << "renameFile() File not found, new: " << newName << "old: " << oldName;
        return;
    }
    (*m_file2Item)[newName] = it.value();
    m_file2Item->erase(it);
}

void KateProject::removeFile(const QString &file)
{
    auto it = m_file2Item->find(file);
    if (it == m_file2Item->end()) {
        qWarning() << "removeFile() File not found: " << file;
        return;
    }
    m_file2Item->erase(it);
}

/**
 * Read a JSON document from file.
 *
 * In case of an error, the returned object verifies isNull() is true.
 */
QJsonDocument KateProject::readJSONFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.exists() || !file.open(QFile::ReadOnly)) {
        return QJsonDocument();
    }

    /**
     * parse the whole file, bail out again on error!
     */
    const QByteArray jsonData = file.readAll();
    QJsonParseError parseError{};
    QJsonDocument document(QJsonDocument::fromJson(jsonData, &parseError));

    if (parseError.error != QJsonParseError::NoError) {
        return QJsonDocument();
    }

    return document;
}

QVariantMap KateProject::readProjectFile() const
{
    QJsonDocument project(readJSONFile(m_fileName));
    // bail out on error
    if (project.isNull()) {
        return QVariantMap();
    }

    /**
     * convenience; align with auto-generated projects
     * generate 'name' and 'files' if not specified explicitly,
     * so those parts need not be given if only wishes to specify additional
     * project configuration (e.g. build, ctags)
     */
    if (project.isObject()) {
        auto dir = QFileInfo(m_fileName).dir();
        auto object = project.object();

        // if there are local settings (.kateproject.local), override values
        {
            const auto localSettings = readJSONFile(projectLocalFileName(QStringLiteral("local")));
            if (!localSettings.isNull() && localSettings.isObject()) {
                object = json::merge(object, localSettings.object());
            }
        }

        auto name = object[QStringLiteral("name")];
        if (name.isUndefined() || name.isNull()) {
            name = dir.dirName();
        }
        auto files = object[QStringLiteral("files")];
        if (files.isUndefined() || files.isNull()) {
            // support all we can, could try to detect,
            // but it will be sorted out upon loading anyway
            QJsonArray afiles;
            for (const auto &t : {QStringLiteral("git"), QStringLiteral("hg"), QStringLiteral("svn"), QStringLiteral("darcs")}) {
                afiles.push_back(QJsonObject{{t, true}});
            }
            files = afiles;
        }
        project.setObject(object);
    }

    return project.toVariant().toMap();
}

bool KateProject::loadFromData(const QVariantMap &globalProject, const QString &directory)
{
    m_baseDir = directory;
    m_fileName = QDir(directory).filePath(QStringLiteral(".kateproject"));
    m_globalProject = globalProject;
    return load(globalProject);
}

bool KateProject::load(const QVariantMap &globalProject, bool force)
{
    /**
     * no name, bad => bail out
     */
    if (globalProject[QStringLiteral("name")].toString().isEmpty()) {
        return false;
    }

    /**
     * support out-of-source project files
     */
    if (!globalProject[QStringLiteral("directory")].toString().isEmpty()) {
        m_baseDir = QFileInfo(globalProject[QStringLiteral("directory")].toString()).canonicalFilePath();
    }

    /**
     * anything changed?
     * else be done without forced reload!
     */
    if (!force && (m_projectMap == globalProject)) {
        return true;
    }

    /**
     * setup global attributes in this object
     */
    m_projectMap = globalProject;

    // emit that we changed stuff
    Q_EMIT projectMapChanged();

    // trigger loading of project in background thread
    QString indexDir;
    if (m_plugin->getIndexEnabled()) {
        indexDir = m_plugin->getIndexDirectory().toLocalFile();
        // if empty, use regular tempdir
        if (indexDir.isEmpty()) {
            indexDir = QDir::tempPath();
        }
    }

    // let's run the stuff in our own thread pool
    // do manual queued connect, as only run() is done in extra thread, object stays in this one
    auto w = new KateProjectWorker(m_baseDir, indexDir, m_projectMap, force);
    connect(w, &KateProjectWorker::loadDone, this, &KateProject::loadProjectDone, Qt::QueuedConnection);
    connect(w, &KateProjectWorker::loadIndexDone, this, &KateProject::loadIndexDone, Qt::QueuedConnection);
    m_threadPool.start(w);

    // we are done here
    return true;
}

void KateProject::loadProjectDone(const KateProjectSharedQStandardItem &topLevel, KateProjectSharedQHashStringItem file2Item)
{
    m_model.clear();
    m_model.invisibleRootItem()->appendColumn(topLevel->takeColumn(0));

    m_file2Item = std::move(file2Item);

    /**
     * readd the documents that are open atm
     */
    m_untrackedDocumentsRoot = nullptr;
    for (auto i = m_documents.constBegin(); i != m_documents.constEnd(); i++) {
        registerDocument(i.key());
    }

    Q_EMIT modelChanged();
}

void KateProject::loadIndexDone(KateProjectSharedProjectIndex projectIndex)
{
    /**
     * move to our project
     */
    m_projectIndex = std::move(projectIndex);

    /**
     * notify external world that data is available
     */
    Q_EMIT indexChanged();
}

QString KateProject::projectLocalFileName(const QString &suffix) const
{
    /**
     * nothing on empty file names for project
     * should not happen
     */
    if (m_baseDir.isEmpty() || suffix.isEmpty()) {
        return QString();
    }

    /**
     * compute full file name
     */
    return QDir(m_baseDir).filePath(QStringLiteral(".kateproject.") + suffix);
}

QTextDocument *KateProject::notesDocument()
{
    /**
     * already there?
     */
    if (m_notesDocument) {
        return m_notesDocument;
    }

    /**
     * else create it
     */
    m_notesDocument = new QTextDocument(this);
    m_notesDocument->setDocumentLayout(new QPlainTextDocumentLayout(m_notesDocument));

    /**
     * get file name
     */
    const QString notesFileName = projectLocalFileName(QStringLiteral("notes"));
    if (notesFileName.isEmpty()) {
        return m_notesDocument;
    }

    /**
     * and load text if possible
     */
    QFile inFile(notesFileName);
    if (inFile.open(QIODevice::ReadOnly)) {
        QTextStream inStream(&inFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        inStream.setCodec("UTF-8");
#endif
        m_notesDocument->setPlainText(inStream.readAll());
    }

    /**
     * and be done
     */
    return m_notesDocument;
}

void KateProject::saveNotesDocument()
{
    /**
     * no notes document, nothing to do
     */
    if (!m_notesDocument) {
        return;
    }

    /**
     * get content & filename
     */
    const QString content = m_notesDocument->toPlainText();
    const QString notesFileName = projectLocalFileName(QStringLiteral("notes"));
    if (notesFileName.isEmpty()) {
        return;
    }

    /**
     * no content => unlink file, if there
     */
    if (content.isEmpty()) {
        if (QFile::exists(notesFileName)) {
            QFile::remove(notesFileName);
        }
        return;
    }

    /**
     * else: save content to file
     */
    QFile outFile(projectLocalFileName(QStringLiteral("notes")));
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QTextStream outStream(&outFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        outStream.setCodec("UTF-8");
#endif
        outStream << content;
    }
}

void KateProject::slotModifiedChanged(KTextEditor::Document *document)
{
    KateProjectItem *item = itemForFile(m_documents.value(document));

    if (!item) {
        return;
    }

    item->slotModifiedChanged(document);
}

void KateProject::slotModifiedOnDisk(KTextEditor::Document *document, bool isModified, KTextEditor::ModificationInterface::ModifiedOnDiskReason reason)
{
    KateProjectItem *item = itemForFile(m_documents.value(document));

    if (!item) {
        return;
    }

    item->slotModifiedOnDisk(document, isModified, reason);
}

void KateProject::registerDocument(KTextEditor::Document *document)
{
    // remember the document, if not already there
    if (!m_documents.contains(document)) {
        m_documents[document] = document->url().toLocalFile();
    }

    // try to get item for the document
    KateProjectItem *item = itemForFile(document->url().toLocalFile());

    // if we got one, we are done, else create a dummy!
    // clang-format off
    if (item) {
        disconnect(document, &KTextEditor::Document::modifiedChanged, this, &KateProject::slotModifiedChanged);
        disconnect(document,
                   SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
                   this,
                   SLOT(slotModifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));
        item->slotModifiedChanged(document);

        /*FIXME    item->slotModifiedOnDisk(document,document->isModified(),qobject_cast<KTextEditor::ModificationInterface*>(document)->modifiedOnDisk());
         * FIXME*/

        connect(document, &KTextEditor::Document::modifiedChanged, this, &KateProject::slotModifiedChanged);
        connect(document,
                SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
                this,
                SLOT(slotModifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));

        return;
    }
    // clang-format on

    registerUntrackedDocument(document);
}

void KateProject::registerUntrackedDocument(KTextEditor::Document *document)
{
    // perhaps create the parent item
    if (!m_untrackedDocumentsRoot) {
        m_untrackedDocumentsRoot = new KateProjectItem(KateProjectItem::Directory, i18n("<untracked>"));
        m_model.insertRow(0, m_untrackedDocumentsRoot);
    }

    // create document item
    QFileInfo fileInfo(document->url().toLocalFile());
    KateProjectItem *fileItem = new KateProjectItem(KateProjectItem::File, fileInfo.fileName());
    fileItem->slotModifiedChanged(document);
    connect(document, &KTextEditor::Document::modifiedChanged, this, &KateProject::slotModifiedChanged);
    // clang-format off
    connect(document,
            SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
            this,
            SLOT(slotModifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));
    // clang-format on

    bool inserted = false;
    for (int i = 0; i < m_untrackedDocumentsRoot->rowCount(); ++i) {
        if (m_untrackedDocumentsRoot->child(i)->data(Qt::UserRole).toString() > document->url().toLocalFile()) {
            m_untrackedDocumentsRoot->insertRow(i, fileItem);
            inserted = true;
            break;
        }
    }
    if (!inserted) {
        m_untrackedDocumentsRoot->appendRow(fileItem);
    }

    fileItem->setData(document->url().toLocalFile(), Qt::UserRole);
    fileItem->setData(QVariant(true), Qt::UserRole + 3);

    if (!m_file2Item) {
        m_file2Item = KateProjectSharedQHashStringItem(new QHash<QString, KateProjectItem *>());
    }
    (*m_file2Item)[document->url().toLocalFile()] = fileItem;
}

void KateProject::unregisterDocument(KTextEditor::Document *document)
{
    if (!m_documents.contains(document)) {
        return;
    }

    disconnect(document, &KTextEditor::Document::modifiedChanged, this, &KateProject::slotModifiedChanged);

    const QString &file = m_documents.value(document);

    if (m_untrackedDocumentsRoot) {
        KateProjectItem *item = static_cast<KateProjectItem *>(itemForFile(file));
        if (item && item->data(Qt::UserRole + 3).toBool()) {
            unregisterUntrackedItem(item);
            m_file2Item->remove(file);
        }
    }

    m_documents.remove(document);
}

void KateProject::unregisterUntrackedItem(const KateProjectItem *item)
{
    for (int i = 0; i < m_untrackedDocumentsRoot->rowCount(); ++i) {
        if (m_untrackedDocumentsRoot->child(i) == item) {
            m_untrackedDocumentsRoot->removeRow(i);
            break;
        }
    }

    if (m_untrackedDocumentsRoot->rowCount() < 1) {
        m_model.removeRow(0);
        m_untrackedDocumentsRoot = nullptr;
    }
}
