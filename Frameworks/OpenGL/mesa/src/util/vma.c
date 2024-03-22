/*
 * Copyright © 2018 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <inttypes.h>

#include "util/macros.h"
#include "util/u_math.h"
#include "util/vma.h"

struct util_vma_hole {
   struct list_head link;
   uint64_t offset;
   uint64_t size;
};

#define util_vma_foreach_hole(_hole, _heap) \
   list_for_each_entry(struct util_vma_hole, _hole, &(_heap)->holes, link)

#define util_vma_foreach_hole_safe(_hole, _heap) \
   list_for_each_entry_safe(struct util_vma_hole, _hole, &(_heap)->holes, link)

#define util_vma_foreach_hole_safe_rev(_hole, _heap) \
   list_for_each_entry_safe_rev(struct util_vma_hole, _hole, &(_heap)->holes, link)

void
util_vma_heap_init(struct util_vma_heap *heap,
                   uint64_t start, uint64_t size)
{
   list_inithead(&heap->holes);
   heap->free_size = 0;
   if (size > 0)
      util_vma_heap_free(heap, start, size);

   /* Default to using high addresses */
   heap->alloc_high = true;

   /* Default to not having a nospan alignment */
   heap->nospan_shift = 0;
}

void
util_vma_heap_finish(struct util_vma_heap *heap)
{
   util_vma_foreach_hole_safe(hole, heap)
      free(hole);
}

#ifndef NDEBUG
static void
util_vma_heap_validate(struct util_vma_heap *heap)
{
   uint64_t free_size = 0;
   uint64_t prev_offset = 0;
   util_vma_foreach_hole(hole, heap) {
      assert(hole->offset > 0);
      assert(hole->size > 0);

      free_size += hole->size;

      if (&hole->link == heap->holes.next) {
         /* This must be the top-most hole.  Assert that, if it overflows, it
          * overflows to 0, i.e. 2^64.
          */
         assert(hole->size + hole->offset == 0 ||
                hole->size + hole->offset > hole->offset);
      } else {
         /* This is not the top-most hole so it must not overflow and, in
          * fact, must be strictly lower than the top-most hole.  If
          * hole->size + hole->offset == prev_offset, then we failed to join
          * holes during a util_vma_heap_free.
          */
         assert(hole->size + hole->offset > hole->offset &&
                hole->size + hole->offset < prev_offset);
      }
      prev_offset = hole->offset;
   }

   assert(free_size == heap->free_size);
}
#else
#define util_vma_heap_validate(heap)
#endif

static void
util_vma_hole_alloc(struct util_vma_heap *heap,
                    struct util_vma_hole *hole,
                    uint64_t offset, uint64_t size)
{
   assert(hole->offset <= offset);
   assert(hole->size >= offset - hole->offset + size);

   if (offset == hole->offset && size == hole->size) {
      /* Just get rid of the hole. */
      list_del(&hole->link);
      free(hole);
      goto done;
   }

   assert(offset - hole->offset <= hole->size - size);
   uint64_t waste = (hole->size - size) - (offset - hole->offset);
   if (waste == 0) {
      /* We allocated at the top.  Shrink the hole down. */
      hole->size -= size;
      goto done;
   }

   if (offset == hole->offset) {
      /* We allocated at the bottom. Shrink the hole up. */
      hole->offset += size;
      hole->size -= size;
      goto done;
   }

   /* We allocated in the middle.  We need to split the old hole into two
    * holes, one high and one low.
    */
   struct util_vma_hole *high_hole = calloc(1, sizeof(*hole));
   high_hole->offset = offset + size;
   high_hole->size = waste;

   /* Adjust the hole to be the amount of space left at he bottom of the
    * original hole.
    */
   hole->size = offset - hole->offset;

   /* Place the new hole before the old hole so that the list is in order
    * from high to low.
    */
   list_addtail(&high_hole->link, &hole->link);

 done:
   heap->free_size -= size;
}

uint64_t
util_vma_heap_alloc(struct util_vma_heap *heap,
                    uint64_t size, uint64_t alignment)
{
   /* The caller is expected to reject zero-size allocations */
   assert(size > 0);
   assert(alignment > 0);

   util_vma_heap_validate(heap);

   /* The requested alignment should not be stronger than the block/nospan
    * alignment.
    */
   if (heap->nospan_shift) {
      assert(ALIGN(BITFIELD64_BIT(heap->nospan_shift), alignment) ==
            BITFIELD64_BIT(heap->nospan_shift));
   }

   if (heap->alloc_high) {
      util_vma_foreach_hole_safe(hole, heap) {
         if (size > hole->size)
            continue;

         /* Compute the offset as the highest address where a chunk of the
          * given size can be without going over the top of the hole.
          *
          * This calculation is known to not overflow because we know that
          * hole->size + hole->offset can only overflow to 0 and size > 0.
          */
         uint64_t offset = (hole->size - size) + hole->offset;

         if (heap->nospan_shift) {
            uint64_t end = offset + size - 1;
            if ((end >> heap->nospan_shift) != (offset >> heap->nospan_shift)) {
               /* can we shift the offset down and still fit in the current hole? */
               end &= ~BITFIELD64_MASK(heap->nospan_shift);
               assert(end >= size);
               offset -= size;
               if (offset < hole->offset)
                  continue;
            }
         }

         /* Align the offset.  We align down and not up because we are
          * allocating from the top of the hole and not the bottom.
          */
         offset = (offset / alignment) * alignment;

         if (offset < hole->offset)
            continue;

         util_vma_hole_alloc(heap, hole, offset, size);
         util_vma_heap_validate(heap);
         return offset;
      }
   } else {
      util_vma_foreach_hole_safe_rev(hole, heap) {
         if (size > hole->size)
            continue;

         uint64_t offset = hole->offset;

         /* Align the offset */
         uint64_t misalign = offset % alignment;
         if (misalign) {
            uint64_t pad = alignment - misalign;
            if (pad > hole->size - size)
               continue;

            offset += pad;
         }

         if (heap->nospan_shift) {
            uint64_t end = offset + size - 1;
            if ((end >> heap->nospan_shift) != (offset >> heap->nospan_shift)) {
               /* can we shift the offset up and still fit in the current hole? */
               offset = end & ~BITFIELD64_MASK(heap->nospan_shift);
               if ((offset + size) > (hole->offset + hole->size))
                  continue;
            }
         }

         util_vma_hole_alloc(heap, hole, offset, size);
         util_vma_heap_validate(heap);
         return offset;
      }
   }

   /* Failed to allocate */
   return 0;
}

