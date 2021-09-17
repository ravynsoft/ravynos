/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2014 Gregor Mi <codestruct@posteo.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "fileutil.h"

// code taken from https://stackoverflow.com/questions/15713529/get-common-parent-of-2-qdir
// note that there is unit test
const QString FileUtil::commonParent(const QString &path1, const QString &path2)
{
    QString ret = path2;

    while (!path1.startsWith(ret)) {
        ret.chop(1);
    }

    if (ret.isEmpty()) {
        return ret;
    }

    while (!ret.endsWith(QLatin1Char('/'))) {
        ret.chop(1);
    }

    return ret;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
