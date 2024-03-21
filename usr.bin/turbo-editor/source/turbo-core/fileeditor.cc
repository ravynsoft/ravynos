#define Uses_MsgBox
#include <tvision/tv.h>

#include <turbo/fileeditor.h>
#include <turbo/tpath.h>

#include <memory>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "utils.h"

namespace turbo {

class PropertyDetector
{
    enum : uint {
        ndEOL = 0x0001,
    };

    uint notDetected {ndEOL};
    int eolType {SC_EOL_LF}; // Default EOL type is LF.

public:

    void analyze(TStringView text);
    void apply(TScintilla &scintilla) const;

};

void PropertyDetector::analyze(TStringView text)
{
    if (text.size())
    {
        char cur = text[0];
        char next = text.size() > 0 ? text[1] : '\0';
        size_t i = 1;
        while (notDetected)
        {
            if (notDetected & ndEOL)
            {
                if (cur == '\r' && next == '\n')
                    eolType = SC_EOL_CRLF, notDetected &= ~ndEOL;
                else if (cur == '\n')
                    eolType = SC_EOL_LF, notDetected &= ~ndEOL;
                else if (cur == '\r')
                    eolType = SC_EOL_CR, notDetected &= ~ndEOL;
            }
            if (++i < text.size())
            {
                cur = next;
                next = text[i];
            }
            else
                break;
        }
    }
}

void PropertyDetector::apply(TScintilla &scintilla) const
{
    call(scintilla, SCI_SETEOLMODE, eolType, 0U);
}

static thread_local char ioBuffer alignas(4*1024) [128*1024];

bool readFile(TScintilla &scintilla, const char *path, FileDialogs &dlgs) noexcept
// Pre: 'scintilla' has no text in it.
{
    using std::ios;
    std::ifstream f(path, ios::in | ios::binary);
    if (f)
    {
        f.seekg(0, ios::end);
        size_t bytesLeft = f.tellg();
        f.seekg(0);
        try
        {
            PropertyDetector props;
            bool ok = true;
            size_t allocBytes = 0;
            size_t readSize;
            while ( readSize = min(bytesLeft, sizeof(ioBuffer)),
                    readSize > 0 && (ok = (bool) f.read(ioBuffer, readSize)) )
            {
                if (allocBytes == 0)
                {
                    // Allocate 1000 extra bytes, like SciTE does.
                    allocBytes = 1000 + min<size_t>(bytesLeft, ((size_t) -1)/2 - 1000);
                    call(scintilla, SCI_ALLOCATE, allocBytes, 0U);
                }
                props.analyze({ioBuffer, readSize});
                call(scintilla, SCI_APPENDTEXT, readSize, (sptr_t) ioBuffer);
                bytesLeft -= readSize;
            }
            if (!ok)
                return dlgs.readError(path, strerror(errno));
            props.apply(scintilla);
        }
        catch (const std::bad_alloc &)
        {
            call(scintilla, SCI_CLEARALL, 0U, 0U);
            return dlgs.fileTooBigError(path, bytesLeft);
        }
    }
    else
        return dlgs.openForReadError(path, strerror(errno));
    return true;
}

void openFile( TFuncView<TScintilla&()> createScintilla,
               TFuncView<void(TScintilla &, const char *)> accept, FileDialogs &dlgs ) noexcept
{
    dlgs.getOpenPath([&] (const char *path) {
        auto &scintilla = createScintilla();
        if (readFile(scintilla, path, dlgs))
        {
            accept(scintilla, path);
            return true;
        }
        destroyScintilla(scintilla);
        return false;
    });
}

bool writeFile(const char *path, TScintilla &scintilla, FileDialogs &dlgs) noexcept
{
    using std::ios;
    std::ofstream f(path, ios::out | ios::binary);
    if (f)
    {
        size_t length = call(scintilla, SCI_GETLENGTH, 0U, 0U);
        bool ok = true;
        size_t writeSize;
        size_t written = 0;
        do {
            writeSize = min(length - written, sizeof(ioBuffer));
            call(scintilla, SCI_SETTARGETRANGE, written, written + writeSize);
            call(scintilla, SCI_GETTARGETTEXT, 0U, (sptr_t) ioBuffer);
            written += writeSize;
        } while (writeSize > 0 && (ok = (bool) f.write(ioBuffer, writeSize)));
        if (!ok)
            return dlgs.writeError(path, strerror(errno));
    }
    else
        return dlgs.openForWriteError(path, strerror(errno));
    return true;
}

bool renameFile(const char *dst, const char *src, TScintilla &scintilla, FileDialogs &dlgs) noexcept
{
    // Try saving first, then renaming.
    if (writeFile(src, scintilla, showNoDialogs) && ::rename(src, dst) == 0)
        return true;
    // If the above doesn't work, try saving at the new location, and then remove
    // the old file.
    else if (writeFile(dst, scintilla, showNoDialogs))
    {
        if (TPath::exists(src) && ::remove(src) != 0)
            dlgs.removeRenamedWarning(dst, src, strerror(errno));
        return true;
    }
    return dlgs.renameError(dst, src, strerror(errno));
}

bool FileEditor::save(FileDialogs &dlgs) noexcept
{
    if (filePath.empty())
        return saveAs(dlgs);
    beforeSave();
    bool existed = TPath::exists(filePath.c_str());
    if (writeFile(filePath.c_str(), scintilla, dlgs))
    {
        if (!existed)
            onFilePathSet();
        afterSave();
        return true;
    }
    return false;
}

bool FileEditor::saveAs(FileDialogs &dlgs) noexcept
{
    bool ok = false;
    dlgs.getSaveAsPath(*this, [&] (const char *path) {
        beforeSave();
        if (writeFile(path, scintilla, dlgs))
        {
            setFilePath(path);
            afterSave();
            return (ok = true);
        }
        return false;
    });
    return ok;
}

bool FileEditor::rename(FileDialogs &dlgs) noexcept
{
    if (filePath.empty())
        return saveAs(dlgs);
    bool ok = false;
    dlgs.getRenamePath(*this, [&] (const char *path) {
        beforeSave();
        if (renameFile(path, filePath.c_str(), scintilla, dlgs))
        {
            setFilePath(path);
            afterSave();
            return (ok = true);
        }
        return false;
    });
    return ok;
}

bool FileEditor::close(FileDialogs &dlgs) noexcept
{
    if (!inSavePoint())
    {
        auto reply = filePath.empty() ?
            dlgs.confirmSaveUntitled(*this) :
            dlgs.confirmSaveModified(*this);
        return (reply == cmYes && save(dlgs)) || reply == cmNo;
    }
    return true;
}

void FileEditor::beforeSave() noexcept
{
    if (!inSavePoint() && !call(scintilla, SCI_CANREDO, 0U, 0U))
    {
        call(scintilla, SCI_BEGINUNDOACTION, 0U, 0U);
        stripTrailingSpaces(scintilla);
        ensureNewlineAtEnd(scintilla);
        call(scintilla, SCI_ENDUNDOACTION, 0U, 0U);
    }
}

void FileEditor::afterSave() noexcept
{
    call(scintilla, SCI_SETSAVEPOINT, 0U, 0U);
}

void FileEditor::detectLanguage() noexcept
{
    language = detectFileLanguage(filePath.c_str());
    lexer = findBuiltInLexer(language);
    applyTheming(lexer, scheme, scintilla);
}

void FileEditor::onFilePathSet() noexcept
{
    detectLanguage();
}

ShowAllDialogs showAllDialogs;
ShowNoDialogs showNoDialogs;
AcceptMissingFilesOnOpen acceptMissingFilesOnOpen;

ushort ShowAllDialogs::confirmSaveUntitled(FileEditor &) noexcept
{
    return messageBox("Save untitled file?", mfConfirmation | mfYesNoCancel);
}

ushort ShowAllDialogs::confirmSaveModified(FileEditor &editor) noexcept
{
    return messageBox( mfConfirmation | mfYesNoCancel,
                       "'%s' has been modified. Save?", editor.filePath.c_str() );
}

ushort ShowAllDialogs::confirmOverwrite(const char *path) noexcept
{
    return messageBox( mfConfirmation | mfYesButton | mfNoButton,
                       "'%s' already exists. Overwrite?", path );
}

void ShowAllDialogs::removeRenamedWarning(const char *dst, const char *src, const char *cause) noexcept
{
    messageBox( mfWarning | mfOKButton,
                "'%s' was created successfully, but '%s' could not be removed: %s.", dst, src, cause );
}

bool ShowAllDialogs::renameError(const char *dst, const char *src, const char *cause) noexcept
{
    messageBox( mfError | mfOKButton,
                "Unable to rename '%s' into '%s': %s.", src, dst, cause );
    return false;
}

bool ShowAllDialogs::fileTooBigError(const char *path, size_t size) noexcept
{
    messageBox( mfError | mfOKButton,
                "Unable to open file '%s': file too big (%zu bytes).", path, size );
    return false;
}

bool ShowAllDialogs::readError(const char *path, const char *cause) noexcept
{
    messageBox( mfError | mfOKButton,
                "Cannot read from file '%s': %s.", path, cause );
    return false;
}

bool ShowAllDialogs::writeError(const char *path, const char *cause) noexcept
{
    messageBox( mfError | mfOKButton,
                "Cannot write into file '%s': %s.", path, cause );
    return false;
}

bool ShowAllDialogs::openForReadError(const char *path, const char *cause) noexcept
{
    messageBox( mfError | mfOKButton,
                "Unable to open file '%s' for read: %s.", path, cause );
    return false;
}

bool ShowAllDialogs::openForWriteError(const char *path, const char *cause) noexcept
{
    messageBox( mfError | mfOKButton,
                "Unable to create or open file '%s' for write: %s. Make sure that the parent directory exists, that you have write access to this file and that enough disk space is available. Otherwise, try saving to a different location.", path, cause );
    return false;
}

void ShowAllDialogs::getOpenPath(TFuncView<bool (const char *)> accept) noexcept
{
    char path[MAXPATH];
    openFileDialog("*.*", "Open file", "~N~ame", fdOpenButton, 0, [&] (TView *dialog) {
        dialog->getData(path);
        fexpand(path);
        return accept(path);
    });
}

static bool canOverwrite(FileDialogs &dlgs, const char *path) noexcept
{
    return !TPath::exists(path) || dlgs.confirmOverwrite(path) == cmYes;
}

void ShowAllDialogs::getSaveAsPath(FileEditor &editor, TFuncView<bool (const char *)> accept) noexcept
{
    char path[MAXPATH];
    std::ostringstream os;
    if (editor.filePath.empty())
        os << "Save untitled file";
    else
        os << "Save file '" << TPath::basename(editor.filePath) << "' as";
    openFileDialog("*.*", os.str(), "~N~ame", fdOKButton, 0, [&] (TView *dialog) {
        dialog->getData(path);
        fexpand(path);
        return canOverwrite(*this, path) && accept(path);
    });
}

void ShowAllDialogs::getRenamePath(FileEditor &editor, TFuncView<bool (const char *)> accept) noexcept
{
    char path[MAXPATH];
    std::ostringstream os;
    os << "Rename file '" << TPath::basename(editor.filePath) << "'";
    openFileDialog("*.*", os.str(), "~N~ame", fdOKButton, 0, [&] (TView *dialog) {
        dialog->getData(path);
        fexpand(path);
        // Don't do anything if renaming to the same file. If the user needed to
        // save the file, they would use the 'save' feature.
        return strcmp(path, editor.filePath.c_str()) == 0 ||
            (canOverwrite(*this, path) && accept(path));
    });
}

ushort ShowNoDialogs::confirmSaveUntitled(FileEditor &) noexcept { return cmCancel; }
ushort ShowNoDialogs::confirmSaveModified(FileEditor &editor) noexcept { return cmCancel; }
ushort ShowNoDialogs::confirmOverwrite(const char *path) noexcept { return cmCancel; }
void ShowNoDialogs::removeRenamedWarning(const char *dst, const char *src, const char *cause) noexcept {}
bool ShowNoDialogs::renameError(const char *dst, const char *src, const char *cause) noexcept { return false; }
bool ShowNoDialogs::fileTooBigError(const char *path, size_t size) noexcept { return false; }
bool ShowNoDialogs::readError(const char *path, const char *cause) noexcept { return false; }
bool ShowNoDialogs::writeError(const char *path, const char *cause) noexcept { return false; }
bool ShowNoDialogs::openForReadError(const char *path, const char *cause) noexcept { return false; }
bool ShowNoDialogs::openForWriteError(const char *path, const char *cause) noexcept { return false; }
void ShowNoDialogs::getOpenPath(TFuncView<bool (const char *)> accept) noexcept {}
void ShowNoDialogs::getSaveAsPath(FileEditor &editor, TFuncView<bool (const char *)> accept) noexcept {}
void ShowNoDialogs::getRenamePath(FileEditor &editor, TFuncView<bool (const char *)> accept) noexcept {}

bool AcceptMissingFilesOnOpen::openForReadError(const char *path, const char *cause) noexcept
{
    if (TPath::exists(path))
        return ShowAllDialogs::openForReadError(path, cause);
    return true;
}

} // namespace turbo
