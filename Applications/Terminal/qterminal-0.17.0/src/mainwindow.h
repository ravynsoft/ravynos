/***************************************************************************
 *   Copyright (C) 2006 by Vladimir Kuznetsov                              *
 *   vovanec@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_qterminal.h"

#include <QMainWindow>
#include <QAction>

#include "qxtglobalshortcut.h"
#include "terminalconfig.h"
#include "dbusaddressable.h"


class QToolButton;

class MainWindow : public QMainWindow, private Ui::mainWindow, public DBusAddressable
{
    Q_OBJECT

public:
    MainWindow(TerminalConfig& cfg,
               bool dropMode,
               QWidget * parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~MainWindow() override;

    bool dropMode() { return m_dropMode; }
    QMap<QString, QAction*> & leaseActions();

    #ifdef HAVE_QDBUS
    QDBusObjectPath getActiveTab();
    QList<QDBusObjectPath> getTabs();
    QDBusObjectPath newTab(const QHash<QString,QVariant> &termArgs);
    void closeWindow();
    #endif

protected:
     bool event(QEvent* event) override;

private:
    QActionGroup *tabPosition, *scrollBarPosition, *keyboardCursorShape;
    QMenu *tabPosMenu, *scrollPosMenu, *keyboardCursorShapeMenu;

    // A parent object for QObjects that are created dynamically based on settings
    // Used to simplify the setting cleanup on reconfiguration: deleting settingOwner frees all related QObjects
    QWidget *settingOwner;

    QMenu *presetsMenu;
    bool m_removeFinished;
    TerminalConfig m_config;

    QDockWidget *m_bookmarksDock;

    void setup_Action(const char *name, QAction *action, const char *defaultShortcut, const QObject *receiver,
                      const char *slot, QMenu *menu = nullptr, const QVariant &data = QVariant());
    QMap< QString, QAction * > actions;

    QStringList menubarOrigTexts;

    void rebuildActions();

    void setup_AppMenu_Actions();
    void setup_ShellMenu_Actions();
    void setup_EditMenu_Actions();
    void setup_ViewMenu_Actions();
    void setup_WindowMenu_Actions();
    void setup_ContextMenu_Actions();
    void setupCustomDirs();

    void closeEvent(QCloseEvent*) override;

    void enableDropMode();
    QToolButton *m_dropLockButton;
    bool m_dropMode;
    QxtGlobalShortcut m_dropShortcut;
    void realign();
    void setDropShortcut(const QKeySequence& dropShortCut);

    bool hasMultipleTabs(QAction *);
    bool hasMultipleSubterminals(QAction *);
    bool hasIndexedTab(QAction *action);

public slots:
    void showHide();
    void updateDisabledActions();

private slots:
    void on_consoleTabulator_currentChanged(int);
    void propertiesChanged();
    void actAbout_triggered();
    void actProperties_triggered();
    void updateActionGroup(QAction *);
    void testClose(bool removeFinished);
    void toggleBookmarks();
    void toggleBorderless();
    void toggleTabBar();
    void toggleMenu();

    void showFullscreen(bool fullscreen);
    void setKeepOpen(bool value);
    void find();

    void newTerminalWindow();
    void bookmarksWidget_callCommand(const QString&);
    void bookmarksDock_visibilityChanged(bool visible);

    void addNewTab();
    void onCurrentTitleChanged(int index);

    void handleHistory();
};
#endif //MAINWINDOW_H
