/***************************************************************************
                         plugin_katetextfilter.h  -  description
                            -------------------
   begin                : FRE Feb 23 2001
   copyright            : (C) 2001 by Joseph Wenninger
   email                : jowenn@bigfoot.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef PLUGIN_KATETEXTFILTER_H
#define PLUGIN_KATETEXTFILTER_H

#include <KTextEditor/Application>
#include <KTextEditor/Command>
#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>
#include <KTextEditor/View>

#include <KProcess>
#include <QVariantList>

class PluginKateTextFilter : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    /**
     * Plugin constructor.
     */
    explicit PluginKateTextFilter(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());

    ~PluginKateTextFilter() override;

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

    void runFilter(KTextEditor::View *kv, const QString &filter);

private:
    QString m_strFilterOutput;
    QString m_stderrOutput;
    QString m_last_command;
    KProcess *m_pFilterProcess = nullptr;
    QStringList completionList;
    bool copyResult = false;
    bool mergeOutput = false;
    bool newDocument = false;
    KTextEditor::MainWindow *m_mainWindow;
public Q_SLOTS:
    void slotEditFilter();
    void slotFilterReceivedStdout();
    void slotFilterReceivedStderr();
    void slotFilterProcessExited(int exitCode, QProcess::ExitStatus exitStatus);
};

class PluginKateTextFilterCommand : public KTextEditor::Command
{
    Q_OBJECT

public:
    PluginKateTextFilterCommand(PluginKateTextFilter *plugin);
    // Kate::Command
    bool exec(KTextEditor::View *view, const QString &cmd, QString &msg, const KTextEditor::Range &range = KTextEditor::Range::invalid()) override;
    bool help(KTextEditor::View *view, const QString &cmd, QString &msg) override;

private:
    PluginKateTextFilter *m_plugin;
};

/**
 * Plugin view to merge the actions into the UI
 */
class PluginViewKateTextFilter : public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    /**
     * Construct plugin view
     * @param plugin our plugin
     * @param mainwindows the mainwindow for this view
     */
    explicit PluginViewKateTextFilter(PluginKateTextFilter *plugin, KTextEditor::MainWindow *mainwindow);

    /**
     * Our Destructor
     */
    ~PluginViewKateTextFilter() override;

private:
    /**
     * the main window we belong to
     */
    KTextEditor::MainWindow *m_mainWindow;
};

#endif
