/********************************************************************
This file is part of the KDE project.

SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
based on clipboard engine:
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
#ifndef KATESESSIONSJOB_H
#define KATESESSIONSJOB_H

#include <Plasma/ServiceJob>

class KateSessionsEngine;

class KateSessionsJob : public Plasma::ServiceJob
{
    Q_OBJECT
public:
    KateSessionsJob(KateSessionsEngine *engine, const QString &destination, const QString &operation, const QVariantMap &parameters, QObject *parent = nullptr);
    ~KateSessionsJob() override = default;

    void start() override;

private:
    KateSessionsEngine *m_engine;
};

#endif
