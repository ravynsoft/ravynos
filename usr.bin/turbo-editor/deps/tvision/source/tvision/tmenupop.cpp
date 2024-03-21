/*------------------------------------------------------------*/
/* filename -       tmenupop.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TMenuPopup member functions               */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TEvent
#define Uses_TRect
#define Uses_TMenu
#define Uses_TMenuItem
#define Uses_TMenuPopup
#define Uses_TProgram
#define Uses_TDeskTop
#include <tvision/tv.h>

TMenuPopup::TMenuPopup(const TRect& bounds, TMenu* aMenu, TMenuView *aParentMenu) noexcept :
    TMenuBox( bounds, aMenu, aParentMenu )
{
    putClickEventOnExit = False;
}

ushort TMenuPopup::execute()
{
    // Do not highlight the default entry, because it would look ugly.
    menu->deflt = 0;
    return TMenuBox::execute();
}

void TMenuPopup::handleEvent(TEvent& event)
{
    switch (event.what)
    {
    case evKeyDown:
        TMenuItem* p = findItem(getCtrlChar(event.keyDown.keyCode));
        if (!p)
            p = hotKey(event.keyDown.keyCode);
        if (p && commandEnabled(p->command))
        {
            event.what = evCommand;
            event.message.command = p->command;
            event.message.infoPtr = NULL;
            putEvent(event);
            clearEvent(event);
        }
        else
            if (getAltChar(event.keyDown.keyCode))
                clearEvent(event);
        break;
    }
    TMenuBox::handleEvent(event);
}

#if !defined(NO_STREAMABLE)

TStreamable *TMenuPopup::build()
{
    return new TMenuPopup( streamableInit );
}

TMenuPopup::TMenuPopup( StreamableInit ) noexcept : TMenuBox( streamableInit )
{
}

#endif
