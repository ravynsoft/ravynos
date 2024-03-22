/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/u_helpers.h"
#include "util/u_memory.h"
#include "util/u_string.h"
#include "util/u_viewport.h"

#include "freedreno_query_hw.h"
#include "freedreno_resource.h"

#include "fd4_blend.h"
#include "fd4_context.h"
#include "fd4_emit.h"
#include "fd4_format.h"
#include "fd4_image.h"
#include "fd4_program.h"
#include "fd4_rasterizer.h"
#include "fd4_texture.h"
#include "fd4_zsa.h"

#define emit_const_user fd4_emit_const_user
#define emit_const_bo   fd4_emit_const_bo
#include "ir3_const.h"

/* regid:          base const register
 * prsc or dwords: buffer containing constant values
 * sizedwords:     size of const value buffer
 */
static void
fd4_emit_const_user(struct fd_ringbuffer *ring,
                    const struct ir3_shader_variant *v, uint32_t regid,
                    uint32_t sizedwords, const uint32_t *dwords)
{
   emit_const_asserts(ring, v, regid, sizedwords);

   OUT_PKT3(ring, CP_LOAD_STATE4, 2 + sizedwords);
   OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(regid / 4) |
                     CP_LOAD_STATE4_0_STATE_SRC(SS4_DIRECT) |
                     CP_LOAD_STATE4_0_STATE_BLOCK(fd4_stage2shadersb(v->type)) |
                     CP_LOAD_STATE4_0_NUM_UNIT(sizedwords / 4));
   OUT_RING(ring, CP_LOAD_STATE4_1_EXT_SRC_ADDR(0) |
                     CP_LOAD_STATE4_1_STATE_TYPE(ST4_CONSTANTS));
   for (int i = 0; i < sizedwords; i++)
      OUT_RING(ring, dwords[i]);
}

static void
fd4_emit_const_bo(struct fd_ringbuffer *ring,
                  const struct ir3_shader_variant *v, uint32_t regid,
                  uint32_t offset, uint32_t sizedwords, struct fd_bo *bo)
{
   uint32_t dst_off = regid / 4;
   assert(dst_off % 4 == 0);
   uint32_t num_unit = sizedwords / 4;
   assert(num_unit % 4 == 0);

   emit_const_asserts(ring, v, regid, sizedwords);

   OUT_PKT3(ring, CP_LOAD_STATE4, 2);
   OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(dst_off) |
                     CP_LOAD_STATE4_0_STATE_SRC(SS4_INDIRECT) |
                     CP_LOAD_STATE4_0_STATE_BLOCK(fd4_stage2shadersb(v->type)) |
                     CP_LOAD_STATE4_0_NUM_UNIT(num_unit));
   OUT_RELOC(ring, bo, offset, CP_LOAD_STATE4_1_STATE_TYPE(ST4_CONSTANTS), 0);
}

static void
fd4_emit_const_ptrs(struct fd_ringbuffer *ring, gl_shader_stage type,
                    uint32_t regid, uint32_t num, struct fd_bo **bos,
                    uint32_t *offsets)
{
   uint32_t anum = align(num, 4);
   uint32_t i;

   assert((regid % 4) == 0);

   OUT_PKT3(ring, CP_LOAD_STATE4, 2 + anum);
   OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(regid / 4) |
                     CP_LOAD_STATE4_0_STATE_SRC(SS4_DIRECT) |
                     CP_LOAD_STATE4_0_STATE_BLOCK(fd4_stage2shadersb(type)) |
                     CP_LOAD_STATE4_0_NUM_UNIT(anum / 4));
   OUT_RING(ring, CP_LOAD_STATE4_1_EXT_SRC_ADDR(0) |
                     CP_LOAD_STATE4_1_STATE_TYPE(ST4_CONSTANTS));

   for (i = 0; i < num; i++) {
      if (bos[i]) {
         OUT_RELOC(ring, bos[i], offsets[i], 0, 0);
      } else {
         OUT_RING(ring, 0xbad00000 | (i << 16));
      }
   }

   for (; i < anum; i++)
      OUT_RING(ring, 0xffffffff);
}

static bool
is_stateobj(struct fd_ringbuffer *ring)
{
   return false;
}

static void
emit_const_ptrs(struct fd_ringbuffer *ring, const struct ir3_shader_variant *v,
                uint32_t dst_offset, uint32_t num, struct fd_bo **bos,
                uint32_t *offsets)
{
   /* TODO inline this */
   assert(dst_offset + num <= v->constlen * 4);
   fd4_emit_const_ptrs(ring, v->type, dst_offset, num, bos, offsets);
}

void
fd4_emit_cs_consts(const struct ir3_shader_variant *v,
                   struct fd_ringbuffer *ring, struct fd_context *ctx,
                   const struct pipe_grid_info *info)
{
   ir3_emit_cs_consts(v, ring, ctx, info);
}

