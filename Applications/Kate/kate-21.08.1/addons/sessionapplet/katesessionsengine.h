/********************************************************************
This file is part of the KDE project.

SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
based on clipboard engine:
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
#ifndef KATESESSIONS_ENGINE_H
#define KATESESSIONS_ENGINE_H

#include <Plasma/DataEngine>

class KateSessionsEngine : public Plasma::DataEngine
{
    Q_OBJECT
public:
    KateSessionsEngine(QObject *parent, const QVariantList &args);
    ~KateSessionsEngine() override;

    Plasma::Service *serviceForSource(const QString &source) override;
};

#endif
