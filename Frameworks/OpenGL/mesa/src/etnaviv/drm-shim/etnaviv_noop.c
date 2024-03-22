/*
 * Copyright (c) 2019 Etnaviv Project
 * Copyright (c) 2019 Zodiac Inflight Innovations
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include "drm-uapi/etnaviv_drm.h"
#include "drm-shim/drm_shim.h"
#include "util/u_debug.h"

bool drm_shim_driver_prefers_first_render_node = true;

struct etna_shim_gpu
{
   const char *name;
   const uint64_t *reg_map;
};

static const struct etna_shim_gpu gpus[] = {
   {
      .name = "GC400",
      .reg_map = (const uint64_t[]){
         [ETNAVIV_PARAM_GPU_MODEL] = 0x400,
         [ETNAVIV_PARAM_GPU_REVISION] = 0x4652,
         [ETNAVIV_PARAM_GPU_FEATURES_0] = 0xa0e9e004,
         [ETNAVIV_PARAM_GPU_FEATURES_1] = 0xe1299fff,
         [ETNAVIV_PARAM_GPU_FEATURES_2] = 0xbe13b219,
         [ETNAVIV_PARAM_GPU_FEATURES_3] = 0xce110010,
         [ETNAVIV_PARAM_GPU_FEATURES_4] = 0x8000001,
         [ETNAVIV_PARAM_GPU_FEATURES_5] = 0x20102,
         [ETNAVIV_PARAM_GPU_FEATURES_6] = 0x120000,
         [ETNAVIV_PARAM_GPU_FEATURES_7] = 0x0,
         [ETNAVIV_PARAM_GPU_STREAM_COUNT] = 0x4,
         [ETNAVIV_PARAM_GPU_REGISTER_MAX] = 0x40,
         [ETNAVIV_PARAM_GPU_THREAD_COUNT] = 0x80,
         [ETNAVIV_PARAM_GPU_VERTEX_CACHE_SIZE] = 0x8,
         [ETNAVIV_PARAM_GPU_SHADER_CORE_COUNT] = 0x1,
         [ETNAVIV_PARAM_GPU_PIXEL_PIPES] = 0x1,
         [ETNAVIV_PARAM_GPU_VERTEX_OUTPUT_BUFFER_SIZE] = 0x80,
         [ETNAVIV_PARAM_GPU_BUFFER_SIZE] = 0x0,
         [ETNAVIV_PARAM_GPU_INSTRUCTION_COUNT] = 0x100,
         [ETNAVIV_PARAM_GPU_NUM_CONSTANTS] = 0x140,
         [ETNAVIV_PARAM_GPU_NUM_VARYINGS] = 0x8,
         [ETNAVIV_PARAM_SOFTPIN_START_ADDR] = ~0ULL,
      }
   },
   {
      .name = "GC2000",
      .reg_map = (const uint64_t[]){
         [ETNAVIV_PARAM_GPU_MODEL] = 0x2000,
         [ETNAVIV_PARAM_GPU_REVISION] = 0x5108,
         [ETNAVIV_PARAM_GPU_FEATURES_0] = 0xe0296cad,
         [ETNAVIV_PARAM_GPU_FEATURES_1] = 0xc9799eff,
         [ETNAVIV_PARAM_GPU_FEATURES_2] = 0x2efbf2d9,
         [ETNAVIV_PARAM_GPU_FEATURES_3] = 0x0,
         [ETNAVIV_PARAM_GPU_FEATURES_4] = 0x0,
         [ETNAVIV_PARAM_GPU_FEATURES_5] = 0x0,
         [ETNAVIV_PARAM_GPU_FEATURES_6] = 0x0,
         [ETNAVIV_PARAM_GPU_FEATURES_7] = 0x0,
         [ETNAVIV_PARAM_GPU_STREAM_COUNT] = 0x8,
         [ETNAVIV_PARAM_GPU_REGISTER_MAX] = 0x40,
         [ETNAVIV_PARAM_GPU_THREAD_COUNT] = 0x400,
         [ETNAVIV_PARAM_GPU_VERTEX_CACHE_SIZE] = 0x10,
         [ETNAVIV_PARAM_GPU_SHADER_CORE_COUNT] = 0x4,
         [ETNAVIV_PARAM_GPU_PIXEL_PIPES] = 0x2,
         [ETNAVIV_PARAM_GPU_VERTEX_OUTPUT_BUFFER_SIZE] = 0x200,
         [ETNAVIV_PARAM_GPU_BUFFER_SIZE] = 0x0,
         [ETNAVIV_PARAM_GPU_INSTRUCTION_COUNT] = 0x200,
         [ETNAVIV_PARAM_GPU_NUM_CONSTANTS] = 0xa8,
         [ETNAVIV_PARAM_GPU_NUM_VARYINGS] = 0xb,
         [ETNAVIV_PARAM_SOFTPIN_START_ADDR] = ~0ULL,
      }
   },
   {
      .name = "GC3000",
      .reg_map = (const uint64_t[]){
         [ETNAVIV_PARAM_GPU_MODEL] = 0x3000,
         [ETNAVIV_PARAM_GPU_REVISION] = 0x5450,
         [ETNAVIV_PARAM_GPU_FEATURES_0] = 0xe0287cad,
         [ETNAVIV_PARAM_GPU_FEATURES_1] = 0xc9799efb,
         [ETNAVIV_PARAM_GPU_FEATURES_2] = 0xfefbfadb,
         [ETNAVIV_PARAM_GPU_FEATURES_3] = 0xeb9d4bbf,
         [ETNAVIV_PARAM_GPU_FEATURES_4] = 0xedffdced,
         [ETNAVIV_PARAM_GPU_FEATURES_5] = 0x930d2f47,
         [ETNAVIV_PARAM_GPU_FEATURES_6] = 0x10000133,
         [ETNAVIV_PARAM_GPU_FEATURES_7] = 0x0,
         [ETNAVIV_PARAM_GPU_STREAM_COUNT] = 0x10,
         [ETNAVIV_PARAM_GPU_REGISTER_MAX] = 0x40,
         [ETNAVIV_PARAM_GPU_THREAD_COUNT] = 0x400,
         [ETNAVIV_PARAM_GPU_VERTEX_CACHE_SIZE] = 0x10,
         [ETNAVIV_PARAM_GPU_SHADER_CORE_COUNT] = 0x4,
         [ETNAVIV_PARAM_GPU_PIXEL_PIPES] = 0x2,
         [ETNAVIV_PARAM_GPU_VERTEX_OUTPUT_BUFFER_SIZE] = 0x400,
         [ETNAVIV_PARAM_GPU_BUFFER_SIZE] = 0x0,
         [ETNAVIV_PARAM_GPU_INSTRUCTION_COUNT] = 0x100,
         [ETNAVIV_PARAM_GPU_NUM_CONSTANTS] = 0x140,
         [ETNAVIV_PARAM_GPU_NUM_VARYINGS] = 0x10,
         [ETNAVIV_PARAM_SOFTPIN_START_ADDR] = ~0ULL,
      }
   },
   {
      .name = "GC7000L",
      .reg_map = (const uint64_t[]){
         [ETNAVIV_PARAM_GPU_MODEL] = 0x7000,
         [ETNAVIV_PARAM_GPU_REVISION] = 0x6214,
         [ETNAVIV_PARAM_GPU_FEATURES_0] = 0xe0287cad,
         [ETNAVIV_PARAM_GPU_FEATURES_1] = 0xc1799eff,
         [ETNAVIV_PARAM_GPU_FEATURES_2] = 0xfefbfad9,
         [ETNAVIV_PARAM_GPU_FEATURES_3] = 0xeb9d4fbf,
         [ETNAVIV_PARAM_GPU_FEATURES_4] = 0xedfffced,
         [ETNAVIV_PARAM_GPU_FEATURES_5] = 0xdb0dafc7,
         [ETNAVIV_PARAM_GPU_FEATURES_6] = 0xbb5ac333,
         [ETNAVIV_PARAM_GPU_FEATURES_7] = 0xfc8ee200,
         [ETNAVIV_PARAM_GPU_STREAM_COUNT] = 0x10,
         [ETNAVIV_PARAM_GPU_REGISTER_MAX] = 0x40,
         [ETNAVIV_PARAM_GPU_THREAD_COUNT] = 0x400,
         [ETNAVIV_PARAM_GPU_VERTEX_CACHE_SIZE] = 0x10,
         [ETNAVIV_PARAM_GPU_SHADER_CORE_COUNT] = 0x4,
         [ETNAVIV_PARAM_GPU_PIXEL_PIPES] = 0x2,
         [ETNAVIV_PARAM_GPU_VERTEX_OUTPUT_BUFFER_SIZE] = 0x400,
         [ETNAVIV_PARAM_GPU_BUFFER_SIZE] = 0x0,
         [ETNAVIV_PARAM_GPU_INSTRUCTION_COUNT] = 0x200,
         [ETNAVIV_PARAM_GPU_NUM_CONSTANTS] = 0x140,
         [ETNAVIV_PARAM_GPU_NUM_VARYINGS] = 0x10,
         [ETNAVIV_PARAM_SOFTPIN_START_ADDR] = 0x00400000,
      }
   }
};

static const struct etna_shim_gpu *shim_gpu;

static int
etnaviv_ioctl_noop(int fd, unsigned long request, void *arg)
{
   return 0;
}

static int
etnaviv_ioctl_gem_new(int fd, unsigned long request, void *arg)
{
   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   struct drm_etnaviv_gem_new *create = arg;
   struct shim_bo *bo = calloc(1, sizeof(*bo));

   drm_shim_bo_init(bo, create->size);
   create->handle = drm_shim_bo_get_handle(shim_fd, bo);
   drm_shim_bo_put(bo);

   return 0;
}

static int
etnaviv_ioctl_gem_info(int fd, unsigned long request, void *arg)
{
   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   struct drm_etnaviv_gem_info *args = arg;
   struct shim_bo *bo = drm_shim_bo_lookup(shim_fd, args->handle);

   args->offset = drm_shim_bo_get_mmap_offset(shim_fd, bo);
   drm_shim_bo_put(bo);

   return 0;
}

static int
etnaviv_ioctl_get_param(int fd, unsigned long request, void *arg)
{
   struct drm_etnaviv_param *gp = arg;

   if (gp->param > ETNAVIV_PARAM_SOFTPIN_START_ADDR) {
      fprintf(stderr, "Unknown DRM_IOCTL_ETNAVIV_GET_PARAM %d\n", gp->param);
      return -1;
   }

   gp->value = shim_gpu->reg_map[gp->param];

   return 0;
}

static ioctl_fn_t driver_ioctls[] = {
   [DRM_ETNAVIV_GET_PARAM] = etnaviv_ioctl_get_param,
   [DRM_ETNAVIV_GEM_NEW] = etnaviv_ioctl_gem_new,
   [DRM_ETNAVIV_GEM_INFO] = etnaviv_ioctl_gem_info,
   [DRM_ETNAVIV_GEM_CPU_PREP] = etnaviv_ioctl_noop,
   [DRM_ETNAVIV_GEM_CPU_FINI] = etnaviv_ioctl_noop,
   [DRM_ETNAVIV_GEM_SUBMIT] = etnaviv_ioctl_noop,
   [DRM_ETNAVIV_WAIT_FENCE] = etnaviv_ioctl_noop,
   [DRM_ETNAVIV_GEM_USERPTR] = etnaviv_ioctl_noop,
   [DRM_ETNAVIV_GEM_WAIT] = etnaviv_ioctl_noop,
   [DRM_ETNAVIV_PM_QUERY_DOM] = etnaviv_ioctl_noop,
   [DRM_ETNAVIV_PM_QUERY_SIG] = etnaviv_ioctl_noop,
};

void
drm_shim_driver_init(void)
{
   shim_device.bus_type = DRM_BUS_PLATFORM;
   shim_device.driver_name = "etnaviv";
   shim_device.driver_ioctls = driver_ioctls;
   shim_device.driver_ioctl_count = ARRAY_SIZE(driver_ioctls);

   /* etnaviv uses the DRM version to expose features, instead of getparam. */
   shim_device.version_major = 1;
   shim_device.version_minor = 1;
   shim_device.version_patchlevel = 0;

   drm_shim_override_file("DRIVER=etnaviv\n"
         "MODALIAS=platform:etnaviv\n",
         "/sys/dev/char/%d:%d/device/uevent",
         DRM_MAJOR, render_node_minor);

   /* decide what GPU to emulate */
   const char *gpu = debug_get_option("ETNA_SHIM_GPU", "GC2000");

   for (unsigned i = 0; i < ARRAY_SIZE(gpus); i++) {
      if (strncasecmp(gpu, gpus[i].name, strlen(gpus[i].name)) == 0) {
         shim_gpu = &gpus[i];
         break;
      }
   }

   /* NOTE: keep keep default value and fallback index in sync */
   if (!shim_gpu)
      shim_gpu = &gpus[1];

   fprintf(stderr, "Using %s as shim gpu\n", shim_gpu->name);
}
