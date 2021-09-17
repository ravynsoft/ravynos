/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2017 Héctor Mesa Jiménez <hector@lcc.uma.es>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_PROJECT_CODE_ANALYSIS_TOOL_H
#define KATE_PROJECT_CODE_ANALYSIS_TOOL_H

#include <QObject>
#include <QString>
#include <QStringList>

class KateProject;
namespace KTextEditor
{
class MainWindow;
}

/**
 * Information provider for a code analysis tool
 */
class KateProjectCodeAnalysisTool : public QObject
{
    Q_OBJECT
protected:
    explicit KateProjectCodeAnalysisTool(QObject *parent = nullptr);

    /**
     * Current project
     */
    KateProject *m_project = nullptr;

    KTextEditor::MainWindow *m_mainWindow;

public:
    ~KateProjectCodeAnalysisTool() override;

    /**
     * bind to this project
     * @param project project this tool will analyze
     */
    virtual void setProject(KateProject *project);

    /**
     * @return tool descriptive name
     */
    virtual QString name() const = 0;

    /**
     * @return tool short description
     */
    virtual QString description() const = 0;

    /**
     * @returns a string containing the file extensions this
     * tool should be run, separated by '|',
     * e.g. "cpp|cxx"
     * NOTE that this is used directly as part of a regular expression.
     * If more flexibility is required this method probably will change
     */
    virtual QString fileExtensions() const = 0;

    /**
     * filter relevant files
     * @param files set of files in project
     * @return relevant files that can be analyzed
     */
    virtual QStringList filter(const QStringList &files) const = 0;

    /**
     * @return tool path
     */
    virtual QString path() const = 0;

    /**
     * @return arguments required for the tool
     * NOTE that this method is not const because here setActualFilesCount might be called
     */
    virtual QStringList arguments() = 0;

    /**
     * @return warning message when the tool is not installed
     */
    virtual QString notInstalledMessage() const = 0;

    /**
     * parse output line
     * @param line
     * @return file, line, severity, message
     */
    virtual QStringList parseLine(const QString &line) const = 0;

    /**
     * Tells the tool runner if the returned process exit code
     * was a successful one.
     *
     * The default implementation returns true on exitCode 0.
     *
     * Override this method for a tool that use a non-zero exit code
     * e.g. if the processing itself was successful but not all files
     * had no linter errors.
     */
    virtual bool isSuccessfulExitCode(int exitCode) const;

    /**
     * @return messages passed to the tool through stdin
     * This is used when the files are not passed as arguments to the tool.
     *
     * NOTE that this method is not const because here setActualFilesCount might be called
     */
    virtual QString stdinMessages() = 0;

    /**
     * @returns the number of files to be processed after the filter
     * has been applied
     */
    int getActualFilesCount() const;

    /**
     * To be called by derived classes
     */
    void setActualFilesCount(int count);

    void setMainWindow(KTextEditor::MainWindow *mainWin);

private:
    int m_filesCount = 0;
};

Q_DECLARE_METATYPE(KateProjectCodeAnalysisTool *)

#endif // KATE_PROJECT_CODE_ANALYSIS_TOOL_H
