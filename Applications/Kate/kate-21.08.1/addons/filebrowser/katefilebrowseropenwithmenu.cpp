/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2020 Mario Aichinger <aichingm@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// BEGIN Includes
#include "katefilebrowseropenwithmenu.h"
// END Includes

// BEGIN KateFileBrowserOpenWithMenu

KateFileBrowserOpenWithMenu::KateFileBrowserOpenWithMenu(const QString &title, QWidget *parent)
    : QMenu(title, parent)
{
}

KateFileBrowserOpenWithMenu::~KateFileBrowserOpenWithMenu()
{
}

// END Constructor/Destructor

// kate: space-indent on; indent-width 2; replace-tabs on;
