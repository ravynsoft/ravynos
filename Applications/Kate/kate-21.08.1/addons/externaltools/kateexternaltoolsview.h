/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KTEXTEDITOR_EXTERNALTOOLS_H
#define KTEXTEDITOR_EXTERNALTOOLS_H

namespace KTextEditor
{
class MainWindow;
}
namespace KTextEditor
{
class Document;
class View;
}

#include <KActionMenu>
#include <KMacroExpander>
#include <KXMLGUIClient>

class QTextDocument;

class KActionCollection;
class KateExternalToolsPlugin;
class KateExternalTool;

namespace Ui
{
class ToolView;
}

/**
 * Menu action that displays all KateExternalTool in a submenu.
 * Enables/disables the tool actions whenever the view changes, depending on the mimetype.
 */
class KateExternalToolsMenuAction : public KActionMenu
{
    Q_OBJECT
public:
    KateExternalToolsMenuAction(const QString &text,
                                KActionCollection *collection,
                                KateExternalToolsPlugin *plugin,
                                class KTextEditor::MainWindow *mw = nullptr);
    virtual ~KateExternalToolsMenuAction();

    /**
     * This will load all the configured services.
     */
    void reload();

    KActionCollection *actionCollection() const
    {
        return m_actionCollection;
    }

private Q_SLOTS:
    /**
     * Called whenever the current view changed.
     * Calls updateActionState() for the corresponding document.
     */
    void slotViewChanged(KTextEditor::View *view);

    /**
     * Required to enable/disable the tools that depend on specific mimetypes.
     */
    void updateActionState(KTextEditor::Document *activeDoc);

    /**
     * Triggered via Tools > External Tools > Configure...
     */
    void showConfigPage();

private:
    KateExternalToolsPlugin *m_plugin;
    KTextEditor::MainWindow *m_mainwindow; // for the actions to access view/doc managers
    KActionCollection *m_actionCollection;
    QMetaObject::Connection m_docUrlChangedConnection;
};

class KateExternalToolsPluginView : public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    KateExternalToolsPluginView(KTextEditor::MainWindow *mainWindow, KateExternalToolsPlugin *plugin);

    /**
     * Virtual destructor.
     */
    ~KateExternalToolsPluginView();

    /**
     * Returns the associated mainWindow
     */
    KTextEditor::MainWindow *mainWindow() const;

public Q_SLOTS:
    /**
     * Called by the plugin view to reload the menu
     */
    void rebuildMenu();

    /**
     * Creates the tool view. If already existing, does nothing.
     */
    void createToolView();

    /**
     * Shows the tool view. The toolview will be created, if not yet existing.
     */
    void showToolView();

    /**
     * Clears the toolview data. If no toolview is around, nothing happens.
     */
    void clearToolView();

    /**
     * Sets the output data to data;
     */
    void setOutputData(const QString &data);

    /**
     * Deletes the tool view, if existing.
     */
    void deleteToolView();

    /**
     * On Escape, hide tool view.
     */
    void handleEsc(QEvent *event);

Q_SIGNALS:
    /**
     * Signal for outgoing message, the host application will handle them!
     * Will only be handled inside the main windows of this plugin view.
     * @param message outgoing message we send to the host application
     */
    void message(const QVariantMap &message);

private:
    KateExternalToolsPlugin *m_plugin;
    KTextEditor::MainWindow *m_mainWindow;
    KateExternalToolsMenuAction *m_externalToolsMenu = nullptr;
    QWidget *m_toolView = nullptr;
    Ui::ToolView *m_ui = nullptr;
    QTextDocument *m_outputDoc = nullptr;
};

#endif // KTEXTEDITOR_EXTERNALTOOLS_H

// kate: space-indent on; indent-width 4; replace-tabs on;
