/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2018 Gregor Mi <codestruct@posteo.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 **/

#ifndef KATE_FILEACTIONS_H
#define KATE_FILEACTIONS_H

#include <utility>
#include <vector>

#include <QString>

class QWidget;
namespace KTextEditor
{
class Document;
}

namespace KateFileActions
{
/**
 * Copies the file path to clipboard.
 * If the document has no file, the clipboard will be emptied.
 */
void copyFilePathToClipboard(KTextEditor::Document *document);

/**
 * Tries to open and highlight the underlying url in the filemanager
 */
void openContainingFolder(KTextEditor::Document *document);

/**
 * Shows a Rename dialog to rename the file associated with the document.
 * The document will be closed an reopened.
 *
 * Nothing is done if the document is nullptr or has no associated file.
 */
void renameDocumentFile(QWidget *parent, KTextEditor::Document *document);

void openFilePropertiesDialog(KTextEditor::Document *document);

/**
 * Asks the user if the file should really be deleted. If yes, the file
 * is deleted from disk and the document closed.
 *
 * Nothing is done if the document is nullptr or has no associated file.
 */
void deleteDocumentFile(QWidget *parent, KTextEditor::Document *document);

/**
 * @returns a list of supported diff tools (names of the executables)
 */
QStringList supportedDiffTools();

/**
 * Runs an external program to compare the underlying files of two given documents.
 *
 * @p diffExecutable tested to work with "kdiff3", "kompare", and "meld"
 * @see supportedDiffTools()
 *
 * The parameters documentA and documentB must not be nullptr. Otherwise an assertion fails.
 *
 * If @p documentA or @p documentB have an empty url,
 * then an empty string is passed to the diff program instead of a local file path.
 *
 * @returns true if program was started successfully; otherwise false
 * (which can mean the program is not installed)
 *
 * IDEA for later: compare with unsaved buffer data instead of underlying file
 */
bool compareWithExternalProgram(KTextEditor::Document *documentA, KTextEditor::Document *documentB, const QString &diffExecutable);
}

#endif
