/*
 * Copyright © 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_gpu_info.h"
#include "ac_shader_util.h"
#include "ac_debug.h"
#include "ac_surface.h"

#include "addrlib/src/amdgpu_asic_addr.h"
#include "sid.h"
#include "util/macros.h"
#include "util/u_cpu_detect.h"
#include "util/u_math.h"
#include "util/os_misc.h"
#include "util/bitset.h"

#include <stdio.h>
#include <ctype.h>

#define AMDGPU_MI100_RANGE       0x32, 0x3C
#define AMDGPU_MI200_RANGE       0x3C, 0xFF
#define AMDGPU_GFX940_RANGE      0x46, 0xFF

#define ASICREV_IS_MI100(r)      ASICREV_IS(r, MI100)
#define ASICREV_IS_MI200(r)      ASICREV_IS(r, MI200)
#define ASICREV_IS_GFX940(r)     ASICREV_IS(r, GFX940)

#ifdef _WIN32
#define DRM_CAP_ADDFB2_MODIFIERS 0x10
#define DRM_CAP_SYNCOBJ 0x13
#define DRM_CAP_SYNCOBJ_TIMELINE 0x14
#define AMDGPU_GEM_DOMAIN_GTT 0x2
#define AMDGPU_GEM_DOMAIN_VRAM 0x4
#define AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED (1 << 0)
#define AMDGPU_GEM_CREATE_ENCRYPTED (1 << 10)
#define AMDGPU_HW_IP_GFX 0
#define AMDGPU_HW_IP_COMPUTE 1
#define AMDGPU_HW_IP_DMA 2
#define AMDGPU_HW_IP_UVD 3
#define AMDGPU_HW_IP_VCE 4
#define AMDGPU_HW_IP_UVD_ENC 5
#define AMDGPU_HW_IP_VCN_DEC 6
#define AMDGPU_HW_IP_VCN_ENC 7
#define AMDGPU_HW_IP_VCN_JPEG 8
#define AMDGPU_HW_IP_VPE 9
#define AMDGPU_IDS_FLAGS_FUSION 0x1
#define AMDGPU_IDS_FLAGS_PREEMPTION 0x2
#define AMDGPU_IDS_FLAGS_TMZ 0x4
#define AMDGPU_IDS_FLAGS_CONFORMANT_TRUNC_COORD 0x8
#define AMDGPU_INFO_FW_VCE 0x1
#define AMDGPU_INFO_FW_UVD 0x2
#define AMDGPU_INFO_FW_GFX_ME 0x04
#define AMDGPU_INFO_FW_GFX_PFP 0x05
#define AMDGPU_INFO_FW_GFX_CE 0x06
#define AMDGPU_INFO_DEV_INFO 0x16
#define AMDGPU_INFO_MEMORY 0x19
#define AMDGPU_INFO_VIDEO_CAPS_DECODE 0
#define AMDGPU_INFO_VIDEO_CAPS_ENCODE 1
#define AMDGPU_INFO_FW_GFX_MEC 0x08
#define AMDGPU_INFO_MAX_IBS 0x22

#define AMDGPU_VRAM_TYPE_UNKNOWN 0
#define AMDGPU_VRAM_TYPE_GDDR1 1
#define AMDGPU_VRAM_TYPE_DDR2  2
#define AMDGPU_VRAM_TYPE_GDDR3 3
#define AMDGPU_VRAM_TYPE_GDDR4 4
#define AMDGPU_VRAM_TYPE_GDDR5 5
#define AMDGPU_VRAM_TYPE_HBM   6
#define AMDGPU_VRAM_TYPE_DDR3  7
#define AMDGPU_VRAM_TYPE_DDR4  8
#define AMDGPU_VRAM_TYPE_GDDR6 9
#define AMDGPU_VRAM_TYPE_DDR5  10
#define AMDGPU_VRAM_TYPE_LPDDR4 11
#define AMDGPU_VRAM_TYPE_LPDDR5 12

struct drm_amdgpu_heap_info {
   uint64_t total_heap_size;
};
struct drm_amdgpu_memory_info {
   struct drm_amdgpu_heap_info vram;
   struct drm_amdgpu_heap_info cpu_accessible_vram;
   struct drm_amdgpu_heap_info gtt;
};
struct drm_amdgpu_info_device {
	/** PCI Device ID */
	uint32_t device_id;
	/** Internal chip revision: A0, A1, etc.) */
	uint32_t chip_rev;
	uint32_t external_rev;
	/** Revision id in PCI Config space */
	uint32_t pci_rev;
	uint32_t family;
	uint32_t num_shader_engines;
	uint32_t num_shader_arrays_per_engine;
	/* in KHz */
	uint32_t gpu_counter_freq;
	uint64_t max_engine_clock;
	uint64_t max_memory_clock;
	/* cu information */
	uint32_t cu_active_number;
	/* NOTE: cu_ao_mask is INVALID, DON'T use it */
	uint32_t cu_ao_mask;
	uint32_t cu_bitmap[4][4];
	/** Render backend pipe mask. One render backend is CB+DB. */
	uint32_t enabled_rb_pipes_mask;
	uint32_t num_rb_pipes;
	uint32_t num_hw_gfx_contexts;
	/* PCIe version (the smaller of the GPU and the CPU/motherboard) */
	uint32_t pcie_gen;
	uint64_t ids_flags;
	/** Starting virtual address for UMDs. */
	uint64_t virtual_address_offset;
	/** The maximum virtual address */
	uint64_t virtual_address_max;
	/** Required alignment of virtual addresses. */
	uint32_t virtual_address_alignment;
	/** Page table entry - fragment size */
	uint32_t pte_fragment_size;
	uint32_t gart_page_size;
	/** constant engine ram size*/
	uint32_t ce_ram_size;
	/** video memory type info*/
	uint32_t vram_type;
	/** video memory bit width*/
	uint32_t vram_bit_width;
	/* vce harvesting instance */
	uint32_t vce_harvest_config;
	/* gfx double offchip LDS buffers */
	uint32_t gc_double_offchip_lds_buf;
	/* NGG Primitive Buffer */
	uint64_t prim_buf_gpu_addr;
	/* NGG Position Buffer */
	uint64_t pos_buf_gpu_addr;
	/* NGG Control Sideband */
	uint64_t cntl_sb_buf_gpu_addr;
	/* NGG Parameter Cache */
	uint64_t param_buf_gpu_addr;
	uint32_t prim_buf_size;
	uint32_t pos_buf_size;
	uint32_t cntl_sb_buf_size;
	uint32_t param_buf_size;
	/* wavefront size*/
	uint32_t wave_front_size;
	/* shader visible vgprs*/
	uint32_t num_shader_visible_vgprs;
	/* CU per shader array*/
	uint32_t num_cu_per_sh;
	/* number of tcc blocks*/
	uint32_t num_tcc_blocks;
	/* gs vgt table depth*/
	uint32_t gs_vgt_table_depth;
	/* gs primitive buffer depth*/
	uint32_t gs_prim_buffer_depth;
	/* max gs wavefront per vgt*/
	uint32_t max_gs_waves_per_vgt;
	/* PCIe number of lanes (the smaller of the GPU and the CPU/motherboard) */
	uint32_t pcie_num_lanes;
	/* always on cu bitmap */
	uint32_t cu_ao_bitmap[4][4];
	/** Starting high virtual address for UMDs. */
	uint64_t high_va_offset;
	/** The maximum high virtual address */
	uint64_t high_va_max;
	/* gfx10 pa_sc_tile_steering_override */
	uint32_t pa_sc_tile_steering_override;
	/* disabled TCCs */
	uint64_t tcc_disabled_mask;
	uint64_t min_engine_clock;
	uint64_t min_memory_clock;
	/* The following fields are only set on gfx11+, older chips set 0. */
	uint32_t tcp_cache_size;       /* AKA GL0, VMEM cache */
	uint32_t num_sqc_per_wgp;
	uint32_t sqc_data_cache_size;  /* AKA SMEM cache */
	uint32_t sqc_inst_cache_size;
	uint32_t gl1c_cache_size;
	uint32_t gl2c_cache_size;
	uint64_t mall_size;            /* AKA infinity cache */
	/* high 32 bits of the rb pipes mask */
	uint32_t enabled_rb_pipes_mask_hi;
	/* shadow area size for gfx11 */
	uint32_t shadow_size;
	/* shadow area base virtual alignment for gfx11 */
	uint32_t shadow_alignment;
	/* context save area size for gfx11 */
	uint32_t csa_size;
	/* context save area base virtual alignment for gfx11 */
	uint32_t csa_alignment;
};
struct drm_amdgpu_info_hw_ip {
   uint32_t hw_ip_version_major;
   uint32_t hw_ip_version_minor;
   uint32_t ib_start_alignment;
   uint32_t ib_size_alignment;
   uint32_t available_rings;
   uint32_t ip_discovery_version;
};
typedef struct _drmPciBusInfo {
   uint16_t domain;
   uint8_t bus;
   uint8_t dev;
   uint8_t func;
} drmPciBusInfo, *drmPciBusInfoPtr;
typedef struct _drmDevice {
   union {
      drmPciBusInfoPtr pci;
   } businfo;
} drmDevice, *drmDevicePtr;
enum amdgpu_sw_info {
   amdgpu_sw_info_address32_hi = 0,
};
typedef struct amdgpu_device *amdgpu_device_handle;
typedef struct amdgpu_bo *amdgpu_bo_handle;
struct amdgpu_bo_alloc_request {
   uint64_t alloc_size;
   uint64_t phys_alignment;
   uint32_t preferred_heap;
   uint64_t flags;
};
struct amdgpu_gds_resource_info {
   uint32_t gds_gfx_partition_size;
   uint32_t gds_total_size;
};
struct amdgpu_buffer_size_alignments {
   uint64_t size_local;
   uint64_t size_remote;
};
struct amdgpu_heap_info {
   uint64_t heap_size;
};
struct amdgpu_gpu_info {
   uint32_t asic_id;
   uint32_t chip_external_rev;
   uint32_t family_id;
   uint64_t ids_flags;
   uint64_t max_engine_clk;
   uint64_t max_memory_clk;
   uint32_t num_shader_engines;
   uint32_t num_shader_arrays_per_engine;
   uint32_t rb_pipes;
   uint32_t enabled_rb_pipes_mask;
   uint32_t gpu_counter_freq;
   uint32_t mc_arb_ramcfg;
   uint32_t gb_addr_cfg;
   uint32_t gb_tile_mode[32];
   uint32_t gb_macro_tile_mode[16];
   uint32_t cu_bitmap[4][4];
   uint32_t vram_type;
   uint32_t vram_bit_width;
   uint32_t ce_ram_size;
   uint32_t vce_harvest_config;
   uint32_t pci_rev_id;
};
static int drmGetCap(int fd, uint64_t capability, uint64_t *value)
{
   return -EINVAL;
}
static void drmFreeDevice(drmDevicePtr *device)
{
}
static int drmGetDevice2(int fd, uint32_t flags, drmDevicePtr *device)
{
   return -ENODEV;
}
static int amdgpu_bo_alloc(amdgpu_device_handle dev,
   struct amdgpu_bo_alloc_request *alloc_buffer,
   amdgpu_bo_handle *buf_handle)
{
   return -EINVAL;
}
static int amdgpu_bo_free(amdgpu_bo_handle buf_handle)
{
   return -EINVAL;
}
static int amdgpu_query_buffer_size_alignment(amdgpu_device_handle dev,
   struct amdgpu_buffer_size_alignments
   *info)
{
   return -EINVAL;
}
static int amdgpu_query_firmware_version(amdgpu_device_handle dev, unsigned fw_type,
   unsigned ip_instance, unsigned index,
   uint32_t *version, uint32_t *feature)
{
   return -EINVAL;
}
static int amdgpu_query_hw_ip_info(amdgpu_device_handle dev, unsigned type,
   unsigned ip_instance,
   struct drm_amdgpu_info_hw_ip *info)
{
   return -EINVAL;
}
static int amdgpu_query_heap_info(amdgpu_device_handle dev, uint32_t heap,
   uint32_t flags, struct amdgpu_heap_info *info)
{
   return -EINVAL;
}
static int amdgpu_query_gpu_info(amdgpu_device_handle dev,
   struct amdgpu_gpu_info *info)
{
   return -EINVAL;
}
static int amdgpu_query_info(amdgpu_device_handle dev, unsigned info_id,
   unsigned size, void *value)
{
   return -EINVAL;
}
static int amdgpu_query_sw_info(amdgpu_device_handle dev, enum amdgpu_sw_info info,
   void *value)
{
   return -EINVAL;
}
static int amdgpu_query_gds_info(amdgpu_device_handle dev,
   struct amdgpu_gds_resource_info *gds_info)
{
   return -EINVAL;
}
static int amdgpu_query_video_caps_info(amdgpu_device_handle dev, unsigned cap_type,
                                 unsigned size, void *value)
{
   return -EINVAL;
}
static const char *amdgpu_get_marketing_name(amdgpu_device_handle dev)
{
   return NULL;
}
static intptr_t readlink(const char *path, char *buf, size_t bufsiz)
{
   return -1;
}
#else
#include "drm-uapi/amdgpu_drm.h"
#include <amdgpu.h>
#include <xf86drm.h>
#include <unistd.h>
#endif

#define CIK_TILE_MODE_COLOR_2D 14

static bool has_syncobj(int fd)
{
   uint64_t value;
   if (drmGetCap(fd, DRM_CAP_SYNCOBJ, &value))
      return false;
   return value ? true : false;
}

static bool has_timeline_syncobj(int fd)
{
   uint64_t value;
   if (drmGetCap(fd, DRM_CAP_SYNCOBJ_TIMELINE, &value))
      return false;
   return value ? true : false;
}

