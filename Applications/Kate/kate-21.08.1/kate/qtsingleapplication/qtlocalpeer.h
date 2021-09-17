/*
 * SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-only WITH Qt-GPL-exception-1.0
 */

#pragma once

#include <qtlockedfile.h>

#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>

namespace SharedTools
{
class QtLocalPeer : public QObject
{
    Q_OBJECT

public:
    explicit QtLocalPeer(QObject *parent = 0, const QString &appId = QString());
    bool isClient();
    bool sendMessage(const QString &message, int timeout, bool block);
    QString applicationId() const
    {
        return id;
    }
    static QString appSessionId(const QString &appId);

Q_SIGNALS:
    void messageReceived(const QString &message, QObject *socket);

protected Q_SLOTS:
    void receiveConnection();

protected:
    QString id;
    QString socketName;
    QLocalServer *server;
    QtLockedFile lockFile;
};

} // namespace SharedTools
