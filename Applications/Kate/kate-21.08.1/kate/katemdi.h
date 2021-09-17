/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2002, 2003 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __KATE_MDI_H__
#define __KATE_MDI_H__

#include <KTextEditor/Plugin>

#include <KParts/MainWindow>

#include <KMultiTabBar>
#include <KToggleAction>
#include <KXMLGUIClient>

#include <QChildEvent>
#include <QEvent>
#include <QFrame>
#include <QPointer>
#include <QSplitter>

#include <map>
#include <unordered_map>
#include <vector>

class KActionMenu;
class QAction;
class QPixmap;
class KConfigBase;

namespace KTextEditor
{
class ConfigPageInterface;
}

namespace KateMDI
{
class ToolView;

class ToggleToolViewAction : public KToggleAction
{
    Q_OBJECT

public:
    ToggleToolViewAction(const QString &text, ToolView *tv, QObject *parent);

protected Q_SLOTS:
    void slotToggled(bool) override;
    void toolVisibleChanged(bool);

private:
    ToolView *m_tv;
};

class GUIClient : public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    GUIClient(class MainWindow *mw);

    void registerToolView(ToolView *tv);
    void unregisterToolView(ToolView *tv);
    void updateSidebarsVisibleAction();

private Q_SLOTS:
    void clientAdded(KXMLGUIClient *client);
    void updateActions();

private:
    MainWindow *m_mw;
    KToggleAction *m_showSidebarsAction;
    std::vector<QAction *> m_toolViewActions;
    std::unordered_map<ToolView *, QAction *> m_toolToAction;
    KActionMenu *m_toolMenu;
};

class ToolView : public QFrame
{
    Q_OBJECT

    friend class Sidebar;
    friend class MainWindow;
    friend class GUIClient;
    friend class ToggleToolViewAction;

protected:
    /**
     * ToolView
     * Objects of this clas represent a toolview in the mainwindow
     * you should only add one widget as child to this toolview, it will
     * be automatically set to be the focus proxy of the toolview
     * @param mainwin main window for this toolview
     * @param sidebar sidebar of this toolview
     * @param parent parent widget, e.g. the splitter of one of the sidebars
     */
    ToolView(class MainWindow *mainwin, class Sidebar *sidebar, QWidget *parent);

public:
    /**
     * destruct me, this is allowed for all, will care itself that the toolview is removed
     * from the mainwindow and sidebar
     */
    ~ToolView() override;

Q_SIGNALS:
    /**
     * toolview hidden or shown
     * @param visible is this toolview made visible?
     */
    void toolVisibleChanged(bool visible);

    /**
     * some internal methodes needed by the main window and the sidebars
     */
protected:
    MainWindow *mainWindow()
    {
        return m_mainWin;
    }

    Sidebar *sidebar()
    {
        return m_sidebar;
    }

    void setToolVisible(bool vis);

public:
    bool toolVisible() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void childEvent(QChildEvent *ev) override;
    void actionEvent(QActionEvent *event) override;

private:
    MainWindow *m_mainWin;
    Sidebar *m_sidebar;
    KToolBar *m_toolbar;

    /// plugin this view belongs to, may be 0
    QPointer<KTextEditor::Plugin> plugin;

    /**
     * unique id
     */
    QString id;

    /**
     * is visible in sidebar
     */
    bool m_toolVisible;

    /**
     * is this view persistent?
     */
    bool persistent;

    QIcon icon;
    QString text;
};

class Sidebar : public KMultiTabBar
{
    Q_OBJECT

public:
    Sidebar(KMultiTabBar::KMultiTabBarPosition pos, class MainWindow *mainwin, QWidget *parent);

    void setSplitter(QSplitter *sp);

public:
    ToolView *addWidget(const QIcon &icon, const QString &text, ToolView *widget);
    bool removeWidget(ToolView *widget);

    bool showWidget(ToolView *widget);
    bool hideWidget(ToolView *widget);

    void setLastSize(int s)
    {
        m_lastSize = s;
    }
    int lastSize() const
    {
        return m_lastSize;
    }
    void updateLastSize();

    bool splitterVisible() const
    {
        return m_ownSplit->isVisible();
    }

    void restoreSession();

    /**
     * restore the current session config from given object, use current group
     * @param config config object to use
     */
    void restoreSession(KConfigGroup &config);

    /**
     * save the current session config to given object, use current group
     * @param config config object to use
     */
    void saveSession(KConfigGroup &config);

public Q_SLOTS:
    // reimplemented, to block a show() call if all sidebars are forced hidden
    void setVisible(bool visible) override;
private Q_SLOTS:
    void tabClicked(int);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private Q_SLOTS:
    void buttonPopupActivate(QAction *);

private:
    MainWindow *m_mainWin;

    KMultiTabBar::KMultiTabBarPosition m_pos{};
    QSplitter *m_splitter;
    KMultiTabBar *m_tabBar = nullptr;
    QSplitter *m_ownSplit;

