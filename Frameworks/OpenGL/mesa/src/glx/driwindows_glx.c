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

#include "glxclient.h"
#include "glx_error.h"
#include "dri_common.h"
#include "util/macros.h"
#include "windows/xwindowsdri.h"
#include "windows/windowsgl.h"

struct driwindows_display
{
   __GLXDRIdisplay base;
   int event_base;
};

struct driwindows_context
{
   struct glx_context base;
   windowsContext *windowsContext;
};

struct driwindows_config
{
   struct glx_config base;
   int pxfi;
};

struct driwindows_screen
{
   struct glx_screen base;
   __DRIscreen *driScreen;
   __GLXDRIscreen vtable;
   Bool copySubBuffer;
};

struct driwindows_drawable
{
   __GLXDRIdrawable base;
   windowsDrawable *windowsDrawable;
};

/**
 * GLXDRI functions
 */

static void
driwindows_destroy_context(struct glx_context *context)
{
   struct driwindows_context *pcp = (struct driwindows_context *) context;

   driReleaseDrawables(&pcp->base);

   free((char *) context->extensions);

   windows_destroy_context(pcp->windowsContext);

   free(pcp);
}

static int
driwindows_bind_context(struct glx_context *context, GLXDrawable draw, GLXDrawable read)
{
   struct driwindows_context *pcp = (struct driwindows_context *) context;
   struct driwindows_drawable *pdraw, *pread;

   pdraw = (struct driwindows_drawable *) driFetchDrawable(context, draw);
   pread = (struct driwindows_drawable *) driFetchDrawable(context, read);

   driReleaseDrawables(&pcp->base);

   if (pdraw == NULL || pread == NULL)
      return GLXBadDrawable;

   if (windows_bind_context(pcp->windowsContext,
                           pdraw->windowsDrawable, pread->windowsDrawable))
      return Success;

   return GLXBadContext;
}

static void
driwindows_unbind_context(struct glx_context *context)
{
   struct driwindows_context *pcp = (struct driwindows_context *) context;

   windows_unbind_context(pcp->windowsContext);
}

static const struct glx_context_vtable driwindows_context_vtable = {
   .destroy             = driwindows_destroy_context,
   .bind                = driwindows_bind_context,
   .unbind              = driwindows_unbind_context,
   .wait_gl             = NULL,
   .wait_x              = NULL,
};

static struct glx_context *
driwindows_create_context(struct glx_screen *base,
                          struct glx_config *config_base,
                          struct glx_context *shareList, int renderType)
{
   struct driwindows_context *pcp, *pcp_shared;
   struct driwindows_config *config = (struct driwindows_config *) config_base;
   struct driwindows_screen *psc = (struct driwindows_screen *) base;
   windowsContext *shared = NULL;

   if (!psc->base.driScreen)
      return NULL;

   /* Check the renderType value */
   if (!validate_renderType_against_config(config_base, renderType))
       return NULL;

   if (shareList) {
      /* If the shareList context is not on this renderer, we cannot possibly
       * create a context that shares with it.
       */
      if (shareList->vtable->destroy != driwindows_destroy_context) {
         return NULL;
      }

      pcp_shared = (struct driwindows_context *) shareList;
      shared = pcp_shared->windowsContext;
   }

   pcp = calloc(1, sizeof *pcp);
   if (pcp == NULL)
      return NULL;

   if (!glx_context_init(&pcp->base, &psc->base, &config->base)) {
      free(pcp);
      return NULL;
   }

   pcp->base.renderType = renderType;

   InfoMessageF("visualID %x, fbConfigID %x -> pxfi %d\n", config_base->visualID, config_base->fbconfigID, config->pxfi);

   pcp->windowsContext = windows_create_context(config->pxfi, shared);

   if (!pcp->windowsContext) {
      free(pcp);
      return NULL;
   }

   pcp->base.vtable = &driwindows_context_vtable;

   return &pcp->base;
}

