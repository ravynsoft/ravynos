/*
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_pipe.h"
#include "util/format/u_format.h"
#include "util/format_srgb.h"
#include "util/u_helpers.h"
#include "util/hash_table.h"

static bool si_can_use_compute_blit(struct si_context *sctx, enum pipe_format format,
                                    unsigned num_samples, bool is_store, bool has_dcc)
{
   /* TODO: This format fails AMD_TEST=imagecopy. */
   if (format == PIPE_FORMAT_A8R8_UNORM && is_store)
      return false;

   /* MSAA image stores are broken. AMD_DEBUG=nofmask fixes them, implying that the FMASK
    * expand pass doesn't work, but let's use the gfx blit, which should be faster because
    * it doesn't require expanding the FMASK.
    *
    * TODO: Broken MSAA stores can cause app issues, though this issue might only affect
    *       internal blits, not sure.
    *
    * EQAA image stores are also unimplemented, which should be rejected here after MSAA
    * image stores are fixed.
    */
   if (num_samples > 1 && is_store)
      return false;

   if (util_format_is_depth_or_stencil(format))
      return false;

   /* Image stores support DCC since GFX10. */
   if (has_dcc && is_store && sctx->gfx_level < GFX10)
      return false;

   return true;
}

static void si_use_compute_copy_for_float_formats(struct si_context *sctx,
                                                  struct pipe_resource *texture,
                                                  unsigned level)
{
   struct si_texture *tex = (struct si_texture *)texture;

   /* If we are uploading into FP16 or R11G11B10_FLOAT via a blit, CB clobbers NaNs,
    * so in order to preserve them exactly, we have to use the compute blit.
    * The compute blit is used only when the destination doesn't have DCC, so
    * disable it here, which is kinda a hack.
    * If we are uploading into 32-bit floats with DCC via a blit, NaNs will also get
    * lost so we need to disable DCC as well.
    *
    * This makes KHR-GL45.texture_view.view_classes pass on gfx9.
    */
   if (vi_dcc_enabled(tex, level) &&
       util_format_is_float(texture->format) &&
       /* Check if disabling DCC enables the compute copy. */
       !si_can_use_compute_blit(sctx, texture->format, texture->nr_samples, true, true) &&
       si_can_use_compute_blit(sctx, texture->format, texture->nr_samples, true, false)) {
      si_texture_disable_dcc(sctx, tex);
   }
}

/* Determine the cache policy. */
static enum si_cache_policy get_cache_policy(struct si_context *sctx, enum si_coherency coher,
                                             uint64_t size)
{
   if ((sctx->gfx_level >= GFX9 && (coher == SI_COHERENCY_CB_META ||
                                     coher == SI_COHERENCY_DB_META ||
                                     coher == SI_COHERENCY_CP)) ||
       (sctx->gfx_level >= GFX7 && coher == SI_COHERENCY_SHADER))
      return L2_LRU; /* it's faster if L2 doesn't evict anything  */

   return L2_BYPASS;
}

unsigned si_get_flush_flags(struct si_context *sctx, enum si_coherency coher,
                            enum si_cache_policy cache_policy)
{
   switch (coher) {
   default:
   case SI_COHERENCY_NONE:
   case SI_COHERENCY_CP:
      return 0;
   case SI_COHERENCY_SHADER:
      return SI_CONTEXT_INV_SCACHE | SI_CONTEXT_INV_VCACHE |
             (cache_policy == L2_BYPASS ? SI_CONTEXT_INV_L2 : 0);
   case SI_COHERENCY_CB_META:
      return SI_CONTEXT_FLUSH_AND_INV_CB;
   case SI_COHERENCY_DB_META:
      return SI_CONTEXT_FLUSH_AND_INV_DB;
   }
}

static bool si_is_buffer_idle(struct si_context *sctx, struct si_resource *buf,
                              unsigned usage)
{
   return !si_cs_is_buffer_referenced(sctx, buf->buf, usage) &&
          sctx->ws->buffer_wait(sctx->ws, buf->buf, 0, usage);
}

static void si_improve_sync_flags(struct si_context *sctx, struct pipe_resource *dst,
                                  struct pipe_resource *src, unsigned *flags)
{
   if (dst->target != PIPE_BUFFER || (src && src->target != PIPE_BUFFER))
      return;

   if (si_is_buffer_idle(sctx, si_resource(dst), RADEON_USAGE_READWRITE) &&
       (!src || si_is_buffer_idle(sctx, si_resource(src), RADEON_USAGE_WRITE))) {
      /* Idle buffers don't have to sync. */
      *flags &= ~(SI_OP_SYNC_GE_BEFORE | SI_OP_SYNC_PS_BEFORE | SI_OP_SYNC_CS_BEFORE |
                  SI_OP_SYNC_CPDMA_BEFORE);
      return;
   }

   const unsigned cs_mask = SI_BIND_CONSTANT_BUFFER(PIPE_SHADER_COMPUTE) |
                            SI_BIND_SHADER_BUFFER(PIPE_SHADER_COMPUTE) |
                            SI_BIND_IMAGE_BUFFER(PIPE_SHADER_COMPUTE) |
                            SI_BIND_SAMPLER_BUFFER(PIPE_SHADER_COMPUTE);

   const unsigned ps_mask = SI_BIND_CONSTANT_BUFFER(PIPE_SHADER_FRAGMENT) |
                            SI_BIND_SHADER_BUFFER(PIPE_SHADER_FRAGMENT) |
                            SI_BIND_IMAGE_BUFFER(PIPE_SHADER_FRAGMENT) |
                            SI_BIND_SAMPLER_BUFFER(PIPE_SHADER_FRAGMENT);

   unsigned bind_history = si_resource(dst)->bind_history |
                           (src ? si_resource(src)->bind_history : 0);

   /* Clear SI_OP_SYNC_CS_BEFORE if the buffer has never been used with a CS. */
   if (*flags & SI_OP_SYNC_CS_BEFORE && !(bind_history & cs_mask))
      *flags &= ~SI_OP_SYNC_CS_BEFORE;

   /* Clear SI_OP_SYNC_PS_BEFORE if the buffer has never been used with a PS. */
   if (*flags & SI_OP_SYNC_PS_BEFORE && !(bind_history & ps_mask)) {
      *flags &= ~SI_OP_SYNC_PS_BEFORE;
      *flags |= SI_OP_SYNC_GE_BEFORE;
   }
}

