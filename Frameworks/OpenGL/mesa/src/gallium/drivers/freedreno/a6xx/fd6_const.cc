/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
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
 */

#define FD_BO_NO_HARDPIN 1

#include "fd6_const.h"
#include "fd6_compute.h"
#include "fd6_pack.h"

#define emit_const_user fd6_emit_const_user
#define emit_const_bo   fd6_emit_const_bo
#include "ir3_const.h"

/* regid:          base const register
 * prsc or dwords: buffer containing constant values
 * sizedwords:     size of const value buffer
 */
void
fd6_emit_const_user(struct fd_ringbuffer *ring,
                    const struct ir3_shader_variant *v, uint32_t regid,
                    uint32_t sizedwords, const uint32_t *dwords)
{
   emit_const_asserts(ring, v, regid, sizedwords);

   /* NOTE we cheat a bit here, since we know mesa is aligning
    * the size of the user buffer to 16 bytes.  And we want to
    * cut cycles in a hot path.
    */
   uint32_t align_sz = align(sizedwords, 4);

   if (fd6_geom_stage(v->type)) {
      OUT_PKTBUF(ring, CP_LOAD_STATE6_GEOM, dwords, align_sz,
         CP_LOAD_STATE6_0(.dst_off = regid / 4, .state_type = ST6_CONSTANTS,
                          .state_src = SS6_DIRECT,
                          .state_block = fd6_stage2shadersb(v->type),
                          .num_unit = DIV_ROUND_UP(sizedwords, 4)),
         CP_LOAD_STATE6_1(),
         CP_LOAD_STATE6_2());
   } else {
      OUT_PKTBUF(ring, CP_LOAD_STATE6_FRAG, dwords, align_sz,
         CP_LOAD_STATE6_0(.dst_off = regid / 4, .state_type = ST6_CONSTANTS,
                          .state_src = SS6_DIRECT,
                          .state_block = fd6_stage2shadersb(v->type),
                          .num_unit = DIV_ROUND_UP(sizedwords, 4)),
         CP_LOAD_STATE6_1(),
         CP_LOAD_STATE6_2());
   }
}
void
fd6_emit_const_bo(struct fd_ringbuffer *ring,
                  const struct ir3_shader_variant *v, uint32_t regid,
                  uint32_t offset, uint32_t sizedwords, struct fd_bo *bo)
{
   uint32_t dst_off = regid / 4;
   assert(dst_off % 4 == 0);
   uint32_t num_unit = DIV_ROUND_UP(sizedwords, 4);
   assert(num_unit % 4 == 0);

   emit_const_asserts(ring, v, regid, sizedwords);

   if (fd6_geom_stage(v->type)) {
      OUT_PKT(ring, CP_LOAD_STATE6_GEOM,
              CP_LOAD_STATE6_0(.dst_off = dst_off, .state_type = ST6_CONSTANTS,
                               .state_src = SS6_INDIRECT,
                               .state_block = fd6_stage2shadersb(v->type),
                               .num_unit = num_unit, ),
              CP_LOAD_STATE6_EXT_SRC_ADDR(.bo = bo, .bo_offset = offset));
   } else {
      OUT_PKT(ring, CP_LOAD_STATE6_FRAG,
              CP_LOAD_STATE6_0(.dst_off = dst_off, .state_type = ST6_CONSTANTS,
                               .state_src = SS6_INDIRECT,
                               .state_block = fd6_stage2shadersb(v->type),
                               .num_unit = num_unit, ),
              CP_LOAD_STATE6_EXT_SRC_ADDR(.bo = bo, .bo_offset = offset));
   }
}

static bool
is_stateobj(struct fd_ringbuffer *ring)
{
   return true;
}

static void
emit_const_ptrs(struct fd_ringbuffer *ring, const struct ir3_shader_variant *v,
                uint32_t dst_offset, uint32_t num, struct fd_bo **bos,
                uint32_t *offsets)
{
   unreachable("shouldn't be called on a6xx");
}

static void
emit_stage_tess_consts(struct fd_ringbuffer *ring, const struct ir3_shader_variant *v,
                       uint32_t *params, int num_params)
{
   const struct ir3_const_state *const_state = ir3_const_state(v);
   const unsigned regid = const_state->offsets.primitive_param;
   int size = MIN2(1 + regid, v->constlen) - regid;
   if (size > 0)
      fd6_emit_const_user(ring, v, regid * 4, num_params, params);
}

