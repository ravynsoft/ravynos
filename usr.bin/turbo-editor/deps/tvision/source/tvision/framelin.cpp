/*------------------------------------------------------------*/
/* filename -  framelin.cpp                                   */
/*                                                            */
/* function(s)                                                */
/*             TFrame frameLine member function               */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TFrame
#define Uses_TGroup
#include <tvision/tv.h>

#include <stdlib.h>
#include <malloc.h>

void TFrame::frameLine( TDrawBuffer& frameBuf, short y, short n, TColorAttr color )
{
    uchar *FrameMask = (uchar*) alloca(size.x);
    int x;

    FrameMask[0] = initFrame[n];
    for (x = 1; x < size.x - 1; ++x)
        FrameMask[x] = initFrame[n + 1];
    FrameMask[size.x - 1] = initFrame[n + 2];

    TView* v = owner->last->next;
    for(; v != (TView *) this; v = v->next)
    {
        if ((v->options & ofFramed) && (v->state & sfVisible))
        {
            ushort mask = 0;
            if (y < v->origin.y)
            {
                if (y == v->origin.y - 1)
                    mask = 0x0A06;
            }
            else if (y < v->origin.y + v->size.y)
                mask = 0x0005;
            else if (y == v->origin.y + v->size.y)
                mask = 0x0A03;

            if (mask)
            {
                int start = max(v->origin.x, 1);
                int end = min(v->origin.x + v->size.x, size.x - 1);
                if (start < end)
                {
                    uchar maskLow = mask & 0x00FF;
                    uchar maskHigh = (mask & 0xFF00) >> 8;
                    FrameMask[start - 1] |= maskLow;
                    FrameMask[end] |= maskLow ^ maskHigh;
                    if (maskLow)
                        for (x = start; x < end; ++x)
                            FrameMask[x] |= maskHigh;
                }
            }
        }
    }

    for (x = 0; x < size.x; ++x)
    {
        frameBuf.putChar(x, frameChars[FrameMask[x]]);
        frameBuf.putAttribute(x, color);
    }
}
