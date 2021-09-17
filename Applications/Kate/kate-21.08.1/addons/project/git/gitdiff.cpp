/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2020 Jonathan Verner <jonathan.verner@matfyz.cz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "gitdiff.h"

#include <QDebug>
#include <QRegularExpression>
#include <QSharedData>
#include <QString>
#include <QUrl>
#include <QtGlobal>

/* A class representing a diff hunk (a collection of localized changes) */
class DiffHunk
{
public:
    /* Metadata for the hunk */
    uint srcStart /**< the 1-based (!) start line number of the range in the source file where the hunk applies */
        ,
        srcCount /**< the size of the range (in # of lines) in the source where the hunk applies  (i.e. ctx lines + deleted lines)*/
        ,
        tgtStart /**< the 1-based (!) start line number of the range in the target file where the hunk applies */
        ,
        tgtCount /**< the size of the range (in # of lines) in the target where the hunk applies  (i.e. ctx lines + deleted lines)*/
        ,
        headingLineIdx /**< The 0-based line number (in the whole diff) of the hunk header line (the one starting with `@@`) */
        ;
    QString srcFile /**< The source filename */
        ,
        tgtFile /**< The target filename */
        ,
        heading /**< The heading of the hunk (the stuff in the header line after the position spec, i.e. after the second `@@`) */
        ;
    QStringList lines; /**< The lines comprising the hunk (excluding the header) */

    /**
     * @returns the 0-based line number (in the whole diff) of the last line contained in the hunk.
     */
    uint lastLineIdx() const
    {
        return headingLineIdx + lines.size();
    }

    /**
     * @param lineIdx the 0-based line number of the tested line in the whole diff
     * @returns true if the line is part of the diff and false otherwise
     * @note: Returns true also for the header line (the one starting with `@@`)
     */
    bool containsDiffLine(uint lineIdx) const
    {
        return headingLineIdx <= lineIdx && lineIdx <= lastLineIdx();
    }

    /**
     * Returns the index of the line within the hunk
     *
     * @param diffLineIdx the 0-based indes of the line in the diff
     *
     * @note assumes that the line is contained within the hunk
     * @note if the line is a header line, -1 is returned; otherwise the returned
     * number is the index of the line in the `lines` list
     */
    int diffLineToHunkLine(uint diffLineIdx) const
    {
        return diffLineIdx - (headingLineIdx + 1);
    }

    /**
     * A helper method to construct a hunk header from the provided info
     *
     * A hunk header has the following form:
     *
     *      @@ oldStart,oldCount newStart,newCount @@ heading
     * e.g.
     *      @@ -36,14 +36,28 @@ public:
     *
     * @returns the hunk header
     */
    static QString formatHeader(uint oldStart, uint oldCount, uint newStart, uint newCount, QString head);

    /**
     * The following operators define a PARTIAL order on the hunks list.
     * A hunk H is strictly below a hunk K iff the endline of H is strictly below
     * the start line of K. In particular, the only non-overlapping hunks are
     * ordered.
     */
    bool operator<(const DiffHunk &b) const
    {
        return lastLineIdx() < b.headingLineIdx;
    }
    bool operator<(uint line) const
    {
        return lastLineIdx() < line;
    }
    bool operator<=(const DiffHunk &b) const
    {
        return lastLineIdx() <= b.headingLineIdx;
    }
    bool operator<=(uint line) const
    {
        return lastLineIdx() <= line;
    }
    bool operator>(const DiffHunk &b) const
    {
        return headingLineIdx > b.lastLineIdx();
    }
    bool operator>(uint line) const
    {
        return headingLineIdx > line;
    }
    bool operator>=(const DiffHunk &b)
    {
        return headingLineIdx >= b.lastLineIdx();
    }
    bool operator>=(uint line) const
    {
        return headingLineIdx >= line;
    }
};

/* RegExp matching a hunk header line */
Q_GLOBAL_STATIC_WITH_ARGS(const QRegularExpression, HUNK_HEADER_RE, (QLatin1String("^@@ -([0-9,]+) \\+([0-9,]+) @@(.*)")))
// static const auto HUNK_HEADER_RE = QRegularExpression(QStringLiteral("^@@ -([0-9,]+) \\+([0-9,]+) @@(.*)"));

