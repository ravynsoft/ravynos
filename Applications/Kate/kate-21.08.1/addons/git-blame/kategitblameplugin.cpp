/*
    SPDX-FileCopyrightText: 2021 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kategitblameplugin.h"
#include "gitblametooltip.h"

#include <algorithm>

#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>
#include <KXMLGUIFactory>

#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/InlineNoteInterface>
#include <KTextEditor/InlineNoteProvider>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>

#include <QDir>
#include <QUrl>

#include <QFontMetricsF>
#include <QHash>
#include <QPainter>
#include <QRegularExpression>
#include <QVariant>

GitBlameInlineNoteProvider::GitBlameInlineNoteProvider(KateGitBlamePluginView *pluginView)
    : KTextEditor::InlineNoteProvider()
    , m_pluginView(pluginView)
{
}

GitBlameInlineNoteProvider::~GitBlameInlineNoteProvider()
{
    QPointer<KTextEditor::View> view = m_pluginView->activeView();
    if (view) {
        qobject_cast<KTextEditor::InlineNoteInterface *>(view)->unregisterInlineNoteProvider(this);
    }
}

QVector<int> GitBlameInlineNoteProvider::inlineNotes(int line) const
{
    if (!m_pluginView->hasBlameInfo()) {
        return QVector<int>();
    }

    QPointer<KTextEditor::Document> doc = m_pluginView->activeDocument();
    if (!doc) {
        qDebug() << "no document";
        return QVector<int>();
    }

    if (m_mode == KateGitBlameMode::None) {
        return {};
    }

    int lineLen = doc->line(line).size();
    QPointer<KTextEditor::View> view = m_pluginView->activeView();
    if (view->cursorPosition().line() == line || m_mode == KateGitBlameMode::AllLines) {
        return QVector<int>{lineLen + 4};
    }
    return QVector<int>();
}

QSize GitBlameInlineNoteProvider::inlineNoteSize(const KTextEditor::InlineNote &note) const
{
    return QSize(note.lineHeight() * 50, note.lineHeight());
}

void GitBlameInlineNoteProvider::paintInlineNote(const KTextEditor::InlineNote &note, QPainter &painter) const
{
    QFont font = note.font();
    painter.setFont(font);
    const QFontMetrics fm(note.font());

    int lineNr = note.position().line();
    const KateGitBlameInfo &info = m_pluginView->blameInfo(lineNr);

    QString text = info.title.isEmpty()
        ? i18nc("git-blame information \"author: date \"", " %1: %2 ", info.name, m_locale.toString(info.date, QLocale::NarrowFormat))
        : i18nc("git-blame information \"author: date: commit title \"",
                " %1: %2: %3 ",
                info.name,
                m_locale.toString(info.date, QLocale::NarrowFormat),
                info.title);
    QRect rectangle{0, 0, fm.horizontalAdvance(text), note.lineHeight()};

    auto editor = KTextEditor::Editor::instance();
    auto color = QColor::fromRgba(editor->theme().textColor(KSyntaxHighlighting::Theme::Normal));
    color.setAlpha(0);
    painter.setPen(color);
    color.setAlpha(8);
    painter.setBrush(color);
    painter.drawRect(rectangle);

    color.setAlpha(note.underMouse() ? 130 : 90);
    painter.setPen(color);
    painter.setBrush(color);
    painter.drawText(rectangle, text);
}

void GitBlameInlineNoteProvider::inlineNoteActivated(const KTextEditor::InlineNote &note, Qt::MouseButtons buttons, const QPoint &)
{
    if ((buttons & Qt::LeftButton) != 0) {
        int lineNr = note.position().line();
        const KateGitBlameInfo &info = m_pluginView->blameInfo(lineNr);

        // Hack: view->mainWindow()->view() to de-constify view
        Q_ASSERT(note.view() == m_pluginView->activeView());
        m_pluginView->showCommitInfo(info.commitHash, note.view()->mainWindow()->activeView());
    }
}

void GitBlameInlineNoteProvider::cycleMode()
{
    int newMode = (int)m_mode + 1;
    if (newMode > (int)KateGitBlameMode::Count) {
        newMode = 0;
    }
    setMode(KateGitBlameMode(newMode));
}

void GitBlameInlineNoteProvider::setMode(KateGitBlameMode mode)
{
    m_mode = mode;
    Q_EMIT inlineNotesReset();
}

K_PLUGIN_FACTORY_WITH_JSON(KateGitBlamePluginFactory, "kategitblameplugin.json", registerPlugin<KateGitBlamePlugin>();)

KateGitBlamePlugin::KateGitBlamePlugin(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
{
}

KateGitBlamePlugin::~KateGitBlamePlugin()
{
}

QObject *KateGitBlamePlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    return new KateGitBlamePluginView(this, mainWindow);
}

KateGitBlamePluginView::KateGitBlamePluginView(KateGitBlamePlugin *plugin, KTextEditor::MainWindow *mainwindow)
    : QObject(plugin)
    , m_mainWindow(mainwindow)
    , m_inlineNoteProvider(this)
    , m_blameInfoProc(this)
    , m_showProc(this)
{
    KXMLGUIClient::setComponentName(QStringLiteral("kategitblameplugin"), i18n("Git Blame"));
    setXMLFile(QStringLiteral("ui.rc"));
    QAction *showBlameAction = actionCollection()->addAction(QStringLiteral("git_blame_show"));
    showBlameAction->setText(i18n("Show Git Blame Details"));
    actionCollection()->setDefaultShortcut(showBlameAction, Qt::CTRL | Qt::ALT | Qt::Key_G);
    QAction *toggleBlameAction = actionCollection()->addAction(QStringLiteral("git_blame_toggle"));
    toggleBlameAction->setText(i18n("Toggle Git Blame Details"));
    m_mainWindow->guiFactory()->addClient(this);

    connect(showBlameAction, &QAction::triggered, plugin, [this, showBlameAction]() {
        KTextEditor::View *kv = m_mainWindow->activeView();
        if (!kv) {
            return;
        }
        setToolTipIgnoreKeySequence(showBlameAction->shortcut());
        int lineNr = kv->cursorPosition().line();
        const KateGitBlameInfo &info = blameInfo(lineNr);
        showCommitInfo(info.commitHash, kv);
    });
    connect(toggleBlameAction, &QAction::triggered, this, [this]() {
        m_inlineNoteProvider.cycleMode();
    });

    connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &KateGitBlamePluginView::viewChanged);

    connect(&m_blameInfoProc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &KateGitBlamePluginView::blameFinished);

    connect(&m_showProc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &KateGitBlamePluginView::showFinished);

    m_inlineNoteProvider.setMode(KateGitBlameMode::SingleLine);
}

KateGitBlamePluginView::~KateGitBlamePluginView()
{
    m_mainWindow->guiFactory()->removeClient(this);
}

QPointer<KTextEditor::View> KateGitBlamePluginView::activeView() const
{
    return m_mainWindow->activeView();
}

QPointer<KTextEditor::Document> KateGitBlamePluginView::activeDocument() const
{
    if (m_mainWindow->activeView() && m_mainWindow->activeView()->document()) {
        return m_mainWindow->activeView()->document();
    }
    return nullptr;
}

void KateGitBlamePluginView::viewChanged(KTextEditor::View *view)
{
    m_blameInfo.clear();

    if (m_lastView) {
        qobject_cast<KTextEditor::InlineNoteInterface *>(m_lastView)->unregisterInlineNoteProvider(&m_inlineNoteProvider);
    }
    m_lastView = view;

    if (view == nullptr || view->document() == nullptr) {
        return;
    }

    qobject_cast<KTextEditor::InlineNoteInterface *>(view)->registerInlineNoteProvider(&m_inlineNoteProvider);

    startBlameProcess(view->document()->url());
}

void KateGitBlamePluginView::startBlameProcess(const QUrl &url)
{
    if (m_blameInfoProc.state() != QProcess::NotRunning) {
        // Wait for the previous process to be done...
        return;
    }

    QString fileName{url.fileName()};
    QDir dir{url.toLocalFile()};
    dir.cdUp();

    QStringList args{QStringLiteral("blame"), QStringLiteral("--date=iso-strict"), QStringLiteral("./%1").arg(fileName)};

    m_blameInfoProc.setWorkingDirectory(dir.absolutePath());
    m_blameInfoProc.start(QStringLiteral("git"), args, QIODevice::ReadOnly);
    m_blameUrl = url;
}

void KateGitBlamePluginView::startShowProcess(const QUrl &url, const QString &hash)
{
    if (m_showProc.state() != QProcess::NotRunning) {
        // Wait for the previous process to be done...
        return;
    }
    if (hash == m_activeCommitInfo.m_hash) {
        // We have already the data
        return;
    }

    QDir dir{url.toLocalFile()};
    dir.cdUp();

    QStringList args{QStringLiteral("show"), hash};
    m_showProc.setWorkingDirectory(dir.absolutePath());
    m_showProc.start(QStringLiteral("git"), args, QIODevice::ReadOnly);
}

void KateGitBlamePluginView::showCommitInfo(const QString &hash, KTextEditor::View *view)
{
    if (hash == m_activeCommitInfo.m_hash) {
        m_showHash.clear();
        m_tooltip.show(m_activeCommitInfo.m_content, view);
    } else {
        m_showHash = hash;
        startShowProcess(view->document()->url(), hash);
    }
}

void KateGitBlamePluginView::blameFinished(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
    QString stdErr = QString::fromUtf8(m_blameInfoProc.readAllStandardError());
    const QStringList stdOut = QString::fromUtf8(m_blameInfoProc.readAllStandardOutput()).split(QLatin1Char('\n'));

    // check if the git process was running for a previous document when the view changed.
    // if that is the case re-trigger the process and skip this data
    KTextEditor::Document *doc = activeDocument();
    if (!doc || m_blameUrl != doc->url()) {
        viewChanged(m_mainWindow->activeView());
        return;
    }

    const static QRegularExpression lineReg(
        QStringLiteral("([^\\^\\s]+)[^\\(]+\\((.*)\\s+(\\d\\d\\d\\d-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\d\\S+)[^\\)]+\\)\\s(.*)"));

    m_blameInfo.clear();

    for (const auto &line : stdOut) {
        const QRegularExpressionMatch match = lineReg.match(line);
        if (match.hasMatch()) {
            m_blameInfo.append(
                {match.captured(1), match.captured(2).trimmed(), QDateTime::fromString(match.captured(3), Qt::ISODate), QString(), match.captured(4)});
        }
    }
}

void KateGitBlamePluginView::showFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString stdErr = QString::fromUtf8(m_showProc.readAllStandardError());
    QString stdOut = QString::fromUtf8(m_showProc.readAllStandardOutput());

    // Try to avoid crashes caused by QTextBrowser running out of memory
    // Seems to be a limit somewhere as the memory reaches 8G and then crashes
    // even if there is more free memory
    // Anyways, this popup is not optimal for > 1m characters...
    if (stdOut.size() > 1000000) {
        stdOut = stdOut.left(1000000) + i18n("\n--- Output truncated ---\n");
    }

    if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
        return;
    }
    QStringList args = m_showProc.arguments();
    if (args.size() != 2) {
        qWarning() << "Wrong number of parameters:" << args;
        return;
    }

    int titleStart = 0;
    for (int i = 0; i < 4; ++i) {
        titleStart = stdOut.indexOf(QLatin1Char('\n'), titleStart + 1);
        if (titleStart < 0 || titleStart >= stdOut.size() - 1) {
            qWarning() << "This is not a known git show format";
            return;
        }
    }

    int titleEnd = stdOut.indexOf(QLatin1Char('\n'), titleStart + 1);
    if (titleEnd < 0 || titleEnd >= stdOut.size() - 1) {
        qWarning() << "This is not a known git show format";
        return;
    }

    m_activeCommitInfo.m_title = stdOut.mid(titleStart, titleEnd - titleStart);
    m_activeCommitInfo.m_hash = args[1];
    m_activeCommitInfo.m_title = m_activeCommitInfo.m_title.trimmed();
    m_activeCommitInfo.m_content = stdOut;

    if (!m_showHash.isEmpty() && m_showHash != args[1]) {
        startShowProcess(m_mainWindow->activeView()->document()->url(), m_showHash);
        return;
    }
    if (!m_showHash.isEmpty()) {
        m_showHash.clear();
        m_tooltip.show(stdOut, m_mainWindow->activeView());
    }
}

bool KateGitBlamePluginView::hasBlameInfo() const
{
    return !m_blameInfo.isEmpty();
}

const KateGitBlameInfo &KateGitBlamePluginView::blameInfo(int lineNr)
{
    if (m_blameInfo.isEmpty() || !activeDocument()) {
        return blameGetUpdateInfo(-1);
    }

    int adjustedLineNr = lineNr + m_lineOffset;
    const QStringView lineText = activeDocument()->line(lineNr);

    if (adjustedLineNr >= 0 && adjustedLineNr < m_blameInfo.size()) {
        if (m_blameInfo[adjustedLineNr].line == lineText) {
            return blameGetUpdateInfo(adjustedLineNr);
        }
    }

    // search for the line 100 lines before and after until it matches
    m_lineOffset = 0;
    while (m_lineOffset < 100 && lineNr + m_lineOffset >= 0 && lineNr + m_lineOffset < m_blameInfo.size()) {
        if (m_blameInfo[lineNr + m_lineOffset].line == lineText) {
            return blameGetUpdateInfo(lineNr + m_lineOffset);
        }
        m_lineOffset++;
    }

    m_lineOffset = 0;
    while (m_lineOffset > -100 && lineNr + m_lineOffset >= 0 && (lineNr + m_lineOffset) < m_blameInfo.size()) {
        if (m_blameInfo[lineNr + m_lineOffset].line == lineText) {
            return blameGetUpdateInfo(lineNr + m_lineOffset);
        }
        m_lineOffset--;
    }

    return blameGetUpdateInfo(-1);
}

const KateGitBlameInfo &KateGitBlamePluginView::blameGetUpdateInfo(int lineNr)
{
    static const KateGitBlameInfo dummy{QStringLiteral("hash"), i18n("Not Committed Yet"), QDateTime::currentDateTime(), QString(), QString()};

    if (m_blameInfo.isEmpty() || lineNr < 0 || lineNr >= m_blameInfo.size()) {
        return dummy;
    }

    KateGitBlameInfo &info = m_blameInfo[lineNr];
    if (info.commitHash == m_activeCommitInfo.m_hash) {
        if (info.title != m_activeCommitInfo.m_title) {
            info.title = m_activeCommitInfo.m_title;
        }
    } else {
        startShowProcess(m_mainWindow->activeView()->document()->url(), info.commitHash);
    }
    return info;
}

void KateGitBlamePluginView::setToolTipIgnoreKeySequence(QKeySequence sequence)
{
    m_tooltip.setIgnoreKeySequence(sequence);
}

void KateGitBlamePluginView::CommitInfo::clear()
{
    m_hash.clear();
    m_title.clear();
    m_content.clear();
}

#include "kategitblameplugin.moc"
