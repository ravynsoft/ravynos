/*
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_pipe.h"
#include "sid.h"
#include "si_build_pm4.h"

/* Set this if you want the ME to wait until CP DMA is done.
 * It should be set on the last CP DMA packet. */
#define CP_DMA_SYNC (1 << 0)

/* Set this if the source data was used as a destination in a previous CP DMA
 * packet. It's for preventing a read-after-write (RAW) hazard between two
 * CP DMA packets. */
#define CP_DMA_RAW_WAIT    (1 << 1)
#define CP_DMA_DST_IS_GDS  (1 << 2)
#define CP_DMA_CLEAR       (1 << 3)
#define CP_DMA_PFP_SYNC_ME (1 << 4)
#define CP_DMA_SRC_IS_GDS  (1 << 5)

/* The max number of bytes that can be copied per packet. */
static inline unsigned cp_dma_max_byte_count(struct si_context *sctx)
{
   unsigned max =
      sctx->gfx_level >= GFX11 ? 32767 :
      sctx->gfx_level >= GFX9 ? S_415_BYTE_COUNT_GFX9(~0u) : S_415_BYTE_COUNT_GFX6(~0u);

   /* make it aligned for optimal performance */
   return max & ~(SI_CPDMA_ALIGNMENT - 1);
}

/* should cp dma skip the hole in sparse bo */
static inline bool cp_dma_sparse_wa(struct si_context *sctx, struct si_resource *sdst)
{
   if ((sctx->gfx_level == GFX9) && sdst && (sdst->flags & RADEON_FLAG_SPARSE))
      return true;

   return false;
}

/* Emit a CP DMA packet to do a copy from one buffer to another, or to clear
 * a buffer. The size must fit in bits [20:0]. If CP_DMA_CLEAR is set, src_va is a 32-bit
 * clear value.
 */
static void si_emit_cp_dma(struct si_context *sctx, struct radeon_cmdbuf *cs, uint64_t dst_va,
                           uint64_t src_va, unsigned size, unsigned flags,
                           enum si_cache_policy cache_policy)
{
   uint32_t header = 0, command = 0;

   assert(size <= cp_dma_max_byte_count(sctx));
   assert(sctx->gfx_level != GFX6 || cache_policy == L2_BYPASS);

   if (sctx->gfx_level >= GFX9)
      command |= S_415_BYTE_COUNT_GFX9(size);
   else
      command |= S_415_BYTE_COUNT_GFX6(size);

   /* Sync flags. */
   if (flags & CP_DMA_SYNC)
      header |= S_411_CP_SYNC(1);

   if (flags & CP_DMA_RAW_WAIT)
      command |= S_415_RAW_WAIT(1);

   /* Src and dst flags. */
   if (sctx->gfx_level >= GFX9 && !(flags & CP_DMA_CLEAR) && src_va == dst_va) {
      header |= S_411_DST_SEL(V_411_NOWHERE); /* prefetch only */
   } else if (flags & CP_DMA_DST_IS_GDS) {
      header |= S_411_DST_SEL(V_411_GDS);
      /* GDS increments the address, not CP. */
      command |= S_415_DAS(V_415_REGISTER) | S_415_DAIC(V_415_NO_INCREMENT);
   } else if (sctx->gfx_level >= GFX7 && cache_policy != L2_BYPASS) {
      header |=
         S_501_DST_SEL(V_501_DST_ADDR_TC_L2) | S_501_DST_CACHE_POLICY(cache_policy == L2_STREAM);
   }

   if (flags & CP_DMA_CLEAR) {
      header |= S_411_SRC_SEL(V_411_DATA);
   } else if (flags & CP_DMA_SRC_IS_GDS) {
      header |= S_411_SRC_SEL(V_411_GDS);
      /* Both of these are required for GDS. It does increment the address. */
      command |= S_415_SAS(V_415_REGISTER) | S_415_SAIC(V_415_NO_INCREMENT);
   } else if (sctx->gfx_level >= GFX7 && cache_policy != L2_BYPASS) {
      header |=
         S_501_SRC_SEL(V_501_SRC_ADDR_TC_L2) | S_501_SRC_CACHE_POLICY(cache_policy == L2_STREAM);
   }