static bool has_modifiers(int fd)
{
   uint64_t value;
   if (drmGetCap(fd, DRM_CAP_ADDFB2_MODIFIERS, &value))
      return false;
   return value ? true : false;
}

static uint64_t fix_vram_size(uint64_t size)
{
   /* The VRAM size is underreported, so we need to fix it, because
    * it's used to compute the number of memory modules for harvesting.
    */
   return align64(size, 256 * 1024 * 1024);
}

static bool
has_tmz_support(amdgpu_device_handle dev, struct radeon_info *info, uint32_t ids_flags)
{
   struct amdgpu_bo_alloc_request request = {0};
   int r;
   amdgpu_bo_handle bo;

   if (ids_flags & AMDGPU_IDS_FLAGS_TMZ)
      return true;

   /* AMDGPU_IDS_FLAGS_TMZ is supported starting from drm_minor 40 */
   if (info->drm_minor >= 40)
      return false;

   /* Find out ourselves if TMZ is enabled */
   if (info->gfx_level < GFX9)
      return false;

   if (info->drm_minor < 36)
      return false;

   request.alloc_size = 256;
   request.phys_alignment = 1024;
   request.preferred_heap = AMDGPU_GEM_DOMAIN_VRAM;
   request.flags = AMDGPU_GEM_CREATE_ENCRYPTED;
   r = amdgpu_bo_alloc(dev, &request, &bo);
   if (r)
      return false;
   amdgpu_bo_free(bo);
   return true;
}

static void set_custom_cu_en_mask(struct radeon_info *info)
{
   info->spi_cu_en = ~0;

   const char *cu_env_var = os_get_option("AMD_CU_MASK");
   if (!cu_env_var)
      return;

   int size = strlen(cu_env_var);
   char *str = alloca(size + 1);
   memset(str, 0, size + 1);

   size = 0;

   /* Strip whitespace. */
   for (unsigned src = 0; cu_env_var[src]; src++) {
      if (cu_env_var[src] != ' ' && cu_env_var[src] != '\t' &&
          cu_env_var[src] != '\n' && cu_env_var[src] != '\r') {
         str[size++] = cu_env_var[src];
      }
   }

   /* The following syntax is used, all whitespace is ignored:
    *   ID = [0-9][0-9]*                         ex. base 10 numbers
    *   ID_list = (ID | ID-ID)[, (ID | ID-ID)]*  ex. 0,2-4,7
    *   CU_list = 0x[0-F]* | ID_list             ex. 0x337F OR 0,2-4,7
    *   AMD_CU_MASK = CU_list
    *
    * It's a CU mask within a shader array. It's applied to all shader arrays.
    */
   bool is_good_form = true;
   uint32_t spi_cu_en = 0;

   if (size > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
      str += 2;
      size -= 2;

      for (unsigned i = 0; i < size; i++)
         is_good_form &= isxdigit(str[i]) != 0;

      if (!is_good_form) {
         fprintf(stderr, "amd: invalid AMD_CU_MASK: ill-formed hex value\n");
      } else {
         spi_cu_en = strtol(str, NULL, 16);
      }
   } else {
      /* Parse ID_list. */
      long first = 0, last = -1;

      if (!isdigit(*str)) {
         is_good_form = false;
      } else {
         while (*str) {
            bool comma = false;

            if (isdigit(*str)) {
               first = last = strtol(str, &str, 10);
            } else if (*str == '-') {
               str++;
               /* Parse a digit after a dash. */
               if (isdigit(*str)) {
                  last = strtol(str, &str, 10);
               } else {
                  fprintf(stderr, "amd: invalid AMD_CU_MASK: expected a digit after -\n");
                  is_good_form = false;
                  break;
               }
            } else if (*str == ',') {
               comma = true;
               str++;
               if (!isdigit(*str)) {
                  fprintf(stderr, "amd: invalid AMD_CU_MASK: expected a digit after ,\n");
                  is_good_form = false;
                  break;
               }
            }

            if (comma || !*str) {
               if (first > last) {
                  fprintf(stderr, "amd: invalid AMD_CU_MASK: range not increasing (%li, %li)\n", first, last);
                  is_good_form = false;
                  break;
               }
               if (last > 31) {
                  fprintf(stderr, "amd: invalid AMD_CU_MASK: index too large (%li)\n", last);
                  is_good_form = false;
                  break;
               }

               spi_cu_en |= BITFIELD_RANGE(first, last - first + 1);
               last = -1;
            }
         }
      }
   }

   /* The mask is parsed. Now assign bits to CUs. */
   if (is_good_form) {
      bool error = false;

      /* Clear bits that have no effect. */
      spi_cu_en &= BITFIELD_MASK(info->max_good_cu_per_sa);

      if (!spi_cu_en) {
         fprintf(stderr, "amd: invalid AMD_CU_MASK: at least 1 CU in each SA must be enabled\n");
         error = true;
      }

      if (info->has_graphics) {
         uint32_t min_full_cu_mask = BITFIELD_MASK(info->min_good_cu_per_sa);

         /* The hw ignores all non-compute CU masks if any of them is 0. Disallow that. */
         if ((spi_cu_en & min_full_cu_mask) == 0) {
            fprintf(stderr, "amd: invalid AMD_CU_MASK: at least 1 CU from 0x%x per SA must be "
                            "enabled (SPI limitation)\n", min_full_cu_mask);
            error = true;
         }

         /* We usually disable 1 or 2 CUs for VS and GS, which means at last 1 other CU
          * must be enabled.
          */
         uint32_t cu_mask_ge, unused;
         ac_compute_late_alloc(info, false, false, false, &unused, &cu_mask_ge);
         cu_mask_ge &= min_full_cu_mask;

         if ((spi_cu_en & cu_mask_ge) == 0) {
            fprintf(stderr, "amd: invalid AMD_CU_MASK: at least 1 CU from 0x%x per SA must be "
                            "enabled (late alloc constraint for GE)\n", cu_mask_ge);
            error = true;
         }

         if ((min_full_cu_mask & spi_cu_en & ~cu_mask_ge) == 0) {
            fprintf(stderr, "amd: invalid AMD_CU_MASK: at least 1 CU from 0x%x per SA must be "
                            "enabled (late alloc constraint for PS)\n",
                    min_full_cu_mask & ~cu_mask_ge);
            error = true;
         }
      }

      if (!error) {
         info->spi_cu_en = spi_cu_en;
         info->spi_cu_en_has_effect = spi_cu_en & BITFIELD_MASK(info->max_good_cu_per_sa);
      }
   }
}

static bool ac_query_pci_bus_info(int fd, struct radeon_info *info)
{
   drmDevicePtr devinfo;

   /* Get PCI info. */
   int r = drmGetDevice2(fd, 0, &devinfo);
   if (r) {
      fprintf(stderr, "amdgpu: drmGetDevice2 failed.\n");
      info->pci.valid = false;
      return false;
   }
   info->pci.domain = devinfo->businfo.pci->domain;
   info->pci.bus = devinfo->businfo.pci->bus;
   info->pci.dev = devinfo->businfo.pci->dev;
   info->pci.func = devinfo->businfo.pci->func;
   info->pci.valid = true;

   drmFreeDevice(&devinfo);
   return true;
}

static void handle_env_var_force_family(struct radeon_info *info)
{
   const char *family = debug_get_option("AMD_FORCE_FAMILY", NULL);

   if (!family)
      return;

   for (unsigned i = CHIP_TAHITI; i < CHIP_LAST; i++) {
      if (!strcmp(family, ac_get_llvm_processor_name(i))) {
         /* Override family and gfx_level. */
         info->family = i;
         info->name = "NOOP";
         info->gfx_level = ac_get_gfx_level(i);
         info->family_id = ac_get_family_id(i);
         info->family_overridden = true;
         return;
      }
   }

   fprintf(stderr, "radeonsi: Unknown family: %s\n", family);
   exit(1);
}

bool ac_query_gpu_info(int fd, void *dev_p, struct radeon_info *info,
                       bool require_pci_bus_info)
{
   struct amdgpu_gpu_info amdinfo;
   struct drm_amdgpu_info_device device_info = {0};
   struct amdgpu_buffer_size_alignments alignment_info = {0};
   uint32_t vce_version = 0, vce_feature = 0, uvd_version = 0, uvd_feature = 0;
   int r, i, j;
   amdgpu_device_handle dev = dev_p;

   STATIC_ASSERT(AMDGPU_HW_IP_GFX == AMD_IP_GFX);
   STATIC_ASSERT(AMDGPU_HW_IP_COMPUTE == AMD_IP_COMPUTE);
   STATIC_ASSERT(AMDGPU_HW_IP_DMA == AMD_IP_SDMA);
   STATIC_ASSERT(AMDGPU_HW_IP_UVD == AMD_IP_UVD);
   STATIC_ASSERT(AMDGPU_HW_IP_VCE == AMD_IP_VCE);
   STATIC_ASSERT(AMDGPU_HW_IP_UVD_ENC == AMD_IP_UVD_ENC);
   STATIC_ASSERT(AMDGPU_HW_IP_VCN_DEC == AMD_IP_VCN_DEC);
   STATIC_ASSERT(AMDGPU_HW_IP_VCN_ENC == AMD_IP_VCN_ENC);
   STATIC_ASSERT(AMDGPU_HW_IP_VCN_JPEG == AMD_IP_VCN_JPEG);
   STATIC_ASSERT(AMDGPU_HW_IP_VPE == AMD_IP_VPE);

   handle_env_var_force_family(info);

   if (!ac_query_pci_bus_info(fd, info)) {
      if (require_pci_bus_info)
         return false;
   }

   assert(info->drm_major == 3);
   info->is_amdgpu = true;

   if (info->drm_minor < 27) {
      fprintf(stderr, "amdgpu: DRM version is %u.%u.%u, but this driver is "
                      "only compatible with 3.27.0 (kernel 4.20+) or later.\n",
              info->drm_major, info->drm_minor, info->drm_patchlevel);
      return false;
   }

   /* Query hardware and driver information. */
   r = amdgpu_query_gpu_info(dev, &amdinfo);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_query_gpu_info failed.\n");
      return false;
   }

   r = amdgpu_query_info(dev, AMDGPU_INFO_DEV_INFO, sizeof(device_info), &device_info);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_query_info(dev_info) failed.\n");
      return false;
   }

   r = amdgpu_query_buffer_size_alignment(dev, &alignment_info);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_query_buffer_size_alignment failed.\n");
      return false;
   }

   for (unsigned ip_type = 0; ip_type < AMD_NUM_IP_TYPES; ip_type++) {
      struct drm_amdgpu_info_hw_ip ip_info = {0};

      r = amdgpu_query_hw_ip_info(dev, ip_type, 0, &ip_info);
      if (r || !ip_info.available_rings)
         continue;

      /* Gfx6-8 don't set ip_discovery_version. */
      if (info->drm_minor >= 48 && ip_info.ip_discovery_version) {
         info->ip[ip_type].ver_major = (ip_info.ip_discovery_version >> 16) & 0xff;
         info->ip[ip_type].ver_minor = (ip_info.ip_discovery_version >> 8) & 0xff;
         info->ip[ip_type].ver_rev = ip_info.ip_discovery_version & 0xff;
      } else {
         info->ip[ip_type].ver_major = ip_info.hw_ip_version_major;
         info->ip[ip_type].ver_minor = ip_info.hw_ip_version_minor;

         /* Fix incorrect IP versions reported by the kernel. */
         if (device_info.family == FAMILY_NV &&
             (ASICREV_IS(device_info.external_rev, NAVI10) ||
              ASICREV_IS(device_info.external_rev, NAVI12) ||
              ASICREV_IS(device_info.external_rev, NAVI14)))
            info->ip[AMD_IP_GFX].ver_minor = info->ip[AMD_IP_COMPUTE].ver_minor = 1;
         else if (device_info.family == FAMILY_NV ||
                  device_info.family == FAMILY_VGH ||
                  device_info.family == FAMILY_RMB ||
                  device_info.family == FAMILY_RPL ||
                  device_info.family == FAMILY_MDN)
            info->ip[AMD_IP_GFX].ver_minor = info->ip[AMD_IP_COMPUTE].ver_minor = 3;
      }
      info->ip[ip_type].num_queues = util_bitcount(ip_info.available_rings);

      /* According to the kernel, only SDMA and VPE require 256B alignment, but use it
       * for all queues because the kernel reports wrong limits for some of the queues.
       * This is only space allocation alignment, so it's OK to keep it like this even
       * when it's greater than what the queues require.
       */
      info->ip[ip_type].ib_alignment = MAX3(ip_info.ib_start_alignment,
                                            ip_info.ib_size_alignment, 256);
   }

   /* Set dword padding minus 1. */
   info->ip[AMD_IP_GFX].ib_pad_dw_mask = 0x7;
   info->ip[AMD_IP_COMPUTE].ib_pad_dw_mask = 0x7;
   info->ip[AMD_IP_SDMA].ib_pad_dw_mask = 0xf;
   info->ip[AMD_IP_UVD].ib_pad_dw_mask = 0xf;
   info->ip[AMD_IP_VCE].ib_pad_dw_mask = 0x3f;
   info->ip[AMD_IP_UVD_ENC].ib_pad_dw_mask = 0x3f;
   info->ip[AMD_IP_VCN_DEC].ib_pad_dw_mask = 0xf;
   info->ip[AMD_IP_VCN_ENC].ib_pad_dw_mask = 0x3f;
   info->ip[AMD_IP_VCN_JPEG].ib_pad_dw_mask = 0xf;
   info->ip[AMD_IP_VPE].ib_pad_dw_mask = 0xf;

   /* Only require gfx or compute. */
   if (!info->ip[AMD_IP_GFX].num_queues && !info->ip[AMD_IP_COMPUTE].num_queues) {
      fprintf(stderr, "amdgpu: failed to find gfx or compute.\n");
      return false;
   }

   r = amdgpu_query_firmware_version(dev, AMDGPU_INFO_FW_GFX_ME, 0, 0, &info->me_fw_version,
                                     &info->me_fw_feature);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_query_firmware_version(me) failed.\n");
      return false;
   }

   r = amdgpu_query_firmware_version(dev, AMDGPU_INFO_FW_GFX_MEC, 0, 0, &info->mec_fw_version,
                                     &info->mec_fw_feature);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_query_firmware_version(mec) failed.\n");
      return false;
   }

   r = amdgpu_query_firmware_version(dev, AMDGPU_INFO_FW_GFX_PFP, 0, 0, &info->pfp_fw_version,
                                     &info->pfp_fw_feature);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_query_firmware_version(pfp) failed.\n");
      return false;
   }

   r = amdgpu_query_firmware_version(dev, AMDGPU_INFO_FW_UVD, 0, 0, &uvd_version, &uvd_feature);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_query_firmware_version(uvd) failed.\n");
      return false;
   }

   r = amdgpu_query_firmware_version(dev, AMDGPU_INFO_FW_VCE, 0, 0, &vce_version, &vce_feature);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_query_firmware_version(vce) failed.\n");
      return false;
   }

   r = amdgpu_query_sw_info(dev, amdgpu_sw_info_address32_hi, &info->address32_hi);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_query_sw_info(address32_hi) failed.\n");
      return false;
   }

   struct drm_amdgpu_memory_info meminfo = {0};

   r = amdgpu_query_info(dev, AMDGPU_INFO_MEMORY, sizeof(meminfo), &meminfo);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_query_info(memory) failed.\n");
      return false;
   }

   /* Note: usable_heap_size values can be random and can't be relied on. */
   info->gart_size_kb = DIV_ROUND_UP(meminfo.gtt.total_heap_size, 1024);
   info->vram_size_kb = DIV_ROUND_UP(fix_vram_size(meminfo.vram.total_heap_size), 1024);
   info->vram_vis_size_kb = DIV_ROUND_UP(meminfo.cpu_accessible_vram.total_heap_size, 1024);

   if (info->drm_minor >= 41) {
      amdgpu_query_video_caps_info(dev, AMDGPU_INFO_VIDEO_CAPS_DECODE,
                                   sizeof(info->dec_caps), &(info->dec_caps));
      amdgpu_query_video_caps_info(dev, AMDGPU_INFO_VIDEO_CAPS_ENCODE,
                                   sizeof(info->enc_caps), &(info->enc_caps));
   }

   /* Add some margin of error, though this shouldn't be needed in theory. */
   info->all_vram_visible = info->vram_size_kb * 0.9 < info->vram_vis_size_kb;

   /* Set chip identification. */
   info->pci_id = device_info.device_id;
   info->pci_rev_id = device_info.pci_rev;
   info->vce_harvest_config = device_info.vce_harvest_config;

