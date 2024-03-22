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

#ifdef HAVE_LIBDRM
#include <xf86drm.h>
#endif
#include "util/compiler.h"
#include "util/macros.h"

#include "eglcurrent.h"
#include "egldevice.h"
#include "eglglobals.h"
#include "egllog.h"
#include "egltypedefs.h"

struct _egl_device {
   _EGLDevice *Next;

   const char *extensions;

   EGLBoolean MESA_device_software;
   EGLBoolean EXT_device_drm;
   EGLBoolean EXT_device_drm_render_node;

#ifdef HAVE_LIBDRM
   drmDevicePtr device;
#endif
};

void
_eglFiniDevice(void)
{
   _EGLDevice *dev_list, *dev;

   /* atexit function is called with global mutex locked */

   dev_list = _eglGlobal.DeviceList;

   /* The first device is static allocated SW device */
   assert(dev_list);
   assert(_eglDeviceSupports(dev_list, _EGL_DEVICE_SOFTWARE));
   dev_list = dev_list->Next;

   while (dev_list) {
      /* pop list head */
      dev = dev_list;
      dev_list = dev_list->Next;

#ifdef HAVE_LIBDRM
      assert(_eglDeviceSupports(dev, _EGL_DEVICE_DRM));
      drmFreeDevice(&dev->device);
#endif
      free(dev);
   }

   _eglGlobal.DeviceList = NULL;
}

EGLBoolean
_eglCheckDeviceHandle(EGLDeviceEXT device)
{
   _EGLDevice *cur;

   simple_mtx_lock(_eglGlobal.Mutex);
   cur = _eglGlobal.DeviceList;
   while (cur) {
      if (cur == (_EGLDevice *)device)
         break;
      cur = cur->Next;
   }
   simple_mtx_unlock(_eglGlobal.Mutex);
   return (cur != NULL);
}

_EGLDevice _eglSoftwareDevice = {
   /* TODO: EGL_EXT_device_drm support for KMS + llvmpipe */
   .extensions = "EGL_MESA_device_software EGL_EXT_device_drm_render_node",
   .MESA_device_software = EGL_TRUE,
   .EXT_device_drm_render_node = EGL_TRUE,
};

#ifdef HAVE_LIBDRM
/*
 * Negative value on error, zero if newly added, one if already in list.
 */
static int
_eglAddDRMDevice(drmDevicePtr device)
{
   _EGLDevice *dev;

   assert(device->available_nodes & ((1 << DRM_NODE_RENDER)));

   /* TODO: uncomment this assert, which is a sanity check.
    *
    * assert(device->available_nodes & ((1 << DRM_NODE_PRIMARY)));
    *
    * DRM shim does not expose a primary node, so the CI would fail if we had
    * this assert. DRM shim is being used to run shader-db. We need to
    * investigate what should be done (probably fixing DRM shim).
    */

   dev = _eglGlobal.DeviceList;

   /* The first device is always software */
   assert(dev);
   assert(_eglDeviceSupports(dev, _EGL_DEVICE_SOFTWARE));

   while (dev->Next) {
      dev = dev->Next;

      assert(_eglDeviceSupports(dev, _EGL_DEVICE_DRM));
      if (drmDevicesEqual(device, dev->device) != 0)
         return 1;
   }

   dev->Next = calloc(1, sizeof(_EGLDevice));
   if (!dev->Next)
      return -1;

   dev = dev->Next;
   dev->extensions = "EGL_EXT_device_drm EGL_EXT_device_drm_render_node";
   dev->EXT_device_drm = EGL_TRUE;
   dev->EXT_device_drm_render_node = EGL_TRUE;
   dev->device = device;

   return 0;
}
#endif

/* Finds a device in DeviceList, for the given fd.
 *
 * The fd must be of a render-capable device, as there are only render-capable
 * devices in DeviceList.
 *
 * If a software device, the fd is ignored.
 */
_EGLDevice *
_eglFindDevice(int fd, bool software)
{
   _EGLDevice *dev;

   simple_mtx_lock(_eglGlobal.Mutex);
   dev = _eglGlobal.DeviceList;

   /* The first device is always software */
   assert(dev);
   assert(_eglDeviceSupports(dev, _EGL_DEVICE_SOFTWARE));
   if (software)
      goto out;

#ifdef HAVE_LIBDRM
   drmDevicePtr device;

   if (drmGetDevice2(fd, 0, &device) != 0) {
      dev = NULL;
      goto out;
   }

   while (dev->Next) {
      dev = dev->Next;

      if (_eglDeviceSupports(dev, _EGL_DEVICE_DRM) &&
          drmDevicesEqual(device, dev->device) != 0) {
         goto cleanup_drm;
      }
   }

   /* Couldn't find an EGLDevice for the device. */
   dev = NULL;

cleanup_drm:
   drmFreeDevice(&device);

#else
   _eglLog(_EGL_FATAL,
           "Driver bug: Built without libdrm, yet looking for HW device");
   dev = NULL;
#endif

out:
   simple_mtx_unlock(_eglGlobal.Mutex);
   return dev;
}

#ifdef HAVE_LIBDRM
drmDevicePtr
_eglDeviceDrm(_EGLDevice *dev)
{
   if (!dev)
      return NULL;

   return dev->device;
}
#endif

