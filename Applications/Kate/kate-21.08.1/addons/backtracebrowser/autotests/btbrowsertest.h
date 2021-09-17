/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2014 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef BT_BROWSER_TEST_H
#define BT_BROWSER_TEST_H

#include <QObject>

class KateBtBrowserTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void testParser();
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
