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

#include "anv_private.h"
#include "test_common.h"

void state_pool_padding_test(void);

void state_pool_padding_test(void)
{
   struct anv_physical_device physical_device = {
      .use_softpin = true,
   };
   struct anv_device device = {};
   struct anv_state_pool state_pool;

   anv_device_set_physical(&device, &physical_device);
   pthread_mutex_init(&device.mutex, NULL);
   anv_bo_cache_init(&device.bo_cache, &device);
   anv_state_pool_init(&state_pool, &device, "test", 4096, 0, 4096);

   /* Get the size of the underlying block_pool */
   struct anv_block_pool *bp = &state_pool.block_pool;
   uint64_t pool_size = bp->size;

   /* Grab one so the pool has some initial usage */
   anv_state_pool_alloc(&state_pool, 16, 16);

   /* Grab a state that is the size of the initial allocation */
   struct anv_state state = anv_state_pool_alloc(&state_pool, pool_size, 16);

   /* The pool must have grown */
   ASSERT(bp->size > pool_size);

   /* And the state must have been allocated at the end of the original size  */
   ASSERT(state.offset == pool_size);

   /* A new allocation that fits into the returned empty space should have an
    * offset within the original pool size
    */
   state = anv_state_pool_alloc(&state_pool, 4096, 16);
   ASSERT(state.offset + state.alloc_size <= pool_size);

   /* We should be able to allocate pool->block_size'd chunks in the returned area
    */
   int left_chunks = pool_size / 4096 - 2;
   for (int i = 0; i < left_chunks; i++) {
      state = anv_state_pool_alloc(&state_pool, 4096, 16);
      ASSERT(state.offset + state.alloc_size <= pool_size);
   }

   /* Now the next chunk to be allocated should make the pool grow again */
   pool_size = bp->size;
   state = anv_state_pool_alloc(&state_pool, 4096, 16);
   ASSERT(bp->size > pool_size);
   ASSERT(state.offset == pool_size);

   anv_state_pool_finish(&state_pool);
   anv_bo_cache_finish(&device.bo_cache);
   pthread_mutex_destroy(&device.mutex);
}
