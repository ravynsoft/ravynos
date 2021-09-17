/* Description : Kate CTags plugin
 *
 * SPDX-FileCopyrightText: 2008-2011 Kare Sars <kare.sars@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kate_ctags_view.h"
#include "kate_ctags_debug.h"
#include "kate_ctags_plugin.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>

#include <KActionCollection>
#include <KConfigGroup>
#include <KXMLGUIFactory>
#include <QMenu>

#include <KLocalizedString>
#include <KMessageBox>
#include <KStringHandler>
#include <QStandardPaths>

/******************************************************************/
KateCTagsView::KateCTagsView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainWin)
    : QObject(mainWin)
    , m_proc(nullptr)
{
    KXMLGUIClient::setComponentName(QStringLiteral("katectags"), i18n("Kate CTags"));
    setXMLFile(QStringLiteral("ui.rc"));

    m_toolView = mainWin->createToolView(plugin,
                                         QStringLiteral("kate_plugin_katectagsplugin"),
                                         KTextEditor::MainWindow::Bottom,
                                         QIcon::fromTheme(QStringLiteral("application-x-ms-dos-executable")),
                                         i18n("CTags"));
    m_mWin = mainWin;

    QAction *back = actionCollection()->addAction(QStringLiteral("ctags_return_step"));
    back->setText(i18n("Jump back one step"));
    connect(back, &QAction::triggered, this, &KateCTagsView::stepBack);

    QAction *decl = actionCollection()->addAction(QStringLiteral("ctags_lookup_current_as_declaration"));
    decl->setText(i18n("Go to Declaration"));
    connect(decl, &QAction::triggered, this, &KateCTagsView::gotoDeclaration);

    QAction *defin = actionCollection()->addAction(QStringLiteral("ctags_lookup_current_as_definition"));
    defin->setText(i18n("Go to Definition"));
    connect(defin, &QAction::triggered, this, &KateCTagsView::gotoDefinition);

    QAction *lookup = actionCollection()->addAction(QStringLiteral("ctags_lookup_current"));
    lookup->setText(i18n("Lookup Current Text"));
    connect(lookup, &QAction::triggered, this, &KateCTagsView::lookupTag);

    QAction *updateDB = actionCollection()->addAction(QStringLiteral("ctags_update_global_db"));
    updateDB->setText(i18n("Configure ..."));
    connect(updateDB, &QAction::triggered, this, [this, plugin](bool) {
        if (m_mWin) {
            KateCTagsPlugin *p = static_cast<KateCTagsPlugin *>(plugin);
            QDialog *confWin = new QDialog(m_mWin->window());
            confWin->setAttribute(Qt::WA_DeleteOnClose);
            auto confPage = p->configPage(0, confWin);
            auto controls = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, confWin);
            connect(confWin, &QDialog::accepted, confPage, &KTextEditor::ConfigPage::apply);
            connect(controls, &QDialogButtonBox::accepted, confWin, &QDialog::accept);
            connect(controls, &QDialogButtonBox::rejected, confWin, &QDialog::reject);
            auto layout = new QVBoxLayout(confWin);
            layout->addWidget(confPage);
            layout->addWidget(controls);
            confWin->setLayout(layout);
            confWin->setWindowTitle(i18nc("@title:window", "Configure CTags Plugin"));
            confWin->setWindowIcon(confPage->icon());
            confWin->show();
            confWin->exec();
        }
    });

    // popup menu
    m_menu = new KActionMenu(i18n("CTags"), this);
    actionCollection()->addAction(QStringLiteral("popup_ctags"), m_menu);

    m_gotoDec = m_menu->menu()->addAction(i18n("Go to Declaration: %1", QString()), this, &KateCTagsView::gotoDeclaration);
    m_gotoDef = m_menu->menu()->addAction(i18n("Go to Definition: %1", QString()), this, &KateCTagsView::gotoDefinition);
    m_lookup = m_menu->menu()->addAction(i18n("Lookup: %1", QString()), this, &KateCTagsView::lookupTag);

    connect(m_menu->menu(), &QMenu::aboutToShow, this, &KateCTagsView::aboutToShow);

    QWidget *ctagsWidget = new QWidget(m_toolView.data());
    m_ctagsUi.setupUi(ctagsWidget);
    m_ctagsUi.cmdEdit->setText(DEFAULT_CTAGS_CMD);

    m_ctagsUi.addButton->setToolTip(i18n("Add a directory to index."));
    m_ctagsUi.addButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

    m_ctagsUi.delButton->setToolTip(i18n("Remove a directory."));
    m_ctagsUi.delButton->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));

    m_ctagsUi.updateButton->setToolTip(i18n("(Re-)generate the session specific CTags database."));
    m_ctagsUi.updateButton->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));

    m_ctagsUi.updateButton2->setToolTip(i18n("(Re-)generate the session specific CTags database."));
    m_ctagsUi.updateButton2->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));

    m_ctagsUi.resetCMD->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));

    m_ctagsUi.tagsFile->setToolTip(i18n("Select new or existing database file."));
    m_ctagsUi.tagsFile->setMode(KFile::File);

    connect(m_ctagsUi.resetCMD, &QToolButton::clicked, this, &KateCTagsView::resetCMD);
    connect(m_ctagsUi.addButton, &QPushButton::clicked, this, &KateCTagsView::addTagTarget);
    connect(m_ctagsUi.delButton, &QPushButton::clicked, this, &KateCTagsView::delTagTarget);
    connect(m_ctagsUi.updateButton, &QPushButton::clicked, this, &KateCTagsView::updateSessionDB);
    connect(m_ctagsUi.updateButton2, &QPushButton::clicked, this, &KateCTagsView::updateSessionDB);
    connect(&m_proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &KateCTagsView::updateDone);
    connect(&m_proc, &QProcess::readyReadStandardError, this, [this]() {
        QString error = QString::fromLocal8Bit(m_proc.readAllStandardError());
        KMessageBox::sorry(nullptr, error);
    });

    m_gotoSymbWidget = new GotoSymbolWidget(mainWin, this);
    auto openLocal = actionCollection()->addAction(QStringLiteral("open_local_gts"));
    openLocal->setText(i18n("Go To Local Symbol"));
    actionCollection()->setDefaultShortcut(openLocal, Qt::CTRL | Qt::ALT | Qt::Key_P);
    connect(openLocal, &QAction::triggered, this, &KateCTagsView::showSymbols);

    auto openGlobal = actionCollection()->addAction(QStringLiteral("open_global_gts"));
    openGlobal->setText(i18n("Go To Global Symbol"));
    actionCollection()->setDefaultShortcut(openGlobal, Qt::CTRL | Qt::SHIFT | Qt::Key_P);
    connect(openGlobal, &QAction::triggered, this, &KateCTagsView::showGlobalSymbols);

    connect(m_ctagsUi.inputEdit, &QLineEdit::textChanged, this, &KateCTagsView::startEditTmr);

    m_editTimer.setSingleShot(true);
    connect(&m_editTimer, &QTimer::timeout, this, &KateCTagsView::editLookUp);

    connect(m_ctagsUi.tagTreeWidget, &QTreeWidget::itemActivated, this, &KateCTagsView::tagHitClicked);

    connect(m_mWin, &KTextEditor::MainWindow::unhandledShortcutOverride, this, &KateCTagsView::handleEsc);

    m_toolView->layout()->addWidget(ctagsWidget);
    m_toolView->installEventFilter(this);

    m_mWin->guiFactory()->addClient(this);

    m_commonDB = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1String("/katectags/common_db");
}

