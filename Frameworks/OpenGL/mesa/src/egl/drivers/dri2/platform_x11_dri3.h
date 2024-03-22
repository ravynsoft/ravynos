/*
 * Copyright © 2015 Boyan Ding
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef EGL_X11_DRI3_INCLUDED
#define EGL_X11_DRI3_INCLUDED

#include "platform_x11.h"

_EGL_DRIVER_TYPECAST(dri3_egl_surface, _EGLSurface, obj)

struct dri3_egl_surface {
   struct dri2_egl_surface surf;
   struct loader_dri3_drawable loader_drawable;
};

extern const __DRIimageLoaderExtension dri3_image_loader_extension;
extern struct dri2_egl_display_vtbl dri3_x11_display_vtbl;

EGLBoolean
dri3_x11_connect(struct dri2_egl_display *dri2_dpy);

#endif
