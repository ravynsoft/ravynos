/*
 * Copyright (c) 2011-2013 Luc Verhaegen <libv@skynet.be>
 * Copyright (c) 2017-2019 Lima Project
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
 */

#include "util/format/u_format.h"
#include "util/u_debug.h"
#include "util/u_draw.h"
#include "util/half_float.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_pack_color.h"
#include "util/u_split_draw.h"
#include "util/u_upload_mgr.h"
#include "util/u_prim.h"
#include "util/u_vbuf.h"
#include "util/hash_table.h"

#include "lima_context.h"
#include "lima_screen.h"
#include "lima_resource.h"
#include "lima_program.h"
#include "lima_bo.h"
#include "lima_job.h"
#include "lima_texture.h"
#include "lima_util.h"
#include "lima_gpu.h"

#include "pan_minmax_cache.h"

#include <drm-uapi/lima_drm.h>

static void
lima_clip_scissor_to_viewport(struct lima_context *ctx)
{
   struct lima_context_framebuffer *fb = &ctx->framebuffer;
   struct pipe_scissor_state *cscissor = &ctx->clipped_scissor;
   int viewport_left, viewport_right, viewport_bottom, viewport_top;

   if (ctx->rasterizer && ctx->rasterizer->base.scissor) {
      struct pipe_scissor_state *scissor = &ctx->scissor;
      cscissor->minx = scissor->minx;
      cscissor->maxx = scissor->maxx;
      cscissor->miny = scissor->miny;
      cscissor->maxy = scissor->maxy;
   } else {
      cscissor->minx = 0;
      cscissor->maxx = fb->base.width;
      cscissor->miny = 0;
      cscissor->maxy = fb->base.height;
   }

   viewport_left = MAX2(ctx->viewport.left, 0);
   cscissor->minx = MAX2(cscissor->minx, viewport_left);
   viewport_right = MIN2(MAX2(ctx->viewport.right, 0), fb->base.width);
   cscissor->maxx = MIN2(cscissor->maxx, viewport_right);
   if (cscissor->minx > cscissor->maxx)
      cscissor->minx = cscissor->maxx;

   viewport_bottom = MAX2(ctx->viewport.bottom, 0);
   cscissor->miny = MAX2(cscissor->miny, viewport_bottom);
   viewport_top = MIN2(MAX2(ctx->viewport.top, 0), fb->base.height);
   cscissor->maxy = MIN2(cscissor->maxy, viewport_top);
   if (cscissor->miny > cscissor->maxy)
      cscissor->miny = cscissor->maxy;
}

static void
lima_extend_viewport(struct lima_context *ctx, const struct pipe_draw_info *info)
{
   /* restore the original values */
   ctx->ext_viewport.left = ctx->viewport.left;
   ctx->ext_viewport.right = ctx->viewport.right;
   ctx->ext_viewport.bottom = ctx->viewport.bottom;
   ctx->ext_viewport.top = ctx->viewport.top;

   if (info->mode != MESA_PRIM_LINES)
      return;

   if (!ctx->rasterizer)
      return;

   float line_width = ctx->rasterizer->base.line_width;

   if (line_width == 1.0f)
      return;

   ctx->ext_viewport.left = ctx->viewport.left - line_width / 2;
   ctx->ext_viewport.right = ctx->viewport.right + line_width / 2;
   ctx->ext_viewport.bottom = ctx->viewport.bottom - line_width / 2;
   ctx->ext_viewport.top = ctx->viewport.top + line_width / 2;
}

static bool
lima_is_scissor_zero(struct lima_context *ctx)
{
   struct pipe_scissor_state *cscissor = &ctx->clipped_scissor;

   return cscissor->minx == cscissor->maxx || cscissor->miny == cscissor->maxy;
}

static void
lima_update_job_wb(struct lima_context *ctx, unsigned buffers)
{
   struct lima_job *job = lima_job_get(ctx);
   struct lima_context_framebuffer *fb = &ctx->framebuffer;

   /* add to job when the buffer is dirty and resolve is clear (not added before) */
   if (fb->base.nr_cbufs && (buffers & PIPE_CLEAR_COLOR0) &&
       !(job->resolve & PIPE_CLEAR_COLOR0)) {
      struct lima_resource *res = lima_resource(fb->base.cbufs[0]->texture);
      lima_flush_job_accessing_bo(ctx, res->bo, true);
      _mesa_hash_table_insert(ctx->write_jobs, &res->base, job);
      lima_job_add_bo(job, LIMA_PIPE_PP, res->bo, LIMA_SUBMIT_BO_WRITE);
   }

   /* add to job when the buffer is dirty and resolve is clear (not added before) */
   if (fb->base.zsbuf && (buffers & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL)) &&
       !(job->resolve & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL))) {
      struct lima_resource *res = lima_resource(fb->base.zsbuf->texture);
      lima_flush_job_accessing_bo(ctx, res->bo, true);
      _mesa_hash_table_insert(ctx->write_jobs, &res->base, job);
      lima_job_add_bo(job, LIMA_PIPE_PP, res->bo, LIMA_SUBMIT_BO_WRITE);
   }

   job->resolve |= buffers;
}

