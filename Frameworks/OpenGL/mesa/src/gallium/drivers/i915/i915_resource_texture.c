/**************************************************************************
 *
 * Copyright 2006 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/
/*
 * Authors:
 *   Keith Whitwell <keithw@vmware.com>
 *   Michel Dänzer <daenzer@vmware.com>
 */

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_rect.h"

#include "i915_context.h"
#include "i915_debug.h"
#include "i915_resource.h"
#include "i915_screen.h"
#include "i915_winsys.h"

#define DEBUG_TEXTURES 0

/*
 * Helper function and arrays
 */

/**
 * Initial offset for Cube map.
 */
static const int initial_offsets[6][2] = {
   [PIPE_TEX_FACE_POS_X] = {0, 0}, [PIPE_TEX_FACE_POS_Y] = {1, 0},
   [PIPE_TEX_FACE_POS_Z] = {1, 1}, [PIPE_TEX_FACE_NEG_X] = {0, 2},
   [PIPE_TEX_FACE_NEG_Y] = {1, 2}, [PIPE_TEX_FACE_NEG_Z] = {1, 3},
};

/**
 * Step offsets for Cube map.
 */
static const int step_offsets[6][2] = {
   [PIPE_TEX_FACE_POS_X] = {0, 2},  [PIPE_TEX_FACE_POS_Y] = {-1, 2},
   [PIPE_TEX_FACE_POS_Z] = {-1, 1}, [PIPE_TEX_FACE_NEG_X] = {0, 2},
   [PIPE_TEX_FACE_NEG_Y] = {-1, 2}, [PIPE_TEX_FACE_NEG_Z] = {-1, 1},
};

/**
 * For compressed level 2
 */
static const int bottom_offsets[6] = {
   [PIPE_TEX_FACE_POS_X] = 16 + 0 * 8, [PIPE_TEX_FACE_POS_Y] = 16 + 1 * 8,
   [PIPE_TEX_FACE_POS_Z] = 16 + 2 * 8, [PIPE_TEX_FACE_NEG_X] = 16 + 3 * 8,
   [PIPE_TEX_FACE_NEG_Y] = 16 + 4 * 8, [PIPE_TEX_FACE_NEG_Z] = 16 + 5 * 8,
};

static inline unsigned
align_nblocksx(enum pipe_format format, unsigned width, unsigned align_to)
{
   return align(util_format_get_nblocksx(format, width), align_to);
}

static inline unsigned
align_nblocksy(enum pipe_format format, unsigned width, unsigned align_to)
{
   return align(util_format_get_nblocksy(format, width), align_to);
}

static inline unsigned
get_pot_stride(enum pipe_format format, unsigned width)
{
   return util_next_power_of_two(util_format_get_stride(format, width));
}

static inline const char *
get_tiling_string(enum i915_winsys_buffer_tile tile)
{
   switch (tile) {
   case I915_TILE_NONE:
      return "none";
   case I915_TILE_X:
      return "x";
   case I915_TILE_Y:
      return "y";
   default:
      assert(false);
      return "?";
   }
}

/*
 * More advanced helper funcs
 */

static void
i915_texture_set_level_info(struct i915_texture *tex, unsigned level,
                            unsigned nr_images)
{
   assert(level < ARRAY_SIZE(tex->nr_images));
   assert(nr_images);
   assert(!tex->image_offset[level]);

   tex->nr_images[level] = nr_images;
   tex->image_offset[level] = MALLOC(nr_images * sizeof(struct offset_pair));
   tex->image_offset[level][0].nblocksx = 0;
   tex->image_offset[level][0].nblocksy = 0;
}

unsigned
i915_texture_offset(const struct i915_texture *tex, unsigned level,
                    unsigned layer)
{
   unsigned x, y;
   x = tex->image_offset[level][layer].nblocksx *
       util_format_get_blocksize(tex->b.format);
   y = tex->image_offset[level][layer].nblocksy;

   return y * tex->stride + x;
}

