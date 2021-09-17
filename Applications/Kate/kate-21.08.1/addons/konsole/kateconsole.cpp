/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
   SPDX-FileCopyrightText: 2007 Anders Lund <anders@alweb.dk>
   SPDX-FileCopyrightText: 2017 Ederag <edera@gmx.fr>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kateconsole.h"

#include <KLocalizedString>
#include <ktexteditor/document.h>
#include <ktexteditor/message.h>
#include <ktexteditor/view.h>

#include <KActionCollection>
#include <KShell>
#include <QAction>
#include <kde_terminal_interface.h>
#include <kparts/part.h>

#include <KMessageBox>

#include <QApplication>
#include <QCheckBox>
#include <QFileInfo>
#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QShowEvent>
#include <QStyle>
#include <QVBoxLayout>

#include <KAboutData>
#include <KAuthorized>
#include <KConfigGroup>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KSharedConfig>
#include <KXMLGUIFactory>

K_PLUGIN_FACTORY_WITH_JSON(KateKonsolePluginFactory, "katekonsoleplugin.json", registerPlugin<KateKonsolePlugin>();)

static const QStringList s_escapeExceptions{QStringLiteral("vi"), QStringLiteral("vim"), QStringLiteral("nvim")};

KateKonsolePlugin::KateKonsolePlugin(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
{
    m_previousEditorEnv = qgetenv("EDITOR");
    if (!KAuthorized::authorize(QStringLiteral("shell_access"))) {
        KMessageBox::sorry(nullptr, i18n("You do not have enough karma to access a shell or terminal emulation"));
    }
}

void setEditorEnv(const QByteArray &value)
{
    if (value.isNull()) {
        qunsetenv("EDITOR");
    } else {
        qputenv("EDITOR", value.data());
    }
}

KateKonsolePlugin::~KateKonsolePlugin()
{
    setEditorEnv(m_previousEditorEnv);
}

QObject *KateKonsolePlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    KateKonsolePluginView *view = new KateKonsolePluginView(this, mainWindow);
    return view;
}

KTextEditor::ConfigPage *KateKonsolePlugin::configPage(int number, QWidget *parent)
{
    if (number != 0) {
        return nullptr;
    }
    return new KateKonsoleConfigPage(parent, this);
}

void KateKonsolePlugin::readConfig()
{
    for (KateKonsolePluginView *view : qAsConst(mViews)) {
        view->readConfig();
    }
}

KateKonsolePluginView::KateKonsolePluginView(KateKonsolePlugin *plugin, KTextEditor::MainWindow *mainWindow)
    : QObject(mainWindow)
    , m_plugin(plugin)
{
    // init console
    QWidget *toolview = mainWindow->createToolView(plugin,
                                                   QStringLiteral("kate_private_plugin_katekonsoleplugin"),
                                                   KTextEditor::MainWindow::Bottom,
                                                   QIcon::fromTheme(QStringLiteral("dialog-scripts")),
                                                   i18n("Terminal Panel"));
    m_console = new KateConsole(m_plugin, mainWindow, toolview);

    // register this view
    m_plugin->mViews.append(this);
}

KateKonsolePluginView::~KateKonsolePluginView()
{
    // unregister this view
    m_plugin->mViews.removeAll(this);

    // cleanup, kill toolview + console
    QWidget *toolview = m_console->parentWidget();
    delete m_console;
    delete toolview;
}

void KateKonsolePluginView::readConfig()
{
    m_console->readConfig();
}

