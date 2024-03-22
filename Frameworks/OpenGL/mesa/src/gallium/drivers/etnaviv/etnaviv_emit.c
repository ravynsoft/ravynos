/*
 * Copyright (c) 2014-2015 Etnaviv Project
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
 * Authors:
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 */

#include "etnaviv_emit.h"

#include "etnaviv_blend.h"
#include "etnaviv_compiler.h"
#include "etnaviv_context.h"
#include "etnaviv_rasterizer.h"
#include "etnaviv_resource.h"
#include "etnaviv_rs.h"
#include "etnaviv_screen.h"
#include "etnaviv_shader.h"
#include "etnaviv_texture.h"
#include "etnaviv_translate.h"
#include "etnaviv_uniforms.h"
#include "etnaviv_util.h"
#include "etnaviv_zsa.h"
#include "hw/common.xml.h"
#include "hw/state.xml.h"
#include "hw/state_blt.xml.h"
#include "util/u_math.h"

/* Queue a STALL command (queues 2 words) */
static inline void
CMD_STALL(struct etna_cmd_stream *stream, uint32_t from, uint32_t to)
{
   etna_cmd_stream_emit(stream, VIV_FE_STALL_HEADER_OP_STALL);
   etna_cmd_stream_emit(stream, VIV_FE_STALL_TOKEN_FROM(from) | VIV_FE_STALL_TOKEN_TO(to));
}

void
etna_stall(struct etna_cmd_stream *stream, uint32_t from, uint32_t to)
{
   bool blt = (from == SYNC_RECIPIENT_BLT) || (to == SYNC_RECIPIENT_BLT);
   etna_cmd_stream_reserve(stream, blt ? 8 : 4);

   if (blt) {
      etna_emit_load_state(stream, VIVS_BLT_ENABLE >> 2, 1, 0);
      etna_cmd_stream_emit(stream, 1);
   }

   /* TODO: set bit 28/29 of token after BLT COPY_BUFFER */
   etna_emit_load_state(stream, VIVS_GL_SEMAPHORE_TOKEN >> 2, 1, 0);
   etna_cmd_stream_emit(stream, VIVS_GL_SEMAPHORE_TOKEN_FROM(from) | VIVS_GL_SEMAPHORE_TOKEN_TO(to));

   if (from == SYNC_RECIPIENT_FE) {
      /* if the frontend is to be stalled, queue a STALL frontend command */
      CMD_STALL(stream, from, to);
   } else {
      /* otherwise, load the STALL token state */
      etna_emit_load_state(stream, VIVS_GL_STALL_TOKEN >> 2, 1, 0);
      etna_cmd_stream_emit(stream, VIVS_GL_STALL_TOKEN_FROM(from) | VIVS_GL_STALL_TOKEN_TO(to));
   }

   if (blt) {
      etna_emit_load_state(stream, VIVS_BLT_ENABLE >> 2, 1, 0);
      etna_cmd_stream_emit(stream, 0);
   }
}

