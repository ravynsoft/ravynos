/*
 * Copyright © 2013 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util/libdrm.h"

#include "intel_device_info.h"
#include "intel_wa.h"
#include "i915/intel_device_info.h"
#include "xe/intel_device_info.h"

#include "util/u_debug.h"
#include "util/log.h"
#include "util/macros.h"

static const struct {
   const char *name;
   int pci_id;
} name_map[] = {
   { "lpt", 0x27a2 },
   { "brw", 0x2a02 },
   { "g4x", 0x2a42 },
   { "ilk", 0x0042 },
   { "snb", 0x0126 },
   { "ivb", 0x016a },
   { "hsw", 0x0d2e },
   { "byt", 0x0f33 },
   { "bdw", 0x162e },
   { "chv", 0x22B3 },
   { "skl", 0x1912 },
   { "bxt", 0x5A85 },
   { "kbl", 0x5912 },
   { "aml", 0x591C },
   { "glk", 0x3185 },
   { "cfl", 0x3E9B },
   { "whl", 0x3EA1 },
   { "cml", 0x9b41 },
   { "icl", 0x8a52 },
   { "ehl", 0x4571 },
   { "jsl", 0x4E71 },
   { "tgl", 0x9a49 },
   { "rkl", 0x4c8a },
   { "dg1", 0x4905 },
   { "adl", 0x4680 },
   { "sg1", 0x4907 },
   { "rpl", 0xa780 },
   { "dg2", 0x5690 },
   { "mtl", 0x7d60 },
};

/**
 * Get the PCI ID for the device name.
 *
 * Returns -1 if the device is not known.
 */
int
intel_device_name_to_pci_device_id(const char *name)
{
   for (unsigned i = 0; i < ARRAY_SIZE(name_map); i++) {
      if (!strcmp(name_map[i].name, name))
         return name_map[i].pci_id;
   }

   return -1;
}

static const struct intel_device_info intel_device_info_gfx3 = {
   .ver = 3,
   .platform = INTEL_PLATFORM_GFX3,
   .simulator_id = -1,
   .num_slices = 1,
   .num_subslices = { 1, },
   .max_eus_per_subslice = 8,
   .num_thread_per_eu = 4,
   .timestamp_frequency = 12500000,
};

static const struct intel_device_info intel_device_info_i965 = {
   .ver = 4,
   .platform = INTEL_PLATFORM_I965,
   .has_negative_rhw_bug = true,
   .num_slices = 1,
   .num_subslices = { 1, },
   .max_eus_per_subslice = 8,
   .num_thread_per_eu = 4,
   .max_vs_threads = 16,
   .max_gs_threads = 2,
   .max_wm_threads = 8 * 4,
   .urb = {
      .size = 256,
   },
   .timestamp_frequency = 12500000,
   .simulator_id = -1,
};

static const struct intel_device_info intel_device_info_g4x = {
   .ver = 4,
   .verx10 = 45,
   .has_pln = true,
   .has_compr4 = true,
   .has_surface_tile_offset = true,
   .platform = INTEL_PLATFORM_G4X,
   .num_slices = 1,
   .num_subslices = { 1, },
   .max_eus_per_subslice = 10,
   .num_thread_per_eu = 5,
   .max_vs_threads = 32,
   .max_gs_threads = 2,
   .max_wm_threads = 10 * 5,
   .urb = {
      .size = 384,
   },
   .timestamp_frequency = 12500000,
   .simulator_id = -1,
};

static const struct intel_device_info intel_device_info_ilk = {
   .ver = 5,
   .platform = INTEL_PLATFORM_ILK,
   .has_pln = true,
   .has_compr4 = true,
   .has_surface_tile_offset = true,
   .num_slices = 1,
   .num_subslices = { 1, },
   .max_eus_per_subslice = 12,
   .num_thread_per_eu = 6,
   .max_vs_threads = 72,
   .max_gs_threads = 32,
   .max_wm_threads = 12 * 6,
   .urb = {
      .size = 1024,
   },
   .timestamp_frequency = 12500000,
   .simulator_id = -1,
};

static const struct intel_device_info intel_device_info_snb_gt1 = {
   .ver = 6,
   .gt = 1,
   .platform = INTEL_PLATFORM_SNB,
   .has_hiz_and_separate_stencil = true,
   .has_llc = true,
   .has_pln = true,
   .has_surface_tile_offset = true,
   .needs_unlit_centroid_workaround = true,
   .num_slices = 1,
   .num_subslices = { 1, },
   .max_eus_per_subslice = 6,
   .num_thread_per_eu = 6, /* Not confirmed */
   .max_vs_threads = 24,
   .max_gs_threads = 21, /* conservative; 24 if rendering disabled. */
   .max_wm_threads = 40,
   .urb = {
      .size = 32,
      .min_entries = {
         [MESA_SHADER_VERTEX]   = 24,
      },
      .max_entries = {
         [MESA_SHADER_VERTEX]   = 256,
         [MESA_SHADER_GEOMETRY] = 256,
      },
   },
   .timestamp_frequency = 12500000,
   .simulator_id = -1,
};

static const struct intel_device_info intel_device_info_snb_gt2 = {
   .ver = 6,
   .gt = 2,
   .platform = INTEL_PLATFORM_SNB,
   .has_hiz_and_separate_stencil = true,
   .has_llc = true,
   .has_pln = true,
   .has_surface_tile_offset = true,
   .needs_unlit_centroid_workaround = true,
   .num_slices = 1,
   .num_subslices = { 1, },
   .max_eus_per_subslice = 12,
   .num_thread_per_eu = 6, /* Not confirmed */
   .max_vs_threads = 60,
   .max_gs_threads = 60,
   .max_wm_threads = 80,
   .urb = {
      .size = 64,
      .min_entries = {
         [MESA_SHADER_VERTEX]   = 24,
      },
      .max_entries = {
         [MESA_SHADER_VERTEX]   = 256,
         [MESA_SHADER_GEOMETRY] = 256,
      },
   },
   .timestamp_frequency = 12500000,
   .simulator_id = -1,
};

#define GFX7_FEATURES                               \
   .ver = 7,                                        \
   .has_hiz_and_separate_stencil = true,            \
   .must_use_separate_stencil = true,               \
   .has_llc = true,                                 \
   .has_pln = true,                                 \
   .has_64bit_float = true,                         \
   .has_surface_tile_offset = true,                 \
   .timestamp_frequency = 12500000,                 \
   .max_constant_urb_size_kb = 16

static const struct intel_device_info intel_device_info_ivb_gt1 = {
   GFX7_FEATURES, .platform = INTEL_PLATFORM_IVB, .gt = 1,
   .num_slices = 1,
   .num_subslices = { 1, },
   .max_eus_per_subslice = 6,
   .num_thread_per_eu = 6,
   .l3_banks = 2,
   .max_vs_threads = 36,
   .max_tcs_threads = 36,
   .max_tes_threads = 36,
   .max_gs_threads = 36,
   .max_wm_threads = 48,
   .max_cs_threads = 36,
   .urb = {
      .min_entries = {
         [MESA_SHADER_VERTEX]    = 32,
         [MESA_SHADER_TESS_EVAL] = 10,
      },
      .max_entries = {
         [MESA_SHADER_VERTEX]    = 512,
         [MESA_SHADER_TESS_CTRL] = 32,
         [MESA_SHADER_TESS_EVAL] = 288,
         [MESA_SHADER_GEOMETRY]  = 192,
      },
   },
   .simulator_id = 7,
};

static const struct intel_device_info intel_device_info_ivb_gt2 = {
   GFX7_FEATURES, .platform = INTEL_PLATFORM_IVB, .gt = 2,
   .num_slices = 1,
   .num_subslices = { 1, },
   .max_eus_per_subslice = 12,
   .num_thread_per_eu = 8, /* Not sure why this isn't a multiple of
                            * @max_wm_threads ... */
   .l3_banks = 4,
   .max_vs_threads = 128,
   .max_tcs_threads = 128,
   .max_tes_threads = 128,
   .max_gs_threads = 128,
   .max_wm_threads = 172,
   .max_cs_threads = 64,
   .urb = {
      .min_entries = {
         [MESA_SHADER_VERTEX]    = 32,
         [MESA_SHADER_TESS_EVAL] = 10,
      },
      .max_entries = {
         [MESA_SHADER_VERTEX]    = 704,
         [MESA_SHADER_TESS_CTRL] = 64,
         [MESA_SHADER_TESS_EVAL] = 448,
         [MESA_SHADER_GEOMETRY]  = 320,
      },
   },
   .simulator_id = 7,
};

