/***************************************************************************
 *   Copyright (C) 2010 by Petr Vanek                                      *
 *   petr@scribus.info                                                     *
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

#ifndef TERMWIDGETHOLDER_H
#define TERMWIDGETHOLDER_H

#include <QWidget>
#include "termwidget.h"
#include "terminalconfig.h"
#include "dbusaddressable.h"
class QSplitter;



typedef enum NavigationDirection {
    Left,
    Right,
    Top,
    Bottom
} NavigationDirection;


/*! \brief TermWidget group/session manager.

This widget (one per TabWidget tab) is a "proxy" widget beetween TabWidget and
unspecified count of TermWidgets. Basically it should look like a single TermWidget
for TabWidget - with its signals and slots.

Splitting and collapsing of TermWidgets is done here.
*/
class TermWidgetHolder : public QWidget
#ifdef HAVE_QDBUS
    , public DBusAddressable
#endif
{
    Q_OBJECT

    public:
        TermWidgetHolder(TerminalConfig &cfg, QWidget * parent=nullptr);
        ~TermWidgetHolder() override;

        void propertiesChanged();
        void setInitialFocus();

        void loadSession();
        void saveSession(const QString & name);
        void zoomIn(uint step);
        void zoomOut(uint step);

        TermWidget* currentTerminal();
        TermWidget* split(TermWidget * term, Qt::Orientation orientation, TerminalConfig cfg);

        #ifdef HAVE_QDBUS
        QDBusObjectPath getActiveTerminal();
        QList<QDBusObjectPath> getTerminals();
        QDBusObjectPath getWindow();
        void closeTab();
        #endif


    public slots:
        void splitHorizontal(TermWidget * term);
        void splitVertical(TermWidget * term);
        void splitCollapse(TermWidget * term);
        void setWDir(const QString & wdir);
        void directionalNavigation(NavigationDirection dir);
        void clearActiveTerminal();
        void onTermTitleChanged(QString title, QString icon) const;

    signals:
        void finished();
        void lastTerminalClosed();
        void renameSession();
        void termTitleChanged(QString title, QString icon) const;

    private:
        QString m_wdir;
        QString m_shell;
        TermWidget * m_currentTerm;

        void split(TermWidget * term, Qt::Orientation orientation);
        TermWidget * newTerm(TerminalConfig &cfg);

    private slots:
        void setCurrentTerminal(TermWidget* term);
        void handle_finished();
};

#endif

