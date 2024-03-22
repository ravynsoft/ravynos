/*
 * Copyright (C) 2012-2018 Rob Clark <robclark@freedesktop.org>
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

#ifndef FREEDRENO_RINGBUFFER_H_
#define FREEDRENO_RINGBUFFER_H_

#include <stdio.h>
#include "util/u_atomic.h"
#include "util/u_debug.h"

#include "adreno_common.xml.h"
#include "adreno_pm4.xml.h"
#include "freedreno_drmif.h"
#include "freedreno_pm4.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fd_submit;
struct fd_ringbuffer;

enum fd_ringbuffer_flags {

   /* Primary ringbuffer for a submit, ie. an IB1 level rb
    * which kernel must setup RB->IB1 CP_INDIRECT_BRANCH
    * packets.
    */
   FD_RINGBUFFER_PRIMARY = 0x1,

   /* Hint that the stateobj will be used for streaming state
    * that is used once or a few times and then discarded.
    *
    * For sub-allocation, non streaming stateobj's should be
    * sub-allocated from a page size buffer, so one long lived
    * state obj doesn't prevent other pages from being freed.
    * (Ie. it would be no worse than allocating a page sized
    * bo for each small non-streaming stateobj).
    *
    * But streaming stateobj's could be sub-allocated from a
    * larger buffer to reduce the alloc/del overhead.
    */
   FD_RINGBUFFER_STREAMING = 0x2,

   /* Indicates that "growable" cmdstream can be used,
    * consisting of multiple physical cmdstream buffers
    */
   FD_RINGBUFFER_GROWABLE = 0x4,

   /* Internal use only: */
   _FD_RINGBUFFER_OBJECT = 0x8,
};

/* A submit object manages/tracks all the state buildup for a "submit"
 * ioctl to the kernel.  Additionally, with the exception of long-lived
 * non-STREAMING stateobj rb's, rb's are allocated from the submit.
 */
struct fd_submit *fd_submit_new(struct fd_pipe *pipe);

/* NOTE: all ringbuffer's create from the submit should be unref'd
 * before destroying the submit.
 */
void fd_submit_del(struct fd_submit *submit);

struct fd_submit * fd_submit_ref(struct fd_submit *submit);

/* Allocate a new rb from the submit. */
struct fd_ringbuffer *fd_submit_new_ringbuffer(struct fd_submit *submit,
                                               uint32_t size,
                                               enum fd_ringbuffer_flags flags);

/* in_fence_fd: -1 for no in-fence, else fence fd
 * if use_fence_fd is true the output fence will be dma_fence fd backed
 */
struct fd_fence *fd_submit_flush(struct fd_submit *submit, int in_fence_fd,
                                 bool use_fence_fd);

struct fd_ringbuffer;
struct fd_reloc;

struct fd_ringbuffer_funcs {
   void (*grow)(struct fd_ringbuffer *ring, uint32_t size);

   /**
    * Alternative to emit_reloc for the softpin case, where we only need
    * to track that the bo is used (and not track all the extra info that
    * the kernel would need to do a legacy reloc.
    */
   void (*emit_bo)(struct fd_ringbuffer *ring, struct fd_bo *bo);
   void (*assert_attached)(struct fd_ringbuffer *ring, struct fd_bo *bo);

   void (*emit_reloc)(struct fd_ringbuffer *ring, const struct fd_reloc *reloc);
   uint32_t (*emit_reloc_ring)(struct fd_ringbuffer *ring,
                               struct fd_ringbuffer *target, uint32_t cmd_idx);
   uint32_t (*cmd_count)(struct fd_ringbuffer *ring);
   bool (*check_size)(struct fd_ringbuffer *ring);
   void (*destroy)(struct fd_ringbuffer *ring);
};

/* the ringbuffer object is not opaque so that OUT_RING() type stuff
 * can be inlined.  Note that users should not make assumptions about
 * the size of this struct.
 */
struct fd_ringbuffer {
   uint32_t *cur, *end, *start;
   const struct fd_ringbuffer_funcs *funcs;

   // size or end coudl probably go away
   int size;
   int32_t refcnt;
   enum fd_ringbuffer_flags flags;
};

/* Allocate a new long-lived state object, not associated with
 * a submit:
 */
