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

/* THIS IS NOT AN X CONSORTIUM STANDARD */

#include <X11/Xlibint.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include "windowsdristr.h"
#include "xwindowsdri.h"
#include <stdio.h>

static XExtensionInfo _windowsdri_info_data;
static XExtensionInfo *windowsdri_info = &_windowsdri_info_data;
static char *windowsdri_extension_name = WINDOWSDRINAME;

#define WindowsDRICheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, windowsdri_extension_name, val)

/*****************************************************************************
 *                                                                           *
 *                         private utility routines                          *
 *                                                                           *
 *****************************************************************************/

static int close_display(Display * dpy, XExtCodes * extCodes);

static /* const */ XExtensionHooks windowsdri_extension_hooks = {
   NULL,                        /* create_gc */
   NULL,                        /* copy_gc */
   NULL,                        /* flush_gc */
   NULL,                        /* free_gc */
   NULL,                        /* create_font */
   NULL,                        /* free_font */
   close_display,               /* close_display */
   NULL,                        /* wire_to_event */
   NULL,                        /* event_to_wire */
   NULL,                        /* error */
   NULL,                        /* error_string */
};

static
XEXT_GENERATE_FIND_DISPLAY(find_display, windowsdri_info,
                           windowsdri_extension_name,
                           &windowsdri_extension_hooks,
                           WindowsDRINumberEvents, NULL)

static
XEXT_GENERATE_CLOSE_DISPLAY(close_display, windowsdri_info)

/*****************************************************************************
 *                                                                           *
 *                 public Windows-DRI Extension routines                     *
 *                                                                           *
 *****************************************************************************/

#if 0
#include <stdio.h>
#define TRACE(msg, ...)  fprintf(stderr, "WindowsDRI" msg "\n", ##__VA_ARGS__);
#else
#define TRACE(msg, ...)
#endif

Bool
XWindowsDRIQueryExtension(dpy, event_basep, error_basep)
     Display *dpy;
     int *event_basep, *error_basep;
{
   XExtDisplayInfo *info = find_display(dpy);

   TRACE("QueryExtension:");
   if (XextHasExtension(info)) {
      *event_basep = info->codes->first_event;
      *error_basep = info->codes->first_error;
      TRACE("QueryExtension: return True");
      return True;
   }
   else {
      TRACE("QueryExtension: return False");
      return False;
   }
}

Bool
XWindowsDRIQueryVersion(dpy, majorVersion, minorVersion, patchVersion)
     Display *dpy;
     int *majorVersion;
     int *minorVersion;
     int *patchVersion;
{
   XExtDisplayInfo *info = find_display(dpy);
   xWindowsDRIQueryVersionReply rep;
   xWindowsDRIQueryVersionReq *req;

   TRACE("QueryVersion:");
   WindowsDRICheckExtension(dpy, info, False);

   LockDisplay(dpy);
   GetReq(WindowsDRIQueryVersion, req);
   req->reqType = info->codes->major_opcode;
   req->driReqType = X_WindowsDRIQueryVersion;
   if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
      UnlockDisplay(dpy);
      SyncHandle();
      TRACE("QueryVersion: return False");
      return False;
   }
   *majorVersion = rep.majorVersion;
   *minorVersion = rep.minorVersion;
   *patchVersion = rep.patchVersion;
   UnlockDisplay(dpy);
   SyncHandle();
   TRACE("QueryVersion: %d.%d.%d", *majorVersion, *minorVersion, *patchVersion);
   return True;
}

Bool
XWindowsDRIQueryDirectRenderingCapable(dpy, screen, isCapable)
     Display *dpy;
     int screen;
     Bool *isCapable;
{
   XExtDisplayInfo *info = find_display(dpy);
   xWindowsDRIQueryDirectRenderingCapableReply rep;
   xWindowsDRIQueryDirectRenderingCapableReq *req;

   TRACE("QueryDirectRenderingCapable:");
   WindowsDRICheckExtension(dpy, info, False);

   LockDisplay(dpy);
   GetReq(WindowsDRIQueryDirectRenderingCapable, req);
   req->reqType = info->codes->major_opcode;
   req->driReqType = X_WindowsDRIQueryDirectRenderingCapable;
   req->screen = screen;
   if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
      UnlockDisplay(dpy);
      SyncHandle();
      TRACE("QueryDirectRenderingCapable: return False");
      return False;
   }
   *isCapable = rep.isCapable;
   UnlockDisplay(dpy);
   SyncHandle();
   TRACE("QueryDirectRenderingCapable:return True");
   return True;
}

Bool
XWindowsDRIQueryDrawable(Display *dpy, int screen, Drawable drawable,
                        unsigned int * type, void ** handle)
{
   XExtDisplayInfo *info = find_display(dpy);
   xWindowsDRIQueryDrawableReply rep;
   xWindowsDRIQueryDrawableReq *req;

   TRACE("QueryDrawable: XID %lx", drawable);
   WindowsDRICheckExtension(dpy, info, False);

   LockDisplay(dpy);
   GetReq(WindowsDRIQueryDrawable, req);
   req->reqType = info->codes->major_opcode;
   req->driReqType = X_WindowsDRIQueryDrawable;
   req->screen = screen;
   req->drawable = drawable;

   if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
      UnlockDisplay(dpy);
      SyncHandle();
      TRACE("QueryDrawable: return False");
      return False;
   }

   *type = rep.drawable_type;

   // Note that despite being a derived type of void *, HANDLEs are defined to
   // be a sign-extended 32 bit value (so they can be passed to 32-bit
   // processes safely)
   *handle = (void *)(intptr_t)rep.handle;

   UnlockDisplay(dpy);
   SyncHandle();
   TRACE("QueryDrawable: type %d, handle %p", *type, *handle);
   return True;
}

Bool
XWindowsDRIFBConfigToPixelFormat(Display *dpy, int screen, int fbConfigID,
                                int *pxfi)
{
   XExtDisplayInfo *info = find_display(dpy);
   xWindowsDRIFBConfigToPixelFormatReply rep;
   xWindowsDRIFBConfigToPixelFormatReq *req;

   TRACE("FBConfigToPixelFormat: fbConfigID 0x%x", fbConfigID);
   WindowsDRICheckExtension(dpy, info, False);

   LockDisplay(dpy);
   GetReq(WindowsDRIFBConfigToPixelFormat, req);
   req->reqType = info->codes->major_opcode;
   req->driReqType = X_WindowsDRIFBConfigToPixelFormat;
   req->screen = screen;
   req->fbConfigID = fbConfigID;

   if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
      UnlockDisplay(dpy);
      SyncHandle();
      TRACE("FBConfigToPixelFormat: return False");
      return False;
   }

   *pxfi = rep.pixelFormatIndex;

   UnlockDisplay(dpy);
   SyncHandle();
   TRACE("FBConfigToPixelFormat: pixelformatindex %d", *pxfi);
   return True;
}