/******************************************************************/
KateCTagsView::~KateCTagsView()
{
    if (m_mWin && m_mWin->guiFactory()) {
        m_mWin->guiFactory()->removeClient(this);
    }

    if (m_toolView) {
        delete m_toolView;
    }
}

/******************************************************************/
void KateCTagsView::aboutToShow()
{
    QString currWord = currentWord();
    if (currWord.isEmpty()) {
        return;
    }

    if (Tags::hasTag(m_commonDB, currWord) || Tags::hasTag(m_ctagsUi.tagsFile->text(), currWord)) {
        QString squeezed = KStringHandler::csqueeze(currWord, 30);

        m_gotoDec->setText(i18n("Go to Declaration: %1", squeezed));
        m_gotoDef->setText(i18n("Go to Definition: %1", squeezed));
        m_lookup->setText(i18n("Lookup: %1", squeezed));
    }
}

/******************************************************************/
void KateCTagsView::readSessionConfig(const KConfigGroup &cg)
{
    m_ctagsUi.cmdEdit->setText(cg.readEntry("TagsGenCMD", DEFAULT_CTAGS_CMD));

    int numEntries = cg.readEntry("SessionNumTargets", 0);
    QString nr;
    QString target;
    for (int i = 0; i < numEntries; i++) {
        nr = QStringLiteral("%1").arg(i, 3);
        target = cg.readEntry(QStringLiteral("SessionTarget_%1").arg(nr), QString());
        if (!listContains(target)) {
            new QListWidgetItem(target, m_ctagsUi.targetList);
        }
    }

    QString sessionDB = cg.readEntry("SessionDatabase", QString());
    m_ctagsUi.tagsFile->setText(sessionDB);
}

