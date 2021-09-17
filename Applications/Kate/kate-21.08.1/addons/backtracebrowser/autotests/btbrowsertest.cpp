/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2014 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "btbrowsertest.h"
#include "btparser.h"

#include <QFileInfo>
#include <QTemporaryFile>
#include <QtTestWidgets>

QTEST_MAIN(KateBtBrowserTest)

void KateBtBrowserTest::initTestCase()
{
}

void KateBtBrowserTest::cleanupTestCase()
{
}

void KateBtBrowserTest::testParser()
{
    // make sure the different types of gdb backtraces are parsed correctly

    // 1) #24 0xb688ff8e in QApplication::notify (this=0xbf997e8c, receiver=0x82607e8, e=0xbf997074) at kernel/qapplication.cpp:3115
    // 2) #39 0xb634211c in g_main_context_dispatch () from /usr/lib/libglib-2.0.so.0
    // 3) #41 0x0805e690 in ?? ()
    // 4) #5  0xffffe410 in __kernel_vsyscall ()

    QString bt = QStringLiteral("#24 0xb688ff8e in QApplication::notify (this=0xbf997e8c, receiver=0x82607e8, e=0xbf997074) at kernel/qapplication.cpp:3115");
    QList<BtInfo> info = KateBtParser::parseBacktrace(bt);

    QVERIFY(info.size() == 1);
    QCOMPARE(info[0].type, BtInfo::Source);
    QCOMPARE(info[0].original, bt);
    QCOMPARE(info[0].filename, QLatin1String("kernel/qapplication.cpp"));
    QCOMPARE(info[0].function, QLatin1String("QApplication::notify (this=0xbf997e8c, receiver=0x82607e8, e=0xbf997074)"));
    QCOMPARE(info[0].address, QLatin1String("0xb688ff8e"));
    QCOMPARE(info[0].line, 3115);
    QCOMPARE(info[0].step, 24);

    bt = QStringLiteral("#39 0xb634211c in g_main_context_dispatch () from /usr/lib/libglib-2.0.so.0");
    info = KateBtParser::parseBacktrace(bt);
    QVERIFY(info.size() == 1);
    QCOMPARE(info[0].type, BtInfo::Lib);
    QCOMPARE(info[0].original, bt);
    QCOMPARE(info[0].filename, QLatin1String("/usr/lib/libglib-2.0.so.0"));
    QCOMPARE(info[0].function, QLatin1String("g_main_context_dispatch ()"));
    QCOMPARE(info[0].address, QLatin1String("0xb634211c"));
    QCOMPARE(info[0].line, -1);
    QCOMPARE(info[0].step, 39);

    bt = QStringLiteral("#41 0x0805e690 in ?? ()");
    info = KateBtParser::parseBacktrace(bt);
    QVERIFY(info.size() == 1);
    QCOMPARE(info[0].type, BtInfo::Unknown);
    QCOMPARE(info[0].original, bt);
    QCOMPARE(info[0].filename, QString());
    QCOMPARE(info[0].function, QString());
    QCOMPARE(info[0].address, QLatin1String("0x0805e690"));
    QCOMPARE(info[0].line, -1);
    QCOMPARE(info[0].step, 41);

    bt = QStringLiteral("#5  0xffffe410 in __kernel_vsyscall ()");
    info = KateBtParser::parseBacktrace(bt);
    QVERIFY(info.size() == 1);
    QCOMPARE(info[0].type, BtInfo::Unknown);
    QCOMPARE(info[0].original, bt);
    QCOMPARE(info[0].filename, QString());
    QCOMPARE(info[0].function, QLatin1String("__kernel_vsyscall ()"));
    QCOMPARE(info[0].address, QLatin1String("0xffffe410"));
    QCOMPARE(info[0].line, -1);
    QCOMPARE(info[0].step, 5);

    bt = QStringLiteral("    Thread 1 (Thread 0x7fb6ba260780 (LWP 16447)):\n[KCrash Handler]");
    info = KateBtParser::parseBacktrace(bt);
    QVERIFY(info.empty());
}

// kate: space-indent on; indent-width 4; replace-tabs on;
