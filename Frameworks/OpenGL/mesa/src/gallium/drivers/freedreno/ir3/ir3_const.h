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

#include "ir3/ir3_nir.h"

/* This has to reach into the fd_context a bit more than the rest of
 * ir3, but it needs to be aligned with the compiler, so both agree
 * on which const regs hold what.  And the logic is identical between
 * ir3 generations, the only difference is small details in the actual
 * CP_LOAD_STATE packets (which is handled inside the generation
 * specific ctx->emit_const(_bo)() fxns)
 *
 * This file should be included in only a single .c file per gen, which
 * defines the following functions:
 */

static bool is_stateobj(struct fd_ringbuffer *ring);

static void emit_const_user(struct fd_ringbuffer *ring,
                            const struct ir3_shader_variant *v, uint32_t regid,
                            uint32_t size, const uint32_t *user_buffer);

static void emit_const_bo(struct fd_ringbuffer *ring,
                          const struct ir3_shader_variant *v, uint32_t regid,
                          uint32_t offset, uint32_t size, struct fd_bo *bo);

static void
emit_const_prsc(struct fd_ringbuffer *ring, const struct ir3_shader_variant *v,
                uint32_t regid, uint32_t offset, uint32_t size,
                struct pipe_resource *buffer)
{
   struct fd_resource *rsc = fd_resource(buffer);
   emit_const_bo(ring, v, regid, offset, size, rsc->bo);
}

static void emit_const_ptrs(struct fd_ringbuffer *ring,
                            const struct ir3_shader_variant *v,
                            uint32_t dst_offset, uint32_t num,
                            struct fd_bo **bos, uint32_t *offsets);

static void
emit_const_asserts(struct fd_ringbuffer *ring,
                   const struct ir3_shader_variant *v, uint32_t regid,
                   uint32_t sizedwords)
{
   assert((regid % 4) == 0);
   assert((sizedwords % 4) == 0);
   assert(regid + sizedwords <= v->constlen * 4);
}

static void
ring_wfi(struct fd_batch *batch, struct fd_ringbuffer *ring) assert_dt
{
   /* when we emit const state via ring (IB2) we need a WFI, but when
    * it is emit'd via stateobj, we don't
    */
   if (is_stateobj(ring))
      return;

   fd_wfi(batch, ring);
}

/**
 * Indirectly calculates size of cmdstream needed for ir3_emit_user_consts().
 * Returns number of packets, and total size of all the payload.
 *
 * The value can be a worst-case, ie. some shader variants may not read all
 * consts, etc.
 *
 * Returns size in dwords.
 */
static inline void
ir3_user_consts_size(struct ir3_ubo_analysis_state *state, unsigned *packets,
                     unsigned *size)
{
   *packets = *size = 0;

   for (uint32_t i = 0; i < ARRAY_SIZE(state->range); i++) {
      if (state->range[i].start < state->range[i].end) {
         *size += state->range[i].end - state->range[i].start;
         (*packets)++;
      }
   }
}

/**
 * Uploads the referenced subranges of the nir constant_data to the hardware's
 * constant buffer.
 */
static inline void
ir3_emit_constant_data(const struct ir3_shader_variant *v,
                       struct fd_ringbuffer *ring)
{
   const struct ir3_const_state *const_state = ir3_const_state(v);
   const struct ir3_ubo_analysis_state *state = &const_state->ubo_state;

   for (unsigned i = 0; i < state->num_enabled; i++) {
      unsigned ubo = state->range[i].ubo.block;
      if (ubo != const_state->constant_data_ubo)
         continue;

      uint32_t size = state->range[i].end - state->range[i].start;

      /* Pre-a6xx, we might have ranges enabled in the shader that aren't
       * used in the binning variant.
       */
      if (16 * v->constlen <= state->range[i].offset)
         continue;

      /* and even if the start of the const buffer is before
       * first_immediate, the end may not be:
       */
      size = MIN2(size, (16 * v->constlen) - state->range[i].offset);

      if (size == 0)
         continue;

      emit_const_bo(ring, v, state->range[i].offset / 4,
                    v->info.constant_data_offset + state->range[i].start,
                    size / 4, v->bo);
   }
}

