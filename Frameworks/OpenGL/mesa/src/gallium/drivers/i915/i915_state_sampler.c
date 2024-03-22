/**************************************************************************
 *
 * Copyright 2003 VMware, Inc.
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

#include "pipe/p_context.h"
#include "pipe/p_state.h"

#include "i915_context.h"
#include "i915_reg.h"
#include "i915_resource.h"
#include "i915_state.h"
#include "i915_state_inlines.h"

/*
 * A note about min_lod & max_lod.
 *
 * There is a circular dependancy between the sampler state
 * and the map state to be submitted to hw.
 *
 * Two condition must be meet:
 * min_lod =< max_lod == true
 * max_lod =< last_level == true
 *
 *
 * This is all fine and dandy if it were for the fact that max_lod
 * is set on the map state instead of the sampler state. That is
 * the max_lod we submit on map is:
 * max_lod = MIN2(last_level, max_lod);
 *
 * So we need to update the map state when we change samplers and
 * we need to change the sampler state when map state is changed.
 * The first part is done by calling update_texture in update_samplers
 * and the second part is done else where in code tracking the state
 * changes.
 */

/***********************************************************************
 * Samplers
 */

/**
 * Compute i915 texture sampling state.
 *
 * Recalculate all state from scratch.  Perhaps not the most
 * efficient, but this has gotten complex enough that we need
 * something which is understandable and reliable.
 * \param state  returns the 3 words of compute state
 */
static void
update_sampler(struct i915_context *i915, uint32_t unit,
               const struct i915_sampler_state *sampler,
               const struct i915_texture *tex, unsigned state[3])
{
   const struct pipe_resource *pt = &tex->b;
   unsigned minlod, lastlod;

   state[0] = sampler->state[0];
   state[1] = sampler->state[1];
   state[2] = sampler->state[2];

   if (pt->format == PIPE_FORMAT_UYVY || pt->format == PIPE_FORMAT_YUYV)
      state[0] |= SS2_COLORSPACE_CONVERSION;

   if (util_format_is_srgb(pt->format)) {
      state[0] |= SS2_REVERSE_GAMMA_ENABLE;
   }

   /* There is no HW support for 1D textures, so we just make them 2D textures
    * with h=1, but that means we need to make the Y coordinate not contribute
    * to bringing any border color in.  Clearing it sets it to WRAP.
    */
   if (pt->target == PIPE_TEXTURE_1D) {
      state[1] &= ~SS3_TCY_ADDR_MODE_MASK;
   }

   /* The GLES2 spec says textures are incomplete (return 0,0,0,1) if:
    *
    * "A cube map sampler is called, any of the corresponding texture images are
    * non-power-of-two images, and either the texture wrap mode is not
    * CLAMP_TO_EDGE, or the minification filter is neither NEAREST nor LINEAR."
    *
    * while the i915 spec says:
    *
    * "When using cube map texture coordinates, only TEXCOORDMODE_CLAMP and *
    *  TEXCOORDMODE_CUBE settings are valid, and each TC component must have the
    *  same Address Control mode. TEXCOORDMODE_CUBE is not valid unless the
    *  width and height of the cube map are power-of-2."
    *
    * We don't expose support for the seamless cube map extension, so always use
    * edge clamping.
    */
   if (pt->target == PIPE_TEXTURE_CUBE) {
      state[1] &= ~(SS3_TCX_ADDR_MODE_MASK | SS3_TCY_ADDR_MODE_MASK |
                    SS3_TCZ_ADDR_MODE_MASK);
      state[1] |= (TEXCOORDMODE_CLAMP_EDGE << SS3_TCX_ADDR_MODE_SHIFT);
      state[1] |= (TEXCOORDMODE_CLAMP_EDGE << SS3_TCY_ADDR_MODE_SHIFT);
      state[1] |= (TEXCOORDMODE_CLAMP_EDGE << SS3_TCZ_ADDR_MODE_SHIFT);
   }

   /* 3D textures don't seem to respect the border color.
    * Fallback if there's ever a danger that they might refer to
    * it.
    *
    * Effectively this means fallback on 3D clamp or
    * clamp_to_border.
    *
    * XXX: Check if this is true on i945.
    * XXX: Check if this bug got fixed in release silicon.
    */
#if 0
   {
      const unsigned ws = sampler->templ->wrap_s;
      const unsigned wt = sampler->templ->wrap_t;
      const unsigned wr = sampler->templ->wrap_r;
      if (pt->target == PIPE_TEXTURE_3D &&
          (sampler->templ->min_img_filter != PIPE_TEX_FILTER_NEAREST ||
           sampler->templ->mag_img_filter != PIPE_TEX_FILTER_NEAREST) &&
          (ws == PIPE_TEX_WRAP_CLAMP ||
           wt == PIPE_TEX_WRAP_CLAMP ||
           wr == PIPE_TEX_WRAP_CLAMP ||
           ws == PIPE_TEX_WRAP_CLAMP_TO_BORDER ||
           wt == PIPE_TEX_WRAP_CLAMP_TO_BORDER || 
           wr == PIPE_TEX_WRAP_CLAMP_TO_BORDER)) {
         if (i915->conformance_mode > 0) {
            assert(0);
            /*             sampler->fallback = true; */
            /* TODO */
         }
      }
   }
#endif

   /* See note at the top of file */
   minlod = sampler->minlod;
   lastlod = pt->last_level << 4;

   if (lastlod < minlod) {
      minlod = lastlod;
   }

   state[1] |= (sampler->minlod << SS3_MIN_LOD_SHIFT);
   state[1] |= (unit << SS3_TEXTUREMAP_INDEX_SHIFT);
}

