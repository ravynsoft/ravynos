/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
 *  SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __SNIPPETREPOSITORY_H__
#define __SNIPPETREPOSITORY_H__

#include <QDir>
#include <QObject>
#include <QStandardItem>
#include <QStringList>

namespace KTextEditor
{
}

/**
 * Each object of this type represents a repository of snippets. Each repository
 * has a name and will be saved to an XML file that includes all items of this repository.
 *
 * To access the snippets in this repo, iterate over it's children and dynamic_cast as required.
 * To add a snippet, @p appendRow() it.
 * To access the name of the repository, use @p text() and @p setText().
 *
 * NOTE: Unchecked repositories are considered "disabled" in the sense that their snippets
 *       won't show up during code completion.
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 * @author Milian Wolff <mail@milianw.de>
 */
class SnippetRepository : public QObject, public QStandardItem
{
    Q_OBJECT

public:
    /**
     * Creates a new SnippetRepository. When @p file exists it will be parsed (XML).
     *
     * @param file Location of the snippet's repository file.
     */
    SnippetRepository(const QString &file);
    ~SnippetRepository() override;

    /**
     * Creates a snippet repository for the given name and adds it to the SnippetStore.
     */
    static SnippetRepository *createRepoFromName(const QString &name);

    /**
     * The license for the snippets contained in this repository.
     */
    QString license() const;
    /**
     * Sets the license for the snippets contained in this repository.
     */
    void setLicense(const QString &license);

    /**
     * The author(s) of the snippets contained in this repository.
     */
    QString authors() const;
    /**
     * Sets the author(s) of the snippets contained in this repository.
     */
    void setAuthors(const QString &authors);

    /**
     * The valid filetypes for the snippets contained in this repository.
     * Empty list means no restriction on the modes.
     * @see KTextEditor::Document::mode()
     */
    QStringList fileTypes() const;
    /**
     * Sets the valid filetypes for the snippets contained in this repository.
     * An empty list, or any list which contains an element "*" is treated as
     * a no-restriction filter.
     */
    void setFileTypes(const QStringList &filetypes);

    /**
     * The path to this repository's file.
     */
    const QString &file() const;

    /**
     * The namespace associated with this repository.
     * Used in CodeCompletion for filtering.
     */
    QString completionNamespace() const;
    /**
     * Sets the code completion namespace for this repository.
     */
    void setCompletionNamespace(const QString &completionNamespace);

    /**
     * The QtScript(s) associated with this repository.
     *
     * @since KDE 4.5
     */
    QString script() const;

    /**
     * Sets the QtScript(s) associated with this repository.
     *
     * @since KDE 4.5
     */
    void setScript(const QString &script);

    /**
     * Remove this repository from the disk. Also deletes the item and all its children.
     */
    void remove();

    /**
     * Save this repository to disk.
     */
    void save();

    /**
     * Get directory for data storage from QStandardPaths
     */
    static QDir dataPath();

    QVariant data(int role = Qt::UserRole + 1) const override;
    void setData(const QVariant &value, int role = Qt::UserRole + 1) override;

private Q_SLOTS:
    /// parses the XML file and load the containing snippets.
    void slotParseFile();

private:
    /// path to the repository file
    QString m_file;
    /// license of the snippets in this repo
    QString m_license;
    /// author(s) of the snippets in this repo
    QString m_authors;
    /// valid filetypes for the snippets in this repo
    QStringList m_filetypes;
    /// filtering namespace for code completion
    QString m_namespace;
    /// QtScript with functions to be used in the snippets; common to all snippets
    QString m_script;
};

#endif
