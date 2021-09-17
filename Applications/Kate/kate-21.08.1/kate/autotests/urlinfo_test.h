/*
    SPDX-FileCopyrightText: 2020 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: MIT
*/

#pragma once

#include <QObject>

#include <urlinfo.h>

class UrlInfoTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void someUrls();
    void someCursors();
    void urlWithColonAtStart();
};
