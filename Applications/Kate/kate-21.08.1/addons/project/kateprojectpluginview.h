/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KATE_PROJECT_PLUGIN_VIEW_H_
#define _KATE_PROJECT_PLUGIN_VIEW_H_

#include <QComboBox>
#include <QMenu>
#include <QPointer>
#include <QStackedWidget>
#include <QToolButton>

#include <KTextEditor/View>
#include <KXMLGUIClient>

#include <memory>

class QAction;
class QDir;
class KateProject;
class KateProjectView;
class KateProjectPlugin;
class KateProjectInfoView;

typedef QMap<QString, QString> QStringMap;
Q_DECLARE_METATYPE(QStringMap)

class KateProjectPluginView : public QObject, public KXMLGUIClient
{
    Q_OBJECT

    Q_PROPERTY(QString projectFileName READ projectFileName NOTIFY projectFileNameChanged)
    Q_PROPERTY(QString projectName READ projectName)
    Q_PROPERTY(QString projectBaseDir READ projectBaseDir)
    Q_PROPERTY(QVariantMap projectMap READ projectMap NOTIFY projectMapChanged)
    Q_PROPERTY(QStringList projectFiles READ projectFiles)

    Q_PROPERTY(QString allProjectsCommonBaseDir READ allProjectsCommonBaseDir)
    Q_PROPERTY(QStringList allProjectsFiles READ allProjectsFiles)
    Q_PROPERTY(QStringMap allProjects READ allProjects)

public:
    KateProjectPluginView(KateProjectPlugin *plugin, KTextEditor::MainWindow *mainWindow);
    ~KateProjectPluginView() override;

    /**
     * content of current active project, as variant map
     * @return empty map if no project active, else content of project JSON
     */
    QVariantMap projectMap() const;

    /**
     * which project file is currently active?
     * @return empty string if none, else project file name
     */
    QString projectFileName() const;

    /**
     * Returns the name of the project
     */
    QString projectName() const;

    /**
     * Returns the base directory of the project
     */
    QString projectBaseDir() const;

    /**
     * files for the current active project?
     * @return empty list if none, else project files as stringlist
     */
    QStringList projectFiles() const;

    /**
     * Example: Two projects are loaded with baseDir1="/home/dev/project1" and
     * baseDir2="/home/dev/project2". Then "/home/dev/" is returned.
     * @see projectBaseDir().
     * Used for the Search&Replace plugin for option "Search in all open projects".
     */
    QString allProjectsCommonBaseDir() const;

    /**
     * @returns a flat list of files for all open projects (@see also projectFiles())
     */
    QStringList allProjectsFiles() const;

    /**
     * @returns a map of all open projects which maps base directory to name
     */
    QMap<QString, QString> allProjects() const;

    /**
     * the main window we belong to
     * @return our main window
     */
    KTextEditor::MainWindow *mainWindow() const
    {
        return m_mainWindow;
    }

    /**
     * the plugin we belong to
     * @return our plugin
     */
    KateProjectPlugin *plugin() const
    {
        return m_plugin;
    }

    /**
     * @brief Shows diff in a fixed view, i.e., the view is recycled instead
     * of creating new view every time
     * @param contents diff contents
     */
    void showDiffInFixedView(const QByteArray &contents);

    /**
     * Same as above with call back for setting a context menu
     *
     * @param cb Callback on the view. This should always take KTextEditor::View*
     * as a parameter. This is mainly used to plug-in context-menu actions.
     */
    template<typename ViewCallback>
    void showDiffInFixedView(const QByteArray &contents, ViewCallback cb)
    {
        showDiffInFixedView(contents);
        cb(m_fixedView.view);
    }

    /**
     * Open terminal view at \p dirPath location for project \p project
     */
    void openTerminal(const QString &dirPath, KateProject *project);

public Q_SLOTS:
    /**
     * Create views for given project.
     * Either gives existing ones or creates new one
     * @param project project we want view for
     * @return views (normal + info view)
     */
    QPair<KateProjectView *, KateProjectInfoView *> viewForProject(KateProject *project);

    /**
     * Switch to project for given dir, if any there!
     * @param dir dir with the project
     */
    void switchToProject(const QDir &dir);

private Q_SLOTS:
    /**
     * Plugin config updated
     */
    void slotConfigUpdated();

    /**
     * New view got created, we need to update our connections
     * @param view new created view
     */
    void slotViewCreated(KTextEditor::View *view);

    /**
     * View got destroyed.
     * @param view deleted view
     */
    void slotViewDestroyed(QObject *view);

    /**
     * Activate the previous project.
     */
    void slotProjectPrev();

    /**
     * Activate the next project.
     */
    void slotProjectNext();

    /**
     * Reload current project, if any.
     * This will trigger a reload with force.
     */
    void slotProjectReload();