static void
i915_texture_set_image_offset(struct i915_texture *tex, unsigned level,
                              unsigned img, unsigned nblocksx,
                              unsigned nblocksy)
{
   /* for the first image and level make sure offset is zero */
   assert(!(img == 0 && level == 0) || (nblocksx == 0 && nblocksy == 0));
   assert(img < tex->nr_images[level]);

   tex->image_offset[level][img].nblocksx = nblocksx;
   tex->image_offset[level][img].nblocksy = nblocksy;

#if DEBUG_TEXTURES
   debug_printf("%s: %p level %u, img %u (%u, %u)\n", __func__, tex, level, img,
                x, y);
#endif
}

static enum i915_winsys_buffer_tile
i915_texture_tiling(struct i915_screen *is, struct i915_texture *tex)
{
   if (!is->debug.tiling)
      return I915_TILE_NONE;

   if (tex->b.target == PIPE_TEXTURE_1D)
      return I915_TILE_NONE;

   if (util_format_is_compressed(tex->b.format))
      return I915_TILE_X;

   if (is->debug.use_blitter)
      return I915_TILE_X;
   else
      return I915_TILE_Y;
}

/*
 * Shared layout functions
 */

/**
 * Special case to deal with scanout textures.
 */
static bool
i9x5_scanout_layout(struct i915_texture *tex)
{
   struct pipe_resource *pt = &tex->b;

   if (pt->last_level > 0 || util_format_get_blocksize(pt->format) != 4)
      return false;

   if (pt->width0 >= 240) {
      tex->stride = align(util_format_get_stride(pt->format, pt->width0), 64);
      tex->total_nblocksy = align_nblocksy(pt->format, pt->height0, 8);
      tex->tiling = I915_TILE_X;
      /* special case for cursors */
   } else if (pt->width0 == 64 && pt->height0 == 64) {
      tex->stride = get_pot_stride(pt->format, pt->width0);
      tex->total_nblocksy = align_nblocksy(pt->format, pt->height0, 8);
   } else {
      return false;
   }

   i915_texture_set_level_info(tex, 0, 1);
   i915_texture_set_image_offset(tex, 0, 0, 0, 0);

#if DEBUG_TEXTURES
   debug_printf("%s size: %d,%d,%d offset %d,%d (0x%x)\n", __func__, pt->width0,
                pt->height0, util_format_get_blocksize(pt->format), tex->stride,
                tex->total_nblocksy, tex->stride * tex->total_nblocksy);
#endif

   return true;
}

/**
 * Special case to deal with shared textures.
 */
static bool
i9x5_display_target_layout(struct i915_texture *tex)
{
   struct pipe_resource *pt = &tex->b;

   if (pt->last_level > 0 || util_format_get_blocksize(pt->format) != 4)
      return false;

   /* fallback to normal textures for small textures */
   if (pt->width0 < 240)
      return false;

   i915_texture_set_level_info(tex, 0, 1);
   i915_texture_set_image_offset(tex, 0, 0, 0, 0);

   tex->stride = align(util_format_get_stride(pt->format, pt->width0), 64);
   tex->total_nblocksy = align_nblocksy(pt->format, pt->height0, 8);
   tex->tiling = I915_TILE_X;

#if DEBUG_TEXTURES
   debug_printf("%s size: %d,%d,%d offset %d,%d (0x%x)\n", __func__, pt->width0,
                pt->height0, util_format_get_blocksize(pt->format), tex->stride,
                tex->total_nblocksy, tex->stride * tex->total_nblocksy);
#endif

   return true;
}

/**
 * Helper function for special layouts
 */
static bool
i9x5_special_layout(struct i915_texture *tex)
{
   struct pipe_resource *pt = &tex->b;

   /* Scanouts needs special care */
   if (pt->bind & PIPE_BIND_SCANOUT)
      if (i9x5_scanout_layout(tex))
         return true;

   /* Shared buffers needs to be compatible with X servers
    *
    * XXX: need a better name than shared for this if it is to be part
    * of core gallium, and probably move the flag to resource.flags,
    * rather than bindings.
    */
   if (pt->bind & (PIPE_BIND_SHARED | PIPE_BIND_DISPLAY_TARGET))
      if (i9x5_display_target_layout(tex))
         return true;

   return false;
}

