/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "schemabrowserwidget.h"
#include "schemawidget.h"

#include <QTreeView>
#include <QVBoxLayout>

SchemaBrowserWidget::SchemaBrowserWidget(QWidget *parent, SQLManager *manager)
    : QWidget(parent)
    , m_schemaWidget(new SchemaWidget(this, manager))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_schemaWidget);
    setLayout(layout);
}

SchemaBrowserWidget::~SchemaBrowserWidget()
{
}

SchemaWidget *SchemaBrowserWidget::schemaWidget() const
{
    return m_schemaWidget;
}
