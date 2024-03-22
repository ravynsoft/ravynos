/*
 * Copyright © 2023 Google, Inc.
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

#include "freedreno_batch.h"

#include "fd6_barrier.h"
#include "fd6_context.h"

/* TODO probably more of the various fd6_event_write() should be
 * consolidated here.
 */

static uint32_t
event_write(struct fd_context *ctx, struct fd_ringbuffer *ring,
            enum vgt_event_type evt)
{
   bool timestamp = false;
   switch (evt) {
   case CACHE_FLUSH_TS:
   case WT_DONE_TS:
   case RB_DONE_TS:
   case PC_CCU_FLUSH_DEPTH_TS:
   case PC_CCU_FLUSH_COLOR_TS:
   case PC_CCU_RESOLVE_TS:
      timestamp = true;
      break;
   default:
      break;
   }

   OUT_PKT7(ring, CP_EVENT_WRITE, timestamp ? 4 : 1);
   OUT_RING(ring, CP_EVENT_WRITE_0_EVENT(evt));
   if (timestamp) {
      struct fd6_context *fd6_ctx = fd6_context(ctx);
      uint32_t seqno = ++fd6_ctx->seqno;
      OUT_RELOC(ring, control_ptr(fd6_ctx, seqno)); /* ADDR_LO/HI */
      OUT_RING(ring, seqno);

      return seqno;
   }

   return 0;
}

void
fd6_emit_flushes(struct fd_context *ctx, struct fd_ringbuffer *ring,
                 unsigned flushes)
{
   /* Experiments show that invalidating CCU while it still has data in it
    * doesn't work, so make sure to always flush before invalidating in case
    * any data remains that hasn't yet been made available through a barrier.
    * However it does seem to work for UCHE.
    */
   if (flushes & (FD6_FLUSH_CCU_COLOR | FD6_INVALIDATE_CCU_COLOR))
      event_write(ctx, ring, PC_CCU_FLUSH_COLOR_TS);

   if (flushes & (FD6_FLUSH_CCU_DEPTH | FD6_INVALIDATE_CCU_DEPTH))
      event_write(ctx, ring, PC_CCU_FLUSH_DEPTH_TS);

   if (flushes & FD6_INVALIDATE_CCU_COLOR)
      event_write(ctx, ring, PC_CCU_INVALIDATE_COLOR);

   if (flushes & FD6_INVALIDATE_CCU_DEPTH)
      event_write(ctx, ring, PC_CCU_INVALIDATE_DEPTH);

   if (flushes & FD6_FLUSH_CACHE)
      event_write(ctx, ring, CACHE_FLUSH_TS);

   if (flushes & FD6_INVALIDATE_CACHE)
      event_write(ctx, ring, CACHE_INVALIDATE);

   if (flushes & FD6_WAIT_MEM_WRITES)
      OUT_PKT7(ring, CP_WAIT_MEM_WRITES, 0);

   if (flushes & FD6_WAIT_FOR_IDLE)
      OUT_PKT7(ring, CP_WAIT_FOR_IDLE, 0);

   if (flushes & FD6_WAIT_FOR_ME)
      OUT_PKT7(ring, CP_WAIT_FOR_ME, 0);
}

void
fd6_barrier_flush(struct fd_batch *batch)
{
   fd6_emit_flushes(batch->ctx, batch->draw, batch->barrier);
   batch->barrier = 0;
}

static void
add_flushes(struct pipe_context *pctx, unsigned flushes)
   assert_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_batch *batch = NULL;

   /* If there is an active compute/nondraw batch, that is the one
    * we want to add the flushes to.  Ie. last op was a launch_grid,
    * if the next one is a launch_grid then the barriers should come
    * between them.  If the next op is a draw_vbo then the batch
    * switch is a sufficient barrier so it doesn't really matter.
    */
   fd_batch_reference(&batch, ctx->batch_nondraw);
   if (!batch)
      fd_batch_reference(&batch, ctx->batch);

   /* A batch flush is already a sufficient barrier: */
   if (!batch)
      return;

   batch->barrier |= flushes;

   fd_batch_reference(&batch, NULL);
}

static void
fd6_texture_barrier(struct pipe_context *pctx, unsigned flags)
   in_dt
{
   unsigned flushes = 0;

   if (flags & PIPE_TEXTURE_BARRIER_SAMPLER) {
      /* If we are sampling from the fb, we could get away with treating
       * this as a PIPE_TEXTURE_BARRIER_FRAMEBUFFER in sysmem mode, but
       * that won't work out in gmem mode because we don't patch the tex
       * state outside of the case that the frag shader tells us it is
       * an fb-read.  And in particular, the fb-read case guarantees us
       * that the read will be from the same texel, but the fb-bound-as-
       * tex case does not.
       *
       * We could try to be clever here and detect if zsbuf/cbuf[n] is
       * bound as a texture, but that doesn't really help if it is bound
       * as a texture after the barrier without a lot of extra book-
       * keeping.  So hopefully no one calls glTextureBarrierNV() just
       * for lolz.
       */
      pctx->flush(pctx, NULL, 0);
      return;
   }

   if (flags & PIPE_TEXTURE_BARRIER_FRAMEBUFFER) {
      flushes |= FD6_WAIT_FOR_IDLE | FD6_WAIT_FOR_ME |
            FD6_FLUSH_CCU_COLOR | FD6_FLUSH_CCU_DEPTH |
            FD6_FLUSH_CACHE | FD6_INVALIDATE_CACHE;
   }

   add_flushes(pctx, flushes);
}

static void
fd6_memory_barrier(struct pipe_context *pctx, unsigned flags)
   in_dt
{
   unsigned flushes = 0;

   if (flags & (PIPE_BARRIER_SHADER_BUFFER |
                PIPE_BARRIER_CONSTANT_BUFFER |
                PIPE_BARRIER_VERTEX_BUFFER |
                PIPE_BARRIER_INDEX_BUFFER |
                PIPE_BARRIER_STREAMOUT_BUFFER)) {
      flushes |= FD6_WAIT_FOR_IDLE;
   }

   if (flags & (PIPE_BARRIER_TEXTURE |
                PIPE_BARRIER_IMAGE |
                PIPE_BARRIER_UPDATE_BUFFER |
                PIPE_BARRIER_UPDATE_TEXTURE)) {
      flushes |= FD6_FLUSH_CACHE | FD6_WAIT_FOR_IDLE;
   }

   if (flags & PIPE_BARRIER_INDIRECT_BUFFER) {
      flushes |= FD6_FLUSH_CACHE | FD6_WAIT_FOR_IDLE;

     /* Various firmware bugs/inconsistencies mean that some indirect draw opcodes
      * do not wait for WFI's to complete before executing. Add a WAIT_FOR_ME if
      * pending for these opcodes. This may result in a few extra WAIT_FOR_ME's
      * with these opcodes, but the alternative would add unnecessary WAIT_FOR_ME's
      * before draw opcodes that don't need it.
      */
      if (fd_context(pctx)->screen->info->a6xx.indirect_draw_wfm_quirk) {
         flushes |= FD6_WAIT_FOR_ME;
      }
   }

   if (flags & PIPE_BARRIER_FRAMEBUFFER) {
      fd6_texture_barrier(pctx, PIPE_TEXTURE_BARRIER_FRAMEBUFFER);
   }

   add_flushes(pctx, flushes);
}

void
fd6_barrier_init(struct pipe_context *pctx)
{
   pctx->texture_barrier = fd6_texture_barrier;
   pctx->memory_barrier = fd6_memory_barrier;
}
