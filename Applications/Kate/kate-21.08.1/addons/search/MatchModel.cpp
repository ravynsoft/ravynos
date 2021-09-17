/*
    SPDX-FileCopyrightText: 2021 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "MatchModel.h"
#include <KLocalizedString>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTimer>
#include <algorithm> // std::count_if

#include <ktexteditor/movinginterface.h>
#include <ktexteditor/movingrange.h>

static const quintptr InfoItemId = 0xFFFFFFFF;
static const quintptr FileItemId = 0x7FFFFFFF;

// Model indexes
// - (0, 0, InfoItemId) (row, column, internalId)
//   | - (0, 0, FileItemId)
//   |    | - (0, 0, 0)
//   |    | - (1, 0, 0)
//   | - (1, 0, FileItemId)
//   |    | - (0, 0, 1)
//   |    | - (1, 0, 1)

static QUrl localFileDirUp(const QUrl &url)
{
    if (!url.isLocalFile()) {
        return url;
    }

    // else go up
    return QUrl::fromLocalFile(QFileInfo(url.toLocalFile()).dir().absolutePath());
}

MatchModel::MatchModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_infoUpdateTimer.setInterval(100); // FIXME why does this delay not work?
    m_infoUpdateTimer.setSingleShot(true);
    connect(&m_infoUpdateTimer, &QTimer::timeout, this, [this]() {
        dataChanged(createIndex(0, 0, InfoItemId), createIndex(0, 0, InfoItemId));
    });
}

MatchModel::~MatchModel()
{
}

void MatchModel::setDocumentManager(KTextEditor::Application *manager)
{
    m_docManager = manager;
    connect(m_docManager, &KTextEditor::Application::documentWillBeDeleted, this, &MatchModel::cancelReplace);
}

void MatchModel::setSearchPlace(MatchModel::SearchPlaces searchPlace)
{
    m_searchPlace = searchPlace;
    if (!m_infoUpdateTimer.isActive()) {
        m_infoUpdateTimer.start();
    }
}

void MatchModel::setFileListUpdate(const QString &path)
{
    m_lastSearchPath = path;
    m_searchState = Preparing;
    if (!m_infoUpdateTimer.isActive()) {
        m_infoUpdateTimer.start();
    }
}

void MatchModel::setSearchState(MatchModel::SearchState searchState)
{
    m_searchState = searchState;
    if (!m_infoUpdateTimer.isActive()) {
        m_infoUpdateTimer.start();
    }
    if (m_searchState == SearchDone) {
        beginResetModel();
        std::sort(m_matchFiles.begin(), m_matchFiles.end(), [](const MatchFile &l, const MatchFile &r) {
            return l.fileUrl < r.fileUrl;
        });
        for (int i = 0; i < m_matchFiles.size(); ++i) {
            m_matchFileIndexHash[m_matchFiles[i].fileUrl] = i;
        }
        endResetModel();
    }
}

void MatchModel::setBaseSearchPath(const QString &baseSearchPath)
{
    m_resultBaseDir = baseSearchPath;
    if (!m_infoUpdateTimer.isActive()) {
        m_infoUpdateTimer.start();
    }
}

void MatchModel::setProjectName(const QString &projectName)
{
    m_projectName = projectName;
    if (!m_infoUpdateTimer.isActive()) {
        m_infoUpdateTimer.start();
    }
}

void MatchModel::clear()
{
    beginResetModel();
    m_matchFiles.clear();
    m_matchFileIndexHash.clear();
    endResetModel();
}

/** This function returns the row index of the specified file.
 * If the file does not exist in the model, the file will be added to the model. */
int MatchModel::matchFileRow(const QUrl &fileUrl) const
{
    return m_matchFileIndexHash.value(fileUrl, -1);
}

/** This function is used to add a match to a new file */
void MatchModel::addMatches(const QUrl &fileUrl, const QVector<KateSearchMatch> &searchMatches)
{
    m_lastMatchUrl = fileUrl;
    m_searchState = Searching;
    // update match/search info
    if (!m_infoUpdateTimer.isActive()) {
        m_infoUpdateTimer.start();
    }

    if (m_matchFiles.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, 0);
        endInsertRows();
    }

    if (searchMatches.isEmpty()) {
        return;
    }

    int fileIndex = matchFileRow(fileUrl);
    if (fileIndex == -1) {
        fileIndex = m_matchFiles.size();
        m_matchFileIndexHash.insert(fileUrl, fileIndex);
        beginInsertRows(createIndex(0, 0, InfoItemId), fileIndex, fileIndex);
        // We are always starting the insert at the end, so we could optimize by delaying/grouping the signaling of the updates
        m_matchFiles.append(MatchFile());
        m_matchFiles[fileIndex].fileUrl = fileUrl;
        endInsertRows();
    }

    int matchIndex = m_matchFiles[fileIndex].matches.size();
    beginInsertRows(createIndex(fileIndex, 0, FileItemId), matchIndex, matchIndex + searchMatches.size() - 1);
    m_matchFiles[fileIndex].matches += searchMatches;
    endInsertRows();
}

