/*
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_build_pm4.h"
#include "util/u_memory.h"
#include "util/u_suballoc.h"

static void si_set_streamout_enable(struct si_context *sctx, bool enable);

static inline void si_so_target_reference(struct si_streamout_target **dst,
                                          struct pipe_stream_output_target *src)
{
   pipe_so_target_reference((struct pipe_stream_output_target **)dst, src);
}

static struct pipe_stream_output_target *si_create_so_target(struct pipe_context *ctx,
                                                             struct pipe_resource *buffer,
                                                             unsigned buffer_offset,
                                                             unsigned buffer_size)
{
   struct si_streamout_target *t;
   struct si_resource *buf = si_resource(buffer);

   t = CALLOC_STRUCT(si_streamout_target);
   if (!t) {
      return NULL;
   }

   t->b.reference.count = 1;
   t->b.context = ctx;
   pipe_resource_reference(&t->b.buffer, buffer);
   t->b.buffer_offset = buffer_offset;
   t->b.buffer_size = buffer_size;

   util_range_add(&buf->b.b, &buf->valid_buffer_range, buffer_offset, buffer_offset + buffer_size);
   return &t->b;
}

static void si_so_target_destroy(struct pipe_context *ctx, struct pipe_stream_output_target *target)
{
   struct si_streamout_target *t = (struct si_streamout_target *)target;
   pipe_resource_reference(&t->b.buffer, NULL);
   si_resource_reference(&t->buf_filled_size, NULL);
   FREE(t);
}

void si_streamout_buffers_dirty(struct si_context *sctx)
{
   if (!sctx->streamout.enabled_mask)
      return;

   si_mark_atom_dirty(sctx, &sctx->atoms.s.streamout_begin);
   si_set_streamout_enable(sctx, true);
}

static void si_set_streamout_targets(struct pipe_context *ctx, unsigned num_targets,
                                     struct pipe_stream_output_target **targets,
                                     const unsigned *offsets)
{
   struct si_context *sctx = (struct si_context *)ctx;
   unsigned old_num_targets = sctx->streamout.num_targets;
   unsigned i;

   /* We are going to unbind the buffers. Mark which caches need to be flushed. */
   if (old_num_targets && sctx->streamout.begin_emitted) {
      /* Stop streamout. */
      si_emit_streamout_end(sctx);

      /* Since streamout uses vector writes which go through TC L2
       * and most other clients can use TC L2 as well, we don't need
       * to flush it.
       *
       * The only cases which requires flushing it is VGT DMA index
       * fetching (on <= GFX7) and indirect draw data, which are rare
       * cases. Thus, flag the TC L2 dirtiness in the resource and
       * handle it at draw call time.
       */
      for (i = 0; i < old_num_targets; i++)
         if (sctx->streamout.targets[i])
            si_resource(sctx->streamout.targets[i]->b.buffer)->TC_L2_dirty = true;

      /* Invalidate the scalar cache in case a streamout buffer is
       * going to be used as a constant buffer.
       *
       * Invalidate vL1, because streamout bypasses it (done by
       * setting GLC=1 in the store instruction), but vL1 in other
       * CUs can contain outdated data of streamout buffers.
       *
       * VS_PARTIAL_FLUSH is required if the buffers are going to be
       * used as an input immediately.
       */
      sctx->flags |= SI_CONTEXT_INV_SCACHE | SI_CONTEXT_INV_VCACHE |
                     SI_CONTEXT_VS_PARTIAL_FLUSH | SI_CONTEXT_PFP_SYNC_ME;
      si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
   }

   /* TODO: This is a hack that fixes these failures. It shouldn't be necessary.
    *    spec@ext_transform_feedback@immediate-reuse
    *    spec@ext_transform_feedback@immediate-reuse-index-buffer
    *    spec@ext_transform_feedback@immediate-reuse-uniform-buffer
    *    .. and some dEQP-GLES[23].functional.fragment_ops.random.*
    */
   if (sctx->gfx_level >= GFX11)
      si_flush_gfx_cs(sctx, 0, NULL);

   /* Streamout buffers must be bound in 2 places:
    * 1) in VGT by setting the VGT_STRMOUT registers
    * 2) as shader resources
    */
   unsigned enabled_mask = 0, append_bitmask = 0;

   for (i = 0; i < num_targets; i++) {
      si_so_target_reference(&sctx->streamout.targets[i], targets[i]);

      if (!targets[i]) {
         si_set_internal_shader_buffer(sctx, SI_VS_STREAMOUT_BUF0 + i, NULL);
         continue;
      }

      enabled_mask |= 1 << i;

      if (offsets[i] == ((unsigned)-1))
         append_bitmask |= 1 << i;

      /* Allocate space for the filled buffer size. */
      struct si_streamout_target *t = sctx->streamout.targets[i];
      if (!t->buf_filled_size) {
         unsigned buf_filled_size_size = sctx->gfx_level >= GFX11 ? 8 : 4;
         u_suballocator_alloc(&sctx->allocator_zeroed_memory, buf_filled_size_size, 4,
                              &t->buf_filled_size_offset,
                              (struct pipe_resource **)&t->buf_filled_size);
      }

      /* Bind it to the shader. */
      struct pipe_shader_buffer sbuf;
      sbuf.buffer = targets[i]->buffer;

      if (sctx->gfx_level >= GFX11) {
         sbuf.buffer_offset = targets[i]->buffer_offset;
         sbuf.buffer_size = targets[i]->buffer_size;
      } else {
         sbuf.buffer_offset = 0;
         sbuf.buffer_size = targets[i]->buffer_offset + targets[i]->buffer_size;
      }

      si_set_internal_shader_buffer(sctx, SI_VS_STREAMOUT_BUF0 + i, &sbuf);
      si_resource(targets[i]->buffer)->bind_history |= SI_BIND_STREAMOUT_BUFFER;
   }
   for (; i < old_num_targets; i++) {
      si_so_target_reference(&sctx->streamout.targets[i], NULL);
      si_set_internal_shader_buffer(sctx, SI_VS_STREAMOUT_BUF0 + i, NULL);
   }

   if (!!sctx->streamout.enabled_mask != !!enabled_mask)
      sctx->do_update_shaders = true; /* to keep/remove streamout shader code as an optimization */

   sctx->streamout.num_targets = num_targets;
   sctx->streamout.enabled_mask = enabled_mask;
   sctx->streamout.append_bitmask = append_bitmask;

   /* Update dirty state bits. */
   if (num_targets) {
      si_streamout_buffers_dirty(sctx);

      /* All readers of the streamout targets need to be finished before we can
       * start writing to them.
       */
      sctx->flags |= SI_CONTEXT_PS_PARTIAL_FLUSH | SI_CONTEXT_CS_PARTIAL_FLUSH |
                     SI_CONTEXT_PFP_SYNC_ME;
      si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
   } else {
      si_set_atom_dirty(sctx, &sctx->atoms.s.streamout_begin, false);
      si_set_streamout_enable(sctx, false);
   }
}

