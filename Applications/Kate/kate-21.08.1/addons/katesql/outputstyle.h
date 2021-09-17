/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef OUTPUTSTYLE_H
#define OUTPUTSTYLE_H

#include <QBrush>
#include <QFont>
#include <QMetaType>
#include <QString>

struct OutputStyle {
    QFont font;
    QBrush background;
    QBrush foreground;
};

// Q_DECLARE_METATYPE(OutputStyle)

#endif // OUTPUTSTYLE_H