#define identify_chip2(asic, chipname)                                                             \
   if (ASICREV_IS(device_info.external_rev, asic)) {                                             \
      info->family = CHIP_##chipname;                                                              \
      info->name = #chipname;                                                                      \
   }
#define identify_chip(chipname) identify_chip2(chipname, chipname)

   if (!info->family_overridden) {
      switch (device_info.family) {
      case FAMILY_SI:
         identify_chip(TAHITI);
         identify_chip(PITCAIRN);
         identify_chip2(CAPEVERDE, VERDE);
         identify_chip(OLAND);
         identify_chip(HAINAN);
         break;
      case FAMILY_CI:
         identify_chip(BONAIRE);
         identify_chip(HAWAII);
         break;
      case FAMILY_KV:
         identify_chip2(SPECTRE, KAVERI);
         identify_chip2(SPOOKY, KAVERI);
         identify_chip2(KALINDI, KABINI);
         identify_chip2(GODAVARI, KABINI);
         break;
      case FAMILY_VI:
         identify_chip(ICELAND);
         identify_chip(TONGA);
         identify_chip(FIJI);
         identify_chip(POLARIS10);
         identify_chip(POLARIS11);
         identify_chip(POLARIS12);
         identify_chip(VEGAM);
         break;
      case FAMILY_CZ:
         identify_chip(CARRIZO);
         identify_chip(STONEY);
         break;
      case FAMILY_AI:
         identify_chip(VEGA10);
         identify_chip(VEGA12);
         identify_chip(VEGA20);
         identify_chip(MI100);
         identify_chip(MI200);
         identify_chip(GFX940);
         break;
      case FAMILY_RV:
         identify_chip(RAVEN);
         identify_chip(RAVEN2);
         identify_chip(RENOIR);
         break;
      case FAMILY_NV:
         identify_chip(NAVI10);
         identify_chip(NAVI12);
         identify_chip(NAVI14);
         identify_chip(NAVI21);
         identify_chip(NAVI22);
         identify_chip(NAVI23);
         identify_chip(NAVI24);
         break;
      case FAMILY_VGH:
         identify_chip(VANGOGH);
         break;
      case FAMILY_RMB:
         identify_chip(REMBRANDT);
         break;
      case FAMILY_RPL:
         identify_chip2(RAPHAEL, RAPHAEL_MENDOCINO);
         break;
      case FAMILY_MDN:
         identify_chip2(MENDOCINO, RAPHAEL_MENDOCINO);
         break;
      case FAMILY_NV3:
         identify_chip(NAVI31);
         identify_chip(NAVI32);
         identify_chip(NAVI33);
         break;
      case FAMILY_GFX1103:
         identify_chip(GFX1103_R1);
         identify_chip(GFX1103_R2);
         break;
      case FAMILY_GFX1150:
         identify_chip(GFX1150);
         break;
      }

      if (info->ip[AMD_IP_GFX].ver_major == 11 && info->ip[AMD_IP_GFX].ver_minor == 5)
         info->gfx_level = GFX11_5;
      else if (info->ip[AMD_IP_GFX].ver_major == 11 && info->ip[AMD_IP_GFX].ver_minor == 0)
         info->gfx_level = GFX11;
      else if (info->ip[AMD_IP_GFX].ver_major == 10 && info->ip[AMD_IP_GFX].ver_minor == 3)
         info->gfx_level = GFX10_3;
      else if (info->ip[AMD_IP_GFX].ver_major == 10 && info->ip[AMD_IP_GFX].ver_minor == 1)
         info->gfx_level = GFX10;
      else if (info->ip[AMD_IP_GFX].ver_major == 9 || info->ip[AMD_IP_COMPUTE].ver_major == 9)
         info->gfx_level = GFX9;
      else if (info->ip[AMD_IP_GFX].ver_major == 8)
         info->gfx_level = GFX8;
      else if (info->ip[AMD_IP_GFX].ver_major == 7)
         info->gfx_level = GFX7;
      else if (info->ip[AMD_IP_GFX].ver_major == 6)
         info->gfx_level = GFX6;
      else {
         fprintf(stderr, "amdgpu: Unknown gfx version: %u.%u\n",
                 info->ip[AMD_IP_GFX].ver_major, info->ip[AMD_IP_GFX].ver_minor);
         return false;
      }

      info->family_id = device_info.family;
      info->chip_external_rev = device_info.external_rev;
      info->chip_rev = device_info.chip_rev;
      info->marketing_name = amdgpu_get_marketing_name(dev);
      info->is_pro_graphics = info->marketing_name && (strstr(info->marketing_name, "Pro") ||
                                                       strstr(info->marketing_name, "PRO") ||
                                                       strstr(info->marketing_name, "Frontier"));
   }

   if (!info->name) {
      fprintf(stderr, "amdgpu: unknown (family_id, chip_external_rev): (%u, %u)\n",
              device_info.family, device_info.external_rev);
      return false;
   }

   memset(info->lowercase_name, 0, sizeof(info->lowercase_name));
   for (unsigned i = 0; info->name[i] && i < ARRAY_SIZE(info->lowercase_name) - 1; i++)
      info->lowercase_name[i] = tolower(info->name[i]);

   char proc_fd[64];
   snprintf(proc_fd, sizeof(proc_fd), "/proc/self/fd/%u", fd);
   UNUSED int _result = readlink(proc_fd, info->dev_filename, sizeof(info->dev_filename));

