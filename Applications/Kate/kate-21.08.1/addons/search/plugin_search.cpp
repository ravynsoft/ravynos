/*
    SPDX-FileCopyrightText: 2011-21 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plugin_search.h"
#include "KateSearchCommand.h"
#include "htmldelegate.h"

#include <ktexteditor/configinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/movinginterface.h>
#include <ktexteditor/movingrange.h>
#include <ktexteditor/view.h>

#include "kacceleratormanager.h"
#include <KAboutData>
#include <KActionCollection>
#include <KColorScheme>
#include <KLineEdit>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KUrlCompletion>

#include <KConfigGroup>
#include <KXMLGUIFactory>

#include <QClipboard>
#include <QComboBox>
#include <QCompleter>
#include <QDir>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMenu>
#include <QMetaObject>
#include <QPoint>
#include <QScrollBar>
#include <QTextDocument>

static QUrl localFileDirUp(const QUrl &url)
{
    if (!url.isLocalFile()) {
        return url;
    }

    // else go up
    return QUrl::fromLocalFile(QFileInfo(url.toLocalFile()).dir().absolutePath());
}

static QAction *
menuEntry(QMenu *menu, const QString &before, const QString &after, const QString &desc, QString menuBefore = QString(), QString menuAfter = QString());

/**
 * When the action is triggered the cursor will be placed between @p before and @p after.
 */
static QAction *menuEntry(QMenu *menu, const QString &before, const QString &after, const QString &desc, QString menuBefore, QString menuAfter)
{
    if (menuBefore.isEmpty()) {
        menuBefore = before;
    }
    if (menuAfter.isEmpty()) {
        menuAfter = after;
    }

    QAction *const action = menu->addAction(menuBefore + menuAfter + QLatin1Char('\t') + desc);
    if (!action) {
        return nullptr;
    }

    action->setData(QString(before + QLatin1Char(' ') + after));
    return action;
}

/**
 * adds items and separators for special chars in "replace" field
 */
static void addSpecialCharsHelperActionsForReplace(QSet<QAction *> *actionList, QMenu *menu)
{
    QSet<QAction *> &actionPointers = *actionList;
    QString emptyQSTring;

    actionPointers << menuEntry(menu, QStringLiteral("\\n"), emptyQSTring, i18n("Line break"));
    actionPointers << menuEntry(menu, QStringLiteral("\\t"), emptyQSTring, i18n("Tab"));
}

/**
 * adds items and separators for regex in "search" field
 */
static void addRegexHelperActionsForSearch(QSet<QAction *> *actionList, QMenu *menu)
{
    QSet<QAction *> &actionPointers = *actionList;
    QString emptyQSTring;

    actionPointers << menuEntry(menu, QStringLiteral("^"), emptyQSTring, i18n("Beginning of line"));
    actionPointers << menuEntry(menu, QStringLiteral("$"), emptyQSTring, i18n("End of line"));
    menu->addSeparator();
    actionPointers << menuEntry(menu, QStringLiteral("."), emptyQSTring, i18n("Any single character (excluding line breaks)"));
    actionPointers << menuEntry(menu, QStringLiteral("[.]"), emptyQSTring, i18n("Literal dot"));
    menu->addSeparator();
    actionPointers << menuEntry(menu, QStringLiteral("+"), emptyQSTring, i18n("One or more occurrences"));
    actionPointers << menuEntry(menu, QStringLiteral("*"), emptyQSTring, i18n("Zero or more occurrences"));
    actionPointers << menuEntry(menu, QStringLiteral("?"), emptyQSTring, i18n("Zero or one occurrences"));
    actionPointers
        << menuEntry(menu, QStringLiteral("{"), QStringLiteral(",}"), i18n("<a> through <b> occurrences"), QStringLiteral("{a"), QStringLiteral(",b}"));
    menu->addSeparator();
    actionPointers << menuEntry(menu, QStringLiteral("("), QStringLiteral(")"), i18n("Group, capturing"));
    actionPointers << menuEntry(menu, QStringLiteral("|"), emptyQSTring, i18n("Or"));
    actionPointers << menuEntry(menu, QStringLiteral("["), QStringLiteral("]"), i18n("Set of characters"));
    actionPointers << menuEntry(menu, QStringLiteral("[^"), QStringLiteral("]"), i18n("Negative set of characters"));
    actionPointers << menuEntry(menu, QStringLiteral("(?:"), QStringLiteral(")"), i18n("Group, non-capturing"), QStringLiteral("(?:E"));
    actionPointers << menuEntry(menu, QStringLiteral("(?="), QStringLiteral(")"), i18n("Lookahead"), QStringLiteral("(?=E"));
    actionPointers << menuEntry(menu, QStringLiteral("(?!"), QStringLiteral(")"), i18n("Negative lookahead"), QStringLiteral("(?!E"));

    menu->addSeparator();
    actionPointers << menuEntry(menu, QStringLiteral("\\n"), emptyQSTring, i18n("Line break"));
    actionPointers << menuEntry(menu, QStringLiteral("\\t"), emptyQSTring, i18n("Tab"));
    actionPointers << menuEntry(menu, QStringLiteral("\\b"), emptyQSTring, i18n("Word boundary"));
    actionPointers << menuEntry(menu, QStringLiteral("\\B"), emptyQSTring, i18n("Not word boundary"));
    actionPointers << menuEntry(menu, QStringLiteral("\\d"), emptyQSTring, i18n("Digit"));
    actionPointers << menuEntry(menu, QStringLiteral("\\D"), emptyQSTring, i18n("Non-digit"));
    actionPointers << menuEntry(menu, QStringLiteral("\\s"), emptyQSTring, i18n("Whitespace (excluding line breaks)"));
    actionPointers << menuEntry(menu, QStringLiteral("\\S"), emptyQSTring, i18n("Non-whitespace (excluding line breaks)"));
    actionPointers << menuEntry(menu, QStringLiteral("\\w"), emptyQSTring, i18n("Word character (alphanumerics plus '_')"));
    actionPointers << menuEntry(menu, QStringLiteral("\\W"), emptyQSTring, i18n("Non-word character"));
}

/**
 * adds items and separators for regex in "replace" field
 */
static void addRegexHelperActionsForReplace(QSet<QAction *> *actionList, QMenu *menu)
{
    QSet<QAction *> &actionPointers = *actionList;
    QString emptyQSTring;

    menu->addSeparator();
    actionPointers << menuEntry(menu, QStringLiteral("\\0"), emptyQSTring, i18n("Regular expression capture 0 (whole match)"));
    actionPointers << menuEntry(menu, QStringLiteral("\\"), emptyQSTring, i18n("Regular expression capture 1-9"), QStringLiteral("\\#"));
    actionPointers << menuEntry(menu, QStringLiteral("\\{"), QStringLiteral("}"), i18n("Regular expression capture 0-999"), QStringLiteral("\\{#"));
    menu->addSeparator();
    actionPointers << menuEntry(menu, QStringLiteral("\\U\\"), emptyQSTring, i18n("Upper-cased capture 0-9"), QStringLiteral("\\U\\#"));
    actionPointers << menuEntry(menu, QStringLiteral("\\U\\{"), QStringLiteral("}"), i18n("Upper-cased capture 0-999"), QStringLiteral("\\U\\{###"));
    actionPointers << menuEntry(menu, QStringLiteral("\\L\\"), emptyQSTring, i18n("Lower-cased capture 0-9"), QStringLiteral("\\L\\#"));
    actionPointers << menuEntry(menu, QStringLiteral("\\L\\{"), QStringLiteral("}"), i18n("Lower-cased capture 0-999"), QStringLiteral("\\L\\{###"));
}

/**
 * inserts text and sets cursor position
 */
static void regexHelperActOnAction(QAction *resultAction, const QSet<QAction *> &actionList, QLineEdit *lineEdit)
{
    if (resultAction && actionList.contains(resultAction)) {
        const int cursorPos = lineEdit->cursorPosition();
        QStringList beforeAfter = resultAction->data().toString().split(QLatin1Char(' '));
        if (beforeAfter.size() != 2) {
            return;
        }
        lineEdit->insert(beforeAfter[0] + beforeAfter[1]);
        lineEdit->setCursorPosition(cursorPos + beforeAfter[0].count());
        lineEdit->setFocus();
    }
}

Results::Results(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    treeView->setItemDelegate(new SPHtmlDelegate(treeView));
    treeView->setModel(&matchModel);

    auto updateColors = [this](KTextEditor::Editor *e) {
        if (!e) {
            return;
        }

        const auto theme = e->theme();
        auto bg = QColor::fromRgba(theme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor));
        auto hl = QColor::fromRgba(theme.editorColor(KSyntaxHighlighting::Theme::TextSelection));
        auto search = QColor::fromRgba(theme.editorColor(KSyntaxHighlighting::Theme::SearchHighlight));
        auto replace = QColor::fromRgba(theme.editorColor(KSyntaxHighlighting::Theme::ReplaceHighlight));
        auto fg = QColor::fromRgba(theme.textColor(KSyntaxHighlighting::Theme::Normal));

        auto pal = treeView->palette();
        pal.setColor(QPalette::Base, bg);
        pal.setColor(QPalette::Highlight, hl);
        pal.setColor(QPalette::Text, fg);
        matchModel.setMatchColors(fg.name(QColor::HexArgb), search.name(QColor::HexArgb), replace.name(QColor::HexArgb));
        treeView->setPalette(pal);

        Q_EMIT colorsChanged();
    };

    auto e = KTextEditor::Editor::instance();
    connect(e, &KTextEditor::Editor::configChanged, this, updateColors);
    updateColors(e);
}

K_PLUGIN_FACTORY_WITH_JSON(KatePluginSearchFactory, "katesearch.json", registerPlugin<KatePluginSearch>();)