static void
emit_textures(struct fd_context *ctx, struct fd_ringbuffer *ring,
              enum a4xx_state_block sb, struct fd_texture_stateobj *tex,
              const struct ir3_shader_variant *v)
{
   static const uint32_t bcolor_reg[] = {
      [SB4_VS_TEX] = REG_A4XX_TPL1_TP_VS_BORDER_COLOR_BASE_ADDR,
      [SB4_FS_TEX] = REG_A4XX_TPL1_TP_FS_BORDER_COLOR_BASE_ADDR,
      [SB4_CS_TEX] = REG_A4XX_TPL1_TP_CS_BORDER_COLOR_BASE_ADDR,
   };
   struct fd4_context *fd4_ctx = fd4_context(ctx);
   bool needs_border = false;
   unsigned i;

   if (tex->num_samplers > 0 || tex->num_textures > 0) {
      int num_samplers = tex->num_samplers;

      /* We want to always make sure that there's at least one sampler if
       * there are going to be texture accesses. Gallium might not upload a
       * sampler for e.g. buffer textures.
       */
      if (num_samplers == 0)
         num_samplers++;

      /* not sure if this is an a420.0 workaround, but we seem
       * to need to emit these in pairs.. emit a final dummy
       * entry if odd # of samplers:
       */
      num_samplers = align(num_samplers, 2);

      /* output sampler state: */
      OUT_PKT3(ring, CP_LOAD_STATE4, 2 + (2 * num_samplers));
      OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(0) |
                        CP_LOAD_STATE4_0_STATE_SRC(SS4_DIRECT) |
                        CP_LOAD_STATE4_0_STATE_BLOCK(sb) |
                        CP_LOAD_STATE4_0_NUM_UNIT(num_samplers));
      OUT_RING(ring, CP_LOAD_STATE4_1_STATE_TYPE(ST4_SHADER) |
                        CP_LOAD_STATE4_1_EXT_SRC_ADDR(0));
      for (i = 0; i < tex->num_samplers; i++) {
         static const struct fd4_sampler_stateobj dummy_sampler = {};
         const struct fd4_sampler_stateobj *sampler =
            tex->samplers[i] ? fd4_sampler_stateobj(tex->samplers[i])
                             : &dummy_sampler;
         OUT_RING(ring, sampler->texsamp0);
         OUT_RING(ring, sampler->texsamp1);

         needs_border |= sampler->needs_border;
      }

      for (; i < num_samplers; i++) {
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
      }
   }

   if (tex->num_textures > 0) {
      unsigned num_textures = tex->num_textures + v->astc_srgb.count + v->tg4.count;

      /* emit texture state: */
      OUT_PKT3(ring, CP_LOAD_STATE4, 2 + (8 * num_textures));
      OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(0) |
                        CP_LOAD_STATE4_0_STATE_SRC(SS4_DIRECT) |
                        CP_LOAD_STATE4_0_STATE_BLOCK(sb) |
                        CP_LOAD_STATE4_0_NUM_UNIT(num_textures));
      OUT_RING(ring, CP_LOAD_STATE4_1_STATE_TYPE(ST4_CONSTANTS) |
                        CP_LOAD_STATE4_1_EXT_SRC_ADDR(0));
      for (i = 0; i < tex->num_textures; i++) {
         static const struct fd4_pipe_sampler_view dummy_view = {};
         const struct fd4_pipe_sampler_view *view =
            tex->textures[i] ? fd4_pipe_sampler_view(tex->textures[i])
                             : &dummy_view;

         OUT_RING(ring, view->texconst0);
         OUT_RING(ring, view->texconst1);
         OUT_RING(ring, view->texconst2);
         OUT_RING(ring, view->texconst3);
         if (view->base.texture) {
            struct fd_resource *rsc = fd_resource(view->base.texture);
            if (view->base.format == PIPE_FORMAT_X32_S8X24_UINT)
               rsc = rsc->stencil;
            OUT_RELOC(ring, rsc->bo, view->offset, view->texconst4, 0);
         } else {
            OUT_RING(ring, 0x00000000);
         }
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
      }

      for (i = 0; i < v->astc_srgb.count; i++) {
         static const struct fd4_pipe_sampler_view dummy_view = {};
         const struct fd4_pipe_sampler_view *view;
         unsigned idx = v->astc_srgb.orig_idx[i];

         view = tex->textures[idx] ? fd4_pipe_sampler_view(tex->textures[idx])
                                   : &dummy_view;

         assert(view->texconst0 & A4XX_TEX_CONST_0_SRGB);

         OUT_RING(ring, view->texconst0 & ~A4XX_TEX_CONST_0_SRGB);
         OUT_RING(ring, view->texconst1);
         OUT_RING(ring, view->texconst2);
         OUT_RING(ring, view->texconst3);
         if (view->base.texture) {
            struct fd_resource *rsc = fd_resource(view->base.texture);
            OUT_RELOC(ring, rsc->bo, view->offset, view->texconst4, 0);
         } else {
            OUT_RING(ring, 0x00000000);
         }
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
      }

      for (i = 0; i < v->tg4.count; i++) {
         static const struct fd4_pipe_sampler_view dummy_view = {};
         const struct fd4_pipe_sampler_view *view;
         unsigned idx = v->tg4.orig_idx[i];

         view = tex->textures[idx] ? fd4_pipe_sampler_view(tex->textures[idx])
                                   : &dummy_view;

         unsigned texconst0 = view->texconst0 & ~(0xfff << 4);
         texconst0 |= A4XX_TEX_CONST_0_SWIZ_X(A4XX_TEX_X) |
            A4XX_TEX_CONST_0_SWIZ_Y(A4XX_TEX_Y) |
            A4XX_TEX_CONST_0_SWIZ_Z(A4XX_TEX_Z) |
            A4XX_TEX_CONST_0_SWIZ_W(A4XX_TEX_W);

         /* Remap integer formats as unorm (will be fixed up in shader) */
         if (util_format_is_pure_integer(view->base.format)) {
            texconst0 &= ~A4XX_TEX_CONST_0_FMT__MASK;
            switch (fd4_pipe2tex(view->base.format)) {
            case TFMT4_8_8_8_8_UINT:
            case TFMT4_8_8_8_8_SINT:
               texconst0 |= A4XX_TEX_CONST_0_FMT(TFMT4_8_8_8_8_UNORM);
               break;
            case TFMT4_8_8_UINT:
            case TFMT4_8_8_SINT:
               texconst0 |= A4XX_TEX_CONST_0_FMT(TFMT4_8_8_UNORM);
               break;
            case TFMT4_8_UINT:
            case TFMT4_8_SINT:
               texconst0 |= A4XX_TEX_CONST_0_FMT(TFMT4_8_UNORM);
               break;

            case TFMT4_16_16_16_16_UINT:
            case TFMT4_16_16_16_16_SINT:
               texconst0 |= A4XX_TEX_CONST_0_FMT(TFMT4_16_16_16_16_UNORM);
               break;
            case TFMT4_16_16_UINT:
            case TFMT4_16_16_SINT:
               texconst0 |= A4XX_TEX_CONST_0_FMT(TFMT4_16_16_UNORM);
               break;
            case TFMT4_16_UINT:
            case TFMT4_16_SINT:
               texconst0 |= A4XX_TEX_CONST_0_FMT(TFMT4_16_UNORM);
               break;

            case TFMT4_32_32_32_32_UINT:
            case TFMT4_32_32_32_32_SINT:
               texconst0 |= A4XX_TEX_CONST_0_FMT(TFMT4_32_32_32_32_FLOAT);
               break;
            case TFMT4_32_32_UINT:
            case TFMT4_32_32_SINT:
               texconst0 |= A4XX_TEX_CONST_0_FMT(TFMT4_32_32_FLOAT);
               break;
            case TFMT4_32_UINT:
            case TFMT4_32_SINT:
               texconst0 |= A4XX_TEX_CONST_0_FMT(TFMT4_32_FLOAT);
               break;

            case TFMT4_10_10_10_2_UINT:
               texconst0 |= A4XX_TEX_CONST_0_FMT(TFMT4_10_10_10_2_UNORM);
               break;

            default:
               assert(0);
            }
         }

         OUT_RING(ring, texconst0);
         OUT_RING(ring, view->texconst1);
         OUT_RING(ring, view->texconst2);
         OUT_RING(ring, view->texconst3);
         if (view->base.texture) {
            struct fd_resource *rsc = fd_resource(view->base.texture);
            OUT_RELOC(ring, rsc->bo, view->offset, view->texconst4, 0);
         } else {
            OUT_RING(ring, 0x00000000);
         }
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
      }
   } else {
      assert(v->astc_srgb.count == 0);
      assert(v->tg4.count == 0);
   }

   if (needs_border) {
      unsigned off;
      void *ptr;

      u_upload_alloc(fd4_ctx->border_color_uploader, 0,
                     BORDER_COLOR_UPLOAD_SIZE, BORDER_COLOR_UPLOAD_SIZE, &off,
                     &fd4_ctx->border_color_buf, &ptr);

      fd_setup_border_colors(tex, ptr, 0);
      OUT_PKT0(ring, bcolor_reg[sb], 1);
      OUT_RELOC(ring, fd_resource(fd4_ctx->border_color_buf)->bo, off, 0, 0);

      u_upload_unmap(fd4_ctx->border_color_uploader);
   }
}

