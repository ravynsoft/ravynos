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

#ifndef FD4_DRAW_H_
#define FD4_DRAW_H_

#include "pipe/p_context.h"

#include "freedreno_draw.h"

void fd4_draw_init(struct pipe_context *pctx);

/* draw packet changed on a4xx, so cannot reuse one from a2xx/a3xx.. */

static inline uint32_t
DRAW4(enum pc_di_primtype prim_type, enum pc_di_src_sel source_select,
      enum a4xx_index_size index_size, enum pc_di_vis_cull_mode vis_cull_mode)
{
   return CP_DRAW_INDX_OFFSET_0_PRIM_TYPE(prim_type) |
          CP_DRAW_INDX_OFFSET_0_SOURCE_SELECT(source_select) |
          CP_DRAW_INDX_OFFSET_0_INDEX_SIZE(index_size) |
          CP_DRAW_INDX_OFFSET_0_VIS_CULL(vis_cull_mode);
}

static inline void
fd4_draw(struct fd_batch *batch, struct fd_ringbuffer *ring,
         enum pc_di_primtype primtype, enum pc_di_vis_cull_mode vismode,
         enum pc_di_src_sel src_sel, uint32_t count, uint32_t instances,
         enum a4xx_index_size idx_type, uint32_t max_indices,
         uint32_t idx_offset, struct pipe_resource *idx_buffer)
{
   /* for debug after a lock up, write a unique counter value
    * to scratch7 for each draw, to make it easier to match up
    * register dumps to cmdstream.  The combination of IB
    * (scratch6) and DRAW is enough to "triangulate" the
    * particular draw that caused lockup.
    */
   emit_marker(ring, 7);

   OUT_PKT3(ring, CP_DRAW_INDX_OFFSET, idx_buffer ? 6 : 3);
   if (vismode == USE_VISIBILITY) {
      /* leave vis mode blank for now, it will be patched up when
       * we know if we are binning or not
       */
      OUT_RINGP(ring, DRAW4(primtype, src_sel, idx_type, 0),
                &batch->draw_patches);
   } else {
      OUT_RING(ring, DRAW4(primtype, src_sel, idx_type, vismode));
   }
   OUT_RING(ring, instances); /* NumInstances */
   OUT_RING(ring, count);     /* NumIndices */
   if (idx_buffer) {
      OUT_RING(ring, 0x0); /* XXX */
      OUT_RELOC(ring, fd_resource(idx_buffer)->bo, idx_offset, 0, 0);
      OUT_RING(ring, max_indices);
   }

   emit_marker(ring, 7);

   fd_reset_wfi(batch);
}

static inline void
fd4_draw_emit(struct fd_batch *batch, struct fd_ringbuffer *ring,
              enum pc_di_primtype primtype, enum pc_di_vis_cull_mode vismode,
              const struct pipe_draw_info *info,
              const struct pipe_draw_indirect_info *indirect,
              const struct pipe_draw_start_count_bias *draw, unsigned index_offset)
{
   struct pipe_resource *idx_buffer = NULL;
   enum a4xx_index_size idx_type;
   enum pc_di_src_sel src_sel;
   uint32_t idx_size, idx_offset;

   if (indirect && indirect->buffer) {
      struct fd_resource *ind = fd_resource(indirect->buffer);

      emit_marker(ring, 7);

      if (info->index_size) {
         struct pipe_resource *idx = info->index.resource;

         OUT_PKT3(ring, CP_DRAW_INDX_INDIRECT, 4);
         OUT_RINGP(ring,
                   DRAW4(primtype, DI_SRC_SEL_DMA,
                         fd4_size2indextype(info->index_size), 0),
                   &batch->draw_patches);
         OUT_RELOC(ring, fd_resource(idx)->bo, index_offset, 0, 0);
         OUT_RING(ring, A4XX_CP_DRAW_INDX_INDIRECT_2_INDX_SIZE(idx->width0 -
                                                               index_offset));
         OUT_RELOC(ring, ind->bo, indirect->offset, 0, 0);
      } else {
         OUT_PKT3(ring, CP_DRAW_INDIRECT, 2);
         OUT_RINGP(ring, DRAW4(primtype, DI_SRC_SEL_AUTO_INDEX, 0, 0),
                   &batch->draw_patches);
         OUT_RELOC(ring, ind->bo, indirect->offset, 0, 0);
      }

      emit_marker(ring, 7);
      fd_reset_wfi(batch);

      return;
   }

   if (info->index_size) {
      assert(!info->has_user_indices);

      idx_buffer = info->index.resource;
      idx_type = fd4_size2indextype(info->index_size);
      idx_size = info->index_size * draw->count;
      idx_offset = index_offset + draw->start * info->index_size;
      src_sel = DI_SRC_SEL_DMA;
   } else {
      idx_buffer = NULL;
      idx_type = INDEX4_SIZE_32_BIT;
      idx_size = 0;
      idx_offset = 0;
      src_sel = DI_SRC_SEL_AUTO_INDEX;
   }

   fd4_draw(batch, ring, primtype, vismode, src_sel, draw->count,
            info->instance_count, idx_type, idx_size, idx_offset, idx_buffer);
}

#endif /* FD4_DRAW_H_ */
