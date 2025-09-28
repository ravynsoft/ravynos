#define Uses_TLabel
#define Uses_TButton
#define Uses_TEvent
#define Uses_TKeys
#include <tvision/tv.h>

#include "gotoline.h"

#include <turbo/editor.h>

GoToLineBox::GoToLineBox(const TRect &bounds, turbo::Editor &aEditor) noexcept :
    TGroup(bounds),
    editor(aEditor)
{
    options |= ofFramed | ofFirstClick;

    auto *bckgrnd = new TView({0, 0, size.x, height});
    bckgrnd->growMode = gfGrowHiX | gfGrowHiY;
    bckgrnd->eventMask = 0;
    insert(bckgrnd);

    auto *lineText = "~L~ine:";
    auto *goToText = "~G~o to";
    TRect rLabelL {0, 0, 1 + cstrlen(lineText), 1};
    TRect rBoxL {rLabelL.b.x + 1, 0, rLabelL.b.x + 1 + 14, 1};
    TRect rGoTo {rBoxL.b.x, 0, rBoxL.b.x + cstrlen(goToText) + 4, 2};

    ilLine = new GoToLineInputLine(rBoxL, *new GoToLineValidator(editor));
    ilLine->growMode = 0;
    sptr_t pos = editor.callScintilla(SCI_GETCURRENTPOS, 0U, 0U);
    sptr_t line = editor.callScintilla(SCI_LINEFROMPOSITION, pos, 0U);
    sprintf(ilLine->data, "%lu", ulong(line + 1));
    ilLine->selectAll(true);

    auto *lblLine = new TLabel(rLabelL, lineText, ilLine);
    lblLine->growMode = 0;
    insert(lblLine);

    auto *btnGoTo = new TButton(rGoTo, goToText, cmOK, bfBroadcast);
    btnGoTo->growMode = 0;
    insert(btnGoTo);

    // This must be the first selectable view.
    insert(ilLine);
}

void GoToLineBox::shutDown()
{
    ilLine = nullptr;
    TGroup::shutDown();
}

void GoToLineBox::handleEvent(TEvent &ev)
{
    TGroup::handleEvent(ev);
    if (ev.what == evKeyDown)
    {
        switch (ev.keyDown.keyCode)
        {
            case kbTab:
                focusNext(False);
                clearEvent(ev);
                break;
            case kbShiftTab:
                focusNext(True);
                clearEvent(ev);
                break;
        }
    }
    else if (ev.what == evCommand && ev.message.command == cmFindGoToLineBox)
        clearEvent(ev);
    else if (ev.what == evBroadcast && ev.message.command == cmOK)
    {
        if (ilLine)
            goToLine(ilLine->data);
    }
}

void GoToLineBox::goToLine(const char *input)
{
    ulong line;
    if (sscanf(input, "%lu", &line) == 1)
        editor.callScintilla(SCI_GOTOLINE, line - 1, 0U);
    TEvent ev;
    ev.what = evCommand;
    ev.message.command = cmCloseView;
    ev.message.infoPtr = this;
    putEvent(ev);
}

Boolean GoToLineValidator::isValid(const char *s)
{
    return true;
}

Boolean GoToLineValidator::isValidInput(char *s, Boolean)
{
    TRangeValidator::max = editor.callScintilla(SCI_GETLINECOUNT, 0U, 0U);
    return s[0] == '\0' || TRangeValidator::isValid(s);
}

void GoToLineInputLine::handleEvent(TEvent &ev)
{
    if (ev.what == evKeyDown && ev.keyDown.keyCode == kbEnter)
    {
        message(owner, evBroadcast, cmOK, nullptr);
        clearEvent(ev);
    }
    else
        TInputLine::handleEvent(ev);
}
