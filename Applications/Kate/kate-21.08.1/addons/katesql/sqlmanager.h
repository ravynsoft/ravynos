/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef SQLMANAGER_H
#define SQLMANAGER_H

class ConnectionModel;
class KConfigGroup;

#include "connection.h"
#include <KWallet>
#include <QSqlQuery>

class SQLManager : public QObject
{
    Q_OBJECT

public:
    SQLManager(QObject *parent = nullptr);
    ~SQLManager() override;

    ConnectionModel *connectionModel();
    void createConnection(const Connection &conn);
    bool testConnection(const Connection &conn, QSqlError &error);
    bool isValidAndOpen(const QString &connection);

    KWallet::Wallet *openWallet();
    int storeCredentials(const Connection &conn);
    int readCredentials(const QString &name, QString &password);

public Q_SLOTS:
    void removeConnection(const QString &name);
    void reopenConnection(const QString &name);
    void loadConnections(KConfigGroup *connectionsGroup);
    void saveConnections(KConfigGroup *connectionsGroup);
    void runQuery(const QString &text, const QString &connection);

protected:
    void saveConnection(KConfigGroup *connectionsGroup, const Connection &conn);

Q_SIGNALS:
    void connectionCreated(const QString &name);
    void connectionRemoved(const QString &name);
    void connectionAboutToBeClosed(const QString &name);

    void queryActivated(QSqlQuery &query, const QString &connection);

    void error(const QString &message);
    void success(const QString &message);

private:
    ConnectionModel *m_model;
    KWallet::Wallet *m_wallet = nullptr;
};

#endif // SQLMANAGER_H
