/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KATE_PROJECT_INFO_VIEW_CODE_ANALYSIS_H
#define KATE_PROJECT_INFO_VIEW_CODE_ANALYSIS_H

#include <QComboBox>
#include <QLabel>
#include <QPointer>
#include <QProcess>
#include <QPushButton>
#include <QTreeView>
#include <QWidget>

class KateProjectPluginView;
class KateProjectCodeAnalysisTool;
class KMessageWidget;
class KateProject;
class QStandardItemModel;

/**
 * View for Code Analysis.
 * cppcheck and perhaps later more...
 */
class KateProjectInfoViewCodeAnalysis : public QWidget
{
    Q_OBJECT

public:
    /**
     * construct project info view for given project
     * @param pluginView our plugin view
     * @param project project this view is for
     */
    KateProjectInfoViewCodeAnalysis(KateProjectPluginView *pluginView, KateProject *project);

    /**
     * deconstruct info view
     */
    ~KateProjectInfoViewCodeAnalysis() override;

    /**
     * our project.
     * @return project
     */
    KateProject *project() const
    {
        return m_project;
    }

private Q_SLOTS:
    /**
     * Called if the tool is changed (currently via Combobox)
     */
    void slotToolSelectionChanged(int);

    /**
     * Called if start/stop button is clicked.
     */
    void slotStartStopClicked();

    /**
     * More checker output is available
     */
    void slotReadyRead();

    /**
     * item got clicked, do stuff, like open document
     * @param index model index of clicked item
     */
    void slotClicked(const QModelIndex &index);

    /**
     * Analysis finished
     * @param exitCode analyzer process exit code
     * @param exitStatus analyzer process exit status
     */
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    /**
     * our plugin view
     */
    KateProjectPluginView *m_pluginView;

    /**
     * our project
     */
    KateProject *m_project;

    /**
     * information widget showing a warning about missing ctags.
     */
    QPointer<KMessageWidget> m_messageWidget;

    /**
     * start/stop analysis button
     */
    QPushButton *m_startStopAnalysis;

    /**
     * tree view for results
     */
    QTreeView *m_treeView;

    /**
     * standard item model for results
     */
    QStandardItemModel *m_model;

    /**
     * running analyzer process
     */
    QProcess *m_analyzer;

    /**
     * currently selected tool
     */
    KateProjectCodeAnalysisTool *m_analysisTool;

    /**
     * UI element to select the tool
     */
    QComboBox *m_toolSelector;

    /**
     * contains a rich text to explain what the current tool does
     */
    QString m_toolInfoText;
};

#endif
