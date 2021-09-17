//
// Description: Kate Plugin for GDB integration
//
//
// SPDX-FileCopyrightText: 2010 Ian Wakeling <ian.wakeling@ntlworld.com>
// SPDX-FileCopyrightText: 2010-2014 Kåre Särs <kare.sars@iki.fi>
//
//  SPDX-License-Identifier: LGPL-2.0-only

#include "plugin_kategdb.h"

#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QLayout>
#include <QScrollBar>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QTreeWidget>

#include <KActionCollection>
#include <KConfigGroup>
#include <KXMLGUIFactory>
#include <QAction>
#include <QMenu>

#include <KAboutData>
#include <KColorScheme>
#include <KHistoryComboBox>
#include <KLocalizedString>
#include <KPluginFactory>

#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/view.h>

K_PLUGIN_FACTORY_WITH_JSON(KatePluginGDBFactory, "kategdbplugin.json", registerPlugin<KatePluginGDB>();)

KatePluginGDB::KatePluginGDB(QObject *parent, const VariantList &)
    : KTextEditor::Plugin(parent)
{
    // FIXME KF5 KGlobal::locale()->insertCatalog("kategdbplugin");
}

KatePluginGDB::~KatePluginGDB()
{
}

QObject *KatePluginGDB::createView(KTextEditor::MainWindow *mainWindow)
{
    return new KatePluginGDBView(this, mainWindow);
}