static void si_launch_grid_internal(struct si_context *sctx, const struct pipe_grid_info *info,
                                    void *shader, unsigned flags)
{
   /* Wait for previous shaders to finish. */
   if (flags & SI_OP_SYNC_GE_BEFORE)
      sctx->flags |= SI_CONTEXT_VS_PARTIAL_FLUSH;

   if (flags & SI_OP_SYNC_PS_BEFORE)
      sctx->flags |= SI_CONTEXT_PS_PARTIAL_FLUSH;

   if (flags & SI_OP_SYNC_CS_BEFORE)
      sctx->flags |= SI_CONTEXT_CS_PARTIAL_FLUSH;

   /* Invalidate L0-L1 caches. */
   /* sL0 is never invalidated, because src resources don't use it. */
   if (!(flags & SI_OP_SKIP_CACHE_INV_BEFORE))
      sctx->flags |= SI_CONTEXT_INV_VCACHE;

   /* Set settings for driver-internal compute dispatches. */
   sctx->flags &= ~SI_CONTEXT_START_PIPELINE_STATS;
   if (sctx->num_hw_pipestat_streamout_queries)
      sctx->flags |= SI_CONTEXT_STOP_PIPELINE_STATS;

   if (sctx->flags)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);

   if (!(flags & SI_OP_CS_RENDER_COND_ENABLE))
      sctx->render_cond_enabled = false;

   /* Force-disable fbfetch because there are unsolvable recursion problems. */
   si_force_disable_ps_colorbuf0_slot(sctx);

   /* Skip decompression to prevent infinite recursion. */
   sctx->blitter_running = true;

   /* Dispatch compute. */
   void *saved_cs = sctx->cs_shader_state.program;
   sctx->b.bind_compute_state(&sctx->b, shader);
   sctx->b.launch_grid(&sctx->b, info);
   sctx->b.bind_compute_state(&sctx->b, saved_cs);

   /* Restore default settings. */
   sctx->flags &= ~SI_CONTEXT_STOP_PIPELINE_STATS;
   if (sctx->num_hw_pipestat_streamout_queries)
      sctx->flags |= SI_CONTEXT_START_PIPELINE_STATS;

   sctx->render_cond_enabled = sctx->render_cond;
   sctx->blitter_running = false;

   /* We force-disabled fbfetch, so recompute the state. */
   si_update_ps_colorbuf0_slot(sctx);

   if (flags & SI_OP_SYNC_AFTER) {
      sctx->flags |= SI_CONTEXT_CS_PARTIAL_FLUSH;

      if (flags & SI_OP_CS_IMAGE) {
         /* Make sure image stores are visible to CB, which doesn't use L2 on GFX6-8. */
         sctx->flags |= sctx->gfx_level <= GFX8 ? SI_CONTEXT_WB_L2 : 0;
         /* Make sure image stores are visible to all CUs. */
         sctx->flags |= SI_CONTEXT_INV_VCACHE;
         /* Make sure RBs see our DCC changes. */
         if (sctx->gfx_level >= GFX10 && sctx->screen->info.tcc_rb_non_coherent) {
            unsigned enabled_mask = sctx->images[PIPE_SHADER_COMPUTE].enabled_mask;
            while (enabled_mask) {
               int i = u_bit_scan(&enabled_mask);
               if (sctx->images[PIPE_SHADER_COMPUTE].views[i].access & SI_IMAGE_ACCESS_ALLOW_DCC_STORE) {
                  sctx->flags |= SI_CONTEXT_INV_L2;
                  break;
               }
            }
         }
      } else {
         /* Make sure buffer stores are visible to all CUs. */
         sctx->flags |= SI_CONTEXT_INV_SCACHE | SI_CONTEXT_INV_VCACHE | SI_CONTEXT_PFP_SYNC_ME;
      }
   }

   if (sctx->flags)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
}

void si_launch_grid_internal_ssbos(struct si_context *sctx, struct pipe_grid_info *info,
                                   void *shader, unsigned flags, enum si_coherency coher,
                                   unsigned num_buffers, const struct pipe_shader_buffer *buffers,
                                   unsigned writeable_bitmask)
{
   if (!(flags & SI_OP_SKIP_CACHE_INV_BEFORE)) {
      sctx->flags |= si_get_flush_flags(sctx, coher, SI_COMPUTE_DST_CACHE_POLICY);
      si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
   }

   /* Save states. */
   struct pipe_shader_buffer saved_sb[3] = {};
   assert(num_buffers <= ARRAY_SIZE(saved_sb));
   si_get_shader_buffers(sctx, PIPE_SHADER_COMPUTE, 0, num_buffers, saved_sb);

   unsigned saved_writable_mask = 0;
   for (unsigned i = 0; i < num_buffers; i++) {
      if (sctx->const_and_shader_buffers[PIPE_SHADER_COMPUTE].writable_mask &
          (1u << si_get_shaderbuf_slot(i)))
         saved_writable_mask |= 1 << i;
   }

   /* Bind buffers and launch compute. */
   si_set_shader_buffers(&sctx->b, PIPE_SHADER_COMPUTE, 0, num_buffers, buffers,
                         writeable_bitmask,
                         true /* don't update bind_history to prevent unnecessary syncs later */);
   si_launch_grid_internal(sctx, info, shader, flags);

   /* Do cache flushing at the end. */
   if (get_cache_policy(sctx, coher, 0) == L2_BYPASS) {
      if (flags & SI_OP_SYNC_AFTER) {
         sctx->flags |= SI_CONTEXT_WB_L2;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
      }
   } else {
      while (writeable_bitmask)
         si_resource(buffers[u_bit_scan(&writeable_bitmask)].buffer)->TC_L2_dirty = true;
   }

   /* Restore states. */
   sctx->b.set_shader_buffers(&sctx->b, PIPE_SHADER_COMPUTE, 0, num_buffers, saved_sb,
                              saved_writable_mask);
   for (int i = 0; i < num_buffers; i++)
      pipe_resource_reference(&saved_sb[i].buffer, NULL);
}

/**
 * Clear a buffer using read-modify-write with a 32-bit write bitmask.
 * The clear value has 32 bits.
 */
void si_compute_clear_buffer_rmw(struct si_context *sctx, struct pipe_resource *dst,
                                 unsigned dst_offset, unsigned size,
                                 uint32_t clear_value, uint32_t writebitmask,
                                 unsigned flags, enum si_coherency coher)
{
   assert(dst_offset % 4 == 0);
   assert(size % 4 == 0);

   assert(dst->target != PIPE_BUFFER || dst_offset + size <= dst->width0);

   /* Use buffer_load_dwordx4 and buffer_store_dwordx4 per thread. */
   unsigned dwords_per_instruction = 4;
   unsigned block_size = 64; /* it's always 64x1x1 */
   unsigned dwords_per_wave = dwords_per_instruction * block_size;

   unsigned num_dwords = size / 4;
   unsigned num_instructions = DIV_ROUND_UP(num_dwords, dwords_per_instruction);

   struct pipe_grid_info info = {};
   info.block[0] = MIN2(block_size, num_instructions);
   info.block[1] = 1;
   info.block[2] = 1;
   info.grid[0] = DIV_ROUND_UP(num_dwords, dwords_per_wave);
   info.grid[1] = 1;
   info.grid[2] = 1;

   struct pipe_shader_buffer sb = {};
   sb.buffer = dst;
   sb.buffer_offset = dst_offset;
   sb.buffer_size = size;

   sctx->cs_user_data[0] = clear_value & writebitmask;
   sctx->cs_user_data[1] = ~writebitmask;

   if (!sctx->cs_clear_buffer_rmw)
      sctx->cs_clear_buffer_rmw = si_create_clear_buffer_rmw_cs(sctx);

   si_launch_grid_internal_ssbos(sctx, &info, sctx->cs_clear_buffer_rmw, flags, coher,
                                 1, &sb, 0x1);
}

