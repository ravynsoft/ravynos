/*
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "amdgpu_devices.h"
#include "common/amd_family.h"
#include "drm-shim/drm_shim.h"
#include "drm-uapi/amdgpu_drm.h"
#include "util/log.h"

static const struct amdgpu_device *amdgpu_dev;

bool drm_shim_driver_prefers_first_render_node = true;

static int
amdgpu_ioctl_noop(int fd, unsigned long request, void *arg)
{
   return 0;
}

static int
amdgpu_ioctl_gem_create(int fd, unsigned long request, void *_arg)
{
   union drm_amdgpu_gem_create *arg = _arg;
   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   struct shim_bo *bo = calloc(1, sizeof(*bo));
   int ret;

   ret = drm_shim_bo_init(bo, arg->in.bo_size);
   if (ret) {
      free(bo);
      return ret;
   }

   arg->out.handle = drm_shim_bo_get_handle(shim_fd, bo);

   drm_shim_bo_put(bo);

   return 0;
}

static int
amdgpu_ioctl_gem_mmap(int fd, unsigned long request, void *_arg)
{
   union drm_amdgpu_gem_mmap *arg = _arg;
   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   struct shim_bo *bo = drm_shim_bo_lookup(shim_fd, arg->in.handle);

   arg->out.addr_ptr = drm_shim_bo_get_mmap_offset(shim_fd, bo);

   return 0;
}

static void
amdgpu_info_hw_ip_info(uint32_t type, struct drm_amdgpu_info_hw_ip *out)
{
   switch (type) {
   case AMDGPU_HW_IP_GFX:
      *out = amdgpu_dev->hw_ip_gfx;
      break;
   case AMDGPU_HW_IP_COMPUTE:
      *out = amdgpu_dev->hw_ip_compute;
      break;
   default:
      break;
   }
}

static void
amdgpu_info_fw_version(uint32_t type, struct drm_amdgpu_info_firmware *out)
{
   switch (type) {
   case AMDGPU_INFO_FW_GFX_ME:
      *out = amdgpu_dev->fw_gfx_me;
      break;
   case AMDGPU_INFO_FW_GFX_PFP:
      *out = amdgpu_dev->fw_gfx_pfp;
      break;
   case AMDGPU_INFO_FW_GFX_MEC:
      *out = amdgpu_dev->fw_gfx_mec;
      break;
   default:
      break;
   }
}

static void
amdgpu_info_read_mmr_reg(uint32_t reg, uint32_t count, uint32_t instance, uint32_t *vals)
{
   for (int i = 0; i < count; i++) {
      /* linear search */
      bool found = false;
      uint32_t val = 0;
      for (int j = 0; j < amdgpu_dev->mmr_reg_count; j++) {
         const uint32_t *triple = &amdgpu_dev->mmr_regs[j * 3];
         if (triple[0] == reg + i && triple[1] == instance) {
            val = triple[2];
            found = true;
            break;
         }
      }

      if (!found)
         mesa_logw("reg 0x%04x is unknown", reg + i);

      vals[i] = val;
   }
}

static void
amdgpu_info_dev_info(struct drm_amdgpu_info_device *out)
{
   *out = amdgpu_dev->dev;
}

static void
amdgpu_info_memory(struct drm_amdgpu_memory_info *out)
{
   *out = amdgpu_dev->mem;

   /* override all but total_heap_size */
   out->vram.usable_heap_size = out->vram.total_heap_size;
   out->vram.heap_usage = 0;
   out->vram.max_allocation = out->vram.total_heap_size * 3 / 4;
   out->cpu_accessible_vram.usable_heap_size = out->cpu_accessible_vram.total_heap_size;
   out->cpu_accessible_vram.heap_usage = 0;
   out->cpu_accessible_vram.max_allocation = out->cpu_accessible_vram.total_heap_size * 3 / 4;
   out->gtt.usable_heap_size = out->gtt.total_heap_size;
   out->gtt.heap_usage = 0;
   out->gtt.max_allocation = out->gtt.total_heap_size * 3 / 4;
}

static void
amdgpu_info_video_caps(uint32_t type, struct drm_amdgpu_info_video_caps *out)
{
   /* nop */
}