   radeon_begin(cs);

   if (sctx->gfx_level >= GFX7) {
      radeon_emit(PKT3(PKT3_DMA_DATA, 5, 0));
      radeon_emit(header);
      radeon_emit(src_va);       /* SRC_ADDR_LO [31:0] */
      radeon_emit(src_va >> 32); /* SRC_ADDR_HI [31:0] */
      radeon_emit(dst_va);       /* DST_ADDR_LO [31:0] */
      radeon_emit(dst_va >> 32); /* DST_ADDR_HI [31:0] */
      radeon_emit(command);
   } else {
      header |= S_411_SRC_ADDR_HI(src_va >> 32);

      radeon_emit(PKT3(PKT3_CP_DMA, 4, 0));
      radeon_emit(src_va);                  /* SRC_ADDR_LO [31:0] */
      radeon_emit(header);                  /* SRC_ADDR_HI [15:0] + flags. */
      radeon_emit(dst_va);                  /* DST_ADDR_LO [31:0] */
      radeon_emit((dst_va >> 32) & 0xffff); /* DST_ADDR_HI [15:0] */
      radeon_emit(command);
   }

   /* CP DMA is executed in ME, but index buffers are read by PFP.
    * This ensures that ME (CP DMA) is idle before PFP starts fetching
    * indices. If we wanted to execute CP DMA in PFP, this packet
    * should precede it.
    */
   if (sctx->has_graphics && flags & CP_DMA_PFP_SYNC_ME) {
      radeon_emit(PKT3(PKT3_PFP_SYNC_ME, 0, 0));
      radeon_emit(0);
   }
   radeon_end();
}

void si_cp_dma_wait_for_idle(struct si_context *sctx, struct radeon_cmdbuf *cs)
{
   /* Issue a dummy DMA that copies zero bytes.
    *
    * The DMA engine will see that there's no work to do and skip this
    * DMA request, however, the CP will see the sync flag and still wait
    * for all DMAs to complete.
    */
   si_emit_cp_dma(sctx, cs, 0, 0, 0, CP_DMA_SYNC, L2_BYPASS);
}

static void si_cp_dma_prepare(struct si_context *sctx, struct pipe_resource *dst,
                              struct pipe_resource *src, unsigned byte_count,
                              uint64_t remaining_size, unsigned user_flags, enum si_coherency coher,
                              bool *is_first, unsigned *packet_flags)
{
   if (!(user_flags & SI_OP_CPDMA_SKIP_CHECK_CS_SPACE))
      si_need_gfx_cs_space(sctx, 0);

   /* This must be done after need_cs_space. */
   if (dst)
      radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, si_resource(dst),
                                RADEON_USAGE_WRITE | RADEON_PRIO_CP_DMA);
   if (src)
      radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, si_resource(src),
                                RADEON_USAGE_READ | RADEON_PRIO_CP_DMA);

   /* Flush the caches for the first copy only.
    * Also wait for the previous CP DMA operations.
    */
   if (*is_first && sctx->flags)
      si_emit_cache_flush_direct(sctx);

   if (user_flags & SI_OP_SYNC_CPDMA_BEFORE && *is_first && !(*packet_flags & CP_DMA_CLEAR))
      *packet_flags |= CP_DMA_RAW_WAIT;

   *is_first = false;

   /* Do the synchronization after the last dma, so that all data
    * is written to memory.
    */
   if (user_flags & SI_OP_SYNC_AFTER && byte_count == remaining_size) {
      *packet_flags |= CP_DMA_SYNC;

      if (coher == SI_COHERENCY_SHADER)
         *packet_flags |= CP_DMA_PFP_SYNC_ME;
   }
}

void si_cp_dma_clear_buffer(struct si_context *sctx, struct radeon_cmdbuf *cs,
                            struct pipe_resource *dst, uint64_t offset, uint64_t size,
                            unsigned value, unsigned user_flags, enum si_coherency coher,
                            enum si_cache_policy cache_policy)
{
   struct si_resource *sdst = si_resource(dst);
   uint64_t va = (sdst ? sdst->gpu_address : 0) + offset;
   bool is_first = true;

