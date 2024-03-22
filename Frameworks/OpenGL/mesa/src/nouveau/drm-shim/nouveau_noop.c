/*
 * Copyright © 2021 Ilia Mirkin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "drm-uapi/nouveau_drm.h"
#include "nouveau/nvif/ioctl.h"
#include "nouveau/nvif/cl0080.h"
#include "drm-shim/drm_shim.h"
#include "util//u_math.h"

#include "../../gallium/drivers/nouveau/nv_object.xml.h"
bool drm_shim_driver_prefers_first_render_node = true;

struct nouveau_device {
   uint64_t next_offset;
};

static struct nouveau_device nouveau = {
   .next_offset = 0x1000,
};

struct nouveau_shim_bo {
   struct shim_bo base;
   uint64_t offset;
};

static struct nouveau_shim_bo *
nouveau_shim_bo(struct shim_bo *bo)
{
   return (struct nouveau_shim_bo *)bo;
}

struct nouveau_device_info {
   uint32_t chip_id;
};

static struct nouveau_device_info device_info;

static int
nouveau_ioctl_noop(int fd, unsigned long request, void *arg)
{
   return 0;
}

static int
nouveau_ioctl_gem_new(int fd, unsigned long request, void *arg)
{
   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   struct drm_nouveau_gem_new *create = arg;
   struct nouveau_shim_bo *bo = calloc(1, sizeof(*bo));

   drm_shim_bo_init(&bo->base, create->info.size);

   assert(ULONG_MAX - nouveau.next_offset > create->info.size);

   create->info.handle = drm_shim_bo_get_handle(shim_fd, &bo->base);
   create->info.map_handle = drm_shim_bo_get_mmap_offset(shim_fd, &bo->base);

   if (create->align != 0)
      nouveau.next_offset = align64(nouveau.next_offset, create->align);
   create->info.offset = nouveau.next_offset;
   nouveau.next_offset += create->info.size;

   bo->offset = create->info.offset;

   drm_shim_bo_put(&bo->base);

   return 0;
}

static int
nouveau_ioctl_gem_info(int fd, unsigned long request, void *arg)
{
   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   struct drm_nouveau_gem_info *info = arg;
   struct nouveau_shim_bo *bo =
      nouveau_shim_bo(drm_shim_bo_lookup(shim_fd, info->handle));
   info->map_handle = drm_shim_bo_get_mmap_offset(shim_fd, &bo->base);
   info->offset = bo->offset;
   info->size = bo->base.size;

   drm_shim_bo_put(&bo->base);

   return 0;
}

static int
nouveau_ioctl_gem_pushbuf(int fd, unsigned long request, void *arg)
{
   struct drm_nouveau_gem_pushbuf *submit = arg;
   submit->vram_available = 3ULL << 30;
   submit->gart_available = 1ULL << 40;
   return 0;
}

static int
nouveau_ioctl_channel_alloc(int fd, unsigned long request, void *arg)
{
   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   struct drm_nouveau_channel_alloc *alloc = arg;
   if (device_info.chip_id == 0x50 || device_info.chip_id >= 0x80)
      alloc->pushbuf_domains = NOUVEAU_GEM_DOMAIN_VRAM | NOUVEAU_GEM_DOMAIN_GART;
   else
      alloc->pushbuf_domains = NOUVEAU_GEM_DOMAIN_GART;

   /* NOTE: this will get leaked since we don't handle the channel
    * free. However only one channel is created per screen, so impact should
    * be limited. */
   struct nouveau_shim_bo *notify = calloc(1, sizeof(*notify));
   drm_shim_bo_init(&notify->base, 0x1000);
   notify->offset = nouveau.next_offset;
   nouveau.next_offset += 0x1000;
   alloc->notifier_handle = drm_shim_bo_get_handle(shim_fd, &notify->base);

   drm_shim_bo_put(&notify->base);

   return 0;
}

