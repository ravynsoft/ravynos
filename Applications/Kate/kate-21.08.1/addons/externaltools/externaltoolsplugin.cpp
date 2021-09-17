/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "externaltoolsplugin.h"

#include "kateexternaltool.h"
#include "kateexternaltoolscommand.h"
#include "kateexternaltoolsconfigwidget.h"
#include "kateexternaltoolsview.h"
#include "katetoolrunner.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>
#include <QAction>
#include <kparts/part.h>

#include <KAboutData>
#include <KAuthorized>
#include <KConfig>
#include <KConfigGroup>
#include <KPluginFactory>
#include <KXMLGUIFactory>

#include <QClipboard>
#include <QGuiApplication>

static QVector<KateExternalTool> readDefaultTools()
{
    QVector<KateExternalTool> tools;
    KConfig systemConfig(QStringLiteral("defaultexternaltoolsrc"));
    KConfigGroup config(&systemConfig, "Global");
    const int toolCount = config.readEntry("tools", 0);
    for (int i = 0; i < toolCount; ++i) {
        config = KConfigGroup(&systemConfig, QStringLiteral("Tool %1").arg(i));

        KateExternalTool t;
        t.load(config);
        tools.push_back(t);
    }
    return tools;
}

K_PLUGIN_FACTORY_WITH_JSON(KateExternalToolsFactory, "externaltoolsplugin.json", registerPlugin<KateExternalToolsPlugin>();)

KateExternalToolsPlugin::KateExternalToolsPlugin(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
{
    // read built-in external tools from compiled-in resource file
    m_defaultTools = readDefaultTools();

    // load config from disk
    reload();
}

KateExternalToolsPlugin::~KateExternalToolsPlugin()
{
    clearTools();
}

QObject *KateExternalToolsPlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    KateExternalToolsPluginView *view = new KateExternalToolsPluginView(mainWindow, this);
    connect(this, &KateExternalToolsPlugin::externalToolsChanged, view, &KateExternalToolsPluginView::rebuildMenu);
    return view;
}

void KateExternalToolsPlugin::clearTools()
{
    delete m_command;
    m_command = nullptr;
    m_commands.clear();
    qDeleteAll(m_tools);
    m_tools.clear();
}

void KateExternalToolsPlugin::reload()
{
    clearTools();

    KConfig _config(QStringLiteral("externaltools"), KConfig::NoGlobals, QStandardPaths::ApplicationsLocation);
    KConfigGroup config(&_config, "Global");
    const int toolCount = config.readEntry("tools", 0);
    const bool firstStart = config.readEntry("firststart", true);

    if (!firstStart || toolCount > 0) {
        // read user config
        for (int i = 0; i < toolCount; ++i) {
            config = KConfigGroup(&_config, QStringLiteral("Tool %1").arg(i));

            auto t = new KateExternalTool();
            t->load(config);
            m_tools.push_back(t);
        }
    } else {
        // first start -> use system config
        for (const auto &tool : m_defaultTools) {
            m_tools.push_back(new KateExternalTool(tool));
        }
    }

    // FIXME test for a command name first!
    for (auto tool : m_tools) {
        if (tool->hasexec && (!tool->cmdname.isEmpty())) {
            m_commands.push_back(tool->cmdname);
        }
    }

    if (KAuthorized::authorizeAction(QStringLiteral("shell_access"))) {
        m_command = new KateExternalToolsCommand(this);
    }

    Q_EMIT externalToolsChanged();
}

QStringList KateExternalToolsPlugin::commands() const
{
    return m_commands;
}

const KateExternalTool *KateExternalToolsPlugin::toolForCommand(const QString &cmd) const
{
    for (auto tool : m_tools) {
        if (tool->cmdname == cmd) {
            return tool;
        }
    }
    return nullptr;
}

const QVector<KateExternalTool *> &KateExternalToolsPlugin::tools() const
{
    return m_tools;
}

QVector<KateExternalTool> KateExternalToolsPlugin::defaultTools() const
{
    return m_defaultTools;
}

void KateExternalToolsPlugin::runTool(const KateExternalTool &tool, KTextEditor::View *view)
{
    // expand the macros in command if any,
    // and construct a command with an absolute path
    auto mw = view->mainWindow();

    // save documents if requested
    if (tool.saveMode == KateExternalTool::SaveMode::CurrentDocument) {
        // only save if modified, to avoid unnecessary recompiles
        if (view->document()->isModified()) {
            view->document()->save();
        }
    } else if (tool.saveMode == KateExternalTool::SaveMode::AllDocuments) {
        const auto guiClients = mw->guiFactory()->clients();
        for (KXMLGUIClient *client : guiClients) {
            if (QAction *a = client->actionCollection()->action(QStringLiteral("file_save_all"))) {
                a->trigger();
                break;
            }
        }
    }

    // copy tool
    std::unique_ptr<KateExternalTool> copy(new KateExternalTool(tool));

    // clear previous toolview data
    auto pluginView = viewForMainWindow(mw);
    pluginView->clearToolView();

    // expand macros
    auto editor = KTextEditor::Editor::instance();
    editor->expandText(copy->executable, view, copy->executable);
    editor->expandText(copy->arguments, view, copy->arguments);
    editor->expandText(copy->workingDir, view, copy->workingDir);
    editor->expandText(copy->input, view, copy->input);

    const QString messageText = copy->input.isEmpty() ? i18n("Running %1: %2 %3", copy->name, copy->executable, copy->arguments)
                                                      : i18n("Running %1: %2 %3 with input %4", copy->name, copy->executable, copy->arguments, tool.input);

    // use generic output view for status
    QVariantMap genericMessage;
    genericMessage.insert(QStringLiteral("type"), QStringLiteral("Info"));
    genericMessage.insert(QStringLiteral("category"), i18n("External Tools"));
    genericMessage.insert(QStringLiteral("categoryIcon"), QIcon::fromTheme(QStringLiteral("system-run")));
    genericMessage.insert(QStringLiteral("text"), messageText);
    Q_EMIT pluginView->message(genericMessage);

    // Allocate runner on heap such that it lives as long as the child
    // process is running and does not block the main thread.
    auto runner = new KateToolRunner(std::move(copy), view, this);

    // use QueuedConnection, since handleToolFinished deletes the runner
    connect(runner, &KateToolRunner::toolFinished, this, &KateExternalToolsPlugin::handleToolFinished, Qt::QueuedConnection);
    runner->run();
}