/* emit texture state for mem->gmem restore operation.. eventually it would
 * be good to get rid of this and use normal CSO/etc state for more of these
 * special cases..
 */
void
fd4_emit_gmem_restore_tex(struct fd_ringbuffer *ring, unsigned nr_bufs,
                          struct pipe_surface **bufs)
{
   unsigned char mrt_comp[A4XX_MAX_RENDER_TARGETS];
   int i;

   for (i = 0; i < A4XX_MAX_RENDER_TARGETS; i++) {
      mrt_comp[i] = (i < nr_bufs) ? 0xf : 0;
   }

   /* output sampler state: */
   OUT_PKT3(ring, CP_LOAD_STATE4, 2 + (2 * nr_bufs));
   OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(0) |
                     CP_LOAD_STATE4_0_STATE_SRC(SS4_DIRECT) |
                     CP_LOAD_STATE4_0_STATE_BLOCK(SB4_FS_TEX) |
                     CP_LOAD_STATE4_0_NUM_UNIT(nr_bufs));
   OUT_RING(ring, CP_LOAD_STATE4_1_STATE_TYPE(ST4_SHADER) |
                     CP_LOAD_STATE4_1_EXT_SRC_ADDR(0));
   for (i = 0; i < nr_bufs; i++) {
      OUT_RING(ring, A4XX_TEX_SAMP_0_XY_MAG(A4XX_TEX_NEAREST) |
                        A4XX_TEX_SAMP_0_XY_MIN(A4XX_TEX_NEAREST) |
                        A4XX_TEX_SAMP_0_WRAP_S(A4XX_TEX_CLAMP_TO_EDGE) |
                        A4XX_TEX_SAMP_0_WRAP_T(A4XX_TEX_CLAMP_TO_EDGE) |
                        A4XX_TEX_SAMP_0_WRAP_R(A4XX_TEX_REPEAT));
      OUT_RING(ring, 0x00000000);
   }

   /* emit texture state: */
   OUT_PKT3(ring, CP_LOAD_STATE4, 2 + (8 * nr_bufs));
   OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(0) |
                     CP_LOAD_STATE4_0_STATE_SRC(SS4_DIRECT) |
                     CP_LOAD_STATE4_0_STATE_BLOCK(SB4_FS_TEX) |
                     CP_LOAD_STATE4_0_NUM_UNIT(nr_bufs));
   OUT_RING(ring, CP_LOAD_STATE4_1_STATE_TYPE(ST4_CONSTANTS) |
                     CP_LOAD_STATE4_1_EXT_SRC_ADDR(0));
   for (i = 0; i < nr_bufs; i++) {
      if (bufs[i]) {
         struct fd_resource *rsc = fd_resource(bufs[i]->texture);
         enum pipe_format format = fd_gmem_restore_format(bufs[i]->format);

         /* The restore blit_zs shader expects stencil in sampler 0,
          * and depth in sampler 1
          */
         if (rsc->stencil && (i == 0)) {
            rsc = rsc->stencil;
            format = fd_gmem_restore_format(rsc->b.b.format);
         }

         /* note: PIPE_BUFFER disallowed for surfaces */
         unsigned lvl = bufs[i]->u.tex.level;
         unsigned offset =
            fd_resource_offset(rsc, lvl, bufs[i]->u.tex.first_layer);

         /* z32 restore is accomplished using depth write.  If there is
          * no stencil component (ie. PIPE_FORMAT_Z32_FLOAT_S8X24_UINT)
          * then no render target:
          *
          * (The same applies for z32_s8x24, since for stencil sampler
          * state the above 'if' will replace 'format' with s8)
          */
         if ((format == PIPE_FORMAT_Z32_FLOAT) ||
             (format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT))
            mrt_comp[i] = 0;

         assert(bufs[i]->u.tex.first_layer == bufs[i]->u.tex.last_layer);

         OUT_RING(ring, A4XX_TEX_CONST_0_FMT(fd4_pipe2tex(format)) |
                           A4XX_TEX_CONST_0_TYPE(A4XX_TEX_2D) |
                           fd4_tex_swiz(format, PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y,
                                        PIPE_SWIZZLE_Z, PIPE_SWIZZLE_W));
         OUT_RING(ring, A4XX_TEX_CONST_1_WIDTH(bufs[i]->width) |
                           A4XX_TEX_CONST_1_HEIGHT(bufs[i]->height));
         OUT_RING(ring, A4XX_TEX_CONST_2_PITCH(fd_resource_pitch(rsc, lvl)));
         OUT_RING(ring, 0x00000000);
         OUT_RELOC(ring, rsc->bo, offset, 0, 0);
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
      } else {
         OUT_RING(ring, A4XX_TEX_CONST_0_FMT(0) |
                           A4XX_TEX_CONST_0_TYPE(A4XX_TEX_2D) |
                           A4XX_TEX_CONST_0_SWIZ_X(A4XX_TEX_ONE) |
                           A4XX_TEX_CONST_0_SWIZ_Y(A4XX_TEX_ONE) |
                           A4XX_TEX_CONST_0_SWIZ_Z(A4XX_TEX_ONE) |
                           A4XX_TEX_CONST_0_SWIZ_W(A4XX_TEX_ONE));
         OUT_RING(ring, A4XX_TEX_CONST_1_WIDTH(0) | A4XX_TEX_CONST_1_HEIGHT(0));
         OUT_RING(ring, A4XX_TEX_CONST_2_PITCH(0));
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
         OUT_RING(ring, 0x00000000);
      }
   }

   OUT_PKT0(ring, REG_A4XX_RB_RENDER_COMPONENTS, 1);
   OUT_RING(ring, A4XX_RB_RENDER_COMPONENTS_RT0(mrt_comp[0]) |
                     A4XX_RB_RENDER_COMPONENTS_RT1(mrt_comp[1]) |
                     A4XX_RB_RENDER_COMPONENTS_RT2(mrt_comp[2]) |
                     A4XX_RB_RENDER_COMPONENTS_RT3(mrt_comp[3]) |
                     A4XX_RB_RENDER_COMPONENTS_RT4(mrt_comp[4]) |
                     A4XX_RB_RENDER_COMPONENTS_RT5(mrt_comp[5]) |
                     A4XX_RB_RENDER_COMPONENTS_RT6(mrt_comp[6]) |
                     A4XX_RB_RENDER_COMPONENTS_RT7(mrt_comp[7]));
}

static void
emit_ssbos(struct fd_context *ctx, struct fd_ringbuffer *ring,
      enum a4xx_state_block sb, struct fd_shaderbuf_stateobj *so)
{
   unsigned count = util_last_bit(so->enabled_mask);

   if (count == 0)
      return;

