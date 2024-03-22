/*
 * Copyright © 2015 Intel Corporation
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
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <sys/mman.h>

#include "anv_private.h"

#include "util/anon_file.h"
#include "util/futex.h"

#ifdef HAVE_VALGRIND
#define VG_NOACCESS_READ(__ptr) ({                       \
   VALGRIND_MAKE_MEM_DEFINED((__ptr), sizeof(*(__ptr))); \
   __typeof(*(__ptr)) __val = *(__ptr);                  \
   VALGRIND_MAKE_MEM_NOACCESS((__ptr), sizeof(*(__ptr)));\
   __val;                                                \
})
#define VG_NOACCESS_WRITE(__ptr, __val) ({                  \
   VALGRIND_MAKE_MEM_UNDEFINED((__ptr), sizeof(*(__ptr)));  \
   *(__ptr) = (__val);                                      \
   VALGRIND_MAKE_MEM_NOACCESS((__ptr), sizeof(*(__ptr)));   \
})
#else
#define VG_NOACCESS_READ(__ptr) (*(__ptr))
#define VG_NOACCESS_WRITE(__ptr, __val) (*(__ptr) = (__val))
#endif

#ifndef MAP_POPULATE
#define MAP_POPULATE 0
#endif

/* Design goals:
 *
 *  - Lock free (except when resizing underlying bos)
 *
 *  - Constant time allocation with typically only one atomic
 *
 *  - Multiple allocation sizes without fragmentation
 *
 *  - Can grow while keeping addresses and offset of contents stable
 *
 *  - All allocations within one bo so we can point one of the
 *    STATE_BASE_ADDRESS pointers at it.
 *
 * The overall design is a two-level allocator: top level is a fixed size, big
 * block (8k) allocator, which operates out of a bo.  Allocation is done by
 * either pulling a block from the free list or growing the used range of the
 * bo.  Growing the range may run out of space in the bo which we then need to
 * grow.  Growing the bo is tricky in a multi-threaded, lockless environment:
 * we need to keep all pointers and contents in the old map valid.  GEM bos in
 * general can't grow, but we use a trick: we create a memfd and use ftruncate
 * to grow it as necessary.  We mmap the new size and then create a gem bo for
 * it using the new gem userptr ioctl.  Without heavy-handed locking around
 * our allocation fast-path, there isn't really a way to munmap the old mmap,
 * so we just keep it around until garbage collection time.  While the block
 * allocator is lockless for normal operations, we block other threads trying
 * to allocate while we're growing the map.  It shouldn't happen often, and
 * growing is fast anyway.
 *
 * At the next level we can use various sub-allocators.  The state pool is a
 * pool of smaller, fixed size objects, which operates much like the block
 * pool.  It uses a free list for freeing objects, but when it runs out of
 * space it just allocates a new block from the block pool.  This allocator is
 * intended for longer lived state objects such as SURFACE_STATE and most
 * other persistent state objects in the API.  We may need to track more info
 * with these object and a pointer back to the CPU object (eg VkImage).  In
 * those cases we just allocate a slightly bigger object and put the extra
 * state after the GPU state object.
 *
 * The state stream allocator works similar to how the i965 DRI driver streams
 * all its state.  Even with Vulkan, we need to emit transient state (whether
 * surface state base or dynamic state base), and for that we can just get a
 * block and fill it up.  These cases are local to a command buffer and the
 * sub-allocator need not be thread safe.  The streaming allocator gets a new
 * block when it runs out of space and chains them together so they can be
 * easily freed.
 */

/* Allocations are always at least 64 byte aligned, so 1 is an invalid value.
 * We use it to indicate the free list is empty. */
#define EMPTY UINT32_MAX

/* On FreeBSD PAGE_SIZE is already defined in
 * /usr/include/machine/param.h that is indirectly
 * included here.
 */
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

struct anv_mmap_cleanup {
   void *map;
   size_t size;
};

static inline uint32_t
ilog2_round_up(uint32_t value)
{
   assert(value != 0);
   return 32 - __builtin_clz(value - 1);
}

static inline uint32_t
round_to_power_of_two(uint32_t value)
{
   return 1 << ilog2_round_up(value);
}

struct anv_state_table_cleanup {
   void *map;
   size_t size;
};

#define ANV_STATE_TABLE_CLEANUP_INIT ((struct anv_state_table_cleanup){0})
#define ANV_STATE_ENTRY_SIZE (sizeof(struct anv_free_entry))

static VkResult
anv_state_table_expand_range(struct anv_state_table *table, uint32_t size);

VkResult
anv_state_table_init(struct anv_state_table *table,
                    struct anv_device *device,
                    uint32_t initial_entries)
{
   VkResult result;

   table->device = device;

   /* Just make it 2GB up-front.  The Linux kernel won't actually back it
    * with pages until we either map and fault on one of them or we use
    * userptr and send a chunk of it off to the GPU.
    */
   table->fd = os_create_anonymous_file(BLOCK_POOL_MEMFD_SIZE, "state table");
   if (table->fd == -1)
      return vk_error(device, VK_ERROR_INITIALIZATION_FAILED);

   if (!u_vector_init(&table->cleanups, 8,
                      sizeof(struct anv_state_table_cleanup))) {
      result = vk_error(device, VK_ERROR_INITIALIZATION_FAILED);
      goto fail_fd;
   }

   table->state.next = 0;
   table->state.end = 0;
   table->size = 0;

   uint32_t initial_size = initial_entries * ANV_STATE_ENTRY_SIZE;
   result = anv_state_table_expand_range(table, initial_size);
   if (result != VK_SUCCESS)
      goto fail_cleanups;

   return VK_SUCCESS;

 fail_cleanups:
   u_vector_finish(&table->cleanups);
 fail_fd:
   close(table->fd);

   return result;
}

static VkResult
anv_state_table_expand_range(struct anv_state_table *table, uint32_t size)
{
   void *map;
   struct anv_state_table_cleanup *cleanup;

   /* Assert that we only ever grow the pool */
   assert(size >= table->state.end);

   /* Make sure that we don't go outside the bounds of the memfd */
   if (size > BLOCK_POOL_MEMFD_SIZE)
      return vk_error(table->device, VK_ERROR_OUT_OF_HOST_MEMORY);

   cleanup = u_vector_add(&table->cleanups);
   if (!cleanup)
      return vk_error(table->device, VK_ERROR_OUT_OF_HOST_MEMORY);

   *cleanup = ANV_STATE_TABLE_CLEANUP_INIT;

   /* Just leak the old map until we destroy the pool.  We can't munmap it
    * without races or imposing locking on the block allocate fast path. On
    * the whole the leaked maps adds up to less than the size of the
    * current map.  MAP_POPULATE seems like the right thing to do, but we
    * should try to get some numbers.
    */
   map = mmap(NULL, size, PROT_READ | PROT_WRITE,
              MAP_SHARED | MAP_POPULATE, table->fd, 0);
   if (map == MAP_FAILED) {
      return vk_errorf(table->device, VK_ERROR_OUT_OF_HOST_MEMORY,
                       "mmap failed: %m");
   }

   cleanup->map = map;
   cleanup->size = size;

   table->map = map;
   table->size = size;

   return VK_SUCCESS;
}

static VkResult
anv_state_table_grow(struct anv_state_table *table)
{
   VkResult result = VK_SUCCESS;

   uint32_t used = align(table->state.next * ANV_STATE_ENTRY_SIZE, PAGE_SIZE);
   uint32_t old_size = table->size;

   /* The block pool is always initialized to a nonzero size and this function
    * is always called after initialization.
    */
   assert(old_size > 0);

   uint32_t required = MAX2(used, old_size);
   if (used * 2 <= required) {
      /* If we're in this case then this isn't the firsta allocation and we
       * already have enough space on both sides to hold double what we
       * have allocated.  There's nothing for us to do.
       */
      goto done;
   }

   uint32_t size = old_size * 2;
   while (size < required)
      size *= 2;

   assert(size > table->size);

   result = anv_state_table_expand_range(table, size);

 done:
   return result;
}

void
anv_state_table_finish(struct anv_state_table *table)
{
   struct anv_state_table_cleanup *cleanup;

   u_vector_foreach(cleanup, &table->cleanups) {
      if (cleanup->map)
         munmap(cleanup->map, cleanup->size);
   }

   u_vector_finish(&table->cleanups);

   close(table->fd);
}

