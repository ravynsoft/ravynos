/**************************************************************************
 *
 * Copyright 2013 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include <assert.h>
#include <string.h>
#include <stdbool.h>

#if defined(HAVE_X11_PLATFORM)
#include <X11/Xlib.h>
#else
#define XOpenDisplay(x) NULL
#define XCloseDisplay(x)
#define Display void
#endif

#include "util/simple_mtx.h"
#include "util/u_thread.h"
#include "util/u_memory.h"
#include "loader/loader.h"

#include "vid_omx_common.h"

static simple_mtx_t omx_lock = SIMPLE_MTX_INITIALIZER;
static Display *omx_display = NULL;
static struct vl_screen *omx_screen = NULL;
static unsigned omx_usecount = 0;
static const char *omx_render_node = NULL;
static int drm_fd;

struct vl_screen *omx_get_screen(void)
{
   static bool first_time = true;
   simple_mtx_lock(&omx_lock);

   if (!omx_screen) {
      if (first_time) {
         omx_render_node = debug_get_option("OMX_RENDER_NODE", NULL);
         first_time = false;
      }
      if (omx_render_node) {
         drm_fd = loader_open_device(omx_render_node);
         if (drm_fd < 0)
            goto error;

         omx_screen = vl_drm_screen_create(drm_fd);
         if (!omx_screen) {
            close(drm_fd);
            goto error;
         }
      } else {
         omx_display = XOpenDisplay(NULL);
         if (!omx_display)
            goto error;

         omx_screen = vl_dri3_screen_create(omx_display, 0);
         if (!omx_screen)
            omx_screen = vl_dri2_screen_create(omx_display, 0);
         if (!omx_screen) {
            XCloseDisplay(omx_display);
            goto error;
         }
      }
   }

   ++omx_usecount;

   simple_mtx_unlock(&omx_lock);
   return omx_screen;

error:
   simple_mtx_unlock(&omx_lock);
   return NULL;
}

void omx_put_screen(void)
{
   simple_mtx_lock(&omx_lock);
   if ((--omx_usecount) == 0) {
      omx_screen->destroy(omx_screen);
      omx_screen = NULL;

      if (omx_render_node)
         close(drm_fd);
      else
         XCloseDisplay(omx_display);
   }
   simple_mtx_unlock(&omx_lock);
}