/**
 * Cube layout used on i915 and for non-compressed textures on i945.
 *
 * Hardware layout looks like:
 *
 * +-------+-------+
 * |       |       |
 * |       |       |
 * |       |       |
 * |  +x   |  +y   |
 * |       |       |
 * |       |       |
 * |       |       |
 * |       |       |
 * +---+---+-------+
 * |   |   |       |
 * | +x| +y|       |
 * |   |   |       |
 * |   |   |       |
 * +-+-+---+  +z   |
 * | | |   |       |
 * +-+-+ +z|       |
 *   | |   |       |
 * +-+-+---+-------+
 * |       |       |
 * |       |       |
 * |       |       |
 * |  -x   |  -y   |
 * |       |       |
 * |       |       |
 * |       |       |
 * |       |       |
 * +---+---+-------+
 * |   |   |       |
 * | -x| -y|       |
 * |   |   |       |
 * |   |   |       |
 * +-+-+---+  -z   |
 * | | |   |       |
 * +-+-+ -z|       |
 *   | |   |       |
 *   +-+---+-------+
 *
 */
static void
i9x5_texture_layout_cube(struct i915_texture *tex)
{
   struct pipe_resource *pt = &tex->b;
   unsigned width = util_next_power_of_two(pt->width0);
   const unsigned nblocks = util_format_get_nblocksx(pt->format, width);
   unsigned level;
   unsigned face;

   assert(pt->width0 == pt->height0); /* cubemap images are square */

   /* double pitch for cube layouts */
   tex->stride = align(nblocks * util_format_get_blocksize(pt->format) * 2, 4);
   tex->total_nblocksy = nblocks * 4;

   for (level = 0; level <= pt->last_level; level++)
      i915_texture_set_level_info(tex, level, 6);

   for (face = 0; face < 6; face++) {
      unsigned x = initial_offsets[face][0] * nblocks;
      unsigned y = initial_offsets[face][1] * nblocks;
      unsigned d = nblocks;

      for (level = 0; level <= pt->last_level; level++) {
         i915_texture_set_image_offset(tex, level, face, x, y);
         d >>= 1;
         x += step_offsets[face][0] * d;
         y += step_offsets[face][1] * d;
      }
   }
}

/*
 * i915 layout functions
 */

static void
i915_texture_layout_2d(struct i915_texture *tex)
{
   struct pipe_resource *pt = &tex->b;
   unsigned level;
   unsigned width = util_next_power_of_two(pt->width0);
   unsigned height = util_next_power_of_two(pt->height0);
   unsigned nblocksy = util_format_get_nblocksy(pt->format, width);
   unsigned align_y = 2;

   if (util_format_is_compressed(pt->format))
      align_y = 1;

   tex->stride = align(util_format_get_stride(pt->format, width), 4);
   tex->total_nblocksy = 0;

   for (level = 0; level <= pt->last_level; level++) {
      i915_texture_set_level_info(tex, level, 1);
      i915_texture_set_image_offset(tex, level, 0, 0, tex->total_nblocksy);

      tex->total_nblocksy += nblocksy;

      width = u_minify(width, 1);
      height = u_minify(height, 1);
      nblocksy = align_nblocksy(pt->format, height, align_y);
   }
}

