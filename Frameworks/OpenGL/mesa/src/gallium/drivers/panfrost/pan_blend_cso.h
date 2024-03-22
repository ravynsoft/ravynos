/*
 * Copyright (C) 2019 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors (Collabora):
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 *
 */

#ifndef __PAN_BLEND_CSO_H
#define __PAN_BLEND_CSO_H

#include "util/hash_table.h"
#include "nir.h"
#include "pan_blend.h"
#include "pan_job.h"

struct panfrost_bo;

struct pan_blend_info {
   unsigned constant_mask : 4;
   bool fixed_function    : 1;
   bool enabled           : 1;
   bool load_dest         : 1;
   bool opaque            : 1;
   bool alpha_zero_nop    : 1;
   bool alpha_one_store   : 1;
};

struct panfrost_blend_state {
   struct pipe_blend_state base;
   struct pan_blend_state pan;
   struct pan_blend_info info[PIPE_MAX_COLOR_BUFS];
   uint32_t equation[PIPE_MAX_COLOR_BUFS];

   /* info.load presented as a bitfield for draw call hot paths */
   unsigned load_dest_mask : PIPE_MAX_COLOR_BUFS;

   /* info.enabled presented as a bitfield for draw call hot paths */
   unsigned enabled_mask : PIPE_MAX_COLOR_BUFS;
};

mali_ptr panfrost_get_blend(struct panfrost_batch *batch, unsigned rt,
                            struct panfrost_bo **bo, unsigned *shader_offset);

#endif
