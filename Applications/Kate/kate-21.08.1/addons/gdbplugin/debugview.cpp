//
// debugview.cpp
//
// Description: Manages the interaction with GDB
//
//
// SPDX-FileCopyrightText: 2008-2010 Ian Wakeling <ian.wakeling@ntlworld.com>
// SPDX-FileCopyrightText: 2011 Kåre Särs <kare.sars@iki.fi>
//
//  SPDX-License-Identifier: LGPL-2.0-only

#include "debugview.h"

#include <QFile>
#include <QRegularExpression>
#include <QTimer>

#include <KLocalizedString>
#include <KMessageBox>
#include <KUrlRequester>

#include <signal.h>
#include <stdlib.h>

static const QString PromptStr = QStringLiteral("(prompt)");

DebugView::DebugView(QObject *parent)
    : QObject(parent)
    , m_debugProcess(nullptr)
    , m_state(none)
    , m_subState(normal)
    , m_debugLocationChanged(true)
    , m_queryLocals(false)
{
}

DebugView::~DebugView()
{
    if (m_debugProcess.state() != QProcess::NotRunning) {
        m_debugProcess.kill();
        m_debugProcess.blockSignals(true);
        m_debugProcess.waitForFinished();
    }
}

void DebugView::runDebugger(const GDBTargetConf &conf, const QStringList &ioFifos)
{
    if (conf.executable.isEmpty()) {
        return;
    }
    m_targetConf = conf;
    if (ioFifos.size() == 3) {
        m_ioPipeString = QStringLiteral("< %1 1> %2 2> %3").arg(ioFifos[0], ioFifos[1], ioFifos[2]);
    }

    if (m_state == none) {
        m_outBuffer.clear();
        m_errBuffer.clear();
        m_errorList.clear();

        // create a process to control GDB
        m_debugProcess.setWorkingDirectory(m_targetConf.workDir);

        connect(&m_debugProcess, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, &DebugView::slotError);

        connect(&m_debugProcess, &QProcess::readyReadStandardError, this, &DebugView::slotReadDebugStdErr);

        connect(&m_debugProcess, &QProcess::readyReadStandardOutput, this, &DebugView::slotReadDebugStdOut);

        connect(&m_debugProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &DebugView::slotDebugFinished);

        m_debugProcess.start(m_targetConf.gdbCmd, QStringList());

        m_nextCommands << QStringLiteral("set pagination off");
        m_state = ready;
    } else {
        // On startup the gdb prompt will trigger the "nextCommands",
        // here we have to trigger it manually.
        QTimer::singleShot(0, this, &DebugView::issueNextCommand);
    }
    m_nextCommands << QStringLiteral("file \"%1\"").arg(m_targetConf.executable);
    m_nextCommands << QStringLiteral("set args %1 %2").arg(m_targetConf.arguments, m_ioPipeString);
    m_nextCommands << QStringLiteral("set inferior-tty /dev/null");
    m_nextCommands << m_targetConf.customInit;
    m_nextCommands << QStringLiteral("(Q) info breakpoints");
}

bool DebugView::debuggerRunning() const
{
    return (m_state != none);
}

bool DebugView::debuggerBusy() const
{
    return (m_state == executingCmd);
}

bool DebugView::hasBreakpoint(const QUrl &url, int line)
{
    for (const auto &breakpoint : qAsConst(m_breakPointList)) {
        if ((url == breakpoint.file) && (line == breakpoint.line)) {
            return true;
        }
    }
    return false;
}

void DebugView::toggleBreakpoint(QUrl const &url, int line)
{
    if (m_state == ready) {
        QString cmd;
        if (hasBreakpoint(url, line)) {
            cmd = QStringLiteral("clear %1:%2").arg(url.path()).arg(line);
        } else {
            cmd = QStringLiteral("break %1:%2").arg(url.path()).arg(line);
        }
        issueCommand(cmd);
    }
}

void DebugView::slotError()
{
    KMessageBox::sorry(nullptr, i18n("Could not start debugger process"));
}