#define VCN_IP_VERSION(mj, mn, rv) (((mj) << 16) | ((mn) << 8) | (rv))

   for (unsigned i = AMD_IP_VCN_DEC; i <= AMD_IP_VCN_JPEG; ++i) {
      if (!info->ip[i].num_queues)
         continue;

      switch(VCN_IP_VERSION(info->ip[i].ver_major,
                            info->ip[i].ver_minor,
                            info->ip[i].ver_rev)) {
      case VCN_IP_VERSION(1, 0, 0):
         info->vcn_ip_version = VCN_1_0_0;
         break;
      case VCN_IP_VERSION(1, 0, 1):
         info->vcn_ip_version = VCN_1_0_1;
         break;
      case VCN_IP_VERSION(2, 0, 0):
         info->vcn_ip_version = VCN_2_0_0;
         break;
      case VCN_IP_VERSION(2, 0, 2):
         info->vcn_ip_version = VCN_2_0_2;
         break;
      case VCN_IP_VERSION(2, 0, 3):
         info->vcn_ip_version = VCN_2_0_3;
         break;
      case VCN_IP_VERSION(2, 2, 0):
         info->vcn_ip_version = VCN_2_2_0;
         break;
      case VCN_IP_VERSION(2, 5, 0):
         info->vcn_ip_version = VCN_2_5_0;
         break;
      case VCN_IP_VERSION(2, 6, 0):
         info->vcn_ip_version = VCN_2_6_0;
         break;
      case VCN_IP_VERSION(3, 0, 0):
         /* Navi24 version need to be revised if it fallbacks to the older way
	  * with default version as 3.0.0, since Navi24 has different feature
	  * sets from other VCN3 family */
         info->vcn_ip_version = (info->family != CHIP_NAVI24) ? VCN_3_0_0 : VCN_3_0_33;
         break;
      case VCN_IP_VERSION(3, 0, 2):
         info->vcn_ip_version = VCN_3_0_2;
         break;
      case VCN_IP_VERSION(3, 0, 16):
         info->vcn_ip_version = VCN_3_0_16;
         break;
      case VCN_IP_VERSION(3, 0, 33):
         info->vcn_ip_version = VCN_3_0_33;
         break;
      case VCN_IP_VERSION(3, 1, 1):
         info->vcn_ip_version = VCN_3_1_1;
         break;
      case VCN_IP_VERSION(3, 1, 2):
         info->vcn_ip_version = VCN_3_1_2;
         break;
      case VCN_IP_VERSION(4, 0, 0):
         info->vcn_ip_version = VCN_4_0_0;
         break;
      case VCN_IP_VERSION(4, 0, 2):
         info->vcn_ip_version = VCN_4_0_2;
         break;
      case VCN_IP_VERSION(4, 0, 3):
         info->vcn_ip_version = VCN_4_0_3;
         break;
      case VCN_IP_VERSION(4, 0, 4):
         info->vcn_ip_version = VCN_4_0_4;
         break;
      case VCN_IP_VERSION(4, 0, 5):
         info->vcn_ip_version = VCN_4_0_5;
         break;
      default:
         info->vcn_ip_version = VCN_UNKNOWN;
      }
      break;
   }

   /* Set which chips have dedicated VRAM. */
   info->has_dedicated_vram = !(device_info.ids_flags & AMDGPU_IDS_FLAGS_FUSION);

   /* The kernel can split large buffers in VRAM but not in GTT, so large
    * allocations can fail or cause buffer movement failures in the kernel.
    */
   if (info->has_dedicated_vram)
      info->max_heap_size_kb = info->vram_size_kb;
   else
      info->max_heap_size_kb = info->gart_size_kb;

   info->vram_type = device_info.vram_type;
   info->memory_bus_width = device_info.vram_bit_width;

   /* Set which chips have uncached device memory. */
   info->has_l2_uncached = info->gfx_level >= GFX9;

   /* Set hardware information. */
   /* convert the shader/memory clocks from KHz to MHz */
   info->max_gpu_freq_mhz = device_info.max_engine_clock / 1000;
   info->memory_freq_mhz_effective = info->memory_freq_mhz = device_info.max_memory_clock / 1000;
   info->max_tcc_blocks = device_info.num_tcc_blocks;
   info->max_se = device_info.num_shader_engines;
   info->max_sa_per_se = device_info.num_shader_arrays_per_engine;
   info->num_cu_per_sh = device_info.num_cu_per_sh;
   info->uvd_fw_version = info->ip[AMD_IP_UVD].num_queues ? uvd_version : 0;
   info->vce_fw_version = info->ip[AMD_IP_VCE].num_queues ? vce_version : 0;

   info->memory_freq_mhz_effective *= ac_memory_ops_per_clock(info->vram_type);

   info->has_userptr = true;
   info->has_syncobj = has_syncobj(fd);
   info->has_timeline_syncobj = has_timeline_syncobj(fd);
   info->has_fence_to_handle = info->has_syncobj;
   info->has_local_buffers = true;
   info->has_bo_metadata = true;
   info->has_eqaa_surface_allocator = info->gfx_level < GFX11;
   /* Disable sparse mappings on GFX6 due to VM faults in CP DMA. Enable them once
    * these faults are mitigated in software.
    */
   info->has_sparse_vm_mappings = info->gfx_level >= GFX7;
   info->has_scheduled_fence_dependency = info->drm_minor >= 28;
   info->has_gang_submit = info->drm_minor >= 49;
   info->has_gpuvm_fault_query = info->drm_minor >= 55;
   info->has_tmz_support = has_tmz_support(dev, info, device_info.ids_flags);
   info->kernel_has_modifiers = has_modifiers(fd);
   info->uses_kernel_cu_mask = false; /* Not implemented in the kernel. */
   info->has_graphics = info->ip[AMD_IP_GFX].num_queues > 0;

   info->pa_sc_tile_steering_override = device_info.pa_sc_tile_steering_override;
   info->max_render_backends = device_info.num_rb_pipes;
   /* The value returned by the kernel driver was wrong. */
   if (info->family == CHIP_KAVERI)
      info->max_render_backends = 2;

   info->clock_crystal_freq = device_info.gpu_counter_freq;
   if (!info->clock_crystal_freq) {
      fprintf(stderr, "amdgpu: clock crystal frequency is 0, timestamps will be wrong\n");
      info->clock_crystal_freq = 1;
   }
   if (info->gfx_level >= GFX10) {
      info->tcc_cache_line_size = 128;

      if (info->drm_minor >= 35) {
         info->num_tcc_blocks = info->max_tcc_blocks - util_bitcount64(device_info.tcc_disabled_mask);
      } else {
         /* This is a hack, but it's all we can do without a kernel upgrade. */
         info->num_tcc_blocks = info->vram_size_kb / (512 * 1024);
         if (info->num_tcc_blocks > info->max_tcc_blocks)
            info->num_tcc_blocks /= 2;
      }
   } else {
      if (!info->has_graphics && info->family >= CHIP_MI200)
         info->tcc_cache_line_size = 128;
      else
         info->tcc_cache_line_size = 64;

      info->num_tcc_blocks = info->max_tcc_blocks;
   }

   info->tcc_rb_non_coherent = !util_is_power_of_two_or_zero(info->num_tcc_blocks);

   if (info->drm_minor >= 52) {
      info->sqc_inst_cache_size = device_info.sqc_inst_cache_size * 1024;
      info->sqc_scalar_cache_size = device_info.sqc_data_cache_size * 1024;
      info->num_sqc_per_wgp = device_info.num_sqc_per_wgp;
   }

   /* Firmware wrongly reports 0 bytes of MALL being present on Navi33.
    * Work around this by manually computing cache sizes. */
   if (info->gfx_level >= GFX11 && info->drm_minor >= 52 && info->family != CHIP_NAVI33) {
      info->tcp_cache_size = device_info.tcp_cache_size * 1024;
      info->l1_cache_size = device_info.gl1c_cache_size * 1024;
      info->l2_cache_size = device_info.gl2c_cache_size * 1024;
      info->l3_cache_size_mb = DIV_ROUND_UP(device_info.mall_size, 1024 * 1024);
   } else {
      if (info->gfx_level >= GFX11) {
         info->tcp_cache_size = 32768;
         info->l1_cache_size = 256 * 1024;
      } else {
         info->tcp_cache_size = 16384;
         info->l1_cache_size = 128 * 1024;
      }

      if (info->gfx_level >= GFX10_3 && info->has_dedicated_vram) {
         info->l3_cache_size_mb = info->num_tcc_blocks *
                                  (info->family == CHIP_NAVI21 ||
                                   info->family == CHIP_NAVI22 ? 8 : 4);
      }

      switch (info->family) {
      case CHIP_TAHITI:
      case CHIP_PITCAIRN:
      case CHIP_OLAND:
      case CHIP_HAWAII:
      case CHIP_KABINI:
      case CHIP_TONGA:
      case CHIP_STONEY:
      case CHIP_RAVEN2:
         info->l2_cache_size = info->num_tcc_blocks * 64 * 1024;
         break;
      case CHIP_VERDE:
      case CHIP_HAINAN:
      case CHIP_BONAIRE:
      case CHIP_KAVERI:
      case CHIP_ICELAND:
      case CHIP_CARRIZO:
      case CHIP_FIJI:
      case CHIP_POLARIS12:
      case CHIP_VEGAM:
      case CHIP_RAPHAEL_MENDOCINO:
         info->l2_cache_size = info->num_tcc_blocks * 128 * 1024;
         break;
      default:
         info->l2_cache_size = info->num_tcc_blocks * 256 * 1024;
         break;
      case CHIP_REMBRANDT:
      case CHIP_GFX1103_R1:
         info->l2_cache_size = info->num_tcc_blocks * 512 * 1024;
         break;
      }
   }

   info->mc_arb_ramcfg = amdinfo.mc_arb_ramcfg;
   info->gb_addr_config = amdinfo.gb_addr_cfg;
   if (info->gfx_level >= GFX9) {
      info->num_tile_pipes = 1 << G_0098F8_NUM_PIPES(info->gb_addr_config);
      info->pipe_interleave_bytes = 256 << G_0098F8_PIPE_INTERLEAVE_SIZE_GFX9(info->gb_addr_config);
   } else {
      unsigned pipe_config = G_009910_PIPE_CONFIG(amdinfo.gb_tile_mode[CIK_TILE_MODE_COLOR_2D]);
      info->num_tile_pipes = ac_pipe_config_to_num_pipes(pipe_config);
      info->pipe_interleave_bytes = 256 << G_0098F8_PIPE_INTERLEAVE_SIZE_GFX6(info->gb_addr_config);
   }
   info->r600_has_virtual_memory = true;

   /* LDS is 64KB per CU (4 SIMDs on GFX6-9), which is 16KB per SIMD (usage above
    * 16KB makes some SIMDs unoccupied).
    *
    * GFX10+: LDS is 128KB in WGP mode and 64KB in CU mode. Assume the WGP mode is used.
    * GFX7+: Workgroups can use up to 64KB.
    * GFX6: There is 64KB LDS per CU, but a workgroup can only use up to 32KB.
    */
   info->lds_size_per_workgroup = info->gfx_level >= GFX10  ? 128 * 1024
                                  : info->gfx_level >= GFX7 ? 64 * 1024
                                                            : 32 * 1024;

   /* lds_encode_granularity is the block size used for encoding registers.
    * lds_alloc_granularity is what the hardware will align the LDS size to.
    */
   info->lds_encode_granularity = info->gfx_level >= GFX7 ? 128 * 4 : 64 * 4;
   info->lds_alloc_granularity = info->gfx_level >= GFX10_3 ? 256 * 4 : info->lds_encode_granularity;

   /* The mere presence of CLEAR_STATE in the IB causes random GPU hangs
    * on GFX6. Some CLEAR_STATE cause asic hang on radeon kernel, etc.
    * SPI_VS_OUT_CONFIG. So only enable GFX7 CLEAR_STATE on amdgpu kernel.
    */
   info->has_clear_state = info->gfx_level >= GFX7;

   info->has_distributed_tess =
      info->gfx_level >= GFX10 || (info->gfx_level >= GFX8 && info->max_se >= 2);

   info->has_dcc_constant_encode =
      info->family == CHIP_RAVEN2 || info->family == CHIP_RENOIR || info->gfx_level >= GFX10;

   info->has_rbplus = info->family == CHIP_STONEY || info->gfx_level >= GFX9;

   /* Some chips have RB+ registers, but don't support RB+. Those must
    * always disable it.
    */
   info->rbplus_allowed =
      info->has_rbplus &&
      (info->family == CHIP_STONEY || info->family == CHIP_VEGA12 || info->family == CHIP_RAVEN ||
       info->family == CHIP_RAVEN2 || info->family == CHIP_RENOIR || info->gfx_level >= GFX10_3);

   info->has_out_of_order_rast =
      info->gfx_level >= GFX8 && info->gfx_level <= GFX9 && info->max_se >= 2;

   /* Whether chips support double rate packed math instructions. */
   info->has_packed_math_16bit = info->gfx_level >= GFX9;

   /* Whether chips support dot product instructions. A subset of these support a smaller
    * instruction encoding which accumulates with the destination.
    */
   info->has_accelerated_dot_product =
      info->family == CHIP_VEGA20 ||
      (info->family >= CHIP_MI100 && info->family != CHIP_NAVI10);

   /* TODO: Figure out how to use LOAD_CONTEXT_REG on GFX6-GFX7. */
   info->has_load_ctx_reg_pkt =
      info->gfx_level >= GFX9 || (info->gfx_level >= GFX8 && info->me_fw_feature >= 41);

   info->cpdma_prefetch_writes_memory = info->gfx_level <= GFX8;

   info->has_gfx9_scissor_bug = info->family == CHIP_VEGA10 || info->family == CHIP_RAVEN;

   info->has_tc_compat_zrange_bug = info->gfx_level >= GFX8 && info->gfx_level <= GFX9;

   info->has_small_prim_filter_sample_loc_bug =
      (info->family >= CHIP_POLARIS10 && info->family <= CHIP_POLARIS12) ||
      info->family == CHIP_VEGA10 || info->family == CHIP_RAVEN;

   info->has_ls_vgpr_init_bug = info->family == CHIP_VEGA10 || info->family == CHIP_RAVEN;

   /* DB_DFSM_CONTROL.POPS_DRAIN_PS_ON_OVERLAP must be enabled for 8 or more coverage or
    * depth/stencil samples with POPS (PAL waMiscPopsMissedOverlap).
    */
   info->has_pops_missed_overlap_bug = info->family == CHIP_VEGA10 || info->family == CHIP_RAVEN;

   /* Drawing from 0-sized index buffers causes hangs on gfx10. */
   info->has_zero_index_buffer_bug = info->gfx_level == GFX10;

   /* Whether chips are affected by the image load/sample/gather hw bug when
    * DCC is enabled (ie. WRITE_COMPRESS_ENABLE should be 0).
    */
   info->has_image_load_dcc_bug = info->family == CHIP_NAVI23 ||
                                  info->family == CHIP_VANGOGH ||
                                  info->family == CHIP_REMBRANDT;

   /* DB has a bug when ITERATE_256 is set to 1 that can cause a hang. The
    * workaround is to set DECOMPRESS_ON_Z_PLANES to 2 for 4X MSAA D/S images.
    */
   info->has_two_planes_iterate256_bug = info->gfx_level == GFX10;

   /* GFX10+Navi21: NGG->legacy transitions require VGT_FLUSH. */
   info->has_vgt_flush_ngg_legacy_bug = info->gfx_level == GFX10 ||
                                        info->family == CHIP_NAVI21;

   /* First Navi2x chips have a hw bug that doesn't allow to write
    * depth/stencil from a FS for multi-pixel fragments.
    */
   info->has_vrs_ds_export_bug = info->family == CHIP_NAVI21 ||
                                 info->family == CHIP_NAVI22 ||
                                 info->family == CHIP_VANGOGH;

   /* HW bug workaround when CS threadgroups > 256 threads and async compute
    * isn't used, i.e. only one compute job can run at a time.  If async
    * compute is possible, the threadgroup size must be limited to 256 threads
    * on all queues to avoid the bug.
    * Only GFX6 and certain GFX7 chips are affected.
    *
    * FIXME: RADV doesn't limit the number of threads for async compute.
    */
   info->has_cs_regalloc_hang_bug = info->gfx_level == GFX6 ||
                                    info->family == CHIP_BONAIRE ||
                                    info->family == CHIP_KABINI;

   /* HW bug workaround with async compute dispatches when threadgroup > 4096.
    * The workaround is to change the "threadgroup" dimension mode to "thread"
    * dimension mode.
    */
   info->has_async_compute_threadgroup_bug = info->family == CHIP_ICELAND ||
                                             info->family == CHIP_TONGA;

   /* GFX7 CP requires 32 bytes alignment for the indirect buffer arguments on
    * the compute queue.
    */
   info->has_async_compute_align32_bug = info->gfx_level == GFX7;

   /* Support for GFX10.3 was added with F32_ME_FEATURE_VERSION_31 but the
    * feature version wasn't bumped.
    */
   info->has_32bit_predication = (info->gfx_level >= GFX10 &&
                                  info->me_fw_feature >= 32) ||
                                 (info->gfx_level == GFX9 &&
                                  info->me_fw_feature >= 52);

   /* Firmware bug with DISPATCH_TASKMESH_INDIRECT_MULTI_ACE packets.
    * On old MEC FW versions, it hangs the GPU when indirect count is zero.
    */
   info->has_taskmesh_indirect0_bug = info->gfx_level == GFX10_3 &&
                                      info->mec_fw_version < 100;

   info->has_export_conflict_bug = info->gfx_level == GFX11;

   /* Convert the SDMA version in the current GPU to an enum. */
   info->sdma_ip_version =
      (enum sdma_version)SDMA_VERSION_VALUE(info->ip[AMD_IP_SDMA].ver_major,
                                            info->ip[AMD_IP_SDMA].ver_minor);

   /* SDMA v1.0-3.x (GFX6-8) can't ignore page faults on unmapped sparse resources. */
   info->sdma_supports_sparse = info->sdma_ip_version >= SDMA_4_0;

   /* SDMA v5.0+ (GFX10+) supports DCC and HTILE, but Navi 10 has issues with it according to PAL. */
   info->sdma_supports_compression = info->sdma_ip_version >= SDMA_5_0 && info->family != CHIP_NAVI10;

   /* Get the number of good compute units. */
   info->num_cu = 0;
   for (i = 0; i < info->max_se; i++) {
      for (j = 0; j < info->max_sa_per_se; j++) {
         if (info->gfx_level >= GFX11) {
            assert(info->max_sa_per_se <= 2);
            info->cu_mask[i][j] = device_info.cu_bitmap[i % 4][(i / 4) * 2 + j];
         } else if (info->family == CHIP_MI100) {
            /* The CU bitmap in amd gpu info structure is
             * 4x4 size array, and it's usually suitable for Vega
             * ASICs which has 4*2 SE/SA layout.
             * But for MI100, SE/SA layout is changed to 8*1.
             * To mostly reduce the impact, we make it compatible
             * with current bitmap array as below:
             *    SE4 --> cu_bitmap[0][1]
             *    SE5 --> cu_bitmap[1][1]
             *    SE6 --> cu_bitmap[2][1]
             *    SE7 --> cu_bitmap[3][1]
             */
            assert(info->max_sa_per_se == 1);
            info->cu_mask[i][0] = device_info.cu_bitmap[i % 4][i / 4];
         } else {
            info->cu_mask[i][j] = device_info.cu_bitmap[i][j];
         }
         info->num_cu += util_bitcount(info->cu_mask[i][j]);
      }
   }

   /* Derive the number of enabled SEs from the CU mask. */
   if (info->gfx_level >= GFX10_3 && info->max_se > 1) {
      info->num_se = 0;

      for (unsigned se = 0; se < info->max_se; se++) {
         for (unsigned sa = 0; sa < info->max_sa_per_se; sa++) {
            if (info->cu_mask[se][sa]) {
               info->num_se++;
               break;
            }
         }
      }
   } else {
      /* GFX10 and older always enable all SEs because they don't support SE harvesting. */
      info->num_se = info->max_se;
   }

   /* On GFX10, only whole WGPs (in units of 2 CUs) can be disabled,
    * and max - min <= 2.
    */
   unsigned cu_group = info->gfx_level >= GFX10 ? 2 : 1;
   info->max_good_cu_per_sa =
      DIV_ROUND_UP(info->num_cu, (info->num_se * info->max_sa_per_se * cu_group)) *
      cu_group;
   info->min_good_cu_per_sa =
      (info->num_cu / (info->num_se * info->max_sa_per_se * cu_group)) * cu_group;

   memcpy(info->si_tile_mode_array, amdinfo.gb_tile_mode, sizeof(amdinfo.gb_tile_mode));

   info->enabled_rb_mask = device_info.enabled_rb_pipes_mask;
   if (info->drm_minor >= 52)
      info->enabled_rb_mask |= (uint64_t)device_info.enabled_rb_pipes_mask_hi << 32;

   memcpy(info->cik_macrotile_mode_array, amdinfo.gb_macro_tile_mode,
          sizeof(amdinfo.gb_macro_tile_mode));

   info->pte_fragment_size = alignment_info.size_local;
   info->gart_page_size = alignment_info.size_remote;

   if (info->gfx_level == GFX6)
      info->gfx_ib_pad_with_type2 = true;

   if (info->gfx_level >= GFX11) {
      /* With num_cu = 4 in gfx11 measured power for idle, video playback and observed
       * power savings, hence enable dcc with retile for gfx11 with num_cu >= 4.
       */
       info->use_display_dcc_with_retile_blit = info->num_cu >= 4;
   } else if (info->gfx_level == GFX10_3) {
      /* Displayable DCC with retiling is known to increase power consumption on Raphael
       * and Mendocino, so disable it on the smallest APUs. We need a proof that
       * displayable DCC doesn't regress bigger chips in the same way.
       */
      info->use_display_dcc_with_retile_blit = info->num_cu > 4;
   } else if (info->gfx_level == GFX9 && !info->has_dedicated_vram &&
              info->drm_minor >= 31) {
      if (info->max_render_backends == 1) {
         info->use_display_dcc_unaligned = true;
      } else {
         /* there may be power increase for small APUs with less num_cu. */
         info->use_display_dcc_with_retile_blit = info->num_cu > 4;
      }
   }

   info->has_stable_pstate = info->drm_minor >= 45;

   if (info->gfx_level >= GFX11) {
      info->pc_lines = 1024;
      info->pbb_max_alloc_count = 16; /* minimum is 2, maximum is 256 */
   } else if (info->gfx_level >= GFX9 && info->has_graphics) {
      unsigned pc_lines = 0;

      switch (info->family) {
      case CHIP_VEGA10:
      case CHIP_VEGA12:
      case CHIP_VEGA20:
         pc_lines = 2048;
         break;
      case CHIP_RAVEN:
      case CHIP_RAVEN2:
      case CHIP_RENOIR:
      case CHIP_NAVI10:
      case CHIP_NAVI12:
      case CHIP_NAVI21:
      case CHIP_NAVI22:
      case CHIP_NAVI23:
         pc_lines = 1024;
         break;
      case CHIP_NAVI14:
      case CHIP_NAVI24:
         pc_lines = 512;
         break;
      case CHIP_VANGOGH:
      case CHIP_REMBRANDT:
      case CHIP_RAPHAEL_MENDOCINO:
         pc_lines = 256;
         break;
      default:
         assert(0);
      }

      info->pc_lines = pc_lines;

      if (info->gfx_level >= GFX10) {
         info->pbb_max_alloc_count = pc_lines / 3;
      } else {
         info->pbb_max_alloc_count = MIN2(128, pc_lines / (4 * info->max_se));
      }
   }

   if (info->gfx_level >= GFX10_3)
      info->max_waves_per_simd = 16;
   else if (info->gfx_level == GFX10)
      info->max_waves_per_simd = 20;
   else if (info->family >= CHIP_POLARIS10 && info->family <= CHIP_VEGAM)
      info->max_waves_per_simd = 8;
   else
      info->max_waves_per_simd = 10;

   if (info->gfx_level >= GFX10) {
      info->num_physical_sgprs_per_simd = 128 * info->max_waves_per_simd;
      info->min_sgpr_alloc = 128;
      info->sgpr_alloc_granularity = 128;
   } else if (info->gfx_level >= GFX8) {
      info->num_physical_sgprs_per_simd = 800;
      info->min_sgpr_alloc = 16;
      info->sgpr_alloc_granularity = 16;
   } else {
      info->num_physical_sgprs_per_simd = 512;
      info->min_sgpr_alloc = 8;
      info->sgpr_alloc_granularity = 8;
   }

   info->has_3d_cube_border_color_mipmap = info->has_graphics || info->family == CHIP_MI100;
   info->has_image_opcodes = debug_get_bool_option("AMD_IMAGE_OPCODES",
                                                   info->has_graphics || info->family < CHIP_GFX940);
   info->never_stop_sq_perf_counters = info->gfx_level == GFX10 ||
                                       info->gfx_level == GFX10_3;
   info->never_send_perfcounter_stop = info->gfx_level == GFX11;
   info->has_sqtt_rb_harvest_bug = (info->family == CHIP_NAVI23 ||
                                    info->family == CHIP_NAVI24 ||
                                    info->family == CHIP_REMBRANDT ||
                                    info->family == CHIP_VANGOGH) &&
                                   util_bitcount64(info->enabled_rb_mask) !=
                                   info->max_render_backends;

   /* On GFX10.3, the polarity of AUTO_FLUSH_MODE is inverted. */
   info->has_sqtt_auto_flush_mode_bug = info->gfx_level == GFX10_3;

   info->max_sgpr_alloc = info->family == CHIP_TONGA || info->family == CHIP_ICELAND ? 96 : 104;

   if (!info->has_graphics && info->family >= CHIP_MI200) {
      info->min_wave64_vgpr_alloc = 8;
      info->max_vgpr_alloc = 512;
      info->wave64_vgpr_alloc_granularity = 8;
   } else {
      info->min_wave64_vgpr_alloc = 4;
      info->max_vgpr_alloc = 256;
      info->wave64_vgpr_alloc_granularity = 4;
   }

   /* Some GPU info was broken before DRM 3.45.0. */
   if (info->drm_minor >= 45 && device_info.num_shader_visible_vgprs) {
      /* The Gfx10 VGPR count is in Wave32, so divide it by 2 for Wave64.
       * Gfx6-9 numbers are in Wave64.
       */
      if (info->gfx_level >= GFX10)
         info->num_physical_wave64_vgprs_per_simd = device_info.num_shader_visible_vgprs / 2;
      else
         info->num_physical_wave64_vgprs_per_simd = device_info.num_shader_visible_vgprs;
   } else if (info->gfx_level >= GFX10) {
      info->num_physical_wave64_vgprs_per_simd = 512;
   } else {
      info->num_physical_wave64_vgprs_per_simd = 256;
   }

   info->num_simd_per_compute_unit = info->gfx_level >= GFX10 ? 2 : 4;

   /* BIG_PAGE is supported since gfx10.3 and requires VRAM. VRAM is only guaranteed
    * with AMDGPU_GEM_CREATE_DISCARDABLE. DISCARDABLE was added in DRM 3.47.0.
    */
   info->discardable_allows_big_page = info->gfx_level >= GFX10_3 &&
                                       info->has_dedicated_vram &&
                                       info->drm_minor >= 47;

   /* The maximum number of scratch waves. The number is only a function of the number of CUs.
    * It should be large enough to hold at least 1 threadgroup. Use the minimum per-SA CU count.
    *
    * We can decrease the number to make it fit into the infinity cache.
    */
   const unsigned max_waves_per_tg = 32; /* 1024 threads in Wave32 */
   info->max_scratch_waves = MAX2(32 * info->min_good_cu_per_sa * info->max_sa_per_se * info->num_se,
                                  max_waves_per_tg);
   info->num_rb = util_bitcount64(info->enabled_rb_mask);
   info->max_gflops = (info->gfx_level >= GFX11 ? 256 : 128) * info->num_cu * info->max_gpu_freq_mhz / 1000;
   info->memory_bandwidth_gbps = DIV_ROUND_UP(info->memory_freq_mhz_effective * info->memory_bus_width / 8, 1000);
   info->has_pcie_bandwidth_info = info->drm_minor >= 51;

   if (info->has_pcie_bandwidth_info) {
      info->pcie_gen = device_info.pcie_gen;
      info->pcie_num_lanes = device_info.pcie_num_lanes;

      /* Source: https://en.wikipedia.org/wiki/PCI_Express#History_and_revisions */
      switch (info->pcie_gen) {
      case 1:
         info->pcie_bandwidth_mbps = info->pcie_num_lanes * 0.25 * 1024;
         break;
      case 2:
         info->pcie_bandwidth_mbps = info->pcie_num_lanes * 0.5 * 1024;
         break;
      case 3:
         info->pcie_bandwidth_mbps = info->pcie_num_lanes * 0.985 * 1024;
         break;
      case 4:
         info->pcie_bandwidth_mbps = info->pcie_num_lanes * 1.969 * 1024;
         break;
      case 5:
         info->pcie_bandwidth_mbps = info->pcie_num_lanes * 3.938 * 1024;
         break;
      case 6:
         info->pcie_bandwidth_mbps = info->pcie_num_lanes * 7.563 * 1024;
         break;
      case 7:
         info->pcie_bandwidth_mbps = info->pcie_num_lanes * 15.125 * 1024;
         break;
      }
   }

   /* The number of IBs per submit isn't infinite, it depends on the IP type
    * (ie. some initial setup needed for a submit) and the packet size.
    * It can be calculated according to the kernel source code as:
    * (ring->max_dw - emit_frame_size) / emit_ib_size
    */
   r = amdgpu_query_info(dev, AMDGPU_INFO_MAX_IBS,
                         sizeof(info->max_submitted_ibs), info->max_submitted_ibs);
   if (r) {
      /* When the number of IBs can't be queried from the kernel, we choose a
       * rough estimate that should work well (as of kernel 6.3).
       */
      for (unsigned i = 0; i < AMD_NUM_IP_TYPES; ++i)
         info->max_submitted_ibs[i] = 50;

      info->max_submitted_ibs[AMD_IP_GFX] = info->gfx_level >= GFX7 ? 192 : 144;
      info->max_submitted_ibs[AMD_IP_COMPUTE] = 124;
      info->max_submitted_ibs[AMD_IP_VCN_JPEG] = 16;
      for (unsigned i = 0; i < AMD_NUM_IP_TYPES; ++i) {
         /* Clear out max submitted IB count for IPs that have no queues. */
         if (!info->ip[i].num_queues)
            info->max_submitted_ibs[i] = 0;
      }
   }

   if (info->gfx_level >= GFX11) {
      if (info->l3_cache_size_mb) {
         info->attribute_ring_size_per_se = 1400 * 1024;
      } else {
         assert(info->num_se == 1);

         if (info->l2_cache_size >= 2 * 1024 * 1024)
            info->attribute_ring_size_per_se = 768 * 1024;
         else
            info->attribute_ring_size_per_se = info->l2_cache_size / 2;
      }

      /* The size must be aligned to 64K per SE and must be at most 16M in total. */
      info->attribute_ring_size_per_se = align(info->attribute_ring_size_per_se, 64 * 1024);
      assert(info->attribute_ring_size_per_se * info->max_se <= 16 * 1024 * 1024);

      info->conformant_trunc_coord =
         info->drm_minor >= 52 &&
         device_info.ids_flags & AMDGPU_IDS_FLAGS_CONFORMANT_TRUNC_COORD;
   }

   if (info->gfx_level >= GFX11 && device_info.shadow_size > 0) {
      info->has_fw_based_shadowing = true;
      info->fw_based_mcbp.shadow_size = device_info.shadow_size;
      info->fw_based_mcbp.shadow_alignment = device_info.shadow_alignment;
      info->fw_based_mcbp.csa_size = device_info.csa_size;
      info->fw_based_mcbp.csa_alignment = device_info.csa_alignment;
   }

   /* WARNING: Register shadowing decreases performance by up to 50% on GFX11 with current FW. */
   info->register_shadowing_required = device_info.ids_flags & AMDGPU_IDS_FLAGS_PREEMPTION &&
                                       info->gfx_level < GFX11;

   if (info->gfx_level >= GFX11 && info->has_dedicated_vram) {
      info->has_set_context_pairs_packed = true;
      info->has_set_sh_pairs_packed = info->register_shadowing_required;
   }

   set_custom_cu_en_mask(info);

   const char *ib_filename = debug_get_option("AMD_PARSE_IB", NULL);
   if (ib_filename) {
      FILE *f = fopen(ib_filename, "r");
      if (f) {
         fseek(f, 0, SEEK_END);
         size_t size = ftell(f);
         uint32_t *ib = (uint32_t *)malloc(size);
         fseek(f, 0, SEEK_SET);
         size_t n_read = fread(ib, 1, size, f);
         fclose(f);

         if (n_read != size) {
            fprintf(stderr, "failed to read %zu bytes from '%s'\n", size, ib_filename);
            exit(1);
         }

         ac_parse_ib(stdout, ib, size / 4, NULL, 0, "IB", info->gfx_level, info->family,
                     AMD_IP_GFX, NULL, NULL);
         free(ib);
         exit(0);
      }
   }
   return true;
}

