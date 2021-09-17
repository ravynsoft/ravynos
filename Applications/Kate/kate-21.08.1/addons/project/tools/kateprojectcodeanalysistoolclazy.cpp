/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kateprojectcodeanalysistoolclazy.h"
#include "kateproject.h"

#include <KLocalizedString>

#include <QDir>
#include <QRegularExpression>

KateProjectCodeAnalysisToolClazy::KateProjectCodeAnalysisToolClazy(QObject *parent)
    : KateProjectCodeAnalysisTool(parent)
{
}

QString KateProjectCodeAnalysisToolClazy::name() const
{
    return i18n("Clazy (Qt/C++)");
}

QString KateProjectCodeAnalysisToolClazy::description() const
{
    return i18n("Clazy is a static analysis tool for Qt/C++ code");
}

QString KateProjectCodeAnalysisToolClazy::fileExtensions() const
{
    return QStringLiteral("cpp|cxx|cc|c++|tpp|txx");
}

QStringList KateProjectCodeAnalysisToolClazy::filter(const QStringList &files) const
{
    // c++ files
    return files.filter(
        QRegularExpression(QStringLiteral("\\.(") + fileExtensions().replace(QStringLiteral("+"), QStringLiteral("\\+")) + QStringLiteral(")$")));
}

QString KateProjectCodeAnalysisToolClazy::path() const
{
    return QStringLiteral("clazy-standalone");
}

static QString buildDirectory(const QVariantMap &projectMap)
{
    const QVariantMap buildMap = projectMap[QStringLiteral("build")].toMap();
    const QString buildDir = buildMap[QStringLiteral("directory")].toString();
    return buildDir;
}

QStringList KateProjectCodeAnalysisToolClazy::arguments()
{
    if (!m_project) {
        return {};
    }

    QString compileCommandsDir = compileCommandsDirectory();

    QStringList args;
    if (!compileCommandsDir.isEmpty()) {
        args = QStringList{QStringLiteral("-p"), compileCommandsDir};
    }

    auto &&fileList = filter(m_project->files());
    setActualFilesCount(fileList.size());
    QString files = fileList.join(QLatin1Char(' '));

    return args << fileList;
}

QString KateProjectCodeAnalysisToolClazy::notInstalledMessage() const
{
    return i18n("Please install 'clazy'.");
}

QStringList KateProjectCodeAnalysisToolClazy::parseLine(const QString &line) const
{
    //"/path/kate/kate/kateapp.cpp:529:10: warning: Missing reference in range-for with non trivial type (QJsonValue) [-Wclazy-range-loop]"
    int idxColon = line.indexOf(QLatin1Char(':'));
    if (idxColon < 0) {
        return {};
    }
    QString file = line.mid(0, idxColon);
    idxColon++;
    int nextColon = line.indexOf(QLatin1Char(':'), idxColon);
    QString lineNo = line.mid(idxColon, nextColon - idxColon);

    int spaceIdx = line.indexOf(QLatin1Char(' '), nextColon);
    if (spaceIdx < 0) {
        return {};
    }

    idxColon = line.indexOf(QLatin1Char(':'), spaceIdx);
    if (idxColon < 0) {
        return {};
    }

    QString severity = line.mid(spaceIdx + 1, idxColon - (spaceIdx + 1));

    idxColon++;

    QString msg = line.mid(idxColon);

    return {file, lineNo, severity, msg};
}

QString KateProjectCodeAnalysisToolClazy::stdinMessages()
{
    return QString();
}

QString KateProjectCodeAnalysisToolClazy::compileCommandsDirectory() const
{
    QString buildDir = buildDirectory(m_project->projectMap());
    const QString compCommandsFile = QStringLiteral("compile_commands.json");

    if (buildDir.startsWith(QLatin1String("./"))) {
        buildDir = buildDir.mid(2);
    }

    /**
     * list of absoloute paths to check compile commands
     */
    const QString possiblePaths[4] = {
        /** Absoloute build path in .kateproject e.g from cmake */
        buildDir,
        /** Relative path in .kateproject e.g */
        m_project->baseDir() + (buildDir.startsWith(QLatin1Char('/')) ? buildDir : QLatin1Char('/') + buildDir),
        /** Check for the commonly existing "build/" directory */
        m_project->baseDir() + QStringLiteral("/build"),
        /** Project base, maybe it has a symlink to compile_commands.json file */
        m_project->baseDir(),
    };

    /**
     * Check all paths one by one for compile_commands.json and exit when found
     */
    QString compileCommandsDir;
    for (const QString &path : possiblePaths) {
        if (path.isEmpty()) {
            continue;
        }
        const QString guessedPath = QDir(path).filePath(compCommandsFile);
        const bool dirHasCompileComds = QFile::exists(guessedPath);
        if (dirHasCompileComds) {
            compileCommandsDir = guessedPath;
            break;
        }
    }

    return compileCommandsDir;
}
