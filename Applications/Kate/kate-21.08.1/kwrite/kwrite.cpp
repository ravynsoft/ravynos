/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kwrite.h"

#include "config.h"
#include "kwriteapplication.h"

#include <ktexteditor/application.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/modificationinterface.h>

#include <KActionCollection>
#include <KConfig>
#include <KConfigGui>
#include <KEditToolBar>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRecentFilesAction>
#include <KShortcutsDialog>
#include <KSqueezedTextLabel>
#include <KStandardAction>
#include <KToggleAction>
#include <KXMLGUIFactory>

#ifdef KF5Activities_FOUND
#include <KActivities/ResourceInstance>
#endif

#include <QApplication>
#include <QDir>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFileOpenEvent>
#include <QLabel>
#include <QMenuBar>
#include <QMimeData>
#include <QTextCodec>
#include <QTimer>

KWrite::KWrite(KTextEditor::Document *doc, KWriteApplication *app)
    : m_app(app)
    , m_mainWindow(this)
{
    if (!doc) {
        doc = KTextEditor::Editor::instance()->createDocument(nullptr);

        // enable the modified on disk warning dialogs if any
        if (qobject_cast<KTextEditor::ModificationInterface *>(doc)) {
            qobject_cast<KTextEditor::ModificationInterface *>(doc)->setModifiedOnDiskWarning(true);
        }

        m_app->addDocument(doc);
    }

    m_view = doc->createView(this);

    setCentralWidget(m_view);

    setupActions();

    // signals for the statusbar
    connect(m_view->document(), &KTextEditor::Document::modifiedChanged, this, &KWrite::modifiedChanged);
    connect(m_view->document(), &KTextEditor::Document::documentNameChanged, this, &KWrite::documentNameChanged);
    connect(m_view->document(), &KTextEditor::Document::readWriteChanged, this, &KWrite::documentNameChanged);
    connect(m_view->document(), &KTextEditor::Document::documentUrlChanged, this, &KWrite::urlChanged);

    setAcceptDrops(true);
    // clang-format off
    connect(m_view, SIGNAL(dropEventPass(QDropEvent*)), this, SLOT(slotDropEvent(QDropEvent*)));
    // clang-format on

    setXMLFile(QStringLiteral("kwriteui.rc"));
    createShellGUI(true);
    guiFactory()->addClient(m_view);

    // FIXME: make sure the config dir exists, any idea how to do it more cleanly?
    QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).mkpath(QStringLiteral("."));

    // call it as last thing, must be sure everything is already set up ;)
    setAutoSaveSettings();

    readConfig();

    documentNameChanged();
    show();

    // give view focus
    m_view->setFocus(Qt::OtherFocusReason);

    /**
     * handle mac os x like file open request via event filter
     */
    qApp->installEventFilter(this);
}

KWrite::~KWrite()
{
    m_app->removeWindow(this);
    guiFactory()->removeClient(m_view);

    KTextEditor::Document *doc = m_view->document();
    delete m_view;

    // kill document, if last view is closed
    if (doc->views().isEmpty()) {
        m_app->removeDocument(doc);
        delete doc;
    }

    KSharedConfig::openConfig()->sync();
}

QSize KWrite::sizeHint() const
{
    /**
     * have some useful size hint, else we have mini windows per default
     */
    return (QSize(640, 480).expandedTo(minimumSizeHint()));
}

void KWrite::setupActions()
{
    m_closeAction = actionCollection()->addAction(KStandardAction::Close, QStringLiteral("file_close"), this, SLOT(slotFlush()));
    m_closeAction->setIcon(QIcon::fromTheme(QStringLiteral("document-close")));
    m_closeAction->setWhatsThis(i18n("Use this command to close the current document"));
    m_closeAction->setDisabled(true);

    // setup File menu
    actionCollection()
        ->addAction(KStandardAction::New, QStringLiteral("file_new"), this, SLOT(slotNew()))
        ->setWhatsThis(i18n("Use this command to create a new document"));
    actionCollection()
        ->addAction(KStandardAction::Open, QStringLiteral("file_open"), this, SLOT(slotOpen()))
        ->setWhatsThis(i18n("Use this command to open an existing document for editing"));

    m_recentFiles = KStandardAction::openRecent(this, QOverload<const QUrl &>::of(&KWrite::slotOpen), this);
    actionCollection()->addAction(m_recentFiles->objectName(), m_recentFiles);
    m_recentFiles->setWhatsThis(i18n("This lists files which you have opened recently, and allows you to easily open them again."));

    QAction *a = actionCollection()->addAction(QStringLiteral("view_new_view"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("window-new")));
    a->setText(i18n("&New Window"));
    connect(a, &QAction::triggered, this, &KWrite::newView);
    a->setWhatsThis(i18n("Create another view containing the current document"));

    actionCollection()->addAction(KStandardAction::Quit, this, SLOT(close()))->setWhatsThis(i18n("Close the current document view"));

    // setup Settings menu
    setStandardToolBarMenuEnabled(true);

    m_paShowMenuBar = KStandardAction::showMenubar(this, &KWrite::toggleMenuBar, actionCollection());

    m_paShowStatusBar = KStandardAction::showStatusbar(this, &KWrite::toggleStatusBar, this);
    actionCollection()->addAction(m_paShowStatusBar->objectName(), m_paShowStatusBar);
    m_paShowStatusBar->setWhatsThis(i18n("Use this command to show or hide the view's statusbar"));

    m_paShowPath = new KToggleAction(i18n("Sho&w Path in Titlebar"), this);
    actionCollection()->addAction(QStringLiteral("set_showPath"), m_paShowPath);
    connect(m_paShowPath, &QAction::triggered, this, &KWrite::documentNameChanged);
    m_paShowPath->setWhatsThis(i18n("Show the complete document path in the window caption"));

    a = actionCollection()->addAction(KStandardAction::KeyBindings, this, SLOT(editKeys()));
    a->setWhatsThis(i18n("Configure the application's keyboard shortcut assignments."));

    a = actionCollection()->addAction(KStandardAction::ConfigureToolbars, QStringLiteral("options_configure_toolbars"), this, SLOT(editToolbars()));
    a->setWhatsThis(i18n("Configure which items should appear in the toolbar(s)."));
}