void KateExternalToolsPlugin::handleToolFinished(KateToolRunner *runner, int exitCode, bool crashed)
{
    auto view = runner->view();
    if (view && !runner->outputData().isEmpty()) {
        switch (runner->tool()->outputMode) {
        case KateExternalTool::OutputMode::InsertAtCursor: {
            KTextEditor::Document::EditingTransaction transaction(view->document());
            view->removeSelection();
            view->insertText(runner->outputData());
            break;
        }
        case KateExternalTool::OutputMode::ReplaceSelectedText: {
            KTextEditor::Document::EditingTransaction transaction(view->document());
            view->removeSelectionText();
            view->insertText(runner->outputData());
            break;
        }
        case KateExternalTool::OutputMode::ReplaceCurrentDocument: {
            KTextEditor::Document::EditingTransaction transaction(view->document());
            auto cursor = view->cursorPosition();
            view->document()->clear();
            view->insertText(runner->outputData());
            view->setCursorPosition(cursor);
            break;
        }
        case KateExternalTool::OutputMode::AppendToCurrentDocument: {
            view->document()->insertText(view->document()->documentEnd(), runner->outputData());
            break;
        }
        case KateExternalTool::OutputMode::InsertInNewDocument: {
            auto mainWindow = view->mainWindow();
            auto newView = mainWindow->openUrl({});
            newView->insertText(runner->outputData());
            mainWindow->activateView(newView->document());
            break;
        }
        case KateExternalTool::OutputMode::CopyToClipboard: {
            QGuiApplication::clipboard()->setText(runner->outputData());
            break;
        }
        default:
            break;
        }
    }

    if (view && runner->tool()->reload) {
        // updates-enabled trick: avoid some flicker
        const bool wereUpdatesEnabled = view->updatesEnabled();
        view->setUpdatesEnabled(false);
        view->document()->documentReload();
        view->setUpdatesEnabled(wereUpdatesEnabled);
    }

    KateExternalToolsPluginView *pluginView = runner->view() ? viewForMainWindow(runner->view()->mainWindow()) : nullptr;
    if (pluginView) {
        bool hasOutputInPane = false;
        if (runner->tool()->outputMode == KateExternalTool::OutputMode::DisplayInPane) {
            pluginView->setOutputData(runner->outputData());
            hasOutputInPane = !runner->outputData().isEmpty();
        }

        QString messageBody;
        QString messageType = QStringLiteral("Info");
        if (!runner->errorData().isEmpty()) {
            messageBody += i18n("Data written to stderr:\n");
            messageBody += runner->errorData();
            messageBody += QStringLiteral("\n");
            messageType = QStringLiteral("Warning");
        }
        if (crashed || exitCode != 0) {
            messageType = QStringLiteral("Error");
        }

        // print crash or exit code
        if (crashed) {
            messageBody += i18n("%1 crashed", runner->tool()->translatedName());
        } else if (exitCode != 0) {
            messageBody += i18n("%1 finished with exit code %2", runner->tool()->translatedName(), exitCode);
        }

        // use generic output view for status
        QVariantMap genericMessage;
        genericMessage.insert(QStringLiteral("type"), messageType);
        genericMessage.insert(QStringLiteral("category"), i18n("External Tools"));
        genericMessage.insert(QStringLiteral("categoryIcon"), QIcon::fromTheme(QStringLiteral("system-run")));
        genericMessage.insert(QStringLiteral("text"), messageBody);
        Q_EMIT pluginView->message(genericMessage);

        // on successful execution => show output
        // otherwise the global output pane settings will ensure we see the error output
        if (!(crashed || exitCode != 0) && hasOutputInPane) {
            pluginView->showToolView();
        }
    }

    delete runner;
}

int KateExternalToolsPlugin::configPages() const
{
    return 1;
}

KTextEditor::ConfigPage *KateExternalToolsPlugin::configPage(int number, QWidget *parent)
{
    if (number == 0) {
        return new KateExternalToolsConfigWidget(parent, this);
    }
    return nullptr;
}

void KateExternalToolsPlugin::registerPluginView(KateExternalToolsPluginView *view)
{
    Q_ASSERT(!m_views.contains(view));
    m_views.push_back(view);
}

void KateExternalToolsPlugin::unregisterPluginView(KateExternalToolsPluginView *view)
{
    Q_ASSERT(m_views.contains(view));
    m_views.removeAll(view);
}

KateExternalToolsPluginView *KateExternalToolsPlugin::viewForMainWindow(KTextEditor::MainWindow *mainWindow) const
{
    for (auto view : m_views) {
        if (view->mainWindow() == mainWindow) {
            return view;
        }
    }
    return nullptr;
}

#include "externaltoolsplugin.moc"

// kate: space-indent on; indent-width 4; replace-tabs on;
