/***************************************************************************
 *   This file is part of Kate build plugin                                *
 *   SPDX-FileCopyrightText: 2014 Kåre Särs <kare.sars@iki.fi>                           *
 *                                                                         *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 ***************************************************************************/

#include "TargetModel.h"
#include <KLocalizedString>
#include <QDebug>
#include <QTimer>

TargetModel::TargetSet::TargetSet(const QString &_name, const QString &_dir)
    : name(_name)
    , workDir(_dir)
{
}

// Model data
// parent is the m_targets list index or InvalidIndex if it is a root element
// row is the m_targets.commands list index or m_targets index if it is a root element
// column 0 is command name or target-set name if it is a root element
// column 1 is the command or working directory if it is a root element

TargetModel::TargetModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}
TargetModel::~TargetModel()
{
}

void TargetModel::clear()
{
    m_targets.clear();
}

void TargetModel::setDefaultCmd(int rootRow, const QString &defCmd)
{
    if (rootRow < 0 || rootRow >= m_targets.size()) {
        qDebug() << "rootRow not valid";
        return;
    }

    for (int i = 0; i < m_targets[rootRow].commands.size(); i++) {
        if (defCmd == m_targets[rootRow].commands[i].first) {
            m_targets[rootRow].defaultCmd = defCmd;
            return;
        }
    }
}

int TargetModel::getDefaultCmdIndex(int rootRow) const
{
    if (rootRow < 0 || rootRow >= m_targets.size()) {
        qDebug() << "rootRow not valid";
        return 0;
    }

    auto defCmd = m_targets[rootRow].defaultCmd;
    for (int i = 0; i < m_targets[rootRow].commands.size(); i++) {
        if (defCmd == m_targets[rootRow].commands[i].first) {
            return i;
        }
    }

    return 0;
}

int TargetModel::addTargetSet(const QString &setName, const QString &workDir)
{
    // make the name unique
    QString newName = setName;
    for (int i = 0; i < m_targets.count(); i++) {
        if (m_targets[i].name == newName) {
            newName += QStringLiteral(" 2");
            i = -1;
        }
    }

    beginInsertRows(QModelIndex(), m_targets.count(), m_targets.count());
    TargetModel::TargetSet targetSet(newName, workDir);
    m_targets << targetSet;
    endInsertRows();
    return m_targets.count() - 1;
}

QModelIndex TargetModel::addCommand(int rootRow, const QString &cmdName, const QString &command)
{
    if (rootRow < 0 || rootRow >= m_targets.size()) {
        qDebug() << "rootRow not valid";
        return QModelIndex();
    }

    // make the name unique
    QString newName = cmdName;
    for (int i = 0; i < m_targets[rootRow].commands.count(); i++) {
        if (m_targets[rootRow].commands[i].first == newName) {
            newName += QStringLiteral(" 2");
            i = -1;
        }
    }

    QModelIndex rootIndex = createIndex(rootRow, 0, InvalidIndex);
    beginInsertRows(rootIndex, m_targets[rootRow].commands.count(), m_targets[rootRow].commands.count());
    m_targets[rootRow].commands << QPair<QString, QString>(newName, command);
    endInsertRows();
    return createIndex(m_targets[rootRow].commands.size() - 1, 0, rootRow);
}

