/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QMetaType>
#include <QString>

struct Connection {
    enum Status { UNKNOWN = 0, ONLINE = 1, OFFLINE = 2, REQUIRE_PASSWORD = 3 };

    QString name;
    QString driver;
    QString hostname;
    QString username;
    QString password;
    QString database;
    QString options;
    int port;
    Status status;
};

Q_DECLARE_METATYPE(Connection)

#endif // CONNECTION_H