static void si_flush_vgt_streamout(struct si_context *sctx)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   unsigned reg_strmout_cntl;

   radeon_begin(cs);

   /* The register is at different places on different ASICs. */
   if (sctx->gfx_level >= GFX9) {
      reg_strmout_cntl = R_0300FC_CP_STRMOUT_CNTL;
      radeon_emit(PKT3(PKT3_WRITE_DATA, 3, 0));
      radeon_emit(S_370_DST_SEL(V_370_MEM_MAPPED_REGISTER) | S_370_ENGINE_SEL(V_370_ME));
      radeon_emit(R_0300FC_CP_STRMOUT_CNTL >> 2);
      radeon_emit(0);
      radeon_emit(0);
   } else if (sctx->gfx_level >= GFX7) {
      reg_strmout_cntl = R_0300FC_CP_STRMOUT_CNTL;
      radeon_set_uconfig_reg(reg_strmout_cntl, 0);
   } else {
      reg_strmout_cntl = R_0084FC_CP_STRMOUT_CNTL;
      radeon_set_config_reg(reg_strmout_cntl, 0);
   }

   radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
   radeon_emit(EVENT_TYPE(V_028A90_SO_VGTSTREAMOUT_FLUSH) | EVENT_INDEX(0));

   radeon_emit(PKT3(PKT3_WAIT_REG_MEM, 5, 0));
   radeon_emit(WAIT_REG_MEM_EQUAL); /* wait until the register is equal to the reference value */
   radeon_emit(reg_strmout_cntl >> 2); /* register */
   radeon_emit(0);
   radeon_emit(S_0084FC_OFFSET_UPDATE_DONE(1)); /* reference value */
   radeon_emit(S_0084FC_OFFSET_UPDATE_DONE(1)); /* mask */
   radeon_emit(4);                              /* poll interval */
   radeon_end();
}

