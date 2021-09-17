//
// configview.h
//
// Description: View for configuring the set of targets to be used with the debugger
//
//
// SPDX-FileCopyrightText: 2010 Ian Wakeling <ian.wakeling@ntlworld.com>
// SPDX-FileCopyrightText: 2012 Kåre Särs <kare.sars@iki.fi>
//
//  SPDX-License-Identifier: LGPL-2.0-only

#ifndef CONFIGVIEW_H
#define CONFIGVIEW_H

#include "advanced_settings.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QResizeEvent>
#include <QToolButton>
#include <QWidget>

#include <QList>

#include <KActionCollection>
#include <KConfigGroup>
#include <KSelectAction>
#include <KTextEditor/MainWindow>

struct GDBTargetConf {
    QString targetName;
    QString executable;
    QString workDir;
    QString arguments;
    QString gdbCmd;
    QStringList customInit;
    QStringList srcPaths;
};

class ConfigView : public QWidget
{
    Q_OBJECT
public:
    enum TargetStringOrder { NameIndex = 0, ExecIndex, WorkDirIndex, ArgsIndex, GDBIndex, CustomStartIndex };

    ConfigView(QWidget *parent, KTextEditor::MainWindow *mainWin);
    ~ConfigView() override;

public:
    void registerActions(KActionCollection *actionCollection);

    void readConfig(const KConfigGroup &config);
    void writeConfig(KConfigGroup &config);

    const GDBTargetConf currentTarget() const;
    bool takeFocusAlways() const;
    bool showIOTab() const;

Q_SIGNALS:
    void showIO(bool show);

    void configChanged();

private Q_SLOTS:
    void slotTargetEdited(const QString &newText);
    void slotTargetSelected(int index);
    void slotAddTarget();
    void slotCopyTarget();
    void slotDeleteTarget();
    void slotAdvancedClicked();
    void slotBrowseExec();
    void slotBrowseDir();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void saveCurrentToIndex(int index);
    void loadFromIndex(int index);
    void setAdvancedOptions();

private:
    KTextEditor::MainWindow *m_mainWindow;
    QComboBox *m_targetCombo;
    int m_currentTarget = 0;
    QToolButton *m_addTarget;
    QToolButton *m_copyTarget;
    QToolButton *m_deleteTarget;
    QFrame *m_line;

    QLineEdit *m_executable;
    QToolButton *m_browseExe;

    QLineEdit *m_workingDirectory;
    QToolButton *m_browseDir;

    QLineEdit *m_arguments;

    QCheckBox *m_takeFocus;
    QCheckBox *m_redirectTerminal;
    QPushButton *m_advancedSettings;
    QBoxLayout *m_checBoxLayout;

    bool m_useBottomLayout;
    QLabel *m_execLabel;
    QLabel *m_workDirLabel;
    QLabel *m_argumentsLabel;
    KSelectAction *m_targetSelectAction = nullptr;

    AdvancedGDBSettings *m_advanced;
};

#endif
