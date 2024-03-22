/*
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <xf86drm.h>
#include <inttypes.h>

#include "drm-uapi/amdgpu_drm.h"
#include "util/macros.h"

static int
amdgpu_info_hw_ip_info(int fd, uint32_t type, struct drm_amdgpu_info_hw_ip *info)
{
   struct drm_amdgpu_info arg = {
      .return_pointer = (uint64_t)info,
      .return_size = sizeof(*info),
      .query = AMDGPU_INFO_HW_IP_INFO,
      .query_hw_ip = {
         .type = type,
      },
   };

   memset(info, 0, sizeof(*info));

   return drmIoctl(fd, DRM_IOCTL_AMDGPU_INFO, &arg);
}

static int
amdgpu_info_fw_version(int fd, uint32_t type, struct drm_amdgpu_info_firmware *info)
{
   struct drm_amdgpu_info arg = {
      .return_pointer = (uint64_t)info,
      .return_size = sizeof(*info),
      .query = AMDGPU_INFO_FW_VERSION,
      .query_fw = {
         .fw_type = type,
      },
   };

   memset(info, 0, sizeof(*info));

   return drmIoctl(fd, DRM_IOCTL_AMDGPU_INFO, &arg);
}

static int
amdgpu_info_read_mmr_reg(int fd, uint32_t reg, uint32_t count, uint32_t instance, uint32_t *vals)
{
   struct drm_amdgpu_info arg = {
      .return_pointer = (uint64_t)vals,
      .return_size = sizeof(*vals) * count,
      .query = AMDGPU_INFO_READ_MMR_REG,
      .read_mmr_reg = {
         .dword_offset = reg,
         .count = count,
         .instance = instance,
      },
   };

   memset(vals, 0, sizeof(*vals) * count);

   return drmIoctl(fd, DRM_IOCTL_AMDGPU_INFO, &arg);
}

static int
amdgpu_info_dev_info(int fd, struct drm_amdgpu_info_device *info)
{
   struct drm_amdgpu_info arg = {
      .return_pointer = (uint64_t)info,
      .return_size = sizeof(*info),
      .query = AMDGPU_INFO_DEV_INFO,
   };

   memset(info, 0, sizeof(*info));

   return drmIoctl(fd, DRM_IOCTL_AMDGPU_INFO, &arg);
}

static int
amdgpu_info_memory(int fd, struct drm_amdgpu_memory_info *info)
{
   struct drm_amdgpu_info arg = {
      .return_pointer = (uint64_t)info,
      .return_size = sizeof(*info),
      .query = AMDGPU_INFO_MEMORY,
   };

   memset(info, 0, sizeof(*info));

   return drmIoctl(fd, DRM_IOCTL_AMDGPU_INFO, &arg);
}

static void
amdgpu_dump_memory(int fd)
{
   struct drm_amdgpu_memory_info info;
   if (amdgpu_info_memory(fd, &info))
      return;

   printf(".mem = {\n");
   printf("   .vram = {\n");
   printf("      .total_heap_size = UINT64_C(%"PRIu64"),\n", (uint64_t)info.vram.total_heap_size);
   printf("      .usable_heap_size = UINT64_C(%"PRIu64"),\n", (uint64_t)info.vram.usable_heap_size);
   printf("      .heap_usage = UINT64_C(%"PRIu64"),\n", (uint64_t)info.vram.heap_usage);
   printf("      .max_allocation = UINT64_C(%"PRIu64"),\n", (uint64_t)info.vram.max_allocation);
   printf("   },\n");
   printf("   .cpu_accessible_vram = {\n");
   printf("      .total_heap_size = UINT64_C(%"PRIu64"),\n",
          (uint64_t)info.cpu_accessible_vram.total_heap_size);
   printf("      .usable_heap_size = UINT64_C(%"PRIu64"),\n",
          (uint64_t)info.cpu_accessible_vram.usable_heap_size);
   printf("      .heap_usage = UINT64_C(%"PRIu64"),\n",
          (uint64_t)info.cpu_accessible_vram.heap_usage);
   printf("      .max_allocation = UINT64_C(%"PRIu64"),\n",
          (uint64_t)info.cpu_accessible_vram.max_allocation);
   printf("   },\n");
   printf("   .gtt = {\n");
   printf("      .total_heap_size = UINT64_C(%"PRIu64"),\n", (uint64_t)info.gtt.total_heap_size);
   printf("      .usable_heap_size = UINT64_C(%"PRIu64"),\n", (uint64_t)info.gtt.usable_heap_size);
   printf("      .heap_usage = UINT64_C(%"PRIu64"),\n", (uint64_t)info.gtt.heap_usage);
   printf("      .max_allocation = UINT64_C(%"PRIu64"),\n", (uint64_t)info.gtt.max_allocation);
   printf("   },\n");
   printf("},\n");
}

static void
amdgpu_dump_dev_info(int fd)
{
   static const struct {
      const char *name;
      uint32_t family;
   } families[] = {
#define FAMILY(x) { "AMDGPU_FAMILY_" #x, AMDGPU_FAMILY_##x }
      /* clang-format off */
      FAMILY(SI),
      FAMILY(CI),
      FAMILY(KV),
      FAMILY(VI),
      FAMILY(CZ),
      FAMILY(AI),
      FAMILY(RV),
      FAMILY(NV),
      FAMILY(VGH),
      FAMILY(GC_11_0_0),
      FAMILY(YC),
      FAMILY(GC_11_0_1),
      FAMILY(GC_10_3_6),
      FAMILY(GC_10_3_7),
   /* clang-format on */
