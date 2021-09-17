/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KTEXTEDITOR_KATE_EXTERNALTOOLS_COMMAND_H
#define KTEXTEDITOR_KATE_EXTERNALTOOLS_COMMAND_H

#include <KTextEditor/Command>

class KateExternalToolsPlugin;
class KateExternalTool;

/**
 * Helper class that registers and executes the respective external tool.
 */
class KateExternalToolsCommand : public KTextEditor::Command
{
public:
    KateExternalToolsCommand(KateExternalToolsPlugin *plugin);
    virtual ~KateExternalToolsCommand() = default;

public:
    bool exec(KTextEditor::View *view, const QString &cmd, QString &msg, const KTextEditor::Range &range = KTextEditor::Range::invalid()) override;
    bool help(KTextEditor::View *view, const QString &cmd, QString &msg) override;

private:
    void runTool(KateExternalTool &tool, KTextEditor::View *view);

private:
    KateExternalToolsPlugin *m_plugin;
};

#endif // KTEXTEDITOR_KATE_EXTERNALTOOLS_COMMAND_H

// kate: space-indent on; indent-width 4; replace-tabs on;