static const struct intel_device_info intel_device_info_byt = {
   GFX7_FEATURES, .platform = INTEL_PLATFORM_BYT, .gt = 1,
   .num_slices = 1,
   .num_subslices = { 1, },
   .max_eus_per_subslice = 4,
   .num_thread_per_eu = 8,
   .l3_banks = 1,
   .has_llc = false,
   .max_vs_threads = 36,
   .max_tcs_threads = 36,
   .max_tes_threads = 36,
   .max_gs_threads = 36,
   .max_wm_threads = 48,
   .max_cs_threads = 32,
   .urb = {
      .min_entries = {
         [MESA_SHADER_VERTEX]    = 32,
         [MESA_SHADER_TESS_EVAL] = 10,
      },
      .max_entries = {
         [MESA_SHADER_VERTEX]    = 512,
         [MESA_SHADER_TESS_CTRL] = 32,
         [MESA_SHADER_TESS_EVAL] = 288,
         [MESA_SHADER_GEOMETRY]  = 192,
      },
   },
   .simulator_id = 10,
};

#define HSW_FEATURES \
   GFX7_FEATURES, \
   .platform = INTEL_PLATFORM_HSW, \
   .verx10 = 75, \
   .supports_simd16_3src = true

static const struct intel_device_info intel_device_info_hsw_gt1 = {
   HSW_FEATURES, .gt = 1,
   .num_slices = 1,
   .num_subslices = { 1, },
   .max_eus_per_subslice = 10,
   .num_thread_per_eu = 7,
   .l3_banks = 2,
   .max_vs_threads = 70,
   .max_tcs_threads = 70,
   .max_tes_threads = 70,
   .max_gs_threads = 70,
   .max_wm_threads = 102,
   .max_cs_threads = 70,
   .urb = {
      .min_entries = {
         [MESA_SHADER_VERTEX]    = 32,
         [MESA_SHADER_TESS_EVAL] = 10,
      },
      .max_entries = {
         [MESA_SHADER_VERTEX]    = 640,
         [MESA_SHADER_TESS_CTRL] = 64,
         [MESA_SHADER_TESS_EVAL] = 384,
         [MESA_SHADER_GEOMETRY]  = 256,
      },
   },
   .simulator_id = 9,
};

static const struct intel_device_info intel_device_info_hsw_gt2 = {
   HSW_FEATURES, .gt = 2,
   .num_slices = 1,
   .num_subslices = { 2, },
   .max_eus_per_subslice = 10,
   .num_thread_per_eu = 7,
   .l3_banks = 4,
   .max_vs_threads = 280,
   .max_tcs_threads = 256,
   .max_tes_threads = 280,
   .max_gs_threads = 256,
   .max_wm_threads = 204,
   .max_cs_threads = 70,
   .urb = {
      .min_entries = {
         [MESA_SHADER_VERTEX]    = 64,
         [MESA_SHADER_TESS_EVAL] = 10,
      },
      .max_entries = {
         [MESA_SHADER_VERTEX]    = 1664,
         [MESA_SHADER_TESS_CTRL] = 128,
         [MESA_SHADER_TESS_EVAL] = 960,
         [MESA_SHADER_GEOMETRY]  = 640,
      },
   },
   .simulator_id = 9,
};

static const struct intel_device_info intel_device_info_hsw_gt3 = {
   HSW_FEATURES, .gt = 3,
   .num_slices = 2,
   .num_subslices = { 2, 2, },
   .max_eus_per_subslice = 10,
   .num_thread_per_eu = 7,
   .l3_banks = 8,
   .max_vs_threads = 280,
   .max_tcs_threads = 256,
   .max_tes_threads = 280,
   .max_gs_threads = 256,
   .max_wm_threads = 408,
   .max_cs_threads = 70,
   .urb = {
      .min_entries = {
         [MESA_SHADER_VERTEX]    = 64,
         [MESA_SHADER_TESS_EVAL] = 10,
      },
      .max_entries = {
         [MESA_SHADER_VERTEX]    = 1664,
         [MESA_SHADER_TESS_CTRL] = 128,
         [MESA_SHADER_TESS_EVAL] = 960,
         [MESA_SHADER_GEOMETRY]  = 640,
      },
   },
   .max_constant_urb_size_kb = 32,
   .simulator_id = 9,
};

/* It's unclear how well supported sampling from the hiz buffer is on GFX8,
 * so keep things conservative for now and set has_sample_with_hiz = false.
 */
#define GFX8_FEATURES                               \
   .ver = 8,                                        \
   .has_hiz_and_separate_stencil = true,            \
   .must_use_separate_stencil = true,               \
   .has_llc = true,                                 \
   .has_sample_with_hiz = false,                    \
   .has_pln = true,                                 \
   .has_integer_dword_mul = true,                   \
   .has_64bit_float = true,                         \
   .has_64bit_int = true,                           \
   .supports_simd16_3src = true,                    \
   .has_surface_tile_offset = true,                 \
   .num_thread_per_eu = 7,                          \
   .max_vs_threads = 504,                           \
   .max_tcs_threads = 504,                          \
   .max_tes_threads = 504,                          \
   .max_gs_threads = 504,                           \
   .max_wm_threads = 384,                           \
   .max_threads_per_psd = 64,                       \
   .timestamp_frequency = 12500000,                 \
   .max_constant_urb_size_kb = 32

static const struct intel_device_info intel_device_info_bdw_gt1 = {
   GFX8_FEATURES, .gt = 1,
   .platform = INTEL_PLATFORM_BDW,
   .num_slices = 1,
   .num_subslices = { 2, },
   .max_eus_per_subslice = 6,
   .l3_banks = 2,
   .max_cs_threads = 42,
   .urb = {
      .min_entries = {
         [MESA_SHADER_VERTEX]    = 64,
         [MESA_SHADER_TESS_EVAL] = 34,
      },
      .max_entries = {
         [MESA_SHADER_VERTEX]    = 2560,
         [MESA_SHADER_TESS_CTRL] = 504,
         [MESA_SHADER_TESS_EVAL] = 1536,
         /* Reduced from 960, seems to be similar to the bug on Gfx9 GT1. */
         [MESA_SHADER_GEOMETRY]  = 690,
      },
   },
   .simulator_id = 11,
};

static const struct intel_device_info intel_device_info_bdw_gt2 = {
   GFX8_FEATURES, .gt = 2,
   .platform = INTEL_PLATFORM_BDW,
   .num_slices = 1,
   .num_subslices = { 3, },
   .max_eus_per_subslice = 8,
   .l3_banks = 4,
   .max_cs_threads = 56,
   .urb = {
      .min_entries = {
         [MESA_SHADER_VERTEX]    = 64,
         [MESA_SHADER_TESS_EVAL] = 34,
      },
      .max_entries = {
         [MESA_SHADER_VERTEX]    = 2560,
         [MESA_SHADER_TESS_CTRL] = 504,
         [MESA_SHADER_TESS_EVAL] = 1536,
         [MESA_SHADER_GEOMETRY]  = 960,
      },
   },
   .simulator_id = 11,
};

static const struct intel_device_info intel_device_info_bdw_gt3 = {
   GFX8_FEATURES, .gt = 3,
   .platform = INTEL_PLATFORM_BDW,
   .num_slices = 2,
   .num_subslices = { 3, 3, },
   .max_eus_per_subslice = 8,
   .l3_banks = 8,
   .max_cs_threads = 56,
   .urb = {
      .min_entries = {
         [MESA_SHADER_VERTEX]    = 64,
         [MESA_SHADER_TESS_EVAL] = 34,
      },
      .max_entries = {
         [MESA_SHADER_VERTEX]    = 2560,
         [MESA_SHADER_TESS_CTRL] = 504,
         [MESA_SHADER_TESS_EVAL] = 1536,
         [MESA_SHADER_GEOMETRY]  = 960,
      },
   },
   .simulator_id = 11,
};

