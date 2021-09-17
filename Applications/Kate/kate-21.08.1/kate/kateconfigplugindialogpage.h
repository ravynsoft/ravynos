/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __KATE_CONFIGPLUGINDIALOGPAGE_H__
#define __KATE_CONFIGPLUGINDIALOGPAGE_H__

#include <QFrame>
#include <QTreeWidget>

class KatePluginListItem;

class KatePluginListView : public QTreeWidget
{
    Q_OBJECT

public:
    KatePluginListView(QWidget *parent = nullptr);

Q_SIGNALS:
    void stateChange(KatePluginListItem *, bool);

private Q_SLOTS:
    void stateChanged(QTreeWidgetItem *);
};

class KateConfigPluginPage : public QFrame
{
    Q_OBJECT

public:
    KateConfigPluginPage(QWidget *parent, class KateConfigDialog *dialog);

private:
    class KateConfigDialog *myDialog;

Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void stateChange(KatePluginListItem *, bool);

    void loadPlugin(KatePluginListItem *);
    void unloadPlugin(KatePluginListItem *);
};

#endif
