/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectinfoviewcodeanalysis.h"
#include "kateproject.h"
#include "kateprojectcodeanalysistool.h"
#include "kateprojectpluginview.h"
#include "tools/kateprojectcodeanalysisselector.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QToolTip>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KMessageWidget>
#include <QTimer>

#include <KTextEditor/MainWindow>

KateProjectInfoViewCodeAnalysis::KateProjectInfoViewCodeAnalysis(KateProjectPluginView *pluginView, KateProject *project)
    : m_pluginView(pluginView)
    , m_project(project)
    , m_messageWidget(nullptr)
    , m_startStopAnalysis(new QPushButton(i18n("Start Analysis...")))
    , m_treeView(new QTreeView(this))
    , m_model(new QStandardItemModel(m_treeView))
    , m_analyzer(nullptr)
    , m_analysisTool(nullptr)
    , m_toolSelector(new QComboBox())
{
    /**
     * default style
     */
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeView->setUniformRowHeights(true);
    m_treeView->setRootIsDecorated(false);
    m_model->setHorizontalHeaderLabels(QStringList() << i18n("File") << i18n("Line") << i18n("Severity") << i18n("Message"));

    /**
     * attach model
     * kill selection model
     */
    QItemSelectionModel *m = m_treeView->selectionModel();
    m_treeView->setModel(m_model);
    delete m;

    m_treeView->setSortingEnabled(true);
    m_treeView->sortByColumn(1, Qt::AscendingOrder);
    m_treeView->sortByColumn(2, Qt::AscendingOrder);

    /**
     * Connect selection change callback
     * and attach model to code analysis selector
     */
    connect(m_toolSelector,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this,
            &KateProjectInfoViewCodeAnalysis::slotToolSelectionChanged);
    m_toolSelector->setModel(KateProjectCodeAnalysisSelector::model(this));

    /**
     * layout widget
     */
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    // top: selector and buttons...
    QHBoxLayout *hlayout = new QHBoxLayout;
    layout->addLayout(hlayout);
    hlayout->setSpacing(0);
    hlayout->addWidget(m_toolSelector);
    auto infoButton = new QPushButton(QIcon::fromTheme(QStringLiteral("documentinfo")), QString(), this);
    infoButton->setFocusPolicy(Qt::FocusPolicy::TabFocus);
    connect(infoButton, &QPushButton::clicked, this, [this]() {
        QToolTip::showText(QCursor::pos(), m_toolInfoText);
    });
    hlayout->addWidget(infoButton);
    hlayout->addWidget(m_startStopAnalysis);
    hlayout->addStretch();
    // below: result list...
    layout->addWidget(m_treeView);
    setLayout(layout);

    /**
     * connect needed signals
     */
    connect(m_startStopAnalysis, &QPushButton::clicked, this, &KateProjectInfoViewCodeAnalysis::slotStartStopClicked);
    connect(m_treeView, &QTreeView::clicked, this, &KateProjectInfoViewCodeAnalysis::slotClicked);
}

KateProjectInfoViewCodeAnalysis::~KateProjectInfoViewCodeAnalysis()
{
    if (m_analyzer && m_analyzer->state() != QProcess::NotRunning) {
        m_analyzer->kill();
        m_analyzer->blockSignals(true);
        m_analyzer->waitForFinished();
    }
    delete m_analyzer;
}

void KateProjectInfoViewCodeAnalysis::slotToolSelectionChanged(int)
{
    m_analysisTool = m_toolSelector->currentData(Qt::UserRole + 1).value<KateProjectCodeAnalysisTool *>();
    m_toolInfoText = i18n("%1<br/><br/>The tool will be run on all project files which match this list of file extensions:<br/><br/><b>%2</b>",
                          m_analysisTool->description(),
                          m_analysisTool->fileExtensions());
}