static void
i915_texture_layout_3d(struct i915_texture *tex)
{
   struct pipe_resource *pt = &tex->b;
   unsigned level;

   unsigned width = util_next_power_of_two(pt->width0);
   unsigned height = util_next_power_of_two(pt->height0);
   unsigned depth = util_next_power_of_two(pt->depth0);
   unsigned nblocksy = util_format_get_nblocksy(pt->format, height);
   unsigned stack_nblocksy = 0;

   /* Calculate the size of a single slice.
    */
   tex->stride = align(util_format_get_stride(pt->format, width), 4);

   /* XXX: hardware expects/requires 9 levels at minimum.
    */
   for (level = 0; level <= MAX2(8, pt->last_level); level++) {
      i915_texture_set_level_info(tex, level, depth);

      stack_nblocksy += MAX2(2, nblocksy);

      width = u_minify(width, 1);
      height = u_minify(height, 1);
      nblocksy = util_format_get_nblocksy(pt->format, height);
   }

   /* Fixup depth image_offsets:
    */
   for (level = 0; level <= pt->last_level; level++) {
      unsigned i;
      for (i = 0; i < depth; i++)
         i915_texture_set_image_offset(tex, level, i, 0, i * stack_nblocksy);

      depth = u_minify(depth, 1);
   }

   /* Multiply slice size by texture depth for total size.  It's
    * remarkable how wasteful of memory the i915 texture layouts
    * are.  They are largely fixed in the i945.
    */
   tex->total_nblocksy = stack_nblocksy * util_next_power_of_two(pt->depth0);
}

static bool
i915_texture_layout(struct i915_texture *tex)
{
   switch (tex->b.target) {
   case PIPE_TEXTURE_1D:
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_RECT:
      if (!i9x5_special_layout(tex))
         i915_texture_layout_2d(tex);
      break;
   case PIPE_TEXTURE_3D:
      i915_texture_layout_3d(tex);
      break;
   case PIPE_TEXTURE_CUBE:
      i9x5_texture_layout_cube(tex);
      break;
   default:
      assert(0);
      return false;
   }

   return true;
}

/*
 * i945 layout functions
 */

static void
i945_texture_layout_2d(struct i915_texture *tex)
{
   struct pipe_resource *pt = &tex->b;
   int align_x = 4, align_y = 2;
   unsigned level;
   unsigned x = 0;
   unsigned y = 0;
   unsigned width = util_next_power_of_two(pt->width0);
   unsigned height = util_next_power_of_two(pt->height0);
   unsigned nblocksx = util_format_get_nblocksx(pt->format, width);
   unsigned nblocksy = util_format_get_nblocksy(pt->format, height);

   if (util_format_is_compressed(pt->format)) {
      align_x = 1;
      align_y = 1;
   }

   tex->stride = align(util_format_get_stride(pt->format, width), 4);

   /* May need to adjust pitch to accommodate the placement of
    * the 2nd mipmap level.  This occurs when the alignment
    * constraints of mipmap placement push the right edge of the
    * 2nd mipmap level out past the width of its parent.
    */
   if (pt->last_level > 0) {
      unsigned mip1_nblocksx =
         align_nblocksx(pt->format, u_minify(width, 1), align_x) +
         util_format_get_nblocksx(pt->format, u_minify(width, 2));

      if (mip1_nblocksx > nblocksx)
         tex->stride = mip1_nblocksx * util_format_get_blocksize(pt->format);
   }

   /* Pitch must be a whole number of dwords
    */
   tex->stride = align(tex->stride, 64);
   tex->total_nblocksy = 0;

   for (level = 0; level <= pt->last_level; level++) {
      i915_texture_set_level_info(tex, level, 1);
      i915_texture_set_image_offset(tex, level, 0, x, y);

      /* Because the images are packed better, the final offset
       * might not be the maximal one:
       */
      tex->total_nblocksy = MAX2(tex->total_nblocksy, y + nblocksy);

      /* Layout_below: step right after second mipmap level.
       */
      if (level == 1) {
         x += nblocksx;
      } else {
         y += nblocksy;
      }

      width = u_minify(width, 1);
      height = u_minify(height, 1);
      nblocksx = align_nblocksx(pt->format, width, align_x);
      nblocksy = align_nblocksy(pt->format, height, align_y);
   }
}