KateConsole::KateConsole(KateKonsolePlugin *plugin, KTextEditor::MainWindow *mw, QWidget *parent)
    : QWidget(parent)
    , m_part(nullptr)
    , m_mw(mw)
    , m_toolView(parent)
    , m_plugin(plugin)
{
    KXMLGUIClient::setComponentName(QStringLiteral("katekonsole"), i18n("Kate Terminal"));
    setXMLFile(QStringLiteral("ui.rc"));

    // make sure we have a vertical layout
    new QVBoxLayout(this);
    layout()->setContentsMargins(0, 0, 0, 0);

    QAction *a = actionCollection()->addAction(QStringLiteral("katekonsole_tools_pipe_to_terminal"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("dialog-scripts")));
    a->setText(i18nc("@action", "&Pipe to Terminal"));
    connect(a, &QAction::triggered, this, &KateConsole::slotPipeToConsole);

    a = actionCollection()->addAction(QStringLiteral("katekonsole_tools_sync"));
    a->setText(i18nc("@action", "S&ynchronize Terminal with Current Document"));
    connect(a, &QAction::triggered, this, &KateConsole::slotManualSync);

    a = actionCollection()->addAction(QStringLiteral("katekonsole_tools_run"));
    a->setText(i18nc("@action", "Run Current Document"));
    connect(a, &QAction::triggered, this, &KateConsole::slotRun);

    a = actionCollection()->addAction(QStringLiteral("katekonsole_tools_toggle_visibility"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("dialog-scripts")));
    a->setText(i18nc("@action", "S&how Terminal Panel"));
    actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::Key_F4));
    connect(a, &QAction::triggered, this, &KateConsole::slotToggleVisibility);

    a = actionCollection()->addAction(QStringLiteral("katekonsole_tools_toggle_focus"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("swap-panels")));
    a->setText(i18nc("@action", "&Focus Terminal Panel"));
    actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F4));
    connect(a, &QAction::triggered, this, &KateConsole::slotToggleFocus);

    connect(m_mw, &KTextEditor::MainWindow::unhandledShortcutOverride, this, &KateConsole::handleEsc);

    m_mw->guiFactory()->addClient(this);

    readConfig();
}

KateConsole::~KateConsole()
{
    m_mw->guiFactory()->removeClient(this);
    if (m_part) {
        disconnect(m_part, &KParts::ReadOnlyPart::destroyed, this, &KateConsole::slotDestroyed);
    }
}

void KateConsole::loadConsoleIfNeeded()
{
    if (m_part) {
        return;
    }

    if (!window() || !parentWidget()) {
        return;
    }
    if (!window() || !isVisibleTo(window())) {
        return;
    }

    /**
     * get konsole part factory
     */
    KPluginFactory *factory = KPluginLoader(QStringLiteral("konsolepart")).factory();
    if (!factory) {
        return;
    }

    m_part = factory->create<KParts::ReadOnlyPart>(this, this);

    if (!m_part) {
        return;
    }

    layout()->addWidget(m_part->widget());

    // start the terminal
    qobject_cast<TerminalInterface *>(m_part)->showShellInDir(QString());

    //   KGlobal::locale()->insertCatalog("konsole"); // FIXME KF5: insert catalog

    setFocusProxy(m_part->widget());
    m_part->widget()->show();

    connect(m_part, &KParts::ReadOnlyPart::destroyed, this, &KateConsole::slotDestroyed);
    // clang-format off
    connect(m_part, SIGNAL(overrideShortcut(QKeyEvent*,bool&)), this, SLOT(overrideShortcut(QKeyEvent*,bool&)));
    // clang-format on
    slotSync();
}

void KateConsole::slotDestroyed()
{
    m_part = nullptr;
    m_currentPath.clear();
    setFocusProxy(nullptr);

    // hide the dockwidget
    if (parentWidget()) {
        m_mw->hideToolView(m_toolView);
    }
}

void KateConsole::overrideShortcut(QKeyEvent *, bool &override)
{
    /**
     * let konsole handle all shortcuts
     */
    override = true;
}

void KateConsole::showEvent(QShowEvent *)
{
    if (m_part) {
        return;
    }

    loadConsoleIfNeeded();
}

void KateConsole::cd(const QString &path)
{
    if (m_currentPath == path) {
        return;
    }

    if (!m_part) {
        return;
    }

    m_currentPath = path;
    QString command = QLatin1String(" cd ") + KShell::quoteArg(m_currentPath) + QLatin1Char('\n');

    // special handling for some interpreters
    TerminalInterface *t = qobject_cast<TerminalInterface *>(m_part);
    if (t) {
        // ghci doesn't allow \space dir names, does allow spaces in dir names
        // irb can take spaces or \space but doesn't allow " 'path' "
        if (t->foregroundProcessName() == QLatin1String("irb")) {
            command = QLatin1String("Dir.chdir(\"") + path + QLatin1String("\") \n");
        } else if (t->foregroundProcessName() == QLatin1String("ghc")) {
            command = QLatin1String(":cd ") + path + QLatin1Char('\n');
        }
    }

    // Send prior Ctrl-E, Ctrl-U to ensure the line is empty
    sendInput(QStringLiteral("\x05\x15"));
    sendInput(command);
}

