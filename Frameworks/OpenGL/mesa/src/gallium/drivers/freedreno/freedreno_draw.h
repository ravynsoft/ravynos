/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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

#ifndef FREEDRENO_DRAW_H_
#define FREEDRENO_DRAW_H_

#include "pipe/p_context.h"
#include "pipe/p_state.h"

#include "freedreno_context.h"
#include "freedreno_resource.h"
#include "freedreno_screen.h"
#include "freedreno_util.h"

struct fd_ringbuffer;

void fd_draw_init(struct pipe_context *pctx);

#ifndef __cplusplus
static inline void
fd_draw(struct fd_batch *batch, struct fd_ringbuffer *ring,
        enum pc_di_primtype primtype, enum pc_di_vis_cull_mode vismode,
        enum pc_di_src_sel src_sel, uint32_t count, uint8_t instances,
        enum pc_di_index_size idx_type, uint32_t idx_size, uint32_t idx_offset,
        struct pipe_resource *idx_buffer)
{
   /* for debug after a lock up, write a unique counter value
    * to scratch7 for each draw, to make it easier to match up
    * register dumps to cmdstream.  The combination of IB
    * (scratch6) and DRAW is enough to "triangulate" the
    * particular draw that caused lockup.
    */
   emit_marker(ring, 7);

   if (is_a3xx_p0(batch->ctx->screen)) {
      /* dummy-draw workaround: */
      OUT_PKT3(ring, CP_DRAW_INDX, 3);
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, DRAW(1, DI_SRC_SEL_AUTO_INDEX, INDEX_SIZE_IGN,
                          USE_VISIBILITY, 0));
      OUT_RING(ring, 0); /* NumIndices */

      /* ugg, hard-code register offset to avoid pulling in the
       * a3xx register headers into something #included from a2xx
       */
      OUT_PKT0(ring, 0x2206, 1); /* A3XX_HLSQ_CONST_VSPRESV_RANGE_REG */
      OUT_RING(ring, 0);
   }

   if (is_a20x(batch->ctx->screen)) {
      /* a20x has a different draw command for drawing with binning data
       * note: if we do patching we will have to insert a NOP
       *
       * binning data is is 1 byte/vertex (8x8x4 bin position of vertex)
       * base ptr set by the CP_SET_DRAW_INIT_FLAGS command
       *
       * TODO: investigate the faceness_cull_select parameter to see how
       * it is used with hw binning to use "faceness" bits
       */
      uint32_t size = 2;
      if (vismode)
         size += 2;
      if (idx_buffer)
         size += 2;

      BEGIN_RING(ring, size + 1);
      if (vismode)
         util_dynarray_append(&batch->draw_patches, uint32_t *, ring->cur);

      OUT_PKT3(ring, vismode ? CP_DRAW_INDX_BIN : CP_DRAW_INDX, size);
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, DRAW_A20X(primtype, DI_FACE_CULL_NONE, src_sel, idx_type,
                               vismode, vismode, count));
      if (vismode == USE_VISIBILITY) {
         OUT_RING(ring, batch->num_vertices);
         OUT_RING(ring, count);
      }
   } else {
      OUT_PKT3(ring, CP_DRAW_INDX, idx_buffer ? 5 : 3);
      OUT_RING(ring, 0x00000000); /* viz query info. */
      if (vismode == USE_VISIBILITY) {
         /* leave vis mode blank for now, it will be patched up when
          * we know if we are binning or not
          */
         OUT_RINGP(ring, DRAW(primtype, src_sel, idx_type, 0, instances),
                   &batch->draw_patches);
      } else {
         OUT_RING(ring, DRAW(primtype, src_sel, idx_type, vismode, instances));
      }
      OUT_RING(ring, count); /* NumIndices */
   }

   if (idx_buffer) {
      OUT_RELOC(ring, fd_resource(idx_buffer)->bo, idx_offset, 0, 0);
      OUT_RING(ring, idx_size);
   }

   emit_marker(ring, 7);

   fd_reset_wfi(batch);
}

static inline enum pc_di_index_size
size2indextype(unsigned index_size)
{
   switch (index_size) {
   case 1:
      return INDEX_SIZE_8_BIT;
   case 2:
      return INDEX_SIZE_16_BIT;
   case 4:
      return INDEX_SIZE_32_BIT;
   }
   DBG("unsupported index size: %d", index_size);
   assert(0);
   return INDEX_SIZE_IGN;
}

/* this is same for a2xx/a3xx, so split into helper: */
static inline void
fd_draw_emit(struct fd_batch *batch, struct fd_ringbuffer *ring,
             enum pc_di_primtype primtype, enum pc_di_vis_cull_mode vismode,
             const struct pipe_draw_info *info,
             const struct pipe_draw_start_count_bias *draw, unsigned index_offset)
{
   struct pipe_resource *idx_buffer = NULL;
   enum pc_di_index_size idx_type = INDEX_SIZE_IGN;
   enum pc_di_src_sel src_sel;
   uint32_t idx_size, idx_offset;

   if (info->index_size) {
      assert(!info->has_user_indices);

      idx_buffer = info->index.resource;
      idx_type = size2indextype(info->index_size);
      idx_size = info->index_size * draw->count;
      idx_offset = index_offset + draw->start * info->index_size;
      src_sel = DI_SRC_SEL_DMA;
   } else {
      idx_buffer = NULL;
      idx_type = INDEX_SIZE_IGN;
      idx_size = 0;
      idx_offset = 0;
      src_sel = DI_SRC_SEL_AUTO_INDEX;
   }

   fd_draw(batch, ring, primtype, vismode, src_sel, draw->count,
           info->instance_count - 1, idx_type, idx_size, idx_offset,
           idx_buffer);
}
#endif

static inline void
fd_blend_tracking(struct fd_context *ctx)
   assert_dt
{
   if (ctx->dirty & FD_DIRTY_BLEND) {
      struct fd_batch *batch = ctx->batch;
      struct pipe_framebuffer_state *pfb = &batch->framebuffer;

      if (ctx->blend->logicop_enable)
         batch->gmem_reason |= FD_GMEM_LOGICOP_ENABLED;
      for (unsigned i = 0; i < pfb->nr_cbufs; i++) {
         if (ctx->blend->rt[i].blend_enable)
            batch->gmem_reason |= FD_GMEM_BLEND_ENABLED;
      }
   }
}

#endif /* FREEDRENO_DRAW_H_ */
