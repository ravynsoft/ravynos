/*
 * Copyright © 2008 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Soft-
 * ware"), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, provided that the above copyright
 * notice(s) and this permission notice appear in all copies of the Soft-
 * ware and that both the above copyright notice(s) and this permission
 * notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABIL-
 * ITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY
 * RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN
 * THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSE-
 * QUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFOR-
 * MANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization of
 * the copyright holder.
 *
 * Authors:
 *   Kristian Høgsberg (krh@redhat.com)
 */


#ifdef GLX_DIRECT_RENDERING

#include <stdio.h>
#include <X11/Xlibint.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <X11/extensions/dri2proto.h>
#include "dri2.h"
#include "glxclient.h"
#include "GL/glxext.h"

/* Allow the build to work with an older versions of dri2proto.h and
 * dri2tokens.h.
 */
#if DRI2_MINOR < 1
#undef DRI2_MINOR
#define DRI2_MINOR 1
#define X_DRI2GetBuffersWithFormat 7
#endif


static char dri2ExtensionName[] = DRI2_NAME;
static XExtensionInfo _dri2Info_data;
static XExtensionInfo *dri2Info = &_dri2Info_data;
static XEXT_GENERATE_CLOSE_DISPLAY (DRI2CloseDisplay, dri2Info)

static Bool
DRI2WireToEvent(Display *dpy, XEvent *event, xEvent *wire);
static Status
DRI2EventToWire(Display *dpy, XEvent *event, xEvent *wire);
static int
DRI2Error(Display *display, xError *err, XExtCodes *codes, int *ret_code);

static /* const */ XExtensionHooks dri2ExtensionHooks = {
  NULL,                   /* create_gc */
  NULL,                   /* copy_gc */
  NULL,                   /* flush_gc */
  NULL,                   /* free_gc */
  NULL,                   /* create_font */
  NULL,                   /* free_font */
  DRI2CloseDisplay,       /* close_display */
  DRI2WireToEvent,        /* wire_to_event */
  DRI2EventToWire,        /* event_to_wire */
  DRI2Error,              /* error */
  NULL,                   /* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (DRI2FindDisplay,
                                   dri2Info,
                                   dri2ExtensionName,
                                   &dri2ExtensionHooks,
                                   0, NULL)

static Bool
DRI2WireToEvent(Display *dpy, XEvent *event, xEvent *wire)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);
   struct glx_drawable *glxDraw;

   XextCheckExtension(dpy, info, dri2ExtensionName, False);

   switch ((wire->u.u.type & 0x7f) - info->codes->first_event) {

   case DRI2_BufferSwapComplete:
   {
      GLXBufferSwapComplete *aevent = (GLXBufferSwapComplete *)event;
      xDRI2BufferSwapComplete2 *awire = (xDRI2BufferSwapComplete2 *)wire;
      __GLXDRIdrawable *pdraw;

      pdraw = dri2GetGlxDrawableFromXDrawableId(dpy, awire->drawable);
      if (pdraw == NULL)
         return False;

      /* Ignore swap events if we're not looking for them */
      aevent->type = dri2GetSwapEventType(dpy, awire->drawable);
      if(!aevent->type)
         return False;

      aevent->serial = _XSetLastRequestRead(dpy, (xGenericReply *) wire);
      aevent->send_event = (awire->type & 0x80) != 0;
      aevent->display = dpy;
      aevent->drawable = awire->drawable;
      switch (awire->event_type) {
      case DRI2_EXCHANGE_COMPLETE:
	 aevent->event_type = GLX_EXCHANGE_COMPLETE_INTEL;
	 break;
      case DRI2_BLIT_COMPLETE:
	 aevent->event_type = GLX_COPY_COMPLETE_INTEL;
	 break;
      case DRI2_FLIP_COMPLETE:
	 aevent->event_type = GLX_FLIP_COMPLETE_INTEL;
	 break;
      default:
	 /* unknown swap completion type */
	 return False;
      }
      aevent->ust = ((CARD64)awire->ust_hi << 32) | awire->ust_lo;
      aevent->msc = ((CARD64)awire->msc_hi << 32) | awire->msc_lo;

      glxDraw = GetGLXDrawable(dpy, pdraw->drawable);
      if (glxDraw != NULL) {
         if (awire->sbc < glxDraw->lastEventSbc)
            glxDraw->eventSbcWrap += 0x100000000;
         glxDraw->lastEventSbc = awire->sbc;
         aevent->sbc = awire->sbc + glxDraw->eventSbcWrap;
      } else {
         aevent->sbc = awire->sbc;
      }

      return True;
   }
   case DRI2_InvalidateBuffers:
   {
      xDRI2InvalidateBuffers *awire = (xDRI2InvalidateBuffers *)wire;

      dri2InvalidateBuffers(dpy, awire->drawable);
      return False;
   }
   default:
      /* client doesn't support server event */
      break;
   }

   return False;
}