   OUT_PKT3(ring, CP_LOAD_STATE4, 2 + (4 * count));
   OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(0) |
         CP_LOAD_STATE4_0_STATE_SRC(SS4_DIRECT) |
         CP_LOAD_STATE4_0_STATE_BLOCK(sb) |
         CP_LOAD_STATE4_0_NUM_UNIT(count));
   OUT_RING(ring, CP_LOAD_STATE4_1_STATE_TYPE(0) |
         CP_LOAD_STATE4_1_EXT_SRC_ADDR(0));
   for (unsigned i = 0; i < count; i++) {
      struct pipe_shader_buffer *buf = &so->sb[i];
      if (buf->buffer) {
         struct fd_resource *rsc = fd_resource(buf->buffer);
         OUT_RELOC(ring, rsc->bo, buf->buffer_offset, 0, 0);
      } else {
         OUT_RING(ring, 0x00000000);
      }
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, 0x00000000);
   }

   OUT_PKT3(ring, CP_LOAD_STATE4, 2 + (2 * count));
   OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(0) |
         CP_LOAD_STATE4_0_STATE_SRC(SS4_DIRECT) |
         CP_LOAD_STATE4_0_STATE_BLOCK(sb) |
         CP_LOAD_STATE4_0_NUM_UNIT(count));
   OUT_RING(ring, CP_LOAD_STATE4_1_STATE_TYPE(1) |
         CP_LOAD_STATE4_1_EXT_SRC_ADDR(0));
   for (unsigned i = 0; i < count; i++) {
      struct pipe_shader_buffer *buf = &so->sb[i];
      unsigned sz = buf->buffer_size;

      /* width is in dwords, overflows into height: */
      sz /= 4;

      OUT_RING(ring, A4XX_SSBO_1_0_WIDTH(sz));
      OUT_RING(ring, A4XX_SSBO_1_1_HEIGHT(sz >> 16));
   }
}

void
fd4_emit_vertex_bufs(struct fd_ringbuffer *ring, struct fd4_emit *emit)
{
   int32_t i, j, last = -1;
   uint32_t total_in = 0;
   const struct fd_vertex_state *vtx = emit->vtx;
   const struct ir3_shader_variant *vp = fd4_emit_get_vp(emit);
   unsigned vertex_regid = regid(63, 0);
   unsigned instance_regid = regid(63, 0);
   unsigned vtxcnt_regid = regid(63, 0);

   /* Note that sysvals come *after* normal inputs: */
   for (i = 0; i < vp->inputs_count; i++) {
      if (!vp->inputs[i].compmask)
         continue;
      if (vp->inputs[i].sysval) {
         switch (vp->inputs[i].slot) {
         case SYSTEM_VALUE_VERTEX_ID_ZERO_BASE:
            vertex_regid = vp->inputs[i].regid;
            break;
         case SYSTEM_VALUE_INSTANCE_ID:
            instance_regid = vp->inputs[i].regid;
            break;
         case SYSTEM_VALUE_VERTEX_CNT:
            vtxcnt_regid = vp->inputs[i].regid;
            break;
         default:
            unreachable("invalid system value");
            break;
         }
      } else if (i < vtx->vtx->num_elements) {
         last = i;
      }
   }

   for (i = 0, j = 0; i <= last; i++) {
      assert(!vp->inputs[i].sysval);
      if (vp->inputs[i].compmask) {
         struct pipe_vertex_element *elem = &vtx->vtx->pipe[i];
         const struct pipe_vertex_buffer *vb =
            &vtx->vertexbuf.vb[elem->vertex_buffer_index];
         struct fd_resource *rsc = fd_resource(vb->buffer.resource);
         enum pipe_format pfmt = elem->src_format;
         enum a4xx_vtx_fmt fmt = fd4_pipe2vtx(pfmt);
         bool switchnext = (i != last) || (vertex_regid != regid(63, 0)) ||
                           (instance_regid != regid(63, 0)) ||
                           (vtxcnt_regid != regid(63, 0));
         bool isint = util_format_is_pure_integer(pfmt);
         uint32_t fs = util_format_get_blocksize(pfmt);
         uint32_t off = vb->buffer_offset + elem->src_offset;
         uint32_t size = vb->buffer.resource->width0 - off;
         assert(fmt != VFMT4_NONE);

         OUT_PKT0(ring, REG_A4XX_VFD_FETCH(j), 4);
         OUT_RING(ring, A4XX_VFD_FETCH_INSTR_0_FETCHSIZE(fs - 1) |
                           A4XX_VFD_FETCH_INSTR_0_BUFSTRIDE(elem->src_stride) |
                           COND(elem->instance_divisor,
                                A4XX_VFD_FETCH_INSTR_0_INSTANCED) |
                           COND(switchnext, A4XX_VFD_FETCH_INSTR_0_SWITCHNEXT));
         OUT_RELOC(ring, rsc->bo, off, 0, 0);
         OUT_RING(ring, A4XX_VFD_FETCH_INSTR_2_SIZE(size));
         OUT_RING(ring, A4XX_VFD_FETCH_INSTR_3_STEPRATE(
                           MAX2(1, elem->instance_divisor)));

         OUT_PKT0(ring, REG_A4XX_VFD_DECODE_INSTR(j), 1);
         OUT_RING(ring,
                  A4XX_VFD_DECODE_INSTR_CONSTFILL |
                     A4XX_VFD_DECODE_INSTR_WRITEMASK(vp->inputs[i].compmask) |
                     A4XX_VFD_DECODE_INSTR_FORMAT(fmt) |
                     A4XX_VFD_DECODE_INSTR_SWAP(fd4_pipe2swap(pfmt)) |
                     A4XX_VFD_DECODE_INSTR_REGID(vp->inputs[i].regid) |
                     A4XX_VFD_DECODE_INSTR_SHIFTCNT(fs) |
                     A4XX_VFD_DECODE_INSTR_LASTCOMPVALID |
                     COND(isint, A4XX_VFD_DECODE_INSTR_INT) |
                     COND(switchnext, A4XX_VFD_DECODE_INSTR_SWITCHNEXT));

         total_in += util_bitcount(vp->inputs[i].compmask);
         j++;
      }
   }

   /* hw doesn't like to be configured for zero vbo's, it seems: */
   if (last < 0) {
      /* just recycle the shader bo, we just need to point to *something*
       * valid:
       */
      struct fd_bo *dummy_vbo = vp->bo;
      bool switchnext = (vertex_regid != regid(63, 0)) ||
                        (instance_regid != regid(63, 0)) ||
                        (vtxcnt_regid != regid(63, 0));

      OUT_PKT0(ring, REG_A4XX_VFD_FETCH(0), 4);
      OUT_RING(ring, A4XX_VFD_FETCH_INSTR_0_FETCHSIZE(0) |
                        A4XX_VFD_FETCH_INSTR_0_BUFSTRIDE(0) |
                        COND(switchnext, A4XX_VFD_FETCH_INSTR_0_SWITCHNEXT));
      OUT_RELOC(ring, dummy_vbo, 0, 0, 0);
      OUT_RING(ring, A4XX_VFD_FETCH_INSTR_2_SIZE(1));
      OUT_RING(ring, A4XX_VFD_FETCH_INSTR_3_STEPRATE(1));

      OUT_PKT0(ring, REG_A4XX_VFD_DECODE_INSTR(0), 1);
      OUT_RING(ring, A4XX_VFD_DECODE_INSTR_CONSTFILL |
                        A4XX_VFD_DECODE_INSTR_WRITEMASK(0x1) |
                        A4XX_VFD_DECODE_INSTR_FORMAT(VFMT4_8_UNORM) |
                        A4XX_VFD_DECODE_INSTR_SWAP(XYZW) |
                        A4XX_VFD_DECODE_INSTR_REGID(regid(0, 0)) |
                        A4XX_VFD_DECODE_INSTR_SHIFTCNT(1) |
                        A4XX_VFD_DECODE_INSTR_LASTCOMPVALID |
                        COND(switchnext, A4XX_VFD_DECODE_INSTR_SWITCHNEXT));

      total_in = 1;
      j = 1;
   }

   OUT_PKT0(ring, REG_A4XX_VFD_CONTROL_0, 5);
   OUT_RING(ring, A4XX_VFD_CONTROL_0_TOTALATTRTOVS(total_in) |
                     0xa0000 | /* XXX */
                     A4XX_VFD_CONTROL_0_STRMDECINSTRCNT(j) |
                     A4XX_VFD_CONTROL_0_STRMFETCHINSTRCNT(j));
   OUT_RING(ring, A4XX_VFD_CONTROL_1_MAXSTORAGE(129) | // XXX
                     A4XX_VFD_CONTROL_1_REGID4VTX(vertex_regid) |
                     A4XX_VFD_CONTROL_1_REGID4INST(instance_regid));
   OUT_RING(ring, 0x00000000); /* XXX VFD_CONTROL_2 */
   OUT_RING(ring, A4XX_VFD_CONTROL_3_REGID_VTXCNT(vtxcnt_regid));
   OUT_RING(ring, 0x00000000); /* XXX VFD_CONTROL_4 */

   /* cache invalidate, otherwise vertex fetch could see
    * stale vbo contents:
    */
   OUT_PKT0(ring, REG_A4XX_UCHE_INVALIDATE0, 2);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, 0x00000012);
}

