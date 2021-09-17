/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>

    SPDX-License-Identifier: MIT
*/

#include "lspclientplugin.h"
#include "lspclientconfigpage.h"
#include "lspclientpluginview.h"

#include "lspclient_debug.h"

#include <KConfigGroup>
#include <KDirWatch>
#include <KPluginFactory>
#include <KSharedConfig>

#include <QDir>
#include <QStandardPaths>

static const QString CONFIG_LSPCLIENT{QStringLiteral("lspclient")};
static const QString CONFIG_SYMBOL_DETAILS{QStringLiteral("SymbolDetails")};
static const QString CONFIG_SYMBOL_TREE{QStringLiteral("SymbolTree")};
static const QString CONFIG_SYMBOL_EXPAND{QStringLiteral("SymbolExpand")};
static const QString CONFIG_SYMBOL_SORT{QStringLiteral("SymbolSort")};
static const QString CONFIG_COMPLETION_DOC{QStringLiteral("CompletionDocumentation")};
static const QString CONFIG_REFERENCES_DECLARATION{QStringLiteral("ReferencesDeclaration")};
static const QString CONFIG_COMPLETION_PARENS{QStringLiteral("CompletionParens")};
static const QString CONFIG_AUTO_HOVER{QStringLiteral("AutoHover")};
static const QString CONFIG_TYPE_FORMATTING{QStringLiteral("TypeFormatting")};
static const QString CONFIG_INCREMENTAL_SYNC{QStringLiteral("IncrementalSync")};
static const QString CONFIG_HIGHLIGHT_GOTO{QStringLiteral("HighlightGoto")};
static const QString CONFIG_DIAGNOSTICS{QStringLiteral("Diagnostics")};
static const QString CONFIG_DIAGNOSTICS_HIGHLIGHT{QStringLiteral("DiagnosticsHighlight")};
static const QString CONFIG_DIAGNOSTICS_MARK{QStringLiteral("DiagnosticsMark")};
static const QString CONFIG_DIAGNOSTICS_HOVER{QStringLiteral("DiagnosticsHover")};
static const QString CONFIG_DIAGNOSTICS_SIZE{QStringLiteral("DiagnosticsSize")};
static const QString CONFIG_MESSAGES{QStringLiteral("Messages")};
static const QString CONFIG_SERVER_CONFIG{QStringLiteral("ServerConfiguration")};
static const QString CONFIG_SEMANTIC_HIGHLIGHTING{QStringLiteral("SemanticHighlighting")};
static const QString CONFIG_SIGNATURE_HELP{QStringLiteral("SignatureHelp")};

K_PLUGIN_FACTORY_WITH_JSON(LSPClientPluginFactory, "lspclientplugin.json", registerPlugin<LSPClientPlugin>();)

LSPClientPlugin::LSPClientPlugin(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
    , m_settingsPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QStringLiteral("/lspclient"))
    , m_defaultConfigPath(QUrl::fromLocalFile(m_settingsPath + QStringLiteral("/settings.json")))
{
    // ensure settings path exist, for e.g. local settings.json
    QDir().mkpath(m_settingsPath);

    /**
     * handle plugin verbosity
     * the m_debugMode will be used to e.g. set debug level for started clangd, too
     */
    m_debugMode = (qgetenv("LSPCLIENT_DEBUG") == QByteArray("1"));
    if (!m_debugMode) {
        QLoggingCategory::setFilterRules(QStringLiteral("katelspclientplugin.debug=false\nkatelspclientplugin.info=false"));
    } else {
        QLoggingCategory::setFilterRules(QStringLiteral("katelspclientplugin.debug=true\nkatelspclientplugin.info=true"));
    }

    readConfig();
}

LSPClientPlugin::~LSPClientPlugin()
{
}

QObject *LSPClientPlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    return LSPClientPluginView::new_(this, mainWindow);
}

int LSPClientPlugin::configPages() const
{
    return 1;
}

KTextEditor::ConfigPage *LSPClientPlugin::configPage(int number, QWidget *parent)
{
    if (number != 0) {
        return nullptr;
    }

    return new LSPClientConfigPage(parent, this);
}

