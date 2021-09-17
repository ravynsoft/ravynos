/*
    SPDX-FileCopyrightText: 2021 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateoutputview.h"
#include "kateapp.h"
#include "katemainwindow.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <KTextEditor/ConfigInterface>
#include <KTextEditor/Editor>

#include <QClipboard>
#include <QDateTime>
#include <QGuiApplication>
#include <QMenu>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

#include <kfts_fuzzy_match.h>

class KateOutputTreeView : public QTreeView
{
public:
    KateOutputTreeView(QWidget *parent)
        : QTreeView(parent)
    {
        // copy action, default off, is enabled on selection!
        m_copyAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18nc("@action:inmenu", "Copy to Clipboard"), this);
        connect(m_copyAction, &QAction::triggered, this, &KateOutputTreeView::slotCopySelected);
        m_copyAction->setEnabled(false);
    }

    // we want no branches!
    void drawBranches(QPainter *, const QRect &, const QModelIndex &) const override
    {
    }

    // activate copy action based on selection
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override
    {
        QTreeView::selectionChanged(selected, deselected);
        m_copyAction->setEnabled(!selected.indexes().isEmpty());
    }

    // provide simple context menu, e.g. for copy&paste
    void contextMenuEvent(QContextMenuEvent *event) override
    {
        QMenu menu;
        menu.addAction(m_copyAction);
        menu.exec(viewport()->mapToGlobal(event->pos()));
        event->accept();
    }

    // access to copy action for outside tool buttons etc.
    QAction *copyAction()
    {
        return m_copyAction;
    }

private Q_SLOTS:
    void slotCopySelected()
    {
        // collect the stuff
        QString clipboardText;
        int row = -1;
        for (const auto selected : selectedIndexes()) {
            // we want to separate columns by " " and rows by "\n"

            // first element: just append + remember row
            if (row == -1) {
                clipboardText += selected.data().toString();
                row = selected.row();
                continue;
            }

            // same line, space separated
            if (row == selected.row()) {
                clipboardText += QLatin1Char(' ') + selected.data().toString();
                continue;
            }

            // new line, add \n
            if (row != selected.row()) {
                clipboardText += QLatin1Char('\n') + selected.data().toString();
                row = selected.row();
                continue;
            }
        }
        if (!clipboardText.isEmpty()) {
            QGuiApplication::clipboard()->setText(clipboardText);
        }
    }

private:
    /**
     * action to copy current selection to clipboard
     */
    QAction *m_copyAction = nullptr;
};

class OutputSortFilterProxyModel final : public QSortFilterProxyModel
{
public:
    OutputSortFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
    }

    void setFilterString(const QString &string)
    {
        beginResetModel();
        m_pattern = string;
        endResetModel();
    }

protected:
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override
    {
        const int l = sourceLeft.data(WeightRole).toInt();
        const int r = sourceRight.data(WeightRole).toInt();
        return l < r;
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (m_pattern.isEmpty()) {
            return true;
        }

        const auto idxCat = sourceModel()->index(sourceRow, 1, sourceParent);
        const auto idxType = sourceModel()->index(sourceRow, 2, sourceParent);
        const auto idxBody = sourceModel()->index(sourceRow, 3, sourceParent);

        const QString cat = idxCat.data().toString();
        const QString type = idxType.data().toString();
        const QString body = idxBody.data().toString();

        int scorec = 0;
        int scoret = 0;
        const bool resc = kfts::fuzzy_match(m_pattern, cat, scorec);
        const bool rest = kfts::fuzzy_match(m_pattern, type, scoret);
        const bool resb = body.contains(m_pattern, Qt::CaseInsensitive);

        const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
        sourceModel()->setData(idx, scorec + scoret, WeightRole);
        return resc || rest || resb;
    }

private:
    QString m_pattern;
    static constexpr int WeightRole = Qt::UserRole + 1;
};

KateOutputView::KateOutputView(KateMainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_mainWindow(mainWindow)
{
    m_proxyModel = new OutputSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(&m_messagesModel);
    m_proxyModel->setRecursiveFilteringEnabled(true);

    // simple vbox layout with just the tree view ATM
    // TODO: e.g. filter and such!
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_messagesTreeView = new KateOutputTreeView(this);
    m_messagesTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_messagesTreeView->setHeaderHidden(true);
    m_messagesTreeView->setRootIsDecorated(false);
    m_messagesTreeView->setUniformRowHeights(true);
    m_messagesTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_messagesTreeView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    m_messagesTreeView->setModel(m_proxyModel);
    m_messagesTreeView->setIndentation(0);

    // filter line edit
    m_filterLine.installEventFilter(this);
    m_filterLine.setPlaceholderText(i18n("Type to filter..."));
    connect(&m_filterLine, &QLineEdit::textChanged, this, [this](const QString &text) {
        static_cast<OutputSortFilterProxyModel *>(m_proxyModel)->setFilterString(text);
        m_messagesTreeView->expandAll();
    });

    // copy button
    auto copy = new QToolButton(this);
    copy->setDefaultAction(m_messagesTreeView->copyAction());

    // clear button
    auto clear = new QToolButton(this);
    clear->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-history")));
    clear->setToolTip(i18n("Clear all messages"));
    connect(clear, &QPushButton::clicked, this, [this] {
        m_messagesModel.clear();
    });

    // setup top horizontal layout
    // tried toolbar, has bad spacing
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(&m_filterLine);
    hLayout->addWidget(copy);
    hLayout->addWidget(clear);
    hLayout->setStretch(0, 1);

    // tree view
    layout->addLayout(hLayout);
    layout->addWidget(m_messagesTreeView);
    // read config once
    readConfig();

    // handle config changes
    connect(KateApp::self(), &KateApp::configurationChanged, this, &KateOutputView::readConfig);
    connect(KTextEditor::Editor::instance(), &KTextEditor::Editor::configChanged, this, &KateOutputView::readConfig);

    // needed to have some view, can be removed if Editor::font is there
    QTimer::singleShot(0, this, &KateOutputView::readConfig);
}

void KateOutputView::readConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup cgGeneral = KConfigGroup(config, "General");
    m_showOutputViewForMessageType = cgGeneral.readEntry("Show output view for message type", 1);

    // use editor fonts
    const auto theme = KTextEditor::Editor::instance()->theme();
    auto pal = m_messagesTreeView->palette();
    pal.setColor(QPalette::Base, QColor::fromRgba(theme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor)));
    pal.setColor(QPalette::Highlight, QColor::fromRgba(theme.editorColor(KSyntaxHighlighting::Theme::TextSelection)));
    pal.setColor(QPalette::Text, QColor::fromRgba(theme.textColor(KSyntaxHighlighting::Theme::Normal)));
    m_messagesTreeView->setPalette(pal);

    // remove later in favor or Editor::font
    QFont font;
    if (const auto ciface = qobject_cast<KTextEditor::ConfigInterface *>(m_mainWindow->activeView())) {
        font = ciface->configValue(QStringLiteral("font")).value<QFont>();
    } else {
        font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    }
    m_messagesTreeView->setFont(font);
}

