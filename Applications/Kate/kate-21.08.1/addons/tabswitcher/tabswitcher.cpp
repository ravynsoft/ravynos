/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2014-2019 Dominik Haumann <dhaumann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tabswitcher.h"
#include "tabswitcherfilesmodel.h"
#include "tabswitchertreeview.h"

#include <KTextEditor/Application>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KXMLGUIFactory>

#include <QAction>
#include <QScrollBar>
#include <QStandardItemModel>

K_PLUGIN_FACTORY_WITH_JSON(TabSwitcherPluginFactory, "tabswitcherplugin.json", registerPlugin<TabSwitcherPlugin>();)

TabSwitcherPlugin::TabSwitcherPlugin(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
{
}

QObject *TabSwitcherPlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    return new TabSwitcherPluginView(this, mainWindow);
}

TabSwitcherPluginView::TabSwitcherPluginView(TabSwitcherPlugin *plugin, KTextEditor::MainWindow *mainWindow)
    : QObject(mainWindow)
    , m_plugin(plugin)
    , m_mainWindow(mainWindow)
{
    // register this view
    m_plugin->m_views.append(this);

    m_model = new detail::TabswitcherFilesModel(this);
    m_treeView = new TabSwitcherTreeView();
    m_treeView->setModel(m_model);

    KXMLGUIClient::setComponentName(QStringLiteral("tabswitcher"), i18n("Document Switcher"));
    setXMLFile(QStringLiteral("ui.rc"));

    // note: call after m_treeView is created
    setupActions();

    // fill the model
    setupModel();

    // register action in menu
    m_mainWindow->guiFactory()->addClient(this);

    // popup connections
    connect(m_treeView, &TabSwitcherTreeView::pressed, this, &TabSwitcherPluginView::switchToClicked);
    connect(m_treeView, &TabSwitcherTreeView::itemActivated, this, &TabSwitcherPluginView::activateView);

    // track existing documents
    connect(KTextEditor::Editor::instance()->application(), &KTextEditor::Application::documentCreated, this, &TabSwitcherPluginView::registerDocument);
    connect(KTextEditor::Editor::instance()->application(), &KTextEditor::Application::documentWillBeDeleted, this, &TabSwitcherPluginView::unregisterDocument);

    // track lru activation of views to raise the respective documents in the model
    connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &TabSwitcherPluginView::raiseView);
}

TabSwitcherPluginView::~TabSwitcherPluginView()
{
    // delete popup widget
    delete m_treeView;

    // unregister action in menu
    m_mainWindow->guiFactory()->removeClient(this);

    // unregister this view
    m_plugin->m_views.removeAll(this);
}

void TabSwitcherPluginView::setupActions()
{
    auto aNext = actionCollection()->addAction(QStringLiteral("view_lru_document_next"));
    aNext->setText(i18n("Last Used Views"));
    aNext->setIcon(QIcon::fromTheme(QStringLiteral("go-next-view-page")));
    actionCollection()->setDefaultShortcut(aNext, Qt::CTRL | Qt::Key_Tab);
    aNext->setWhatsThis(i18n("Opens a list to walk through the list of last used views."));
    aNext->setStatusTip(i18n("Walk through the list of last used views"));
    connect(aNext, &QAction::triggered, this, &TabSwitcherPluginView::walkForward);

    auto aPrev = actionCollection()->addAction(QStringLiteral("view_lru_document_prev"));
    aPrev->setText(i18n("Last Used Views (Reverse)"));
    aPrev->setIcon(QIcon::fromTheme(QStringLiteral("go-previous-view-page")));
    actionCollection()->setDefaultShortcut(aPrev, Qt::CTRL | Qt::SHIFT | Qt::Key_Tab);
    aPrev->setWhatsThis(i18n("Opens a list to walk through the list of last used views in reverse."));
    aPrev->setStatusTip(i18n("Walk through the list of last used views"));
    connect(aPrev, &QAction::triggered, this, &TabSwitcherPluginView::walkBackward);

    auto aClose = actionCollection()->addAction(QStringLiteral("view_lru_document_close"));
    aClose->setText(i18n("Close View"));
    aClose->setShortcutContext(Qt::WidgetShortcut);
    actionCollection()->setDefaultShortcut(aClose, Qt::CTRL | Qt::Key_W);
    aClose->setWhatsThis(i18n("Closes the selected view in the list of last used views."));
    aClose->setStatusTip(i18n("Closes the selected view in the list of last used views."));
    connect(aClose, &QAction::triggered, this, &TabSwitcherPluginView::closeView);

    // make sure action work when the popup has focus
    m_treeView->addAction(aNext);
    m_treeView->addAction(aPrev);
    m_treeView->addAction(aClose);
}

void TabSwitcherPluginView::setupModel()
{
    const auto documents = KTextEditor::Editor::instance()->application()->documents();
    // initial fill of model
    for (auto doc : documents) {
        registerDocument(doc);
    }
}