KatePluginSearch::KatePluginSearch(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
{
    // ensure we can send over vector of matches via queued connection
    qRegisterMetaType<QVector<KateSearchMatch>>();

    m_searchCommand = new KateSearchCommand(this);
}

KatePluginSearch::~KatePluginSearch()
{
    delete m_searchCommand;
}

QObject *KatePluginSearch::createView(KTextEditor::MainWindow *mainWindow)
{
    KatePluginSearchView *view = new KatePluginSearchView(this, mainWindow, KTextEditor::Editor::instance()->application());
    connect(m_searchCommand, &KateSearchCommand::setSearchPlace, view, &KatePluginSearchView::setSearchPlace);
    connect(m_searchCommand, &KateSearchCommand::setCurrentFolder, view, &KatePluginSearchView::setCurrentFolder);
    connect(m_searchCommand, &KateSearchCommand::setSearchString, view, &KatePluginSearchView::setSearchString);
    connect(m_searchCommand, &KateSearchCommand::startSearch, view, &KatePluginSearchView::startSearch);
    connect(m_searchCommand, &KateSearchCommand::setRegexMode, view, &KatePluginSearchView::setRegexMode);
    connect(m_searchCommand, &KateSearchCommand::setCaseInsensitive, view, &KatePluginSearchView::setCaseInsensitive);
    connect(m_searchCommand, &KateSearchCommand::setExpandResults, view, &KatePluginSearchView::setExpandResults);
    connect(m_searchCommand, SIGNAL(newTab()), view, SLOT(addTab()));

    connect(view, &KatePluginSearchView::searchBusy, m_searchCommand, &KateSearchCommand::setBusy);

    return view;
}

bool ContainerWidget::focusNextPrevChild(bool next)
{
    QWidget *fw = focusWidget();
    bool found = false;
    Q_EMIT nextFocus(fw, &found, next);

    if (found) {
        return true;
    }
    return QWidget::focusNextPrevChild(next);
}

void KatePluginSearchView::nextFocus(QWidget *currentWidget, bool *found, bool next)
{
    *found = false;

    if (!currentWidget) {
        return;
    }

    // we use the object names here because there can be multiple trees (on multiple result tabs)
    if (next) {
        if (currentWidget->objectName() == QLatin1String("treeView") || currentWidget == m_ui.binaryCheckBox) {
            m_ui.searchCombo->setFocus();
            *found = true;
            return;
        }
        if (currentWidget == m_ui.excludeCombo && m_ui.searchPlaceCombo->currentIndex() > MatchModel::Folder) {
            m_ui.searchCombo->setFocus();
            *found = true;
            return;
        }
        if (currentWidget == m_ui.displayOptions) {
            if (m_ui.displayOptions->isChecked()) {
                if (m_ui.searchPlaceCombo->currentIndex() < MatchModel::Folder) {
                    m_ui.searchCombo->setFocus();
                    *found = true;
                    return;
                } else if (m_ui.searchPlaceCombo->currentIndex() == MatchModel::Folder) {
                    m_ui.folderRequester->setFocus();
                    *found = true;
                    return;
                } else {
                    m_ui.filterCombo->setFocus();
                    *found = true;
                    return;
                }
            } else {
                Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
                if (!res) {
                    return;
                }
                res->treeView->setFocus();
                *found = true;
                return;
            }
        }
    } else {
        if (currentWidget == m_ui.searchCombo) {
            if (m_ui.displayOptions->isChecked()) {
                if (m_ui.searchPlaceCombo->currentIndex() < MatchModel::Folder) {
                    m_ui.displayOptions->setFocus();
                    *found = true;
                    return;
                } else if (m_ui.searchPlaceCombo->currentIndex() == MatchModel::Folder) {
                    m_ui.binaryCheckBox->setFocus();
                    *found = true;
                    return;
                } else {
                    m_ui.excludeCombo->setFocus();
                    *found = true;
                    return;
                }
            } else {
                Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
                if (!res) {
                    return;
                }
                res->treeView->setFocus();
            }
            *found = true;
            return;
        } else {
            if (currentWidget->objectName() == QLatin1String("treeView")) {
                m_ui.displayOptions->setFocus();
                *found = true;
                return;
            }
        }
    }
}

KatePluginSearchView::KatePluginSearchView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainWin, KTextEditor::Application *application)
    : QObject(mainWin)
    , m_kateApp(application)
    , m_mainWindow(mainWin)
{
    KXMLGUIClient::setComponentName(QStringLiteral("katesearch"), i18n("Kate Search & Replace"));
    setXMLFile(QStringLiteral("ui.rc"));

    m_toolView = mainWin->createToolView(plugin,
                                         QStringLiteral("kate_plugin_katesearch"),
                                         KTextEditor::MainWindow::Bottom,
                                         QIcon::fromTheme(QStringLiteral("edit-find")),
                                         i18n("Search and Replace"));

    ContainerWidget *container = new ContainerWidget(m_toolView);
    m_ui.setupUi(container);
    container->setFocusProxy(m_ui.searchCombo);
    connect(container, &ContainerWidget::nextFocus, this, &KatePluginSearchView::nextFocus);

    QAction *a = actionCollection()->addAction(QStringLiteral("search_in_files"));
    actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    a->setText(i18n("Search in Files"));
    connect(a, &QAction::triggered, this, &KatePluginSearchView::openSearchView);

    a = actionCollection()->addAction(QStringLiteral("search_in_files_new_tab"));
    a->setText(i18n("Search in Files (in new tab)"));
    // first add tab, then open search view, since open search view switches to show the search options
    connect(a, &QAction::triggered, this, &KatePluginSearchView::addTab);
    connect(a, &QAction::triggered, this, &KatePluginSearchView::openSearchView);

    a = actionCollection()->addAction(QStringLiteral("go_to_next_match"));
    a->setText(i18n("Go to Next Match"));
    actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::Key_F6));
    connect(a, &QAction::triggered, this, &KatePluginSearchView::goToNextMatch);

    a = actionCollection()->addAction(QStringLiteral("go_to_prev_match"));
    a->setText(i18n("Go to Previous Match"));
    actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::SHIFT | Qt::Key_F6));
    connect(a, &QAction::triggered, this, &KatePluginSearchView::goToPreviousMatch);

    // Only show the tab bar when there is more than one tab
    m_ui.resultTabWidget->tabBar()->setAutoHide(true);
    m_ui.resultTabWidget->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);
    KAcceleratorManager::setNoAccel(m_ui.resultTabWidget);

    // Gnome does not seem to have all icons we want, so we use fall-back icons for those that are missing.
    QIcon dispOptIcon = QIcon::fromTheme(QStringLiteral("games-config-options"), QIcon::fromTheme(QStringLiteral("preferences-system")));
    QIcon matchCaseIcon = QIcon::fromTheme(QStringLiteral("format-text-superscript"), QIcon::fromTheme(QStringLiteral("format-text-bold")));
    QIcon useRegExpIcon = QIcon::fromTheme(QStringLiteral("code-context"), QIcon::fromTheme(QStringLiteral("edit-find-replace")));
    QIcon expandResultsIcon = QIcon::fromTheme(QStringLiteral("view-list-tree"), QIcon::fromTheme(QStringLiteral("format-indent-more")));

    m_ui.displayOptions->setIcon(dispOptIcon);
    m_ui.searchButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
    m_ui.nextButton->setIcon(QIcon::fromTheme(QStringLiteral("go-down-search")));
    m_ui.stopButton->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
    m_ui.matchCase->setIcon(matchCaseIcon);
    m_ui.useRegExp->setIcon(useRegExpIcon);
    m_ui.expandResults->setIcon(expandResultsIcon);
    m_ui.searchPlaceCombo->setItemIcon(MatchModel::CurrentFile, QIcon::fromTheme(QStringLiteral("text-plain")));
    m_ui.searchPlaceCombo->setItemIcon(MatchModel::OpenFiles, QIcon::fromTheme(QStringLiteral("text-plain")));
    m_ui.searchPlaceCombo->setItemIcon(MatchModel::Folder, QIcon::fromTheme(QStringLiteral("folder")));
    m_ui.folderUpButton->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    m_ui.currentFolderButton->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    m_ui.newTabButton->setIcon(QIcon::fromTheme(QStringLiteral("tab-new")));

    m_ui.filterCombo->setToolTip(i18n("Comma separated list of file types to search in. Example: \"*.cpp,*.h\"\n"));
    m_ui.excludeCombo->setToolTip(i18n("Comma separated list of files and directories to exclude from the search. Example: \"build*\""));

    addTab();

    // get url-requester's combo box and sanely initialize
    KComboBox *cmbUrl = m_ui.folderRequester->comboBox();
    cmbUrl->setDuplicatesEnabled(false);
    cmbUrl->setEditable(true);
    m_ui.folderRequester->setMode(KFile::Directory | KFile::LocalOnly);
    KUrlCompletion *cmpl = new KUrlCompletion(KUrlCompletion::DirCompletion);
    cmbUrl->setCompletionObject(cmpl);
    cmbUrl->setAutoDeleteCompletionObject(true);

    connect(m_ui.newTabButton, &QToolButton::clicked, this, &KatePluginSearchView::addTab);
    connect(m_ui.resultTabWidget, &QTabWidget::tabCloseRequested, this, &KatePluginSearchView::tabCloseRequested);
    connect(m_ui.resultTabWidget, &QTabWidget::currentChanged, this, &KatePluginSearchView::resultTabChanged);

    connect(m_ui.folderUpButton, &QToolButton::clicked, this, &KatePluginSearchView::navigateFolderUp);
    connect(m_ui.currentFolderButton, &QToolButton::clicked, this, &KatePluginSearchView::setCurrentFolder);
    connect(m_ui.expandResults, &QToolButton::clicked, this, &KatePluginSearchView::expandResults);

    connect(m_ui.searchCombo, &QComboBox::editTextChanged, &m_changeTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(m_ui.matchCase, &QToolButton::toggled, &m_changeTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(m_ui.matchCase, &QToolButton::toggled, this, [=] {
        Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
        if (res) {
            res->matchCase = m_ui.matchCase->isChecked();
        }
    });

    connect(m_ui.searchCombo->lineEdit(), &QLineEdit::returnPressed, this, &KatePluginSearchView::startSearch);
    // connecting to returnPressed() of the folderRequester doesn't work, I haven't found out why yet. But connecting to the linedit works:
    connect(m_ui.folderRequester->comboBox()->lineEdit(), &QLineEdit::returnPressed, this, &KatePluginSearchView::startSearch);
    connect(m_ui.filterCombo, static_cast<void (KComboBox::*)()>(&KComboBox::returnPressed), this, &KatePluginSearchView::startSearch);
    connect(m_ui.excludeCombo, static_cast<void (KComboBox::*)()>(&KComboBox::returnPressed), this, &KatePluginSearchView::startSearch);
    connect(m_ui.searchButton, &QPushButton::clicked, this, &KatePluginSearchView::startSearch);

    connect(m_ui.displayOptions, &QToolButton::toggled, this, &KatePluginSearchView::toggleOptions);
    connect(m_ui.searchPlaceCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &KatePluginSearchView::searchPlaceChanged);
    connect(m_ui.searchPlaceCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int) {
        if (m_ui.searchPlaceCombo->currentIndex() == MatchModel::Folder) {
            m_ui.displayOptions->setChecked(true);
        }
    });

    connect(m_ui.stopButton, &QPushButton::clicked, this, &KatePluginSearchView::stopClicked);

    connect(m_ui.nextButton, &QToolButton::clicked, this, &KatePluginSearchView::goToNextMatch);

    connect(m_ui.replaceButton, &QPushButton::clicked, this, &KatePluginSearchView::replaceSingleMatch);
    connect(m_ui.replaceCheckedBtn, &QPushButton::clicked, this, &KatePluginSearchView::replaceChecked);
    connect(m_ui.replaceCombo->lineEdit(), &QLineEdit::returnPressed, this, &KatePluginSearchView::replaceChecked);

    m_ui.displayOptions->setChecked(true);

    connect(&m_searchOpenFiles, &SearchOpenFiles::matchesFound, this, &KatePluginSearchView::matchesFound);
    connect(&m_searchOpenFiles, &SearchOpenFiles::searchDone, this, &KatePluginSearchView::searchDone);

    m_diskSearchDoneTimer.setSingleShot(true);
    m_diskSearchDoneTimer.setInterval(10);
    connect(&m_diskSearchDoneTimer, &QTimer::timeout, this, &KatePluginSearchView::searchDone);

    m_updateCheckedStateTimer.setSingleShot(true);
    m_updateCheckedStateTimer.setInterval(10);
    connect(&m_updateCheckedStateTimer, &QTimer::timeout, this, &KatePluginSearchView::updateMatchMarks);

    // queued connect to signals emitted outside of background thread
    connect(&m_folderFilesList, &FolderFilesList::fileListReady, this, &KatePluginSearchView::folderFileListChanged, Qt::QueuedConnection);
    connect(
        &m_folderFilesList,
        &FolderFilesList::searching,
        this,
        [this](const QString &path) {
            Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
            if (res) {
                res->matchModel.setFileListUpdate(path);
            }
        },
        Qt::QueuedConnection);

    connect(m_kateApp, &KTextEditor::Application::documentWillBeDeleted, this, &KatePluginSearchView::clearDocMarksAndRanges);
    connect(m_kateApp, &KTextEditor::Application::documentWillBeDeleted, &m_searchOpenFiles, &SearchOpenFiles::cancelSearch);
    connect(m_kateApp, &KTextEditor::Application::documentWillBeDeleted, this, [this]() {
        Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
        if (res) {
            res->matchModel.cancelReplace();
        }
    });

    m_ui.searchCombo->lineEdit()->setPlaceholderText(i18n("Find"));
    // Hook into line edit context menus
    m_ui.searchCombo->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_ui.searchCombo, &QComboBox::customContextMenuRequested, this, &KatePluginSearchView::searchContextMenu);
    m_ui.searchCombo->completer()->setCompletionMode(QCompleter::PopupCompletion);
    m_ui.searchCombo->completer()->setCaseSensitivity(Qt::CaseSensitive);
    m_ui.searchCombo->setInsertPolicy(QComboBox::NoInsert);
    m_ui.searchCombo->lineEdit()->setClearButtonEnabled(true);
    m_ui.searchCombo->setMaxCount(25);
    QAction *searchComboActionForInsertRegexButton =
        m_ui.searchCombo->lineEdit()->addAction(QIcon::fromTheme(QStringLiteral("code-context"), QIcon::fromTheme(QStringLiteral("edit-find-replace"))),
                                                QLineEdit::TrailingPosition);
    connect(searchComboActionForInsertRegexButton, &QAction::triggered, this, [this]() {
        QMenu menu;
        QSet<QAction *> actionList;
        addRegexHelperActionsForSearch(&actionList, &menu);
        auto &&action = menu.exec(QCursor::pos());
        regexHelperActOnAction(action, actionList, m_ui.searchCombo->lineEdit());
    });

    m_ui.replaceCombo->lineEdit()->setPlaceholderText(i18n("Replace"));
    // Hook into line edit context menus
    m_ui.replaceCombo->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_ui.replaceCombo, &QComboBox::customContextMenuRequested, this, &KatePluginSearchView::replaceContextMenu);
    m_ui.replaceCombo->completer()->setCompletionMode(QCompleter::PopupCompletion);
    m_ui.replaceCombo->completer()->setCaseSensitivity(Qt::CaseSensitive);
    m_ui.replaceCombo->setInsertPolicy(QComboBox::NoInsert);
    m_ui.replaceCombo->lineEdit()->setClearButtonEnabled(true);
    m_ui.replaceCombo->setMaxCount(25);
    QAction *replaceComboActionForInsertRegexButton =
        m_ui.replaceCombo->lineEdit()->addAction(QIcon::fromTheme(QStringLiteral("code-context")), QLineEdit::TrailingPosition);
    connect(replaceComboActionForInsertRegexButton, &QAction::triggered, this, [this]() {
        QMenu menu;
        QSet<QAction *> actionList;
        addRegexHelperActionsForReplace(&actionList, &menu);
        auto &&action = menu.exec(QCursor::pos());
        regexHelperActOnAction(action, actionList, m_ui.replaceCombo->lineEdit());
    });
    QAction *replaceComboActionForInsertSpecialButton = m_ui.replaceCombo->lineEdit()->addAction(QIcon::fromTheme(QStringLiteral("insert-text")), //
                                                                                                 QLineEdit::TrailingPosition);
    connect(replaceComboActionForInsertSpecialButton, &QAction::triggered, this, [this]() {
        QMenu menu;
        QSet<QAction *> actionList;
        addSpecialCharsHelperActionsForReplace(&actionList, &menu);
        auto &&action = menu.exec(QCursor::pos());
        regexHelperActOnAction(action, actionList, m_ui.replaceCombo->lineEdit());
    });

    connect(m_ui.useRegExp, &QToolButton::toggled, &m_changeTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    auto onRegexToggleChanged = [=] {
        Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
        if (res) {
            bool useRegExp = m_ui.useRegExp->isChecked();
            res->useRegExp = useRegExp;
            searchComboActionForInsertRegexButton->setVisible(useRegExp);
            replaceComboActionForInsertRegexButton->setVisible(useRegExp);
        }
    };
    connect(m_ui.useRegExp, &QToolButton::toggled, this, onRegexToggleChanged);
    onRegexToggleChanged(); // invoke initially
    m_changeTimer.setInterval(300);
    m_changeTimer.setSingleShot(true);
    connect(&m_changeTimer, &QTimer::timeout, this, &KatePluginSearchView::startSearchWhileTyping);

    m_toolView->setMinimumHeight(container->sizeHint().height());

    connect(m_mainWindow, &KTextEditor::MainWindow::unhandledShortcutOverride, this, &KatePluginSearchView::handleEsc);

    // watch for project plugin view creation/deletion
    connect(m_mainWindow, &KTextEditor::MainWindow::pluginViewCreated, this, &KatePluginSearchView::slotPluginViewCreated);

    connect(m_mainWindow, &KTextEditor::MainWindow::pluginViewDeleted, this, &KatePluginSearchView::slotPluginViewDeleted);

    connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &KatePluginSearchView::updateMatchMarks);

    // Connect signals from project plugin to our slots
    m_projectPluginView = m_mainWindow->pluginView(QStringLiteral("kateprojectplugin"));
    slotPluginViewCreated(QStringLiteral("kateprojectplugin"), m_projectPluginView);

    searchPlaceChanged();

    m_toolView->installEventFilter(this);

    m_mainWindow->guiFactory()->addClient(this);
}

