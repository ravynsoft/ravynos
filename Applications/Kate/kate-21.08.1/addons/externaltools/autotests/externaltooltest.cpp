/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "externaltooltest.h"
#include "../kateexternaltool.h"
#include "../katetoolrunner.h"

#include <QString>
#include <QTest>

#include <KConfig>
#include <KConfigGroup>

QTEST_MAIN(ExternalToolTest)

void ExternalToolTest::initTestCase()
{
}

void ExternalToolTest::cleanupTestCase()
{
}

void ExternalToolTest::testLoadSave()
{
    KConfig config;
    KConfigGroup cg(&config, "tool");

    KateExternalTool tool;
    tool.category = QStringLiteral("Git Tools");
    tool.name = QStringLiteral("git cola");
    tool.icon = QStringLiteral("git-cola");
    tool.executable = QStringLiteral("git-cola");
    tool.arguments = QStringLiteral("none");
    tool.input = QStringLiteral("in");
    tool.workingDir = QStringLiteral("/usr/bin");
    tool.mimetypes = QStringList{QStringLiteral("everything")};
    tool.hasexec = true;
    tool.actionName = QStringLiteral("asdf");
    tool.cmdname = QStringLiteral("git-cola");
    tool.saveMode = KateExternalTool::SaveMode::None;

    tool.save(cg);

    KateExternalTool clonedTool;
    clonedTool.load(cg);
    QCOMPARE(tool, clonedTool);
}

void ExternalToolTest::testRunListDirectory()
{
    // Skip, if 'ls' is not installed
    if (QStandardPaths::findExecutable(QStringLiteral("ls")).isEmpty()) {
        QSKIP("'ls' not found - skipping test");
    }

    std::unique_ptr<KateExternalTool> tool(new KateExternalTool());
    tool->category = QStringLiteral("Tools");
    tool->name = QStringLiteral("ls");
    tool->icon = QStringLiteral("none");
    tool->executable = QStringLiteral("ls");
    tool->arguments = QStringLiteral("/usr");
    tool->workingDir = QStringLiteral("/tmp");
    tool->mimetypes = QStringList{};
    tool->hasexec = true;
    tool->actionName = QStringLiteral("ls");
    tool->cmdname = QStringLiteral("ls");
    tool->saveMode = KateExternalTool::SaveMode::None;
    std::unique_ptr<KateExternalTool> tool2(new KateExternalTool(*tool));

    // 1. /tmp $ ls /usr
    KateToolRunner runner1(std::move(tool), nullptr);
    runner1.run();
    runner1.waitForFinished();
    QVERIFY(runner1.outputData().contains(QStringLiteral("bin")));

    // 2. /usr $ ls
    tool2->arguments.clear();
    tool2->workingDir = QStringLiteral("/usr");
    KateToolRunner runner2(std::move(tool2), nullptr);
    runner2.run();
    runner2.waitForFinished();
    QVERIFY(runner2.outputData().contains(QStringLiteral("bin")));

    // 1. and 2. must give the same result
    QCOMPARE(runner1.outputData(), runner2.outputData());
}

void ExternalToolTest::testRunTac()
{
    // Skip, if 'tac' is not installed
    if (QStandardPaths::findExecutable(QStringLiteral("tac")).isEmpty()) {
        QSKIP("'tac' not found - skipping test");
    }

    std::unique_ptr<KateExternalTool> tool(new KateExternalTool());
    tool->name = QStringLiteral("tac");
    tool->executable = QStringLiteral("tac");
    tool->input = QStringLiteral("a\nb\nc\n");
    tool->saveMode = KateExternalTool::SaveMode::None;

    // run tac to reverse order
    KateToolRunner runner(std::move(tool), nullptr);
    runner.run();
    runner.waitForFinished();
    QCOMPARE(runner.outputData(), QStringLiteral("c\nb\na\n"));
}

// kate: space-indent on; indent-width 4; replace-tabs on;
