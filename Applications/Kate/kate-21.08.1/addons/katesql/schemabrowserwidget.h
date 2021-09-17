/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef SCHEMABROWSERWIDGET_H
#define SCHEMABROWSERWIDGET_H

class SQLManager;
class SchemaWidget;

#include <QWidget>

class SchemaBrowserWidget : public QWidget
{
    Q_OBJECT

public:
    SchemaBrowserWidget(QWidget *parent, SQLManager *manager);
    ~SchemaBrowserWidget() override;

    SchemaWidget *schemaWidget() const;

private:
    SchemaWidget *m_schemaWidget;
};

#endif // SCHEMABROWSERWIDGET_H