static const struct intel_device_info intel_device_info_chv = {
   GFX8_FEATURES, .platform = INTEL_PLATFORM_CHV, .gt = 1,
   .has_llc = false,
   .has_integer_dword_mul = false,
   .num_slices = 1,
   .num_subslices = { 2, },
   .max_eus_per_subslice = 8,
   .l3_banks = 2,
   .max_vs_threads = 80,
   .max_tcs_threads = 80,
   .max_tes_threads = 80,
   .max_gs_threads = 80,
   .max_wm_threads = 128,
   .max_cs_threads = 6 * 7,
   .urb = {
      .min_entries = {
         [MESA_SHADER_VERTEX]    = 34,
         [MESA_SHADER_TESS_EVAL] = 34,
      },
      .max_entries = {
         [MESA_SHADER_VERTEX]    = 640,
         [MESA_SHADER_TESS_CTRL] = 80,
         [MESA_SHADER_TESS_EVAL] = 384,
         [MESA_SHADER_GEOMETRY]  = 256,
      },
   },
   .simulator_id = 13,
};

#define GFX9_HW_INFO                                \
   .ver = 9,                                        \
   .max_vs_threads = 336,                           \
   .max_gs_threads = 336,                           \
   .max_tcs_threads = 336,                          \
   .max_tes_threads = 336,                          \
   .max_threads_per_psd = 64,                       \
   .max_cs_threads = 56,                            \
   .timestamp_frequency = 12000000,                 \
   .urb = {                                         \
      .min_entries = {                              \
         [MESA_SHADER_VERTEX]    = 64,              \
         [MESA_SHADER_TESS_EVAL] = 34,              \
      },                                            \
      .max_entries = {                              \
         [MESA_SHADER_VERTEX]    = 1856,            \
         [MESA_SHADER_TESS_CTRL] = 672,             \
         [MESA_SHADER_TESS_EVAL] = 1120,            \
         [MESA_SHADER_GEOMETRY]  = 640,             \
      },                                            \
   }

#define GFX9_LP_FEATURES                           \
   GFX8_FEATURES,                                  \
   GFX9_HW_INFO,                                   \
   .has_integer_dword_mul = false,                 \
   .gt = 1,                                        \
   .has_llc = false,                               \
   .has_sample_with_hiz = true,                    \
   .has_illegal_ccs_values = true,                 \
   .num_slices = 1,                                \
   .num_thread_per_eu = 6,                         \
   .max_vs_threads = 112,                          \
   .max_tcs_threads = 112,                         \
   .max_tes_threads = 112,                         \
   .max_gs_threads = 112,                          \
   .max_cs_threads = 6 * 6,                        \
   .timestamp_frequency = 19200000,                \
   .urb = {                                        \
      .min_entries = {                             \
         [MESA_SHADER_VERTEX]    = 34,             \
         [MESA_SHADER_TESS_EVAL] = 34,             \
      },                                           \
      .max_entries = {                             \
         [MESA_SHADER_VERTEX]    = 704,            \
         [MESA_SHADER_TESS_CTRL] = 256,            \
         [MESA_SHADER_TESS_EVAL] = 416,            \
         [MESA_SHADER_GEOMETRY]  = 256,            \
      },                                           \
   }

#define GFX9_LP_FEATURES_3X6                       \
   GFX9_LP_FEATURES,                               \
   .num_subslices = { 3, },                        \
   .max_eus_per_subslice = 6

#define GFX9_LP_FEATURES_2X6                       \
   GFX9_LP_FEATURES,                               \
   .num_subslices = { 2, },                        \
   .max_eus_per_subslice = 6,                       \
   .max_vs_threads = 56,                           \
   .max_tcs_threads = 56,                          \
   .max_tes_threads = 56,                          \
   .max_gs_threads = 56,                           \
   .max_cs_threads = 6 * 6,                        \
   .urb = {                                        \
      .min_entries = {                             \
         [MESA_SHADER_VERTEX]    = 34,             \
         [MESA_SHADER_TESS_EVAL] = 34,             \
      },                                           \
      .max_entries = {                             \
         [MESA_SHADER_VERTEX]    = 352,            \
         [MESA_SHADER_TESS_CTRL] = 128,            \
         [MESA_SHADER_TESS_EVAL] = 208,            \
         [MESA_SHADER_GEOMETRY]  = 128,            \
      },                                           \
   }

#define GFX9_FEATURES                               \
   GFX8_FEATURES,                                   \
   GFX9_HW_INFO,                                    \
   .has_sample_with_hiz = true,                     \
   .has_illegal_ccs_values = true,                                    \
   .cooperative_matrix_configurations = {                             \
    { SCOPE_SUBGROUP, 8, 8, 16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16 }, \
    { SCOPE_SUBGROUP, 8, 8, 16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT32, INTEL_CMAT_FLOAT32 }, \
    { SCOPE_SUBGROUP, 8, 8, 32, INTEL_CMAT_SINT8, INTEL_CMAT_SINT8, INTEL_CMAT_SINT32, INTEL_CMAT_SINT32 },       \
    { SCOPE_SUBGROUP, 8, 8, 32, INTEL_CMAT_UINT8, INTEL_CMAT_UINT8, INTEL_CMAT_UINT32, INTEL_CMAT_UINT32 },       \
   }

static const struct intel_device_info intel_device_info_skl_gt1 = {
   GFX9_FEATURES, .gt = 1,
   .platform = INTEL_PLATFORM_SKL,
   .num_slices = 1,
   .num_subslices = { 2, },
   .max_eus_per_subslice = 6,
   .l3_banks = 2,
   /* GT1 seems to have a bug in the top of the pipe (VF/VS?) fixed functions
    * leading to some vertices to go missing if we use too much URB.
    */
   .urb.max_entries[MESA_SHADER_VERTEX] = 928,
   .simulator_id = 12,
};

static const struct intel_device_info intel_device_info_skl_gt2 = {
   GFX9_FEATURES, .gt = 2,
   .platform = INTEL_PLATFORM_SKL,
   .num_slices = 1,
   .num_subslices = { 3, },
   .max_eus_per_subslice = 8,
   .l3_banks = 4,
   .simulator_id = 12,
};

static const struct intel_device_info intel_device_info_skl_gt3 = {
   GFX9_FEATURES, .gt = 3,
   .platform = INTEL_PLATFORM_SKL,
   .num_slices = 2,
   .num_subslices = { 3, 3, },
   .max_eus_per_subslice = 8,
   .l3_banks = 8,
   .simulator_id = 12,
};

static const struct intel_device_info intel_device_info_skl_gt4 = {
   GFX9_FEATURES, .gt = 4,
   .platform = INTEL_PLATFORM_SKL,
   .num_slices = 3,
   .num_subslices = { 3, 3, 3, },
   .max_eus_per_subslice = 8,
   .l3_banks = 12,
   /* From the "L3 Allocation and Programming" documentation:
    *
    * "URB is limited to 1008KB due to programming restrictions.  This is not a
    * restriction of the L3 implementation, but of the FF and other clients.
    * Therefore, in a GT4 implementation it is possible for the programmed
    * allocation of the L3 data array to provide 3*384KB=1152KB for URB, but
    * only 1008KB of this will be used."
    */
   .simulator_id = 12,
};

static const struct intel_device_info intel_device_info_bxt = {
   GFX9_LP_FEATURES_3X6,
   .platform = INTEL_PLATFORM_BXT,
   .l3_banks = 2,
   .simulator_id = 14,
};

static const struct intel_device_info intel_device_info_bxt_2x6 = {
   GFX9_LP_FEATURES_2X6,
   .platform = INTEL_PLATFORM_BXT,
   .l3_banks = 1,
   .simulator_id = 14,
};
/*
 * Note: for all KBL SKUs, the PRM says SKL for GS entries, not SKL+.
 * There's no KBL entry. Using the default SKL (GFX9) GS entries value.
 */

static const struct intel_device_info intel_device_info_kbl_gt1 = {
   GFX9_FEATURES,
   .platform = INTEL_PLATFORM_KBL,
   .gt = 1,

   .max_cs_threads = 7 * 6,
   .num_slices = 1,
   .num_subslices = { 2, },
   .max_eus_per_subslice = 6,
   .l3_banks = 2,
   /* GT1 seems to have a bug in the top of the pipe (VF/VS?) fixed functions
    * leading to some vertices to go missing if we use too much URB.
    */
   .urb.max_entries[MESA_SHADER_VERTEX] = 928,
   .urb.max_entries[MESA_SHADER_GEOMETRY] = 256,
   .simulator_id = 16,
};