KatePluginGDBView::KatePluginGDBView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainWin)
    : QObject(mainWin)
    , m_mainWin(mainWin)
{
    m_lastExecUrl = QUrl();
    m_lastExecLine = -1;
    m_lastExecFrame = 0;
    m_kateApplication = KTextEditor::Editor::instance()->application();
    m_focusOnInput = true;
    m_activeThread = -1;

    KXMLGUIClient::setComponentName(QStringLiteral("kategdb"), i18n("Kate GDB"));
    setXMLFile(QStringLiteral("ui.rc"));

    m_toolView.reset(m_mainWin->createToolView(plugin,
                                               i18n("Debug View"),
                                               KTextEditor::MainWindow::Bottom,
                                               QIcon(QStringLiteral(":/kategdb/22-actions-debug-kategdb.png")),
                                               i18n("Debug View")));

    m_localsStackToolView.reset(m_mainWin->createToolView(plugin,
                                                          i18n("Locals and Stack"),
                                                          KTextEditor::MainWindow::Right,
                                                          QIcon(QStringLiteral(":/kategdb/22-actions-debug-kategdb.png")),
                                                          i18n("Locals and Stack")));

    m_tabWidget = new QTabWidget(m_toolView.get());
    // Output
    m_outputArea = new QTextEdit();
    m_outputArea->setAcceptRichText(false);
    m_outputArea->setReadOnly(true);
    m_outputArea->setUndoRedoEnabled(false);
    // fixed wide font, like konsole
    m_outputArea->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    // alternate color scheme, like konsole
    KColorScheme schemeView(QPalette::Active, KColorScheme::View);
    m_outputArea->setTextBackgroundColor(schemeView.foreground().color());
    m_outputArea->setTextColor(schemeView.background().color());
    QPalette p = m_outputArea->palette();
    p.setColor(QPalette::Base, schemeView.foreground().color());
    m_outputArea->setPalette(p);

    // input
    m_inputArea = new KHistoryComboBox(true);
    connect(m_inputArea, static_cast<void (KHistoryComboBox::*)()>(&KHistoryComboBox::returnPressed), this, &KatePluginGDBView::slotSendCommand);
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_inputArea, 10);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    m_outputArea->setFocusProxy(m_inputArea); // take the focus from the outputArea

    m_gdbPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_gdbPage);
    layout->addWidget(m_outputArea);
    layout->addLayout(inputLayout);
    layout->setStretch(0, 10);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // stack page
    QWidget *stackContainer = new QWidget();
    QVBoxLayout *stackLayout = new QVBoxLayout(stackContainer);
    m_threadCombo = new QComboBox();
    m_stackTree = new QTreeWidget();
    stackLayout->addWidget(m_threadCombo);
    stackLayout->addWidget(m_stackTree);
    stackLayout->setStretch(0, 10);
    stackLayout->setContentsMargins(0, 0, 0, 0);
    stackLayout->setSpacing(0);
    QStringList headers;
    headers << QStringLiteral("  ") << i18nc("Column label (frame number)", "Nr") << i18nc("Column label", "Frame");
    m_stackTree->setHeaderLabels(headers);
    m_stackTree->setRootIsDecorated(false);
    m_stackTree->resizeColumnToContents(0);
    m_stackTree->resizeColumnToContents(1);
    m_stackTree->setAutoScroll(false);
    connect(m_stackTree, &QTreeWidget::itemActivated, this, &KatePluginGDBView::stackFrameSelected);

    connect(m_threadCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &KatePluginGDBView::threadSelected);

    m_localsView = new LocalsView();

    QSplitter *locStackSplitter = new QSplitter(m_localsStackToolView.get());
    locStackSplitter->addWidget(m_localsView);
    locStackSplitter->addWidget(stackContainer);
    locStackSplitter->setOrientation(Qt::Vertical);

    // config page
    m_configView = new ConfigView(nullptr, mainWin);

    m_ioView = std::make_unique<IOView>();
    connect(m_configView, &ConfigView::showIO, this, &KatePluginGDBView::showIO);

    m_tabWidget->addTab(m_gdbPage, i18nc("Tab label", "GDB Output"));
    m_tabWidget->addTab(m_configView, i18nc("Tab label", "Settings"));

    m_debugView = new DebugView(this);
    connect(m_debugView, &DebugView::readyForInput, this, &KatePluginGDBView::enableDebugActions);

    connect(m_debugView, &DebugView::outputText, this, &KatePluginGDBView::addOutputText);

    connect(m_debugView, &DebugView::outputError, this, &KatePluginGDBView::addErrorText);

    connect(m_debugView, &DebugView::debugLocationChanged, this, &KatePluginGDBView::slotGoTo);

    connect(m_debugView, &DebugView::breakPointSet, this, &KatePluginGDBView::slotBreakpointSet);

    connect(m_debugView, &DebugView::breakPointCleared, this, &KatePluginGDBView::slotBreakpointCleared);

    connect(m_debugView, &DebugView::clearBreakpointMarks, this, &KatePluginGDBView::clearMarks);

    connect(m_debugView, &DebugView::programEnded, this, &KatePluginGDBView::programEnded);

    connect(m_debugView, &DebugView::gdbEnded, this, &KatePluginGDBView::programEnded);

    connect(m_debugView, &DebugView::gdbEnded, this, &KatePluginGDBView::gdbEnded);

    connect(m_debugView, &DebugView::stackFrameInfo, this, &KatePluginGDBView::insertStackFrame);

    connect(m_debugView, &DebugView::stackFrameChanged, this, &KatePluginGDBView::stackFrameChanged);

    connect(m_debugView, &DebugView::infoLocal, m_localsView, &LocalsView::addLocal);

    connect(m_debugView, &DebugView::threadInfo, this, &KatePluginGDBView::insertThread);

    connect(m_debugView, &DebugView::sourceFileNotFound, this, [this](const QString &fileName) {
        displayMessage(xi18nc("@info",
                              "<title>Could not open file:</title><nl/>%1<br/>Try adding a search path to Advanced Settings -> Source file search paths",
                              fileName),
                       KTextEditor::Message::Error);
    });

    connect(m_localsView, &LocalsView::localsVisible, m_debugView, &DebugView::slotQueryLocals);

    connect(m_configView, &ConfigView::configChanged, this, [this]() {
        GDBTargetConf config = m_configView->currentTarget();
        if (m_debugView->targetName() == config.targetName) {
            m_debugView->setFileSearchPaths(config.srcPaths);
        }
    });

    // Actions
    m_configView->registerActions(actionCollection());

    QAction *a = actionCollection()->addAction(QStringLiteral("debug"));
    a->setText(i18n("Start Debugging"));
    a->setIcon(QIcon(QStringLiteral(":/kategdb/22-actions-debug-kategdb.png")));
    connect(a, &QAction::triggered, this, &KatePluginGDBView::slotDebug);

    a = actionCollection()->addAction(QStringLiteral("kill"));
    a->setText(i18n("Kill / Stop Debugging"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-stop")));
    connect(a, &QAction::triggered, m_debugView, &DebugView::slotKill);

    a = actionCollection()->addAction(QStringLiteral("rerun"));
    a->setText(i18n("Restart Debugging"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    connect(a, &QAction::triggered, this, &KatePluginGDBView::slotRestart);

    a = actionCollection()->addAction(QStringLiteral("toggle_breakpoint"));
    a->setText(i18n("Toggle Breakpoint / Break"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
    connect(a, &QAction::triggered, this, &KatePluginGDBView::slotToggleBreakpoint);

    a = actionCollection()->addAction(QStringLiteral("step_in"));
    a->setText(i18n("Step In"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-step-into")));
    connect(a, &QAction::triggered, m_debugView, &DebugView::slotStepInto);

    a = actionCollection()->addAction(QStringLiteral("step_over"));
    a->setText(i18n("Step Over"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-step-over")));
    connect(a, &QAction::triggered, m_debugView, &DebugView::slotStepOver);

    a = actionCollection()->addAction(QStringLiteral("step_out"));
    a->setText(i18n("Step Out"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-step-out")));
    connect(a, &QAction::triggered, m_debugView, &DebugView::slotStepOut);

    a = actionCollection()->addAction(QStringLiteral("move_pc"));
    a->setText(i18nc("Move Program Counter (next execution)", "Move PC"));
    connect(a, &QAction::triggered, this, &KatePluginGDBView::slotMovePC);

    a = actionCollection()->addAction(QStringLiteral("run_to_cursor"));
    a->setText(i18n("Run To Cursor"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-run-cursor")));
    connect(a, &QAction::triggered, this, &KatePluginGDBView::slotRunToCursor);

    a = actionCollection()->addAction(QStringLiteral("continue"));
    a->setText(i18n("Continue"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    connect(a, &QAction::triggered, m_debugView, &DebugView::slotContinue);

    a = actionCollection()->addAction(QStringLiteral("print_value"));
    a->setText(i18n("Print Value"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("document-preview")));
    connect(a, &QAction::triggered, this, &KatePluginGDBView::slotValue);

    // popup context m_menu
    m_menu = new KActionMenu(i18n("Debug"), this);
    actionCollection()->addAction(QStringLiteral("popup_gdb"), m_menu);
    connect(m_menu->menu(), &QMenu::aboutToShow, this, &KatePluginGDBView::aboutToShowMenu);

    m_breakpoint = m_menu->menu()->addAction(i18n("popup_breakpoint"), this, &KatePluginGDBView::slotToggleBreakpoint);

    QAction *popupAction = m_menu->menu()->addAction(i18n("popup_run_to_cursor"), this, &KatePluginGDBView::slotRunToCursor);
    popupAction->setText(i18n("Run To Cursor"));
    popupAction = m_menu->menu()->addAction(QStringLiteral("move_pc"), this, &KatePluginGDBView::slotMovePC);
    popupAction->setText(i18nc("Move Program Counter (next execution)", "Move PC"));

    enableDebugActions(false);

    connect(m_mainWin, &KTextEditor::MainWindow::unhandledShortcutOverride, this, &KatePluginGDBView::handleEsc);

    m_toolView->installEventFilter(this);

    m_mainWin->guiFactory()->addClient(this);
}

KatePluginGDBView::~KatePluginGDBView()
{
    m_mainWin->guiFactory()->removeClient(this);
}

void KatePluginGDBView::readSessionConfig(const KConfigGroup &config)
{
    m_configView->readConfig(config);
}

void KatePluginGDBView::writeSessionConfig(KConfigGroup &config)
{
    m_configView->writeConfig(config);
}

void KatePluginGDBView::slotDebug()
{
    disconnect(m_ioView.get(), &IOView::stdOutText, nullptr, nullptr);
    disconnect(m_ioView.get(), &IOView::stdErrText, nullptr, nullptr);
    if (m_configView->showIOTab()) {
        connect(m_ioView.get(), &IOView::stdOutText, m_ioView.get(), &IOView::addStdOutText);
        connect(m_ioView.get(), &IOView::stdErrText, m_ioView.get(), &IOView::addStdErrText);
    } else {
        connect(m_ioView.get(), &IOView::stdOutText, this, &KatePluginGDBView::addOutputText);
        connect(m_ioView.get(), &IOView::stdErrText, this, &KatePluginGDBView::addErrorText);
    }
    QStringList ioFifos;
    ioFifos << m_ioView->stdinFifo();
    ioFifos << m_ioView->stdoutFifo();
    ioFifos << m_ioView->stderrFifo();

    enableDebugActions(true);
    m_mainWin->showToolView(m_toolView.get());
    m_tabWidget->setCurrentWidget(m_gdbPage);
    QScrollBar *sb = m_outputArea->verticalScrollBar();
    sb->setValue(sb->maximum());
    m_localsView->clear();

    m_debugView->runDebugger(m_configView->currentTarget(), ioFifos);
}

void KatePluginGDBView::slotRestart()
{
    m_mainWin->showToolView(m_toolView.get());
    m_tabWidget->setCurrentWidget(m_gdbPage);
    QScrollBar *sb = m_outputArea->verticalScrollBar();
    sb->setValue(sb->maximum());
    m_localsView->clear();

    m_debugView->slotReRun();
}

void KatePluginGDBView::aboutToShowMenu()
{
    if (!m_debugView->debuggerRunning() || m_debugView->debuggerBusy()) {
        m_breakpoint->setText(i18n("Insert breakpoint"));
        m_breakpoint->setDisabled(true);
        return;
    }

    m_breakpoint->setDisabled(false);

    KTextEditor::View *editView = m_mainWin->activeView();
    QUrl url = editView->document()->url();
    int line = editView->cursorPosition().line();

    line++; // GDB uses 1 based line numbers, kate uses 0 based...

    if (m_debugView->hasBreakpoint(url, line)) {
        m_breakpoint->setText(i18n("Remove breakpoint"));
    } else {
        m_breakpoint->setText(i18n("Insert breakpoint"));
    }
}

void KatePluginGDBView::slotToggleBreakpoint()
{
    if (!actionCollection()->action(QStringLiteral("continue"))->isEnabled()) {
        m_debugView->slotInterrupt();
    } else {
        KTextEditor::View *editView = m_mainWin->activeView();
        QUrl currURL = editView->document()->url();
        int line = editView->cursorPosition().line();

        m_debugView->toggleBreakpoint(currURL, line + 1);
    }
}

void KatePluginGDBView::slotBreakpointSet(const QUrl &file, int line)
{
    KTextEditor::MarkInterfaceV2 *iface = qobject_cast<KTextEditor::MarkInterfaceV2 *>(m_kateApplication->findUrl(file));

    if (iface) {
        iface->setMarkDescription(KTextEditor::MarkInterface::BreakpointActive, i18n("Breakpoint"));
        iface->setMarkIcon(KTextEditor::MarkInterface::BreakpointActive, QIcon::fromTheme(QStringLiteral("media-playback-pause")));
        iface->addMark(line, KTextEditor::MarkInterface::BreakpointActive);
    }
}

void KatePluginGDBView::slotBreakpointCleared(const QUrl &file, int line)
{
    KTextEditor::MarkInterface *iface = qobject_cast<KTextEditor::MarkInterface *>(m_kateApplication->findUrl(file));

    if (iface) {
        iface->removeMark(line, KTextEditor::MarkInterface::BreakpointActive);
    }
}

void KatePluginGDBView::slotMovePC()
{
    KTextEditor::View *editView = m_mainWin->activeView();
    QUrl currURL = editView->document()->url();
    KTextEditor::Cursor cursor = editView->cursorPosition();

    m_debugView->movePC(currURL, cursor.line() + 1);
}

void KatePluginGDBView::slotRunToCursor()
{
    KTextEditor::View *editView = m_mainWin->activeView();
    QUrl currURL = editView->document()->url();
    KTextEditor::Cursor cursor = editView->cursorPosition();

    // GDB starts lines from 1, kate returns lines starting from 0 (displaying 1)
    m_debugView->runToCursor(currURL, cursor.line() + 1);
}

void KatePluginGDBView::slotGoTo(const QUrl &url, int lineNum)
{
    // skip not existing files
    if (!QFile::exists(url.toLocalFile())) {
        m_lastExecLine = -1;
        return;
    }

    m_lastExecUrl = url;
    m_lastExecLine = lineNum;

    KTextEditor::View *editView = m_mainWin->openUrl(m_lastExecUrl);
    editView->setCursorPosition(KTextEditor::Cursor(m_lastExecLine, 0));
    m_mainWin->window()->raise();
    m_mainWin->window()->setFocus();
}

void KatePluginGDBView::enableDebugActions(bool enable)
{
    actionCollection()->action(QStringLiteral("step_in"))->setEnabled(enable);
    actionCollection()->action(QStringLiteral("step_over"))->setEnabled(enable);
    actionCollection()->action(QStringLiteral("step_out"))->setEnabled(enable);
    actionCollection()->action(QStringLiteral("move_pc"))->setEnabled(enable);
    actionCollection()->action(QStringLiteral("run_to_cursor"))->setEnabled(enable);
    actionCollection()->action(QStringLiteral("popup_gdb"))->setEnabled(enable);
    actionCollection()->action(QStringLiteral("continue"))->setEnabled(enable);
    actionCollection()->action(QStringLiteral("print_value"))->setEnabled(enable);

    // "toggle breakpoint" doubles as interrupt while the program is running
    actionCollection()->action(QStringLiteral("toggle_breakpoint"))->setEnabled(m_debugView->debuggerRunning());
    actionCollection()->action(QStringLiteral("kill"))->setEnabled(m_debugView->debuggerRunning());
    actionCollection()->action(QStringLiteral("rerun"))->setEnabled(m_debugView->debuggerRunning());

    m_inputArea->setEnabled(enable);
    m_threadCombo->setEnabled(enable);
    m_stackTree->setEnabled(enable);
    m_localsView->setEnabled(enable);

    if (enable) {
        m_inputArea->setFocusPolicy(Qt::WheelFocus);

        if (m_focusOnInput || m_configView->takeFocusAlways()) {
            m_inputArea->setFocus();
            m_focusOnInput = false;
        } else {
            m_mainWin->activeView()->setFocus();
        }
    } else {
        m_inputArea->setFocusPolicy(Qt::NoFocus);
        if (m_mainWin->activeView()) {
            m_mainWin->activeView()->setFocus();
        }
    }

    m_ioView->enableInput(!enable && m_debugView->debuggerRunning());

    if ((m_lastExecLine > -1)) {
        KTextEditor::MarkInterfaceV2 *iface = qobject_cast<KTextEditor::MarkInterfaceV2 *>(m_kateApplication->findUrl(m_lastExecUrl));

        if (iface) {
            if (enable) {
                iface->setMarkDescription(KTextEditor::MarkInterface::Execution, i18n("Execution point"));
                iface->setMarkIcon(KTextEditor::MarkInterface::Execution, QIcon::fromTheme(QStringLiteral("arrow-right")));
                iface->addMark(m_lastExecLine, KTextEditor::MarkInterface::Execution);
            } else {
                iface->removeMark(m_lastExecLine, KTextEditor::MarkInterface::Execution);
            }
        }
    }
}

void KatePluginGDBView::programEnded()
{
    // don't set the execution mark on exit
    m_lastExecLine = -1;
    m_stackTree->clear();
    m_localsView->clear();
    m_threadCombo->clear();

    // Indicate the state change by showing the debug outputArea
    m_mainWin->showToolView(m_toolView.get());
    m_tabWidget->setCurrentWidget(m_gdbPage);
}

void KatePluginGDBView::gdbEnded()
{
    m_outputArea->clear();
    m_localsView->clear();
    m_ioView->clearOutput();
    clearMarks();
}

void KatePluginGDBView::clearMarks()
{
    KTextEditor::MarkInterface *iface;
    const auto documents = m_kateApplication->documents();
    for (KTextEditor::Document *doc : documents) {
        iface = qobject_cast<KTextEditor::MarkInterface *>(doc);
        if (iface) {
            const QHash<int, KTextEditor::Mark *> marks = iface->marks();
            QHashIterator<int, KTextEditor::Mark *> i(marks);
            while (i.hasNext()) {
                i.next();
                if ((i.value()->type == KTextEditor::MarkInterface::Execution) || (i.value()->type == KTextEditor::MarkInterface::BreakpointActive)) {
                    iface->removeMark(i.value()->line, i.value()->type);
                }
            }
        }
    }
}

void KatePluginGDBView::slotSendCommand()
{
    QString cmd = m_inputArea->currentText();

    if (cmd.isEmpty()) {
        cmd = m_lastCommand;
    }

    m_inputArea->addToHistory(cmd);
    m_inputArea->setCurrentItem(QString());
    m_focusOnInput = true;
    m_lastCommand = cmd;
    m_debugView->issueCommand(cmd);

    QScrollBar *sb = m_outputArea->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void KatePluginGDBView::insertStackFrame(QString const &level, QString const &info)
{
    if (level.isEmpty() && info.isEmpty()) {
        m_stackTree->resizeColumnToContents(2);
        return;
    }

    if (level == QLatin1Char('0')) {
        m_stackTree->clear();
    }
    QStringList columns;
    columns << QStringLiteral("  "); // icon place holder
    columns << level;
    int lastSpace = info.lastIndexOf(QLatin1Char(' '));
    QString shortInfo = info.mid(lastSpace);
    columns << shortInfo;

    QTreeWidgetItem *item = new QTreeWidgetItem(columns);
    item->setToolTip(2, QStringLiteral("<qt>%1<qt>").arg(info));
    m_stackTree->insertTopLevelItem(level.toInt(), item);
}

void KatePluginGDBView::stackFrameSelected()
{
    m_debugView->issueCommand(QStringLiteral("(Q)f %1").arg(m_stackTree->currentIndex().row()));
}

void KatePluginGDBView::stackFrameChanged(int level)
{
    QTreeWidgetItem *current = m_stackTree->topLevelItem(m_lastExecFrame);
    QTreeWidgetItem *next = m_stackTree->topLevelItem(level);

    if (current) {
        current->setIcon(0, QIcon());
    }
    if (next) {
        next->setIcon(0, QIcon::fromTheme(QStringLiteral("arrow-right")));
    }
    m_lastExecFrame = level;
}

void KatePluginGDBView::insertThread(int number, bool active)
{
    if (number < 0) {
        m_threadCombo->clear();
        m_activeThread = -1;
        return;
    }
    if (!active) {
        m_threadCombo->addItem(QIcon::fromTheme(QStringLiteral("")).pixmap(10, 10), i18n("Thread %1", number), number);
    } else {
        m_threadCombo->addItem(QIcon::fromTheme(QStringLiteral("arrow-right")).pixmap(10, 10), i18n("Thread %1", number), number);
        m_activeThread = m_threadCombo->count() - 1;
    }
    m_threadCombo->setCurrentIndex(m_activeThread);
}

void KatePluginGDBView::threadSelected(int thread)
{
    m_debugView->issueCommand(QStringLiteral("thread %1").arg(m_threadCombo->itemData(thread).toInt()));
}

QString KatePluginGDBView::currentWord()
{
    KTextEditor::View *kv = m_mainWin->activeView();
    if (!kv) {
        qDebug() << "no KTextEditor::View";
        return QString();
    }

    if (!kv->cursorPosition().isValid()) {
        qDebug() << "cursor not valid!";
        return QString();
    }

    int line = kv->cursorPosition().line();
    int col = kv->cursorPosition().column();

    QString linestr = kv->document()->line(line);

    int startPos = qMax(qMin(col, linestr.length() - 1), 0);
    int lindex = linestr.length() - 1;
    int endPos = startPos;
    while (startPos >= 0
           && (linestr[startPos].isLetterOrNumber() || linestr[startPos] == QLatin1Char('_') || linestr[startPos] == QLatin1Char('~')
               || ((startPos > 1) && (linestr[startPos] == QLatin1Char('.')) && !linestr[startPos - 1].isSpace())
               || ((startPos > 2) && (linestr[startPos] == QLatin1Char('>')) && (linestr[startPos - 1] == QLatin1Char('-'))
                   && !linestr[startPos - 2].isSpace()))) {
        if (linestr[startPos] == QLatin1Char('>')) {
            startPos--;
        }
        startPos--;
    }
    while (
        endPos < linestr.length()
        && (linestr[endPos].isLetterOrNumber() || linestr[endPos] == QLatin1Char('_')
            || ((endPos < lindex - 1) && (linestr[endPos] == QLatin1Char('.')) && !linestr[endPos + 1].isSpace())
            || ((endPos < lindex - 2) && (linestr[endPos] == QLatin1Char('-')) && (linestr[endPos + 1] == QLatin1Char('>')) && !linestr[endPos + 2].isSpace())
            || ((endPos > 1) && (linestr[endPos - 1] == QLatin1Char('-')) && (linestr[endPos] == QLatin1Char('>'))))) {
        if (linestr[endPos] == QLatin1Char('-')) {
            endPos++;
        }
        endPos++;
    }
    if (startPos == endPos) {
        qDebug() << "no word found!";
        return QString();
    }

    // qDebug() << linestr.mid(startPos+1, endPos-startPos-1);
    return linestr.mid(startPos + 1, endPos - startPos - 1);
}

void KatePluginGDBView::slotValue()
{
    QString variable;
    KTextEditor::View *editView = m_mainWin->activeView();
    if (editView && editView->selection() && editView->selectionRange().onSingleLine()) {
        variable = editView->selectionText();
    }

    if (variable.isEmpty()) {
        variable = currentWord();
    }

    if (variable.isEmpty()) {
        return;
    }

    QString cmd = QStringLiteral("print %1").arg(variable);
    m_debugView->issueCommand(cmd);
    m_inputArea->addToHistory(cmd);
    m_inputArea->setCurrentItem(QString());

    m_mainWin->showToolView(m_toolView.get());
    m_tabWidget->setCurrentWidget(m_gdbPage);

    QScrollBar *sb = m_outputArea->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void KatePluginGDBView::showIO(bool show)
{
    if (show) {
        m_tabWidget->addTab(m_ioView.get(), i18n("IO"));
    } else {
        m_tabWidget->removeTab(m_tabWidget->indexOf(m_ioView.get()));
    }
}

void KatePluginGDBView::addOutputText(QString const &text)
{
    QScrollBar *scrollb = m_outputArea->verticalScrollBar();
    if (!scrollb) {
        return;
    }
    bool atEnd = (scrollb->value() == scrollb->maximum());

    QTextCursor cursor = m_outputArea->textCursor();
    if (!cursor.atEnd()) {
        cursor.movePosition(QTextCursor::End);
    }
    cursor.insertText(text);

    if (atEnd) {
        scrollb->setValue(scrollb->maximum());
    }
}

void KatePluginGDBView::addErrorText(QString const &text)
{
    m_outputArea->setFontItalic(true);
    addOutputText(text);
    m_outputArea->setFontItalic(false);
}

bool KatePluginGDBView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if ((obj == m_toolView.get()) && (ke->key() == Qt::Key_Escape)) {
            m_mainWin->hideToolView(m_toolView.get());
            event->accept();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void KatePluginGDBView::handleEsc(QEvent *e)
{
    if (!m_mainWin || !m_toolView) {
        return;
    }

    QKeyEvent *k = static_cast<QKeyEvent *>(e);
    if (k->key() == Qt::Key_Escape && k->modifiers() == Qt::NoModifier) {
        if (m_toolView->isVisible()) {
            m_mainWin->hideToolView(m_toolView.get());
        }
    }
}

void KatePluginGDBView::displayMessage(const QString &msg, KTextEditor::Message::MessageType level)
{
    KTextEditor::View *kv = m_mainWin->activeView();
    if (!kv) {
        return;
    }

    delete m_infoMessage;
    m_infoMessage = new KTextEditor::Message(msg, level);
    m_infoMessage->setWordWrap(true);
    m_infoMessage->setPosition(KTextEditor::Message::BottomInView);
    m_infoMessage->setAutoHide(8000);
    m_infoMessage->setAutoHideMode(KTextEditor::Message::Immediate);
    m_infoMessage->setView(kv);
    kv->document()->postMessage(m_infoMessage);
}

#include "plugin_kategdb.moc"
