/*
 * Copyright © 2010 Intel Corporation
 * Copyright © 2011 Apple Inc.
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
 *   Kristian Høgsberg (krh@bitplanet.net)
 */

#if defined(GLX_USE_APPLEGL)

#include <stdbool.h>
#include <dlfcn.h>

#include "glxclient.h"
#include "apple/apple_glx_context.h"
#include "apple/apple_glx.h"
#include "apple/apple_cgl.h"
#include "glx_error.h"

static void
applegl_destroy_context(struct glx_context *gc)
{
   apple_glx_destroy_context(&gc->driContext, gc->psc->dpy);
}

static int
applegl_bind_context(
    struct glx_context *gc,
    GLXDrawable draw, GLXDrawable read)
{
   Display *dpy = gc->psc->dpy;
   bool error = apple_glx_make_current_context(
       dpy,
       NULL,
       gc ? gc->driContext : NULL, draw);

   apple_glx_diagnostic("%s: error %s\n", __func__, error ? "YES" : "NO");
   if (error)
      return 1; /* GLXBadContext is the same as Success (0) */

   apple_glapi_set_dispatch();

   return Success;
}

static void
applegl_unbind_context(struct glx_context *gc)
{
   Display *dpy;
   bool error;

   /* If we don't have a context, then we have nothing to unbind */
   if (!gc)
      return;

   dpy = gc->psc->dpy;

   error = apple_glx_make_current_context(
       dpy,
       (gc != &dummyContext) ? gc->driContext : NULL,
       NULL, None);

   apple_glx_diagnostic("%s: error %s\n", __func__, error ? "YES" : "NO");
}

static void
applegl_wait_gl(struct glx_context *gc)
{
   glFinish();
}

static void
applegl_wait_x(struct glx_context *gc)
{
   Display *dpy = gc->psc->dpy;
   apple_glx_waitx(dpy, gc->driContext);
}

void *
applegl_get_proc_address(const char *symbol)
{
   return dlsym(apple_cgl_get_dl_handle(), symbol);
}

static const struct glx_context_vtable applegl_context_vtable = {
   .destroy             = applegl_destroy_context,
   .bind                = applegl_bind_context,
   .unbind              = applegl_unbind_context,
   .wait_gl             = applegl_wait_gl,
   .wait_x              = applegl_wait_x,
};

struct glx_context *
applegl_create_context(
    struct glx_screen *psc,
    struct glx_config *config,
    struct glx_context *shareList, int renderType)
{
   struct glx_context *gc;
   int errorcode;
   bool x11error;
   Display *dpy = psc->dpy;
   int screen = psc->scr;

   /* TODO: Integrate this with apple_glx_create_context and make
    * struct apple_glx_context inherit from struct glx_context. */

   if (!config)
      return NULL;

   gc = calloc(1, sizeof(*gc));
   if (gc == NULL)
      return NULL;

   if (!glx_context_init(gc, psc, config)) {
      free(gc);
      return NULL;
   }

   gc->vtable = &applegl_context_vtable;
   gc->driContext = NULL;

   /* TODO: darwin: Integrate with above to do indirect */
   if (apple_glx_create_context(&gc->driContext, dpy, screen, config,
                                shareList ? shareList->driContext : NULL,
                                &errorcode, &x11error)) {
      __glXSendError(dpy, errorcode, 0, X_GLXCreateContext, x11error);
      gc->vtable->destroy(gc);
      return NULL;
   }

   gc->currentContextTag = -1;
   gc->config = config;
   gc->isDirect = GL_TRUE;
   gc->xid = 1; /* Just something not None, so we know when to destroy
                 * it in MakeContextCurrent. */

   return gc;
}

static const struct glx_screen_vtable applegl_screen_vtable = {
   .create_context         = applegl_create_context,
   .create_context_attribs = NULL,
   .query_renderer_integer = NULL,
   .query_renderer_string  = NULL,
};

_X_HIDDEN struct glx_screen *
applegl_create_screen(int screen, struct glx_display * priv)
{
   struct glx_screen *psc;

   psc = calloc(1, sizeof *psc);
   if (psc == NULL)
      return NULL;

   glx_screen_init(psc, screen, priv);
   psc->vtable = &applegl_screen_vtable;

   return psc;
}

_X_HIDDEN int
applegl_create_display(struct glx_display *glx_dpy)
{
   if(!apple_init_glx(glx_dpy->dpy))
      return 1;

   return GLXBadContext;
}

#endif
