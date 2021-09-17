/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
 *  SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __SNIPPETSTORE_H__
#define __SNIPPETSTORE_H__

#include <KConfigGroup>
#include <QStandardItemModel>

class SnippetRepository;
class KateSnippetGlobal;

namespace KTextEditor
{
}

/**
 * This class is implemented as singelton.
 * It represents the model containing all snippet repositories with their snippets.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 * @author Milian Wolff <mail@milianw.de>
 */
class SnippetStore : public QStandardItemModel
{
    Q_OBJECT

public:
    /**
     * Initialize the SnippetStore.
     */
    static void init(KateSnippetGlobal *plugin);
    /**
     * Returns the SnippetStore. Call init() to set it up first.
     */
    static SnippetStore *self();

    ~SnippetStore() override;
    KConfigGroup getConfig();
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    /**
     * Returns the repository for the given @p file if there is any.
     */
    SnippetRepository *repositoryForFile(const QString &file);

private:
    SnippetStore(KateSnippetGlobal *plugin);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    static SnippetStore *m_self;
    KateSnippetGlobal *m_plugin;
};

#endif
