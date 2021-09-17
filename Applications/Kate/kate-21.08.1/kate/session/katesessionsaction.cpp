/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katesessionsaction.h"

#include "kateapp.h"
#include "katesessionmanager.h"

#include <QMenu>
#include <algorithm>

KateSessionsAction::KateSessionsAction(const QString &text, QObject *parent, KateSessionManager *manager)
    : KActionMenu(text, parent)
{
    m_manager = manager ? manager : KateApp::self()->sessionManager();

    connect(menu(), &QMenu::aboutToShow, this, &KateSessionsAction::slotAboutToShow);

    sessionsGroup = new QActionGroup(menu());

    // reason for Qt::QueuedConnection: when switching session with N mainwindows
    // to e.g. 1 mainwindow, the last N - 1 mainwindows are deleted. Invoking
    // a session switch without queued connection deletes a mainwindow in which
    // the current code path is executed ---> crash. See bug #227008.
    connect(sessionsGroup, &QActionGroup::triggered, this, &KateSessionsAction::openSession, Qt::QueuedConnection);

    connect(m_manager, &KateSessionManager::sessionChanged, this, &KateSessionsAction::slotSessionChanged);

    setDisabled(m_manager->sessionList().empty());
}

void KateSessionsAction::slotAboutToShow()
{
    qDeleteAll(sessionsGroup->actions());

    KateSessionList slist = m_manager->sessionList();
    std::sort(slist.begin(), slist.end(), KateSession::compareByTimeDesc);

    slist = slist.mid(0, 10); // take first 10

    // sort the reduced list alphabetically (#364089)
    std::sort(slist.begin(), slist.end(), KateSession::compareByName);

    for (const KateSession::Ptr &session : qAsConst(slist)) {
        QString sessionName = session->name();
        sessionName.replace(QStringLiteral("&"), QStringLiteral("&&"));
        QAction *action = new QAction(sessionName, sessionsGroup);
        action->setData(QVariant(session->name()));
        action->setCheckable(true);
        action->setChecked(session == m_manager->activeSession());
        menu()->addAction(action);
    }
}

void KateSessionsAction::openSession(QAction *action)
{
    const QString name = action->data().toString();
    m_manager->activateSession(name);
}

void KateSessionsAction::slotSessionChanged()
{
    setDisabled(m_manager->sessionList().empty());
}