static void si_compute_clear_12bytes_buffer(struct si_context *sctx, struct pipe_resource *dst,
                                            unsigned dst_offset, unsigned size,
                                            const uint32_t *clear_value, unsigned flags,
                                            enum si_coherency coher)
{
   assert(dst_offset % 4 == 0);
   assert(size % 4 == 0);
   unsigned size_12 = DIV_ROUND_UP(size, 12);

   struct pipe_shader_buffer sb = {0};
   sb.buffer = dst;
   sb.buffer_offset = dst_offset;
   sb.buffer_size = size;

   memcpy(sctx->cs_user_data, clear_value, 12);

   struct pipe_grid_info info = {0};

   if (!sctx->cs_clear_12bytes_buffer)
      sctx->cs_clear_12bytes_buffer = si_clear_12bytes_buffer_shader(sctx);

   info.block[0] = 64;
   info.last_block[0] = size_12 % 64;
   info.block[1] = 1;
   info.block[2] = 1;
   info.grid[0] = DIV_ROUND_UP(size_12, 64);
   info.grid[1] = 1;
   info.grid[2] = 1;

   si_launch_grid_internal_ssbos(sctx, &info, sctx->cs_clear_12bytes_buffer, flags, coher,
                                 1, &sb, 0x1);
}

static void si_compute_do_clear_or_copy(struct si_context *sctx, struct pipe_resource *dst,
                                        unsigned dst_offset, struct pipe_resource *src,
                                        unsigned src_offset, unsigned size,
                                        const uint32_t *clear_value, unsigned clear_value_size,
                                        unsigned flags, enum si_coherency coher)
{
   assert(src_offset % 4 == 0);
   assert(dst_offset % 4 == 0);
   assert(size % 4 == 0);

   assert(dst->target != PIPE_BUFFER || dst_offset + size <= dst->width0);
   assert(!src || src_offset + size <= src->width0);

   /* The memory accesses are coalesced, meaning that the 1st instruction writes
    * the 1st contiguous block of data for the whole wave, the 2nd instruction
    * writes the 2nd contiguous block of data, etc.
    */
   unsigned dwords_per_thread =
      src ? SI_COMPUTE_COPY_DW_PER_THREAD : SI_COMPUTE_CLEAR_DW_PER_THREAD;
   unsigned instructions_per_thread = MAX2(1, dwords_per_thread / 4);
   unsigned dwords_per_instruction = dwords_per_thread / instructions_per_thread;
   /* The shader declares the block size like this: */
   unsigned block_size = si_determine_wave_size(sctx->screen, NULL);
   unsigned dwords_per_wave = dwords_per_thread * block_size;

   unsigned num_dwords = size / 4;
   unsigned num_instructions = DIV_ROUND_UP(num_dwords, dwords_per_instruction);

   struct pipe_grid_info info = {};
   info.block[0] = MIN2(block_size, num_instructions);
   info.block[1] = 1;
   info.block[2] = 1;
   info.grid[0] = DIV_ROUND_UP(num_dwords, dwords_per_wave);
   info.grid[1] = 1;
   info.grid[2] = 1;

   struct pipe_shader_buffer sb[2] = {};
   sb[0].buffer = dst;
   sb[0].buffer_offset = dst_offset;
   sb[0].buffer_size = size;

   bool shader_dst_stream_policy = SI_COMPUTE_DST_CACHE_POLICY != L2_LRU;

   if (src) {
      sb[1].buffer = src;
      sb[1].buffer_offset = src_offset;
      sb[1].buffer_size = size;

      if (!sctx->cs_copy_buffer) {
         sctx->cs_copy_buffer = si_create_dma_compute_shader(
            sctx, SI_COMPUTE_COPY_DW_PER_THREAD, shader_dst_stream_policy, true);
      }

      si_launch_grid_internal_ssbos(sctx, &info, sctx->cs_copy_buffer, flags, coher,
                                    2, sb, 0x1);
   } else {
      assert(clear_value_size >= 4 && clear_value_size <= 16 &&
             util_is_power_of_two_or_zero(clear_value_size));

      for (unsigned i = 0; i < 4; i++)
         sctx->cs_user_data[i] = clear_value[i % (clear_value_size / 4)];

      if (!sctx->cs_clear_buffer) {
         sctx->cs_clear_buffer = si_create_dma_compute_shader(
            sctx, SI_COMPUTE_CLEAR_DW_PER_THREAD, shader_dst_stream_policy, false);
      }

      si_launch_grid_internal_ssbos(sctx, &info, sctx->cs_clear_buffer, flags, coher,
                                    1, sb, 0x1);
   }
}

void si_clear_buffer(struct si_context *sctx, struct pipe_resource *dst,
                     uint64_t offset, uint64_t size, uint32_t *clear_value,
                     uint32_t clear_value_size, unsigned flags,
                     enum si_coherency coher, enum si_clear_method method)
{
   if (!size)
      return;

   si_improve_sync_flags(sctx, dst, NULL, &flags);

   ASSERTED unsigned clear_alignment = MIN2(clear_value_size, 4);

   assert(clear_value_size != 3 && clear_value_size != 6); /* 12 is allowed. */
   assert(offset % clear_alignment == 0);
   assert(size % clear_alignment == 0);
   assert(size < (UINT_MAX & ~0xf)); /* TODO: test 64-bit sizes in all codepaths */

   uint32_t clamped;
   if (util_lower_clearsize_to_dword(clear_value, (int*)&clear_value_size, &clamped))
      clear_value = &clamped;

   if (clear_value_size == 12) {
      si_compute_clear_12bytes_buffer(sctx, dst, offset, size, clear_value, flags, coher);
      return;
   }

   uint64_t aligned_size = size & ~3ull;
   if (aligned_size >= 4) {
      uint64_t compute_min_size;

      if (sctx->gfx_level <= GFX8) {
         /* CP DMA clears are terribly slow with GTT on GFX6-8, which can always
          * happen due to BO evictions.
          */
         compute_min_size = 0;
      } else {
         /* Use a small enough size because CP DMA is slower than compute with bigger sizes. */
         compute_min_size = 4 * 1024;
      }

      /* TODO: use compute for unaligned big sizes */
      if (method == SI_AUTO_SELECT_CLEAR_METHOD && (
           clear_value_size > 4 ||
           (clear_value_size == 4 && offset % 4 == 0 && size > compute_min_size))) {
         method = SI_COMPUTE_CLEAR_METHOD;
      }
      if (method == SI_COMPUTE_CLEAR_METHOD) {
         si_compute_do_clear_or_copy(sctx, dst, offset, NULL, 0, aligned_size, clear_value,
                                     clear_value_size, flags, coher);
      } else {
         assert(clear_value_size == 4);
         si_cp_dma_clear_buffer(sctx, &sctx->gfx_cs, dst, offset, aligned_size, *clear_value,
                                flags, coher, get_cache_policy(sctx, coher, size));
      }

      offset += aligned_size;
      size -= aligned_size;
   }

