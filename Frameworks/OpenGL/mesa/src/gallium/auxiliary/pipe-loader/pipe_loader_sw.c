/**************************************************************************
 *
 * Copyright 2012 Francisco Jerez
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

#ifdef HAVE_DRISW_KMS
#include <fcntl.h>
#endif

#include "pipe_loader_priv.h"

#include "util/os_file.h"
#include "util/u_memory.h"
#include "util/u_dl.h"
#include "sw/dri/dri_sw_winsys.h"
#include "sw/kms-dri/kms_dri_sw_winsys.h"
#include "sw/null/null_sw_winsys.h"
#include "sw/wrapper/wrapper_sw_winsys.h"
#include "target-helpers/sw_helper_public.h"
#include "target-helpers/inline_debug_helper.h"
#include "frontend/drisw_api.h"
#include "frontend/sw_driver.h"
#include "frontend/sw_winsys.h"
#include "util/driconf.h"

struct pipe_loader_sw_device {
   struct pipe_loader_device base;
   const struct sw_driver_descriptor *dd;
#ifndef GALLIUM_STATIC_TARGETS
   struct util_dl_library *lib;
#endif
   struct sw_winsys *ws;
   int fd;
};

#define pipe_loader_sw_device(dev) ((struct pipe_loader_sw_device *)dev)

static const struct pipe_loader_ops pipe_loader_sw_ops;
#if defined(HAVE_DRI) && defined(HAVE_ZINK)
static const struct pipe_loader_ops pipe_loader_vk_ops;
#endif

#ifdef GALLIUM_STATIC_TARGETS
static const struct sw_driver_descriptor driver_descriptors = {
   .create_screen = sw_screen_create_vk,
   .winsys = {
#ifdef HAVE_DRI
      {
         .name = "dri",
         .create_winsys_dri = dri_create_sw_winsys,
      },
#endif
#ifdef HAVE_DRISW_KMS
      {
         .name = "kms_dri",
         .create_winsys_kms_dri = kms_dri_create_winsys,
      },
#endif
#ifndef __ANDROID__
      {
         .name = "null",
         .create_winsys = null_sw_create,
      },
      {
         .name = "wrapped",
         .create_winsys_wrapped = wrapper_sw_winsys_wrap_pipe_screen,
      },
#endif
      { 0 },
   }
};
#endif

#if defined(GALLIUM_STATIC_TARGETS) && defined(HAVE_ZINK) && defined(HAVE_DRI)
static const struct sw_driver_descriptor kopper_driver_descriptors = {
   .create_screen = sw_screen_create_zink,
   .winsys = {
      {
         .name = "dri",
         .create_winsys_dri = dri_create_sw_winsys,
      },
#ifdef HAVE_DRISW_KMS
      {
         .name = "kms_dri",
         .create_winsys_kms_dri = kms_dri_create_winsys,
      },
#endif
#ifndef __ANDROID__
      {
         .name = "null",
         .create_winsys = null_sw_create,
      },
      {
         .name = "wrapped",
         .create_winsys_wrapped = wrapper_sw_winsys_wrap_pipe_screen,
      },
#endif
      { 0 },
   }
};
#endif

static bool
pipe_loader_sw_probe_init_common(struct pipe_loader_sw_device *sdev)
{
   sdev->base.type = PIPE_LOADER_DEVICE_SOFTWARE;
   sdev->base.driver_name = "swrast";
   sdev->base.ops = &pipe_loader_sw_ops;
   sdev->fd = -1;

#ifdef GALLIUM_STATIC_TARGETS
   sdev->dd = &driver_descriptors;
   if (!sdev->dd)
      return false;
#else
   const char *search_dir = getenv("GALLIUM_PIPE_SEARCH_DIR");
   if (search_dir == NULL)
      search_dir = PIPE_SEARCH_DIR;

   sdev->lib = pipe_loader_find_module("swrast", search_dir);
   if (!sdev->lib)
      return false;

   sdev->dd = (const struct sw_driver_descriptor *)
      util_dl_get_proc_address(sdev->lib, "swrast_driver_descriptor");

   if (!sdev->dd){
      util_dl_close(sdev->lib);
      sdev->lib = NULL;
      return false;
   }
#endif

   return true;
}

#if defined(HAVE_DRI) && defined(HAVE_ZINK)
static bool
pipe_loader_vk_probe_init_common(struct pipe_loader_sw_device *sdev)
{
   sdev->base.type = PIPE_LOADER_DEVICE_PLATFORM;
   sdev->base.driver_name = "kopper";
   sdev->base.ops = &pipe_loader_vk_ops;
   sdev->fd = -1;

#ifdef GALLIUM_STATIC_TARGETS
   sdev->dd = &kopper_driver_descriptors;
   if (!sdev->dd)
      return false;
#else
   const char *search_dir = getenv("GALLIUM_PIPE_SEARCH_DIR");
   if (search_dir == NULL)
      search_dir = PIPE_SEARCH_DIR;

   sdev->lib = pipe_loader_find_module("swrast", search_dir);
   if (!sdev->lib)
      return false;

   sdev->dd = (const struct sw_driver_descriptor *)
      util_dl_get_proc_address(sdev->lib, "swrast_driver_descriptor");

   if (!sdev->dd){
      util_dl_close(sdev->lib);
      sdev->lib = NULL;
      return false;
   }
#endif

   return true;
}
#endif

static void
pipe_loader_sw_probe_teardown_common(struct pipe_loader_sw_device *sdev)
{
#ifndef GALLIUM_STATIC_TARGETS
   if (sdev->lib)
      util_dl_close(sdev->lib);
#endif
}

#ifdef HAVE_DRI
bool
pipe_loader_sw_probe_dri(struct pipe_loader_device **devs, const struct drisw_loader_funcs *drisw_lf)
{
   struct pipe_loader_sw_device *sdev = CALLOC_STRUCT(pipe_loader_sw_device);
   int i;

   if (!sdev)
      return false;

   if (!pipe_loader_sw_probe_init_common(sdev))
      goto fail;

   for (i = 0; sdev->dd->winsys[i].name; i++) {
      if (strcmp(sdev->dd->winsys[i].name, "dri") == 0) {
         sdev->ws = sdev->dd->winsys[i].create_winsys_dri(drisw_lf);
         break;
      }
   }
   if (!sdev->ws)
      goto fail;

   *devs = &sdev->base;
   return true;

fail:
   pipe_loader_sw_probe_teardown_common(sdev);
   FREE(sdev);
   return false;
}
#ifdef HAVE_ZINK
bool
pipe_loader_vk_probe_dri(struct pipe_loader_device **devs, const struct drisw_loader_funcs *drisw_lf)
{
   struct pipe_loader_sw_device *sdev = CALLOC_STRUCT(pipe_loader_sw_device);
   int i;

   if (!sdev)
      return false;

   if (!pipe_loader_vk_probe_init_common(sdev))
      goto fail;

   for (i = 0; sdev->dd->winsys[i].name; i++) {
      if (strcmp(sdev->dd->winsys[i].name, "dri") == 0) {
         sdev->ws = sdev->dd->winsys[i].create_winsys_dri(drisw_lf);
         break;
      }
   }
   if (!sdev->ws)
      goto fail;

   *devs = &sdev->base;
   return true;

fail:
   pipe_loader_sw_probe_teardown_common(sdev);
   FREE(sdev);
   return false;
}
#endif
#endif

#ifdef HAVE_DRISW_KMS
bool
pipe_loader_sw_probe_kms(struct pipe_loader_device **devs, int fd)
{
   struct pipe_loader_sw_device *sdev = CALLOC_STRUCT(pipe_loader_sw_device);
   int i;

   if (!sdev)
      return false;

   if (!pipe_loader_sw_probe_init_common(sdev))
      goto fail;

   if (fd < 0 || (sdev->fd = os_dupfd_cloexec(fd)) < 0)
      goto fail;

   for (i = 0; sdev->dd->winsys[i].name; i++) {
      if (strcmp(sdev->dd->winsys[i].name, "kms_dri") == 0) {
         sdev->ws = sdev->dd->winsys[i].create_winsys_kms_dri(sdev->fd);
         break;
      }
   }
   if (!sdev->ws)
      goto fail;

   *devs = &sdev->base;
   return true;

fail:
   pipe_loader_sw_probe_teardown_common(sdev);
   if (sdev->fd != -1)
      close(sdev->fd);
   FREE(sdev);
   return false;
}
#endif

bool
pipe_loader_sw_probe_null(struct pipe_loader_device **devs)
{
   struct pipe_loader_sw_device *sdev = CALLOC_STRUCT(pipe_loader_sw_device);
   int i;

   if (!sdev)
      return false;

   if (!pipe_loader_sw_probe_init_common(sdev))
      goto fail;

   for (i = 0; sdev->dd->winsys[i].name; i++) {
      if (strcmp(sdev->dd->winsys[i].name, "null") == 0) {
         sdev->ws = sdev->dd->winsys[i].create_winsys();
         break;
      }
   }
   if (!sdev->ws)
      goto fail;

   *devs = &sdev->base;
   return true;

fail:
   pipe_loader_sw_probe_teardown_common(sdev);
   FREE(sdev);
   return false;
}

int
pipe_loader_sw_probe(struct pipe_loader_device **devs, int ndev)
{
   int i = 1;

   if (i <= ndev) {
      if (!pipe_loader_sw_probe_null(devs)) {
         i--;
      }
   }

   return i;
}

bool
pipe_loader_sw_probe_wrapped(struct pipe_loader_device **dev,
                             struct pipe_screen *screen)
{
   struct pipe_loader_sw_device *sdev = CALLOC_STRUCT(pipe_loader_sw_device);
   int i;

   if (!sdev)
      return false;

   if (!pipe_loader_sw_probe_init_common(sdev))
      goto fail;

   for (i = 0; sdev->dd->winsys[i].name; i++) {
      if (strcmp(sdev->dd->winsys[i].name, "wrapped") == 0) {
         sdev->ws = sdev->dd->winsys[i].create_winsys_wrapped(screen);
         break;
      }
   }
   if (!sdev->ws)
      goto fail;

   *dev = &sdev->base;
   return true;

fail:
   pipe_loader_sw_probe_teardown_common(sdev);
   FREE(sdev);
   return false;
}

static void
pipe_loader_sw_release(struct pipe_loader_device **dev)
{
   UNUSED struct pipe_loader_sw_device *sdev =
      pipe_loader_sw_device(*dev);

   sdev->ws->destroy(sdev->ws);
#ifndef GALLIUM_STATIC_TARGETS
   if (sdev->lib)
      util_dl_close(sdev->lib);
#endif

#ifdef HAVE_DRISW_KMS
   if (sdev->fd != -1)
      close(sdev->fd);
#endif

   pipe_loader_base_release(dev);
}

static const struct driOptionDescription *
pipe_loader_sw_get_driconf(struct pipe_loader_device *dev, unsigned *count)
{
   *count = 0;
   return NULL;
}

#if defined(HAVE_DRI) && defined(HAVE_ZINK)
static const driOptionDescription zink_driconf[] = {
      #include "gallium/drivers/zink/driinfo_zink.h"
};

static const struct driOptionDescription *
pipe_loader_vk_get_driconf(struct pipe_loader_device *dev, unsigned *count)
{
   *count = ARRAY_SIZE(zink_driconf);
   return zink_driconf;
}
#endif

static struct pipe_screen *
pipe_loader_sw_create_screen(struct pipe_loader_device *dev,
                             const struct pipe_screen_config *config, bool sw_vk)
{
   struct pipe_loader_sw_device *sdev = pipe_loader_sw_device(dev);
   struct pipe_screen *screen;

   screen = sdev->dd->create_screen(sdev->ws, config, sw_vk);

   return screen ? debug_screen_wrap(screen) : NULL;
}

static const struct pipe_loader_ops pipe_loader_sw_ops = {
   .create_screen = pipe_loader_sw_create_screen,
   .get_driconf = pipe_loader_sw_get_driconf,
   .release = pipe_loader_sw_release
};

#if defined(HAVE_DRI) && defined(HAVE_ZINK)
static const struct pipe_loader_ops pipe_loader_vk_ops = {
   .create_screen = pipe_loader_sw_create_screen,
   .get_driconf = pipe_loader_vk_get_driconf,
   .release = pipe_loader_sw_release
};
#endif