static void si_emit_streamout_begin(struct si_context *sctx, unsigned index)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   struct si_streamout_target **t = sctx->streamout.targets;

   if (sctx->gfx_level < GFX11)
      si_flush_vgt_streamout(sctx);

   for (unsigned i = 0; i < sctx->streamout.num_targets; i++) {
      if (!t[i])
         continue;

      t[i]->stride_in_dw = sctx->streamout.stride_in_dw[i];

      if (sctx->gfx_level >= GFX11) {
         if (sctx->streamout.append_bitmask & (1 << i)) {
            /* Restore the register value. */
            si_cp_copy_data(sctx, cs, COPY_DATA_REG, NULL,
                            (R_031088_GDS_STRMOUT_DWORDS_WRITTEN_0 / 4) + i,
                            COPY_DATA_SRC_MEM, t[i]->buf_filled_size,
                            t[i]->buf_filled_size_offset);
         } else {
            /* Set to 0. */
            radeon_begin(cs);
            radeon_set_uconfig_reg(R_031088_GDS_STRMOUT_DWORDS_WRITTEN_0 + i * 4, 0);
            radeon_end();
         }
      } else {
         /* Legacy streamout.
          *
          * The hw binds streamout buffers as shader resources. VGT only counts primitives
          * and tells the shader through SGPRs what to do.
          */
         radeon_begin(cs);
         radeon_set_context_reg_seq(R_028AD0_VGT_STRMOUT_BUFFER_SIZE_0 + 16 * i, 2);
         radeon_emit((t[i]->b.buffer_offset + t[i]->b.buffer_size) >> 2); /* BUFFER_SIZE (in DW) */
         radeon_emit(sctx->streamout.stride_in_dw[i]);                                    /* VTX_STRIDE (in DW) */

         if (sctx->streamout.append_bitmask & (1 << i) && t[i]->buf_filled_size_valid) {
            uint64_t va = t[i]->buf_filled_size->gpu_address + t[i]->buf_filled_size_offset;

            /* Append. */
            radeon_emit(PKT3(PKT3_STRMOUT_BUFFER_UPDATE, 4, 0));
            radeon_emit(STRMOUT_SELECT_BUFFER(i) |
                        STRMOUT_OFFSET_SOURCE(STRMOUT_OFFSET_FROM_MEM)); /* control */
            radeon_emit(0);                                              /* unused */
            radeon_emit(0);                                              /* unused */
            radeon_emit(va);                                             /* src address lo */
            radeon_emit(va >> 32);                                       /* src address hi */

            radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, t[i]->buf_filled_size,
                                      RADEON_USAGE_READ | RADEON_PRIO_SO_FILLED_SIZE);
         } else {
            /* Start from the beginning. */
            radeon_emit(PKT3(PKT3_STRMOUT_BUFFER_UPDATE, 4, 0));
            radeon_emit(STRMOUT_SELECT_BUFFER(i) |
                        STRMOUT_OFFSET_SOURCE(STRMOUT_OFFSET_FROM_PACKET)); /* control */
            radeon_emit(0);                                                 /* unused */
            radeon_emit(0);                                                 /* unused */
            radeon_emit(t[i]->b.buffer_offset >> 2); /* buffer offset in DW */
            radeon_emit(0);                          /* unused */
         }
         radeon_end_update_context_roll(sctx);
      }
   }

   sctx->streamout.begin_emitted = true;
}