   assert(size && size % 4 == 0);

   if (user_flags & SI_OP_SYNC_GE_BEFORE)
      sctx->flags |= SI_CONTEXT_VS_PARTIAL_FLUSH;

   if (user_flags & SI_OP_SYNC_CS_BEFORE)
      sctx->flags |= SI_CONTEXT_CS_PARTIAL_FLUSH;

   if (user_flags & SI_OP_SYNC_PS_BEFORE)
      sctx->flags |= SI_CONTEXT_PS_PARTIAL_FLUSH;

   /* Mark the buffer range of destination as valid (initialized),
    * so that transfer_map knows it should wait for the GPU when mapping
    * that range. */
   if (sdst) {
      util_range_add(dst, &sdst->valid_buffer_range, offset, offset + size);

      if (!(user_flags & SI_OP_SKIP_CACHE_INV_BEFORE))
         sctx->flags |= si_get_flush_flags(sctx, coher, cache_policy);
   }

   if (sctx->flags)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);

   while (size) {
      unsigned byte_count = MIN2(size, cp_dma_max_byte_count(sctx));
      unsigned dma_flags = CP_DMA_CLEAR | (sdst ? 0 : CP_DMA_DST_IS_GDS);

      if (cp_dma_sparse_wa(sctx,sdst)) {
         unsigned skip_count =
            sctx->ws->buffer_find_next_committed_memory(sdst->buf,
                  va - sdst->gpu_address, &byte_count);
         va += skip_count;
         size -= skip_count;
      }

      if (!byte_count)
         continue;

      si_cp_dma_prepare(sctx, dst, NULL, byte_count, size, user_flags, coher, &is_first,
                        &dma_flags);

      /* Emit the clear packet. */
      si_emit_cp_dma(sctx, cs, va, value, byte_count, dma_flags, cache_policy);

      size -= byte_count;
      va += byte_count;
   }

   if (sdst && cache_policy != L2_BYPASS)
      sdst->TC_L2_dirty = true;

   /* If it's not a framebuffer fast clear... */
   if (coher == SI_COHERENCY_SHADER)
      sctx->num_cp_dma_calls++;
}

/**
 * Realign the CP DMA engine. This must be done after a copy with an unaligned
 * size.
 *
 * \param size  Remaining size to the CP DMA alignment.
 */
static void si_cp_dma_realign_engine(struct si_context *sctx, unsigned size, unsigned user_flags,
                                     enum si_coherency coher, enum si_cache_policy cache_policy,
                                     bool *is_first)
{
   uint64_t va;
   unsigned dma_flags = 0;
   unsigned scratch_size = SI_CPDMA_ALIGNMENT * 2;

   assert(size < SI_CPDMA_ALIGNMENT);

   /* Use the scratch buffer as the dummy buffer. The 3D engine should be
    * idle at this point.
    */
   if (!sctx->scratch_buffer || sctx->scratch_buffer->b.b.width0 < scratch_size) {
      si_resource_reference(&sctx->scratch_buffer, NULL);
      sctx->scratch_buffer = si_aligned_buffer_create(&sctx->screen->b,
                                                      PIPE_RESOURCE_FLAG_UNMAPPABLE | SI_RESOURCE_FLAG_DRIVER_INTERNAL |
                                                      SI_RESOURCE_FLAG_DISCARDABLE,
                                                      PIPE_USAGE_DEFAULT, scratch_size, 256);
      if (!sctx->scratch_buffer)
         return;

      si_mark_atom_dirty(sctx, &sctx->atoms.s.scratch_state);
   }

   si_cp_dma_prepare(sctx, &sctx->scratch_buffer->b.b, &sctx->scratch_buffer->b.b, size, size,
                     user_flags, coher, is_first, &dma_flags);

   va = sctx->scratch_buffer->gpu_address;
   si_emit_cp_dma(sctx, &sctx->gfx_cs, va, va + SI_CPDMA_ALIGNMENT, size, dma_flags, cache_policy);
}