static void
lima_clear(struct pipe_context *pctx, unsigned buffers, const struct pipe_scissor_state *scissor_state,
           const union pipe_color_union *color, double depth, unsigned stencil)
{
   struct lima_context *ctx = lima_context(pctx);
   struct lima_job *job = lima_job_get(ctx);

   /* flush if this job already contains any draw, otherwise multi clear can be
    * combined into a single job */
   if (lima_job_has_draw_pending(job)) {
      lima_do_job(job);
      job = lima_job_get(ctx);
   }

   lima_update_job_wb(ctx, buffers);

   /* no need to reload if cleared */
   if (ctx->framebuffer.base.nr_cbufs && (buffers & PIPE_CLEAR_COLOR0)) {
      struct lima_surface *surf = lima_surface(ctx->framebuffer.base.cbufs[0]);
      surf->reload &= ~PIPE_CLEAR_COLOR0;
   }

   struct lima_job_clear *clear = &job->clear;
   clear->buffers = buffers;

   if (buffers & PIPE_CLEAR_COLOR0) {
      clear->color_8pc =
         ((uint32_t)float_to_ubyte(color->f[3]) << 24) |
         ((uint32_t)float_to_ubyte(color->f[2]) << 16) |
         ((uint32_t)float_to_ubyte(color->f[1]) << 8) |
         float_to_ubyte(color->f[0]);

      clear->color_16pc =
         ((uint64_t)float_to_ushort(color->f[3]) << 48) |
         ((uint64_t)float_to_ushort(color->f[2]) << 32) |
         ((uint64_t)float_to_ushort(color->f[1]) << 16) |
         float_to_ushort(color->f[0]);
   }

   struct lima_surface *zsbuf = lima_surface(ctx->framebuffer.base.zsbuf);

   if (buffers & PIPE_CLEAR_DEPTH) {
      clear->depth = util_pack_z(PIPE_FORMAT_Z24X8_UNORM, depth);
      if (zsbuf)
         zsbuf->reload &= ~PIPE_CLEAR_DEPTH;
   }

   if (buffers & PIPE_CLEAR_STENCIL) {
      clear->stencil = stencil;
      if (zsbuf)
         zsbuf->reload &= ~PIPE_CLEAR_STENCIL;
   }

   ctx->dirty |= LIMA_CONTEXT_DIRTY_CLEAR;

   lima_damage_rect_union(&job->damage_rect,
                          0, ctx->framebuffer.base.width,
                          0, ctx->framebuffer.base.height);
}

enum lima_attrib_type {
   LIMA_ATTRIB_FLOAT = 0x000,
   LIMA_ATTRIB_I32   = 0x001,
   LIMA_ATTRIB_U32   = 0x002,
   LIMA_ATTRIB_FP16  = 0x003,
   LIMA_ATTRIB_I16   = 0x004,
   LIMA_ATTRIB_U16   = 0x005,
   LIMA_ATTRIB_I8    = 0x006,
   LIMA_ATTRIB_U8    = 0x007,
   LIMA_ATTRIB_I8N   = 0x008,
   LIMA_ATTRIB_U8N   = 0x009,
   LIMA_ATTRIB_I16N  = 0x00A,
   LIMA_ATTRIB_U16N  = 0x00B,
   LIMA_ATTRIB_I32N  = 0x00D,
   LIMA_ATTRIB_U32N  = 0x00E,
   LIMA_ATTRIB_FIXED = 0x101
};

static enum lima_attrib_type
lima_pipe_format_to_attrib_type(enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);
   int i = util_format_get_first_non_void_channel(format);
   const struct util_format_channel_description *c = desc->channel + i;

   switch (c->type) {
   case UTIL_FORMAT_TYPE_FLOAT:
      if (c->size == 16)
         return LIMA_ATTRIB_FP16;
      else
         return LIMA_ATTRIB_FLOAT;
   case UTIL_FORMAT_TYPE_FIXED:
      return LIMA_ATTRIB_FIXED;
   case UTIL_FORMAT_TYPE_SIGNED:
      if (c->size == 8) {
         if (c->normalized)
            return LIMA_ATTRIB_I8N;
         else
            return LIMA_ATTRIB_I8;
      }
      else if (c->size == 16) {
         if (c->normalized)
            return LIMA_ATTRIB_I16N;
         else
            return LIMA_ATTRIB_I16;
      }
      else if (c->size == 32) {
         if (c->normalized)
            return LIMA_ATTRIB_I32N;
         else
            return LIMA_ATTRIB_I32;
      }
      break;
   case UTIL_FORMAT_TYPE_UNSIGNED:
      if (c->size == 8) {
         if (c->normalized)
            return LIMA_ATTRIB_U8N;
         else
            return LIMA_ATTRIB_U8;
      }
      else if (c->size == 16) {
         if (c->normalized)
            return LIMA_ATTRIB_U16N;
         else
            return LIMA_ATTRIB_U16;
      }
      else if (c->size == 32) {
         if (c->normalized)
            return LIMA_ATTRIB_U32N;
         else
            return LIMA_ATTRIB_U32;
      }
      break;
   }

   return LIMA_ATTRIB_FLOAT;
}

static void
lima_pack_vs_cmd(struct lima_context *ctx, const struct pipe_draw_info *info,
                 const struct pipe_draw_start_count_bias *draw)
{
   struct lima_context_constant_buffer *ccb =
      ctx->const_buffer + PIPE_SHADER_VERTEX;
   struct lima_vs_compiled_shader *vs = ctx->vs;
   struct lima_job *job = lima_job_get(ctx);

   VS_CMD_BEGIN(&job->vs_cmd_array, 24);

   if (!info->index_size) {
      VS_CMD_ARRAYS_SEMAPHORE_BEGIN_1();
      VS_CMD_ARRAYS_SEMAPHORE_BEGIN_2();
   }
   int uniform_size = MIN2(vs->state.uniform_size, ccb->size);

   int size = uniform_size + vs->state.constant_size + 32;
   VS_CMD_UNIFORMS_ADDRESS(
      lima_ctx_buff_va(ctx, lima_ctx_buff_gp_uniform),
      align(size, 16));

   VS_CMD_SHADER_ADDRESS(ctx->vs->bo->va, ctx->vs->state.shader_size);
   VS_CMD_SHADER_INFO(ctx->vs->state.prefetch, ctx->vs->state.shader_size);

   int num_outputs = ctx->vs->state.num_outputs;
   int num_attributes = ctx->vertex_elements->num_elements;
   VS_CMD_VARYING_ATTRIBUTE_COUNT(num_outputs, MAX2(1, num_attributes));

   VS_CMD_UNKNOWN1();

   VS_CMD_ATTRIBUTES_ADDRESS(
      lima_ctx_buff_va(ctx, lima_ctx_buff_gp_attribute_info),
      MAX2(1, num_attributes));

   VS_CMD_VARYINGS_ADDRESS(
      lima_ctx_buff_va(ctx, lima_ctx_buff_gp_varying_info),
      num_outputs);

   unsigned num = info->index_size ? (ctx->max_index - ctx->min_index + 1) : draw->count;
   VS_CMD_DRAW(num, info->index_size);

   VS_CMD_UNKNOWN2();

   VS_CMD_ARRAYS_SEMAPHORE_END(info->index_size);

   VS_CMD_END();
}

