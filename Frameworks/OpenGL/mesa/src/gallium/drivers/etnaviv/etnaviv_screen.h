/*
 * Copyright (c) 2012-2015 Etnaviv Project
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
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#ifndef H_ETNAVIV_SCREEN
#define H_ETNAVIV_SCREEN

#include "etnaviv_internal.h"
#include "etnaviv_perfmon.h"

#include "util/u_thread.h"
#include "pipe/p_screen.h"
#include "renderonly/renderonly.h"
#include "util/set.h"
#include "util/slab.h"
#include "util/u_dynarray.h"
#include "util/u_helpers.h"
#include "util/u_queue.h"
#include "compiler/nir/nir.h"

struct etna_bo;

/* Enum with indices for each of the feature words */
enum viv_features_word {
   viv_chipFeatures = 0,
   viv_chipMinorFeatures0 = 1,
   viv_chipMinorFeatures1 = 2,
   viv_chipMinorFeatures2 = 3,
   viv_chipMinorFeatures3 = 4,
   viv_chipMinorFeatures4 = 5,
   viv_chipMinorFeatures5 = 6,
   viv_chipMinorFeatures6 = 7,
   viv_chipMinorFeatures7 = 8,
   viv_chipMinorFeatures8 = 9,
   viv_chipMinorFeatures9 = 10,
   viv_chipMinorFeatures10 = 11,
   viv_chipMinorFeatures11 = 12,
   viv_chipMinorFeatures12 = 13,
   VIV_FEATURES_WORD_COUNT /* Must be last */
};

/** Convenience macro to probe features from state.xml.h:
 * VIV_FEATURE(chipFeatures, FAST_CLEAR)
 * VIV_FEATURE(chipMinorFeatures1, AUTO_DISABLE)
 */
#define VIV_FEATURE(screen, word, feature) \
   ((screen->features[viv_ ## word] & (word ## _ ## feature)) != 0)

struct etna_screen {
   struct pipe_screen base;

   struct etna_device *dev;
   struct etna_gpu *gpu;
   struct etna_pipe *pipe;
   struct etna_perfmon *perfmon;
   struct renderonly *ro;

   struct util_dynarray supported_pm_queries;
   struct slab_parent_pool transfer_pool;

   uint32_t model;
   uint32_t revision;
   uint32_t features[VIV_FEATURES_WORD_COUNT];

   struct etna_specs specs;

   uint32_t drm_version;

   struct etna_compiler *compiler;
   struct util_queue shader_compiler_queue;

   /* dummy render target for GPUs that can't fully disable the color pipe */
   struct etna_reloc dummy_rt_reloc;

   /* dummy texture descriptor */
   struct etna_reloc dummy_desc_reloc;
};

static inline struct etna_screen *
etna_screen(struct pipe_screen *pscreen)
{
   return (struct etna_screen *)pscreen;
}

struct etna_bo *
etna_screen_bo_from_handle(struct pipe_screen *pscreen,
                           struct winsys_handle *whandle);

struct pipe_screen *
etna_screen_create(struct etna_device *dev, struct etna_gpu *gpu,
                   struct renderonly *ro);

static inline size_t
etna_screen_get_tile_size(struct etna_screen *screen, uint8_t ts_mode,
                          bool is_msaa)
{
   if (!VIV_FEATURE(screen, chipMinorFeatures6, CACHE128B256BPERLINE)) {
      if (VIV_FEATURE(screen, chipMinorFeatures4, SMALL_MSAA) && is_msaa)
         return 256;
      return 64;
   }

   if (ts_mode == TS_MODE_256B)
      return 256;
   else
      return 128;
}

#endif
