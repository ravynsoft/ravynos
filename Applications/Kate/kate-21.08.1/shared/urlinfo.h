/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef URLINFO_H
#define URLINFO_H

#include <KTextEditor/Cursor>

#include <QDir>
#include <QRegularExpression>
#include <QString>
#include <QUrl>
#include <QUrlQuery>

/**
 * Represents a file to be opened, consisting of its URL and the cursor to jump to.
 */
class UrlInfo
{
public:
    /**
     * Parses a file path argument and determines its line number and column and full path,
     * we use absolute file paths because we will e.g. pass this over dbus to other processes.
     *
     * @param path path passed on e.g. command line to parse into an URL
     */
    UrlInfo(QString path)
        : cursor(KTextEditor::Cursor::invalid())
    {
        QString currentDirPath = QDir::current().absolutePath();
        if (!currentDirPath.endsWith(QLatin1Char('/'))) {
            currentDirPath += QLatin1Char('/');
        }

        // QDir::isAbsolutePath()/absoluteFilePath() treat paths starting with ':' as Qt
        // Resource (qrc) paths, and consider them absolute
        if (!path.startsWith(QLatin1Char(':')) && QDir::isAbsolutePath(path)) {
            if (QFile::exists(path)) { // Existing absolute path, no cursor can be detected
                url = QUrl::fromLocalFile(path);
                return;
            }
        } else {
            // Relative path, maybe starting with ':'; we concatenate the absolute path manually
            QString absolutePath = currentDirPath + path;

            if (QFile::exists(absolutePath)) { // Existing absolute path, no cursor can be detected
                url = QUrl::fromLocalFile(absolutePath);
                return;
            }
        }

        /**
         * ok, the path as is, is no existing file, now, cut away :xx:yy stuff as cursor
         * this will make test:50 to test with line 50
         */
        const auto match = QRegularExpression(QStringLiteral(":(\\d+)(?::(\\d+))?:?$")).match(path);
        if (match.isValid()) {
            /**
             * cut away the line/column specification from the path
             */
            path.chop(match.capturedLength());

            /**
             * After cutting the line/column part, if the file exists make "path" absolute.
             * Note that we can't rely on QUrl::fromUserInput() because of paths starting with
             * ':', see comment above about QDir::isAbsolutePath().
             */
            const QString absolutePath = currentDirPath + path;
            if (QFile::exists(absolutePath)) {
                path = absolutePath;
            }

            /**
             * set right cursor position
             * don't use an invalid column when the line is valid
             */
            const int line = match.captured(1).toInt() - 1;
            const int column = qMax(0, match.captured(2).toInt() - 1);
            cursor.setPosition(line, column);
        }

        /**
         * Construct url: "path" has already been made absolute above.
         * This should work with:
         * - local paths, "/path/to/somefile" becomes file:///path/to/some/file
         * - file: urls, file:///path/to/some/file
         * - remote urls, e.g. sftp://1.2.3.4:22/path/to/some/file
         */
        url = QUrl::fromUserInput(path, QString(), QUrl::AssumeLocalFile);

        /**
         * Set cursor position if we can extract from URL query string
         */
        if (url.hasQuery()) {
            QUrlQuery urlQuery(url);
            QString lineStr = urlQuery.queryItemValue(QStringLiteral("line"));
            QString columnStr = urlQuery.queryItemValue(QStringLiteral("column"));

            int line = 0;
            int column = 0;
            bool setCursor = false;

            if (!lineStr.isEmpty()) {
                line = lineStr.toInt();
                line > 0 && line--;
                setCursor = true;
            }

            if (!columnStr.isEmpty()) {
                column = columnStr.toInt();
                column > 0 && column--;
                setCursor = true;
            }

            if (setCursor) {
                cursor.setPosition(line, column);
            }
        }
    }

    /**
     * url computed out of the passed path
     */
    QUrl url;

    /**
     * initial cursor position, if any found inside the path as line/column specification at the end
     */
    KTextEditor::Cursor cursor;
};

#endif // URLINFO_H
