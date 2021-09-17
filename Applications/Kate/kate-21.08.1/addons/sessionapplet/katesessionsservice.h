/********************************************************************
This file is part of the KDE project.

SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
based on clipboard engine:
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
#ifndef KATESESSIONSSERVICE_H
#define KATESESSIONSSERVICE_H

#include <Plasma/Service>

class KateSessionsEngine;

class KateSessionsService : public Plasma::Service
{
    Q_OBJECT
public:
    KateSessionsService(KateSessionsEngine *engine, const QString &uuid);
    ~KateSessionsService() override = default;

protected:
    Plasma::ServiceJob *createJob(const QString &operation, QVariantMap &parameters) override;

private:
    KateSessionsEngine *m_engine;
    QString m_uuid;
};

#endif
