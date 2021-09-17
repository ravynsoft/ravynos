/***************************************************************************
                          plugin_katetextfilter.cpp  -  description
                             -------------------
    begin                : FRE Feb 23 2001
    copyright            : (C) 2001 by Joseph Wenninger <jowenn@bigfoot.com>
    copyright            : (C) 2009 Dominik Haumann <dhaumann kde org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "plugin_katetextfilter.h"

#include "ui_textfilterwidget.h"

#include <ktexteditor/editor.h>
#include <ktexteditor/message.h>

#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <QAction>
#include <QDialog>
#include <QString>

#include <KActionCollection>
#include <KAuthorized>
#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>
#include <KXMLGUIFactory>

#include <QApplication>
#include <QClipboard>

K_PLUGIN_FACTORY_WITH_JSON(TextFilterPluginFactory, "textfilterplugin.json", registerPlugin<PluginKateTextFilter>();)

PluginKateTextFilter::PluginKateTextFilter(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
{
    // register command
    new PluginKateTextFilterCommand(this);
}

PluginKateTextFilter::~PluginKateTextFilter()
{
    // cleanup the process the right way (TM)
    if (m_pFilterProcess) {
        m_pFilterProcess->kill();
        m_pFilterProcess->waitForFinished();
        delete m_pFilterProcess;
    }
}

QObject *PluginKateTextFilter::createView(KTextEditor::MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    // create a plugin view
    return new PluginViewKateTextFilter(this, mainWindow);
}

void PluginKateTextFilter::slotFilterReceivedStdout()
{
    m_strFilterOutput += QString::fromLocal8Bit(m_pFilterProcess->readAllStandardOutput());
}

void PluginKateTextFilter::slotFilterReceivedStderr()
{
    const QString block = QString::fromLocal8Bit(m_pFilterProcess->readAllStandardError());
    if (mergeOutput) {
        m_strFilterOutput += block;
    } else {
        m_stderrOutput += block;
    }
}

void PluginKateTextFilter::slotFilterProcessExited(int, QProcess::ExitStatus)
{
    KTextEditor::View *kv(KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView());
    if (!kv) {
        return;
    }

    // Is there any error output to display?
    if (!mergeOutput && !m_stderrOutput.isEmpty()) {
        QPointer<KTextEditor::Message> message =
            new KTextEditor::Message(xi18nc("@info", "<title>Result of:</title><nl /><pre><code>$ %1\n<nl />%2</code></pre>", m_last_command, m_stderrOutput),
                                     KTextEditor::Message::Error);
        message->setWordWrap(true);
        message->setAutoHide(1000);
        kv->document()->postMessage(message);
    }

    if (newDocument) {
        auto v = m_mainWindow->openUrl(QUrl());
        if (v && v->document()) {
            v->document()->setText(m_strFilterOutput);
        }
        return;
    }

    if (copyResult) {
        QApplication::clipboard()->setText(m_strFilterOutput);
        return;
    }

    // Do not even try to change the document if no result collected...
    if (m_strFilterOutput.isEmpty()) {
        return;
    }

    KTextEditor::Document::EditingTransaction transaction(kv->document());

    KTextEditor::Cursor start = kv->cursorPosition();
    if (kv->selection()) {
        start = kv->selectionRange().start();
        kv->removeSelectionText();
    }

    kv->setCursorPosition(start); // for block selection

    kv->insertText(m_strFilterOutput);
}

static void slipInFilter(KProcess &proc, KTextEditor::View &view, const QString &command)
{
    QString inputText;

    if (view.selection()) {
        inputText = view.selectionText();
    } else {
        inputText = view.document()->text();
    }

    proc.clearProgram();
    proc.setShellCommand(command);

    proc.start();
    QByteArray encoded = inputText.toLocal8Bit();
    proc.write(encoded);
    proc.closeWriteChannel();
    //  TODO: Put up a modal dialog to defend the text from further
    //  keystrokes while the command is out. With a cancel button...
}

void PluginKateTextFilter::slotEditFilter()
{
    if (!KAuthorized::authorize(QStringLiteral("shell_access"))) {
        KMessageBox::sorry(nullptr,
                           i18n("You are not allowed to execute arbitrary external applications. If "
                                "you want to be able to do this, contact your system administrator."),
                           i18n("Access Restrictions"));
        return;
    }
    if (!KTextEditor::Editor::instance()->application()->activeMainWindow()) {
        return;
    }

    KTextEditor::View *kv(KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView());
    if (!kv) {
        return;
    }

    QDialog dialog(KTextEditor::Editor::instance()->application()->activeMainWindow()->window());

    Ui::TextFilterWidget ui;
    ui.setupUi(&dialog);
    ui.filterBox->setFocus();

    dialog.setWindowTitle(i18n("Text Filter"));

    KConfigGroup config(KSharedConfig::openConfig(), "PluginTextFilter");
    QStringList items = config.readEntry("Completion list", QStringList());
    copyResult = config.readEntry("Copy result", false);
    mergeOutput = config.readEntry("Merge output", true);
    newDocument = config.readEntry("New Document", false);
    ui.filterBox->setMaxCount(10);
    ui.filterBox->setHistoryItems(items, true);
    ui.filterBox->setMinimumContentsLength(80);
    ui.copyResult->setChecked(copyResult);
    ui.mergeOutput->setChecked(mergeOutput);
    ui.newDoc->setChecked(newDocument);

    if (dialog.exec() == QDialog::Accepted) {
        copyResult = ui.copyResult->isChecked();
        mergeOutput = ui.mergeOutput->isChecked();
        newDocument = ui.newDoc->isChecked();
        const QString filter = ui.filterBox->currentText();
        if (!filter.isEmpty()) {
            ui.filterBox->addToHistory(filter);
            config.writeEntry("New Document", newDocument);
            config.writeEntry("Completion list", ui.filterBox->historyItems());
            config.writeEntry("Copy result", copyResult);
            config.writeEntry("Merge output", mergeOutput);
            m_last_command = filter;
            runFilter(kv, filter);
        }
    }
}

void PluginKateTextFilter::runFilter(KTextEditor::View *kv, const QString &filter)
{
    m_strFilterOutput.clear();
    m_stderrOutput.clear();

    if (!m_pFilterProcess) {
        m_pFilterProcess = new KProcess;

        connect(m_pFilterProcess, &KProcess::readyReadStandardOutput, this, &PluginKateTextFilter::slotFilterReceivedStdout);

        connect(m_pFilterProcess, &KProcess::readyReadStandardError, this, &PluginKateTextFilter::slotFilterReceivedStderr);

        connect(m_pFilterProcess,
                static_cast<void (KProcess::*)(int, KProcess::ExitStatus)>(&KProcess::finished),
                this,
                &PluginKateTextFilter::slotFilterProcessExited);
    }
    m_pFilterProcess->setOutputChannelMode(mergeOutput ? KProcess::MergedChannels : KProcess::SeparateChannels);

    slipInFilter(*m_pFilterProcess, *kv, filter);
}

// BEGIN Kate::Command methods

PluginKateTextFilterCommand::PluginKateTextFilterCommand(PluginKateTextFilter *plugin)
    : KTextEditor::Command(QStringList() << QStringLiteral("textfilter"), plugin)
    , m_plugin(plugin)
{
}

bool PluginKateTextFilterCommand::exec(KTextEditor::View *view, const QString &cmd, QString &msg, const KTextEditor::Range &)
{
    QString filter = cmd.section(QLatin1Char(' '), 1).trimmed();

    if (filter.isEmpty()) {
        msg = i18n("Usage: textfilter COMMAND");
        return false;
    }

    m_plugin->runFilter(view, filter);
    return true;
}

bool PluginKateTextFilterCommand::help(KTextEditor::View *, const QString &, QString &msg)
{
    msg = i18n(
        "<qt><p>Usage: <code>textfilter COMMAND</code></p>"
        "<p>Replace the selection with the output of the specified shell command.</p></qt>");
    return true;
}
// END

PluginViewKateTextFilter::PluginViewKateTextFilter(PluginKateTextFilter *plugin, KTextEditor::MainWindow *mainwindow)
    : QObject(mainwindow)
    , m_mainWindow(mainwindow)
{
    // setup right xml gui data
    KXMLGUIClient::setComponentName(QStringLiteral("textfilter"), i18n("Text Filter"));
    setXMLFile(QStringLiteral("ui.rc"));

    // create our one and only action
    QAction *a = actionCollection()->addAction(QStringLiteral("edit_filter"));
    a->setText(i18n("&Filter Through Command..."));
    actionCollection()->setDefaultShortcut(a, Qt::CTRL | Qt::Key_Backslash);
    connect(a, &QAction::triggered, plugin, &PluginKateTextFilter::slotEditFilter);

    // register us at the UI
    mainwindow->guiFactory()->addClient(this);
}

PluginViewKateTextFilter::~PluginViewKateTextFilter()
{
    // remove us from the UI again
    m_mainWindow->guiFactory()->removeClient(this);
}

// required for TextFilterPluginFactory vtable
#include "plugin_katetextfilter.moc"