KatePluginSearchView::~KatePluginSearchView()
{
    cancelDiskFileSearch();
    clearMarksAndRanges();
    m_mainWindow->guiFactory()->removeClient(this);
    delete m_toolView;
}

void KatePluginSearchView::navigateFolderUp()
{
    // navigate one folder up
    m_ui.folderRequester->setUrl(localFileDirUp(m_ui.folderRequester->url()));
}

void KatePluginSearchView::setCurrentFolder()
{
    if (!m_mainWindow) {
        return;
    }
    KTextEditor::View *editView = m_mainWindow->activeView();
    if (editView && editView->document()) {
        // upUrl as we want the folder not the file
        m_ui.folderRequester->setUrl(localFileDirUp(editView->document()->url()));
    }
    m_ui.displayOptions->setChecked(true);
}

void KatePluginSearchView::openSearchView()
{
    if (!m_mainWindow) {
        return;
    }
    if (!m_toolView->isVisible()) {
        m_mainWindow->showToolView(m_toolView);
    }
    m_ui.searchCombo->setFocus(Qt::OtherFocusReason);
    if (m_ui.searchPlaceCombo->currentIndex() == MatchModel::Folder) {
        m_ui.displayOptions->setChecked(true);
    }

    KTextEditor::View *editView = m_mainWindow->activeView();
    if (editView && editView->document()) {
        if (m_ui.folderRequester->text().isEmpty()) {
            // upUrl as we want the folder not the file
            m_ui.folderRequester->setUrl(localFileDirUp(editView->document()->url()));
        }
        QString selection;
        if (editView->selection()) {
            selection = editView->selectionText();
            // remove possible trailing '\n'
            if (selection.endsWith(QLatin1Char('\n'))) {
                selection = selection.left(selection.size() - 1);
            }
        }
        if (selection.isEmpty()) {
            selection = editView->document()->wordAt(editView->cursorPosition());
        }

        if (!selection.isEmpty() && !selection.contains(QLatin1Char('\n'))) {
            m_ui.searchCombo->blockSignals(true);
            m_ui.searchCombo->lineEdit()->setText(selection);
            m_ui.searchCombo->blockSignals(false);
        }

        m_ui.searchCombo->lineEdit()->selectAll();
        m_searchJustOpened = true;
        startSearchWhileTyping();
    }
}

void KatePluginSearchView::handleEsc(QEvent *e)
{
    if (!m_mainWindow) {
        return;
    }

    QKeyEvent *k = static_cast<QKeyEvent *>(e);
    if (k->key() == Qt::Key_Escape && k->modifiers() == Qt::NoModifier) {
        static ulong lastTimeStamp;
        if (lastTimeStamp == k->timestamp()) {
            // Same as previous... This looks like a bug somewhere...
            return;
        }
        lastTimeStamp = k->timestamp();
        if (!m_matchRanges.isEmpty()) {
            clearMarksAndRanges();
        } else if (m_toolView->isVisible()) {
            m_mainWindow->hideToolView(m_toolView);
        }
        // uncheck all so no new marks are added again when switching views
        Results *curResults = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
        if (curResults) {
            curResults->matchModel.uncheckAll();
        }
    }
}

void KatePluginSearchView::setSearchString(const QString &pattern)
{
    m_ui.searchCombo->lineEdit()->setText(pattern);
}

void KatePluginSearchView::toggleOptions(bool show)
{
    m_ui.stackedWidget->setCurrentIndex((show) ? 1 : 0);
}

void KatePluginSearchView::setSearchPlace(int place)
{
    if (place >= m_ui.searchPlaceCombo->count()) {
        // This probably means the project plugin is not active or no project loaded
        // fallback to search in folder
        qDebug() << place << "is not a valid search place index";
        place = MatchModel::Folder;
    }
    m_ui.searchPlaceCombo->setCurrentIndex(place);
}

