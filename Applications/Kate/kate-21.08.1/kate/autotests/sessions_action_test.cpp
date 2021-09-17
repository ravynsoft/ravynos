/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2019 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sessions_action_test.h"
#include "kateapp.h"
#include "katesessionmanager.h"
#include "katesessionsaction.h"

#include <QActionGroup>
#include <QCommandLineParser>
#include <QTemporaryDir>
#include <QtTestWidgets>

QTEST_MAIN(KateSessionsActionTest)

void KateSessionsActionTest::initTestCase()
{
    /**
     * init resources from our static lib
     */
    Q_INIT_RESOURCE(kate);

    // we need an application object, as session loading will trigger modifications to that
    m_app = new KateApp(QCommandLineParser());
    m_app->sessionManager()->activateAnonymousSession();
}

void KateSessionsActionTest::cleanupTestCase()
{
    delete m_app;
}

void KateSessionsActionTest::init()
{
    m_tempdir = new QTemporaryDir;
    QVERIFY(m_tempdir->isValid());

    m_manager = new KateSessionManager(this, m_tempdir->path());
    m_ac = new KateSessionsAction(QStringLiteral("menu"), this, m_manager);
}

void KateSessionsActionTest::cleanup()
{
    delete m_ac;
    delete m_manager;
    delete m_tempdir;
}
void KateSessionsActionTest::basic()
{
    m_ac->slotAboutToShow();
    QCOMPARE(m_ac->isEnabled(), false);

    QList<QAction *> actions = m_ac->sessionsGroup->actions();
    QCOMPARE(actions.size(), 0);
}

void KateSessionsActionTest::limit()
{
    for (int i = 0; i < 14; i++) {
        m_manager->activateSession(QStringLiteral("session %1").arg(i));
    }

    QCOMPARE(m_manager->activeSession()->isAnonymous(), false);
    QCOMPARE(m_manager->sessionList().size(), 14);
    QCOMPARE(m_ac->isEnabled(), true);

    m_ac->slotAboutToShow();

    QList<QAction *> actions = m_ac->sessionsGroup->actions();
    QCOMPARE(actions.size(), 10);
}

void KateSessionsActionTest::timesorted()
{
    /*
    m_manager->activateSession("one", false, false);
    m_manager->saveActiveSession();
    m_manager->activateSession("two", false, false);
    m_manager->saveActiveSession();
    m_manager->activateSession("three", false, false);
    m_manager->saveActiveSession();

    KateSessionList list = m_manager->sessionList();
    QCOMPARE(list.size(), 3);

    TODO: any idea how to test this simply and not calling utime()?
    */
}