void ac_compute_driver_uuid(char *uuid, size_t size)
{
   char amd_uuid[] = "AMD-MESA-DRV";

   assert(size >= sizeof(amd_uuid));

   memset(uuid, 0, size);
   strncpy(uuid, amd_uuid, size);
}

void ac_compute_device_uuid(const struct radeon_info *info, char *uuid, size_t size)
{
   uint32_t *uint_uuid = (uint32_t *)uuid;

   assert(size >= sizeof(uint32_t) * 4);

   /**
    * Use the device info directly instead of using a sha1. GL/VK UUIDs
    * are 16 byte vs 20 byte for sha1, and the truncation that would be
    * required would get rid of part of the little entropy we have.
    * */
   memset(uuid, 0, size);
   if (!info->pci.valid) {
      fprintf(stderr,
              "ac_compute_device_uuid's output is based on invalid pci bus info.\n");
   }
   uint_uuid[0] = info->pci.domain;
   uint_uuid[1] = info->pci.bus;
   uint_uuid[2] = info->pci.dev;
   uint_uuid[3] = info->pci.func;
}

void ac_print_gpu_info(const struct radeon_info *info, FILE *f)
{
   fprintf(f, "Device info:\n");
   fprintf(f, "    name = %s\n", info->name);
   fprintf(f, "    marketing_name = %s\n", info->marketing_name);
   fprintf(f, "    dev_filename = %s\n", info->dev_filename);
   fprintf(f, "    num_se = %i\n", info->num_se);
   fprintf(f, "    num_rb = %i\n", info->num_rb);
   fprintf(f, "    num_cu = %i\n", info->num_cu);
   fprintf(f, "    max_gpu_freq = %i MHz\n", info->max_gpu_freq_mhz);
   fprintf(f, "    max_gflops = %u GFLOPS\n", info->max_gflops);

   if (info->sqc_inst_cache_size) {
      fprintf(f, "    sqc_inst_cache_size = %i KB (%u per WGP)\n",
              DIV_ROUND_UP(info->sqc_inst_cache_size, 1024), info->num_sqc_per_wgp);
   }
   if (info->sqc_scalar_cache_size) {
      fprintf(f, "    sqc_scalar_cache_size = %i KB (%u per WGP)\n",
              DIV_ROUND_UP(info->sqc_scalar_cache_size, 1024), info->num_sqc_per_wgp);
   }

   fprintf(f, "    tcp_cache_size = %i KB\n", DIV_ROUND_UP(info->tcp_cache_size, 1024));

   if (info->gfx_level >= GFX10)
      fprintf(f, "    l1_cache_size = %i KB\n", DIV_ROUND_UP(info->l1_cache_size, 1024));

   fprintf(f, "    l2_cache_size = %i KB\n", DIV_ROUND_UP(info->l2_cache_size, 1024));

   if (info->l3_cache_size_mb)
      fprintf(f, "    l3_cache_size = %i MB\n", info->l3_cache_size_mb);

   fprintf(f, "    memory_channels = %u (TCC blocks)\n", info->num_tcc_blocks);
   fprintf(f, "    memory_size = %u GB (%u MB)\n",
           DIV_ROUND_UP(info->vram_size_kb, (1024 * 1024)),
           DIV_ROUND_UP(info->vram_size_kb, 1024));
   fprintf(f, "    memory_freq = %u GHz\n", DIV_ROUND_UP(info->memory_freq_mhz_effective, 1000));
   fprintf(f, "    memory_bus_width = %u bits\n", info->memory_bus_width);
   fprintf(f, "    memory_bandwidth = %u GB/s\n", info->memory_bandwidth_gbps);
   fprintf(f, "    pcie_gen = %u\n", info->pcie_gen);
   fprintf(f, "    pcie_num_lanes = %u\n", info->pcie_num_lanes);
   fprintf(f, "    pcie_bandwidth = %1.1f GB/s\n", info->pcie_bandwidth_mbps / 1024.0);
   fprintf(f, "    clock_crystal_freq = %i KHz\n", info->clock_crystal_freq);

   const char *ip_string[AMD_NUM_IP_TYPES] = {
      [AMD_IP_GFX] = "GFX",
      [AMD_IP_COMPUTE] = "COMP",
      [AMD_IP_SDMA] = "SDMA",
      [AMD_IP_UVD] = "UVD",
      [AMD_IP_VCE] = "VCE",
      [AMD_IP_UVD_ENC] = "UVD_ENC",
      [AMD_IP_VCN_DEC] = "VCN_DEC",
      [AMD_IP_VCN_ENC] = (info->vcn_ip_version >= VCN_4_0_0) ? "VCN" : "VCN_ENC",
      [AMD_IP_VCN_JPEG] = "VCN_JPG",
      [AMD_IP_VPE] = "VPE",
   };

   for (unsigned i = 0; i < AMD_NUM_IP_TYPES; i++) {
      if (info->ip[i].num_queues) {
         fprintf(f, "    IP %-7s %2u.%u \tqueues:%u \talign:%u \tpad_dw:0x%x\n", ip_string[i],
                 info->ip[i].ver_major, info->ip[i].ver_minor, info->ip[i].num_queues,
                 info->ip[i].ib_alignment, info->ip[i].ib_pad_dw_mask);
      }
   }

   fprintf(f, "Identification:\n");
   if (info->pci.valid)
      fprintf(f, "    pci (domain:bus:dev.func): %04x:%02x:%02x.%x\n", info->pci.domain, info->pci.bus,
              info->pci.dev, info->pci.func);
   else
      fprintf(f, "    pci (domain:bus:dev.func): unknown\n");
   fprintf(f, "    pci_id = 0x%x\n", info->pci_id);
   fprintf(f, "    pci_rev_id = 0x%x\n", info->pci_rev_id);
   fprintf(f, "    family = %i\n", info->family);
   fprintf(f, "    gfx_level = %i\n", info->gfx_level);
   fprintf(f, "    family_id = %i\n", info->family_id);
   fprintf(f, "    chip_external_rev = %i\n", info->chip_external_rev);
   fprintf(f, "    chip_rev = %i\n", info->chip_rev);

   fprintf(f, "Flags:\n");
   fprintf(f, "    family_overridden = %u\n", info->family_overridden);
   fprintf(f, "    is_pro_graphics = %u\n", info->is_pro_graphics);
   fprintf(f, "    has_graphics = %i\n", info->has_graphics);
   fprintf(f, "    has_clear_state = %u\n", info->has_clear_state);
   fprintf(f, "    has_distributed_tess = %u\n", info->has_distributed_tess);
   fprintf(f, "    has_dcc_constant_encode = %u\n", info->has_dcc_constant_encode);
   fprintf(f, "    has_rbplus = %u\n", info->has_rbplus);
   fprintf(f, "    rbplus_allowed = %u\n", info->rbplus_allowed);
   fprintf(f, "    has_load_ctx_reg_pkt = %u\n", info->has_load_ctx_reg_pkt);
   fprintf(f, "    has_out_of_order_rast = %u\n", info->has_out_of_order_rast);
   fprintf(f, "    cpdma_prefetch_writes_memory = %u\n", info->cpdma_prefetch_writes_memory);
   fprintf(f, "    has_gfx9_scissor_bug = %i\n", info->has_gfx9_scissor_bug);
   fprintf(f, "    has_tc_compat_zrange_bug = %i\n", info->has_tc_compat_zrange_bug);
   fprintf(f, "    has_small_prim_filter_sample_loc_bug = %i\n", info->has_small_prim_filter_sample_loc_bug);
   fprintf(f, "    has_ls_vgpr_init_bug = %i\n", info->has_ls_vgpr_init_bug);
   fprintf(f, "    has_pops_missed_overlap_bug = %i\n", info->has_pops_missed_overlap_bug);
   fprintf(f, "    has_32bit_predication = %i\n", info->has_32bit_predication);
   fprintf(f, "    has_3d_cube_border_color_mipmap = %i\n", info->has_3d_cube_border_color_mipmap);
   fprintf(f, "    has_image_opcodes = %i\n", info->has_image_opcodes);
   fprintf(f, "    never_stop_sq_perf_counters = %i\n", info->never_stop_sq_perf_counters);
   fprintf(f, "    has_sqtt_rb_harvest_bug = %i\n", info->has_sqtt_rb_harvest_bug);
   fprintf(f, "    has_sqtt_auto_flush_mode_bug = %i\n", info->has_sqtt_auto_flush_mode_bug);
   fprintf(f, "    never_send_perfcounter_stop = %i\n", info->never_send_perfcounter_stop);
   fprintf(f, "    discardable_allows_big_page = %i\n", info->discardable_allows_big_page);
   fprintf(f, "    has_taskmesh_indirect0_bug = %i\n", info->has_taskmesh_indirect0_bug);
   fprintf(f, "    has_set_context_pairs_packed = %i\n", info->has_set_context_pairs_packed);
   fprintf(f, "    has_set_sh_pairs_packed = %i\n", info->has_set_sh_pairs_packed);
   fprintf(f, "    conformant_trunc_coord = %i\n", info->conformant_trunc_coord);

   fprintf(f, "Display features:\n");
   fprintf(f, "    use_display_dcc_unaligned = %u\n", info->use_display_dcc_unaligned);
   fprintf(f, "    use_display_dcc_with_retile_blit = %u\n", info->use_display_dcc_with_retile_blit);

   fprintf(f, "Memory info:\n");
   fprintf(f, "    pte_fragment_size = %u\n", info->pte_fragment_size);
   fprintf(f, "    gart_page_size = %u\n", info->gart_page_size);
   fprintf(f, "    gart_size = %i MB\n", (int)DIV_ROUND_UP(info->gart_size_kb, 1024));
   fprintf(f, "    vram_size = %i MB\n", (int)DIV_ROUND_UP(info->vram_size_kb, 1024));
   fprintf(f, "    vram_vis_size = %i MB\n", (int)DIV_ROUND_UP(info->vram_vis_size_kb, 1024));
   fprintf(f, "    vram_type = %i\n", info->vram_type);
   fprintf(f, "    max_heap_size_kb = %i MB\n", (int)DIV_ROUND_UP(info->max_heap_size_kb, 1024));
   fprintf(f, "    min_alloc_size = %u\n", info->min_alloc_size);
   fprintf(f, "    address32_hi = 0x%x\n", info->address32_hi);
   fprintf(f, "    has_dedicated_vram = %u\n", info->has_dedicated_vram);
   fprintf(f, "    all_vram_visible = %u\n", info->all_vram_visible);
   fprintf(f, "    max_tcc_blocks = %i\n", info->max_tcc_blocks);
   fprintf(f, "    tcc_cache_line_size = %u\n", info->tcc_cache_line_size);
   fprintf(f, "    tcc_rb_non_coherent = %u\n", info->tcc_rb_non_coherent);
   fprintf(f, "    pc_lines = %u\n", info->pc_lines);
   fprintf(f, "    lds_size_per_workgroup = %u\n", info->lds_size_per_workgroup);
   fprintf(f, "    lds_alloc_granularity = %i\n", info->lds_alloc_granularity);
   fprintf(f, "    lds_encode_granularity = %i\n", info->lds_encode_granularity);
   fprintf(f, "    max_memory_clock = %i MHz\n", info->memory_freq_mhz);

   fprintf(f, "CP info:\n");
   fprintf(f, "    gfx_ib_pad_with_type2 = %i\n", info->gfx_ib_pad_with_type2);
   fprintf(f, "    me_fw_version = %i\n", info->me_fw_version);
   fprintf(f, "    me_fw_feature = %i\n", info->me_fw_feature);
   fprintf(f, "    mec_fw_version = %i\n", info->mec_fw_version);
   fprintf(f, "    mec_fw_feature = %i\n", info->mec_fw_feature);
   fprintf(f, "    pfp_fw_version = %i\n", info->pfp_fw_version);
   fprintf(f, "    pfp_fw_feature = %i\n", info->pfp_fw_feature);

   fprintf(f, "Multimedia info:\n");
   fprintf(f, "    vce_encode = %u\n", info->ip[AMD_IP_VCE].num_queues);

   if (info->family >= CHIP_NAVI31 || info->family == CHIP_GFX940)
      fprintf(f, "    vcn_unified = %u\n", info->ip[AMD_IP_VCN_UNIFIED].num_queues);
   else {
      fprintf(f, "    vcn_decode = %u\n", info->ip[AMD_IP_VCN_DEC].num_queues);
      fprintf(f, "    vcn_encode = %u\n", info->ip[AMD_IP_VCN_ENC].num_queues);
   }

   fprintf(f, "    uvd_fw_version = %u\n", info->uvd_fw_version);
   fprintf(f, "    vce_fw_version = %u\n", info->vce_fw_version);
   fprintf(f, "    vce_harvest_config = %i\n", info->vce_harvest_config);

   fprintf(f, "Kernel & winsys capabilities:\n");
   fprintf(f, "    drm = %i.%i.%i\n", info->drm_major, info->drm_minor, info->drm_patchlevel);
   fprintf(f, "    has_userptr = %i\n", info->has_userptr);
   fprintf(f, "    has_syncobj = %u\n", info->has_syncobj);
   fprintf(f, "    has_timeline_syncobj = %u\n", info->has_timeline_syncobj);
   fprintf(f, "    has_fence_to_handle = %u\n", info->has_fence_to_handle);
   fprintf(f, "    has_local_buffers = %u\n", info->has_local_buffers);
   fprintf(f, "    has_bo_metadata = %u\n", info->has_bo_metadata);
   fprintf(f, "    has_eqaa_surface_allocator = %u\n", info->has_eqaa_surface_allocator);
   fprintf(f, "    has_sparse_vm_mappings = %u\n", info->has_sparse_vm_mappings);
   fprintf(f, "    has_stable_pstate = %u\n", info->has_stable_pstate);
   fprintf(f, "    has_scheduled_fence_dependency = %u\n", info->has_scheduled_fence_dependency);
   fprintf(f, "    has_gang_submit = %u\n", info->has_gang_submit);
   fprintf(f, "    has_gpuvm_fault_query = %u\n", info->has_gpuvm_fault_query);
   fprintf(f, "    register_shadowing_required = %u\n", info->register_shadowing_required);
   fprintf(f, "    has_fw_based_shadowing = %u\n", info->has_fw_based_shadowing);
   if (info->has_fw_based_shadowing) {
      fprintf(f, "        * shadow size: %u (alignment: %u)\n",
         info->fw_based_mcbp.shadow_size,
         info->fw_based_mcbp.shadow_alignment);
      fprintf(f, "        * csa size: %u (alignment: %u)\n",
         info->fw_based_mcbp.csa_size,
         info->fw_based_mcbp.csa_alignment);
   }

   fprintf(f, "    has_tmz_support = %u\n", info->has_tmz_support);
   for (unsigned i = 0; i < AMD_NUM_IP_TYPES; i++) {
      if (info->max_submitted_ibs[i]) {
         fprintf(f, "    IP %-7s max_submitted_ibs = %u\n", ip_string[i],
                 info->max_submitted_ibs[i]);
      }
   }
   fprintf(f, "    kernel_has_modifiers = %u\n", info->kernel_has_modifiers);
   fprintf(f, "    uses_kernel_cu_mask = %u\n", info->uses_kernel_cu_mask);

   fprintf(f, "Shader core info:\n");
   for (unsigned i = 0; i < info->max_se; i++) {
      for (unsigned j = 0; j < info->max_sa_per_se; j++) {
         fprintf(f, "    cu_mask[SE%u][SA%u] = 0x%x \t(%u)\tCU_EN = 0x%x\n", i, j,
                 info->cu_mask[i][j], util_bitcount(info->cu_mask[i][j]),
                 info->spi_cu_en & BITFIELD_MASK(util_bitcount(info->cu_mask[i][j])));
      }
   }
   fprintf(f, "    spi_cu_en_has_effect = %i\n", info->spi_cu_en_has_effect);
   fprintf(f, "    max_good_cu_per_sa = %i\n", info->max_good_cu_per_sa);
   fprintf(f, "    min_good_cu_per_sa = %i\n", info->min_good_cu_per_sa);
   fprintf(f, "    max_se = %i\n", info->max_se);
   fprintf(f, "    max_sa_per_se = %i\n", info->max_sa_per_se);
   fprintf(f, "    num_cu_per_sh = %i\n", info->num_cu_per_sh);
   fprintf(f, "    max_waves_per_simd = %i\n", info->max_waves_per_simd);
   fprintf(f, "    num_physical_sgprs_per_simd = %i\n", info->num_physical_sgprs_per_simd);
   fprintf(f, "    num_physical_wave64_vgprs_per_simd = %i\n",
           info->num_physical_wave64_vgprs_per_simd);
   fprintf(f, "    num_simd_per_compute_unit = %i\n", info->num_simd_per_compute_unit);
   fprintf(f, "    min_sgpr_alloc = %i\n", info->min_sgpr_alloc);
   fprintf(f, "    max_sgpr_alloc = %i\n", info->max_sgpr_alloc);
   fprintf(f, "    sgpr_alloc_granularity = %i\n", info->sgpr_alloc_granularity);
   fprintf(f, "    min_wave64_vgpr_alloc = %i\n", info->min_wave64_vgpr_alloc);
   fprintf(f, "    max_vgpr_alloc = %i\n", info->max_vgpr_alloc);
   fprintf(f, "    wave64_vgpr_alloc_granularity = %i\n", info->wave64_vgpr_alloc_granularity);
   fprintf(f, "    max_scratch_waves = %i\n", info->max_scratch_waves);
   fprintf(f, "    attribute_ring_size_per_se = %u\n", info->attribute_ring_size_per_se);

   fprintf(f, "Render backend info:\n");
   fprintf(f, "    pa_sc_tile_steering_override = 0x%x\n", info->pa_sc_tile_steering_override);
   fprintf(f, "    max_render_backends = %i\n", info->max_render_backends);
   fprintf(f, "    num_tile_pipes = %i\n", info->num_tile_pipes);
   fprintf(f, "    pipe_interleave_bytes = %i\n", info->pipe_interleave_bytes);
   fprintf(f, "    enabled_rb_mask = 0x%" PRIx64 "\n", info->enabled_rb_mask);
   fprintf(f, "    max_alignment = %u\n", (unsigned)info->max_alignment);
   fprintf(f, "    pbb_max_alloc_count = %u\n", info->pbb_max_alloc_count);

   fprintf(f, "GB_ADDR_CONFIG: 0x%08x\n", info->gb_addr_config);
   if (info->gfx_level >= GFX10) {
      fprintf(f, "    num_pipes = %u\n", 1 << G_0098F8_NUM_PIPES(info->gb_addr_config));
      fprintf(f, "    pipe_interleave_size = %u\n",
              256 << G_0098F8_PIPE_INTERLEAVE_SIZE_GFX9(info->gb_addr_config));
      fprintf(f, "    max_compressed_frags = %u\n",
              1 << G_0098F8_MAX_COMPRESSED_FRAGS(info->gb_addr_config));
      if (info->gfx_level >= GFX10_3)
         fprintf(f, "    num_pkrs = %u\n", 1 << G_0098F8_NUM_PKRS(info->gb_addr_config));
   } else if (info->gfx_level == GFX9) {
      fprintf(f, "    num_pipes = %u\n", 1 << G_0098F8_NUM_PIPES(info->gb_addr_config));
      fprintf(f, "    pipe_interleave_size = %u\n",
              256 << G_0098F8_PIPE_INTERLEAVE_SIZE_GFX9(info->gb_addr_config));
      fprintf(f, "    max_compressed_frags = %u\n",
              1 << G_0098F8_MAX_COMPRESSED_FRAGS(info->gb_addr_config));
      fprintf(f, "    bank_interleave_size = %u\n",
              1 << G_0098F8_BANK_INTERLEAVE_SIZE(info->gb_addr_config));
      fprintf(f, "    num_banks = %u\n", 1 << G_0098F8_NUM_BANKS(info->gb_addr_config));
      fprintf(f, "    shader_engine_tile_size = %u\n",
              16 << G_0098F8_SHADER_ENGINE_TILE_SIZE(info->gb_addr_config));
      fprintf(f, "    num_shader_engines = %u\n",
              1 << G_0098F8_NUM_SHADER_ENGINES_GFX9(info->gb_addr_config));
      fprintf(f, "    num_gpus = %u (raw)\n", G_0098F8_NUM_GPUS_GFX9(info->gb_addr_config));
      fprintf(f, "    multi_gpu_tile_size = %u (raw)\n",
              G_0098F8_MULTI_GPU_TILE_SIZE(info->gb_addr_config));
      fprintf(f, "    num_rb_per_se = %u\n", 1 << G_0098F8_NUM_RB_PER_SE(info->gb_addr_config));
      fprintf(f, "    row_size = %u\n", 1024 << G_0098F8_ROW_SIZE(info->gb_addr_config));
      fprintf(f, "    num_lower_pipes = %u (raw)\n", G_0098F8_NUM_LOWER_PIPES(info->gb_addr_config));
      fprintf(f, "    se_enable = %u (raw)\n", G_0098F8_SE_ENABLE(info->gb_addr_config));
   } else {
      fprintf(f, "    num_pipes = %u\n", 1 << G_0098F8_NUM_PIPES(info->gb_addr_config));
      fprintf(f, "    pipe_interleave_size = %u\n",
              256 << G_0098F8_PIPE_INTERLEAVE_SIZE_GFX6(info->gb_addr_config));
      fprintf(f, "    bank_interleave_size = %u\n",
              1 << G_0098F8_BANK_INTERLEAVE_SIZE(info->gb_addr_config));
      fprintf(f, "    num_shader_engines = %u\n",
              1 << G_0098F8_NUM_SHADER_ENGINES_GFX6(info->gb_addr_config));
      fprintf(f, "    shader_engine_tile_size = %u\n",
              16 << G_0098F8_SHADER_ENGINE_TILE_SIZE(info->gb_addr_config));
      fprintf(f, "    num_gpus = %u (raw)\n", G_0098F8_NUM_GPUS_GFX6(info->gb_addr_config));
      fprintf(f, "    multi_gpu_tile_size = %u (raw)\n",
              G_0098F8_MULTI_GPU_TILE_SIZE(info->gb_addr_config));
      fprintf(f, "    row_size = %u\n", 1024 << G_0098F8_ROW_SIZE(info->gb_addr_config));
      fprintf(f, "    num_lower_pipes = %u (raw)\n", G_0098F8_NUM_LOWER_PIPES(info->gb_addr_config));
   }
}