void KateConsole::sendInput(const QString &text)
{
    loadConsoleIfNeeded();

    if (!m_part) {
        return;
    }

    TerminalInterface *t = qobject_cast<TerminalInterface *>(m_part);

    if (!t) {
        return;
    }

    t->sendInput(text);
}

void KateConsole::slotPipeToConsole()
{
    if (KMessageBox::warningContinueCancel(
            m_mw->window(),
            i18n("Do you really want to pipe the text to the console? This will execute any contained commands with your user rights."),
            i18n("Pipe to Terminal?"),
            KGuiItem(i18n("Pipe to Terminal")),
            KStandardGuiItem::cancel(),
            QStringLiteral("Pipe To Terminal Warning"))
        != KMessageBox::Continue) {
        return;
    }

    KTextEditor::View *v = m_mw->activeView();

    if (!v) {
        return;
    }

    if (v->selection()) {
        sendInput(v->selectionText());
    } else {
        sendInput(v->document()->text());
    }
}

void KateConsole::slotSync()
{
    if (m_mw->activeView()) {
        QUrl u = m_mw->activeView()->document()->url();
        if (u.isValid() && u.isLocalFile()) {
            QFileInfo fi(u.toLocalFile());
            cd(fi.absolutePath());
        } else if (!u.isEmpty()) {
            sendInput(QStringLiteral("### ") + i18n("Sorry, cannot cd into '%1'", u.toLocalFile()) + QLatin1Char('\n'));
        }
    }
}

void KateConsole::slotViewOrUrlChanged(KTextEditor::View *view)
{
    disconnect(m_urlChangedConnection);
    if (view) {
        KTextEditor::Document *doc = view->document();
        m_urlChangedConnection = connect(doc, &KParts::ReadOnlyPart::urlChanged, this, &KateConsole::slotSync);
    }

    slotSync();
}

void KateConsole::slotManualSync()
{
    m_currentPath.clear();
    slotSync();
    if (!m_part || !m_part->widget()->isVisible()) {
        m_mw->showToolView(parentWidget());
    }
}

void KateConsole::slotRun()
{
    if (m_mw->activeView()) {
        KTextEditor::Document *document = m_mw->activeView()->document();
        QUrl u = document->url();
        if (!u.isLocalFile()) {
            QPointer<KTextEditor::Message> message = new KTextEditor::Message(i18n("Not a local file: '%1'", u.path()), KTextEditor::Message::Error);
            // auto hide is enabled and set to a sane default value of several seconds.
            message->setAutoHide(2000);
            message->setAutoHideMode(KTextEditor::Message::Immediate);
            document->postMessage(message);
            return;
        }
        // ensure that file is saved
        if (document->isModified()) {
            document->save();
        }

        // The string that should be output to terminal, upon acceptance
        QString output_str;
        // Set prefix first
        QString first_line = document->line(0);
        QString shebang = QString::fromLatin1("#!");
        if (first_line.startsWith(shebang)) {
            // If there's a shebang, respect it
            output_str += first_line.remove(shebang).append(QLatin1Char(' '));
        } else {
            output_str += KConfigGroup(KSharedConfig::openConfig(), "Konsole").readEntry("RunPrefix", "");
        }
        // then filename
        QFileInfo file_path = QFileInfo(u.path());
        if (KConfigGroup(KSharedConfig::openConfig(), "Konsole").readEntry("RemoveExtension", true)) {
            // append filename without extension (i.e. keep only the basename)
            output_str += file_path.absoluteFilePath().remove(file_path.suffix());
        } else {
            // append filename to the terminal
            output_str += file_path.absoluteFilePath();
        }

        if (KMessageBox::Continue
            != KMessageBox::warningContinueCancel(m_mw->window(),
                                                  i18n("Do you really want to Run the document ?\n"
                                                       "This will execute the following command,\n"
                                                       "with your user rights, in the terminal:\n"
                                                       "'%1'",
                                                       output_str),
                                                  i18n("Run in Terminal?"),
                                                  KGuiItem(i18n("Run")),
                                                  KStandardGuiItem::cancel(),
                                                  QStringLiteral("Konsole: Run in Terminal Warning"))) {
            return;
        }
        // echo to terminal
        sendInput(output_str + QLatin1Char('\n'));
    }
}