void MatchModel::setMatchColors(const QString &foreground, const QString &background, const QString &replaceBackground)
{
    m_foregroundColor = foreground;
    m_searchBackgroundColor = background;
    m_replaceHighlightColor = replaceBackground;
}

KateSearchMatch *MatchModel::matchFromIndex(const QModelIndex &matchIndex)
{
    if (!isMatch(matchIndex)) {
        qDebug() << "Not a valid match index";
        return nullptr;
    }

    int fileRow = matchIndex.internalId();
    int matchRow = matchIndex.row();

    return &m_matchFiles[fileRow].matches[matchRow];
}

KTextEditor::Range MatchModel::matchRange(const QModelIndex &matchIndex) const
{
    if (!isMatch(matchIndex)) {
        qDebug() << "Not a valid match index";
        return KTextEditor::Range();
    }
    int fileRow = matchIndex.internalId();
    int matchRow = matchIndex.row();
    return m_matchFiles[fileRow].matches[matchRow].range;
}

const QVector<KateSearchMatch> &MatchModel::fileMatches(const QUrl &fileUrl) const
{
    static const QVector<KateSearchMatch> EmptyDummy;

    int row = matchFileRow(fileUrl);
    if (row < 0 || row >= m_matchFiles.size()) {
        return EmptyDummy;
    }
    return m_matchFiles[row].matches;
}

void MatchModel::updateMatchRanges(const QVector<KTextEditor::MovingRange *> &ranges)
{
    if (ranges.isEmpty()) {
        return;
    }

    const QUrl &fileUrl = ranges.first()->document()->url();
    // NOTE: we assume there are only ranges for one document in the provided ranges
    // NOTE: we also assume the document is not deleted as we clear the ranges when the document is deleted

    int fileRow = matchFileRow(fileUrl);
    if (fileRow < 0 || fileRow >= m_matchFiles.size()) {
        // qDebug() << "No such results" << fileRow << fileUrl;
        return; // No such document in the results
    }

    QVector<KateSearchMatch> &matches = m_matchFiles[fileRow].matches;

    if (ranges.size() != matches.size()) {
        // The sizes do not match so we cannot match the ranges easily.. abort
        qDebug() << ranges.size() << "!=" << matches.size();
        return;
    }

    if (ranges.size() > 1000) {
        // if we have > 1000 matches in a file it could get slow to update it all the time
        return;
    }

    for (int i = 0; i < ranges.size(); ++i) {
        matches[i].range = ranges[i]->toRange();
    }
    QModelIndex rootFileIndex = index(fileRow, 0, createIndex(0, 0, InfoItemId));
    dataChanged(index(0, 0, rootFileIndex), index(matches.count() - 1, 0, rootFileIndex));
}

/** This function is used to replace a match */
bool MatchModel::replaceMatch(KTextEditor::Document *doc, const QModelIndex &matchIndex, const QRegularExpression &regExp, const QString &replaceString)
{
    if (!doc) {
        qDebug() << "No doc";
        return false;
    }

    Match *matchItem = matchFromIndex(matchIndex);

    if (!matchItem) {
        qDebug() << "Not a valid index";
        return false;
    }

    // don't replace an already replaced item
    if (!matchItem->replaceText.isEmpty()) {
        // qDebug() << "not replacing already replaced item";
        return false;
    }

    // Check that the text has not been modified and still matches + get captures for the replace
    QString matchLines = doc->text(matchItem->range);
    QRegularExpressionMatch match = regExp.match(matchLines);
    if (match.capturedStart() != 0) {
        qDebug() << matchLines << "Does not match" << regExp.pattern();
        return false;
    }

    // Modify the replace string according to this match
    QString replaceText = replaceString;
    replaceText.replace(QLatin1String("\\\\"), QLatin1String("¤Search&Replace¤"));

    // allow captures \0 .. \9
    for (int j = qMin(9, match.lastCapturedIndex()); j >= 0; --j) {
        QString captureLX = QStringLiteral("\\L\\%1").arg(j);
        QString captureUX = QStringLiteral("\\U\\%1").arg(j);
        QString captureX = QStringLiteral("\\%1").arg(j);
        replaceText.replace(captureLX, match.captured(j).toLower());
        replaceText.replace(captureUX, match.captured(j).toUpper());
        replaceText.replace(captureX, match.captured(j));
    }

    // allow captures \{0} .. \{9999999}...
    for (int j = match.lastCapturedIndex(); j >= 0; --j) {
        QString captureLX = QStringLiteral("\\L\\{%1}").arg(j);
        QString captureUX = QStringLiteral("\\U\\{%1}").arg(j);
        QString captureX = QStringLiteral("\\{%1}").arg(j);
        replaceText.replace(captureLX, match.captured(j).toLower());
        replaceText.replace(captureUX, match.captured(j).toUpper());
        replaceText.replace(captureX, match.captured(j));
    }

    replaceText.replace(QLatin1String("\\n"), QLatin1String("\n"));
    replaceText.replace(QLatin1String("\\t"), QLatin1String("\t"));
    replaceText.replace(QLatin1String("¤Search&Replace¤"), QLatin1String("\\"));

    // Replace the string
    doc->replaceText(matchItem->range, replaceText);

    // update the range
    int newEndLine = matchItem->range.start().line() + replaceText.count(QLatin1Char('\n'));
    int lastNL = replaceText.lastIndexOf(QLatin1Char('\n'));
    int newEndColumn = lastNL == -1 ? matchItem->range.start().column() + replaceText.length() : replaceText.length() - lastNL - 1;
    matchItem->range.setEnd(KTextEditor::Cursor{newEndLine, newEndColumn});

    matchItem->replaceText = replaceText;
    return true;
}

