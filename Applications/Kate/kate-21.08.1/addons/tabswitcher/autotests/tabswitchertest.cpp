/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2018 Gregor Mi <codestruct@posteo.org>
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "tabswitchertest.h"
#include "tabswitcherfilesmodel.h"

#include <QTest>

QTEST_MAIN(KateTabSwitcherTest)

void KateTabSwitcherTest::initTestCase()
{
}

void KateTabSwitcherTest::cleanupTestCase()
{
}

void KateTabSwitcherTest::testLongestCommonPrefix()
{
    QFETCH(std::vector<QString>, input_strings);
    QFETCH(QString, expected);

    QCOMPARE(detail::longestCommonPrefix(input_strings), expected);
}

void KateTabSwitcherTest::testLongestCommonPrefix_data()
{
    QTest::addColumn<std::vector<QString>>("input_strings");
    QTest::addColumn<QString>("expected");
    std::vector<QString> strs;

    strs.clear();
    strs.push_back(QStringLiteral("/home/user1/a"));
    strs.push_back(QStringLiteral("/home/user1/bc"));
    QTest::newRow("standard case") << strs << QStringLiteral("/home/user1/");

    strs.clear();
    strs.push_back(QStringLiteral("/home/a"));
    strs.push_back(QStringLiteral("/home/b"));
    strs.push_back(QLatin1String(""));
    QTest::newRow("empty string at the end of the list") << strs << QString();

    strs.clear();
    strs.push_back(QLatin1String(""));
    strs.push_back(QStringLiteral("/home/a"));
    strs.push_back(QStringLiteral("/home/b"));
    strs.push_back(QLatin1String(""));
    QTest::newRow("empty string not only at the end of the list") << strs << QString();

    strs.clear();
    strs.push_back(QStringLiteral("/home/a"));
    strs.push_back(QStringLiteral("/etc/b"));
    QTest::newRow("a prefix with length 1") << strs << QStringLiteral("/");

    strs.clear();
    strs.push_back(QStringLiteral("a"));
    strs.push_back(QStringLiteral("a"));
    QTest::newRow("two equal strings") << strs << QStringLiteral("a");

    strs.clear();
    strs.push_back(QStringLiteral("/home/autolink"));
    strs.push_back(QStringLiteral("/home/async"));
    QTest::newRow("find correct path prefix") << strs << QStringLiteral("/home/");
}
