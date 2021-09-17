/*
 *  SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ktexteditorpreviewview.h"

#include "ktexteditorpreviewplugin.h"
#include "previewwidget.h"

// KF
#include <KLocalizedString>
#include <KTextEditor/MainWindow>

// Qt
#include <QIcon>
#include <QLayout>

using namespace KTextEditorPreview;

KTextEditorPreviewView::KTextEditorPreviewView(KTextEditorPreviewPlugin *plugin, KTextEditor::MainWindow *mainWindow)
    : QObject(mainWindow)
{
    Q_UNUSED(plugin);

    m_toolView = mainWindow->createToolView(plugin,
                                            QStringLiteral("ktexteditorpreviewplugin"),
                                            KTextEditor::MainWindow::Right,
                                            QIcon::fromTheme(QStringLiteral("document-preview")),
                                            i18n("Preview"));

    // add preview widget
    m_previewView = new PreviewWidget(plugin, mainWindow, m_toolView.data());
    m_toolView->layout()->setContentsMargins(0, 0, 0, 0);
    m_toolView->layout()->addWidget(m_previewView);
    m_toolView->addActions(m_previewView->actions());
}

KTextEditorPreviewView::~KTextEditorPreviewView()
{
    delete m_toolView;
}

void KTextEditorPreviewView::readSessionConfig(const KConfigGroup &config)
{
    m_previewView->readSessionConfig(config);
}

void KTextEditorPreviewView::writeSessionConfig(KConfigGroup &config)
{
    m_previewView->writeSessionConfig(config);
}