VkResult
anv_state_table_add(struct anv_state_table *table, uint32_t *idx,
                    uint32_t count)
{
   struct anv_block_state state, old, new;
   VkResult result;

   assert(idx);

   while(1) {
      state.u64 = __sync_fetch_and_add(&table->state.u64, count);
      if (state.next + count <= state.end) {
         assert(table->map);
         struct anv_free_entry *entry = &table->map[state.next];
         for (int i = 0; i < count; i++) {
            entry[i].state.idx = state.next + i;
         }
         *idx = state.next;
         return VK_SUCCESS;
      } else if (state.next <= state.end) {
         /* We allocated the first block outside the pool so we have to grow
          * the pool.  pool_state->next acts a mutex: threads who try to
          * allocate now will get block indexes above the current limit and
          * hit futex_wait below.
          */
         new.next = state.next + count;
         do {
            result = anv_state_table_grow(table);
            if (result != VK_SUCCESS)
               return result;
            new.end = table->size / ANV_STATE_ENTRY_SIZE;
         } while (new.end < new.next);

         old.u64 = __sync_lock_test_and_set(&table->state.u64, new.u64);
         if (old.next != state.next)
            futex_wake(&table->state.end, INT_MAX);
      } else {
         futex_wait(&table->state.end, state.end, NULL);
         continue;
      }
   }
}

void
anv_free_list_push(union anv_free_list *list,
                   struct anv_state_table *table,
                   uint32_t first, uint32_t count)
{
   union anv_free_list current, old, new;
   uint32_t last = first;

   for (uint32_t i = 1; i < count; i++, last++)
      table->map[last].next = last + 1;

   old.u64 = list->u64;
   do {
      current = old;
      table->map[last].next = current.offset;
      new.offset = first;
      new.count = current.count + 1;
      old.u64 = __sync_val_compare_and_swap(&list->u64, current.u64, new.u64);
   } while (old.u64 != current.u64);
}

struct anv_state *
anv_free_list_pop(union anv_free_list *list,
                  struct anv_state_table *table)
{
   union anv_free_list current, new, old;

   current.u64 = list->u64;
   while (current.offset != EMPTY) {
      __sync_synchronize();
      new.offset = table->map[current.offset].next;
      new.count = current.count + 1;
      old.u64 = __sync_val_compare_and_swap(&list->u64, current.u64, new.u64);
      if (old.u64 == current.u64) {
         struct anv_free_entry *entry = &table->map[current.offset];
         return &entry->state;
      }
      current = old;
   }

   return NULL;
}

static VkResult
anv_block_pool_expand_range(struct anv_block_pool *pool,
                            uint32_t center_bo_offset, uint32_t size);

VkResult
anv_block_pool_init(struct anv_block_pool *pool,
                    struct anv_device *device,
                    const char *name,
                    uint64_t start_address,
                    uint32_t initial_size)
{
   VkResult result;

   pool->name = name;
   pool->device = device;
   pool->use_relocations = anv_use_relocations(device->physical);
   pool->nbos = 0;
   pool->size = 0;
   pool->center_bo_offset = 0;
   pool->start_address = intel_canonical_address(start_address);
   pool->map = NULL;

   if (!pool->use_relocations) {
      pool->bo = NULL;
      pool->fd = -1;
   } else {
      /* Just make it 2GB up-front.  The Linux kernel won't actually back it
       * with pages until we either map and fault on one of them or we use
       * userptr and send a chunk of it off to the GPU.
       */
      pool->fd = os_create_anonymous_file(BLOCK_POOL_MEMFD_SIZE, "block pool");
      if (pool->fd == -1)
         return vk_error(device, VK_ERROR_INITIALIZATION_FAILED);

      pool->wrapper_bo = (struct anv_bo) {
         .refcount = 1,
         .offset = -1,
         .is_wrapper = true,
      };
      pool->bo = &pool->wrapper_bo;
   }

   if (!u_vector_init(&pool->mmap_cleanups, 8,
                      sizeof(struct anv_mmap_cleanup))) {
      result = vk_error(device, VK_ERROR_INITIALIZATION_FAILED);
      goto fail_fd;
   }

   pool->state.next = 0;
   pool->state.end = 0;
   pool->back_state.next = 0;
   pool->back_state.end = 0;

   result = anv_block_pool_expand_range(pool, 0, initial_size);
   if (result != VK_SUCCESS)
      goto fail_mmap_cleanups;

   /* Make the entire pool available in the front of the pool.  If back
    * allocation needs to use this space, the "ends" will be re-arranged.
    */
   pool->state.end = pool->size;

   return VK_SUCCESS;

 fail_mmap_cleanups:
   u_vector_finish(&pool->mmap_cleanups);
 fail_fd:
   if (pool->fd >= 0)
      close(pool->fd);

   return result;
}

void
anv_block_pool_finish(struct anv_block_pool *pool)
{
   anv_block_pool_foreach_bo(bo, pool) {
      assert(bo->refcount == 1);
      anv_device_release_bo(pool->device, bo);
   }

   struct anv_mmap_cleanup *cleanup;
   u_vector_foreach(cleanup, &pool->mmap_cleanups)
      munmap(cleanup->map, cleanup->size);
   u_vector_finish(&pool->mmap_cleanups);

   if (pool->fd >= 0)
      close(pool->fd);
}

