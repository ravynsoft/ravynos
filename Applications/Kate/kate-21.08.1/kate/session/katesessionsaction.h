/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __KATE_SESSIONS_ACTION_H__
#define __KATE_SESSIONS_ACTION_H__

#include "katetests_export.h"
#include <KActionMenu>

class KateSessionManager;

class KATE_TESTS_EXPORT KateSessionsAction : public KActionMenu
{
    Q_OBJECT

public:
    KateSessionsAction(const QString &text, QObject *parent, KateSessionManager *manager = nullptr);

public Q_SLOTS:
    void slotAboutToShow();
    void openSession(QAction *action);
    void slotSessionChanged();

private:
    friend class KateSessionsActionTest; // tfuj
    QActionGroup *sessionsGroup;
    KateSessionManager *m_manager;
};

#endif