/**
 * Uploads sub-ranges of UBOs to the hardware's constant buffer (UBO access
 * outside of these ranges will be done using full UBO accesses in the
 * shader).
 */
static inline void
ir3_emit_user_consts(const struct ir3_shader_variant *v,
                     struct fd_ringbuffer *ring,
                     struct fd_constbuf_stateobj *constbuf)
{
   const struct ir3_const_state *const_state = ir3_const_state(v);
   const struct ir3_ubo_analysis_state *state = &const_state->ubo_state;

   for (unsigned i = 0; i < state->num_enabled; i++) {
      assert(!state->range[i].ubo.bindless);
      unsigned ubo = state->range[i].ubo.block;
      if (!(constbuf->enabled_mask & (1 << ubo)) ||
          ubo == const_state->constant_data_ubo) {
         continue;
      }
      struct pipe_constant_buffer *cb = &constbuf->cb[ubo];

      uint32_t size = state->range[i].end - state->range[i].start;
      uint32_t offset = cb->buffer_offset + state->range[i].start;

      /* Pre-a6xx, we might have ranges enabled in the shader that aren't
       * used in the binning variant.
       */
      if (16 * v->constlen <= state->range[i].offset)
         continue;

      /* and even if the start of the const buffer is before
       * first_immediate, the end may not be:
       */
      size = MIN2(size, (16 * v->constlen) - state->range[i].offset);

      if (size == 0)
         continue;

      /* things should be aligned to vec4: */
      assert((state->range[i].offset % 16) == 0);
      assert((size % 16) == 0);
      assert((offset % 16) == 0);

      if (cb->user_buffer) {
         uint8_t *p = (uint8_t *)cb->user_buffer;
         p += state->range[i].start;
         emit_const_user(ring, v, state->range[i].offset / 4, size / 4, (uint32_t *)p);
      } else {
         emit_const_prsc(ring, v, state->range[i].offset / 4, offset, size / 4,
                         cb->buffer);
      }
   }
}

static inline void
ir3_emit_ubos(struct fd_context *ctx, const struct ir3_shader_variant *v,
              struct fd_ringbuffer *ring, struct fd_constbuf_stateobj *constbuf)
{
   const struct ir3_const_state *const_state = ir3_const_state(v);
   uint32_t offset = const_state->offsets.ubo;

   /* a6xx+ uses UBO state and ldc instead of pointers emitted in
    * const state and ldg:
    */
   if (ctx->screen->gen >= 6)
      return;

   if (v->constlen > offset) {
      uint32_t params = const_state->num_ubos;
      uint32_t offsets[params];
      struct fd_bo *bos[params];

      for (uint32_t i = 0; i < params; i++) {
         if (i == const_state->constant_data_ubo) {
            bos[i] = v->bo;
            offsets[i] = v->info.constant_data_offset;
            continue;
         }

         struct pipe_constant_buffer *cb = &constbuf->cb[i];

         /* If we have user pointers (constbuf 0, aka GL uniforms), upload
          * them to a buffer now, and save it in the constbuf so that we
          * don't have to reupload until they get changed.
          */
         if (cb->user_buffer) {
            struct pipe_context *pctx = &ctx->base;
            u_upload_data(pctx->stream_uploader, 0, cb->buffer_size, 64,
                          cb->user_buffer, &cb->buffer_offset, &cb->buffer);
            cb->user_buffer = NULL;
         }

         if ((constbuf->enabled_mask & (1 << i)) && cb->buffer) {
            offsets[i] = cb->buffer_offset;
            bos[i] = fd_resource(cb->buffer)->bo;
         } else {
            offsets[i] = 0;
            bos[i] = NULL;
         }
      }

      assert(offset * 4 + params <= v->constlen * 4);

      emit_const_ptrs(ring, v, offset * 4, params, bos, offsets);
   }
}

