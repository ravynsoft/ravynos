/********************************************************************
This file is part of the KDE project.

SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
based on clipboard engine:
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
#include "katesessionsservice.h"
#include "katesessionsengine.h"
#include "katesessionsjob.h"
#include <QDebug>

KateSessionsService::KateSessionsService(KateSessionsEngine *engine, const QString &uuid)
    : m_engine(engine)
    , m_uuid(uuid)
{
    setName(QStringLiteral("org.kde.plasma.katesessions"));
}

Plasma::ServiceJob *KateSessionsService::createJob(const QString &operation, QVariantMap &parameters)
{
    qDebug() << "creating KateSessionsJob";
    return new KateSessionsJob(m_engine, m_uuid, operation, parameters, this);
}
