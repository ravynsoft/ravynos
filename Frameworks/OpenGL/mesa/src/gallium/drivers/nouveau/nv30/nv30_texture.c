/*
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 *
 */

#include "util/u_inlines.h"
#include "util/format/u_format.h"

#include "nv_object.xml.h"
#include "nv30/nv30-40_3d.xml.h"
#include "nv30/nv30_context.h"
#include "nv30/nv30_format.h"

#define NV30_3D_TEX_WRAP_S_MIRROR_REPEAT NV30_3D_TEX_WRAP_S_MIRRORED_REPEAT
#define NV30_WRAP(n) \
   case PIPE_TEX_WRAP_##n: ret = NV30_3D_TEX_WRAP_S_##n; break
#define NV40_WRAP(n) \
   case PIPE_TEX_WRAP_##n: ret = NV40_3D_TEX_WRAP_S_##n; break

static inline unsigned
wrap_mode(unsigned pipe)
{
   unsigned ret = NV30_3D_TEX_WRAP_S_REPEAT;

   switch (pipe) {
   NV30_WRAP(REPEAT);
   NV30_WRAP(MIRROR_REPEAT);
   NV30_WRAP(CLAMP_TO_EDGE);
   NV30_WRAP(CLAMP_TO_BORDER);
   NV30_WRAP(CLAMP);
   NV40_WRAP(MIRROR_CLAMP_TO_EDGE);
   NV40_WRAP(MIRROR_CLAMP_TO_BORDER);
   NV40_WRAP(MIRROR_CLAMP);
   default:
      break;
   }

   return ret >> NV30_3D_TEX_WRAP_S__SHIFT;
}

static inline unsigned
filter_mode(const struct pipe_sampler_state *cso)
{
   unsigned filter;

   switch (cso->mag_img_filter) {
   case PIPE_TEX_FILTER_LINEAR:
      filter = NV30_3D_TEX_FILTER_MAG_LINEAR;
      break;
   default:
      filter = NV30_3D_TEX_FILTER_MAG_NEAREST;
      break;
   }

   switch (cso->min_img_filter) {
   case PIPE_TEX_FILTER_LINEAR:
      switch (cso->min_mip_filter) {
      case PIPE_TEX_MIPFILTER_NEAREST:
         filter |= NV30_3D_TEX_FILTER_MIN_LINEAR_MIPMAP_NEAREST;
         break;
      case PIPE_TEX_MIPFILTER_LINEAR:
         filter |= NV30_3D_TEX_FILTER_MIN_LINEAR_MIPMAP_LINEAR;
         break;
      default:
         filter |= NV30_3D_TEX_FILTER_MIN_LINEAR;
         break;
      }
      break;
   default:
      switch (cso->min_mip_filter) {
      case PIPE_TEX_MIPFILTER_NEAREST:
         filter |= NV30_3D_TEX_FILTER_MIN_NEAREST_MIPMAP_NEAREST;
         break;
      case PIPE_TEX_MIPFILTER_LINEAR:
         filter |= NV30_3D_TEX_FILTER_MIN_NEAREST_MIPMAP_LINEAR;
         break;
      default:
         filter |= NV30_3D_TEX_FILTER_MIN_NEAREST;
         break;
      }
      break;
   }

   return filter;
}

static inline unsigned
compare_mode(const struct pipe_sampler_state *cso)
{
   if (cso->compare_mode != PIPE_TEX_COMPARE_R_TO_TEXTURE)
      return 0;

   switch (cso->compare_func) {
   case PIPE_FUNC_NEVER   : return NV30_3D_TEX_WRAP_RCOMP_NEVER;
   case PIPE_FUNC_GREATER : return NV30_3D_TEX_WRAP_RCOMP_GREATER;
   case PIPE_FUNC_EQUAL   : return NV30_3D_TEX_WRAP_RCOMP_EQUAL;
   case PIPE_FUNC_GEQUAL  : return NV30_3D_TEX_WRAP_RCOMP_GEQUAL;
   case PIPE_FUNC_LESS    : return NV30_3D_TEX_WRAP_RCOMP_LESS;
   case PIPE_FUNC_NOTEQUAL: return NV30_3D_TEX_WRAP_RCOMP_NOTEQUAL;
   case PIPE_FUNC_LEQUAL  : return NV30_3D_TEX_WRAP_RCOMP_LEQUAL;
   case PIPE_FUNC_ALWAYS  : return NV30_3D_TEX_WRAP_RCOMP_ALWAYS;
   default:
      return 0;
   }
}

