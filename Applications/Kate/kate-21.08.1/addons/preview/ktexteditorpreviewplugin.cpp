/*
 *  SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ktexteditorpreviewplugin.h"

#include "ktexteditorpreviewview.h"
#include <ktexteditorpreview_debug.h>

// KF
#include <KPluginFactory>
#include <KTextEditor/MainWindow>

K_PLUGIN_FACTORY_WITH_JSON(KTextEditorPreviewPluginFactory, "ktexteditorpreview.json", registerPlugin<KTextEditorPreviewPlugin>();)

KTextEditorPreviewPlugin::KTextEditorPreviewPlugin(QObject *parent, const QVariantList & /*args*/)
    : KTextEditor::Plugin(parent)
{
}

KTextEditorPreviewPlugin::~KTextEditorPreviewPlugin() = default;

QObject *KTextEditorPreviewPlugin::createView(KTextEditor::MainWindow *mainwindow)
{
    return new KTextEditorPreviewView(this, mainwindow);
}

// needed for K_PLUGIN_FACTORY_WITH_JSON
#include <ktexteditorpreviewplugin.moc>
