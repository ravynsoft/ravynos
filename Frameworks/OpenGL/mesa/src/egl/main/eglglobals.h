/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * Copyright 2009-2010 Chia-I Wu <olvaffe@gmail.com>
 * Copyright 2010-2011 LunarG, Inc.
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

#ifndef EGLGLOBALS_INCLUDED
#define EGLGLOBALS_INCLUDED

#include <stdbool.h>
#include "util/simple_mtx.h"

#include "egltypedefs.h"

enum {
   _EGL_DEBUG_BIT_CRITICAL = 0x1,
   _EGL_DEBUG_BIT_ERROR = 0x2,
   _EGL_DEBUG_BIT_WARN = 0x4,
   _EGL_DEBUG_BIT_INFO = 0x8,
};

/**
 * Global library data
 */
struct _egl_global {
   simple_mtx_t *Mutex;

   /* the list of all displays */
   _EGLDisplay *DisplayList;

   _EGLDevice *DeviceList;

   EGLint NumAtExitCalls;
   void (*AtExitCalls[10])(void);

   /*
    * Under libglvnd, the client extension string has to be split into two
    * strings, one for platform extensions, and one for everything else.
    * For a non-glvnd build create a concatenated one.
    */
#if USE_LIBGLVND
   const char *ClientOnlyExtensionString;
   const char *PlatformExtensionString;
#else
   const char *ClientExtensionString;
#endif

   EGLDEBUGPROCKHR debugCallback;
   unsigned int debugTypesEnabled;
};

extern struct _egl_global _eglGlobal;

extern void
_eglAddAtExitCall(void (*func)(void));

static inline unsigned int
DebugBitFromType(EGLenum type)
{
   assert(type >= EGL_DEBUG_MSG_CRITICAL_KHR && type <= EGL_DEBUG_MSG_INFO_KHR);
   return (1 << (type - EGL_DEBUG_MSG_CRITICAL_KHR));
}

/**
 * Perform validity checks on a generic pointer.
 */
extern EGLBoolean
_eglPointerIsDereferenceable(void *p);

#endif /* EGLGLOBALS_INCLUDED */
