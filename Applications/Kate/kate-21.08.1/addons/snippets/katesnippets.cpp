/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "katesnippets.h"
#include "snippetcompletionmodel.h"
#include "snippetview.h"

#include <QAction>
#include <QBoxLayout>

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KXMLGUIFactory>

#include <KTextEditor/CodeCompletionInterface>

K_PLUGIN_FACTORY_WITH_JSON(KateSnippetsPluginFactory, "katesnippetsplugin.json", registerPlugin<KateSnippetsPlugin>();)

KateSnippetsPlugin::KateSnippetsPlugin(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
    , m_snippetGlobal(new KateSnippetGlobal(this))
{
}

KateSnippetsPlugin::~KateSnippetsPlugin()
{
}

QObject *KateSnippetsPlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    KateSnippetsPluginView *view = new KateSnippetsPluginView(this, mainWindow);
    return view;
}

KateSnippetsPluginView::KateSnippetsPluginView(KateSnippetsPlugin *plugin, KTextEditor::MainWindow *mainWindow)
    : QObject(mainWindow)
    , m_plugin(plugin)
    , m_mainWindow(mainWindow)
    , m_toolView(nullptr)
    , m_snippets(nullptr)
{
    KXMLGUIClient::setComponentName(QStringLiteral("katesnippets"), i18n("Snippets tool view"));
    setXMLFile(QStringLiteral("ui.rc"));

    // Toolview for snippets
    m_toolView = mainWindow->createToolView(plugin,
                                            QStringLiteral("kate_private_plugin_katesnippetsplugin"),
                                            KTextEditor::MainWindow::Right,
                                            QIcon::fromTheme(QStringLiteral("document-new")),
                                            i18n("Snippets"));

    // add snippets widget
    m_snippets = new SnippetView(KateSnippetGlobal::self(), mainWindow, m_toolView.data());
    m_toolView->layout()->addWidget(m_snippets);
    m_snippets->setupActionsForWindow(mainWindow->window());
    m_toolView->addActions(m_snippets->actions());

    // create actions
    QAction *a = actionCollection()->addAction(QStringLiteral("tools_create_snippet"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
    a->setText(i18n("Create Snippet"));
    connect(a, &QAction::triggered, this, &KateSnippetsPluginView::createSnippet);

    connect(mainWindow, &KTextEditor::MainWindow::viewCreated, this, &KateSnippetsPluginView::slotViewCreated);

    /**
     * connect for all already existing views
     */
    const auto views = mainWindow->views();
    for (KTextEditor::View *view : views) {
        slotViewCreated(view);
    }

    // register if factory around
    if (auto factory = m_mainWindow->guiFactory()) {
        factory->addClient(this);
    }
}

KateSnippetsPluginView::~KateSnippetsPluginView()
{
    // cleanup for all views
    for (auto view : qAsConst(m_textViews)) {
        if (!view) {
            continue;
        }
        auto iface = qobject_cast<KTextEditor::CodeCompletionInterface *>(view);
        iface->unregisterCompletionModel(KateSnippetGlobal::self()->completionModel());
    }

    // unregister if factory around
    if (auto factory = m_mainWindow->guiFactory()) {
        factory->removeClient(this);
    }

    if (m_toolView) {
        delete m_toolView;
    }
}

void KateSnippetsPluginView::slotViewCreated(KTextEditor::View *view)
{
    m_textViews.append(QPointer<KTextEditor::View>(view));

    // add snippet completion
    auto model = KateSnippetGlobal::self()->completionModel();
    auto iface = qobject_cast<KTextEditor::CodeCompletionInterface *>(view);
    iface->unregisterCompletionModel(model);
    iface->registerCompletionModel(model);
}

void KateSnippetsPluginView::createSnippet()
{
    KateSnippetGlobal::self()->createSnippet(m_mainWindow->activeView());
}

#include "katesnippets.moc"
