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
 */

#ifndef __PAN_ENCODER_H
#define __PAN_ENCODER_H

#include "util/macros.h"

#include <stdbool.h>
#include "genxml/gen_macros.h"
#include "util/format/u_format.h"
#include "pan_bo.h"
#include "pan_device.h"

/* Tiler structure size computation */

unsigned panfrost_tiler_header_size(unsigned width, unsigned height,
                                    unsigned mask, bool hierarchy);

unsigned panfrost_tiler_full_size(unsigned width, unsigned height,
                                  unsigned mask, bool hierarchy);

unsigned panfrost_choose_hierarchy_mask(unsigned width, unsigned height,
                                        unsigned vertex_count, bool hierarchy);

#if defined(PAN_ARCH) && PAN_ARCH <= 5
static inline unsigned
panfrost_tiler_get_polygon_list_size(const struct panfrost_device *dev,
                                     unsigned fb_width, unsigned fb_height,
                                     unsigned vertex_count)
{
   if (!vertex_count)
      return MALI_MIDGARD_TILER_MINIMUM_HEADER_SIZE + 4;

   bool hierarchy = !dev->model->quirks.no_hierarchical_tiling;
   unsigned hierarchy_mask = panfrost_choose_hierarchy_mask(
      fb_width, fb_height, vertex_count, hierarchy);

   return panfrost_tiler_full_size(fb_width, fb_height, hierarchy_mask,
                                   hierarchy) +
          panfrost_tiler_header_size(fb_width, fb_height, hierarchy_mask,
                                     hierarchy);
}
#endif

/* Stack sizes */

unsigned panfrost_get_stack_shift(unsigned stack_size);

unsigned panfrost_get_total_stack_size(unsigned thread_size,
                                       unsigned threads_per_core,
                                       unsigned core_id_range);

/* Attributes / instancing */

unsigned panfrost_padded_vertex_count(unsigned vertex_count);

unsigned panfrost_compute_magic_divisor(unsigned hw_divisor, unsigned *o_shift,
                                        unsigned *extra_flags);

#ifdef PAN_ARCH
/* Records for gl_VertexID and gl_InstanceID use special encodings on Midgard */

#if PAN_ARCH <= 5
static inline void
panfrost_vertex_id(unsigned padded_count,
                   struct mali_attribute_buffer_packed *attr, bool instanced)
{
   pan_pack(attr, ATTRIBUTE_VERTEX_ID, cfg) {
      if (instanced) {
         cfg.divisor_r = __builtin_ctz(padded_count);
         cfg.divisor_p = padded_count >> (cfg.divisor_r + 1);
      } else {
         /* Large values so the modulo is a no-op */
         cfg.divisor_r = 0x1F;
         cfg.divisor_p = 0x4;
      }
   }
}

static inline void
panfrost_instance_id(unsigned padded_count,
                     struct mali_attribute_buffer_packed *attr, bool instanced)
{
   pan_pack(attr, ATTRIBUTE_INSTANCE_ID, cfg) {
      if (!instanced || padded_count <= 1) {
         /* Divide by large number to force to 0 */
         cfg.divisor_p = ((1u << 31) - 1);
         cfg.divisor_r = 0x1F;
         cfg.divisor_e = 0x1;
      } else if (util_is_power_of_two_or_zero(padded_count)) {
         /* Can't underflow since padded_count >= 2 */
         cfg.divisor_r = __builtin_ctz(padded_count) - 1;
      } else {
         cfg.divisor_p = panfrost_compute_magic_divisor(
            padded_count, &cfg.divisor_r, &cfg.divisor_e);
      }
   }
}
#endif /* PAN_ARCH <= 5 */

/* Sampler comparison functions are flipped in OpenGL from the hardware, so we
 * need to be able to flip accordingly */

