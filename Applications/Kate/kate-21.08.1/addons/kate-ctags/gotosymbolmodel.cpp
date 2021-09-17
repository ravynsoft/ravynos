/*
    SPDX-FileCopyrightText: 2020 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "gotosymbolmodel.h"

#include <KLocalizedString>
#include <QDebug>
#include <QProcess>

GotoSymbolModel::GotoSymbolModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int GotoSymbolModel::columnCount(const QModelIndex &) const
{
    return 1;
}

int GotoSymbolModel::rowCount(const QModelIndex &) const
{
    return m_rows.count();
}

QVariant GotoSymbolModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto &row = m_rows.at(index.row());
    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return row.name;
        }
    } else if (role == Qt::DecorationRole) {
        if (index.column() == 0) {
            return row.icon;
        }
    } else if (role == Qt::UserRole) {
        return row.line;
    }

    return QVariant();
}

void GotoSymbolModel::refresh(const QString &filePath)
{
    static const QIcon nsIcon = QIcon::fromTheme(QStringLiteral("code-block"));
    static const QIcon classIcon = QIcon::fromTheme(QStringLiteral("code-class"));
    static const QIcon funcIcon = QIcon::fromTheme(QStringLiteral("code-function"));
    static const QIcon varIcon = QIcon::fromTheme(QStringLiteral("code-variable"));
    static const QIcon defIcon = nsIcon;

    beginResetModel();
    m_rows.clear();
    endResetModel();

    QProcess p;
    p.start(QStringLiteral("ctags"), {QStringLiteral("-x"), QStringLiteral("--_xformat=%{name}%{signature}\t%{kind}\t%{line}"), filePath});

    QByteArray out;
    if (p.waitForFinished()) {
        out = p.readAllStandardOutput();
    } else {
        qWarning() << "Ctags failed";
        beginResetModel();
        m_rows.append(SymbolItem{i18n("CTags executable not found."), -1, QIcon()});
        endResetModel();
        return;
    }

    QVector<SymbolItem> symItems;
    const auto tags = out.split('\n');
    symItems.reserve(tags.size());
    for (const auto &tag : tags) {
        const auto items = tag.split('\t');
        if (items.isEmpty() || items.count() < 3) {
            continue;
        }

        SymbolItem item;
        item.name = QLatin1String(items.at(0));
        // this happens in markdown names for some reason
        if (item.name.endsWith(QLatin1Char('-'))) {
            item.name.chop(1);
        }

        switch (items.at(1).at(0)) {
        case 'f':
            item.icon = funcIcon;
            break;
        case 'm':
            if (items.at(1) == "method") {
                item.icon = funcIcon;
            } else {
                item.icon = defIcon;
            }
            break;
        case 'g':
            if (items.at(1) == "getter") {
                item.icon = funcIcon;
            } else {
                item.icon = defIcon;
            }
            break;
        case 'c':
        case 's':
            if (items.at(1) == "class" || items.at(1) == "struct") {
                item.icon = classIcon;
            } else {
                item.icon = defIcon;
            }
            break;
        case 'n':
            if (items.at(1) == "namespace") {
                item.icon = nsIcon;
            }
            break;
        case 'v':
            item.icon = varIcon;
            break;
        default:
            item.icon = defIcon;
            break;
        }

        item.line = items.at(2).toInt();
        symItems.append(item);
    }

    beginResetModel();
    if (!symItems.isEmpty()) {
        m_rows = std::move(symItems);
    } else {
        m_rows.append(SymbolItem{i18n("CTags was unable to parse this file."), -1, QIcon()});
    }
    endResetModel();
}