void KateProjectInfoViewCodeAnalysis::slotStartStopClicked()
{
    /**
     * get files for the external tool
     */
    m_analysisTool = m_toolSelector->currentData(Qt::UserRole + 1).value<KateProjectCodeAnalysisTool *>();
    m_analysisTool->setProject(m_project);
    m_analysisTool->setMainWindow(m_pluginView->mainWindow());

    /**
     * clear existing entries
     */
    m_model->removeRows(0, m_model->rowCount(), QModelIndex());

    /**
     * launch selected tool
     */
    delete m_analyzer;
    m_analyzer = new QProcess;
    m_analyzer->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_analyzer, &QProcess::readyRead, this, &KateProjectInfoViewCodeAnalysis::slotReadyRead);
    connect(m_analyzer, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &KateProjectInfoViewCodeAnalysis::finished);

    m_analyzer->start(m_analysisTool->path(), m_analysisTool->arguments());

    if (m_messageWidget) {
        delete m_messageWidget;
        m_messageWidget = nullptr;
    }

    if (!m_analyzer->waitForStarted()) {
        m_messageWidget = new KMessageWidget(this);
        m_messageWidget->setCloseButtonVisible(true);
        m_messageWidget->setMessageType(KMessageWidget::Warning);
        m_messageWidget->setWordWrap(false);
        m_messageWidget->setText(m_analysisTool->notInstalledMessage());
        static_cast<QVBoxLayout *>(layout())->addWidget(m_messageWidget);
        m_messageWidget->animatedShow();
        return;
    }

    m_startStopAnalysis->setEnabled(false);

    /**
     * write files list and close write channel
     */
    const QString stdinMessage = m_analysisTool->stdinMessages();
    if (!stdinMessage.isEmpty()) {
        m_analyzer->write(stdinMessage.toLocal8Bit());
    }
    m_analyzer->closeWriteChannel();
}

void KateProjectInfoViewCodeAnalysis::slotReadyRead()
{
    /**
     * get results of analysis
     */
    while (m_analyzer->canReadLine()) {
        /**
         * get one line, split it, skip it, if too few elements
         */
        QString line = QString::fromLocal8Bit(m_analyzer->readLine());
        QStringList elements = m_analysisTool->parseLine(line);
        if (elements.size() < 4) {
            continue;
        }

        /**
         * feed into model
         */
        QList<QStandardItem *> items;
        QStandardItem *fileNameItem = new QStandardItem(QFileInfo(elements[0]).fileName());
        fileNameItem->setToolTip(elements[0]);
        items << fileNameItem;
        items << new QStandardItem(elements[1]);
        items << new QStandardItem(elements[2]);
        const auto message = elements[3].simplified();
        auto messageItem = new QStandardItem(message);
        messageItem->setToolTip(message);
        items << messageItem;
        m_model->appendRow(items);
    }

    /**
     * tree view polish ;)
     */
    m_treeView->resizeColumnToContents(2);
    m_treeView->resizeColumnToContents(1);
    m_treeView->resizeColumnToContents(0);
}

void KateProjectInfoViewCodeAnalysis::slotClicked(const QModelIndex &index)
{
    /**
     * get path
     */
    QString filePath = m_model->item(index.row(), 0)->toolTip();
    if (filePath.isEmpty()) {
        return;
    }

    /**
     * create view
     */
    KTextEditor::View *view = m_pluginView->mainWindow()->openUrl(QUrl::fromLocalFile(filePath));
    if (!view) {
        return;
    }

    /**
     * set cursor, if possible
     */
    int line = m_model->item(index.row(), 1)->text().toInt();
    if (line >= 1) {
        view->setCursorPosition(KTextEditor::Cursor(line - 1, 0));
    }
}

void KateProjectInfoViewCodeAnalysis::finished(int exitCode, QProcess::ExitStatus)
{
    m_startStopAnalysis->setEnabled(true);
    m_messageWidget = new KMessageWidget(this);
    m_messageWidget->setCloseButtonVisible(true);
    m_messageWidget->setWordWrap(false);

    if (m_analysisTool->isSuccessfulExitCode(exitCode)) {
        // normally 0 is successful but there are exceptions
        m_messageWidget->setMessageType(KMessageWidget::Information);
        m_messageWidget->setText(i18np("Analysis on %1 file finished.", "Analysis on %1 files finished.", m_analysisTool->getActualFilesCount()));

        // hide after 3 seconds
        QTimer::singleShot(3000, this, [this]() {
            if (m_messageWidget) {
                m_messageWidget->animatedHide();
            }
        });
    } else {
        // unfortunately, output was eaten by slotReadyRead()
        // TODO: get stderr output, show it here
        m_messageWidget->setMessageType(KMessageWidget::Warning);
        m_messageWidget->setText(i18np("Analysis on %1 file failed with exit code %2.",
                                       "Analysis on %1 files failed with exit code %2.",
                                       m_analysisTool->getActualFilesCount(),
                                       exitCode));
    }

    static_cast<QVBoxLayout *>(layout())->addWidget(m_messageWidget);
    m_messageWidget->animatedShow();
}
