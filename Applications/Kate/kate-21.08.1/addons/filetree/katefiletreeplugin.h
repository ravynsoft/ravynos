/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2010 Thomas Fjellstrom <thomas@fjellstrom.ca>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_FILETREE_PLUGIN_H
#define KATE_FILETREE_PLUGIN_H

#include <QIcon>
#include <QTimer>

#include <KTextEditor/Command>
#include <KTextEditor/Plugin>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/sessionconfiginterface.h>

#include "katefiletreepluginsettings.h"

#include <KXMLGUIClient>

class KToolBar;

class KateFileTree;
class KateFileTreeModel;
class KateFileTreeProxyModel;
class KateFileTreeConfigPage;
class KateFileTreePluginView;

class QLineEdit;

class KateFileTreePlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit KateFileTreePlugin(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());
    ~KateFileTreePlugin() override;

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

    int configPages() const override;
    KTextEditor::ConfigPage *configPage(int number = 0, QWidget *parent = nullptr) override;

    const KateFileTreePluginSettings &settings();

    void applyConfig(bool shadingEnabled,
                     const QColor &viewShade,
                     const QColor &editShade,
                     bool listMode,
                     int sortRole,
                     bool showFullPath,
                     bool showToolbar,
                     bool closeButton);

public Q_SLOTS:
    void viewDestroyed(QObject *view);

private:
    QList<KateFileTreePluginView *> m_views;
    KateFileTreeConfigPage *m_confPage = nullptr;
    KateFileTreePluginSettings m_settings;
};

class KateFileTreePluginView : public QObject, public KXMLGUIClient, public KTextEditor::SessionConfigInterface
{
    Q_OBJECT

    Q_INTERFACES(KTextEditor::SessionConfigInterface)

public:
    /**
     * Constructor.
     */
    KateFileTreePluginView(KTextEditor::MainWindow *mainWindow, KateFileTreePlugin *plug);

    /**
     * Virtual destructor.
     */
    ~KateFileTreePluginView() override;

    void readSessionConfig(const KConfigGroup &config) override;
    void writeSessionConfig(KConfigGroup &config) override;

    /**
     * The file tree model.
     * @return the file tree model
     */
    KateFileTreeModel *model() const;
    /**
     * The file tree proxy model.
     * @return the file tree proxy model
     */
    KateFileTreeProxyModel *proxy() const;
    /**
     * The file tree.
     * @return the file tree
     */
    KateFileTree *tree() const;

    void setToolbarVisible(bool);
    void setListMode(bool listMode);

    bool hasLocalPrefs() const;
    void setHasLocalPrefs(bool);

protected:
    void setupActions();

private:
    QWidget *m_toolView;
    KToolBar *m_toolbar;
    KateFileTree *m_fileTree;
    KateFileTreeProxyModel *m_proxyModel;
    QLineEdit *m_filter;
    KateFileTreeModel *m_documentModel;
    bool m_hasLocalPrefs = false;
    KateFileTreePlugin *m_plug;
    KTextEditor::MainWindow *m_mainWindow;
    QTimer m_documentsCreatedTimer;
    QList<KTextEditor::Document *> m_documentsCreated;

private Q_SLOTS:
    void showToolView();
    void hideToolView();
    void showActiveDocument();
    void activateDocument(KTextEditor::Document *);
    void viewChanged(KTextEditor::View * = nullptr);
    void documentOpened(KTextEditor::Document *);
    void documentClosed(KTextEditor::Document *);
    void viewModeChanged(bool);
    void sortRoleChanged(int);
    void slotDocumentsCreated();
    void slotDocumentSave() const;
    void slotDocumentSaveAs() const;
};

#endif // KATE_FILETREE_PLUGIN_H