#define EMIT_STATE(state_name, src_value) \
   etna_coalsence_emit(stream, &coalesce, VIVS_##state_name, src_value)

#define EMIT_STATE_FIXP(state_name, src_value) \
   etna_coalsence_emit_fixp(stream, &coalesce, VIVS_##state_name, src_value)

#define EMIT_STATE_RELOC(state_name, src_value) \
   etna_coalsence_emit_reloc(stream, &coalesce, VIVS_##state_name, src_value)

#define ETNA_3D_CONTEXT_SIZE  (400) /* keep this number above "Total state updates (fixed)" from gen_weave_state tool */

static unsigned
required_stream_size(struct etna_context *ctx)
{
   unsigned size = ETNA_3D_CONTEXT_SIZE;

   /* stall + flush */
   size += 2 + 4;

   /* vertex elements */
   size += ctx->vertex_elements->num_elements + 1;

   /* uniforms - worst case (2 words per uniform load) */
   size += ctx->shader.vs->uniforms.count * 2;
   size += ctx->shader.fs->uniforms.count * 2;

   /* shader */
   size += ctx->shader_state.vs_inst_mem_size + 1;
   size += ctx->shader_state.ps_inst_mem_size + 1;

   /* DRAW_INDEXED_PRIMITIVES command */
   size += 6;

   /* reserve for alignment etc. */
   size += 64;

   return size;
}

/* Emit state that only exists on HALTI5+ */
static void
emit_halti5_only_state(struct etna_context *ctx, int vs_output_count)
{
   struct etna_cmd_stream *stream = ctx->stream;
   uint32_t dirty = ctx->dirty;
   struct etna_coalesce coalesce;

   etna_coalesce_start(stream, &coalesce);
   if (unlikely(dirty & (ETNA_DIRTY_SHADER))) {
      /* Magic states (load balancing, inter-unit sync, buffers) */
      /*007C4*/ EMIT_STATE(FE_HALTI5_ID_CONFIG, ctx->shader_state.FE_HALTI5_ID_CONFIG);
      /*00870*/ EMIT_STATE(VS_HALTI5_OUTPUT_COUNT, vs_output_count | ((vs_output_count * 0x10) << 8));
      /*008A0*/ EMIT_STATE(VS_HALTI5_UNK008A0, 0x0001000e | ((0x110/vs_output_count) << 20));
      for (int x = 0; x < 4; ++x) {
         /*008E0*/ EMIT_STATE(VS_HALTI5_OUTPUT(x), ctx->shader_state.VS_OUTPUT[x]);
      }
   }
   if (unlikely(dirty & (ETNA_DIRTY_VERTEX_ELEMENTS | ETNA_DIRTY_SHADER))) {
      for (int x = 0; x < 4; ++x) {
         /*008C0*/ EMIT_STATE(VS_HALTI5_INPUT(x), ctx->shader_state.VS_INPUT[x]);
      }
   }
   if (unlikely(dirty & (ETNA_DIRTY_SHADER))) {
      /*00A90*/ EMIT_STATE(PA_VARYING_NUM_COMPONENTS(0), ctx->shader_state.GL_VARYING_NUM_COMPONENTS[0]);
      /*00A94*/ EMIT_STATE(PA_VARYING_NUM_COMPONENTS(1), ctx->shader_state.GL_VARYING_NUM_COMPONENTS[1]);
      /*00AA8*/ EMIT_STATE(PA_VS_OUTPUT_COUNT, vs_output_count);
      /*01080*/ EMIT_STATE(PS_VARYING_NUM_COMPONENTS(0), ctx->shader_state.GL_VARYING_NUM_COMPONENTS[0]);
      /*01084*/ EMIT_STATE(PS_VARYING_NUM_COMPONENTS(1), ctx->shader_state.GL_VARYING_NUM_COMPONENTS[1]);
      /*03888*/ EMIT_STATE(GL_HALTI5_SH_SPECIALS, ctx->shader_state.GL_HALTI5_SH_SPECIALS);
   }
   etna_coalesce_end(stream, &coalesce);
}

/* Emit state that no longer exists on HALTI5 */
static void
emit_pre_halti5_state(struct etna_context *ctx)
{
   struct etna_cmd_stream *stream = ctx->stream;
   uint32_t dirty = ctx->dirty;
   struct etna_coalesce coalesce;

   etna_coalesce_start(stream, &coalesce);
   if (unlikely(dirty & (ETNA_DIRTY_SHADER))) {
      /*00800*/ EMIT_STATE(VS_END_PC, ctx->shader_state.VS_END_PC);
   }
   if (unlikely(dirty & (ETNA_DIRTY_SHADER))) {
      for (int x = 0; x < 4; ++x) {
        /*00810*/ EMIT_STATE(VS_OUTPUT(x), ctx->shader_state.VS_OUTPUT[x]);
      }
   }
   if (unlikely(dirty & (ETNA_DIRTY_VERTEX_ELEMENTS | ETNA_DIRTY_SHADER))) {
      for (int x = 0; x < 4; ++x) {
        /*00820*/ EMIT_STATE(VS_INPUT(x), ctx->shader_state.VS_INPUT[x]);
      }
   }
   if (unlikely(dirty & (ETNA_DIRTY_SHADER))) {
      /*00838*/ EMIT_STATE(VS_START_PC, ctx->shader_state.VS_START_PC);
   }
   if (unlikely(dirty & (ETNA_DIRTY_SHADER))) {
      for (int x = 0; x < 10; ++x) {
         /*00A40*/ EMIT_STATE(PA_SHADER_ATTRIBUTES(x), ctx->shader_state.PA_SHADER_ATTRIBUTES[x]);
      }
   }
   if (unlikely(dirty & (ETNA_DIRTY_FRAMEBUFFER))) {
      /*00E04*/ EMIT_STATE(RA_MULTISAMPLE_UNK00E04, ctx->framebuffer.RA_MULTISAMPLE_UNK00E04);
      for (int x = 0; x < 4; ++x) {
         /*00E10*/ EMIT_STATE(RA_MULTISAMPLE_UNK00E10(x), ctx->framebuffer.RA_MULTISAMPLE_UNK00E10[x]);
      }
      for (int x = 0; x < 16; ++x) {
         /*00E40*/ EMIT_STATE(RA_CENTROID_TABLE(x), ctx->framebuffer.RA_CENTROID_TABLE[x]);
      }
   }
   if (unlikely(dirty & (ETNA_DIRTY_SHADER | ETNA_DIRTY_FRAMEBUFFER))) {
      /*01000*/ EMIT_STATE(PS_END_PC, ctx->shader_state.PS_END_PC);
   }
   if (unlikely(dirty & (ETNA_DIRTY_SHADER | ETNA_DIRTY_FRAMEBUFFER))) {
      /*01018*/ EMIT_STATE(PS_START_PC, ctx->shader_state.PS_START_PC);
   }
   if (unlikely(dirty & (ETNA_DIRTY_SHADER))) {
      /*03820*/ EMIT_STATE(GL_VARYING_NUM_COMPONENTS, ctx->shader_state.GL_VARYING_NUM_COMPONENTS[0]);
      for (int x = 0; x < 2; ++x) {
         /*03828*/ EMIT_STATE(GL_VARYING_COMPONENT_USE(x), ctx->shader_state.GL_VARYING_COMPONENT_USE[x]);
      }
      /*03834*/ EMIT_STATE(GL_VARYING_NUM_COMPONENTS2, ctx->shader_state.GL_VARYING_NUM_COMPONENTS[1]);
   }
   etna_coalesce_end(stream, &coalesce);
}

/* Weave state before draw operation. This function merges all the compiled
 * state blocks under the context into one device register state. Parts of
 * this state that are changed since last call (dirty) will be uploaded as
 * state changes in the command buffer. */
void
etna_emit_state(struct etna_context *ctx)
{
   struct etna_cmd_stream *stream = ctx->stream;
   struct etna_screen *screen = ctx->screen;
   unsigned ccw = ctx->rasterizer->front_ccw;


   /* Pre-reserve the command buffer space which we are likely to need.
    * This must cover all the state emitted below, and the following
    * draw command. */
   etna_cmd_stream_reserve(stream, required_stream_size(ctx));

   uint32_t dirty = ctx->dirty;

   /* Pre-processing: see what caches we need to flush before making state changes. */
   uint32_t to_flush = 0, to_flush_separate = 0;
   if (unlikely(dirty & (ETNA_DIRTY_BLEND)))
      to_flush |= VIVS_GL_FLUSH_CACHE_COLOR;
   if (unlikely(dirty & ETNA_DIRTY_ZSA))
      to_flush |= VIVS_GL_FLUSH_CACHE_DEPTH;
   if (unlikely(dirty & (ETNA_DIRTY_TEXTURE_CACHES))) {
      to_flush |= VIVS_GL_FLUSH_CACHE_TEXTURE;
      to_flush_separate |= VIVS_GL_FLUSH_CACHE_TEXTUREVS;
   }
   if (unlikely(dirty & (ETNA_DIRTY_FRAMEBUFFER))) /* Framebuffer config changed? */
      to_flush |= VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH;
   if (DBG_ENABLED(ETNA_DBG_CFLUSH_ALL)) {
      to_flush |= VIVS_GL_FLUSH_CACHE_TEXTURE | VIVS_GL_FLUSH_CACHE_COLOR |
                  VIVS_GL_FLUSH_CACHE_DEPTH;
      to_flush_separate |= VIVS_GL_FLUSH_CACHE_TEXTUREVS;
   }

   if (to_flush) {
      etna_set_state(stream, VIVS_GL_FLUSH_CACHE, to_flush);
      if (to_flush_separate)
         etna_set_state(stream, VIVS_GL_FLUSH_CACHE, to_flush_separate);
      etna_stall(stream, SYNC_RECIPIENT_RA, SYNC_RECIPIENT_PE);
   }

   /* Flush TS cache before changing TS configuration. */
   if (unlikely(dirty & ETNA_DIRTY_TS)) {
      etna_set_state(stream, VIVS_TS_FLUSH_CACHE, VIVS_TS_FLUSH_CACHE_FLUSH);
   }

   /* Update vertex elements. This is different from any of the other states, in that
    * a) the number of vertex elements written matters: so write only active ones
    * b) the vertex element states must all be written: do not skip entries that stay the same */
   if (dirty & (ETNA_DIRTY_VERTEX_ELEMENTS)) {
      if (screen->specs.halti >= 5) {
         /*17800*/ etna_set_state_multi(stream, VIVS_NFE_GENERIC_ATTRIB_CONFIG0(0),
            ctx->vertex_elements->num_elements,
            ctx->vertex_elements->NFE_GENERIC_ATTRIB_CONFIG0);
         /*17A00*/ etna_set_state_multi(stream, VIVS_NFE_GENERIC_ATTRIB_SCALE(0),
            ctx->vertex_elements->num_elements,
            ctx->vertex_elements->NFE_GENERIC_ATTRIB_SCALE);
         /*17A80*/ etna_set_state_multi(stream, VIVS_NFE_GENERIC_ATTRIB_CONFIG1(0),
            ctx->vertex_elements->num_elements,
            ctx->vertex_elements->NFE_GENERIC_ATTRIB_CONFIG1);
      } else {
         /* Special case: vertex elements must always be sent in full if changed */
         /*00600*/ etna_set_state_multi(stream, VIVS_FE_VERTEX_ELEMENT_CONFIG(0),
            ctx->vertex_elements->num_elements,
            ctx->vertex_elements->FE_VERTEX_ELEMENT_CONFIG);
         if (screen->specs.halti >= 2) {
            /*00780*/ etna_set_state_multi(stream, VIVS_FE_GENERIC_ATTRIB_SCALE(0),
               ctx->vertex_elements->num_elements,
               ctx->vertex_elements->NFE_GENERIC_ATTRIB_SCALE);
         }
      }
   }
   unsigned vs_output_count = etna_rasterizer_state(ctx->rasterizer)->point_size_per_vertex
                           ? ctx->shader_state.VS_OUTPUT_COUNT_PSIZE
                           : ctx->shader_state.VS_OUTPUT_COUNT;

   /* The following code is originally generated by gen_merge_state.py, to
    * emit state in increasing order of address (this makes it possible to merge
    * consecutive register updates into one SET_STATE command)
    *
    * There have been some manual changes, where the weaving operation is not
    * simply bitwise or:
    * - scissor fixp
    * - num vertex elements
    * - scissor handling
    * - num samplers
    * - texture lod
    * - ETNA_DIRTY_TS
    * - removed ETNA_DIRTY_BASE_SETUP statements -- these are guaranteed to not
    * change anyway
    * - PS / framebuffer interaction for MSAA
    * - move update of GL_MULTI_SAMPLE_CONFIG first
    * - add unlikely()/likely()
    */
   struct etna_coalesce coalesce;

   etna_coalesce_start(stream, &coalesce);

   /* begin only EMIT_STATE -- make sure no new etna_reserve calls are done here
    * directly
    *    or indirectly */
   /* multi sample config is set first, and outside of the normal sorting
    * order, as changing the multisample state clobbers PS.INPUT_COUNT (and
    * possibly PS.TEMP_REGISTER_CONTROL).
    */
   if (unlikely(dirty & (ETNA_DIRTY_FRAMEBUFFER | ETNA_DIRTY_SAMPLE_MASK))) {
      uint32_t val = VIVS_GL_MULTI_SAMPLE_CONFIG_MSAA_ENABLES(ctx->sample_mask);
      val |= ctx->framebuffer.GL_MULTI_SAMPLE_CONFIG;

      /*03818*/ EMIT_STATE(GL_MULTI_SAMPLE_CONFIG, val);
   }
   if (likely(dirty & (ETNA_DIRTY_INDEX_BUFFER))) {
      /*00644*/ EMIT_STATE_RELOC(FE_INDEX_STREAM_BASE_ADDR, &ctx->index_buffer.FE_INDEX_STREAM_BASE_ADDR);
      /*00648*/ EMIT_STATE(FE_INDEX_STREAM_CONTROL, ctx->index_buffer.FE_INDEX_STREAM_CONTROL);
   }
   if (likely(dirty & (ETNA_DIRTY_INDEX_BUFFER))) {
      /*00674*/ EMIT_STATE(FE_PRIMITIVE_RESTART_INDEX, ctx->index_buffer.FE_PRIMITIVE_RESTART_INDEX);
   }
   if (likely(dirty & (ETNA_DIRTY_VERTEX_BUFFERS))) {
      if (screen->specs.halti >= 2) { /* HALTI2+: NFE_VERTEX_STREAMS */
         for (int x = 0; x < ctx->vertex_buffer.count; ++x) {
            /*14600*/ EMIT_STATE_RELOC(NFE_VERTEX_STREAMS_BASE_ADDR(x), &ctx->vertex_buffer.cvb[x].FE_VERTEX_STREAM_BASE_ADDR);
         }
      } else if(screen->specs.stream_count > 1) { /* hw w/ multiple vertex streams */
         for (int x = 0; x < ctx->vertex_buffer.count; ++x) {
            /*00680*/ EMIT_STATE_RELOC(FE_VERTEX_STREAMS_BASE_ADDR(x), &ctx->vertex_buffer.cvb[x].FE_VERTEX_STREAM_BASE_ADDR);
         }
      } else { /* hw w/ single vertex stream */
         /*0064C*/ EMIT_STATE_RELOC(FE_VERTEX_STREAM_BASE_ADDR, &ctx->vertex_buffer.cvb[0].FE_VERTEX_STREAM_BASE_ADDR);
      }
   }
   /* gallium has instance divisor as part of elements state */
   if (dirty & (ETNA_DIRTY_VERTEX_ELEMENTS)) {
      for (int x = 0; x < ctx->vertex_elements->num_buffers; ++x) {
         if (ctx->vertex_buffer.cvb[x].FE_VERTEX_STREAM_BASE_ADDR.bo) {
            if (screen->specs.halti >= 2)
               /*14640*/ EMIT_STATE(NFE_VERTEX_STREAMS_CONTROL(x), ctx->vertex_elements->FE_VERTEX_STREAM_CONTROL[x]);
            else if (screen->specs.stream_count > 1)
               /*006A0*/ EMIT_STATE(FE_VERTEX_STREAMS_CONTROL(x), ctx->vertex_elements->FE_VERTEX_STREAM_CONTROL[x]);
            else
               /*00650*/ EMIT_STATE(FE_VERTEX_STREAM_CONTROL, ctx->vertex_elements->FE_VERTEX_STREAM_CONTROL[0]);
         }
      }
      if (screen->specs.halti >= 2) {
         for (int x = 0; x < ctx->vertex_elements->num_buffers; ++x) {
            /*14680*/ EMIT_STATE(NFE_VERTEX_STREAMS_VERTEX_DIVISOR(x), ctx->vertex_elements->NFE_VERTEX_STREAMS_VERTEX_DIVISOR[x]);
         }
      }
   }

   if (unlikely(dirty & (ETNA_DIRTY_SHADER | ETNA_DIRTY_RASTERIZER))) {

      /*00804*/ EMIT_STATE(VS_OUTPUT_COUNT, vs_output_count);
   }
   if (unlikely(dirty & (ETNA_DIRTY_VERTEX_ELEMENTS | ETNA_DIRTY_SHADER))) {
      /*00808*/ EMIT_STATE(VS_INPUT_COUNT, ctx->shader_state.VS_INPUT_COUNT);
      /*0080C*/ EMIT_STATE(VS_TEMP_REGISTER_CONTROL, ctx->shader_state.VS_TEMP_REGISTER_CONTROL);
   }
   if (unlikely(dirty & (ETNA_DIRTY_SHADER))) {
      /*00830*/ EMIT_STATE(VS_LOAD_BALANCING, ctx->shader_state.VS_LOAD_BALANCING);
   }
   if (unlikely(dirty & (ETNA_DIRTY_VIEWPORT))) {
      /*00A00*/ EMIT_STATE_FIXP(PA_VIEWPORT_SCALE_X, ctx->viewport.PA_VIEWPORT_SCALE_X);
      /*00A04*/ EMIT_STATE_FIXP(PA_VIEWPORT_SCALE_Y, ctx->viewport.PA_VIEWPORT_SCALE_Y);
      /*00A08*/ EMIT_STATE(PA_VIEWPORT_SCALE_Z, ctx->viewport.PA_VIEWPORT_SCALE_Z);
      /*00A0C*/ EMIT_STATE_FIXP(PA_VIEWPORT_OFFSET_X, ctx->viewport.PA_VIEWPORT_OFFSET_X);
      /*00A10*/ EMIT_STATE_FIXP(PA_VIEWPORT_OFFSET_Y, ctx->viewport.PA_VIEWPORT_OFFSET_Y);
      /*00A14*/ EMIT_STATE(PA_VIEWPORT_OFFSET_Z, ctx->viewport.PA_VIEWPORT_OFFSET_Z);
   }
   if (unlikely(dirty & (ETNA_DIRTY_RASTERIZER))) {
      struct etna_rasterizer_state *rasterizer = etna_rasterizer_state(ctx->rasterizer);

      /*00A18*/ EMIT_STATE(PA_LINE_WIDTH, rasterizer->PA_LINE_WIDTH);
      /*00A1C*/ EMIT_STATE(PA_POINT_SIZE, rasterizer->PA_POINT_SIZE);
      /*00A28*/ EMIT_STATE(PA_SYSTEM_MODE, rasterizer->PA_SYSTEM_MODE);
   }
   if (unlikely(dirty & (ETNA_DIRTY_SHADER))) {
      /*00A30*/ EMIT_STATE(PA_ATTRIBUTE_ELEMENT_COUNT, ctx->shader_state.PA_ATTRIBUTE_ELEMENT_COUNT);
   }
   if (unlikely(dirty & (ETNA_DIRTY_RASTERIZER | ETNA_DIRTY_SHADER))) {
      uint32_t val = etna_rasterizer_state(ctx->rasterizer)->PA_CONFIG;
      /*00A34*/ EMIT_STATE(PA_CONFIG, val & ctx->shader_state.PA_CONFIG);
   }
   if (unlikely(dirty & (ETNA_DIRTY_RASTERIZER))) {
      struct etna_rasterizer_state *rasterizer = etna_rasterizer_state(ctx->rasterizer);
      /*00A38*/ EMIT_STATE(PA_WIDE_LINE_WIDTH0, rasterizer->PA_LINE_WIDTH);
      /*00A3C*/ EMIT_STATE(PA_WIDE_LINE_WIDTH1, rasterizer->PA_LINE_WIDTH);
   }
   if (unlikely(dirty & (ETNA_DIRTY_SCISSOR_CLIP))) {
      /*00C00*/ EMIT_STATE_FIXP(SE_SCISSOR_LEFT, ctx->clipping.minx << 16);
      /*00C04*/ EMIT_STATE_FIXP(SE_SCISSOR_TOP, ctx->clipping.miny << 16);
      /*00C08*/ EMIT_STATE_FIXP(SE_SCISSOR_RIGHT, (ctx->clipping.maxx << 16) + ETNA_SE_SCISSOR_MARGIN_RIGHT);
      /*00C0C*/ EMIT_STATE_FIXP(SE_SCISSOR_BOTTOM, (ctx->clipping.maxy << 16) + ETNA_SE_SCISSOR_MARGIN_BOTTOM);
   }
   if (unlikely(dirty & (ETNA_DIRTY_RASTERIZER))) {
      struct etna_rasterizer_state *rasterizer = etna_rasterizer_state(ctx->rasterizer);

      /*00C10*/ EMIT_STATE(SE_DEPTH_SCALE, rasterizer->SE_DEPTH_SCALE);
      /*00C14*/ EMIT_STATE(SE_DEPTH_BIAS, rasterizer->SE_DEPTH_BIAS);
      /*00C18*/ EMIT_STATE(SE_CONFIG, rasterizer->SE_CONFIG);
   }
   if (unlikely(dirty & (ETNA_DIRTY_SCISSOR_CLIP))) {
      /*00C20*/ EMIT_STATE_FIXP(SE_CLIP_RIGHT, (ctx->clipping.maxx << 16) + ETNA_SE_CLIP_MARGIN_RIGHT);
      /*00C24*/ EMIT_STATE_FIXP(SE_CLIP_BOTTOM, (ctx->clipping.maxy << 16) + ETNA_SE_CLIP_MARGIN_BOTTOM);
   }
   if (unlikely(dirty & (ETNA_DIRTY_SHADER))) {
      /*00E00*/ EMIT_STATE(RA_CONTROL, ctx->shader_state.RA_CONTROL);
   }
   if (unlikely(dirty & (ETNA_DIRTY_ZSA))) {
      /*00E08*/ EMIT_STATE(RA_EARLY_DEPTH, etna_zsa_state(ctx->zsa)->RA_DEPTH_CONFIG);
   }
   if (unlikely(dirty & (ETNA_DIRTY_SHADER | ETNA_DIRTY_FRAMEBUFFER))) {
      /*01004*/ EMIT_STATE(PS_OUTPUT_REG, ctx->shader_state.PS_OUTPUT_REG);
      /*01008*/ EMIT_STATE(PS_INPUT_COUNT,
                           ctx->framebuffer.msaa_mode
                              ? ctx->shader_state.PS_INPUT_COUNT_MSAA
                              : ctx->shader_state.PS_INPUT_COUNT);
      /*0100C*/ EMIT_STATE(PS_TEMP_REGISTER_CONTROL,
                           ctx->framebuffer.msaa_mode
                              ? ctx->shader_state.PS_TEMP_REGISTER_CONTROL_MSAA
                              : ctx->shader_state.PS_TEMP_REGISTER_CONTROL);
      /*01010*/ EMIT_STATE(PS_CONTROL, ctx->framebuffer.PS_CONTROL);
      /*01030*/ EMIT_STATE(PS_CONTROL_EXT, ctx->framebuffer.PS_CONTROL_EXT);
   }
   if (unlikely(dirty & (ETNA_DIRTY_ZSA | ETNA_DIRTY_FRAMEBUFFER))) {
      /*01400*/ EMIT_STATE(PE_DEPTH_CONFIG, (etna_zsa_state(ctx->zsa)->PE_DEPTH_CONFIG |
                                             ctx->framebuffer.PE_DEPTH_CONFIG));
   }
   if (unlikely(dirty & (ETNA_DIRTY_VIEWPORT))) {
      /*01404*/ EMIT_STATE(PE_DEPTH_NEAR, ctx->viewport.PE_DEPTH_NEAR);
      /*01408*/ EMIT_STATE(PE_DEPTH_FAR, ctx->viewport.PE_DEPTH_FAR);
   }
   if (unlikely(dirty & (ETNA_DIRTY_FRAMEBUFFER))) {
      /*0140C*/ EMIT_STATE(PE_DEPTH_NORMALIZE, ctx->framebuffer.PE_DEPTH_NORMALIZE);

      if (screen->specs.halti < 0 || screen->model == 0x880) {
         /*01410*/ EMIT_STATE_RELOC(PE_DEPTH_ADDR, &ctx->framebuffer.PE_DEPTH_ADDR);
      }

      /*01414*/ EMIT_STATE(PE_DEPTH_STRIDE, ctx->framebuffer.PE_DEPTH_STRIDE);
   }

   if (unlikely(dirty & (ETNA_DIRTY_ZSA | ETNA_DIRTY_RASTERIZER))) {
      uint32_t val = etna_zsa_state(ctx->zsa)->PE_STENCIL_OP[ccw];
      /*01418*/ EMIT_STATE(PE_STENCIL_OP, val);
   }
   if (unlikely(dirty & (ETNA_DIRTY_ZSA | ETNA_DIRTY_STENCIL_REF | ETNA_DIRTY_RASTERIZER))) {
      uint32_t val = etna_zsa_state(ctx->zsa)->PE_STENCIL_CONFIG[ccw];
      /*0141C*/ EMIT_STATE(PE_STENCIL_CONFIG, val | ctx->stencil_ref.PE_STENCIL_CONFIG[ccw]);
   }
   if (unlikely(dirty & (ETNA_DIRTY_ZSA))) {
      uint32_t val = etna_zsa_state(ctx->zsa)->PE_ALPHA_OP;
      /*01420*/ EMIT_STATE(PE_ALPHA_OP, val);
   }
   if (unlikely(dirty & (ETNA_DIRTY_BLEND_COLOR))) {
      /*01424*/ EMIT_STATE(PE_ALPHA_BLEND_COLOR, ctx->blend_color.PE_ALPHA_BLEND_COLOR);
   }
   if (unlikely(dirty & (ETNA_DIRTY_BLEND))) {
      uint32_t val = etna_blend_state(ctx->blend)->PE_ALPHA_CONFIG;
      /*01428*/ EMIT_STATE(PE_ALPHA_CONFIG, val);
   }
   if (unlikely(dirty & (ETNA_DIRTY_BLEND | ETNA_DIRTY_FRAMEBUFFER))) {
      uint32_t val;
      /* Use the components and overwrite bits in framebuffer.PE_COLOR_FORMAT
       * as a mask to enable the bits from blend PE_COLOR_FORMAT */
      val = ~(VIVS_PE_COLOR_FORMAT_COMPONENTS__MASK |
              VIVS_PE_COLOR_FORMAT_OVERWRITE);
      val |= etna_blend_state(ctx->blend)->PE_COLOR_FORMAT;
      val &= ctx->framebuffer.PE_COLOR_FORMAT;
      /*0142C*/ EMIT_STATE(PE_COLOR_FORMAT, val);
   }
   if (unlikely(dirty & (ETNA_DIRTY_FRAMEBUFFER))) {
      if (screen->specs.halti >= 0 && screen->model != 0x880) {
         /*01434*/ EMIT_STATE(PE_COLOR_STRIDE, ctx->framebuffer.PE_COLOR_STRIDE);
         /*01454*/ EMIT_STATE(PE_HDEPTH_CONTROL, ctx->framebuffer.PE_HDEPTH_CONTROL);
         /*01460*/ EMIT_STATE_RELOC(PE_PIPE_COLOR_ADDR(0), &ctx->framebuffer.PE_PIPE_COLOR_ADDR[0]);
         /*01464*/ EMIT_STATE_RELOC(PE_PIPE_COLOR_ADDR(1), &ctx->framebuffer.PE_PIPE_COLOR_ADDR[1]);
         /*01480*/ EMIT_STATE_RELOC(PE_PIPE_DEPTH_ADDR(0), &ctx->framebuffer.PE_PIPE_DEPTH_ADDR[0]);
         /*01484*/ EMIT_STATE_RELOC(PE_PIPE_DEPTH_ADDR(1), &ctx->framebuffer.PE_PIPE_DEPTH_ADDR[1]);
      } else {
         /*01430*/ EMIT_STATE_RELOC(PE_COLOR_ADDR, &ctx->framebuffer.PE_COLOR_ADDR);
         /*01434*/ EMIT_STATE(PE_COLOR_STRIDE, ctx->framebuffer.PE_COLOR_STRIDE);
         /*01454*/ EMIT_STATE(PE_HDEPTH_CONTROL, ctx->framebuffer.PE_HDEPTH_CONTROL);
      }
   }
   if (unlikely(dirty & (ETNA_DIRTY_STENCIL_REF | ETNA_DIRTY_RASTERIZER | ETNA_DIRTY_ZSA))) {
      uint32_t val = etna_zsa_state(ctx->zsa)->PE_STENCIL_CONFIG_EXT;
      if (!ctx->zsa->stencil[1].enabled &&
          ctx->zsa->stencil[0].enabled &&
          ctx->zsa->stencil[0].valuemask)
	  val |= ctx->stencil_ref.PE_STENCIL_CONFIG_EXT[!ccw];
      else
	  val |= ctx->stencil_ref.PE_STENCIL_CONFIG_EXT[ccw];
      /*014A0*/ EMIT_STATE(PE_STENCIL_CONFIG_EXT, val);
   }
   if (unlikely(dirty & (ETNA_DIRTY_BLEND | ETNA_DIRTY_FRAMEBUFFER))) {
      struct etna_blend_state *blend = etna_blend_state(ctx->blend);
      /*014A4*/ EMIT_STATE(PE_LOGIC_OP, blend->PE_LOGIC_OP | ctx->framebuffer.PE_LOGIC_OP);
   }
   if (unlikely(dirty & (ETNA_DIRTY_BLEND))) {
      struct etna_blend_state *blend = etna_blend_state(ctx->blend);
      for (int x = 0; x < 2; ++x) {
         /*014A8*/ EMIT_STATE(PE_DITHER(x), blend->PE_DITHER[x]);
      }
   }
   if (unlikely(dirty & (ETNA_DIRTY_BLEND_COLOR)) &&
       VIV_FEATURE(screen, chipMinorFeatures1, HALF_FLOAT)) {
         /*014B0*/ EMIT_STATE(PE_ALPHA_COLOR_EXT0, ctx->blend_color.PE_ALPHA_COLOR_EXT0);
         /*014B4*/ EMIT_STATE(PE_ALPHA_COLOR_EXT1, ctx->blend_color.PE_ALPHA_COLOR_EXT1);
   }
   if (unlikely(dirty & (ETNA_DIRTY_ZSA | ETNA_DIRTY_RASTERIZER))) {
      /*014B8*/ EMIT_STATE(PE_STENCIL_CONFIG_EXT2, etna_zsa_state(ctx->zsa)->PE_STENCIL_CONFIG_EXT2[ccw]);
   }
   if (unlikely(dirty & (ETNA_DIRTY_FRAMEBUFFER)) && screen->specs.halti >= 3)
      /*014BC*/ EMIT_STATE(PE_MEM_CONFIG, ctx->framebuffer.PE_MEM_CONFIG);
   if (unlikely(dirty & (ETNA_DIRTY_FRAMEBUFFER | ETNA_DIRTY_TS))) {
      /*01654*/ EMIT_STATE(TS_MEM_CONFIG, ctx->framebuffer.TS_MEM_CONFIG);
      /*01658*/ EMIT_STATE_RELOC(TS_COLOR_STATUS_BASE, &ctx->framebuffer.TS_COLOR_STATUS_BASE);
      /*0165C*/ EMIT_STATE_RELOC(TS_COLOR_SURFACE_BASE, &ctx->framebuffer.TS_COLOR_SURFACE_BASE);
      /*01660*/ EMIT_STATE(TS_COLOR_CLEAR_VALUE, ctx->framebuffer.TS_COLOR_CLEAR_VALUE);
      /*01664*/ EMIT_STATE_RELOC(TS_DEPTH_STATUS_BASE, &ctx->framebuffer.TS_DEPTH_STATUS_BASE);
      /*01668*/ EMIT_STATE_RELOC(TS_DEPTH_SURFACE_BASE, &ctx->framebuffer.TS_DEPTH_SURFACE_BASE);
      /*0166C*/ EMIT_STATE(TS_DEPTH_CLEAR_VALUE, ctx->framebuffer.TS_DEPTH_CLEAR_VALUE);
      /*016BC*/ EMIT_STATE(TS_COLOR_CLEAR_VALUE_EXT, ctx->framebuffer.TS_COLOR_CLEAR_VALUE_EXT);
   }
   if (unlikely(dirty & (ETNA_DIRTY_SHADER))) {
      /*0381C*/ EMIT_STATE(GL_VARYING_TOTAL_COMPONENTS, ctx->shader_state.GL_VARYING_TOTAL_COMPONENTS);
   }
   etna_coalesce_end(stream, &coalesce);
   /* end only EMIT_STATE */

   /* Emit strongly architecture-specific state */
   if (screen->specs.halti >= 5)
      emit_halti5_only_state(ctx, vs_output_count);
   else
      emit_pre_halti5_state(ctx);

   /* Beginning from Halti0 some of the new shader and sampler states are not
    * self-synchronizing anymore. Thus we need to stall the FE on PE completion
    * before loading the new states to avoid corrupting the state of the
    * in-flight draw.
    */
   if (screen->specs.halti >= 0 &&
       (ctx->dirty & (ETNA_DIRTY_SHADER | ETNA_DIRTY_CONSTBUF |
                      ETNA_DIRTY_SAMPLERS | ETNA_DIRTY_SAMPLER_VIEWS)))
      etna_stall(ctx->stream, SYNC_RECIPIENT_FE, SYNC_RECIPIENT_PE);

   ctx->emit_texture_state(ctx);

   /* We need to update the uniform cache only if one of the following bits are
    * set in ctx->dirty:
    * - ETNA_DIRTY_SHADER
    * - ETNA_DIRTY_CONSTBUF
    * - uniforms_dirty_bits
    *
    * In case of ETNA_DIRTY_SHADER we need load all uniforms from the cache. In
    * all
    * other cases we can load on the changed uniforms.
    */
   static const uint32_t uniform_dirty_bits =
      ETNA_DIRTY_SHADER | ETNA_DIRTY_CONSTBUF;

   /**** Large dynamically-sized state ****/
   bool do_uniform_flush = screen->specs.halti < 5;
   if (dirty & (ETNA_DIRTY_SHADER)) {
      /* Special case: a new shader was loaded; simply re-load all uniforms and
       * shader code at once */
      /* This sequence is special, do not change ordering unless necessary. According to comment
         snippets in the Vivante kernel driver a process called "steering" goes on while programming
         shader state. This (as I understand it) means certain unified states are "steered"
         toward a specific shader unit (VS/PS/...) based on either explicit flags in register
         00860, or what other state is written before "auto-steering". So this means some
         state can legitimately be programmed multiple times.
       */

      if (screen->specs.halti >= 5) { /* ICACHE (HALTI5) */
         assert(ctx->shader_state.VS_INST_ADDR.bo && ctx->shader_state.PS_INST_ADDR.bo);
         /* Set icache (VS) */
         etna_set_state(stream, VIVS_VS_NEWRANGE_LOW, 0);
         etna_set_state(stream, VIVS_VS_NEWRANGE_HIGH, ctx->shader_state.vs_inst_mem_size / 4);
         assert(ctx->shader_state.VS_INST_ADDR.bo);
         etna_set_state_reloc(stream, VIVS_VS_INST_ADDR, &ctx->shader_state.VS_INST_ADDR);
         etna_set_state(stream, VIVS_SH_CONFIG, 0x00000002);
         etna_set_state(stream, VIVS_VS_ICACHE_CONTROL, VIVS_VS_ICACHE_CONTROL_ENABLE);
         etna_set_state(stream, VIVS_VS_ICACHE_COUNT, ctx->shader_state.vs_inst_mem_size / 4 - 1);

         /* Set icache (PS) */
         etna_set_state(stream, VIVS_PS_NEWRANGE_LOW, 0);
         etna_set_state(stream, VIVS_PS_NEWRANGE_HIGH, ctx->shader_state.ps_inst_mem_size / 4);
         assert(ctx->shader_state.PS_INST_ADDR.bo);
         etna_set_state_reloc(stream, VIVS_PS_INST_ADDR, &ctx->shader_state.PS_INST_ADDR);
         etna_set_state(stream, VIVS_SH_CONFIG, 0x00000002);
         etna_set_state(stream, VIVS_VS_ICACHE_CONTROL, VIVS_VS_ICACHE_CONTROL_ENABLE);
         etna_set_state(stream, VIVS_PS_ICACHE_COUNT, ctx->shader_state.ps_inst_mem_size / 4 - 1);

      } else if (ctx->shader_state.VS_INST_ADDR.bo || ctx->shader_state.PS_INST_ADDR.bo) {
         /* ICACHE (pre-HALTI5) */
         assert(screen->specs.has_icache && screen->specs.has_shader_range_registers);
         /* Set icache (VS) */
         etna_set_state(stream, VIVS_VS_RANGE, (ctx->shader_state.vs_inst_mem_size / 4 - 1) << 16);
         etna_set_state(stream, VIVS_VS_ICACHE_CONTROL,
               VIVS_VS_ICACHE_CONTROL_ENABLE |
               VIVS_VS_ICACHE_CONTROL_FLUSH_VS);
         assert(ctx->shader_state.VS_INST_ADDR.bo);
         etna_set_state_reloc(stream, VIVS_VS_INST_ADDR, &ctx->shader_state.VS_INST_ADDR);

         /* Set icache (PS) */
         etna_set_state(stream, VIVS_PS_RANGE, (ctx->shader_state.ps_inst_mem_size / 4 - 1) << 16);
         etna_set_state(stream, VIVS_VS_ICACHE_CONTROL,
               VIVS_VS_ICACHE_CONTROL_ENABLE |
               VIVS_VS_ICACHE_CONTROL_FLUSH_PS);
         assert(ctx->shader_state.PS_INST_ADDR.bo);
         etna_set_state_reloc(stream, VIVS_PS_INST_ADDR, &ctx->shader_state.PS_INST_ADDR);
      } else {
         /* Upload shader directly, first flushing and disabling icache if
          * supported on this hw */
         if (screen->specs.has_icache) {
            etna_set_state(stream, VIVS_VS_ICACHE_CONTROL,
                  VIVS_VS_ICACHE_CONTROL_FLUSH_PS |
                  VIVS_VS_ICACHE_CONTROL_FLUSH_VS);
         }
         if (screen->specs.has_shader_range_registers) {
            etna_set_state(stream, VIVS_VS_RANGE, (ctx->shader_state.vs_inst_mem_size / 4 - 1) << 16);
            etna_set_state(stream, VIVS_PS_RANGE, ((ctx->shader_state.ps_inst_mem_size / 4 - 1 + 0x100) << 16) |
                                        0x100);
         }
         etna_set_state_multi(stream, screen->specs.vs_offset,
                              ctx->shader_state.vs_inst_mem_size,
                              ctx->shader_state.VS_INST_MEM);
         etna_set_state_multi(stream, screen->specs.ps_offset,
                              ctx->shader_state.ps_inst_mem_size,
                              ctx->shader_state.PS_INST_MEM);
      }

      if (screen->specs.has_unified_uniforms) {
         etna_set_state(stream, VIVS_VS_UNIFORM_BASE, 0);
         etna_set_state(stream, VIVS_PS_UNIFORM_BASE, screen->specs.max_vs_uniforms);
      }

      if (do_uniform_flush)
         etna_set_state(stream, VIVS_VS_UNIFORM_CACHE, VIVS_VS_UNIFORM_CACHE_FLUSH);

      etna_uniforms_write(ctx, ctx->shader.vs, ctx->constant_buffer[PIPE_SHADER_VERTEX].cb);

      if (do_uniform_flush)
         etna_set_state(stream, VIVS_VS_UNIFORM_CACHE, VIVS_VS_UNIFORM_CACHE_FLUSH | VIVS_VS_UNIFORM_CACHE_PS);

      etna_uniforms_write(ctx, ctx->shader.fs, ctx->constant_buffer[PIPE_SHADER_FRAGMENT].cb);

      if (screen->specs.halti >= 5) {
         /* HALTI5 needs to be prompted to pre-fetch shaders */
         etna_set_state(stream, VIVS_VS_ICACHE_PREFETCH, 0x00000000);
         etna_set_state(stream, VIVS_PS_ICACHE_PREFETCH, 0x00000000);
         etna_stall(stream, SYNC_RECIPIENT_RA, SYNC_RECIPIENT_PE);
      }
   } else {
      /* ideally this cache would only be flushed if there are VS uniform changes */
      if (do_uniform_flush)
         etna_set_state(stream, VIVS_VS_UNIFORM_CACHE, VIVS_VS_UNIFORM_CACHE_FLUSH);

      if (dirty & (uniform_dirty_bits | ctx->shader.vs->uniforms_dirty_bits))
         etna_uniforms_write(ctx, ctx->shader.vs, ctx->constant_buffer[PIPE_SHADER_VERTEX].cb);

      /* ideally this cache would only be flushed if there are PS uniform changes */
      if (do_uniform_flush)
         etna_set_state(stream, VIVS_VS_UNIFORM_CACHE, VIVS_VS_UNIFORM_CACHE_FLUSH | VIVS_VS_UNIFORM_CACHE_PS);

      if (dirty & (uniform_dirty_bits | ctx->shader.fs->uniforms_dirty_bits))
         etna_uniforms_write(ctx, ctx->shader.fs, ctx->constant_buffer[PIPE_SHADER_FRAGMENT].cb);
   }
/**** End of state update ****/
#undef EMIT_STATE
#undef EMIT_STATE_FIXP
#undef EMIT_STATE_RELOC
   ctx->dirty = 0;
   ctx->dirty_sampler_views = 0;
}