QStringList KatePluginSearchView::filterFiles(const QStringList &files) const
{
    QString types = m_ui.filterCombo->currentText();
    QString excludes = m_ui.excludeCombo->currentText();
    if (((types.isEmpty() || types == QLatin1String("*"))) && (excludes.isEmpty())) {
        // shortcut for use all files
        return files;
    }

    if (types.isEmpty()) {
        types = QStringLiteral("*");
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const auto SkipEmptyParts = QString::SkipEmptyParts;
#else
    const auto SkipEmptyParts = Qt::SkipEmptyParts;
#endif
    const QStringList tmpTypes = types.split(QLatin1Char(','), SkipEmptyParts);
    QVector<QRegularExpression> typeList;
    for (const auto &type : tmpTypes) {
        typeList << QRegularExpression(QRegularExpression::wildcardToRegularExpression(type.trimmed()));
    }

    const QStringList tmpExcludes = excludes.split(QLatin1Char(','), SkipEmptyParts);
    QVector<QRegularExpression> excludeList;
    for (const auto &exclude : tmpExcludes) {
        excludeList << QRegularExpression(QRegularExpression::wildcardToRegularExpression(exclude.trimmed()));
    }

    QStringList filteredFiles;
    for (const QString &filePath : files) {
        bool isInSubDir = filePath.startsWith(m_resultBaseDir);
        QString nameToCheck = filePath;
        if (isInSubDir) {
            nameToCheck = filePath.mid(m_resultBaseDir.size());
        }

        bool skip = false;
        const QStringList pathSplit = nameToCheck.split(QLatin1Char('/'), SkipEmptyParts);
        for (const auto &regex : qAsConst(excludeList)) {
            for (const auto &part : pathSplit) {
                QRegularExpressionMatch match = regex.match(part);
                if (match.hasMatch()) {
                    skip = true;
                    break;
                }
            }
        }
        if (skip) {
            continue;
        }

        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();

        for (const auto &regex : qAsConst(typeList)) {
            QRegularExpressionMatch match = regex.match(fileName);
            if (match.hasMatch()) {
                filteredFiles << filePath;
                break;
            }
        }
    }
    return filteredFiles;
}

void KatePluginSearchView::folderFileListChanged()
{
    if (!m_curResults) {
        qWarning() << "This is a bug";
        searchDone();
        return;
    }
    QStringList fileList = m_folderFilesList.fileList();

    if (fileList.isEmpty()) {
        searchDone();
        return;
    }

    QList<KTextEditor::Document *> openList;
    for (int i = 0; i < m_kateApp->documents().size(); i++) {
        int index = fileList.indexOf(m_kateApp->documents()[i]->url().toLocalFile());
        if (index != -1) {
            openList << m_kateApp->documents()[i];
            fileList.removeAt(index);
        }
    }

    // search order is important: Open files starts immediately and should finish
    // earliest after first event loop.
    // The DiskFile might finish immediately
    if (!openList.empty()) {
        m_searchOpenFiles.startSearch(openList, m_curResults->regExp);
    }

    startDiskFileSearch(fileList, m_curResults->regExp, m_ui.binaryCheckBox->isChecked());
}

void KatePluginSearchView::startDiskFileSearch(const QStringList &fileList, const QRegularExpression &reg, bool includeBinaryFiles)
{
    if (fileList.isEmpty()) {
        searchDone();
        return;
    }

    // spread work to X threads => default to ideal thread count
    const int threadCount = m_searchDiskFilePool.maxThreadCount();

    // init worklist for these number of threads
    m_worklistForDiskFiles.init(fileList, threadCount);

    // spawn enough runnables, they will pull the files themself from our worklist
    // this must exactly match the count we used to init the worklist above, as this is used to finalize stuff!
    for (int i = 0; i < threadCount; ++i) {
        // new runnable, will pull work from the worklist itself!
        // worklist is used to drive if we need to stop the work, too!
        SearchDiskFiles *runner = new SearchDiskFiles(m_worklistForDiskFiles, reg, includeBinaryFiles);

        // queued connection for the results, this is emitted by a different thread than the runnable object and this one!
        connect(runner, &SearchDiskFiles::matchesFound, this, &KatePluginSearchView::matchesFound, Qt::QueuedConnection);

        // queued connection for the results, this is emitted by a different thread than the runnable object and this one!
        connect(
            runner,
            &SearchDiskFiles::destroyed,
            this,
            [this]() {
                // signal the worklist one runnable more is done
                m_worklistForDiskFiles.markOnRunnableAsDone();

                // if no longer anything running, signal finished!
                if (!m_worklistForDiskFiles.isRunning()) {
                    if (!m_diskSearchDoneTimer.isActive()) {
                        m_diskSearchDoneTimer.start();
                    }
                }
            },
            Qt::QueuedConnection);

        // launch the runnable
        m_searchDiskFilePool.start(runner);
    }
}

void KatePluginSearchView::cancelDiskFileSearch()
{
    // signal canceling to runnables
    m_worklistForDiskFiles.cancel();

    // wait for finalization
    m_searchDiskFilePool.clear();
    m_searchDiskFilePool.waitForDone();
}

bool KatePluginSearchView::searchingDiskFiles()
{
    return m_worklistForDiskFiles.isRunning() || m_folderFilesList.isRunning();
}

void KatePluginSearchView::searchPlaceChanged()
{
    int searchPlace = m_ui.searchPlaceCombo->currentIndex();
    const bool inFolder = (searchPlace == MatchModel::Folder);
    const bool inCurrentProject = searchPlace == MatchModel::Project;
    const bool inAllOpenProjects = searchPlace == MatchModel::AllProjects;

    m_ui.filterCombo->setEnabled(searchPlace >= MatchModel::Folder);
    m_ui.excludeCombo->setEnabled(searchPlace >= MatchModel::Folder);
    m_ui.folderRequester->setEnabled(inFolder);
    m_ui.folderUpButton->setEnabled(inFolder);
    m_ui.currentFolderButton->setEnabled(inFolder);
    m_ui.recursiveCheckBox->setEnabled(inFolder);
    m_ui.hiddenCheckBox->setEnabled(inFolder);
    m_ui.symLinkCheckBox->setEnabled(inFolder);
    m_ui.binaryCheckBox->setEnabled(inFolder || inCurrentProject || inAllOpenProjects);

    if (inFolder && sender() == m_ui.searchPlaceCombo) {
        setCurrentFolder();
    }

    // ... and the labels:
    m_ui.folderLabel->setEnabled(m_ui.folderRequester->isEnabled());
    m_ui.filterLabel->setEnabled(m_ui.filterCombo->isEnabled());
    m_ui.excludeLabel->setEnabled(m_ui.excludeCombo->isEnabled());

    Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (res) {
        res->searchPlaceIndex = searchPlace;
    }
}

void KatePluginSearchView::matchesFound(const QUrl &url, const QVector<KateSearchMatch> &searchMatches)
{
    if (!m_curResults) {
        return;
    }

    m_curResults->matchModel.addMatches(url, searchMatches);
    m_curResults->matches += searchMatches.size();
}

void KatePluginSearchView::stopClicked()
{
    m_folderFilesList.terminateSearch();
    m_searchOpenFiles.cancelSearch();
    cancelDiskFileSearch();
    Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (res) {
        res->matchModel.cancelReplace();
    }
}

/**
 * update the search widget colors and font. This is done on start of every
 * search so that if the user changes the theme, he can see the new colors
 * on the next search
 */
void KatePluginSearchView::updateViewColors()
{
    auto *view = m_mainWindow->activeView();
    KTextEditor::ConfigInterface *ciface = qobject_cast<KTextEditor::ConfigInterface *>(view);
    if (ciface && view) {
        // save for later reuse when the search tree starts getting populated
        QColor search = ciface->configValue(QStringLiteral("search-highlight-color")).value<QColor>();
        if (!search.isValid()) {
            search = Qt::yellow;
        }
        m_replaceHighlightColor = ciface->configValue(QStringLiteral("replace-highlight-color")).value<QColor>();
        if (!m_replaceHighlightColor.isValid()) {
            m_replaceHighlightColor = Qt::green;
        }
        QColor fg = view->defaultStyleAttribute(KTextEditor::dsNormal)->foreground().color();

        if (!m_resultAttr) {
            m_resultAttr = new KTextEditor::Attribute();
        }
        // reset colors at the start of search
        m_resultAttr->clear();
        m_resultAttr->setBackground(search);
        m_resultAttr->setForeground(fg);

        if (m_curResults) {
            auto *delegate = qobject_cast<SPHtmlDelegate *>(m_curResults->treeView->itemDelegate());
            if (delegate) {
                delegate->setDisplayFont(ciface->configValue(QStringLiteral("font")).value<QFont>());
            }
        }
    }
}

// static QElapsedTimer s_timer;
void KatePluginSearchView::startSearch()
{
    // s_timer.start();

    // Forcefully stop any ongoing search or replace
    m_folderFilesList.terminateSearch();
    m_searchOpenFiles.terminateSearch();
    cancelDiskFileSearch();

    Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (res) {
        res->matchModel.cancelReplace();
    }

    m_changeTimer.stop(); // make sure not to start a "while you type" search now
    m_mainWindow->showToolView(m_toolView); // in case we are invoked from the command interface
    m_projectSearchPlaceIndex = 0; // now that we started, don't switch back automatically

    if (m_ui.searchCombo->currentText().isEmpty()) {
        // return pressed in the folder combo or filter combo
        return;
    }

    m_isSearchAsYouType = false;

    QString currentSearchText = m_ui.searchCombo->currentText();
    m_ui.searchCombo->setItemText(0, QString()); // remove the text from index 0 on enter/search
    int index = m_ui.searchCombo->findText(currentSearchText);
    if (index > 0) {
        m_ui.searchCombo->removeItem(index);
    }
    m_ui.searchCombo->insertItem(1, currentSearchText);
    m_ui.searchCombo->setCurrentIndex(1);

    if (m_ui.filterCombo->findText(m_ui.filterCombo->currentText()) == -1) {
        m_ui.filterCombo->insertItem(0, m_ui.filterCombo->currentText());
        m_ui.filterCombo->setCurrentIndex(0);
    }
    if (m_ui.excludeCombo->findText(m_ui.excludeCombo->currentText()) == -1) {
        m_ui.excludeCombo->insertItem(0, m_ui.excludeCombo->currentText());
        m_ui.excludeCombo->setCurrentIndex(0);
    }
    if (m_ui.folderRequester->comboBox()->findText(m_ui.folderRequester->comboBox()->currentText()) == -1) {
        m_ui.folderRequester->comboBox()->insertItem(0, m_ui.folderRequester->comboBox()->currentText());
        m_ui.folderRequester->comboBox()->setCurrentIndex(0);
    }
    m_curResults = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (!m_curResults) {
        qWarning() << "This is a bug";
        return;
    }

    QString pattern = (m_ui.useRegExp->isChecked() ? currentSearchText : QRegularExpression::escape(currentSearchText));
    QRegularExpression::PatternOptions patternOptions = QRegularExpression::UseUnicodePropertiesOption;
    if (!m_ui.matchCase->isChecked()) {
        patternOptions |= QRegularExpression::CaseInsensitiveOption;
    }

    if (pattern.contains(QLatin1String("\\n"))) {
        patternOptions |= QRegularExpression::MultilineOption;
    }
    QRegularExpression reg(pattern, patternOptions);

    if (!reg.isValid()) {
        // qDebug() << "invalid regexp";
        indicateMatch(false);
        return;
    }

    Q_EMIT searchBusy(true);

    updateViewColors();

    m_curResults->regExp = reg;
    m_curResults->useRegExp = m_ui.useRegExp->isChecked();
    m_curResults->matchCase = m_ui.matchCase->isChecked();
    m_curResults->searchPlaceIndex = m_ui.searchPlaceCombo->currentIndex();

    m_ui.newTabButton->setDisabled(true);
    m_ui.searchCombo->setDisabled(true);
    m_ui.searchButton->setDisabled(true);
    m_ui.displayOptions->setChecked(false);
    m_ui.displayOptions->setDisabled(true);
    m_ui.replaceCheckedBtn->setDisabled(true);
    m_ui.replaceButton->setDisabled(true);
    m_ui.stopAndNext->setCurrentWidget(m_ui.stopButton);
    m_ui.replaceCombo->setDisabled(true);
    m_ui.searchPlaceCombo->setDisabled(true);
    m_ui.useRegExp->setDisabled(true);
    m_ui.matchCase->setDisabled(true);
    m_ui.expandResults->setDisabled(true);
    m_ui.currentFolderButton->setDisabled(true);

    clearMarksAndRanges();
    m_curResults->matches = 0;

    m_ui.resultTabWidget->setTabText(m_ui.resultTabWidget->currentIndex(), m_ui.searchCombo->currentText());

    m_toolView->setCursor(Qt::WaitCursor);

    const bool inCurrentProject = m_ui.searchPlaceCombo->currentIndex() == MatchModel::Project;
    const bool inAllOpenProjects = m_ui.searchPlaceCombo->currentIndex() == MatchModel::AllProjects;

    m_curResults->matchModel.clear();
    m_curResults->matchModel.setSearchPlace(static_cast<MatchModel::SearchPlaces>(m_curResults->searchPlaceIndex));
    m_curResults->matchModel.setSearchState(MatchModel::Searching);
    m_curResults->treeView->expand(m_curResults->matchModel.index(0, 0));

    if (m_ui.searchPlaceCombo->currentIndex() == MatchModel::CurrentFile) {
        m_resultBaseDir.clear();
        QList<KTextEditor::Document *> documents;
        KTextEditor::View *activeView = m_mainWindow->activeView();
        if (activeView) {
            documents << activeView->document();
        }
        m_searchOpenFiles.startSearch(documents, reg);
    } else if (m_ui.searchPlaceCombo->currentIndex() == MatchModel::OpenFiles) {
        m_resultBaseDir.clear();
        const QList<KTextEditor::Document *> documents = m_kateApp->documents();
        m_searchOpenFiles.startSearch(documents, reg);
    } else if (m_ui.searchPlaceCombo->currentIndex() == MatchModel::Folder) {
        m_resultBaseDir = m_ui.folderRequester->url().path();
        if (!m_resultBaseDir.isEmpty() && !m_resultBaseDir.endsWith(QLatin1Char('/'))) {
            m_resultBaseDir += QLatin1Char('/');
        }
        m_curResults->matchModel.setBaseSearchPath(m_resultBaseDir);
        m_folderFilesList.generateList(m_ui.folderRequester->text(),
                                       m_ui.recursiveCheckBox->isChecked(),
                                       m_ui.hiddenCheckBox->isChecked(),
                                       m_ui.symLinkCheckBox->isChecked(),
                                       m_ui.filterCombo->currentText(),
                                       m_ui.excludeCombo->currentText());
        // the file list will be ready when the thread returns (connected to folderFileListChanged)
    } else if (inCurrentProject || inAllOpenProjects) {
        /**
         * init search with file list from current project, if any
         */
        m_resultBaseDir.clear();
        QStringList files;
        if (m_projectPluginView) {
            if (inCurrentProject) {
                m_resultBaseDir = m_projectPluginView->property("projectBaseDir").toString();
                m_curResults->matchModel.setProjectName(m_projectPluginView->property("projectName").toString());
            } else {
                m_resultBaseDir = m_projectPluginView->property("allProjectsCommonBaseDir").toString();
                m_curResults->matchModel.setProjectName(m_projectPluginView->property("projectName").toString());
            }

            if (!m_resultBaseDir.endsWith(QLatin1Char('/'))) {
                m_resultBaseDir += QLatin1Char('/');
            }

            QStringList projectFiles;
            if (inCurrentProject) {
                projectFiles = m_projectPluginView->property("projectFiles").toStringList();
            } else {
                projectFiles = m_projectPluginView->property("allProjectsFiles").toStringList();
            }

            files = filterFiles(projectFiles);
        }
        m_curResults->matchModel.setBaseSearchPath(m_resultBaseDir);

        QList<KTextEditor::Document *> openList;
        const auto docs = m_kateApp->documents();
        for (const auto doc : docs) {
            // match project file's list toLocalFile()
            int index = files.indexOf(doc->url().toLocalFile());
            if (index != -1) {
                openList << doc;
                files.removeAt(index);
            }
        }
        // search order is important: Open files starts immediately and should finish
        // earliest after first event loop.
        // The DiskFile might finish immediately
        if (!openList.empty()) {
            m_searchOpenFiles.startSearch(openList, m_curResults->regExp);
        }
        startDiskFileSearch(files, m_curResults->regExp, m_ui.binaryCheckBox->isChecked());
    } else {
        qDebug() << "Case not handled:" << m_ui.searchPlaceCombo->currentIndex();
        Q_ASSERT_X(false, "KatePluginSearchView::startSearch", "case not handled");
    }
}

void KatePluginSearchView::startSearchWhileTyping()
{
    if (searchingDiskFiles() || m_searchOpenFiles.searching()) {
        return;
    }
    updateViewColors();

    m_isSearchAsYouType = true;

    QString currentSearchText = m_ui.searchCombo->currentText();

    m_ui.searchButton->setDisabled(currentSearchText.isEmpty());

    // Do not clear the search results if you press up by mistake
    if (currentSearchText.isEmpty()) {
        return;
    }

    if (!m_mainWindow->activeView()) {
        return;
    }

    KTextEditor::Document *doc = m_mainWindow->activeView()->document();
    if (!doc) {
        return;
    }

    m_curResults = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (!m_curResults) {
        qWarning() << "This is a bug";
        return;
    }

    // check if we typed something or just changed combobox index
    // changing index should not trigger a search-as-you-type
    if (m_ui.searchCombo->currentIndex() > 0 && currentSearchText == m_ui.searchCombo->itemText(m_ui.searchCombo->currentIndex())) {
        return;
    }

    // Now we should have a true typed text change

    QString pattern = (m_ui.useRegExp->isChecked() ? currentSearchText : QRegularExpression::escape(currentSearchText));
    QRegularExpression::PatternOptions patternOptions = QRegularExpression::UseUnicodePropertiesOption;
    if (!m_ui.matchCase->isChecked()) {
        patternOptions |= QRegularExpression::CaseInsensitiveOption;
    }
    if (pattern.contains(QLatin1String("\\n"))) {
        patternOptions |= QRegularExpression::MultilineOption;
    }
    QRegularExpression reg(pattern, patternOptions);

    if (!reg.isValid()) {
        // qDebug() << "invalid regexp";
        indicateMatch(false);
        return;
    }

    Q_EMIT searchBusy(true);

    m_curResults->regExp = reg;
    m_curResults->useRegExp = m_ui.useRegExp->isChecked();

    m_ui.replaceCheckedBtn->setDisabled(true);
    m_ui.replaceButton->setDisabled(true);
    m_ui.nextButton->setDisabled(true);

    int cursorPosition = m_ui.searchCombo->lineEdit()->cursorPosition();
    bool hasSelected = m_ui.searchCombo->lineEdit()->hasSelectedText();
    m_ui.searchCombo->blockSignals(true);
    if (m_ui.searchCombo->count() == 0) {
        m_ui.searchCombo->insertItem(0, currentSearchText);
    } else {
        m_ui.searchCombo->setItemText(0, currentSearchText);
    }
    m_ui.searchCombo->setCurrentIndex(0);
    m_ui.searchCombo->lineEdit()->setCursorPosition(cursorPosition);
    if (hasSelected) {
        // This restores the select all from invoking openSearchView
        // This selects too much if we have a partial selection and toggle match-case/regexp
        m_ui.searchCombo->lineEdit()->selectAll();
    }
    m_ui.searchCombo->blockSignals(false);

    // Prepare for the new search content
    clearMarksAndRanges();
    m_resultBaseDir.clear();
    m_curResults->matches = 0;

    m_curResults->matchModel.clear();
    m_curResults->matchModel.setSearchPlace(MatchModel::CurrentFile);
    m_curResults->matchModel.setSearchState(MatchModel::Searching);
    m_curResults->treeView->expand(m_curResults->matchModel.index(0, 0));

    // Do the search
    int searchStoppedAt = m_searchOpenFiles.searchOpenFile(doc, reg, 0);
    searchWhileTypingDone();

    if (searchStoppedAt != 0) {
        delete m_infoMessage;
        const QString msg = i18n("Searching while you type was interrupted. It would have taken too long.");
        m_infoMessage = new KTextEditor::Message(msg, KTextEditor::Message::Warning);
        m_infoMessage->setPosition(KTextEditor::Message::TopInView);
        m_infoMessage->setAutoHide(3000);
        m_infoMessage->setAutoHideMode(KTextEditor::Message::Immediate);
        m_infoMessage->setView(m_mainWindow->activeView());
        m_mainWindow->activeView()->document()->postMessage(m_infoMessage);
    }
}

void KatePluginSearchView::searchDone()
{
    m_changeTimer.stop(); // avoid "while you type" search directly after

    if (searchingDiskFiles() || m_searchOpenFiles.searching()) {
        return;
    }

    QWidget *fw = QApplication::focusWidget();
    // NOTE: we take the focus widget here before the enabling/disabling
    // moves the focus around.
    m_ui.newTabButton->setDisabled(false);
    m_ui.searchCombo->setDisabled(false);
    m_ui.searchButton->setDisabled(false);
    m_ui.stopAndNext->setCurrentWidget(m_ui.nextButton);
    m_ui.displayOptions->setDisabled(false);
    m_ui.replaceCombo->setDisabled(false);
    m_ui.searchPlaceCombo->setDisabled(false);
    m_ui.useRegExp->setDisabled(false);
    m_ui.matchCase->setDisabled(false);
    m_ui.expandResults->setDisabled(false);
    m_ui.currentFolderButton->setDisabled(m_ui.searchPlaceCombo->currentIndex() != MatchModel::Folder);

    Q_EMIT searchBusy(false);

    if (!m_curResults) {
        return;
    }

    m_ui.replaceCheckedBtn->setDisabled(m_curResults->matches < 1);
    m_ui.replaceButton->setDisabled(m_curResults->matches < 1);
    m_ui.nextButton->setDisabled(m_curResults->matches < 1);

    // Set search to done. This sorts the model and collapses all items in the view
    m_curResults->matchModel.setSearchState(MatchModel::SearchDone);

    // expand the "header item " to display all files and all results if configured
    expandResults();

    m_curResults->treeView->resizeColumnToContents(0);

    indicateMatch(m_curResults->matches > 0);

    m_curResults = nullptr;
    m_toolView->unsetCursor();

    if (fw == m_ui.stopButton) {
        m_ui.searchCombo->setFocus();
    }

    m_searchJustOpened = false;
    updateMatchMarks();

    // qDebug() << "done:" << s_timer.elapsed();
}

void KatePluginSearchView::searchWhileTypingDone()
{
    Q_EMIT searchBusy(false);

    if (!m_curResults) {
        return;
    }

    bool popupVisible = m_ui.searchCombo->lineEdit()->completer()->popup()->isVisible();

    m_ui.replaceCheckedBtn->setDisabled(m_curResults->matches < 1);
    m_ui.replaceButton->setDisabled(m_curResults->matches < 1);
    m_ui.nextButton->setDisabled(m_curResults->matches < 1);

    m_curResults->treeView->expandAll();
    m_curResults->treeView->resizeColumnToContents(0);
    if (m_curResults->treeView->columnWidth(0) < m_curResults->treeView->width() - 30) {
        m_curResults->treeView->setColumnWidth(0, m_curResults->treeView->width() - 30);
    }

    // Set search to done. This sorts the model and collapses all items in the view
    m_curResults->matchModel.setSearchState(MatchModel::SearchDone);

    // expand the "header item " to display all files and all results if configured
    expandResults();

    indicateMatch(m_curResults->matches > 0);

    m_curResults = nullptr;

    if (popupVisible) {
        m_ui.searchCombo->lineEdit()->completer()->complete();
    }
    m_searchJustOpened = false;
    updateMatchMarks();
}

void KatePluginSearchView::indicateMatch(bool hasMatch)
{
    QLineEdit *const lineEdit = m_ui.searchCombo->lineEdit();
    QPalette background(lineEdit->palette());

    if (hasMatch) {
        // Green background for line edit
        KColorScheme::adjustBackground(background, KColorScheme::PositiveBackground);
    } else {
        // Reset background of line edit
        background = QPalette();
    }
    // Red background for line edit
    // KColorScheme::adjustBackground(background, KColorScheme::NegativeBackground);
    // Neutral background
    // KColorScheme::adjustBackground(background, KColorScheme::NeutralBackground);

    lineEdit->setPalette(background);
}

void KatePluginSearchView::replaceSingleMatch()
{
    // Save the search text
    if (m_ui.searchCombo->findText(m_ui.searchCombo->currentText()) == -1) {
        m_ui.searchCombo->insertItem(1, m_ui.searchCombo->currentText());
        m_ui.searchCombo->setCurrentIndex(1);
    }

    // Save the replace text
    if (m_ui.replaceCombo->findText(m_ui.replaceCombo->currentText()) == -1) {
        m_ui.replaceCombo->insertItem(1, m_ui.replaceCombo->currentText());
        m_ui.replaceCombo->setCurrentIndex(1);
    }

    // Check if the cursor is at the current item if not jump there
    Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (!res) {
        return; // Security measure
    }

    QModelIndex itemIndex = res->treeView->currentIndex();
    if (!res->matchModel.isMatch(itemIndex)) {
        goToNextMatch();
    }

    if (!m_mainWindow->activeView() || !m_mainWindow->activeView()->cursorPosition().isValid()) {
        itemSelected(itemIndex); // Correct any bad cursor positions
        return;
    }

    KTextEditor::Range matchRange = res->matchModel.matchRange(itemIndex);

    if (m_mainWindow->activeView()->cursorPosition() != matchRange.start()) {
        itemSelected(itemIndex);
        return;
    }

    Q_EMIT searchBusy(true);

    KTextEditor::Document *doc = m_mainWindow->activeView()->document();

    // FIXME The document might have been edited after the search.
    // Fix the ranges before attempting the replace
    res->matchModel.replaceSingleMatch(doc, itemIndex, res->regExp, m_ui.replaceCombo->currentText());

    goToNextMatch();
}

void KatePluginSearchView::replaceChecked()
{
    // Sync the current documents ranges with the model in case it has been edited
    syncModelRanges();

    if (m_ui.searchCombo->findText(m_ui.searchCombo->currentText()) == -1) {
        m_ui.searchCombo->insertItem(1, m_ui.searchCombo->currentText());
        m_ui.searchCombo->setCurrentIndex(1);
    }

    if (m_ui.replaceCombo->findText(m_ui.replaceCombo->currentText()) == -1) {
        m_ui.replaceCombo->insertItem(1, m_ui.replaceCombo->currentText());
        m_ui.replaceCombo->setCurrentIndex(1);
    }

    m_curResults = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (!m_curResults) {
        qWarning() << "Results not found";
        return;
    }

    Q_EMIT searchBusy(true);

    m_ui.stopAndNext->setCurrentWidget(m_ui.stopButton);
    m_ui.displayOptions->setChecked(false);
    m_ui.displayOptions->setDisabled(true);
    m_ui.newTabButton->setDisabled(true);
    m_ui.searchCombo->setDisabled(true);
    m_ui.searchButton->setDisabled(true);
    m_ui.replaceCheckedBtn->setDisabled(true);
    m_ui.replaceButton->setDisabled(true);
    m_ui.replaceCombo->setDisabled(true);
    m_ui.searchPlaceCombo->setDisabled(true);
    m_ui.useRegExp->setDisabled(true);
    m_ui.matchCase->setDisabled(true);
    m_ui.expandResults->setDisabled(true);
    m_ui.currentFolderButton->setDisabled(true);

    m_curResults->replaceStr = m_ui.replaceCombo->currentText();

    m_curResults->matchModel.replaceChecked(m_curResults->regExp, m_curResults->replaceStr);
}

void KatePluginSearchView::replaceDone()
{
    m_ui.stopAndNext->setCurrentWidget(m_ui.nextButton);
    m_ui.replaceCombo->setDisabled(false);
    m_ui.newTabButton->setDisabled(false);
    m_ui.searchCombo->setDisabled(false);
    m_ui.searchButton->setDisabled(false);
    m_ui.replaceCheckedBtn->setDisabled(false);
    m_ui.replaceButton->setDisabled(false);
    m_ui.displayOptions->setDisabled(false);
    m_ui.searchPlaceCombo->setDisabled(false);
    m_ui.useRegExp->setDisabled(false);
    m_ui.matchCase->setDisabled(false);
    m_ui.expandResults->setDisabled(false);
    m_ui.currentFolderButton->setDisabled(false);
    updateMatchMarks();

    Q_EMIT searchBusy(false);
}

/** Remove all moving ranges and document marks belonging to Search & Replace */
void KatePluginSearchView::clearMarksAndRanges()
{
    // If we have a MovingRange we have a corresponding MatchMark
    while (!m_matchRanges.isEmpty()) {
        clearDocMarksAndRanges(m_matchRanges.first()->document());
    }
}

void KatePluginSearchView::clearDocMarksAndRanges(KTextEditor::Document *doc)
{
    // Before removing the ranges try to update the ranges in the model in case we have document changes.
    syncModelRanges();

    KTextEditor::MarkInterface *iface;
    iface = qobject_cast<KTextEditor::MarkInterface *>(doc);
    if (iface) {
        const QHash<int, KTextEditor::Mark *> marks = iface->marks();
        QHashIterator<int, KTextEditor::Mark *> i(marks);
        while (i.hasNext()) {
            i.next();
            if (i.value()->type & KTextEditor::MarkInterface::markType32) {
                iface->removeMark(i.value()->line, KTextEditor::MarkInterface::markType32);
            }
        }
    }

    m_matchRanges.erase(std::remove_if(m_matchRanges.begin(),
                                       m_matchRanges.end(),
                                       [doc](KTextEditor::MovingRange *r) {
                                           if (r->document() == doc) {
                                               delete r;
                                               return true;
                                           }
                                           return false;
                                       }),
                        m_matchRanges.end());
}

void KatePluginSearchView::addRangeAndMark(KTextEditor::Document *doc,
                                           const KateSearchMatch &match,
                                           KTextEditor::Attribute::Ptr attr,
                                           KTextEditor::MovingInterface *miface)
{
    if (!doc || !match.checked) {
        return;
    }

    bool isReplaced = !match.replaceText.isEmpty();

    // Check that the match still matches
    if (m_curResults) {
        if (!isReplaced) {
            // special handling for "(?=\\n)" in multi-line search
            QRegularExpression tmpReg = m_curResults->regExp;
            if (m_curResults->regExp.pattern().endsWith(QLatin1String("(?=\\n)"))) {
                QString newPatern = tmpReg.pattern();
                newPatern.replace(QStringLiteral("(?=\\n)"), QStringLiteral("$"));
                tmpReg.setPattern(newPatern);
            }
            // Check that the match still matches ;)
            if (tmpReg.match(doc->text(match.range)).capturedStart() != 0) {
                // qDebug() << doc->text(range) << "Does not match" << m_curResults->regExp.pattern();
                return;
            }
        } else {
            if (doc->text(match.range) != match.replaceText) {
                /// qDebug() << doc->text(range) << "Does not match" << itemIndex.data(MatchModel::ReplaceTextRole).toString();
                return;
            }
        }
    }

    // Highlight the match
    if (isReplaced) {
        attr->setBackground(m_replaceHighlightColor);
    }

    KTextEditor::MovingRange *mr = miface->newMovingRange(match.range);
    mr->setZDepth(-90000.0); // Set the z-depth to slightly worse than the selection
    mr->setAttribute(attr);
    mr->setAttributeOnlyForViews(true);
    m_matchRanges.append(mr);

    // Add a match mark
    KTextEditor::MarkInterfaceV2 *iface = qobject_cast<KTextEditor::MarkInterfaceV2 *>(doc);
    if (!iface) {
        return;
    }
    static const auto description = i18n("Search Match");
    iface->setMarkDescription(KTextEditor::MarkInterface::markType32, description);
    iface->setMarkIcon(KTextEditor::MarkInterface::markType32, QIcon());
    iface->addMark(match.range.start().line(), KTextEditor::MarkInterface::markType32);
}

void KatePluginSearchView::updateCheckState(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);

    // check tailored to the way signal is raised by the model
    // keep the check simple in case each one is one of many
    if (roles.size() == 0 || roles.size() > 1 || roles[0] != Qt::CheckStateRole) {
        return;
    }
    // more updates might follow, let's batch those
    if (!m_updateCheckedStateTimer.isActive()) {
        m_updateCheckedStateTimer.start();
    }
}

