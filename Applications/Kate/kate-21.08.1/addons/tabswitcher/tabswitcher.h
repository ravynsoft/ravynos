/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2014 Dominik Haumann <dhaumann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_TAB_SWITCHER_PLUGIN_H
#define KTEXTEDITOR_TAB_SWITCHER_PLUGIN_H

#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>

#include <QList>
#include <QSet>
#include <QVariant>

#include <KXMLGUIClient>

class TabSwitcherPluginView;
class TabSwitcherTreeView;
class QStandardItemModel;
class QModelIndex;
namespace detail
{
class TabswitcherFilesModel;
}

class TabSwitcherPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

    friend TabSwitcherPluginView;

public:
    /**
     * Plugin constructor.
     */
    explicit TabSwitcherPlugin(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());

    /**
     * Create a new tab switcher for @p mainWindow.
     */
    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

private:
    QList<TabSwitcherPluginView *> m_views;
};

class TabSwitcherPluginView : public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    /**
     * View constructor.
     */
    TabSwitcherPluginView(TabSwitcherPlugin *plugin, KTextEditor::MainWindow *mainWindow);

    /**
     * View destructor.
     */
    ~TabSwitcherPluginView() override;

    /**
     * Setup the shortcut actions.
     */
    void setupActions();

    /**
     * Initial fill of model with documents from the application.
     */
    void setupModel();

public Q_SLOTS:
    /**
     * Adds @p document to the model.
     */
    void registerDocument(KTextEditor::Document *document);

    /**
     * Removes @p document from the model.
     */
    void unregisterDocument(KTextEditor::Document *document);

    /**
     * Update the name in the model for @p document.
     */
    void updateDocumentName(KTextEditor::Document *document);

    /**
     * Raise @p view in a lru fashion.
     */
    void raiseView(KTextEditor::View *view);

    /**
     * Focus next item in the treeview.
     */
    void walkForward();

    /**
     * Focus previous item in the treeview.
     */
    void walkBackward();

    /**
     * Activate the document for @p index.
     */
    void switchToClicked(const QModelIndex &index);

    /**
     * Show the document for @p index.
     */
    void activateView(const QModelIndex &index);

    /**
     * Closes the current view
     */
    void closeView();

protected:
    /**
     * Move through the list.
     */
    void walk(const int from, const int to);

    /**
     * Make sure the popup view has a sane size.
     */
    void updateViewGeometry();

private:
    TabSwitcherPlugin *m_plugin;
    KTextEditor::MainWindow *m_mainWindow;
    detail::TabswitcherFilesModel *m_model;
    QSet<KTextEditor::Document *> m_documents;
    TabSwitcherTreeView *m_treeView;
};

#endif // KTEXTEDITOR_TAB_SWITCHER_PLUGIN_H
