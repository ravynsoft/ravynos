/*
 * Copyright 2020 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ZINK_FORMAT_H
#define ZINK_FORMAT_H

#include "util/format/u_formats.h"
#include "util/format/u_format.h"

#include <stdbool.h>
#include <vulkan/vulkan_core.h>

union pipe_color_union;

enum pipe_format
zink_decompose_vertex_format(enum pipe_format format);

bool
zink_format_is_voidable_rgba_variant(enum pipe_format format);
bool
zink_format_is_red_alpha(enum pipe_format format);
bool
zink_format_is_emulated_alpha(enum pipe_format format);
enum pipe_format
zink_format_get_emulated_alpha(enum pipe_format format);
void
zink_format_clamp_channel_color(const struct util_format_description *desc, union pipe_color_union *dst, const union pipe_color_union *src, unsigned i);
void
zink_format_clamp_channel_srgb(const struct util_format_description *desc, union pipe_color_union *dst, const union pipe_color_union *src, unsigned i);

static inline bool
zink_format_needs_mutable(enum pipe_format a, enum pipe_format b)
{
   if (a == b)
      return false;
   if (util_format_is_srgb(a))
      return util_format_linear(a) != b;
   if (util_format_is_srgb(b))
      return util_format_linear(b) != a;
   return true;
}
#endif
