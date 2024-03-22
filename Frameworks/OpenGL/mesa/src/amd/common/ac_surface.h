/*
 * Copyright © 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AC_SURFACE_H
#define AC_SURFACE_H

#include "amd_family.h"
#include "util/format/u_format.h"

/* NIR is optional. Some components don't want to include NIR with ac_surface.h. */
#ifdef AC_SURFACE_INCLUDE_NIR
#include "compiler/nir/nir_builder.h"
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations. */
struct ac_addrlib;

struct amdgpu_gpu_info;
struct radeon_info;

#define RADEON_SURF_MAX_LEVELS 15

enum radeon_surf_mode
{
   RADEON_SURF_MODE_LINEAR_ALIGNED = 1,
   RADEON_SURF_MODE_1D = 2,
   RADEON_SURF_MODE_2D = 3,
};

/* This describes D/S/Z/R swizzle modes.
 * Defined in the GB_TILE_MODEn.MICRO_TILE_MODE_NEW order.
 */
enum radeon_micro_mode
{
   RADEON_MICRO_MODE_DISPLAY = 0,
   RADEON_MICRO_MODE_STANDARD = 1,
   RADEON_MICRO_MODE_DEPTH = 2,
   RADEON_MICRO_MODE_RENDER = 3, /* gfx9 and older: rotated */
};

/* the first 16 bits are reserved for libdrm_radeon, don't use them */
#define RADEON_SURF_SCANOUT      (1 << 16)
#define RADEON_SURF_ZBUFFER      (1 << 17)
#define RADEON_SURF_SBUFFER      (1 << 18)
#define RADEON_SURF_Z_OR_SBUFFER (RADEON_SURF_ZBUFFER | RADEON_SURF_SBUFFER)
/* bits 19 and 20 are reserved for libdrm_radeon, don't use them */
#define RADEON_SURF_FMASK                 (1 << 21)
#define RADEON_SURF_DISABLE_DCC           (1ull << 22)
#define RADEON_SURF_TC_COMPATIBLE_HTILE   (1ull << 23)
#define RADEON_SURF_IMPORTED              (1ull << 24)
#define RADEON_SURF_CONTIGUOUS_DCC_LAYERS (1ull << 25)
#define RADEON_SURF_SHAREABLE             (1ull << 26)
#define RADEON_SURF_NO_RENDER_TARGET      (1ull << 27)
/* Force a swizzle mode (gfx9+) or tile mode (gfx6-8).
 * If this is not set, optimize for space. */
#define RADEON_SURF_FORCE_SWIZZLE_MODE    (1ull << 28)
#define RADEON_SURF_NO_FMASK              (1ull << 29)
#define RADEON_SURF_NO_HTILE              (1ull << 30)
#define RADEON_SURF_FORCE_MICRO_TILE_MODE (1ull << 31)
#define RADEON_SURF_PRT                   (1ull << 32)
#define RADEON_SURF_VRS_RATE              (1ull << 33)
/* Block compressed + linear format is not supported in addrlib. These surface can be
 * used as transfer resource. This flag indicates not to set flags.texture flag for
 * color surface in gfx9_compute_surface(). */
#define RADEON_SURF_NO_TEXTURE            (1ull << 34)
#define RADEON_SURF_NO_STENCIL_ADJUST     (1ull << 35)

struct legacy_surf_level {
   uint32_t offset_256B;   /* divided by 256, the hw can only do 40-bit addresses */
   uint32_t slice_size_dw; /* in dwords; max = 4GB / 4. */
   unsigned nblk_x : 15;
   unsigned nblk_y : 15;
   enum radeon_surf_mode mode : 2;
};

struct legacy_surf_dcc_level {
   uint32_t dcc_offset;    /* relative offset within DCC mip tree */
   uint32_t dcc_fast_clear_size;
   uint32_t dcc_slice_fast_clear_size;
};

struct legacy_surf_fmask {
   unsigned slice_tile_max; /* max 4M */
   uint8_t tiling_index;    /* max 31 */
   uint8_t bankh;           /* max 8 */
   uint16_t pitch_in_pixels;
};

