/*
 * © Copyright2018-2019 Alyssa Rosenzweig
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
 */

#ifndef PAN_RESOURCE_H
#define PAN_RESOURCE_H

#include "drm-uapi/drm.h"
#include "util/u_range.h"
#include "pan_minmax_cache.h"
#include "pan_screen.h"
#include "pan_texture.h"

#define LAYOUT_CONVERT_THRESHOLD 8
#define PAN_MAX_BATCHES          32

#define PAN_BIND_SHARED_MASK                                                   \
   (PIPE_BIND_DISPLAY_TARGET | PIPE_BIND_SCANOUT | PIPE_BIND_SHARED)

struct panfrost_resource {
   struct pipe_resource base;
   struct {
      struct pipe_scissor_state extent;
      struct {
         bool enable;
         unsigned stride;
         unsigned size;
         BITSET_WORD *data;
      } tile_map;
   } damage;

   struct renderonly_scanout *scanout;

   struct panfrost_resource *separate_stencil;

   struct util_range valid_buffer_range;

   /* Description of the resource layout */
   struct pan_image image;

   struct {
      /* Is the checksum for this image valid? Implicitly refers to
       * the first slice; we only checksum non-mipmapped 2D images */
      bool crc;

      /* Has anything been written to this slice? */
      BITSET_DECLARE(data, MAX_MIP_LEVELS);
   } valid;

   /* Whether the modifier can be changed */
   bool modifier_constant;

   /* Used to decide when to convert to another modifier */
   uint16_t modifier_updates;

   /* Do all pixels have the same stencil value? */
   bool constant_stencil;

   /* The stencil value if constant_stencil is set */
   uint8_t stencil_value;

   /* Cached min/max values for index buffers */
   struct panfrost_minmax_cache *index_cache;
};

static inline struct panfrost_resource *
pan_resource(struct pipe_resource *p)
{
   return (struct panfrost_resource *)p;
}

struct panfrost_transfer {
   struct pipe_transfer base;
   void *map;
   struct {
      struct pipe_resource *rsrc;
      struct pipe_box box;
   } staging;
};

static inline struct panfrost_transfer *
pan_transfer(struct pipe_transfer *p)
{
   return (struct panfrost_transfer *)p;
}

void panfrost_resource_screen_init(struct pipe_screen *screen);

void panfrost_resource_screen_destroy(struct pipe_screen *screen);

void panfrost_resource_context_init(struct pipe_context *pctx);

/* Blitting */

enum panfrost_blitter_op /* bitmask */
{
   PAN_SAVE_TEXTURES = 1,
   PAN_SAVE_FRAMEBUFFER = 2,
   PAN_SAVE_FRAGMENT_STATE = 4,
   PAN_SAVE_FRAGMENT_CONSTANT = 8,
   PAN_DISABLE_RENDER_COND = 16,
};

enum {
   PAN_RENDER_BLIT =
      PAN_SAVE_TEXTURES | PAN_SAVE_FRAMEBUFFER | PAN_SAVE_FRAGMENT_STATE,
   PAN_RENDER_BLIT_COND = PAN_SAVE_TEXTURES | PAN_SAVE_FRAMEBUFFER |
                          PAN_SAVE_FRAGMENT_STATE | PAN_DISABLE_RENDER_COND,
   PAN_RENDER_BASE = PAN_SAVE_FRAMEBUFFER | PAN_SAVE_FRAGMENT_STATE,
   PAN_RENDER_COND =
      PAN_SAVE_FRAMEBUFFER | PAN_SAVE_FRAGMENT_STATE | PAN_DISABLE_RENDER_COND,
   PAN_RENDER_CLEAR = PAN_SAVE_FRAGMENT_STATE | PAN_SAVE_FRAGMENT_CONSTANT,
};

void panfrost_blitter_save(struct panfrost_context *ctx,
                           const enum panfrost_blitter_op blitter_op);

void panfrost_blit(struct pipe_context *pipe,
                   const struct pipe_blit_info *info);

void panfrost_resource_set_damage_region(struct pipe_screen *screen,
                                         struct pipe_resource *res,
                                         unsigned int nrects,
                                         const struct pipe_box *rects);

void panfrost_set_image_view_planes(struct pan_image_view *iview,
                                    struct pipe_resource *texture);

static inline enum mali_texture_dimension
panfrost_translate_texture_dimension(enum pipe_texture_target t)
{
   switch (t) {
   case PIPE_BUFFER:
   case PIPE_TEXTURE_1D:
   case PIPE_TEXTURE_1D_ARRAY:
      return MALI_TEXTURE_DIMENSION_1D;

   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_RECT:
      return MALI_TEXTURE_DIMENSION_2D;

   case PIPE_TEXTURE_3D:
      return MALI_TEXTURE_DIMENSION_3D;

   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_CUBE_ARRAY:
      return MALI_TEXTURE_DIMENSION_CUBE;

   default:
      unreachable("Unknown target");
   }
}

struct pipe_resource *
panfrost_resource_create_with_modifier(struct pipe_screen *screen,
                                       const struct pipe_resource *template,
                                       uint64_t modifier);

struct panfrost_bo *panfrost_get_afbc_superblock_sizes(
   struct panfrost_context *ctx, struct panfrost_resource *rsrc,
   unsigned first_level, unsigned last_level, unsigned *out_offsets);

bool panfrost_should_pack_afbc(struct panfrost_device *dev,
                               const struct panfrost_resource *rsrc);

void panfrost_pack_afbc(struct panfrost_context *ctx,
                        struct panfrost_resource *prsrc);

void pan_resource_modifier_convert(struct panfrost_context *ctx,
                                   struct panfrost_resource *rsrc,
                                   uint64_t modifier, bool copy_resource,
                                   const char *reason);

void pan_legalize_afbc_format(struct panfrost_context *ctx,
                              struct panfrost_resource *rsrc,
                              enum pipe_format format, bool write,
                              bool discard);
void pan_dump_resource(struct panfrost_context *ctx,
                       struct panfrost_resource *rsc);

void panfrost_blit_no_afbc_legalization(struct pipe_context *pipe,
                                        const struct pipe_blit_info *info);

#endif /* PAN_RESOURCE_H */