/******************************************************************/
void KateCTagsView::writeSessionConfig(KConfigGroup &cg)
{
    cg.writeEntry("TagsGenCMD", m_ctagsUi.cmdEdit->text());
    cg.writeEntry("SessionNumTargets", m_ctagsUi.targetList->count());

    QString nr;
    for (int i = 0; i < m_ctagsUi.targetList->count(); i++) {
        nr = QStringLiteral("%1").arg(i, 3);
        cg.writeEntry(QStringLiteral("SessionTarget_%1").arg(nr), m_ctagsUi.targetList->item(i)->text());
    }

    cg.writeEntry("SessionDatabase", m_ctagsUi.tagsFile->text());

    cg.sync();
}

/******************************************************************/
void KateCTagsView::stepBack()
{
    if (m_jumpStack.isEmpty()) {
        return;
    }

    TagJump back;
    back = m_jumpStack.pop();

    m_mWin->openUrl(back.url);
    m_mWin->activeView()->setCursorPosition(back.cursor);
    m_mWin->activeView()->setFocus();
}

/******************************************************************/
void KateCTagsView::lookupTag()
{
    QString currWord = currentWord();
    if (currWord.isEmpty()) {
        return;
    }

    setNewLookupText(currWord);
    Tags::TagList list = Tags::getExactMatches(m_ctagsUi.tagsFile->text(), currWord);
    if (list.empty()) {
        list = Tags::getExactMatches(m_commonDB, currWord);
    }
    displayHits(list);

    // activate the hits tab
    m_ctagsUi.tabWidget->setCurrentIndex(0);
    m_mWin->showToolView(m_toolView);
}

/******************************************************************/
void KateCTagsView::editLookUp()
{
    Tags::TagList list = Tags::getPartialMatches(m_ctagsUi.tagsFile->text(), m_ctagsUi.inputEdit->text());
    if (list.empty()) {
        list = Tags::getPartialMatches(m_commonDB, m_ctagsUi.inputEdit->text());
    }
    displayHits(list);
}

/******************************************************************/
void KateCTagsView::gotoDefinition()
{
    QString currWord = currentWord();
    if (currWord.isEmpty()) {
        return;
    }

    QStringList types;
    types << QStringLiteral("S") << QStringLiteral("d") << QStringLiteral("f") << QStringLiteral("t") << QStringLiteral("v");
    gotoTagForTypes(currWord, types);
}

/******************************************************************/
void KateCTagsView::gotoDeclaration()
{
    QString currWord = currentWord();
    if (currWord.isEmpty()) {
        return;
    }

    QStringList types;
    types << QStringLiteral("L") << QStringLiteral("c") << QStringLiteral("e") << QStringLiteral("g") << QStringLiteral("m") << QStringLiteral("n")
          << QStringLiteral("p") << QStringLiteral("s") << QStringLiteral("u") << QStringLiteral("x");
    gotoTagForTypes(currWord, types);
}