static inline void
ir3_emit_image_dims(struct fd_screen *screen,
                    const struct ir3_shader_variant *v,
                    struct fd_ringbuffer *ring,
                    struct fd_shaderimg_stateobj *si)
{
   const struct ir3_const_state *const_state = ir3_const_state(v);
   uint32_t offset = const_state->offsets.image_dims;
   if (v->constlen > offset) {
      uint32_t dims[align(const_state->image_dims.count, 4)];
      unsigned mask = const_state->image_dims.mask;

      while (mask) {
         struct pipe_image_view *img;
         struct fd_resource *rsc;
         unsigned index = u_bit_scan(&mask);
         unsigned off = const_state->image_dims.off[index];

         img = &si->si[index];
         rsc = fd_resource(img->resource);

         dims[off + 0] = util_format_get_blocksize(img->format);
         if (img->resource->target != PIPE_BUFFER) {
            struct fdl_slice *slice = fd_resource_slice(rsc, img->u.tex.level);
            /* note for 2d/cube/etc images, even if re-interpreted
             * as a different color format, the pixel size should
             * be the same, so use original dimensions for y and z
             * stride:
             */
            dims[off + 1] = fd_resource_pitch(rsc, img->u.tex.level);
            /* see corresponding logic in fd_resource_offset(): */
            if (rsc->layout.layer_first) {
               dims[off + 2] = rsc->layout.layer_size;
            } else {
               dims[off + 2] = slice->size0;
            }
         } else {
            /* For buffer-backed images, the log2 of the format's
             * bytes-per-pixel is placed on the 2nd slot. This is useful
             * when emitting image_size instructions, for which we need
             * to divide by bpp for image buffers. Since the bpp
             * can only be power-of-two, the division is implemented
             * as a SHR, and for that it is handy to have the log2 of
             * bpp as a constant. (log2 = first-set-bit - 1)
             */
            dims[off + 1] = ffs(dims[off + 0]) - 1;
         }
      }
      uint32_t size = MIN2(ARRAY_SIZE(dims), v->constlen * 4 - offset * 4);

      emit_const_user(ring, v, offset * 4, size, dims);
   }
}

static inline void
ir3_emit_immediates(const struct ir3_shader_variant *v,
                    struct fd_ringbuffer *ring)
{
   const struct ir3_const_state *const_state = ir3_const_state(v);
   uint32_t base = const_state->offsets.immediate;
   int size = DIV_ROUND_UP(const_state->immediates_count, 4);

   /* truncate size to avoid writing constants that shader
    * does not use:
    */
   size = MIN2(size + base, v->constlen) - base;

   /* convert out of vec4: */
   base *= 4;
   size *= 4;

   if (size > 0)
      emit_const_user(ring, v, base, size, const_state->immediates);

   /* NIR constant data has the same lifetime as immediates, so upload it
    * now, too.
    */
   ir3_emit_constant_data(v, ring);
}

static inline void
ir3_emit_link_map(const struct ir3_shader_variant *producer,
                  const struct ir3_shader_variant *consumer,
                  struct fd_ringbuffer *ring)
{
   const struct ir3_const_state *const_state = ir3_const_state(consumer);
   uint32_t base = const_state->offsets.primitive_map;
   int size = DIV_ROUND_UP(consumer->input_size, 4);

   /* truncate size to avoid writing constants that shader
    * does not use:
    */
   size = MIN2(size + base, consumer->constlen) - base;

   /* convert out of vec4: */
   base *= 4;
   size *= 4;

   if (size > 0)
      emit_const_user(ring, consumer, base, size, producer->output_loc);
}

/* emit stream-out buffers: */
static inline void
emit_tfbos(struct fd_context *ctx, const struct ir3_shader_variant *v,
           struct fd_ringbuffer *ring)
{
   /* streamout addresses after driver-params: */
   const struct ir3_const_state *const_state = ir3_const_state(v);
   uint32_t offset = const_state->offsets.tfbo;
   if (v->constlen > offset) {
      struct fd_streamout_stateobj *so = &ctx->streamout;
      const struct ir3_stream_output_info *info = &v->stream_output;
      uint32_t params = 4;
      uint32_t offsets[params];
      struct fd_bo *bos[params];

      for (uint32_t i = 0; i < params; i++) {
         struct pipe_stream_output_target *target = so->targets[i];

         if (target) {
            offsets[i] =
               (so->offsets[i] * info->stride[i] * 4) + target->buffer_offset;
            bos[i] = fd_resource(target->buffer)->bo;
         } else {
            offsets[i] = 0;
            bos[i] = NULL;
         }
      }

      assert(offset * 4 + params <= v->constlen * 4);

      emit_const_ptrs(ring, v, offset * 4, params, bos, offsets);
   }
}

