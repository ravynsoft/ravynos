/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>
   SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
   SPDX-FileCopyrightText: 2009 Dominik Haumann <dhaumann kde org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KATE_FILEBROWSER_PLUGIN_H
#define KATE_FILEBROWSER_PLUGIN_H

#include <KTextEditor/SessionConfigInterface>
#include <ktexteditor/configpage.h>
#include <ktexteditor/document.h>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/plugin.h>

class KateFileBrowser;
class KateFileBrowserPluginView;

class KateFileBrowserPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit KateFileBrowserPlugin(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());
    ~KateFileBrowserPlugin() override
    {
    }

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

    int configPages() const override;
    KTextEditor::ConfigPage *configPage(int number = 0, QWidget *parent = nullptr) override;

public Q_SLOTS:
    void viewDestroyed(QObject *view);

private:
    QList<KateFileBrowserPluginView *> m_views;
};

class KateFileBrowserPluginView : public QObject, public KTextEditor::SessionConfigInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::SessionConfigInterface)

public:
    /**
     * Constructor.
     */
    KateFileBrowserPluginView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainWindow);

    /**
     * Virtual destructor.
     */
    ~KateFileBrowserPluginView() override;

    void readSessionConfig(const KConfigGroup &config) override;
    void writeSessionConfig(KConfigGroup &config) override;

private:
    bool eventFilter(QObject *, QEvent *) override;

    QWidget *m_toolView;
    KateFileBrowser *m_fileBrowser;
    KTextEditor::MainWindow *m_mainWindow;
    friend class KateFileBrowserPlugin;
};

#endif // KATE_FILEBROWSER_PLUGIN_H

// kate: space-indent on; indent-width 2; replace-tabs on;