static void
i945_texture_layout_3d(struct i915_texture *tex)
{
   struct pipe_resource *pt = &tex->b;
   unsigned width = util_next_power_of_two(pt->width0);
   unsigned height = util_next_power_of_two(pt->height0);
   unsigned depth = util_next_power_of_two(pt->depth0);
   unsigned nblocksy = util_format_get_nblocksy(pt->format, height);
   unsigned pack_x_pitch, pack_x_nr;
   unsigned pack_y_pitch;
   unsigned level;

   tex->stride = align(util_format_get_stride(pt->format, width), 4);
   tex->total_nblocksy = 0;

   pack_y_pitch = MAX2(nblocksy, 2);
   pack_x_pitch = tex->stride / util_format_get_blocksize(pt->format);
   pack_x_nr = 1;

   for (level = 0; level <= pt->last_level; level++) {
      int x = 0;
      int y = 0;
      unsigned q, j;

      i915_texture_set_level_info(tex, level, depth);

      for (q = 0; q < depth;) {
         for (j = 0; j < pack_x_nr && q < depth; j++, q++) {
            i915_texture_set_image_offset(tex, level, q, x,
                                          y + tex->total_nblocksy);
            x += pack_x_pitch;
         }

         x = 0;
         y += pack_y_pitch;
      }

      tex->total_nblocksy += y;

      if (pack_x_pitch > 4) {
         pack_x_pitch >>= 1;
         pack_x_nr <<= 1;
         assert(pack_x_pitch * pack_x_nr *
                   util_format_get_blocksize(pt->format) <=
                tex->stride);
      }

      if (pack_y_pitch > 2) {
         pack_y_pitch >>= 1;
      }

      width = u_minify(width, 1);
      height = u_minify(height, 1);
      depth = u_minify(depth, 1);
      nblocksy = util_format_get_nblocksy(pt->format, height);
   }
}

/**
 * Compressed cube texture map layout for i945 and later.
 *
 * The hardware layout looks like the 830-915 layout, except for the small
 * sizes.  A zoomed in view of the layout for 945 is:
 *
 * +-------+-------+
 * |  8x8  |  8x8  |
 * |       |       |
 * |       |       |
 * |  +x   |  +y   |
 * |       |       |
 * |       |       |
 * |       |       |
 * |       |       |
 * +---+---+-------+
 * |4x4|   |  8x8  |
 * | +x|   |       |
 * |   |   |       |
 * |   |   |       |
 * +---+   |  +z   |
 * |4x4|   |       |
 * | +y|   |       |
 * |   |   |       |
 * +---+   +-------+
 *
 * ...
 *
 * +-------+-------+
 * |  8x8  |  8x8  |
 * |       |       |
 * |       |       |
 * |  -x   |  -y   |
 * |       |       |
 * |       |       |
 * |       |       |
 * |       |       |
 * +---+---+-------+
 * |4x4|   |  8x8  |
 * | -x|   |       |
 * |   |   |       |
 * |   |   |       |
 * +---+   |  -z   |
 * |4x4|   |       |
 * | -y|   |       |
 * |   |   |       |
 * +---+   +---+---+---+---+---+---+---+---+---+
 * |4x4|   |4x4|   |2x2|   |2x2|   |2x2|   |2x2|
 * | +z|   | -z|   | +x|   | +y|   | +z|   | -x| ...
 * |   |   |   |   |   |   |   |   |   |   |   |
 * +---+   +---+   +---+   +---+   +---+   +---+
 *
 * The bottom row continues with the remaining 2x2 then the 1x1 mip contents
 * in order, with each of them aligned to a 8x8 block boundary.  Thus, for
 * 32x32 cube maps and smaller, the bottom row layout is going to dictate the
 * pitch of the tree.  For a tree with 4x4 images, the pitch is at least
 * 14 * 8 = 112 texels, for 2x2 it is at least 12 * 8 texels, and for 1x1
 * it is 6 * 8 texels.
 */
