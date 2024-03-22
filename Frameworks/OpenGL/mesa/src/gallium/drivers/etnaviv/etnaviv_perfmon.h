/*
 * Copyright (c) 2017 Etnaviv Project
 * Copyright (C) 2017 Zodiac Inflight Innovations
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
 *    Rob Clark <robclark@freedesktop.org>
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#ifndef H_ETNAVIV_PERFMON
#define H_ETNAVIV_PERFMON

#include "etnaviv_query.h"
#include <pipe/p_screen.h>

struct etna_screen;

#define ETNA_QUERY_HI_GROUP_ID                           1
#define ETNA_QUERY_PE_GROUP_ID                           2
#define ETNA_QUERY_SH_GROUP_ID                           3
#define ETNA_QUERY_PA_GROUP_ID                           4
#define ETNA_QUERY_SE_GROUP_ID                           5
#define ETNA_QUERY_RA_GROUP_ID                           6
#define ETNA_QUERY_TX_GROUP_ID                           7
#define ETNA_QUERY_MC_GROUP_ID                           8

#define ETNA_QUERY_HI_TOTAL_READ_BYTES8                  (ETNA_PM_QUERY_BASE + 0)
#define ETNA_QUERY_HI_TOTAL_WRITE_BYTES8                 (ETNA_PM_QUERY_BASE + 1)
#define ETNA_QUERY_HI_TOTAL_CYCLES                       (ETNA_PM_QUERY_BASE + 2)
#define ETNA_QUERY_HI_IDLE_CYCLES                        (ETNA_PM_QUERY_BASE + 3)
#define ETNA_QUERY_HI_AXI_CYCLES_READ_REQUEST_STALLED    (ETNA_PM_QUERY_BASE + 4)
#define ETNA_QUERY_HI_AXI_CYCLES_WRITE_REQUEST_STALLED   (ETNA_PM_QUERY_BASE + 5)
#define ETNA_QUERY_HI_AXI_CYCLES_WRITE_DATA_STALLED      (ETNA_PM_QUERY_BASE + 6)

#define ETNA_QUERY_PE_PIXEL_COUNT_KILLED_BY_COLOR_PIPE   (ETNA_PM_QUERY_BASE + 7)
#define ETNA_QUERY_PE_PIXEL_COUNT_KILLED_BY_DEPTH_PIPE   (ETNA_PM_QUERY_BASE + 8)
#define ETNA_QUERY_PE_PIXEL_COUNT_DRAWN_BY_COLOR_PIPE    (ETNA_PM_QUERY_BASE + 9)
#define ETNA_QUERY_PE_PIXEL_COUNT_DRAWN_BY_DEPTH_PIPE    (ETNA_PM_QUERY_BASE + 10)
#define ETNA_QUERY_PE_PIXELS_RENDERED_2D                 (ETNA_PM_QUERY_BASE + 11)

#define ETNA_QUERY_SH_SHADER_CYCLES                      (ETNA_PM_QUERY_BASE + 12)
#define ETNA_QUERY_SH_PS_INST_COUNTER                    (ETNA_PM_QUERY_BASE + 13)
#define ETNA_QUERY_SH_RENDERED_PIXEL_COUNTER             (ETNA_PM_QUERY_BASE + 14)
#define ETNA_QUERY_SH_VS_INST_COUNTER                    (ETNA_PM_QUERY_BASE + 15)
#define ETNA_QUERY_SH_RENDERED_VERTICE_COUNTER           (ETNA_PM_QUERY_BASE + 16)
#define ETNA_QUERY_SH_VTX_BRANCH_INST_COUNTER            (ETNA_PM_QUERY_BASE + 17)
#define ETNA_QUERY_SH_VTX_TEXLD_INST_COUNTER             (ETNA_PM_QUERY_BASE + 18)
#define ETNA_QUERY_SH_PXL_BRANCH_INST_COUNTER            (ETNA_PM_QUERY_BASE + 19)
#define ETNA_QUERY_SH_PXL_TEXLD_INST_COUNTER             (ETNA_PM_QUERY_BASE + 20)

#define ETNA_QUERY_PA_INPUT_VTX_COUNTER                  (ETNA_PM_QUERY_BASE + 21)
#define ETNA_QUERY_PA_INPUT_PRIM_COUNTER                 (ETNA_PM_QUERY_BASE + 22)
#define ETNA_QUERY_PA_OUTPUT_PRIM_COUNTER                (ETNA_PM_QUERY_BASE + 23)
#define ETNA_QUERY_PA_DEPTH_CLIPPED_COUNTER              (ETNA_PM_QUERY_BASE + 24)
#define ETNA_QUERY_PA_TRIVIAL_REJECTED_COUNTER           (ETNA_PM_QUERY_BASE + 25)
#define ETNA_QUERY_PA_CULLED_COUNTER                     (ETNA_PM_QUERY_BASE + 26)

#define ETNA_QUERY_SE_CULLED_TRIANGLE_COUNT              (ETNA_PM_QUERY_BASE + 27)
#define ETNA_QUERY_SE_CULLED_LINES_COUNT                 (ETNA_PM_QUERY_BASE + 28)

#define ETNA_QUERY_RA_VALID_PIXEL_COUNT                  (ETNA_PM_QUERY_BASE + 29)
#define ETNA_QUERY_RA_TOTAL_QUAD_COUNT                   (ETNA_PM_QUERY_BASE + 30)
#define ETNA_QUERY_RA_VALID_QUAD_COUNT_AFTER_EARLY_Z     (ETNA_PM_QUERY_BASE + 31)
#define ETNA_QUERY_RA_TOTAL_PRIMITIVE_COUNT              (ETNA_PM_QUERY_BASE + 32)
#define ETNA_QUERY_RA_PIPE_CACHE_MISS_COUNTER            (ETNA_PM_QUERY_BASE + 33)
#define ETNA_QUERY_RA_PREFETCH_CACHE_MISS_COUNTER        (ETNA_PM_QUERY_BASE + 34)
#define ETNA_QUERY_RA_CULLED_QUAD_COUNT                  (ETNA_PM_QUERY_BASE + 35)

#define ETNA_QUERY_TX_TOTAL_BILINEAR_REQUESTS            (ETNA_PM_QUERY_BASE + 36)
#define ETNA_QUERY_TX_TOTAL_TRILINEAR_REQUESTS           (ETNA_PM_QUERY_BASE + 37)
#define ETNA_QUERY_TX_TOTAL_DISCARDED_TEXTURE_REQUESTS   (ETNA_PM_QUERY_BASE + 38)
#define ETNA_QUERY_TX_TOTAL_TEXTURE_REQUESTS             (ETNA_PM_QUERY_BASE + 39)
#define ETNA_QUERY_TX_MEM_READ_COUNT                     (ETNA_PM_QUERY_BASE + 40)
#define ETNA_QUERY_TX_MEM_READ_IN_8B_COUNT               (ETNA_PM_QUERY_BASE + 41)
#define ETNA_QUERY_TX_CACHE_MISS_COUNT                   (ETNA_PM_QUERY_BASE + 42)
#define ETNA_QUERY_TX_CACHE_HIT_TEXEL_COUNT              (ETNA_PM_QUERY_BASE + 43)
#define ETNA_QUERY_TX_CACHE_MISS_TEXEL_COUNT             (ETNA_PM_QUERY_BASE + 44)

#define ETNA_QUERY_MC_TOTAL_READ_REQ_8B_FROM_PIPELINE    (ETNA_PM_QUERY_BASE + 45)
#define ETNA_QUERY_MC_TOTAL_READ_REQ_8B_FROM_IP          (ETNA_PM_QUERY_BASE + 46)
#define ETNA_QUERY_MC_TOTAL_WRITE_REQ_8B_FROM_PIPELINE   (ETNA_PM_QUERY_BASE + 47)

struct etna_perfmon_source
{
   const char *domain;
   const char *signal;
};

struct etna_perfmon_config
{
   const char *name;
   unsigned type;
   unsigned group_id;
   const struct etna_perfmon_source *source;
   bool multiply_with_8;
};

struct etna_perfmon_signal *
etna_pm_query_signal(struct etna_perfmon *perfmon,
                     const struct etna_perfmon_source *source);

static inline bool
etna_pm_cfg_supported(struct etna_perfmon *perfmon,
                      const struct etna_perfmon_config *cfg)
{
   struct etna_perfmon_signal *signal = etna_pm_query_signal(perfmon, cfg->source);

   return !!signal;
}

void
etna_pm_query_setup(struct etna_screen *screen);

const struct etna_perfmon_config *
etna_pm_query_config(unsigned type);

int
etna_pm_get_driver_query_info(struct pipe_screen *pscreen, unsigned index,
                              struct pipe_driver_query_info *info);

int
etna_pm_get_driver_query_group_info(struct pipe_screen *pscreen,
                                    unsigned index,
                                    struct pipe_driver_query_group_info *info);

#endif