static const struct intel_device_info intel_device_info_kbl_gt1_5 = {
   GFX9_FEATURES,
   .platform = INTEL_PLATFORM_KBL,
   .gt = 1,

   .max_cs_threads = 7 * 6,
   .num_slices = 1,
   .num_subslices = { 3, },
   .max_eus_per_subslice = 6,
   .l3_banks = 4,
   .simulator_id = 16,
};

static const struct intel_device_info intel_device_info_kbl_gt2 = {
   GFX9_FEATURES,
   .platform = INTEL_PLATFORM_KBL,
   .gt = 2,

   .num_slices = 1,
   .num_subslices = { 3, },
   .max_eus_per_subslice = 8,
   .l3_banks = 4,
   .simulator_id = 16,
};

static const struct intel_device_info intel_device_info_kbl_gt3 = {
   GFX9_FEATURES,
   .platform = INTEL_PLATFORM_KBL,
   .gt = 3,

   .num_slices = 2,
   .num_subslices = { 3, 3, },
   .max_eus_per_subslice = 8,
   .l3_banks = 8,
   .simulator_id = 16,
};

static const struct intel_device_info intel_device_info_kbl_gt4 = {
   GFX9_FEATURES,
   .platform = INTEL_PLATFORM_KBL,
   .gt = 4,

   /*
    * From the "L3 Allocation and Programming" documentation:
    *
    * "URB is limited to 1008KB due to programming restrictions.  This
    *  is not a restriction of the L3 implementation, but of the FF and
    *  other clients.  Therefore, in a GT4 implementation it is
    *  possible for the programmed allocation of the L3 data array to
    *  provide 3*384KB=1152KB for URB, but only 1008KB of this
    *  will be used."
    */
   .num_slices = 3,
   .num_subslices = { 3, 3, 3, },
   .max_eus_per_subslice = 8,
   .l3_banks = 12,
   .simulator_id = 16,
};

static const struct intel_device_info intel_device_info_glk = {
   GFX9_LP_FEATURES_3X6,
   .platform = INTEL_PLATFORM_GLK,
   .l3_banks = 2,
   .simulator_id = 17,
};

static const struct intel_device_info intel_device_info_glk_2x6 = {
   GFX9_LP_FEATURES_2X6,
   .platform = INTEL_PLATFORM_GLK,
   .l3_banks = 2,
   .simulator_id = 17,
};

static const struct intel_device_info intel_device_info_cfl_gt1 = {
   GFX9_FEATURES,
   .platform = INTEL_PLATFORM_CFL,
   .gt = 1,

   .num_slices = 1,
   .num_subslices = { 2, },
   .max_eus_per_subslice = 6,
   .l3_banks = 2,
   /* GT1 seems to have a bug in the top of the pipe (VF/VS?) fixed functions
    * leading to some vertices to go missing if we use too much URB.
    */
   .urb.max_entries[MESA_SHADER_VERTEX] = 928,
   .urb.max_entries[MESA_SHADER_GEOMETRY] = 256,
   .simulator_id = 24,
};
static const struct intel_device_info intel_device_info_cfl_gt2 = {
   GFX9_FEATURES,
   .platform = INTEL_PLATFORM_CFL,
   .gt = 2,

   .num_slices = 1,
   .num_subslices = { 3, },
   .max_eus_per_subslice = 8,
   .l3_banks = 4,
   .simulator_id = 24,
};

static const struct intel_device_info intel_device_info_cfl_gt3 = {
   GFX9_FEATURES,
   .platform = INTEL_PLATFORM_CFL,
   .gt = 3,

   .num_slices = 2,
   .num_subslices = { 3, 3, },
   .max_eus_per_subslice = 8,
   .l3_banks = 8,
   .simulator_id = 24,
};

#define subslices(args...) { args, }

#define GFX11_HW_INFO                               \
   .ver = 11,                                       \
   .has_pln = false,                                \
   .max_vs_threads = 364,                           \
   .max_gs_threads = 224,                           \
   .max_tcs_threads = 224,                          \
   .max_tes_threads = 364,                          \
   .max_threads_per_psd = 64,                       \
   .max_cs_threads = 56

#define GFX11_FEATURES(_gt, _slices, _subslices, _l3, _platform)  \
   GFX8_FEATURES,                                     \
   GFX11_HW_INFO,                                     \
   .platform = _platform,                             \
   .has_64bit_float = false,                          \
   .has_64bit_int = false,                            \
   .has_integer_dword_mul = false,                    \
   .has_sample_with_hiz = false,                      \
   .has_illegal_ccs_values = true,                    \
   .gt = _gt, .num_slices = _slices, .l3_banks = _l3, \
   .num_subslices = _subslices,                       \
   .max_eus_per_subslice = 8,                                         \
   .cooperative_matrix_configurations = {                             \
    { SCOPE_SUBGROUP, 8, 8, 16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16 }, \
    { SCOPE_SUBGROUP, 8, 8, 16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT32, INTEL_CMAT_FLOAT32 }, \
    { SCOPE_SUBGROUP, 8, 8, 32, INTEL_CMAT_SINT8, INTEL_CMAT_SINT8, INTEL_CMAT_SINT32, INTEL_CMAT_SINT32 },       \
    { SCOPE_SUBGROUP, 8, 8, 32, INTEL_CMAT_UINT8, INTEL_CMAT_UINT8, INTEL_CMAT_UINT32, INTEL_CMAT_UINT32 },       \
   }

#define GFX11_URB_MIN_MAX_ENTRIES                     \
   .min_entries = {                                   \
      [MESA_SHADER_VERTEX]    = 64,                   \
      [MESA_SHADER_TESS_EVAL] = 34,                   \
   },                                                 \
   .max_entries = {                                   \
      [MESA_SHADER_VERTEX]    = 2384,                 \
      [MESA_SHADER_TESS_CTRL] = 1032,                 \
      [MESA_SHADER_TESS_EVAL] = 2384,                 \
      [MESA_SHADER_GEOMETRY]  = 1032,                 \
   }

static const struct intel_device_info intel_device_info_icl_gt2 = {
   GFX11_FEATURES(2, 1, subslices(8), 8, INTEL_PLATFORM_ICL),
   .urb = {
      GFX11_URB_MIN_MAX_ENTRIES,
   },
   .simulator_id = 19,
};

static const struct intel_device_info intel_device_info_icl_gt1_5 = {
   GFX11_FEATURES(1, 1, subslices(6), 6, INTEL_PLATFORM_ICL),
   .urb = {
      GFX11_URB_MIN_MAX_ENTRIES,
   },
   .simulator_id = 19,
};

static const struct intel_device_info intel_device_info_icl_gt1 = {
   GFX11_FEATURES(1, 1, subslices(4), 6, INTEL_PLATFORM_ICL),
   .urb = {
      GFX11_URB_MIN_MAX_ENTRIES,
   },
   .simulator_id = 19,
};

static const struct intel_device_info intel_device_info_icl_gt0_5 = {
   GFX11_FEATURES(1, 1, subslices(1), 6, INTEL_PLATFORM_ICL),
   .urb = {
      GFX11_URB_MIN_MAX_ENTRIES,
   },
   .simulator_id = 19,
};

#define GFX11_LP_FEATURES                           \
   .urb = {                                         \
      GFX11_URB_MIN_MAX_ENTRIES,                    \
   },                                               \
   .disable_ccs_repack = true,                      \
   .has_illegal_ccs_values = true,                  \
   .simulator_id = 28

static const struct intel_device_info intel_device_info_ehl_4x8 = {
   GFX11_FEATURES(1, 1, subslices(4), 4, INTEL_PLATFORM_EHL),
   GFX11_LP_FEATURES,
};

static const struct intel_device_info intel_device_info_ehl_4x6 = {
   GFX11_FEATURES(1, 1, subslices(4), 4, INTEL_PLATFORM_EHL),
   GFX11_LP_FEATURES,
   .max_eus_per_subslice = 6,
};

static const struct intel_device_info intel_device_info_ehl_4x5 = {
   GFX11_FEATURES(1, 1, subslices(4), 4, INTEL_PLATFORM_EHL),
   GFX11_LP_FEATURES,
   .max_eus_per_subslice = 5,
};