   /* Handle non-dword alignment. */
   if (size) {
      assert(dst);
      assert(dst->target == PIPE_BUFFER);
      assert(size < 4);

      sctx->b.buffer_subdata(&sctx->b, dst,
                             PIPE_MAP_WRITE |
                             /* TC forbids drivers to invalidate buffers and infer unsynchronized mappings,
                              * so suppress those optimizations. */
                             (sctx->tc ? TC_TRANSFER_MAP_NO_INFER_UNSYNCHRONIZED |
                                         TC_TRANSFER_MAP_NO_INVALIDATE : 0),
                             offset, size, clear_value);
   }
}

static void si_pipe_clear_buffer(struct pipe_context *ctx, struct pipe_resource *dst,
                                 unsigned offset, unsigned size, const void *clear_value,
                                 int clear_value_size)
{
   si_clear_buffer((struct si_context *)ctx, dst, offset, size, (uint32_t *)clear_value,
                   clear_value_size, SI_OP_SYNC_BEFORE_AFTER, SI_COHERENCY_SHADER,
                   SI_AUTO_SELECT_CLEAR_METHOD);
}

void si_copy_buffer(struct si_context *sctx, struct pipe_resource *dst, struct pipe_resource *src,
                    uint64_t dst_offset, uint64_t src_offset, unsigned size, unsigned flags)
{
   if (!size)
      return;

   enum si_coherency coher = SI_COHERENCY_SHADER;
   enum si_cache_policy cache_policy = get_cache_policy(sctx, coher, size);
   uint64_t compute_min_size = 8 * 1024;

   si_improve_sync_flags(sctx, dst, src, &flags);

   /* Only use compute for VRAM copies on dGPUs. */
   /* TODO: use compute for unaligned big sizes */
   if (sctx->screen->info.has_dedicated_vram && si_resource(dst)->domains & RADEON_DOMAIN_VRAM &&
       si_resource(src)->domains & RADEON_DOMAIN_VRAM && size > compute_min_size &&
       dst_offset % 4 == 0 && src_offset % 4 == 0 && size % 4 == 0) {
      si_compute_do_clear_or_copy(sctx, dst, dst_offset, src, src_offset, size, NULL, 0,
                                  flags, coher);
   } else {
      si_cp_dma_copy_buffer(sctx, dst, src, dst_offset, src_offset, size,
                            flags, coher, cache_policy);
   }
}

void si_compute_shorten_ubyte_buffer(struct si_context *sctx, struct pipe_resource *dst, struct pipe_resource *src,
                                     uint64_t dst_offset, uint64_t src_offset, unsigned size, unsigned flags)
{
   if (!size)
      return;

   if (!sctx->cs_ubyte_to_ushort)
      sctx->cs_ubyte_to_ushort = si_create_ubyte_to_ushort_compute_shader(sctx);

   /* Use COHERENCY_NONE to get SI_CONTEXT_WB_L2 automatically used in
    * si_launch_grid_internal_ssbos.
    */
   enum si_coherency coher = SI_COHERENCY_NONE;

   si_improve_sync_flags(sctx, dst, src, &flags);

   struct pipe_grid_info info = {};
   info.block[0] = si_determine_wave_size(sctx->screen, NULL);
   info.block[1] = 1;
   info.block[2] = 1;
   info.grid[0] = DIV_ROUND_UP(size, info.block[0]);
   info.grid[1] = 1;
   info.grid[2] = 1;
   info.last_block[0] = size % info.block[0];

   struct pipe_shader_buffer sb[2] = {};
   sb[0].buffer = dst;
   sb[0].buffer_offset = dst_offset;
   sb[0].buffer_size = dst->width0;

   sb[1].buffer = src;
   sb[1].buffer_offset = src_offset;
   sb[1].buffer_size = src->width0;

   si_launch_grid_internal_ssbos(sctx, &info, sctx->cs_ubyte_to_ushort, flags, coher,
                                 2, sb, 0x1);
}

static unsigned
set_work_size(struct pipe_grid_info *info, unsigned block_x, unsigned block_y, unsigned block_z,
              unsigned work_x, unsigned work_y, unsigned work_z)
{
   info->block[0] = block_x;
   info->block[1] = block_y;
   info->block[2] = block_z;

   unsigned work[3] = {work_x, work_y, work_z};
   for (int i = 0; i < 3; ++i) {
      info->last_block[i] = work[i] % info->block[i];
      info->grid[i] = DIV_ROUND_UP(work[i], info->block[i]);
   }

   return work_z > 1 ? 3 : (work_y > 1 ? 2 : 1);
}

static void si_launch_grid_internal_images(struct si_context *sctx,
                                           struct pipe_image_view *images,
                                           unsigned num_images,
                                           const struct pipe_grid_info *info,
                                           void *shader, unsigned flags)
{
   struct pipe_image_view saved_image[2] = {};
   assert(num_images <= ARRAY_SIZE(saved_image));

   for (unsigned i = 0; i < num_images; i++) {
      assert(sctx->b.screen->is_format_supported(sctx->b.screen, images[i].format,
                                                 images[i].resource->target,
                                                 images[i].resource->nr_samples,
                                                 images[i].resource->nr_storage_samples,
                                                 PIPE_BIND_SHADER_IMAGE));

      /* Always allow DCC stores on gfx10+. */
      if (sctx->gfx_level >= GFX10 &&
          images[i].access & PIPE_IMAGE_ACCESS_WRITE &&
          !(images[i].access & SI_IMAGE_ACCESS_DCC_OFF))
         images[i].access |= SI_IMAGE_ACCESS_ALLOW_DCC_STORE;

      /* Simplify the format according to what image stores support. */
      if (images[i].access & PIPE_IMAGE_ACCESS_WRITE) {
         images[i].format = util_format_linear(images[i].format); /* SRGB not supported */
         /* Keep L8A8 formats as-is because GFX7 is unable to store into R8A8 for some reason. */
         images[i].format = util_format_intensity_to_red(images[i].format);
         images[i].format = util_format_rgbx_to_rgba(images[i].format); /* prevent partial writes */
      }

      /* Save the image. */
      util_copy_image_view(&saved_image[i], &sctx->images[PIPE_SHADER_COMPUTE].views[i]);
   }

   /* This might invoke DCC decompression, so do it first. */
   sctx->b.set_shader_images(&sctx->b, PIPE_SHADER_COMPUTE, 0, num_images, 0, images);

   /* This should be done after set_shader_images. */
   for (unsigned i = 0; i < num_images; i++) {
      /* The driver doesn't decompress resources automatically here, so do it manually. */
      si_decompress_subresource(&sctx->b, images[i].resource, PIPE_MASK_RGBAZS,
                                images[i].u.tex.level, images[i].u.tex.first_layer,
                                images[i].u.tex.last_layer,
                                images[i].access & PIPE_IMAGE_ACCESS_WRITE);
   }

   /* This must be done before the compute shader. */
   for (unsigned i = 0; i < num_images; i++) {
      si_make_CB_shader_coherent(sctx, images[i].resource->nr_samples, true,
            ((struct si_texture*)images[i].resource)->surface.u.gfx9.color.dcc.pipe_aligned);
   }

   si_launch_grid_internal(sctx, info, shader, flags | SI_OP_CS_IMAGE);

   /* Restore images. */
   sctx->b.set_shader_images(&sctx->b, PIPE_SHADER_COMPUTE, 0, num_images, 0, saved_image);
   for (unsigned i = 0; i < num_images; i++)
      pipe_resource_reference(&saved_image[i].resource, NULL);
}

