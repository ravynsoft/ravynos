/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2014 Martin Sandsmark <martin.sandsmark@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "replicodeview.h"

#include "replicodeconfig.h"
#include "replicodesettings.h"
#include <QPushButton>
#include <QTemporaryFile>
#include <QtGlobal>

#include <KLocalizedString>
#include <KXMLGUIFactory>

#include <QAction>
#include <QDebug>
#include <QFileInfo>
#include <QFormLayout>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QVBoxLayout>

#include <KAboutData>
#include <KActionCollection>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>

ReplicodeView::ReplicodeView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainWindow)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_executor(nullptr)
{
    m_runAction = new QAction(QIcon(QStringLiteral("code-block")), i18n("Run replicode"), this);
    connect(m_runAction, &QAction::triggered, this, &ReplicodeView::runReplicode);
    actionCollection()->addAction(QStringLiteral("katereplicode_run"), m_runAction);

    m_stopAction = new QAction(QIcon(QStringLiteral("process-stop")), i18n("Stop replicode"), this);
    connect(m_stopAction, &QAction::triggered, this, &ReplicodeView::stopReplicode);
    actionCollection()->addAction(QStringLiteral("katereplicode_stop"), m_stopAction);
    m_stopAction->setEnabled(false);

    m_toolview = m_mainWindow->createToolView(plugin,
                                              QStringLiteral("kate_private_plugin_katereplicodeplugin_run"),
                                              KTextEditor::MainWindow::Bottom,
                                              QIcon::fromTheme(QStringLiteral("code-block")),
                                              i18n("Replicode Output"));
    m_replicodeOutput = new QListWidget(m_toolview);
    m_replicodeOutput->setSelectionMode(QAbstractItemView::ContiguousSelection);
    connect(m_replicodeOutput, &QListWidget::itemActivated, this, &ReplicodeView::outputClicked);
    m_mainWindow->hideToolView(m_toolview);

    m_configSidebar = m_mainWindow->createToolView(plugin,
                                                   QStringLiteral("kate_private_plugin_katereplicodeplugin_config"),
                                                   KTextEditor::MainWindow::Right,
                                                   QIcon::fromTheme(QStringLiteral("code-block")),
                                                   i18n("Replicode Config"));
    m_configView = new ReplicodeConfig(m_configSidebar);

    m_runButton = new QPushButton(i18nc("shortcut for action", "Run (%1)", m_runAction->shortcut().toString()));
    m_stopButton = new QPushButton(i18nc("shortcut for action", "Stop (%1)", m_stopAction->shortcut().toString()));
    m_stopButton->setEnabled(false);

    QFormLayout *l = qobject_cast<QFormLayout *>(m_configView->widget(0)->layout());
    Q_ASSERT(l);
    l->addRow(m_runButton, m_stopButton);
    connect(m_runButton, &QPushButton::clicked, m_runAction, &QAction::trigger);
    connect(m_stopButton, &QPushButton::clicked, m_stopAction, &QAction::trigger);

    m_mainWindow->guiFactory()->addClient(this);
    connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &ReplicodeView::viewChanged);
}

ReplicodeView::~ReplicodeView()
{
    m_mainWindow->guiFactory()->removeClient(this);
    delete m_executor;
}

void ReplicodeView::viewChanged()
{
    if (m_mainWindow->activeView() && m_mainWindow->activeView()->document()
        && m_mainWindow->activeView()->document()->url().fileName().endsWith(QLatin1String(".replicode"))) {
        m_mainWindow->showToolView(m_configSidebar);
    } else {
        m_mainWindow->hideToolView(m_configSidebar);
        m_mainWindow->hideToolView(m_toolview);
    }
}