void KatePluginSearchView::updateMatchMarks()
{
    // We only keep marks & ranges for one document at a time so clear the rest
    // This will also update the model ranges corresponding to the cleared ranges.
    clearMarksAndRanges();

    if (!m_mainWindow->activeView()) {
        return;
    }

    Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (!res) {
        return;
    }
    m_curResults = res;

    // add the marks if it is not already open
    KTextEditor::Document *doc = m_mainWindow->activeView()->document();
    if (!doc) {
        return;
    }

    // clang-format off
    connect(doc, SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)), this, SLOT(clearMarksAndRanges()), Qt::UniqueConnection);
    // clang-format on
    // Re-add the highlighting on document reload
    connect(doc, &KTextEditor::Document::reloaded, this, &KatePluginSearchView::updateMatchMarks, Qt::UniqueConnection);
    // Re-do highlight upon check mark update
    connect(&res->matchModel, &QAbstractItemModel::dataChanged, this, &KatePluginSearchView::updateCheckState, Qt::UniqueConnection);

    KTextEditor::MovingInterface *miface = qobject_cast<KTextEditor::MovingInterface *>(doc);

    // Add match marks for all matches in the file
    const QVector<KateSearchMatch> &fileMatches = res->matchModel.fileMatches(doc->url());
    for (const KateSearchMatch &match : fileMatches) {
        addRangeAndMark(doc, match, m_resultAttr, miface);
    }
}

