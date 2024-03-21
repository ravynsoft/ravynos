#define Uses_TScrollBar
#define Uses_TFrame
#define Uses_MsgBox
#define Uses_TKeys
#define __INC_EDITORS_H
#include <tvision/tv.h>

#include <turbo/tpath.h>
#include <turbo/util.h>
#include "editwindow.h"
#include "app.h"
#include "apputils.h"
#include "search.h"
#include "gotoline.h"
#include <iostream>
#include <sstream>
using std::ios;

EditorWindow::EditorWindow( const TRect &bounds, TurboEditor &aEditor,
                            active_counter &fileCounter,
                            turbo::SearchSettings &searchSettings,
                            EditorWindowParent &aParent ) noexcept :
    TWindowInit(&initFrame),
    super(bounds, aEditor),
    listHead(this),
    fileNumber(fileCounter),
    parent(aParent),
    searchState {searchSettings}
{
    // Commands that always get enabled when focusing the editor.
    enabledCmds += cmSave;
    enabledCmds += cmSaveAs;
    enabledCmds += cmToggleWrap;
    enabledCmds += cmToggleLineNums;
    enabledCmds += cmFind;
    enabledCmds += cmReplace;
    enabledCmds += cmGoToLine;
    enabledCmds += cmSearchAgain;
    enabledCmds += cmSearchPrev;
    enabledCmds += cmToggleIndent;
    enabledCmds += cmCloseEditor;
    enabledCmds += cmSelUppercase;
    enabledCmds += cmSelLowercase;
    enabledCmds += cmSelCapitalize;
    enabledCmds += cmToggleComment;
    enabledCmds += cmReplaceOne;
    enabledCmds += cmReplaceAll;

    // Commands that always get disabled when unfocusing the editor.
    disabledCmds += enabledCmds;
    disabledCmds += cmRename;
}

void EditorWindow::shutDown()
{
    parent.removeEditor(*this);
    bottomView = nullptr;
    super::shutDown();
}

void EditorWindow::handleEvent(TEvent &ev)
{
    using namespace turbo;
    auto &editor = getEditor();
    TurboFileDialogs dlgs {parent};
    bool handled = true;
    switch (ev.what)
    {
        case evKeyDown:
            switch (ev.keyDown.keyCode)
            {
                case kbEsc:
                    if ((handled = bottomView))
                        closeBottomView();
                    break;
                default:
                    handled = false;
            }
            break;
        case evCommand:
            switch (ev.message.command)
            {
                case cmSave:
                    editor.save(dlgs);
                    break;
                case cmSaveAs:
                    editor.saveAs(dlgs);
                    break;
                case cmRename:
                    editor.rename(dlgs);
                    break;
                case cmToggleWrap:
                    editor.wrapping.toggle(editor.scintilla);
                    editor.redraw();
                    break;
                case cmToggleLineNums:
                    editor.lineNumbers.toggle();
                    editor.redraw();
                    break;
                case cmToggleIndent:
                    editor.autoIndent.toggle();
                    break;
                case cmCloseEditor:
                    ev.message.command = cmClose;
                    handled = false;
                    break;
                case cmSelUppercase:
                    editor.uppercase();
                    editor.partialRedraw();
                    break;
                case cmSelLowercase:
                    editor.lowercase();
                    editor.partialRedraw();
                    break;
                case cmSelCapitalize:
                    editor.capitalize();
                    editor.partialRedraw();
                    break;
                case cmToggleComment:
                    editor.toggleComment();
                    editor.partialRedraw();
                    break;
                case cmFind:
                    openBottomView<FindBox>(searchState);
                    break;
                case cmReplace:
                    openBottomView<ReplaceBox>(searchState);
                    break;
                case cmSearchAgain:
                    editor.search(searchState.findText, sdForward, searchState.settingsPreset.get());
                    editor.partialRedraw();
                    break;
                case cmSearchPrev:
                    editor.search(searchState.findText, sdBackwards, searchState.settingsPreset.get());
                    editor.partialRedraw();
                    break;
                case cmSearchIncr:
                    editor.search(searchState.findText, sdForwardIncremental, searchState.settingsPreset.get());
                    editor.partialRedraw();
                    break;
                case cmReplaceOne:
                    editor.replace(searchState.findText, searchState.replaceText, rmReplaceOne, searchState.settingsPreset.get());
                    editor.partialRedraw();
                    break;
                case cmReplaceAll:
                    editor.replace(searchState.findText, searchState.replaceText, rmReplaceAll, searchState.settingsPreset.get());
                    editor.partialRedraw();
                    break;
                case cmClearReplace:
                    editor.clearReplaceIndicator();
                    editor.partialRedraw();
                    break;
                case cmGoToLine:
                    openBottomView<GoToLineBox>(editor);
                    break;
                case cmCloseView:
                    if ((handled = bottomView && ev.message.infoPtr == bottomView))
                        closeBottomView();
                    break;
                default:
                    handled = false;
            }
            break;
        default:
            handled = false;
    }
    if (handled)
        clearEvent(ev);
    else
        super::handleEvent(ev);
}