/** This function is used to replace a match */
bool MatchModel::replaceSingleMatch(KTextEditor::Document *doc, const QModelIndex &matchIndex, const QRegularExpression &regExp, const QString &replaceString)
{
    if (!doc) {
        qDebug() << "No doc";
        return false;
    }

    if (!isMatch(matchIndex)) {
        qDebug() << "This should not be possible";
        return false;
    }

    if (matchIndex.internalId() == InfoItemId || matchIndex.internalId() == FileItemId) {
        qDebug() << "You cannot replace a file or the info item";
        return false;
    }

    // Create a vector of moving ranges for updating the tree-view after replace
    QVector<KTextEditor::MovingRange *> matchRanges;
    KTextEditor::MovingInterface *miface = qobject_cast<KTextEditor::MovingInterface *>(doc);

    // Only add items after "matchIndex"
    int fileRow = matchIndex.internalId();
    int matchRow = matchIndex.row();

    QVector<Match> &matches = m_matchFiles[fileRow].matches;

    for (int i = matchRow + 1; i < matches.size(); ++i) {
        KTextEditor::MovingRange *mr = miface->newMovingRange(matches[i].range);
        matchRanges.append(mr);
    }

    // The first range in the vector is for this match
    if (!replaceMatch(doc, matchIndex, regExp, replaceString)) {
        return false;
    }

    // Update the items after the matchIndex
    for (int i = matchRow + 1; i < matches.size(); ++i) {
        Q_ASSERT(!matchRanges.isEmpty());
        KTextEditor::MovingRange *mr = matchRanges.takeFirst();
        matches[i].range = mr->toRange();
        delete mr;
    }
    Q_ASSERT(matchRanges.isEmpty());

    dataChanged(createIndex(matchRow, 0, fileRow), createIndex(matches.size() - 1, 0, fileRow));

    return true;
}

void MatchModel::doReplaceNextMatch()
{
    Q_ASSERT(m_docManager);

    if (m_cancelReplace || m_replaceFile >= m_matchFiles.size()) {
        m_replaceFile = -1;
        Q_EMIT replaceDone();
        return;
    }

    // NOTE The document managers signal documentWillBeDeleted() must be connected to
    // cancelReplace(). A closed file could lead to a crash if it is not handled.
    // this is now done in setDocumentManager()

    MatchFile &matchFile = m_matchFiles[m_replaceFile];

    if (matchFile.checkState == Qt::Unchecked) {
        m_replaceFile++;
        QTimer::singleShot(0, this, &MatchModel::doReplaceNextMatch);
        return;
    }

    KTextEditor::Document *doc;
    doc = m_docManager->findUrl(matchFile.fileUrl);
    if (!doc) {
        doc = m_docManager->openUrl(matchFile.fileUrl);
    }

    if (!doc) {
        qDebug() << "Failed to open the document" << matchFile.fileUrl;
        m_replaceFile++;
        QTimer::singleShot(0, this, &MatchModel::doReplaceNextMatch);
        return;
    }

    if (doc->url() != matchFile.fileUrl) {
        qDebug() << "url differences" << matchFile.fileUrl << doc->url();
        matchFile.fileUrl = doc->url();
    }

    auto &matches = matchFile.matches;

    // Create a vector of moving ranges for updating the matches after replace
    QVector<KTextEditor::MovingRange *> matchRanges;
    matchRanges.reserve(matches.size());
    KTextEditor::MovingInterface *miface = qobject_cast<KTextEditor::MovingInterface *>(doc);
    for (const auto &match : qAsConst(matches)) {
        matchRanges.append(miface->newMovingRange(match.range));
    }

    // Make one transaction for the whole replace to speed up things
    // and get all replacements in one "undo"
    KTextEditor::Document::EditingTransaction transaction(doc);

    for (int i = 0; i < matches.size(); ++i) {
        if (matches[i].checked) {
            replaceMatch(doc, createIndex(i, 0, m_replaceFile), m_regExp, m_replaceText);
        }
        // The document has been modified -> make sure the next match has the correct range
        if (i < matches.size() - 1) {
            matches[i + 1].range = matchRanges[i + 1]->toRange();
        }
    }

    dataChanged(createIndex(0, 0, m_replaceFile), createIndex(matches.size() - 1, 0, m_replaceFile));

    // free our moving ranges
    qDeleteAll(matchRanges);

    m_replaceFile++;
    QTimer::singleShot(0, this, &MatchModel::doReplaceNextMatch);
}

