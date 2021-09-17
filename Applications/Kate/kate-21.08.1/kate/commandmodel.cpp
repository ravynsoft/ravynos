/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "commandmodel.h"

#include <KLocalizedString>
#include <QAction>
#include <QDebug>

CommandModel::CommandModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void CommandModel::refresh(QVector<QPair<QString, QAction *>> actionList)
{
    QVector<Item> temp;
    temp.reserve(actionList.size());
    for (auto action : actionList) {
        if (!action.second) {
            continue;
        }
        temp.push_back({action.first, action.second, -1});
    }

    int score = 0;
    for (const auto &actionName : m_lastTriggered) {
        for (auto &item : temp) {
            if (actionName == item.action->text()) {
                item.score = score++;
                break;
            }
        }
    }

    beginResetModel();
    m_rows = std::move(temp);
    endResetModel();
}

QVariant CommandModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    };

    auto entry = m_rows[index.row()];
    int col = index.column();

    switch (role) {
    case Qt::DisplayRole:
        if (col == 0) {
            return QString(entry.component + QStringLiteral(": ") + KLocalizedString::removeAcceleratorMarker(entry.action->text()));
        } else {
            return entry.action->shortcut().toString();
        }
    case Qt::DecorationRole:
        if (col == 0) {
            return entry.action->icon();
        }
        break;
    case Qt::TextAlignmentRole:
        if (col == 0) {
            return Qt::AlignLeft;
        } else {
            return Qt::AlignRight;
        }
    case Qt::UserRole: {
        QVariant v;
        v.setValue(entry.action);
        return v;
    }
    case Role::Score:
        return entry.score;
    }

    return {};
}

void CommandModel::actionTriggered(const QString &name)
{
    if (m_lastTriggered.size() == 6) {
        m_lastTriggered.pop_front();
    }
    m_lastTriggered.push_back(name);
}

QVector<QString> CommandModel::lastUsedActions()
{
    return m_lastTriggered;
}

void CommandModel::setLastUsedActions(const QVector<QString> &actionNames)
{
    m_lastTriggered = actionNames;
}