static void
lima_pack_plbu_cmd(struct lima_context *ctx, const struct pipe_draw_info *info,
                   const struct pipe_draw_start_count_bias *draw)
{
   struct lima_vs_compiled_shader *vs = ctx->vs;
   struct pipe_scissor_state *cscissor = &ctx->clipped_scissor;
   struct lima_job *job = lima_job_get(ctx);
   PLBU_CMD_BEGIN(&job->plbu_cmd_array, 32);

   PLBU_CMD_VIEWPORT_LEFT(fui(ctx->ext_viewport.left));
   PLBU_CMD_VIEWPORT_RIGHT(fui(ctx->ext_viewport.right));
   PLBU_CMD_VIEWPORT_BOTTOM(fui(ctx->ext_viewport.bottom));
   PLBU_CMD_VIEWPORT_TOP(fui(ctx->ext_viewport.top));

   if (!info->index_size)
      PLBU_CMD_ARRAYS_SEMAPHORE_BEGIN();

   int cf = ctx->rasterizer->base.cull_face;
   int ccw = ctx->rasterizer->base.front_ccw;
   uint32_t cull = 0;
   bool force_point_size = false;

   if (cf != PIPE_FACE_NONE) {
      if (cf & PIPE_FACE_FRONT)
         cull |= ccw ? 0x00040000 : 0x00020000;
      if (cf & PIPE_FACE_BACK)
         cull |= ccw ? 0x00020000 : 0x00040000;
   }

   /* Specify point size with PLBU command if shader doesn't write */
   if (info->mode == MESA_PRIM_POINTS && ctx->vs->state.point_size_idx == -1)
      force_point_size = true;

   /* Specify line width with PLBU command for lines */
   if (info->mode > MESA_PRIM_POINTS && info->mode < MESA_PRIM_TRIANGLES)
      force_point_size = true;

   PLBU_CMD_PRIMITIVE_SETUP(force_point_size, cull, info->index_size);

   PLBU_CMD_RSW_VERTEX_ARRAY(
      lima_ctx_buff_va(ctx, lima_ctx_buff_pp_plb_rsw),
      ctx->gp_output->va);

   /* TODO
    * - we should set it only for the first draw that enabled the scissor and for
    *   latter draw only if scissor is dirty
    */

   assert(cscissor->minx < cscissor->maxx && cscissor->miny < cscissor->maxy);
   PLBU_CMD_SCISSORS(cscissor->minx, cscissor->maxx, cscissor->miny, cscissor->maxy);

   lima_damage_rect_union(&job->damage_rect, cscissor->minx, cscissor->maxx,
                          cscissor->miny, cscissor->maxy);

   PLBU_CMD_UNKNOWN1();

   PLBU_CMD_DEPTH_RANGE_NEAR(fui(ctx->viewport.near));
   PLBU_CMD_DEPTH_RANGE_FAR(fui(ctx->viewport.far));

   if ((info->mode == MESA_PRIM_POINTS && ctx->vs->state.point_size_idx == -1) ||
       ((info->mode >= MESA_PRIM_LINES) && (info->mode < MESA_PRIM_TRIANGLES)))
   {
      uint32_t v = info->mode == MESA_PRIM_POINTS ?
         fui(ctx->rasterizer->base.point_size) : fui(ctx->rasterizer->base.line_width);
      PLBU_CMD_LOW_PRIM_SIZE(v);
   }

   if (info->index_size) {
      PLBU_CMD_INDEXED_DEST(ctx->gp_output->va);
      if (vs->state.point_size_idx != -1)
         PLBU_CMD_INDEXED_PT_SIZE(ctx->gp_output->va + ctx->gp_output_point_size_offt);

      PLBU_CMD_INDICES(ctx->index_res->bo->va + draw->start * info->index_size + ctx->index_offset);
   }
   else {
      /* can this make the attribute info static? */
      PLBU_CMD_DRAW_ARRAYS(info->mode, draw->start, draw->count);
   }

   PLBU_CMD_ARRAYS_SEMAPHORE_END();

   if (info->index_size)
      PLBU_CMD_DRAW_ELEMENTS(info->mode, ctx->min_index, draw->count);

   PLBU_CMD_END();
}

static int
lima_blend_func(enum pipe_blend_func pipe)
{
   switch (pipe) {
   case PIPE_BLEND_ADD:
      return 2;
   case PIPE_BLEND_SUBTRACT:
      return 0;
   case PIPE_BLEND_REVERSE_SUBTRACT:
      return 1;
   case PIPE_BLEND_MIN:
      return 4;
   case PIPE_BLEND_MAX:
      return 5;
   }
   return -1;
}