static const struct intel_device_info intel_device_info_ehl_4x4 = {
   GFX11_FEATURES(1, 1, subslices(4), 4, INTEL_PLATFORM_EHL),
   GFX11_LP_FEATURES,
   .max_eus_per_subslice = 4,
};

static const struct intel_device_info intel_device_info_ehl_2x8 = {
   GFX11_FEATURES(1, 1, subslices(2), 4, INTEL_PLATFORM_EHL),
   GFX11_LP_FEATURES,
};

static const struct intel_device_info intel_device_info_ehl_2x4 = {
   GFX11_FEATURES(1, 1, subslices(2), 4, INTEL_PLATFORM_EHL),
   GFX11_LP_FEATURES,
   .max_eus_per_subslice = 4,
};

#define GFX12_HW_INFO                               \
   .ver = 12,                                       \
   .has_pln = false,                                \
   .has_sample_with_hiz = false,                    \
   .has_aux_map = true,                             \
   .max_vs_threads = 546,                           \
   .max_gs_threads = 336,                           \
   .max_tcs_threads = 336,                          \
   .max_tes_threads = 546,                          \
   .max_threads_per_psd = 64,                       \
   .max_cs_threads = 112, /* threads per DSS */     \
   .urb = {                                         \
      .size = 512, /* For intel_stub_gpu */         \
      .min_entries = {                              \
         [MESA_SHADER_VERTEX]    = 64,              \
         [MESA_SHADER_TESS_EVAL] = 34,              \
      },                                            \
      .max_entries = {                              \
         [MESA_SHADER_VERTEX]    = 3576,            \
         [MESA_SHADER_TESS_CTRL] = 1548,            \
         [MESA_SHADER_TESS_EVAL] = 3576,            \
         [MESA_SHADER_GEOMETRY]  = 1548,            \
      },                                            \
   }

#define GFX12_FEATURES(_gt, _slices, _l3)                       \
   GFX8_FEATURES,                                               \
   GFX12_HW_INFO,                                               \
   .has_64bit_float = false,                                    \
   .has_64bit_int = false,                                      \
   .has_integer_dword_mul = false,                              \
   .gt = _gt, .num_slices = _slices, .l3_banks = _l3,           \
   .simulator_id = 22,                                          \
   .max_eus_per_subslice = 16,                                  \
   .pat = {                                                     \
         .cached_coherent = PAT_ENTRY(0, WB, 2WAY),             \
         .scanout = PAT_ENTRY(1, WC, NONE),                     \
         .writeback_incoherent = PAT_ENTRY(0, WB, 2WAY),        \
         .writecombining = PAT_ENTRY(1, WC, NONE),              \
   },                                                           \
   .cooperative_matrix_configurations = {                       \
    { SCOPE_SUBGROUP, 8, 8, 16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16 }, \
    { SCOPE_SUBGROUP, 8, 8, 16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT32, INTEL_CMAT_FLOAT32 }, \
    { SCOPE_SUBGROUP, 8, 8, 32, INTEL_CMAT_SINT8, INTEL_CMAT_SINT8, INTEL_CMAT_SINT32, INTEL_CMAT_SINT32 },       \
    { SCOPE_SUBGROUP, 8, 8, 32, INTEL_CMAT_UINT8, INTEL_CMAT_UINT8, INTEL_CMAT_UINT32, INTEL_CMAT_UINT32 },       \
   }

#define dual_subslices(args...) { args, }

#define GFX12_GT05_FEATURES                                     \
   GFX12_FEATURES(1, 1, 4),                                     \
   .num_subslices = dual_subslices(1)

#define GFX12_GT_FEATURES(_gt)                                  \
   GFX12_FEATURES(_gt, 1, _gt == 1 ? 4 : 8),                    \
   .num_subslices = dual_subslices(_gt == 1 ? 2 : 6)

static const struct intel_device_info intel_device_info_tgl_gt1 = {
   GFX12_GT_FEATURES(1),
   .platform = INTEL_PLATFORM_TGL,
};

static const struct intel_device_info intel_device_info_tgl_gt2 = {
   GFX12_GT_FEATURES(2),
   .platform = INTEL_PLATFORM_TGL,
};

static const struct intel_device_info intel_device_info_rkl_gt05 = {
   GFX12_GT05_FEATURES,
   .platform = INTEL_PLATFORM_RKL,
};

static const struct intel_device_info intel_device_info_rkl_gt1 = {
   GFX12_GT_FEATURES(1),
   .platform = INTEL_PLATFORM_RKL,
};

static const struct intel_device_info intel_device_info_adl_gt05 = {
   GFX12_GT05_FEATURES,
   .platform = INTEL_PLATFORM_ADL,
   .display_ver = 13,
};

static const struct intel_device_info intel_device_info_adl_gt1 = {
   GFX12_GT_FEATURES(1),
   .platform = INTEL_PLATFORM_ADL,
   .display_ver = 13,
};

static const struct intel_device_info intel_device_info_adl_n = {
   GFX12_GT_FEATURES(1),
   .platform = INTEL_PLATFORM_ADL,
   .display_ver = 13,
   .is_adl_n = true,
};

static const struct intel_device_info intel_device_info_adl_gt2 = {
   GFX12_GT_FEATURES(2),
   .platform = INTEL_PLATFORM_ADL,
   .display_ver = 13,
};

static const struct intel_device_info intel_device_info_rpl = {
   GFX12_FEATURES(1, 1, 4),
   .num_subslices = dual_subslices(2),
   .platform = INTEL_PLATFORM_RPL,
   .display_ver = 13,
};

static const struct intel_device_info intel_device_info_rpl_p = {
   GFX12_GT_FEATURES(2),
   .platform = INTEL_PLATFORM_RPL,
   .display_ver = 13,
};

#define GFX12_DG1_SG1_FEATURES                           \
   GFX12_GT_FEATURES(2),                                 \
   .platform = INTEL_PLATFORM_DG1,                       \
   .has_llc = false,                                     \
   .has_local_mem = true,                                \
   .urb.size = 768,                                      \
   .simulator_id = 30,                                   \
   /* There is no PAT table for DG1, using TGL one */    \
   .pat = {                                              \
         .cached_coherent = PAT_ENTRY(0, WB, 2WAY),      \
         .scanout = PAT_ENTRY(1, WC, NONE),              \
         .writeback_incoherent = PAT_ENTRY(0, WB, 2WAY), \
         .writecombining = PAT_ENTRY(1, WC, NONE),       \
   }

static const struct intel_device_info intel_device_info_dg1 = {
   GFX12_DG1_SG1_FEATURES,
};

static const struct intel_device_info intel_device_info_sg1 = {
   GFX12_DG1_SG1_FEATURES,
};

#define XEHP_URB_MIN_MAX_ENTRIES                        \
   .min_entries = {                                     \
      [MESA_SHADER_VERTEX]    = 64,                     \
      [MESA_SHADER_TESS_EVAL] = 34,                     \
   },                                                   \
   .max_entries = {                                     \
      [MESA_SHADER_VERTEX]    = 3832, /* BSpec 47138 */ \
      [MESA_SHADER_TESS_CTRL] = 1548, /* BSpec 47137 */ \
      [MESA_SHADER_TESS_EVAL] = 3576, /* BSpec 47135 */ \
      [MESA_SHADER_GEOMETRY]  = 1548, /* BSpec 47136 */ \
   }

