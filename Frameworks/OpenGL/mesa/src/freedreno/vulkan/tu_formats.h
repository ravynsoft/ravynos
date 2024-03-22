/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#ifndef TU_FORMATS_H
#define TU_FORMATS_H

#include "tu_common.h"

struct tu_native_format
{
   enum a6xx_format fmt : 8;
   enum a3xx_color_swap swap : 8;
};

enum pipe_format tu_vk_format_to_pipe_format(VkFormat vk_format);

static inline bool
tu_pipe_format_is_float16(enum pipe_format format)
{
   const struct util_format_description *desc =
      util_format_description(format);
   const int c = util_format_get_first_non_void_channel(format);
   if (c < 0)
      return false;

   return desc->channel[c].type == UTIL_FORMAT_TYPE_FLOAT && desc->channel[c].size == 16;
}

struct tu_native_format tu6_format_vtx(enum pipe_format format);
struct tu_native_format tu6_format_color(enum pipe_format format, enum a6xx_tile_mode tile_mode);
struct tu_native_format tu6_format_texture(enum pipe_format format, enum a6xx_tile_mode tile_mode);

bool tu6_mutable_format_list_ubwc_compatible(const VkImageFormatListCreateInfo *fmt_list);

#endif /* TU_FORMATS_H */