void KateConsole::slotToggleVisibility()
{
    QAction *action = actionCollection()->action(QStringLiteral("katekonsole_tools_toggle_visibility"));
    if (!m_part || !m_part->widget()->isVisible()) {
        m_mw->showToolView(parentWidget());
        action->setText(i18nc("@action", "&Hide Terminal Panel"));
    } else {
        m_mw->hideToolView(m_toolView);
        action->setText(i18nc("@action", "S&how Terminal Panel"));
    }
}

void KateConsole::slotToggleFocus()
{
    QAction *action = actionCollection()->action(QStringLiteral("katekonsole_tools_toggle_focus"));
    if (!m_part) {
        m_mw->showToolView(parentWidget());
        action->setText(i18n("Defocus Terminal Panel"));
        return; // this shows and focuses the konsole
    }

    if (!m_part) {
        return;
    }

    if (m_part->widget()->hasFocus()) {
        if (m_mw->activeView()) {
            m_mw->activeView()->setFocus();
        }
        action->setText(i18n("Focus Terminal Panel"));
    } else {
        // show the view if it is hidden
        if (parentWidget()->isHidden()) {
            m_mw->showToolView(parentWidget());
        } else { // should focus the widget too!
            m_part->widget()->setFocus(Qt::OtherFocusReason);
        }
        action->setText(i18n("Defocus Terminal Panel"));
    }
}

void KateConsole::readConfig()
{
    disconnect(m_mw, &KTextEditor::MainWindow::viewChanged, this, &KateConsole::slotViewOrUrlChanged);
    disconnect(m_urlChangedConnection);

    if (KConfigGroup(KSharedConfig::openConfig(), "Konsole").readEntry("AutoSyncronize", true)) {
        connect(m_mw, &KTextEditor::MainWindow::viewChanged, this, &KateConsole::slotViewOrUrlChanged);
    }

    if (KConfigGroup(KSharedConfig::openConfig(), "Konsole").readEntry("SetEditor", false)) {
        qputenv("EDITOR", "kate -b");
    } else {
        setEditorEnv(m_plugin->previousEditorEnv());
    }
}

void KateConsole::handleEsc(QEvent *e)
{
    if (!KConfigGroup(KSharedConfig::openConfig(), "Konsole").readEntry("KonsoleEscKeyBehaviour", true)) {
        return;
    }

    QStringList exceptList = KConfigGroup(KSharedConfig::openConfig(), "Konsole").readEntry("KonsoleEscKeyExceptions", s_escapeExceptions);

    if (!m_mw || !m_part || !m_toolView || !e) {
        return;
    }

    QKeyEvent *k = static_cast<QKeyEvent *>(e);
    if (k->key() == Qt::Key_Escape && k->modifiers() == Qt::NoModifier) {
        const auto app = qobject_cast<TerminalInterface *>(m_part)->foregroundProcessName();
        if (m_toolView && m_toolView->isVisible() && !exceptList.contains(app)) {
            m_mw->hideToolView(m_toolView);
        }
    }
}

