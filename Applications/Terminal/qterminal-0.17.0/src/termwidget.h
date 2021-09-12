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

#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <qtermwidget.h>
#include "terminalconfig.h"

#include <QAction>
#include "dbusaddressable.h"

class TermWidgetImpl : public QTermWidget
{
    Q_OBJECT

//        QMap< QString, QAction * > actionMap;

    public:

        TermWidgetImpl(TerminalConfig &cfg, QWidget * parent=nullptr);
        void propertiesChanged();

    signals:
        void renameSession();
        void removeCurrentSession();

    public slots:
        void zoomIn();
        void zoomOut();
        void zoomReset();

    private slots:
        void customContextMenuCall(const QPoint & pos);
        void activateUrl(const QUrl& url, bool fromContextMenu);
};


class TermWidget : public QWidget, public DBusAddressable
{
    Q_OBJECT

    TermWidgetImpl * m_term;
    QVBoxLayout * m_layout;
    QColor m_border;

    public:
        TermWidget(TerminalConfig &cfg, QWidget * parent=nullptr);

        void propertiesChanged();
        QStringList availableKeyBindings() { return m_term->availableKeyBindings(); }

        TermWidgetImpl * impl() { return m_term; }

        #ifdef HAVE_QDBUS
        QDBusObjectPath splitHorizontal(const QHash<QString,QVariant> &termArgs);
        QDBusObjectPath splitVertical(const QHash<QString,QVariant> &termArgs);
        QDBusObjectPath getTab();
        void sendText(const QString& text);
        void closeTerminal();
        #endif

    signals:
        void finished();
        void renameSession();
        void removeCurrentSession();
        void splitHorizontal(TermWidget * self);
        void splitVertical(TermWidget * self);
        void splitCollapse(TermWidget * self);
        void termGetFocus(TermWidget * self);
        void termTitleChanged(QString titleText, QString icon);

    public slots:

    protected:
        bool focusNextPrevChild(bool) override {
            // prevent focus change with Tab and, especially, Backtab
            return false;
        }
        void paintEvent (QPaintEvent * event) override;

    private slots:
        void term_termGetFocus();
        void term_termLostFocus();
};

#endif