void KatePluginSearchView::syncModelRanges()
{
    if (!m_curResults) {
        return;
    }
    // NOTE: We assume there are only ranges for one document in the ranges at a time...
    m_curResults->matchModel.updateMatchRanges(m_matchRanges);
}

void KatePluginSearchView::expandResults()
{
    m_curResults = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (!m_curResults) {
        qWarning() << "Results not found";
        return;
    }

    // we expand recursively if we either are told so or we have just one toplevel match item
    QModelIndex rootItem = m_curResults->matchModel.index(0, 0);
    if ((m_ui.expandResults->isChecked() && m_curResults->matchModel.rowCount(rootItem) < 200) || m_curResults->matchModel.rowCount(rootItem) == 1) {
        m_curResults->treeView->expandAll();
    } else {
        // first collapse all and the expand the root, much faster than collapsing all children manually
        m_curResults->treeView->collapseAll();
        m_curResults->treeView->expand(rootItem);
    }
}

void KatePluginSearchView::itemSelected(const QModelIndex &item)
{
    m_curResults = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (!m_curResults) {
        qDebug() << "No result widget available";
        return;
    }

    // Sync the current document matches with the model before jumping
    // FIXME do we want to do this on every edit in stead?
    syncModelRanges();

    // open any children to go to the first match in the file
    QModelIndex matchItem = item;
    while (m_curResults->matchModel.hasChildren(matchItem)) {
        matchItem = m_curResults->matchModel.index(0, 0, matchItem);
    }

    m_curResults->treeView->setCurrentIndex(matchItem);

    // get stuff
    int toLine = matchItem.data(MatchModel::StartLineRole).toInt();
    int toColumn = matchItem.data(MatchModel::StartColumnRole).toInt();
    QUrl url = matchItem.data(MatchModel::FileUrlRole).toUrl();
    KTextEditor::Document *doc = m_kateApp->findUrl(url);

    // add the marks to the document if it is not already open
    if (!doc) {
        doc = m_kateApp->openUrl(url);
    }
    if (!doc) {
        qDebug() << "Could not open" << url;
        Q_ASSERT(false); // If we get here we have a bug
        return;
    }

    // open the right view...
    m_mainWindow->activateView(doc);

    // any view active?
    if (!m_mainWindow->activeView()) {
        qDebug() << "Could not activate view for:" << url;
        Q_ASSERT(false);
        return;
    }

    // set the cursor to the correct position
    m_mainWindow->activeView()->setCursorPosition(KTextEditor::Cursor(toLine, toColumn));
    m_mainWindow->activeView()->setFocus();
}

