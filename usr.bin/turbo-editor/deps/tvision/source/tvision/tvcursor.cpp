/*------------------------------------------------------------*/
/* filename -       tvcursor.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TView resetCursor member function         */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TView
#define Uses_TGroup
#define Uses_TScreen
#define Uses_THardwareInfo
#include <tvision/tv.h>

#ifdef __FLAT__

struct TVCursor {

    TView *self;
    int x, y;

    void resetCursor(TView *);
    int computeCaretSize();
    Boolean caretCovered(TView *) const;
    int decideCaretSize() const;

};

void TView::resetCursor()
{
    TVCursor().resetCursor(this);
}

void TVCursor::resetCursor(TView *p)
{
    self = p;
    x = self->cursor.x;
    y = self->cursor.y;
    int caretSize = computeCaretSize();
    if (caretSize)
        THardwareInfo::setCaretPosition(x, y);
    THardwareInfo::setCaretSize(caretSize);
}

int TVCursor::computeCaretSize()
{
    if (!(~self->state & (sfVisible | sfCursorVis | sfFocused)))
    {
        TView *v = self;
        while (0 <= y && y < v->size.y && 0 <= x && x < v->size.x)
        {
            y += v->origin.y;
            x += v->origin.x;
            if (v->owner)
            {
                if (v->owner->state & sfVisible)
                {
                    if (caretCovered(v))
                        break;
                    v = v->owner;
                }
                else break;
            }
            else return decideCaretSize();
        }
    }
    return 0;
}

Boolean TVCursor::caretCovered(TView *v) const
{
    TView *u = v->owner->last->next;
    for (; u != v; u = u->next)
    {
        if ( (u->state & sfVisible)
             && (u->origin.y <= y && y < u->origin.y + u->size.y)
             && (u->origin.x <= x && x < u->origin.x + u->size.x) )
            return True;
    }
    return False;
}

int TVCursor::decideCaretSize() const
{
    if (self->state & sfCursorIns)
        return 100;
    return TScreen::cursorLines & 0x0F;
}

#endif