void
fd4_emit_state(struct fd_context *ctx, struct fd_ringbuffer *ring,
               struct fd4_emit *emit)
{
   const struct ir3_shader_variant *vp = fd4_emit_get_vp(emit);
   const struct ir3_shader_variant *fp = fd4_emit_get_fp(emit);
   const enum fd_dirty_3d_state dirty = emit->dirty;

   emit_marker(ring, 5);

   if ((dirty & FD_DIRTY_FRAMEBUFFER) && !emit->binning_pass) {
      struct pipe_framebuffer_state *pfb = &ctx->batch->framebuffer;
      unsigned char mrt_comp[A4XX_MAX_RENDER_TARGETS] = {0};

      for (unsigned i = 0; i < A4XX_MAX_RENDER_TARGETS; i++) {
         mrt_comp[i] = ((i < pfb->nr_cbufs) && pfb->cbufs[i]) ? 0xf : 0;
      }

      OUT_PKT0(ring, REG_A4XX_RB_RENDER_COMPONENTS, 1);
      OUT_RING(ring, A4XX_RB_RENDER_COMPONENTS_RT0(mrt_comp[0]) |
                        A4XX_RB_RENDER_COMPONENTS_RT1(mrt_comp[1]) |
                        A4XX_RB_RENDER_COMPONENTS_RT2(mrt_comp[2]) |
                        A4XX_RB_RENDER_COMPONENTS_RT3(mrt_comp[3]) |
                        A4XX_RB_RENDER_COMPONENTS_RT4(mrt_comp[4]) |
                        A4XX_RB_RENDER_COMPONENTS_RT5(mrt_comp[5]) |
                        A4XX_RB_RENDER_COMPONENTS_RT6(mrt_comp[6]) |
                        A4XX_RB_RENDER_COMPONENTS_RT7(mrt_comp[7]));
   }

   if (dirty & (FD_DIRTY_ZSA | FD_DIRTY_FRAMEBUFFER)) {
      struct fd4_zsa_stateobj *zsa = fd4_zsa_stateobj(ctx->zsa);
      struct pipe_framebuffer_state *pfb = &ctx->batch->framebuffer;
      uint32_t rb_alpha_control = zsa->rb_alpha_control;

      if (util_format_is_pure_integer(pipe_surface_format(pfb->cbufs[0])))
         rb_alpha_control &= ~A4XX_RB_ALPHA_CONTROL_ALPHA_TEST;

      OUT_PKT0(ring, REG_A4XX_RB_ALPHA_CONTROL, 1);
      OUT_RING(ring, rb_alpha_control);

      OUT_PKT0(ring, REG_A4XX_RB_STENCIL_CONTROL, 2);
      OUT_RING(ring, zsa->rb_stencil_control);
      OUT_RING(ring, zsa->rb_stencil_control2);
   }

   if (dirty & (FD_DIRTY_ZSA | FD_DIRTY_STENCIL_REF)) {
      struct fd4_zsa_stateobj *zsa = fd4_zsa_stateobj(ctx->zsa);
      struct pipe_stencil_ref *sr = &ctx->stencil_ref;

      OUT_PKT0(ring, REG_A4XX_RB_STENCILREFMASK, 2);
      OUT_RING(ring, zsa->rb_stencilrefmask |
                        A4XX_RB_STENCILREFMASK_STENCILREF(sr->ref_value[0]));
      OUT_RING(ring, zsa->rb_stencilrefmask_bf |
                        A4XX_RB_STENCILREFMASK_BF_STENCILREF(sr->ref_value[1]));
   }

   if (dirty & (FD_DIRTY_ZSA | FD_DIRTY_RASTERIZER | FD_DIRTY_PROG)) {
      struct fd4_zsa_stateobj *zsa = fd4_zsa_stateobj(ctx->zsa);
      bool fragz = fp->no_earlyz || fp->has_kill || fp->writes_pos;
      bool latez = !fp->fs.early_fragment_tests && fragz;
      bool clamp = !ctx->rasterizer->depth_clip_near;

      OUT_PKT0(ring, REG_A4XX_RB_DEPTH_CONTROL, 1);
      OUT_RING(ring, zsa->rb_depth_control |
                        COND(clamp, A4XX_RB_DEPTH_CONTROL_Z_CLAMP_ENABLE) |
                        COND(latez, A4XX_RB_DEPTH_CONTROL_EARLY_Z_DISABLE) |
                        COND(fragz && fp->fragcoord_compmask != 0,
                             A4XX_RB_DEPTH_CONTROL_FORCE_FRAGZ_TO_FS));

      /* maybe this register/bitfield needs a better name.. this
       * appears to be just disabling early-z
       */
      OUT_PKT0(ring, REG_A4XX_GRAS_ALPHA_CONTROL, 1);
      OUT_RING(ring, zsa->gras_alpha_control |
                        COND(latez, A4XX_GRAS_ALPHA_CONTROL_ALPHA_TEST_ENABLE) |
                        COND(fragz && fp->fragcoord_compmask != 0,
                             A4XX_GRAS_ALPHA_CONTROL_FORCE_FRAGZ_TO_FS));
   }

