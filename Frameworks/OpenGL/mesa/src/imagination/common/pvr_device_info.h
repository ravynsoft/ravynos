/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_DEVICE_INFO_H
#define PVR_DEVICE_INFO_H

/* TODO: This file is currently hand-maintained. However, the intention is to
 * auto-generate it in the future based on the hwdefs.
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include "util/log.h"
#include "util/macros.h"

#define PVR_BVNC_PACK_SHIFT_B 48
#define PVR_BVNC_PACK_SHIFT_V 32
#define PVR_BVNC_PACK_SHIFT_N 16
#define PVR_BVNC_PACK_SHIFT_C 0

#define PVR_BVNC_PACK_MASK_B UINT64_C(0xFFFF000000000000)
#define PVR_BVNC_PACK_MASK_V UINT64_C(0x0000FFFF00000000)
#define PVR_BVNC_PACK_MASK_N UINT64_C(0x00000000FFFF0000)
#define PVR_BVNC_PACK_MASK_C UINT64_C(0x000000000000FFFF)

/**
 * Packs B, V, N and C values into a 64-bit unsigned integer.
 *
 * The packed layout is as follows:
 *
 * \verbatim
 *    +--------+--------+--------+-------+
 *    | 63..48 | 47..32 | 31..16 | 15..0 |
 *    +========+========+========+=======+
 *    | B      | V      | N      | C     |
 *    +--------+--------+--------+-------+
 * \endverbatim
 *
 * #pvr_get_packed_bvnc() should be used instead of this macro when a
 * #pvr_device_information is available in order to ensure proper type checking.
 *
 * \param b Branch ID.
 * \param v Version ID.
 * \param n Number of scalable units.
 * \param c Config ID.
 * \return Packed BVNC.
 *
 * \sa #pvr_get_packed_bvnc(), #PVR_BVNC_UNPACK_B(), #PVR_BVNC_UNPACK_V(),
 * #PVR_BVNC_UNPACK_N() and #PVR_BVNC_UNPACK_C()
 */
#define PVR_BVNC_PACK(b, v, n, c)                                       \
   ((((uint64_t)(b) << PVR_BVNC_PACK_SHIFT_B) & PVR_BVNC_PACK_MASK_B) | \
    (((uint64_t)(v) << PVR_BVNC_PACK_SHIFT_V) & PVR_BVNC_PACK_MASK_V) | \
    (((uint64_t)(n) << PVR_BVNC_PACK_SHIFT_N) & PVR_BVNC_PACK_MASK_N) | \
    (((uint64_t)(c) << PVR_BVNC_PACK_SHIFT_C) & PVR_BVNC_PACK_MASK_C))

/**
 * Unpacks B value (branch ID) from packed BVNC.
 *
 * \param bvnc Packed BVNC.
 * \return Branch ID.
 *
 * \sa #PVR_BVNC_UNPACK_V(), #PVR_BVNC_UNPACK_N(), #PVR_BVNC_UNPACK_C(),
 * #pvr_get_packed_bvnc() and #PVR_BVNC_PACK()
 */
#define PVR_BVNC_UNPACK_B(bvnc) \
   ((uint16_t)(((bvnc)&PVR_BVNC_PACK_MASK_B) >> PVR_BVNC_PACK_SHIFT_B))

/**
 * Unpacks V value (version ID) from packed BVNC.
 *
 * \param bvnc Packed BVNC.
 * \return Version ID.
 *
 * \sa #PVR_BVNC_UNPACK_B(), #PVR_BVNC_UNPACK_N(), #PVR_BVNC_UNPACK_C(),
 * #pvr_get_packed_bvnc() and #PVR_BVNC_PACK()
 */
#define PVR_BVNC_UNPACK_V(bvnc) \
   ((uint16_t)(((bvnc)&PVR_BVNC_PACK_MASK_V) >> PVR_BVNC_PACK_SHIFT_V))