bool
util_vma_heap_alloc_addr(struct util_vma_heap *heap,
                         uint64_t offset, uint64_t size)
{
   /* An offset of 0 is reserved for allocation failure.  It is not a valid
    * address and cannot be allocated.
    */
   assert(offset > 0);

   /* Allocating something with a size of 0 is also not valid. */
   assert(size > 0);

   /* It's possible for offset + size to wrap around if we touch the top of
    * the 64-bit address space, but we cannot go any higher than 2^64.
    */
   assert(offset + size == 0 || offset + size > offset);

   /* Find the hole if one exists. */
   util_vma_foreach_hole_safe(hole, heap) {
      if (hole->offset > offset)
         continue;

      /* Holes are ordered high-to-low so the first hole we find with
       * hole->offset <= is our hole.  If it's not big enough to contain the
       * requested range, then the allocation fails.
       */
      assert(hole->offset <= offset);
      if (hole->size < offset - hole->offset + size)
         return false;

      util_vma_hole_alloc(heap, hole, offset, size);
      return true;
   }

   /* We didn't find a suitable hole */
   return false;
}

void
util_vma_heap_free(struct util_vma_heap *heap,
                   uint64_t offset, uint64_t size)
{
   /* An offset of 0 is reserved for allocation failure.  It is not a valid
    * address and cannot be freed.
    */
   assert(offset > 0);

   /* Freeing something with a size of 0 is also not valid. */
   assert(size > 0);

   /* It's possible for offset + size to wrap around if we touch the top of
    * the 64-bit address space, but we cannot go any higher than 2^64.
    */
   assert(offset + size == 0 || offset + size > offset);

   util_vma_heap_validate(heap);

   /* Find immediately higher and lower holes if they exist. */
   struct util_vma_hole *high_hole = NULL, *low_hole = NULL;
   util_vma_foreach_hole(hole, heap) {
      if (hole->offset <= offset) {
         low_hole = hole;
         break;
      }
      high_hole = hole;
   }

   if (high_hole)
      assert(offset + size <= high_hole->offset);
   bool high_adjacent = high_hole && offset + size == high_hole->offset;

   if (low_hole) {
      assert(low_hole->offset + low_hole->size > low_hole->offset);
      assert(low_hole->offset + low_hole->size <= offset);
   }
   bool low_adjacent = low_hole && low_hole->offset + low_hole->size == offset;

   if (low_adjacent && high_adjacent) {
      /* Merge the two holes */
      low_hole->size += size + high_hole->size;
      list_del(&high_hole->link);
      free(high_hole);
   } else if (low_adjacent) {
      /* Merge into the low hole */
      low_hole->size += size;
   } else if (high_adjacent) {
      /* Merge into the high hole */
      high_hole->offset = offset;
      high_hole->size += size;
   } else {
      /* Neither hole is adjacent; make a new one */
      struct util_vma_hole *hole = calloc(1, sizeof(*hole));

      hole->offset = offset;
      hole->size = size;

      /* Add it after the high hole so we maintain high-to-low ordering */
      if (high_hole)
         list_add(&hole->link, &high_hole->link);
      else
         list_add(&hole->link, &heap->holes);
   }

   heap->free_size += size;
   util_vma_heap_validate(heap);
}

void
util_vma_heap_print(struct util_vma_heap *heap, FILE *fp,
                    const char *tab, uint64_t total_size)
{
   fprintf(fp, "%sutil_vma_heap:\n", tab);

   uint64_t total_free = 0;
   util_vma_foreach_hole(hole, heap) {
      fprintf(fp, "%s    hole: offset = %"PRIu64" (0x%"PRIx64"), "
              "size = %"PRIu64" (0x%"PRIx64")\n",
              tab, hole->offset, hole->offset, hole->size, hole->size);
      total_free += hole->size;
   }
   assert(total_free <= total_size);
   assert(total_free == heap->free_size);
   fprintf(fp, "%s%"PRIu64"B (0x%"PRIx64") free (%.2f%% full)\n",
           tab, total_free, total_free,
           ((double)(total_size - total_free) / (double)total_size) * 100);
}