/* We don't actually support this.  It doesn't make sense for clients to
 * send each other DRI2 events.
 */
static Status
DRI2EventToWire(Display *dpy, XEvent *event, xEvent *wire)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);

   XextCheckExtension(dpy, info, dri2ExtensionName, False);

   switch (event->type) {
   default:
      /* client doesn't support server event */
      break;
   }

   return Success;
}

static int
DRI2Error(Display *display, xError *err, XExtCodes *codes, int *ret_code)
{
    if (err->majorCode == codes->major_opcode &&
	err->errorCode == BadDrawable &&
	err->minorCode == X_DRI2CopyRegion)
	return True;

    /* If the X drawable was destroyed before the GLX drawable, the
     * DRI2 drawble will be gone by the time we call
     * DRI2DestroyDrawable.  So just ignore BadDrawable here. */
    if (err->majorCode == codes->major_opcode &&
	err->errorCode == BadDrawable &&
	err->minorCode == X_DRI2DestroyDrawable)
	return True;

    /* If the server is non-local DRI2Connect will raise BadRequest.
     * Swallow this so that DRI2Connect can signal this in its return code */
    if (err->majorCode == codes->major_opcode &&
        err->minorCode == X_DRI2Connect &&
        err->errorCode == BadRequest) {
	*ret_code = False;
	return True;
    }

    return False;
}

Bool
DRI2QueryExtension(Display * dpy, int *eventBase, int *errorBase)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);

   if (XextHasExtension(info)) {
      *eventBase = info->codes->first_event;
      *errorBase = info->codes->first_error;
      return True;
   }

   return False;
}

Bool
DRI2QueryVersion(Display * dpy, int *major, int *minor)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);
   xDRI2QueryVersionReply rep;
   xDRI2QueryVersionReq *req;
   int i, nevents;

   XextCheckExtension(dpy, info, dri2ExtensionName, False);

   LockDisplay(dpy);
   GetReq(DRI2QueryVersion, req);
   req->reqType = info->codes->major_opcode;
   req->dri2ReqType = X_DRI2QueryVersion;
   req->majorVersion = DRI2_MAJOR;
   req->minorVersion = DRI2_MINOR;
   if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
      UnlockDisplay(dpy);
      SyncHandle();
      return False;
   }
   *major = rep.majorVersion;
   *minor = rep.minorVersion;
   UnlockDisplay(dpy);
   SyncHandle();

   switch (rep.minorVersion) {
   case 1:
	   nevents = 0;
	   break;
   case 2:
	   nevents = 1;
	   break;
   case 3:
   default:
	   nevents = 2;
	   break;
   }
	
   for (i = 0; i < nevents; i++) {
       XESetWireToEvent (dpy, info->codes->first_event + i, DRI2WireToEvent);
       XESetEventToWire (dpy, info->codes->first_event + i, DRI2EventToWire);
   }

   return True;
}

