#define Uses_TKeys
#define Uses_TText
#define Uses_TEvent
#include <tvision/tv.h>

#include <turbo/editor.h>
#include "icmds.h"

namespace turbo {

EditorView::EditorView(const TRect &bounds) noexcept :
    TSurfaceView(bounds)
{
    growMode = gfGrowHiX | gfGrowHiY;
    options |= ofSelectable | ofFirstClick;
    eventMask |= evMouseUp | evMouseMove | evMouseAuto | evBroadcast;
}

static TPoint getDelta(TScintilla &scintilla)
{
    return {
        (int) call(scintilla, SCI_GETXOFFSET, 0U, 0U),
        (int) call(scintilla, SCI_GETFIRSTVISIBLELINE, 0U, 0U),
    };
}

void EditorView::handleEvent(TEvent &ev)
{
    if (!editor)
        return;
    TView::handleEvent(ev);
    auto &scintilla = editor->scintilla;
    switch (ev.what)
    {
        case evKeyDown:
            if (ev.keyDown.keyCode == kbIns)
                setState(sfCursorIns, !getState(sfCursorIns));
            if (ev.keyDown.controlKeyState & kbPaste)
                handlePaste(ev);
            else
                handleKeyDown(scintilla, ev.keyDown);
            editor->redraw(); // partialRedraw() is broken for this action.
            clearEvent(ev);
            break;
        case evMouseDown:
            // Middle button drag
            if (ev.mouse.buttons & mbMiddleButton)
            {
                TPoint lastMouse = makeLocal(ev.mouse.where);
                while (mouseEvent(ev, evMouse))
                {
                    TPoint mouse = makeLocal(ev.mouse.where);
                    TPoint d = getDelta(scintilla) + (lastMouse - mouse);
                    editor->scrollTo(d);
                    editor->partialRedraw();
                    lastMouse = mouse;
                }
            }
            else
            {
                // Text selection
                do {
                    TPoint where = makeLocal(ev.mouse.where) + delta;
                    if (ev.what == evMouseWheel)
                    {
                        // Mouse wheel while holding button down.
                        editor->scrollBarEvent(ev);
                        ev.mouse.where = where;
                        ev.what = evMouseMove;
                        // For some reason, the caret is not always updated
                        // unless this is invoked twice.
                        handleMouse(scintilla, ev.what, ev.mouse);
                        handleMouse(scintilla, ev.what, ev.mouse);
                    }
                    else
                    {
                        ev.mouse.where = where;
                        if (!handleMouse(scintilla, ev.what, ev.mouse))
                        {
                            editor->partialRedraw();
                            break;
                        }
                    }
                    editor->partialRedraw();
                } while (mouseEvent(ev, evMouseDown | evMouseMove | evMouseAuto | evMouseWheel));
            }
            clearEvent(ev);
            break;
        case evCommand:
            switch (ev.message.command)
            {
                case cmUndo:
                    call(scintilla, SCI_UNDO, 0U, 0U);
                    editor->redraw(); // partialRedraw() is broken for this action.
                    clearEvent(ev);
                    break;
                case cmRedo:
                    call(scintilla, SCI_REDO, 0U, 0U);
                    editor->redraw(); // partialRedraw() is broken for this action.
                    clearEvent(ev);
                    break;
                case cmCut:
                    call(scintilla, SCI_CUT, 0U, 0U);
                    editor->partialRedraw();
                    clearEvent(ev);
                    break;
                case cmCopy:
                    call(scintilla, SCI_COPY, 0U, 0U);
                    clearEvent(ev);
                    break;
                case cmPaste:
                    call(scintilla, SCI_PASTE, 0U, 0U);
                    clearEvent(ev);
                    break;
            }
            break;
        case evBroadcast:
            if ( ev.message.command == cmScrollBarChanged &&
                 editor->handleScrollBarChanged((TScrollBar *) ev.message.infoPtr) )
            {
                editor->partialRedraw();
                clearEvent(ev);
            }
            else if (isInternalMessage(ev.message, InternalCommands::cmGetEditor))
            {
                clearEvent(ev);
                ev.message.infoPtr = editor;
            }
            break;
    }
    if (ev.what == evNothing && canUpdateCommands())
        updateCommands();
}

void EditorView::handlePaste(TEvent &ev)
// Pre: 'editor' is non-null.
{
    auto &scintilla = editor->scintilla;
    clearBeforeTentativeStart(scintilla);
    call(scintilla, SCI_BEGINUNDOACTION, 0U, 0U);

    char buf[4096];
    size_t length;
    while (textEvent(ev, buf, length))
    {
        TStringView text(buf, length);
        insertPasteStream(scintilla, text);
    }

    call(scintilla, SCI_SCROLLCARET, 0U, 0U);
    call(scintilla, SCI_ENDUNDOACTION, 0U, 0U);
}

void EditorView::draw()
{
    if (!editor)
        TSurfaceView::draw();
    // 'surface' is only set when the draw is triggered by EditorState.
    else if (!surface)
        editor->redraw();
    else
    {
        TPoint p = pointMainCaret(editor->scintilla);
        cursor = p - delta;
        TSurfaceView::draw();
    }
}

bool EditorView::canUpdateCommands()
{
    return (~state & (sfActive | sfSelected)) == 0;
}

void EditorView::setState(ushort aState, bool enable)
{
    bool updateBefore = canUpdateCommands();
    TSurfaceView::setState(aState, enable);
    bool updateAfter = canUpdateCommands();
    if (updateBefore != updateAfter)
        updateCommands();
}

void EditorView::setCmdState(ushort command, bool enable)
{
    TCommandSet s;
    s += command;
    if (enable && canUpdateCommands())
        enableCommands(s);
    else
        disableCommands(s);
}

void EditorView::updateCommands()
{
    bool canUndo = editor && editor->callScintilla(SCI_CANUNDO, 0U, 0U);
    bool canRedo = editor && editor->callScintilla(SCI_CANREDO, 0U, 0U);
    bool hasSelection = editor && (
        editor->callScintilla(SCI_GETCURRENTPOS, 0U, 0U) !=
        editor->callScintilla(SCI_GETANCHOR, 0U, 0U)
    );
    setCmdState(cmUndo, canUndo);
    setCmdState(cmRedo, canRedo);
    setCmdState(cmCut, hasSelection);
    setCmdState(cmCopy, hasSelection);
    setCmdState(cmPaste, editor != nullptr);
}

} // namespace turbo
