/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectinfoviewindex.h"
#include "kateproject.h"
#include "kateprojectplugin.h"
#include "kateprojectpluginview.h"

#include <KLocalizedString>
#include <KMessageWidget>
#include <QAction>
#include <QVBoxLayout>

#include <KTextEditor/MainWindow>

KateProjectInfoViewIndex::KateProjectInfoViewIndex(KateProjectPluginView *pluginView, KateProject *project, QWidget *parent)
    : QWidget(parent)
    , m_pluginView(pluginView)
    , m_project(project)
    , m_messageWidget(nullptr)
    , m_lineEdit(new QLineEdit())
    , m_treeView(new QTreeView())
    , m_model(new QStandardItemModel(m_treeView))
{
    /**
     * default style
     */
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeView->setUniformRowHeights(true);
    m_treeView->setRootIsDecorated(false);
    m_model->setHorizontalHeaderLabels(QStringList() << i18n("Name") << i18n("Kind") << i18n("File") << i18n("Line"));
    m_lineEdit->setPlaceholderText(i18n("Search"));
    m_lineEdit->setClearButtonEnabled(true);

    /**
     * attach model
     * kill selection model
     */
    QItemSelectionModel *m = m_treeView->selectionModel();
    m_treeView->setModel(m_model);
    delete m;

    /**
     * layout widget
     */
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->addWidget(m_lineEdit);
    layout->addWidget(m_treeView);
    setLayout(layout);
    setFocusProxy(m_lineEdit);

    /**
     * connect needed signals
     */
    connect(m_pluginView, &KateProjectPluginView::projectLookupWord, m_lineEdit, &QLineEdit::setText);
    connect(m_lineEdit, &QLineEdit::textChanged, this, &KateProjectInfoViewIndex::slotTextChanged);
    connect(m_treeView, &QTreeView::clicked, this, &KateProjectInfoViewIndex::slotClicked);
    if (m_project) {
        connect(m_project, &KateProject::indexChanged, this, &KateProjectInfoViewIndex::indexAvailable);
    } else {
        connect(m_pluginView, &KateProjectPluginView::gotoSymbol, this, &KateProjectInfoViewIndex::slotGotoSymbol);
        enableWidgets(true);
    }

    /**
     * trigger once search with nothing
     */
    slotTextChanged(QString());
}

KateProjectInfoViewIndex::~KateProjectInfoViewIndex()
{
}

void KateProjectInfoViewIndex::slotGotoSymbol(const QString &text, int &results)
{
    // trigger fill model
    m_lineEdit->setText(text);
    results = m_model->rowCount();
    // immediately goto if only a single option
    if (results == 1) {
        slotClicked(m_model->index(0, 0));
    }
}

void KateProjectInfoViewIndex::slotTextChanged(const QString &text)
{
    /**
     * init
     */
    m_treeView->setSortingEnabled(false);
    m_model->setRowCount(0);

    /**
     * get results
     */
    if (m_project && m_project->projectIndex() && !text.isEmpty()) {
        m_project->projectIndex()->findMatches(*m_model, text, KateProjectIndex::FindMatches);
    } else if (!text.isEmpty()) {
        for (const auto &project : m_pluginView->plugin()->projects()) {
            if (project->projectIndex()) {
                project->projectIndex()->findMatches(*m_model, text, KateProjectIndex::FindMatches, TAG_FULLMATCH | TAG_OBSERVECASE);
            }
        }
    }

    /**
     * tree view polish ;)
     */
    m_treeView->setSortingEnabled(true);
    m_treeView->resizeColumnToContents(2);
    m_treeView->resizeColumnToContents(1);
    m_treeView->resizeColumnToContents(0);
}

void KateProjectInfoViewIndex::slotClicked(const QModelIndex &index)
{
    /**
     * get path
     */
    QString filePath = m_model->item(index.row(), 2)->text();
    if (filePath.isEmpty()) {
        return;
    }

    /** Save current position before jumping **/
    auto currentView = m_pluginView->mainWindow()->activeView();
    QUrl url;
    KTextEditor::Cursor pos;
    if (currentView) {
        url = currentView->document()->url();
        pos = currentView->cursorPosition();
    }

    /**
     * create view
     */
    KTextEditor::View *view = m_pluginView->mainWindow()->openUrl(QUrl::fromLocalFile(filePath));
    if (!view) {
        return;
    }

    /** save current position in location history */
    Q_EMIT m_pluginView->addPositionToHistory(url, pos);

    /**
     * set cursor, if possible
     */
    int line = m_model->item(index.row(), 3)->text().toInt();
    if (line >= 1) {
        view->setCursorPosition(KTextEditor::Cursor(line - 1, 0));

        /** save the jump position in location history */
        Q_EMIT m_pluginView->addPositionToHistory(view->document()->url(), {line - 1, 0});
    }
}

void KateProjectInfoViewIndex::indexAvailable()
{
    const bool valid = m_project->projectIndex() && m_project->projectIndex()->isValid();
    enableWidgets(valid);
}

void KateProjectInfoViewIndex::enableWidgets(bool valid)
{
    /**
     * update enabled state of widgets
     */
    m_lineEdit->setEnabled(valid);
    m_treeView->setEnabled(valid);

    /**
     * if index exists, hide possible message widget, else create it
     */
    if (valid) {
        if (m_messageWidget && m_messageWidget->isVisible()) {
            m_messageWidget->animatedHide();
        }
    } else if (!m_messageWidget) {
        m_messageWidget = new KMessageWidget();
        m_messageWidget->setCloseButtonVisible(true);
        m_messageWidget->setMessageType(KMessageWidget::Warning);
        m_messageWidget->setWordWrap(false);
        static_cast<QVBoxLayout *>(layout())->insertWidget(0, m_messageWidget);
        m_messageWidget->animatedShow();
    }

    // disabled or failed to create?
    if (!valid && m_project->projectIndex()) {
        m_messageWidget->setText(i18n("The index could not be created. Please install 'ctags'."));
        // Make sure we remove the action
        const QList<QAction *> actions = m_messageWidget->actions();
        if (actions.size() == 1) {
            m_messageWidget->removeAction(actions.first());
        }
    } else if (!valid) {
        if (!m_messageWidget->text().isEmpty()) {
            return;
        }
        m_messageWidget->setText(i18n("Indexing is not enabled"));
        auto enableIndexing = new QAction(i18n("Enable indexing"), m_messageWidget);
        connect(enableIndexing, &QAction::triggered, m_messageWidget, [this]() {
            m_project->plugin()->setIndex(true, QUrl());
            m_project->reload(true);
        });
        m_messageWidget->addAction(enableIndexing);
    }
}
