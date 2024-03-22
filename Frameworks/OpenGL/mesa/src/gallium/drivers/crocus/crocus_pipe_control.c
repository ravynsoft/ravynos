/*
 * Copyright © 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file crocus_pipe_control.c
 *
 * PIPE_CONTROL is the main flushing and synchronization primitive on Intel
 * GPUs.  It can invalidate caches, stall until rendering reaches various
 * stages of completion, write to memory, and other things.  In a way, it's
 * a swiss army knife command - it has all kinds of capabilities, but some
 * significant limitations as well.
 *
 * Unfortunately, it's notoriously complicated and difficult to use.  Many
 * sub-commands can't be used together.  Some are meant to be used at the
 * top of the pipeline (invalidating caches before drawing), while some are
 * meant to be used at the end (stalling or flushing after drawing).
 *
 * Also, there's a list of restrictions a mile long, which vary by generation.
 * Do this before doing that, or suffer the consequences (usually a GPU hang).
 *
 * This file contains helpers for emitting them safely.  You can simply call
 * crocus_emit_pipe_control_flush() with the desired operations (as logical
 * PIPE_CONTROL_* bits), and it will take care of splitting it into multiple
 * PIPE_CONTROL commands as necessary.  The per-generation workarounds are
 * applied in crocus_emit_raw_pipe_control() in crocus_state.c.
 */

#include "crocus_context.h"
#include "util/hash_table.h"
#include "util/set.h"

/**
 * Emit a PIPE_CONTROL with various flushing flags.
 *
 * The caller is responsible for deciding what flags are appropriate for the
 * given generation.
 */
void
crocus_emit_pipe_control_flush(struct crocus_batch *batch,
                               const char *reason,
                               uint32_t flags)
{
   const struct intel_device_info *devinfo = &batch->screen->devinfo;

   if (devinfo->ver >= 6 &&
       (flags & PIPE_CONTROL_CACHE_FLUSH_BITS) &&
       (flags & PIPE_CONTROL_CACHE_INVALIDATE_BITS)) {
      /* A pipe control command with flush and invalidate bits set
       * simultaneously is an inherently racy operation on Gen6+ if the
       * contents of the flushed caches were intended to become visible from
       * any of the invalidated caches.  Split it in two PIPE_CONTROLs, the
       * first one should stall the pipeline to make sure that the flushed R/W
       * caches are coherent with memory once the specified R/O caches are
       * invalidated.  On pre-Gen6 hardware the (implicit) R/O cache
       * invalidation seems to happen at the bottom of the pipeline together
       * with any write cache flush, so this shouldn't be a concern.  In order
       * to ensure a full stall, we do an end-of-pipe sync.
       */
      crocus_emit_end_of_pipe_sync(batch, reason,
                                   flags & PIPE_CONTROL_CACHE_FLUSH_BITS);
      flags &= ~(PIPE_CONTROL_CACHE_FLUSH_BITS | PIPE_CONTROL_CS_STALL);
   }

   batch->screen->vtbl.emit_raw_pipe_control(batch, reason, flags, NULL, 0, 0);
}

/**
 * Emit a PIPE_CONTROL that writes to a buffer object.
 *
 * \p flags should contain one of the following items:
 *  - PIPE_CONTROL_WRITE_IMMEDIATE
 *  - PIPE_CONTROL_WRITE_TIMESTAMP
 *  - PIPE_CONTROL_WRITE_DEPTH_COUNT
 */
void
crocus_emit_pipe_control_write(struct crocus_batch *batch,
                               const char *reason, uint32_t flags,
                               struct crocus_bo *bo, uint32_t offset,
                               uint64_t imm)
{
   batch->screen->vtbl.emit_raw_pipe_control(batch, reason, flags, bo, offset, imm);
}

