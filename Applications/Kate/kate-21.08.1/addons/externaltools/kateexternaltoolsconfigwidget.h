/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KTEXTEDITOR_EXTERNALTOOLS_CONFIGWIDGET_H
#define KTEXTEDITOR_EXTERNALTOOLS_CONFIGWIDGET_H

#include "ui_configwidget.h"
#include "ui_tooldialog.h"

#include <KTextEditor/Application>
#include <KTextEditor/ConfigPage>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>

#include <QDialog>
#include <QPixmap>
#include <QStandardItemModel>

class KConfig;
class KateExternalToolsPlugin;
class KateExternalTool;

/**
 * The config widget.
 * The config widget allows the user to view a list of services of the type
 * "Kate/ExternalTool" and add, remove or edit them.
 */
class KateExternalToolsConfigWidget : public KTextEditor::ConfigPage, public Ui::ExternalToolsConfigWidget
{
    Q_OBJECT
public:
    KateExternalToolsConfigWidget(QWidget *parent, KateExternalToolsPlugin *plugin);
    virtual ~KateExternalToolsConfigWidget();

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public Q_SLOTS:
    void apply() override;
    void reset() override;
    void defaults() override
    {
        reset();
    }

private Q_SLOTS:
    void addNewTool(KateExternalTool *tool);
    void lazyInitDefaultsMenu(QMenu *defaultsMenu);
    void slotAddDefaultTool(int defaultToolsIndex);
    void slotAddCategory();
    void slotAddTool();
    void slotEdit();
    void slotRemove();
    void slotSelectionChanged();

    /**
     * Helper to open the ToolDialog.
     * Returns true, if the user clicked OK.
     */
    bool editTool(KateExternalTool *tool);

    /**
     * Creates a new category or returns existing one.
     */
    QStandardItem *addCategory(const QString &translatedCategory);

    /**
     * Returns the currently active category. The returned pointer is always valid.
     */
    QStandardItem *currentCategory() const;

    /**
     * Clears the tools model.
     */
    void clearTools();

private:
    KConfig *m_config = nullptr;
    bool m_changed = false;
    KateExternalToolsPlugin *m_plugin;
    QStandardItemModel m_toolsModel;
    QStandardItem *m_noCategory = nullptr;
};

/**
 * A Dialog to edit a single KateExternalTool object
 */
class KateExternalToolServiceEditor : public QDialog
{
    Q_OBJECT

public:
    explicit KateExternalToolServiceEditor(KateExternalTool *tool, KateExternalToolsPlugin *plugin, QWidget *parent = nullptr);

private Q_SLOTS:
    /**
     * Run when the OK button is clicked, to ensure critical values are provided.
     */
    void slotOKClicked();

    /**
     * show a mimetype chooser dialog
     */
    void showMTDlg();

public:
    Ui::ToolDialog ui;

private:
    KateExternalToolsPlugin *m_plugin;
    KateExternalTool *m_tool;
};

#endif // KTEXTEDITOR_EXTERNALTOOLS_CONFIGWIDGET_H

// kate: space-indent on; indent-width 4; replace-tabs on;