   if (dirty & FD_DIRTY_RASTERIZER) {
      struct fd4_rasterizer_stateobj *rasterizer =
         fd4_rasterizer_stateobj(ctx->rasterizer);

      OUT_PKT0(ring, REG_A4XX_GRAS_SU_MODE_CONTROL, 1);
      OUT_RING(ring, rasterizer->gras_su_mode_control |
                        A4XX_GRAS_SU_MODE_CONTROL_RENDERING_PASS);

      OUT_PKT0(ring, REG_A4XX_GRAS_SU_POINT_MINMAX, 2);
      OUT_RING(ring, rasterizer->gras_su_point_minmax);
      OUT_RING(ring, rasterizer->gras_su_point_size);

      OUT_PKT0(ring, REG_A4XX_GRAS_SU_POLY_OFFSET_SCALE, 3);
      OUT_RING(ring, rasterizer->gras_su_poly_offset_scale);
      OUT_RING(ring, rasterizer->gras_su_poly_offset_offset);
      OUT_RING(ring, rasterizer->gras_su_poly_offset_clamp);

      OUT_PKT0(ring, REG_A4XX_GRAS_CL_CLIP_CNTL, 1);
      OUT_RING(ring, rasterizer->gras_cl_clip_cntl);
   }

   /* NOTE: since primitive_restart is not actually part of any
    * state object, we need to make sure that we always emit
    * PRIM_VTX_CNTL.. either that or be more clever and detect
    * when it changes.
    */
   if (emit->info) {
      const struct pipe_draw_info *info = emit->info;
      struct fd4_rasterizer_stateobj *rast =
         fd4_rasterizer_stateobj(ctx->rasterizer);
      uint32_t val = rast->pc_prim_vtx_cntl;

      if (info->index_size && info->primitive_restart)
         val |= A4XX_PC_PRIM_VTX_CNTL_PRIMITIVE_RESTART;

      val |= COND(vp->writes_psize, A4XX_PC_PRIM_VTX_CNTL_PSIZE);

      if (fp->total_in > 0) {
         uint32_t varout = align(fp->total_in, 16) / 16;
         if (varout > 1)
            varout = align(varout, 2);
         val |= A4XX_PC_PRIM_VTX_CNTL_VAROUT(varout);
      }

      OUT_PKT0(ring, REG_A4XX_PC_PRIM_VTX_CNTL, 2);
      OUT_RING(ring, val);
      OUT_RING(ring, rast->pc_prim_vtx_cntl2);
   }

   /* NOTE: scissor enabled bit is part of rasterizer state: */
   if (dirty & (FD_DIRTY_SCISSOR | FD_DIRTY_RASTERIZER)) {
      struct pipe_scissor_state *scissor = fd_context_get_scissor(ctx);

      OUT_PKT0(ring, REG_A4XX_GRAS_SC_WINDOW_SCISSOR_BR, 2);
      OUT_RING(ring, A4XX_GRAS_SC_WINDOW_SCISSOR_BR_X(scissor->maxx) |
                        A4XX_GRAS_SC_WINDOW_SCISSOR_BR_Y(scissor->maxy));
      OUT_RING(ring, A4XX_GRAS_SC_WINDOW_SCISSOR_TL_X(scissor->minx) |
                        A4XX_GRAS_SC_WINDOW_SCISSOR_TL_Y(scissor->miny));

      ctx->batch->max_scissor.minx =
         MIN2(ctx->batch->max_scissor.minx, scissor->minx);
      ctx->batch->max_scissor.miny =
         MIN2(ctx->batch->max_scissor.miny, scissor->miny);
      ctx->batch->max_scissor.maxx =
         MAX2(ctx->batch->max_scissor.maxx, scissor->maxx);
      ctx->batch->max_scissor.maxy =
         MAX2(ctx->batch->max_scissor.maxy, scissor->maxy);
   }

   if (dirty & FD_DIRTY_VIEWPORT) {
      struct pipe_viewport_state *vp = & ctx->viewport[0];

      fd_wfi(ctx->batch, ring);

      OUT_PKT0(ring, REG_A4XX_GRAS_CL_VPORT_XOFFSET_0, 6);
      OUT_RING(ring, A4XX_GRAS_CL_VPORT_XOFFSET_0(vp->translate[0]));
      OUT_RING(ring, A4XX_GRAS_CL_VPORT_XSCALE_0(vp->scale[0]));
      OUT_RING(ring, A4XX_GRAS_CL_VPORT_YOFFSET_0(vp->translate[1]));
      OUT_RING(ring, A4XX_GRAS_CL_VPORT_YSCALE_0(vp->scale[1]));
      OUT_RING(ring, A4XX_GRAS_CL_VPORT_ZOFFSET_0(vp->translate[2]));
      OUT_RING(ring, A4XX_GRAS_CL_VPORT_ZSCALE_0(vp->scale[2]));
   }

   if (dirty &
       (FD_DIRTY_VIEWPORT | FD_DIRTY_RASTERIZER | FD_DIRTY_FRAMEBUFFER)) {
      float zmin, zmax;
      int depth = 24;
      if (ctx->batch->framebuffer.zsbuf) {
         depth = util_format_get_component_bits(
            pipe_surface_format(ctx->batch->framebuffer.zsbuf),
            UTIL_FORMAT_COLORSPACE_ZS, 0);
      }
      util_viewport_zmin_zmax(&ctx->viewport[0], ctx->rasterizer->clip_halfz,
                              &zmin, &zmax);

      OUT_PKT0(ring, REG_A4XX_RB_VPORT_Z_CLAMP(0), 2);
      if (depth == 32) {
         OUT_RING(ring, fui(zmin));
         OUT_RING(ring, fui(zmax));
      } else if (depth == 16) {
         OUT_RING(ring, (uint32_t)(zmin * 0xffff));
         OUT_RING(ring, (uint32_t)(zmax * 0xffff));
      } else {
         OUT_RING(ring, (uint32_t)(zmin * 0xffffff));
         OUT_RING(ring, (uint32_t)(zmax * 0xffffff));
      }
   }

   if (dirty & (FD_DIRTY_PROG | FD_DIRTY_FRAMEBUFFER)) {
      struct pipe_framebuffer_state *pfb = &ctx->batch->framebuffer;
      unsigned n = pfb->nr_cbufs;
      /* if we have depth/stencil, we need at least on MRT: */
      if (pfb->zsbuf)
         n = MAX2(1, n);
      fd4_program_emit(ring, emit, n, pfb->cbufs);
   }

