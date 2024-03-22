/*
 * (C) Copyright IBM Corporation 2004
 * All Rights Reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file glx_pbuffer.c
 * Implementation of pbuffer related functions.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#include <inttypes.h>
#include "glxclient.h"
#include <X11/extensions/extutil.h>
#include <X11/extensions/Xext.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include "glxextensions.h"

#include <X11/Xlib-xcb.h>
#include <xcb/xproto.h>

#ifdef GLX_USE_APPLEGL
#include <pthread.h>
#include "apple/apple_glx_drawable.h"
#endif

#include "glx_error.h"

#ifndef GLX_USE_APPLEGL
/**
 * Change a drawable's attribute.
 *
 * This function is used to implement \c glXSelectEvent and
 * \c glXSelectEventSGIX.
 */
static void
ChangeDrawableAttribute(Display * dpy, GLXDrawable drawable,
                        const CARD32 * attribs, size_t num_attribs)
{
   struct glx_display *priv = __glXInitialize(dpy);
#ifdef GLX_DIRECT_RENDERING
   __GLXDRIdrawable *pdraw;
   int i;
#endif
   CARD32 *output;
   CARD8 opcode;

   if ((priv == NULL) || (dpy == NULL) || (drawable == 0)) {
      return;
   }

   opcode = __glXSetupForCommand(dpy);
   if (!opcode)
      return;

   LockDisplay(dpy);

   xGLXChangeDrawableAttributesReq *req;
   GetReqExtra(GLXChangeDrawableAttributes, 8 * num_attribs, req);
   output = (CARD32 *) (req + 1);

   req->reqType = opcode;
   req->glxCode = X_GLXChangeDrawableAttributes;
   req->drawable = drawable;
   req->numAttribs = (CARD32) num_attribs;

   (void) memcpy(output, attribs, sizeof(CARD32) * 2 * num_attribs);

   UnlockDisplay(dpy);
   SyncHandle();

#ifdef GLX_DIRECT_RENDERING
   pdraw = GetGLXDRIDrawable(dpy, drawable);

   if (!pdraw)
      return;

   for (i = 0; i < num_attribs; i++) {
      switch(attribs[i * 2]) {
      case GLX_EVENT_MASK:
	 /* Keep a local copy for masking out DRI2 proto events as needed */
	 pdraw->eventMask = attribs[i * 2 + 1];
	 break;
      }
   }
#endif

   return;
}


#ifdef GLX_DIRECT_RENDERING
static GLenum
determineTextureTarget(const int *attribs, int numAttribs)
{
   GLenum target = 0;
   int i;

   for (i = 0; i < numAttribs; i++) {
      if (attribs[2 * i] == GLX_TEXTURE_TARGET_EXT) {
         switch (attribs[2 * i + 1]) {
         case GLX_TEXTURE_2D_EXT:
            target = GL_TEXTURE_2D;
            break;
         case GLX_TEXTURE_RECTANGLE_EXT:
            target = GL_TEXTURE_RECTANGLE_ARB;
            break;
         }
      }
   }

   return target;
}

static GLenum
determineTextureFormat(const int *attribs, int numAttribs)
{
   int i;

   for (i = 0; i < numAttribs; i++) {
      if (attribs[2 * i] == GLX_TEXTURE_FORMAT_EXT)
         return attribs[2 * i + 1];
   }

   return 0;
}
#endif

static GLboolean
CreateDRIDrawable(Display *dpy, struct glx_config *config,
		  XID drawable, XID glxdrawable, int type,
		  const int *attrib_list, size_t num_attribs)
{
#ifdef GLX_DIRECT_RENDERING
   struct glx_display *const priv = __glXInitialize(dpy);
   __GLXDRIdrawable *pdraw;
   struct glx_screen *psc;

   if (priv == NULL) {
      fprintf(stderr, "failed to create drawable\n");
      return GL_FALSE;
   }

   psc = priv->screens[config->screen];
   if (psc->driScreen == NULL)
      return GL_TRUE;

   pdraw = psc->driScreen->createDrawable(psc, drawable, glxdrawable,
                                          type, config);
   if (pdraw == NULL) {
      fprintf(stderr, "failed to create drawable\n");
      return GL_FALSE;
   }

   if (__glxHashInsert(priv->drawHash, glxdrawable, pdraw)) {
      pdraw->destroyDrawable(pdraw);
      return GL_FALSE;
   }

   pdraw->textureTarget = determineTextureTarget(attrib_list, num_attribs);
   pdraw->textureFormat = determineTextureFormat(attrib_list, num_attribs);

   pdraw->refcount = 1;
#endif

   return GL_TRUE;
}

