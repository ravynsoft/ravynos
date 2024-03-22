/**************************************************************************
 *
 * Copyright 2009 Younes Manton.
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
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/*
 * Target makefiles directly refer to vl_winsys_dri.c to avoid DRI dependency
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef vl_winsys_h
#define vl_winsys_h

#ifdef HAVE_X11_PLATFORM
#include <X11/Xlib.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <unknwn.h>
#endif
#include "pipe/p_defines.h"
#include "util/format/u_formats.h"

struct pipe_screen;
struct pipe_surface;
struct pipe_loader_device;

struct vl_screen
{
   void (*destroy)(struct vl_screen *vscreen);

   struct pipe_resource *
   (*texture_from_drawable)(struct vl_screen *vscreen, void *drawable);

   struct u_rect *
   (*get_dirty_area)(struct vl_screen *vscreen);

   uint64_t
   (*get_timestamp)(struct vl_screen *vscreen, void *drawable);

   void
   (*set_next_timestamp)(struct vl_screen *vscreen, uint64_t stamp);

   void *
   (*get_private)(struct vl_screen *vscreen);

   void
   (*set_back_texture_from_output)(struct vl_screen *vscreen,
                                   struct pipe_resource *buffer,
                                   uint32_t width, uint32_t height);

   struct pipe_screen *pscreen;
   struct pipe_loader_device *dev;

   void *xcb_screen;
   uint32_t color_depth;
};

#ifdef HAVE_X11_PLATFORM
uint32_t
vl_dri2_format_for_depth(struct vl_screen *vscreen, int depth);

struct vl_screen *
vl_dri2_screen_create(Display *display, int screen);
#else
static inline struct vl_screen *
vl_dri2_screen_create(void *display, int screen) { return NULL; };
#endif

#if defined(HAVE_X11_PLATFORM) && defined(HAVE_DRI3)
struct vl_screen *
vl_dri3_screen_create(Display *display, int screen);
#else
static inline struct vl_screen *
vl_dri3_screen_create(void *display, int screen) { return NULL; };
#endif

#ifdef _WIN32
struct vl_screen *vl_win32_screen_create(LUID *adapter);
struct vl_screen *vl_win32_screen_create_from_d3d12_device(IUnknown* d3d12_device);
#else
/* Always enable the DRM vl winsys */
struct vl_screen *
vl_drm_screen_create(int fd);

#ifdef USE_XSHM
struct vl_screen *
vl_xlib_swrast_screen_create(Display *display, int screen);
static inline struct vl_screen *
vl_vgem_drm_screen_create(int fd) { return NULL; }
#else
struct vl_screen *
vl_vgem_drm_screen_create(int fd);
static inline struct vl_screen *
vl_xlib_swrast_screen_create(void *display, int screen) { return NULL; }
#endif
#endif

#endif
#ifdef __cplusplus
}
#endif