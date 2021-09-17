/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "pushpulldialog.h"

#include <QProcess>

PushPullDialog::PushPullDialog(QWidget *mainWindow, const QString &repoPath)
    : QuickDialog(nullptr, mainWindow)
    , m_repo(repoPath)
{
}

void PushPullDialog::openDialog(PushPullDialog::Mode m)
{
    if (m == Push) {
        m_lineEdit.setText(buildPushString());
    } else if (m == Pull) {
        m_lineEdit.setText(buildPullString());
    }
    exec();
}

/**
 * This is not for display, hence not reusing gitutils here
 */
static QString currentBranchName(const QString &repo)
{
    QProcess git;
    git.setWorkingDirectory(repo);

    QStringList args{QStringLiteral("symbolic-ref"), QStringLiteral("--short"), QStringLiteral("HEAD")};

    git.start(QStringLiteral("git"), args, QProcess::ReadOnly);
    if (git.waitForStarted() && git.waitForFinished(-1)) {
        if (git.exitStatus() == QProcess::NormalExit && git.exitCode() == 0) {
            return QString::fromUtf8(git.readAllStandardOutput().trimmed());
        }
    }
    // give up
    return QString();
}

static QStringList remotesList(const QString &repo)
{
    QProcess git;
    git.setWorkingDirectory(repo);

    QStringList args{QStringLiteral("remote")};

    git.start(QStringLiteral("git"), args, QProcess::ReadOnly);
    if (git.waitForStarted() && git.waitForFinished(-1)) {
        if (git.exitStatus() == QProcess::NormalExit && git.exitCode() == 0) {
            return QString::fromUtf8(git.readAllStandardOutput()).split(QLatin1Char('\n'));
        }
    }
    return {};
}

QString PushPullDialog::buildPushString()
{
    auto br = currentBranchName(m_repo);
    if (br.isEmpty()) {
        return QStringLiteral("git push");
    }

    auto remotes = remotesList(m_repo);
    if (!remotes.contains(QStringLiteral("origin"))) {
        return QStringLiteral("git push");
    }

    return QStringLiteral("git push %1 %2").arg(QStringLiteral("origin")).arg(br);
}

QString PushPullDialog::buildPullString()
{
    auto br = currentBranchName(m_repo);
    if (br.isEmpty()) {
        return QStringLiteral("git pull");
    }

    auto remotes = remotesList(m_repo);
    if (!remotes.contains(QStringLiteral("origin"))) {
        return QStringLiteral("git pull");
    }

    return QStringLiteral("git pull %1 %2").arg(QStringLiteral("origin")).arg(br);
}

void PushPullDialog::slotReturnPressed()
{
    if (!m_lineEdit.text().isEmpty()) {
        auto args = m_lineEdit.text().split(QLatin1Char(' '));
        if (args.first() == QStringLiteral("git")) {
            args.pop_front();
            Q_EMIT runGitCommand(args);
        }
    }

    hide();
}
