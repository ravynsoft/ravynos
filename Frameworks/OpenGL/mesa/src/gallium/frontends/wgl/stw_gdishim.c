/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* Certain Win32-like platforms (i.e. Xbox GDK) do not support the GDI library.
 * stw_gdishim acts as a shim layer to provide the APIs required for gallium.
 */

#include "stw_gdishim.h"
#include "stw_pixelformat.h"
#include "stw_framebuffer.h"

int GetPixelFormat(HDC hdc)
{
   return stw_pixelformat_get(hdc);
}

int DescribePixelFormat(
   HDC                     hdc,
   int                     iPixelFormat,
   UINT                    nBytes,
   LPPIXELFORMATDESCRIPTOR ppfd
)
{
   if (iPixelFormat >= stw_pixelformat_get_count(hdc))
      return 0;

   const struct stw_pixelformat_info* info = stw_pixelformat_get_info(iPixelFormat);
   memcpy(ppfd, &info->pfd, nBytes);
   return 1;
}

BOOL SetPixelFormat(
   HDC                         hdc,
   int                         format,
   const PIXELFORMATDESCRIPTOR* ppfd
)
{
// TODO: can we support this?
#if 0
   struct stw_framebuffer* fb;

   fb = stw_framebuffer_from_hdc(hdc);
   if (fb && fb->pfi) {
      fb->pfi->iPixelFormat = format;
      stw_framebuffer_unlock(fb);
      return true;
   }
#endif
   return false;
}

void StretchDIBits(HDC hdc, unsigned int xDest, unsigned int yDest, unsigned int DestWidth, unsigned int DestHeight, unsigned int xSrc, unsigned int ySrc, unsigned int SrcWidth, unsigned int SrcHeight, void* lpBits, void* lpbmi, unsigned int iUsage, DWORD rop)
{

}
