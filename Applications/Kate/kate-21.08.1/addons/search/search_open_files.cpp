/*
    SPDX-FileCopyrightText: 2011-21 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "search_open_files.h"

SearchOpenFiles::SearchOpenFiles(QObject *parent)
    : QObject(parent)
{
    m_nextRunTimer.setInterval(0);
    m_nextRunTimer.setSingleShot(true);
    connect(&m_nextRunTimer, &QTimer::timeout, this, [this]() {
        doSearchNextFile(m_nextLine);
    });
}

bool SearchOpenFiles::searching()
{
    return !m_cancelSearch;
}

void SearchOpenFiles::startSearch(const QList<KTextEditor::Document *> &list, const QRegularExpression &regexp)
{
    if (m_nextFileIndex != -1) {
        return;
    }

    m_docList = list;
    m_nextFileIndex = 0;
    m_regExp = regexp;
    m_cancelSearch = false;
    m_terminateSearch = false;
    m_statusTime.restart();
    m_nextLine = 0;
    m_nextRunTimer.start(0);
}

void SearchOpenFiles::terminateSearch()
{
    m_cancelSearch = true;
    m_terminateSearch = true;
    m_nextFileIndex = -1;
    m_nextLine = -1;
    m_nextRunTimer.stop();
}

void SearchOpenFiles::cancelSearch()
{
    m_cancelSearch = true;
}

void SearchOpenFiles::doSearchNextFile(int startLine)
{
    if (m_cancelSearch || m_nextFileIndex >= m_docList.size()) {
        m_nextFileIndex = -1;
        m_cancelSearch = true;
        m_nextLine = -1;
        return;
    }

    // NOTE The document managers signal documentWillBeDeleted() must be connected to
    // cancelSearch(). A closed file could lead to a crash if it is not handled.
    int line = searchOpenFile(m_docList[m_nextFileIndex], m_regExp, startLine);
    if (line == 0) {
        // file searched go to next
        m_nextFileIndex++;
        if (m_nextFileIndex == m_docList.size()) {
            m_nextFileIndex = -1;
            m_cancelSearch = true;
            Q_EMIT searchDone();
        } else {
            m_nextLine = 0;
        }
    } else {
        m_nextLine = line;
    }
    m_nextRunTimer.start();
}

int SearchOpenFiles::searchOpenFile(KTextEditor::Document *doc, const QRegularExpression &regExp, int startLine)
{
    if (m_statusTime.elapsed() > 100) {
        m_statusTime.restart();
        Q_EMIT searching(doc->url().toString());
    }

    if (regExp.pattern().contains(QLatin1String("\\n"))) {
        return searchMultiLineRegExp(doc, regExp, startLine);
    }

    return searchSingleLineRegExp(doc, regExp, startLine);
}

int SearchOpenFiles::searchSingleLineRegExp(KTextEditor::Document *doc, const QRegularExpression &regExp, int startLine)
{
    int column;
    QElapsedTimer time;

    time.start();
    int resultLine = 0;
    QVector<KateSearchMatch> matches;
    for (int line = startLine; line < doc->lines(); line++) {
        if (time.elapsed() > 100) {
            // qDebug() << "Search time exceeded" << time.elapsed() << line;
            resultLine = line;
            break;
        }
        QRegularExpressionMatch match;
        match = regExp.match(doc->line(line));
        column = match.capturedStart();

        while (column != -1 && !match.captured().isEmpty()) {
            int endColumn = column + match.capturedLength();
            int preContextStart = qMax(0, column - MatchModel::PreContextLen);
            const QString &lineStr = doc->line(line);
            QString preContext = lineStr.mid(preContextStart, column - preContextStart);
            QString postContext = lineStr.mid(endColumn, MatchModel::PostContextLen);

            matches.push_back(KateSearchMatch{preContext,
                                              match.captured(),
                                              postContext,
                                              QString(),
                                              KTextEditor::Range{line, column, line, column + match.capturedLength()},
                                              true});
            match = regExp.match(doc->line(line), column + match.capturedLength());
            column = match.capturedStart();
        }
    }

    // Q_EMIT all matches batched
    Q_EMIT matchesFound(doc->url(), matches);

    return resultLine;
}

int SearchOpenFiles::searchMultiLineRegExp(KTextEditor::Document *doc, const QRegularExpression &regExp, int inStartLine)
{
    int column = 0;
    int startLine = 0;
    QElapsedTimer time;
    time.start();
    QRegularExpression tmpRegExp = regExp;

    if (inStartLine == 0) {
        // Copy the whole file to a temporary buffer to be able to search newlines
        m_fullDoc.clear();
        m_lineStart.clear();
        m_lineStart << 0;
        for (int i = 0; i < doc->lines(); i++) {
            m_fullDoc += doc->line(i) + QLatin1Char('\n');
            m_lineStart << m_fullDoc.size();
        }
        if (!regExp.pattern().endsWith(QLatin1Char('$'))) {
            // if regExp ends with '$' leave the extra newline at the end as
            // '$' will be replaced with (?=\\n), which needs the extra newline
            m_fullDoc.remove(m_fullDoc.size() - 1, 1);
        }
    } else {
        if (inStartLine > 0 && inStartLine < m_lineStart.size()) {
            column = m_lineStart[inStartLine];
            startLine = inStartLine;
        } else {
            return 0;
        }
    }

    if (regExp.pattern().endsWith(QLatin1Char('$'))) {
        QString newPatern = tmpRegExp.pattern();
        newPatern.replace(QStringLiteral("$"), QStringLiteral("(?=\\n)"));
        tmpRegExp.setPattern(newPatern);
    }

    QRegularExpressionMatch match;
    match = tmpRegExp.match(m_fullDoc, column);
    column = match.capturedStart();
    int resultLine = 0;
    QVector<KateSearchMatch> matches;
    while (column != -1 && !match.captured().isEmpty()) {
        // search for the line number of the match
        int i;
        startLine = -1;
        for (i = 1; i < m_lineStart.size(); i++) {
            if (m_lineStart[i] > column) {
                startLine = i - 1;
                break;
            }
        }
        if (startLine == -1) {
            break;
        }

        int startColumn = (column - m_lineStart[startLine]);
        int endLine = startLine + match.captured().count(QLatin1Char('\n'));
        int lastNL = match.captured().lastIndexOf(QLatin1Char('\n'));
        int endColumn = lastNL == -1 ? startColumn + match.captured().length() : match.captured().length() - lastNL - 1;

        int preContextStart = qMax(0, startColumn - MatchModel::PreContextLen);
        QString preContext = doc->line(startLine).mid(preContextStart, startColumn - preContextStart);
        QString postContext = doc->line(endLine).mid(endColumn, MatchModel::PostContextLen);

        matches.push_back(
            KateSearchMatch{preContext, match.captured(), postContext, QString(), KTextEditor::Range{startLine, startColumn, endLine, endColumn}, true});
        match = tmpRegExp.match(m_fullDoc, column + match.capturedLength());
        column = match.capturedStart();

        if (time.elapsed() > 100) {
            // qDebug() << "Search time exceeded" << time.elapsed() << line;
            resultLine = startLine;
            break;
        }
    }

    // Q_EMIT all matches batched
    Q_EMIT matchesFound(doc->url(), matches);

    return resultLine;
}
