/*
    SPDX-FileCopyrightText: 2020 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef GOTOSYMBOLMODEL_H
#define GOTOSYMBOLMODEL_H

#include <QAbstractTableModel>
#include <QIcon>
#include <QString>
#include <QVector>

struct SymbolItem {
    QString name;
    int line;
    QIcon icon;
};

class GotoSymbolModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GotoSymbolModel(QObject *parent = nullptr);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void refresh(const QString &filePath);

private:
    QVector<SymbolItem> m_rows;
};

#endif // GOTOSYMBOLMODEL_H
