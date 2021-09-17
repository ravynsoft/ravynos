//
// Description: Kate Plugin for GDB integration
//
//
// SPDX-FileCopyrightText: 2010 Ian Wakeling <ian.wakeling@ntlworld.com>
// SPDX-FileCopyrightText: 2010-2014 Kåre Särs <kare.sars@iki.fi>
//
//  SPDX-License-Identifier: LGPL-2.0-only

#ifndef PLUGIN_KATEGDB_H
#define PLUGIN_KATEGDB_H

#include <QPointer>

#include <KActionMenu>
#include <KTextEditor/Application>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Message>
#include <KTextEditor/Plugin>
#include <KTextEditor/SessionConfigInterface>
#include <KXMLGUIClient>

#include "configview.h"
#include "debugview.h"
#include "ioview.h"
#include "localsview.h"

class KHistoryComboBox;
class QTextEdit;
class QTreeWidget;

typedef QList<QVariant> VariantList;

class KatePluginGDB : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit KatePluginGDB(QObject *parent = nullptr, const VariantList & = VariantList());
    ~KatePluginGDB() override;

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;
};

class KatePluginGDBView : public QObject, public KXMLGUIClient, public KTextEditor::SessionConfigInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::SessionConfigInterface)

public:
    KatePluginGDBView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainWin);
    ~KatePluginGDBView() override;

    // reimplemented: read and write session config
    void readSessionConfig(const KConfigGroup &config) override;
    void writeSessionConfig(KConfigGroup &config) override;

private Q_SLOTS:
    void slotDebug();
    void slotRestart();
    void slotToggleBreakpoint();
    void slotMovePC();
    void slotRunToCursor();
    void slotGoTo(const QUrl &fileName, int lineNum);
    void slotValue();

    void aboutToShowMenu();
    void slotBreakpointSet(const QUrl &file, int line);
    void slotBreakpointCleared(const QUrl &file, int line);
    void slotSendCommand();
    void enableDebugActions(bool enable);
    void programEnded();
    void gdbEnded();

    void insertStackFrame(QString const &level, QString const &info);
    void stackFrameChanged(int level);
    void stackFrameSelected();

    void insertThread(int number, bool active);
    void threadSelected(int thread);

    void showIO(bool show);
    void addOutputText(QString const &text);
    void addErrorText(QString const &text);
    void clearMarks();
    void handleEsc(QEvent *e);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    QString currentWord();

    void displayMessage(const QString &message, KTextEditor::Message::MessageType level);

    KTextEditor::Application *m_kateApplication;
    KTextEditor::MainWindow *m_mainWin;
    std::unique_ptr<QWidget> m_toolView;
    std::unique_ptr<QWidget> m_localsStackToolView;
    QTabWidget *m_tabWidget;
    QTextEdit *m_outputArea;
    KHistoryComboBox *m_inputArea;
    QWidget *m_gdbPage;
    QComboBox *m_threadCombo;
    int m_activeThread;
    QTreeWidget *m_stackTree;
    QString m_lastCommand;
    DebugView *m_debugView;
    ConfigView *m_configView;
    std::unique_ptr<IOView> m_ioView;
    LocalsView *m_localsView;
    QPointer<KActionMenu> m_menu;
    QAction *m_breakpoint;
    QUrl m_lastExecUrl;
    int m_lastExecLine;
    int m_lastExecFrame;
    bool m_focusOnInput;
    QPointer<KTextEditor::Message> m_infoMessage;
};

#endif