void KatePluginSearchView::goToNextMatch()
{
    Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (!res) {
        return;
    }
    m_curResults = res;

    m_ui.displayOptions->setChecked(false);

    QModelIndex currentIndex = res->treeView->currentIndex();
    bool focusInView = m_mainWindow->activeView() && m_mainWindow->activeView()->hasFocus();

    if (!currentIndex.isValid() && focusInView) {
        // no item has been visited && focus is not in searchCombo (probably in the view) ->
        // jump to the closest match after current cursor position
        QUrl docUrl = m_mainWindow->activeView()->document()->url();

        // check if current file is in the file list
        currentIndex = res->matchModel.firstFileMatch(docUrl);
        if (currentIndex.isValid()) {
            // We have the index of the first match in the file
            // expand the file item
            res->treeView->expand(currentIndex.parent());

            // check if we can get the next match after the
            currentIndex = res->matchModel.closestMatchAfter(docUrl, m_mainWindow->activeView()->cursorPosition());
            if (currentIndex.isValid()) {
                itemSelected(currentIndex);
                delete m_infoMessage;
                const QString msg = i18n("Next from cursor");
                m_infoMessage = new KTextEditor::Message(msg, KTextEditor::Message::Information);
                m_infoMessage->setPosition(KTextEditor::Message::BottomInView);
                m_infoMessage->setAutoHide(2000);
                m_infoMessage->setAutoHideMode(KTextEditor::Message::Immediate);
                m_infoMessage->setView(m_mainWindow->activeView());
                m_mainWindow->activeView()->document()->postMessage(m_infoMessage);
                return;
            }
        }
    }

    if (!currentIndex.isValid()) {
        currentIndex = res->matchModel.firstMatch();
        if (currentIndex.isValid()) {
            itemSelected(currentIndex);
            delete m_infoMessage;
            const QString msg = i18n("Starting from first match");
            m_infoMessage = new KTextEditor::Message(msg, KTextEditor::Message::Information);
            m_infoMessage->setPosition(KTextEditor::Message::TopInView);
            m_infoMessage->setAutoHide(2000);
            m_infoMessage->setAutoHideMode(KTextEditor::Message::Immediate);
            m_infoMessage->setView(m_mainWindow->activeView());
            m_mainWindow->activeView()->document()->postMessage(m_infoMessage);
            return;
        }
    }
    if (!currentIndex.isValid()) {
        // no matches to activate
        return;
    }

    // we had an active item go to next
    currentIndex = res->matchModel.nextMatch(currentIndex);
    itemSelected(currentIndex);
    if (currentIndex == res->matchModel.firstMatch()) {
        delete m_infoMessage;
        const QString msg = i18n("Continuing from first match");
        m_infoMessage = new KTextEditor::Message(msg, KTextEditor::Message::Information);
        m_infoMessage->setPosition(KTextEditor::Message::TopInView);
        m_infoMessage->setAutoHide(2000);
        m_infoMessage->setAutoHideMode(KTextEditor::Message::Immediate);
        m_infoMessage->setView(m_mainWindow->activeView());
        m_mainWindow->activeView()->document()->postMessage(m_infoMessage);
    }
}

void KatePluginSearchView::goToPreviousMatch()
{
    Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (!res) {
        return;
    }
    m_curResults = res;

    m_ui.displayOptions->setChecked(false);

    QModelIndex currentIndex = res->treeView->currentIndex();
    bool focusInView = m_mainWindow->activeView() && m_mainWindow->activeView()->hasFocus();

    if (!currentIndex.isValid() && focusInView) {
        // no item has been visited && focus is not in the view ->
        // jump to the closest match before current cursor position
        QUrl docUrl = m_mainWindow->activeView()->document()->url();

        // check if current file is in the file list
        currentIndex = res->matchModel.firstFileMatch(docUrl);
        if (currentIndex.isValid()) {
            // We have the index of the first match in the file
            // expand the file item
            res->treeView->expand(currentIndex.parent());

            // check if we can get the next match after the
            currentIndex = res->matchModel.closestMatchBefore(docUrl, m_mainWindow->activeView()->cursorPosition());
            if (currentIndex.isValid()) {
                itemSelected(currentIndex);
                delete m_infoMessage;
                const QString msg = i18n("Next from cursor");
                m_infoMessage = new KTextEditor::Message(msg, KTextEditor::Message::Information);
                m_infoMessage->setPosition(KTextEditor::Message::BottomInView);
                m_infoMessage->setAutoHide(2000);
                m_infoMessage->setAutoHideMode(KTextEditor::Message::Immediate);
                m_infoMessage->setView(m_mainWindow->activeView());
                m_mainWindow->activeView()->document()->postMessage(m_infoMessage);
                return;
            }
        }
    }

    if (!currentIndex.isValid()) {
        currentIndex = res->matchModel.lastMatch();
        if (currentIndex.isValid()) {
            itemSelected(currentIndex);
            delete m_infoMessage;
            const QString msg = i18n("Starting from last match");
            m_infoMessage = new KTextEditor::Message(msg, KTextEditor::Message::Information);
            m_infoMessage->setPosition(KTextEditor::Message::TopInView);
            m_infoMessage->setAutoHide(2000);
            m_infoMessage->setAutoHideMode(KTextEditor::Message::Immediate);
            m_infoMessage->setView(m_mainWindow->activeView());
            m_mainWindow->activeView()->document()->postMessage(m_infoMessage);
            return;
        }
    }
    if (!currentIndex.isValid()) {
        // no matches to activate
        return;
    }

    // we had an active item go to next
    currentIndex = res->matchModel.prevMatch(currentIndex);
    itemSelected(currentIndex);
    if (currentIndex == res->matchModel.lastMatch()) {
        delete m_infoMessage;
        const QString msg = i18n("Continuing from last match");
        m_infoMessage = new KTextEditor::Message(msg, KTextEditor::Message::Information);
        m_infoMessage->setPosition(KTextEditor::Message::TopInView);
        m_infoMessage->setAutoHide(2000);
        m_infoMessage->setAutoHideMode(KTextEditor::Message::Immediate);
        m_infoMessage->setView(m_mainWindow->activeView());
        m_mainWindow->activeView()->document()->postMessage(m_infoMessage);
    }
}

void KatePluginSearchView::setRegexMode(bool enabled)
{
    m_ui.useRegExp->setChecked(enabled);
}

void KatePluginSearchView::setCaseInsensitive(bool enabled)
{
    m_ui.matchCase->setChecked(enabled);
}

void KatePluginSearchView::setExpandResults(bool enabled)
{
    m_ui.expandResults->setChecked(enabled);
}

void KatePluginSearchView::readSessionConfig(const KConfigGroup &cg)
{
    m_ui.searchCombo->clear();
    m_ui.searchCombo->addItem(QString()); // Add empty Item
    m_ui.searchCombo->addItems(cg.readEntry("Search", QStringList()));
    m_ui.replaceCombo->clear();
    m_ui.replaceCombo->addItem(QString()); // Add empty Item
    m_ui.replaceCombo->addItems(cg.readEntry("Replaces", QStringList()));
    m_ui.matchCase->setChecked(cg.readEntry("MatchCase", false));
    m_ui.useRegExp->setChecked(cg.readEntry("UseRegExp", false));
    m_ui.expandResults->setChecked(cg.readEntry("ExpandSearchResults", false));

    int searchPlaceIndex = cg.readEntry("Place", 1);
    if (searchPlaceIndex < 0) {
        searchPlaceIndex = MatchModel::Folder; // for the case we happen to read -1 as Place
    }
    if ((searchPlaceIndex >= MatchModel::Project) && (searchPlaceIndex >= m_ui.searchPlaceCombo->count())) {
        // handle the case that project mode was selected, but not yet available
        m_projectSearchPlaceIndex = searchPlaceIndex;
        searchPlaceIndex = MatchModel::Folder;
    }
    m_ui.searchPlaceCombo->setCurrentIndex(searchPlaceIndex);

    m_ui.recursiveCheckBox->setChecked(cg.readEntry("Recursive", true));
    m_ui.hiddenCheckBox->setChecked(cg.readEntry("HiddenFiles", false));
    m_ui.symLinkCheckBox->setChecked(cg.readEntry("FollowSymLink", false));
    m_ui.binaryCheckBox->setChecked(cg.readEntry("BinaryFiles", false));
    m_ui.folderRequester->comboBox()->clear();
    m_ui.folderRequester->comboBox()->addItems(cg.readEntry("SearchDiskFiless", QStringList()));
    m_ui.folderRequester->setText(cg.readEntry("SearchDiskFiles", QString()));
    m_ui.filterCombo->clear();
    m_ui.filterCombo->addItems(cg.readEntry("Filters", QStringList()));
    m_ui.filterCombo->setCurrentIndex(cg.readEntry("CurrentFilter", -1));
    m_ui.excludeCombo->clear();
    m_ui.excludeCombo->addItems(cg.readEntry("ExcludeFilters", QStringList()));
    m_ui.excludeCombo->setCurrentIndex(cg.readEntry("CurrentExcludeFilter", -1));
    m_ui.displayOptions->setChecked(searchPlaceIndex == MatchModel::Folder);
}

void KatePluginSearchView::writeSessionConfig(KConfigGroup &cg)
{
    QStringList searchHistoy;
    for (int i = 1; i < m_ui.searchCombo->count(); i++) {
        searchHistoy << m_ui.searchCombo->itemText(i);
    }
    cg.writeEntry("Search", searchHistoy);
    QStringList replaceHistoy;
    for (int i = 1; i < m_ui.replaceCombo->count(); i++) {
        replaceHistoy << m_ui.replaceCombo->itemText(i);
    }
    cg.writeEntry("Replaces", replaceHistoy);

    cg.writeEntry("MatchCase", m_ui.matchCase->isChecked());
    cg.writeEntry("UseRegExp", m_ui.useRegExp->isChecked());
    cg.writeEntry("ExpandSearchResults", m_ui.expandResults->isChecked());

    cg.writeEntry("Place", m_ui.searchPlaceCombo->currentIndex());
    cg.writeEntry("Recursive", m_ui.recursiveCheckBox->isChecked());
    cg.writeEntry("HiddenFiles", m_ui.hiddenCheckBox->isChecked());
    cg.writeEntry("FollowSymLink", m_ui.symLinkCheckBox->isChecked());
    cg.writeEntry("BinaryFiles", m_ui.binaryCheckBox->isChecked());
    QStringList folders;
    for (int i = 0; i < qMin(m_ui.folderRequester->comboBox()->count(), 10); i++) {
        folders << m_ui.folderRequester->comboBox()->itemText(i);
    }
    cg.writeEntry("SearchDiskFiless", folders);
    cg.writeEntry("SearchDiskFiles", m_ui.folderRequester->text());
    QStringList filterItems;
    for (int i = 0; i < qMin(m_ui.filterCombo->count(), 10); i++) {
        filterItems << m_ui.filterCombo->itemText(i);
    }
    cg.writeEntry("Filters", filterItems);
    cg.writeEntry("CurrentFilter", m_ui.filterCombo->findText(m_ui.filterCombo->currentText()));

    QStringList excludeFilterItems;
    for (int i = 0; i < qMin(m_ui.excludeCombo->count(), 10); i++) {
        excludeFilterItems << m_ui.excludeCombo->itemText(i);
    }
    cg.writeEntry("ExcludeFilters", excludeFilterItems);
    cg.writeEntry("CurrentExcludeFilter", m_ui.excludeCombo->findText(m_ui.excludeCombo->currentText()));
}

void KatePluginSearchView::addTab()
{
    if ((sender() != m_ui.newTabButton) && (m_ui.resultTabWidget->count() > 0)
        && m_ui.resultTabWidget->tabText(m_ui.resultTabWidget->currentIndex()).isEmpty()) {
        return;
    }

    Results *res = new Results();

    connect(res, &Results::colorsChanged, this, [this]() {
        updateViewColors();
    });

    res->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    res->treeView->setRootIsDecorated(false);
    connect(res->treeView, &QTreeView::doubleClicked, this, &KatePluginSearchView::itemSelected, Qt::UniqueConnection);
    connect(res->treeView, &QTreeView::customContextMenuRequested, this, &KatePluginSearchView::customResMenuRequested, Qt::UniqueConnection);
    res->matchModel.setDocumentManager(m_kateApp);
    connect(&res->matchModel, &MatchModel::replaceDone, this, &KatePluginSearchView::replaceDone);

    res->searchPlaceIndex = m_ui.searchPlaceCombo->currentIndex();
    res->useRegExp = m_ui.useRegExp->isChecked();
    res->matchCase = m_ui.matchCase->isChecked();
    m_ui.resultTabWidget->addTab(res, QString());
    m_ui.resultTabWidget->setCurrentIndex(m_ui.resultTabWidget->count() - 1);
    m_ui.stackedWidget->setCurrentIndex(0);
    m_ui.displayOptions->setChecked(false);

    res->treeView->installEventFilter(this);
}