static VkResult
anv_block_pool_expand_range(struct anv_block_pool *pool,
                            uint32_t center_bo_offset, uint32_t size)
{
   /* Assert that we only ever grow the pool */
   assert(center_bo_offset >= pool->back_state.end);
   assert(size - center_bo_offset >= pool->state.end);

   /* Assert that we don't go outside the bounds of the memfd */
   assert(center_bo_offset <= BLOCK_POOL_MEMFD_CENTER);
   assert(!pool->use_relocations ||
          size - center_bo_offset <=
          BLOCK_POOL_MEMFD_SIZE - BLOCK_POOL_MEMFD_CENTER);

   /* For state pool BOs we have to be a bit careful about where we place them
    * in the GTT.  There are two documented workarounds for state base address
    * placement : Wa32bitGeneralStateOffset and Wa32bitInstructionBaseOffset
    * which state that those two base addresses do not support 48-bit
    * addresses and need to be placed in the bottom 32-bit range.
    * Unfortunately, this is not quite accurate.
    *
    * The real problem is that we always set the size of our state pools in
    * STATE_BASE_ADDRESS to 0xfffff (the maximum) even though the BO is most
    * likely significantly smaller.  We do this because we do not no at the
    * time we emit STATE_BASE_ADDRESS whether or not we will need to expand
    * the pool during command buffer building so we don't actually have a
    * valid final size.  If the address + size, as seen by STATE_BASE_ADDRESS
    * overflows 48 bits, the GPU appears to treat all accesses to the buffer
    * as being out of bounds and returns zero.  For dynamic state, this
    * usually just leads to rendering corruptions, but shaders that are all
    * zero hang the GPU immediately.
    *
    * The easiest solution to do is exactly what the bogus workarounds say to
    * do: restrict these buffers to 32-bit addresses.  We could also pin the
    * BO to some particular location of our choosing, but that's significantly
    * more work than just not setting a flag.  So, we explicitly DO NOT set
    * the EXEC_OBJECT_SUPPORTS_48B_ADDRESS flag and the kernel does all of the
    * hard work for us.  When using softpin, we're in control and the fixed
    * addresses we choose are fine for base addresses.
    */
   enum anv_bo_alloc_flags bo_alloc_flags = ANV_BO_ALLOC_CAPTURE;
   if (pool->use_relocations)
      bo_alloc_flags |= ANV_BO_ALLOC_32BIT_ADDRESS;

   if (!pool->use_relocations) {
      uint32_t new_bo_size = size - pool->size;
      struct anv_bo *new_bo;
      assert(center_bo_offset == 0);
      VkResult result = anv_device_alloc_bo(pool->device,
                                            pool->name,
                                            new_bo_size,
                                            bo_alloc_flags |
                                            ANV_BO_ALLOC_FIXED_ADDRESS |
                                            ANV_BO_ALLOC_MAPPED |
                                            ANV_BO_ALLOC_SNOOPED,
                                            pool->start_address + pool->size,
                                            &new_bo);
      if (result != VK_SUCCESS)
         return result;

      pool->bos[pool->nbos++] = new_bo;

      /* This pointer will always point to the first BO in the list */
      pool->bo = pool->bos[0];
   } else {
      /* Just leak the old map until we destroy the pool.  We can't munmap it
       * without races or imposing locking on the block allocate fast path. On
       * the whole the leaked maps adds up to less than the size of the
       * current map.  MAP_POPULATE seems like the right thing to do, but we
       * should try to get some numbers.
       */
      void *map = mmap(NULL, size, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_POPULATE, pool->fd,
                       BLOCK_POOL_MEMFD_CENTER - center_bo_offset);
      if (map == MAP_FAILED)
         return vk_errorf(pool->device, VK_ERROR_MEMORY_MAP_FAILED,
                          "mmap failed: %m");

      struct anv_bo *new_bo;
      VkResult result = anv_device_import_bo_from_host_ptr(pool->device,
                                                           map, size,
                                                           bo_alloc_flags,
                                                           0 /* client_address */,
                                                           &new_bo);
      if (result != VK_SUCCESS) {
         munmap(map, size);
         return result;
      }

      struct anv_mmap_cleanup *cleanup = u_vector_add(&pool->mmap_cleanups);
      if (!cleanup) {
         munmap(map, size);
         anv_device_release_bo(pool->device, new_bo);
         return vk_error(pool->device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }
      cleanup->map = map;
      cleanup->size = size;

      /* Now that we mapped the new memory, we can write the new
       * center_bo_offset back into pool and update pool->map. */
      pool->center_bo_offset = center_bo_offset;
      pool->map = map + center_bo_offset;

      pool->bos[pool->nbos++] = new_bo;
      pool->wrapper_bo.map = new_bo;
   }

   assert(pool->nbos < ANV_MAX_BLOCK_POOL_BOS);
   pool->size = size;

   return VK_SUCCESS;
}

/** Returns current memory map of the block pool.
 *
 * The returned pointer points to the map for the memory at the specified
 * offset. The offset parameter is relative to the "center" of the block pool
 * rather than the start of the block pool BO map.
 */
void*
anv_block_pool_map(struct anv_block_pool *pool, int32_t offset, uint32_t size)
{
   if (!pool->use_relocations) {
      struct anv_bo *bo = NULL;
      int32_t bo_offset = 0;
      anv_block_pool_foreach_bo(iter_bo, pool) {
         if (offset < bo_offset + iter_bo->size) {
            bo = iter_bo;
            break;
         }
         bo_offset += iter_bo->size;
      }
      assert(bo != NULL);
      assert(offset >= bo_offset);
      assert((offset - bo_offset) + size <= bo->size);

      return bo->map + (offset - bo_offset);
   } else {
      return pool->map + offset;
   }
}

/** Grows and re-centers the block pool.
 *
 * We grow the block pool in one or both directions in such a way that the
 * following conditions are met:
 *
 *  1) The size of the entire pool is always a power of two.
 *
 *  2) The pool only grows on both ends.  Neither end can get
 *     shortened.
 *
 *  3) At the end of the allocation, we have about twice as much space
 *     allocated for each end as we have used.  This way the pool doesn't
 *     grow too far in one direction or the other.
 *
 *  4) If the _alloc_back() has never been called, then the back portion of
 *     the pool retains a size of zero.  (This makes it easier for users of
 *     the block pool that only want a one-sided pool.)
 *
 *  5) We have enough space allocated for at least one more block in
 *     whichever side `state` points to.
 *
 *  6) The center of the pool is always aligned to both the block_size of
 *     the pool and a 4K CPU page.
 */
static uint32_t
anv_block_pool_grow(struct anv_block_pool *pool, struct anv_block_state *state,
                    uint32_t contiguous_size)
{
   VkResult result = VK_SUCCESS;

   pthread_mutex_lock(&pool->device->mutex);

   assert(state == &pool->state || state == &pool->back_state);

   /* Gather a little usage information on the pool.  Since we may have
    * threadsd waiting in queue to get some storage while we resize, it's
    * actually possible that total_used will be larger than old_size.  In
    * particular, block_pool_alloc() increments state->next prior to
    * calling block_pool_grow, so this ensures that we get enough space for
    * which ever side tries to grow the pool.
    *
    * We align to a page size because it makes it easier to do our
    * calculations later in such a way that we state page-aigned.
    */
   uint32_t back_used = align(pool->back_state.next, PAGE_SIZE);
   uint32_t front_used = align(pool->state.next, PAGE_SIZE);
   uint32_t total_used = front_used + back_used;

   assert(state == &pool->state || back_used > 0);

   uint32_t old_size = pool->size;

   /* The block pool is always initialized to a nonzero size and this function
    * is always called after initialization.
    */
   assert(old_size > 0);

   const uint32_t old_back = pool->center_bo_offset;
   const uint32_t old_front = old_size - pool->center_bo_offset;

   /* The back_used and front_used may actually be smaller than the actual
    * requirement because they are based on the next pointers which are
    * updated prior to calling this function.
    */
   uint32_t back_required = MAX2(back_used, old_back);
   uint32_t front_required = MAX2(front_used, old_front);

   if (!pool->use_relocations) {
      /* With softpin, the pool is made up of a bunch of buffers with separate
       * maps.  Make sure we have enough contiguous space that we can get a
       * properly contiguous map for the next chunk.
       */
      assert(old_back == 0);
      front_required = MAX2(front_required, old_front + contiguous_size);
   }

   if (back_used * 2 <= back_required && front_used * 2 <= front_required) {
      /* If we're in this case then this isn't the firsta allocation and we
       * already have enough space on both sides to hold double what we
       * have allocated.  There's nothing for us to do.
       */
      goto done;
   }

   uint32_t size = old_size * 2;
   while (size < back_required + front_required)
      size *= 2;

   assert(size > pool->size);

   /* We compute a new center_bo_offset such that, when we double the size
    * of the pool, we maintain the ratio of how much is used by each side.
    * This way things should remain more-or-less balanced.
    */
   uint32_t center_bo_offset;
   if (back_used == 0) {
      /* If we're in this case then we have never called alloc_back().  In
       * this case, we want keep the offset at 0 to make things as simple
       * as possible for users that don't care about back allocations.
       */
      center_bo_offset = 0;
   } else {
      /* Try to "center" the allocation based on how much is currently in
       * use on each side of the center line.
       */
      center_bo_offset = ((uint64_t)size * back_used) / total_used;

      /* Align down to a multiple of the page size */
      center_bo_offset &= ~(PAGE_SIZE - 1);

      assert(center_bo_offset >= back_used);

      /* Make sure we don't shrink the back end of the pool */
      if (center_bo_offset < back_required)
         center_bo_offset = back_required;

      /* Make sure that we don't shrink the front end of the pool */
      if (size - center_bo_offset < front_required)
         center_bo_offset = size - front_required;
   }

   assert(center_bo_offset % PAGE_SIZE == 0);

   result = anv_block_pool_expand_range(pool, center_bo_offset, size);

done:
   pthread_mutex_unlock(&pool->device->mutex);

   if (result == VK_SUCCESS) {
      /* Return the appropriate new size.  This function never actually
       * updates state->next.  Instead, we let the caller do that because it
       * needs to do so in order to maintain its concurrency model.
       */
      if (state == &pool->state) {
         return pool->size - pool->center_bo_offset;
      } else {
         assert(pool->center_bo_offset > 0);
         return pool->center_bo_offset;
      }
   } else {
      return 0;
   }
}

static uint32_t
anv_block_pool_alloc_new(struct anv_block_pool *pool,
                         struct anv_block_state *pool_state,
                         uint32_t block_size, uint32_t *padding)
{
   struct anv_block_state state, old, new;

   /* Most allocations won't generate any padding */
   if (padding)
      *padding = 0;

   while (1) {
      state.u64 = __sync_fetch_and_add(&pool_state->u64, block_size);
      if (state.next + block_size <= state.end) {
         return state.next;
      } else if (state.next <= state.end) {
         if (!pool->use_relocations && state.next < state.end) {
            /* We need to grow the block pool, but still have some leftover
             * space that can't be used by that particular allocation. So we
             * add that as a "padding", and return it.
             */
            uint32_t leftover = state.end - state.next;

            /* If there is some leftover space in the pool, the caller must
             * deal with it.
             */
            assert(leftover == 0 || padding);
            if (padding)
               *padding = leftover;
            state.next += leftover;
         }

         /* We allocated the first block outside the pool so we have to grow
          * the pool.  pool_state->next acts a mutex: threads who try to
          * allocate now will get block indexes above the current limit and
          * hit futex_wait below.
          */
         new.next = state.next + block_size;
         do {
            new.end = anv_block_pool_grow(pool, pool_state, block_size);
         } while (new.end < new.next);

         old.u64 = __sync_lock_test_and_set(&pool_state->u64, new.u64);
         if (old.next != state.next)
            futex_wake(&pool_state->end, INT_MAX);
         return state.next;
      } else {
         futex_wait(&pool_state->end, state.end, NULL);
         continue;
      }
   }
}

int32_t
anv_block_pool_alloc(struct anv_block_pool *pool,
                     uint32_t block_size, uint32_t *padding)
{
   uint32_t offset;

   offset = anv_block_pool_alloc_new(pool, &pool->state, block_size, padding);

   return offset;
}

/* Allocates a block out of the back of the block pool.
 *
 * This will allocated a block earlier than the "start" of the block pool.
 * The offsets returned from this function will be negative but will still
 * be correct relative to the block pool's map pointer.
 *
 * If you ever use anv_block_pool_alloc_back, then you will have to do
 * gymnastics with the block pool's BO when doing relocations.
 */
int32_t
anv_block_pool_alloc_back(struct anv_block_pool *pool,
                          uint32_t block_size)
{
   int32_t offset = anv_block_pool_alloc_new(pool, &pool->back_state,
                                             block_size, NULL);

   /* The offset we get out of anv_block_pool_alloc_new() is actually the
    * number of bytes downwards from the middle to the end of the block.
    * We need to turn it into a (negative) offset from the middle to the
    * start of the block.
    */
   assert(offset >= 0);
   return -(offset + block_size);
}

VkResult
anv_state_pool_init(struct anv_state_pool *pool,
                    struct anv_device *device,
                    const char *name,
                    uint64_t base_address,
                    int32_t start_offset,
                    uint32_t block_size)
{
   /* We don't want to ever see signed overflow */
   assert(start_offset < INT32_MAX - (int32_t)BLOCK_POOL_MEMFD_SIZE);

   uint32_t initial_size = block_size * 16;

   VkResult result = anv_block_pool_init(&pool->block_pool, device, name,
                                         base_address + start_offset,
                                         initial_size);
   if (result != VK_SUCCESS)
      return result;

   pool->start_offset = start_offset;

   result = anv_state_table_init(&pool->table, device, 64);
   if (result != VK_SUCCESS) {
      anv_block_pool_finish(&pool->block_pool);
      return result;
   }

   assert(util_is_power_of_two_or_zero(block_size));
   pool->block_size = block_size;
   pool->back_alloc_free_list = ANV_FREE_LIST_EMPTY;
   for (unsigned i = 0; i < ANV_STATE_BUCKETS; i++) {
      pool->buckets[i].free_list = ANV_FREE_LIST_EMPTY;
      pool->buckets[i].block.next = 0;
      pool->buckets[i].block.end = 0;
   }
   VG(VALGRIND_CREATE_MEMPOOL(pool, 0, false));

   return VK_SUCCESS;
}

void
anv_state_pool_finish(struct anv_state_pool *pool)
{
   VG(VALGRIND_DESTROY_MEMPOOL(pool));
   anv_state_table_finish(&pool->table);
   anv_block_pool_finish(&pool->block_pool);
}

static uint32_t
anv_fixed_size_state_pool_alloc_new(struct anv_fixed_size_state_pool *pool,
                                    struct anv_block_pool *block_pool,
                                    uint32_t state_size,
                                    uint32_t block_size,
                                    uint32_t *padding)
{
   struct anv_block_state block, old, new;
   uint32_t offset;

   /* We don't always use anv_block_pool_alloc(), which would set *padding to
    * zero for us. So if we have a pointer to padding, we must zero it out
    * ourselves here, to make sure we always return some sensible value.
    */
   if (padding)
      *padding = 0;

   /* If our state is large, we don't need any sub-allocation from a block.
    * Instead, we just grab whole (potentially large) blocks.
    */
   if (state_size >= block_size)
      return anv_block_pool_alloc(block_pool, state_size, padding);

 restart:
   block.u64 = __sync_fetch_and_add(&pool->block.u64, state_size);

   if (block.next < block.end) {
      return block.next;
   } else if (block.next == block.end) {
      offset = anv_block_pool_alloc(block_pool, block_size, padding);
      new.next = offset + state_size;
      new.end = offset + block_size;
      old.u64 = __sync_lock_test_and_set(&pool->block.u64, new.u64);
      if (old.next != block.next)
         futex_wake(&pool->block.end, INT_MAX);
      return offset;
   } else {
      futex_wait(&pool->block.end, block.end, NULL);
      goto restart;
   }
}

static uint32_t
anv_state_pool_get_bucket(uint32_t size)
{
   unsigned size_log2 = ilog2_round_up(size);
   assert(size_log2 <= ANV_MAX_STATE_SIZE_LOG2);
   if (size_log2 < ANV_MIN_STATE_SIZE_LOG2)
      size_log2 = ANV_MIN_STATE_SIZE_LOG2;
   return size_log2 - ANV_MIN_STATE_SIZE_LOG2;
}

static uint32_t
anv_state_pool_get_bucket_size(uint32_t bucket)
{
   uint32_t size_log2 = bucket + ANV_MIN_STATE_SIZE_LOG2;
   return 1 << size_log2;
}

/** Helper to push a chunk into the state table.
 *
 * It creates 'count' entries into the state table and update their sizes,
 * offsets and maps, also pushing them as "free" states.
 */
static void
anv_state_pool_return_blocks(struct anv_state_pool *pool,
                             uint32_t chunk_offset, uint32_t count,
                             uint32_t block_size)
{
   /* Disallow returning 0 chunks */
   assert(count != 0);

   /* Make sure we always return chunks aligned to the block_size */
   assert(chunk_offset % block_size == 0);

   uint32_t st_idx;
   UNUSED VkResult result = anv_state_table_add(&pool->table, &st_idx, count);
   assert(result == VK_SUCCESS);
   for (int i = 0; i < count; i++) {
      /* update states that were added back to the state table */
      struct anv_state *state_i = anv_state_table_get(&pool->table,
                                                      st_idx + i);
      state_i->alloc_size = block_size;
      state_i->offset = pool->start_offset + chunk_offset + block_size * i;
      state_i->map = anv_block_pool_map(&pool->block_pool,
                                        state_i->offset,
                                        state_i->alloc_size);
   }

   uint32_t block_bucket = anv_state_pool_get_bucket(block_size);
   anv_free_list_push(&pool->buckets[block_bucket].free_list,
                      &pool->table, st_idx, count);
}

/** Returns a chunk of memory back to the state pool.
 *
 * Do a two-level split. If chunk_size is bigger than divisor
 * (pool->block_size), we return as many divisor sized blocks as we can, from
 * the end of the chunk.
 *
 * The remaining is then split into smaller blocks (starting at small_size if
 * it is non-zero), with larger blocks always being taken from the end of the
 * chunk.
 */
static void
anv_state_pool_return_chunk(struct anv_state_pool *pool,
                            uint32_t chunk_offset, uint32_t chunk_size,
                            uint32_t small_size)
{
   uint32_t divisor = pool->block_size;
   uint32_t nblocks = chunk_size / divisor;
   uint32_t rest = chunk_size - nblocks * divisor;

   if (nblocks > 0) {
      /* First return divisor aligned and sized chunks. We start returning
       * larger blocks from the end of the chunk, since they should already be
       * aligned to divisor. Also anv_state_pool_return_blocks() only accepts
       * aligned chunks.
       */
      uint32_t offset = chunk_offset + rest;
      anv_state_pool_return_blocks(pool, offset, nblocks, divisor);
   }

   chunk_size = rest;
   divisor /= 2;

   if (small_size > 0 && small_size < divisor)
      divisor = small_size;

   uint32_t min_size = 1 << ANV_MIN_STATE_SIZE_LOG2;

   /* Just as before, return larger divisor aligned blocks from the end of the
    * chunk first.
    */
   while (chunk_size > 0 && divisor >= min_size) {
      nblocks = chunk_size / divisor;
      rest = chunk_size - nblocks * divisor;
      if (nblocks > 0) {
         anv_state_pool_return_blocks(pool, chunk_offset + rest,
                                      nblocks, divisor);
         chunk_size = rest;
      }
      divisor /= 2;
   }
}

static struct anv_state
anv_state_pool_alloc_no_vg(struct anv_state_pool *pool,
                           uint32_t size, uint32_t align)
{
   uint32_t bucket = anv_state_pool_get_bucket(MAX2(size, align));

   struct anv_state *state;
   uint32_t alloc_size = anv_state_pool_get_bucket_size(bucket);
   int32_t offset;

   /* Try free list first. */
   state = anv_free_list_pop(&pool->buckets[bucket].free_list,
                             &pool->table);
   if (state) {
      assert(state->offset >= pool->start_offset);
      goto done;
   }

   /* Try to grab a chunk from some larger bucket and split it up */
   for (unsigned b = bucket + 1; b < ANV_STATE_BUCKETS; b++) {
      state = anv_free_list_pop(&pool->buckets[b].free_list, &pool->table);
      if (state) {
         unsigned chunk_size = anv_state_pool_get_bucket_size(b);
         int32_t chunk_offset = state->offset;

         /* First lets update the state we got to its new size. offset and map
          * remain the same.
          */
         state->alloc_size = alloc_size;

         /* Now return the unused part of the chunk back to the pool as free
          * blocks
          *
          * There are a couple of options as to what we do with it:
          *
          *    1) We could fully split the chunk into state.alloc_size sized
          *       pieces.  However, this would mean that allocating a 16B
          *       state could potentially split a 2MB chunk into 512K smaller
          *       chunks.  This would lead to unnecessary fragmentation.
          *
          *    2) The classic "buddy allocator" method would have us split the
          *       chunk in half and return one half.  Then we would split the
          *       remaining half in half and return one half, and repeat as
          *       needed until we get down to the size we want.  However, if
          *       you are allocating a bunch of the same size state (which is
          *       the common case), this means that every other allocation has
          *       to go up a level and every fourth goes up two levels, etc.
          *       This is not nearly as efficient as it could be if we did a
          *       little more work up-front.
          *
          *    3) Split the difference between (1) and (2) by doing a
          *       two-level split.  If it's bigger than some fixed block_size,
          *       we split it into block_size sized chunks and return all but
          *       one of them.  Then we split what remains into
          *       state.alloc_size sized chunks and return them.
          *
          * We choose something close to option (3), which is implemented with
          * anv_state_pool_return_chunk(). That is done by returning the
          * remaining of the chunk, with alloc_size as a hint of the size that
          * we want the smaller chunk split into.
          */
         anv_state_pool_return_chunk(pool, chunk_offset + alloc_size,
                                     chunk_size - alloc_size, alloc_size);
         goto done;
      }
   }

   uint32_t padding;
   offset = anv_fixed_size_state_pool_alloc_new(&pool->buckets[bucket],
                                                &pool->block_pool,
                                                alloc_size,
                                                pool->block_size,
                                                &padding);
   /* Every time we allocate a new state, add it to the state pool */
   uint32_t idx;
   UNUSED VkResult result = anv_state_table_add(&pool->table, &idx, 1);
   assert(result == VK_SUCCESS);

   state = anv_state_table_get(&pool->table, idx);
   state->offset = pool->start_offset + offset;
   state->alloc_size = alloc_size;
   state->map = anv_block_pool_map(&pool->block_pool, offset, alloc_size);

   if (padding > 0) {
      uint32_t return_offset = offset - padding;
      anv_state_pool_return_chunk(pool, return_offset, padding, 0);
   }

done:
   return *state;
}

struct anv_state
anv_state_pool_alloc(struct anv_state_pool *pool, uint32_t size, uint32_t align)
{
   if (size == 0)
      return ANV_STATE_NULL;

   struct anv_state state = anv_state_pool_alloc_no_vg(pool, size, align);
   VG(VALGRIND_MEMPOOL_ALLOC(pool, state.map, size));
   return state;
}

struct anv_state
anv_state_pool_alloc_back(struct anv_state_pool *pool)
{
   struct anv_state *state;
   uint32_t alloc_size = pool->block_size;

   /* This function is only used with pools where start_offset == 0 */
   assert(pool->start_offset == 0);

   state = anv_free_list_pop(&pool->back_alloc_free_list, &pool->table);
   if (state) {
      assert(state->offset < pool->start_offset);
      goto done;
   }

   int32_t offset;
   offset = anv_block_pool_alloc_back(&pool->block_pool,
                                      pool->block_size);
   uint32_t idx;
   UNUSED VkResult result = anv_state_table_add(&pool->table, &idx, 1);
   assert(result == VK_SUCCESS);

   state = anv_state_table_get(&pool->table, idx);
   state->offset = pool->start_offset + offset;
   state->alloc_size = alloc_size;
   state->map = anv_block_pool_map(&pool->block_pool, offset, alloc_size);

done:
   VG(VALGRIND_MEMPOOL_ALLOC(pool, state->map, state->alloc_size));
   return *state;
}

static void
anv_state_pool_free_no_vg(struct anv_state_pool *pool, struct anv_state state)
{
   assert(util_is_power_of_two_or_zero(state.alloc_size));
   unsigned bucket = anv_state_pool_get_bucket(state.alloc_size);

   if (state.offset < pool->start_offset) {
      assert(state.alloc_size == pool->block_size);
      anv_free_list_push(&pool->back_alloc_free_list,
                         &pool->table, state.idx, 1);
   } else {
      anv_free_list_push(&pool->buckets[bucket].free_list,
                         &pool->table, state.idx, 1);
   }
}

void
anv_state_pool_free(struct anv_state_pool *pool, struct anv_state state)
{
   if (state.alloc_size == 0)
      return;

   VG(VALGRIND_MEMPOOL_FREE(pool, state.map));
   anv_state_pool_free_no_vg(pool, state);
}

struct anv_state_stream_block {
   struct anv_state block;

   /* The next block */
   struct anv_state_stream_block *next;

#ifdef HAVE_VALGRIND
   /* A pointer to the first user-allocated thing in this block.  This is
    * what valgrind sees as the start of the block.
    */
   void *_vg_ptr;
#endif
};

/* The state stream allocator is a one-shot, single threaded allocator for
 * variable sized blocks.  We use it for allocating dynamic state.
 */
void
anv_state_stream_init(struct anv_state_stream *stream,
                      struct anv_state_pool *state_pool,
                      uint32_t block_size)
{
   stream->state_pool = state_pool;
   stream->block_size = block_size;

   stream->block = ANV_STATE_NULL;

   /* Ensure that next + whatever > block_size.  This way the first call to
    * state_stream_alloc fetches a new block.
    */
   stream->next = block_size;

   util_dynarray_init(&stream->all_blocks, NULL);

   VG(VALGRIND_CREATE_MEMPOOL(stream, 0, false));
}

void
anv_state_stream_finish(struct anv_state_stream *stream)
{
   util_dynarray_foreach(&stream->all_blocks, struct anv_state, block) {
      VG(VALGRIND_MEMPOOL_FREE(stream, block->map));
      VG(VALGRIND_MAKE_MEM_NOACCESS(block->map, block->alloc_size));
      anv_state_pool_free_no_vg(stream->state_pool, *block);
   }
   util_dynarray_fini(&stream->all_blocks);

   VG(VALGRIND_DESTROY_MEMPOOL(stream));
}

struct anv_state
anv_state_stream_alloc(struct anv_state_stream *stream,
                       uint32_t size, uint32_t alignment)
{
   if (size == 0)
      return ANV_STATE_NULL;

   assert(alignment <= PAGE_SIZE);

   uint32_t offset = align(stream->next, alignment);
   if (offset + size > stream->block.alloc_size) {
      uint32_t block_size = stream->block_size;
      if (block_size < size)
         block_size = round_to_power_of_two(size);

      stream->block = anv_state_pool_alloc_no_vg(stream->state_pool,
                                                 block_size, PAGE_SIZE);
      util_dynarray_append(&stream->all_blocks,
                           struct anv_state, stream->block);
      VG(VALGRIND_MAKE_MEM_NOACCESS(stream->block.map, block_size));

      /* Reset back to the start */
      stream->next = offset = 0;
      assert(offset + size <= stream->block.alloc_size);
   }
   const bool new_block = stream->next == 0;

   struct anv_state state = stream->block;
   state.offset += offset;
   state.alloc_size = size;
   state.map += offset;

   stream->next = offset + size;

   if (new_block) {
      assert(state.map == stream->block.map);
      VG(VALGRIND_MEMPOOL_ALLOC(stream, state.map, size));
   } else {
      /* This only updates the mempool.  The newly allocated chunk is still
       * marked as NOACCESS. */
      VG(VALGRIND_MEMPOOL_CHANGE(stream, stream->block.map, stream->block.map,
                                 stream->next));
      /* Mark the newly allocated chunk as undefined */
      VG(VALGRIND_MAKE_MEM_UNDEFINED(state.map, state.alloc_size));
   }

   return state;
}

void
anv_state_reserved_pool_init(struct anv_state_reserved_pool *pool,
                             struct anv_state_pool *parent,
                             uint32_t count, uint32_t size, uint32_t alignment)
{
   pool->pool = parent;
   pool->reserved_blocks = ANV_FREE_LIST_EMPTY;
   pool->count = count;

   for (unsigned i = 0; i < count; i++) {
      struct anv_state state = anv_state_pool_alloc(pool->pool, size, alignment);
      anv_free_list_push(&pool->reserved_blocks, &pool->pool->table, state.idx, 1);
   }
}

void
anv_state_reserved_pool_finish(struct anv_state_reserved_pool *pool)
{
   struct anv_state *state;

   while ((state = anv_free_list_pop(&pool->reserved_blocks, &pool->pool->table))) {
      anv_state_pool_free(pool->pool, *state);
      pool->count--;
   }
   assert(pool->count == 0);
}

struct anv_state
anv_state_reserved_pool_alloc(struct anv_state_reserved_pool *pool)
{
   return *anv_free_list_pop(&pool->reserved_blocks, &pool->pool->table);
}

void
anv_state_reserved_pool_free(struct anv_state_reserved_pool *pool,
                             struct anv_state state)
{
   anv_free_list_push(&pool->reserved_blocks, &pool->pool->table, state.idx, 1);
}

void
anv_bo_pool_init(struct anv_bo_pool *pool, struct anv_device *device,
                 const char *name)
{
   pool->name = name;
   pool->device = device;
   for (unsigned i = 0; i < ARRAY_SIZE(pool->free_list); i++) {
      util_sparse_array_free_list_init(&pool->free_list[i],
                                       &device->bo_cache.bo_map, 0,
                                       offsetof(struct anv_bo, free_index));
   }

   VG(VALGRIND_CREATE_MEMPOOL(pool, 0, false));
}

void
anv_bo_pool_finish(struct anv_bo_pool *pool)
{
   for (unsigned i = 0; i < ARRAY_SIZE(pool->free_list); i++) {
      while (1) {
         struct anv_bo *bo =
            util_sparse_array_free_list_pop_elem(&pool->free_list[i]);
         if (bo == NULL)
            break;

         /* anv_device_release_bo is going to "free" it */
         VG(VALGRIND_MALLOCLIKE_BLOCK(bo->map, bo->size, 0, 1));
         anv_device_release_bo(pool->device, bo);
      }
   }

   VG(VALGRIND_DESTROY_MEMPOOL(pool));
}

VkResult
anv_bo_pool_alloc(struct anv_bo_pool *pool, uint32_t size,
                  struct anv_bo **bo_out)
{
   const unsigned size_log2 = size < 4096 ? 12 : ilog2_round_up(size);
   const unsigned pow2_size = 1 << size_log2;
   const unsigned bucket = size_log2 - 12;
   assert(bucket < ARRAY_SIZE(pool->free_list));

   struct anv_bo *bo =
      util_sparse_array_free_list_pop_elem(&pool->free_list[bucket]);
   if (bo != NULL) {
      VG(VALGRIND_MEMPOOL_ALLOC(pool, bo->map, size));
      *bo_out = bo;
      return VK_SUCCESS;
   }

   VkResult result = anv_device_alloc_bo(pool->device,
                                         pool->name,
                                         pow2_size,
                                         ANV_BO_ALLOC_MAPPED |
                                         ANV_BO_ALLOC_SNOOPED |
                                         ANV_BO_ALLOC_CAPTURE,
                                         0 /* explicit_address */,
                                         &bo);
   if (result != VK_SUCCESS)
      return result;

   /* We want it to look like it came from this pool */
   VG(VALGRIND_FREELIKE_BLOCK(bo->map, 0));
   VG(VALGRIND_MEMPOOL_ALLOC(pool, bo->map, size));

   *bo_out = bo;

   return VK_SUCCESS;
}

void
anv_bo_pool_free(struct anv_bo_pool *pool, struct anv_bo *bo)
{
   VG(VALGRIND_MEMPOOL_FREE(pool, bo->map));

   assert(util_is_power_of_two_or_zero(bo->size));
   const unsigned size_log2 = ilog2_round_up(bo->size);
   const unsigned bucket = size_log2 - 12;
   assert(bucket < ARRAY_SIZE(pool->free_list));

   assert(util_sparse_array_get(&pool->device->bo_cache.bo_map,
                                bo->gem_handle) == bo);
   util_sparse_array_free_list_push(&pool->free_list[bucket],
                                    &bo->gem_handle, 1);
}

// Scratch pool

void
anv_scratch_pool_init(struct anv_device *device, struct anv_scratch_pool *pool)
{
   memset(pool, 0, sizeof(*pool));
}

void
anv_scratch_pool_finish(struct anv_device *device, struct anv_scratch_pool *pool)
{
   for (unsigned s = 0; s < ARRAY_SIZE(pool->bos[0]); s++) {
      for (unsigned i = 0; i < 16; i++) {
         if (pool->bos[i][s] != NULL)
            anv_device_release_bo(device, pool->bos[i][s]);
      }
   }
}

struct anv_bo *
anv_scratch_pool_alloc(struct anv_device *device, struct anv_scratch_pool *pool,
                       gl_shader_stage stage, unsigned per_thread_scratch)
{
   if (per_thread_scratch == 0)
      return NULL;

   unsigned scratch_size_log2 = ffs(per_thread_scratch / 2048);
   assert(scratch_size_log2 < 16);

   assert(stage < ARRAY_SIZE(pool->bos));

   const struct intel_device_info *devinfo = device->info;

   struct anv_bo *bo = p_atomic_read(&pool->bos[scratch_size_log2][stage]);

   if (bo != NULL)
      return bo;

   assert(stage < ARRAY_SIZE(devinfo->max_scratch_ids));
   uint32_t size = per_thread_scratch * devinfo->max_scratch_ids[stage];

   /* Even though the Scratch base pointers in 3DSTATE_*S are 64 bits, they
    * are still relative to the general state base address.  When we emit
    * STATE_BASE_ADDRESS, we set general state base address to 0 and the size
    * to the maximum (1 page under 4GB).  This allows us to just place the
    * scratch buffers anywhere we wish in the bottom 32 bits of address space
    * and just set the scratch base pointer in 3DSTATE_*S using a relocation.
    * However, in order to do so, we need to ensure that the kernel does not
    * place the scratch BO above the 32-bit boundary.
    *
    * NOTE: Technically, it can't go "anywhere" because the top page is off
    * limits.  However, when EXEC_OBJECT_SUPPORTS_48B_ADDRESS is set, the
    * kernel allocates space using
    *
    *    end = min_t(u64, end, (1ULL << 32) - I915_GTT_PAGE_SIZE);
    *
    * so nothing will ever touch the top page.
    */
   VkResult result = anv_device_alloc_bo(device, "scratch", size,
                                         ANV_BO_ALLOC_32BIT_ADDRESS,
                                         0 /* explicit_address */,
                                         &bo);
   if (result != VK_SUCCESS)
      return NULL; /* TODO */

   struct anv_bo *current_bo =
      p_atomic_cmpxchg(&pool->bos[scratch_size_log2][stage], NULL, bo);
   if (current_bo) {
      anv_device_release_bo(device, bo);
      return current_bo;
   } else {
      return bo;
   }
}

VkResult
anv_bo_cache_init(struct anv_bo_cache *cache, struct anv_device *device)
{
   util_sparse_array_init(&cache->bo_map, sizeof(struct anv_bo), 1024);

   if (pthread_mutex_init(&cache->mutex, NULL)) {
      util_sparse_array_finish(&cache->bo_map);
      return vk_errorf(device, VK_ERROR_OUT_OF_HOST_MEMORY,
                       "pthread_mutex_init failed: %m");
   }

   return VK_SUCCESS;
}

void
anv_bo_cache_finish(struct anv_bo_cache *cache)
{
   util_sparse_array_finish(&cache->bo_map);
   pthread_mutex_destroy(&cache->mutex);
}

#define ANV_BO_CACHE_SUPPORTED_FLAGS \
   (EXEC_OBJECT_WRITE | \
    EXEC_OBJECT_ASYNC | \
    EXEC_OBJECT_SUPPORTS_48B_ADDRESS | \
    EXEC_OBJECT_PINNED | \
    EXEC_OBJECT_CAPTURE)

static uint32_t
anv_bo_alloc_flags_to_bo_flags(struct anv_device *device,
                               enum anv_bo_alloc_flags alloc_flags)
{
   struct anv_physical_device *pdevice = device->physical;

   uint64_t bo_flags = 0;
   if (!(alloc_flags & ANV_BO_ALLOC_32BIT_ADDRESS) &&
       pdevice->supports_48bit_addresses)
      bo_flags |= EXEC_OBJECT_SUPPORTS_48B_ADDRESS;

   if ((alloc_flags & ANV_BO_ALLOC_CAPTURE) && pdevice->has_exec_capture)
      bo_flags |= EXEC_OBJECT_CAPTURE;

   if (alloc_flags & ANV_BO_ALLOC_IMPLICIT_WRITE) {
      assert(alloc_flags & ANV_BO_ALLOC_IMPLICIT_SYNC);
      bo_flags |= EXEC_OBJECT_WRITE;
   }

   if (!(alloc_flags & ANV_BO_ALLOC_IMPLICIT_SYNC) && pdevice->has_exec_async)
      bo_flags |= EXEC_OBJECT_ASYNC;

   if (pdevice->use_softpin)
      bo_flags |= EXEC_OBJECT_PINNED;

   return bo_flags;
}

static void
anv_bo_finish(struct anv_device *device, struct anv_bo *bo)
{
   if (bo->offset != 0 && anv_bo_is_pinned(bo) && !bo->has_fixed_address)
      anv_vma_free(device, bo->offset, bo->size);

   if (bo->map && !bo->from_host_ptr)
      anv_device_unmap_bo(device, bo, bo->map, bo->size);

   assert(bo->gem_handle != 0);
   anv_gem_close(device, bo->gem_handle);
}

static VkResult
anv_bo_vma_alloc_or_close(struct anv_device *device,
                          struct anv_bo *bo,
                          enum anv_bo_alloc_flags alloc_flags,
                          uint64_t explicit_address)
{
   assert(anv_bo_is_pinned(bo));
   assert(explicit_address == intel_48b_address(explicit_address));

   uint32_t align = 4096;

   if (alloc_flags & ANV_BO_ALLOC_FIXED_ADDRESS) {
      bo->has_fixed_address = true;
      bo->offset = explicit_address;
   } else {
      bo->offset = anv_vma_alloc(device, bo->size,
                                 align, alloc_flags, explicit_address);
      if (bo->offset == 0) {
         anv_bo_finish(device, bo);
         return vk_errorf(device, VK_ERROR_OUT_OF_DEVICE_MEMORY,
                          "failed to allocate virtual address for BO");
      }
   }

   return VK_SUCCESS;
}

VkResult
anv_device_alloc_bo(struct anv_device *device,
                    const char *name,
                    uint64_t size,
                    enum anv_bo_alloc_flags alloc_flags,
                    uint64_t explicit_address,
                    struct anv_bo **bo_out)
{
   const uint32_t bo_flags =
      anv_bo_alloc_flags_to_bo_flags(device, alloc_flags);
   assert(bo_flags == (bo_flags & ANV_BO_CACHE_SUPPORTED_FLAGS));

   /* The kernel is going to give us whole pages anyway */
   size = align64(size, 4096);

   uint32_t gem_handle = anv_gem_create(device, size);
   if (gem_handle == 0)
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   struct anv_bo new_bo = {
      .name = name,
      .gem_handle = gem_handle,
      .refcount = 1,
      .offset = -1,
      .size = size,
      .flags = bo_flags,
      .is_external = (alloc_flags & ANV_BO_ALLOC_EXTERNAL),
      .has_client_visible_address =
         (alloc_flags & ANV_BO_ALLOC_CLIENT_VISIBLE_ADDRESS) != 0,
   };

   if (alloc_flags & ANV_BO_ALLOC_MAPPED) {
      VkResult result = anv_device_map_bo(device, &new_bo, 0, size,
                                          0 /* gem_flags */, &new_bo.map);
      if (unlikely(result != VK_SUCCESS)) {
         anv_gem_close(device, new_bo.gem_handle);
         return result;
      }
   }

   if (alloc_flags & ANV_BO_ALLOC_SNOOPED) {
      assert(alloc_flags & ANV_BO_ALLOC_MAPPED);
      /* We don't want to change these defaults if it's going to be shared
       * with another process.
       */
      assert(!(alloc_flags & ANV_BO_ALLOC_EXTERNAL));

      /* Regular objects are created I915_CACHING_CACHED on LLC platforms and
       * I915_CACHING_NONE on non-LLC platforms.  For many internal state
       * objects, we'd rather take the snooping overhead than risk forgetting
       * a CLFLUSH somewhere.  Userptr objects are always created as
       * I915_CACHING_CACHED, which on non-LLC means snooped so there's no
       * need to do this there.
       */
      if (!device->info->has_llc) {
         anv_gem_set_caching(device, new_bo.gem_handle,
                             I915_CACHING_CACHED);
      }
   }

   if (anv_bo_is_pinned(&new_bo)) {
      VkResult result = anv_bo_vma_alloc_or_close(device, &new_bo,
                                                  alloc_flags,
                                                  explicit_address);
      if (result != VK_SUCCESS)
         return result;
   } else {
      assert(!new_bo.has_client_visible_address);
   }

   assert(new_bo.gem_handle);

   /* If we just got this gem_handle from anv_bo_init_new then we know no one
    * else is touching this BO at the moment so we don't need to lock here.
    */
   struct anv_bo *bo = anv_device_lookup_bo(device, new_bo.gem_handle);
   *bo = new_bo;

   *bo_out = bo;

   return VK_SUCCESS;
}

VkResult
anv_device_map_bo(struct anv_device *device,
                  struct anv_bo *bo,
                  uint64_t offset,
                  size_t size,
                  uint32_t gem_flags,
                  void **map_out)
{
   assert(!bo->is_wrapper && !bo->from_host_ptr);
   assert(size > 0);

   void *map = anv_gem_mmap(device, bo->gem_handle, offset, size, gem_flags);
   if (unlikely(map == MAP_FAILED))
      return vk_errorf(device, VK_ERROR_MEMORY_MAP_FAILED, "mmap failed: %m");

   assert(map != NULL);

   if (map_out)
      *map_out = map;

   return VK_SUCCESS;
}

void
anv_device_unmap_bo(struct anv_device *device,
                    struct anv_bo *bo,
                    void *map, size_t map_size)
{
   assert(!bo->is_wrapper && !bo->from_host_ptr);

   anv_gem_munmap(device, map, map_size);
}

VkResult
anv_device_import_bo_from_host_ptr(struct anv_device *device,
                                   void *host_ptr, uint32_t size,
                                   enum anv_bo_alloc_flags alloc_flags,
                                   uint64_t client_address,
                                   struct anv_bo **bo_out)
{
   assert(!(alloc_flags & (ANV_BO_ALLOC_MAPPED |
                           ANV_BO_ALLOC_SNOOPED |
                           ANV_BO_ALLOC_FIXED_ADDRESS)));

   struct anv_bo_cache *cache = &device->bo_cache;
   const uint32_t bo_flags =
      anv_bo_alloc_flags_to_bo_flags(device, alloc_flags);
   assert(bo_flags == (bo_flags & ANV_BO_CACHE_SUPPORTED_FLAGS));

   uint32_t gem_handle = anv_gem_userptr(device, host_ptr, size);
   if (!gem_handle)
      return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);

   pthread_mutex_lock(&cache->mutex);

   struct anv_bo *bo = anv_device_lookup_bo(device, gem_handle);
   if (bo->refcount > 0) {
      /* VK_EXT_external_memory_host doesn't require handling importing the
       * same pointer twice at the same time, but we don't get in the way.  If
       * kernel gives us the same gem_handle, only succeed if the flags match.
       */
      assert(bo->gem_handle == gem_handle);
      if (bo_flags != bo->flags) {
         pthread_mutex_unlock(&cache->mutex);
         return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                          "same host pointer imported two different ways");
      }

      if (bo->has_client_visible_address !=
          ((alloc_flags & ANV_BO_ALLOC_CLIENT_VISIBLE_ADDRESS) != 0)) {
         pthread_mutex_unlock(&cache->mutex);
         return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                          "The same BO was imported with and without buffer "
                          "device address");
      }

      if (client_address && client_address != intel_48b_address(bo->offset)) {
         pthread_mutex_unlock(&cache->mutex);
         return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                          "The same BO was imported at two different "
                          "addresses");
      }

      __sync_fetch_and_add(&bo->refcount, 1);
   } else {
      struct anv_bo new_bo = {
         .name = "host-ptr",
         .gem_handle = gem_handle,
         .refcount = 1,
         .offset = -1,
         .size = size,
         .map = host_ptr,
         .flags = bo_flags,
         .is_external = true,
         .from_host_ptr = true,
         .has_client_visible_address =
            (alloc_flags & ANV_BO_ALLOC_CLIENT_VISIBLE_ADDRESS) != 0,
      };

      if (anv_bo_is_pinned(&new_bo)) {
         VkResult result = anv_bo_vma_alloc_or_close(device, &new_bo,
                                                     alloc_flags,
                                                     client_address);
         if (result != VK_SUCCESS) {
            pthread_mutex_unlock(&cache->mutex);
            return result;
         }
      } else {
         assert(!new_bo.has_client_visible_address);
      }

      *bo = new_bo;
   }

   pthread_mutex_unlock(&cache->mutex);
   *bo_out = bo;

   return VK_SUCCESS;
}

