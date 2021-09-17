/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kateexternaltoolscommand.h"
#include "externaltoolsplugin.h"
#include "kateexternaltool.h"

#include <KLocalizedString>

KateExternalToolsCommand::KateExternalToolsCommand(KateExternalToolsPlugin *plugin)
    : KTextEditor::Command(plugin->commands())
    , m_plugin(plugin)
{
}

bool KateExternalToolsCommand::exec(KTextEditor::View *view, const QString &cmd, QString &msg, const KTextEditor::Range &range)
{
    Q_UNUSED(msg)
    Q_UNUSED(range)

    const QString command = cmd.trimmed();
    const auto tool = m_plugin->toolForCommand(command);
    if (tool) {
        m_plugin->runTool(*tool, view);
        return true;
    }
    return false;
}

bool KateExternalToolsCommand::help(KTextEditor::View *, const QString &cmd, QString &msg)
{
    const QString command = cmd.trimmed();
    const auto tool = m_plugin->toolForCommand(command);
    if (tool) {
        msg = i18n("Starts the external tool '%1'", tool->name);
        return true;
    }

    return false;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