void EditorWindow::setState(ushort aState, Boolean enable)
{
    super::setState(aState, enable);
    if (aState == sfActive)
    {
        updateCommands();
        if (enable)
            parent.handleFocus(*this);
    }
}

Boolean EditorWindow::valid(ushort command)
{
    auto &editor = getEditor();
    if (super::valid(command))
    {
        if (command != cmValid)
        {
            TurboFileDialogs dlgs {parent};
            return editor.close(dlgs);
        }
        return true;
    }
    return false;
}

const char* EditorWindow::getTitle(short)
{
    return formatTitle();
}

void EditorWindow::updateCommands() noexcept
{
    if (!filePath().empty())
        enabledCmds += cmRename;
    if (state & sfActive)
        enableCommands(enabledCmds);
    else
        disableCommands(disabledCmds);
}

void EditorWindow::handleNotification(const SCNotification &scn, turbo::Editor &editor)
{
    using namespace turbo;
    super::handleNotification(scn, editor);
    switch (scn.nmhdr.code)
    {
        case SCN_SAVEPOINTREACHED:
            updateCommands();
            parent.handleTitleChange(*this);
            editor.redraw();
            break;
    }
}

static void growEditor(turbo::Editor &editor, int dy)
{
    turbo::forEachNotNull([&] (TView &v) {
        TRect r = v.getBounds();
        r.b.y += dy;
        v.setBounds(r);
    }, editor.view, editor.leftMargin);
}

void EditorWindow::closeBottomView()
// Pre: 'bottomView' is not null.
{
    int dy = bottomView->size.y + !!(bottomView->options & ofFramed);
    growEditor(editor, dy);
    destroy(bottomView);
    bottomView = nullptr;
    editor.redraw();
}

void EditorWindow::setBottomView(TView *view)
{
    if (bottomView)
        closeBottomView();
    insert(view);
    bottomView = view;
    int dy = -(bottomView->size.y + !!(bottomView->options & ofFramed));
    growEditor(editor, dy);
}

template <class T, class ...Args>
void EditorWindow::openBottomView(Args&& ...args)
{
    auto *view = (T *) message(bottomView, evCommand, T::findCommand, nullptr);
    if (!view)
    {
        TRect r = getExtent().grow(-1, -1);
        r.a.y = r.b.y - T::height;
        view = new T(r, static_cast<Args &&>(args)...);
        view->growMode = gfGrowAll & ~gfGrowLoX;
        view->setState(sfActive, state & sfActive);
        setBottomView(view);
        editor.redraw();
    }
    view->select();
    view->resetCurrent();
}

const char* EditorWindow::formatTitle(ushort flags) noexcept
{
    bool inSavePoint = (flags & tfNoSavePoint) || editor.inSavePoint();
    TitleState titleState {fileNumber.counter, fileNumber.number, inSavePoint};
    if (lastTitleState != titleState)
    {
        lastTitleState = titleState;
        TStringView name = filePath().empty() ? "Untitled" : TPath::basename(filePath());
        std::ostringstream os;
        os << name;
        if (fileNumber.number > 1)
            os << " (" << fileNumber.number << ')';
        if (!inSavePoint)
            os << '*';
        title = os.str();
    }
    return title.c_str();
}

void EditorWindow::sizeLimits(TPoint &min, TPoint &max)
{
    super::sizeLimits(min, max);
    if (bottomView)
        min.y = ::max(min.y, bottomView->size.y + !!(bottomView->options & ofFramed) + 3);
}
