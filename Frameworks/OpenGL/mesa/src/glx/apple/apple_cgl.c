/*
 Copyright (c) 2008 Apple Inc.
 
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
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "apple_cgl.h"
#include "apple_glx.h"

#ifndef OPENGL_FRAMEWORK_PATH
#define OPENGL_FRAMEWORK_PATH "/System/Library/Frameworks/OpenGL.framework/Versions/A/OpenGL"
#endif

static void *dl_handle = NULL;

struct apple_cgl_api apple_cgl;

static bool initialized = false;

static void *
sym(void *h, const char *name)
{
   void *s;

   s = dlsym(h, name);

   if (NULL == s) {
      fprintf(stderr, "error: %s\n", dlerror());
      abort();
   }

   return s;
}

void
apple_cgl_init(void)
{
   void *h;
   const char *opengl_framework_path;

   if (initialized)
      return;

   opengl_framework_path = getenv("OPENGL_FRAMEWORK_PATH");
   if (!opengl_framework_path) {
      opengl_framework_path = OPENGL_FRAMEWORK_PATH;
   }

   (void) dlerror();            /*drain dlerror */
   h = dlopen(opengl_framework_path, RTLD_NOW);

   if (NULL == h) {
      fprintf(stderr, "error: unable to dlopen %s : %s\n",
              opengl_framework_path, dlerror());
      abort();
   }

   dl_handle = h;

   apple_cgl.get_version = sym(h, "CGLGetVersion");

   apple_cgl.get_version(&apple_cgl.version_major, &apple_cgl.version_minor);

   apple_glx_diagnostic("CGL major %d minor %d\n", apple_cgl.version_major, apple_cgl.version_minor);

   if (1 != apple_cgl.version_major) {
      fprintf(stderr, "WARNING: the CGL major version has changed!\n"
              "libGL may be incompatible!\n");
   }

   apple_cgl.choose_pixel_format = sym(h, "CGLChoosePixelFormat");
   apple_cgl.destroy_pixel_format = sym(h, "CGLDestroyPixelFormat");

   apple_cgl.clear_drawable = sym(h, "CGLClearDrawable");
   apple_cgl.flush_drawable = sym(h, "CGLFlushDrawable");

   apple_cgl.create_context = sym(h, "CGLCreateContext");
   apple_cgl.destroy_context = sym(h, "CGLDestroyContext");

   apple_cgl.set_current_context = sym(h, "CGLSetCurrentContext");
   apple_cgl.get_current_context = sym(h, "CGLGetCurrentContext");
   apple_cgl.error_string = sym(h, "CGLErrorString");

   apple_cgl.set_off_screen = sym(h, "CGLSetOffScreen");

   apple_cgl.copy_context = sym(h, "CGLCopyContext");

   apple_cgl.create_pbuffer = sym(h, "CGLCreatePBuffer");
   apple_cgl.destroy_pbuffer = sym(h, "CGLDestroyPBuffer");
   apple_cgl.set_pbuffer = sym(h, "CGLSetPBuffer");

   initialized = true;
}

void *
apple_cgl_get_dl_handle(void)
{
   return dl_handle;
}