QModelIndex TargetModel::copyTargetOrSet(const QModelIndex &index)
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    quint32 rootRow = index.internalId();
    if (rootRow == InvalidIndex) {
        rootRow = index.row();
        if (m_targets.count() <= static_cast<int>(rootRow)) {
            return QModelIndex();
        }

        beginInsertRows(QModelIndex(), m_targets.count(), m_targets.count());

        QString newName = m_targets[rootRow].name + QStringLiteral(" 2");
        for (int i = 0; i < m_targets.count(); i++) {
            if (m_targets[i].name == newName) {
                newName += QStringLiteral(" 2");
                i = -1;
            }
        }
        m_targets << m_targets[rootRow];
        m_targets.last().name = newName;
        endInsertRows();

        return createIndex(m_targets.count() - 1, 0, InvalidIndex);
        ;
    }

    if (m_targets.count() <= static_cast<int>(rootRow)) {
        return QModelIndex();
    }
    if (index.row() < 0) {
        return QModelIndex();
    }
    if (index.row() >= m_targets[rootRow].commands.count()) {
        return QModelIndex();
    }

    QModelIndex rootIndex = createIndex(rootRow, 0, InvalidIndex);
    beginInsertRows(rootIndex, m_targets[rootRow].commands.count(), m_targets[rootRow].commands.count());

    QString newName = m_targets[rootRow].commands[index.row()].first + QStringLiteral(" 2");
    for (int i = 0; i < m_targets[rootRow].commands.count(); i++) {
        if (m_targets[rootRow].commands[i].first == newName) {
            newName += QStringLiteral(" 2");
            i = -1;
        }
    }
    m_targets[rootRow].commands << QPair<QString, QString>(newName, m_targets[rootRow].commands[index.row()].second);

    endInsertRows();
    return createIndex(m_targets[rootRow].commands.count() - 1, 0, rootRow);
}

QModelIndex TargetModel::defaultTarget(int targetSet)
{
    return createIndex(getDefaultCmdIndex(targetSet), 0, targetSet);
}

void TargetModel::deleteItem(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    if (index.internalId() == InvalidIndex) {
        beginRemoveRows(index.parent(), index.row(), index.row());
        m_targets.removeAt(index.row());
        endRemoveRows();
    } else if (index.internalId() < static_cast<quint64>(m_targets.size()) && m_targets[static_cast<int>(index.internalId())].commands.count() > index.row()) {
        beginRemoveRows(index.parent(), index.row(), index.row());
        m_targets[static_cast<int>(index.internalId())].commands.removeAt(index.row());
        endRemoveRows();
    }
}

void TargetModel::deleteTargetSet(const QString &targetSet)
{
    for (int i = 0; i < m_targets.count(); i++) {
        if (m_targets[i].name == targetSet) {
            beginRemoveRows(QModelIndex(), i, i);
            m_targets.removeAt(i);
            endRemoveRows();
            return;
        }
    }
}

const QString TargetModel::command(const QModelIndex &itemIndex) const
{
    if (!itemIndex.isValid()) {
        return QString();
    }
    quint32 rRow = itemIndex.internalId();
    int cRow = itemIndex.row();
    if (rRow == TargetModel::InvalidIndex) {
        rRow = cRow;
        cRow = 0;
    }

    if (static_cast<int>(rRow) >= m_targets.count()) {
        return QString();
    }

    if (cRow < 0 || cRow >= m_targets[static_cast<int>(rRow)].commands.count()) {
        return QString();
    }

    return m_targets[rRow].commands[cRow].second;
}

const QString TargetModel::cmdName(const QModelIndex &itemIndex) const
{
    if (!itemIndex.isValid()) {
        return QString();
    }
    quint32 rRow = itemIndex.internalId();
    int cRow = itemIndex.row();
    if (rRow == TargetModel::InvalidIndex) {
        rRow = cRow;
        cRow = 0;
    }

    if (static_cast<int>(rRow) >= m_targets.count()) {
        return QString();
    }

    if (cRow < 0 || cRow >= m_targets[static_cast<int>(rRow)].commands.count()) {
        return QString();
    }

    return m_targets[static_cast<int>(rRow)].commands[cRow].first;
}

const QString TargetModel::workDir(const QModelIndex &itemIndex) const
{
    if (!itemIndex.isValid()) {
        return QString();
    }
    quint32 rRow = itemIndex.internalId();
    int cRow = itemIndex.row();
    if (rRow == TargetModel::InvalidIndex) {
        rRow = cRow;
        cRow = 0;
    }

    if (static_cast<int>(rRow) >= m_targets.count()) {
        return QString();
    }

    return m_targets[static_cast<int>(rRow)].workDir;
}

const QString TargetModel::targetName(const QModelIndex &itemIndex) const
{
    if (!itemIndex.isValid()) {
        return QString();
    }
    quint32 rRow = itemIndex.internalId();
    int cRow = itemIndex.row();
    if (rRow == TargetModel::InvalidIndex) {
        rRow = cRow;
        cRow = 0;
    }

    if (static_cast<int>(rRow) >= m_targets.count()) {
        return QString();
    }

    return m_targets[static_cast<int>(rRow)].name;
}

