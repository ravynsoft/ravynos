/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2002, 2003 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __KATE_WAITER_H__
#define __KATE_WAITER_H__

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QObject>

class KateWaiter : public QObject
{
    Q_OBJECT

public:
    KateWaiter(const QString &service, const QStringList &tokens);

public Q_SLOTS:
    void exiting();

    void documentClosed(const QString &token);

    void serviceOwnerChanged(const QString &, const QString &, const QString &);

private:
    QStringList m_tokens;
    QDBusServiceWatcher m_watcher;
};

#endif
