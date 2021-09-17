/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_TOOLRUNNER_TEST_H
#define KATE_TOOLRUNNER_TEST_H

#include <QObject>

class ExternalToolTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void testLoadSave();
    void testRunListDirectory();
    void testRunTac();
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