/**
 * Unpacks N value (number of scalable units) from packed BVNC.
 *
 * \param bvnc Packed BVNC.
 * \return Number of scalable units.
 *
 * \sa #PVR_BVNC_UNPACK_B(), #PVR_BVNC_UNPACK_V(), #PVR_BVNC_UNPACK_C(),
 * #pvr_get_packed_bvnc() and #PVR_BVNC_PACK()
 */
#define PVR_BVNC_UNPACK_N(bvnc) \
   ((uint16_t)(((bvnc)&PVR_BVNC_PACK_MASK_N) >> PVR_BVNC_PACK_SHIFT_N))

/**
 * Unpacks C value (config ID) from packed BVNC.
 *
 * \param bvnc Packed BVNC.
 * \return Config ID.
 *
 * \sa #PVR_BVNC_UNPACK_B(), #PVR_BVNC_UNPACK_V(), #PVR_BVNC_UNPACK_N(),
 * #pvr_get_packed_bvnc() and #PVR_BVNC_PACK()
 */
#define PVR_BVNC_UNPACK_C(bvnc) \
   ((uint16_t)(((bvnc)&PVR_BVNC_PACK_MASK_C) >> PVR_BVNC_PACK_SHIFT_C))

/**
 * Tests whether a physical device has a given feature.
 *
 * Feature names are derived from those found in #pvr_device_features by
 * dropping the 'has_' prefix, which is applied by this macro.
 *
 * \param dev_info #pvr_device_info object associated with the target physical
 *                 device.
 * \param feature  Device feature name.
 *
 * \return
 *  * true if the named feature is present in the hardware.
 *  * false if the named feature is not present in the hardware.
 *
 * \sa #PVR_FEATURE_VALUE() and #PVR_GET_FEATURE_VALUE()
 */
#define PVR_HAS_FEATURE(dev_info, feature) ((dev_info)->features.has_##feature)

/**
 * Gets a physical device feature value if feature is supported.
 *
 * Feature names are derived from those found in #pvr_device_features by
 * dropping the 'has_' prefix.
 *
 * This macro should be used in preference to #PVR_GET_FEATURE_VALUE() as it has
 * proper error handling.
 *
 * \param dev_info  #pvr_device_info object associated with the target physical
 *                  device.
 * \param feature   Feature name.
 * \param value_out Feature value.
 *
 * \return
 *  * 0 on success, or
 *  * -%EINVAL if the named feature is not present in the hardware.
 *
 * \sa #PVR_HAS_FEATURE() and #PVR_GET_FEATURE_VALUE()
 */
#define PVR_FEATURE_VALUE(dev_info, feature, value_out)    \
   ({                                                      \
      const struct pvr_device_info *__dev_info = dev_info; \
      int __ret = -EINVAL;                                 \
      if (__dev_info->features.has_##feature) {            \
         *(value_out) = __dev_info->features.feature;      \
         __ret = 0;                                        \
      }                                                    \
      __ret;                                               \
   })

/**
 * Gets a physical device feature value if supported, but otherwise returns a
 * default value.
 *
 * Feature names are derived from those found in #pvr_device_features by
 * dropping the 'has_' prefix.
 *
 * #PVR_FEATURE_VALUE() should be used in preference to this macro when errors
 * can be returned by the caller. This macro is intended for cases where errors
 * can't be returned.
 *
 * \param dev_info      #pvr_device_info object associated with the target
 *                      physical device.
 * \param feature       Feature name.
 * \param default_value Default feature value.
 *
 * \return Feature value.
 *
 * \sa #PVR_HAS_FEATURE() and #PVR_FEATURE_VALUE()
 */