void si_emit_streamout_end(struct si_context *sctx)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   struct si_streamout_target **t = sctx->streamout.targets;

   if (sctx->gfx_level >= GFX11) {
      /* Wait for streamout to finish before reading GDS_STRMOUT registers. */
      sctx->flags |= SI_CONTEXT_VS_PARTIAL_FLUSH;
      si_emit_cache_flush_direct(sctx);
   } else {
      si_flush_vgt_streamout(sctx);
   }

   for (unsigned i = 0; i < sctx->streamout.num_targets; i++) {
      if (!t[i])
         continue;

      if (sctx->gfx_level >= GFX11) {
         si_cp_copy_data(sctx, &sctx->gfx_cs, COPY_DATA_DST_MEM,
                         t[i]->buf_filled_size, t[i]->buf_filled_size_offset,
                         COPY_DATA_REG, NULL,
                         (R_031088_GDS_STRMOUT_DWORDS_WRITTEN_0 >> 2) + i);
         sctx->flags |= SI_CONTEXT_PFP_SYNC_ME;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
      } else {
         uint64_t va = t[i]->buf_filled_size->gpu_address + t[i]->buf_filled_size_offset;

         radeon_begin(cs);
         radeon_emit(PKT3(PKT3_STRMOUT_BUFFER_UPDATE, 4, 0));
         radeon_emit(STRMOUT_SELECT_BUFFER(i) | STRMOUT_OFFSET_SOURCE(STRMOUT_OFFSET_NONE) |
                     STRMOUT_STORE_BUFFER_FILLED_SIZE); /* control */
         radeon_emit(va);                                  /* dst address lo */
         radeon_emit(va >> 32);                            /* dst address hi */
         radeon_emit(0);                                   /* unused */
         radeon_emit(0);                                   /* unused */

         /* Zero the buffer size. The counters (primitives generated,
          * primitives emitted) may be enabled even if there is not
          * buffer bound. This ensures that the primitives-emitted query
          * won't increment. */
         radeon_set_context_reg(R_028AD0_VGT_STRMOUT_BUFFER_SIZE_0 + 16 * i, 0);
         radeon_end_update_context_roll(sctx);

         radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, t[i]->buf_filled_size,
                                   RADEON_USAGE_WRITE | RADEON_PRIO_SO_FILLED_SIZE);
      }

      t[i]->buf_filled_size_valid = true;
   }

   sctx->streamout.begin_emitted = false;
}

/* STREAMOUT CONFIG DERIVED STATE
 *
 * Streamout must be enabled for the PRIMITIVES_GENERATED query to work.
 * The buffer mask is an independent state, so no writes occur if there
 * are no buffers bound.
 */

static void si_emit_streamout_enable(struct si_context *sctx, unsigned index)
{
   assert(sctx->gfx_level < GFX11);

   radeon_begin(&sctx->gfx_cs);
   radeon_set_context_reg_seq(R_028B94_VGT_STRMOUT_CONFIG, 2);
   radeon_emit(S_028B94_STREAMOUT_0_EN(si_get_strmout_en(sctx)) |
               S_028B94_RAST_STREAM(0) |
               S_028B94_STREAMOUT_1_EN(si_get_strmout_en(sctx)) |
               S_028B94_STREAMOUT_2_EN(si_get_strmout_en(sctx)) |
               S_028B94_STREAMOUT_3_EN(si_get_strmout_en(sctx)));
   radeon_emit(sctx->streamout.hw_enabled_mask & sctx->streamout.enabled_stream_buffers_mask);
   radeon_end();
}

static void si_set_streamout_enable(struct si_context *sctx, bool enable)
{
   if (sctx->gfx_level >= GFX11)
      return;

   bool old_strmout_en = si_get_strmout_en(sctx);
   unsigned old_hw_enabled_mask = sctx->streamout.hw_enabled_mask;

   sctx->streamout.streamout_enabled = enable;

   sctx->streamout.hw_enabled_mask =
      sctx->streamout.enabled_mask | (sctx->streamout.enabled_mask << 4) |
      (sctx->streamout.enabled_mask << 8) | (sctx->streamout.enabled_mask << 12);

   if ((old_strmout_en != si_get_strmout_en(sctx)) ||
       (old_hw_enabled_mask != sctx->streamout.hw_enabled_mask))
      si_mark_atom_dirty(sctx, &sctx->atoms.s.streamout_enable);
}

void si_update_prims_generated_query_state(struct si_context *sctx, unsigned type, int diff)
{
   if (sctx->gfx_level < GFX11 && type == PIPE_QUERY_PRIMITIVES_GENERATED) {
      bool old_strmout_en = si_get_strmout_en(sctx);

      sctx->streamout.num_prims_gen_queries += diff;
      assert(sctx->streamout.num_prims_gen_queries >= 0);

      sctx->streamout.prims_gen_query_enabled = sctx->streamout.num_prims_gen_queries != 0;

      if (old_strmout_en != si_get_strmout_en(sctx))
         si_mark_atom_dirty(sctx, &sctx->atoms.s.streamout_enable);

      if (si_update_ngg(sctx)) {
         si_shader_change_notify(sctx);
         sctx->do_update_shaders = true;
      }
   }
}

void si_init_streamout_functions(struct si_context *sctx)
{
   sctx->b.create_stream_output_target = si_create_so_target;
   sctx->b.stream_output_target_destroy = si_so_target_destroy;
   sctx->b.set_stream_output_targets = si_set_streamout_targets;
   sctx->atoms.s.streamout_begin.emit = si_emit_streamout_begin;

   if (sctx->gfx_level < GFX11)
      sctx->atoms.s.streamout_enable.emit = si_emit_streamout_enable;
}
