/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2010 Thomas Fjellstrom <thomas@fjellstrom.ca>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// BEGIN Includes

#include "katefiletreeplugin.h"
#include "katefiletree.h"
#include "katefiletreeconfigpage.h"
#include "katefiletreemodel.h"
#include "katefiletreeproxymodel.h"

#include <ktexteditor/application.h>
#include <ktexteditor/view.h>

#include <KAboutData>
#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KToolBar>
#include <KXMLGUIFactory>
#include <KXmlGuiWindow>

#include <QAction>
#include <QApplication>
#include <QLineEdit>
#include <QStyle>
#include <QTimer>

#include "katefiletreedebug.h"

// END Includes

K_PLUGIN_FACTORY_WITH_JSON(KateFileTreeFactory, "katefiletreeplugin.json", registerPlugin<KateFileTreePlugin>();)

Q_LOGGING_CATEGORY(FILETREE, "kate-filetree", QtWarningMsg)

// BEGIN KateFileTreePlugin
KateFileTreePlugin::KateFileTreePlugin(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
{
}

KateFileTreePlugin::~KateFileTreePlugin()
{
    m_settings.save();
}

QObject *KateFileTreePlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    KateFileTreePluginView *view = new KateFileTreePluginView(mainWindow, this);
    connect(view, &KateFileTreePluginView::destroyed, this, &KateFileTreePlugin::viewDestroyed);
    m_views.append(view);

    return view;
}

void KateFileTreePlugin::viewDestroyed(QObject *view)
{
    // do not access the view pointer, since it is partially destroyed already
    m_views.removeAll(static_cast<KateFileTreePluginView *>(view));
}

int KateFileTreePlugin::configPages() const
{
    return 1;
}

KTextEditor::ConfigPage *KateFileTreePlugin::configPage(int number, QWidget *parent)
{
    if (number != 0) {
        return nullptr;
    }

    KateFileTreeConfigPage *page = new KateFileTreeConfigPage(parent, this);
    return page;
}

const KateFileTreePluginSettings &KateFileTreePlugin::settings()
{
    return m_settings;
}

void KateFileTreePlugin::applyConfig(bool shadingEnabled,
                                     const QColor &viewShade,
                                     const QColor &editShade,
                                     bool listMode,
                                     int sortRole,
                                     bool showFullPath,
                                     bool showToolbar,
                                     bool showCloseButton)
{
    // save to settings
    m_settings.setShadingEnabled(shadingEnabled);
    m_settings.setViewShade(viewShade);
    m_settings.setEditShade(editShade);

    m_settings.setListMode(listMode);
    m_settings.setSortRole(sortRole);
    m_settings.setShowFullPathOnRoots(showFullPath);
    m_settings.setShowToolbar(showToolbar);
    m_settings.setShowCloseButton(showCloseButton);
    m_settings.save();

    // update views
    for (KateFileTreePluginView *view : qAsConst(m_views)) {
        view->setHasLocalPrefs(false);
        view->model()->setShadingEnabled(shadingEnabled);
        view->model()->setViewShade(viewShade);
        view->model()->setEditShade(editShade);
        view->setListMode(listMode);
        view->proxy()->setSortRole(sortRole);
        view->model()->setShowFullPathOnRoots(showFullPath);
        view->setToolbarVisible(showToolbar);
        view->tree()->setShowCloseButton(showCloseButton);
    }
}

// END KateFileTreePlugin