   if (!emit->skip_consts) { /* evil hack to deal sanely with clear path */
      ir3_emit_vs_consts(vp, ring, ctx, emit->info, emit->indirect, emit->draw);
      if (!emit->binning_pass)
         ir3_emit_fs_consts(fp, ring, ctx);
   }

   if ((dirty & FD_DIRTY_BLEND)) {
      struct fd4_blend_stateobj *blend = fd4_blend_stateobj(ctx->blend);
      uint32_t i;

      for (i = 0; i < A4XX_MAX_RENDER_TARGETS; i++) {
         enum pipe_format format =
            pipe_surface_format(ctx->batch->framebuffer.cbufs[i]);
         bool is_int = util_format_is_pure_integer(format);
         bool has_alpha = util_format_has_alpha(format);
         uint32_t control = blend->rb_mrt[i].control;

         if (is_int) {
            control &= A4XX_RB_MRT_CONTROL_COMPONENT_ENABLE__MASK;
            control |= A4XX_RB_MRT_CONTROL_ROP_CODE(ROP_COPY);
         }

         if (!has_alpha) {
            control &= ~A4XX_RB_MRT_CONTROL_BLEND2;
         }

         OUT_PKT0(ring, REG_A4XX_RB_MRT_CONTROL(i), 1);
         OUT_RING(ring, control);

         OUT_PKT0(ring, REG_A4XX_RB_MRT_BLEND_CONTROL(i), 1);
         OUT_RING(ring, blend->rb_mrt[i].blend_control);
      }

      OUT_PKT0(ring, REG_A4XX_RB_FS_OUTPUT, 1);
      OUT_RING(ring,
               blend->rb_fs_output | A4XX_RB_FS_OUTPUT_SAMPLE_MASK(0xffff));
   }

   if (dirty & FD_DIRTY_BLEND_COLOR) {
      struct pipe_blend_color *bcolor = &ctx->blend_color;

      OUT_PKT0(ring, REG_A4XX_RB_BLEND_RED, 8);
      OUT_RING(ring, A4XX_RB_BLEND_RED_FLOAT(bcolor->color[0]) |
                        A4XX_RB_BLEND_RED_UINT(CLAMP(bcolor->color[0], 0.f, 1.f) * 0xff) |
                        A4XX_RB_BLEND_RED_SINT(CLAMP(bcolor->color[0], -1.f, 1.f) * 0x7f));
      OUT_RING(ring, A4XX_RB_BLEND_RED_F32(bcolor->color[0]));
      OUT_RING(ring, A4XX_RB_BLEND_GREEN_FLOAT(bcolor->color[1]) |
                        A4XX_RB_BLEND_GREEN_UINT(CLAMP(bcolor->color[1], 0.f, 1.f) * 0xff) |
                        A4XX_RB_BLEND_GREEN_SINT(CLAMP(bcolor->color[1], -1.f, 1.f) * 0x7f));
      OUT_RING(ring, A4XX_RB_BLEND_GREEN_F32(bcolor->color[1]));
      OUT_RING(ring, A4XX_RB_BLEND_BLUE_FLOAT(bcolor->color[2]) |
                        A4XX_RB_BLEND_BLUE_UINT(CLAMP(bcolor->color[2], 0.f, 1.f) * 0xff) |
                        A4XX_RB_BLEND_BLUE_SINT(CLAMP(bcolor->color[2], -1.f, 1.f) * 0x7f));
      OUT_RING(ring, A4XX_RB_BLEND_BLUE_F32(bcolor->color[2]));
      OUT_RING(ring, A4XX_RB_BLEND_ALPHA_FLOAT(bcolor->color[3]) |
                        A4XX_RB_BLEND_ALPHA_UINT(CLAMP(bcolor->color[3], 0.f, 1.f) * 0xff) |
                        A4XX_RB_BLEND_ALPHA_SINT(CLAMP(bcolor->color[3], -1.f, 1.f) * 0x7f));
      OUT_RING(ring, A4XX_RB_BLEND_ALPHA_F32(bcolor->color[3]));
   }

   if (ctx->dirty_shader[PIPE_SHADER_VERTEX] & FD_DIRTY_SHADER_TEX)
      emit_textures(ctx, ring, SB4_VS_TEX, &ctx->tex[PIPE_SHADER_VERTEX], vp);

   if (ctx->dirty_shader[PIPE_SHADER_FRAGMENT] & FD_DIRTY_SHADER_TEX)
      emit_textures(ctx, ring, SB4_FS_TEX, &ctx->tex[PIPE_SHADER_FRAGMENT], fp);

   if (!emit->binning_pass) {
      if (ctx->dirty_shader[PIPE_SHADER_FRAGMENT] & FD_DIRTY_SHADER_SSBO)
         emit_ssbos(ctx, ring, SB4_SSBO, &ctx->shaderbuf[PIPE_SHADER_FRAGMENT]);

      if (ctx->dirty_shader[PIPE_SHADER_FRAGMENT] & FD_DIRTY_SHADER_IMAGE)
         fd4_emit_images(ctx, ring, PIPE_SHADER_FRAGMENT, fp);
   }
}

void
fd4_emit_cs_state(struct fd_context *ctx, struct fd_ringbuffer *ring,
                  struct ir3_shader_variant *cp)
{
   enum fd_dirty_shader_state dirty = ctx->dirty_shader[PIPE_SHADER_COMPUTE];
   unsigned num_textures = ctx->tex[PIPE_SHADER_COMPUTE].num_textures +
      cp->astc_srgb.count +
      cp->tg4.count;

   if (dirty & FD_DIRTY_SHADER_TEX) {
      emit_textures(ctx, ring, SB4_CS_TEX, &ctx->tex[PIPE_SHADER_COMPUTE], cp);

      OUT_PKT0(ring, REG_A4XX_TPL1_TP_TEX_COUNT, 1);
      OUT_RING(ring, 0);
   }

   OUT_PKT0(ring, REG_A4XX_TPL1_TP_FS_TEX_COUNT, 1);
   OUT_RING(ring, A4XX_TPL1_TP_FS_TEX_COUNT_CS(
               ctx->shaderimg[PIPE_SHADER_COMPUTE].enabled_mask ? 0x80 : num_textures));

   if (dirty & FD_DIRTY_SHADER_SSBO)
      emit_ssbos(ctx, ring, SB4_CS_SSBO, &ctx->shaderbuf[PIPE_SHADER_COMPUTE]);

   if (dirty & FD_DIRTY_SHADER_IMAGE)
      fd4_emit_images(ctx, ring, PIPE_SHADER_COMPUTE, cp);
}

/* emit setup at begin of new cmdstream buffer (don't rely on previous
 * state, there could have been a context switch between ioctls):
 */
