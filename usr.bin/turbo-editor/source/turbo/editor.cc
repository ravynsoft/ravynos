#define Uses_MsgBox
#include <tvision/tv.h>

#include "editor.h"
#include "editwindow.h"

void TurboEditor::onFilePathSet() noexcept
{
    bool noLastLanguage = !language;
    super::onFilePathSet();
    if (noLastLanguage && language)
        lineNumbers.setState(true);
}

ushort TurboFileDialogs::confirmSaveUntitled(turbo::FileEditor &editor) noexcept
{
    if (auto *window = (EditorWindow *) editor.parent)
    {
        auto *title = window->formatTitle(EditorWindow::tfNoSavePoint);
        return messageBox( mfConfirmation | mfYesNoCancel,
                           "Save '%s'?", title );
    }
    return super::confirmSaveUntitled(editor);
}

void TurboFileDialogs::getOpenPath(TFuncView<bool (const char *)> accept) noexcept
{
    CwdGuard cwd {app.getFileDialogDir()};
    super::getOpenPath(accept);
}

void TurboFileDialogs::getSaveAsPath(turbo::FileEditor &editor, TFuncView<bool (const char *)> accept) noexcept
{
    CwdGuard cwd {app.getFileDialogDir()};
    super::getSaveAsPath(editor, accept);
}

void TurboFileDialogs::getRenamePath(turbo::FileEditor &editor, TFuncView<bool (const char *)> accept) noexcept
{
    CwdGuard cwd {app.getFileDialogDir()};
    super::getRenamePath(editor, accept);
}
