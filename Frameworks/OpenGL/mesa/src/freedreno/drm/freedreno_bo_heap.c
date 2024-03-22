/*
 * Copyright © 2022 Google, Inc.
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

#include "freedreno_drmif.h"
#include "freedreno_priv.h"

struct sa_bo {
   struct fd_bo base;
   struct fd_bo_heap *heap;
   unsigned offset;
};
FD_DEFINE_CAST(fd_bo, sa_bo);

#define HEAP_DEBUG 0

static void heap_clean(struct fd_bo_heap *heap, bool idle);
static void heap_dump(struct fd_bo_heap *heap);

struct fd_bo_heap *
fd_bo_heap_new(struct fd_device *dev, uint32_t flags)
{
   struct fd_bo_heap *heap;

   /* We cannot suballocate shared buffers! Implicit sync is not supported! */
   assert(!(flags & FD_BO_SHARED));

   /* No internal buffers either, we need userspace fencing: */
   assert(!(flags & _FD_BO_NOSYNC));

   heap = calloc(1, sizeof(*heap));

   heap->dev = dev;
   heap->flags = flags;
   simple_mtx_init(&heap->lock, mtx_plain);
   list_inithead(&heap->freelist);

   /* Note that util_vma_heap_init doesn't like offset==0, so we shift the
    * entire range by one block size (see block_idx()):
    */
   util_vma_heap_init(&heap->heap, FD_BO_HEAP_BLOCK_SIZE,
                      FD_BO_HEAP_BLOCK_SIZE * ARRAY_SIZE(heap->blocks));
   heap->heap.alloc_high = false;
   heap->heap.nospan_shift = ffs(FD_BO_HEAP_BLOCK_SIZE) - 1;

   heap_dump(heap);

   return heap;
}

void fd_bo_heap_destroy(struct fd_bo_heap *heap)
{
   /* drain the freelist: */
   heap_clean(heap, false);

   util_vma_heap_finish(&heap->heap);
   for (unsigned i = 0; i < ARRAY_SIZE(heap->blocks); i++)
      if (heap->blocks[i])
         fd_bo_del(heap->blocks[i]);
   free(heap);
}

static bool
sa_idle(struct fd_bo *bo)
{
   enum fd_bo_state state = fd_bo_state(bo);
   assert(state != FD_BO_STATE_UNKNOWN);
   return state == FD_BO_STATE_IDLE;
}

/**
 * The backing block is determined by the offset within the heap, since all
 * the blocks are equal size
 */
static unsigned
block_idx(struct sa_bo *s)
{
   /* The vma allocator doesn't like offset=0 so the range is shifted up
    * by one block size:
    */
   return (s->offset / FD_BO_HEAP_BLOCK_SIZE) - 1;
}

static unsigned
block_offset(struct sa_bo *s)
{
   return s->offset % FD_BO_HEAP_BLOCK_SIZE;
}

static void
heap_dump(struct fd_bo_heap *heap)
{
   if (!HEAP_DEBUG)
      return;
   fprintf(stderr, "HEAP[%x]: freelist: %u\n", heap->flags, list_length(&heap->freelist));
   util_vma_heap_print(&heap->heap, stderr, "",
                       FD_BO_HEAP_BLOCK_SIZE * ARRAY_SIZE(heap->blocks));
}

static void
sa_release(struct fd_bo *bo)
{
   struct sa_bo *s = to_sa_bo(bo);

   simple_mtx_assert_locked(&s->heap->lock);

   /*
    * We don't track heap allocs in valgrind
    * VG_BO_FREE(bo);
    */

   fd_bo_fini_fences(bo);

   if (HEAP_DEBUG)
      mesa_logi("release: %08x-%x idx=%d", s->offset, bo->size, block_idx(s));

   util_vma_heap_free(&s->heap->heap, s->offset, bo->size);

   /* Drop our reference to the backing block object: */
   fd_bo_del(s->heap->blocks[block_idx(s)]);

   list_del(&bo->node);

   if ((++s->heap->cnt % 256) == 0)
      heap_dump(s->heap);

   free(bo);
}

static int
sa_madvise(struct fd_bo *bo, int willneed)
{
   return willneed;
}