struct fd_ringbuffer *fd_ringbuffer_new_object(struct fd_pipe *pipe,
                                               uint32_t size);

/*
 * Helpers for ref/unref with some extra debugging.. unref() returns true if
 * the object is still live
 */

static inline void
ref(int32_t *ref)
{
   ASSERTED int32_t count = p_atomic_inc_return(ref);
   /* We should never see a refcnt transition 0->1, this is a sign of a
    * zombie coming back from the dead!
    */
   assert(count != 1);
}

static inline bool
unref(int32_t *ref)
{
   int32_t count = p_atomic_dec_return(ref);
   assert(count != -1);
   return count == 0;
}

static inline void
fd_ringbuffer_del(struct fd_ringbuffer *ring)
{
   if (--ring->refcnt > 0)
      return;

   ring->funcs->destroy(ring);
}

static inline struct fd_ringbuffer *
fd_ringbuffer_ref(struct fd_ringbuffer *ring)
{
   ring->refcnt++;
   return ring;
}

static inline void
fd_ringbuffer_grow(struct fd_ringbuffer *ring, uint32_t ndwords)
{
   assert(ring->funcs->grow); /* unsupported on kgsl */

   ring->funcs->grow(ring, ring->size);
}

static inline bool
fd_ringbuffer_check_size(struct fd_ringbuffer *ring)
{
   return ring->funcs->check_size(ring);
}

static inline void
fd_ringbuffer_emit(struct fd_ringbuffer *ring, uint32_t data)
{
   (*ring->cur++) = data;
}

struct fd_reloc {
   struct fd_bo *bo;
   uint64_t iova;
   uint64_t orval;
#define FD_RELOC_READ  0x0001
#define FD_RELOC_WRITE 0x0002
#define FD_RELOC_DUMP  0x0004
   uint32_t offset;
   int32_t shift;
};

/* We always mark BOs for write, instead of tracking it across reloc
 * sources in userspace.  On the kernel side, this means we track a single
 * excl fence in the BO instead of a set of read fences, which is cheaper.
 * The downside is that a dmabuf-shared device won't be able to read in
 * parallel with a read-only access by freedreno, but most other drivers
 * have decided that that usecase isn't important enough to do this
 * tracking, as well.
 */
#define FD_RELOC_FLAGS_INIT (FD_RELOC_READ | FD_RELOC_WRITE)

/* NOTE: relocs are 2 dwords on a5xx+ */

static inline void
fd_ringbuffer_attach_bo(struct fd_ringbuffer *ring, struct fd_bo *bo)
{
   ring->funcs->emit_bo(ring, bo);
}

static inline void
fd_ringbuffer_assert_attached(struct fd_ringbuffer *ring, struct fd_bo *bo)
{
#ifndef NDEBUG
   ring->funcs->assert_attached(ring, bo);
#endif
}

static inline void
fd_ringbuffer_reloc(struct fd_ringbuffer *ring, const struct fd_reloc *reloc)
{
   ring->funcs->emit_reloc(ring, reloc);
}

static inline uint32_t
fd_ringbuffer_cmd_count(struct fd_ringbuffer *ring)
{
   if (!ring->funcs->cmd_count)
      return 1;
   return ring->funcs->cmd_count(ring);
}

static inline uint32_t
fd_ringbuffer_emit_reloc_ring_full(struct fd_ringbuffer *ring,
                                   struct fd_ringbuffer *target,
                                   uint32_t cmd_idx)
{
   return ring->funcs->emit_reloc_ring(ring, target, cmd_idx);
}

static inline uint32_t
offset_bytes(void *end, void *start)
{
   return ((char *)end) - ((char *)start);
}

static inline uint32_t
fd_ringbuffer_size(struct fd_ringbuffer *ring)
{
   /* only really needed for stateobj ringbuffers, and won't really
    * do what you expect for growable rb's.. so lets just restrict
    * this to stateobj's for now:
    */
   assert(!(ring->flags & FD_RINGBUFFER_GROWABLE));
   return offset_bytes(ring->cur, ring->start);
}

static inline bool
fd_ringbuffer_empty(struct fd_ringbuffer *ring)
{
   return (fd_ringbuffer_cmd_count(ring) == 1) &&
          (offset_bytes(ring->cur, ring->start) == 0);
}