#define XEHP_FEATURES(_gt, _slices, _l3)                        \
   GFX8_FEATURES,                                               \
   .has_64bit_float = false,                                    \
   .has_64bit_int = false,                                      \
   .has_integer_dword_mul = false,                              \
   .gt = _gt, .num_slices = _slices, .l3_banks = _l3,           \
   .num_subslices = dual_subslices(1), /* updated by topology */\
   .ver = 12,                                                   \
   .has_pln = false,                                            \
   .has_sample_with_hiz = false,                                \
   .max_vs_threads = 546,  /* BSpec 46312 */                    \
   .max_gs_threads = 336,  /* BSpec 46299 */                    \
   .max_tcs_threads = 336, /* BSpec 46300 */                    \
   .max_tes_threads = 546, /* BSpec 46298 */                    \
   .max_threads_per_psd = 64,                                   \
   .max_cs_threads = 112, /* threads per DSS */                 \
   .urb = {                                                     \
      .size = 768, /* For intel_stub_gpu */                     \
      XEHP_URB_MIN_MAX_ENTRIES,                                 \
   },                                                           \
   .num_thread_per_eu = 8 /* BSpec 44472 */,                    \
   .max_eus_per_subslice = 16,                                  \
   .verx10 = 125,                                               \
   .has_llc = false,                                            \
   .has_lsc = true,                                             \
   .has_local_mem = true,                                       \
   .has_aux_map = false,                                        \
   .simulator_id = 29,                                          \
   .cooperative_matrix_configurations = {                       \
    { SCOPE_SUBGROUP, 8, 8, 16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16 }, \
    { SCOPE_SUBGROUP, 8, 8, 16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT16, INTEL_CMAT_FLOAT32, INTEL_CMAT_FLOAT32 }, \
    { SCOPE_SUBGROUP, 8, 8, 32, INTEL_CMAT_SINT8, INTEL_CMAT_SINT8, INTEL_CMAT_SINT32, INTEL_CMAT_SINT32 },       \
    { SCOPE_SUBGROUP, 8, 8, 32, INTEL_CMAT_UINT8, INTEL_CMAT_UINT8, INTEL_CMAT_UINT32, INTEL_CMAT_UINT32 },       \
   }

#define DG2_FEATURES                                            \
   /* (Sub)slice info comes from the kernel topology info */    \
   XEHP_FEATURES(0, 1, 0),                                      \
   .display_ver = 13,                                           \
   .revision = 4, /* For offline compiler */                    \
   .apply_hwconfig = true,                                      \
   .has_coarse_pixel_primitive_and_cb = true,                   \
   .has_mesh_shading = true,                                    \
   .has_ray_tracing = true,                                     \
   .has_flat_ccs = true,                                        \
   /* There is no PAT table for DG2, using TGL ones */          \
   .pat = {                                                     \
         .cached_coherent = PAT_ENTRY(0, WB, 1WAY),             \
         .scanout = PAT_ENTRY(1, WC, NONE),                     \
         .writeback_incoherent = PAT_ENTRY(0, WB, 2WAY),        \
         .writecombining = PAT_ENTRY(1, WC, NONE),              \
   }

static const struct intel_device_info intel_device_info_dg2_g10 = {
   DG2_FEATURES,
   .platform = INTEL_PLATFORM_DG2_G10,
};

static const struct intel_device_info intel_device_info_dg2_g11 = {
   DG2_FEATURES,
   .platform = INTEL_PLATFORM_DG2_G11,
};

static const struct intel_device_info intel_device_info_dg2_g12 = {
   DG2_FEATURES,
   .platform = INTEL_PLATFORM_DG2_G12,
};

static const struct intel_device_info intel_device_info_atsm_g10 = {
   DG2_FEATURES,
   .platform = INTEL_PLATFORM_ATSM_G10,
};

static const struct intel_device_info intel_device_info_atsm_g11 = {
   DG2_FEATURES,
   .platform = INTEL_PLATFORM_ATSM_G11,
};

#define MTL_FEATURES                                            \
   /* (Sub)slice info comes from the kernel topology info */    \
   XEHP_FEATURES(0, 1, 0),                                      \
   .has_local_mem = false,                                      \
   .has_aux_map = true,                                         \
   .apply_hwconfig = true,                                      \
   .has_64bit_float = true,                                     \
   .has_64bit_float_via_math_pipe = true,                       \
   .has_integer_dword_mul = false,                              \
   .has_coarse_pixel_primitive_and_cb = true,                   \
   .has_mesh_shading = true,                                    \
   .has_ray_tracing = true,                                     \
   .pat = {                                                     \
         .cached_coherent = PAT_ENTRY(3, WB, 1WAY),             \
         .scanout = PAT_ENTRY(1, WC, NONE),                     \
         .writeback_incoherent = PAT_ENTRY(0, WB, NONE),        \
         .writecombining = PAT_ENTRY(1, WC, NONE),              \
   }

static const struct intel_device_info intel_device_info_mtl_u = {
   MTL_FEATURES,
   .platform = INTEL_PLATFORM_MTL_U,
};

static const struct intel_device_info intel_device_info_mtl_h = {
   MTL_FEATURES,
   .platform = INTEL_PLATFORM_MTL_H,
};

void
intel_device_info_topology_reset_masks(struct intel_device_info *devinfo)
{
   devinfo->subslice_slice_stride = 0;
   devinfo->eu_subslice_stride = 0;
   devinfo->eu_slice_stride = 0;

   devinfo->num_slices = 0;
   memset(devinfo->num_subslices, 0, sizeof(devinfo->num_subslices));

   memset(&devinfo->slice_masks, 0, sizeof(devinfo->slice_masks));
   memset(devinfo->subslice_masks, 0, sizeof(devinfo->subslice_masks));
   memset(devinfo->eu_masks, 0, sizeof(devinfo->eu_masks));
   memset(devinfo->ppipe_subslices, 0, sizeof(devinfo->ppipe_subslices));
}

void
intel_device_info_topology_update_counts(struct intel_device_info *devinfo)
{
   devinfo->num_slices = __builtin_popcount(devinfo->slice_masks);
   devinfo->subslice_total = 0;
   for (int s = 0; s < devinfo->max_slices; s++) {
      if (!intel_device_info_slice_available(devinfo, s))
         continue;

      for (int b = 0; b < devinfo->subslice_slice_stride; b++) {
         devinfo->num_subslices[s] +=
            __builtin_popcount(devinfo->subslice_masks[s * devinfo->subslice_slice_stride + b]);
      }
      devinfo->subslice_total += devinfo->num_subslices[s];
   }
   assert(devinfo->num_slices > 0);
   assert(devinfo->subslice_total > 0);
}

void
intel_device_info_update_pixel_pipes(struct intel_device_info *devinfo, uint8_t *subslice_masks)
{
   if (devinfo->ver < 11)
      return;

   /* The kernel only reports one slice on all existing ICL+ platforms, even
    * if multiple slices are present. The slice mask is allowed to have the
    * accurate value greater than 1 on gfx12.5+ platforms though, in order to
    * be tolerant with the behavior of our simulation environment.
    */
   assert(devinfo->slice_masks == 1 || devinfo->verx10 >= 125);

   /* Count the number of subslices on each pixel pipe. Assume that every
    * contiguous group of 4 subslices in the mask belong to the same pixel
    * pipe. However note that on TGL+ the kernel returns a mask of enabled
    * *dual* subslices instead of actual subslices somewhat confusingly, so
    * each pixel pipe only takes 2 bits in the mask even though it's still 4
    * subslices.
    */
   const unsigned ppipe_bits = devinfo->ver >= 12 ? 2 : 4;
   for (unsigned p = 0; p < INTEL_DEVICE_MAX_PIXEL_PIPES; p++) {
      const unsigned offset = p * ppipe_bits;
      const unsigned subslice_idx = offset /
         devinfo->max_subslices_per_slice * devinfo->subslice_slice_stride;
      const unsigned ppipe_mask =
         BITFIELD_RANGE(offset % devinfo->max_subslices_per_slice, ppipe_bits);

      if (subslice_idx < ARRAY_SIZE(devinfo->subslice_masks))
         devinfo->ppipe_subslices[p] =
            __builtin_popcount(subslice_masks[subslice_idx] & ppipe_mask);
      else
         devinfo->ppipe_subslices[p] = 0;
   }
}

void
intel_device_info_update_l3_banks(struct intel_device_info *devinfo)
{
   if (devinfo->ver != 12)
      return;

   if (devinfo->verx10 >= 125) {
      if (devinfo->subslice_total > 16) {
         assert(devinfo->subslice_total <= 32);
         devinfo->l3_banks = 32;
      } else if (devinfo->subslice_total > 8) {
         devinfo->l3_banks = 16;
      } else {
         devinfo->l3_banks = 8;
      }
   } else {
      assert(devinfo->num_slices == 1);
      if (devinfo->subslice_total >= 6) {
         assert(devinfo->subslice_total == 6);
         devinfo->l3_banks = 8;
      } else if (devinfo->subslice_total > 2) {
         devinfo->l3_banks = 6;
      } else {
         devinfo->l3_banks = 4;
      }
   }
}

