/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "outputwidget.h"
#include "dataoutputwidget.h"
#include "textoutputwidget.h"
#include <KLocalizedString>

KateSQLOutputWidget::KateSQLOutputWidget(QWidget *parent)
    : QTabWidget(parent)

{
    addTab(m_textOutputWidget = new TextOutputWidget(this), QIcon::fromTheme(QStringLiteral("view-list-text")), i18nc("@title:window", "SQL Text Output"));
    addTab(m_dataOutputWidget = new DataOutputWidget(this), QIcon::fromTheme(QStringLiteral("view-form-table")), i18nc("@title:window", "SQL Data Output"));
}

KateSQLOutputWidget::~KateSQLOutputWidget()
{
}

// kate: space-indent on; indent-width 2; replace-tabs on;