static void
i945_texture_layout_cube(struct i915_texture *tex)
{
   struct pipe_resource *pt = &tex->b;
   unsigned width = util_next_power_of_two(pt->width0);
   const unsigned nblocks = util_format_get_nblocksx(pt->format, width);
   const unsigned dim = width;
   unsigned level;
   unsigned face;

   assert(pt->width0 == pt->height0); /* cubemap images are square */
   assert(util_format_is_compressed(pt->format)); /* compressed only */

   /*
    * Depending on the size of the largest images, pitch can be
    * determined either by the old-style packing of cubemap faces,
    * or the final row of 4x4, 2x2 and 1x1 faces below this.
    *
    * 64  * 2 / 4 = 32
    * 14 * 2 = 28
    */
   if (width >= 64)
      tex->stride = nblocks * 2 * util_format_get_blocksize(pt->format);
   else
      tex->stride = 14 * 2 * util_format_get_blocksize(pt->format);

   /*
    * Something similary apply for height as well.
    */
   if (width >= 4)
      tex->total_nblocksy = nblocks * 4 + 1;
   else
      tex->total_nblocksy = 1;

   /* Set all the levels to effectively occupy the whole rectangular region */
   for (level = 0; level <= pt->last_level; level++)
      i915_texture_set_level_info(tex, level, 6);

   for (face = 0; face < 6; face++) {
      /* all calculations in pixels */
      unsigned total_height = tex->total_nblocksy * 4;
      unsigned x = initial_offsets[face][0] * dim;
      unsigned y = initial_offsets[face][1] * dim;
      unsigned d = dim;

      if (dim == 4 && face >= 4) {
         x = (face - 4) * 8;
         y = tex->total_nblocksy * 4 - 4; /* 4 = 1 block */
      } else if (dim < 4 && (face > 0)) {
         x = face * 8;
         y = total_height - 4;
      }

      for (level = 0; level <= pt->last_level; level++) {
         i915_texture_set_image_offset(tex, level, face,
                                       util_format_get_nblocksx(pt->format, x),
                                       util_format_get_nblocksy(pt->format, y));

         d >>= 1;

         switch (d) {
         case 4:
            switch (face) {
            case PIPE_TEX_FACE_POS_X:
            case PIPE_TEX_FACE_NEG_X:
               x += step_offsets[face][0] * d;
               y += step_offsets[face][1] * d;
               break;
            case PIPE_TEX_FACE_POS_Y:
            case PIPE_TEX_FACE_NEG_Y:
               y += 12;
               x -= 8;
               break;
            case PIPE_TEX_FACE_POS_Z:
            case PIPE_TEX_FACE_NEG_Z:
               y = total_height - 4;
               x = (face - 4) * 8;
               break;
            }
            break;
         case 2:
            y = total_height - 4;
            x = bottom_offsets[face];
            break;
         case 1:
            x += 48;
            break;
         default:
            x += step_offsets[face][0] * d;
            y += step_offsets[face][1] * d;
            break;
         }
      }
   }
}

static bool
i945_texture_layout(struct i915_texture *tex)
{
   switch (tex->b.target) {
   case PIPE_TEXTURE_1D:
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_RECT:
      if (!i9x5_special_layout(tex))
         i945_texture_layout_2d(tex);
      break;
   case PIPE_TEXTURE_3D:
      i945_texture_layout_3d(tex);
      break;
   case PIPE_TEXTURE_CUBE:
      if (!util_format_is_compressed(tex->b.format))
         i9x5_texture_layout_cube(tex);
      else
         i945_texture_layout_cube(tex);
      break;
   default:
      assert(0);
      return false;
   }

   return true;
}

/*
 * Screen texture functions
 */

bool
i915_resource_get_handle(struct pipe_screen *screen,
                         struct pipe_context *context,
                         struct pipe_resource *texture,
                         struct winsys_handle *whandle, unsigned usage)
{
   if (texture->target == PIPE_BUFFER)
      return false;

   struct i915_screen *is = i915_screen(screen);
   struct i915_texture *tex = i915_texture(texture);
   struct i915_winsys *iws = is->iws;

   return iws->buffer_get_handle(iws, tex->buffer, whandle, tex->stride);
}