int ac_get_gs_table_depth(enum amd_gfx_level gfx_level, enum radeon_family family)
{
   if (gfx_level >= GFX9)
      return -1;

   switch (family) {
   case CHIP_OLAND:
   case CHIP_HAINAN:
   case CHIP_KAVERI:
   case CHIP_KABINI:
   case CHIP_ICELAND:
   case CHIP_CARRIZO:
   case CHIP_STONEY:
      return 16;
   case CHIP_TAHITI:
   case CHIP_PITCAIRN:
   case CHIP_VERDE:
   case CHIP_BONAIRE:
   case CHIP_HAWAII:
   case CHIP_TONGA:
   case CHIP_FIJI:
   case CHIP_POLARIS10:
   case CHIP_POLARIS11:
   case CHIP_POLARIS12:
   case CHIP_VEGAM:
      return 32;
   default:
      unreachable("Unknown GPU");
   }
}

void ac_get_raster_config(const struct radeon_info *info, uint32_t *raster_config_p,
                          uint32_t *raster_config_1_p, uint32_t *se_tile_repeat_p)
{
   unsigned raster_config, raster_config_1, se_tile_repeat;

   switch (info->family) {
   /* 1 SE / 1 RB */
   case CHIP_HAINAN:
   case CHIP_KABINI:
   case CHIP_STONEY:
      raster_config = 0x00000000;
      raster_config_1 = 0x00000000;
      break;
   /* 1 SE / 4 RBs */
   case CHIP_VERDE:
      raster_config = 0x0000124a;
      raster_config_1 = 0x00000000;
      break;
   /* 1 SE / 2 RBs (Oland is special) */
   case CHIP_OLAND:
      raster_config = 0x00000082;
      raster_config_1 = 0x00000000;
      break;
   /* 1 SE / 2 RBs */
   case CHIP_KAVERI:
   case CHIP_ICELAND:
   case CHIP_CARRIZO:
      raster_config = 0x00000002;
      raster_config_1 = 0x00000000;
      break;
   /* 2 SEs / 4 RBs */
   case CHIP_BONAIRE:
   case CHIP_POLARIS11:
   case CHIP_POLARIS12:
      raster_config = 0x16000012;
      raster_config_1 = 0x00000000;
      break;
   /* 2 SEs / 8 RBs */
   case CHIP_TAHITI:
   case CHIP_PITCAIRN:
      raster_config = 0x2a00126a;
      raster_config_1 = 0x00000000;
      break;
   /* 4 SEs / 8 RBs */
   case CHIP_TONGA:
   case CHIP_POLARIS10:
      raster_config = 0x16000012;
      raster_config_1 = 0x0000002a;
      break;
   /* 4 SEs / 16 RBs */
   case CHIP_HAWAII:
   case CHIP_FIJI:
   case CHIP_VEGAM:
      raster_config = 0x3a00161a;
      raster_config_1 = 0x0000002e;
      break;
   default:
      fprintf(stderr, "ac: Unknown GPU, using 0 for raster_config\n");
      raster_config = 0x00000000;
      raster_config_1 = 0x00000000;
      break;
   }

   /* drm/radeon on Kaveri is buggy, so disable 1 RB to work around it.
    * This decreases performance by up to 50% when the RB is the bottleneck.
    */
   if (info->family == CHIP_KAVERI && !info->is_amdgpu)
      raster_config = 0x00000000;

   /* Fiji: Old kernels have incorrect tiling config. This decreases
    * RB performance by 25%. (it disables 1 RB in the second packer)
    */
   if (info->family == CHIP_FIJI && info->cik_macrotile_mode_array[0] == 0x000000e8) {
      raster_config = 0x16000012;
      raster_config_1 = 0x0000002a;
   }

   unsigned se_width = 8 << G_028350_SE_XSEL_GFX6(raster_config);
   unsigned se_height = 8 << G_028350_SE_YSEL_GFX6(raster_config);

   /* I don't know how to calculate this, though this is probably a good guess. */
   se_tile_repeat = MAX2(se_width, se_height) * info->max_se;

   *raster_config_p = raster_config;
   *raster_config_1_p = raster_config_1;
   if (se_tile_repeat_p)
      *se_tile_repeat_p = se_tile_repeat;
}