#undef FAMILY
   };

   struct drm_amdgpu_info_device info;
   if (amdgpu_info_dev_info(fd, &info))
      return;

   const char *family_name = NULL;
   for (int i = 0; i < ARRAY_SIZE(families); i++) {
      if (families[i].family == info.family) {
         family_name = families[i].name;
         break;
      }
   }
   if (!family_name)
      return;

   printf(".dev = {\n");
   printf("   .device_id = 0x%04x,\n", info.device_id);
   printf("   .chip_rev = 0x%02x,\n", info.chip_rev);
   printf("   .external_rev = 0x%02x,\n", info.external_rev);
   printf("   .pci_rev = 0x%02x,\n", info.pci_rev);

   printf("   .family = %s,\n", family_name);

   printf("   .num_shader_engines = %u,\n", info.num_shader_engines);
   printf("   .num_shader_arrays_per_engine = %u,\n", info.num_shader_arrays_per_engine);
   printf("   .gpu_counter_freq = %u,\n", info.gpu_counter_freq);
   printf("   .max_engine_clock = UINT64_C(%"PRIu64"),\n", (uint64_t)info.max_engine_clock);
   printf("   .max_memory_clock = UINT64_C(%"PRIu64"),\n", (uint64_t)info.max_memory_clock);
   printf("   .cu_active_number = %u,\n", info.cu_active_number);
   printf("   .cu_ao_mask = 0x%x,\n", info.cu_ao_mask);

   printf("   .cu_bitmap = {\n");
   for (int i = 0; i < ARRAY_SIZE(info.cu_bitmap); i++) {
      printf("      {");
      for (int j = 0; j < ARRAY_SIZE(info.cu_bitmap[i]); j++)
         printf(" 0x%x,", info.cu_bitmap[i][j]);
      printf(" },\n");
   }
   printf("   },\n");

   printf("   .enabled_rb_pipes_mask = 0x%x,\n", info.enabled_rb_pipes_mask);
   printf("   .num_rb_pipes = %u,\n", info.num_rb_pipes);
   printf("   .num_hw_gfx_contexts = %u,\n", info.num_hw_gfx_contexts);
   printf("   .pcie_gen = %u,\n", info.pcie_gen);
   printf("   .ids_flags = UINT64_C(0x%"PRIx64"),\n", (uint64_t)info.ids_flags);
   printf("   .virtual_address_offset = UINT64_C(0x%"PRIx64"),\n",
          (uint64_t)info.virtual_address_offset);
   printf("   .virtual_address_max = UINT64_C(0x%"PRIx64"),\n", (uint64_t)info.virtual_address_max);
   printf("   .virtual_address_alignment = %u,\n", info.virtual_address_alignment);
   printf("   .pte_fragment_size = %u,\n", info.pte_fragment_size);
   printf("   .gart_page_size = %u,\n", info.gart_page_size);
   printf("   .ce_ram_size = %u,\n", info.ce_ram_size);
   printf("   .vram_type = %u,\n", info.vram_type);
   printf("   .vram_bit_width = %u,\n", info.vram_bit_width);
   printf("   .vce_harvest_config = %u,\n", info.vce_harvest_config);
   printf("   .gc_double_offchip_lds_buf = %u,\n", info.gc_double_offchip_lds_buf);
   printf("   .prim_buf_gpu_addr = UINT64_C(%"PRIu64"),\n", (uint64_t)info.prim_buf_gpu_addr);
   printf("   .pos_buf_gpu_addr = UINT64_C(%"PRIu64"),\n", (uint64_t)info.pos_buf_gpu_addr);
   printf("   .cntl_sb_buf_gpu_addr = UINT64_C(%"PRIu64"),\n", (uint64_t)info.cntl_sb_buf_gpu_addr);
   printf("   .param_buf_gpu_addr = UINT64_C(%"PRIu64"),\n", (uint64_t)info.param_buf_gpu_addr);
   printf("   .prim_buf_size = %u,\n", info.prim_buf_size);
   printf("   .pos_buf_size = %u,\n", info.pos_buf_size);
   printf("   .cntl_sb_buf_size = %u,\n", info.cntl_sb_buf_size);
   printf("   .param_buf_size = %u,\n", info.param_buf_size);
   printf("   .wave_front_size = %u,\n", info.wave_front_size);
   printf("   .num_shader_visible_vgprs = %u,\n", info.num_shader_visible_vgprs);
   printf("   .num_cu_per_sh = %u,\n", info.num_cu_per_sh);
   printf("   .num_tcc_blocks = %u,\n", info.num_tcc_blocks);
   printf("   .gs_vgt_table_depth = %u,\n", info.gs_vgt_table_depth);
   printf("   .gs_prim_buffer_depth = %u,\n", info.gs_prim_buffer_depth);
   printf("   .max_gs_waves_per_vgt = %u,\n", info.max_gs_waves_per_vgt);
   printf("   .pcie_num_lanes = %u,\n", info.pcie_num_lanes);

   printf("   .cu_ao_bitmap = {\n");
   for (int i = 0; i < ARRAY_SIZE(info.cu_ao_bitmap); i++) {
      printf("      {");
      for (int j = 0; j < ARRAY_SIZE(info.cu_ao_bitmap[i]); j++)
         printf(" 0x%x,", info.cu_ao_bitmap[i][j]);
      printf(" },\n");
   }
   printf("   },\n");

   printf("   .high_va_offset = UINT64_C(0x%"PRIx64"),\n", (uint64_t)info.high_va_offset);
   printf("   .high_va_max = UINT64_C(0x%"PRIx64"),\n", (uint64_t)info.high_va_max);
   printf("   .pa_sc_tile_steering_override = %u,\n", info.pa_sc_tile_steering_override);
   printf("   .tcc_disabled_mask = UINT64_C(%"PRIu64"),\n", (uint64_t)info.tcc_disabled_mask);
   printf("   .min_engine_clock = UINT64_C(%"PRIu64"),\n", (uint64_t)info.min_engine_clock);
   printf("   .min_memory_clock = UINT64_C(%"PRIu64"),\n", (uint64_t)info.min_memory_clock);
   printf("   .tcp_cache_size = %u,\n", info.tcp_cache_size);
   printf("   .num_sqc_per_wgp = %u,\n", info.num_sqc_per_wgp);
   printf("   .sqc_data_cache_size = %u,\n", info.sqc_data_cache_size);
   printf("   .sqc_inst_cache_size = %u,\n", info.sqc_inst_cache_size);
   printf("   .gl1c_cache_size = %u,\n", info.gl1c_cache_size);
   printf("   .gl2c_cache_size = %u,\n", info.gl2c_cache_size);
   printf("   .mall_size = UINT64_C(%"PRIu64"),\n", (uint64_t)info.mall_size);
   printf("   .enabled_rb_pipes_mask_hi = %u,\n", info.enabled_rb_pipes_mask_hi);
   printf("},\n");
}

