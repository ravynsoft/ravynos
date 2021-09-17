/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2018 Gregor Mi <codestruct@posteo.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QObject>

class KateTabSwitcherTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void testLongestCommonPrefix();
    void testLongestCommonPrefix_data();
};
