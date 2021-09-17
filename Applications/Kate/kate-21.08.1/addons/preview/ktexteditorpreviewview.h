/*
 *  SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KTEXTEDITORPREVIEWVIEW_H
#define KTEXTEDITORPREVIEWVIEW_H

// KF
#include <KTextEditor/SessionConfigInterface>
// Qt
#include <QObject>
#include <QPointer>

namespace KTextEditorPreview
{
class PreviewWidget;
}

namespace KTextEditor
{
class MainWindow;
class View;
}

class KTextEditorPreviewPlugin;

class QWidget;

class KTextEditorPreviewView : public QObject, public KTextEditor::SessionConfigInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::SessionConfigInterface)

public:
    KTextEditorPreviewView(KTextEditorPreviewPlugin *plugin, KTextEditor::MainWindow *mainWindow);
    ~KTextEditorPreviewView() override;

    void readSessionConfig(const KConfigGroup &config) override;
    void writeSessionConfig(KConfigGroup &config) override;

private:
    QPointer<QWidget> m_toolView;
    KTextEditorPreview::PreviewWidget *m_previewView;
};

#endif
