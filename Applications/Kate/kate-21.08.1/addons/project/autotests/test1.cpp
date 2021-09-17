/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2014 Gregor Mi <codestruct@posteo.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "test1.h"
#include "fileutil.h"
#include "tools/kateprojectcodeanalysistoolshellcheck.h"

#include <QTest>

#include <QString>

QTEST_MAIN(Test1)

void Test1::initTestCase()
{
}

void Test1::cleanupTestCase()
{
}

void Test1::testCommonParent()
{
    QCOMPARE(FileUtil::commonParent(QLatin1String("/usr/local/bin"), QLatin1String("/usr/local/bin")), QLatin1String("/usr/local/"));
    QCOMPARE(FileUtil::commonParent(QLatin1String("/usr/local"), QLatin1String("/usr/local/bin")), QLatin1String("/usr/"));
    QCOMPARE(FileUtil::commonParent(QLatin1String("~/dev/proj1"), QLatin1String("~/dev/proj222")), QLatin1String("~/dev/"));
}

void Test1::testShellCheckParsing()
{
    QString line = QStringLiteral("script.sh:3:11: note: Use ./*glob* or -- *glob* so ... options. [SC2035]");
    KateProjectCodeAnalysisToolShellcheck sc(nullptr);
    QStringList outList = sc.parseLine(line);
    // qDebug() << outList;
    QCOMPARE(outList.size(), 4);
}

// kate: space-indent on; indent-width 4; replace-tabs on;