static inline void
emit_common_consts(const struct ir3_shader_variant *v,
                   struct fd_ringbuffer *ring, struct fd_context *ctx,
                   enum pipe_shader_type t) assert_dt
{
   enum fd_dirty_shader_state dirty = ctx->dirty_shader[t];

   /* When we use CP_SET_DRAW_STATE objects to emit constant state,
    * if we emit any of it we need to emit all.  This is because
    * we are using the same state-group-id each time for uniform
    * state, and if previous update is never evaluated (due to no
    * visible primitives in the current tile) then the new stateobj
    * completely replaces the old one.
    *
    * Possibly if we split up different parts of the const state to
    * different state-objects we could avoid this.
    */
   if (dirty && is_stateobj(ring))
      dirty = (enum fd_dirty_shader_state)~0;

   if (dirty & (FD_DIRTY_SHADER_PROG | FD_DIRTY_SHADER_CONST)) {
      struct fd_constbuf_stateobj *constbuf;
      bool shader_dirty;

      constbuf = &ctx->constbuf[t];
      shader_dirty = !!(dirty & FD_DIRTY_SHADER_PROG);

      ring_wfi(ctx->batch, ring);

      ir3_emit_user_consts(v, ring, constbuf);
      ir3_emit_ubos(ctx, v, ring, constbuf);
      if (shader_dirty)
         ir3_emit_immediates(v, ring);
   }

   if (dirty & (FD_DIRTY_SHADER_PROG | FD_DIRTY_SHADER_IMAGE)) {
      struct fd_shaderimg_stateobj *si = &ctx->shaderimg[t];
      ring_wfi(ctx->batch, ring);
      ir3_emit_image_dims(ctx->screen, v, ring, si);
   }
}

/* emit kernel params */
static inline void
emit_kernel_params(struct fd_context *ctx, const struct ir3_shader_variant *v,
                   struct fd_ringbuffer *ring, const struct pipe_grid_info *info)
   assert_dt
{
   const struct ir3_const_state *const_state = ir3_const_state(v);
   uint32_t offset = const_state->offsets.kernel_params;
   if (v->constlen > offset) {
      ring_wfi(ctx->batch, ring);
      emit_const_user(ring, v, offset * 4,
                      align(v->cs.req_input_mem, 4),
                      (uint32_t *)info->input);
   }
}