bool si_compute_copy_image(struct si_context *sctx, struct pipe_resource *dst, unsigned dst_level,
                           struct pipe_resource *src, unsigned src_level, unsigned dstx,
                           unsigned dsty, unsigned dstz, const struct pipe_box *src_box,
                           unsigned flags)
{
   struct si_texture *ssrc = (struct si_texture*)src;
   struct si_texture *sdst = (struct si_texture*)dst;

   si_use_compute_copy_for_float_formats(sctx, dst, dst_level);

   /* The compute copy is mandatory for compressed and subsampled formats because the gfx copy
    * doesn't support them. In all other cases, call si_can_use_compute_blit.
    *
    * The format is identical (we only need to check the src format) except compressed formats,
    * which can be paired with an equivalent integer format.
    */
   if (!util_format_is_compressed(src->format) &&
       !util_format_is_compressed(dst->format) &&
       !util_format_is_subsampled_422(src->format)) {
      bool src_can_use_compute_blit =
         si_can_use_compute_blit(sctx, src->format, src->nr_samples, false,
                                 vi_dcc_enabled(ssrc, src_level));

      if (!src_can_use_compute_blit)
         return false;

      bool dst_can_use_compute_blit =
         si_can_use_compute_blit(sctx, dst->format, dst->nr_samples, true,
                                 vi_dcc_enabled(sdst, dst_level));

      if (!dst_can_use_compute_blit && !sctx->has_graphics &&
          si_can_use_compute_blit(sctx, dst->format, dst->nr_samples, false,
                                  vi_dcc_enabled(sdst, dst_level))) {
         /* Non-graphics context don't have a blitter, so try harder to do
          * a compute blit by disabling dcc on the destination texture.
          */
         dst_can_use_compute_blit = si_texture_disable_dcc(sctx, sdst);
      }

      if (!dst_can_use_compute_blit)
         return false;
   }

   enum pipe_format src_format = util_format_linear(src->format);
   enum pipe_format dst_format = util_format_linear(dst->format);
   bool is_linear = ssrc->surface.is_linear || sdst->surface.is_linear;

   assert(util_format_is_subsampled_422(src_format) == util_format_is_subsampled_422(dst_format));

   /* Interpret as integer values to avoid NaN issues */
   if (!vi_dcc_enabled(ssrc, src_level) &&
       !vi_dcc_enabled(sdst, dst_level) &&
       src_format == dst_format &&
       util_format_is_float(src_format) &&
       !util_format_is_compressed(src_format)) {
      switch(util_format_get_blocksizebits(src_format)) {
        case 16:
          src_format = dst_format = PIPE_FORMAT_R16_UINT;
          break;
        case 32:
          src_format = dst_format = PIPE_FORMAT_R32_UINT;
          break;
        case 64:
          src_format = dst_format = PIPE_FORMAT_R32G32_UINT;
          break;
        case 128:
          src_format = dst_format = PIPE_FORMAT_R32G32B32A32_UINT;
          break;
        default:
          assert(false);
      }
   }

   /* Interpret compressed formats as UINT. */
   struct pipe_box new_box;
   unsigned src_access = 0, dst_access = 0;

   /* Note that staging copies do compressed<->UINT, so one of the formats is already UINT. */
   if (util_format_is_compressed(src_format) || util_format_is_compressed(dst_format)) {
      if (util_format_is_compressed(src_format))
         src_access |= SI_IMAGE_ACCESS_BLOCK_FORMAT_AS_UINT;
      if (util_format_is_compressed(dst_format))
         dst_access |= SI_IMAGE_ACCESS_BLOCK_FORMAT_AS_UINT;

      dstx = util_format_get_nblocksx(dst_format, dstx);
      dsty = util_format_get_nblocksy(dst_format, dsty);

      new_box.x = util_format_get_nblocksx(src_format, src_box->x);
      new_box.y = util_format_get_nblocksy(src_format, src_box->y);
      new_box.z = src_box->z;
      new_box.width = util_format_get_nblocksx(src_format, src_box->width);
      new_box.height = util_format_get_nblocksy(src_format, src_box->height);
      new_box.depth = src_box->depth;
      src_box = &new_box;

      if (ssrc->surface.bpe == 8)
         src_format = dst_format = PIPE_FORMAT_R16G16B16A16_UINT; /* 64-bit block */
      else
         src_format = dst_format = PIPE_FORMAT_R32G32B32A32_UINT; /* 128-bit block */
   }

   if (util_format_is_subsampled_422(src_format)) {
      assert(src_format == dst_format);

      src_access |= SI_IMAGE_ACCESS_BLOCK_FORMAT_AS_UINT;
      dst_access |= SI_IMAGE_ACCESS_BLOCK_FORMAT_AS_UINT;

      dstx = util_format_get_nblocksx(src_format, dstx);

      src_format = dst_format = PIPE_FORMAT_R32_UINT;

      /* Interpreting 422 subsampled format (16 bpp) as 32 bpp
       * should force us to divide src_box->x, dstx and width by 2.
       * But given that ac_surface allocates this format as 32 bpp
       * and that surf_size is then modified to pack the values
       * we must keep the original values to get the correct results.
       */
   }

   /* SNORM blitting has precision issues. Use the SINT equivalent instead, which doesn't
    * force DCC decompression.
    */
   if (util_format_is_snorm(dst_format))
      src_format = dst_format = util_format_snorm_to_sint(dst_format);

   if (src_box->width == 0 || src_box->height == 0 || src_box->depth == 0)
      return true; /* success - nothing to do */

   struct pipe_image_view image[2] = {0};
   image[0].resource = src;
   image[0].shader_access = image[0].access = PIPE_IMAGE_ACCESS_READ | src_access;
   image[0].format = src_format;
   image[0].u.tex.level = src_level;
   image[0].u.tex.first_layer = 0;
   image[0].u.tex.last_layer = util_max_layer(src, src_level);
   image[1].resource = dst;
   image[1].shader_access = image[1].access = PIPE_IMAGE_ACCESS_WRITE | dst_access;
   image[1].format = dst_format;
   image[1].u.tex.level = dst_level;
   image[1].u.tex.first_layer = 0;
   image[1].u.tex.last_layer = util_max_layer(dst, dst_level);

   struct pipe_grid_info info = {0};

   bool dst_is_1d = dst->target == PIPE_TEXTURE_1D ||
                    dst->target == PIPE_TEXTURE_1D_ARRAY;
   bool src_is_1d = src->target == PIPE_TEXTURE_1D ||
                    src->target == PIPE_TEXTURE_1D_ARRAY;
   int block_x, block_y;
   int block_z = 1;

   /* Choose the block dimensions based on the copy area size. */
   if (src_box->height <= 4) {
      block_y = util_next_power_of_two(src_box->height);
      block_x = 64 / block_y;
   } else if (src_box->width <= 4) {
      block_x = util_next_power_of_two(src_box->width);
      block_y = 64 / block_x;
   } else if (is_linear) {
      block_x = 64;
      block_y = 1;
   } else {
      block_x = 8;
      block_y = 8;
   }

   sctx->cs_user_data[0] = src_box->x | (dstx << 16);
   sctx->cs_user_data[1] = src_box->y | (dsty << 16);
   sctx->cs_user_data[2] = src_box->z | (dstz << 16);

   unsigned wg_dim =
      set_work_size(&info, block_x, block_y, block_z,
                    src_box->width, src_box->height, src_box->depth);

   void **copy_image_cs_ptr = &sctx->cs_copy_image[wg_dim - 1][src_is_1d][dst_is_1d];
   if (!*copy_image_cs_ptr)
      *copy_image_cs_ptr = si_create_copy_image_cs(sctx, wg_dim, src_is_1d, dst_is_1d);

   assert(*copy_image_cs_ptr);

   si_launch_grid_internal_images(sctx, image, 2, &info, *copy_image_cs_ptr, flags);
   return true;
}

