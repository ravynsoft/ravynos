/*
    SPDX-FileCopyrightText: 2020 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef GOTOGLOBALSYMBOLMODEL_H
#define GOTOGLOBALSYMBOLMODEL_H

#include "tags.h"

#include <QAbstractTableModel>
#include <QPair>

class GotoGlobalSymbolModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Roles {
        Name = Qt::UserRole,
        Pattern,
        FileUrl,
    };

    explicit GotoGlobalSymbolModel(QObject *parent = nullptr);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief removes useless symbols like anon namespace etc for better UI
     */
    QString filterName(QString tagName) const;

    void setSymbolsData(Tags::TagList rows)
    {
        beginResetModel();
        m_rows = std::move(rows);
        endResetModel();
    }

private:
    Tags::TagList m_rows;
};

#endif // GOTOGLOBALSYMBOLMODEL_H