#define LOG_DWORDS 0

static inline void
OUT_RING(struct fd_ringbuffer *ring, uint32_t data)
{
   if (LOG_DWORDS) {
      fprintf(stderr, "ring[%p]: OUT_RING   %04x:  %08x", ring,
              (uint32_t)(ring->cur - ring->start), data);
   }
   fd_ringbuffer_emit(ring, data);
}

static inline uint64_t
__reloc_iova(struct fd_bo *bo, uint32_t offset, uint64_t orval, int32_t shift)
{
   uint64_t iova = fd_bo_get_iova(bo) + offset;

   if (shift < 0)
      iova >>= -shift;
   else
      iova <<= shift;

   iova |= orval;

   return iova;
}

/*
 * NOTE: OUT_RELOC() is 2 dwords (64b) on a5xx+
 */
static inline void
OUT_RELOC(struct fd_ringbuffer *ring, struct fd_bo *bo, uint32_t offset,
          uint64_t orval, int32_t shift)
{
   if (LOG_DWORDS) {
      fprintf(stderr, "ring[%p]: OUT_RELOC   %04x:  %p+%u << %d", ring,
              (uint32_t)(ring->cur - ring->start), bo, offset, shift);
   }
   assert(offset < fd_bo_size(bo));

   uint64_t iova = __reloc_iova(bo, offset, orval, shift);

#if FD_BO_NO_HARDPIN
   uint64_t *cur = (uint64_t *)ring->cur;
   *cur = iova;
   ring->cur += 2;
   fd_ringbuffer_assert_attached(ring, bo);
#else
   struct fd_reloc reloc = {
         .bo = bo,
         .iova = iova,
         .orval = orval,
         .offset = offset,
         .shift = shift,
   };

   fd_ringbuffer_reloc(ring, &reloc);
#endif
}

static inline void
OUT_RB(struct fd_ringbuffer *ring, struct fd_ringbuffer *target)
{
   fd_ringbuffer_emit_reloc_ring_full(ring, target, 0);
}

static inline void
BEGIN_RING(struct fd_ringbuffer *ring, uint32_t ndwords)
{
   if (unlikely(ring->cur + ndwords > ring->end))
      fd_ringbuffer_grow(ring, ndwords);
}

static inline void
OUT_PKT0(struct fd_ringbuffer *ring, uint16_t regindx, uint16_t cnt)
{
   BEGIN_RING(ring, cnt + 1);
   OUT_RING(ring, pm4_pkt0_hdr(regindx, cnt));
}

static inline void
OUT_PKT2(struct fd_ringbuffer *ring)
{
   BEGIN_RING(ring, 1);
   OUT_RING(ring, CP_TYPE2_PKT);
}

static inline void
OUT_PKT3(struct fd_ringbuffer *ring, uint8_t opcode, uint16_t cnt)
{
   BEGIN_RING(ring, cnt + 1);
   OUT_RING(ring, CP_TYPE3_PKT | ((cnt - 1) << 16) | ((opcode & 0xFF) << 8));
}

/*
 * Starting with a5xx, pkt4/pkt7 are used instead of pkt0/pkt3
 */

static inline void
OUT_PKT4(struct fd_ringbuffer *ring, uint16_t regindx, uint16_t cnt)
{
   BEGIN_RING(ring, cnt + 1);
   OUT_RING(ring, pm4_pkt4_hdr((uint16_t)regindx, (uint16_t)cnt));
}

static inline void
OUT_PKT7(struct fd_ringbuffer *ring, uint32_t opcode, uint32_t cnt)
{
   BEGIN_RING(ring, cnt + 1);
   OUT_RING(ring, pm4_pkt7_hdr((uint8_t)opcode, (uint16_t)cnt));
}

static inline void
OUT_WFI(struct fd_ringbuffer *ring)
{
   OUT_PKT3(ring, CP_WAIT_FOR_IDLE, 1);
   OUT_RING(ring, 0x00000000);
}

static inline void
OUT_WFI5(struct fd_ringbuffer *ring)
{
   OUT_PKT7(ring, CP_WAIT_FOR_IDLE, 0);
}

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* FREEDRENO_RINGBUFFER_H_ */