/* RegExp matching a meta line containing a source of target filename */
Q_GLOBAL_STATIC_WITH_ARGS(const QRegularExpression, DIFF_FILENAME_RE, (QLatin1String("^[-+]{3} [ab]/(.*)")))

/* RegExp matching a meta line (hunk header, filename, other info) */
Q_GLOBAL_STATIC_WITH_ARGS(const QRegularExpression, META_LINE_RE, (QLatin1String("(^[-+]{3} )|^[^-+ ]")))

/* RegExps matching conflict delimiting lines */
Q_GLOBAL_STATIC_WITH_ARGS(const QRegularExpression, CONFLICT_START_RE, (QLatin1String("^<<<<<<<")))
Q_GLOBAL_STATIC_WITH_ARGS(const QRegularExpression, CONFLICT_MID_RE, (QLatin1String("^=======")))
Q_GLOBAL_STATIC_WITH_ARGS(const QRegularExpression, CONFLICT_END_RE, (QLatin1String("^>>>>>>>")))
Q_GLOBAL_STATIC_WITH_ARGS(const QRegularExpression, CONFLICT_RE, (QLatin1String("(^>>>>>>>)|(^=======)|(^<<<<<<<)")))

QString formatRange(uint start, uint count)
{
    if (count == 1)
        return QString().setNum(start);
    return QString().setNum(start) + QLatin1Char(',') + QString().setNum(count);
}
std::pair<uint, uint> parseRange(const QString &range)
{
    int commaPos = range.indexOf(QLatin1Char(','));
    if (commaPos > -1) {
        return {range.midRef(0, commaPos).toInt(), range.midRef(commaPos + 1).toInt()};
    }
    return {range.toInt(), 1};
}

/* Creates a hunk header line (starting with @@)
 *
 * Note: The line will not end with a newline */
QString DiffHunk::formatHeader(uint oldStart, uint oldCount, uint newStart, uint newCount, QString head)
{
    return QLatin1String("@@ -") + formatRange(oldStart, oldCount) + QLatin1String(" +") + formatRange(newStart, newCount) + QLatin1String(" @@") + head;
}

/**
 * Parses a unified diff into a list of "diff hunks" (each hunk starts with a
 * line starting with @@ and represents a collection of localized changes).
 *
 * @param diff a diff in git's unified diff format
 * @returns a list of hunk structures
 *
 * The diff is assumed to be a collection of hunks, where each hunk has the
 * following structure:
 *
 *   METADATA
 *   --- a/SOURCE_PATH
 *   +++ b/TARGET_PATH
 *   HUNK HEADER
 *   HUNK CONTENT
 *
 * All metadata lines match the @ref:META_LINE_RE regexp (starts with anything
 * except for a '+', '-' and ' ') and these are discarded except for the hunk
 * header and source/target path specifications. The path specifications
 * are assumed to apply to all following hunks until a new path specification
 * is found and are stored in the srcFileName and tgtFileName attributes of
 * the hunk structure.
 *
 *
 * Hunk Header
 * -----------
 *
 * The hunk header has the following form
 *
 *   @@ -SRC_OFFSET[, SRC_CHANGES_COUNT] +TGT_OFFSET[, TGT_CHANGES_COUNT] @@ Heading
 *
 * where the SRC_OFFSET is a 1-based line index pointing to the source file where
 * the hunk applies and TGT_OFFSET is a 1-based line index pointing to the target
 * file where the hunk applies. These are parsed into the srcStart and tgtStart
 * attributes of the hunk structure.
 *
 * The optional SRC_CHANGES_COUNTS (assumed to be 1 if not present) specifies the
 * number of context lines (starting with ' ') plus the number of deleted lines
 * (starting with '-'). Similarly, the optional TGT_CHANGES_COUNT specifies the
 * number of context lines plus the number of added lines (starting with '+').
 * These are parsed and stored in the srcCount and tgtCount attributes of the hunk
 * structure, but not checked (!). I.e. if the diff hunk has more/less changes then
 * specified, the returned hunk structure will have invalid src & tgt counts.
 *
 * Finally the Heading, used as a visual aid for users, is supposed to show the line
 * where the nearest enclosing function scope of the hunk starts. It is parsed and
 * stored in the heading attribute.
 *
 * Hunk Content
 * ------------
 *
 * The hunk content is a collection of lines which
 *
 *   1) start with '+' (additions); or
 *   2) start with '-' (deletions); or
 *   3) start with ' ' (context lines); or
 *   4) are empty (context lines); or
 *   5) are within conflict markers
 *
 * These lines are parsed and stored in the lines attribute of the hunk structure.
 * The parsing of the hunk stops when a METADATA line (outside of conflict markers)
 * is encountered or the end of the file is reached.
 *
 * Conflict Markers
 * ----------------
 *
 * Conflict markers are collections of lines of the form:
 *
 *   >>>>>>> our ref
 *   our content
 *   ...
 *   =======
 *   their content
 *   ...
 *   <<<<<<< their ref
 *
 * And show content which a merge was not able to merge automatically.
 * Strictly speaking, these should not appear in diffs, but git diff
 * generates them anyway for files with unresolved conflicts.
 */