/** Initiate a replace of all matches that have been checked */
void MatchModel::replaceChecked(const QRegularExpression &regExp, const QString &replaceString)
{
    Q_ASSERT(m_docManager != nullptr);
    if (m_replaceFile != -1) {
        Q_ASSERT(m_replaceFile != -1);
        return; // already replacing
    }

    m_replaceFile = 0;
    m_regExp = regExp;
    m_replaceText = replaceString;
    m_cancelReplace = false;
    doReplaceNextMatch();
}

void MatchModel::cancelReplace()
{
    m_replaceFile = -1;
    m_cancelReplace = true;
}

static QString nbsFormated(int number, int width)
{
    QString str = QString::number(number);
    int strWidth = str.size();
    str.reserve(width);
    while (strWidth < width) {
        str = QStringLiteral("&nbsp;") + str;
        strWidth++;
    }
    return str;
}

QString MatchModel::infoHtmlString() const
{
    if (m_matchFiles.isEmpty() && m_searchState == SearchDone && m_lastMatchUrl.isEmpty()) {
        return QString();
    }

    int matchesTotal = 0;
    int checkedTotal = 0;
    for (const auto &matchFile : qAsConst(m_matchFiles)) {
        matchesTotal += matchFile.matches.size();
        checkedTotal += std::count_if(matchFile.matches.begin(), matchFile.matches.end(), [](const KateSearchMatch &match) {
            return match.checked;
        });
    }

    if (m_searchState == Preparing) {
        if (m_lastSearchPath.size() >= 73) {
            return i18n("<b><i>Generating file list: ...%1</i></b>", m_lastSearchPath.right(70));
        } else {
            return i18n("<b><i>Generating file list: ...%1</i></b>", m_lastSearchPath);
        }
    }

    if (m_searchState == Searching) {
        QString searchUrl = m_lastMatchUrl.toDisplayString(QUrl::PreferLocalFile);

        if (searchUrl.size() > 73) {
            return i18np("<b><i>One match found, searching: ...%2</i></b>",
                         "<b><i>%1 matches found, searching: ...%2</i></b>",
                         matchesTotal,
                         searchUrl.right(70));
        } else {
            return i18np("<b><i>One match found, searching: %2</i></b>", "<b><i>%1 matches found, searching: %2</i></b>", matchesTotal, searchUrl);
        }
    }

    QString checkedStr = i18np("One checked", "%1 checked", checkedTotal);

    switch (m_searchPlace) {
    case CurrentFile:
        return i18np("<b><i>One match (%2) found in file</i></b>", "<b><i>%1 matches (%2) found in current file</i></b>", matchesTotal, checkedStr);
    case MatchModel::OpenFiles:
        return i18np("<b><i>One match (%2) found in open files</i></b>", "<b><i>%1 matches (%2) found in open files</i></b>", matchesTotal, checkedStr);
        break;
    case MatchModel::Folder:
        return i18np("<b><i>One match (%3) found in folder %2</i></b>",
                     "<b><i>%1 matches (%3) found in folder %2</i></b>",
                     matchesTotal,
                     m_resultBaseDir,
                     checkedStr);
        break;
    case MatchModel::Project: {
        return i18np("<b><i>One match (%4) found in project %2 (%3)</i></b>",
                     "<b><i>%1 matches (%4) found in project %2 (%3)</i></b>",
                     matchesTotal,
                     m_projectName,
                     m_resultBaseDir,
                     checkedStr);
        break;
    }
    case MatchModel::AllProjects: // "in Open Projects"
        return i18np("<b><i>One match (%3) found in all open projects (common parent: %2)</i></b>",
                     "<b><i>%1 matches (%3) found in all open projects (common parent: %2)</i></b>",
                     matchesTotal,
                     m_resultBaseDir,
                     checkedStr);
        break;
    }

    return QString();
}

