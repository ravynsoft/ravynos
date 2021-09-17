/*
 * This file is part of the Kate project.
 *
 * SPDX-FileCopyrightText: 2021 Héctor Mesa Jiménez <wmj.py@gmx.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QObject>

class JsonUtilsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMerge();
};

// kate: space-indent on; indent-width 4; replace-tabs on;