static int
lima_blend_factor(enum pipe_blendfactor pipe)
{
   /* Bits 0-2 indicate the blendfactor type,
    * Bit 3 is set if blendfactor is inverted
    * Bit 4 is set if blendfactor has alpha */
   switch (pipe) {
   case PIPE_BLENDFACTOR_SRC_COLOR:
      return 0 << 4 | 0 << 3 | 0;
   case PIPE_BLENDFACTOR_SRC_ALPHA:
      return 1 << 4 | 0 << 3 | 0;
   case PIPE_BLENDFACTOR_INV_SRC_COLOR:
      return 0 << 4 | 1 << 3 | 0;
   case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
      return 1 << 4 | 1 << 3 | 0;

   case PIPE_BLENDFACTOR_DST_COLOR:
      return 0 << 4 | 0 << 3 | 1;
   case PIPE_BLENDFACTOR_DST_ALPHA:
      return 1 << 4 | 0 << 3 | 1;
   case PIPE_BLENDFACTOR_INV_DST_COLOR:
      return 0 << 4 | 1 << 3 | 1;
   case PIPE_BLENDFACTOR_INV_DST_ALPHA:
      return 1 << 4 | 1 << 3 | 1;

   case PIPE_BLENDFACTOR_CONST_COLOR:
      return 0 << 4 | 0 << 3 | 2;
   case PIPE_BLENDFACTOR_CONST_ALPHA:
      return 1 << 4 | 0 << 3 | 2;
   case PIPE_BLENDFACTOR_INV_CONST_COLOR:
      return 0 << 4 | 1 << 3 | 2;
   case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
      return 1 << 4 | 1 << 3 | 2;

   case PIPE_BLENDFACTOR_ZERO:
      return 0 << 4 | 0 << 3 | 3;
   case PIPE_BLENDFACTOR_ONE:
      return 0 << 4 | 1 << 3 | 3;

   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
      return 0 << 4 | 0 << 3 | 4;

   case PIPE_BLENDFACTOR_SRC1_COLOR:
      return 0 << 4 | 0 << 3 | 5;
   case PIPE_BLENDFACTOR_SRC1_ALPHA:
      return 1 << 4 | 0 << 3 | 5;
   case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
      return 0 << 4 | 1 << 3 | 5;
   case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
      return 1 << 4 | 1 << 3 | 5;
   }
   return -1;
}

static int
lima_calculate_alpha_blend(enum pipe_blend_func rgb_func, enum pipe_blend_func alpha_func,
                           enum pipe_blendfactor rgb_src_factor, enum pipe_blendfactor rgb_dst_factor,
                           enum pipe_blendfactor alpha_src_factor, enum pipe_blendfactor alpha_dst_factor)
{
   /* PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE has to be changed to PIPE_BLENDFACTOR_ONE
    * if it is set for alpha_src or alpha_dst.
    */
   if (alpha_src_factor == PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE)
      alpha_src_factor = PIPE_BLENDFACTOR_ONE;

   if (alpha_dst_factor == PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE)
      alpha_dst_factor = PIPE_BLENDFACTOR_ONE;

   /* MIN and MAX ops actually do OP(As * S + Ad * D, Ad), so
    * we need to set S to 1 and D to 0 to get correct result */
   if (alpha_func == PIPE_BLEND_MIN ||
       alpha_func == PIPE_BLEND_MAX) {
      alpha_src_factor = PIPE_BLENDFACTOR_ONE;
      alpha_dst_factor = PIPE_BLENDFACTOR_ZERO;
   }

   /* MIN and MAX ops actually do OP(Cs * S + Cd * D, Cd), so
    * we need to set S to 1 and D to 0 to get correct result */
   if (rgb_func == PIPE_BLEND_MIN ||
       rgb_func == PIPE_BLEND_MAX) {
      rgb_src_factor = PIPE_BLENDFACTOR_ONE;
      rgb_dst_factor = PIPE_BLENDFACTOR_ZERO;
   }

   return lima_blend_func(rgb_func) |
      (lima_blend_func(alpha_func) << 3) |
      (lima_blend_factor(rgb_src_factor) << 6) |
      (lima_blend_factor(rgb_dst_factor) << 11) |
      /* alpha_src and alpha_dst are 4 bit, so need to mask 5th bit */
      ((lima_blend_factor(alpha_src_factor) & 0xf) << 16) |
      ((lima_blend_factor(alpha_dst_factor) & 0xf) << 20) |
      0x0C000000; /* need to check if this is GLESv1 glAlphaFunc */
}

static int
lima_stencil_op(enum pipe_stencil_op pipe)
{
   switch (pipe) {
   case PIPE_STENCIL_OP_KEEP:
      return 0;
   case PIPE_STENCIL_OP_ZERO:
      return 2;
   case PIPE_STENCIL_OP_REPLACE:
      return 1;
   case PIPE_STENCIL_OP_INCR:
      return 6;
   case PIPE_STENCIL_OP_DECR:
      return 7;
   case PIPE_STENCIL_OP_INCR_WRAP:
      return 4;
   case PIPE_STENCIL_OP_DECR_WRAP:
      return 5;
   case PIPE_STENCIL_OP_INVERT:
      return 3;
   }
   return -1;
}

static unsigned
lima_calculate_depth_test(struct pipe_depth_stencil_alpha_state *depth,
                          struct pipe_rasterizer_state *rst)
{
   int offset_scale = 0, offset_units = 0;
   enum pipe_compare_func func = (depth->depth_enabled ? depth->depth_func : PIPE_FUNC_ALWAYS);

   offset_scale = CLAMP(rst->offset_scale * 4, -128, 127);
   if (offset_scale < 0)
      offset_scale += 0x100;

   offset_units = CLAMP(rst->offset_units * 2, -128, 127);
   if (offset_units < 0)
      offset_units += 0x100;

   return (depth->depth_enabled && depth->depth_writemask) |
      ((int)func << 1) |
      (offset_scale << 16) |
      (offset_units << 24);
}