VkResult
anv_device_import_bo(struct anv_device *device,
                     int fd,
                     enum anv_bo_alloc_flags alloc_flags,
                     uint64_t client_address,
                     struct anv_bo **bo_out)
{
   assert(!(alloc_flags & (ANV_BO_ALLOC_MAPPED |
                           ANV_BO_ALLOC_SNOOPED |
                           ANV_BO_ALLOC_FIXED_ADDRESS)));

   struct anv_bo_cache *cache = &device->bo_cache;
   const uint32_t bo_flags =
      anv_bo_alloc_flags_to_bo_flags(device, alloc_flags);
   assert(bo_flags == (bo_flags & ANV_BO_CACHE_SUPPORTED_FLAGS));

   pthread_mutex_lock(&cache->mutex);

   uint32_t gem_handle = anv_gem_fd_to_handle(device, fd);
   if (!gem_handle) {
      pthread_mutex_unlock(&cache->mutex);
      return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
   }

   struct anv_bo *bo = anv_device_lookup_bo(device, gem_handle);
   if (bo->refcount > 0) {
      /* We have to be careful how we combine flags so that it makes sense.
       * Really, though, if we get to this case and it actually matters, the
       * client has imported a BO twice in different ways and they get what
       * they have coming.
       */
      uint64_t new_flags = 0;
      new_flags |= (bo->flags | bo_flags) & EXEC_OBJECT_WRITE;
      new_flags |= (bo->flags & bo_flags) & EXEC_OBJECT_ASYNC;
      new_flags |= (bo->flags & bo_flags) & EXEC_OBJECT_SUPPORTS_48B_ADDRESS;
      new_flags |= (bo->flags | bo_flags) & EXEC_OBJECT_PINNED;
      new_flags |= (bo->flags | bo_flags) & EXEC_OBJECT_CAPTURE;

      /* It's theoretically possible for a BO to get imported such that it's
       * both pinned and not pinned.  The only way this can happen is if it
       * gets imported as both a semaphore and a memory object and that would
       * be an application error.  Just fail out in that case.
       */
      if ((bo->flags & EXEC_OBJECT_PINNED) !=
          (bo_flags & EXEC_OBJECT_PINNED)) {
         pthread_mutex_unlock(&cache->mutex);
         return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                          "The same BO was imported two different ways");
      }