static void
amdgpu_dump_mmr_regs(int fd)
{
   struct drm_amdgpu_info_device info;
   if (amdgpu_info_dev_info(fd, &info))
      return;

#define READ_REG(fd, reg, cnt, instance, rec)                                                      \
   do {                                                                                            \
      if (rec.count + cnt > ARRAY_SIZE(rec.vals))                                                  \
         return;                                                                                   \
      if (amdgpu_info_read_mmr_reg(fd, reg, cnt, instance, rec.vals + rec.count))                  \
         return;                                                                                   \
      for (int i = 0; i < cnt; i++) {                                                              \
         rec.regs[rec.count + i] = reg + i;                                                        \
         rec.instances[rec.count + i] = instance;                                                  \
      }                                                                                            \
      rec.count += cnt;                                                                            \
   } while (0)

   struct {
      uint32_t regs[256];
      uint32_t instances[256];
      uint32_t vals[256];
      uint32_t count;
   } rec = { 0 };

   /* GB_ADDR_CONFIG */
   READ_REG(fd, 0x263e, 1, 0xffffffff, rec);

   if (info.family < AMDGPU_FAMILY_AI) {
      for (int i = 0; i < info.num_shader_engines; i++) {
         const uint32_t instance =
            (i << AMDGPU_INFO_MMR_SE_INDEX_SHIFT) |
            (AMDGPU_INFO_MMR_SH_INDEX_MASK << AMDGPU_INFO_MMR_SH_INDEX_SHIFT);
         /* CC_RB_BACKEND_DISABLE */
         READ_REG(fd, 0x263d, 1, instance, rec);
         /* PA_SC_RASTER_CONFIG */
         READ_REG(fd, 0xa0d4, 1, instance, rec);
         /* PA_SC_RASTER_CONFIG_1 */
         if (info.family >= AMDGPU_FAMILY_CI)
            READ_REG(fd, 0xa0d5, 1, instance, rec);
      }

      /* MC_ARB_RAMCFG */
      READ_REG(fd, 0x9d8, 1, 0xffffffff, rec);
      /* GB_TILE_MODE0 */
      READ_REG(fd, 0x2644, 32, 0xffffffff, rec);
      /* GB_MACROTILE_MODE0 */
      if (info.family >= AMDGPU_FAMILY_CI)
         READ_REG(fd, 0x2664, 16, 0xffffffff, rec);
   }

#undef READ_REG

   printf(".mmr_regs = {\n");
   for (int i = 0; i < rec.count; i++)
      printf("   0x%04x, 0x%08x, 0x%08x,\n", rec.regs[i], rec.instances[i], rec.vals[i]);
   printf("},\n");
   printf(".mmr_reg_count = %d,\n", rec.count);
}