static void
lima_pack_render_state(struct lima_context *ctx, const struct pipe_draw_info *info)
{
   struct lima_fs_compiled_shader *fs = ctx->fs;
   struct lima_render_state *render =
      lima_ctx_buff_alloc(ctx, lima_ctx_buff_pp_plb_rsw,
                          sizeof(*render));
   bool early_z = true;
   bool pixel_kill = true;

   /* do hw support RGBA independ blend?
    * PIPE_CAP_INDEP_BLEND_ENABLE
    *
    * how to handle the no cbuf only zbuf case?
    */
   struct pipe_rt_blend_state *rt = ctx->blend->base.rt;
   render->blend_color_bg = float_to_ubyte(ctx->blend_color.color[2]) |
      (float_to_ubyte(ctx->blend_color.color[1]) << 16);
   render->blend_color_ra = float_to_ubyte(ctx->blend_color.color[0]) |
      (float_to_ubyte(ctx->blend_color.color[3]) << 16);

   if (rt->blend_enable) {
      render->alpha_blend = lima_calculate_alpha_blend(rt->rgb_func, rt->alpha_func,
         rt->rgb_src_factor, rt->rgb_dst_factor,
         rt->alpha_src_factor, rt->alpha_dst_factor);
   }
   else {
      /*
       * Special handling for blending disabled.
       * Binary driver is generating the same alpha_value,
       * as when we would just enable blending, without changing/setting any blend equation/params.
       * Normaly in this case mesa would set all rt fields (func/factor) to zero.
       */
      render->alpha_blend = lima_calculate_alpha_blend(PIPE_BLEND_ADD, PIPE_BLEND_ADD,
         PIPE_BLENDFACTOR_ONE, PIPE_BLENDFACTOR_ZERO,
         PIPE_BLENDFACTOR_ONE, PIPE_BLENDFACTOR_ZERO);
   }

   render->alpha_blend |= (rt->colormask & PIPE_MASK_RGBA) << 28;

   struct pipe_rasterizer_state *rst = &ctx->rasterizer->base;
   render->depth_test = lima_calculate_depth_test(&ctx->zsa->base, rst);

   if (!rst->depth_clip_near || ctx->viewport.near == 0.0f)
      render->depth_test |= 0x10; /* don't clip depth near */
   if (!rst->depth_clip_far || ctx->viewport.far == 1.0f)
      render->depth_test |= 0x20; /* don't clip depth far */

   if (fs->state.frag_depth_reg != -1) {
      render->depth_test |= (fs->state.frag_depth_reg << 6);
      /* Shader writes depth */
      render->depth_test |= 0x801;
   }

   uint16_t far, near;

   near = float_to_ushort(ctx->viewport.near);
   far = float_to_ushort(ctx->viewport.far);

   /* overlap with plbu? any place can remove one? */
   render->depth_range = near | (far << 16);

   struct pipe_stencil_state *stencil = ctx->zsa->base.stencil;
   struct pipe_stencil_ref *ref = &ctx->stencil_ref;

   if (stencil[0].enabled) { /* stencil is enabled */
      render->stencil_front = stencil[0].func |
         (lima_stencil_op(stencil[0].fail_op) << 3) |
         (lima_stencil_op(stencil[0].zfail_op) << 6) |
         (lima_stencil_op(stencil[0].zpass_op) << 9) |
         (ref->ref_value[0] << 16) |
         (stencil[0].valuemask << 24);
      render->stencil_back = render->stencil_front;
      render->stencil_test = (stencil[0].writemask & 0xff) | (stencil[0].writemask & 0xff) << 8;
      if (stencil[1].enabled) { /* two-side is enabled */
         render->stencil_back = stencil[1].func |
            (lima_stencil_op(stencil[1].fail_op) << 3) |
            (lima_stencil_op(stencil[1].zfail_op) << 6) |
            (lima_stencil_op(stencil[1].zpass_op) << 9) |
            (ref->ref_value[1] << 16) |
            (stencil[1].valuemask << 24);
         render->stencil_test = (stencil[0].writemask & 0xff) | (stencil[1].writemask & 0xff) << 8;
      }
      /* TODO: Find out, what (render->stecil_test & 0xff000000) is */
   }
   else {
      /* Default values, when stencil is disabled:
       * stencil[0|1].valuemask = 0xff
       * stencil[0|1].func = PIPE_FUNC_ALWAYS
       * stencil[0|1].writemask = 0xff
       */
      render->stencil_front = 0xff000007;
      render->stencil_back = 0xff000007;
      render->stencil_test = 0x0000ffff;
   }

   /* need more investigation */
   if (info->mode == MESA_PRIM_POINTS)
      render->multi_sample = 0x00000000;
   else if (info->mode < MESA_PRIM_TRIANGLES)
      render->multi_sample = 0x00000400;
   else
      render->multi_sample = 0x00000800;
   if (ctx->framebuffer.base.samples)
      render->multi_sample |= 0x68;
   if (ctx->blend->base.alpha_to_coverage)
      render->multi_sample |= (1 << 7);
   if (ctx->blend->base.alpha_to_one)
      render->multi_sample |= (1 << 8);
   render->multi_sample |= (ctx->sample_mask << 12);

   /* Set gl_FragColor register, need to specify it 4 times */
   render->multi_sample |= (fs->state.frag_color0_reg << 28) |
                           (fs->state.frag_color0_reg << 24) |
                           (fs->state.frag_color0_reg << 20) |
                           (fs->state.frag_color0_reg << 16);

   /* alpha test */
   if (ctx->zsa->base.alpha_enabled) {
      render->multi_sample |= ctx->zsa->base.alpha_func;
      render->stencil_test |= float_to_ubyte(ctx->zsa->base.alpha_ref_value) << 16;
   } else {
      /* func = PIPE_FUNC_ALWAYS */
      render->multi_sample |= 0x7;
   }

   render->shader_address =
      ctx->fs->bo->va | (((uint32_t *)ctx->fs->bo->map)[0] & 0x1F);

   /* seems not needed */
   render->uniforms_address = 0x00000000;

   render->textures_address = 0x00000000;

   render->aux0 = (ctx->vs->state.varying_stride >> 3);
   render->aux1 = 0x00000000;
   if (ctx->rasterizer->base.front_ccw)
      render->aux1 = 0x00001000;

   if (ctx->blend->base.dither)
      render->aux1 |= 0x00002000;

   if (fs->state.uses_discard ||
       ctx->zsa->base.alpha_enabled ||
       fs->state.frag_depth_reg != -1 ||
       ctx->blend->base.alpha_to_coverage) {
      early_z = false;
      pixel_kill = false;
   }

   if (rt->blend_enable)
      pixel_kill = false;

   if ((rt->colormask & PIPE_MASK_RGBA) != PIPE_MASK_RGBA)
      pixel_kill = false;

   if (early_z)
      render->aux0 |= 0x300;

   if (pixel_kill)
      render->aux0 |= 0x1000;

   if (ctx->tex_stateobj.num_samplers) {
      render->textures_address =
         lima_ctx_buff_va(ctx, lima_ctx_buff_pp_tex_desc);
      render->aux0 |= ctx->tex_stateobj.num_samplers << 14;
      render->aux0 |= 0x20;
   }

   if (ctx->const_buffer[PIPE_SHADER_FRAGMENT].buffer) {
      render->uniforms_address =
         lima_ctx_buff_va(ctx, lima_ctx_buff_pp_uniform_array);
      uint32_t size = ctx->buffer_state[lima_ctx_buff_pp_uniform].size;
      uint32_t bits = 0;
      if (size >= 8) {
         bits = util_last_bit(size >> 3) - 1;
         bits += size & u_bit_consecutive(0, bits + 3) ? 1 : 0;
      }
      render->uniforms_address |= bits > 0xf ? 0xf : bits;

      render->aux0 |= 0x80;
      render->aux1 |= 0x10000;
   }

   /* Set secondary output color */
   if (fs->state.frag_color1_reg != -1)
      render->aux0 |= (fs->state.frag_color1_reg << 28);

   if (ctx->vs->state.num_varyings) {
      render->varying_types = 0x00000000;
      render->varyings_address = ctx->gp_output->va +
                                 ctx->gp_output_varyings_offt;
      for (int i = 0, index = 0; i < ctx->vs->state.num_outputs; i++) {
         int val;

         if (i == ctx->vs->state.gl_pos_idx ||
             i == ctx->vs->state.point_size_idx)
            continue;

         struct lima_varying_info *v = ctx->vs->state.varying + i;
         if (v->component_size == 4)
            val = v->components > 2 ? 0 : 1;
         else
            val = v->components > 2 ? 2 : 3;

         if (index < 10)
            render->varying_types |= val << (3 * index);
         else if (index == 10) {
            render->varying_types |= val << 30;
            render->varyings_address |= val >> 2;
         }
         else if (index == 11)
            render->varyings_address |= val << 1;

         index++;
      }
   }
   else {
      render->varying_types = 0x00000000;
      render->varyings_address = 0x00000000;
   }

   struct lima_job *job = lima_job_get(ctx);

   lima_dump_command_stream_print(
      job->dump, render, sizeof(*render),
      false, "add render state at va %x\n",
      lima_ctx_buff_va(ctx, lima_ctx_buff_pp_plb_rsw));

   lima_dump_rsw_command_stream_print(
      job->dump, render, sizeof(*render),
      lima_ctx_buff_va(ctx, lima_ctx_buff_pp_plb_rsw));
}