QString MatchModel::fileToHtmlString(const MatchFile &matchFile) const
{
    QString path = matchFile.fileUrl.isLocalFile() ? localFileDirUp(matchFile.fileUrl).path() : matchFile.fileUrl.url();
    if (!path.isEmpty() && !path.endsWith(QLatin1Char('/'))) {
        path += QLatin1Char('/');
    }

    QString tmpStr = QStringLiteral("%1<b>%2: %3</b>").arg(path, matchFile.fileUrl.fileName()).arg(matchFile.matches.size());

    return tmpStr;
}

QString MatchModel::matchToHtmlString(const Match &match) const
{
    QString pre = match.preMatchStr;
    if (match.preMatchStr.size() == PreContextLen) {
        pre.replace(0, 3, QLatin1String("..."));
    }
    pre = pre.toHtmlEscaped();

    QString matchStr = match.matchStr.toHtmlEscaped();
    ;

    QString replaceStr = match.replaceText.toHtmlEscaped();

    if (!replaceStr.isEmpty()) {
        matchStr = QLatin1String("<i><s>") + matchStr + QLatin1String("</s></i> ");
    }
    matchStr = QStringLiteral("<span style=\"background-color:%1; color:%2;\">%3</span>").arg(m_searchBackgroundColor, m_foregroundColor, matchStr);

    if (!replaceStr.isEmpty()) {
        matchStr += QStringLiteral("<span style=\"background-color:%1; color:%2;\">%3</span>").arg(m_replaceHighlightColor, m_foregroundColor, replaceStr);
    }

    matchStr.replace(QLatin1Char('\n'), QStringLiteral("\\n"));
    matchStr.replace(QLatin1Char('\t'), QStringLiteral("\\t"));

    QString post = match.postMatchStr;
    int nlIndex = post.indexOf(QLatin1Char('\n'));
    if (nlIndex != -1) {
        post = post.mid(0, nlIndex);
    }
    if (post.size() == PostContextLen) {
        post.replace(PostContextLen - 3, 3, QLatin1String("..."));
    }
    post = post.toHtmlEscaped();

    // (line:col)[space][space] ...Line text pre [highlighted match] Line text post....
    QString displayText = QStringLiteral("<span style=\"color:%1;\">&nbsp;<b>%2:%3</b></span>&nbsp;")
                              .arg(m_foregroundColor)
                              .arg(nbsFormated(match.range.start().line() + 1, 3))
                              .arg(nbsFormated(match.range.start().column() + 1, 3))
        + pre + matchStr + post;

    return displayText;
}

QString MatchModel::infoToPlainText() const
{
    if (m_matchFiles.isEmpty() && m_searchState == SearchDone) {
        return QString();
    }

    int matchesTotal = 0;
    int checkedTotal = 0;
    for (const auto &matchFile : qAsConst(m_matchFiles)) {
        matchesTotal += matchFile.matches.size();
        checkedTotal += std::count_if(matchFile.matches.begin(), matchFile.matches.end(), [](const KateSearchMatch &match) {
            return match.checked;
        });
    }

    if (m_searchState == Preparing) {
        if (m_lastSearchPath.size() >= 73) {
            return i18n("Generating file list: ...%1", m_lastSearchPath.right(70));
        } else {
            return i18n("Generating file list: ...%1", m_lastSearchPath);
        }
    }

    if (m_searchState == Searching) {
        QString searchUrl = m_lastMatchUrl.toDisplayString(QUrl::PreferLocalFile);

        if (searchUrl.size() > 73) {
            return i18np("One match found, searching: ...%2", "%1 matches found, searching: ...%2", matchesTotal, searchUrl.right(70));
        } else {
            return i18np("One match found, searching: %2", "%1 matches found, searching: %2", matchesTotal, searchUrl);
        }
    }

    QString checkedStr = i18np("One checked", "%1 checked", checkedTotal);

    switch (m_searchPlace) {
    case CurrentFile:
        return i18np("One match (%2) found in file", "%1 matches (%2) found in current file", matchesTotal, checkedStr);
    case MatchModel::OpenFiles:
        return i18np("One match (%2) found in open files", "%1 matches (%2) found in open files", matchesTotal, checkedStr);
        break;
    case MatchModel::Folder:
        return i18np("One match (%3) found in folder %2", "%1 matches (%3) found in folder %2", matchesTotal, m_resultBaseDir, checkedStr);
        break;
    case MatchModel::Project: {
        return i18np("One match (%4) found in project %2 (%3)",
                     "%1 matches (%4) found in project %2 (%3)",
                     matchesTotal,
                     m_projectName,
                     m_resultBaseDir,
                     checkedStr);
        break;
    }
    case MatchModel::AllProjects: // "in Open Projects"
        return i18np("One match (%3) found in all open projects (common parent: %2)",
                     "%1 matches (%3) found in all open projects (common parent: %2)",
                     matchesTotal,
                     m_resultBaseDir,
                     checkedStr);
        break;
    }

    return QString();
}