/**
 * Do memcpy between buffers using CP DMA.
 * If src or dst is NULL, it means read or write GDS, respectively.
 *
 * \param user_flags    bitmask of SI_CPDMA_*
 */
void si_cp_dma_copy_buffer(struct si_context *sctx, struct pipe_resource *dst,
                           struct pipe_resource *src, uint64_t dst_offset, uint64_t src_offset,
                           unsigned size, unsigned user_flags, enum si_coherency coher,
                           enum si_cache_policy cache_policy)
{
   uint64_t main_dst_offset, main_src_offset;
   unsigned skipped_size = 0;
   unsigned realign_size = 0;
   unsigned gds_flags = (dst ? 0 : CP_DMA_DST_IS_GDS) | (src ? 0 : CP_DMA_SRC_IS_GDS);
   bool is_first = true;

   assert(size);

   if (dst) {
      /* Skip this for the L2 prefetch. */
      if (dst != src || dst_offset != src_offset) {
         /* Mark the buffer range of destination as valid (initialized),
          * so that transfer_map knows it should wait for the GPU when mapping
          * that range. */
         util_range_add(dst, &si_resource(dst)->valid_buffer_range, dst_offset, dst_offset + size);
      }

      dst_offset += si_resource(dst)->gpu_address;
   }
   if (src)
      src_offset += si_resource(src)->gpu_address;

   /* The workarounds aren't needed on Fiji and beyond. */
   if (sctx->family <= CHIP_CARRIZO || sctx->family == CHIP_STONEY) {
      /* If the size is not aligned, we must add a dummy copy at the end
       * just to align the internal counter. Otherwise, the DMA engine
       * would slow down by an order of magnitude for following copies.
       */
      if (size % SI_CPDMA_ALIGNMENT)
         realign_size = SI_CPDMA_ALIGNMENT - (size % SI_CPDMA_ALIGNMENT);

      /* If the copy begins unaligned, we must start copying from the next
       * aligned block and the skipped part should be copied after everything
       * else has been copied. Only the src alignment matters, not dst.
       *
       * GDS doesn't need the source address to be aligned.
       */
      if (src && src_offset % SI_CPDMA_ALIGNMENT) {
         skipped_size = SI_CPDMA_ALIGNMENT - (src_offset % SI_CPDMA_ALIGNMENT);
         /* The main part will be skipped if the size is too small. */
         skipped_size = MIN2(skipped_size, size);
         size -= skipped_size;
      }
   }

   /* TMZ handling */
   if (unlikely(radeon_uses_secure_bos(sctx->ws))) {
      bool secure = src && (si_resource(src)->flags & RADEON_FLAG_ENCRYPTED);
      assert(!secure || (!dst || (si_resource(dst)->flags & RADEON_FLAG_ENCRYPTED)));
      if (secure != sctx->ws->cs_is_secure(&sctx->gfx_cs)) {
         si_flush_gfx_cs(sctx, RADEON_FLUSH_ASYNC_START_NEXT_GFX_IB_NOW |
                               RADEON_FLUSH_TOGGLE_SECURE_SUBMISSION, NULL);
      }
   }

   if (user_flags & SI_OP_SYNC_GE_BEFORE)
      sctx->flags |= SI_CONTEXT_VS_PARTIAL_FLUSH;

   if (user_flags & SI_OP_SYNC_CS_BEFORE)
      sctx->flags |= SI_CONTEXT_CS_PARTIAL_FLUSH;

   if (user_flags & SI_OP_SYNC_PS_BEFORE)
      sctx->flags |= SI_CONTEXT_PS_PARTIAL_FLUSH;

   if ((dst || src) && !(user_flags & SI_OP_SKIP_CACHE_INV_BEFORE))
         sctx->flags |= si_get_flush_flags(sctx, coher, cache_policy);

