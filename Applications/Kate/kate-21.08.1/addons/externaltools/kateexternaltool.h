/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KTEXTEDITOR_KATE_EXTERNALTOOL_H
#define KTEXTEDITOR_KATE_EXTERNALTOOL_H

#include <QMetaType>
#include <QString>
#include <QStringList>

class KConfigGroup;

/**
 * This class defines a single external tool.
 */
class KateExternalTool
{
public:
    /**
     * Defines whether any document should be saved before running the tool.
     */
    enum class SaveMode {
        //! Do not save any document.
        None,
        //! Save current document.
        CurrentDocument,
        //! Save all documents
        AllDocuments
    };

    /**
     * Defines where to redirect stdout from the tool.
     */
    enum class OutputMode {
        Ignore,
        InsertAtCursor,
        ReplaceSelectedText,
        ReplaceCurrentDocument,
        AppendToCurrentDocument,
        InsertInNewDocument,
        CopyToClipboard,
        DisplayInPane
    };

public:
    /// The category used in the menu to categorize the tool.
    QString category;
    /// The name used in the menu.
    QString name;
    /// the icon to use in the menu.
    QString icon;
    /// The name or path of the executable.
    QString executable;
    /// The command line arguments.
    QString arguments;
    /// The stdin input.
    QString input;
    /// The working directory, if specified.
    QString workingDir;
    /// Optional list of mimetypes for which this action is valid.
    QStringList mimetypes;
    /// The name for the action for persistent keyboard shortcuts.
    /// This is generated first time the action is is created.
    QString actionName;
    /// The name for the commandline.
    QString cmdname;
    /// Possibly save documents prior to activating the tool command.
    SaveMode saveMode = SaveMode::None;
    /// Reload current document after execution
    bool reload = false;
    /// Defines where to redirect the tool's output
    OutputMode outputMode = OutputMode::Ignore;

public:
    /// This is set when loading the Tool from disk.
    bool hasexec = false;

    /**
     * @return true if mimetypes is empty, or the @p mimetype matches.
     */
    bool matchesMimetype(const QString &mimetype) const;

    /**
     * @return true if "executable" exists and has the executable bit set, or is
     * empty.
     * This is run at least once, and the tool is disabled if it fails.
     */
    bool checkExec() const;

    /**
     * Load tool data from the config group @p cg.
     */
    void load(const KConfigGroup &cg);

    /**
     * Save tool data to the config group @p cg.
     */
    void save(KConfigGroup &cg) const;

    /**
     * Returns the translated name if possible.
     */
    QString translatedName() const;

    /**
     * Returns the translated category if possible.
     */
    QString translatedCategory() const;
};

/**
 * Compares for equality. All fields have to match.
 */
bool operator==(const KateExternalTool &lhs, const KateExternalTool &rhs);

// for use in QVariant (QAction::setData() and QAction::data())
Q_DECLARE_METATYPE(KateExternalTool *)

#endif // KTEXTEDITOR_KATE_EXTERNALTOOL_H

// kate: space-indent on; indent-width 4; replace-tabs on;
