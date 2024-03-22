/*
 * Copyright © 2019 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef GFX10_FORMAT_TABLE_H
#define GFX10_FORMAT_TABLE_H

#include "util/format/u_formats.h"
#include "ac_gpu_info.h"

#include <stdbool.h>

struct gfx10_format {
   unsigned img_format : 9;

   /* Various formats are only supported with workarounds for vertex fetch,
    * and some 32_32_32 formats are supported natively, but only for buffers
    * (possibly with some image support, actually, but no filtering). */
   bool buffers_only : 1;
};

extern const struct gfx10_format gfx10_format_table[PIPE_FORMAT_COUNT];
extern const struct gfx10_format gfx11_format_table[PIPE_FORMAT_COUNT];

static inline
const struct gfx10_format* ac_get_gfx10_format_table(const struct radeon_info *info)
{
   if (info->gfx_level >= GFX11)
      return gfx11_format_table;
   else
      return gfx10_format_table;
}

#endif /* GFX10_FORMAT_TABLE_H */