   if (sctx->flags)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);

   /* This is the main part doing the copying. Src is always aligned. */
   main_dst_offset = dst_offset + skipped_size;
   main_src_offset = src_offset + skipped_size;

   while (size) {
      unsigned byte_count = MIN2(size, cp_dma_max_byte_count(sctx));
      unsigned dma_flags = gds_flags;

      if (cp_dma_sparse_wa(sctx, si_resource(dst))) {
         unsigned skip_count =
            sctx->ws->buffer_find_next_committed_memory(si_resource(dst)->buf,
                  main_dst_offset - si_resource(dst)->gpu_address, &byte_count);
         main_dst_offset += skip_count;
         main_src_offset += skip_count;
         size -= skip_count;
      }

      if (cp_dma_sparse_wa(sctx, si_resource(src))) {
         unsigned skip_count =
            sctx->ws->buffer_find_next_committed_memory(si_resource(src)->buf,
                  main_src_offset - si_resource(src)->gpu_address, &byte_count);
         main_dst_offset += skip_count;
         main_src_offset += skip_count;
         size -= skip_count;
      }

      if (!byte_count)
         continue;

      si_cp_dma_prepare(sctx, dst, src, byte_count, size + skipped_size + realign_size, user_flags,
                        coher, &is_first, &dma_flags);

      si_emit_cp_dma(sctx, &sctx->gfx_cs, main_dst_offset, main_src_offset, byte_count, dma_flags,
                     cache_policy);

      size -= byte_count;
      main_src_offset += byte_count;
      main_dst_offset += byte_count;
   }

   /* Copy the part we skipped because src wasn't aligned. */
   if (skipped_size) {
      unsigned dma_flags = gds_flags;

      si_cp_dma_prepare(sctx, dst, src, skipped_size, skipped_size + realign_size, user_flags,
                        coher, &is_first, &dma_flags);

      si_emit_cp_dma(sctx, &sctx->gfx_cs, dst_offset, src_offset, skipped_size, dma_flags,
                     cache_policy);
   }

   /* Finally, realign the engine if the size wasn't aligned. */
   if (realign_size) {
      si_cp_dma_realign_engine(sctx, realign_size, user_flags, coher, cache_policy, &is_first);
   }

   if (dst && cache_policy != L2_BYPASS)
      si_resource(dst)->TC_L2_dirty = true;

   /* If it's not a prefetch or GDS copy... */
   if (dst && src && (dst != src || dst_offset != src_offset))
      sctx->num_cp_dma_calls++;
}