struct legacy_surf_layout {
   unsigned bankw : 4;               /* max 8 */
   unsigned bankh : 4;               /* max 8 */
   unsigned mtilea : 4;              /* max 8 */
   unsigned tile_split : 13;         /* max 4K */
   unsigned stencil_tile_split : 13; /* max 4K */
   unsigned pipe_config : 5;         /* max 17 */
   unsigned num_banks : 5;           /* max 16 */
   unsigned macro_tile_index : 4;    /* max 15 */

   /* Whether the depth miptree or stencil miptree as used by the DB are
    * adjusted from their TC compatible form to ensure depth/stencil
    * compatibility. If either is true, the corresponding plane cannot be
    * sampled from.
    */
   unsigned depth_adjusted : 1;
   unsigned stencil_adjusted : 1;

   struct legacy_surf_level level[RADEON_SURF_MAX_LEVELS];
   uint8_t tiling_index[RADEON_SURF_MAX_LEVELS];

   union {
      /* Color layout */
      struct {
         struct legacy_surf_dcc_level dcc_level[RADEON_SURF_MAX_LEVELS];
         struct legacy_surf_fmask fmask;
         unsigned cmask_slice_tile_max;
      } color;

      /* Z/S layout */
      struct {
         struct legacy_surf_level stencil_level[RADEON_SURF_MAX_LEVELS];
         uint8_t stencil_tiling_index[RADEON_SURF_MAX_LEVELS];
      } zs;
   };
};

/* Same as addrlib - AddrResourceType. */
enum gfx9_resource_type
{
   RADEON_RESOURCE_1D = 0,
   RADEON_RESOURCE_2D,
   RADEON_RESOURCE_3D,
};

struct gfx9_surf_meta_flags {
   uint8_t rb_aligned : 1;   /* optimal for RBs */
   uint8_t pipe_aligned : 1; /* optimal for TC */
   uint8_t independent_64B_blocks : 1;
   uint8_t independent_128B_blocks : 1;
   uint8_t max_compressed_block_size : 2;
   uint8_t display_equation_valid : 1;
};

struct gfx9_surf_meta_level {
   unsigned offset;
   unsigned size; /* the size of one level in one layer (the image is an array of layers
                   * where each layer has an array of levels) */
};

/**
 * Meta address equation.
 *
 * DCC/HTILE address equation for doing DCC/HTILE address computations in shaders.
 *
 * ac_surface_meta_address_test.c contains the reference implementation.
 * ac_nir_{dcc,htile}_addr_from_coord is the NIR implementation.
 *
 * For DCC:
 * The gfx9 equation doesn't support mipmapping.
 * The gfx10 equation doesn't support mipmapping and MSAA.
 * (those are also limitations of Addr2ComputeDccAddrFromCoord)
 *
 * For HTILE:
 * The gfx9 equation isn't implemented.
 * The gfx10 equation doesn't support mipmapping.
 */
struct gfx9_meta_equation {
   uint16_t meta_block_width;
   uint16_t meta_block_height;
   uint16_t meta_block_depth;

   union {
      /* The gfx9 DCC equation is chip-specific, and it varies with:
       * - resource type
       * - swizzle_mode
       * - bpp
       * - number of samples
       * - number of fragments
       * - pipe_aligned
       * - rb_aligned
       */
      struct {
         uint8_t num_bits;
         uint8_t num_pipe_bits;

         struct {
            struct {
               uint8_t dim:3; /* 0..4 */
               uint8_t ord:5; /* 0..31 */
            } coord[5]; /* 0..num_coords-1 */
         } bit[20]; /* 0..num_bits-1 */
      } gfx9;

      /* The gfx10 DCC equation is chip-specific, it requires 64KB_R_X, and it varies with:
       * - bpp
       * - number of samples
       * - number of fragments
       * - pipe_aligned
       *
       * The gfx10 HTILE equation is chip-specific, it requires 64KB_Z_X, and it varies with:
       * - number of samples
       */
      uint16_t gfx10_bits[64];
   } u;
};

struct gfx9_surf_layout {
   uint16_t epitch;           /* gfx9 only, not on gfx10 */
   uint8_t swizzle_mode;      /* color or depth */
   bool uses_custom_pitch;    /* only used by gfx10.3+ */

   enum gfx9_resource_type resource_type:8; /* 1D, 2D or 3D */
   uint16_t surf_pitch;                   /* in blocks */
   uint16_t surf_height;