void LSPClientPlugin::readConfig()
{
    KConfigGroup config(KSharedConfig::openConfig(), CONFIG_LSPCLIENT);
    m_symbolDetails = config.readEntry(CONFIG_SYMBOL_DETAILS, false);
    m_symbolTree = config.readEntry(CONFIG_SYMBOL_TREE, true);
    m_symbolExpand = config.readEntry(CONFIG_SYMBOL_EXPAND, true);
    m_symbolSort = config.readEntry(CONFIG_SYMBOL_SORT, false);
    m_complDoc = config.readEntry(CONFIG_COMPLETION_DOC, true);
    m_refDeclaration = config.readEntry(CONFIG_REFERENCES_DECLARATION, true);
    m_complParens = config.readEntry(CONFIG_COMPLETION_PARENS, true);
    m_autoHover = config.readEntry(CONFIG_AUTO_HOVER, true);
    m_onTypeFormatting = config.readEntry(CONFIG_TYPE_FORMATTING, false);
    m_incrementalSync = config.readEntry(CONFIG_INCREMENTAL_SYNC, false);
    m_highlightGoto = config.readEntry(CONFIG_HIGHLIGHT_GOTO, true);
    m_diagnostics = config.readEntry(CONFIG_DIAGNOSTICS, true);
    m_diagnosticsHighlight = config.readEntry(CONFIG_DIAGNOSTICS_HIGHLIGHT, true);
    m_diagnosticsMark = config.readEntry(CONFIG_DIAGNOSTICS_MARK, true);
    m_diagnosticsHover = config.readEntry(CONFIG_DIAGNOSTICS_HOVER, true);
    m_diagnosticsSize = config.readEntry(CONFIG_DIAGNOSTICS_SIZE, 1024);
    m_messages = config.readEntry(CONFIG_MESSAGES, true);
    m_configPath = config.readEntry(CONFIG_SERVER_CONFIG, QUrl());
    m_semanticHighlighting = config.readEntry(CONFIG_SEMANTIC_HIGHLIGHTING, false);
    m_signatureHelp = config.readEntry(CONFIG_SIGNATURE_HELP, true);

    Q_EMIT update();
}

void LSPClientPlugin::writeConfig() const
{
    KConfigGroup config(KSharedConfig::openConfig(), CONFIG_LSPCLIENT);
    config.writeEntry(CONFIG_SYMBOL_DETAILS, m_symbolDetails);
    config.writeEntry(CONFIG_SYMBOL_TREE, m_symbolTree);
    config.writeEntry(CONFIG_SYMBOL_EXPAND, m_symbolExpand);
    config.writeEntry(CONFIG_SYMBOL_SORT, m_symbolSort);
    config.writeEntry(CONFIG_COMPLETION_DOC, m_complDoc);
    config.writeEntry(CONFIG_REFERENCES_DECLARATION, m_refDeclaration);
    config.writeEntry(CONFIG_COMPLETION_PARENS, m_complParens);
    config.writeEntry(CONFIG_AUTO_HOVER, m_autoHover);
    config.writeEntry(CONFIG_TYPE_FORMATTING, m_onTypeFormatting);
    config.writeEntry(CONFIG_INCREMENTAL_SYNC, m_incrementalSync);
    config.writeEntry(CONFIG_HIGHLIGHT_GOTO, m_highlightGoto);
    config.writeEntry(CONFIG_DIAGNOSTICS, m_diagnostics);
    config.writeEntry(CONFIG_DIAGNOSTICS_HIGHLIGHT, m_diagnosticsHighlight);
    config.writeEntry(CONFIG_DIAGNOSTICS_MARK, m_diagnosticsMark);
    config.writeEntry(CONFIG_DIAGNOSTICS_HOVER, m_diagnosticsHover);
    config.writeEntry(CONFIG_DIAGNOSTICS_SIZE, m_diagnosticsSize);
    config.writeEntry(CONFIG_MESSAGES, m_messages);
    config.writeEntry(CONFIG_SERVER_CONFIG, m_configPath);
    config.writeEntry(CONFIG_SEMANTIC_HIGHLIGHTING, m_semanticHighlighting);
    config.writeEntry(CONFIG_SIGNATURE_HELP, m_signatureHelp);

    Q_EMIT update();
}

#include "lspclientplugin.moc"