void DebugView::slotReadDebugStdOut()
{
    m_outBuffer += QString::fromLocal8Bit(m_debugProcess.readAllStandardOutput().data());
    int end = 0;
    // handle one line at a time
    do {
        end = m_outBuffer.indexOf(QLatin1Char('\n'));
        if (end < 0) {
            break;
        }
        processLine(m_outBuffer.mid(0, end));
        m_outBuffer.remove(0, end + 1);
    } while (1);

    if (m_outBuffer == QLatin1String("(gdb) ") || m_outBuffer == QLatin1String(">")) {
        m_outBuffer.clear();
        processLine(PromptStr);
    }
}

void DebugView::slotReadDebugStdErr()
{
    m_errBuffer += QString::fromLocal8Bit(m_debugProcess.readAllStandardError().data());
    int end = 0;
    // add whole lines at a time to the error list
    do {
        end = m_errBuffer.indexOf(QLatin1Char('\n'));
        if (end < 0) {
            break;
        }
        m_errorList << m_errBuffer.mid(0, end);
        m_errBuffer.remove(0, end + 1);
    } while (1);

    processErrors();
}

void DebugView::slotDebugFinished(int /*exitCode*/, QProcess::ExitStatus status)
{
    if (status != QProcess::NormalExit) {
        Q_EMIT outputText(i18n("*** gdb exited normally ***") + QLatin1Char('\n'));
    }

    m_state = none;
    Q_EMIT readyForInput(false);

    // remove all old breakpoints
    BreakPoint bPoint;
    while (!m_breakPointList.empty()) {
        bPoint = m_breakPointList.takeFirst();
        Q_EMIT breakPointCleared(bPoint.file, bPoint.line - 1);
    }

    Q_EMIT gdbEnded();
}

void DebugView::movePC(QUrl const &url, int line)
{
    if (m_state == ready) {
        QString cmd = QStringLiteral("tbreak %1:%2").arg(url.path()).arg(line);
        m_nextCommands << QStringLiteral("jump %1:%2").arg(url.path()).arg(line);
        issueCommand(cmd);
    }
}

void DebugView::runToCursor(QUrl const &url, int line)
{
    if (m_state == ready) {
        QString cmd = QStringLiteral("tbreak %1:%2").arg(url.path()).arg(line);
        m_nextCommands << QStringLiteral("continue");
        issueCommand(cmd);
    }
}

void DebugView::slotInterrupt()
{
    if (m_state == executingCmd) {
        m_debugLocationChanged = true;
    }
    const auto pid = m_debugProcess.processId();
    if (pid != 0) {
        ::kill(pid, SIGINT);
    }
}

void DebugView::slotKill()
{
    if (m_state != ready) {
        slotInterrupt();
        m_state = ready;
    }
    issueCommand(QStringLiteral("kill"));
}

void DebugView::slotReRun()
{
    slotKill();
    m_nextCommands << QStringLiteral("file \"%1\"").arg(m_targetConf.executable);
    m_nextCommands << QStringLiteral("set args %1 %2").arg(m_targetConf.arguments).arg(m_ioPipeString);
    m_nextCommands << QStringLiteral("set inferior-tty /dev/null");
    m_nextCommands << m_targetConf.customInit;
    m_nextCommands << QStringLiteral("(Q) info breakpoints");

    m_nextCommands << QStringLiteral("tbreak main");
    m_nextCommands << QStringLiteral("run");
    m_nextCommands << QStringLiteral("p setvbuf(stdout, 0, %1, 1024)").arg(_IOLBF);
    m_nextCommands << QStringLiteral("continue");
}

void DebugView::slotStepInto()
{
    issueCommand(QStringLiteral("step"));
}

void DebugView::slotStepOver()
{
    issueCommand(QStringLiteral("next"));
}

void DebugView::slotStepOut()
{
    issueCommand(QStringLiteral("finish"));
}

