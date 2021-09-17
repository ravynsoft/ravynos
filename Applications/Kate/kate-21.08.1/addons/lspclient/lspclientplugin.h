/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>

    SPDX-License-Identifier: MIT
*/

#ifndef LSPCLIENTPLUGIN_H
#define LSPCLIENTPLUGIN_H

#include <QMap>
#include <QUrl>
#include <QVariant>

#include <KTextEditor/Plugin>

class LSPClientPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit LSPClientPlugin(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());
    ~LSPClientPlugin() override;

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

    int configPages() const override;
    KTextEditor::ConfigPage *configPage(int number = 0, QWidget *parent = nullptr) override;

    void readConfig();
    void writeConfig() const;

    // path for local setting files, auto-created on load
    const QString m_settingsPath;

    // default config path
    const QUrl m_defaultConfigPath;

    // settings
    bool m_symbolDetails = false;
    bool m_symbolExpand = false;
    bool m_symbolTree = false;
    bool m_symbolSort = false;
    bool m_complDoc = false;
    bool m_refDeclaration = false;
    bool m_complParens = false;
    bool m_diagnostics = false;
    bool m_diagnosticsHighlight = false;
    bool m_diagnosticsMark = false;
    bool m_diagnosticsHover = false;
    unsigned m_diagnosticsSize = 0;
    bool m_messages = false;
    bool m_autoHover = false;
    bool m_onTypeFormatting = false;
    bool m_incrementalSync = false;
    bool m_highlightGoto = true;
    QUrl m_configPath;
    bool m_semanticHighlighting = false;
    bool m_signatureHelp = true;

    // debug mode?
    bool m_debugMode = false;

    // get current config path
    QUrl configPath() const
    {
        return m_configPath.isEmpty() ? m_defaultConfigPath : m_configPath;
    }

private:
Q_SIGNALS:
    // signal settings update
    void update() const;
};

#endif
