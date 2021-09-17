/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "katepluginmanager.h"

#include "kateapp.h"
#include "katedebug.h"
#include "katemainwindow.h"
#include "kateoutputview.h"

#include <KConfig>
#include <KConfigGroup>
#include <KPluginFactory>
#include <KPluginLoader>

#include <QFile>
#include <QFileInfo>
#include <QMetaObject>

#include <ktexteditor/sessionconfiginterface.h>

QString KatePluginInfo::saveName() const
{
    return QFileInfo(metaData.fileName()).baseName();
}

bool KatePluginInfo::operator<(const KatePluginInfo &other) const
{
    if (sortOrder != other.sortOrder) {
        return sortOrder < other.sortOrder;
    }

    return saveName() < other.saveName();
}

KatePluginManager::KatePluginManager(QObject *parent)
    : QObject(parent)
{
    setupPluginList();
}

KatePluginManager::~KatePluginManager()
{
    unloadAllPlugins();
}

void KatePluginManager::setupPluginList()
{
    // activate a hand-picked list of plugins per default, give them a hand-picked sort order for loading
    const QMap<QString, int> defaultPlugins{
        {QStringLiteral("katefiletreeplugin"), -1000},
        {QStringLiteral("katesearchplugin"), -900},
        {QStringLiteral("kateprojectplugin"), -800},
        {QStringLiteral("tabswitcherplugin"), -100},
        {QStringLiteral("textfilterplugin"), -100},
        {QStringLiteral("externaltoolsplugin"), -100}
#ifndef WIN32
        ,
        {QStringLiteral("katefilebrowserplugin"), -100} // currently works badly on Windows
        ,
        {QStringLiteral("katekonsoleplugin"), -100} // currently does not work on Windows at all
#endif
    };

    // handle all install KTextEditor plugins
    m_pluginList.clear();
    QSet<QString> unique;

    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("ktexteditor"));
    for (const auto &pluginMetaData : plugins) {
        KatePluginInfo info;
        info.metaData = pluginMetaData;

        // only load plugins once, even if found multiple times!
        if (unique.contains(info.saveName())) {
            continue;
        }

        info.defaultLoad = defaultPlugins.contains(info.saveName());
        info.sortOrder = defaultPlugins.value(info.saveName());
        info.load = false;
        info.plugin = nullptr;
        m_pluginList.push_back(info);
        unique.insert(info.saveName());
    }

    // sort to ensure some deterministic plugin load order, this is important for tool-view creation order
    std::sort(m_pluginList.begin(), m_pluginList.end());

    // construct fast lookup map, do this after vector has final size, resize will invalidate the pointers!
    m_name2Plugin.clear();
    for (auto &pluginInfo : m_pluginList) {
        m_name2Plugin[pluginInfo.saveName()] = &pluginInfo;
    }
}

void KatePluginManager::loadConfig(KConfig *config)
{
    // first: unload the plugins
    unloadAllPlugins();

    /**
     * ask config object
     */
    if (config) {
        KConfigGroup cg = KConfigGroup(config, QStringLiteral("Kate Plugins"));

        // disable all plugin if no config, beside the ones marked as default load
        for (auto &pluginInfo : m_pluginList) {
            pluginInfo.load = cg.readEntry(pluginInfo.saveName(), pluginInfo.defaultLoad);
        }
    }

    /**
     * load plugins
     */
    for (auto &pluginInfo : m_pluginList) {
        if (pluginInfo.load) {
            /**
             * load plugin + trigger update of GUI for already existing main windows
             */
            loadPlugin(&pluginInfo);
            enablePluginGUI(&pluginInfo);

            // restore config
            if (auto interface = qobject_cast<KTextEditor::SessionConfigInterface *>(pluginInfo.plugin)) {
                KConfigGroup group(config, QStringLiteral("Plugin:%1:").arg(pluginInfo.saveName()));
                interface->readSessionConfig(group);
            }
        }
    }
}

void KatePluginManager::writeConfig(KConfig *config)
{
    Q_ASSERT(config);

    KConfigGroup cg = KConfigGroup(config, QStringLiteral("Kate Plugins"));
    for (const KatePluginInfo &plugin : qAsConst(m_pluginList)) {
        QString saveName = plugin.saveName();

        cg.writeEntry(saveName, plugin.load);

        // save config
        if (auto interface = qobject_cast<KTextEditor::SessionConfigInterface *>(plugin.plugin)) {
            KConfigGroup group(config, QStringLiteral("Plugin:%1:").arg(saveName));
            interface->writeSessionConfig(group);
        }
    }
}

void KatePluginManager::unloadAllPlugins()
{
    for (auto &pluginInfo : m_pluginList) {
        if (pluginInfo.plugin) {
            unloadPlugin(&pluginInfo);
        }
    }
}

void KatePluginManager::enableAllPluginsGUI(KateMainWindow *win, KConfigBase *config)
{
    for (auto &pluginInfo : m_pluginList) {
        if (pluginInfo.plugin) {
            enablePluginGUI(&pluginInfo, win, config);
        }
    }
}

void KatePluginManager::disableAllPluginsGUI(KateMainWindow *win)
{
    for (auto &pluginInfo : m_pluginList) {
        if (pluginInfo.plugin) {
            disablePluginGUI(&pluginInfo, win);
        }
    }
}

bool KatePluginManager::loadPlugin(KatePluginInfo *item)
{
    /**
     * try to load the plugin
     */
    auto factory = KPluginLoader(item->metaData.fileName()).factory();
    if (factory) {
        item->plugin = factory->create<KTextEditor::Plugin>(this, QVariantList() << item->saveName());
    }
    item->load = item->plugin != nullptr;

    /**
     * tell the world about the success
     */
    if (item->plugin) {
        Q_EMIT KateApp::self()->wrapper()->pluginCreated(item->saveName(), item->plugin);
    }

    return item->plugin != nullptr;
}