// load on url
void KWrite::loadURL(const QUrl &url)
{
    m_view->document()->openUrl(url);

#ifdef KF5Activities_FOUND
    if (!m_activityResource) {
        m_activityResource = new KActivities::ResourceInstance(winId(), this);
    }
    m_activityResource->setUri(m_view->document()->url());
#endif

    m_closeAction->setEnabled(true);
}

// is closing the window wanted by user ?
bool KWrite::queryClose()
{
    if (m_view->document()->views().count() > 1) {
        return true;
    }

    if (m_view->document()->queryClose()) {
        writeConfig();

        return true;
    }

    return false;
}

void KWrite::slotFlush()
{
    if (m_view->document()->closeUrl()) {
        m_closeAction->setDisabled(true);
    }
}

void KWrite::modifiedChanged()
{
    documentNameChanged();
    m_closeAction->setEnabled(true);
}

void KWrite::slotNew()
{
    m_app->newWindow();
}

void KWrite::slotOpen()
{
    // if file is not local, then remove filename from url
    QList<QUrl> urls;
    if (m_view->document()->url().isLocalFile()) {
        urls = QFileDialog::getOpenFileUrls(this, i18n("Open File"), m_view->document()->url());
    } else {
        urls = QFileDialog::getOpenFileUrls(this, i18n("Open File"), m_view->document()->url().adjusted(QUrl::RemoveFilename));
    }
    for (const QUrl &url : urls) {
        slotOpen(url);
    }
}

void KWrite::slotOpen(const QUrl &url)
{
    if (url.isEmpty()) {
        return;
    }

    if (m_view->document()->isModified() || !m_view->document()->url().isEmpty()) {
        KWrite *t = m_app->newWindow();
        t->loadURL(url);
    } else {
        loadURL(url);
    }
}

void KWrite::urlChanged()
{
    if (!m_view->document()->url().isEmpty()) {
        m_recentFiles->addUrl(m_view->document()->url());
    }

    // update caption
    documentNameChanged();
}

void KWrite::newView()
{
    m_app->newWindow(m_view->document());
}

void KWrite::toggleMenuBar(bool showMessage)
{
    if (m_paShowMenuBar->isChecked()) {
        menuBar()->show();
        if (m_view->contextMenu()) {
            m_view->contextMenu()->removeAction(m_paShowMenuBar);
        }
    } else {
        if (showMessage) {
            const QString accel = m_paShowMenuBar->shortcut().toString();
            KMessageBox::information(this,
                                     i18n("This will hide the menu bar completely."
                                          " You can show it again by typing %1.",
                                          accel),
                                     i18n("Hide menu bar"),
                                     QStringLiteral("HideMenuBarWarning"));
        }
        menuBar()->hide();
        if (m_view->contextMenu()) {
            m_view->contextMenu()->addAction(m_paShowMenuBar);
        }
    }
}

void KWrite::toggleStatusBar()
{
    m_view->setStatusBarEnabled(m_paShowStatusBar->isChecked());
}

void KWrite::editKeys()
{
    KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);
    dlg.addCollection(actionCollection());
    if (m_view) {
        dlg.addCollection(m_view->actionCollection());
    }
    dlg.configure();
}

void KWrite::editToolbars()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("MainWindow");
    saveMainWindowSettings(cfg);
    KEditToolBar dlg(guiFactory(), this);

    connect(&dlg, &KEditToolBar::newToolBarConfig, this, &KWrite::slotNewToolbarConfig);
    dlg.exec();
}

void KWrite::slotNewToolbarConfig()
{
    applyMainWindowSettings(KSharedConfig::openConfig()->group("MainWindow"));
}