void KateOutputView::slotMessage(const QVariantMap &message)
{
    /**
     * discard all messages without any real text
     */
    const auto text = message.value(QStringLiteral("text")).toString().trimmed();
    if (text.isEmpty()) {
        return;
    }

    /**
     * date time column: we want to know when a message arrived
     * TODO: perhaps store full date time for more stuff later
     */
    auto dateTimeColumn = new QStandardItem();
    const QDateTime current = QDateTime::currentDateTime();
    dateTimeColumn->setText(current.time().toString(Qt::TextDate));

    /**
     * category
     * provided by sender to better categorize the output into stuff like: lsp, git, ...
     * optional icon support
     */
    auto categoryColumn = new QStandardItem();
    categoryColumn->setText(message.value(QStringLiteral("category")).toString().trimmed());
    const auto categoryIcon = message.value(QStringLiteral("categoryIcon")).value<QIcon>();
    if (categoryIcon.isNull()) {
        categoryColumn->setIcon(QIcon::fromTheme(QStringLiteral("dialog-scripts")));
    } else {
        categoryColumn->setIcon(categoryIcon);
    }

    /**
     * type column: shows the type, icons for some types only
     */
    bool shouldShowOutputToolView = false;
    auto typeColumn = new QStandardItem();
    const auto typeString = message.value(QStringLiteral("type")).toString();
    if (typeString == QLatin1String("Error")) {
        shouldShowOutputToolView = (m_showOutputViewForMessageType >= 1);
        typeColumn->setText(i18nc("@info", "Error"));
        typeColumn->setIcon(QIcon::fromTheme(QStringLiteral("data-error")));
    } else if (typeString == QLatin1String("Warning")) {
        shouldShowOutputToolView = (m_showOutputViewForMessageType >= 2);
        typeColumn->setText(i18nc("@info", "Warning"));
        typeColumn->setIcon(QIcon::fromTheme(QStringLiteral("data-warning")));
    } else if (typeString == QLatin1String("Info")) {
        shouldShowOutputToolView = (m_showOutputViewForMessageType >= 3);
        typeColumn->setText(i18nc("@info", "Info"));
        typeColumn->setIcon(QIcon::fromTheme(QStringLiteral("data-information")));
    } else {
        shouldShowOutputToolView = (m_showOutputViewForMessageType >= 4);
        typeColumn->setText(i18nc("@info", "Log"));
        typeColumn->setIcon(QIcon::fromTheme(QStringLiteral("dialog-messages")));
    }

    /**
     * body column, plain text
     * we ensured above that we have some
     * split it into lines, we want nice fixed-height parts
     * we will add extra rows for everything below the first line
     */
    const auto textLines = text.split(QLatin1Char('\n'));
    Q_ASSERT(!textLines.empty());
    auto bodyColumn = new QStandardItem(textLines.at(0));
    auto lastItemForScrolling = bodyColumn;
    for (int i = 1; i < textLines.size(); ++i) {
        lastItemForScrolling = new QStandardItem(textLines.at(i));
        dateTimeColumn->appendRow({new QStandardItem(), new QStandardItem(), new QStandardItem(), lastItemForScrolling});
    }

    /**
     * add new message to model
     */
    m_messagesModel.appendRow({dateTimeColumn, categoryColumn, typeColumn, bodyColumn});

    /**
     * expand the new thingy
     */
    m_messagesTreeView->expand(m_proxyModel->mapFromSource(dateTimeColumn->index()));

    /**
     * ensure correct sizing
     * OPTIMIZE: we can do that only if e.g. a first time a new type/category pops up
     */
    m_messagesTreeView->resizeColumnToContents(0);
    m_messagesTreeView->resizeColumnToContents(1);
    m_messagesTreeView->resizeColumnToContents(2);

    /**
     * ensure last item is visible
     */
    m_messagesTreeView->scrollTo(m_proxyModel->mapFromSource(lastItemForScrolling->index()));

    /**
     * if message requires it => show the tool view if hidden
     */
    if (shouldShowOutputToolView) {
        m_mainWindow->showToolView(parentWidget());
    }
}