struct fd_ringbuffer *
fd6_build_tess_consts(struct fd6_emit *emit)
{
   struct fd_context *ctx = emit->ctx;

   struct fd_ringbuffer *constobj = fd_submit_new_ringbuffer(
      ctx->batch->submit, 0x1000, FD_RINGBUFFER_STREAMING);

   /* VS sizes are in bytes since that's what STLW/LDLW use, while the HS
    * size is dwords, since that's what LDG/STG use.
    */
   unsigned num_vertices = emit->hs
                              ? ctx->patch_vertices
                              : emit->gs->gs.vertices_in;

   uint32_t vs_params[4] = {
      emit->vs->output_size * num_vertices * 4, /* vs primitive stride */
      emit->vs->output_size * 4,                /* vs vertex stride */
      0, 0};

   emit_stage_tess_consts(constobj, emit->vs, vs_params, ARRAY_SIZE(vs_params));

   if (emit->hs) {
      uint32_t hs_params[4] = {
         emit->vs->output_size * num_vertices * 4, /* vs primitive stride */
         emit->vs->output_size * 4,                /* vs vertex stride */
         emit->hs->output_size, ctx->patch_vertices};

      emit_stage_tess_consts(constobj, emit->hs, hs_params,
                             ARRAY_SIZE(hs_params));

      if (emit->gs)
         num_vertices = emit->gs->gs.vertices_in;

      uint32_t ds_params[4] = {
         emit->ds->output_size * num_vertices * 4, /* ds primitive stride */
         emit->ds->output_size * 4,                /* ds vertex stride */
         emit->hs->output_size, /* hs vertex stride (dwords) */
         emit->hs->tess.tcs_vertices_out};

      emit_stage_tess_consts(constobj, emit->ds, ds_params,
                             ARRAY_SIZE(ds_params));
   }

   if (emit->gs) {
      const struct ir3_shader_variant *prev;
      if (emit->ds)
         prev = emit->ds;
      else
         prev = emit->vs;

      uint32_t gs_params[4] = {
         prev->output_size * num_vertices * 4, /* ds primitive stride */
         prev->output_size * 4,                /* ds vertex stride */
         0,
         0,
      };

      num_vertices = emit->gs->gs.vertices_in;
      emit_stage_tess_consts(constobj, emit->gs, gs_params,
                             ARRAY_SIZE(gs_params));
   }

   return constobj;
}

static void
fd6_emit_ubos(const struct ir3_shader_variant *v, struct fd_ringbuffer *ring,
              struct fd_constbuf_stateobj *constbuf)
{
   const struct ir3_const_state *const_state = ir3_const_state(v);
   int num_ubos = const_state->num_ubos;

   if (!num_ubos)
      return;

   OUT_PKT7(ring, fd6_stage2opcode(v->type), 3 + (2 * num_ubos));
   OUT_RING(ring, CP_LOAD_STATE6_0_DST_OFF(0) |
                     CP_LOAD_STATE6_0_STATE_TYPE(ST6_UBO) |
                     CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
                     CP_LOAD_STATE6_0_STATE_BLOCK(fd6_stage2shadersb(v->type)) |
                     CP_LOAD_STATE6_0_NUM_UNIT(num_ubos));
   OUT_RING(ring, CP_LOAD_STATE6_1_EXT_SRC_ADDR(0));
   OUT_RING(ring, CP_LOAD_STATE6_2_EXT_SRC_ADDR_HI(0));

   for (int i = 0; i < num_ubos; i++) {
      /* NIR constant data is packed into the end of the shader. */
      if (i == const_state->constant_data_ubo) {
         int size_vec4s = DIV_ROUND_UP(v->constant_data_size, 16);
         OUT_RELOC(ring, v->bo, v->info.constant_data_offset,
                   (uint64_t)A6XX_UBO_1_SIZE(size_vec4s) << 32, 0);
         continue;
      }

      struct pipe_constant_buffer *cb = &constbuf->cb[i];

      if (cb->buffer) {
         int size_vec4s = DIV_ROUND_UP(cb->buffer_size, 16);
         OUT_RELOC(ring, fd_resource(cb->buffer)->bo, cb->buffer_offset,
                   (uint64_t)A6XX_UBO_1_SIZE(size_vec4s) << 32, 0);
      } else {
         OUT_RING(ring, 0xbad00000 | (i << 16));
         OUT_RING(ring, A6XX_UBO_1_SIZE(0));
      }
   }
}

unsigned
fd6_user_consts_cmdstream_size(const struct ir3_shader_variant *v)
{
   if (!v)
      return 0;

   struct ir3_const_state *const_state = ir3_const_state(v);
   struct ir3_ubo_analysis_state *ubo_state = &const_state->ubo_state;
   unsigned packets, size;

   /* pre-calculate size required for userconst stateobj: */
   ir3_user_consts_size(ubo_state, &packets, &size);

   /* also account for UBO addresses: */
   packets += 1;
   size += 2 * const_state->num_ubos;

   unsigned sizedwords = (4 * packets) + size;
   return sizedwords * 4;
}