void DebugView::slotContinue()
{
    issueCommand(QStringLiteral("continue"));
}

static const QRegularExpression breakpointList(QStringLiteral("\\ANum\\s+Type\\s+Disp\\s+Enb\\s+Address\\s+What.*\\z"));
static const QRegularExpression breakpointListed(QStringLiteral("\\A(\\d)\\s+breakpoint\\s+keep\\sy\\s+0x[\\da-f]+\\sin\\s.+\\sat\\s([^:]+):(\\d+).*\\z"));
static const QRegularExpression stackFrameAny(QStringLiteral("\\A#(\\d+)\\s(.*)\\z"));
static const QRegularExpression stackFrameFile(QStringLiteral("\\A#(\\d+)\\s+(?:0x[\\da-f]+\\s*in\\s)*(\\S+)(\\s\\(.*\\)) at ([^:]+):(\\d+).*\\z"));
static const QRegularExpression changeFile(
    QStringLiteral("\\A(?:(?:Temporary\\sbreakpoint|Breakpoint)\\s*\\d+,\\s*|0x[\\da-f]+\\s*in\\s*)?[^\\s]+\\s*\\([^)]*\\)\\s*at\\s*([^:]+):(\\d+).*\\z"));
static const QRegularExpression changeLine(QStringLiteral("\\A(\\d+)\\s+.*\\z"));
static const QRegularExpression breakPointReg(QStringLiteral("\\ABreakpoint\\s+(\\d+)\\s+at\\s+0x[\\da-f]+:\\s+file\\s+([^\\,]+)\\,\\s+line\\s+(\\d+).*\\z"));
static const QRegularExpression breakPointMultiReg(QStringLiteral("\\ABreakpoint\\s+(\\d+)\\s+at\\s+0x[\\da-f]+:\\s+([^\\,]+):(\\d+).*\\z"));
static const QRegularExpression breakPointDel(QStringLiteral("\\ADeleted\\s+breakpoint.*\\z"));
static const QRegularExpression exitProgram(QStringLiteral("\\A(?:Program|.*Inferior.*)\\s+exited.*\\z"));
static const QRegularExpression threadLine(QStringLiteral("\\A\\**\\s+(\\d+)\\s+Thread.*\\z"));