Bool
DRI2Connect(Display * dpy, XID window, char **driverName, char **deviceName)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);
   xDRI2ConnectReply rep;
   xDRI2ConnectReq *req;

   XextCheckExtension(dpy, info, dri2ExtensionName, False);

   LockDisplay(dpy);
   GetReq(DRI2Connect, req);
   req->reqType = info->codes->major_opcode;
   req->dri2ReqType = X_DRI2Connect;
   req->window = window;

   req->driverType = DRI2DriverDRI;
   {
      char *prime = getenv("DRI_PRIME");
      if (prime) {
         uint32_t primeid;
         errno = 0;
         primeid = strtoul(prime, NULL, 0);
         if (errno == 0)
            req->driverType |=
               ((primeid & DRI2DriverPrimeMask) << DRI2DriverPrimeShift);
      }
   }

   if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
      UnlockDisplay(dpy);
      SyncHandle();
      return False;
   }

   if (rep.driverNameLength == 0 && rep.deviceNameLength == 0) {
      UnlockDisplay(dpy);
      SyncHandle();
      return False;
   }

   *driverName = malloc(rep.driverNameLength + 1);
   if (*driverName == NULL) {
      _XEatData(dpy,
                ((rep.driverNameLength + 3) & ~3) +
                ((rep.deviceNameLength + 3) & ~3));
      UnlockDisplay(dpy);
      SyncHandle();
      return False;
   }
   _XReadPad(dpy, *driverName, rep.driverNameLength);
   (*driverName)[rep.driverNameLength] = '\0';

   *deviceName = malloc(rep.deviceNameLength + 1);
   if (*deviceName == NULL) {
      free(*driverName);
      _XEatData(dpy, ((rep.deviceNameLength + 3) & ~3));
      UnlockDisplay(dpy);
      SyncHandle();
      return False;
   }
   _XReadPad(dpy, *deviceName, rep.deviceNameLength);
   (*deviceName)[rep.deviceNameLength] = '\0';

   UnlockDisplay(dpy);
   SyncHandle();

   return True;
}

Bool
DRI2Authenticate(Display * dpy, XID window, drm_magic_t magic)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);
   xDRI2AuthenticateReq *req;
   xDRI2AuthenticateReply rep;

   XextCheckExtension(dpy, info, dri2ExtensionName, False);

   LockDisplay(dpy);
   GetReq(DRI2Authenticate, req);
   req->reqType = info->codes->major_opcode;
   req->dri2ReqType = X_DRI2Authenticate;
   req->window = window;
   req->magic = magic;

   if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
      UnlockDisplay(dpy);
      SyncHandle();
      return False;
   }

   UnlockDisplay(dpy);
   SyncHandle();

   return rep.authenticated;
}

void
DRI2CreateDrawable(Display * dpy, XID drawable)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);
   xDRI2CreateDrawableReq *req;

   XextSimpleCheckExtension(dpy, info, dri2ExtensionName);

   LockDisplay(dpy);
   GetReq(DRI2CreateDrawable, req);
   req->reqType = info->codes->major_opcode;
   req->dri2ReqType = X_DRI2CreateDrawable;
   req->drawable = drawable;
   UnlockDisplay(dpy);
   SyncHandle();
}

void
DRI2DestroyDrawable(Display * dpy, XID drawable)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);
   xDRI2DestroyDrawableReq *req;

   XextSimpleCheckExtension(dpy, info, dri2ExtensionName);

   XSync(dpy, False);

   LockDisplay(dpy);
   GetReq(DRI2DestroyDrawable, req);
   req->reqType = info->codes->major_opcode;
   req->dri2ReqType = X_DRI2DestroyDrawable;
   req->drawable = drawable;
   UnlockDisplay(dpy);
   SyncHandle();
}