std::vector<DiffHunk> parseHunks(VcsDiff &diff)
{
    std::vector<DiffHunk> ret;
    int lineNo = -1;
    QString curSrcFileName, curTgtFileName;
    QStringListIterator lines(diff.diff().split(QLatin1Char('\n')));
    while (lines.hasNext()) {
        lineNo++;
        auto curln = lines.next();
        auto m = DIFF_FILENAME_RE->match(curln);
        if (m.hasMatch()) {
            if (curln.startsWith(QLatin1Char('-')))
                curSrcFileName = m.captured(1);
            else if (curln.startsWith(QLatin1Char('+')))
                curTgtFileName = m.captured(1);
            continue;
        }
        m = HUNK_HEADER_RE->match(curln);
        if (!m.hasMatch())
            continue;
        const auto oldRange = parseRange(m.captured(1));
        const auto newRange = parseRange(m.captured(2));
        const auto heading = m.captured(3);
        uint firstLineIdx = lineNo;
        QStringList hunkLines;
        while (lines.hasNext() && (CONFLICT_START_RE->match(lines.peekNext()).hasMatch() || !META_LINE_RE->match(lines.peekNext()).hasMatch())) {
            // Consume the conflict
            if (CONFLICT_START_RE->match(lines.peekNext()).hasMatch()) {
                lineNo++;
                hunkLines << lines.next();
                while (lines.hasNext() && !CONFLICT_END_RE->match(lines.peekNext()).hasMatch()) {
                    lineNo++;
                    hunkLines << lines.next();
                }
                if (!CONFLICT_END_RE->match(lines.peekNext()).hasMatch()) {
                    qWarning() << "Invalid diff format, end of file reached before conflict finished";
                    qDebug() << diff.diff();
                    break;
                }
            }
            lineNo++;
            hunkLines << lines.next();
        }

        // The number of filenames present in the diff should match the number
        // of hunks
        ret.push_back(
            DiffHunk{oldRange.first, oldRange.second, newRange.first, newRange.second, firstLineIdx, curSrcFileName, curTgtFileName, heading, hunkLines});
    }

    // If the diff ends with a newline, for the last hunk, when splitting into lines above
    // we will always get an empty string at the end, which we now remove
    if (diff.diff().endsWith(QLatin1Char('\n'))) {
        if (ret.size() > 0 && ret.back().lines.size() > 0) {
            ret.back().lines.pop_back();
        } else {
            qWarning() << "Failed to parse a diff, produced no hunks";
            qDebug() << "Failed diff:" << diff.diff();
        }
    }

    return ret;
}

class VcsDiffPrivate
{
public:
    QUrl baseDiff;
    QString diff;
    uint depth = 0;
    std::vector<DiffHunk> hunks;

    enum Dest {
        SRC = '-',
        TGT = '+',
    };

