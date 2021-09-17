/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008-2014 Dominik Haumann <dhaumann kde org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef BTPARSER_H
#define BTPARSER_H

#include <QList>
#include <QString>

class BtInfo
{
public:
    enum Type { Source = 0, Lib, Unknown, Invalid };

    /**
     * Default constructor => invalid element
     */
    BtInfo() = default;

public:
    QString original;
    QString filename;
    QString function;
    QString address;
    int step = -1;
    int line = -1;

    Type type = Invalid;
};

namespace KateBtParser
{
QList<BtInfo> parseBacktrace(const QString &bt);

}

#endif // BTPARSER_H

// kate: space-indent on; indent-width 4; replace-tabs on;