static void
emit_user_consts(const struct ir3_shader_variant *v,
                 struct fd_ringbuffer *ring,
                 struct fd_constbuf_stateobj *constbuf)
{
   ir3_emit_user_consts(v, ring, constbuf);
   fd6_emit_ubos(v, ring, constbuf);
}

template <fd6_pipeline_type PIPELINE>
struct fd_ringbuffer *
fd6_build_user_consts(struct fd6_emit *emit)
{
   struct fd_context *ctx = emit->ctx;
   unsigned sz = emit->prog->user_consts_cmdstream_size;

   struct fd_ringbuffer *constobj =
      fd_submit_new_ringbuffer(ctx->batch->submit, sz, FD_RINGBUFFER_STREAMING);

   emit_user_consts(emit->vs, constobj, &ctx->constbuf[PIPE_SHADER_VERTEX]);
   if (PIPELINE == HAS_TESS_GS) {
      if (emit->hs) {
         emit_user_consts(emit->hs, constobj, &ctx->constbuf[PIPE_SHADER_TESS_CTRL]);
         emit_user_consts(emit->ds, constobj, &ctx->constbuf[PIPE_SHADER_TESS_EVAL]);
      }
      if (emit->gs) {
         emit_user_consts(emit->gs, constobj, &ctx->constbuf[PIPE_SHADER_GEOMETRY]);
      }
   }
   emit_user_consts(emit->fs, constobj, &ctx->constbuf[PIPE_SHADER_FRAGMENT]);

   return constobj;
}

template struct fd_ringbuffer * fd6_build_user_consts<HAS_TESS_GS>(struct fd6_emit *emit);
template struct fd_ringbuffer * fd6_build_user_consts<NO_TESS_GS>(struct fd6_emit *emit);

template <fd6_pipeline_type PIPELINE>
struct fd_ringbuffer *
fd6_build_driver_params(struct fd6_emit *emit)
{
   struct fd_context *ctx = emit->ctx;
   struct fd6_context *fd6_ctx = fd6_context(ctx);
   unsigned num_dp = emit->prog->num_driver_params;

   if (!num_dp) {
      fd6_ctx->has_dp_state = false;
      return NULL;
   }

   unsigned size_dwords = num_dp * (4 + IR3_DP_VS_COUNT);  /* 4dw PKT7 header */
   struct fd_ringbuffer *dpconstobj = fd_submit_new_ringbuffer(
         ctx->batch->submit, size_dwords * 4, FD_RINGBUFFER_STREAMING);

   if (emit->vs->need_driver_params) {
      ir3_emit_driver_params(emit->vs, dpconstobj, ctx, emit->info,
                             emit->indirect, emit->draw, emit->draw_id);
   }

   if (PIPELINE == HAS_TESS_GS) {
      if (emit->gs && emit->gs->need_driver_params) {
         ir3_emit_driver_params(emit->gs, dpconstobj, ctx, emit->info,
                                emit->indirect, emit->draw, 0);
      }

      if (emit->hs && emit->hs->need_driver_params) {
         ir3_emit_hs_driver_params(emit->hs, dpconstobj, ctx);
      }

      if (emit->ds && emit->ds->need_driver_params) {
         ir3_emit_driver_params(emit->ds, dpconstobj, ctx, emit->info,
                                emit->indirect, emit->draw, 0);
      }
   }

   fd6_ctx->has_dp_state = true;

   return dpconstobj;
}

template struct fd_ringbuffer * fd6_build_driver_params<HAS_TESS_GS>(struct fd6_emit *emit);
template struct fd_ringbuffer * fd6_build_driver_params<NO_TESS_GS>(struct fd6_emit *emit);

void
fd6_emit_cs_driver_params(struct fd_context *ctx,
                          struct fd_ringbuffer *ring,
                          struct fd6_compute_state *cs,
                          const struct pipe_grid_info *info)
{
   ir3_emit_cs_driver_params(cs->v, ring, ctx, info);
}

void
fd6_emit_cs_user_consts(struct fd_context *ctx,
                        struct fd_ringbuffer *ring,
                        struct fd6_compute_state *cs)
{
   emit_user_consts(cs->v, ring, &ctx->constbuf[PIPE_SHADER_COMPUTE]);
}

void
fd6_emit_immediates(const struct ir3_shader_variant *v,
                    struct fd_ringbuffer *ring)
{
   ir3_emit_immediates(v, ring);
}

void
fd6_emit_link_map(const struct ir3_shader_variant *producer,
                  const struct ir3_shader_variant *consumer,
                  struct fd_ringbuffer *ring)
{
   ir3_emit_link_map(producer, consumer, ring);
}
