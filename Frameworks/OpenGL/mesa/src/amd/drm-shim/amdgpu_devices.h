/*
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stddef.h>
#include "common/amd_family.h"
#include "drm-uapi/amdgpu_drm.h"

struct amdgpu_device {
   const char *name;
   enum radeon_family radeon_family;

   struct drm_amdgpu_info_hw_ip hw_ip_gfx;
   struct drm_amdgpu_info_hw_ip hw_ip_compute;

   struct drm_amdgpu_info_firmware fw_gfx_me;
   struct drm_amdgpu_info_firmware fw_gfx_pfp;
   struct drm_amdgpu_info_firmware fw_gfx_mec;

   uint32_t mmr_regs[256 * 3];
   uint32_t mmr_reg_count;

   struct drm_amdgpu_info_device dev;
   struct drm_amdgpu_memory_info mem;
};

extern const struct amdgpu_device amdgpu_devices[];
extern const size_t num_amdgpu_devices;
