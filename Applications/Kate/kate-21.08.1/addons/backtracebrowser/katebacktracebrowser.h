/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008-2014 Dominik Haumann <dhaumann kde org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KATE_BACKTRACEBROWSER_H
#define KATE_BACKTRACEBROWSER_H

#include <KTextEditor/Plugin>
#include <ktexteditor/configpage.h>
#include <ktexteditor/mainwindow.h>

#include "btdatabase.h"
#include "btfileindexer.h"
#include "ui_btbrowserwidget.h"
#include "ui_btconfigwidget.h"

#include <QDialog>
#include <QString>
#include <QTimer>

class KateBtConfigWidget;
class KateBtBrowserWidget;

class KateBtBrowserPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit KateBtBrowserPlugin(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());
    ~KateBtBrowserPlugin() override;

    static KateBtBrowserPlugin &self();

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

    KateBtDatabase &database();
    BtFileIndexer &fileIndexer();

    void startIndexer();

Q_SIGNALS:
    void newStatus(const QString &);

public:
    int configPages() const override;
    KTextEditor::ConfigPage *configPage(int number, QWidget *parent = nullptr) override;

    //
    // private data
    //
private:
    KateBtDatabase db;
    BtFileIndexer indexer;
    static KateBtBrowserPlugin *s_self;
};

class KateBtBrowserPluginView : public QObject
{
    Q_OBJECT

public:
    KateBtBrowserPluginView(KateBtBrowserPlugin *plugin, KTextEditor::MainWindow *mainWindow);

    /**
     * Virtual destructor.
     */
    ~KateBtBrowserPluginView() override;

private:
    KateBtBrowserPlugin *m_plugin;
    KateBtBrowserWidget *m_widget;
};

class KateBtBrowserWidget : public QWidget, public Ui::BtBrowserWidget
{
    Q_OBJECT

public:
    KateBtBrowserWidget(KTextEditor::MainWindow *mainwindow, QWidget *parent);

    ~KateBtBrowserWidget() override;

    void loadBacktrace(const QString &bt);

public Q_SLOTS:
    void loadFile();
    void loadClipboard();
    void configure();
    void clearStatus();
    void setStatus(const QString &status);

private Q_SLOTS:
    void itemActivated(QTreeWidgetItem *item, int column);

private:
    KTextEditor::MainWindow *mw;
    QTimer timer;
};

class KateBtConfigWidget : public KTextEditor::ConfigPage, private Ui::BtConfigWidget
{
    Q_OBJECT
public:
    explicit KateBtConfigWidget(QWidget *parent = nullptr);
    ~KateBtConfigWidget() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public Q_SLOTS:
    void apply() override;
    void reset() override;
    void defaults() override;

private Q_SLOTS:
    void add();
    void remove();
    void textChanged();

private:
    bool m_changed;
};

class KateBtConfigDialog : public QDialog
{
    Q_OBJECT
public:
    KateBtConfigDialog(QWidget *parent = nullptr);
    ~KateBtConfigDialog() override;

private:
    KateBtConfigWidget *m_configWidget;
};

#endif // KATE_BACKTRACEBROWSER_H

// kate: space-indent on; indent-width 4; replace-tabs on;