void ac_get_harvested_configs(const struct radeon_info *info, unsigned raster_config,
                              unsigned *cik_raster_config_1_p, unsigned *raster_config_se)
{
   unsigned sh_per_se = MAX2(info->max_sa_per_se, 1);
   unsigned num_se = MAX2(info->max_se, 1);
   unsigned rb_mask = info->enabled_rb_mask;
   unsigned num_rb = MIN2(info->max_render_backends, 16);
   unsigned rb_per_pkr = MIN2(num_rb / num_se / sh_per_se, 2);
   unsigned rb_per_se = num_rb / num_se;
   unsigned se_mask[4];
   unsigned se;

   se_mask[0] = ((1 << rb_per_se) - 1) & rb_mask;
   se_mask[1] = (se_mask[0] << rb_per_se) & rb_mask;
   se_mask[2] = (se_mask[1] << rb_per_se) & rb_mask;
   se_mask[3] = (se_mask[2] << rb_per_se) & rb_mask;

   assert(num_se == 1 || num_se == 2 || num_se == 4);
   assert(sh_per_se == 1 || sh_per_se == 2);
   assert(rb_per_pkr == 1 || rb_per_pkr == 2);

   if (info->gfx_level >= GFX7) {
      unsigned raster_config_1 = *cik_raster_config_1_p;
      if ((num_se > 2) && ((!se_mask[0] && !se_mask[1]) || (!se_mask[2] && !se_mask[3]))) {
         raster_config_1 &= C_028354_SE_PAIR_MAP;

         if (!se_mask[0] && !se_mask[1]) {
            raster_config_1 |= S_028354_SE_PAIR_MAP(V_028354_RASTER_CONFIG_SE_PAIR_MAP_3);
         } else {
            raster_config_1 |= S_028354_SE_PAIR_MAP(V_028354_RASTER_CONFIG_SE_PAIR_MAP_0);
         }
         *cik_raster_config_1_p = raster_config_1;
      }
   }

   for (se = 0; se < num_se; se++) {
      unsigned pkr0_mask = ((1 << rb_per_pkr) - 1) << (se * rb_per_se);
      unsigned pkr1_mask = pkr0_mask << rb_per_pkr;
      int idx = (se / 2) * 2;

      raster_config_se[se] = raster_config;
      if ((num_se > 1) && (!se_mask[idx] || !se_mask[idx + 1])) {
         raster_config_se[se] &= C_028350_SE_MAP;

         if (!se_mask[idx]) {
            raster_config_se[se] |= S_028350_SE_MAP(V_028350_RASTER_CONFIG_SE_MAP_3);
         } else {
            raster_config_se[se] |= S_028350_SE_MAP(V_028350_RASTER_CONFIG_SE_MAP_0);
         }
      }

      pkr0_mask &= rb_mask;
      pkr1_mask &= rb_mask;
      if (rb_per_se > 2 && (!pkr0_mask || !pkr1_mask)) {
         raster_config_se[se] &= C_028350_PKR_MAP;

         if (!pkr0_mask) {
            raster_config_se[se] |= S_028350_PKR_MAP(V_028350_RASTER_CONFIG_PKR_MAP_3);
         } else {
            raster_config_se[se] |= S_028350_PKR_MAP(V_028350_RASTER_CONFIG_PKR_MAP_0);
         }
      }

      if (rb_per_se >= 2) {
         unsigned rb0_mask = 1 << (se * rb_per_se);
         unsigned rb1_mask = rb0_mask << 1;

         rb0_mask &= rb_mask;
         rb1_mask &= rb_mask;
         if (!rb0_mask || !rb1_mask) {
            raster_config_se[se] &= C_028350_RB_MAP_PKR0;

            if (!rb0_mask) {
               raster_config_se[se] |= S_028350_RB_MAP_PKR0(V_028350_RASTER_CONFIG_RB_MAP_3);
            } else {
               raster_config_se[se] |= S_028350_RB_MAP_PKR0(V_028350_RASTER_CONFIG_RB_MAP_0);
            }
         }

         if (rb_per_se > 2) {
            rb0_mask = 1 << (se * rb_per_se + rb_per_pkr);
            rb1_mask = rb0_mask << 1;
            rb0_mask &= rb_mask;
            rb1_mask &= rb_mask;
            if (!rb0_mask || !rb1_mask) {
               raster_config_se[se] &= C_028350_RB_MAP_PKR1;

               if (!rb0_mask) {
                  raster_config_se[se] |= S_028350_RB_MAP_PKR1(V_028350_RASTER_CONFIG_RB_MAP_3);
               } else {
                  raster_config_se[se] |= S_028350_RB_MAP_PKR1(V_028350_RASTER_CONFIG_RB_MAP_0);
               }
            }
         }
      }
   }
}

