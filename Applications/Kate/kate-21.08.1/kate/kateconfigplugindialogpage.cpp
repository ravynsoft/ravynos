/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kateconfigplugindialogpage.h"

#include "kateapp.h"
#include "kateconfigdialog.h"
#include "katepluginmanager.h"

#include <KLocalizedString>

#include <QVBoxLayout>

class KatePluginListItem : public QTreeWidgetItem
{
public:
    KatePluginListItem(bool checked, KatePluginInfo *info);

    KatePluginInfo *info() const
    {
        return mInfo;
    }

protected:
    void stateChange(bool);

private:
    KatePluginInfo *mInfo;
};

KatePluginListItem::KatePluginListItem(bool checked, KatePluginInfo *info)
    : mInfo(info)
{
    setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
}

KatePluginListView::KatePluginListView(QWidget *parent)
    : QTreeWidget(parent)
{
    setRootIsDecorated(false);
    connect(this, &KatePluginListView::itemChanged, this, &KatePluginListView::stateChanged);
}

void KatePluginListView::stateChanged(QTreeWidgetItem *item)
{
    Q_EMIT stateChange(static_cast<KatePluginListItem *>(item), item->checkState(0) == Qt::Checked);
}

KateConfigPluginPage::KateConfigPluginPage(QWidget *parent, KateConfigDialog *dialog)
    : QFrame(parent)
    , myDialog(dialog)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    KatePluginListView *listView = new KatePluginListView(this);
    layout->addWidget(listView);

    QStringList headers;
    headers << i18n("Name") << i18n("Description");
    listView->setHeaderLabels(headers);
    listView->setWhatsThis(
        i18n("Here you can see all available Kate plugins. Those with a check mark are loaded, and will be loaded again the next time Kate is started."));

    KatePluginList &pluginList(KateApp::self()->pluginManager()->pluginList());
    for (auto &pluginInfo : pluginList) {
        QTreeWidgetItem *item = new KatePluginListItem(pluginInfo.load, &pluginInfo);
        item->setText(0, pluginInfo.metaData.name());
        item->setText(1, pluginInfo.metaData.description());
        listView->addTopLevelItem(item);
    }

    listView->resizeColumnToContents(0);
    listView->sortByColumn(0, Qt::AscendingOrder);
    connect(listView, &KatePluginListView::stateChange, this, &KateConfigPluginPage::stateChange);
}

void KateConfigPluginPage::stateChange(KatePluginListItem *item, bool b)
{
    if (b) {
        loadPlugin(item);
    } else {
        unloadPlugin(item);
    }

    Q_EMIT changed();
}

void KateConfigPluginPage::loadPlugin(KatePluginListItem *item)
{
    const bool ok = KateApp::self()->pluginManager()->loadPlugin(item->info());
    if (!ok) {
        return;
    }
    KateApp::self()->pluginManager()->enablePluginGUI(item->info());
    myDialog->addPluginPage(item->info()->plugin);

    item->setCheckState(0, Qt::Checked);
}

void KateConfigPluginPage::unloadPlugin(KatePluginListItem *item)
{
    myDialog->removePluginPage(item->info()->plugin);
    KateApp::self()->pluginManager()->unloadPlugin(item->info());

    item->setCheckState(0, Qt::Unchecked);
}