/**
 * Restriction [DevSNB, DevIVB]:
 *
 * Prior to changing Depth/Stencil Buffer state (i.e. any combination of
 * 3DSTATE_DEPTH_BUFFER, 3DSTATE_CLEAR_PARAMS, 3DSTATE_STENCIL_BUFFER,
 * 3DSTATE_HIER_DEPTH_BUFFER) SW must first issue a pipelined depth stall
 * (PIPE_CONTROL with Depth Stall bit set), followed by a pipelined depth
 * cache flush (PIPE_CONTROL with Depth Flush Bit set), followed by
 * another pipelined depth stall (PIPE_CONTROL with Depth Stall bit set),
 * unless SW can otherwise guarantee that the pipeline from WM onwards is
 * already flushed (e.g., via a preceding MI_FLUSH).
 */
void
crocus_emit_depth_stall_flushes(struct crocus_batch *batch)
{
   UNUSED const struct intel_device_info *devinfo = &batch->screen->devinfo;

   assert(devinfo->ver >= 6);

   /* Starting on BDW, these pipe controls are unnecessary.
    *
    *   WM HW will internally manage the draining pipe and flushing of the caches
    *   when this command is issued. The PIPE_CONTROL restrictions are removed.
    */
   if (devinfo->ver >= 8)
      return;

   crocus_emit_pipe_control_flush(batch, "depth stall", PIPE_CONTROL_DEPTH_STALL);
   crocus_emit_pipe_control_flush(batch, "depth stall", PIPE_CONTROL_DEPTH_CACHE_FLUSH);
   crocus_emit_pipe_control_flush(batch, "depth stall", PIPE_CONTROL_DEPTH_STALL);
}

/*
 * From Sandybridge PRM, volume 2, "1.7.2 End-of-Pipe Synchronization":
 *
 *  Write synchronization is a special case of end-of-pipe
 *  synchronization that requires that the render cache and/or depth
 *  related caches are flushed to memory, where the data will become
 *  globally visible. This type of synchronization is required prior to
 *  SW (CPU) actually reading the result data from memory, or initiating
 *  an operation that will use as a read surface (such as a texture
 *  surface) a previous render target and/or depth/stencil buffer
 *
 * From Haswell PRM, volume 2, part 1, "End-of-Pipe Synchronization":
 *
 *  Exercising the write cache flush bits (Render Target Cache Flush
 *  Enable, Depth Cache Flush Enable, DC Flush) in PIPE_CONTROL only
 *  ensures the write caches are flushed and doesn't guarantee the data
 *  is globally visible.
 *
 *  SW can track the completion of the end-of-pipe-synchronization by
 *  using "Notify Enable" and "PostSync Operation - Write Immediate
 *  Data" in the PIPE_CONTROL command.
 */
void
crocus_emit_end_of_pipe_sync(struct crocus_batch *batch,
                             const char *reason, uint32_t flags)
{
   const struct intel_device_info *devinfo = &batch->screen->devinfo;

   if (devinfo->ver >= 6) {
      /* From Sandybridge PRM, volume 2, "1.7.3.1 Writing a Value to Memory":
       *
       *    "The most common action to perform upon reaching a synchronization
       *    point is to write a value out to memory. An immediate value
       *    (included with the synchronization command) may be written."
       *
       * From Broadwell PRM, volume 7, "End-of-Pipe Synchronization":
       *
       *    "In case the data flushed out by the render engine is to be read
       *    back in to the render engine in coherent manner, then the render
       *    engine has to wait for the fence completion before accessing the
       *    flushed data. This can be achieved by following means on various
       *    products: PIPE_CONTROL command with CS Stall and the required
       *    write caches flushed with Post-Sync-Operation as Write Immediate
       *    Data.
       *
       *    Example:
       *       - Workload-1 (3D/GPGPU/MEDIA)
       *       - PIPE_CONTROL (CS Stall, Post-Sync-Operation Write Immediate
       *         Data, Required Write Cache Flush bits set)
       *       - Workload-2 (Can use the data produce or output by Workload-1)
       */
      crocus_emit_pipe_control_write(batch, reason,
                                     flags | PIPE_CONTROL_CS_STALL |
                                     PIPE_CONTROL_WRITE_IMMEDIATE,
                                     batch->ice->workaround_bo,
                                     batch->ice->workaround_offset, 0);

      if (batch->screen->devinfo.platform == INTEL_PLATFORM_HSW) {
#define GEN7_3DPRIM_START_INSTANCE      0x243C
         batch->screen->vtbl.load_register_mem32(batch, GEN7_3DPRIM_START_INSTANCE,
                                                 batch->ice->workaround_bo,
                                                 batch->ice->workaround_offset);
      }
   } else {
      /* On gen4-5, a regular pipe control seems to suffice. */
      crocus_emit_pipe_control_flush(batch, reason, flags);
   }
}