/******************************************************************/
void KateCTagsView::gotoTagForTypes(const QString &word, const QStringList &types)
{
    Tags::TagList list = Tags::getMatches(m_ctagsUi.tagsFile->text(), word, false, types);
    if (list.empty()) {
        list = Tags::getMatches(m_commonDB, word, false, types);
    }

    // qCDebug(KTECTAGS) << "found" << list.count() << word << types;
    setNewLookupText(word);

    if (list.count() < 1) {
        m_ctagsUi.tagTreeWidget->clear();
        new QTreeWidgetItem(m_ctagsUi.tagTreeWidget, QStringList(i18n("No hits found")));
        m_ctagsUi.tabWidget->setCurrentIndex(0);
        m_mWin->showToolView(m_toolView);
        return;
    }

    displayHits(list);

    if (list.count() == 1) {
        Tags::TagEntry tag = list.first();
        jumpToTag(tag.file, tag.pattern, word);
    } else {
        Tags::TagEntry tag = list.first();
        jumpToTag(tag.file, tag.pattern, word);
        m_ctagsUi.tabWidget->setCurrentIndex(0);
        m_mWin->showToolView(m_toolView);
    }
}

/******************************************************************/
void KateCTagsView::setNewLookupText(const QString &newString)
{
    m_ctagsUi.inputEdit->blockSignals(true);
    m_ctagsUi.inputEdit->setText(newString);
    m_ctagsUi.inputEdit->blockSignals(false);
}

/******************************************************************/
void KateCTagsView::displayHits(const Tags::TagList &list)
{
    m_ctagsUi.tagTreeWidget->clear();
    if (list.isEmpty()) {
        new QTreeWidgetItem(m_ctagsUi.tagTreeWidget, QStringList(i18n("No hits found")));
        return;
    }
    m_ctagsUi.tagTreeWidget->setSortingEnabled(false);

    for (const auto &tag : list) {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_ctagsUi.tagTreeWidget);
        item->setText(0, tag.tag);
        item->setText(1, tag.type);
        item->setText(2, tag.file);
        item->setData(0, Qt::UserRole, tag.pattern);

        QString pattern = tag.pattern;
        pattern.replace(QStringLiteral("\\/"), QStringLiteral("/"));
        pattern = pattern.mid(2, pattern.length() - 4);
        pattern = pattern.trimmed();

        item->setData(0, Qt::ToolTipRole, pattern);
        item->setData(1, Qt::ToolTipRole, pattern);
        item->setData(2, Qt::ToolTipRole, pattern);
    }
    m_ctagsUi.tagTreeWidget->setSortingEnabled(true);
}

/******************************************************************/
void KateCTagsView::tagHitClicked(QTreeWidgetItem *item)
{
    // get stuff
    const QString file = item->data(2, Qt::DisplayRole).toString();
    const QString pattern = item->data(0, Qt::UserRole).toString();
    const QString word = item->data(0, Qt::DisplayRole).toString();

    jumpToTag(file, pattern, word);
}

/******************************************************************/
QString KateCTagsView::currentWord()
{
    KTextEditor::View *kv = m_mWin->activeView();
    if (!kv) {
        qCDebug(KTECTAGS) << "no KTextEditor::View";
        return QString();
    }

    if (kv->selection() && kv->selectionRange().onSingleLine()) {
        return kv->selectionText();
    }

    if (!kv->cursorPosition().isValid()) {
        qCDebug(KTECTAGS) << "cursor not valid!";
        return QString();
    }

    int line = kv->cursorPosition().line();
    int col = kv->cursorPosition().column();
    bool includeColon = m_ctagsUi.cmdEdit->text().contains(QLatin1String("--extra=+q"));

    QString linestr = kv->document()->line(line);

    int startPos = qMax(qMin(col, linestr.length() - 1), 0);
    int endPos = startPos;
    while (startPos >= 0
           && (linestr[startPos].isLetterOrNumber() || (linestr[startPos] == QLatin1Char(':') && includeColon) || linestr[startPos] == QLatin1Char('_')
               || linestr[startPos] == QLatin1Char('~'))) {
        startPos--;
    }
    while (endPos < linestr.length()
           && (linestr[endPos].isLetterOrNumber() || (linestr[endPos] == QLatin1Char(':') && includeColon) || linestr[endPos] == QLatin1Char('_'))) {
        endPos++;
    }
    if (startPos == endPos) {
        qCDebug(KTECTAGS) << "no word found!";
        return QString();
    }

    linestr = linestr.mid(startPos + 1, endPos - startPos - 1);

    while (linestr.endsWith(QLatin1Char(':'))) {
        linestr.remove(linestr.size() - 1, 1);
    }

    while (linestr.startsWith(QLatin1Char(':'))) {
        linestr.remove(0, 1);
    }

    // qCDebug(KTECTAGS) << linestr;
    return linestr;
}

