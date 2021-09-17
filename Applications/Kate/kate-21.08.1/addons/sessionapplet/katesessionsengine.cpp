/********************************************************************
This file is part of the KDE project.

SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
based on clipboard engine:
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
#include "katesessionsengine.h"
#include "katesessionsmodel.h"
#include "katesessionsservice.h"
#include <QDebug>

static const QString s_sessionsSourceName = QStringLiteral("katesessions");

KateSessionsEngine::KateSessionsEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
{
    setData(s_sessionsSourceName, QStringLiteral("test_data"), QLatin1String("This is just for testing"));
    setModel(s_sessionsSourceName, new KateSessionsModel(this));
}

KateSessionsEngine::~KateSessionsEngine()
{
}

Plasma::Service *KateSessionsEngine::serviceForSource(const QString &source)
{
    qDebug() << "Creating KateSessionService";
    Plasma::Service *service = new KateSessionsService(this, source);
    service->setParent(this);
    return service;
}

K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(org.kde.plasma.katesessions, KateSessionsEngine, "plasma-dataengine-katesessions.json")

#include "katesessionsengine.moc"