static void
lima_update_gp_attribute_info(struct lima_context *ctx, const struct pipe_draw_info *info,
                              const struct pipe_draw_start_count_bias *draw)
{
   struct lima_job *job = lima_job_get(ctx);
   struct lima_vertex_element_state *ve = ctx->vertex_elements;
   struct lima_context_vertex_buffer *vb = &ctx->vertex_buffers;

   uint32_t *attribute =
      lima_ctx_buff_alloc(ctx, lima_ctx_buff_gp_attribute_info,
                          MAX2(1, ve->num_elements) * 8);

   int n = 0;
   for (int i = 0; i < ve->num_elements; i++) {
      struct pipe_vertex_element *pve = ve->pipe + i;

      assert(pve->vertex_buffer_index < vb->count);
      assert(vb->enabled_mask & (1 << pve->vertex_buffer_index));

      struct pipe_vertex_buffer *pvb = vb->vb + pve->vertex_buffer_index;
      struct lima_resource *res = lima_resource(pvb->buffer.resource);

      lima_job_add_bo(job, LIMA_PIPE_GP, res->bo, LIMA_SUBMIT_BO_READ);

      unsigned start = info->index_size ? (ctx->min_index + draw->index_bias) : draw->start;
      attribute[n++] = res->bo->va + pvb->buffer_offset + pve->src_offset
         + start * pve->src_stride;
      attribute[n++] = (pve->src_stride << 11) |
         (lima_pipe_format_to_attrib_type(pve->src_format) << 2) |
         (util_format_get_nr_components(pve->src_format) - 1);
   }

   lima_dump_command_stream_print(
      job->dump, attribute, n * 4, false, "update attribute info at va %x\n",
      lima_ctx_buff_va(ctx, lima_ctx_buff_gp_attribute_info));
}

static void
lima_update_gp_uniform(struct lima_context *ctx)
{
   struct lima_context_constant_buffer *ccb =
      ctx->const_buffer + PIPE_SHADER_VERTEX;
   struct lima_vs_compiled_shader *vs = ctx->vs;
   int uniform_size = MIN2(vs->state.uniform_size, ccb->size);

   int size = uniform_size + vs->state.constant_size + 32;
   void *vs_const_buff =
      lima_ctx_buff_alloc(ctx, lima_ctx_buff_gp_uniform, size);

   if (ccb->buffer)
      memcpy(vs_const_buff, ccb->buffer, uniform_size);

   memcpy(vs_const_buff + uniform_size,
          ctx->viewport.transform.scale,
          sizeof(ctx->viewport.transform.scale));
   memcpy(vs_const_buff + uniform_size + 16,
          ctx->viewport.transform.translate,
          sizeof(ctx->viewport.transform.translate));

   if (vs->constant)
      memcpy(vs_const_buff + uniform_size + 32,
             vs->constant, vs->state.constant_size);

   struct lima_job *job = lima_job_get(ctx);

   if (lima_debug & LIMA_DEBUG_GP) {
      float *vs_const_buff_f = vs_const_buff;
      printf("gp uniforms:\n");
      for (int i = 0; i < (size / sizeof(float)); i++) {
         if ((i % 4) == 0)
            printf("%4d:", i / 4);
         printf(" %8.4f", vs_const_buff_f[i]);
         if ((i % 4) == 3)
            printf("\n");
      }
      printf("\n");
   }

   lima_dump_command_stream_print(
      job->dump, vs_const_buff, size, true,
      "update gp uniform at va %x\n",
      lima_ctx_buff_va(ctx, lima_ctx_buff_gp_uniform));
}

