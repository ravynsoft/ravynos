/*
    SPDX-FileCopyrightText: 2020 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "gotoglobalsymbolmodel.h"

#include <QFileInfo>
#include <QIcon>

GotoGlobalSymbolModel::GotoGlobalSymbolModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int GotoGlobalSymbolModel::columnCount(const QModelIndex &) const
{
    return 1;
}

int GotoGlobalSymbolModel::rowCount(const QModelIndex &) const
{
    return m_rows.size();
}

QString GotoGlobalSymbolModel::filterName(QString tagName) const
{
    // remove anon namespace
    int __anonIdx = tagName.indexOf(QStringLiteral("__anon"));
    if (__anonIdx != -1) {
        int scopeOpIdx = tagName.indexOf(QStringLiteral("::"), __anonIdx) + 2;
        tagName.remove(__anonIdx, scopeOpIdx - __anonIdx);
    }
    return tagName;
}

QVariant GotoGlobalSymbolModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    static const QIcon defIcon = QIcon::fromTheme(QStringLiteral("code-block"));
    static const QIcon funcIcon = QIcon::fromTheme(QStringLiteral("code-function"));
    static const QIcon varIcon = QIcon::fromTheme(QStringLiteral("code-variable"));

    const Tags::TagEntry &row = m_rows.at(index.row());
    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return filterName(row.tag);
        }
    } else if (role == Qt::UserRole) {
        return row.tag;
    } else if (role == Qt::DecorationRole) {
        if (row.type == QLatin1String("function") || row.type == QLatin1String("member")) {
            return funcIcon;
        } else if (row.type.startsWith(QLatin1String("var"))) {
            return varIcon;
        } else {
            return defIcon;
        }
    } else if (role == Pattern) {
        return row.pattern;
    } else if (role == FileUrl) {
        return row.file;
    }

    return QVariant();
}