DRI2Buffer *
DRI2GetBuffers(Display * dpy, XID drawable,
               int *width, int *height,
               unsigned int *attachments, int count, int *outCount)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);
   xDRI2GetBuffersReply rep;
   xDRI2GetBuffersReq *req;
   DRI2Buffer *buffers;
   xDRI2Buffer repBuffer;
   CARD32 *p;
   int i;

   XextCheckExtension(dpy, info, dri2ExtensionName, NULL);

   LockDisplay(dpy);
   GetReqExtra(DRI2GetBuffers, count * 4, req);
   req->reqType = info->codes->major_opcode;
   req->dri2ReqType = X_DRI2GetBuffers;
   req->drawable = drawable;
   req->count = count;
   p = (CARD32 *) & req[1];
   for (i = 0; i < count; i++)
      p[i] = attachments[i];

   if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
      UnlockDisplay(dpy);
      SyncHandle();
      return NULL;
   }

   *width = rep.width;
   *height = rep.height;
   *outCount = rep.count;

   buffers = malloc(rep.count * sizeof buffers[0]);
   if (buffers == NULL) {
      _XEatData(dpy, rep.count * sizeof repBuffer);
      UnlockDisplay(dpy);
      SyncHandle();
      return NULL;
   }

   for (i = 0; i < rep.count; i++) {
      _XReadPad(dpy, (char *) &repBuffer, sizeof repBuffer);
      buffers[i].attachment = repBuffer.attachment;
      buffers[i].name = repBuffer.name;
      buffers[i].pitch = repBuffer.pitch;
      buffers[i].cpp = repBuffer.cpp;
      buffers[i].flags = repBuffer.flags;
   }

   UnlockDisplay(dpy);
   SyncHandle();

   return buffers;
}


DRI2Buffer *
DRI2GetBuffersWithFormat(Display * dpy, XID drawable,
                         int *width, int *height,
                         unsigned int *attachments, int count, int *outCount)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);
   xDRI2GetBuffersReply rep;
   xDRI2GetBuffersReq *req;
   DRI2Buffer *buffers;
   xDRI2Buffer repBuffer;
   CARD32 *p;
   int i;

   XextCheckExtension(dpy, info, dri2ExtensionName, NULL);

   LockDisplay(dpy);
   GetReqExtra(DRI2GetBuffers, count * (4 * 2), req);
   req->reqType = info->codes->major_opcode;
   req->dri2ReqType = X_DRI2GetBuffersWithFormat;
   req->drawable = drawable;
   req->count = count;
   p = (CARD32 *) & req[1];
   for (i = 0; i < (count * 2); i++)
      p[i] = attachments[i];

   if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
      UnlockDisplay(dpy);
      SyncHandle();
      return NULL;
   }

   *width = rep.width;
   *height = rep.height;
   *outCount = rep.count;

   buffers = malloc(rep.count * sizeof buffers[0]);
   if (buffers == NULL) {
      _XEatData(dpy, rep.count * sizeof repBuffer);
      UnlockDisplay(dpy);
      SyncHandle();
      return NULL;
   }

   for (i = 0; i < rep.count; i++) {
      _XReadPad(dpy, (char *) &repBuffer, sizeof repBuffer);
      buffers[i].attachment = repBuffer.attachment;
      buffers[i].name = repBuffer.name;
      buffers[i].pitch = repBuffer.pitch;
      buffers[i].cpp = repBuffer.cpp;
      buffers[i].flags = repBuffer.flags;
   }

   UnlockDisplay(dpy);
   SyncHandle();

   return buffers;
}


void
DRI2CopyRegion(Display * dpy, XID drawable, XserverRegion region,
               CARD32 dest, CARD32 src)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);
   xDRI2CopyRegionReq *req;
   xDRI2CopyRegionReply rep;

   XextSimpleCheckExtension(dpy, info, dri2ExtensionName);

   LockDisplay(dpy);
   GetReq(DRI2CopyRegion, req);
   req->reqType = info->codes->major_opcode;
   req->dri2ReqType = X_DRI2CopyRegion;
   req->drawable = drawable;
   req->region = region;
   req->dest = dest;
   req->src = src;

   _XReply(dpy, (xReply *) & rep, 0, xFalse);

   UnlockDisplay(dpy);
   SyncHandle();
}

#endif /* GLX_DIRECT_RENDERING */
