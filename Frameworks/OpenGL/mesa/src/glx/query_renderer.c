/*
 * Copyright © 2013 Intel Corporation
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
#include "glxclient.h"
#include "glx_error.h"

#include <assert.h>

static Bool
__glXQueryRendererInteger(struct glx_screen *psc, int attribute,
                          unsigned int *value)
{
   unsigned int values_for_query = 0;
   unsigned int buffer[32];
   int err;

   /* This probably means the caller is trying to use an extension function
    * that isn't actually supported.
    */
   if (psc->vtable->query_renderer_integer == NULL)
      return False;

   switch (attribute) {
   case GLX_RENDERER_VENDOR_ID_MESA:
   case GLX_RENDERER_DEVICE_ID_MESA:
      values_for_query = 1;
      break;
   case GLX_RENDERER_VERSION_MESA:
      values_for_query = 3;
      break;
   case GLX_RENDERER_ACCELERATED_MESA:
   case GLX_RENDERER_VIDEO_MEMORY_MESA:
   case GLX_RENDERER_UNIFIED_MEMORY_ARCHITECTURE_MESA:
   case GLX_RENDERER_PREFERRED_PROFILE_MESA:
      values_for_query = 1;
      break;
   case GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA:
   case GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA:
   case GLX_RENDERER_OPENGL_ES_PROFILE_VERSION_MESA:
   case GLX_RENDERER_OPENGL_ES2_PROFILE_VERSION_MESA:
      values_for_query = 2;
      break;

   default:
      return False;
   }

   err = psc->vtable->query_renderer_integer(psc, attribute, buffer);

   /* If there was no error, copy the correct number of values from the driver
    * out to the application.
    */
   if (err == 0)
      memcpy(value, buffer, sizeof(unsigned int) * values_for_query);

   return err == 0;
}

_X_HIDDEN Bool
glXQueryRendererIntegerMESA(Display *dpy, int screen,
                            int renderer, int attribute,
                            unsigned int *value)
{
   struct glx_screen *psc;

   if (dpy == NULL)
      return False;

   /* This probably means the caller passed the wrong display pointer or
    * screen number.
    */
   psc = GetGLXScreenConfigs(dpy, screen);
   if (psc == NULL)
      return False;

   /* Right now only a single renderer per display / screen combination is
    * supported.
    */
   if (renderer != 0)
      return False;

   return __glXQueryRendererInteger(psc, attribute, value);
}

_X_HIDDEN Bool
glXQueryCurrentRendererIntegerMESA(int attribute, unsigned int *value)
{
   struct glx_context *gc = __glXGetCurrentContext();

   if (gc == &dummyContext)
      return False;

   return __glXQueryRendererInteger(gc->psc, attribute, value);
}

static const char *
__glXQueryRendererString(struct glx_screen *psc, int attribute)
{
   const char *value;
   int err;

   /* This probably means the caller is trying to use an extension function
    * that isn't actually supported.
    */
   if (psc->vtable->query_renderer_integer == NULL)
      return NULL;

   switch (attribute) {
   case GLX_RENDERER_VENDOR_ID_MESA:
   case GLX_RENDERER_DEVICE_ID_MESA:
      break;
   default:
      return NULL;
   }

   err = psc->vtable->query_renderer_string(psc, attribute, &value);
   return (err == 0) ? value : NULL;
}

_X_HIDDEN const char *
glXQueryRendererStringMESA(Display *dpy, int screen,
                           int renderer, int attribute)
{
   struct glx_screen *psc;

   if (dpy == NULL)
      return NULL;

   /* This probably means the caller passed the wrong display pointer or
    * screen number.
    */
   psc = GetGLXScreenConfigs(dpy, screen);
   if (psc == NULL)
      return NULL;

   /* Right now only a single renderer per display / screen combination is
    * supported.
    */
   if (renderer != 0)
      return NULL;

   return __glXQueryRendererString(psc, attribute);
}

_X_HIDDEN const char *
glXQueryCurrentRendererStringMESA(int attribute)
{
   struct glx_context *gc = __glXGetCurrentContext();

   if (gc == &dummyContext)
      return NULL;

   return __glXQueryRendererString(gc->psc, attribute);
}
