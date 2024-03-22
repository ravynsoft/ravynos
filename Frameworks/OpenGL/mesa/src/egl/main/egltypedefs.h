/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * Copyright 2009-2010 Chia-I Wu <olvaffe@gmail.com>
 * Copyright 2010 LunarG, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef EGLTYPEDEFS_INCLUDED
#define EGLTYPEDEFS_INCLUDED

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_angle.h>
#include <EGL/eglmesaext.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _egl_array _EGLArray;

typedef struct _egl_config _EGLConfig;

typedef struct _egl_context _EGLContext;

typedef struct _egl_device _EGLDevice;

typedef struct _egl_display _EGLDisplay;

typedef struct _egl_driver _EGLDriver;

typedef struct _egl_extensions _EGLExtensions;

typedef struct _egl_image _EGLImage;

typedef struct _egl_image_attribs _EGLImageAttribs;

typedef struct _egl_mode _EGLMode;

typedef struct _egl_resource _EGLResource;

typedef struct _egl_screen _EGLScreen;

typedef struct _egl_surface _EGLSurface;

typedef struct _egl_sync _EGLSync;

typedef struct _egl_thread_info _EGLThreadInfo;

#ifdef __cplusplus
}
#endif

#endif /* EGLTYPEDEFS_INCLUDED */
