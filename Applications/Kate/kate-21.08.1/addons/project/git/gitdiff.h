/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
    SPDX-FileCopyrightText: 2020 Jonathan Verner <jonathan.verner@matfyz.cz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KDEVPLATFORM_VCSDIFF_H
#define KDEVPLATFORM_VCSDIFF_H

#include <QMetaType>
#include <QVector>
#include <QtGlobal>
#include <memory>

class QUrl;
class QString;

/**
 * A class representing a unified diff, possibly with
 * conflict markers.
 *
 * A diff is assumed to be a collection of hunks, where each hunk has the
 * following structure:
 *
 *   METADATA
 *   --- a/SOURCE_PATH
 *   +++ b/TARGET_PATH
 *   HUNK1 HEADER
 *   HUNK1 CONTENT
 *   [METADATA]
 *   HUNK2 HEADER
 *   HUNK2 CONTENT
 *   ...
 *
 * METADATA are lines which start with anything except for a '+', '-' and ' '.
 * The path specifications may optionally precede a hunk and are assumed to
 * apply to all following hunks until a new path specification
 * is found. These indicate the files for which the diff is generated.
 *
 * Hunk Header
 * ------------
 *
 * Each hunk header has the following form
 *
 *   @@ -SRC_OFFSET[, SRC_CHANGES_COUNT] +TGT_OFFSET[, TGT_CHANGES_COUNT] @@ Heading
 *
 * where the SRC_OFFSET is a 1-based line index pointing to the source file where
 * the hunk applies and TGT_OFFSET is a 1-based line index pointing to the target
 * file where the hunk applies. The optional SRC_CHANGES_COUNTS (assumed to be 1
 * if not present) specifies the number of context lines plus the number of
 * deleted lines. Similarly, the optional TGT_CHANGES_COUNT specifies the
 * number of context lines plus the number of added lines. The Heading, used as a
 * visual aid for users, is supposed to show the line where the nearest enclosing
 * function scope of the hunk starts.
 *
 * Hunk Content
 * ------------
 *
 * The hunk content is a collection of lines which starting with '+' (additions),
 * '-' (deletions) and ' ' (context lines; empty lines are also take to be context
 * lines). Additionally, a hunk may contain conflict markers which are of the form
 *
 *   >>>>>>> our ref
 *   our content
 *   ...
 *   =======
 *   their content
 *   ...
 *   <<<<<<< their ref
 *
 * and indicate unresolved conflicts.
 *
 */
class VcsDiff
{
public:
    /* Used to represent a patch or its inverse */
    enum DiffDirection {
        Normal = 0 /**< the unchanged patch */
        ,
        Forward = 0 /**< the unchanged patch */
        ,
        Reverse = 1 /**< the inverse of the patch (i.e. a new patch which, when applied, undoes the old patch) */
    };

    VcsDiff();
    ~VcsDiff();
    VcsDiff(VcsDiff &&rhs);

    /**
     * @returns the source of the diff.
     */
    QString diff() const;

    /** @returns the base directory of the diff. */
    QUrl baseDiff() const;

    /**
     * Depth - number of directories to left-strip from paths in the patch - see "patch -p"
     * Defaults to 0
     */
    uint depth() const;

    /** Sets the base directory of the diff to the @p url */
    void setBaseDiff(const QUrl &url);

    /** Sets the depth of the diff to @p depth */
    void setDepth(const uint depth);

    /** Sets the diff source and parses it.
     *
     * @param diff the diff in unified diff format
     */
    void setDiff(const QString &diff);

    /** @returns whether or not there are changes in the diff */
    bool isEmpty() const;

    /**
     * Creates a standalone diff containing the differences in a given range.
     *
     * @returns a diff containing only the changes from the current diff
     *  which are in the range startLine-endLine (in the diff text)
     *
     * @param startLine 0-based line number (in the diff) of the first line in the range
     * @param endLine 0-based line number (in the diff) of the last line in the range
     * @param dir if set to Reverse, the role of src and tgt are reversed, i.e. a diff is
     *            generated which can be applied to the target to get the source.
     */
    VcsDiff subDiff(const uint startLine, const uint endLine, DiffDirection dir = Normal) const;

    /**
     * Creates a new standalone diff from a single hunk identified by a containing line.
     *
     * @returns a diff containing only the changes from hunk containing
     * the line the line
     *
     * @param line 0-based line number (in the diff) of the line in the hunk
     * @param dir if set to Reverse, the role of src and tgt are reversed, i.e. a diff is
     *            generated which can be applied to the target to get the source.
     */
    VcsDiff subDiffHunk(const uint line, DiffDirection dir = Normal) const;

    /**
     * Maps a line position in the diff to a corresponding line position in the source file.
     *
     * @param line a 0-based line position in the diff
     * @returns the 0-based line position in the source file or -1 if no such position exists.
     */
    int diffLineToSourceLine(const uint line) const;

    /**
     * Maps a line position in the diff to a corresponding line position in the source file.
     *
     * @param line a 0-based line position in the diff
     * @returns the 0-based line position in the source file or -1 if no such position exists.
     */
    int diffLineToTargetLine(const uint line) const;

    /**
     * Represents a pair of files which are compared
     */
    struct FilePair {
        QString source;
        QString target;
        bool operator==(const FilePair p) const
        {
            return (source == p.source && target == p.target);
        }
    };

    /**
     * @returns a list of filename pairs that the patch applies to
     *
     * @note: Each file-pair is only listed once for each consecutive run
     * of hunks which apply to it.
     */
    const QVector<FilePair> fileNames() const;

private:
    std::unique_ptr<class VcsDiffPrivate> d;
};

#endif