unsigned
ac_get_compute_resource_limits(const struct radeon_info *info, unsigned waves_per_threadgroup,
                               unsigned max_waves_per_sh, unsigned threadgroups_per_cu)
{
   unsigned compute_resource_limits = S_00B854_SIMD_DEST_CNTL(waves_per_threadgroup % 4 == 0);

   if (info->gfx_level >= GFX7) {
      unsigned num_cu_per_se = info->num_cu / info->num_se;

      /* Gfx9 should set the limit to max instead of 0 to fix high priority compute. */
      if (info->gfx_level == GFX9 && !max_waves_per_sh) {
         max_waves_per_sh = info->max_good_cu_per_sa * info->num_simd_per_compute_unit *
                            info->max_waves_per_simd;
      }

      /* Force even distribution on all SIMDs in CU if the workgroup
       * size is 64. This has shown some good improvements if # of CUs
       * per SE is not a multiple of 4.
       */
      if (num_cu_per_se % 4 && waves_per_threadgroup == 1)
         compute_resource_limits |= S_00B854_FORCE_SIMD_DIST(1);

      assert(threadgroups_per_cu >= 1 && threadgroups_per_cu <= 8);
      compute_resource_limits |=
         S_00B854_WAVES_PER_SH(max_waves_per_sh) | S_00B854_CU_GROUP_COUNT(threadgroups_per_cu - 1);
   } else {
      /* GFX6 */
      if (max_waves_per_sh) {
         unsigned limit_div16 = DIV_ROUND_UP(max_waves_per_sh, 16);
         compute_resource_limits |= S_00B854_WAVES_PER_SH_GFX6(limit_div16);
      }
   }
   return compute_resource_limits;
}

void ac_get_hs_info(const struct radeon_info *info,
                    struct ac_hs_info *hs)
{
   bool double_offchip_buffers = info->gfx_level >= GFX7 &&
                                 info->family != CHIP_CARRIZO &&
                                 info->family != CHIP_STONEY;
   unsigned max_offchip_buffers_per_se;
   unsigned max_offchip_buffers;
   unsigned offchip_granularity;
   unsigned hs_offchip_param;

   hs->tess_offchip_block_dw_size =
      info->family == CHIP_HAWAII ? 4096 : 8192;

   /*
    * Per RadeonSI:
    * This must be one less than the maximum number due to a hw limitation.
    * Various hardware bugs need this.
    *
    * Per AMDVLK:
    * Vega10 should limit max_offchip_buffers to 508 (4 * 127).
    * Gfx7 should limit max_offchip_buffers to 508
    * Gfx6 should limit max_offchip_buffers to 126 (2 * 63)
    *
    * Follow AMDVLK here.
    */
   if (info->gfx_level >= GFX11) {
      max_offchip_buffers_per_se = 256; /* TODO: we could decrease this to reduce memory/cache usage */
   } else if (info->gfx_level >= GFX10) {
      max_offchip_buffers_per_se = 128;
   } else if (info->family == CHIP_VEGA12 || info->family == CHIP_VEGA20) {
      /* Only certain chips can use the maximum value. */
      max_offchip_buffers_per_se = double_offchip_buffers ? 128 : 64;
   } else {
      max_offchip_buffers_per_se = double_offchip_buffers ? 127 : 63;
   }

   max_offchip_buffers = max_offchip_buffers_per_se * info->max_se;

   /* Hawaii has a bug with offchip buffers > 256 that can be worked
    * around by setting 4K granularity.
    */
   if (hs->tess_offchip_block_dw_size == 4096) {
      assert(info->family == CHIP_HAWAII);
      offchip_granularity = V_03093C_X_4K_DWORDS;
   } else {
      assert(hs->tess_offchip_block_dw_size == 8192);
      offchip_granularity = V_03093C_X_8K_DWORDS;
   }

   switch (info->gfx_level) {
   case GFX6:
      max_offchip_buffers = MIN2(max_offchip_buffers, 126);
      break;
   case GFX7:
   case GFX8:
   case GFX9:
      max_offchip_buffers = MIN2(max_offchip_buffers, 508);
      break;
   case GFX10:
      break;
   default:
      break;
   }

   hs->max_offchip_buffers = max_offchip_buffers;

   if (info->gfx_level >= GFX11) {
      /* OFFCHIP_BUFFERING is per SE. */
      hs_offchip_param = S_03093C_OFFCHIP_BUFFERING_GFX103(max_offchip_buffers_per_se - 1) |
                         S_03093C_OFFCHIP_GRANULARITY_GFX103(offchip_granularity);
   } else if (info->gfx_level >= GFX10_3) {
      hs_offchip_param = S_03093C_OFFCHIP_BUFFERING_GFX103(max_offchip_buffers - 1) |
                         S_03093C_OFFCHIP_GRANULARITY_GFX103(offchip_granularity);
   } else if (info->gfx_level >= GFX7) {
      if (info->gfx_level >= GFX8)
         --max_offchip_buffers;
      hs_offchip_param = S_03093C_OFFCHIP_BUFFERING_GFX7(max_offchip_buffers) |
                         S_03093C_OFFCHIP_GRANULARITY_GFX7(offchip_granularity);
   } else {
      hs_offchip_param = S_0089B0_OFFCHIP_BUFFERING(max_offchip_buffers);
   }

   hs->hs_offchip_param = hs_offchip_param;

   hs->tess_factor_ring_size = 48 * 1024 * info->max_se;
   hs->tess_offchip_ring_offset = align(hs->tess_factor_ring_size, 64 * 1024);
   hs->tess_offchip_ring_size = hs->max_offchip_buffers * hs->tess_offchip_block_dw_size * 4;
}

static uint16_t get_task_num_entries(enum radeon_family fam)
{
   /* Number of task shader ring entries. Needs to be a power of two.
    * Use a low number on smaller chips so we don't waste space,
    * but keep it high on bigger chips so it doesn't inhibit parallelism.
    *
    * This number is compiled into task/mesh shaders as a constant.
    * In order to ensure this works fine with the shader cache, we must
    * base this decision on the chip family, not the number of CUs in
    * the current GPU. (So, the cache remains consistent for all
    * chips in the same family.)
    */
   switch (fam) {
   case CHIP_VANGOGH:
   case CHIP_NAVI24:
   case CHIP_REMBRANDT:
      return 256;
   case CHIP_NAVI21:
   case CHIP_NAVI22:
   case CHIP_NAVI23:
   default:
      return 1024;
   }
}

void ac_get_task_info(const struct radeon_info *info,
                      struct ac_task_info *task_info)
{
   const uint16_t num_entries = get_task_num_entries(info->family);
   const uint32_t draw_ring_bytes = num_entries * AC_TASK_DRAW_ENTRY_BYTES;
   const uint32_t payload_ring_bytes = num_entries * AC_TASK_PAYLOAD_ENTRY_BYTES;

   /* Ensure that the addresses of each ring are 256 byte aligned. */
   task_info->num_entries = num_entries;
   task_info->draw_ring_offset = ALIGN(AC_TASK_CTRLBUF_BYTES, 256);
   task_info->payload_ring_offset = ALIGN(task_info->draw_ring_offset + draw_ring_bytes, 256);
   task_info->bo_size_bytes = task_info->payload_ring_offset + payload_ring_bytes;
}

uint32_t ac_memory_ops_per_clock(uint32_t vram_type)
{
   /* Based on MemoryOpsPerClockTable from PAL. */
   switch (vram_type) {
   case AMDGPU_VRAM_TYPE_GDDR1:
   case AMDGPU_VRAM_TYPE_GDDR3: /* last in low-end Evergreen */
   case AMDGPU_VRAM_TYPE_GDDR4: /* last in R7xx, not used much */
   case AMDGPU_VRAM_TYPE_UNKNOWN:
   default:
      return 0;
   case AMDGPU_VRAM_TYPE_DDR2:
   case AMDGPU_VRAM_TYPE_DDR3:
   case AMDGPU_VRAM_TYPE_DDR4:
   case AMDGPU_VRAM_TYPE_LPDDR4:
   case AMDGPU_VRAM_TYPE_HBM: /* same for HBM2 and HBM3 */
      return 2;
   case AMDGPU_VRAM_TYPE_DDR5:
   case AMDGPU_VRAM_TYPE_LPDDR5:
   case AMDGPU_VRAM_TYPE_GDDR5: /* last in Polaris and low-end Navi14 */
      return 4;
   case AMDGPU_VRAM_TYPE_GDDR6:
      return 16;
   }
}