    /**
     * Getting project for others windows
     * and closing project documents.
     */
    void slotProjectAboutToClose();

    /**
     * Close current project.
     */
    void slotProjectClose(KateProject *project);

    /**
     * Lookup current word
     */
    void slotProjectIndex();

    /**
     * Goto current word
     */
    void slotGotoSymbol();

Q_SIGNALS:

    /**
     * Emitted if project is about to close.
     */
    void pluginProjectRemoved(QString baseDir, QString name);

    /**
     * Emitted if project is added.
     */
    void pluginProjectAdded(QString baseDir, QString name);

    /**
     * Emitted if project is about to close.
     */
    void pluginProjectClose(KateProject *project);

    /**
     * Emitted if projectFileName changed.
     */
    void projectFileNameChanged();

    /**
     * Emitted if projectMap changed.
     */
    void projectMapChanged();

    /**
     * Emitted when a ctags lookup in requested
     * @param word lookup word
     */
    void projectLookupWord(const QString &word);

    /**
     * Emitted when a ctags goto sysmbol is requested
     * @param word lookup word
     */
    void gotoSymbol(const QString &word, int &results);

    /**
     * Signal for outgoing message, the host application will handle them!
     * Will only be handled inside the main windows of this plugin view.
     * @param message outgoing message we send to the host application
     */
    void message(const QVariantMap &message);

    /**
     * Signal for location changed. Location gets saved in history
     * of the current KateViewSpace
     * @param document url
     * @param c pos in document
     */
    void addPositionToHistory(const QUrl &url, KTextEditor::Cursor c);

private Q_SLOTS:
    /**
     * This slot is called whenever the active view changes in our main window.
     */
    void slotViewChanged();

    /**
     * Current project changed.
     * @param index index in toolbox
     */
    void slotCurrentChanged(int index);

    /**
     * Url changed, to auto-load projects
     */
    void slotDocumentUrlChanged(KTextEditor::Document *document);

    /**
     * Show context menu
     */
    void slotContextMenuAboutToShow();

    /**
     * Handle esc key and hide the toolview
     */
    void handleEsc(QEvent *e);

    /**
     * Update git status on toolview shown
     */
    void slotUpdateStatus(bool visible);

    /**
     * Open a folder / project
     */
    void openDirectoryOrProject();

    /**
     * Show projects To-Dos and Fix-mes
     */
    void showProjectTodos();

private:
    /**
     * find current selected or under cursor word
     */
    QString currentWord() const;

private:
    /**
     * our plugin
     */
    KateProjectPlugin *m_plugin;

    /**
     * the main window we belong to
     */
    KTextEditor::MainWindow *m_mainWindow;

    /**
     * our projects toolview
     */
    QWidget *m_toolView;

    /**
     * our projects info toolview
     */
    QWidget *m_toolInfoView;

    /**
     * our projects info toolview
     */
    std::unique_ptr<QWidget> m_gitToolView;

    /**
     * our cross-projects toolview
     */
    QWidget *m_toolMultiView;

    /**
     * combo box with all loaded projects inside
     */
    QComboBox *m_projectsCombo;

    /**
     * combo box with all loaded projects inside
     */
    QComboBox *m_projectsComboGit;

    /**
     * Reload button
     */
    QToolButton *m_reloadButton;

    /**
     * Closeing button
     */
    QToolButton *m_closeProjectButton;

    /**
     * Git status refresh button
     */
    QToolButton *m_gitStatusRefreshButton;

    /**
     * stacked widget will all currently created project views
     */
    QStackedWidget *m_stackedProjectViews;

    /**
     * stacked widget will all currently created project info views
     */
    QStackedWidget *m_stackedProjectInfoViews;

    /**
     * stacked widget will all currently created git views
     */
    QStackedWidget *m_stackedgitViews;

    /**
     * project => view
     */
    QMap<KateProject *, QPair<KateProjectView *, KateProjectInfoView *>> m_project2View;

    /**
     * remember current active view text editor view
     * might be 0
     */
    QPointer<KTextEditor::View> m_activeTextEditorView;

    /**
     * remember for which text views we might need to cleanup stuff
     */
    QSet<QObject *> m_textViews;

    /**
     * lookup action
     */
    QAction *m_lookupAction;

    /**
     * goto symbol action
     */
    QAction *m_gotoSymbolAction;
    QAction *m_gotoSymbolActionAppMenu;

    class FixedView
    {
    public:
        QPointer<KTextEditor::View> view;
        QPointer<QMenu> defaultMenu;

        void restoreMenu()
        {
            if (view && defaultMenu) {
                view->setContextMenu(defaultMenu);
            }
        }
    };

    /**
     * Fixed view for viewing diffs
     */
    FixedView m_fixedView;
};

#endif