void KatePluginManager::unloadPlugin(KatePluginInfo *item)
{
    disablePluginGUI(item);
    delete item->plugin;
    KTextEditor::Plugin *plugin = item->plugin;
    item->plugin = nullptr;
    item->load = false;
    Q_EMIT KateApp::self()->wrapper()->pluginDeleted(item->saveName(), plugin);
}

void KatePluginManager::enablePluginGUI(KatePluginInfo *item, KateMainWindow *win, KConfigBase *config)
{
    // plugin around at all?
    if (!item->plugin) {
        return;
    }

    // lookup if there is already a view for it..
    QObject *createdView = nullptr;
    if (!win->pluginViews().contains(item->plugin)) {
        // ensure messaging is connected, if available, for the complete plugin
        if (item->plugin->metaObject()->indexOfSignal("message(QVariantMap)") != -1) {
            connect(item->plugin, SIGNAL(message(const QVariantMap &)), win->outputView(), SLOT(slotMessage(const QVariantMap &)), Qt::UniqueConnection);
        }

        // create the view + try to correctly load shortcuts, if it's a GUI Client
        createdView = item->plugin->createView(win->wrapper());
        if (createdView) {
            win->pluginViews().insert(item->plugin, createdView);

            // ensure messaging is connected, if available, for view, too!
            if (createdView->metaObject()->indexOfSignal("message(QVariantMap)") != -1) {
                connect(createdView, SIGNAL(message(const QVariantMap &)), win->outputView(), SLOT(slotMessage(const QVariantMap &)), Qt::UniqueConnection);
            }

            // ensure location tracking is connected for view
            if (createdView->metaObject()->indexOfSignal("addPositionToHistory(QUrl,KTextEditor::Cursor)") != -1) {
                connect(createdView,
                        SIGNAL(addPositionToHistory(QUrl, KTextEditor::Cursor)),
                        win->viewManager(),
                        SLOT(addPositionToHistory(const QUrl &, KTextEditor::Cursor)),
                        Qt::UniqueConnection);
            }
        }
    }

    // load session config if needed
    if (config && win->pluginViews().contains(item->plugin)) {
        if (auto interface = qobject_cast<KTextEditor::SessionConfigInterface *>(win->pluginViews().value(item->plugin))) {
            KConfigGroup group(config, QStringLiteral("Plugin:%1:MainWindow:0").arg(item->saveName()));
            interface->readSessionConfig(group);
        }
    }

    if (createdView) {
        Q_EMIT win->wrapper()->pluginViewCreated(item->saveName(), createdView);
    }
}

void KatePluginManager::enablePluginGUI(KatePluginInfo *item)
{
    // plugin around at all?
    if (!item->plugin) {
        return;
    }

    // enable the gui for all mainwindows...
    for (int i = 0; i < KateApp::self()->mainWindowsCount(); i++) {
        enablePluginGUI(item, KateApp::self()->mainWindow(i), nullptr);
    }
}

void KatePluginManager::disablePluginGUI(KatePluginInfo *item, KateMainWindow *win)
{
    // plugin around at all?
    if (!item->plugin) {
        return;
    }

    // lookup if there is a view for it..
    if (!win->pluginViews().contains(item->plugin)) {
        return;
    }

    // really delete the view of this plugin
    QObject *deletedView = win->pluginViews().value(item->plugin);
    delete deletedView;
    win->pluginViews().remove(item->plugin);
    Q_EMIT win->wrapper()->pluginViewDeleted(item->saveName(), deletedView);
}

void KatePluginManager::disablePluginGUI(KatePluginInfo *item)
{
    // plugin around at all?
    if (!item->plugin) {
        return;
    }

    // disable the gui for all mainwindows...
    for (int i = 0; i < KateApp::self()->mainWindowsCount(); i++) {
        disablePluginGUI(item, KateApp::self()->mainWindow(i));
    }
}

KTextEditor::Plugin *KatePluginManager::plugin(const QString &name)
{
    /**
     * name known?
     */
    if (!m_name2Plugin.contains(name)) {
        return nullptr;
    }

    /**
     * real plugin instance, if any ;)
     */
    return m_name2Plugin.value(name)->plugin;
}

bool KatePluginManager::pluginAvailable(const QString &name)
{
    return m_name2Plugin.contains(name);
}

class KTextEditor::Plugin *KatePluginManager::loadPlugin(const QString &name, bool permanent)
{
    /**
     * name known?
     */
    if (!m_name2Plugin.contains(name)) {
        return nullptr;
    }

    /**
     * load, bail out on error
     */
    loadPlugin(m_name2Plugin.value(name));
    if (!m_name2Plugin.value(name)->plugin) {
        return nullptr;
    }

    /**
     * perhaps patch not load again back to "ok, load it once again on next loadConfig"
     */
    m_name2Plugin.value(name)->load = permanent;
    return m_name2Plugin.value(name)->plugin;
}

void KatePluginManager::unloadPlugin(const QString &name, bool permanent)
{
    /**
     * name known?
     */
    if (!m_name2Plugin.contains(name)) {
        return;
    }

    /**
     * unload
     */
    unloadPlugin(m_name2Plugin.value(name));

    /**
     * perhaps patch load again back to "ok, load it once again on next loadConfig"
     */
    m_name2Plugin.value(name)->load = !permanent;
}