// BEGIN KateFileTreePluginView
KateFileTreePluginView::KateFileTreePluginView(KTextEditor::MainWindow *mainWindow, KateFileTreePlugin *plug)
    : QObject(mainWindow)
    , m_plug(plug)
    , m_mainWindow(mainWindow)
{
    KXMLGUIClient::setComponentName(QStringLiteral("katefiletree"), i18n("Kate File Tree"));
    setXMLFile(QStringLiteral("ui.rc"));

    m_toolView = mainWindow->createToolView(plug,
                                            QStringLiteral("kate_private_plugin_katefiletreeplugin"),
                                            KTextEditor::MainWindow::Left,
                                            QIcon::fromTheme(QStringLiteral("document-open")),
                                            i18n("Documents"));

    // create toolbar
    m_toolbar = new KToolBar(m_toolView);
    m_toolbar->setMovable(false);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolbar->setContextMenuPolicy(Qt::NoContextMenu);

    // ensure reasonable icons sizes, like e.g. the quick-open and co. icons
    // the normal toolbar sizes are TOO large, e.g. for scaled stuff even more!
    const int iconSize = m_toolView->style()->pixelMetric(QStyle::PM_ButtonIconSize, nullptr, m_toolView);
    m_toolbar->setIconSize(QSize(iconSize, iconSize));

    // create filetree
    m_fileTree = new KateFileTree(m_toolView);
    m_fileTree->setSortingEnabled(true);
    m_fileTree->setShowCloseButton(m_plug->settings().showCloseButton());

    connect(m_fileTree, &KateFileTree::activateDocument, this, &KateFileTreePluginView::activateDocument);
    connect(m_fileTree, &KateFileTree::viewModeChanged, this, &KateFileTreePluginView::viewModeChanged);
    connect(m_fileTree, &KateFileTree::sortRoleChanged, this, &KateFileTreePluginView::sortRoleChanged);

    m_documentModel = new KateFileTreeModel(this);
    m_proxyModel = new KateFileTreeProxyModel(this);
    m_proxyModel->setSourceModel(m_documentModel);
    m_proxyModel->setDynamicSortFilter(true);
    m_proxyModel->setRecursiveFilteringEnabled(true);

    m_documentModel->setShowFullPathOnRoots(m_plug->settings().showFullPathOnRoots());
    m_documentModel->setShadingEnabled(m_plug->settings().shadingEnabled());
    m_documentModel->setViewShade(m_plug->settings().viewShade());
    m_documentModel->setEditShade(m_plug->settings().editShade());

    m_filter = new QLineEdit(m_toolView);
    m_filter->setPlaceholderText(QStringLiteral("Filter..."));
    m_filter->setClearButtonEnabled(true);
    connect(m_filter, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_proxyModel->setFilterRegularExpression(QRegularExpression(text, QRegularExpression::CaseInsensitiveOption));
        if (!text.isEmpty()) {
            QTimer::singleShot(100, m_fileTree, &QTreeView::expandAll);
        }
    });

    connect(KTextEditor::Editor::instance()->application(),
            &KTextEditor::Application::documentWillBeDeleted,
            m_documentModel,
            &KateFileTreeModel::documentClosed);
    connect(KTextEditor::Editor::instance()->application(), &KTextEditor::Application::documentCreated, this, &KateFileTreePluginView::documentOpened);
    connect(KTextEditor::Editor::instance()->application(), &KTextEditor::Application::documentWillBeDeleted, this, &KateFileTreePluginView::documentClosed);

    // delayed update for new documents to be more efficient if multiple ones are created at once
    m_documentsCreatedTimer.setSingleShot(true);
    m_documentsCreatedTimer.setInterval(0);
    connect(&m_documentsCreatedTimer, &QTimer::timeout, this, &KateFileTreePluginView::slotDocumentsCreated);

    connect(m_documentModel, &KateFileTreeModel::triggerViewChangeAfterNameChange, [=] {
        KateFileTreePluginView::viewChanged();
    });

    m_fileTree->setModel(m_proxyModel);

    m_fileTree->setDragEnabled(false);
    m_fileTree->setDragDropMode(QAbstractItemView::InternalMove);
    m_fileTree->setDropIndicatorShown(false);

    m_fileTree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_fileTree->selectionModel(), &QItemSelectionModel::currentChanged, m_fileTree, &KateFileTree::slotCurrentChanged);

    connect(mainWindow, &KTextEditor::MainWindow::viewChanged, this, &KateFileTreePluginView::viewChanged);

    //
    // actions
    //
    setupActions();

    mainWindow->guiFactory()->addClient(this);

    setToolbarVisible(m_plug->settings().showToolbar());

    m_proxyModel->setSortRole(Qt::DisplayRole);

    m_proxyModel->sort(0, Qt::AscendingOrder);
    m_proxyModel->invalidate();
}

