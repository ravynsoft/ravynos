/*
 Copyright (c) 2009 Apple Inc.
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation files
 (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
 HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 
 Except as contained in this notice, the name(s) of the above
 copyright holders shall not be used in advertising or otherwise to
 promote the sale, use or other dealings in this Software without
 prior written authorization.
*/
#include <stdbool.h>
#include <assert.h>
#include <X11/Xlibint.h>
#include <X11/extensions/extutil.h>
#include <X11/extensions/Xext.h>
#include "glxclient.h"
#include "glx_error.h"

void
__glXSendError(Display * dpy, int_fast8_t errorCode, uint_fast32_t resourceID,
               uint_fast16_t minorCode, bool coreX11error)
{
   struct glx_display *glx_dpy = __glXInitialize(dpy);
   xError error;

   assert(glx_dpy);

   LockDisplay(dpy);

   error.type = X_Error;

   if (coreX11error) {
      error.errorCode = errorCode;
   }
   else {
      error.errorCode = glx_dpy->codes.first_error + errorCode;
   }

   error.sequenceNumber = dpy->request;
   error.resourceID = resourceID;
   error.minorCode = minorCode;
   error.majorCode = glx_dpy->codes.major_opcode;

   _XError(dpy, &error);

   UnlockDisplay(dpy);
}

void
__glXSendErrorForXcb(Display * dpy, const xcb_generic_error_t *err)
{
   xError error;

   LockDisplay(dpy);

   error.type = X_Error;
   error.errorCode = err->error_code;
   error.sequenceNumber = err->sequence;
   error.resourceID = err->resource_id;
   error.minorCode = err->minor_code;
   error.majorCode = err->major_code;

   _XError(dpy, &error);

   UnlockDisplay(dpy);
}