   uint64_t surf_offset; /* 0 unless imported with an offset */
   /* The size of the 2D plane containing all mipmap levels. */
   uint64_t surf_slice_size;
   /* Mipmap level offset within the slice in bytes. Only valid for LINEAR. */
   uint64_t offset[RADEON_SURF_MAX_LEVELS];  /* up to 16K * 16K * 16 * ~1.33 */
   /* Mipmap level pitch in elements. Only valid for LINEAR. */
   uint16_t pitch[RADEON_SURF_MAX_LEVELS];

   uint16_t base_mip_width;
   uint16_t base_mip_height;

   /* Pitch of level in blocks, only valid for prt images. */
   uint16_t prt_level_pitch[RADEON_SURF_MAX_LEVELS];
   /* Offset within slice in bytes, only valid for prt images. */
   uint64_t prt_level_offset[RADEON_SURF_MAX_LEVELS]; /* up to 64K * 64K * 16 * ~1.33 */

   /* DCC or HTILE level info */
   struct gfx9_surf_meta_level meta_levels[RADEON_SURF_MAX_LEVELS];

   union {
      /* Color */
      struct {
         struct gfx9_surf_meta_flags dcc; /* metadata of color */
         uint8_t fmask_swizzle_mode;
         uint16_t fmask_epitch;     /* gfx9 only, not on gfx10 */

         uint16_t dcc_pitch_max;
         uint16_t dcc_height;

         uint8_t dcc_block_width;
         uint8_t dcc_block_height;
         uint8_t dcc_block_depth;

         /* Displayable DCC. This is always rb_aligned=0 and pipe_aligned=0.
          * The 3D engine doesn't support that layout except for chips with 1 RB.
          * All other chips must set rb_aligned=1.
          * A compute shader needs to convert from aligned DCC to unaligned.
          */
         uint8_t display_dcc_alignment_log2;
         uint32_t display_dcc_size;
         uint16_t display_dcc_pitch_max; /* (mip chain pitch - 1) */
         uint16_t display_dcc_height;
         bool dcc_retile_use_uint16;     /* if all values fit into uint16_t */
         uint32_t dcc_retile_num_elements;
         void *dcc_retile_map;

         /* CMASK level info (only level 0) */
         struct gfx9_surf_meta_level cmask_level0;

         /* For DCC retiling. */
         struct gfx9_meta_equation dcc_equation; /* 2D only */
         struct gfx9_meta_equation display_dcc_equation;

         /* For FCE compute. */
         struct gfx9_meta_equation cmask_equation; /* 2D only */
      } color;

      /* Z/S */
      struct {
         uint64_t stencil_offset; /* separate stencil */
         uint16_t stencil_epitch;   /* gfx9 only, not on gfx10 */
         uint8_t stencil_swizzle_mode;

         /* For HTILE VRS. */
         struct gfx9_meta_equation htile_equation;
      } zs;
   };
};

struct radeon_surf {
   /* Format properties. */
   uint8_t blk_w : 4;
   uint8_t blk_h : 4;
   uint8_t bpe : 5;
   /* Display, standard(thin), depth, render(rotated). AKA D,S,Z,R swizzle modes. */
   uint8_t micro_tile_mode : 3;
   /* Number of mipmap levels where DCC or HTILE is enabled starting from level 0.
    * Non-zero levels may be disabled due to alignment constraints, but not
    * the first level.
    */
   uint8_t num_meta_levels : 4;
   uint8_t is_linear : 1;
   uint8_t has_stencil : 1;
   /* This might be true even if micro_tile_mode isn't displayable or rotated. */
   uint8_t is_displayable : 1;
   uint8_t first_mip_tail_level : 4;

   /* These are return values. Some of them can be set by the caller, but
    * they will be treated as hints (e.g. bankw, bankh) and might be
    * changed by the calculator.
    */

   /* Not supported yet for depth + stencil. */
   uint16_t prt_tile_width;   /* up to 256 roughly (for 64KB tiles) */
   uint16_t prt_tile_height;  /* up to 256 roughly (for 64KB tiles) */
   uint16_t prt_tile_depth;   /* up to 32 roughly (for 64KB thick tiles) */