void si_test_gds(struct si_context *sctx)
{
   struct pipe_context *ctx = &sctx->b;
   struct pipe_resource *src, *dst;
   unsigned r[4] = {};
   unsigned offset = debug_get_num_option("OFFSET", 16);

   src = pipe_buffer_create(ctx->screen, 0, PIPE_USAGE_DEFAULT, 16);
   dst = pipe_buffer_create(ctx->screen, 0, PIPE_USAGE_DEFAULT, 16);
   si_cp_dma_clear_buffer(sctx, &sctx->gfx_cs, src, 0, 4, 0xabcdef01, SI_OP_SYNC_BEFORE_AFTER,
                          SI_COHERENCY_SHADER, L2_BYPASS);
   si_cp_dma_clear_buffer(sctx, &sctx->gfx_cs, src, 4, 4, 0x23456789, SI_OP_SYNC_BEFORE_AFTER,
                          SI_COHERENCY_SHADER, L2_BYPASS);
   si_cp_dma_clear_buffer(sctx, &sctx->gfx_cs, src, 8, 4, 0x87654321, SI_OP_SYNC_BEFORE_AFTER,
                          SI_COHERENCY_SHADER, L2_BYPASS);
   si_cp_dma_clear_buffer(sctx, &sctx->gfx_cs, src, 12, 4, 0xfedcba98, SI_OP_SYNC_BEFORE_AFTER,
                          SI_COHERENCY_SHADER, L2_BYPASS);
   si_cp_dma_clear_buffer(sctx, &sctx->gfx_cs, dst, 0, 16, 0xdeadbeef, SI_OP_SYNC_BEFORE_AFTER,
                          SI_COHERENCY_SHADER, L2_BYPASS);

   si_cp_dma_copy_buffer(sctx, NULL, src, offset, 0, 16, SI_OP_SYNC_BEFORE_AFTER,
                         SI_COHERENCY_NONE, L2_BYPASS);
   si_cp_dma_copy_buffer(sctx, dst, NULL, 0, offset, 16, SI_OP_SYNC_BEFORE_AFTER,
                         SI_COHERENCY_NONE, L2_BYPASS);

   pipe_buffer_read(ctx, dst, 0, sizeof(r), r);
   printf("GDS copy  = %08x %08x %08x %08x -> %s\n", r[0], r[1], r[2], r[3],
          r[0] == 0xabcdef01 && r[1] == 0x23456789 && r[2] == 0x87654321 && r[3] == 0xfedcba98
             ? "pass"
             : "fail");

   si_cp_dma_clear_buffer(sctx, &sctx->gfx_cs, NULL, offset, 16, 0xc1ea4146,
                          SI_OP_SYNC_BEFORE_AFTER, SI_COHERENCY_NONE, L2_BYPASS);
   si_cp_dma_copy_buffer(sctx, dst, NULL, 0, offset, 16, SI_OP_SYNC_BEFORE_AFTER,
                         SI_COHERENCY_NONE, L2_BYPASS);

   pipe_buffer_read(ctx, dst, 0, sizeof(r), r);
   printf("GDS clear = %08x %08x %08x %08x -> %s\n", r[0], r[1], r[2], r[3],
          r[0] == 0xc1ea4146 && r[1] == 0xc1ea4146 && r[2] == 0xc1ea4146 && r[3] == 0xc1ea4146
             ? "pass"
             : "fail");

   pipe_resource_reference(&src, NULL);
   pipe_resource_reference(&dst, NULL);
   exit(0);
}

void si_cp_write_data(struct si_context *sctx, struct si_resource *buf, unsigned offset,
                      unsigned size, unsigned dst_sel, unsigned engine, const void *data)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;

   assert(offset % 4 == 0);
   assert(size % 4 == 0);

   if (sctx->gfx_level == GFX6 && dst_sel == V_370_MEM)
      dst_sel = V_370_MEM_GRBM;

   radeon_add_to_buffer_list(sctx, cs, buf, RADEON_USAGE_WRITE | RADEON_PRIO_CP_DMA);
   uint64_t va = buf->gpu_address + offset;

   radeon_begin(cs);
   radeon_emit(PKT3(PKT3_WRITE_DATA, 2 + size / 4, 0));
   radeon_emit(S_370_DST_SEL(dst_sel) | S_370_WR_CONFIRM(1) | S_370_ENGINE_SEL(engine));
   radeon_emit(va);
   radeon_emit(va >> 32);
   radeon_emit_array((const uint32_t *)data, size / 4);
   radeon_end();
}

void si_cp_copy_data(struct si_context *sctx, struct radeon_cmdbuf *cs, unsigned dst_sel,
                     struct si_resource *dst, unsigned dst_offset, unsigned src_sel,
                     struct si_resource *src, unsigned src_offset)
{
   /* cs can point to the compute IB, which has the buffer list in gfx_cs. */
   if (dst) {
      radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, dst, RADEON_USAGE_WRITE | RADEON_PRIO_CP_DMA);
   }
   if (src) {
      radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, src, RADEON_USAGE_READ | RADEON_PRIO_CP_DMA);
   }

   uint64_t dst_va = (dst ? dst->gpu_address : 0ull) + dst_offset;
   uint64_t src_va = (src ? src->gpu_address : 0ull) + src_offset;

   radeon_begin(cs);
   radeon_emit(PKT3(PKT3_COPY_DATA, 4, 0));
   radeon_emit(COPY_DATA_SRC_SEL(src_sel) | COPY_DATA_DST_SEL(dst_sel) | COPY_DATA_WR_CONFIRM);
   radeon_emit(src_va);
   radeon_emit(src_va >> 32);
   radeon_emit(dst_va);
   radeon_emit(dst_va >> 32);
   radeon_end();
}
