/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KATEPROJECTCODEANALYSISTOOLCLAZY_H
#define KATEPROJECTCODEANALYSISTOOLCLAZY_H

#include <kateprojectcodeanalysistool.h>

class KateProjectCodeAnalysisToolClazy : public KateProjectCodeAnalysisTool
{
public:
    explicit KateProjectCodeAnalysisToolClazy(QObject *parent = nullptr);

    ~KateProjectCodeAnalysisToolClazy() override = default;

    QString name() const override;

    QString description() const override;

    QString fileExtensions() const override;

    QStringList filter(const QStringList &files) const override;

    QString path() const override;

    QStringList arguments() override;

    QString notInstalledMessage() const override;

    QStringList parseLine(const QString &line) const override;

    QString stdinMessages() override;

    QString compileCommandsDirectory() const;
};

#endif // KATEPROJECTCODEANALYSISTOOLCLAZY_H
