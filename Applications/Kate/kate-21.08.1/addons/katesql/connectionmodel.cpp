/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "connectionmodel.h"

#include <QDebug>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QSize>
#include <QStringList>
#include <QVariant>

ConnectionModel::ConnectionModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_icons[Connection::UNKNOWN] = QIcon::fromTheme(QStringLiteral("user-offline"));
    m_icons[Connection::ONLINE] = QIcon::fromTheme(QStringLiteral("user-online"));
    m_icons[Connection::OFFLINE] = QIcon::fromTheme(QStringLiteral("user-offline"));
    m_icons[Connection::REQUIRE_PASSWORD] = QIcon::fromTheme(QStringLiteral("user-invisible"));
}

ConnectionModel::~ConnectionModel()
{
}

int ConnectionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_connections.count();
}

QVariant ConnectionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const QString key = m_connections.keys().at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return QVariant(m_connections.value(key).name);

    case Qt::UserRole:
        return QVariant::fromValue<Connection>(m_connections.value(key));

    case Qt::DecorationRole:
        return m_icons[m_connections.value(key).status];

    case Qt::SizeHintRole: {
        const QFontMetrics metrics(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
        return QSize(metrics.boundingRect(m_connections.value(key).name).width(), 22);
    }

    default:
        return QVariant();
    }

    return QVariant();
}

int ConnectionModel::addConnection(const Connection &conn)
{
    /// FIXME
    if (m_connections.contains(conn.name)) {
        qDebug() << "a connection named" << conn.name << "already exists!";
        return -1;
    }

    int pos = m_connections.count();

    beginInsertRows(QModelIndex(), pos, pos);

    m_connections[conn.name] = conn;

    endInsertRows();

    return m_connections.keys().indexOf(conn.name);
}

void ConnectionModel::removeConnection(const QString &name)
{
    int pos = m_connections.keys().indexOf(name);

    beginRemoveRows(QModelIndex(), pos, pos);

    m_connections.remove(name);

    endRemoveRows();
}

int ConnectionModel::indexOf(const QString &name)
{
    return m_connections.keys().indexOf(name);
}

Connection::Status ConnectionModel::status(const QString &name) const
{
    if (!m_connections.contains(name)) {
        return Connection::UNKNOWN;
    }

    return m_connections[name].status;
}

void ConnectionModel::setStatus(const QString &name, const Connection::Status status)
{
    if (!m_connections.contains(name)) {
        return;
    }

    m_connections[name].status = status;

    const int i = indexOf(name);

    Q_EMIT dataChanged(index(i), index(i));
}

void ConnectionModel::setPassword(const QString &name, const QString &password)
{
    if (!m_connections.contains(name)) {
        return;
    }

    m_connections[name].password = password;

    const int i = indexOf(name);

    Q_EMIT dataChanged(index(i), index(i));
}