void DebugView::processLine(QString line)
{
    if (line.isEmpty()) {
        return;
    }

    static QRegularExpressionMatch match;
    switch (m_state) {
    case none:
    case ready:
        if (PromptStr == line) {
            // we get here after initialization
            QTimer::singleShot(0, this, &DebugView::issueNextCommand);
        }
        break;

    case executingCmd:
        if (breakpointList.match(line).hasMatch()) {
            m_state = listingBreakpoints;
            Q_EMIT clearBreakpointMarks();
            m_breakPointList.clear();
        } else if (line.contains(QLatin1String("No breakpoints or watchpoints."))) {
            Q_EMIT clearBreakpointMarks();
            m_breakPointList.clear();
        } else if ((match = stackFrameAny.match(line)).hasMatch()) {
            if (m_lastCommand.contains(QLatin1String("info stack"))) {
                Q_EMIT stackFrameInfo(match.captured(1), match.captured(2));
            } else {
                m_subState = (m_subState == normal) ? stackFrameSeen : stackTraceSeen;

                m_newFrameLevel = match.captured(1).toInt();

                if ((match = stackFrameFile.match(line)).hasMatch()) {
                    m_newFrameFile = match.captured(4);
                }
            }
        } else if ((match = changeFile.match(line)).hasMatch()) {
            m_currentFile = match.captured(1).trimmed();
            int lineNum = match.captured(2).toInt();

            if (!m_nextCommands.contains(QLatin1String("continue"))) {
                // GDB uses 1 based line numbers, kate uses 0 based...
                Q_EMIT debugLocationChanged(resolveFileName(m_currentFile), lineNum - 1);
            }
            m_debugLocationChanged = true;
        } else if ((match = changeLine.match(line)).hasMatch()) {
            int lineNum = match.captured(1).toInt();

            if (m_subState == stackFrameSeen) {
                m_currentFile = m_newFrameFile;
            }
            if (!m_nextCommands.contains(QLatin1String("continue"))) {
                // GDB uses 1 based line numbers, kate uses 0 based...
                Q_EMIT debugLocationChanged(resolveFileName(m_currentFile), lineNum - 1);
            }
            m_debugLocationChanged = true;
        } else if ((match = breakPointReg.match(line)).hasMatch()) {
            BreakPoint breakPoint;
            breakPoint.number = match.captured(1).toInt();
            breakPoint.file = resolveFileName(match.captured(2));
            breakPoint.line = match.captured(3).toInt();
            m_breakPointList << breakPoint;
            Q_EMIT breakPointSet(breakPoint.file, breakPoint.line - 1);
        } else if ((match = breakPointMultiReg.match(line)).hasMatch()) {
            BreakPoint breakPoint;
            breakPoint.number = match.captured(1).toInt();
            breakPoint.file = resolveFileName(match.captured(2));
            breakPoint.line = match.captured(3).toInt();
            m_breakPointList << breakPoint;
            Q_EMIT breakPointSet(breakPoint.file, breakPoint.line - 1);
        } else if (breakPointDel.match(line).hasMatch()) {
            line.remove(QStringLiteral("Deleted breakpoint"));
            line.remove(QLatin1Char('s')); // in case of multiple breakpoints
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            QStringList numbers = line.split(QLatin1Char(' '), QString::SkipEmptyParts);
#else
            QStringList numbers = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
#endif
            for (int i = 0; i < numbers.size(); i++) {
                for (int j = 0; j < m_breakPointList.size(); j++) {
                    if (numbers[i].toInt() == m_breakPointList[j].number) {
                        Q_EMIT breakPointCleared(m_breakPointList[j].file, m_breakPointList[j].line - 1);
                        m_breakPointList.removeAt(j);
                        break;
                    }
                }
            }
        } else if (exitProgram.match(line).hasMatch() || line.contains(QLatin1String("The program no longer exists"))
                   || line.contains(QLatin1String("Kill the program being debugged"))) {
            // if there are still commands to execute remove them to remove unneeded output
            // except  if the "kill was for "re-run"
            if ((!m_nextCommands.empty()) && !m_nextCommands[0].contains(QLatin1String("file"))) {
                m_nextCommands.clear();
            }
            m_debugLocationChanged = false; // do not insert (Q) commands
            Q_EMIT programEnded();
        } else if (PromptStr == line) {
            if (m_subState == stackFrameSeen) {
                Q_EMIT stackFrameChanged(m_newFrameLevel);
            }
            m_state = ready;

            // Give the error a possibility get noticed since stderr and stdout are not in sync
            QTimer::singleShot(0, this, &DebugView::issueNextCommand);
        }
        break;

    case listingBreakpoints:;
        if ((match = breakpointListed.match(line)).hasMatch()) {
            BreakPoint breakPoint;
            breakPoint.number = match.captured(1).toInt();
            breakPoint.file = resolveFileName(match.captured(2));
            breakPoint.line = match.captured(3).toInt();
            m_breakPointList << breakPoint;
            Q_EMIT breakPointSet(breakPoint.file, breakPoint.line - 1);
        } else if (PromptStr == line) {
            m_state = ready;
            QTimer::singleShot(0, this, &DebugView::issueNextCommand);
        }
        break;
    case infoArgs:
        if (PromptStr == line) {
            m_state = ready;
            QTimer::singleShot(0, this, &DebugView::issueNextCommand);
        } else {
            Q_EMIT infoLocal(line);
        }
        break;
    case printThis:
        if (PromptStr == line) {
            m_state = ready;
            QTimer::singleShot(0, this, &DebugView::issueNextCommand);
        } else {
            Q_EMIT infoLocal(line);
        }
        break;
    case infoLocals:
        if (PromptStr == line) {
            m_state = ready;
            Q_EMIT infoLocal(QString());
            QTimer::singleShot(0, this, &DebugView::issueNextCommand);
        } else {
            Q_EMIT infoLocal(line);
        }
        break;
    case infoStack:
        if (PromptStr == line) {
            m_state = ready;
            Q_EMIT stackFrameInfo(QString(), QString());
            QTimer::singleShot(0, this, &DebugView::issueNextCommand);
        } else if ((match = stackFrameAny.match(line)).hasMatch()) {
            Q_EMIT stackFrameInfo(match.captured(1), match.captured(2));
        }
        break;
    case infoThreads:
        if (PromptStr == line) {
            m_state = ready;
            QTimer::singleShot(0, this, &DebugView::issueNextCommand);
        } else if ((match = threadLine.match(line)).hasMatch()) {
            Q_EMIT threadInfo(match.captured(1).toInt(), (line[0] == QLatin1Char('*')));
        }
        break;
    }
    outputTextMaybe(line);
}