static void
DestroyDRIDrawable(Display *dpy, GLXDrawable drawable)
{
#ifdef GLX_DIRECT_RENDERING
   struct glx_display *const priv = __glXInitialize(dpy);
   __GLXDRIdrawable *pdraw = GetGLXDRIDrawable(dpy, drawable);

   if (priv != NULL && pdraw != NULL) {
      pdraw->destroyDrawable(pdraw);
      __glxHashDelete(priv->drawHash, drawable);
   }
#endif
}

/**
 * Get a drawable's attribute.
 *
 * This function is used to implement \c glXGetSelectedEvent and
 * \c glXGetSelectedEventSGIX.
 *
 * \todo
 * The number of attributes returned is likely to be small, probably less than
 * 10.  Given that, this routine should try to use an array on the stack to
 * capture the reply rather than always calling Xmalloc.
 */
int
__glXGetDrawableAttribute(Display * dpy, GLXDrawable drawable,
                          int attribute, unsigned int *value)
{
   struct glx_display *priv;
   xGLXGetDrawableAttributesReply reply;
   CARD32 *data;
   CARD8 opcode;
   unsigned int length;
   unsigned int i;
   unsigned int num_attributes;
   int found = 0;

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
   __GLXDRIdrawable *pdraw;
#endif

   if (dpy == NULL)
      return 0;

   /* Page 38 (page 52 of the PDF) of glxencode1.3.pdf says:
    *
    *     "If drawable is not a valid GLX drawable, a GLXBadDrawable error is
    *     generated."
    */
   if (drawable == 0) {
      XNoOp(dpy);
      __glXSendError(dpy, GLXBadDrawable, 0, X_GLXGetDrawableAttributes, false);
      return 0;
   }

   priv = __glXInitialize(dpy);
   if (priv == NULL)
      return 0;

   *value = 0;

   opcode = __glXSetupForCommand(dpy);
   if (!opcode)
      return 0;

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
   pdraw = GetGLXDRIDrawable(dpy, drawable);

   if (attribute == GLX_BACK_BUFFER_AGE_EXT) {
      struct glx_context *gc = __glXGetCurrentContext();
      struct glx_screen *psc;

      /* The GLX_EXT_buffer_age spec says:
       *
       *   "If querying GLX_BACK_BUFFER_AGE_EXT and <draw> is not bound to
       *   the calling thread's current context a GLXBadDrawable error is
       *   generated."
       */
      if (pdraw == NULL || gc == &dummyContext || gc->currentDpy != dpy ||
         (gc->currentDrawable != drawable &&
         gc->currentReadable != drawable)) {
         XNoOp(dpy);
         __glXSendError(dpy, GLXBadDrawable, drawable,
                        X_GLXGetDrawableAttributes, false);
         return 0;
      }

      psc = pdraw->psc;

      if (psc->driScreen->getBufferAge != NULL)
         *value = psc->driScreen->getBufferAge(pdraw);

      return 1;
   }

   if (pdraw) {
      if (attribute == GLX_SWAP_INTERVAL_EXT) {
         *value = pdraw->psc->driScreen->getSwapInterval(pdraw);
         return 1;
      } else if (attribute == GLX_MAX_SWAP_INTERVAL_EXT) {
         *value = pdraw->psc->driScreen->maxSwapInterval;
         return 1;
      } else if (attribute == GLX_LATE_SWAPS_TEAR_EXT) {
         *value = __glXExtensionBitIsEnabled(pdraw->psc,
                                             EXT_swap_control_tear_bit);
         return 1;
      }
   }
#endif

   LockDisplay(dpy);

   xGLXGetDrawableAttributesReq *req;
   GetReq(GLXGetDrawableAttributes, req);
   req->reqType = opcode;
   req->glxCode = X_GLXGetDrawableAttributes;
   req->drawable = drawable;

   _XReply(dpy, (xReply *) & reply, 0, False);

   if (reply.type == X_Error) {
      UnlockDisplay(dpy);
      SyncHandle();
      return 0;
   }

   length = reply.length;
   if (length) {
      num_attributes = reply.numAttribs;
      data = malloc(length * sizeof(CARD32));
      if (data == NULL) {
         /* Throw data on the floor */
         _XEatData(dpy, length);
      }
      else {
         _XRead(dpy, (char *) data, length * sizeof(CARD32));

         /* Search the set of returned attributes for the attribute requested by
          * the caller.
          */
         for (i = 0; i < num_attributes; i++) {
            if (data[i * 2] == attribute) {
               found = 1;
               *value = data[(i * 2) + 1];
               break;
            }
         }

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
         if (pdraw != NULL) {
            if (!pdraw->textureTarget)
               pdraw->textureTarget =
                  determineTextureTarget((const int *) data, num_attributes);
            if (!pdraw->textureFormat)
               pdraw->textureFormat =
                  determineTextureFormat((const int *) data, num_attributes);
         }
#endif

         free(data);
      }
   }

   UnlockDisplay(dpy);
   SyncHandle();

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
   if (pdraw && attribute == GLX_FBCONFIG_ID && !found) {
      /* If we failed to lookup the GLX_FBCONFIG_ID, it may be because the drawable is
       * a bare Window, so try differently by first figure out its visual, then GLX
       * visual like driInferDrawableConfig does.
       */
      xcb_get_window_attributes_cookie_t cookie = { 0 };
      xcb_get_window_attributes_reply_t *attr = NULL;

      xcb_connection_t *conn = XGetXCBConnection(dpy);

      if (conn) {
         cookie = xcb_get_window_attributes(conn, drawable);
         attr = xcb_get_window_attributes_reply(conn, cookie, NULL);
         if (attr) {
            /* Find the Window's GLX Visual */
            struct glx_config *conf = glx_config_find_visual(pdraw->psc->configs, attr->visual);
            free(attr);

            if (conf)
               *value = conf->fbconfigID;
         }
      }
   }
#endif

   return found;
}