KateFileTreePluginView::~KateFileTreePluginView()
{
    m_mainWindow->guiFactory()->removeClient(this);

    // clean up tree and toolview
    delete m_fileTree->parentWidget();
    // delete m_toolView;
    // and TreeModel
    delete m_documentModel;
}

void KateFileTreePluginView::setupActions()
{
    auto aPrev = actionCollection()->addAction(QStringLiteral("filetree_prev_document"));
    aPrev->setText(i18n("Previous Document"));
    aPrev->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    actionCollection()->setDefaultShortcut(aPrev, Qt::ALT | Qt::Key_Up);
    connect(aPrev, &QAction::triggered, m_fileTree, &KateFileTree::slotDocumentPrev);

    auto aNext = actionCollection()->addAction(QStringLiteral("filetree_next_document"));
    aNext->setText(i18n("Next Document"));
    aNext->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    actionCollection()->setDefaultShortcut(aNext, Qt::ALT | Qt::Key_Down);
    connect(aNext, &QAction::triggered, m_fileTree, &KateFileTree::slotDocumentNext);

    auto aShowActive = actionCollection()->addAction(QStringLiteral("filetree_show_active_document"));
    aShowActive->setText(i18n("&Show Active"));
    aShowActive->setIcon(QIcon::fromTheme(QStringLiteral("folder-sync")));
    connect(aShowActive, &QAction::triggered, this, &KateFileTreePluginView::showActiveDocument);

    auto aSave = actionCollection()->addAction(QStringLiteral("filetree_save"), this, SLOT(slotDocumentSave()));
    aSave->setText(i18n("Save Current Document"));
    aSave->setToolTip(i18n("Save the current document"));
    aSave->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));

    auto aSaveAs = actionCollection()->addAction(QStringLiteral("filetree_save_as"), this, SLOT(slotDocumentSaveAs()));
    aSaveAs->setText(i18n("Save Current Document As"));
    aSaveAs->setToolTip(i18n("Save current document under new name"));
    aSaveAs->setIcon(QIcon::fromTheme(QStringLiteral("document-save-as")));

    /**
     * add new & open, if hosting application has it
     */
    if (KXmlGuiWindow *parentClient = qobject_cast<KXmlGuiWindow *>(m_mainWindow->window())) {
        bool newOrOpen = false;
        if (auto a = parentClient->action("file_new")) {
            m_toolbar->addAction(a);
            newOrOpen = true;
        }
        if (auto a = parentClient->action("file_open")) {
            m_toolbar->addAction(a);
            newOrOpen = true;
        }
        if (newOrOpen) {
            m_toolbar->addSeparator();
        }
    }

    /**
     * add own actions
     */
    m_toolbar->addAction(aPrev);
    m_toolbar->addAction(aNext);
    m_toolbar->addSeparator();
    m_toolbar->addAction(aSave);
    m_toolbar->addAction(aSaveAs);
}

KateFileTreeModel *KateFileTreePluginView::model() const
{
    return m_documentModel;
}

KateFileTreeProxyModel *KateFileTreePluginView::proxy() const
{
    return m_proxyModel;
}

KateFileTree *KateFileTreePluginView::tree() const
{
    return m_fileTree;
}

void KateFileTreePluginView::documentOpened(KTextEditor::Document *doc)
{
    // enqueue and start update timer to collapse updates
    m_documentsCreatedTimer.start();
    m_documentsCreated.append(doc);
}

