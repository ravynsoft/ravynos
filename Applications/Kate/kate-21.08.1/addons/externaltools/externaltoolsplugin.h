/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KTEXTEDITOR_EXTERNALTOOLS_PLUGIN_H
#define KTEXTEDITOR_EXTERNALTOOLS_PLUGIN_H

#include <KTextEditor/Plugin>
#include <QVector>

namespace KTextEditor
{
class View;
}

class KateExternalToolsMenuAction;
class KateExternalToolsPluginView;
class KateExternalToolsCommand;
class KateExternalTool;
class KateToolRunner;

class KateExternalToolsPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit KateExternalToolsPlugin(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());
    virtual ~KateExternalToolsPlugin();

    /**
     * Reimplemented to return the number of config pages, in this case 1.
     */
    int configPages() const override;

    /**
     * Reimplemented to return the KateExternalToolConfigWidget for number==0.
     */
    KTextEditor::ConfigPage *configPage(int number = 0, QWidget *parent = nullptr) override;

    /**
     * Reimplemented to instantiate a PluginView for each MainWindow.
     */
    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

    /**
     * Deletes all tools.
     */
    void clearTools();

    /**
     * Reloads the external tools from disk.
     */
    void reload();

    /**
     * Returns a list of KTextEDitor::Command strings. This is needed by
     * the KateExternalToolsCommand constructor to pass the list of commands to
     * the KTextEditor::Editor.
     */
    QStringList commands() const;

    /**
     * Returns the KateExternalTool for a specific command line command 'cmd.
     */
    const KateExternalTool *toolForCommand(const QString &cmd) const;

    /**
     * Returns a list of all existing external tools.
     */
    const QVector<KateExternalTool *> &tools() const;

    /**
     * Returns the list of external tools that are shipped by default with
     * the external tools plugin.
     */
    QVector<KateExternalTool> defaultTools() const;

    /**
     * Executes the tool based on the view as current document.
     */
    void runTool(const KateExternalTool &tool, KTextEditor::View *view);

Q_SIGNALS:
    /**
     * This signal is emitted whenever the external tools change.
     * This is typically the case when external tools were modified,
     * added, or removed via the config page.
     */
    void externalToolsChanged();

public:
    /**
     * Called by the KateExternalToolsPluginView to register itself.
     */
    void registerPluginView(KateExternalToolsPluginView *view);

    /**
     * Called by the KateExternalToolsPluginView to unregister itself.
     */
    void unregisterPluginView(KateExternalToolsPluginView *view);

    /**
     * Returns the KateExternalToolsPluginView for the given mainWindow.
     */
    KateExternalToolsPluginView *viewForMainWindow(KTextEditor::MainWindow *mainWindow) const;

private:
    QVector<KateExternalTool> m_defaultTools;
    QVector<KateExternalToolsPluginView *> m_views;
    QVector<KateExternalTool *> m_tools;
    QStringList m_commands;
    KateExternalToolsCommand *m_command = nullptr;

private Q_SLOTS:
    /**
     * Called whenever an external tool is done.
     */
    void handleToolFinished(KateToolRunner *runner, int exitCode, bool crashed);
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