static int dummyErrorHandler(Display *display, xError *err, XExtCodes *codes,
                             int *ret_code)
{
    return 1; /* do nothing */
}

static void
protocolDestroyDrawable(Display *dpy, GLXDrawable drawable, CARD32 glxCode)
{
   xGLXDestroyPbufferReq *req;
   CARD8 opcode;

   opcode = __glXSetupForCommand(dpy);
   if (!opcode)
      return;

   LockDisplay(dpy);

   GetReq(GLXDestroyPbuffer, req);
   req->reqType = opcode;
   req->glxCode = glxCode;
   req->pbuffer = (GLXPbuffer) drawable;

   UnlockDisplay(dpy);
   SyncHandle();

   /* Viewperf2020/Sw calls XDestroyWindow(win) and then glXDestroyWindow(win),
    * causing an X error and abort. This is the workaround.
    */
   struct glx_display *priv = __glXInitialize(dpy);

   if (priv->screens[0] &&
       priv->screens[0]->allow_invalid_glx_destroy_window) {
      void *old = XESetError(priv->dpy, priv->codes.extension,
                             dummyErrorHandler);
      XSync(dpy, false);
      XESetError(priv->dpy, priv->codes.extension, old);
   }
}

/**
 * Create a non-pbuffer GLX drawable.
 */
static GLXDrawable
CreateDrawable(Display *dpy, struct glx_config *config,
               Drawable drawable, int type, const int *attrib_list)
{
   xGLXCreateWindowReq *req;
   struct glx_drawable *glxDraw;
   CARD32 *data;
   unsigned int i;
   CARD8 opcode;
   GLXDrawable xid;

   if (!config)
      return None;

   i = 0;
   if (attrib_list) {
      while (attrib_list[i * 2] != None)
         i++;
   }

   opcode = __glXSetupForCommand(dpy);
   if (!opcode)
      return None;

   glxDraw = malloc(sizeof(*glxDraw));
   if (!glxDraw)
      return None;

   LockDisplay(dpy);
   GetReqExtra(GLXCreateWindow, 8 * i, req);
   data = (CARD32 *) (req + 1);

   req->reqType = opcode;
   req->screen = config->screen;
   req->fbconfig = config->fbconfigID;
   req->window = drawable;
   req->glxwindow = xid = XAllocID(dpy);
   req->numAttribs = i;

   if (type == GLX_WINDOW_BIT)
      req->glxCode = X_GLXCreateWindow;
   else
      req->glxCode = X_GLXCreatePixmap;

   if (attrib_list)
      memcpy(data, attrib_list, 8 * i);

   UnlockDisplay(dpy);
   SyncHandle();

   if (InitGLXDrawable(dpy, glxDraw, drawable, xid)) {
      free(glxDraw);
      return None;
   }

   if (!CreateDRIDrawable(dpy, config, drawable, xid, type, attrib_list, i)) {
      CARD8 glxCode;
      if (type == GLX_PIXMAP_BIT)
         glxCode = X_GLXDestroyPixmap;
      else
         glxCode = X_GLXDestroyWindow;
      protocolDestroyDrawable(dpy, xid, glxCode);
      xid = None;
   }

   return xid;
}


