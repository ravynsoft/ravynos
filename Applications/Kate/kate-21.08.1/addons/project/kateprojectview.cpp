/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectview.h"
#include "branchcheckoutdialog.h"
#include "filehistorywidget.h"
#include "git/gitutils.h"
#include "gitwidget.h"
#include "kateprojectfiltermodel.h"
#include "kateprojectpluginview.h"

#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>

#include <KActionCollection>
#include <KLineEdit>
#include <KLocalizedString>

#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QVBoxLayout>

KateProjectView::KateProjectView(KateProjectPluginView *pluginView, KateProject *project, KTextEditor::MainWindow *mainWindow)
    : m_pluginView(pluginView)
    , m_project(project)
    , m_treeView(new KateProjectViewTree(pluginView, project))
    , m_stackWidget(new QStackedWidget(this))
    , m_filter(new KLineEdit())
    , m_branchBtn(new QToolButton)
{
    /**
     * layout tree view and co.
     */
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_branchBtn);
    layout->addWidget(m_stackWidget);
    layout->addWidget(m_filter);
    setLayout(layout);

    m_stackWidget->addWidget(m_treeView);

    m_branchBtn->setAutoRaise(true);
    m_branchBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_branchBtn->setSizePolicy(QSizePolicy::Minimum, m_branchBtn->sizePolicy().verticalPolicy());
    m_branchBtn->setIcon(QIcon(QStringLiteral(":/icons/icons/sc-apps-git.svg")));

    // let tree get focus for keyboard selection of file to open
    setFocusProxy(m_treeView);

    // add to actionCollection so that this is available in Kate Command bar
    auto chckbr = pluginView->actionCollection()->addAction(QStringLiteral("checkout_branch"), this, [this] {
        m_branchBtn->click();
    });
    chckbr->setText(i18n("Checkout Git Branch"));

    /**
     * setup filter line edit
     */
    m_filter->setPlaceholderText(i18n("Filter..."));
    m_filter->setClearButtonEnabled(true);
    connect(m_filter, &KLineEdit::textChanged, this, &KateProjectView::filterTextChanged);

    /**
     * Setup git checkout stuff
     */
    connect(m_branchBtn, &QPushButton::clicked, this, [this, mainWindow] {
        BranchCheckoutDialog bd(mainWindow->window(), m_pluginView, m_project->baseDir());
        bd.openDialog();
    });

    checkAndRefreshGit();

    connect(m_project, &KateProject::modelChanged, this, &KateProjectView::checkAndRefreshGit);
    connect(&m_branchChangedWatcher, &QFileSystemWatcher::fileChanged, this, [this] {
        m_project->reload(true);
    });

    // file history
    connect(m_treeView, &KateProjectViewTree::showFileHistory, this, &KateProjectView::showFileGitHistory);
}

KateProjectView::~KateProjectView()
{
}

void KateProjectView::selectFile(const QString &file)
{
    m_treeView->selectFile(file);
}

void KateProjectView::openSelectedDocument()
{
    m_treeView->openSelectedDocument();
}

void KateProjectView::filterTextChanged(const QString &filterText)
{
    /**
     * filter
     */
    static_cast<KateProjectFilterProxyModel *>(m_treeView->model())->setFilterString(filterText);

    /**
     * expand
     */
    if (!filterText.isEmpty()) {
        QTimer::singleShot(100, m_treeView, &QTreeView::expandAll);
    }
}

void KateProjectView::setTreeViewAsCurrent()
{
    Q_ASSERT(m_treeView != m_stackWidget->currentWidget());

    auto currentFileHistory = m_stackWidget->currentWidget();
    m_stackWidget->removeWidget(currentFileHistory);
    delete currentFileHistory;

    m_stackWidget->setCurrentWidget(m_treeView);
}

void KateProjectView::showFileGitHistory(const QString &file)
{
    // create on demand and on switch back delete
    auto fhs = new FileHistoryWidget(file);
    connect(fhs, &FileHistoryWidget::backClicked, this, &KateProjectView::setTreeViewAsCurrent);
    connect(fhs, &FileHistoryWidget::commitClicked, this, [this](const QByteArray &diff) {
        m_pluginView->showDiffInFixedView(diff);
    });
    connect(fhs, &FileHistoryWidget::errorMessage, m_pluginView, [this](const QString &s, bool warn) {
        QVariantMap genericMessage;
        genericMessage.insert(QStringLiteral("type"), warn ? QStringLiteral("Error") : QStringLiteral("Info"));
        genericMessage.insert(QStringLiteral("category"), i18n("Git"));
        genericMessage.insert(QStringLiteral("categoryIcon"), QIcon(QStringLiteral(":/icons/icons/sc-apps-git.svg")));
        genericMessage.insert(QStringLiteral("text"), s);
        Q_EMIT m_pluginView->message(genericMessage);
    });
    m_stackWidget->addWidget(fhs);
    m_stackWidget->setCurrentWidget(fhs);
}

void KateProjectView::checkAndRefreshGit()
{
    const auto dotGitPath = GitUtils::getDotGitPath(m_project->baseDir());
    /**
     * Not in a git repo or git was removed
     */
    if (!dotGitPath.has_value()) {
        if (!m_branchChangedWatcher.files().isEmpty()) {
            m_branchChangedWatcher.removePaths(m_branchChangedWatcher.files());
        }
        m_branchBtn->setHidden(true);
    } else {
        m_branchBtn->setHidden(false);
        m_branchBtn->setText(GitUtils::getCurrentBranchName(dotGitPath.value()));
        if (m_branchChangedWatcher.files().isEmpty()) {
            m_branchChangedWatcher.addPath(dotGitPath.value() + QStringLiteral(".git/HEAD"));
        }
    }
}
