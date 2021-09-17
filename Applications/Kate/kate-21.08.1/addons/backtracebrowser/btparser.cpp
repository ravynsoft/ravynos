/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008-2014 Dominik Haumann <dhaumann kde org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "btparser.h"

#include <QDebug>
#include <QRegularExpression>
#include <QStringList>

static QString eolDelimiter(const QString &str)
{
    // find the split character
    QString separator(QLatin1Char('\n'));
    if (str.indexOf(QLatin1String("\r\n")) != -1) {
        separator = QStringLiteral("\r\n");
    } else if (str.indexOf(QLatin1Char('\r')) != -1) {
        separator = QLatin1Char('\r');
    }
    return separator;
}

static bool lineNoLessThan(const QString &lhs, const QString &rhs)
{
    const QRegularExpression rx(QStringLiteral("^#(\\d+)"));
    QRegularExpressionMatch match = rx.match(lhs);
    int ilhs = match.capturedStart(0);
    int lhsLn = match.captured(1).toInt();
    match = rx.match(rhs);
    int irhs = match.capturedStart(0);
    int rhsLn = match.captured(1).toInt();
    if (ilhs != -1 && irhs != -1) {
        return lhsLn < rhsLn;
    } else {
        return lhs < rhs;
    }
}

static QStringList normalizeBt(const QStringList &l)
{
    QStringList normalized;

    bool append = false;

    for (int i = 0; i < l.size(); ++i) {
        QString str = l[i].trimmed();
        if (str.length()) {
            if (str[0] == QLatin1Char('#')) {
                normalized << str;
                append = true;
            } else if (append) {
                normalized.last() += QLatin1Char(' ') + str;
            }
        } else {
            append = false;
        }
    }

    std::sort(normalized.begin(), normalized.end(), lineNoLessThan);

    // now every single line contains a whole backtrace info
    return normalized;
}

static BtInfo parseBtLine(const QString &line)
{
    // the syntax types we support are
    // a) #24 0xb688ff8e in QApplication::notify (this=0xbf997e8c, receiver=0x82607e8, e=0xbf997074) at kernel/qapplication.cpp:3115
    // b) #39 0xb634211c in g_main_context_dispatch () from /usr/lib/libglib-2.0.so.0
    // c) #41 0x0805e690 in ?? ()
    // d) #5  0xffffe410 in __kernel_vsyscall ()

    // try a) cap #number(1), address(2), function(3), filename(4), linenumber(5)
    static const QRegularExpression rxa(QStringLiteral("^#(\\d+)\\s+(0x\\w+)\\s+in\\s+(.+)\\s+at\\s+(.+):(\\d+)$"));
    QRegularExpressionMatch match = rxa.match(line);
    if (match.hasMatch()) {
        BtInfo info;
        info.original = line;
        info.filename = match.captured(4);
        info.function = match.captured(3);
        info.address = match.captured(2);
        info.line = match.captured(5).toInt();
        info.step = match.captured(1).toInt();
        info.type = BtInfo::Source;
        return info;
    }

    // try b) cap #number(1), address(2), function(3), lib(4)
    static const QRegularExpression rxb(QStringLiteral("^#(\\d+)\\s+(0x\\w+)\\s+in\\s+(.+)\\s+from\\s+(.+)$"));
    match = rxb.match(line);
    if (match.hasMatch()) {
        BtInfo info;
        info.original = line;
        info.filename = match.captured(4);
        info.function = match.captured(3);
        info.address = match.captured(2);
        info.line = -1;
        info.step = match.captured(1).toInt();
        info.type = BtInfo::Lib;
        return info;
    }

    // try c) #41 0x0805e690 in ?? ()
    static const QRegularExpression rxc(QStringLiteral("^#(\\d+)\\s+(0x\\w+)\\s+in\\s+\\?\\?\\s+\\(\\)$"));
    match = rxc.match(line);
    if (match.hasMatch()) {
        BtInfo info;
        info.original = line;
        info.filename = QString();
        info.function = QString();
        info.address = match.captured(2);
        info.line = -1;
        info.step = match.captured(1).toInt();
        info.type = BtInfo::Unknown;
        return info;
    }

    // try d) #5  0xffffe410 in __kernel_vsyscall ()
    static const QRegularExpression rxd(QStringLiteral("^#(\\d+)\\s+(0x\\w+)\\s+in\\s+(.+)$"));
    match = rxd.match(line);
    if (match.hasMatch()) {
        BtInfo info;
        info.original = line;
        info.filename = QString();
        info.function = match.captured(3);
        info.address = match.captured(2);
        info.line = -1;
        info.step = match.captured(1).toInt();
        info.type = BtInfo::Unknown;
        return info;
    }

    qDebug() << "Unknown backtrace line:" << line;

    BtInfo info;
    info.type = BtInfo::Invalid;
    return info;
}

QList<BtInfo> KateBtParser::parseBacktrace(const QString &bt)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QStringList l = bt.split(eolDelimiter(bt), QString::SkipEmptyParts);
#else
    QStringList l = bt.split(eolDelimiter(bt), Qt::SkipEmptyParts);
#endif

    l = normalizeBt(l);

    QList<BtInfo> btList;
    for (int i = 0; i < l.size(); ++i) {
        BtInfo info = parseBtLine(l[i]);
        if (info.type != BtInfo::Invalid) {
            btList.append(parseBtLine(l[i]));
        }
    }

    return btList;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