void *
i915_texture_transfer_map(struct pipe_context *pipe,
                          struct pipe_resource *resource, unsigned level,
                          unsigned usage, const struct pipe_box *box,
                          struct pipe_transfer **ptransfer)
{
   struct i915_context *i915 = i915_context(pipe);
   struct i915_texture *tex = i915_texture(resource);
   struct i915_transfer *transfer = slab_alloc_st(&i915->texture_transfer_pool);
   bool use_staging_texture = false;
   struct i915_winsys *iws = i915_screen(pipe->screen)->iws;
   enum pipe_format format = resource->format;
   unsigned offset;
   char *map;

   if (!transfer)
      return NULL;

   transfer->b.resource = resource;
   transfer->b.level = level;
   transfer->b.usage = usage;
   transfer->b.box = *box;
   transfer->b.stride = tex->stride;
   transfer->staging_texture = NULL;
   /* XXX: handle depth textures everyhwere*/
   transfer->b.layer_stride = 0;

   /* if we use staging transfers, only support textures we can render to,
    * because we need that for u_blitter */
   if (i915->blitter &&
       util_blitter_is_copy_supported(i915->blitter, resource, resource) &&
       (usage & PIPE_MAP_WRITE) &&
       !(usage &
         (PIPE_MAP_READ | PIPE_MAP_DONTBLOCK | PIPE_MAP_UNSYNCHRONIZED)))
      use_staging_texture = true;

   use_staging_texture = false;

   if (use_staging_texture) {
      /*
       * Allocate the untiled staging texture.
       * If the alloc fails, transfer->staging_texture is NULL and we fallback
       * to a map()
       */
      transfer->staging_texture =
         i915_texture_create(pipe->screen, resource, true);
   }

   if (resource->target != PIPE_TEXTURE_3D &&
       resource->target != PIPE_TEXTURE_CUBE) {
      assert(box->z == 0);
      assert(box->depth == 1);
   }

   if (transfer->staging_texture) {
      tex = i915_texture(transfer->staging_texture);
   } else {
      /* TODO this is a sledgehammer */
      tex = i915_texture(resource);
      pipe->flush(pipe, NULL, 0);
   }

   offset = i915_texture_offset(tex, transfer->b.level, box->z);

   map = iws->buffer_map(iws, tex->buffer,
                         (transfer->b.usage & PIPE_MAP_WRITE) ? true : false);
   if (!map) {
      pipe_resource_reference(&transfer->staging_texture, NULL);
      FREE(transfer);
      return NULL;
   }

   *ptransfer = &transfer->b;

   return map + offset +
          box->y / util_format_get_blockheight(format) * transfer->b.stride +
          box->x / util_format_get_blockwidth(format) *
             util_format_get_blocksize(format);
}

void
i915_texture_transfer_unmap(struct pipe_context *pipe,
                            struct pipe_transfer *transfer)
{
   struct i915_context *i915 = i915_context(pipe);
   struct i915_transfer *itransfer = (struct i915_transfer *)transfer;
   struct i915_texture *tex = i915_texture(itransfer->b.resource);
   struct i915_winsys *iws = i915_screen(tex->b.screen)->iws;

   if (itransfer->staging_texture)
      tex = i915_texture(itransfer->staging_texture);

   iws->buffer_unmap(iws, tex->buffer);

   if ((itransfer->staging_texture) && (transfer->usage & PIPE_MAP_WRITE)) {
      struct pipe_box sbox;

      u_box_origin_2d(itransfer->b.box.width, itransfer->b.box.height, &sbox);
      pipe->resource_copy_region(pipe, itransfer->b.resource,
                                 itransfer->b.level, itransfer->b.box.x,
                                 itransfer->b.box.y, itransfer->b.box.z,
                                 itransfer->staging_texture, 0, &sbox);
      pipe->flush(pipe, NULL, 0);
      pipe_resource_reference(&itransfer->staging_texture, NULL);
   }

   slab_free_st(&i915->texture_transfer_pool, itransfer);
}

