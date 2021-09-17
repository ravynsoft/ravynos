/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2014 Martin Sandsmark <martin.sandsmark@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "replicodeplugin.h"
#include "replicodeconfigpage.h"

#include <KPluginFactory>

#include <KTextEditor/Application>

K_PLUGIN_FACTORY_WITH_JSON(KateReplicodePluginFactory, "katereplicodeplugin.json", registerPlugin<ReplicodePlugin>();)

ReplicodePlugin::ReplicodePlugin(QObject *parent, const QList<QVariant> &args)
    : KTextEditor::Plugin(qobject_cast<KTextEditor::Application *>(parent))
{
    Q_UNUSED(args);
}

ReplicodePlugin::~ReplicodePlugin()
{
}

KTextEditor::ConfigPage *ReplicodePlugin::configPage(int number, QWidget *parent)
{
    Q_UNUSED(number);
    Q_ASSERT(number == 0);
    return new ReplicodeConfigPage(parent);
}

#include "replicodeplugin.moc"