    /**
     * Maps a line position in the diff to a corresponding line position in the destination file.
     *
     * @param line a 0-based line position in the diff
     * @param dest specifies the destination file to map to:
     *             either SRC (the source file, '-') or TGT (the target file, '+')
     * @returns the 0-based line position in the destination file or -1 if no such position exists.
     */
    int mapDiffLine(const uint line, const Dest dest) const
    {
        const QLatin1Char skipChar = (dest == SRC) ? QLatin1Char(TGT) : QLatin1Char(SRC);
        for (const auto &h : hunks) {
            if (h.containsDiffLine(line)) {
                int hunkPos = h.diffLineToHunkLine(line);

                // The line refers to the heading line
                if (hunkPos < 0)
                    return -1;

                // Any lines in the diff hunk which come before line and come from the opposite
                // of dest should not be counted (they are not present in the dest)
                int skipCount = 0;
                for (int i = 0; i < hunkPos; i++) {
                    if (h.lines.at(i).startsWith(skipChar))
                        skipCount++;
                }

                // Any lines in the diff hunk which come from the second part (src)/ first part (tgt)
                // of a conflict should not be counted either
                bool inConflict = false; // This is set so that a line inside a conflict is recognized as a valid line
                for (int i = 0; i < hunkPos; i++) {
                    if (CONFLICT_START_RE->match(h.lines.at(i)).hasMatch()) {
                        skipCount++; // skip the conflict marker line
                        if (dest == TGT) {
                            while ((++i) < hunkPos && !CONFLICT_MID_RE->match(h.lines.at(i)).hasMatch()) {
                                skipCount++;
                            }
                        } else {
                            inConflict = true;
                        }
                    }
                    if (CONFLICT_MID_RE->match(h.lines.at(i)).hasMatch()) {
                        skipCount++; // skip the conflict marker line
                        if (dest == SRC) {
                            while ((++i) < hunkPos && !CONFLICT_END_RE->match(h.lines.at(i)).hasMatch())
                                skipCount++;
                        } else {
                            inConflict = true;
                        }
                    }
                    if (CONFLICT_END_RE->match(h.lines.at(i)).hasMatch()) {
                        skipCount++; // skip the conflict marker line
                        inConflict = false;
                    }
                }

                auto ln = h.lines[hunkPos];

                // This works around the fact that inConflict is set even if hunkPos
                // ends up hitting a conflict marker
                if (CONFLICT_RE->match(ln).hasMatch())
                    return -1;

                if (ln.startsWith(dest) || ln.startsWith(QLatin1Char(' ')) || ln.isEmpty() || inConflict) {
                    if (dest == SRC)
                        // The -1 accounts for the fact that srcStart is 1-based
                        // but we need to return 0-based line numbers
                        return h.srcStart - 1 + hunkPos - skipCount;
                    else
                        // The -1 accounts for the fact that srcStart is 1-based
                        // but we need to return 0-based line numbers
                        return h.tgtStart - 1 + hunkPos - skipCount;
                } else
                    return -1;
            }
        }
        return -1;
    }
};

VcsDiff VcsDiff::subDiffHunk(const uint line, DiffDirection dir) const
{
    for (const auto &hunk : d->hunks) {
        if (hunk.containsDiffLine(line)) {
            return subDiff(hunk.headingLineIdx, hunk.lastLineIdx(), dir);
        }
    }

    VcsDiff emptyDiff;
    emptyDiff.setBaseDiff(d->baseDiff);
    emptyDiff.setDepth(d->depth);
    emptyDiff.setDiff(d->diff.mid(0, d->diff.indexOf(QStringLiteral("@@"))));
    return emptyDiff;
}