static struct glx_context *
driwindows_create_context_attribs(struct glx_screen *base,
                                  struct glx_config *config_base,
                                  struct glx_context *shareList,
                                  unsigned num_attribs,
                                  const uint32_t *attribs,
                                  unsigned *error)
{
   struct driwindows_context *pcp, *pcp_shared;
   struct driwindows_config *config = (struct driwindows_config *) config_base;
   struct driwindows_screen *psc = (struct driwindows_screen *) base;
   windowsContext *shared = NULL;

   int i;
   uint32_t renderType = GLX_RGBA_TYPE;

   /* Extract renderType from attribs */
   for (i = 0; i < num_attribs; i++) {
      switch (attribs[i * 2]) {
      case GLX_RENDER_TYPE:
         renderType = attribs[i * 2 + 1];
         break;
      }
   }

   /*
     Perhaps we should map GLX tokens to WGL tokens, but they appear to have
     identical values, so far
   */

   if (!psc->base.driScreen || !config_base)
      return NULL;

   /* Check the renderType value */
   if (!validate_renderType_against_config(config_base, renderType)) {
       return NULL;
   }

   if (shareList) {
      /* If the shareList context is not on this renderer, we cannot possibly
       * create a context that shares with it.
       */
      if (shareList->vtable->destroy != driwindows_destroy_context) {
         return NULL;
      }

      pcp_shared = (struct driwindows_context *) shareList;
      shared = pcp_shared->windowsContext;
   }

   pcp = calloc(1, sizeof *pcp);
   if (pcp == NULL)
      return NULL;

   if (!glx_context_init(&pcp->base, &psc->base, &config->base)) {
      free(pcp);
      return NULL;
   }

   pcp->base.renderType = renderType;

   InfoMessageF("visualID %x, fbConfigID %x -> pxfi %d\n", config_base->visualID, config_base->fbconfigID, config->pxfi);

   pcp->windowsContext = windows_create_context_attribs(config->pxfi,
                                                      shared,
                                                      (const int *)attribs);
   if (pcp->windowsContext == NULL) {
      free(pcp);
      return NULL;
   }

   pcp->base.vtable = &driwindows_context_vtable;

   return &pcp->base;
}

static void
driwindowsDestroyDrawable(__GLXDRIdrawable * pdraw)
{
   struct driwindows_drawable *pdp = (struct driwindows_drawable *) pdraw;

   windows_destroy_drawable(pdp->windowsDrawable);

   free(pdp);
}

static __GLXDRIdrawable *
driwindowsCreateDrawable(struct glx_screen *base, XID xDrawable,
                         GLXDrawable drawable, int type,
                         struct glx_config *modes)
{
   struct driwindows_drawable *pdp;
   struct driwindows_screen *psc = (struct driwindows_screen *) base;

   pdp = calloc(1, sizeof(*pdp));
   if (!pdp)
      return NULL;

   pdp->base.xDrawable = xDrawable;
   pdp->base.drawable = drawable;
   pdp->base.psc = &psc->base;

   /*
      By this stage, the X drawable already exists, but the GLX drawable may
      not.

      Query the server with the XID to find the correct HWND, HPBUFFERARB or
      HBITMAP
   */

   unsigned int type;
   void *handle;

   if (!XWindowsDRIQueryDrawable(psc->base.dpy, base->scr, drawable, &type, &handle))
   {
      free(pdp);
      return NULL;
   }

   /* No handle found is a failure */
   if (!handle) {
      free(pdp);
      return NULL;
   }

   /* Create a new drawable */
   pdp->windowsDrawable = windows_create_drawable(type, handle);

   if (!pdp->windowsDrawable) {
      free(pdp);
      return NULL;
   }

   pdp->base.destroyDrawable = driwindowsDestroyDrawable;

   return &pdp->base;
}

static int64_t
driwindowsSwapBuffers(__GLXDRIdrawable * pdraw,
                 int64_t target_msc, int64_t divisor, int64_t remainder,
                 Bool flush)
{
   struct driwindows_drawable *pdp = (struct driwindows_drawable *) pdraw;

   (void) target_msc;
   (void) divisor;
   (void) remainder;

   if (flush) {
      glFlush();
   }

   windows_swap_buffers(pdp->windowsDrawable);

   return 0;
}

