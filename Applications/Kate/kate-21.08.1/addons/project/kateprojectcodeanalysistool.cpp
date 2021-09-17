/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2017 Héctor Mesa Jiménez <hector@lcc.uma.es>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectcodeanalysistool.h"

KateProjectCodeAnalysisTool::KateProjectCodeAnalysisTool(QObject *parent)
    : QObject(parent)
{
}

KateProjectCodeAnalysisTool::~KateProjectCodeAnalysisTool()
{
}

void KateProjectCodeAnalysisTool::setProject(KateProject *project)
{
    m_project = project;
}

bool KateProjectCodeAnalysisTool::isSuccessfulExitCode(int exitCode) const
{
    return exitCode == 0;
}

int KateProjectCodeAnalysisTool::getActualFilesCount() const
{
    return m_filesCount;
}

void KateProjectCodeAnalysisTool::setActualFilesCount(int count)
{
    m_filesCount = count;
}

void KateProjectCodeAnalysisTool::setMainWindow(KTextEditor::MainWindow *mainWin)
{
    m_mainWindow = mainWin;
}