      /* It's also theoretically possible that someone could export a BO from
       * one heap and import it into another or to import the same BO into two
       * different heaps.  If this happens, we could potentially end up both
       * allowing and disallowing 48-bit addresses.  There's not much we can
       * do about it if we're pinning so we just throw an error and hope no
       * app is actually that stupid.
       */
      if ((new_flags & EXEC_OBJECT_PINNED) &&
          (bo->flags & EXEC_OBJECT_SUPPORTS_48B_ADDRESS) !=
          (bo_flags & EXEC_OBJECT_SUPPORTS_48B_ADDRESS)) {
         pthread_mutex_unlock(&cache->mutex);
         return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                          "The same BO was imported on two different heaps");
      }

      if (bo->has_client_visible_address !=
          ((alloc_flags & ANV_BO_ALLOC_CLIENT_VISIBLE_ADDRESS) != 0)) {
         pthread_mutex_unlock(&cache->mutex);
         return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                          "The same BO was imported with and without buffer "
                          "device address");
      }

      if (client_address && client_address != intel_48b_address(bo->offset)) {
         pthread_mutex_unlock(&cache->mutex);
         return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                          "The same BO was imported at two different "
                          "addresses");
      }

      bo->flags = new_flags;

      __sync_fetch_and_add(&bo->refcount, 1);
   } else {
      off_t size = lseek(fd, 0, SEEK_END);
      if (size == (off_t)-1) {
         anv_gem_close(device, gem_handle);
         pthread_mutex_unlock(&cache->mutex);
         return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
      }

      struct anv_bo new_bo = {
         .name = "imported",
         .gem_handle = gem_handle,
         .refcount = 1,
         .offset = -1,
         .size = size,
         .flags = bo_flags,
         .is_external = true,
         .has_client_visible_address =
            (alloc_flags & ANV_BO_ALLOC_CLIENT_VISIBLE_ADDRESS) != 0,
      };

      if (anv_bo_is_pinned(&new_bo)) {
         VkResult result = anv_bo_vma_alloc_or_close(device, &new_bo,
                                                     alloc_flags,
                                                     client_address);
         if (result != VK_SUCCESS) {
            pthread_mutex_unlock(&cache->mutex);
            return result;
         }
      } else {
         assert(!new_bo.has_client_visible_address);
      }

      *bo = new_bo;
   }

   pthread_mutex_unlock(&cache->mutex);
   *bo_out = bo;

   return VK_SUCCESS;
}

