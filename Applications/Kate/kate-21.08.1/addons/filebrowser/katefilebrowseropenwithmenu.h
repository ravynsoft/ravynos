/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2020 Mario Aichinger <aichingm@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_FILEBROWSEROPENWITHMENU_H
#define KATE_FILEBROWSEROPENWITHMENU_H

#include <KFileItem>
#include <QMenu>

/*
    The KateFileBrowserOpenWithMenu extends a QMenu with a KFileItem property, used to
    pass data of the selected file to the creation of the submenu.
*/

class KateFileBrowserOpenWithMenu : public QMenu
{
    Q_OBJECT
    Q_PROPERTY(KFileItem item READ item WRITE setItem)

public:
    explicit KateFileBrowserOpenWithMenu(const QString &title, QWidget *parent = nullptr);
    ~KateFileBrowserOpenWithMenu();

    void setItem(KFileItem item)
    {
        m_item = item;
    }

    KFileItem item()
    {
        return m_item;
    }

public Q_SLOTS:

private Q_SLOTS:

protected:
private:
    KFileItem m_item;
};

#endif // KATE_FILEBROWSEROPENWITHMENU_H

// kate: space-indent on; indent-width 2; replace-tabs on;
