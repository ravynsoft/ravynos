/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __KATE_PLUGINMANAGER_H__
#define __KATE_PLUGINMANAGER_H__

#include <KTextEditor/Plugin>

#include <KConfigBase>
#include <KPluginMetaData>

#include <QList>
#include <QMap>
#include <QObject>

class KConfig;
class KateMainWindow;

class KatePluginInfo
{
public:
    bool load = false;
    bool defaultLoad = false;
    KPluginMetaData metaData;
    KTextEditor::Plugin *plugin = nullptr;
    int sortOrder = 0;
    QString saveName() const;
    bool operator<(const KatePluginInfo &other) const;
};

typedef QList<KatePluginInfo> KatePluginList;

class KatePluginManager : public QObject
{
    Q_OBJECT

public:
    KatePluginManager(QObject *parent);
    ~KatePluginManager() override;

    void unloadAllPlugins();

    void enableAllPluginsGUI(KateMainWindow *win, KConfigBase *config = nullptr);
    void disableAllPluginsGUI(KateMainWindow *win);

    void loadConfig(KConfig *);
    void writeConfig(KConfig *);

    bool loadPlugin(KatePluginInfo *item);
    void unloadPlugin(KatePluginInfo *item);

    void enablePluginGUI(KatePluginInfo *item, KateMainWindow *win, KConfigBase *config = nullptr);
    void enablePluginGUI(KatePluginInfo *item);

    void disablePluginGUI(KatePluginInfo *item, KateMainWindow *win);
    void disablePluginGUI(KatePluginInfo *item);

    inline KatePluginList &pluginList()
    {
        return m_pluginList;
    }

    KTextEditor::Plugin *plugin(const QString &name);
    bool pluginAvailable(const QString &name);

    KTextEditor::Plugin *loadPlugin(const QString &name, bool permanent = true);
    void unloadPlugin(const QString &name, bool permanent = true);

private:
    void setupPluginList();

    /**
     * all known plugins
     */
    KatePluginList m_pluginList;

    /**
     * fast access map from name => plugin info
     * uses the info stored in the plugin list
     */
    QMap<QString, KatePluginInfo *> m_name2Plugin;
};

#endif