void si_retile_dcc(struct si_context *sctx, struct si_texture *tex)
{
   /* Set the DCC buffer. */
   assert(tex->surface.meta_offset && tex->surface.meta_offset <= UINT_MAX);
   assert(tex->surface.display_dcc_offset && tex->surface.display_dcc_offset <= UINT_MAX);
   assert(tex->surface.display_dcc_offset < tex->surface.meta_offset);
   assert(tex->buffer.bo_size <= UINT_MAX);

   struct pipe_shader_buffer sb = {};
   sb.buffer = &tex->buffer.b.b;
   sb.buffer_offset = tex->surface.display_dcc_offset;
   sb.buffer_size = tex->buffer.bo_size - sb.buffer_offset;

   sctx->cs_user_data[0] = tex->surface.meta_offset - tex->surface.display_dcc_offset;
   sctx->cs_user_data[1] = (tex->surface.u.gfx9.color.dcc_pitch_max + 1) |
                           (tex->surface.u.gfx9.color.dcc_height << 16);
   sctx->cs_user_data[2] = (tex->surface.u.gfx9.color.display_dcc_pitch_max + 1) |
                           (tex->surface.u.gfx9.color.display_dcc_height << 16);

   /* We have only 1 variant per bpp for now, so expect 32 bpp. */
   assert(tex->surface.bpe == 4);

   void **shader = &sctx->cs_dcc_retile[tex->surface.u.gfx9.swizzle_mode];
   if (!*shader)
      *shader = si_create_dcc_retile_cs(sctx, &tex->surface);

   /* Dispatch compute. */
   unsigned width = DIV_ROUND_UP(tex->buffer.b.b.width0, tex->surface.u.gfx9.color.dcc_block_width);
   unsigned height = DIV_ROUND_UP(tex->buffer.b.b.height0, tex->surface.u.gfx9.color.dcc_block_height);

   struct pipe_grid_info info = {};
   info.block[0] = 8;
   info.block[1] = 8;
   info.block[2] = 1;
   info.last_block[0] = width % info.block[0];
   info.last_block[1] = height % info.block[1];
   info.grid[0] = DIV_ROUND_UP(width, info.block[0]);
   info.grid[1] = DIV_ROUND_UP(height, info.block[1]);
   info.grid[2] = 1;

   si_launch_grid_internal_ssbos(sctx, &info, *shader, SI_OP_SYNC_BEFORE,
                                 SI_COHERENCY_CB_META, 1, &sb, 0x1);

   /* Don't flush caches. L2 will be flushed by the kernel fence. */
}

void gfx9_clear_dcc_msaa(struct si_context *sctx, struct pipe_resource *res, uint32_t clear_value,
                         unsigned flags, enum si_coherency coher)
{
   struct si_texture *tex = (struct si_texture*)res;

   assert(sctx->gfx_level < GFX11);

   /* Set the DCC buffer. */
   assert(tex->surface.meta_offset && tex->surface.meta_offset <= UINT_MAX);
   assert(tex->buffer.bo_size <= UINT_MAX);

   struct pipe_shader_buffer sb = {};
   sb.buffer = &tex->buffer.b.b;
   sb.buffer_offset = tex->surface.meta_offset;
   sb.buffer_size = tex->buffer.bo_size - sb.buffer_offset;

   sctx->cs_user_data[0] = (tex->surface.u.gfx9.color.dcc_pitch_max + 1) |
                           (tex->surface.u.gfx9.color.dcc_height << 16);
   sctx->cs_user_data[1] = (clear_value & 0xffff) |
                           ((uint32_t)tex->surface.tile_swizzle << 16);

   /* These variables identify the shader variant. */
   unsigned swizzle_mode = tex->surface.u.gfx9.swizzle_mode;
   unsigned bpe_log2 = util_logbase2(tex->surface.bpe);
   unsigned log2_samples = util_logbase2(tex->buffer.b.b.nr_samples);
   bool fragments8 = tex->buffer.b.b.nr_storage_samples == 8;
   bool is_array = tex->buffer.b.b.array_size > 1;
   void **shader = &sctx->cs_clear_dcc_msaa[swizzle_mode][bpe_log2][fragments8][log2_samples - 2][is_array];

   if (!*shader)
      *shader = gfx9_create_clear_dcc_msaa_cs(sctx, tex);

   /* Dispatch compute. */
   unsigned width = DIV_ROUND_UP(tex->buffer.b.b.width0, tex->surface.u.gfx9.color.dcc_block_width);
   unsigned height = DIV_ROUND_UP(tex->buffer.b.b.height0, tex->surface.u.gfx9.color.dcc_block_height);
   unsigned depth = DIV_ROUND_UP(tex->buffer.b.b.array_size, tex->surface.u.gfx9.color.dcc_block_depth);

   struct pipe_grid_info info = {};
   info.block[0] = 8;
   info.block[1] = 8;
   info.block[2] = 1;
   info.last_block[0] = width % info.block[0];
   info.last_block[1] = height % info.block[1];
   info.grid[0] = DIV_ROUND_UP(width, info.block[0]);
   info.grid[1] = DIV_ROUND_UP(height, info.block[1]);
   info.grid[2] = depth;

   si_launch_grid_internal_ssbos(sctx, &info, *shader, flags, coher, 1, &sb, 0x1);
}