VkResult
anv_device_export_bo(struct anv_device *device,
                     struct anv_bo *bo, int *fd_out)
{
   assert(anv_device_lookup_bo(device, bo->gem_handle) == bo);

   /* This BO must have been flagged external in order for us to be able
    * to export it.  This is done based on external options passed into
    * anv_AllocateMemory.
    */
   assert(bo->is_external);

   int fd = anv_gem_handle_to_fd(device, bo->gem_handle);
   if (fd < 0)
      return vk_error(device, VK_ERROR_TOO_MANY_OBJECTS);

   *fd_out = fd;

   return VK_SUCCESS;
}

VkResult
anv_device_get_bo_tiling(struct anv_device *device,
                         struct anv_bo *bo,
                         enum isl_tiling *tiling_out)
{
   int i915_tiling = anv_gem_get_tiling(device, bo->gem_handle);
   if (i915_tiling < 0) {
      return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                       "failed to get BO tiling: %m");
   }

   *tiling_out = isl_tiling_from_i915_tiling(i915_tiling);

   return VK_SUCCESS;
}

VkResult
anv_device_set_bo_tiling(struct anv_device *device,
                         struct anv_bo *bo,
                         uint32_t row_pitch_B,
                         enum isl_tiling tiling)
{
   int ret = anv_gem_set_tiling(device, bo->gem_handle, row_pitch_B,
                                isl_tiling_to_i915_tiling(tiling));
   if (ret) {
      return vk_errorf(device, VK_ERROR_OUT_OF_DEVICE_MEMORY,
                       "failed to set BO tiling: %m");
   }