static inline void
ir3_emit_driver_params(const struct ir3_shader_variant *v,
                       struct fd_ringbuffer *ring, struct fd_context *ctx,
                       const struct pipe_draw_info *info,
                       const struct pipe_draw_indirect_info *indirect,
                       const struct pipe_draw_start_count_bias *draw,
                       const uint32_t draw_id) assert_dt
{
   assert(v->need_driver_params);

   const struct ir3_const_state *const_state = ir3_const_state(v);
   uint32_t offset = const_state->offsets.driver_param;
   uint32_t vertex_params[IR3_DP_VS_COUNT] = {
      [IR3_DP_DRAWID] = draw_id, /* filled by hw (CP_DRAW_INDIRECT_MULTI) */
      [IR3_DP_VTXID_BASE] = info->index_size ? draw->index_bias : draw->start,
      [IR3_DP_INSTID_BASE] = info->start_instance,
      [IR3_DP_VTXCNT_MAX] = ctx->streamout.max_tf_vtx,
      [IR3_DP_IS_INDEXED_DRAW] = info->index_size != 0 ? ~0 : 0,
   };
   if (v->key.ucp_enables) {
      struct pipe_clip_state *ucp = &ctx->ucp;
      unsigned pos = IR3_DP_UCP0_X;
      for (unsigned i = 0; pos <= IR3_DP_UCP7_W; i++) {
         for (unsigned j = 0; j < 4; j++) {
            vertex_params[pos] = fui(ucp->ucp[i][j]);
            pos++;
         }
      }
   }

   /* Only emit as many params as needed, i.e. up to the highest enabled UCP
    * plane. However a binning pass may drop even some of these, so limit to
    * program max.
    */
   const uint32_t vertex_params_size =
      MIN2(const_state->num_driver_params, (v->constlen - offset) * 4);
   assert(vertex_params_size <= IR3_DP_VS_COUNT);

   bool needs_vtxid_base =
      ir3_find_sysval_regid(v, SYSTEM_VALUE_VERTEX_ID_ZERO_BASE) !=
      regid(63, 0);

   /* for indirect draw, we need to copy VTXID_BASE from
    * indirect-draw parameters buffer.. which is annoying
    * and means we can't easily emit these consts in cmd
    * stream so need to copy them to bo.
    */
   if (indirect && needs_vtxid_base) {
      uint32_t vertex_params_area = align(vertex_params_size, 16);
      struct pipe_resource *vertex_params_rsc =
         pipe_buffer_create(&ctx->screen->base, PIPE_BIND_CONSTANT_BUFFER,
                            PIPE_USAGE_STREAM, vertex_params_area * 4);
      unsigned src_off = indirect->offset;
      void *ptr;

      ptr = fd_bo_map(fd_resource(vertex_params_rsc)->bo);
      memcpy(ptr, vertex_params, vertex_params_size * 4);

      if (info->index_size) {
         /* indexed draw, index_bias is 4th field: */
         src_off += 3 * 4;
      } else {
         /* non-indexed draw, start is 3rd field: */
         src_off += 2 * 4;
      }

      /* copy index_bias or start from draw params: */
      ctx->screen->mem_to_mem(ring, vertex_params_rsc, 0, indirect->buffer,
                              src_off, 1);

      emit_const_prsc(ring, v, offset * 4, 0, vertex_params_area,
                      vertex_params_rsc);

      pipe_resource_reference(&vertex_params_rsc, NULL);
   } else {
      emit_const_user(ring, v, offset * 4, vertex_params_size, vertex_params);
   }

   /* if needed, emit stream-out buffer addresses: */
   if (vertex_params[IR3_DP_VTXCNT_MAX] > 0) {
      emit_tfbos(ctx, v, ring);
   }
}


static inline void
ir3_emit_hs_driver_params(const struct ir3_shader_variant *v,
                          struct fd_ringbuffer *ring,
                          struct fd_context *ctx)
   assert_dt
{
   assert(v->need_driver_params);

   const struct ir3_const_state *const_state = ir3_const_state(v);
   uint32_t offset = const_state->offsets.driver_param;
   uint32_t hs_params[IR3_DP_HS_COUNT] = {
      [IR3_DP_HS_DEFAULT_OUTER_LEVEL_X] = fui(ctx->default_outer_level[0]),
      [IR3_DP_HS_DEFAULT_OUTER_LEVEL_Y] = fui(ctx->default_outer_level[1]),
      [IR3_DP_HS_DEFAULT_OUTER_LEVEL_Z] = fui(ctx->default_outer_level[2]),
      [IR3_DP_HS_DEFAULT_OUTER_LEVEL_W] = fui(ctx->default_outer_level[3]),
      [IR3_DP_HS_DEFAULT_INNER_LEVEL_X] = fui(ctx->default_inner_level[0]),
      [IR3_DP_HS_DEFAULT_INNER_LEVEL_Y] = fui(ctx->default_inner_level[1]),
   };

   const uint32_t hs_params_size =
      MIN2(const_state->num_driver_params, (v->constlen - offset) * 4);
   assert(hs_params_size <= IR3_DP_HS_COUNT);

   emit_const_user(ring, v, offset * 4, hs_params_size, hs_params);
}


static inline void
ir3_emit_vs_consts(const struct ir3_shader_variant *v,
                   struct fd_ringbuffer *ring, struct fd_context *ctx,
                   const struct pipe_draw_info *info,
                   const struct pipe_draw_indirect_info *indirect,
                   const struct pipe_draw_start_count_bias *draw) assert_dt
{
   assert(v->type == MESA_SHADER_VERTEX);

   emit_common_consts(v, ring, ctx, PIPE_SHADER_VERTEX);

   /* emit driver params every time: */
   if (info && v->need_driver_params) {
      ring_wfi(ctx->batch, ring);
      ir3_emit_driver_params(v, ring, ctx, info, indirect, draw, 0);
   }
}