void ReplicodeView::runReplicode()
{
    m_mainWindow->showToolView(m_toolview);
    KTextEditor::View *editor = m_mainWindow->activeView();
    if (!editor || !editor->document()) {
        QMessageBox::warning(m_mainWindow->window(), i18nc("@title:window", "Active Document Not Found"), i18n("Could not find an active document to run."));
        return;
    }

    if (editor->document()->isEmpty()) {
        QMessageBox::warning(m_mainWindow->window(), i18nc("@title:window", "Empty Document"), i18n("Cannot execute an empty document."));
        return;
    }

    QFileInfo sourceFile = QFileInfo(editor->document()->url().toLocalFile());

    if (!sourceFile.isReadable()) {
        QMessageBox::warning(m_mainWindow->window(), i18nc("@title:window", "File Not Found"), i18n("Unable to open source file for reading."));
        return;
    }

    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("Replicode"));
    QString executorPath = config.readEntry<QString>("replicodePath", QString());
    if (executorPath.isEmpty()) {
        QMessageBox::warning(m_mainWindow->window(),
                             i18nc("@title:window", "Replicode Executable Not Found"),
                             i18n("Unable to find replicode executor.\n"
                                  "Please go to settings and set the path to the Replicode executable."));
        return;
    }

    if (m_configView->settingsObject()->userOperatorPath.isEmpty()) {
        QMessageBox::warning(m_mainWindow->window(),
                             i18nc("@title:window", "User Operator Library Not Found"),
                             i18n("Unable to find user operator library.\n"
                                  "Please go to settings and set the path to the library."));
    }

    m_configView->settingsObject()->sourcePath = editor->document()->url().toLocalFile();
    m_configView->load();
    m_configView->settingsObject()->save();

    m_replicodeOutput->clear();

    if (m_executor) {
        delete m_executor;
    }
    m_executor = new QProcess(this);
    m_executor->setWorkingDirectory(sourceFile.canonicalPath());
    connect(m_executor, &QProcess::readyReadStandardError, this, &ReplicodeView::gotStderr);
    connect(m_executor, &QProcess::readyReadStandardOutput, this, &ReplicodeView::gotStdout);
    connect(m_executor, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &ReplicodeView::replicodeFinished);
    connect(m_executor, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, &ReplicodeView::runErrored);
    qDebug() << executorPath << sourceFile.canonicalPath();
    m_completed = false;
    m_executor->start(executorPath, QStringList(), QProcess::ReadOnly);

    m_runAction->setEnabled(false);
    m_runButton->setEnabled(false);
    m_stopAction->setEnabled(true);
    m_stopButton->setEnabled(true);
}

void ReplicodeView::stopReplicode()
{
    if (m_executor) {
        m_executor->kill();
    }
}

void ReplicodeView::outputClicked(QListWidgetItem *item)
{
    QString output = item->text();
    QStringList pieces = output.split(QLatin1Char(':'));

    if (pieces.length() < 2) {
        return;
    }

    QFileInfo file(pieces[0]);
    if (!file.isReadable()) {
        return;
    }

    bool ok = false;
    int lineNumber = pieces[1].toInt(&ok);
    qDebug() << lineNumber;
    if (!ok) {
        return;
    }

    KTextEditor::View *doc = m_mainWindow->openUrl(QUrl::fromLocalFile(pieces[0]));
    doc->setCursorPosition(KTextEditor::Cursor(lineNumber, 0));
    qDebug() << doc->cursorPosition().line();
}

void ReplicodeView::runErrored(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    QListWidgetItem *item = new QListWidgetItem(i18n("Replicode execution failed: %1", m_executor->errorString()));
    item->setForeground(Qt::red);
    m_replicodeOutput->addItem(item);
    m_replicodeOutput->scrollToBottom();
    m_completed = true;
}

void ReplicodeView::replicodeFinished()
{
    if (!m_completed) {
        QListWidgetItem *item = new QListWidgetItem(i18n("Replicode execution finished."));
        item->setForeground(Qt::blue);
        m_replicodeOutput->addItem(item);
        m_replicodeOutput->scrollToBottom();
    }

    m_runAction->setEnabled(true);
    m_runButton->setEnabled(true);
    m_stopAction->setEnabled(false);
    m_stopButton->setEnabled(false);
    //    delete m_executor;
    //    delete m_settingsFile;
    //    m_executor = 0;
    //    m_settingsFile = 0;
}

void ReplicodeView::gotStderr()
{
    const QByteArray output = m_executor->readAllStandardError();
    const auto lines = output.split('\n');
    for (QByteArray line : lines) {
        line = line.simplified();
        if (line.isEmpty()) {
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem(QString::fromLocal8Bit(line));
        item->setForeground(Qt::red);
        m_replicodeOutput->addItem(item);
    }
    m_replicodeOutput->scrollToBottom();
}

void ReplicodeView::gotStdout()
{
    const QByteArray output = m_executor->readAllStandardOutput();
    const auto lines = output.split('\n');
    for (QByteArray line : lines) {
        line = line.simplified();
        if (line.isEmpty()) {
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem(QString::fromLocal8Bit(' ' + line));
        if (line[0] == '>') {
            item->setForeground(Qt::gray);
        }
        m_replicodeOutput->addItem(item);
    }
    m_replicodeOutput->scrollToBottom();
}