/**
 * Destroy a non-pbuffer GLX drawable.
 */
static void
DestroyDrawable(Display * dpy, GLXDrawable drawable, CARD32 glxCode)
{
   protocolDestroyDrawable(dpy, drawable, glxCode);

   DestroyGLXDrawable(dpy, drawable);
   DestroyDRIDrawable(dpy, drawable);

   return;
}


/**
 * Create a pbuffer.
 *
 * This function is used to implement \c glXCreatePbuffer and
 * \c glXCreateGLXPbufferSGIX.
 */
static GLXDrawable
CreatePbuffer(Display * dpy, struct glx_config *config,
              unsigned int width, unsigned int height,
              const int *attrib_list, GLboolean size_in_attribs)
{
   struct glx_display *priv = __glXInitialize(dpy);
   GLXDrawable id = 0;
   CARD32 *data;
   CARD8 opcode;
   unsigned int i;

   if (priv == NULL)
      return None;

   i = 0;
   if (attrib_list) {
      while (attrib_list[i * 2])
         i++;
   }

   opcode = __glXSetupForCommand(dpy);
   if (!opcode)
      return None;

   LockDisplay(dpy);
   id = XAllocID(dpy);

   xGLXCreatePbufferReq *req;
   unsigned int extra = (size_in_attribs) ? 0 : 2;
   GetReqExtra(GLXCreatePbuffer, (8 * (i + extra)), req);
   data = (CARD32 *) (req + 1);

   req->reqType = opcode;
   req->glxCode = X_GLXCreatePbuffer;
   req->screen = config->screen;
   req->fbconfig = config->fbconfigID;
   req->pbuffer = id;
   req->numAttribs = i + extra;

   if (!size_in_attribs) {
      data[(2 * i) + 0] = GLX_PBUFFER_WIDTH;
      data[(2 * i) + 1] = width;
      data[(2 * i) + 2] = GLX_PBUFFER_HEIGHT;
      data[(2 * i) + 3] = height;
      data += 4;
   }

   (void) memcpy(data, attrib_list, sizeof(CARD32) * 2 * i);

   UnlockDisplay(dpy);
   SyncHandle();

   /* xserver created a pixmap with the same id as pbuffer */
   if (!CreateDRIDrawable(dpy, config, id, id, GLX_PBUFFER_BIT, attrib_list, i)) {
      protocolDestroyDrawable(dpy, id, X_GLXDestroyPbuffer);
      id = None;
   }

   return id;
}

/**
 * Destroy a pbuffer.
 *
 * This function is used to implement \c glXDestroyPbuffer and
 * \c glXDestroyGLXPbufferSGIX.
 */
static void
DestroyPbuffer(Display * dpy, GLXDrawable drawable)
{
   struct glx_display *priv = __glXInitialize(dpy);
   CARD8 opcode;

   if ((priv == NULL) || (dpy == NULL) || (drawable == 0)) {
      return;
   }

   opcode = __glXSetupForCommand(dpy);
   if (!opcode)
      return;

   LockDisplay(dpy);

   xGLXDestroyPbufferReq *req;
   GetReq(GLXDestroyPbuffer, req);
   req->reqType = opcode;
   req->glxCode = X_GLXDestroyPbuffer;
   req->pbuffer = (GLXPbuffer) drawable;

   UnlockDisplay(dpy);
   SyncHandle();

   DestroyDRIDrawable(dpy, drawable);

   return;
}

/**
 * Create a new pbuffer.
 */