#define PVR_GET_FEATURE_VALUE(dev_info, feature, default_value)     \
   ({                                                               \
      const struct pvr_device_info *__dev_info = dev_info;          \
      __typeof__(default_value) __ret = default_value;              \
      if (__dev_info->features.has_##feature) {                     \
         __ret = __dev_info->features.feature;                      \
      } else {                                                      \
         mesa_logw("Missing " #feature                              \
                   " feature (defaulting to: " #default_value ")"); \
         assert(0);                                                 \
      }                                                             \
      __ret;                                                        \
   })

/**
 * Tests whether a physical device has a given enhancement.
 *
 * Enhancement numbers are derived from those found in #pvr_device_enhancements
 * by dropping the 'has_ern' prefix, which is applied by this macro.
 *
 * \param dev_info #pvr_device_info object associated with the target physical
 *                 device.
 * \param number   Enhancement number.
 *
 * \return
 *  * true if the enhancement is present in the hardware.
 *  * false if the enhancement is not present in the hardware.
 */
#define PVR_HAS_ERN(dev_info, number) ((dev_info)->enhancements.has_ern##number)

/**
 * Tests whether a physical device has a given quirk.
 *
 * Quirk numbers are derived from those found in #pvr_device_quirks by
 * dropping the 'has_brn' prefix, which is applied by this macro.
 *
 * \param dev_info #pvr_device_info object associated with the target physical
 *                 device.
 * \param number   Quirk number.
 *
 * \return
 *  * true if the quirk is present in the hardware.
 *  * false if the quirk is not present in the hardware.
 */
#define PVR_HAS_QUIRK(dev_info, number) ((dev_info)->quirks.has_brn##number)

struct pvr_device_ident {
   uint16_t b, v, n, c;
   uint32_t device_id;
   const char *series_name;
   const char *public_name;
};

struct pvr_device_features {
   bool has_astc : 1;
   bool has_cluster_grouping : 1;
   bool has_common_store_size_in_dwords : 1;
   bool has_compute : 1;
   bool has_compute_morton_capable : 1;
   bool has_compute_overlap : 1;
   bool has_eight_output_registers : 1;
   bool has_fb_cdc_v4 : 1;
   bool has_fbcdc_algorithm;
   bool has_gpu_multicore_support : 1;
   bool has_gs_rta_support : 1;
   bool has_ipf_creq_pf : 1;
   bool has_isp_max_tiles_in_flight : 1;
   bool has_isp_samples_per_pixel : 1;
   bool has_max_instances_per_pds_task : 1;
   bool has_max_multisample : 1;
   bool has_max_partitions : 1;
   bool has_max_usc_tasks : 1;
   bool has_num_clusters : 1;
   bool has_num_raster_pipes : 1;
   bool has_num_user_clip_planes : 1;
   bool has_paired_tiles : 1;
   bool has_pbe2_in_xe : 1;
   bool has_pbe_filterable_f16 : 1;
   bool has_pbe_yuv : 1;
   bool has_pds_ddmadt : 1;
   bool has_roguexe : 1;
   bool has_screen_size8K : 1;
   bool has_simple_internal_parameter_format : 1;
   bool has_simple_internal_parameter_format_v1 : 1;
   bool has_simple_internal_parameter_format_v2 : 1;
   bool has_simple_parameter_format_version : 1;
   bool has_slc_cache_line_size_bits : 1;
   bool has_slc_mcu_cache_controls : 1;
   bool has_tf_bicubic_filter : 1;
   bool has_tile_per_usc : 1;
   bool has_tile_size_16x16 : 1;
   bool has_tile_size_x : 1;
   bool has_tile_size_y : 1;
   bool has_tpu_array_textures : 1;
   bool has_tpu_border_colour_enhanced : 1;
   bool has_tpu_extended_integer_lookup : 1;
   bool has_tpu_image_state_v2 : 1;
   bool has_usc_f16sop_u8 : 1;
   bool has_usc_min_output_registers_per_pix : 1;
   bool has_usc_pixel_partition_mask : 1;
   bool has_usc_slots : 1;
   bool has_uvs_banks : 1;
   bool has_uvs_pba_entries : 1;
   bool has_uvs_vtx_entries : 1;
   bool has_vdm_cam_size : 1;
   bool has_vdm_degenerate_culling : 1;
   bool has_xpu_max_slaves : 1;
   bool has_xt_top_infrastructure : 1;
   bool has_zls_subtile : 1;

   uint32_t common_store_size_in_dwords;
   uint32_t fbcdc_algorithm;
   uint32_t isp_max_tiles_in_flight;
   uint32_t isp_samples_per_pixel;
   uint32_t max_instances_per_pds_task;
   uint32_t max_multisample;
   uint32_t max_partitions;
   uint32_t max_usc_tasks;
   uint32_t num_clusters;
   uint32_t num_raster_pipes;
   uint32_t num_user_clip_planes;
   uint32_t simple_parameter_format_version;
   uint32_t slc_cache_line_size_bits;
   uint32_t tile_size_x;
   uint32_t tile_size_y;
   uint32_t usc_min_output_registers_per_pix;
   uint32_t usc_slots;
   uint32_t uvs_banks;
   uint32_t uvs_pba_entries;
   uint32_t uvs_vtx_entries;
   uint32_t vdm_cam_size;
   uint32_t xpu_max_slaves;

   /* Derived features. */
   bool has_requires_fb_cdc_zls_setup : 1;
   bool has_s8xe : 1;
};

struct pvr_device_enhancements {
   bool has_ern35421 : 1;
   bool has_ern38020 : 1;
   bool has_ern38748 : 1;
   bool has_ern42064 : 1;
   bool has_ern42307 : 1;
   bool has_ern45493 : 1;
};

struct pvr_device_quirks {
   bool has_brn44079 : 1;
   bool has_brn47727 : 1;
   bool has_brn48492 : 1;
   bool has_brn48545 : 1;
   bool has_brn49032 : 1;
   bool has_brn49927 : 1;
   bool has_brn51025 : 1;
   bool has_brn51210 : 1;
   bool has_brn51764 : 1;
   bool has_brn52354 : 1;
   bool has_brn52942 : 1;
   bool has_brn58839 : 1;
   bool has_brn62269 : 1;
   bool has_brn66011 : 1;
   bool has_brn70165 : 1;
};

struct pvr_device_info {
   struct pvr_device_ident ident;
   struct pvr_device_features features;
   struct pvr_device_enhancements enhancements;
   struct pvr_device_quirks quirks;
};

struct pvr_device_runtime_info {
   uint64_t min_free_list_size;
   uint64_t max_free_list_size;
   uint64_t reserved_shared_size;
   uint64_t total_reserved_partition_size;
   uint64_t num_phantoms;
   uint64_t max_coeffs;
   uint64_t cdm_max_local_mem_size_regs;
   uint32_t core_count;
};

/**
 * Packs B, V, N and C values into a 64-bit unsigned integer.
 *
 * The packed layout is as follows:
 *
 * \verbatim
 *    +--------+--------+--------+-------+
 *    | 63..48 | 47..32 | 31..16 | 15..0 |
 *    +========+========+========+=======+
 *    | B      | V      | N      | C     |
 *    +--------+--------+--------+-------+
 * \endverbatim
 *
 * This should be used in preference to #PVR_BVNC_PACK() when a
 * #pvr_device_info is available in order to ensure proper type checking.
 *
 * \param dev_info Device information.
 * \return Packed BVNC.
 */
static ALWAYS_INLINE uint64_t
pvr_get_packed_bvnc(const struct pvr_device_info *dev_info)
{
   return PVR_BVNC_PACK(dev_info->ident.b,
                        dev_info->ident.v,
                        dev_info->ident.n,
                        dev_info->ident.c);
}

int pvr_device_info_init(struct pvr_device_info *info, uint64_t bvnc);

#endif /* PVR_DEVICE_INFO_H */