/***********************************************************************
 * Sampler views
 */

static uint32_t
translate_texture_format(enum pipe_format pipeFormat,
                         const struct pipe_sampler_view *view)
{
   if ((view->swizzle_r != PIPE_SWIZZLE_X ||
        view->swizzle_g != PIPE_SWIZZLE_Y ||
        view->swizzle_b != PIPE_SWIZZLE_Z ||
        view->swizzle_a != PIPE_SWIZZLE_W) &&
       pipeFormat != PIPE_FORMAT_Z24_UNORM_S8_UINT &&
       pipeFormat != PIPE_FORMAT_Z24X8_UNORM)
      debug_printf("i915: unsupported texture swizzle for format %d\n",
                   pipeFormat);

   switch (pipeFormat) {
   case PIPE_FORMAT_L8_UNORM:
      return MAPSURF_8BIT | MT_8BIT_L8;
   case PIPE_FORMAT_I8_UNORM:
      return MAPSURF_8BIT | MT_8BIT_I8;
   case PIPE_FORMAT_A8_UNORM:
      return MAPSURF_8BIT | MT_8BIT_A8;
   case PIPE_FORMAT_L8A8_UNORM:
      return MAPSURF_16BIT | MT_16BIT_AY88;
   case PIPE_FORMAT_B5G6R5_UNORM:
      return MAPSURF_16BIT | MT_16BIT_RGB565;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
      return MAPSURF_16BIT | MT_16BIT_ARGB1555;
   case PIPE_FORMAT_B4G4R4A4_UNORM:
      return MAPSURF_16BIT | MT_16BIT_ARGB4444;
   case PIPE_FORMAT_B10G10R10A2_UNORM:
      return MAPSURF_32BIT | MT_32BIT_ARGB2101010;
   case PIPE_FORMAT_B8G8R8A8_UNORM:
   case PIPE_FORMAT_B8G8R8A8_SRGB:
      return MAPSURF_32BIT | MT_32BIT_ARGB8888;
   case PIPE_FORMAT_B8G8R8X8_UNORM:
      return MAPSURF_32BIT | MT_32BIT_XRGB8888;
   case PIPE_FORMAT_R8G8B8A8_UNORM:
      return MAPSURF_32BIT | MT_32BIT_ABGR8888;
   case PIPE_FORMAT_R8G8B8X8_UNORM:
      return MAPSURF_32BIT | MT_32BIT_XBGR8888;
   case PIPE_FORMAT_YUYV:
      return (MAPSURF_422 | MT_422_YCRCB_NORMAL);
   case PIPE_FORMAT_UYVY:
      return (MAPSURF_422 | MT_422_YCRCB_SWAPY);
   case PIPE_FORMAT_FXT1_RGB:
   case PIPE_FORMAT_FXT1_RGBA:
      return (MAPSURF_COMPRESSED | MT_COMPRESS_FXT1);
   case PIPE_FORMAT_Z16_UNORM:
      return (MAPSURF_16BIT | MT_16BIT_L16);
   case PIPE_FORMAT_DXT1_RGB:
   case PIPE_FORMAT_DXT1_SRGB:
      return (MAPSURF_COMPRESSED | MT_COMPRESS_DXT1_RGB);
   case PIPE_FORMAT_DXT1_RGBA:
   case PIPE_FORMAT_DXT1_SRGBA:
      return (MAPSURF_COMPRESSED | MT_COMPRESS_DXT1);
   case PIPE_FORMAT_DXT3_RGBA:
   case PIPE_FORMAT_DXT3_SRGBA:
      return (MAPSURF_COMPRESSED | MT_COMPRESS_DXT2_3);
   case PIPE_FORMAT_DXT5_RGBA:
   case PIPE_FORMAT_DXT5_SRGBA:
      return (MAPSURF_COMPRESSED | MT_COMPRESS_DXT4_5);
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
   case PIPE_FORMAT_Z24X8_UNORM: {
      if (view->swizzle_r == PIPE_SWIZZLE_X &&
          view->swizzle_g == PIPE_SWIZZLE_X &&
          view->swizzle_b == PIPE_SWIZZLE_X &&
          view->swizzle_a == PIPE_SWIZZLE_1)
         return (MAPSURF_32BIT | MT_32BIT_xL824);
      if (view->swizzle_r == PIPE_SWIZZLE_X &&
          view->swizzle_g == PIPE_SWIZZLE_X &&
          view->swizzle_b == PIPE_SWIZZLE_X &&
          view->swizzle_a == PIPE_SWIZZLE_X)
         return (MAPSURF_32BIT | MT_32BIT_xI824);
      if (view->swizzle_r == PIPE_SWIZZLE_0 &&
          view->swizzle_g == PIPE_SWIZZLE_0 &&
          view->swizzle_b == PIPE_SWIZZLE_0 &&
          view->swizzle_a == PIPE_SWIZZLE_X)
         return (MAPSURF_32BIT | MT_32BIT_xA824);
      debug_printf("i915: unsupported depth swizzle %d %d %d %d\n",
                   view->swizzle_r, view->swizzle_g, view->swizzle_b,
                   view->swizzle_a);
      return (MAPSURF_32BIT | MT_32BIT_xL824);
   }
   default:
      debug_printf("i915: translate_texture_format() bad image format %x\n",
                   pipeFormat);
      assert(0);
      return 0;
   }
}

