/*
    SPDX-FileCopyrightText: 2011-21 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _SEARCH_OPEN_FILES_H_
#define _SEARCH_OPEN_FILES_H_

#include <QElapsedTimer>
#include <QObject>
#include <QRegularExpression>
#include <QTimer>
#include <ktexteditor/document.h>

#include "MatchModel.h"

class SearchOpenFiles : public QObject
{
    Q_OBJECT

public:
    SearchOpenFiles(QObject *parent = nullptr);

    void startSearch(const QList<KTextEditor::Document *> &list, const QRegularExpression &regexp);
    bool searching();
    void terminateSearch();

public Q_SLOTS:
    void cancelSearch();

    /// return 0 on success or a line number where we stopped.
    int searchOpenFile(KTextEditor::Document *doc, const QRegularExpression &regExp, int startLine);

private Q_SLOTS:
    void doSearchNextFile(int startLine);

private:
    int searchSingleLineRegExp(KTextEditor::Document *doc, const QRegularExpression &regExp, int startLine);
    int searchMultiLineRegExp(KTextEditor::Document *doc, const QRegularExpression &regExp, int startLine);

Q_SIGNALS:
    void matchesFound(const QUrl &url, const QVector<KateSearchMatch> &searchMatches);
    void searchDone();
    void searching(const QString &file);

private:
    QList<KTextEditor::Document *> m_docList;
    int m_nextFileIndex = -1;
    QTimer m_nextRunTimer;
    int m_nextLine = -1;
    QRegularExpression m_regExp;
    bool m_cancelSearch = true;
    bool m_terminateSearch = false;
    QString m_fullDoc;
    QVector<int> m_lineStart;
    QElapsedTimer m_statusTime;
};

#endif