static void
amdgpu_dump_fw_versions(int fd)
{
   static const struct {
      const char *name;
      uint32_t type;
   } fw_vers[] = {
      {
         .name = "gfx_me",
         .type = AMDGPU_INFO_FW_GFX_ME,
      },
      {
         .name = "gfx_pfp",
         .type = AMDGPU_INFO_FW_GFX_PFP,
      },
      {
         .name = "gfx_mec",
         .type = AMDGPU_INFO_FW_GFX_MEC,
      },
   };

   for (int i = 0; i < ARRAY_SIZE(fw_vers); i++) {
      struct drm_amdgpu_info_firmware info;
      if (amdgpu_info_fw_version(fd, fw_vers[i].type, &info))
         continue;

      printf(".fw_%s = {\n", fw_vers[i].name);
      printf("   .ver = %u,\n", info.ver);
      printf("   .feature = %u,\n", info.feature);
      printf("},\n");
   }
}

static void
amdgpu_dump_hw_ips(int fd)
{
   static const struct {
      const char *name;
      uint32_t type;
   } hw_ips[] = {
      {
         .name = "gfx",
         .type = AMDGPU_HW_IP_GFX,
      },
      {
         .name = "compute",
         .type = AMDGPU_HW_IP_COMPUTE,
      },
   };

   for (int i = 0; i < ARRAY_SIZE(hw_ips); i++) {
      struct drm_amdgpu_info_hw_ip info;
      if (amdgpu_info_hw_ip_info(fd, hw_ips[i].type, &info))
         continue;

      printf(".hw_ip_%s = {\n", hw_ips[i].name);
      printf("   .hw_ip_version_major = %u,\n", info.hw_ip_version_major);
      printf("   .hw_ip_version_minor = %u,\n", info.hw_ip_version_minor);
      printf("   .capabilities_flags = UINT64_C(%"PRIu64"),\n", (uint64_t)info.capabilities_flags);
      printf("   .ib_start_alignment = %u,\n", info.ib_start_alignment);
      printf("   .ib_size_alignment = %u,\n", info.ib_size_alignment);
      printf("   .available_rings = 0x%x,\n", info.available_rings);
      printf("   .ip_discovery_version = 0x%04x,\n", info.ip_discovery_version);
      printf("},\n");
   }
}

