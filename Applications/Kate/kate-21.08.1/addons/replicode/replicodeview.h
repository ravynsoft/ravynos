/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2014 Martin Sandsmark <martin.sandsmark@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef REPLICODEVIEW_H
#define REPLICODEVIEW_H

#include <KTextEditor/Plugin>
#include <KTextEditor/SessionConfigInterface>
#include <KXMLGUIClient>
#include <QProcess>

class QListWidgetItem;
class QListWidget;
class QTemporaryFile;
class QListWidget;
class QPushButton;
class QAction;
class ReplicodeConfig;

class ReplicodeView : public QObject, public KXMLGUIClient, public KTextEditor::SessionConfigInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::SessionConfigInterface)

public:
    explicit ReplicodeView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainWindow);
    ~ReplicodeView() override;
    void readSessionConfig(const KConfigGroup &) override
    {
    }
    void writeSessionConfig(KConfigGroup &) override
    {
    }

private Q_SLOTS:
    void runReplicode();
    void stopReplicode();
    void replicodeFinished();
    void gotStderr();
    void gotStdout();
    void runErrored(QProcess::ProcessError);
    void outputClicked(QListWidgetItem *item);
    void viewChanged();

private:
    KTextEditor::MainWindow *m_mainWindow;
    QProcess *m_executor;
    QListWidget *m_replicodeOutput;
    QWidget *m_toolview;
    QWidget *m_configSidebar;
    QPushButton *m_runButton;
    QPushButton *m_stopButton;
    QAction *m_runAction;
    QAction *m_stopAction;
    ReplicodeConfig *m_configView;
    bool m_completed = false;
};

#endif
