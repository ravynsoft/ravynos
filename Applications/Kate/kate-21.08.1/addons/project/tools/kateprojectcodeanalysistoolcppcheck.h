/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2017 Héctor Mesa Jiménez <hector@lcc.uma.es>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_PROJECT_CODE_ANALYSIS_TOOL_CPPCHECK_H
#define KATE_PROJECT_CODE_ANALYSIS_TOOL_CPPCHECK_H

#include "../kateprojectcodeanalysistool.h"

/**
 * Information provider for cppcheck
 */
class KateProjectCodeAnalysisToolCppcheck : public KateProjectCodeAnalysisTool
{
public:
    explicit KateProjectCodeAnalysisToolCppcheck(QObject *parent = nullptr);

    ~KateProjectCodeAnalysisToolCppcheck() override;

    QString name() const override;

    QString description() const override;

    QString fileExtensions() const override;

    virtual QStringList filter(const QStringList &files) const override;

    QString path() const override;

    QStringList arguments() override;

    QString notInstalledMessage() const override;

    QStringList parseLine(const QString &line) const override;

    QString stdinMessages() override;
};

#endif // KATE_PROJECT_CODE_ANALYSIS_TOOL_CPPCHECK_H