_GLX_PUBLIC GLXPbufferSGIX
glXCreateGLXPbufferSGIX(Display * dpy, GLXFBConfigSGIX config,
                        unsigned int width, unsigned int height,
                        int *attrib_list)
{
   return (GLXPbufferSGIX) CreatePbuffer(dpy, (struct glx_config *) config,
                                         width, height,
                                         attrib_list, GL_FALSE);
}

#endif /* GLX_USE_APPLEGL */

/**
 * Create a new pbuffer.
 */
_GLX_PUBLIC GLXPbuffer
glXCreatePbuffer(Display * dpy, GLXFBConfig config, const int *attrib_list)
{
   int i, width, height;
#ifdef GLX_USE_APPLEGL
   GLXPbuffer result;
   int errorcode;
#endif

   width = 0;
   height = 0;

#ifdef GLX_USE_APPLEGL
   for (i = 0; attrib_list[i]; ++i) {
      switch (attrib_list[i]) {
      case GLX_PBUFFER_WIDTH:
         width = attrib_list[i + 1];
         ++i;
         break;

      case GLX_PBUFFER_HEIGHT:
         height = attrib_list[i + 1];
         ++i;
         break;

      case GLX_LARGEST_PBUFFER:
         /* This is a hint we should probably handle, but how? */
         ++i;
         break;

      case GLX_PRESERVED_CONTENTS:
         /* The contents are always preserved with AppleSGLX with CGL. */
         ++i;
         break;

      default:
         return None;
      }
   }

   if (apple_glx_pbuffer_create(dpy, config, width, height, &errorcode,
                                &result)) {
      /* 
       * apple_glx_pbuffer_create only sets the errorcode to core X11
       * errors. 
       */
      __glXSendError(dpy, errorcode, 0, X_GLXCreatePbuffer, true);

      return None;
   }

   return result;
#else
   for (i = 0; attrib_list[i * 2]; i++) {
      switch (attrib_list[i * 2]) {
      case GLX_PBUFFER_WIDTH:
         width = attrib_list[i * 2 + 1];
         break;
      case GLX_PBUFFER_HEIGHT:
         height = attrib_list[i * 2 + 1];
         break;
      }
   }

   return (GLXPbuffer) CreatePbuffer(dpy, (struct glx_config *) config,
                                     width, height, attrib_list, GL_TRUE);
#endif
}


/**
 * Destroy an existing pbuffer.
 */
_GLX_PUBLIC void
glXDestroyPbuffer(Display * dpy, GLXPbuffer pbuf)
{
#ifdef GLX_USE_APPLEGL
   if (apple_glx_pbuffer_destroy(dpy, pbuf)) {
      __glXSendError(dpy, GLXBadPbuffer, pbuf, X_GLXDestroyPbuffer, false);
   }
#else
   DestroyPbuffer(dpy, pbuf);
#endif
}


/**
 * Query an attribute of a drawable.
 */
_GLX_PUBLIC void
glXQueryDrawable(Display * dpy, GLXDrawable drawable,
                 int attribute, unsigned int *value)
{
#ifdef GLX_USE_APPLEGL
   Window root;
   int x, y;
   unsigned int width, height, bd, depth;

   if (apple_glx_pixmap_query(drawable, attribute, value))
      return;                   /*done */

   if (apple_glx_pbuffer_query(drawable, attribute, value))
      return;                   /*done */

   /*
    * The OpenGL spec states that we should report GLXBadDrawable if
    * the drawable is invalid, however doing so would require that we
    * use XSetErrorHandler(), which is known to not be thread safe.
    * If we use a round-trip call to validate the drawable, there could
    * be a race, so instead we just opt in favor of letting the
    * XGetGeometry request fail with a GetGeometry request X error 
    * rather than GLXBadDrawable, in what is hoped to be a rare
    * case of an invalid drawable.  In practice most and possibly all
    * X11 apps using GLX shouldn't notice a difference.
    */
   if (XGetGeometry
       (dpy, drawable, &root, &x, &y, &width, &height, &bd, &depth)) {
      switch (attribute) {
      case GLX_WIDTH:
         *value = width;
         break;

      case GLX_HEIGHT:
         *value = height;
         break;
      }
   }
#else
   __glXGetDrawableAttribute(dpy, drawable, attribute, value);
#endif
}


#ifndef GLX_USE_APPLEGL
/**
 * Query an attribute of a pbuffer.
 */
