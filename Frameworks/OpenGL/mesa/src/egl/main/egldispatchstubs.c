/*
 * (C) Copyright 2016, NVIDIA CORPORATION.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * IBM AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Kyle Brenneman <kbrenneman@nvidia.com>
 */

#include "egldispatchstubs.h"
#include "g_egldispatchstubs.h"

#include <stdlib.h>
#include <string.h>

#include "eglcurrent.h"

static const __EGLapiExports *exports;

const int __EGL_DISPATCH_FUNC_COUNT = __EGL_DISPATCH_COUNT;
int __EGL_DISPATCH_FUNC_INDICES[__EGL_DISPATCH_COUNT + 1];

static int
Compare(const void *l, const void *r)
{
   const char *s = *(const char **)r;
   return strcmp(l, s);
}

static int
FindProcIndex(const char *name)
{
   const char **match =
      bsearch(name, __EGL_DISPATCH_FUNC_NAMES, __EGL_DISPATCH_COUNT,
              sizeof(const char *), Compare);

   if (match == NULL)
      return __EGL_DISPATCH_COUNT;

   return match - __EGL_DISPATCH_FUNC_NAMES;
}

void
__eglInitDispatchStubs(const __EGLapiExports *exportsTable)
{
   int i;
   exports = exportsTable;
   for (i = 0; i < __EGL_DISPATCH_FUNC_COUNT; i++) {
      __EGL_DISPATCH_FUNC_INDICES[i] = -1;
   }
}

void
__eglSetDispatchIndex(const char *name, int dispatchIndex)
{
   int index = FindProcIndex(name);
   __EGL_DISPATCH_FUNC_INDICES[index] = dispatchIndex;
}

void *
__eglDispatchFindDispatchFunction(const char *name)
{
   int index = FindProcIndex(name);
   return (void *)__EGL_DISPATCH_FUNCS[index];
}

static __eglMustCastToProperFunctionPointerType
FetchVendorFunc(__EGLvendorInfo *vendor, int index, EGLint errorCode)
{
   __eglMustCastToProperFunctionPointerType func = NULL;

   if (vendor != NULL) {
      func = exports->fetchDispatchEntry(vendor,
                                         __EGL_DISPATCH_FUNC_INDICES[index]);
   }
   if (func == NULL) {
      if (errorCode != EGL_SUCCESS) {
         // Since we have no vendor, the follow-up eglGetError() call will
         // end up using the GLVND error code. Set it here.
         if (vendor == NULL) {
            exports->setEGLError(errorCode);
         }
         _eglError(errorCode, __EGL_DISPATCH_FUNC_NAMES[index]);
      }
      return NULL;
   }

   if (!exports->setLastVendor(vendor)) {
      // Don't bother trying to set an error code in libglvnd. If
      // setLastVendor failed, then setEGLError would also fail.
      _eglError(errorCode, __EGL_DISPATCH_FUNC_NAMES[index]);
      return NULL;
   }

   return func;
}

__eglMustCastToProperFunctionPointerType
__eglDispatchFetchByCurrent(int index)
{
   __EGLvendorInfo *vendor;

   // Note: This is only used for the eglWait* functions. For those, if
   // there's no current context, then they're supposed to do nothing but
   // return success.
   exports->threadInit();
   vendor = exports->getCurrentVendor();
   return FetchVendorFunc(vendor, index, EGL_SUCCESS);
}

__eglMustCastToProperFunctionPointerType
__eglDispatchFetchByDisplay(EGLDisplay dpy, int index)
{
   __EGLvendorInfo *vendor;

   exports->threadInit();
   vendor = exports->getVendorFromDisplay(dpy);
   return FetchVendorFunc(vendor, index, EGL_BAD_DISPLAY);
}

__eglMustCastToProperFunctionPointerType
__eglDispatchFetchByDevice(EGLDeviceEXT dev, int index)
{
   __EGLvendorInfo *vendor;

   exports->threadInit();
   vendor = exports->getVendorFromDevice(dev);
   return FetchVendorFunc(vendor, index, EGL_BAD_DEVICE_EXT);
}