QVariant TargetModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.column() < 0 || index.column() > 1) {
        return QVariant();
    }
    // Tooltip
    if (role == Qt::ToolTipRole) {
        if (index.column() == 0 && index.parent().isValid()) {
            return i18n("Check the check-box to make the command the default for the target-set.");
        }
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::CheckStateRole) {
        return QVariant();
    }

    int row = index.row();

    if (index.internalId() == InvalidIndex) {
        if (row < 0 || row >= m_targets.size() || role == Qt::CheckStateRole) {
            return QVariant();
        }
        switch (index.column()) {
        case 0:
            return m_targets[row].name;
        case 1:
            return m_targets[row].workDir;
        }
    } else {
        int rootIndex = index.internalId();
        if (rootIndex < 0 || rootIndex >= m_targets.size()) {
            return QVariant();
        }
        if (row < 0 || row >= m_targets[rootIndex].commands.size()) {
            return QVariant();
        }

        if (role == Qt::CheckStateRole) {
            if (index.column() != 0) {
                return QVariant();
            }
            return m_targets[rootIndex].commands[row].first == m_targets[rootIndex].defaultCmd ? Qt::Checked : Qt::Unchecked;
        } else {
            switch (index.column()) {
            case 0:
                return m_targets[rootIndex].commands[row].first;
            case 1:
                return m_targets[rootIndex].commands[row].second;
            }
        }
    }

    return QVariant();
}

QVariant TargetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (section == 0) {
        return i18n("Command/Target-set Name");
    }
    if (section == 1) {
        return i18n("Working Directory / Command");
    }
    return QVariant();
}

bool TargetModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // FIXME
    if (role != Qt::EditRole && role != Qt::CheckStateRole) {
        return false;
    }
    if (!index.isValid()) {
        return false;
    }
    if (index.column() < 0 || index.column() > 1) {
        return false;
    }
    int row = index.row();

    if (index.internalId() == InvalidIndex) {
        if (row < 0 || row >= m_targets.size()) {
            return false;
        }
        switch (index.column()) {
        case 0:
            m_targets[row].name = value.toString();
            return true;
        case 1:
            m_targets[row].workDir = value.toString();
            return true;
        }
    } else {
        int rootIndex = index.internalId();
        if (rootIndex < 0 || rootIndex >= m_targets.size()) {
            return false;
        }
        if (row < 0 || row >= m_targets[rootIndex].commands.size()) {
            return false;
        }

        if (role == Qt::CheckStateRole) {
            if (index.column() == 0) {
                m_targets[rootIndex].defaultCmd = m_targets[rootIndex].commands[row].first;
                Q_EMIT dataChanged(createIndex(0, 0, rootIndex), createIndex(m_targets[rootIndex].commands.size() - 1, 0, rootIndex));
            }
        } else {
            switch (index.column()) {
            case 0:
                m_targets[rootIndex].commands[row].first = value.toString();
                return true;
            case 1:
                m_targets[rootIndex].commands[row].second = value.toString();
                return true;
            }
        }
    }
    return false;
}

Qt::ItemFlags TargetModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    if (index.internalId() != InvalidIndex && index.column() == 0) {
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    }

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int TargetModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_targets.size();
    }

    if (parent.internalId() != InvalidIndex) {
        return 0;
    }

    int row = parent.row();
    if (row < 0 || row >= m_targets.size()) {
        return 0;
    }

    return m_targets[row].commands.size();
}

int TargetModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QModelIndex TargetModel::index(int row, int column, const QModelIndex &parent) const
{
    quint32 rootIndex = InvalidIndex;
    if (parent.isValid()) {
        if (parent.internalId() == InvalidIndex) {
            rootIndex = parent.row();
        }
    }
    return createIndex(row, column, rootIndex);
}

QModelIndex TargetModel::parent(const QModelIndex &child) const
{
    if (child.internalId() == InvalidIndex) {
        return QModelIndex();
    }
    return createIndex(child.internalId(), 0, InvalidIndex);
}