QString MatchModel::fileToPlainText(const MatchFile &matchFile) const
{
    QString path = matchFile.fileUrl.isLocalFile() ? localFileDirUp(matchFile.fileUrl).path() : matchFile.fileUrl.url();
    if (!path.isEmpty() && !path.endsWith(QLatin1Char('/'))) {
        path += QLatin1Char('/');
    }

    QString tmpStr = QStringLiteral("%1%2: %3").arg(path, matchFile.fileUrl.fileName()).arg(matchFile.matches.size());

    return tmpStr;
}

QString MatchModel::matchToPlainText(const Match &match) const
{
    QString pre = match.preMatchStr;

    QString matchStr = match.matchStr;
    matchStr.replace(QLatin1Char('\n'), QStringLiteral("\\n"));

    QString replaceStr = match.replaceText;
    if (!replaceStr.isEmpty()) {
        matchStr = QLatin1String("----") + matchStr + QLatin1String("----");
        matchStr += QLatin1String("++++") + replaceStr + QLatin1String("++++");
    }
    QString post = match.postMatchStr;

    matchStr.replace(QLatin1Char('\n'), QStringLiteral("\\n"));
    matchStr.replace(QLatin1Char('\t'), QStringLiteral("\\t"));
    replaceStr.replace(QLatin1Char('\n'), QStringLiteral("\\n"));
    replaceStr.replace(QLatin1Char('\t'), QStringLiteral("\\t"));

    // (line:col)[space][space] ...Line text pre [highlighted match] Line text post....
    QString displayText = QStringLiteral("%1:%2: ").arg(match.range.start().line() + 1, 3).arg(match.range.start().column() + 1, 3) + pre + matchStr + post;
    return displayText;
}

bool MatchModel::isMatch(const QModelIndex &itemIndex) const
{
    if (!itemIndex.isValid()) {
        return false;
    }
    if (itemIndex.internalId() == InfoItemId) {
        return false;
    }
    if (itemIndex.internalId() == FileItemId) {
        return false;
    }

    return true;
}

QModelIndex MatchModel::fileIndex(const QUrl &url) const
{
    int row = matchFileRow(url);
    if (row == -1) {
        return QModelIndex();
    }
    return createIndex(row, 0, FileItemId);
}

QModelIndex MatchModel::firstMatch() const
{
    if (m_matchFiles.isEmpty()) {
        return QModelIndex();
    }

    return createIndex(0, 0, static_cast<quintptr>(0));
}

QModelIndex MatchModel::lastMatch() const
{
    if (m_matchFiles.isEmpty()) {
        return QModelIndex();
    }
    const MatchFile &matchFile = m_matchFiles.constLast();
    return createIndex(matchFile.matches.size() - 1, 0, m_matchFiles.size() - 1);
}

QModelIndex MatchModel::firstFileMatch(const QUrl &url) const
{
    int row = matchFileRow(url);
    if (row == -1) {
        return QModelIndex();
    }

    // if a file is in the vector it has a match
    return createIndex(0, 0, row);
}

QModelIndex MatchModel::closestMatchAfter(const QUrl &url, const KTextEditor::Cursor &cursor) const
{
    int row = matchFileRow(url);
    if (row < 0) {
        return QModelIndex();
    }
    if (row >= m_matchFiles.size()) {
        return QModelIndex();
    }
    if (!cursor.isValid()) {
        return QModelIndex();
    }

    // if a file is in the vector it has a match
    const MatchFile &matchFile = m_matchFiles[row];

    int i = 0;
    for (; i < matchFile.matches.size() - 1; ++i) {
        if (matchFile.matches[i].range.end() >= cursor) {
            break;
        }
    }

    return createIndex(i, 0, row);
}

QModelIndex MatchModel::closestMatchBefore(const QUrl &url, const KTextEditor::Cursor &cursor) const
{
    int row = matchFileRow(url);
    if (row < 0) {
        return QModelIndex();
    }
    if (row >= m_matchFiles.size()) {
        return QModelIndex();
    }
    if (!cursor.isValid()) {
        return QModelIndex();
    }

    // if a file is in the vector it has a match
    const MatchFile &matchFile = m_matchFiles[row];

    int i = matchFile.matches.size() - 1;
    for (; i >= 0; --i) {
        if (matchFile.matches[i].range.start() <= cursor) {
            break;
        }
    }

    return createIndex(i, 0, row);
}