static void
amdgpu_dump_version(int fd)
{
   const drmVersionPtr ver = drmGetVersion(fd);
   if (!ver)
      return;

   printf(".drm = {\n");
   printf("   .version_major = %d,\n", ver->version_major);
   printf("   .version_minor = %d,\n", ver->version_minor);
   printf("   .version_patchlevel = %d,\n", ver->version_patchlevel);
   printf("   .name = \"%s\",\n", ver->name);
   printf("},\n");

   drmFreeVersion(ver);
}

static void
amdgpu_dump_pci(drmDevicePtr dev)
{
   printf(".pci = {\n");
   printf("   .vendor_id = 0x%04x,\n", dev->deviceinfo.pci->vendor_id);
   printf("   .device_id = 0x%04x,\n", dev->deviceinfo.pci->device_id);
   printf("   .subvendor_id = 0x%04x,\n", dev->deviceinfo.pci->subvendor_id);
   printf("   .subdevice_id = 0x%04x,\n", dev->deviceinfo.pci->subdevice_id);
   printf("   .revision_id = 0x%02x,\n", dev->deviceinfo.pci->revision_id);
   printf("},\n");
}

static void
amdgpu_dump(drmDevicePtr dev)
{
   if (!(dev->available_nodes & (1 << DRM_NODE_RENDER)))
      return;
   if (dev->bustype != DRM_BUS_PCI)
      return;
   if (dev->deviceinfo.pci->vendor_id != 0x1002)
      return;

   int fd = open(dev->nodes[DRM_NODE_RENDER], O_RDWR | O_CLOEXEC);
   if (fd < 0)
      return;

   amdgpu_dump_pci(dev);
   amdgpu_dump_version(fd);
   amdgpu_dump_hw_ips(fd);
   amdgpu_dump_fw_versions(fd);
   amdgpu_dump_mmr_regs(fd);
   amdgpu_dump_dev_info(fd);
   amdgpu_dump_memory(fd);

   close(fd);
}

int
main()
{
   drmDevicePtr devs[8];
   const int count = drmGetDevices2(DRM_DEVICE_GET_PCI_REVISION, devs, ARRAY_SIZE(devs));

   for (int i = 0; i < count; i++)
      amdgpu_dump(devs[i]);

   drmFreeDevices(devs, count);

   return 0;
}