/* Emit a pipelined flush to either flush render and texture cache for
 * reading from a FBO-drawn texture, or flush so that frontbuffer
 * render appears on the screen in DRI1.
 *
 * This is also used for the always_flush_cache driconf debug option.
 */
void
crocus_emit_mi_flush(struct crocus_batch *batch)
{
   const struct intel_device_info *devinfo = &batch->screen->devinfo;
   int flags = PIPE_CONTROL_RENDER_TARGET_FLUSH;
   if (devinfo->ver >= 6) {
      flags |= PIPE_CONTROL_INSTRUCTION_INVALIDATE |
               PIPE_CONTROL_CONST_CACHE_INVALIDATE |
               PIPE_CONTROL_DATA_CACHE_FLUSH |
               PIPE_CONTROL_DEPTH_CACHE_FLUSH |
               PIPE_CONTROL_VF_CACHE_INVALIDATE |
               PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
               PIPE_CONTROL_CS_STALL;
   }
   crocus_emit_pipe_control_flush(batch, "mi flush", flags);
}

/**
 * Emits a PIPE_CONTROL with a non-zero post-sync operation, for
 * implementing two workarounds on gen6.  From section 1.4.7.1
 * "PIPE_CONTROL" of the Sandy Bridge PRM volume 2 part 1:
 *
 * [DevSNB-C+{W/A}] Before any depth stall flush (including those
 * produced by non-pipelined state commands), software needs to first
 * send a PIPE_CONTROL with no bits set except Post-Sync Operation !=
 * 0.
 *
 * [Dev-SNB{W/A}]: Before a PIPE_CONTROL with Write Cache Flush Enable
 * =1, a PIPE_CONTROL with any non-zero post-sync-op is required.
 *
 * And the workaround for these two requires this workaround first:
 *
 * [Dev-SNB{W/A}]: Pipe-control with CS-stall bit set must be sent
 * BEFORE the pipe-control with a post-sync op and no write-cache
 * flushes.
 *
 * And this last workaround is tricky because of the requirements on
 * that bit.  From section 1.4.7.2.3 "Stall" of the Sandy Bridge PRM
 * volume 2 part 1:
 *
 *     "1 of the following must also be set:
 *      - Render Target Cache Flush Enable ([12] of DW1)
 *      - Depth Cache Flush Enable ([0] of DW1)
 *      - Stall at Pixel Scoreboard ([1] of DW1)
 *      - Depth Stall ([13] of DW1)
 *      - Post-Sync Operation ([13] of DW1)
 *      - Notify Enable ([8] of DW1)"
 *
 * The cache flushes require the workaround flush that triggered this
 * one, so we can't use it.  Depth stall would trigger the same.
 * Post-sync nonzero is what triggered this second workaround, so we
 * can't use that one either.  Notify enable is IRQs, which aren't
 * really our business.  That leaves only stall at scoreboard.
 */
void
crocus_emit_post_sync_nonzero_flush(struct crocus_batch *batch)
{
   crocus_emit_pipe_control_flush(batch, "nonzero",
                                  PIPE_CONTROL_CS_STALL |
                                  PIPE_CONTROL_STALL_AT_SCOREBOARD);

   crocus_emit_pipe_control_write(batch, "nonzero",
                                  PIPE_CONTROL_WRITE_IMMEDIATE,
                                  batch->ice->workaround_bo,
                                  batch->ice->workaround_offset, 0);
}