void KWrite::dragEnterEvent(QDragEnterEvent *event)
{
    const QList<QUrl> uriList = event->mimeData()->urls();
    event->setAccepted(!uriList.isEmpty());
}

void KWrite::dropEvent(QDropEvent *event)
{
    slotDropEvent(event);
}

void KWrite::slotDropEvent(QDropEvent *event)
{
    const QList<QUrl> textlist = event->mimeData()->urls();

    for (const QUrl &url : textlist) {
        slotOpen(url);
    }
}

void KWrite::slotEnableActions(bool enable)
{
    QList<QAction *> actions = actionCollection()->actions();
    QList<QAction *>::ConstIterator it = actions.constBegin();
    QList<QAction *>::ConstIterator end = actions.constEnd();

    for (; it != end; ++it) {
        (*it)->setEnabled(enable);
    }

    actions = m_view->actionCollection()->actions();
    it = actions.constBegin();
    end = actions.constEnd();

    for (; it != end; ++it) {
        (*it)->setEnabled(enable);
    }
}

// common config
void KWrite::readConfig(KSharedConfigPtr config)
{
    KConfigGroup cfg(config, "General Options");

    m_paShowMenuBar->setChecked(cfg.readEntry("ShowMenuBar", true));
    m_paShowStatusBar->setChecked(cfg.readEntry("ShowStatusBar", true));
    m_paShowPath->setChecked(cfg.readEntry("ShowPath", false));

    m_recentFiles->loadEntries(config->group("Recent Files"));

    // update visibility of menu bar and status bar
    toggleMenuBar(false);
    m_view->setStatusBarEnabled(m_paShowStatusBar->isChecked());
}

void KWrite::writeConfig(KSharedConfigPtr config)
{
    KConfigGroup generalOptions(config, "General Options");

    generalOptions.writeEntry("ShowMenuBar", m_paShowMenuBar->isChecked());
    generalOptions.writeEntry("ShowStatusBar", m_paShowStatusBar->isChecked());
    generalOptions.writeEntry("ShowPath", m_paShowPath->isChecked());

    m_recentFiles->saveEntries(KConfigGroup(config, "Recent Files"));

    config->sync();
}

// config file
void KWrite::readConfig()
{
    readConfig(KSharedConfig::openConfig());
}

void KWrite::writeConfig()
{
    writeConfig(KSharedConfig::openConfig());
}

// session management
void KWrite::restore(KConfig *config, int n)
{
    readPropertiesInternal(config, n);
}

void KWrite::readProperties(const KConfigGroup &config)
{
    readConfig();

    m_view->readSessionConfig(KConfigGroup(&config, QStringLiteral("General Options")));
}

void KWrite::saveProperties(KConfigGroup &config)
{
    writeConfig();

    config.writeEntry("DocumentNumber", m_app->documents().indexOf(m_view->document()) + 1);

    KConfigGroup cg(&config, QStringLiteral("General Options"));
    m_view->writeSessionConfig(cg);
}

void KWrite::saveGlobalProperties(KConfig *config) // save documents
{
    m_app->saveProperties(config);
}

void KWrite::documentNameChanged()
{
    QString readOnlyCaption;
    if (!m_view->document()->isReadWrite()) {
        readOnlyCaption = i18n(" [read only]");
    }

    if (m_view->document()->url().isEmpty()) {
        setCaption(i18n("Untitled") + readOnlyCaption + QStringLiteral(" [*]"), m_view->document()->isModified());
        return;
    }

    QString c;

    if (m_paShowPath->isChecked()) {
        c = m_view->document()->url().toString(QUrl::PreferLocalFile);

        const QString homePath = QDir::homePath();
        if (c.startsWith(homePath)) {
            c = QLatin1String("~") + c.right(c.length() - homePath.length());
        }

        // File name shouldn't be too long - Maciek
        if (c.length() > 64) {
            c = QLatin1String("...") + c.right(64);
        }
    } else {
        c = m_view->document()->url().fileName();

        // File name shouldn't be too long - Maciek
        if (c.length() > 64) {
            c = c.left(64) + QStringLiteral("...");
        }
    }

    setCaption(c + readOnlyCaption + QStringLiteral(" [*]"), m_view->document()->isModified());
}

bool KWrite::eventFilter(QObject *obj, QEvent *event)
{
    /**
     * handle mac os like file open
     */
    if (event->type() == QEvent::FileOpen) {
        /**
         * try to open and activate the new document, like we would do for stuff
         * opened via file dialog
         */
        QFileOpenEvent *foe = static_cast<QFileOpenEvent *>(event);
        slotOpen(foe->url());
        return true;
    }

    /**
     * else: pass over to default implementation
     */
    return KParts::MainWindow::eventFilter(obj, event);
}

QList<KTextEditor::View *> KWrite::views()
{
    QList<KTextEditor::View *> list;
    list.append(m_view);
    return list;
}

KTextEditor::View *KWrite::activateView(KTextEditor::Document *document)
{
    if (m_view->document() == document) {
        return m_view;
    }

    return nullptr;
}