static void
driwindowsCopySubBuffer(__GLXDRIdrawable * pdraw,
                   int x, int y, int width, int height, Bool flush)
{
   struct driwindows_drawable *pdp = (struct driwindows_drawable *) pdraw;

   if (flush) {
      glFlush();
   }

   windows_copy_subbuffer(pdp->windowsDrawable, x, y, width, height);
}

static void
driwindowsDestroyScreen(struct glx_screen *base)
{
   struct driwindows_screen *psc = (struct driwindows_screen *) base;

   /* Free the direct rendering per screen data */
   psc->driScreen = NULL;
   free(psc);
}

static const struct glx_screen_vtable driwindows_screen_vtable = {
   .create_context         = driwindows_create_context,
   .create_context_attribs = driwindows_create_context_attribs,
   .query_renderer_integer = NULL,
   .query_renderer_string  = NULL,
};

static Bool
driwindowsBindExtensions(struct driwindows_screen *psc)
{
   Bool result = 1;

   const struct
   {
      char *wglext;
      char *glxext;
      Bool mandatory;
   } extensionMap[] = {
      { "WGL_ARB_make_current_read", "GLX_SGI_make_current_read", 0 },
      { "WGL_EXT_swap_control", "GLX_SGI_swap_control", 0 },
      { "WGL_EXT_swap_control", "GLX_MESA_swap_control", 0 },
//      { "WGL_ARB_render_texture", "GLX_EXT_texture_from_pixmap", 0 },
// Not exactly equivalent, needs some more glue to be written
      { "WGL_ARB_pbuffer", "GLX_SGIX_pbuffer", 1 },
      { "WGL_ARB_multisample", "GLX_ARB_multisample", 1 },
      { "WGL_ARB_multisample", "GLX_SGIS_multisample", 1 },
      { "WGL_ARB_create_context", "GLX_ARB_create_context", 0 },
      { "WGL_ARB_create_context_profile", "GLX_ARB_create_context_profile", 0 },
      { "WGL_ARB_create_context_robustness", "GLX_ARB_create_context_robustness", 0 },
      { "WGL_EXT_create_context_es2_profile", "GLX_EXT_create_context_es2_profile", 0 },
   };

   char *wgl_extensions;
   char *gl_extensions;
   int i;

   windows_extensions(&gl_extensions, &wgl_extensions);

   for (i = 0; i < ARRAY_SIZE(extensionMap); i++) {
      if (strstr(wgl_extensions, extensionMap[i].wglext)) {
          __glXEnableDirectExtension(&psc->base, extensionMap[i].glxext);
          InfoMessageF("enabled %s\n", extensionMap[i].glxext);
      }
      else if (extensionMap[i].mandatory) {
         ErrorMessageF("required WGL extension %s is missing\n", extensionMap[i].wglext);
         result = 0;
      }
   }

   /*
       Because it pre-dates WGL_EXT_extensions_string, GL_WIN_swap_hint might
       only be in GL_EXTENSIONS
   */
   if (strstr(gl_extensions, "GL_WIN_swap_hint")) {
      psc->copySubBuffer = 1;
      __glXEnableDirectExtension(&psc->base, "GLX_MESA_copy_sub_buffer");
      InfoMessageF("enabled GLX_MESA_copy_sub_buffer\n");
   }

   free(gl_extensions);
   free(wgl_extensions);

   return result;
}