/**
 * Flush and invalidate all caches (for debugging purposes).
 */
void
crocus_flush_all_caches(struct crocus_batch *batch)
{
   crocus_emit_pipe_control_flush(batch, "debug: flush all caches",
                                  PIPE_CONTROL_CS_STALL |
                                  PIPE_CONTROL_DATA_CACHE_FLUSH |
                                  PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                  PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                  PIPE_CONTROL_VF_CACHE_INVALIDATE |
                                  PIPE_CONTROL_INSTRUCTION_INVALIDATE |
                                  PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
                                  PIPE_CONTROL_CONST_CACHE_INVALIDATE |
                                  PIPE_CONTROL_STATE_CACHE_INVALIDATE);
}

static void
crocus_texture_barrier(struct pipe_context *ctx, unsigned flags)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_batch *render_batch = &ice->batches[CROCUS_BATCH_RENDER];
   struct crocus_batch *compute_batch = &ice->batches[CROCUS_BATCH_COMPUTE];
   const struct intel_device_info *devinfo = &render_batch->screen->devinfo;

   if (devinfo->ver < 6) {
      crocus_emit_mi_flush(render_batch);
      return;
   }

   if (render_batch->contains_draw) {
      crocus_batch_maybe_flush(render_batch, 48);
      crocus_emit_pipe_control_flush(render_batch,
                                     "API: texture barrier (1/2)",
                                     (flags == 1 ? PIPE_CONTROL_DEPTH_CACHE_FLUSH  : 0) |
                                     PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                     PIPE_CONTROL_CS_STALL);
      crocus_emit_pipe_control_flush(render_batch,
                                     "API: texture barrier (2/2)",
                                     PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE);
   }

   if (compute_batch->contains_draw) {
      crocus_batch_maybe_flush(compute_batch, 48);
      crocus_emit_pipe_control_flush(compute_batch,
                                     "API: texture barrier (1/2)",
                                     PIPE_CONTROL_CS_STALL);
      crocus_emit_pipe_control_flush(compute_batch,
                                     "API: texture barrier (2/2)",
                                     PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE);
   }
}

static void
crocus_memory_barrier(struct pipe_context *ctx, unsigned flags)
{
   struct crocus_context *ice = (void *) ctx;
   unsigned bits = PIPE_CONTROL_DATA_CACHE_FLUSH | PIPE_CONTROL_CS_STALL;
   const struct intel_device_info *devinfo = &ice->batches[0].screen->devinfo;

   assert(devinfo->ver >= 7);

   if (flags & (PIPE_BARRIER_VERTEX_BUFFER |
                PIPE_BARRIER_INDEX_BUFFER |
                PIPE_BARRIER_INDIRECT_BUFFER)) {
      bits |= PIPE_CONTROL_VF_CACHE_INVALIDATE;
   }

   if (flags & PIPE_BARRIER_CONSTANT_BUFFER) {
      bits |= PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
              PIPE_CONTROL_CONST_CACHE_INVALIDATE;
   }

   if (flags & (PIPE_BARRIER_TEXTURE | PIPE_BARRIER_FRAMEBUFFER)) {
      bits |= PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
              PIPE_CONTROL_RENDER_TARGET_FLUSH;
   }

   /* Typed surface messages are handled by the render cache on IVB, so we
    * need to flush it too.
    */
   if (devinfo->verx10 < 75)
      bits |= PIPE_CONTROL_RENDER_TARGET_FLUSH;

   for (int i = 0; i < ice->batch_count; i++) {
      if (ice->batches[i].contains_draw) {
         crocus_batch_maybe_flush(&ice->batches[i], 24);
         crocus_emit_pipe_control_flush(&ice->batches[i], "API: memory barrier",
                                        bits);
      }
   }
}

void
crocus_init_flush_functions(struct pipe_context *ctx)
{
   ctx->memory_barrier = crocus_memory_barrier;
   ctx->texture_barrier = crocus_texture_barrier;
}
