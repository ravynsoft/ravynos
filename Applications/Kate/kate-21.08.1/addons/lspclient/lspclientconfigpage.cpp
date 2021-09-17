/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>

    SPDX-License-Identifier: MIT
*/

#include "lspclientconfigpage.h"
#include "lspclientplugin.h"
#include "ui_lspconfigwidget.h"

#include <KLocalizedString>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/SyntaxHighlighter>
#include <KSyntaxHighlighting/Theme>

#include <KTextEditor/Editor>

#include <QJsonDocument>
#include <QJsonParseError>
#include <QPalette>

LSPClientConfigPage::LSPClientConfigPage(QWidget *parent, LSPClientPlugin *plugin)
    : KTextEditor::ConfigPage(parent)
    , m_plugin(plugin)
{
    ui = new Ui::LspConfigWidget();
    ui->setupUi(this);

    // fix-up our two text edits to be proper JSON file editors
    updateHighlighters();

    // ensure we update the highlighters if the repository is updated or theme is changed
    connect(KTextEditor::Editor::instance(), &KTextEditor::Editor::repositoryReloaded, this, &LSPClientConfigPage::updateHighlighters);
    connect(KTextEditor::Editor::instance(), &KTextEditor::Editor::configChanged, this, &LSPClientConfigPage::updateHighlighters);

    // setup default json settings
    QFile defaultConfigFile(QStringLiteral(":/lspclient/settings.json"));
    defaultConfigFile.open(QIODevice::ReadOnly);
    Q_ASSERT(defaultConfigFile.isOpen());
    ui->defaultConfig->setPlainText(QString::fromUtf8(defaultConfigFile.readAll()));

    // setup default config path as placeholder to show user where it is
    ui->edtConfigPath->setPlaceholderText(m_plugin->m_defaultConfigPath.toLocalFile());

    reset();

    for (const auto &cb : {ui->chkSymbolDetails,
                           ui->chkSymbolExpand,
                           ui->chkSymbolSort,
                           ui->chkSymbolTree,
                           ui->chkComplDoc,
                           ui->chkRefDeclaration,
                           ui->chkComplParens,
                           ui->chkDiagnostics,
                           ui->chkDiagnosticsMark,
                           ui->chkDiagnosticsHover,
                           ui->chkMessages,
                           ui->chkOnTypeFormatting,
                           ui->chkIncrementalSync,
                           ui->chkHighlightGoto,
                           ui->chkSemanticHighlighting,
                           ui->chkAutoHover,
                           ui->chkSignatureHelp}) {
        connect(cb, &QCheckBox::toggled, this, &LSPClientConfigPage::changed);
    }
    auto ch = [this](int) {
        this->changed();
    };
    connect(ui->spinDiagnosticsSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, ch);
    connect(ui->edtConfigPath, &KUrlRequester::textChanged, this, &LSPClientConfigPage::configUrlChanged);
    connect(ui->edtConfigPath, &KUrlRequester::urlSelected, this, &LSPClientConfigPage::configUrlChanged);

    auto cfgh = [this](int position, int added, int removed) {
        Q_UNUSED(position);
        // discard format change
        // (e.g. due to syntax highlighting)
        if (added || removed) {
            configTextChanged();
        }
    };
    connect(ui->userConfig->document(), &QTextDocument::contentsChange, this, cfgh);

    // custom control logic
    auto h = [this]() {
        bool enabled = ui->chkDiagnostics->isChecked();
        ui->chkDiagnosticsHighlight->setEnabled(enabled);
        ui->chkDiagnosticsMark->setEnabled(enabled);
        ui->chkDiagnosticsHover->setEnabled(enabled);
        enabled = enabled && ui->chkDiagnosticsHover->isChecked();
        ui->spinDiagnosticsSize->setEnabled(enabled);
        enabled = ui->chkMessages->isChecked();
    };
    connect(this, &LSPClientConfigPage::changed, this, h);
}

LSPClientConfigPage::~LSPClientConfigPage()
{
    delete ui;
}

QString LSPClientConfigPage::name() const
{
    return QString(i18n("LSP Client"));
}

QString LSPClientConfigPage::fullName() const
{
    return QString(i18n("LSP Client"));
}

QIcon LSPClientConfigPage::icon() const
{
    return QIcon::fromTheme(QLatin1String("code-context"));
}

void LSPClientConfigPage::apply()
{
    m_plugin->m_symbolDetails = ui->chkSymbolDetails->isChecked();
    m_plugin->m_symbolTree = ui->chkSymbolTree->isChecked();
    m_plugin->m_symbolExpand = ui->chkSymbolExpand->isChecked();
    m_plugin->m_symbolSort = ui->chkSymbolSort->isChecked();

    m_plugin->m_complDoc = ui->chkComplDoc->isChecked();
    m_plugin->m_refDeclaration = ui->chkRefDeclaration->isChecked();
    m_plugin->m_complParens = ui->chkComplParens->isChecked();

    m_plugin->m_diagnostics = ui->chkDiagnostics->isChecked();
    m_plugin->m_diagnosticsHighlight = ui->chkDiagnosticsHighlight->isChecked();
    m_plugin->m_diagnosticsMark = ui->chkDiagnosticsMark->isChecked();
    m_plugin->m_diagnosticsHover = ui->chkDiagnosticsHover->isChecked();
    m_plugin->m_diagnosticsSize = ui->spinDiagnosticsSize->value();

    m_plugin->m_autoHover = ui->chkAutoHover->isChecked();
    m_plugin->m_onTypeFormatting = ui->chkOnTypeFormatting->isChecked();
    m_plugin->m_incrementalSync = ui->chkIncrementalSync->isChecked();
    m_plugin->m_highlightGoto = ui->chkHighlightGoto->isChecked();
    m_plugin->m_semanticHighlighting = ui->chkSemanticHighlighting->isChecked();
    m_plugin->m_signatureHelp = ui->chkSignatureHelp->isChecked();

    m_plugin->m_messages = ui->chkMessages->isChecked();

    m_plugin->m_configPath = ui->edtConfigPath->url();

    // own scope to ensure file is flushed before we signal below in writeConfig!
    {
        QFile configFile(m_plugin->configPath().toLocalFile());
        configFile.open(QIODevice::WriteOnly);
        if (configFile.isOpen()) {
            configFile.write(ui->userConfig->toPlainText().toUtf8());
        }
    }

    m_plugin->writeConfig();
}

