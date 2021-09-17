/*
    SPDX-FileCopyrightText: 2020 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KateSearchCommand.h"
#include "MatchModel.h"

#include <KLocalizedString>

KateSearchCommand::KateSearchCommand(QObject *parent)
    : KTextEditor::Command(QStringList() << QStringLiteral("grep") << QStringLiteral("newGrep") << QStringLiteral("search") << QStringLiteral("newSearch")
                                         << QStringLiteral("pgrep") << QStringLiteral("newPGrep") << QStringLiteral("preg"),
                           parent)
{
}

void KateSearchCommand::setBusy(bool busy)
{
    m_busy = busy;
}

bool KateSearchCommand::exec(KTextEditor::View * /*view*/, const QString &cmd, QString & /*msg*/, const KTextEditor::Range &)
{
    if (m_busy) {
        return false;
    }
    // create a list of args
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QStringList args(cmd.split(QLatin1Char(' '), QString::KeepEmptyParts));
#else
    QStringList args(cmd.split(QLatin1Char(' '), Qt::KeepEmptyParts));
#endif
    QString command = args.takeFirst();
    QString searchText = args.join(QLatin1Char(' '));

    if (command == QLatin1String("grep") || command == QLatin1String("newGrep")) {
        Q_EMIT setSearchPlace(MatchModel::Folder);
        Q_EMIT setCurrentFolder();
        if (command == QLatin1String("newGrep")) {
            Q_EMIT newTab();
        }
    }

    else if (command == QLatin1String("search") || command == QLatin1String("newSearch")) {
        Q_EMIT setSearchPlace(MatchModel::OpenFiles);
        if (command == QLatin1String("newSearch")) {
            Q_EMIT newTab();
        }
    }

    else if (command == QLatin1String("pgrep") || command == QLatin1String("newPGrep")) {
        Q_EMIT setSearchPlace(MatchModel::Project);
        if (command == QLatin1String("newPGrep")) {
            Q_EMIT newTab();
        }
    }

    /**
     * preg command
     * - Uses regex always
     * - Is case insensitive
     * - Will expand the tree on search completion if -e is used
     */
    else if (command == QLatin1String("preg")) {
        Q_EMIT setSearchPlace(MatchModel::Project);
        Q_EMIT setRegexMode(true);
        Q_EMIT setCaseInsensitive(true);
        Q_EMIT setExpandResults(true);
        Q_EMIT newTab();
    }

    Q_EMIT setSearchString(searchText);
    Q_EMIT startSearch();

    return true;
}

bool KateSearchCommand::help(KTextEditor::View * /*view*/, const QString &cmd, QString &msg)
{
    if (cmd.startsWith(QLatin1String("grep"))) {
        msg = i18n("Usage: grep [pattern to search for in folder]");
    } else if (cmd.startsWith(QLatin1String("newGrep"))) {
        msg = i18n("Usage: newGrep [pattern to search for in folder]");
    }

    else if (cmd.startsWith(QLatin1String("search"))) {
        msg = i18n("Usage: search [pattern to search for in open files]");
    } else if (cmd.startsWith(QLatin1String("newSearch"))) {
        msg = i18n("Usage: search [pattern to search for in open files]");
    }

    else if (cmd.startsWith(QLatin1String("pgrep"))) {
        msg = i18n("Usage: pgrep [pattern to search for in current project]");
    } else if (cmd.startsWith(QLatin1String("newPGrep"))) {
        msg = i18n("Usage: newPGrep [pattern to search for in current project]");
    }

    else if (cmd.startsWith(QLatin1String("preg"))) {
        msg = i18n("Usage: preg [regex pattern to search for in current project]");
    }

    return true;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
