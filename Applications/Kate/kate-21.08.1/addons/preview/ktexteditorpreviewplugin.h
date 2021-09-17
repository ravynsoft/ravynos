/*
 *  SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KTEXTEDITORPREVIEWPLUGIN_H
#define KTEXTEDITORPREVIEWPLUGIN_H

// KF
#include <KTextEditor/Plugin>

class KTextEditorPreviewPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    /**
     * Default constructor, with arguments as expected by KPluginFactory
     */
    KTextEditorPreviewPlugin(QObject *parent, const QVariantList &args);

    ~KTextEditorPreviewPlugin() override;

public: // KTextEditor::Plugin API
    QObject *createView(KTextEditor::MainWindow *mainWindow) override;
};

#endif
