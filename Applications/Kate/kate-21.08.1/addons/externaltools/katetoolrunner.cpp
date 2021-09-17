/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "katetoolrunner.h"

#include "kateexternaltool.h"

#include <KLocalizedString>
#include <KShell>
#include <KTextEditor/View>
#include <QFileInfo>
#include <QRegularExpression>

KateToolRunner::KateToolRunner(std::unique_ptr<KateExternalTool> tool, KTextEditor::View *view, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_tool(std::move(tool))
    , m_process(new QProcess())
{
    m_process->setProcessChannelMode(QProcess::SeparateChannels);
}

KateToolRunner::~KateToolRunner()
{
}

KTextEditor::View *KateToolRunner::view() const
{
    return m_view;
}

KateExternalTool *KateToolRunner::tool() const
{
    return m_tool.get();
}

void KateToolRunner::run()
{
    if (!m_tool->workingDir.isEmpty()) {
        m_process->setWorkingDirectory(m_tool->workingDir);
    } else if (m_view) {
        // if nothing is set, use the current document's directory
        const auto url = m_view->document()->url();
        if (url.isLocalFile()) {
            const QString localFilePath = url.toLocalFile();
            m_process->setWorkingDirectory(QFileInfo(localFilePath).absolutePath());
        }
    }

    QObject::connect(m_process.get(), &QProcess::readyReadStandardOutput, [this]() {
        m_stdout += m_process->readAllStandardOutput();
    });
    QObject::connect(m_process.get(), &QProcess::readyReadStandardError, [this]() {
        m_stderr += m_process->readAllStandardError();
    });
    QObject::connect(m_process.get(),
                     static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                     [this](int exitCode, QProcess::ExitStatus exitStatus) {
                         Q_EMIT toolFinished(this, exitCode, exitStatus == QProcess::CrashExit);
                     });

    // Write stdin to process, if applicable, then close write channel
    QObject::connect(m_process.get(), &QProcess::started, [this]() {
        if (!m_tool->input.isEmpty()) {
            m_process->write(m_tool->input.toLocal8Bit());
        }
        m_process->closeWriteChannel();
    });

    const QStringList args = KShell::splitArgs(m_tool->arguments);
    m_process->start(m_tool->executable, args);
}

void KateToolRunner::waitForFinished()
{
    m_process->waitForFinished();
}

/**
 * Convert input from local encoding to text with only \n line endings
 * @param localOutput local output from tool to convert
 * @return string with proper \n line endings
 */
static QString textFromLocal(const QByteArray &localOutput)
{
    // normalize line endings, to e.g. catch issues with \r\n
    // see bug 436753
    QString s = QString::fromLocal8Bit(localOutput);
    static const QRegularExpression lineEndings(QStringLiteral("\r\n?"));
    s.replace(lineEndings, QStringLiteral("\n"));
    return s;
}

QString KateToolRunner::outputData() const
{
    return textFromLocal(m_stdout);
}

QString KateToolRunner::errorData() const
{
    return textFromLocal(m_stderr);
}

// kate: space-indent on; indent-width 4; replace-tabs on;