static void
lima_update_pp_uniform(struct lima_context *ctx)
{
   const float *const_buff = ctx->const_buffer[PIPE_SHADER_FRAGMENT].buffer;
   size_t const_buff_size = ctx->const_buffer[PIPE_SHADER_FRAGMENT].size / sizeof(float);

   if (!const_buff)
      return;

   uint16_t *fp16_const_buff =
      lima_ctx_buff_alloc(ctx, lima_ctx_buff_pp_uniform,
                          const_buff_size * sizeof(uint16_t));

   uint32_t *array =
      lima_ctx_buff_alloc(ctx, lima_ctx_buff_pp_uniform_array, 4);

   for (int i = 0; i < const_buff_size; i++)
       fp16_const_buff[i] = _mesa_float_to_half(const_buff[i]);

   *array = lima_ctx_buff_va(ctx, lima_ctx_buff_pp_uniform);

   struct lima_job *job = lima_job_get(ctx);

   lima_dump_command_stream_print(
      job->dump, fp16_const_buff, const_buff_size * 2,
      false, "add pp uniform data at va %x\n",
      lima_ctx_buff_va(ctx, lima_ctx_buff_pp_uniform));
   lima_dump_command_stream_print(
      job->dump, array, 4, false, "add pp uniform info at va %x\n",
      lima_ctx_buff_va(ctx, lima_ctx_buff_pp_uniform_array));
}

static void
lima_update_varying(struct lima_context *ctx, const struct pipe_draw_info *info,
                    const struct pipe_draw_start_count_bias *draw)
{
   struct lima_job *job = lima_job_get(ctx);
   struct lima_screen *screen = lima_screen(ctx->base.screen);
   struct lima_vs_compiled_shader *vs = ctx->vs;
   uint32_t gp_output_size;
   unsigned num = info->index_size ? (ctx->max_index - ctx->min_index + 1) : draw->count;

   uint32_t *varying =
      lima_ctx_buff_alloc(ctx, lima_ctx_buff_gp_varying_info,
                          vs->state.num_outputs * 8);
   int n = 0;

   int offset = 0;

   for (int i = 0; i < vs->state.num_outputs; i++) {
      struct lima_varying_info *v = vs->state.varying + i;

      if (i == vs->state.gl_pos_idx ||
          i == vs->state.point_size_idx)
         continue;

      int size = v->component_size * 4;

      /* does component_size == 2 need to be 16 aligned? */
      if (v->component_size == 4)
         offset = align(offset, 16);

      v->offset = offset;
      offset += size;
   }

   vs->state.varying_stride = align(offset, 16);

   /* gl_Position is always present, allocate space for it */
   gp_output_size = align(4 * 4 * num, 0x40);

   /* Allocate space for varyings if there're any */
   if (vs->state.num_varyings) {
      ctx->gp_output_varyings_offt = gp_output_size;
      gp_output_size += align(vs->state.varying_stride * num, 0x40);
   }

   /* Allocate space for gl_PointSize if it's there */
   if (vs->state.point_size_idx != -1) {
      ctx->gp_output_point_size_offt = gp_output_size;
      gp_output_size += 4 * num;
   }

   /* gp_output can be too large for the suballocator, so create a
    * separate bo for it. The bo cache should prevent performance hit.
    */
   ctx->gp_output = lima_bo_create(screen, gp_output_size, 0);
   assert(ctx->gp_output);
   lima_job_add_bo(job, LIMA_PIPE_GP, ctx->gp_output, LIMA_SUBMIT_BO_WRITE);
   lima_job_add_bo(job, LIMA_PIPE_PP, ctx->gp_output, LIMA_SUBMIT_BO_READ);

   for (int i = 0; i < vs->state.num_outputs; i++) {
      struct lima_varying_info *v = vs->state.varying + i;

      if (i == vs->state.gl_pos_idx) {
         /* gl_Position */
         varying[n++] = ctx->gp_output->va;
         varying[n++] = 0x8020;
      } else if (i == vs->state.point_size_idx) {
         /* gl_PointSize */
         varying[n++] = ctx->gp_output->va + ctx->gp_output_point_size_offt;
         varying[n++] = 0x2021;
      } else {
         /* Varying */
         varying[n++] = ctx->gp_output->va + ctx->gp_output_varyings_offt +
                        v->offset;
         varying[n++] = (vs->state.varying_stride << 11) | (v->components - 1) |
            (v->component_size == 2 ? 0x0C : 0);
      }
   }

   lima_dump_command_stream_print(
      job->dump, varying, n * 4, false, "update varying info at va %x\n",
      lima_ctx_buff_va(ctx, lima_ctx_buff_gp_varying_info));
}

static void
lima_draw_vbo_update(struct pipe_context *pctx,
                     const struct pipe_draw_info *info,
                     const struct pipe_draw_start_count_bias *draw)
{
   struct lima_context *ctx = lima_context(pctx);
   struct lima_context_framebuffer *fb = &ctx->framebuffer;
   unsigned buffers = 0;

   if (fb->base.zsbuf) {
      if (ctx->zsa->base.depth_enabled)
         buffers |= PIPE_CLEAR_DEPTH;
      if (ctx->zsa->base.stencil[0].enabled ||
          ctx->zsa->base.stencil[1].enabled)
         buffers |= PIPE_CLEAR_STENCIL;
   }

   if (fb->base.nr_cbufs)
      buffers |= PIPE_CLEAR_COLOR0;

   lima_update_job_wb(ctx, buffers);

   lima_update_gp_attribute_info(ctx, info, draw);

   if ((ctx->dirty & LIMA_CONTEXT_DIRTY_CONST_BUFF &&
        ctx->const_buffer[PIPE_SHADER_VERTEX].dirty) ||
       ctx->dirty & LIMA_CONTEXT_DIRTY_VIEWPORT ||
       ctx->dirty & LIMA_CONTEXT_DIRTY_COMPILED_VS) {
      lima_update_gp_uniform(ctx);
      ctx->const_buffer[PIPE_SHADER_VERTEX].dirty = false;
   }

   lima_update_varying(ctx, info, draw);

   lima_pack_vs_cmd(ctx, info, draw);

   if (ctx->dirty & LIMA_CONTEXT_DIRTY_CONST_BUFF &&
       ctx->const_buffer[PIPE_SHADER_FRAGMENT].dirty) {
      lima_update_pp_uniform(ctx);
      ctx->const_buffer[PIPE_SHADER_FRAGMENT].dirty = false;
   }

   lima_update_textures(ctx);

   lima_pack_render_state(ctx, info);
   lima_pack_plbu_cmd(ctx, info, draw);

   if (ctx->gp_output) {
      lima_bo_unreference(ctx->gp_output); /* held by job */
      ctx->gp_output = NULL;
   }

   ctx->dirty = 0;
}