KateKonsoleConfigPage::KateKonsoleConfigPage(QWidget *parent, KateKonsolePlugin *plugin)
    : KTextEditor::ConfigPage(parent)
    , mPlugin(plugin)
{
    QVBoxLayout *lo = new QVBoxLayout(this);
    lo->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing));
    lo->setContentsMargins(0, 0, 0, 0);

    cbAutoSyncronize = new QCheckBox(i18n("&Automatically synchronize the terminal with the current document when possible"), this);
    lo->addWidget(cbAutoSyncronize);

    QVBoxLayout *vboxRun = new QVBoxLayout;
    QGroupBox *groupRun = new QGroupBox(i18n("Run in terminal"), this);
    // Remove extension
    cbRemoveExtension = new QCheckBox(i18n("&Remove extension"), this);
    vboxRun->addWidget(cbRemoveExtension);
    // Prefix
    QFrame *framePrefix = new QFrame(this);
    QHBoxLayout *hboxPrefix = new QHBoxLayout(framePrefix);
    QLabel *label = new QLabel(i18n("Prefix:"), framePrefix);
    hboxPrefix->addWidget(label);
    lePrefix = new QLineEdit(framePrefix);
    hboxPrefix->addWidget(lePrefix);
    vboxRun->addWidget(framePrefix);
    // show warning next time
    QFrame *frameWarn = new QFrame(this);
    QHBoxLayout *hboxWarn = new QHBoxLayout(frameWarn);
    QPushButton *buttonWarn = new QPushButton(i18n("&Show warning next time"), frameWarn);
    buttonWarn->setWhatsThis(
        i18n("The next time '%1' is executed, "
             "make sure a warning window will pop up, "
             "displaying the command to be sent to terminal, "
             "for review.",
             i18n("Run in terminal")));
    connect(buttonWarn, &QPushButton::pressed, this, &KateKonsoleConfigPage::slotEnableRunWarning);
    hboxWarn->addWidget(buttonWarn);
    vboxRun->addWidget(frameWarn);
    groupRun->setLayout(vboxRun);
    lo->addWidget(groupRun);

    cbSetEditor = new QCheckBox(i18n("Set &EDITOR environment variable to 'kate -b'"), this);
    lo->addWidget(cbSetEditor);
    QLabel *tmp = new QLabel(this);
    tmp->setText(i18n("Important: The document has to be closed to make the console application continue"));
    lo->addWidget(tmp);

    cbSetEscHideKonsole = new QCheckBox(i18n("Hide Konsole on pressing 'Esc'"));
    lo->addWidget(cbSetEscHideKonsole);
    QLabel *hideKonsoleLabel =
        new QLabel(i18n("This may cause issues with terminal apps that use Esc key, for e.g., vim. Add these apps in the input below (Comma separated list)"),
                   this);
    lo->addWidget(hideKonsoleLabel);

    leEscExceptions = new QLineEdit(this);
    lo->addWidget(leEscExceptions);

    reset();
    lo->addStretch();

    connect(cbAutoSyncronize, &QCheckBox::stateChanged, this, &KateKonsoleConfigPage::changed);
    connect(cbRemoveExtension, &QCheckBox::stateChanged, this, &KTextEditor::ConfigPage::changed);
    connect(lePrefix, &QLineEdit::textChanged, this, &KateKonsoleConfigPage::changed);
    connect(cbSetEditor, &QCheckBox::stateChanged, this, &KateKonsoleConfigPage::changed);
    connect(cbSetEscHideKonsole, &QCheckBox::stateChanged, this, &KateKonsoleConfigPage::changed);
    connect(leEscExceptions, &QLineEdit::textChanged, this, &KateKonsoleConfigPage::changed);
}

void KateKonsoleConfigPage::slotEnableRunWarning()
{
    KMessageBox::enableMessage(QStringLiteral("Konsole: Run in Terminal Warning"));
}

QString KateKonsoleConfigPage::name() const
{
    return i18n("Terminal");
}

QString KateKonsoleConfigPage::fullName() const
{
    return i18n("Terminal Settings");
}

QIcon KateKonsoleConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("utilities-terminal"));
}

void KateKonsoleConfigPage::apply()
{
    KConfigGroup config(KSharedConfig::openConfig(), "Konsole");
    config.writeEntry("AutoSyncronize", cbAutoSyncronize->isChecked());
    config.writeEntry("RemoveExtension", cbRemoveExtension->isChecked());
    config.writeEntry("RunPrefix", lePrefix->text());
    config.writeEntry("SetEditor", cbSetEditor->isChecked());
    config.writeEntry("KonsoleEscKeyBehaviour", cbSetEscHideKonsole->isChecked());
    config.writeEntry("KonsoleEscKeyExceptions", leEscExceptions->text().split(QLatin1Char(',')));
    config.sync();
    mPlugin->readConfig();
}

void KateKonsoleConfigPage::reset()
{
    KConfigGroup config(KSharedConfig::openConfig(), "Konsole");
    cbAutoSyncronize->setChecked(config.readEntry("AutoSyncronize", true));
    cbRemoveExtension->setChecked(config.readEntry("RemoveExtension", false));
    lePrefix->setText(config.readEntry("RunPrefix", ""));
    cbSetEditor->setChecked(config.readEntry("SetEditor", false));
    cbSetEscHideKonsole->setChecked(config.readEntry("KonsoleEscKeyBehaviour", true));
    leEscExceptions->setText(config.readEntry("KonsoleEscKeyExceptions", s_escapeExceptions).join(QLatin1Char(',')));
}

#include "kateconsole.moc"

// kate: space-indent on; indent-width 4; replace-tabs on;