    std::map<int, ToolView *> m_idToWidget;
    std::map<ToolView *, int> m_widgetToId;
    std::map<ToolView *, QSize> m_widgetToSize;

    /**
     * list of all toolviews around in this sidebar
     */
    std::vector<ToolView *> m_toolviews;

    int m_lastSize;

    QSize m_preHideSize;

    int m_popupButton = 0;

Q_SIGNALS:
    void sigShowPluginConfigPage(KTextEditor::Plugin *configpageinterface, int id);
};

class MainWindow : public KParts::MainWindow
{
    Q_OBJECT

    friend class ToolView;

    //
    // Constructor area
    //
public:
    /**
     * Constructor
     */
    MainWindow(QWidget *parentWidget = nullptr);

    /**
     * Destructor
     */
    ~MainWindow() override;

    //
    // public interfaces
    //

    /**
     * add a given widget to the given sidebar if possible, name is very important
     * @param plugin pointer to the plugin
     * @param identifier unique identifier for this toolview
     * @param pos position for the toolview, if we are in session restore, this is only a preference
     * @param icon icon to use for the toolview
     * @param text text to use in addition to icon
     * @return created toolview on success or 0
     */
    ToolView *
    createToolView(KTextEditor::Plugin *plugin, const QString &identifier, KMultiTabBar::KMultiTabBarPosition pos, const QIcon &icon, const QString &text);

    /**
     * give you handle to toolview for the given name, 0 if no toolview around
     * @param identifier toolview name
     * @return toolview if existing, else 0
     */
    ToolView *toolView(const QString &identifier) const;

    /**
     * set the toolview's tabbar style.
     * @param style the tabbar style.
     */
    void setToolViewStyle(KMultiTabBar::KMultiTabBarStyle style);

    /**
     * get the toolview's tabbar style. Call this before @p startRestore(),
     * otherwise you overwrite the usersettings.
     * @return toolview's tabbar style
     */
    KMultiTabBar::KMultiTabBarStyle toolViewStyle() const;

    /**
     * get the sidebars' visibility.
     * @return false, if the sidebars' visibility is forced hidden, otherwise true
     */
    bool sidebarsVisible() const;

public Q_SLOTS:
    /**
     * set the sidebars' visibility to @p visible. If false, the sidebars
     * are @e always hidden. Usually you do not have to call this because
     * the user can set this in the menu.
     * @param visible sidebars visibility
     */
    void setSidebarsVisible(bool visible);

protected:
    /**
     * called by toolview destructor
     * @param widget toolview which is destroyed
     */
    void toolViewDeleted(ToolView *widget);

    /**
     * central widget ;)
     * use this as parent for your content
     * this widget will get focus if a toolview is hidden
     * @return central widget
     */
    QWidget *centralWidget() const;

    /**
     * modifiers for existing toolviews
     */
public:
    /**
     * move a toolview around
     * @param widget toolview to move
     * @param pos position to move too, during session restore, only preference
     * @return success
     */
    bool moveToolView(ToolView *widget, KMultiTabBar::KMultiTabBarPosition pos);

    /**
     * show given toolview, discarded while session restore
     * @param widget toolview to show
     * @return success
     */
    bool showToolView(ToolView *widget);

    /**
     * hide given toolview, discarded while session restore
     * @param widget toolview to hide
     * @return success
     */
    bool hideToolView(ToolView *widget);

    /**
     * session saving and restore stuff
     */
public:
    /**
     * start the restore
     * @param config config object to use
     * @param group config group to use
     */
    void startRestore(KConfigBase *config, const QString &group);

    /**
     * finish the restore
     */
    void finishRestore();

    /**
     * save the current session config to given object and group
     * @param group config group to use
     */
    void saveSession(KConfigGroup &group);

    /**
     * internal data ;)
     */
private:
    /**
     * map identifiers to widgets
     */
    std::map<QString, ToolView *> m_idToWidget;

    /**
     * list of all toolviews around
     */
    std::vector<ToolView *> m_toolviews;

    /**
     * widget, which is the central part of the
     * main window ;)
     */
    QWidget *m_centralWidget;

    /**
     * horizontal splitter
     */
    QSplitter *m_hSplitter;

    /**
     * vertical splitter
     */
    QSplitter *m_vSplitter;

    /**
     * sidebars for the four sides
     */
    std::unique_ptr<Sidebar> m_sidebars[4];

    /**
     * sidebars state.
     */
    bool m_sidebarsVisible = true;

    /**
     * config object for session restore, only valid between
     * start and finish restore calls
     */
    KConfigBase *m_restoreConfig = nullptr;

    /**
     * restore group
     */
    QString m_restoreGroup;

    /**
     * out guiclient
     */
    GUIClient *m_guiClient;

Q_SIGNALS:
    void sigShowPluginConfigPage(KTextEditor::Plugin *configpageinterface, int id);
};

}

#endif