static inline uint32_t
ms3_tiling_bits(enum i915_winsys_buffer_tile tiling)
{
   uint32_t tiling_bits = 0;

   switch (tiling) {
   case I915_TILE_Y:
      tiling_bits |= MS3_TILE_WALK_Y;
      FALLTHROUGH;
   case I915_TILE_X:
      tiling_bits |= MS3_TILED_SURFACE;
      FALLTHROUGH;
   case I915_TILE_NONE:
      break;
   }

   return tiling_bits;
}

static void
update_map(struct i915_context *i915, uint32_t unit,
           const struct i915_texture *tex,
           const struct i915_sampler_state *sampler,
           const struct pipe_sampler_view *view, uint32_t state[3])
{
   const struct pipe_resource *pt = &tex->b;
   uint32_t width = pt->width0, height = pt->height0, depth = pt->depth0;
   int first_level = view->u.tex.first_level;
   const uint32_t num_levels = pt->last_level - first_level;
   unsigned max_lod = num_levels * 4;
   bool is_npot = (!util_is_power_of_two_or_zero(pt->width0) ||
                   !util_is_power_of_two_or_zero(pt->height0));
   uint32_t format, pitch;

   /*
    * This is a bit messy. i915 doesn't support NPOT with mipmaps, but we can
    * still texture from a single level. This is useful to make u_blitter work.
    */
   if (is_npot) {
      width = u_minify(width, first_level);
      height = u_minify(height, first_level);
      max_lod = 1;
   }

   assert(tex);
   assert(width);
   assert(height);
   assert(depth);

   format = translate_texture_format(pt->format, view);
   pitch = tex->stride;

   assert(format);
   assert(pitch);

   /* MS3 state */
   state[0] =
      (((height - 1) << MS3_HEIGHT_SHIFT) | ((width - 1) << MS3_WIDTH_SHIFT) |
       format | ms3_tiling_bits(tex->tiling));

   /*
    * XXX When min_filter != mag_filter and there's just one mipmap level,
    * set max_lod = 1 to make sure i915 chooses between min/mag filtering.
    */

   /* See note at the top of file */
   if (max_lod > (sampler->maxlod >> 2))
      max_lod = sampler->maxlod >> 2;

   /* MS4 state */
   state[1] = ((((pitch / 4) - 1) << MS4_PITCH_SHIFT) | MS4_CUBE_FACE_ENA_MASK |
               ((max_lod) << MS4_MAX_LOD_SHIFT) |
               ((depth - 1) << MS4_VOLUME_DEPTH_SHIFT));

   if (is_npot)
      state[2] = i915_texture_offset(tex, first_level, 0);
   else
      state[2] = 0;
}

static void
update_samplers(struct i915_context *i915)
{
   uint32_t unit;

   i915->current.sampler_enable_nr = 0;
   i915->current.sampler_enable_flags = 0x0;

   for (unit = 0;
        unit < i915->num_fragment_sampler_views && unit < i915->num_samplers;
        unit++) {
      /* determine unit enable/disable by looking for a bound texture */
      /* could also examine the fragment program? */
      if (i915->fragment_sampler_views[unit]) {
         struct i915_texture *texture =
            i915_texture(i915->fragment_sampler_views[unit]->texture);

         update_sampler(i915, unit,
                        i915->fragment_sampler[unit],   /* sampler state */
                        texture,                        /* texture */
                        i915->current.sampler[unit]);   /* the result */
         update_map(i915, unit, texture,                /* texture */
                    i915->fragment_sampler[unit],       /* sampler state */
                    i915->fragment_sampler_views[unit], /* sampler view */
                    i915->current.texbuffer[unit]);     /* the result */

         i915->current.sampler_enable_nr++;
         i915->current.sampler_enable_flags |= (1 << unit);
      }
   }

   i915->hardware_dirty |= I915_HW_SAMPLER | I915_HW_MAP;
}

struct i915_tracked_state i915_hw_samplers = {
   "samplers", update_samplers, I915_NEW_SAMPLER | I915_NEW_SAMPLER_VIEW};
