/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "gitutils.h"

#include <QDateTime>
#include <QDebug>
#include <QProcess>

bool GitUtils::isGitRepo(const QString &repo)
{
    QProcess git;
    git.setWorkingDirectory(repo);
    QStringList args{QStringLiteral("rev-parse"), QStringLiteral("--is-inside-work-tree")};
    git.start(QStringLiteral("git"), args, QProcess::ReadOnly);
    if (git.waitForStarted() && git.waitForFinished(-1)) {
        return git.readAll().trimmed() == "true";
    }
    return false;
}

std::optional<QString> GitUtils::getDotGitPath(const QString &repo)
{
    /* This call is intentionally blocking because we need git path for everything else */
    QProcess git;
    git.setProgram(QStringLiteral("git"));
    git.setWorkingDirectory(repo);
    git.setArguments({QStringLiteral("rev-parse"), QStringLiteral("--absolute-git-dir")});
    git.start(QProcess::ReadOnly);
    if (git.waitForStarted() && git.waitForFinished(-1)) {
        if (git.exitStatus() != QProcess::NormalExit || git.exitCode() != 0) {
            return std::nullopt;
        }
        QString dotGitPath = QString::fromUtf8(git.readAllStandardOutput());
        if (dotGitPath.endsWith(QLatin1String("\n"))) {
            dotGitPath.remove(QLatin1String(".git\n"));
        } else {
            dotGitPath.remove(QLatin1String(".git"));
        }
        return dotGitPath;
    }
    return std::nullopt;
}

QString GitUtils::getCurrentBranchName(const QString &repo)
{
    // clang-format off
    QStringList argsList[3] =
    {
        {QStringLiteral("symbolic-ref"), QStringLiteral("--short"), QStringLiteral("HEAD")},
        {QStringLiteral("describe"), QStringLiteral("--exact-match"), QStringLiteral("HEAD")},
        {QStringLiteral("rev-parse"), QStringLiteral("--short"), QStringLiteral("HEAD")}
    };
    // clang-format on

    for (int i = 0; i < 3; ++i) {
        QProcess git;
        git.setWorkingDirectory(repo);
        git.start(QStringLiteral("git"), argsList[i], QProcess::ReadOnly);
        if (git.waitForStarted() && git.waitForFinished(-1)) {
            if (git.exitStatus() == QProcess::NormalExit && git.exitCode() == 0) {
                return QString::fromUtf8(git.readAllStandardOutput().trimmed());
            }
        }
    }

    // give up
    return QString();
}

GitUtils::CheckoutResult GitUtils::checkoutBranch(const QString &repo, const QString &branch)
{
    QProcess git;
    git.setWorkingDirectory(repo);
    QStringList args{QStringLiteral("checkout"), branch};
    git.start(QStringLiteral("git"), args, QProcess::ReadOnly);
    CheckoutResult res;
    res.branch = branch;
    if (git.waitForStarted() && git.waitForFinished(-1)) {
        res.returnCode = git.exitCode();
        res.error = QString::fromUtf8(git.readAllStandardError());
    }
    return res;
}

GitUtils::CheckoutResult GitUtils::checkoutNewBranch(const QString &repo, const QString &newBranch, const QString &fromBranch)
{
    QProcess git;
    git.setWorkingDirectory(repo);
    QStringList args{QStringLiteral("checkout"), QStringLiteral("-q"), QStringLiteral("-b"), newBranch};
    if (!fromBranch.isEmpty()) {
        args.append(fromBranch);
    }
    git.start(QStringLiteral("git"), args, QProcess::ReadOnly);
    CheckoutResult res;
    res.branch = newBranch;
    if (git.waitForStarted() && git.waitForFinished(-1)) {
        res.returnCode = git.exitCode();
        res.error = QString::fromUtf8(git.readAllStandardError());
    }
    return res;
}

static GitUtils::Branch parseLocalBranch(const QString &raw)
{
    static const int len = QStringLiteral("refs/heads/").length();
    return GitUtils::Branch{raw.mid(len), QString(), GitUtils::Head};
}

static GitUtils::Branch parseRemoteBranch(const QString &raw)
{
    static const int len = QStringLiteral("refs/remotes/").length();
    int indexofRemote = raw.indexOf(QLatin1Char('/'), len);
    return GitUtils::Branch{raw.mid(len), raw.mid(len, indexofRemote - len), GitUtils::Remote};
}

QVector<GitUtils::Branch> GitUtils::getAllBranchesAndTags(const QString &repo, RefType ref)
{
    // git for-each-ref --format '%(refname)' --sort=-committerdate ...
    QProcess git;
    git.setWorkingDirectory(repo);

    QStringList args{QStringLiteral("for-each-ref"), QStringLiteral("--format"), QStringLiteral("%(refname)"), QStringLiteral("--sort=-committerdate")};
    if (ref & RefType::Head) {
        args.append(QStringLiteral("refs/heads"));
    }
    if (ref & RefType::Remote) {
        args.append(QStringLiteral("refs/remotes"));
    }
    if (ref & RefType::Tag) {
        args.append(QStringLiteral("refs/tags"));
        args.append(QStringLiteral("--sort=-taggerdate"));
    }

    git.start(QStringLiteral("git"), args, QProcess::ReadOnly);
    QVector<Branch> branches;
    if (git.waitForStarted() && git.waitForFinished(-1)) {
        QString gitout = QString::fromUtf8(git.readAllStandardOutput());
        QStringList out = gitout.split(QLatin1Char('\n'));

        branches.reserve(out.size());
        // clang-format off
        for (const auto &o : out) {
            if (ref & Head && o.startsWith(QLatin1String("refs/heads"))) {
                branches.append(parseLocalBranch(o));
            } else if (ref & Remote && o.startsWith(QLatin1String("refs/remotes"))) {
                branches.append(parseRemoteBranch(o));
            } else if (ref & Tag && o.startsWith(QLatin1String("refs/tags/"))) {
                static const int len = QStringLiteral("refs/tags/").length();
                branches.append({o.mid(len), {}, RefType::Tag});
            }
        }
        // clang-format on
    }

    return branches;
}

QVector<GitUtils::Branch> GitUtils::getAllBranches(const QString &repo)
{
    return getAllBranchesAndTags(repo, static_cast<RefType>(RefType::Head | RefType::Remote));
}

std::pair<QString, QString> GitUtils::getLastCommitMessage(const QString &repo)
{
    // git log -1 --pretty=%B
    QProcess git;
    git.setWorkingDirectory(repo);
    const QStringList args{QStringLiteral("log"), QStringLiteral("-1"), QStringLiteral("--pretty=%B")};
    git.start(QStringLiteral("git"), args, QProcess::ReadOnly);
    if (git.waitForStarted() && git.waitForFinished(-1)) {
        if (git.exitCode() != 0 || git.exitStatus() != QProcess::NormalExit) {
            return {};
        }

        QList<QByteArray> output = git.readAllStandardOutput().split('\n');
        if (output.isEmpty()) {
            return {};
        }

        QString msg = QString::fromUtf8(output.at(0));
        QString desc;
        if (output.size() > 1) {
            desc = std::accumulate(output.cbegin() + 1, output.cend(), QString::fromUtf8(output.at(1)), [](const QString &line, const QByteArray &ba) {
                return QString(line + QString::fromUtf8(ba) + QStringLiteral("\n"));
            });
            desc = desc.trimmed();
        }
        return {msg, desc};
    }
    return {};
}
