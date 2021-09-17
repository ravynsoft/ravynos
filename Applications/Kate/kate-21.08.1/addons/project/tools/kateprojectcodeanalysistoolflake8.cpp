/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2017 Héctor Mesa Jiménez <hector@lcc.uma.es>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectcodeanalysistoolflake8.h"

#include "kateproject.h"

#include <KLocalizedString>
#include <QRegularExpression>

KateProjectCodeAnalysisToolFlake8::KateProjectCodeAnalysisToolFlake8(QObject *parent)
    : KateProjectCodeAnalysisTool(parent)
{
}

KateProjectCodeAnalysisToolFlake8::~KateProjectCodeAnalysisToolFlake8()
{
}

QString KateProjectCodeAnalysisToolFlake8::name() const
{
    return i18n("Flake8 (Python)");
}

QString KateProjectCodeAnalysisToolFlake8::description() const
{
    return i18n("Flake8: Your Tool For Style Guide Enforcement for Python");
}

QString KateProjectCodeAnalysisToolFlake8::fileExtensions() const
{
    return QStringLiteral("py");
}

QStringList KateProjectCodeAnalysisToolFlake8::filter(const QStringList &files) const
{
    // for now we expect files with extension
    return files.filter(QRegularExpression(QStringLiteral("\\.(") + fileExtensions() + QStringLiteral(")$")));
}

QString KateProjectCodeAnalysisToolFlake8::path() const
{
    /*
     * for now, only the executable in the path can be called,
     * but it would be great to be able to specify a version
     * installed in a virtual environment
     */
    return QStringLiteral("flake8");
}

QStringList KateProjectCodeAnalysisToolFlake8::arguments()
{
    QStringList _args;

    _args << QStringLiteral("--exit-zero")
          /*
           * translating a flake8 code to a severity level is subjective,
           * so the code is provided as a severity level.
           */
          << QStringLiteral("--format=%(path)s////%(row)d////%(code)s////%(text)s");

    if (m_project) {
        auto &&fileList = filter(m_project->files());
        setActualFilesCount(fileList.size());
        _args.append(fileList);
    }

    return _args;
}

QString KateProjectCodeAnalysisToolFlake8::notInstalledMessage() const
{
    return i18n("Please install 'flake8'.");
}

QStringList KateProjectCodeAnalysisToolFlake8::parseLine(const QString &line) const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return line.split(QRegularExpression(QStringLiteral("////")), QString::SkipEmptyParts);
#else
    return line.split(QRegularExpression(QStringLiteral("////")), Qt::SkipEmptyParts);
#endif
}

QString KateProjectCodeAnalysisToolFlake8::stdinMessages()
{
    return QString();
}