static struct glx_config *
driwindowsMapConfigs(struct glx_display *priv, int screen, struct glx_config *configs, struct glx_config *fbconfigs)
{
   struct glx_config head, *tail, *m;

   tail = &head;
   head.next = NULL;

   for (m = configs; m; m = m->next) {
      int fbconfigID = GLX_DONT_CARE;
      if (fbconfigs) {
         /*
           visuals have fbconfigID of GLX_DONT_CARE, so search for a fbconfig
           with matching visualID and get the fbconfigID from there
         */
         struct glx_config *f;
         for (f = fbconfigs; f; f = f->next) {
            if (f->visualID == m->visualID)
               fbconfigID = f->fbconfigID;
         }
      }
      else {
         fbconfigID = m->fbconfigID;
      }

      int pxfi;
      XWindowsDRIFBConfigToPixelFormat(priv->dpy, screen, fbconfigID, &pxfi);
      if (pxfi == 0)
         continue;

      struct driwindows_config *config = malloc(sizeof(*config));

      tail->next = &config->base;
      if (tail->next == NULL)
         continue;

      config->base = *m;
      config->pxfi = pxfi;

      tail = tail->next;
   }

   return head.next;
}

static struct glx_screen *
driwindowsCreateScreen(int screen, struct glx_display *priv)
{
   __GLXDRIscreen *psp;
   struct driwindows_screen *psc;
   struct glx_config *configs = NULL, *visuals = NULL;
   int directCapable;

   psc = calloc(1, sizeof *psc);
   if (psc == NULL)
      return NULL;

   if (!glx_screen_init(&psc->base, screen, priv)) {
      free(psc);
      return NULL;
   }

   if (!XWindowsDRIQueryDirectRenderingCapable(psc->base.dpy, screen, &directCapable) ||
       !directCapable) {
      ErrorMessageF("Screen is not Windows-DRI capable\n");
      goto handle_error;
   }

   /* discover native supported extensions */
   if (!driwindowsBindExtensions(psc)) {
      goto handle_error;
   }

   /* Augment configs with pxfi information */
   configs = driwindowsMapConfigs(priv, screen, psc->base.configs, NULL);
   visuals = driwindowsMapConfigs(priv, screen, psc->base.visuals, configs);

   if (!configs || !visuals) {
       ErrorMessageF("No fbConfigs or visuals found\n");
       goto handle_error;
   }

   glx_config_destroy_list(psc->base.configs);
   psc->base.configs = configs;
   glx_config_destroy_list(psc->base.visuals);
   psc->base.visuals = visuals;

   psc->base.vtable = &driwindows_screen_vtable;
   psp = &psc->vtable;
   psc->base.driScreen = psp;
   psp->destroyScreen = driwindowsDestroyScreen;
   psp->createDrawable = driwindowsCreateDrawable;
   psp->swapBuffers = driwindowsSwapBuffers;

   if (psc->copySubBuffer)
      psp->copySubBuffer = driwindowsCopySubBuffer;

   return &psc->base;

handle_error:
   glx_screen_cleanup(&psc->base);

   return NULL;
}

/* Called from __glXFreeDisplayPrivate.
 */
static void
driwindowsDestroyDisplay(__GLXDRIdisplay * dpy)
{
   free(dpy);
}

/*
 * Allocate, initialize and return a  __GLXDRIdisplay object.
 * This is called from __glXInitialize() when we are given a new
 * display pointer.
 */
_X_HIDDEN __GLXDRIdisplay *
driwindowsCreateDisplay(Display * dpy)
{
   struct driwindows_display *pdpyp;

   int eventBase, errorBase;
   int major, minor, patch;

   /* Verify server has Windows-DRI extension */
   if (!XWindowsDRIQueryExtension(dpy, &eventBase, &errorBase)) {
      ErrorMessageF("Windows-DRI extension not available\n");
      return NULL;
   }

   if (!XWindowsDRIQueryVersion(dpy, &major, &minor, &patch)) {
      ErrorMessageF("Fetching Windows-DRI extension version failed\n");
      return NULL;
   }

   if (!windows_check_renderer()) {
      ErrorMessageF("Windows-DRI extension disabled for GDI Generic renderer\n");
      return NULL;
   }

   pdpyp = malloc(sizeof *pdpyp);
   if (pdpyp == NULL)
      return NULL;

   pdpyp->base.destroyDisplay = driwindowsDestroyDisplay;
   pdpyp->base.createScreen = driwindowsCreateScreen;

   pdpyp->event_base = eventBase;

   return &pdpyp->base;
}