QModelIndex MatchModel::nextMatch(const QModelIndex &itemIndex) const
{
    if (!itemIndex.isValid()) {
        return firstMatch();
    }

    int fileRow = itemIndex.internalId() < FileItemId ? itemIndex.internalId() : itemIndex.row();
    if (fileRow < 0 || fileRow >= m_matchFiles.size()) {
        return QModelIndex();
    }

    int matchRow = itemIndex.internalId() < FileItemId ? itemIndex.row() : 0;
    matchRow++;
    if (matchRow >= m_matchFiles[fileRow].matches.size()) {
        fileRow++;
        matchRow = 0;
    }

    if (fileRow >= m_matchFiles.size()) {
        fileRow = 0;
    }
    return createIndex(matchRow, 0, fileRow);
}

QModelIndex MatchModel::prevMatch(const QModelIndex &itemIndex) const
{
    if (!itemIndex.isValid()) {
        return lastMatch();
    }

    int fileRow = itemIndex.internalId() < FileItemId ? itemIndex.internalId() : itemIndex.row();
    if (fileRow < 0 || fileRow >= m_matchFiles.size()) {
        return QModelIndex();
    }

    int matchRow = itemIndex.internalId() < FileItemId ? itemIndex.row() : 0;
    matchRow--;
    if (matchRow < 0) {
        fileRow--;
    }
    if (fileRow < 0) {
        fileRow = m_matchFiles.size() - 1;
    }
    if (matchRow < 0) {
        matchRow = m_matchFiles[fileRow].matches.size() - 1;
    }
    return createIndex(matchRow, 0, fileRow);
}

QVariant MatchModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.column() < 0 || index.column() > 1) {
        return QVariant();
    }

    int fileRow = index.internalId() == InfoItemId ? -1 : index.internalId() == FileItemId ? index.row() : (int)index.internalId();
    int matchRow = index.internalId() == InfoItemId || index.internalId() == FileItemId ? -1 : index.row();

    if (fileRow == -1) {
        // Info Item
        switch (role) {
        case Qt::DisplayRole:
            return infoHtmlString();
        case PlainTextRole:
            return infoToPlainText();
        case Qt::CheckStateRole:
            return m_infoCheckState;
        }
        return QVariant();
    }

    if (fileRow < 0 || fileRow >= m_matchFiles.size()) {
        qDebug() << "Should be a file (or the info item in the near future)" << fileRow;
        return QVariant();
    }

    if (matchRow < 0) {
        // File item
        switch (role) {
        case Qt::DisplayRole:
            return fileToHtmlString(m_matchFiles[fileRow]);
        case Qt::CheckStateRole:
            return m_matchFiles[fileRow].checkState;
        case FileUrlRole:
            return m_matchFiles[fileRow].fileUrl;
        case PlainTextRole:
            return fileToPlainText(m_matchFiles[fileRow]);
        }
    } else if (matchRow < m_matchFiles[fileRow].matches.size()) {
        // Match
        const Match &match = m_matchFiles[fileRow].matches[matchRow];
        switch (role) {
        case Qt::DisplayRole:
            return matchToHtmlString(match);
        case Qt::CheckStateRole:
            return match.checked ? Qt::Checked : Qt::Unchecked;
        case FileUrlRole:
            return m_matchFiles[fileRow].fileUrl;
        case StartLineRole:
            return match.range.start().line();
        case StartColumnRole:
            return match.range.start().column();
        case EndLineRole:
            return match.range.end().line();
        case EndColumnRole:
            return match.range.end().column();
        case PreMatchRole:
            return match.preMatchStr;
        case MatchRole:
            return match.matchStr;
        case PostMatchRole:
            return match.postMatchStr;
        case ReplacedRole:
            return !match.replaceText.isEmpty();
        case ReplaceTextRole:
            return match.replaceText;
        case PlainTextRole:
            return matchToPlainText(match);
        }
    } else {
        qDebug() << "bad index";
        return QVariant();
    }

    return QVariant();
}

bool MatchModel::setFileChecked(int fileRow, bool checked)
{
    if (fileRow < 0 || fileRow >= m_matchFiles.size()) {
        return false;
    }
    QVector<Match> &matches = m_matchFiles[fileRow].matches;
    for (int i = 0; i < matches.size(); ++i) {
        matches[i].checked = checked;
    }
    m_matchFiles[fileRow].checkState = checked ? Qt::Checked : Qt::Unchecked;
    QModelIndex rootFileIndex = index(fileRow, 0, createIndex(0, 0, InfoItemId));
    dataChanged(index(0, 0, rootFileIndex), index(matches.count() - 1, 0, rootFileIndex), QVector<int>{Qt::CheckStateRole});
    dataChanged(rootFileIndex, rootFileIndex, QVector<int>{Qt::CheckStateRole});
    return true;
}