void KateFileTreePluginView::documentClosed(KTextEditor::Document *doc)
{
    m_documentsCreated.removeAll(doc);
    m_proxyModel->invalidate();
}

void KateFileTreePluginView::setToolbarVisible(bool visible)
{
    m_toolbar->setVisible(visible);
}

void KateFileTreePluginView::viewChanged(KTextEditor::View *)
{
    KTextEditor::View *view = m_mainWindow->activeView();
    if (!view) {
        return;
    }

    KTextEditor::Document *doc = view->document();
    QModelIndex index = m_proxyModel->docIndex(doc);

    // update the model on which doc is active
    m_documentModel->documentActivated(doc);

    m_fileTree->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    m_fileTree->scrollTo(index);

    while (index != QModelIndex()) {
        m_fileTree->expand(index);
        index = index.parent();
    }
}

void KateFileTreePluginView::setListMode(bool listMode)
{
    if (listMode) {
        m_documentModel->setListMode(true);
        m_fileTree->setRootIsDecorated(false);
    } else {
        m_documentModel->setListMode(false);
        m_fileTree->setRootIsDecorated(true);
    }

    m_proxyModel->sort(0, Qt::AscendingOrder);
    m_proxyModel->invalidate();
}

void KateFileTreePluginView::viewModeChanged(bool listMode)
{
    setHasLocalPrefs(true);
    setListMode(listMode);
}

void KateFileTreePluginView::sortRoleChanged(int role)
{
    setHasLocalPrefs(true);
    m_proxyModel->setSortRole(role);
    m_proxyModel->invalidate();
}

void KateFileTreePluginView::activateDocument(KTextEditor::Document *doc)
{
    m_mainWindow->activateView(doc);
}

void KateFileTreePluginView::showToolView()
{
    m_mainWindow->showToolView(m_toolView);
    m_toolView->setFocus();
}

void KateFileTreePluginView::hideToolView()
{
    m_mainWindow->hideToolView(m_toolView);
}

void KateFileTreePluginView::showActiveDocument()
{
    // hack?
    viewChanged();
    // make the tool view show if it was hidden
    showToolView();
}

bool KateFileTreePluginView::hasLocalPrefs() const
{
    return m_hasLocalPrefs;
}

void KateFileTreePluginView::setHasLocalPrefs(bool h)
{
    m_hasLocalPrefs = h;
}

void KateFileTreePluginView::readSessionConfig(const KConfigGroup &g)
{
    if (g.exists()) {
        m_hasLocalPrefs = true;
    } else {
        m_hasLocalPrefs = false;
    }

    // we chain to the global settings by using them as the defaults
    //  here in the session view config loading.
    const KateFileTreePluginSettings &defaults = m_plug->settings();

    bool listMode = g.readEntry("listMode", defaults.listMode());

    setListMode(listMode);

    int sortRole = g.readEntry("sortRole", defaults.sortRole());
    m_proxyModel->setSortRole(sortRole);
}

void KateFileTreePluginView::writeSessionConfig(KConfigGroup &g)
{
    if (m_hasLocalPrefs) {
        g.writeEntry("listMode", QVariant(m_documentModel->listMode()));
        g.writeEntry("sortRole", int(m_proxyModel->sortRole()));
    } else {
        g.deleteEntry("listMode");
        g.deleteEntry("sortRole");
    }

    g.sync();
}

void KateFileTreePluginView::slotDocumentsCreated()
{
    // handle potential multiple new documents
    m_documentModel->documentsOpened(m_documentsCreated);
    m_documentsCreated.clear();
    viewChanged();
}

void KateFileTreePluginView::slotDocumentSave() const
{
    if (auto view = m_mainWindow->activeView()) {
        view->document()->documentSave();
    }
}

void KateFileTreePluginView::slotDocumentSaveAs() const
{
    if (auto view = m_mainWindow->activeView()) {
        view->document()->documentSaveAs();
    }
}

// END KateFileTreePluginView

#include "katefiletreeplugin.moc"
