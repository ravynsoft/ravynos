/*
 * Copyright © 2021 Google, Inc.
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

#ifdef X
#undef X
#endif

#if PTRSZ == 32
#define X(n) n##_32
#else
#define X(n) n##_64
#endif

static void X(emit_reloc_common)(struct fd_ringbuffer *ring, uint64_t iova)
{
#if PTRSZ == 64
   uint64_t *p64 = (uint64_t *)ring->cur;
   *p64 = iova;
   ring->cur += 2;
#else
   (*ring->cur++) = (uint32_t)iova;
#endif
}

static void X(fd_ringbuffer_sp_emit_reloc_nonobj)(struct fd_ringbuffer *ring,
                                                  const struct fd_reloc *reloc)
{
   X(emit_reloc_common)(ring, reloc->iova);
   fd_ringbuffer_sp_emit_bo_nonobj(ring, reloc->bo);
}

static void X(fd_ringbuffer_sp_emit_reloc_obj)(struct fd_ringbuffer *ring,
                                               const struct fd_reloc *reloc)
{
   X(emit_reloc_common)(ring, reloc->iova);
   fd_ringbuffer_sp_emit_bo_obj(ring, reloc->bo);
}

static uint32_t X(fd_ringbuffer_sp_emit_reloc_ring)(
   struct fd_ringbuffer *ring, struct fd_ringbuffer *target, uint32_t cmd_idx)
{
   struct fd_ringbuffer_sp *fd_target = to_fd_ringbuffer_sp(target);
   struct fd_bo *bo;
   uint32_t size;

   if ((target->flags & FD_RINGBUFFER_GROWABLE) &&
       (cmd_idx < fd_target->u.nr_cmds)) {
      bo = fd_target->u.cmds[cmd_idx].ring_bo;
      size = fd_target->u.cmds[cmd_idx].size;
   } else {
      bo = fd_target->ring_bo;
      size = offset_bytes(target->cur, target->start);
   }

   if (ring->flags & _FD_RINGBUFFER_OBJECT) {
      X(fd_ringbuffer_sp_emit_reloc_obj)(ring, &(struct fd_reloc){
                .bo = bo,
                .iova = bo->iova + fd_target->offset,
                .offset = fd_target->offset,
             });
   } else {
      X(fd_ringbuffer_sp_emit_reloc_nonobj)(ring, &(struct fd_reloc){
                .bo = bo,
                .iova = bo->iova + fd_target->offset,
                .offset = fd_target->offset,
             });
   }

   if (!(target->flags & _FD_RINGBUFFER_OBJECT))
      return size;

   struct fd_ringbuffer_sp *fd_ring = to_fd_ringbuffer_sp(ring);

   if (ring->flags & _FD_RINGBUFFER_OBJECT) {
      for (unsigned i = 0; i < fd_target->u.nr_reloc_bos; i++) {
         struct fd_bo *target_bo = fd_target->u.reloc_bos[i];
         if (!fd_ringbuffer_references_bo(ring, target_bo))
            APPEND(&fd_ring->u, reloc_bos, fd_bo_ref(target_bo));
      }
   } else {
      struct fd_submit_sp *fd_submit = to_fd_submit_sp(fd_ring->u.submit);

      if (fd_submit->seqno != fd_target->u.last_submit_seqno) {
         for (unsigned i = 0; i < fd_target->u.nr_reloc_bos; i++) {
            fd_submit_append_bo(fd_submit, fd_target->u.reloc_bos[i]);
         }
         fd_target->u.last_submit_seqno = fd_submit->seqno;
      }

#ifndef NDEBUG
      /* Dealing with assert'd BOs is deferred until the submit is known,
       * since the batch resource tracking attaches BOs directly to
       * the submit instead of the long lived stateobj
       */
      for (unsigned i = 0; i < fd_target->u.nr_assert_bos; i++) {
         fd_ringbuffer_sp_assert_attached_nonobj(ring, fd_target->u.assert_bos[i]);
      }
#endif
   }

   return size;
}