VcsDiff VcsDiff::subDiff(const uint startLine, const uint endLine, DiffDirection dir) const
{
    // Code adapted from cola/diffparse.py
    enum LineType { ADD = '+', DEL = '-', CTX = ' ', NO_NEWLINE = '\\' };

    VcsDiff ret;
    ret.setBaseDiff(baseDiff());
    ret.setDepth(depth());

    QStringList lines;
    for (const auto &hunk : d->hunks) {
        // Skip hunks before the first line
        if (hunk < startLine)
            continue;

        // Skip hunks after the last line
        if (hunk > endLine)
            break;

        std::map<LineType, int> counts = {{ADD, 0}, {DEL, 0}, {CTX, 0}, {NO_NEWLINE, 0}};
        QStringList filteredLines;

        // Will be set if the previous line in a hunk was
        // skipped because it was not in the selected range
        bool prevSkipped = false;

        uint lnIdx = hunk.headingLineIdx;

        // Store the number of skipped lines which start the hunk
        // (i.e. lines before a first deletion (addition in case of reverse)
        // so that we can adjust the start appropriately
        int startOffset = 0;
        const auto _lines = QStringList(hunk.lines.constBegin(), hunk.lines.constEnd());
        for (const auto &line : _lines) {
            lnIdx++;
            LineType tp = line.length() > 0 ? (LineType)line[0].toLatin1() : (LineType)0;
            QString content = line.mid(1);

            if (dir == Reverse) {
                if (tp == ADD)
                    tp = DEL;
                else if (tp == DEL)
                    tp = ADD;
            }

            if (lnIdx < startLine || endLine < lnIdx) {
                // skip additions (or deletions if reverse) that are not in range
                if (tp == ADD) {
                    prevSkipped = true;
                    // If we are before the range and
                    // so far we only encountered ADD (or DEL, in case of reverse) lines
                    // these will not be included in the subdiff hunk so we increase the startOffset
                    if (lnIdx < startLine && counts[CTX] == 0)
                        startOffset++;
                    continue;
                }
                // change deletions (or additions if reverse) that are not in range into context
                if (tp == DEL)
                    tp = CTX;
            }

            // If the line immediately before a "No newline" line was
            // skipped (because it was an unselected addition) skip
            // the "No newline" line as well.
            if (tp == NO_NEWLINE && prevSkipped) {
                if (lnIdx <= endLine)
                    startOffset++;
                continue;
            }

            // Empty lines are context lines and we
            // preserve them
            if ((int)tp == 0) {
                filteredLines << content;
                tp = CTX;
            } else {
                filteredLines << QLatin1Char(tp) + content;
            }
            counts[tp]++;
            prevSkipped = false;
        }

        // Skip hunks which have only context lines
        if (counts[ADD] + counts[DEL] == 0)
            continue;

        // Compute the start & counts of the hunks
        uint subSrcStart, subTgtStart;
        if (dir == Reverse) {
            subSrcStart = hunk.tgtStart + startOffset;
            subTgtStart = hunk.srcStart + startOffset;
        } else {
            subSrcStart = hunk.srcStart + startOffset;
            subTgtStart = hunk.tgtStart + startOffset;
        }
        uint subSrcCount = counts[CTX] + counts[DEL];
        uint subTgtCount = counts[CTX] + counts[ADD];

        // Prepend lines identifying the source files
        lines << QStringLiteral("--- a/") + ((dir == Reverse) ? hunk.tgtFile : hunk.srcFile);
        lines << QStringLiteral("+++ b/") + ((dir == Reverse) ? hunk.srcFile : hunk.tgtFile);

        lines << DiffHunk::formatHeader(subSrcStart, subSrcCount, subTgtStart, subTgtCount, hunk.heading);
        lines += filteredLines;
    }
    if (lines.size() > 2)
        ret.setDiff(lines.join(QLatin1Char('\n')) + QLatin1Char('\n'));
    return ret;
}

const QVector<VcsDiff::FilePair> VcsDiff::fileNames() const
{
    QVector<VcsDiff::FilePair> ret;
    VcsDiff::FilePair current;
    for (const auto &h : d->hunks) {
        // List each pair only once
        if (h.srcFile == current.source && h.tgtFile == current.target)
            continue;
        current = {h.srcFile, h.tgtFile};
        ret.push_back(current);
    }
    return ret;
}

int VcsDiff::diffLineToSourceLine(const uint line) const
{
    return d->mapDiffLine(line, VcsDiffPrivate::SRC);
}

int VcsDiff::diffLineToTargetLine(const uint line) const
{
    return d->mapDiffLine(line, VcsDiffPrivate::TGT);
}

VcsDiff::VcsDiff()
    : d(new VcsDiffPrivate)
{
}

VcsDiff::~VcsDiff() = default;

VcsDiff::VcsDiff(VcsDiff &&rhs)
{
    this->d = std::move(rhs.d);
}

bool VcsDiff::isEmpty() const
{
    return d->diff.isEmpty();
}

QString VcsDiff::diff() const
{
    return d->diff;
}

void VcsDiff::setDiff(const QString &s)
{
    d->diff = s;
    d->hunks = parseHunks(*this);
}

QUrl VcsDiff::baseDiff() const
{
    return d->baseDiff;
}

uint VcsDiff::depth() const
{
    return d->depth;
}

void VcsDiff::setBaseDiff(const QUrl &url)
{
    d->baseDiff = url;
}

void VcsDiff::setDepth(const uint depth)
{
    d->depth = depth;
}
