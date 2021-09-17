/*
    SPDX-FileCopyrightText: 2011-21 Kåre Särs <kare.sars@iki.fi>
    SPDX-FileCopyrightText: 2021 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SearchDiskFiles.h"

#include <QDir>
#include <QElapsedTimer>
#include <QTextStream>
#include <QUrl>

SearchDiskFiles::SearchDiskFiles(SearchDiskFilesWorkList &worklist, const QRegularExpression &regexp, const bool includeBinaryFiles)
    : m_worklist(worklist)
    , m_regExp(regexp.pattern(), regexp.patternOptions()) // we WANT to kill the sharing, ELSE WE LOCK US DEAD!
    , m_includeBinaryFiles(includeBinaryFiles)
{
    // ensure we have a proper thread name during e.g. perf profiling
    setObjectName(QStringLiteral("SearchDiskFiles"));
}

void SearchDiskFiles::run()
{
    // do we need to search multiple lines?
    const bool multiLineSearch = m_regExp.pattern().contains(QLatin1String("\\n"));

    // timer to emit matchesFound once in a time even for files without matches
    // this triggers process in the UI
    QElapsedTimer emitTimer;
    emitTimer.start();

    // search, pulls work from the shared work list for all workers
    while (true) {
        // get next file, we get empty string if all done or search canceled!
        const auto fileName = m_worklist.nextFileToSearch();
        if (fileName.isEmpty()) {
            break;
        }

        // open file early, this allows mime-type detection & search to use same io device
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly)) {
            continue;
        }

        // let the right search algorithm compute the matches for this file
        QVector<KateSearchMatch> matches;
        if (multiLineSearch) {
            matches = searchMultiLineRegExp(file);
        } else {
            matches = searchSingleLineRegExp(file);
        }

        // if we have matches or didn't emit something long enough, do so
        // we don't emit for all file to not stall get GUI and lock us a lot ;)
        if (!matches.isEmpty() || emitTimer.hasExpired(100)) {
            Q_EMIT matchesFound(QUrl::fromLocalFile(file.fileName()), matches);
            emitTimer.restart();
        }
    }
}

QVector<KateSearchMatch> SearchDiskFiles::searchSingleLineRegExp(QFile &file)
{
    QTextStream stream(&file);
    QVector<KateSearchMatch> matches;
    QString line;
    int currentLineNumber = 0;
    while (stream.readLineInto(&line)) {
        // check if not binary data....
        // bad, but stuff better than asking QMimeDatabase which is a performance & threading disaster...
        if (!m_includeBinaryFiles && line.contains(QLatin1Char('\0'))) {
            // kill all seen matches and be done
            matches.clear();
            return matches;
        }

        // match all occurrences in the current line
        int columnToStartMatch = 0;
        bool canceled = false;
        while (true) {
            // handle canceling
            if (m_worklist.isCanceled()) {
                canceled = true;
                break;
            }

            // try match at the current interesting column, abort search loop if nothing found!
            const QRegularExpressionMatch match = m_regExp.match(line, columnToStartMatch);
            const int column = match.capturedStart();
            if (column == -1 || match.capturedLength() == 0)
                break;

            // remember match
            const int endColumn = column + match.capturedLength();
            const int preContextStart = qMax(0, column - MatchModel::PreContextLen);
            const QString preContext = line.mid(preContextStart, column - preContextStart);
            const QString postContext = line.mid(endColumn, MatchModel::PostContextLen);
            matches.push_back(KateSearchMatch{preContext,
                                              match.captured(),
                                              postContext,
                                              QString(),
                                              KTextEditor::Range{currentLineNumber, column, currentLineNumber, column + match.capturedLength()},
                                              true});

            // advance match column
            columnToStartMatch = column + match.capturedLength();
        }

        // handle canceling => above we only did break out of the matching loop!
        if (canceled) {
            break;
        }

        // advance to next line
        ++currentLineNumber;
    }
    return matches;
}

QVector<KateSearchMatch> SearchDiskFiles::searchMultiLineRegExp(QFile &file)
{
    int column = 0;
    int line = 0;
    QString fullDoc;
    QVector<int> lineStart;
    QRegularExpression tmpRegExp = m_regExp;

    QVector<KateSearchMatch> matches;
    QTextStream stream(&file);
    fullDoc = stream.readAll();

    // check if not binary data....
    // bad, but stuff better than asking QMimeDatabase which is a performance & threading disaster...
    if (!m_includeBinaryFiles && fullDoc.contains(QLatin1Char('\0'))) {
        // kill all seen matches and be done
        matches.clear();
        return matches;
    }

    fullDoc.remove(QLatin1Char('\r'));

    lineStart.clear();
    lineStart << 0;
    for (int i = 0; i < fullDoc.size() - 1; i++) {
        if (fullDoc[i] == QLatin1Char('\n')) {
            lineStart << i + 1;
        }
    }
    if (tmpRegExp.pattern().endsWith(QLatin1Char('$'))) {
        fullDoc += QLatin1Char('\n');
        QString newPatern = tmpRegExp.pattern();
        newPatern.replace(QStringLiteral("$"), QStringLiteral("(?=\\n)"));
        tmpRegExp.setPattern(newPatern);
    }

    QRegularExpressionMatch match;
    match = tmpRegExp.match(fullDoc);
    column = match.capturedStart();
    while (column != -1 && !match.captured().isEmpty()) {
        if (m_worklist.isCanceled()) {
            break;
        }
        // search for the line number of the match
        int i;
        line = -1;
        for (i = 1; i < lineStart.size(); i++) {
            if (lineStart[i] > column) {
                line = i - 1;
                break;
            }
        }
        if (line == -1) {
            break;
        }
        int startColumn = (column - lineStart[line]);
        int endLine = line + match.captured().count(QLatin1Char('\n'));
        int lastNL = match.captured().lastIndexOf(QLatin1Char('\n'));
        int endColumn = lastNL == -1 ? startColumn + match.captured().length() : match.captured().length() - lastNL - 1;

        int preContextStart = qMax(lineStart[line], column - MatchModel::PreContextLen);
        QString preContext = fullDoc.mid(preContextStart, column - preContextStart);
        QString postContext = fullDoc.mid(column + match.captured().length(), MatchModel::PostContextLen);

        matches.push_back(
            KateSearchMatch{preContext, match.captured(), postContext, QString(), KTextEditor::Range{line, startColumn, endLine, endColumn}, true});

        match = tmpRegExp.match(fullDoc, column + match.capturedLength());
        column = match.capturedStart();
    }
    return matches;
}
