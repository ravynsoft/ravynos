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

#include "windowsgl.h"
#include "windowsgl_internal.h"
#include "windowsdriconst.h"
#include "wgl.h"

#include <stdio.h>

/*
 * Window drawable
 */

static
HDC window_getdc(windowsDrawable *d)
{
   return GetDC(d->hWnd);
}

static
void window_releasedc(windowsDrawable *d, HDC dc)
{
   ReleaseDC(d->hWnd, dc);
}

static struct windowsdrawable_callbacks window_callbacks = {
   .type = WindowsDRIDrawableWindow,
   .getdc = window_getdc,
   .releasedc = window_releasedc
};

/*
 * Pixmap drawable
 */

static
HDC pixmap_getdc(windowsDrawable *d)
{
   return d->dibDC;
}

static
void pixmap_releasedc(windowsDrawable *d, HDC dc)
{
   GdiFlush();
}

static struct windowsdrawable_callbacks pixmap_callbacks = {
   .type = WindowsDRIDrawablePixmap,
   .getdc = pixmap_getdc,
   .releasedc = pixmap_releasedc
};

/*
 * Pbuffer drawable
 */

static
HDC pbuffer_getdc(windowsDrawable *d)
{
   return wglGetPbufferDCARB(d->hPbuffer);
}

static
void pbuffer_releasedc(windowsDrawable *d, HDC dc)
{
   wglReleasePbufferDCARB(d->hPbuffer, dc);
}

static struct windowsdrawable_callbacks pbuffer_callbacks = {
   .type = WindowsDRIDrawablePbuffer,
   .getdc = pbuffer_getdc,
   .releasedc = pbuffer_releasedc
};

/*
 *
 */

windowsDrawable *
windows_create_drawable(int type, void *handle)
{
   windowsDrawable *d;

   d = calloc(1, sizeof *d);
   if (d == NULL)
      return NULL;

   switch (type)
   {
   case WindowsDRIDrawableWindow:
      d->hWnd = handle;
      d->callbacks = &window_callbacks;
      break;

   case WindowsDRIDrawablePixmap:
   {
      BITMAPINFOHEADER *pBmpHeader;
      void *pBits;

      char name[MAX_PATH];

      d->callbacks = &pixmap_callbacks;

      // Access file mapping object by a name
      snprintf(name, sizeof(name), "Local\\CYGWINX_WINDOWSDRI_%08x", (unsigned int)(uintptr_t)handle);
      d->hSection = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, name);
      if (!d->hSection)
         printf("OpenFileMapping failed %x\n", (int)GetLastError());

      // Create a screen-compatible DC
      d->dibDC = CreateCompatibleDC(NULL);

      // Map the shared memory section to access the BITMAPINFOHEADER
      pBmpHeader = (BITMAPINFOHEADER *)MapViewOfFile(d->hSection, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(BITMAPINFOHEADER));
      if (!pBmpHeader)
         printf("MapViewOfFile failed %x\n", (int)GetLastError());

      // Create a DIB using the file mapping
      d->hDIB = CreateDIBSection(d->dibDC, (BITMAPINFO *) pBmpHeader,
                                 DIB_RGB_COLORS, &pBits, d->hSection,
                                 sizeof(BITMAPINFOHEADER));

      // Done with the BITMAPINFOHEADER
      UnmapViewOfFile(pBmpHeader);

      // Select the DIB into the DC
      d->hOldDIB = SelectObject(d->dibDC, d->hDIB);
   }
   break;

   case WindowsDRIDrawablePbuffer:
      d->hPbuffer = handle;
      d->callbacks = &pbuffer_callbacks;
      break;
   }

   return d;
}

void
windows_destroy_drawable(windowsDrawable *drawable)
{
   switch (drawable->callbacks->type)
   {
   case WindowsDRIDrawableWindow:
      break;

   case WindowsDRIDrawablePixmap:
   {
      // Select the default DIB into the DC
      SelectObject(drawable->dibDC, drawable->hOldDIB);

      // delete the screen-compatible DC
      DeleteDC(drawable->dibDC);

      // Delete the DIB
      DeleteObject(drawable->hDIB);

      // Close the file mapping object
      CloseHandle(drawable->hSection);
   }
   break;

   case WindowsDRIDrawablePbuffer:

      break;
   }

   free(drawable);
}