/* Expand FMASK to make it identity, so that image stores can ignore it. */
void si_compute_expand_fmask(struct pipe_context *ctx, struct pipe_resource *tex)
{
   struct si_context *sctx = (struct si_context *)ctx;
   bool is_array = tex->target == PIPE_TEXTURE_2D_ARRAY;
   unsigned log_fragments = util_logbase2(tex->nr_storage_samples);
   unsigned log_samples = util_logbase2(tex->nr_samples);
   assert(tex->nr_samples >= 2);

   assert(sctx->gfx_level < GFX11);

   /* EQAA FMASK expansion is unimplemented. */
   if (tex->nr_samples != tex->nr_storage_samples)
      return;

   si_make_CB_shader_coherent(sctx, tex->nr_samples, true,
                              ((struct si_texture*)tex)->surface.u.gfx9.color.dcc.pipe_aligned);

   /* Save states. */
   struct pipe_image_view saved_image = {0};
   util_copy_image_view(&saved_image, &sctx->images[PIPE_SHADER_COMPUTE].views[0]);

   /* Bind the image. */
   struct pipe_image_view image = {0};
   image.resource = tex;
   /* Don't set WRITE so as not to trigger FMASK expansion, causing
    * an infinite loop. */
   image.shader_access = image.access = PIPE_IMAGE_ACCESS_READ;
   image.format = util_format_linear(tex->format);
   if (is_array)
      image.u.tex.last_layer = tex->array_size - 1;

   ctx->set_shader_images(ctx, PIPE_SHADER_COMPUTE, 0, 1, 0, &image);

   /* Bind the shader. */
   void **shader = &sctx->cs_fmask_expand[log_samples - 1][is_array];
   if (!*shader)
      *shader = si_create_fmask_expand_cs(sctx, tex->nr_samples, is_array);

   /* Dispatch compute. */
   struct pipe_grid_info info = {0};
   info.block[0] = 8;
   info.last_block[0] = tex->width0 % 8;
   info.block[1] = 8;
   info.last_block[1] = tex->height0 % 8;
   info.block[2] = 1;
   info.grid[0] = DIV_ROUND_UP(tex->width0, 8);
   info.grid[1] = DIV_ROUND_UP(tex->height0, 8);
   info.grid[2] = is_array ? tex->array_size : 1;

   si_launch_grid_internal(sctx, &info, *shader, SI_OP_SYNC_BEFORE_AFTER);

   /* Restore previous states. */
   ctx->set_shader_images(ctx, PIPE_SHADER_COMPUTE, 0, 1, 0, &saved_image);
   pipe_resource_reference(&saved_image.resource, NULL);

   /* Array of fully expanded FMASK values, arranged by [log2(fragments)][log2(samples)-1]. */
#define INVALID 0 /* never used */
   static const uint64_t fmask_expand_values[][4] = {
      /* samples */
      /* 2 (8 bpp) 4 (8 bpp)   8 (8-32bpp) 16 (16-64bpp)      fragments */
      {0x02020202, 0x0E0E0E0E, 0xFEFEFEFE, 0xFFFEFFFE},      /* 1 */
      {0x02020202, 0xA4A4A4A4, 0xAAA4AAA4, 0xAAAAAAA4},      /* 2 */
      {INVALID, 0xE4E4E4E4, 0x44443210, 0x4444444444443210}, /* 4 */
      {INVALID, INVALID, 0x76543210, 0x8888888876543210},    /* 8 */
   };

   /* Clear FMASK to identity. */
   struct si_texture *stex = (struct si_texture *)tex;
   si_clear_buffer(sctx, tex, stex->surface.fmask_offset, stex->surface.fmask_size,
                   (uint32_t *)&fmask_expand_values[log_fragments][log_samples - 1],
                   log_fragments >= 2 && log_samples == 4 ? 8 : 4, SI_OP_SYNC_AFTER,
                   SI_COHERENCY_SHADER, SI_AUTO_SELECT_CLEAR_METHOD);
}

void si_init_compute_blit_functions(struct si_context *sctx)
{
   sctx->b.clear_buffer = si_pipe_clear_buffer;
}

/* Clear a region of a color surface to a constant value. */
void si_compute_clear_render_target(struct pipe_context *ctx, struct pipe_surface *dstsurf,
                                    const union pipe_color_union *color, unsigned dstx,
                                    unsigned dsty, unsigned width, unsigned height,
                                    bool render_condition_enabled)
{
   struct si_context *sctx = (struct si_context *)ctx;
   unsigned num_layers = dstsurf->u.tex.last_layer - dstsurf->u.tex.first_layer + 1;
   unsigned data[4 + sizeof(color->ui)] = {dstx, dsty, dstsurf->u.tex.first_layer, 0};

   if (width == 0 || height == 0)
      return;

   if (util_format_is_srgb(dstsurf->format)) {
      union pipe_color_union color_srgb;
      for (int i = 0; i < 3; i++)
         color_srgb.f[i] = util_format_linear_to_srgb_float(color->f[i]);
      color_srgb.f[3] = color->f[3];
      memcpy(data + 4, color_srgb.ui, sizeof(color->ui));
   } else {
      memcpy(data + 4, color->ui, sizeof(color->ui));
   }

   struct pipe_constant_buffer saved_cb = {};
   si_get_pipe_constant_buffer(sctx, PIPE_SHADER_COMPUTE, 0, &saved_cb);

   struct pipe_constant_buffer cb = {};
   cb.buffer_size = sizeof(data);
   cb.user_buffer = data;
   ctx->set_constant_buffer(ctx, PIPE_SHADER_COMPUTE, 0, false, &cb);

   struct pipe_image_view image = {0};
   image.resource = dstsurf->texture;
   image.shader_access = image.access = PIPE_IMAGE_ACCESS_WRITE;
   image.format = util_format_linear(dstsurf->format);
   image.u.tex.level = dstsurf->u.tex.level;
   image.u.tex.first_layer = 0; /* 3D images ignore first_layer (BASE_ARRAY) */
   image.u.tex.last_layer = dstsurf->u.tex.last_layer;

   struct pipe_grid_info info = {0};
   void *shader;

   if (dstsurf->texture->target != PIPE_TEXTURE_1D_ARRAY) {
      if (!sctx->cs_clear_render_target)
         sctx->cs_clear_render_target = si_clear_render_target_shader(sctx, PIPE_TEXTURE_2D_ARRAY);
      shader = sctx->cs_clear_render_target;

      info.block[0] = 8;
      info.last_block[0] = width % 8;
      info.block[1] = 8;
      info.last_block[1] = height % 8;
      info.block[2] = 1;
      info.grid[0] = DIV_ROUND_UP(width, 8);
      info.grid[1] = DIV_ROUND_UP(height, 8);
      info.grid[2] = num_layers;
   } else {
      if (!sctx->cs_clear_render_target_1d_array)
         sctx->cs_clear_render_target_1d_array = si_clear_render_target_shader(sctx, PIPE_TEXTURE_1D_ARRAY);
      shader = sctx->cs_clear_render_target_1d_array;

      info.block[0] = 64;
      info.last_block[0] = width % 64;
      info.block[1] = 1;
      info.block[2] = 1;
      info.grid[0] = DIV_ROUND_UP(width, 64);
      info.grid[1] = num_layers;
      info.grid[2] = 1;
   }

   si_launch_grid_internal_images(sctx, &image, 1, &info, shader,
                                  SI_OP_SYNC_BEFORE_AFTER |
                                  (render_condition_enabled ? SI_OP_CS_RENDER_COND_ENABLE : 0));

   ctx->set_constant_buffer(ctx, PIPE_SHADER_COMPUTE, 0, true, &saved_cb);
}