static inline void
ir3_emit_fs_consts(const struct ir3_shader_variant *v,
                   struct fd_ringbuffer *ring, struct fd_context *ctx) assert_dt
{
   assert(v->type == MESA_SHADER_FRAGMENT);

   emit_common_consts(v, ring, ctx, PIPE_SHADER_FRAGMENT);
}

static inline void
ir3_emit_cs_driver_params(const struct ir3_shader_variant *v,
                          struct fd_ringbuffer *ring, struct fd_context *ctx,
                          const struct pipe_grid_info *info)
   assert_dt
{
   emit_kernel_params(ctx, v, ring, info);

   /* a3xx/a4xx can inject these directly */
   if (ctx->screen->gen <= 4)
      return;

   /* emit compute-shader driver-params: */
   const struct ir3_const_state *const_state = ir3_const_state(v);
   uint32_t offset = const_state->offsets.driver_param;
   if (v->constlen > offset) {
      ring_wfi(ctx->batch, ring);

      if (info->indirect) {
         struct pipe_resource *indirect = NULL;
         unsigned indirect_offset;

         /* This is a bit awkward, but CP_LOAD_STATE.EXT_SRC_ADDR needs
          * to be aligned more strongly than 4 bytes.  So in this case
          * we need a temporary buffer to copy NumWorkGroups.xyz to.
          *
          * TODO if previous compute job is writing to info->indirect,
          * we might need a WFI.. but since we currently flush for each
          * compute job, we are probably ok for now.
          */
         if (info->indirect_offset & 0xf) {
            indirect = pipe_buffer_create(&ctx->screen->base,
                                          PIPE_BIND_COMMAND_ARGS_BUFFER,
                                          PIPE_USAGE_STREAM, 0x1000);
            indirect_offset = 0;

            ctx->screen->mem_to_mem(ring, indirect, 0, info->indirect,
                                    info->indirect_offset, 3);
         } else {
            pipe_resource_reference(&indirect, info->indirect);
            indirect_offset = info->indirect_offset;
         }

         emit_const_prsc(ring, v, offset * 4, indirect_offset, 16, indirect);

         pipe_resource_reference(&indirect, NULL);
      } else {
         // TODO some of these are not part of the indirect state.. so we
         // need to emit some of this directly in both cases.
         uint32_t compute_params[IR3_DP_CS_COUNT] = {
            [IR3_DP_NUM_WORK_GROUPS_X] = info->grid[0],
            [IR3_DP_NUM_WORK_GROUPS_Y] = info->grid[1],
            [IR3_DP_NUM_WORK_GROUPS_Z] = info->grid[2],
            [IR3_DP_WORK_DIM]          = info->work_dim,
            [IR3_DP_BASE_GROUP_X]      = info->grid_base[0],
            [IR3_DP_BASE_GROUP_Y]      = info->grid_base[1],
            [IR3_DP_BASE_GROUP_Z]      = info->grid_base[2],
            [IR3_DP_CS_SUBGROUP_SIZE]  = v->info.subgroup_size,
            [IR3_DP_LOCAL_GROUP_SIZE_X] = info->block[0],
            [IR3_DP_LOCAL_GROUP_SIZE_Y] = info->block[1],
            [IR3_DP_LOCAL_GROUP_SIZE_Z] = info->block[2],
            [IR3_DP_SUBGROUP_ID_SHIFT] = util_logbase2(v->info.subgroup_size),
            [IR3_DP_WORKGROUP_ID_X]    = 0,  // TODO
            [IR3_DP_WORKGROUP_ID_Y]    = 0,  // TODO
            [IR3_DP_WORKGROUP_ID_Z]    = 0,  // TODO
         };
         uint32_t size =
            MIN2(const_state->num_driver_params, v->constlen * 4 - offset * 4);

         emit_const_user(ring, v, offset * 4, size, compute_params);
      }
   }
}

/* emit compute-shader consts: */
static inline void
ir3_emit_cs_consts(const struct ir3_shader_variant *v,
                   struct fd_ringbuffer *ring, struct fd_context *ctx,
                   const struct pipe_grid_info *info) assert_dt
{
   assert(gl_shader_stage_is_compute(v->type));

   emit_common_consts(v, ring, ctx, PIPE_SHADER_COMPUTE);

   ir3_emit_cs_driver_params(v, ring, ctx, info);
}
