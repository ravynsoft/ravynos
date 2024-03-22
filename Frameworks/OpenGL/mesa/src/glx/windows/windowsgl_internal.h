/*
 * Copyright © 2014 Jon Turney
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef windowsgl_internal_h
#define windowsgl_internal_h

#include "windowsgl.h"

#include <X11/Xmd.h> // for BOOL
#include <X11/Xwindows.h> // as this doesn't provide one
#include <GL/gl.h>
#include <GL/wglext.h>

struct _windowsContext
{
   struct glx_config *config;
   windowsContext *shareContext;
   HGLRC ctx;
   int pxfi;
};

struct windowsdrawable_callbacks
{
   int type; // WINDOW, PIXMAP, PBUFFER
   HDC (*getdc) (windowsDrawable *d);
   void (*releasedc) (windowsDrawable *d, HDC dc);
};

struct _windowsDrawable
{
   int pxfi; // 0 if not yet set
   struct windowsdrawable_callbacks *callbacks;

   // for type WINDOW
   HWND hWnd;

   // for type PIXMAP
   HANDLE hSection;
   HDC dibDC;
   HBITMAP hDIB;
   HBITMAP hOldDIB;

   // for type PBUFFER
   HPBUFFERARB hPbuffer;
};

#endif /* windowsgl_internal_h */
