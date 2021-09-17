/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2017 Héctor Mesa Jiménez <hector@lcc.uma.es>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectcodeanalysistoolcppcheck.h"
#include "kateproject.h"

#include <KLocalizedString>
#include <QRegularExpression>
#include <QThread>

KateProjectCodeAnalysisToolCppcheck::KateProjectCodeAnalysisToolCppcheck(QObject *parent)
    : KateProjectCodeAnalysisTool(parent)
{
}

KateProjectCodeAnalysisToolCppcheck::~KateProjectCodeAnalysisToolCppcheck()
{
}

QString KateProjectCodeAnalysisToolCppcheck::name() const
{
    return i18n("Cppcheck (C++)");
}

QString KateProjectCodeAnalysisToolCppcheck::description() const
{
    return i18n("Cppcheck is a static analysis tool for C/C++ code");
}

QString KateProjectCodeAnalysisToolCppcheck::fileExtensions() const
{
    return QStringLiteral("cpp|cxx|cc|c++|c|tpp|txx");
}

QStringList KateProjectCodeAnalysisToolCppcheck::filter(const QStringList &files) const
{
    // c++ files
    return files.filter(
        QRegularExpression(QStringLiteral("\\.(") + fileExtensions().replace(QStringLiteral("+"), QStringLiteral("\\+")) + QStringLiteral(")$")));
}

QString KateProjectCodeAnalysisToolCppcheck::path() const
{
    return QStringLiteral("cppcheck");
}

QStringList KateProjectCodeAnalysisToolCppcheck::arguments()
{
    QStringList _args;

    _args << QStringLiteral("-q") << QStringLiteral("-f") << QStringLiteral("-j") + QString::number(QThread::idealThreadCount())
          << QStringLiteral("--inline-suppr") << QStringLiteral("--enable=all") << QStringLiteral("--template={file}////{line}////{severity}////{message}")
          << QStringLiteral("--file-list=-");

    return _args;
}

QString KateProjectCodeAnalysisToolCppcheck::notInstalledMessage() const
{
    return i18n("Please install 'cppcheck'.");
}

QStringList KateProjectCodeAnalysisToolCppcheck::parseLine(const QString &line) const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return line.split(QRegularExpression(QStringLiteral("////")), QString::SkipEmptyParts);
#else
    return line.split(QRegularExpression(QStringLiteral("////")), Qt::SkipEmptyParts);
#endif
}

QString KateProjectCodeAnalysisToolCppcheck::stdinMessages()
{
    // filenames are written to stdin (--file-list=-)

    if (!m_project) {
        return QString();
    }

    auto &&fileList = filter(m_project->files());
    setActualFilesCount(fileList.size());
    return fileList.join(QLatin1Char('\n'));
}
