/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_PROJECT_H
#define KATE_PROJECT_H

#include "kateprojectindex.h"
#include "kateprojectitem.h"
#include <KTextEditor/ModificationInterface>
#include <QDateTime>
#include <QHash>
#include <QSharedPointer>
#include <QTextDocument>

/**
 * Shared pointer data types.
 * Used to pass pointers over queued connected slots
 */
typedef QSharedPointer<QStandardItem> KateProjectSharedQStandardItem;
Q_DECLARE_METATYPE(KateProjectSharedQStandardItem)

typedef QSharedPointer<QHash<QString, KateProjectItem *>> KateProjectSharedQHashStringItem;
Q_DECLARE_METATYPE(KateProjectSharedQHashStringItem)

typedef QSharedPointer<KateProjectIndex> KateProjectSharedProjectIndex;
Q_DECLARE_METATYPE(KateProjectSharedProjectIndex)

class KateProjectPlugin;
class QThreadPool;

/**
 * Class representing a project.
 * Holds project properties like name, groups, contained files, ...
 */
class KateProject : public QObject
{
    Q_OBJECT

public:
    /**
     * construct empty project
     */
    KateProject(QThreadPool &threadPool, KateProjectPlugin *plugin);

    /**
     * deconstruct project
     */
    ~KateProject() override;

    /**
     * Load a project from project file
     * Only works once, afterwards use reload().
     * @param fileName name of project file
     * @return success
     */
    bool loadFromFile(const QString &fileName);
    bool loadFromData(const QVariantMap &globalProject, const QString &directory);

    /**
     * Try to reload a project.
     * If the reload fails, e.g. because the file is not readable or corrupt, nothing will happen!
     * @param force will enforce the worker to update files list and co even if the content of the file was not changed!
     * @return success
     */
    bool reload(bool force = false);

    /**
     * Accessor to file name.
     * @return file name
     */
    const QString &fileName() const
    {
        return m_fileName;
    }

    /**
     * Return the base directory of this project.
     * @return base directory of project, might not be the directory of the fileName!
     */
    const QString &baseDir() const
    {
        return m_baseDir;
    }

    /**
     * Return the time when the project file has been modified last.
     * @return QFileInfo::lastModified()
     */
    QDateTime fileLastModified() const
    {
        return m_fileLastModified;
    }

    /**
     * Accessor to project map containing the whole project info.
     * @return project info
     */
    const QVariantMap &projectMap() const
    {
        return m_projectMap;
    }

    /**
     * Accessor to project name.
     * @return project name
     */
    QString name() const
    {
        // MSVC doesn't support QStringLiteral here
        return m_projectMap[QStringLiteral("name")].toString();
    }

    /**
     * Accessor for the model.
     * @return model of this project
     */
    QStandardItemModel *model()
    {
        return &m_model;
    }

    /**
     * Flat list of all files in the project
     * @return list of files in project
     */
    QStringList files()
    {
        return m_file2Item ? m_file2Item->keys() : QStringList();
    }

    /**
     * get item for file
     * @param file file to get item for
     * @return item for given file or 0
     */
    KateProjectItem *itemForFile(const QString &file)
    {
        return m_file2Item ? m_file2Item->value(file) : nullptr;
    }

    /**
     * add a new file to the project
     */
    void addFile(const QString &file, KateProjectItem *item)
    {
        if (m_file2Item && item) {
            (*m_file2Item)[file] = item;
        }
    }

    /**
     * rename a file
     */
    void renameFile(const QString &newName, const QString &oldName);
    
    /**
     * remove a file
     */
    void removeFile(const QString &file);

    /**
     * Access to project index.
     * May be null.
     * Don't store this pointer, might change.
     * @return project index
     */
    KateProjectIndex *projectIndex()
    {
        return m_projectIndex.data();
    }

    KateProjectPlugin *plugin()
    {
        return m_plugin;
    }

    /**
     * Computes a suitable file name for the given suffix.
     * If you e.g. want to store a "notes" file, you could pass "notes" and get
     * the full path to projectbasedir/.kateproject.notes
     * @param suffix suffix for the file
     * @return full path for project local file, on error => empty string
     */
    QString projectLocalFileName(const QString &suffix) const;

    /**
     * Document with project local notes.
     * Will be stored in a projectLocalFile "notes.txt".
     * @return notes document
     */
    QTextDocument *notesDocument();

    /**
     * Save the notes document to "notes.txt" if any document around.
     */
    void saveNotesDocument();

    /**
     * Register a document for this project.
     * @param document document to register
     */
    void registerDocument(KTextEditor::Document *document);

    /**
     * Unregister a document for this project.
     * @param document document to unregister
     */
    void unregisterDocument(KTextEditor::Document *document);

private Q_SLOTS:
    bool load(const QVariantMap &globalProject, bool force = false);

    /**
     * Used for worker to send back the results of project loading
     * @param topLevel new toplevel element for model
     * @param file2Item new file => item mapping
     */
    void loadProjectDone(const KateProjectSharedQStandardItem &topLevel, KateProjectSharedQHashStringItem file2Item);

    /**
     * Used for worker to send back the results of index loading
     * @param projectIndex new project index
     */
    void loadIndexDone(KateProjectSharedProjectIndex projectIndex);

    void slotModifiedChanged(KTextEditor::Document *);

    void slotModifiedOnDisk(KTextEditor::Document *document, bool isModified, KTextEditor::ModificationInterface::ModifiedOnDiskReason reason);

Q_SIGNALS:
    /**
     * Emitted on project map changes.
     * This includes the name!
     */
    void projectMapChanged();

    /**
     * Emitted on model changes.
     * This includes the files list, itemForFile mapping!
     */
    void modelChanged();

    /**
     * Emitted when the index creation is finished.
     * This includes the ctags index.
     */
    void indexChanged();

private:
    void registerUntrackedDocument(KTextEditor::Document *document);
    void unregisterUntrackedItem(const KateProjectItem *item);
    QVariantMap readProjectFile() const;
    /**
     * Read a JSON document from file.
     *
     * In case of an error, the returned object verifies isNull() is true.
     */
    static QJsonDocument readJSONFile(const QString &fileName);

private:
    /**
     * Last modification time of the project file
     */
    QDateTime m_fileLastModified;

    /**
     * project file name
     */
    QString m_fileName;

    /**
     * base directory of the project
     */
    QString m_baseDir;

    /**
     * project name
     */
    QString m_name;

    /**
     * variant map representing the project
     */
    QVariantMap m_projectMap;

    /**
     * standard item model with content of this project
     */
    QStandardItemModel m_model;

    /**
     * mapping files => items
     */
    KateProjectSharedQHashStringItem m_file2Item;

    /**
     * project index, if any
     */
    KateProjectSharedProjectIndex m_projectIndex;

    /**
     * notes buffer for project local notes
     */
    QTextDocument *m_notesDocument;

    /**
     * Set of existing documents for this project.
     */
    QHash<KTextEditor::Document *, QString> m_documents;

    /**
     * Parent item for existing documents that are not in the project tree
     */
    QStandardItem *m_untrackedDocumentsRoot;

    /**
     * thread pool used for project worker
     */
    QThreadPool &m_threadPool;

    /**
     * project configuration (read from file or injected)
     */
    QVariantMap m_globalProject;

    /**
     * Project plugin (configuration)
     */
    KateProjectPlugin *m_plugin;
};

#endif