static uint64_t
sa_iova(struct fd_bo *bo)
{
   struct sa_bo *s = to_sa_bo(bo);

   return s->heap->blocks[block_idx(s)]->iova + block_offset(s);
}

static void
sa_set_name(struct fd_bo *bo, const char *fmt, va_list ap)
{
   /* No-op, kernel has a single name for the entire buffer we suballoc from */
}

static void
sa_destroy(struct fd_bo *bo)
{
   struct fd_bo_heap *heap = to_sa_bo(bo)->heap;

   simple_mtx_lock(&heap->lock);
   list_addtail(&bo->node, &heap->freelist);
   simple_mtx_unlock(&heap->lock);
}

static struct fd_bo_funcs heap_bo_funcs = {
      .madvise = sa_madvise,
      .iova = sa_iova,
      .map = fd_bo_map_os_mmap,
      .set_name = sa_set_name,
      .destroy = sa_destroy,
};

/**
 * Get the backing heap block of a suballocated bo
 */
struct fd_bo *
fd_bo_heap_block(struct fd_bo *bo)
{
   assert(suballoc_bo(bo));

   struct sa_bo *s = to_sa_bo(bo);
   return s->heap->blocks[block_idx(s)];
}

static void
heap_clean(struct fd_bo_heap *heap, bool idle)
{
   simple_mtx_lock(&heap->lock);
   foreach_bo_safe (bo, &heap->freelist) {
      /* It might be nice if we could keep freelist sorted by fence # */
      if (idle && !sa_idle(bo))
         break;
      sa_release(bo);
   }
   simple_mtx_unlock(&heap->lock);
}

struct fd_bo *
fd_bo_heap_alloc(struct fd_bo_heap *heap, uint32_t size)
{
   heap_clean(heap, true);

   /* util_vma does not like zero byte allocations, which we get, for
    * ex, with the initial query buffer allocation on pre-a5xx:
    */
   size = MAX2(size, SUBALLOC_ALIGNMENT);

   size = ALIGN(size, SUBALLOC_ALIGNMENT);

   simple_mtx_lock(&heap->lock);
   /* Allocate larger buffers from the bottom, and smaller buffers from top
    * to help limit fragmentation:
    *
    * (The 8k threshold is just a random guess, but seems to work ok)
    */
   heap->heap.alloc_high = (size <= 8 * 1024);
   uint64_t offset = util_vma_heap_alloc(&heap->heap, size, SUBALLOC_ALIGNMENT);
   if (!offset) {
      simple_mtx_unlock(&heap->lock);
      return NULL;
   }

   struct sa_bo *s = calloc(1, sizeof(*s));

   s->heap = heap;
   s->offset = offset;

   assert((s->offset / FD_BO_HEAP_BLOCK_SIZE) == (s->offset + size - 1) / FD_BO_HEAP_BLOCK_SIZE);
   unsigned idx = block_idx(s);
   if (HEAP_DEBUG)
      mesa_logi("alloc: %08x-%x idx=%d", s->offset, size, idx);
   if (!heap->blocks[idx]) {
      heap->blocks[idx] = fd_bo_new(
            heap->dev, FD_BO_HEAP_BLOCK_SIZE, heap->flags,
            "heap-%x-block-%u", heap->flags, idx);
      if (heap->flags == RING_FLAGS)
         fd_bo_mark_for_dump(heap->blocks[idx]);
   }
   /* Take a reference to the backing obj: */
   fd_bo_ref(heap->blocks[idx]);
   simple_mtx_unlock(&heap->lock);

   struct fd_bo *bo = &s->base;

   bo->size = size;
   bo->funcs = &heap_bo_funcs;
   bo->handle = 1; /* dummy handle to make fd_bo_init_common() happy */
   bo->alloc_flags = heap->flags;

   /* Pre-initialize mmap ptr, to avoid trying to os_mmap() */
   bo->map = ((uint8_t *)fd_bo_map(heap->blocks[idx])) + block_offset(s);

   fd_bo_init_common(bo, heap->dev);

   bo->handle = FD_BO_SUBALLOC_HANDLE;

   return bo;
}