void KatePluginSearchView::tabCloseRequested(int index)
{
    Results *tmp = qobject_cast<Results *>(m_ui.resultTabWidget->widget(index));
    if (m_curResults == tmp) {
        m_searchOpenFiles.cancelSearch();
        cancelDiskFileSearch();
        m_folderFilesList.terminateSearch();
    }

    if (m_ui.resultTabWidget->count() > 1) {
        m_ui.resultTabWidget->removeTab(index);
        if (m_curResults == tmp) {
            delete m_curResults;
            m_curResults = nullptr;
        } else {
            delete tmp;
        }
    }

    updateMatchMarks();
}

void KatePluginSearchView::resultTabChanged(int index)
{
    if (index < 0) {
        return;
    }

    Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->widget(index));
    if (!res) {
        // qDebug() << "No res found";
        return;
    }

    m_ui.searchCombo->blockSignals(true);
    m_ui.matchCase->blockSignals(true);
    m_ui.useRegExp->blockSignals(true);
    m_ui.searchPlaceCombo->blockSignals(true);
    m_ui.searchCombo->lineEdit()->setText(m_ui.resultTabWidget->tabText(index));
    m_ui.useRegExp->setChecked(res->useRegExp);
    m_ui.matchCase->setChecked(res->matchCase);
    m_ui.searchPlaceCombo->setCurrentIndex(res->searchPlaceIndex);
    m_ui.searchCombo->blockSignals(false);
    m_ui.matchCase->blockSignals(false);
    m_ui.useRegExp->blockSignals(false);
    m_ui.searchPlaceCombo->blockSignals(false);
    searchPlaceChanged();
    updateMatchMarks();
}

void KatePluginSearchView::onResize(const QSize &size)
{
    bool vertical = size.width() < size.height();

    if (!m_isVerticalLayout && vertical) {
        // Change the layout to vertical (left/right)
        m_isVerticalLayout = true;

        // Search row 1
        m_ui.gridLayout->addWidget(m_ui.searchCombo, 0, 0, 1, 5);
        // Search row 2
        m_ui.gridLayout->addWidget(m_ui.searchButton, 1, 0);
        m_ui.gridLayout->addWidget(m_ui.stopAndNext, 1, 1);
        m_ui.gridLayout->addWidget(m_ui.searchPlaceLayoutW, 1, 2, 1, 3);

        // Replace row 1
        m_ui.gridLayout->addWidget(m_ui.replaceCombo, 2, 0, 1, 5);

        // Replace row 2
        m_ui.gridLayout->addWidget(m_ui.replaceButton, 3, 0);
        m_ui.gridLayout->addWidget(m_ui.replaceCheckedBtn, 3, 1);
        m_ui.gridLayout->addWidget(m_ui.searchOptionsLayoutW, 3, 2);
        m_ui.gridLayout->addWidget(m_ui.newTabButton, 3, 3);
        m_ui.gridLayout->addWidget(m_ui.displayOptions, 3, 4);

        m_ui.gridLayout->setColumnStretch(0, 0);
        m_ui.gridLayout->setColumnStretch(2, 4);

    } else if (m_isVerticalLayout && !vertical) {
        // Change layout to horizontal (top/bottom)
        m_isVerticalLayout = false;
        // Top row
        m_ui.gridLayout->addWidget(m_ui.searchCombo, 0, 0);
        m_ui.gridLayout->addWidget(m_ui.searchButton, 0, 1);
        m_ui.gridLayout->addWidget(m_ui.stopAndNext, 0, 2);
        m_ui.gridLayout->addWidget(m_ui.searchPlaceLayoutW, 0, 3, 1, 3);

        // Second row
        m_ui.gridLayout->addWidget(m_ui.replaceCombo, 1, 0);
        m_ui.gridLayout->addWidget(m_ui.replaceButton, 1, 1);
        m_ui.gridLayout->addWidget(m_ui.replaceCheckedBtn, 1, 2);
        m_ui.gridLayout->addWidget(m_ui.searchOptionsLayoutW, 1, 3);
        m_ui.gridLayout->addWidget(m_ui.newTabButton, 1, 4);
        m_ui.gridLayout->addWidget(m_ui.displayOptions, 1, 5);

        m_ui.gridLayout->setColumnStretch(0, 4);
        m_ui.gridLayout->setColumnStretch(2, 0);
    }
}

void KatePluginSearchView::customResMenuRequested(const QPoint &pos)
{
    QTreeView *tree = qobject_cast<QTreeView *>(sender());
    if (tree == nullptr) {
        return;
    }
    QMenu *menu = new QMenu(tree);

    QAction *copyAll = new QAction(i18n("Copy all"), tree);
    copyAll->setShortcut(QKeySequence::Copy);
    copyAll->setShortcutVisibleInContextMenu(true);
    menu->addAction(copyAll);

    QAction *copyExpanded = new QAction(i18n("Copy expanded"), tree);
    menu->addAction(copyExpanded);

    menu->popup(tree->viewport()->mapToGlobal(pos));

    connect(copyAll, &QAction::triggered, this, [this](bool) {
        copySearchToClipboard(All);
    });
    connect(copyExpanded, &QAction::triggered, this, [this](bool) {
        copySearchToClipboard(AllExpanded);
    });
}

void KatePluginSearchView::copySearchToClipboard(CopyResultType copyType)
{
    Results *res = qobject_cast<Results *>(m_ui.resultTabWidget->currentWidget());
    if (!res) {
        return;
    }
    if (res->matchModel.rowCount() == 0) {
        return;
    }

    QString clipboard;

    QModelIndex rootIndex = res->matchModel.index(0, 0);

    clipboard = rootIndex.data(MatchModel::PlainTextRole).toString();

    int fileCount = res->matchModel.rowCount(rootIndex);
    for (int i = 0; i < fileCount; ++i) {
        QModelIndex fileIndex = res->matchModel.index(i, 0, rootIndex);
        if (res->treeView->isExpanded(fileIndex) || copyType == All) {
            clipboard += QLatin1String("\n") + fileIndex.data(MatchModel::PlainTextRole).toString();
            int matchCount = res->matchModel.rowCount(fileIndex);
            for (int j = 0; j < matchCount; ++j) {
                QModelIndex matchIndex = res->matchModel.index(j, 0, fileIndex);
                clipboard += QLatin1String("\n") + matchIndex.data(MatchModel::PlainTextRole).toString();
            }
        }
    }
    clipboard += QLatin1String("\n");
    QApplication::clipboard()->setText(clipboard);
}

bool KatePluginSearchView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        // Ignore copy in ShortcutOverride and handle it in the KeyPress event
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->matches(QKeySequence::Copy)) {
            event->accept();
            return true;
        }
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        QTreeView *treeView = qobject_cast<QTreeView *>(obj);
        if (treeView) {
            if (ke->matches(QKeySequence::Copy)) {
                copySearchToClipboard(All);
                event->accept();
                return true;
            }
            if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) {
                if (treeView->currentIndex().isValid()) {
                    itemSelected(treeView->currentIndex());
                    event->accept();
                    return true;
                }
            }
        }
        // NOTE: Qt::Key_Escape is handled by handleEsc
    } else if (event->type() == QEvent::Resize) {
        QResizeEvent *re = static_cast<QResizeEvent *>(event);
        if (obj == m_toolView) {
            onResize(re->size());
        }
    }
    return QObject::eventFilter(obj, event);
}

void KatePluginSearchView::searchContextMenu(const QPoint &pos)
{
    QSet<QAction *> actionPointers;

    QMenu *const contextMenu = m_ui.searchCombo->lineEdit()->createStandardContextMenu();
    if (!contextMenu) {
        return;
    }

    if (m_ui.useRegExp->isChecked()) {
        QMenu *menu = contextMenu->addMenu(i18n("Add..."));
        if (!menu) {
            return;
        }

        menu->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

        addRegexHelperActionsForSearch(&actionPointers, menu);
    }

    // Show menu and act
    QAction *const result = contextMenu->exec(m_ui.searchCombo->mapToGlobal(pos));
    regexHelperActOnAction(result, actionPointers, m_ui.searchCombo->lineEdit());
}

void KatePluginSearchView::replaceContextMenu(const QPoint &pos)
{
    QMenu *const contextMenu = m_ui.replaceCombo->lineEdit()->createStandardContextMenu();
    if (!contextMenu) {
        return;
    }

    QMenu *menu = contextMenu->addMenu(i18n("Add..."));
    if (!menu) {
        return;
    }
    menu->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

    QSet<QAction *> actionPointers;
    addSpecialCharsHelperActionsForReplace(&actionPointers, menu);

    if (m_ui.useRegExp->isChecked()) {
        addRegexHelperActionsForReplace(&actionPointers, menu);
    }

    // Show menu and act
    QAction *const result = contextMenu->exec(m_ui.replaceCombo->mapToGlobal(pos));
    regexHelperActOnAction(result, actionPointers, m_ui.replaceCombo->lineEdit());
}

void KatePluginSearchView::slotPluginViewCreated(const QString &name, QObject *pluginView)
{
    // add view
    if (pluginView && name == QLatin1String("kateprojectplugin")) {
        m_projectPluginView = pluginView;
        slotProjectFileNameChanged();
        connect(pluginView, SIGNAL(projectFileNameChanged()), this, SLOT(slotProjectFileNameChanged()));
    }
}

void KatePluginSearchView::slotPluginViewDeleted(const QString &name, QObject *)
{
    // remove view
    if (name == QLatin1String("kateprojectplugin")) {
        m_projectPluginView = nullptr;
        slotProjectFileNameChanged();
    }
}

void KatePluginSearchView::slotProjectFileNameChanged()
{
    // query new project file name
    QString projectFileName;
    if (m_projectPluginView) {
        projectFileName = m_projectPluginView->property("projectFileName").toString();
    }

    // have project, enable gui for it
    if (!projectFileName.isEmpty()) {
        if (m_ui.searchPlaceCombo->count() <= MatchModel::Project) {
            // add "in Project"
            m_ui.searchPlaceCombo->addItem(QIcon::fromTheme(QStringLiteral("project-open")), i18n("In Current Project"));
            // add "in Open Projects"
            m_ui.searchPlaceCombo->addItem(QIcon::fromTheme(QStringLiteral("project-open")), i18n("In All Open Projects"));
            if (m_projectSearchPlaceIndex >= MatchModel::Project) {
                // switch to search "in (all) Project"
                setSearchPlace(m_projectSearchPlaceIndex);
                m_projectSearchPlaceIndex = 0;
            }
        }
    }

    // else: disable gui for it
    else {
        if (m_ui.searchPlaceCombo->count() >= MatchModel::Project) {
            // switch to search "in Open files", if "in Project" is active
            int searchPlaceIndex = m_ui.searchPlaceCombo->currentIndex();
            if (searchPlaceIndex >= MatchModel::Project) {
                m_projectSearchPlaceIndex = searchPlaceIndex;
                setSearchPlace(MatchModel::OpenFiles);
            }

            // remove "in Project" and "in all projects"
            while (m_ui.searchPlaceCombo->count() > MatchModel::Project) {
                m_ui.searchPlaceCombo->removeItem(m_ui.searchPlaceCombo->count() - 1);
            }
        }
    }
}

#include "plugin_search.moc"

// kate: space-indent on; indent-width 4; replace-tabs on;
