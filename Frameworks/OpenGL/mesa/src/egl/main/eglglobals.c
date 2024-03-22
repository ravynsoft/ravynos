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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/simple_mtx.h"

#include "egldevice.h"
#include "egldisplay.h"
#include "eglglobals.h"

#include "util/macros.h"
#include "util/os_misc.h"

#ifdef HAVE_MINCORE
#include <unistd.h>
#include <sys/mman.h>
#endif

static simple_mtx_t _eglGlobalMutex = SIMPLE_MTX_INITIALIZER;

struct _egl_global _eglGlobal = {
   .Mutex = &_eglGlobalMutex,
   .DisplayList = NULL,
   .DeviceList = &_eglSoftwareDevice,
   .NumAtExitCalls = 2,
   .AtExitCalls =
      {
         /* default AtExitCalls, called in reverse order */
         _eglFiniDevice, /* always called last */
         _eglFiniDisplay,
      },

#if USE_LIBGLVND
   .ClientOnlyExtensionString =
#else
   .ClientExtensionString =
#endif
      "EGL_EXT_client_extensions"
#if !DETECT_OS_WINDOWS
      " EGL_EXT_device_base"
      " EGL_EXT_device_enumeration"
      " EGL_EXT_device_query"
#endif
      " EGL_EXT_platform_base"
      " EGL_KHR_client_get_all_proc_addresses"
      " EGL_KHR_debug"

#if USE_LIBGLVND
   ,
   .PlatformExtensionString =
#else
      " "
#endif

#if !DETECT_OS_WINDOWS
      "EGL_EXT_platform_device"
      " EGL_EXT_explicit_device"
#endif
#ifdef HAVE_WAYLAND_PLATFORM
      " EGL_EXT_platform_wayland"
      " EGL_KHR_platform_wayland"
#endif
#ifdef HAVE_X11_PLATFORM
      " EGL_EXT_platform_x11"
      " EGL_KHR_platform_x11"
#endif
#ifdef HAVE_XCB_PLATFORM
      " EGL_EXT_platform_xcb"
#endif
#ifdef HAVE_DRM_PLATFORM
      " EGL_MESA_platform_gbm"
      " EGL_KHR_platform_gbm"
#endif
      " EGL_MESA_platform_surfaceless"
      "",

   .debugCallback = NULL,
   .debugTypesEnabled = _EGL_DEBUG_BIT_CRITICAL | _EGL_DEBUG_BIT_ERROR,
};

static void
_eglAtExit(void)
{
   EGLint i;
   for (i = _eglGlobal.NumAtExitCalls - 1; i >= 0; i--)
      _eglGlobal.AtExitCalls[i]();
}

void
_eglAddAtExitCall(void (*func)(void))
{
   if (func) {
      static EGLBoolean registered = EGL_FALSE;

      simple_mtx_lock(_eglGlobal.Mutex);

      if (!registered) {
         atexit(_eglAtExit);
         registered = EGL_TRUE;
      }

      assert(_eglGlobal.NumAtExitCalls < ARRAY_SIZE(_eglGlobal.AtExitCalls));
      _eglGlobal.AtExitCalls[_eglGlobal.NumAtExitCalls++] = func;

      simple_mtx_unlock(_eglGlobal.Mutex);
   }
}

EGLBoolean
_eglPointerIsDereferenceable(void *p)
{
   uintptr_t addr = (uintptr_t)p;
   uint64_t page_size = 0;
   os_get_page_size(&page_size);
#ifdef HAVE_MINCORE
   unsigned char valid = 0;

   if (p == NULL)
      return EGL_FALSE;

   /* align addr to page_size */
   addr &= ~(page_size - 1);

   /* mincore expects &valid to be unsigned char* on Linux but char* on BSD:
    * we cast pointers to void, to fix type mismatch warnings in all systems
    */
   if (mincore((void *)addr, page_size, (void *)&valid) < 0) {
      return EGL_FALSE;
   }

   /* mincore() returns 0 on success, and -1 on failure.  The last parameter
    * is a vector of bytes with one entry for each page queried.  mincore
    * returns page residency information in the first bit of each byte in the
    * vector.
    *
    * Residency doesn't actually matter when determining whether a pointer is
    * dereferenceable, so the output vector can be ignored.  What matters is
    * whether mincore succeeds. See:
    *
    *   http://man7.org/linux/man-pages/man2/mincore.2.html
    */
   return EGL_TRUE;
#else
   // Without mincore(), we just assume that the first page is unmapped.
   return addr >= page_size;
#endif
}
