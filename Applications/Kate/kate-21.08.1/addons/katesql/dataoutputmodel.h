/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef DATAOUTPUTMODEL_H
#define DATAOUTPUTMODEL_H

struct OutputStyle;

#include "cachedsqlquerymodel.h"

#include <QColor>
#include <QFont>

/// provide colors and styles
class DataOutputModel : public CachedSqlQueryModel
{
    Q_OBJECT

public:
    DataOutputModel(QObject *parent = nullptr);
    ~DataOutputModel() override;

    bool useSystemLocale() const;
    void setUseSystemLocale(bool useSystemLocale);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void clear() override;
    void readConfig();

private:
    QHash<QString, OutputStyle *> m_styles;
    bool m_useSystemLocale;
};

#endif // DATAOUTPUTMODEL_H