static inline enum mali_func
panfrost_flip_compare_func(enum mali_func f)
{
   switch (f) {
   case MALI_FUNC_LESS:
      return MALI_FUNC_GREATER;
   case MALI_FUNC_GREATER:
      return MALI_FUNC_LESS;
   case MALI_FUNC_LEQUAL:
      return MALI_FUNC_GEQUAL;
   case MALI_FUNC_GEQUAL:
      return MALI_FUNC_LEQUAL;
   default:
      return f;
   }
}

#if PAN_ARCH <= 7
/* Compute shaders are invoked with a gl_NumWorkGroups X/Y/Z triplet. Vertex
 * shaders are invoked as (1, vertex_count, instance_count). Compute shaders
 * also have a gl_WorkGroupSize X/Y/Z triplet. These 6 values are packed
 * together in a dynamic bitfield, packed by this routine. */

static inline void
panfrost_pack_work_groups_compute(struct mali_invocation_packed *out,
                                  unsigned num_x, unsigned num_y,
                                  unsigned num_z, unsigned size_x,
                                  unsigned size_y, unsigned size_z,
                                  bool quirk_graphics, bool indirect_dispatch)
{
   /* The values needing packing, in order, and the corresponding shifts.
    * Indicies into shift are off-by-one to make the logic easier */

   unsigned values[6] = {size_x, size_y, size_z, num_x, num_y, num_z};
   unsigned shifts[7] = {0};
   uint32_t packed = 0;

   for (unsigned i = 0; i < 6; ++i) {
      /* Must be positive, otherwise we underflow */
      assert(values[i] >= 1);

      /* OR it in, shifting as required */
      packed |= ((values[i] - 1) << shifts[i]);

      /* How many bits did we use? */
      unsigned bit_count = util_logbase2_ceil(values[i]);

      /* Set the next shift accordingly */
      shifts[i + 1] = shifts[i] + bit_count;
   }

   pan_pack(out, INVOCATION, cfg) {
      cfg.invocations = packed;
      cfg.size_y_shift = shifts[1];
      cfg.size_z_shift = shifts[2];
      cfg.workgroups_x_shift = shifts[3];

      if (!indirect_dispatch) {
         /* Leave zero for the dispatch shader */
         cfg.workgroups_y_shift = shifts[4];
         cfg.workgroups_z_shift = shifts[5];
      }

      /* Quirk: for non-instanced graphics, the blob sets
       * workgroups_z_shift = 32. This doesn't appear to matter to
       * the hardware, but it's good to be bit-identical. */

      if (quirk_graphics && (num_z <= 1))
         cfg.workgroups_z_shift = 32;

      /* For graphics, set to the minimum efficient value. For
       * compute, must equal the workgroup X shift for barriers to
       * function correctly */

      cfg.thread_group_split =
         quirk_graphics ? MALI_SPLIT_MIN_EFFICIENT : cfg.workgroups_x_shift;
   }
}
#endif

#if PAN_ARCH >= 5
/* Format conversion */
static inline enum mali_z_internal_format
panfrost_get_z_internal_format(enum pipe_format fmt)
{
   switch (fmt) {
   case PIPE_FORMAT_Z16_UNORM:
   case PIPE_FORMAT_Z16_UNORM_S8_UINT:
      return MALI_Z_INTERNAL_FORMAT_D16;
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
   case PIPE_FORMAT_Z24X8_UNORM:
      return MALI_Z_INTERNAL_FORMAT_D24;
   case PIPE_FORMAT_Z32_FLOAT:
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      return MALI_Z_INTERNAL_FORMAT_D32;
   default:
      unreachable("Unsupported depth/stencil format.");
   }
}
#endif

#endif /* PAN_ARCH */

#if PAN_ARCH >= 9
static inline void
panfrost_make_resource_table(struct panfrost_ptr base, unsigned index,
                             mali_ptr address, unsigned resource_count)
{
   if (resource_count == 0)
      return;

   pan_pack(base.cpu + index * pan_size(RESOURCE), RESOURCE, cfg) {
      cfg.address = address;
      cfg.size = resource_count * pan_size(BUFFER);
   }
}
#endif

#endif
