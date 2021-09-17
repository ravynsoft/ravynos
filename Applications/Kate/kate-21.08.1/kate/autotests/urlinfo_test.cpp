/*
    SPDX-FileCopyrightText: 2020 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "urlinfo_test.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

QTEST_MAIN(UrlInfoTest)

void UrlInfoTest::someUrls()
{
    // check that some things convert correctly to urls
    QCOMPARE(UrlInfo(QStringLiteral("file:///for_sure_not_there_path_xxcv123/to/file")).url.toString(),
             QStringLiteral("file:///for_sure_not_there_path_xxcv123/to/file"));
    QCOMPARE(UrlInfo(QStringLiteral("sftp://127.0.0.1:1234/path/to/file")).url.toString(), QStringLiteral("sftp://127.0.0.1:1234/path/to/file"));
}

void UrlInfoTest::someCursors()
{
    // check that some things convert correctly to urls + cursors
    QCOMPARE(UrlInfo(QStringLiteral("file:///for_sure_not_there_path_xxcv123/to/file:1234:12")).url.toString(),
             QStringLiteral("file:///for_sure_not_there_path_xxcv123/to/file"));
    QCOMPARE(UrlInfo(QStringLiteral("file:///for_sure_not_there_path_xxcv123/to/file:1234:12")).cursor, KTextEditor::Cursor(1233, 11));
    QCOMPARE(UrlInfo(QStringLiteral("sftp://127.0.0.1:1234/path/to/file:1:1")).url.toString(), QStringLiteral("sftp://127.0.0.1:1234/path/to/file"));
    QCOMPARE(UrlInfo(QStringLiteral("sftp://127.0.0.1:1234/path/to/file:1:1")).cursor, KTextEditor::Cursor(0, 0));
    // check url query string to cursor
    QCOMPARE(UrlInfo(QStringLiteral("sftp://127.0.0.1:1234/path/to/file?line=2&column=3")).cursor, KTextEditor::Cursor(1, 2));
    QCOMPARE(UrlInfo(QStringLiteral("fish://remote/file?some=variable&line=4&")).cursor, KTextEditor::Cursor(3, 0));
    QCOMPARE(UrlInfo(QStringLiteral("file:///directory/file?some=variable&column=5&other=value&line=6")).cursor, KTextEditor::Cursor(5, 4));
    QCOMPARE(UrlInfo(QStringLiteral("~/file?line=7")).url.hasQuery(), false);
}

void UrlInfoTest::urlWithColonAtStart()
{
#ifndef WIN32 // : invalid for filenames on Windows

    // create test file in temporary directory, as qt sees :test..... as absolute, hack with ./:
    QTemporaryDir dir;
    const auto oldCurrent = QDir::currentPath();
    QDir::setCurrent(dir.path());
    QFile test(QStringLiteral("./:test.txt"));
    QVERIFY(test.open(QFile::WriteOnly));

    // see bug 430216 => before this was some absolute file name
    const UrlInfo info(QStringLiteral(":test.txt:123:1"));
    QCOMPARE(info.cursor, KTextEditor::Cursor(122, 0));
    QVERIFY(info.url.isLocalFile());
    QVERIFY(QFileInfo::exists(info.url.toLocalFile()));

    // back to old working dir
    QDir::setCurrent(oldCurrent);
#endif
}