static int
nouveau_ioctl_get_param(int fd, unsigned long request, void *arg)
{
   struct drm_nouveau_getparam *gp = arg;

   switch (gp->param) {
   case NOUVEAU_GETPARAM_CHIPSET_ID:
      gp->value = device_info.chip_id;
      return 0;
   case NOUVEAU_GETPARAM_PCI_VENDOR:
      gp->value = 0x10de;
      return 0;
   case NOUVEAU_GETPARAM_PCI_DEVICE:
      gp->value = 0x1004;
      return 0;
   case NOUVEAU_GETPARAM_BUS_TYPE:
      gp->value = 2 /* NV_PCIE */;
      return 0;
   case NOUVEAU_GETPARAM_FB_SIZE:
      gp->value = 3ULL << 30;
      return 0;
   case NOUVEAU_GETPARAM_AGP_SIZE:
      gp->value = 1ULL << 40;
      return 0;
   case NOUVEAU_GETPARAM_PTIMER_TIME:
      gp->value = 0;
      return 0;
   case NOUVEAU_GETPARAM_HAS_BO_USAGE:
      gp->value = 1;
      return 0;
   case NOUVEAU_GETPARAM_GRAPH_UNITS:
      gp->value = 0x01000101;
      return 0;
   default:
      fprintf(stderr, "Unknown DRM_IOCTL_NOUVEAU_GETPARAM %llu\n",
              (long long unsigned)gp->param);
      return -1;
   }
}