static int
amdgpu_ioctl_info(int fd, unsigned long request, void *arg)
{
   const struct drm_amdgpu_info *info = arg;
   union {
      void *ptr;
      uint32_t *ui32;
   } out = { .ptr = (void *)info->return_pointer };

   switch (info->query) {
   case AMDGPU_INFO_ACCEL_WORKING:
      *out.ui32 = 1;
      break;
   case AMDGPU_INFO_HW_IP_INFO:
      amdgpu_info_hw_ip_info(info->query_hw_ip.type, out.ptr);
      break;
   case AMDGPU_INFO_FW_VERSION:
      amdgpu_info_fw_version(info->query_fw.fw_type, out.ptr);
      break;
   case AMDGPU_INFO_READ_MMR_REG:
      amdgpu_info_read_mmr_reg(info->read_mmr_reg.dword_offset, info->read_mmr_reg.count,
                               info->read_mmr_reg.instance, out.ptr);
      break;
   case AMDGPU_INFO_DEV_INFO:
      amdgpu_info_dev_info(out.ptr);
      break;
   case AMDGPU_INFO_MEMORY:
      amdgpu_info_memory(out.ptr);
      break;
   case AMDGPU_INFO_VIDEO_CAPS:
      amdgpu_info_video_caps(info->video_cap.type, out.ptr);
      break;
   default:
      return -EINVAL;
   }

   return 0;
}

static ioctl_fn_t amdgpu_ioctls[] = {
   [DRM_AMDGPU_GEM_CREATE] = amdgpu_ioctl_gem_create,
   [DRM_AMDGPU_GEM_MMAP] = amdgpu_ioctl_gem_mmap,
   [DRM_AMDGPU_CTX] = amdgpu_ioctl_noop,
   [DRM_AMDGPU_BO_LIST] = amdgpu_ioctl_noop,
   [DRM_AMDGPU_CS] = amdgpu_ioctl_noop,
   [DRM_AMDGPU_INFO] = amdgpu_ioctl_info,
   [DRM_AMDGPU_GEM_METADATA] = amdgpu_ioctl_noop,
   [DRM_AMDGPU_GEM_WAIT_IDLE] = amdgpu_ioctl_noop,
   [DRM_AMDGPU_GEM_VA] = amdgpu_ioctl_noop,
   [DRM_AMDGPU_WAIT_CS] = amdgpu_ioctl_noop,
   [DRM_AMDGPU_GEM_OP] = amdgpu_ioctl_noop,
   [DRM_AMDGPU_GEM_USERPTR] = amdgpu_ioctl_noop,
   [DRM_AMDGPU_WAIT_FENCES] = amdgpu_ioctl_noop,
   [DRM_AMDGPU_VM] = amdgpu_ioctl_noop,
   [DRM_AMDGPU_FENCE_TO_HANDLE] = amdgpu_ioctl_noop,
   [DRM_AMDGPU_SCHED] = amdgpu_ioctl_noop,
};

static void
amdgpu_select_device()
{
   const char *gpu_id = getenv("AMDGPU_GPU_ID");
   if (gpu_id) {
      for (uint32_t i = 0; i < num_amdgpu_devices; i++) {
         const struct amdgpu_device *dev = &amdgpu_devices[i];
         if (!strcasecmp(dev->name, gpu_id)) {
            amdgpu_dev = &amdgpu_devices[i];
            break;
         }
      }
   } else {
      amdgpu_dev = &amdgpu_devices[0];
   }

   if (!amdgpu_dev) {
      mesa_loge("Failed to find amdgpu GPU named \"%s\"\n", gpu_id);
      abort();
   }
}

void
drm_shim_driver_init(void)
{
   amdgpu_select_device();

   shim_device.bus_type = DRM_BUS_PCI;
   shim_device.driver_name = "amdgpu";
   shim_device.driver_ioctls = amdgpu_ioctls;
   shim_device.driver_ioctl_count = ARRAY_SIZE(amdgpu_ioctls);

   shim_device.version_major = 3;
   shim_device.version_minor = 49;
   shim_device.version_patchlevel = 0;

   /* make drmGetDevices2 and drmProcessPciDevice happy */
   static const char uevent_content[] =
      "DRIVER=amdgpu\n"
      "PCI_CLASS=30000\n"
      "PCI_ID=1002:15E7\n"
      "PCI_SUBSYS_ID=1028:1636\n"
      "PCI_SLOT_NAME=0000:04:00.0\n"
      "MODALIAS=pci:v00001002d000015E7sv00001002sd00001636bc03sc00i00\n";
   drm_shim_override_file(uevent_content, "/sys/dev/char/%d:%d/device/uevent", DRM_MAJOR,
                          render_node_minor);
   drm_shim_override_file("0xe9\n", "/sys/dev/char/%d:%d/device/revision", DRM_MAJOR,
                          render_node_minor);
   drm_shim_override_file("0x1002", "/sys/dev/char/%d:%d/device/vendor", DRM_MAJOR,
                          render_node_minor);
   drm_shim_override_file("0x15e7", "/sys/dev/char/%d:%d/device/device", DRM_MAJOR,
                          render_node_minor);
   drm_shim_override_file("0x1002", "/sys/dev/char/%d:%d/device/subsystem_vendor", DRM_MAJOR,
                          render_node_minor);
   drm_shim_override_file("0x1636", "/sys/dev/char/%d:%d/device/subsystem_device", DRM_MAJOR,
                          render_node_minor);
}