/* Return the last component that a compute blit should load and store. */
static unsigned si_format_get_last_blit_component(enum pipe_format format, bool is_dst)
{
   const struct util_format_description *desc = util_format_description(format);
   unsigned num = 0;

   for (unsigned i = 1; i < 4; i++) {
      if (desc->swizzle[i] <= PIPE_SWIZZLE_W ||
          /* If the swizzle is 1 for dst, we need to store 1 explicitly.
           * The hardware stores 0 by default. */
          (is_dst && desc->swizzle[i] == PIPE_SWIZZLE_1))
         num = i;
   }
   return num;
}

static bool si_should_blit_clamp_xy(const struct pipe_blit_info *info)
{
   int src_width = u_minify(info->src.resource->width0, info->src.level);
   int src_height = u_minify(info->src.resource->height0, info->src.level);
   struct pipe_box box = info->src.box;

   /* Eliminate negative width/height/depth. */
   if (box.width < 0) {
      box.x += box.width;
      box.width *= -1;
   }
   if (box.height < 0) {
      box.y += box.height;
      box.height *= -1;
   }

   bool in_bounds = box.x >= 0 && box.x < src_width &&
                    box.y >= 0 && box.y < src_height &&
                    box.x + box.width > 0 && box.x + box.width <= src_width &&
                    box.y + box.height > 0 && box.y + box.height <= src_height;

   /* Return if the box is not in bounds. */
   return !in_bounds;
}

bool si_compute_blit(struct si_context *sctx, const struct pipe_blit_info *info, bool testing)
{
   /* Compute blits require D16 right now (see the ISA).
    *
    * Testing on Navi21 showed that the compute blit is slightly slower than the gfx blit.
    * The compute blit is even slower with DCC stores. VP13 CATIA_plane_pencil is a good test
    * for that because it's mostly just blits.
    *
    * TODO: benchmark the performance on gfx11
    */
   if (sctx->gfx_level < GFX11 && !testing)
      return false;

   if (!si_can_use_compute_blit(sctx, info->dst.format, info->dst.resource->nr_samples, true,
                                vi_dcc_enabled((struct si_texture*)info->dst.resource,
                                               info->dst.level)) ||
       !si_can_use_compute_blit(sctx, info->src.format, info->src.resource->nr_samples, false,
                                vi_dcc_enabled((struct si_texture*)info->src.resource,
                                               info->src.level)))
      return false;

   if (info->alpha_blend ||
       info->num_window_rectangles ||
       info->scissor_enable ||
       /* No scaling. */
       info->dst.box.width != abs(info->src.box.width) ||
       info->dst.box.height != abs(info->src.box.height) ||
       info->dst.box.depth != abs(info->src.box.depth))
      return false;

   assert(info->src.box.depth >= 0);

   /* Shader images. */
   struct pipe_image_view image[2];
   image[0].resource = info->src.resource;
   image[0].shader_access = image[0].access = PIPE_IMAGE_ACCESS_READ;
   image[0].format = info->src.format;
   image[0].u.tex.level = info->src.level;
   image[0].u.tex.first_layer = 0;
   image[0].u.tex.last_layer = util_max_layer(info->src.resource, info->src.level);

   image[1].resource = info->dst.resource;
   image[1].shader_access = image[1].access = PIPE_IMAGE_ACCESS_WRITE;
   image[1].format = info->dst.format;
   image[1].u.tex.level = info->dst.level;
   image[1].u.tex.first_layer = 0;
   image[1].u.tex.last_layer = util_max_layer(info->dst.resource, info->dst.level);

   struct pipe_grid_info grid = {0};
   unsigned wg_dim =
      set_work_size(&grid, 8, 8, 1, info->dst.box.width, info->dst.box.height,
                    info->dst.box.depth);

   /* Get the shader key. */
   const struct util_format_description *dst_desc = util_format_description(info->dst.format);
   unsigned i = util_format_get_first_non_void_channel(info->dst.format);
   union si_compute_blit_shader_key options;
   options.key = 0;

   options.always_true = true;
   options.wg_dim = wg_dim;
   options.src_is_1d = info->src.resource->target == PIPE_TEXTURE_1D ||
                       info->src.resource->target == PIPE_TEXTURE_1D_ARRAY;
   options.dst_is_1d = info->dst.resource->target == PIPE_TEXTURE_1D ||
                       info->dst.resource->target == PIPE_TEXTURE_1D_ARRAY;
   options.src_is_msaa = info->src.resource->nr_samples > 1;
   options.dst_is_msaa = info->dst.resource->nr_samples > 1;
   /* Resolving integer formats only copies sample 0. log2_samples is then unused. */
   options.sample0_only = options.src_is_msaa && !options.dst_is_msaa &&
                          util_format_is_pure_integer(info->src.format);
   unsigned num_samples = MAX2(info->src.resource->nr_samples, info->dst.resource->nr_samples);
   options.log2_samples = options.sample0_only ? 0 : util_logbase2(num_samples);
   options.xy_clamp_to_edge = si_should_blit_clamp_xy(info);
   options.flip_x = info->src.box.width < 0;
   options.flip_y = info->src.box.height < 0;
   options.sint_to_uint = util_format_is_pure_sint(info->src.format) &&
                          util_format_is_pure_uint(info->dst.format);
   options.uint_to_sint = util_format_is_pure_uint(info->src.format) &&
                          util_format_is_pure_sint(info->dst.format);
   options.dst_is_srgb = util_format_is_srgb(info->dst.format);
   options.last_dst_channel = si_format_get_last_blit_component(info->dst.format, true);
   options.last_src_channel = MIN2(si_format_get_last_blit_component(info->src.format, false),
                                   options.last_dst_channel);
   options.use_integer_one = util_format_is_pure_integer(info->dst.format) &&
                             options.last_src_channel < options.last_dst_channel &&
                             options.last_dst_channel == 3;

   /* WARNING: We need this option for AMD_TEST to get results identical with the gfx blit,
    * otherwise we wouldn't be able to fully validate whether everything else works.
    * The test expects that the behavior is identical to u_blitter.
    *
    * Additionally, we need to keep this enabled even when not testing because not doing fp16_rtz
    * breaks "piglit/bin/texsubimage -auto pbo".
    */
   options.fp16_rtz = !util_format_is_pure_integer(info->dst.format) &&
                      dst_desc->channel[i].size <= 10;

   struct hash_entry *entry = _mesa_hash_table_search(sctx->cs_blit_shaders,
                                                      (void*)(uintptr_t)options.key);
   void *shader = entry ? entry->data : NULL;
   if (!shader) {
      shader = si_create_blit_cs(sctx, &options);
      _mesa_hash_table_insert(sctx->cs_blit_shaders,
                              (void*)(uintptr_t)options.key, shader);
   }

   sctx->cs_user_data[0] = (info->src.box.x & 0xffff) | ((info->dst.box.x & 0xffff) << 16);
   sctx->cs_user_data[1] = (info->src.box.y & 0xffff) | ((info->dst.box.y & 0xffff) << 16);
   sctx->cs_user_data[2] = (info->src.box.z & 0xffff) | ((info->dst.box.z & 0xffff) << 16);

   si_launch_grid_internal_images(sctx, image, 2, &grid, shader,
                                  SI_OP_SYNC_BEFORE_AFTER |
                                  (info->render_condition_enable ? SI_OP_CS_RENDER_COND_ENABLE : 0));
   return true;
}
