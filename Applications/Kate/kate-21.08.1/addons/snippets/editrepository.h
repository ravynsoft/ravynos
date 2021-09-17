/*  This file is part of the Kate project.
 *  Based on the snippet plugin from KDevelop 4.
 *
 *  SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
 *  SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef EDITREPOSITORY_H
#define EDITREPOSITORY_H

#include "ui_editrepository.h"

#include <QDialog>

class SnippetRepository;

/**
 * This dialog is used to create/edit snippet repositories and
 * the snippets in them.
 *
 * @author Milian Wolff <mail@milianw.de>
 */
class EditRepository : public QDialog, public Ui::EditRepositoryBase
{
    Q_OBJECT

public:
    /// @p repo set to 0 when you want to create a new repository.
    explicit EditRepository(SnippetRepository *repo, QWidget *parent = nullptr);

private:
    SnippetRepository *m_repo;

private Q_SLOTS:
    void save();
    void validate();
    void updateFileTypes();
};

#endif
