/*
 * Copyright © 2011 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <limits.h>
#include "glxclient.h"
#include "glx_error.h"
#include <xcb/glx.h>
#include <X11/Xlib-xcb.h>

#include <assert.h>

#if INT_MAX != 2147483647
#error This code requires sizeof(uint32_t) == sizeof(int).
#endif

/* An "Atrribs/Attribs" typo was fixed in glxproto.h in Nov 2014.
 * This is in case we don't have the updated header.
 */
#if !defined(X_GLXCreateContextAttribsARB) && \
     defined(X_GLXCreateContextAtrribsARB)
#define X_GLXCreateContextAttribsARB X_GLXCreateContextAtrribsARB
#endif

_X_HIDDEN GLXContext
glXCreateContextAttribsARB(Display *dpy, GLXFBConfig config,
                           GLXContext share_context, Bool direct,
                           const int *attrib_list)
{
   xcb_connection_t *const c = XGetXCBConnection(dpy);
   struct glx_config *const cfg = (struct glx_config *) config;
   struct glx_context *const share = (struct glx_context *) share_context;
   struct glx_context *gc = NULL;
   unsigned num_attribs = 0;
   struct glx_screen *psc;
   xcb_generic_error_t *err;
   xcb_void_cookie_t cookie;
   unsigned error = BadImplementation;
   uint32_t xid, share_xid;
   int screen = -1;

   if (dpy == NULL)
      return NULL;

   /* Count the number of attributes specified by the application.  All
    * attributes appear in pairs, except the terminating None.
    */
   if (attrib_list != NULL) {
      for (/* empty */; attrib_list[num_attribs * 2] != 0; num_attribs++)
         /* empty */ ;
   }

   if (cfg) {
      screen = cfg->screen;
   } else {
      for (unsigned int i = 0; i < num_attribs; i++) {
         if (attrib_list[i * 2] == GLX_SCREEN)
            screen = attrib_list[i * 2 + 1];
      }
      if (screen == -1) {
         __glXSendError(dpy, BadValue, 0, X_GLXCreateContextAttribsARB, True);
         return NULL;
      }
   }

   /* This means that either the caller passed the wrong display pointer or
    * one of the internal GLX data structures (probably the fbconfig) has an
    * error.  There is nothing sensible to do, so return an error.
    */
   psc = GetGLXScreenConfigs(dpy, screen);
   if (psc == NULL)
      return NULL;

   assert(screen == psc->scr);

   /* Some application may request an indirect context but we may want to force a direct
    * one because Xorg only allows indirect contexts if they were enabled.
    */
   if (!direct &&
       psc->force_direct_context) {
      direct = true;
   }

#ifdef GLX_USE_APPLEGL
   gc = applegl_create_context(psc, cfg, share, 0);
#else
   if (direct && psc->vtable->create_context_attribs) {
      gc = psc->vtable->create_context_attribs(psc, cfg, share, num_attribs,
					       (const uint32_t *) attrib_list,
					       &error);
   } else if (!direct) {
      gc = indirect_create_context_attribs(psc, cfg, share, num_attribs,
                                           (const uint32_t *) attrib_list,
                                           &error);
   }
#endif

   if (gc == NULL) {
      /* Increment dpy->request in order to give a unique serial number to the error.
       * This may break creating contexts on some video cards, if libx11 <1.7.4 is used.
       * However, this fixes creating contexts (on some video cards) if libx11 >=1.7.4 is used.
       */
      XNoOp(dpy);
      /* -1 isn't a legal XID, which is sort of the point, we've failed
       * before we even got to XID allocation.
       */
      if (error == GLXBadContext || error == GLXBadFBConfig ||
          error == GLXBadProfileARB)
         __glXSendError(dpy, error, -1, 0, False);
      else
         __glXSendError(dpy, error, -1, 0, True);
      return NULL;
   }

   xid = xcb_generate_id(c);
   share_xid = (share != NULL) ? share->xid : 0;

   /* The manual pages for glXCreateContext and glXCreateNewContext say:
    *
    *     "NULL is returned if execution fails on the client side."
    *
    * If the server generates an error, the application is supposed to catch
    * the protocol error and handle it.  Part of handling the error is freeing
    * the possibly non-NULL value returned by this function.
    */
   cookie =
      xcb_glx_create_context_attribs_arb_checked(c,
                                                 xid,
                                                 cfg ? cfg->fbconfigID : 0,
                                                 screen,
                                                 share_xid,
                                                 gc->isDirect,
                                                 num_attribs,
                                                 (const uint32_t *)
                                                 attrib_list);
   err = xcb_request_check(c, cookie);
   if (err != NULL) {
      if (gc)
         gc->vtable->destroy(gc);
      gc = NULL;

      __glXSendErrorForXcb(dpy, err);
      free(err);
   } else {
      gc->xid = xid;
      gc->share_xid = share_xid;
   }

   return (GLXContext) gc;
}
