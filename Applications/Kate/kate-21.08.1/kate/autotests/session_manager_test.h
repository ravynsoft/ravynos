/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2019 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SESSION_MANAGER_TEST_H
#define KATE_SESSION_MANAGER_TEST_H

#include <QObject>

class KateSessionManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void basic();
    void activateNewNamedSession();
    void anonymousSessionFile();
    void urlizeSessionFile();
    void renameSession();
    void deleteActiveSession();
    void deleteSession();
    void saveActiveSessionWithAnynomous();

    void deletingSessionFilesUnderRunningApp();
    void startNonEmpty();

private:
    class QTemporaryDir *m_tempdir;
    class KateSessionManager *m_manager;
    class KateApp *m_app; // dependency, sigh...
};

#endif