/******************************************************************/
void KateCTagsView::jumpToTag(const QString &file, const QString &pattern, const QString &word)
{
    if (pattern.isEmpty()) {
        return;
    }

    // generate a regexp from the pattern
    // ctags interestingly escapes "/", but apparently nothing else. lets revert that
    QString unescaped = pattern;
    unescaped.replace(QStringLiteral("\\/"), QStringLiteral("/"));

    // most of the time, the ctags pattern has the form /^foo$/
    // but this isn't true for some macro definitions
    // where the form is only /^foo/
    // I have no idea if this is a ctags bug or not, but we have to deal with it

    QString reduced;
    QString escaped;
    QString re_string;

    if (unescaped.endsWith(QLatin1String("$/"))) {
        reduced = unescaped.mid(2, unescaped.length() - 4);
        escaped = QRegularExpression::escape(reduced);
        re_string = QStringLiteral("^%1$").arg(escaped);
    } else {
        reduced = unescaped.mid(2, unescaped.length() - 3);
        escaped = QRegularExpression::escape(reduced);
        re_string = QStringLiteral("^%1").arg(escaped);
    }

    QRegularExpression re(re_string);

    // save current location
    TagJump from;
    from.url = m_mWin->activeView()->document()->url();
    from.cursor = m_mWin->activeView()->cursorPosition();
    m_jumpStack.push(from);

    // open/activate the new file
    QFileInfo fInfo(file);
    // qCDebug(KTECTAGS) << pattern << file << fInfo.absoluteFilePath();
    m_mWin->openUrl(QUrl::fromLocalFile(fInfo.absoluteFilePath()));

    // any view active?
    if (!m_mWin->activeView()) {
        return;
    }

    // look for the line
    QString linestr;
    int line;
    for (line = 0; line < m_mWin->activeView()->document()->lines(); line++) {
        linestr = m_mWin->activeView()->document()->line(line);
        if (linestr.indexOf(re) > -1) {
            break;
        }
    }

    // activate the line
    if (line != m_mWin->activeView()->document()->lines()) {
        // line found now look for the column
        int column = linestr.indexOf(word) + (word.length() / 2);
        m_mWin->activeView()->setCursorPosition(KTextEditor::Cursor(line, column));
    }
    m_mWin->activeView()->setFocus();
}

/******************************************************************/
void KateCTagsView::startEditTmr()
{
    if (m_ctagsUi.inputEdit->text().size() > 3) {
        m_editTimer.start(500);
    }
}