_GLX_PUBLIC void
glXQueryGLXPbufferSGIX(Display * dpy, GLXPbufferSGIX drawable,
                       int attribute, unsigned int *value)
{
   __glXGetDrawableAttribute(dpy, drawable, attribute, value);
}
#endif

/**
 * Select the event mask for a drawable.
 */
_GLX_PUBLIC void
glXSelectEvent(Display * dpy, GLXDrawable drawable, unsigned long mask)
{
#ifdef GLX_USE_APPLEGL
   XWindowAttributes xwattr;

   if (apple_glx_pbuffer_set_event_mask(drawable, mask))
      return;                   /*done */

   /* 
    * The spec allows a window, but currently there are no valid
    * events for a window, so do nothing.
    */
   if (XGetWindowAttributes(dpy, drawable, &xwattr))
      return;                   /*done */
   /* The drawable seems to be invalid.  Report an error. */

   __glXSendError(dpy, GLXBadDrawable, drawable,
                  X_GLXChangeDrawableAttributes, false);
#else
   CARD32 attribs[2];

   attribs[0] = (CARD32) GLX_EVENT_MASK;
   attribs[1] = (CARD32) mask;

   ChangeDrawableAttribute(dpy, drawable, attribs, 1);
#endif
}


/**
 * Get the selected event mask for a drawable.
 */
_GLX_PUBLIC void
glXGetSelectedEvent(Display * dpy, GLXDrawable drawable, unsigned long *mask)
{
#ifdef GLX_USE_APPLEGL
   XWindowAttributes xwattr;

   if (apple_glx_pbuffer_get_event_mask(drawable, mask))
      return;                   /*done */

   /* 
    * The spec allows a window, but currently there are no valid
    * events for a window, so do nothing, but set the mask to 0.
    */
   if (XGetWindowAttributes(dpy, drawable, &xwattr)) {
      /* The window is valid, so set the mask to 0. */
      *mask = 0;
      return;                   /*done */
   }
   /* The drawable seems to be invalid.  Report an error. */

   __glXSendError(dpy, GLXBadDrawable, drawable, X_GLXGetDrawableAttributes,
                  true);
#else
   unsigned int value = 0;


   /* The non-sense with value is required because on LP64 platforms
    * sizeof(unsigned int) != sizeof(unsigned long).  On little-endian
    * we could just type-cast the pointer, but why?
    */

   __glXGetDrawableAttribute(dpy, drawable, GLX_EVENT_MASK_SGIX, &value);
   *mask = value;
#endif
}


_GLX_PUBLIC GLXPixmap
glXCreatePixmap(Display * dpy, GLXFBConfig config, Pixmap pixmap,
                const int *attrib_list)
{
#ifdef GLX_USE_APPLEGL
   const struct glx_config *modes = (const struct glx_config *) config;

   if (apple_glx_pixmap_create(dpy, modes->screen, pixmap, modes))
      return None;

   return pixmap;
#else
   return CreateDrawable(dpy, (struct glx_config *) config,
                         (Drawable) pixmap, GLX_PIXMAP_BIT, attrib_list);
#endif
}


_GLX_PUBLIC GLXWindow
glXCreateWindow(Display * dpy, GLXFBConfig config, Window win,
                const int *attrib_list)
{
#ifdef GLX_USE_APPLEGL
   XWindowAttributes xwattr;
   XVisualInfo *visinfo;

   (void) attrib_list;          /*unused according to GLX 1.4 */

   XGetWindowAttributes(dpy, win, &xwattr);

   visinfo = glXGetVisualFromFBConfig(dpy, config);

   if (NULL == visinfo) {
      __glXSendError(dpy, GLXBadFBConfig, 0, X_GLXCreateWindow, false);
      return None;
   }

   if (visinfo->visualid != XVisualIDFromVisual(xwattr.visual)) {
      __glXSendError(dpy, BadMatch, 0, X_GLXCreateWindow, true);
      return None;
   }

   free(visinfo);

   return win;
#else
   return CreateDrawable(dpy, (struct glx_config *) config,
                         (Drawable) win, GLX_WINDOW_BIT, attrib_list);
#endif
}


_GLX_PUBLIC void
glXDestroyPixmap(Display * dpy, GLXPixmap pixmap)
{
#ifdef GLX_USE_APPLEGL
   if (apple_glx_pixmap_destroy(dpy, pixmap))
      __glXSendError(dpy, GLXBadPixmap, pixmap, X_GLXDestroyPixmap, false);
#else
   DestroyDrawable(dpy, (GLXDrawable) pixmap, X_GLXDestroyPixmap);
#endif
}