void DebugView::processErrors()
{
    QString error;
    while (!m_errorList.empty()) {
        error = m_errorList.takeFirst();
        // qDebug() << error;
        if (error == QLatin1String("The program is not being run.")) {
            if (m_lastCommand == QLatin1String("continue")) {
                m_nextCommands.clear();
                m_nextCommands << QStringLiteral("tbreak main");
                m_nextCommands << QStringLiteral("run");
                m_nextCommands << QStringLiteral("p setvbuf(stdout, 0, %1, 1024)").arg(_IOLBF);
                m_nextCommands << QStringLiteral("continue");
                QTimer::singleShot(0, this, &DebugView::issueNextCommand);
            } else if ((m_lastCommand == QLatin1String("step")) || (m_lastCommand == QLatin1String("next")) || (m_lastCommand == QLatin1String("finish"))) {
                m_nextCommands.clear();
                m_nextCommands << QStringLiteral("tbreak main");
                m_nextCommands << QStringLiteral("run");
                m_nextCommands << QStringLiteral("p setvbuf(stdout, 0, %1, 1024)").arg(_IOLBF);
                QTimer::singleShot(0, this, &DebugView::issueNextCommand);
            } else if ((m_lastCommand == QLatin1String("kill"))) {
                if (!m_nextCommands.empty()) {
                    if (!m_nextCommands[0].contains(QLatin1String("file"))) {
                        m_nextCommands.clear();
                        m_nextCommands << QStringLiteral("quit");
                    }
                    // else continue with "ReRun"
                } else {
                    m_nextCommands << QStringLiteral("quit");
                }
                m_state = ready;
                QTimer::singleShot(0, this, &DebugView::issueNextCommand);
            }
            // else do nothing
        } else if (error.contains(QLatin1String("No line ")) || error.contains(QLatin1String("No source file named"))) {
            // setting a breakpoint failed. Do not continue.
            m_nextCommands.clear();
            Q_EMIT readyForInput(true);
        } else if (error.contains(QLatin1String("No stack"))) {
            m_nextCommands.clear();
            Q_EMIT programEnded();
        }

        if ((m_lastCommand == QLatin1String("(Q)print *this")) && error.contains(QLatin1String("No symbol \"this\" in current context."))) {
            continue;
        }
        Q_EMIT outputError(error + QLatin1Char('\n'));
    }
}

void DebugView::issueCommand(QString const &cmd)
{
    if (m_state == ready) {
        Q_EMIT readyForInput(false);
        m_state = executingCmd;
        if (cmd == QLatin1String("(Q)info locals")) {
            m_state = infoLocals;
        } else if (cmd == QLatin1String("(Q)info args")) {
            m_state = infoArgs;
        } else if (cmd == QLatin1String("(Q)print *this")) {
            m_state = printThis;
        } else if (cmd == QLatin1String("(Q)info stack")) {
            m_state = infoStack;
        } else if (cmd == QLatin1String("(Q)info thread")) {
            Q_EMIT threadInfo(-1, false);
            m_state = infoThreads;
        }
        m_subState = normal;
        m_lastCommand = cmd;

        if (cmd.startsWith(QLatin1String("(Q)"))) {
            m_debugProcess.write(qPrintable(cmd.mid(3)));
        } else {
            Q_EMIT outputText(QStringLiteral("(gdb) ") + cmd + QLatin1Char('\n'));
            m_debugProcess.write(qPrintable(cmd));
        }
        m_debugProcess.write("\n");
    }
}