static int
nouveau_ioctl_nvif(int fd, unsigned long request, void *arg)
{
   struct {
      struct nvif_ioctl_v0 ioctl;
   } *args = arg;

   switch (args->ioctl.type) {
   case NVIF_IOCTL_V0_MTHD: {
      struct {
         struct nvif_ioctl_v0 ioctl;
         struct nvif_ioctl_mthd_v0 mthd;
      } *mthd = (void *)args;
      switch (mthd->mthd.method) {
      case NV_DEVICE_V0_INFO: {
         struct nv_device_info_v0 *info = (void *)&mthd->mthd.data;
         info->chipset = device_info.chip_id;
         info->platform = NV_DEVICE_INFO_V0_PCIE;
         break;
      }
      default:
         break;
      }
      break;
   }
   case NVIF_IOCTL_V0_SCLASS: {
      struct {
         struct nvif_ioctl_v0 ioctl;
         struct nvif_ioctl_sclass_v0 sclass;
      } *sclass = (void *)args;

      if (sclass->sclass.count == 0) {
         sclass->sclass.count = device_info.chip_id >= 0xe0 ? 4 : 3;
         return 0;
      }
      int idx = 0;
      /* m2mf */
      switch (device_info.chip_id & ~0xf) {
      case 0x170:
      case 0x160:
      case 0x140:
      case 0x130:
      case 0x120:
      case 0x110:
      case 0x100:
      case 0xf0:
         sclass->sclass.oclass[idx].oclass = NVF0_P2MF_CLASS;
         break;
      case 0xe0:
         sclass->sclass.oclass[idx].oclass = NVE4_P2MF_CLASS;
         break;
      default:
         sclass->sclass.oclass[idx].oclass = NVC0_M2MF_CLASS;
         break;
      }
      sclass->sclass.oclass[idx].minver = -1;
      sclass->sclass.oclass[idx].maxver = -1;
      idx++;
      if (device_info.chip_id >= 0xe0) {
         switch (device_info.chip_id & ~0xf) {
         case 0x170:
            sclass->sclass.oclass[idx].oclass = AMPERE_DMA_COPY_A;
            break;
         case 0x160:
            sclass->sclass.oclass[idx].oclass = TURING_DMA_COPY_A;
            break;
         case 0x140:
            sclass->sclass.oclass[idx].oclass = VOLTA_DMA_COPY_A;
            break;
         case 0x130:
            sclass->sclass.oclass[idx].oclass = PASCAL_DMA_COPY_A;
            break;
         case 0x120:
         case 0x110:
            sclass->sclass.oclass[idx].oclass = MAXWELL_DMA_COPY_A;
            break;
         case 0x100:
         case 0xf0:
         case 0xe0:
            sclass->sclass.oclass[idx].oclass = KEPLER_DMA_COPY_A;
            break;
         }
         sclass->sclass.oclass[idx].minver = -1;
         sclass->sclass.oclass[idx].maxver = -1;
         idx++;
      }
      /* 2d */
      if (device_info.chip_id >= 0x50) {
         if (device_info.chip_id <= 0xa0)
            sclass->sclass.oclass[idx].oclass = NV50_2D_CLASS;
         else
            sclass->sclass.oclass[idx].oclass = NVC0_2D_CLASS;

         sclass->sclass.oclass[idx].minver = -1;
         sclass->sclass.oclass[idx].maxver = -1;
         idx++;
      }
      /* 3d */
      switch (device_info.chip_id & ~0xf) {
      case 0x170:
         sclass->sclass.oclass[idx].oclass = GA102_3D_CLASS;
         break;
      case 0x160:
         sclass->sclass.oclass[idx].oclass = TU102_3D_CLASS;
         break;
      case 0x140:
         sclass->sclass.oclass[idx].oclass = GV100_3D_CLASS;
         break;
      case 0x130:
         switch (device_info.chip_id) {
         case 0x130:
         case 0x13b:
            sclass->sclass.oclass[idx].oclass = GP100_3D_CLASS;
            break;
         default:
            sclass->sclass.oclass[idx].oclass = GP102_3D_CLASS;
            break;
         }
         break;
      case 0x120:
         sclass->sclass.oclass[idx].oclass = GM200_3D_CLASS;
         break;
      case 0x110:
         sclass->sclass.oclass[idx].oclass = GM107_3D_CLASS;
         break;
      case 0x100:
      case 0xf0:
         sclass->sclass.oclass[idx].oclass = NVF0_3D_CLASS;
         break;
      case 0xe0:
         switch (device_info.chip_id) {
         case 0xea:
            sclass->sclass.oclass[idx].oclass = NVEA_3D_CLASS;
            break;
         default:
            sclass->sclass.oclass[idx].oclass = NVE4_3D_CLASS;
            break;
         }
         break;
      case 0xd0:
         sclass->sclass.oclass[idx].oclass = NVC8_3D_CLASS;
         break;
      default:
      case 0xc0:
         switch (device_info.chip_id) {
         case 0xc8:
            sclass->sclass.oclass[idx].oclass = NVC8_3D_CLASS;
            break;
         case 0xc1:
            sclass->sclass.oclass[idx].oclass = NVC1_3D_CLASS;
            break;
         default:
            sclass->sclass.oclass[idx].oclass = NVC0_3D_CLASS;
            break;
         }
         break;
      }
      sclass->sclass.oclass[idx].minver = -1;
      sclass->sclass.oclass[idx].maxver = -1;
      idx++;
      switch (device_info.chip_id & ~0xf) {
      case 0x170:
         sclass->sclass.oclass[idx].oclass = GA102_COMPUTE_CLASS;
         break;
      case 0x160:
         sclass->sclass.oclass[idx].oclass = TU102_COMPUTE_CLASS;
         break;
      case 0x140:
         sclass->sclass.oclass[idx].oclass = GV100_COMPUTE_CLASS;
         break;
      case 0x130:
         switch (device_info.chip_id) {
         case 0x130:
         case 0x13b:
            sclass->sclass.oclass[idx].oclass = GP100_COMPUTE_CLASS;
            break;
         default:
            sclass->sclass.oclass[idx].oclass = GP104_COMPUTE_CLASS;
            break;
         }
         break;
      case 0x120:
         sclass->sclass.oclass[idx].oclass = GM200_COMPUTE_CLASS;
         break;
      case 0x110:
         sclass->sclass.oclass[idx].oclass = GM107_COMPUTE_CLASS;
         break;
      case 0x100:
      case 0xf0:
         sclass->sclass.oclass[idx].oclass = NVF0_COMPUTE_CLASS;
         break;
      case 0xe0:
         sclass->sclass.oclass[idx].oclass = NVE4_COMPUTE_CLASS;
         break;
      default:
         sclass->sclass.oclass[idx].oclass = NVC0_COMPUTE_CLASS;
         break;
      }
      sclass->sclass.oclass[idx].minver = -1;
      sclass->sclass.oclass[idx].maxver = -1;
      break;
   }
   default:
      break;
   }

   return 0;
}

