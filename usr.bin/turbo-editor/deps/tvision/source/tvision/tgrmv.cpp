/*------------------------------------------------------------*/
/* filename - tgrmv.cpp                                       */
/*                                                            */
/* function(s)                                                */
/*                     TGroup removeView member function      */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TGroup
#define Uses_TView
#include <tvision/tv.h>

void TGroup::removeView( TView *p ) noexcept
{
    if (last)
    {
        TView *s = last;
        while (s->next != p)
        {
            if (s->next == last)
                return;
            s = s->next;
        }
        s->next = p->next;
        if (p == last)
            last = (p == p->next) ? 0 : s;
    }
}