static void
lima_draw_vbo_indexed(struct pipe_context *pctx,
                      const struct pipe_draw_info *info,
                      const struct pipe_draw_start_count_bias *draw)
{
   struct lima_context *ctx = lima_context(pctx);
   struct lima_job *job = lima_job_get(ctx);
   struct pipe_resource *indexbuf = NULL;
   bool needs_indices = true;

   /* Mali Utgard GPU always need min/max index info for index draw,
    * compute it if upper layer does not do for us */
   if (info->index_bounds_valid) {
      ctx->min_index = info->min_index;
      ctx->max_index = info->max_index;
      needs_indices = false;
   }

   if (info->has_user_indices) {
      util_upload_index_buffer(&ctx->base, info, draw, &indexbuf, &ctx->index_offset, 0x40);
      ctx->index_res = lima_resource(indexbuf);
   }
   else {
      ctx->index_res = lima_resource(info->index.resource);
      ctx->index_offset = 0;
      needs_indices = !panfrost_minmax_cache_get(ctx->index_res->index_cache, draw->start,
                                                 draw->count, &ctx->min_index, &ctx->max_index);
   }

   if (needs_indices) {
      u_vbuf_get_minmax_index(pctx, info, draw, &ctx->min_index, &ctx->max_index);
      if (!info->has_user_indices)
         panfrost_minmax_cache_add(ctx->index_res->index_cache, draw->start, draw->count,
                                   ctx->min_index, ctx->max_index);
   }

   lima_job_add_bo(job, LIMA_PIPE_GP, ctx->index_res->bo, LIMA_SUBMIT_BO_READ);
   lima_job_add_bo(job, LIMA_PIPE_PP, ctx->index_res->bo, LIMA_SUBMIT_BO_READ);
   lima_draw_vbo_update(pctx, info, draw);

   if (indexbuf)
      pipe_resource_reference(&indexbuf, NULL);
}

static void
lima_draw_vbo_count(struct pipe_context *pctx,
                    const struct pipe_draw_info *info,
                    const struct pipe_draw_start_count_bias *draw)
{
   static const uint32_t max_verts = 65535;

   struct pipe_draw_start_count_bias local_draw = *draw;
   unsigned start = draw->start;
   unsigned count = draw->count;

   while (count) {
      unsigned this_count = count;
      unsigned step;

      u_split_draw(info, max_verts, &this_count, &step);

      local_draw.start = start;
      local_draw.count = this_count;

      lima_draw_vbo_update(pctx, info, &local_draw);

      count -= step;
      start += step;
   }
}

static void
lima_draw_vbo(struct pipe_context *pctx,
              const struct pipe_draw_info *info,
              unsigned drawid_offset,
              const struct pipe_draw_indirect_info *indirect,
              const struct pipe_draw_start_count_bias *draws,
              unsigned num_draws)
{
   if (num_draws > 1) {
      util_draw_multi(pctx, info, drawid_offset, indirect, draws, num_draws);
      return;
   }

   /* check if draw mode and vertex/index count match,
    * otherwise gp will hang */
   if (!u_trim_pipe_prim(info->mode, (unsigned*)&draws[0].count)) {
      debug_printf("draw mode and vertex/index count mismatch\n");
      return;
   }

   struct lima_context *ctx = lima_context(pctx);

   if (!ctx->uncomp_fs || !ctx->uncomp_vs) {
      debug_warn_once("no shader, skip draw\n");
      return;
   }

   lima_clip_scissor_to_viewport(ctx);
   if (lima_is_scissor_zero(ctx))
      return;

   /* extend the viewport in case of line draws with a line_width > 1.0f,
    * otherwise use the original values */
   lima_extend_viewport(ctx, info);

   if (!lima_update_fs_state(ctx) || !lima_update_vs_state(ctx))
      return;

   struct lima_job *job = lima_job_get(ctx);
   job->pp_max_stack_size = MAX2(job->pp_max_stack_size, ctx->fs->state.stack_size);

   lima_dump_command_stream_print(
      job->dump, ctx->vs->bo->map, ctx->vs->state.shader_size, false,
      "add vs at va %x\n", ctx->vs->bo->va);
   lima_dump_shader(job->dump, ctx->vs->bo->map, ctx->vs->state.shader_size, false);

   lima_dump_command_stream_print(
      job->dump, ctx->fs->bo->map, ctx->fs->state.shader_size, false,
      "add fs at va %x\n", ctx->fs->bo->va);
   lima_dump_shader(job->dump, ctx->fs->bo->map, ctx->fs->state.shader_size, true);

   lima_job_add_bo(job, LIMA_PIPE_GP, ctx->vs->bo, LIMA_SUBMIT_BO_READ);
   lima_job_add_bo(job, LIMA_PIPE_PP, ctx->fs->bo, LIMA_SUBMIT_BO_READ);

   if (info->index_size)
      lima_draw_vbo_indexed(pctx, info, &draws[0]);
   else
      lima_draw_vbo_count(pctx, info, &draws[0]);

   job->draws++;
   /* Flush job if we hit the limit of draws per job otherwise we may
    * hit tile heap size limit */
   if (job->draws > MAX_DRAWS_PER_JOB) {
      unsigned resolve = job->resolve;
      lima_do_job(job);
      /* Subsequent job will need to resolve the same buffers */
      lima_update_job_wb(ctx, resolve);
   }
}

void
lima_draw_init(struct lima_context *ctx)
{
   ctx->base.clear = lima_clear;
   ctx->base.draw_vbo = lima_draw_vbo;
}