   return VK_SUCCESS;
}

static bool
atomic_dec_not_one(uint32_t *counter)
{
   uint32_t old, val;

   val = *counter;
   while (1) {
      if (val == 1)
         return false;

      old = __sync_val_compare_and_swap(counter, val, val - 1);
      if (old == val)
         return true;

      val = old;
   }
}

void
anv_device_release_bo(struct anv_device *device,
                      struct anv_bo *bo)
{
   struct anv_bo_cache *cache = &device->bo_cache;
   assert(anv_device_lookup_bo(device, bo->gem_handle) == bo);

   /* Try to decrement the counter but don't go below one.  If this succeeds
    * then the refcount has been decremented and we are not the last
    * reference.
    */
   if (atomic_dec_not_one(&bo->refcount))
      return;

   pthread_mutex_lock(&cache->mutex);

   /* We are probably the last reference since our attempt to decrement above
    * failed.  However, we can't actually know until we are inside the mutex.
    * Otherwise, someone could import the BO between the decrement and our
    * taking the mutex.
    */
   if (unlikely(__sync_sub_and_fetch(&bo->refcount, 1) > 0)) {
      /* Turns out we're not the last reference.  Unlock and bail. */
      pthread_mutex_unlock(&cache->mutex);
      return;
   }
   assert(bo->refcount == 0);

   /* Memset the BO just in case.  The refcount being zero should be enough to
    * prevent someone from assuming the data is valid but it's safer to just
    * stomp to zero just in case.  We explicitly do this *before* we actually
    * close the GEM handle to ensure that if anyone allocates something and
    * gets the same GEM handle, the memset has already happen and won't stomp
    * all over any data they may write in this BO.
    */
   struct anv_bo old_bo = *bo;

   memset(bo, 0, sizeof(*bo));

   anv_bo_finish(device, &old_bo);

   /* Don't unlock until we've actually closed the BO.  The whole point of
    * the BO cache is to ensure that we correctly handle races with creating
    * and releasing GEM handles and we don't want to let someone import the BO
    * again between mutex unlock and closing the GEM handle.
    */
   pthread_mutex_unlock(&cache->mutex);
}