void TabSwitcherPluginView::registerDocument(KTextEditor::Document *document)
{
    // insert into hash
    m_documents.insert(document);

    // add to model
    m_model->insertDocument(0, document);

    // track document name changes
    connect(document, &KTextEditor::Document::documentNameChanged, this, &TabSwitcherPluginView::updateDocumentName);
}

void TabSwitcherPluginView::unregisterDocument(KTextEditor::Document *document)
{
    // remove from hash
    if (!m_documents.contains(document)) {
        return;
    }
    m_documents.remove(document);

    // remove from model
    m_model->removeDocument(document);

    // disconnect documentNameChanged() signal
    disconnect(document, nullptr, this, nullptr);
}

void TabSwitcherPluginView::updateDocumentName(KTextEditor::Document *document)
{
    if (!m_documents.contains(document)) {
        return;
    }

    // update all items, since a document URL change menas we have to recalculate
    // common prefix path of all items.
    m_model->updateItems();
}

void TabSwitcherPluginView::raiseView(KTextEditor::View *view)
{
    if (!view || !m_documents.contains(view->document())) {
        return;
    }

    m_model->raiseDocument(view->document());
}

void TabSwitcherPluginView::walk(const int from, const int to)
{
    QModelIndex index;
    const int step = from < to ? 1 : -1;
    if (!m_treeView->isVisible()) {
        updateViewGeometry();
        index = m_model->index(from + step, 0);
        if (!index.isValid()) {
            index = m_model->index(0, 0);
        }
        m_treeView->show();
        m_treeView->setFocus();
    } else {
        int newRow = m_treeView->selectionModel()->currentIndex().row() + step;
        if (newRow == to + step) {
            newRow = from;
        }
        index = m_model->index(newRow, 0);
    }

    m_treeView->selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    m_treeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

void TabSwitcherPluginView::walkForward()
{
    walk(0, m_model->rowCount() - 1);
}

void TabSwitcherPluginView::walkBackward()
{
    walk(m_model->rowCount() - 1, 0);
}

void TabSwitcherPluginView::updateViewGeometry()
{
    QWidget *window = m_mainWindow->window();
    const QSize centralSize = window->size();

    // Maximum size of the view is 3/4th of the central widget (the editor area)
    // so the view does not overlap the mainwindow since that looks awkward.
    const QSize viewMaxSize(centralSize.width() * 3 / 4, centralSize.height() * 3 / 4);

    // The actual view size should be as big as the columns/rows need it, but
    // smaller than the max-size. This means the view will get quite high with
    // many open files but I think thats ok. Otherwise one can easily tweak the
    // max size to be only 1/2th of the central widget size
    const int rowHeight = m_treeView->sizeHintForRow(0);
    const int frameWidth = m_treeView->frameWidth();
    // const QSize viewSize(std::min(m_treeView->sizeHintForColumn(0) + 2 * frameWidth + m_treeView->verticalScrollBar()->width(), viewMaxSize.width()), // ORIG
    // line, sizeHintForColumn was QListView but is protected for QTreeView so we introduced sizeHintWidth()
    const QSize viewSize(std::min(m_treeView->sizeHintWidth() + 2 * frameWidth + m_treeView->verticalScrollBar()->width(), viewMaxSize.width()),
                         std::min(std::max(rowHeight * m_model->rowCount() + 2 * frameWidth, rowHeight * 6), viewMaxSize.height()));

    // Position should be central over the editor area, so map to global from
    // parent of central widget since the view is positioned in global coords
    const QPoint centralWidgetPos = window->parentWidget() ? window->mapToGlobal(window->pos()) : window->pos();
    const int xPos = std::max(0, centralWidgetPos.x() + (centralSize.width() - viewSize.width()) / 2);
    const int yPos = std::max(0, centralWidgetPos.y() + (centralSize.height() - viewSize.height()) / 2);

    m_treeView->setFixedSize(viewSize);
    m_treeView->move(xPos, yPos);
}

void TabSwitcherPluginView::switchToClicked(const QModelIndex &index)
{
    m_treeView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    activateView(index);
}

void TabSwitcherPluginView::activateView(const QModelIndex &index)
{
    Q_UNUSED(index)

    // guard against empty selection
    if (m_treeView->selectionModel()->selectedRows().isEmpty()) {
        return;
    }

    const int row = m_treeView->selectionModel()->selectedRows().first().row();

    auto doc = m_model->item(row);
    m_mainWindow->activateView(doc);

    m_treeView->hide();
}

void TabSwitcherPluginView::closeView()
{
    if (m_treeView->selectionModel()->selectedRows().isEmpty()) {
        return;
    }

    const int row = m_treeView->selectionModel()->selectedRows().first().row();
    KTextEditor::Document *doc = m_model->item(row);
    if (doc) {
        KTextEditor::Editor::instance()->application()->closeDocument(doc);
    }
}

// required for TabSwitcherPluginFactory vtable
#include "tabswitcher.moc"