bool MatchModel::setData(const QModelIndex &itemIndex, const QVariant &, int role)
{
    if (role != Qt::CheckStateRole) {
        return false;
    }
    if (!itemIndex.isValid()) {
        return false;
    }
    if (itemIndex.column() != 0) {
        return false;
    }

    // Check/un-check the File Item and it's children
    if (itemIndex.internalId() == InfoItemId) {
        bool checked = m_infoCheckState != Qt::Checked;
        for (int i = 0; i < m_matchFiles.size(); ++i) {
            setFileChecked(i, checked);
        }
        m_infoCheckState = checked ? Qt::Checked : Qt::Unchecked;
        QModelIndex infoIndex = createIndex(0, 0, InfoItemId);
        dataChanged(infoIndex, infoIndex, QVector<int>{Qt::CheckStateRole});
        return true;
    }

    if (itemIndex.internalId() == FileItemId) {
        int fileRrow = itemIndex.row();
        if (fileRrow < 0 || fileRrow >= m_matchFiles.size()) {
            return false;
        }
        bool checked = m_matchFiles[fileRrow].checkState != Qt::Checked; // we toggle the current value
        setFileChecked(fileRrow, checked);

        // compare file items
        Qt::CheckState checkState = m_matchFiles[0].checkState;
        for (int i = 1; i < m_matchFiles.size(); ++i) {
            if (checkState != m_matchFiles[i].checkState) {
                checkState = Qt::PartiallyChecked;
                break;
            }
        }
        m_infoCheckState = checkState;
        QModelIndex infoIndex = createIndex(0, 0, InfoItemId);
        dataChanged(infoIndex, infoIndex, QVector<int>{Qt::CheckStateRole});
        return true;
    }

    int rootRow = itemIndex.internalId();
    if (rootRow < 0 || rootRow >= m_matchFiles.size()) {
        return false;
    }

    int row = itemIndex.row();
    QVector<Match> &matches = m_matchFiles[rootRow].matches;
    if (row < 0 || row >= matches.size()) {
        return false;
    }

    // we toggle the current value
    matches[row].checked = !matches[row].checked;

    int checkedCount = std::count_if(matches.begin(), matches.end(), [](const KateSearchMatch &match) {
        return match.checked;
    });

    if (checkedCount == matches.size()) {
        m_matchFiles[rootRow].checkState = Qt::Checked;
    } else if (checkedCount == 0) {
        m_matchFiles[rootRow].checkState = Qt::Unchecked;
    } else {
        m_matchFiles[rootRow].checkState = Qt::PartiallyChecked;
    }

    QModelIndex rootFileIndex = index(rootRow, 0);
    dataChanged(rootFileIndex, rootFileIndex, QVector<int>{Qt::CheckStateRole});
    dataChanged(index(row, 0, rootFileIndex), index(row, 0, rootFileIndex), QVector<int>{Qt::CheckStateRole});
    return true;
}

void MatchModel::uncheckAll()
{
    for (int i = 0; i < m_matchFiles.size(); ++i) {
        setFileChecked(i, false);
    }
    m_infoCheckState = Qt::Unchecked;
}

Qt::ItemFlags MatchModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    if (index.column() == 0) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int MatchModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return (m_matchFiles.isEmpty() && m_searchState == SearchDone && m_lastMatchUrl.isEmpty()) ? 0 : 1;
    }

    if (parent.internalId() == InfoItemId) {
        return m_matchFiles.size();
    }

    if (parent.internalId() != FileItemId) {
        // matches do not have children
        return 0;
    }

    // If we get here parent.internalId() == FileItemId
    int row = parent.row();
    if (row < 0 || row >= m_matchFiles.size()) {
        return 0;
    }

    return m_matchFiles[row].matches.size();
}

int MatchModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QModelIndex MatchModel::index(int row, int column, const QModelIndex &parent) const
{
    // Create the Info Item
    if (!parent.isValid()) {
        return createIndex(0, 0, InfoItemId);
    }

    // File Item
    if (parent.internalId() == InfoItemId) {
        return createIndex(row, column, FileItemId);
    }

    // Match Item
    if (parent.internalId() == FileItemId) {
        return createIndex(row, column, parent.row());
    }

    // Parent is a match which does not have children
    return QModelIndex();
}

QModelIndex MatchModel::parent(const QModelIndex &child) const
{
    if (child.internalId() == InfoItemId) {
        return QModelIndex();
    }

    if (child.internalId() == FileItemId) {
        return createIndex(0, 0, InfoItemId);
    }

    return createIndex(child.internalId(), 0, FileItemId);
}