void
i915_texture_subdata(struct pipe_context *pipe, struct pipe_resource *resource,
                     unsigned level, unsigned usage, const struct pipe_box *box,
                     const void *data, unsigned stride, uintptr_t layer_stride)
{
   /* i915's cube and 3D maps are not laid out such that one could use a
    * layer_stride to get from one layer to the next, so we have to walk the
    * layers individually.
    */
   struct pipe_box layer_box = *box;
   layer_box.depth = 1;
   for (layer_box.z = box->z; layer_box.z < box->z + box->depth;
        layer_box.z++) {
      u_default_texture_subdata(pipe, resource, level, usage, &layer_box, data,
                                stride, layer_stride);
      data += layer_stride;
   }
}

struct pipe_resource *
i915_texture_create(struct pipe_screen *screen,
                    const struct pipe_resource *template, bool force_untiled)
{
   struct i915_screen *is = i915_screen(screen);
   struct i915_winsys *iws = is->iws;
   struct i915_texture *tex = CALLOC_STRUCT(i915_texture);
   unsigned buf_usage = 0;

   if (!tex)
      return NULL;

   tex->b = *template;
   pipe_reference_init(&tex->b.reference, 1);
   tex->b.screen = screen;

   if ((force_untiled) || (template->usage == PIPE_USAGE_STREAM))
      tex->tiling = I915_TILE_NONE;
   else
      tex->tiling = i915_texture_tiling(is, tex);

   if (is->is_i945) {
      if (!i945_texture_layout(tex))
         goto fail;
   } else {
      if (!i915_texture_layout(tex))
         goto fail;
   }

   /* for scanouts and cursors, cursors arn't scanouts */

   /* XXX: use a custom flag for cursors, don't rely on magically
    * guessing that this is Xorg asking for a cursor
    */
   if ((template->bind & PIPE_BIND_SCANOUT) && template->width0 != 64)
      buf_usage = I915_NEW_SCANOUT;
   else
      buf_usage = I915_NEW_TEXTURE;

   tex->buffer = iws->buffer_create_tiled(
      iws, &tex->stride, tex->total_nblocksy, &tex->tiling, buf_usage);
   if (!tex->buffer)
      goto fail;

   I915_DBG(DBG_TEXTURE, "%s: %p stride %u, blocks (%u, %u) tiling %s\n",
            __func__, tex, tex->stride,
            tex->stride / util_format_get_blocksize(tex->b.format),
            tex->total_nblocksy, get_tiling_string(tex->tiling));

   return &tex->b;

fail:
   FREE(tex);
   return NULL;
}

struct pipe_resource *
i915_texture_from_handle(struct pipe_screen *screen,
                         const struct pipe_resource *template,
                         struct winsys_handle *whandle)
{
   struct i915_screen *is = i915_screen(screen);
   struct i915_texture *tex;
   struct i915_winsys *iws = is->iws;
   struct i915_winsys_buffer *buffer;
   unsigned stride;
   enum i915_winsys_buffer_tile tiling;

   assert(screen);

   buffer = iws->buffer_from_handle(iws, whandle, template->height0, &tiling,
                                    &stride);

   /* Only supports one type */
   if ((template->target != PIPE_TEXTURE_2D &&
        template->target != PIPE_TEXTURE_RECT) ||
       template->last_level != 0 || template->depth0 != 1) {
      return NULL;
   }

   tex = CALLOC_STRUCT(i915_texture);
   if (!tex)
      return NULL;

   tex->b = *template;
   pipe_reference_init(&tex->b.reference, 1);
   tex->b.screen = screen;

   tex->stride = stride;
   tex->tiling = tiling;
   tex->total_nblocksy = align_nblocksy(tex->b.format, tex->b.height0, 8);

   i915_texture_set_level_info(tex, 0, 1);
   i915_texture_set_image_offset(tex, 0, 0, 0, 0);

   tex->buffer = buffer;

   I915_DBG(DBG_TEXTURE, "%s: %p stride %u, blocks (%u, %u) tiling %s\n",
            __func__, tex, tex->stride,
            tex->stride / util_format_get_blocksize(tex->b.format),
            tex->total_nblocksy, get_tiling_string(tex->tiling));

   return &tex->b;
}
