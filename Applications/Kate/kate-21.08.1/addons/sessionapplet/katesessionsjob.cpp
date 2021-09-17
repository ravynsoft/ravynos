/********************************************************************
This file is part of the KDE project.

SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
based on clipboard engine:
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
partly based on code:
SPDX-FileCopyrightText: 2008 Montel Laurent <montel@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
#include "katesessionsjob.h"
#include "katesessionsengine.h"
#include <KIO/CommandLauncherJob>
#include <QDebug>
#include <QUrl>

static const QString s_sessionsSourceName = QStringLiteral("katesessions");

KateSessionsJob::KateSessionsJob(KateSessionsEngine *engine,
                                 const QString &destination,
                                 const QString &operation,
                                 const QVariantMap &parameters,
                                 QObject *parent)
    : Plasma::ServiceJob(destination, operation, parameters, parent)
    , m_engine(engine)
{
}

void KateSessionsJob::start()
{
    qDebug() << "Job started:" << operationName() << " " << destination();
    const QString operation = operationName();
    // first check for operations not needing an item
    if (operation == QLatin1String("newSession")) {
        QString sessionName = parameters().value(QStringLiteral("sessionName")).toString();
        if (sessionName.isEmpty()) {
            setResult(false);
            emitResult();
            return;
        }
        // CHECK IF SESSION EXISTS
        QStringList args;
        args << QStringLiteral("-n") << QStringLiteral("--start") << sessionName;
        auto job = new KIO::CommandLauncherJob(QStringLiteral("kate"), args);
        job->start();
        setResult(true);
        emitResult();
        return;
    } else if (operation == QLatin1String("invoke")) {
        QString dest = destination();
        QStringList args;
        if (dest == QLatin1String("_kate_noargs")) {
            // do nothing
        } else if (dest == QLatin1String("_kate_newsession")) {
            args << QStringLiteral("--startanon");
        } else {
            dest.chop(12); // .katesession
            args << QStringLiteral("-n") << QStringLiteral("--start") << QUrl::fromPercentEncoding(dest.toLatin1());
            // args <<"-n"<< "--start"<<m_sessions[ id-3 ];
        }

        auto job = new KIO::CommandLauncherJob(QStringLiteral("kate"), args);
        job->start();

        setResult(true);
        emitResult();
        return;
    } else if (operation == QLatin1String("remove")) {
        qDebug() << operation << destination();
        setResult(true);
        emitResult();
        return;
    }
    setResult(false);
    emitResult();
}
