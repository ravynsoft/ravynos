/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef _NINE_NINEEXOVERLAYEXTENSION_H_
#define _NINE_NINEEXOVERLAYEXTENSION_H_

#include "iunknown.h"

struct Nine9ExOverlayExtension
{
    struct NineUnknown base;
};
static inline struct Nine9ExOverlayExtension *
Nine9ExOverlayExtension( void *data )
{
    return (struct Nine9ExOverlayExtension *)data;
}

HRESULT NINE_WINAPI
Nine9ExOverlayExtension_CheckDeviceOverlayType( struct Nine9ExOverlayExtension *This,
                                                UINT Adapter,
                                                D3DDEVTYPE DevType,
                                                UINT OverlayWidth,
                                                UINT OverlayHeight,
                                                D3DFORMAT OverlayFormat,
                                                D3DDISPLAYMODEEX *pDisplayMode,
                                                D3DDISPLAYROTATION DisplayRotation,
                                                D3DOVERLAYCAPS *pOverlayCaps );

#endif /* _NINE_NINEEXOVERLAYEXTENSION_H_ */
