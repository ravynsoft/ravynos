/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2018 Gregor Mi <codestruct@posteo.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "../kateprojectcodeanalysistool.h"

/**
 * Information provider for shellcheck
 */
class KateProjectCodeAnalysisToolShellcheck : public KateProjectCodeAnalysisTool
{
public:
    explicit KateProjectCodeAnalysisToolShellcheck(QObject *parent = nullptr);

    ~KateProjectCodeAnalysisToolShellcheck() override;

    QString name() const override;

    QString description() const override;

    QString fileExtensions() const override;

    virtual QStringList filter(const QStringList &files) const override;

    QString path() const override;

    QStringList arguments() override;

    QString notInstalledMessage() const override;

    QStringList parseLine(const QString &line) const override;

    bool isSuccessfulExitCode(int exitCode) const override;

    QString stdinMessages() override;
};