_EGLDevice *
_eglDeviceNext(_EGLDevice *dev)
{
   if (!dev)
      return NULL;

   return dev->Next;
}

EGLBoolean
_eglDeviceSupports(_EGLDevice *dev, _EGLDeviceExtension ext)
{
   switch (ext) {
   case _EGL_DEVICE_SOFTWARE:
      return dev->MESA_device_software;
   case _EGL_DEVICE_DRM:
      return dev->EXT_device_drm;
   case _EGL_DEVICE_DRM_RENDER_NODE:
      return dev->EXT_device_drm_render_node;
   default:
      assert(0);
      return EGL_FALSE;
   };
}

EGLBoolean
_eglQueryDeviceAttribEXT(_EGLDevice *dev, EGLint attribute, EGLAttrib *value)
{
   switch (attribute) {
   default:
      _eglError(EGL_BAD_ATTRIBUTE, "eglQueryDeviceAttribEXT");
      return EGL_FALSE;
   }
}

const char *
_eglQueryDeviceStringEXT(_EGLDevice *dev, EGLint name)
{
   switch (name) {
   case EGL_EXTENSIONS:
      return dev->extensions;
   case EGL_DRM_DEVICE_FILE_EXT:
      if (!_eglDeviceSupports(dev, _EGL_DEVICE_DRM))
         break;
#ifdef HAVE_LIBDRM
      return dev->device->nodes[DRM_NODE_PRIMARY];
#else
      /* This should never happen: we don't yet support EGL_DEVICE_DRM for the
       * software device, and physical devices are only exposed when libdrm is
       * available. */
      assert(0);
      break;
#endif
   case EGL_DRM_RENDER_NODE_FILE_EXT:
      if (!_eglDeviceSupports(dev, _EGL_DEVICE_DRM_RENDER_NODE))
         break;
#ifdef HAVE_LIBDRM
      /* EGLDevice represents a software device, so no render node
       * should be advertised. */
      if (_eglDeviceSupports(dev, _EGL_DEVICE_SOFTWARE))
         return NULL;
      /* We create EGLDevice's only for render capable devices. */
      assert(dev->device->available_nodes & (1 << DRM_NODE_RENDER));
      return dev->device->nodes[DRM_NODE_RENDER];
#else
      /* Physical devices are only exposed when libdrm is available. */
      assert(_eglDeviceSupports(dev, _EGL_DEVICE_SOFTWARE));
      return NULL;
#endif
   }
   _eglError(EGL_BAD_PARAMETER, "eglQueryDeviceStringEXT");
   return NULL;
}

/* Do a fresh lookup for devices.
 *
 * Walks through the DeviceList, discarding no longer available ones
 * and adding new ones as applicable.
 *
 * Must be called with the global lock held.
 */
int
_eglDeviceRefreshList(void)
{
   ASSERTED _EGLDevice *dev;
   int count = 0;

   dev = _eglGlobal.DeviceList;

   /* The first device is always software */
   assert(dev);
   assert(_eglDeviceSupports(dev, _EGL_DEVICE_SOFTWARE));
   count++;

#ifdef HAVE_LIBDRM
   drmDevicePtr devices[64];
   int num_devs, ret;

   num_devs = drmGetDevices2(0, devices, ARRAY_SIZE(devices));
   for (int i = 0; i < num_devs; i++) {
      if (!(devices[i]->available_nodes & (1 << DRM_NODE_RENDER))) {
         drmFreeDevice(&devices[i]);
         continue;
      }

      ret = _eglAddDRMDevice(devices[i]);

      /* Device is not added - error or already present */
      if (ret != 0)
         drmFreeDevice(&devices[i]);

      if (ret >= 0)
         count++;
   }
#endif

   return count;
}

EGLBoolean
_eglQueryDevicesEXT(EGLint max_devices, _EGLDevice **devices,
                    EGLint *num_devices)
{
   _EGLDevice *dev, *devs, *swrast;
   int i = 0, num_devs;

   if ((devices && max_devices <= 0) || !num_devices)
      return _eglError(EGL_BAD_PARAMETER, "eglQueryDevicesEXT");

   simple_mtx_lock(_eglGlobal.Mutex);

   num_devs = _eglDeviceRefreshList();
   devs = _eglGlobal.DeviceList;

#ifdef GALLIUM_SOFTPIPE
   swrast = devs;
#else
   swrast = NULL;
   num_devs--;
#endif

   /* The first device is swrast. Start with the non-swrast device. */
   devs = devs->Next;

   /* bail early if we only care about the count */
   if (!devices) {
      *num_devices = num_devs;
      goto out;
   }

   *num_devices = MIN2(num_devs, max_devices);

   /* Add non-swrast devices first and add swrast last.
    *
    * By default, the user is likely to pick the first device so having the
    * software (aka least performant) one is not a good idea.
    */
   for (i = 0, dev = devs; dev && i < max_devices; i++) {
      devices[i] = dev;
      dev = dev->Next;
   }

   /* User requested the full device list, add the software device. */
   if (max_devices >= num_devs && swrast) {
      assert(_eglDeviceSupports(swrast, _EGL_DEVICE_SOFTWARE));
      devices[num_devs - 1] = swrast;
   }

out:
   simple_mtx_unlock(_eglGlobal.Mutex);

   return EGL_TRUE;
}