_GLX_PUBLIC void
glXDestroyWindow(Display * dpy, GLXWindow win)
{
#ifndef GLX_USE_APPLEGL
   DestroyDrawable(dpy, (GLXDrawable) win, X_GLXDestroyWindow);
#endif
}

_GLX_PUBLIC
GLX_ALIAS_VOID(glXDestroyGLXPbufferSGIX,
               (Display * dpy, GLXPbufferSGIX pbuf),
               (dpy, pbuf), glXDestroyPbuffer)

_GLX_PUBLIC
GLX_ALIAS_VOID(glXSelectEventSGIX,
               (Display * dpy, GLXDrawable drawable,
                unsigned long mask), (dpy, drawable, mask), glXSelectEvent)

_GLX_PUBLIC
GLX_ALIAS_VOID(glXGetSelectedEventSGIX,
               (Display * dpy, GLXDrawable drawable,
                unsigned long *mask), (dpy, drawable, mask),
               glXGetSelectedEvent)

_GLX_PUBLIC GLXPixmap
glXCreateGLXPixmap(Display * dpy, XVisualInfo * vis, Pixmap pixmap)
{
#ifdef GLX_USE_APPLEGL
   int screen = vis->screen;
   struct glx_screen *const psc = GetGLXScreenConfigs(dpy, screen);
   const struct glx_config *config;

   config = glx_config_find_visual(psc->visuals, vis->visualid);

   if(apple_glx_pixmap_create(dpy, vis->screen, pixmap, config))
      return None;

   return pixmap;
#else
   xGLXCreateGLXPixmapReq *req;
   struct glx_drawable *glxDraw;
   GLXPixmap xid;
   CARD8 opcode;

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
   struct glx_display *const priv = __glXInitialize(dpy);

   if (priv == NULL)
      return None;
#endif

   opcode = __glXSetupForCommand(dpy);
   if (!opcode) {
      return None;
   }

   glxDraw = malloc(sizeof(*glxDraw));
   if (!glxDraw)
      return None;

   /* Send the glXCreateGLXPixmap request */
   LockDisplay(dpy);
   GetReq(GLXCreateGLXPixmap, req);
   req->reqType = opcode;
   req->glxCode = X_GLXCreateGLXPixmap;
   req->screen = vis->screen;
   req->visual = vis->visualid;
   req->pixmap = pixmap;
   req->glxpixmap = xid = XAllocID(dpy);
   UnlockDisplay(dpy);
   SyncHandle();

   if (InitGLXDrawable(dpy, glxDraw, pixmap, req->glxpixmap)) {
      free(glxDraw);
      return None;
   }

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
   do {
      /* FIXME: Maybe delay __DRIdrawable creation until the drawable
       * is actually bound to a context... */

      struct glx_screen *psc = GetGLXScreenConfigs(dpy, vis->screen);
      struct glx_config *config = glx_config_find_visual(psc->visuals,
                                                         vis->visualid);

      if (!CreateDRIDrawable(dpy, config, pixmap, xid, GLX_PIXMAP_BIT,
                             NULL, 0)) {
         protocolDestroyDrawable(dpy, xid, X_GLXDestroyGLXPixmap);
         xid = None;
      }
   } while (0);
#endif

   return xid;
#endif
}

/*
** Destroy the named pixmap
*/
_GLX_PUBLIC void
glXDestroyGLXPixmap(Display * dpy, GLXPixmap glxpixmap)
{
#ifdef GLX_USE_APPLEGL
   if(apple_glx_pixmap_destroy(dpy, glxpixmap))
      __glXSendError(dpy, GLXBadPixmap, glxpixmap, X_GLXDestroyPixmap, false);
#else
   DestroyDrawable(dpy, glxpixmap, X_GLXDestroyGLXPixmap);
#endif /* GLX_USE_APPLEGL */
}

_GLX_PUBLIC GLXPixmap
glXCreateGLXPixmapWithConfigSGIX(Display * dpy,
                                 GLXFBConfigSGIX fbconfig,
                                 Pixmap pixmap)
{
   return glXCreatePixmap(dpy, fbconfig, pixmap, NULL);
}
