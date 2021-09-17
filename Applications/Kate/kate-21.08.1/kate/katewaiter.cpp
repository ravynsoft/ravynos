/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2002, 2003 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katewaiter.h"

KateWaiter::KateWaiter(const QString &service, const QStringList &tokens)
    : QObject(QCoreApplication::instance())
    , m_tokens(tokens)
    , m_watcher(service, QDBusConnection::sessionBus())
{
    connect(&m_watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &KateWaiter::serviceOwnerChanged);
}

void KateWaiter::exiting()
{
    QCoreApplication::instance()->quit();
}

void KateWaiter::documentClosed(const QString &token)
{
    m_tokens.removeAll(token);
    if (m_tokens.count() == 0) {
        QCoreApplication::instance()->quit();
    }
}

void KateWaiter::serviceOwnerChanged(const QString &, const QString &, const QString &)
{
    QCoreApplication::instance()->quit();
}