   /* Tile swizzle can be OR'd with low bits of the BASE_256B address.
    * The value is the same for all mipmap levels. Supported tile modes:
    * - GFX6: Only macro tiling.
    * - GFX9: Only *_X and *_T swizzle modes. Level 0 must not be in the mip
    *   tail.
    *
    * Only these surfaces are allowed to set it:
    * - color (if it doesn't have to be displayable)
    * - DCC (same tile swizzle as color)
    * - FMASK
    * - CMASK if it's TC-compatible or if the gen is GFX9
    * - depth/stencil if HTILE is not TC-compatible and if the gen is not GFX9
    */
   uint16_t tile_swizzle; /* it has 16 bits because gfx11 shifts it by 2 bits */
   uint8_t fmask_tile_swizzle;

   /* Use (1 << log2) to compute the alignment. */
   uint8_t surf_alignment_log2;
   uint8_t fmask_alignment_log2;
   uint8_t meta_alignment_log2; /* DCC or HTILE */
   uint8_t cmask_alignment_log2;
   uint8_t alignment_log2;

   /* DRM format modifier. Set to DRM_FORMAT_MOD_INVALID to have addrlib
    * select tiling parameters instead.
    */
   uint64_t modifier;
   uint64_t flags;

   uint64_t surf_size;
   uint64_t fmask_size;
   uint32_t fmask_slice_size; /* max 2^31 (16K * 16K * 8) */

   /* DCC and HTILE (they are very small) */
   uint32_t meta_size;
   uint32_t meta_slice_size;
   uint32_t meta_pitch;

   uint32_t cmask_size;
   uint32_t cmask_slice_size;
   uint16_t cmask_pitch; /* GFX9+ */
   uint16_t cmask_height; /* GFX9+ */

   /* All buffers combined. */
   uint64_t meta_offset; /* DCC or HTILE */
   uint64_t fmask_offset;
   uint64_t cmask_offset;
   uint64_t display_dcc_offset;
   uint64_t total_size;

   union {
      /* Return values for GFX8 and older.
       *
       * Some of them can be set by the caller if certain parameters are
       * desirable. The allocator will try to obey them.
       */
      struct legacy_surf_layout legacy;

      /* GFX9+ return values. */
      struct gfx9_surf_layout gfx9;
   } u;
};

struct ac_surf_info {
   uint32_t width;
   uint32_t height;
   uint32_t depth;
   uint8_t samples;         /* For Z/S: samples; For color: FMASK coverage samples */
   uint8_t storage_samples; /* For color: allocated samples */
   uint8_t levels;
   uint8_t num_channels; /* heuristic for displayability */
   uint16_t array_size;
   uint32_t *surf_index; /* Set a monotonic counter for tile swizzling. */
   uint32_t *fmask_surf_index;
};

struct ac_surf_config {
   struct ac_surf_info info;
   unsigned is_1d : 1;
   unsigned is_3d : 1;
   unsigned is_cube : 1;
   unsigned is_array : 1;
};

/* Output parameters for ac_surface_compute_nbc_view */
struct ac_surf_nbc_view {
   bool valid;
   uint32_t width;
   uint32_t height;
   uint32_t level;
   uint32_t num_levels; /* Used for max_mip in the resource descriptor */
   uint8_t tile_swizzle;
   uint64_t base_address_offset;
};

struct ac_addrlib *ac_addrlib_create(const struct radeon_info *info, uint64_t *max_alignment);
void ac_addrlib_destroy(struct ac_addrlib *addrlib);
void *ac_addrlib_get_handle(struct ac_addrlib *addrlib);

int ac_compute_surface(struct ac_addrlib *addrlib, const struct radeon_info *info,
                       const struct ac_surf_config *config, enum radeon_surf_mode mode,
                       struct radeon_surf *surf);
void ac_surface_zero_dcc_fields(struct radeon_surf *surf);
unsigned ac_pipe_config_to_num_pipes(unsigned pipe_config);

void ac_surface_apply_bo_metadata(const struct radeon_info *info, struct radeon_surf *surf,
                                  uint64_t tiling_flags, enum radeon_surf_mode *mode);
void ac_surface_compute_bo_metadata(const struct radeon_info *info, struct radeon_surf *surf,
                                    uint64_t *tiling_flags);

bool ac_surface_apply_umd_metadata(const struct radeon_info *info, struct radeon_surf *surf,
                                   unsigned num_storage_samples, unsigned num_mipmap_levels,
                                   unsigned size_metadata, const uint32_t metadata[64]);