void
fd4_emit_restore(struct fd_batch *batch, struct fd_ringbuffer *ring)
{
   struct fd_context *ctx = batch->ctx;
   struct fd4_context *fd4_ctx = fd4_context(ctx);

   OUT_PKT0(ring, REG_A4XX_RBBM_PERFCTR_CTL, 1);
   OUT_RING(ring, 0x00000001);

   OUT_PKT0(ring, REG_A4XX_GRAS_DEBUG_ECO_CONTROL, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_SP_MODE_CONTROL, 1);
   OUT_RING(ring, 0x0000001e);

   OUT_PKT0(ring, REG_A4XX_TPL1_TP_MODE_CONTROL, 1);
   OUT_RING(ring, 0x0000003a);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_0D01, 1);
   OUT_RING(ring, 0x00000001);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_0E42, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_UCHE_CACHE_WAYS_VFD, 1);
   OUT_RING(ring, 0x00000007);

   OUT_PKT0(ring, REG_A4XX_UCHE_CACHE_MODE_CONTROL, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_UCHE_INVALIDATE0, 2);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, 0x00000012);

   OUT_PKT0(ring, REG_A4XX_HLSQ_MODE_CONTROL, 1);
   OUT_RING(ring, 0x00000003);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_0CC5, 1);
   OUT_RING(ring, 0x00000006);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_0CC6, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_0EC2, 1);
   OUT_RING(ring, 0x00040000);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_2001, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT3(ring, CP_INVALIDATE_STATE, 1);
   OUT_RING(ring, 0x00001000);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_20EF, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_RB_BLEND_RED, 4);
   OUT_RING(ring, A4XX_RB_BLEND_RED_UINT(0) | A4XX_RB_BLEND_RED_FLOAT(0.0f));
   OUT_RING(ring, A4XX_RB_BLEND_GREEN_UINT(0) | A4XX_RB_BLEND_GREEN_FLOAT(0.0f));
   OUT_RING(ring, A4XX_RB_BLEND_BLUE_UINT(0) | A4XX_RB_BLEND_BLUE_FLOAT(0.0f));
   OUT_RING(ring,
            A4XX_RB_BLEND_ALPHA_UINT(0x7fff) | A4XX_RB_BLEND_ALPHA_FLOAT(1.0f));

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_2152, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_2153, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_2154, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_2155, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_2156, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_2157, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_21C3, 1);
   OUT_RING(ring, 0x0000001d);

   OUT_PKT0(ring, REG_A4XX_PC_GS_PARAM, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_21E6, 1);
   OUT_RING(ring, 0x00000001);

   OUT_PKT0(ring, REG_A4XX_PC_HS_PARAM, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_UNKNOWN_22D7, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_TPL1_TP_TEX_OFFSET, 1);
   OUT_RING(ring, 0x00000000);

   OUT_PKT0(ring, REG_A4XX_TPL1_TP_TEX_COUNT, 1);
   OUT_RING(ring, A4XX_TPL1_TP_TEX_COUNT_VS(16) | A4XX_TPL1_TP_TEX_COUNT_HS(0) |
                     A4XX_TPL1_TP_TEX_COUNT_DS(0) |
                     A4XX_TPL1_TP_TEX_COUNT_GS(0));

   OUT_PKT0(ring, REG_A4XX_TPL1_TP_FS_TEX_COUNT, 1);
   OUT_RING(ring, 16);

   /* we don't use this yet.. probably best to disable.. */
   OUT_PKT3(ring, CP_SET_DRAW_STATE, 2);
   OUT_RING(ring, CP_SET_DRAW_STATE__0_COUNT(0) |
                     CP_SET_DRAW_STATE__0_DISABLE_ALL_GROUPS |
                     CP_SET_DRAW_STATE__0_GROUP_ID(0));
   OUT_RING(ring, CP_SET_DRAW_STATE__1_ADDR_LO(0));

   OUT_PKT0(ring, REG_A4XX_SP_VS_PVT_MEM_PARAM, 2);
   OUT_RING(ring, 0x08000001);                    /* SP_VS_PVT_MEM_PARAM */
   OUT_RELOC(ring, fd4_ctx->vs_pvt_mem, 0, 0, 0); /* SP_VS_PVT_MEM_ADDR */

   OUT_PKT0(ring, REG_A4XX_SP_FS_PVT_MEM_PARAM, 2);
   OUT_RING(ring, 0x08000001);                    /* SP_FS_PVT_MEM_PARAM */
   OUT_RELOC(ring, fd4_ctx->fs_pvt_mem, 0, 0, 0); /* SP_FS_PVT_MEM_ADDR */

   OUT_PKT0(ring, REG_A4XX_GRAS_SC_CONTROL, 1);
   OUT_RING(ring, A4XX_GRAS_SC_CONTROL_RENDER_MODE(RB_RENDERING_PASS) |
                     A4XX_GRAS_SC_CONTROL_MSAA_DISABLE |
                     A4XX_GRAS_SC_CONTROL_MSAA_SAMPLES(MSAA_ONE) |
                     A4XX_GRAS_SC_CONTROL_RASTER_MODE(0));

   OUT_PKT0(ring, REG_A4XX_RB_MSAA_CONTROL, 1);
   OUT_RING(ring, A4XX_RB_MSAA_CONTROL_DISABLE |
                     A4XX_RB_MSAA_CONTROL_SAMPLES(MSAA_ONE));

   OUT_PKT0(ring, REG_A4XX_GRAS_CL_GB_CLIP_ADJ, 1);
   OUT_RING(ring, A4XX_GRAS_CL_GB_CLIP_ADJ_HORZ(0) |
                     A4XX_GRAS_CL_GB_CLIP_ADJ_VERT(0));

   OUT_PKT0(ring, REG_A4XX_RB_ALPHA_CONTROL, 1);
   OUT_RING(ring, A4XX_RB_ALPHA_CONTROL_ALPHA_TEST_FUNC(FUNC_ALWAYS));

   OUT_PKT0(ring, REG_A4XX_RB_FS_OUTPUT, 1);
   OUT_RING(ring, A4XX_RB_FS_OUTPUT_SAMPLE_MASK(0xffff));

   OUT_PKT0(ring, REG_A4XX_GRAS_ALPHA_CONTROL, 1);
   OUT_RING(ring, 0x0);

   fd_hw_query_enable(batch, ring);
}

static void
fd4_mem_to_mem(struct fd_ringbuffer *ring, struct pipe_resource *dst,
               unsigned dst_off, struct pipe_resource *src, unsigned src_off,
               unsigned sizedwords)
{
   struct fd_bo *src_bo = fd_resource(src)->bo;
   struct fd_bo *dst_bo = fd_resource(dst)->bo;
   unsigned i;

   for (i = 0; i < sizedwords; i++) {
      OUT_PKT3(ring, CP_MEM_TO_MEM, 3);
      OUT_RING(ring, 0x00000000);
      OUT_RELOC(ring, dst_bo, dst_off, 0, 0);
      OUT_RELOC(ring, src_bo, src_off, 0, 0);

      dst_off += 4;
      src_off += 4;
   }
}

void
fd4_emit_init_screen(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);

   screen->emit_ib = fd4_emit_ib;
   screen->mem_to_mem = fd4_mem_to_mem;
}

void
fd4_emit_init(struct pipe_context *pctx)
{
}