static ioctl_fn_t driver_ioctls[] = {
   [DRM_NOUVEAU_GETPARAM] = nouveau_ioctl_get_param,
   [DRM_NOUVEAU_NVIF] = nouveau_ioctl_nvif,
   [DRM_NOUVEAU_CHANNEL_ALLOC] = nouveau_ioctl_channel_alloc,
   [DRM_NOUVEAU_CHANNEL_FREE] = nouveau_ioctl_noop,
   [DRM_NOUVEAU_GROBJ_ALLOC] = nouveau_ioctl_noop,
   [DRM_NOUVEAU_NOTIFIEROBJ_ALLOC] = nouveau_ioctl_noop,
   [DRM_NOUVEAU_GPUOBJ_FREE] = nouveau_ioctl_noop,
   [DRM_NOUVEAU_GEM_NEW] = nouveau_ioctl_gem_new,
   [DRM_NOUVEAU_GEM_PUSHBUF] = nouveau_ioctl_gem_pushbuf,
   [DRM_NOUVEAU_GEM_CPU_PREP] = nouveau_ioctl_noop,
   [DRM_NOUVEAU_GEM_INFO] = nouveau_ioctl_gem_info,
   [DRM_NOUVEAU_GEM_CPU_FINI] = nouveau_ioctl_gem_info,
   [DRM_NOUVEAU_VM_INIT] = nouveau_ioctl_noop,
   [DRM_NOUVEAU_VM_BIND] = nouveau_ioctl_noop,
   [DRM_NOUVEAU_EXEC] = nouveau_ioctl_noop,
};

static void
nouveau_driver_get_device_info(void)
{
   const char *env = getenv("NOUVEAU_CHIPSET");

   if (!env) {
      device_info.chip_id = 0xf0;
      return;
   }

   device_info.chip_id = strtol(env, NULL, 16);
}

void
drm_shim_driver_init(void)
{
   shim_device.bus_type = DRM_BUS_PCI;
   shim_device.driver_name = "nouveau";
   shim_device.driver_ioctls = driver_ioctls;
   shim_device.driver_ioctl_count = ARRAY_SIZE(driver_ioctls);

   shim_device.version_major = 1;
   shim_device.version_minor = 3;
   shim_device.version_patchlevel = 1;

   nouveau_driver_get_device_info();

   /* Ask userspace to consider all fences completed. */
   setenv("NOUVEAU_DISABLE_FENCES", "true", true);

   /* nothing looks at the pci id, so fix it to a GTX 780 */
   static const char uevent_content[] =
      "DRIVER=nouveau\n"
      "PCI_CLASS=30000\n"
      "PCI_ID=10de:1004\n"
      "PCI_SUBSYS_ID=1028:075B\n"
      "PCI_SLOT_NAME=0000:01:00.0\n"
      "MODALIAS=pci:v000010ded00005916sv00001028sd0000075Bbc03sc00i00\n";
   drm_shim_override_file(uevent_content,
                          "/sys/dev/char/%d:%d/device/uevent",
                          DRM_MAJOR, render_node_minor);
   drm_shim_override_file("0x0\n",
                          "/sys/dev/char/%d:%d/device/revision",
                          DRM_MAJOR, render_node_minor);
   drm_shim_override_file("0x10de",
                          "/sys/dev/char/%d:%d/device/vendor",
                          DRM_MAJOR, render_node_minor);
   drm_shim_override_file("0x10de",
                          "/sys/devices/pci0000:00/0000:01:00.0/vendor");
   drm_shim_override_file("0x1004",
                          "/sys/dev/char/%d:%d/device/device",
                          DRM_MAJOR, render_node_minor);
   drm_shim_override_file("0x1004",
                          "/sys/devices/pci0000:00/0000:01:00.0/device");
   drm_shim_override_file("0x1234",
                          "/sys/dev/char/%d:%d/device/subsystem_vendor",
                          DRM_MAJOR, render_node_minor);
   drm_shim_override_file("0x1234",
                          "/sys/devices/pci0000:00/0000:01:00.0/subsystem_vendor");
   drm_shim_override_file("0x1234",
                          "/sys/dev/char/%d:%d/device/subsystem_device",
                          DRM_MAJOR, render_node_minor);
   drm_shim_override_file("0x1234",
                          "/sys/devices/pci0000:00/0000:01:00.0/subsystem_device");
}
