/**************************************************************************
 *
 * Copyright 2015, 2018 Collabora
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

#ifndef EGLDEVICE_INCLUDED
#define EGLDEVICE_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#ifdef HAVE_LIBDRM
#include <xf86drm.h>
#endif

#include "egltypedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern _EGLDevice _eglSoftwareDevice;

void
_eglFiniDevice(void);

EGLBoolean
_eglCheckDeviceHandle(EGLDeviceEXT device);

static inline _EGLDevice *
_eglLookupDevice(EGLDeviceEXT device)
{
   _EGLDevice *dev = (_EGLDevice *)device;
   if (!_eglCheckDeviceHandle(device))
      dev = NULL;
   return dev;
}

_EGLDevice *
_eglFindDevice(int fd, bool software);

enum _egl_device_extension {
   _EGL_DEVICE_SOFTWARE,
   _EGL_DEVICE_DRM,
   _EGL_DEVICE_DRM_RENDER_NODE,
};

typedef enum _egl_device_extension _EGLDeviceExtension;

#ifdef HAVE_LIBDRM
drmDevicePtr
_eglDeviceDrm(_EGLDevice *dev);
#else
#define _eglDeviceDrm(dev) NULL
#endif

_EGLDevice *
_eglDeviceNext(_EGLDevice *dev);

EGLBoolean
_eglDeviceSupports(_EGLDevice *dev, _EGLDeviceExtension ext);

int
_eglDeviceRefreshList(void);

EGLBoolean
_eglQueryDeviceAttribEXT(_EGLDevice *dev, EGLint attribute, EGLAttrib *value);

const char *
_eglQueryDeviceStringEXT(_EGLDevice *dev, EGLint name);

EGLBoolean
_eglQueryDevicesEXT(EGLint max_devices, _EGLDevice **devices,
                    EGLint *num_devices);

#ifdef __cplusplus
}
#endif

#endif /* EGLDEVICE_INCLUDED */