static void *
nv30_sampler_state_create(struct pipe_context *pipe,
                          const struct pipe_sampler_state *cso)
{
   struct nouveau_object *eng3d = nv30_context(pipe)->screen->eng3d;
   struct nv30_sampler_state *so;
   const float max_lod = 15.0 + (255.0 / 256.0);

   so = MALLOC_STRUCT(nv30_sampler_state);
   if (!so)
      return NULL;

   so->pipe  = *cso;
   so->fmt   = 0;
   so->wrap  = (wrap_mode(cso->wrap_s) << NV30_3D_TEX_WRAP_S__SHIFT) |
               (wrap_mode(cso->wrap_t) << NV30_3D_TEX_WRAP_T__SHIFT) |
               (wrap_mode(cso->wrap_r) << NV30_3D_TEX_WRAP_R__SHIFT);
   so->en    = 0;
   so->wrap |= compare_mode(cso);
   so->filt  = filter_mode(cso) | 0x00002000;
   so->bcol  = (float_to_ubyte(cso->border_color.f[3]) << 24) |
               (float_to_ubyte(cso->border_color.f[0]) << 16) |
               (float_to_ubyte(cso->border_color.f[1]) <<  8) |
               (float_to_ubyte(cso->border_color.f[2]) <<  0);

   if (eng3d->oclass >= NV40_3D_CLASS) {
      unsigned aniso = cso->max_anisotropy;

      if (cso->unnormalized_coords)
         so->fmt |= NV40_3D_TEX_FORMAT_RECT;

      if (aniso > 1) {
         if      (aniso >= 16) so->en |= NV40_3D_TEX_ENABLE_ANISO_16X;
         else if (aniso >= 12) so->en |= NV40_3D_TEX_ENABLE_ANISO_12X;
         else if (aniso >= 10) so->en |= NV40_3D_TEX_ENABLE_ANISO_10X;
         else if (aniso >=  8) so->en |= NV40_3D_TEX_ENABLE_ANISO_8X;
         else if (aniso >=  6) so->en |= NV40_3D_TEX_ENABLE_ANISO_6X;
         else if (aniso >=  4) so->en |= NV40_3D_TEX_ENABLE_ANISO_4X;
         else                  so->en |= NV40_3D_TEX_ENABLE_ANISO_2X;

         so->wrap |= nv30_context(pipe)->config.aniso;
      }
   } else {
      so->en |= NV30_3D_TEX_ENABLE_ENABLE;

      if      (cso->max_anisotropy >= 8) so->en |= NV30_3D_TEX_ENABLE_ANISO_8X;
      else if (cso->max_anisotropy >= 4) so->en |= NV30_3D_TEX_ENABLE_ANISO_4X;
      else if (cso->max_anisotropy >= 2) so->en |= NV30_3D_TEX_ENABLE_ANISO_2X;
   }

   so->filt |= (int)(cso->lod_bias * 256.0) & 0x1fff;
   so->max_lod = (int)(CLAMP(cso->max_lod, 0.0, max_lod) * 256.0);
   so->min_lod = (int)(CLAMP(cso->min_lod, 0.0, max_lod) * 256.0);
   return so;
}

static void
nv30_sampler_state_delete(struct pipe_context *pipe, void *hwcso)
{
   FREE(hwcso);
}

static void
nv30_bind_sampler_states(struct pipe_context *pipe,
                         enum pipe_shader_type shader, unsigned start_slot,
                         unsigned num_samplers, void **samplers)
{
   switch (shader) {
   case PIPE_SHADER_VERTEX:
      nv40_verttex_sampler_states_bind(pipe, num_samplers, samplers);
      break;
   case PIPE_SHADER_FRAGMENT:
      nv30_fragtex_sampler_states_bind(pipe, num_samplers, samplers);
      break;
   default:
      assert(!"unexpected shader type");
      break;
   }
}

static inline uint32_t
swizzle(const struct nv30_texfmt *fmt, unsigned cmp, unsigned swz)
{
   uint32_t data = fmt->swz[swz].src << 8;
   if (swz <= PIPE_SWIZZLE_W)
      data |= fmt->swz[swz].cmp;
   else
      data |= fmt->swz[cmp].cmp;
   return data;
}