void LSPClientConfigPage::reset()
{
    ui->chkSymbolDetails->setChecked(m_plugin->m_symbolDetails);
    ui->chkSymbolTree->setChecked(m_plugin->m_symbolTree);
    ui->chkSymbolExpand->setChecked(m_plugin->m_symbolExpand);
    ui->chkSymbolSort->setChecked(m_plugin->m_symbolSort);

    ui->chkComplDoc->setChecked(m_plugin->m_complDoc);
    ui->chkRefDeclaration->setChecked(m_plugin->m_refDeclaration);
    ui->chkComplParens->setChecked(m_plugin->m_complParens);

    ui->chkDiagnostics->setChecked(m_plugin->m_diagnostics);
    ui->chkDiagnosticsHighlight->setChecked(m_plugin->m_diagnosticsHighlight);
    ui->chkDiagnosticsMark->setChecked(m_plugin->m_diagnosticsMark);
    ui->chkDiagnosticsHover->setChecked(m_plugin->m_diagnosticsHover);
    ui->spinDiagnosticsSize->setValue(m_plugin->m_diagnosticsSize);

    ui->chkAutoHover->setChecked(m_plugin->m_autoHover);
    ui->chkOnTypeFormatting->setChecked(m_plugin->m_onTypeFormatting);
    ui->chkIncrementalSync->setChecked(m_plugin->m_incrementalSync);
    ui->chkHighlightGoto->setChecked(m_plugin->m_highlightGoto);
    ui->chkSemanticHighlighting->setChecked(m_plugin->m_semanticHighlighting);
    ui->chkSignatureHelp->setChecked(m_plugin->m_signatureHelp);

    ui->chkMessages->setChecked(m_plugin->m_messages);

    ui->edtConfigPath->setUrl(m_plugin->m_configPath);

    readUserConfig(m_plugin->configPath().toLocalFile());
}

void LSPClientConfigPage::defaults()
{
    reset();
}

void LSPClientConfigPage::readUserConfig(const QString &fileName)
{
    QFile configFile(fileName);
    configFile.open(QIODevice::ReadOnly);
    if (configFile.isOpen()) {
        ui->userConfig->setPlainText(QString::fromUtf8(configFile.readAll()));
    } else {
        ui->userConfig->clear();
    }

    updateConfigTextErrorState();
}

void LSPClientConfigPage::updateConfigTextErrorState()
{
    const auto data = ui->userConfig->toPlainText().toUtf8();
    if (data.isEmpty()) {
        ui->userConfigError->setText(i18n("No JSON data to validate."));
        return;
    }

    // check json validity
    QJsonParseError error{};
    auto json = QJsonDocument::fromJson(data, &error);
    if (error.error == QJsonParseError::NoError) {
        if (json.isObject()) {
            ui->userConfigError->setText(i18n("JSON data is valid."));
        } else {
            ui->userConfigError->setText(i18n("JSON data is invalid: no JSON object"));
        }
    } else {
        ui->userConfigError->setText(i18n("JSON data is invalid: %1", error.errorString()));
    }
}

void LSPClientConfigPage::configTextChanged()
{
    // check for errors
    updateConfigTextErrorState();

    // remember changed
    changed();
}

void LSPClientConfigPage::configUrlChanged()
{
    // re-read config
    readUserConfig(ui->edtConfigPath->url().isEmpty() ? m_plugin->m_defaultConfigPath.toLocalFile() : ui->edtConfigPath->url().toLocalFile());

    // remember changed
    changed();
}

void LSPClientConfigPage::updateHighlighters()
{
    for (auto textEdit : {ui->userConfig, static_cast<QTextEdit *>(ui->defaultConfig)}) {
        // setup JSON highlighter for the default json stuff
        auto highlighter = new KSyntaxHighlighting::SyntaxHighlighter(textEdit->document());
        highlighter->setDefinition(KTextEditor::Editor::instance()->repository().definitionForFileName(QStringLiteral("settings.json")));

        // we want mono-spaced font
        textEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

        // we want to have the proper theme for the current palette
        const auto theme = KTextEditor::Editor::instance()->theme();
        auto pal = qApp->palette();
        pal.setColor(QPalette::Base, QColor::fromRgba(theme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor)));
        pal.setColor(QPalette::Highlight, QColor::fromRgba(theme.editorColor(KSyntaxHighlighting::Theme::TextSelection)));
        textEdit->setPalette(pal);
        highlighter->setTheme(theme);
        highlighter->rehighlight();
    }
}