void DebugView::issueNextCommand()
{
    if (m_state == ready) {
        if (!m_nextCommands.empty()) {
            QString cmd = m_nextCommands.takeFirst();
            // qDebug() << "Next command" << cmd;
            issueCommand(cmd);
        } else {
            // FIXME "thread" needs a better generic solution
            if (m_debugLocationChanged || m_lastCommand.startsWith(QLatin1String("thread"))) {
                m_debugLocationChanged = false;
                if (m_queryLocals && !m_lastCommand.startsWith(QLatin1String("(Q)"))) {
                    m_nextCommands << QStringLiteral("(Q)info stack");
                    m_nextCommands << QStringLiteral("(Q)frame");
                    m_nextCommands << QStringLiteral("(Q)info args");
                    m_nextCommands << QStringLiteral("(Q)print *this");
                    m_nextCommands << QStringLiteral("(Q)info locals");
                    m_nextCommands << QStringLiteral("(Q)info thread");
                    issueNextCommand();
                    return;
                }
            }
            Q_EMIT readyForInput(true);
        }
    }
}

QUrl DebugView::resolveFileName(const QString &fileName)
{
    QFileInfo fInfo = QFileInfo(fileName);
    // did we end up with an absolute path or a relative one?
    if (fInfo.exists()) {
        return QUrl::fromUserInput(fInfo.absoluteFilePath());
    }

    if (fInfo.isAbsolute()) {
        // we can not do anything just return the fileName
        return QUrl::fromUserInput(fileName);
    }

    // Now try to add the working path
    fInfo = QFileInfo(m_targetConf.workDir + fileName);
    if (fInfo.exists()) {
        return QUrl::fromUserInput(fInfo.absoluteFilePath());
    }

    // now try the executable path
    fInfo = QFileInfo(QFileInfo(m_targetConf.executable).absolutePath() + fileName);
    if (fInfo.exists()) {
        return QUrl::fromUserInput(fInfo.absoluteFilePath());
    }

    for (const QString &srcPath : qAsConst(m_targetConf.srcPaths)) {
        fInfo = QFileInfo(srcPath + QDir::separator() + fileName);
        if (fInfo.exists()) {
            return QUrl::fromUserInput(fInfo.absoluteFilePath());
        }
    }

    // we can not do anything just return the fileName
    Q_EMIT sourceFileNotFound(fileName);
    return QUrl::fromUserInput(fileName);
}

void DebugView::outputTextMaybe(const QString &text)
{
    if (!m_lastCommand.startsWith(QLatin1String("(Q)")) && !text.contains(PromptStr)) {
        Q_EMIT outputText(text + QLatin1Char('\n'));
    }
}

void DebugView::slotQueryLocals(bool query)
{
    m_queryLocals = query;
    if (query && (m_state == ready) && (m_nextCommands.empty())) {
        m_nextCommands << QStringLiteral("(Q)info stack");
        m_nextCommands << QStringLiteral("(Q)frame");
        m_nextCommands << QStringLiteral("(Q)info args");
        m_nextCommands << QStringLiteral("(Q)print *this");
        m_nextCommands << QStringLiteral("(Q)info locals");
        m_nextCommands << QStringLiteral("(Q)info thread");
        issueNextCommand();
    }
}

QString DebugView::targetName() const
{
    return m_targetConf.targetName;
}

void DebugView::setFileSearchPaths(const QStringList &paths)
{
    m_targetConf.srcPaths = paths;
}