/******************************************************************/
void KateCTagsView::updateSessionDB()
{
    if (m_proc.state() != QProcess::NotRunning) {
        return;
    }

    QString targets;
    QString target;
    for (int i = 0; i < m_ctagsUi.targetList->count(); i++) {
        target = m_ctagsUi.targetList->item(i)->text();
        if (target.endsWith(QLatin1Char('/')) || target.endsWith(QLatin1Char('\\'))) {
            target = target.left(target.size() - 1);
        }
        targets += QLatin1Char('\"') + target + QLatin1String("\" ");
    }

    QString pluginFolder = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1String("/katectags");
    QDir().mkpath(pluginFolder);

    if (m_ctagsUi.tagsFile->text().isEmpty()) {
        // FIXME we need a way to get the session name
        pluginFolder += QLatin1String("/session_db_");
        pluginFolder += QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd_hhmmss"));
        m_ctagsUi.tagsFile->setText(pluginFolder);
    }

    if (targets.isEmpty()) {
        KMessageBox::error(nullptr, i18n("No folders or files to index"));
        QFile::remove(m_ctagsUi.tagsFile->text());
        return;
    }

    QString commandLine = QStringLiteral("%1 -f %2 %3").arg(m_ctagsUi.cmdEdit->text(), m_ctagsUi.tagsFile->text(), targets);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QStringList arguments = m_proc.splitCommand(commandLine);
    QString command = arguments.takeFirst();
    m_proc.start(command, arguments);
#else
    m_proc.start(commandLine);
#endif

    if (!m_proc.waitForStarted(500)) {
        KMessageBox::error(nullptr, i18n("Failed to run \"%1\". exitStatus = %2", commandLine, m_proc.exitStatus()));
        return;
    }
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    m_ctagsUi.updateButton->setDisabled(true);
    m_ctagsUi.updateButton2->setDisabled(true);
}

/******************************************************************/
void KateCTagsView::updateDone(int exitCode, QProcess::ExitStatus status)
{
    if (status == QProcess::CrashExit) {
        KMessageBox::error(m_toolView, i18n("The CTags executable crashed."));
    } else if (exitCode != 0) {
        KMessageBox::error(m_toolView, i18n("The CTags program exited with code %1: %2", exitCode, QString::fromLocal8Bit(m_proc.readAllStandardError())));
    }

    m_ctagsUi.updateButton->setDisabled(false);
    m_ctagsUi.updateButton2->setDisabled(false);
    QApplication::restoreOverrideCursor();
}

/******************************************************************/
void KateCTagsView::addTagTarget()
{
    QFileDialog dialog;
    dialog.setDirectory(QFileInfo(m_mWin->activeView()->document()->url().path()).path());
    dialog.setFileMode(QFileDialog::Directory);

    // i18n("CTags Database Location"));
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QStringList urls = dialog.selectedFiles();

    for (int i = 0; i < urls.size(); i++) {
        if (!listContains(urls[i])) {
            new QListWidgetItem(urls[i], m_ctagsUi.targetList);
        }
    }
}

/******************************************************************/
void KateCTagsView::delTagTarget()
{
    delete m_ctagsUi.targetList->currentItem();
}

/******************************************************************/
bool KateCTagsView::listContains(const QString &target)
{
    for (int i = 0; i < m_ctagsUi.targetList->count(); i++) {
        if (m_ctagsUi.targetList->item(i)->text() == target) {
            return true;
        }
    }
    return false;
}

/******************************************************************/
bool KateCTagsView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if ((obj == m_toolView) && (ke->key() == Qt::Key_Escape)) {
            m_mWin->hideToolView(m_toolView);
            event->accept();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

/******************************************************************/
void KateCTagsView::resetCMD()
{
    m_ctagsUi.cmdEdit->setText(DEFAULT_CTAGS_CMD);
}

/******************************************************************/
void KateCTagsView::handleEsc(QEvent *e)
{
    if (!m_mWin) {
        return;
    }

    QKeyEvent *k = static_cast<QKeyEvent *>(e);
    if (k->key() == Qt::Key_Escape && k->modifiers() == Qt::NoModifier) {
        if (m_toolView->isVisible()) {
            m_mWin->hideToolView(m_toolView);
        }
    }
}

void KateCTagsView::showSymbols()
{
    m_gotoSymbWidget->showSymbols(m_mWin->activeView()->document()->url().toLocalFile());
    m_gotoSymbWidget->show();
    m_gotoSymbWidget->setFocus();
}

void KateCTagsView::showGlobalSymbols()
{
    m_gotoSymbWidget->showGlobalSymbols(m_ctagsUi.tagsFile->text());
    m_gotoSymbWidget->show();
    m_gotoSymbWidget->setFocus();
}