static struct pipe_sampler_view *
nv30_sampler_view_create(struct pipe_context *pipe, struct pipe_resource *pt,
                         const struct pipe_sampler_view *tmpl)
{
   const struct nv30_texfmt *fmt = nv30_texfmt(pipe->screen, tmpl->format);
   struct nouveau_object *eng3d = nv30_context(pipe)->screen->eng3d;
   struct nv30_miptree *mt = nv30_miptree(pt);
   struct nv30_sampler_view *so;

   so = MALLOC_STRUCT(nv30_sampler_view);
   if (!so)
      return NULL;
   so->pipe = *tmpl;
   so->pipe.reference.count = 1;
   so->pipe.texture = NULL;
   so->pipe.context = pipe;
   pipe_resource_reference(&so->pipe.texture, pt);

   so->fmt = NV30_3D_TEX_FORMAT_NO_BORDER;
   switch (pt->target) {
   case PIPE_TEXTURE_1D:
      so->fmt |= NV30_3D_TEX_FORMAT_DIMS_1D;
      break;
   case PIPE_TEXTURE_CUBE:
      so->fmt |= NV30_3D_TEX_FORMAT_CUBIC;
      FALLTHROUGH;
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_RECT:
      so->fmt |= NV30_3D_TEX_FORMAT_DIMS_2D;
      break;
   case PIPE_TEXTURE_3D:
      so->fmt |= NV30_3D_TEX_FORMAT_DIMS_3D;
      break;
   default:
      assert(0);
      so->fmt |= NV30_3D_TEX_FORMAT_DIMS_1D;
      break;
   }

   so->filt = fmt->filter;
   so->wrap = fmt->wrap;
   so->swz  = fmt->swizzle;
   so->swz |= swizzle(fmt, 3, tmpl->swizzle_a);
   so->swz |= swizzle(fmt, 0, tmpl->swizzle_r) << 2;
   so->swz |= swizzle(fmt, 1, tmpl->swizzle_g) << 4;
   so->swz |= swizzle(fmt, 2, tmpl->swizzle_b) << 6;

   /* apparently, we need to ignore the t coordinate for 1D textures to
    * fix piglit tex1d-2dborder
    */
   so->wrap_mask = ~0;
   if (pt->target == PIPE_TEXTURE_1D) {
      so->wrap_mask &= ~NV30_3D_TEX_WRAP_T__MASK;
      so->wrap      |=  NV30_3D_TEX_WRAP_T_REPEAT;
   }

   /* yet more hardware suckage, can't filter 32-bit float formats */
   switch (tmpl->format) {
   case PIPE_FORMAT_R32_FLOAT:
   case PIPE_FORMAT_R32G32B32A32_FLOAT:
      so->filt_mask = ~(NV30_3D_TEX_FILTER_MIN__MASK |
                        NV30_3D_TEX_FILTER_MAG__MASK);
      so->filt     |= NV30_3D_TEX_FILTER_MIN_NEAREST |
                      NV30_3D_TEX_FILTER_MAG_NEAREST;
      break;
   default:
      so->filt_mask = ~0;
      break;
   }

   so->npot_size0 = (pt->width0 << 16) | pt->height0;
   if (eng3d->oclass >= NV40_3D_CLASS) {
      so->npot_size1 = (pt->depth0 << 20) | mt->uniform_pitch;
      if (mt->uniform_pitch)
         so->fmt |= NV40_3D_TEX_FORMAT_LINEAR;
      so->fmt |= 0x00008000;
      so->fmt |= (pt->last_level + 1) << NV40_3D_TEX_FORMAT_MIPMAP_COUNT__SHIFT;
   } else {
      so->swz |= mt->uniform_pitch << NV30_3D_TEX_SWIZZLE_RECT_PITCH__SHIFT;
      if (pt->last_level)
         so->fmt |= NV30_3D_TEX_FORMAT_MIPMAP;
      so->fmt |= util_logbase2(pt->width0)  << 20;
      so->fmt |= util_logbase2(pt->height0) << 24;
      so->fmt |= util_logbase2(pt->depth0)  << 28;
      so->fmt |= 0x00010000;
   }

   so->base_lod = so->pipe.u.tex.first_level << 8;
   so->high_lod = MIN2(pt->last_level, so->pipe.u.tex.last_level) << 8;
   return &so->pipe;
}

static void
nv30_sampler_view_destroy(struct pipe_context *pipe,
                          struct pipe_sampler_view *view)
{
   pipe_resource_reference(&view->texture, NULL);
   FREE(view);
}

void
nv30_texture_init(struct pipe_context *pipe)
{
   pipe->create_sampler_state = nv30_sampler_state_create;
   pipe->delete_sampler_state = nv30_sampler_state_delete;
   pipe->bind_sampler_states = nv30_bind_sampler_states;

   pipe->create_sampler_view = nv30_sampler_view_create;
   pipe->sampler_view_destroy = nv30_sampler_view_destroy;
}