/* Generate mask from the device data. */
static void
fill_masks(struct intel_device_info *devinfo)
{
   /* All of our internal device descriptions assign the same number of
    * subslices for each slice. Just verify that this is true.
    */
   for (int s = 1; s < devinfo->num_slices; s++)
      assert(devinfo->num_subslices[0] == devinfo->num_subslices[s]);

   intel_device_info_i915_update_from_masks(devinfo,
                          (1U << devinfo->num_slices) - 1,
                          (1U << devinfo->num_subslices[0]) - 1,
                          devinfo->num_slices * devinfo->num_subslices[0] *
                          devinfo->max_eus_per_subslice);
}

void
intel_device_info_update_cs_workgroup_threads(struct intel_device_info *devinfo)
{
   /* GPGPU_WALKER::ThreadWidthCounterMaximum is U6-1 so the most threads we
    * can program is 64 without going up to a rectangular group. This only
    * impacts Haswell and TGL which have higher thread counts.
    *
    * INTERFACE_DESCRIPTOR_DATA::NumberofThreadsinGPGPUThreadGroup on Xe-HP+
    * is 10 bits so we have no such restrictions.
    */
   devinfo->max_cs_workgroup_threads =
      devinfo->verx10 >= 125 ? devinfo->max_cs_threads :
                               MIN2(devinfo->max_cs_threads, 64);
}

static bool
intel_device_info_init_common(int pci_id,
                              struct intel_device_info *devinfo)
{
   switch (pci_id) {
#undef CHIPSET
#define CHIPSET(id, family, fam_str, name) \
      case id: *devinfo = intel_device_info_##family; break;
#include "pci_ids/crocus_pci_ids.h"
#include "pci_ids/iris_pci_ids.h"

#undef CHIPSET
#define CHIPSET(id, fam_str, name) \
      case id: *devinfo = intel_device_info_gfx3; break;
#include "pci_ids/i915_pci_ids.h"

   default:
      mesa_logw("Driver does not support the 0x%x PCI ID.", pci_id);
      return false;
   }

   switch (pci_id) {
#undef CHIPSET
#define CHIPSET(_id, _family, _fam_str, _name) \
   case _id: \
      /* sizeof(str_literal) includes the null */ \
      STATIC_ASSERT(sizeof(_name) + sizeof(_fam_str) + 2 <= \
                    sizeof(devinfo->name)); \
      strncpy(devinfo->name, _name " (" _fam_str ")", sizeof(devinfo->name)); \
      break;
#include "pci_ids/crocus_pci_ids.h"
#include "pci_ids/iris_pci_ids.h"
   default:
      strncpy(devinfo->name, "Intel Unknown", sizeof(devinfo->name));
   }

   devinfo->pci_device_id = pci_id;

   fill_masks(devinfo);

   /* From the Skylake PRM, 3DSTATE_PS::Scratch Space Base Pointer:
    *
    * "Scratch Space per slice is computed based on 4 sub-slices.  SW must
    *  allocate scratch space enough so that each slice has 4 slices allowed."
    *
    * The equivalent internal documentation says that this programming note
    * applies to all Gfx9+ platforms.
    *
    * The hardware typically calculates the scratch space pointer by taking
    * the base address, and adding per-thread-scratch-space * thread ID.
    * Extra padding can be necessary depending how the thread IDs are
    * calculated for a particular shader stage.
    */

   switch(devinfo->ver) {
   case 9:
      devinfo->max_wm_threads = 64 /* threads-per-PSD */
                              * devinfo->num_slices
                              * 4; /* effective subslices per slice */
      break;
   case 11:
   case 12:
   case 20:
      devinfo->max_wm_threads = 128 /* threads-per-PSD */
                              * devinfo->num_slices
                              * 8; /* subslices per slice */
      break;
   default:
      assert(devinfo->ver < 9);
      break;
   }

   assert(devinfo->num_slices <= ARRAY_SIZE(devinfo->num_subslices));

   if (devinfo->verx10 == 0)
      devinfo->verx10 = devinfo->ver * 10;

   if (devinfo->display_ver == 0)
      devinfo->display_ver = devinfo->ver;

   if (devinfo->has_mesh_shading) {
      /* Half of push constant space matches the size used in the simplest
       * primitive pipeline (VS + FS). Tweaking this affects performance.
       */
      devinfo->mesh_max_constant_urb_size_kb =
            devinfo->max_constant_urb_size_kb / 2;
   }

   intel_device_info_update_cs_workgroup_threads(devinfo);

   return true;
}

static void
intel_device_info_apply_workarounds(struct intel_device_info *devinfo)
{
   if (intel_needs_workaround(devinfo, 18012660806))
      devinfo->urb.max_entries[MESA_SHADER_GEOMETRY] = 1536;

   /* Fixes issues with:
    * dEQP-GLES31.functional.geometry_shading.layered.render_with_default_layer_cubemap
    * when running on GFX12 platforms with small EU count.
    */
   const uint32_t eu_total = intel_device_info_eu_total(devinfo);
   if (devinfo->verx10 == 120 && eu_total <= 32)
      devinfo->urb.max_entries[MESA_SHADER_GEOMETRY] = 1024;
}

bool
intel_get_device_info_from_pci_id(int pci_id,
                                  struct intel_device_info *devinfo)
{
   intel_device_info_init_common(pci_id, devinfo);

   /* This is a placeholder until a proper value is set. */
   devinfo->kmd_type = INTEL_KMD_TYPE_I915;

   intel_device_info_init_was(devinfo);
   intel_device_info_apply_workarounds(devinfo);

   return true;
}

bool
intel_device_info_compute_system_memory(struct intel_device_info *devinfo, bool update)
{
   uint64_t total_phys;
   if (!os_get_total_physical_memory(&total_phys))
      return false;

   uint64_t available = 0;
   os_get_available_system_memory(&available);

   if (!update)
      devinfo->mem.sram.mappable.size = total_phys;
   else
      assert(devinfo->mem.sram.mappable.size == total_phys);

   devinfo->mem.sram.mappable.free = available;

   return true;
}