void ac_surface_compute_umd_metadata(const struct radeon_info *info, struct radeon_surf *surf,
                                     unsigned num_mipmap_levels, uint32_t desc[8],
                                     unsigned *size_metadata, uint32_t metadata[64],
                                     bool include_tool_md);

bool ac_surface_override_offset_stride(const struct radeon_info *info, struct radeon_surf *surf,
                                       unsigned num_layers, unsigned num_mipmap_levels,
                                       uint64_t offset, unsigned pitch);

struct ac_modifier_options {
	bool dcc; /* Whether to allow DCC. */
	bool dcc_retile; /* Whether to allow use of a DCC retile map. */
};

bool ac_is_modifier_supported(const struct radeon_info *info,
                              const struct ac_modifier_options *options,
                              enum pipe_format format,
                              uint64_t modifier);
bool ac_get_supported_modifiers(const struct radeon_info *info,
                                const struct ac_modifier_options *options,
                                enum pipe_format format,
                                unsigned *mod_count,
                                uint64_t *mods);
bool ac_modifier_has_dcc(uint64_t modifier);
bool ac_modifier_has_dcc_retile(uint64_t modifier);
bool ac_modifier_supports_dcc_image_stores(enum amd_gfx_level gfx_level, uint64_t modifier);
void ac_modifier_max_extent(const struct radeon_info *info,
                            uint64_t modifier, uint32_t *width, uint32_t *height);

unsigned ac_surface_get_nplanes(const struct radeon_surf *surf);
uint64_t ac_surface_get_plane_offset(enum amd_gfx_level gfx_level,
                                     const struct radeon_surf *surf,
                                     unsigned plane, unsigned layer);
uint64_t ac_surface_get_plane_stride(enum amd_gfx_level gfx_level,
                                     const struct radeon_surf *surf,
                                     unsigned plane, unsigned level);
/* Of the whole miplevel, not an individual layer */
uint64_t ac_surface_get_plane_size(const struct radeon_surf *surf,
                                   unsigned plane);

uint64_t ac_surface_addr_from_coord(struct ac_addrlib *addrlib, const struct radeon_info *info,
                                    const struct radeon_surf *surf,
                                    const struct ac_surf_info *surf_info, unsigned level,
                                    unsigned x, unsigned y, unsigned layer, bool is_3d);
void ac_surface_compute_nbc_view(struct ac_addrlib *addrlib, const struct radeon_info *info,
                                 const struct radeon_surf *surf,
                                 const struct ac_surf_info *surf_info, unsigned level,
                                 unsigned layer, struct ac_surf_nbc_view *out);

void ac_surface_print_info(FILE *out, const struct radeon_info *info,
                           const struct radeon_surf *surf);

bool ac_surface_supports_dcc_image_stores(enum amd_gfx_level gfx_level,
                                          const struct radeon_surf *surf);
unsigned ac_get_cb_number_type(enum pipe_format format);
unsigned ac_get_cb_format(enum amd_gfx_level gfx_level, enum pipe_format format);

#ifdef AC_SURFACE_INCLUDE_NIR
nir_def *ac_nir_dcc_addr_from_coord(nir_builder *b, const struct radeon_info *info,
                                        unsigned bpe, struct gfx9_meta_equation *equation,
                                        nir_def *dcc_pitch, nir_def *dcc_height,
                                        nir_def *dcc_slice_size,
                                        nir_def *x, nir_def *y, nir_def *z,
                                        nir_def *sample, nir_def *pipe_xor);

nir_def *ac_nir_cmask_addr_from_coord(nir_builder *b, const struct radeon_info *info,
                                        struct gfx9_meta_equation *equation,
                                        nir_def *cmask_pitch, nir_def *cmask_height,
                                        nir_def *cmask_slice_size,
                                        nir_def *x, nir_def *y, nir_def *z,
                                        nir_def *pipe_xor,
                                        nir_def **bit_position);

nir_def *ac_nir_htile_addr_from_coord(nir_builder *b, const struct radeon_info *info,
                                          struct gfx9_meta_equation *equation,
                                          nir_def *htile_pitch,
                                          nir_def *htile_slice_size,
                                          nir_def *x, nir_def *y, nir_def *z,
                                          nir_def *pipe_xor);
#endif

#ifdef __cplusplus
}
#endif

#endif /* AC_SURFACE_H */