static void
init_max_scratch_ids(struct intel_device_info *devinfo)
{
   /* Determine the max number of subslices that potentially might be used in
    * scratch space ids.
    *
    * For, Gfx11+, scratch space allocation is based on the number of threads
    * in the base configuration.
    *
    * For Gfx9, devinfo->subslice_total is the TOTAL number of subslices and
    * we wish to view that there are 4 subslices per slice instead of the
    * actual number of subslices per slice. The documentation for 3DSTATE_PS
    * "Scratch Space Base Pointer" says:
    *
    *    "Scratch Space per slice is computed based on 4 sub-slices.  SW
    *     must allocate scratch space enough so that each slice has 4
    *     slices allowed."
    *
    * According to the other driver team, this applies to compute shaders
    * as well.  This is not currently documented at all.
    *
    * For Gfx8 and older we user devinfo->subslice_total.
    */
   unsigned subslices;
   if (devinfo->verx10 == 125)
      subslices = 32;
   else if (devinfo->ver == 12)
      subslices = (devinfo->platform == INTEL_PLATFORM_DG1 || devinfo->gt == 2 ? 6 : 2);
   else if (devinfo->ver == 11)
      subslices = 8;
   else if (devinfo->ver >= 9 && devinfo->ver < 11)
      subslices = 4 * devinfo->num_slices;
   else
      subslices = devinfo->subslice_total;
   assert(subslices >= devinfo->subslice_total);

   unsigned scratch_ids_per_subslice;
   if (devinfo->ver >= 12) {
      /* Same as ICL below, but with 16 EUs. */
      scratch_ids_per_subslice = 16 * 8;
   } else if (devinfo->ver >= 11) {
      /* The MEDIA_VFE_STATE docs say:
       *
       *    "Starting with this configuration, the Maximum Number of
       *     Threads must be set to (#EU * 8) for GPGPU dispatches.
       *
       *     Although there are only 7 threads per EU in the configuration,
       *     the FFTID is calculated as if there are 8 threads per EU,
       *     which in turn requires a larger amount of Scratch Space to be
       *     allocated by the driver."
       */
      scratch_ids_per_subslice = 8 * 8;
   } else if (devinfo->platform == INTEL_PLATFORM_HSW) {
      /* WaCSScratchSize:hsw
       *
       * Haswell's scratch space address calculation appears to be sparse
       * rather than tightly packed. The Thread ID has bits indicating
       * which subslice, EU within a subslice, and thread within an EU it
       * is. There's a maximum of two slices and two subslices, so these
       * can be stored with a single bit. Even though there are only 10 EUs
       * per subslice, this is stored in 4 bits, so there's an effective
       * maximum value of 16 EUs. Similarly, although there are only 7
       * threads per EU, this is stored in a 3 bit number, giving an
       * effective maximum value of 8 threads per EU.
       *
       * This means that we need to use 16 * 8 instead of 10 * 7 for the
       * number of threads per subslice.
       */
      scratch_ids_per_subslice = 16 * 8;
   } else if (devinfo->platform == INTEL_PLATFORM_CHV) {
      /* Cherryview devices have either 6 or 8 EUs per subslice, and each
       * EU has 7 threads. The 6 EU devices appear to calculate thread IDs
       * as if it had 8 EUs.
       */
      scratch_ids_per_subslice = 8 * 7;
   } else {
      scratch_ids_per_subslice = devinfo->max_cs_threads;
   }

   unsigned max_thread_ids = scratch_ids_per_subslice * subslices;

   if (devinfo->verx10 >= 125) {
      /* On GFX version 12.5, scratch access changed to a surface-based model.
       * Instead of each shader type having its own layout based on IDs passed
       * from the relevant fixed-function unit, all scratch access is based on
       * thread IDs like it always has been for compute.
       */
      for (int i = MESA_SHADER_VERTEX; i < MESA_SHADER_STAGES; i++)
         devinfo->max_scratch_ids[i] = max_thread_ids;
   } else {
      unsigned max_scratch_ids[] = {
         [MESA_SHADER_VERTEX]    = devinfo->max_vs_threads,
         [MESA_SHADER_TESS_CTRL] = devinfo->max_tcs_threads,
         [MESA_SHADER_TESS_EVAL] = devinfo->max_tes_threads,
         [MESA_SHADER_GEOMETRY]  = devinfo->max_gs_threads,
         [MESA_SHADER_FRAGMENT]  = devinfo->max_wm_threads,
         [MESA_SHADER_COMPUTE]   = max_thread_ids,
      };
      STATIC_ASSERT(sizeof(devinfo->max_scratch_ids) == sizeof(max_scratch_ids));
      memcpy(devinfo->max_scratch_ids, max_scratch_ids,
             sizeof(devinfo->max_scratch_ids));
   }
}

static unsigned
intel_device_info_calc_engine_prefetch(const struct intel_device_info *devinfo,
                                       enum intel_engine_class engine_class)
{
   if (devinfo->verx10 >= 200) {
      switch (engine_class) {
      case INTEL_ENGINE_CLASS_RENDER:
         return 4096;
      case INTEL_ENGINE_CLASS_COMPUTE:
         return 1024;
      default:
         return 512;
      }
   }

   if (intel_device_info_is_mtl(devinfo)) {
      switch (engine_class) {
      case INTEL_ENGINE_CLASS_RENDER:
         return 2048;
      case INTEL_ENGINE_CLASS_COMPUTE:
         return 1024;
      default:
         return 512;
      }
   }

   /* DG2 */
   if (devinfo->verx10 == 125)
      return 1024;

   /* Older than DG2/MTL */
   return 512;
}

bool
intel_get_device_info_from_fd(int fd, struct intel_device_info *devinfo)
{
   /* Get PCI info.
    *
    * Some callers may already have a valid drm device which holds values of
    * PCI fields queried here prior to calling this function. But making this
    * query optional leads to a more cumbersome implementation. These callers
    * still need to initialize the fields somewhere out of this function and
    * rely on an ioctl to get PCI device id for the next step when skipping
    * this drm query.
    */
   drmDevicePtr drmdev = NULL;
   if (drmGetDevice2(fd, DRM_DEVICE_GET_PCI_REVISION, &drmdev)) {
      mesa_loge("Failed to query drm device.");
      return false;
   }
   if (!intel_device_info_init_common(
          drmdev->deviceinfo.pci->device_id, devinfo)) {
      drmFreeDevice(&drmdev);
      return false;
   }
   devinfo->pci_domain = drmdev->businfo.pci->domain;
   devinfo->pci_bus = drmdev->businfo.pci->bus;
   devinfo->pci_dev = drmdev->businfo.pci->dev;
   devinfo->pci_func = drmdev->businfo.pci->func;
   devinfo->pci_device_id = drmdev->deviceinfo.pci->device_id;
   devinfo->pci_revision_id = drmdev->deviceinfo.pci->revision_id;
   drmFreeDevice(&drmdev);
   devinfo->no_hw = debug_get_bool_option("INTEL_NO_HW", false);

   if (devinfo->ver == 10) {
      mesa_loge("Gfx10 support is redacted.");
      return false;
   }

   devinfo->kmd_type = intel_get_kmd_type(fd);
   if (devinfo->kmd_type == INTEL_KMD_TYPE_INVALID) {
      mesa_loge("Unknown kernel mode driver");
      return false;
   }

   /* remaining initializion queries the kernel for device info */
   if (devinfo->no_hw) {
      /* Provide some sensible values for NO_HW. */
      devinfo->gtt_size =
         devinfo->ver >= 8 ? (1ull << 48) : 2ull * 1024 * 1024 * 1024;
      intel_device_info_compute_system_memory(devinfo, false);
      return true;
   }

   bool ret;
   switch (devinfo->kmd_type) {
   case INTEL_KMD_TYPE_I915:
      ret = intel_device_info_i915_get_info_from_fd(fd, devinfo);
      break;
   case INTEL_KMD_TYPE_XE:
      ret = intel_device_info_xe_get_info_from_fd(fd, devinfo);
      break;
   default:
      ret = false;
      unreachable("Missing");
   }
   if (!ret) {
      mesa_logw("Could not get intel_device_info.");
      return false;
   }

   /* region info is required for lmem support */
   if (devinfo->has_local_mem && !devinfo->mem.use_class_instance) {
      mesa_logw("Could not query local memory size.");
      return false;
   }

   /* Gfx7 and older do not support EU/Subslice info */
   assert(devinfo->subslice_total >= 1 || devinfo->ver <= 7);
   devinfo->subslice_total = MAX2(devinfo->subslice_total, 1);

   init_max_scratch_ids(devinfo);

   for (enum intel_engine_class engine = INTEL_ENGINE_CLASS_RENDER;
        engine < ARRAY_SIZE(devinfo->engine_class_prefetch); engine++)
      devinfo->engine_class_prefetch[engine] =
            intel_device_info_calc_engine_prefetch(devinfo, engine);

   intel_device_info_init_was(devinfo);
   intel_device_info_apply_workarounds(devinfo);

   return true;
}

bool intel_device_info_update_memory_info(struct intel_device_info *devinfo, int fd)
{
   bool ret;

   switch (devinfo->kmd_type) {
   case INTEL_KMD_TYPE_I915:
      ret = intel_device_info_i915_query_regions(devinfo, fd, true);
      break;
   case INTEL_KMD_TYPE_XE:
      ret = intel_device_info_xe_query_regions(fd, devinfo, true);
      break;
   default:
      ret = false;
   }
   return ret || intel_device_info_compute_system_memory(devinfo, true);
}

void
intel_device_info_update_after_hwconfig(struct intel_device_info *devinfo)
{
   /* After applying hwconfig values, some items need to be recalculated. */
   devinfo->max_cs_threads =
      devinfo->max_eus_per_subslice * devinfo->num_thread_per_eu;

   intel_device_info_update_cs_workgroup_threads(devinfo);
}

enum intel_wa_steppings
intel_device_info_wa_stepping(struct intel_device_info *devinfo)
{
   if (intel_device_info_is_mtl(devinfo)) {
      if (devinfo->revision < 4)
         return INTEL_STEPPING_A0;
      return INTEL_STEPPING_B0;
   } else if (devinfo->platform == INTEL_PLATFORM_TGL) {
      switch (devinfo->revision) {
      case 0:
         return INTEL_STEPPING_A0;
      case 1:
         return INTEL_STEPPING_B0;
      case 3:
         return INTEL_STEPPING_C0;
      default:
         return INTEL_STEPPING_RELEASE;
      }
   }

   /* all other platforms support only released steppings */
   return INTEL_STEPPING_RELEASE;
}
