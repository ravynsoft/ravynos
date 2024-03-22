/*
 * Copyright © 2019 Intel Corporation
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

#include "util/sparse_array.h"

#include "c11/threads.h"

#include <gtest/gtest.h>

#define NUM_SETS_PER_THREAD (1 << 10)
#define MAX_ARR_SIZE (1 << 20)

static int
test_thread(void *_state)
{
   struct util_sparse_array *arr = (struct util_sparse_array *)_state;
   for (unsigned i = 0; i < NUM_SETS_PER_THREAD; i++) {
      uint32_t idx = rand() % MAX_ARR_SIZE;
      uint32_t *elem = (uint32_t *)util_sparse_array_get(arr, idx);
      *elem = idx;
   }

   return 0;
}

TEST(SparseArrayTest, Multithread)
{
   const int num_runs = 16;

   for (unsigned run_idx = 0; run_idx < num_runs; run_idx++) {
      size_t node_size = 4 << (run_idx / 2);

      struct util_sparse_array arr;
      util_sparse_array_init(&arr, sizeof(uint32_t), node_size);

      thrd_t threads[16];
      for (unsigned i = 0; i < ARRAY_SIZE(threads); i++) {
         int ret = thrd_create(&threads[i], test_thread, &arr);
         ASSERT_EQ(ret, thrd_success);
      }

      for (unsigned i = 0; i < ARRAY_SIZE(threads); i++) {
         int ret = thrd_join(threads[i], NULL);
         ASSERT_EQ(ret, thrd_success);
      }

      util_sparse_array_validate(&arr);

      for (unsigned i = 0; i < MAX_ARR_SIZE; i++) {
         uint32_t *elem = (uint32_t *)util_sparse_array_get(&arr, i);
         ASSERT_TRUE(*elem == 0 || *elem == i)
            << "Element at index " << i << " is " << *elem
            << " but expected zero or " << i;
      }

      util_sparse_array_finish(&arr);
   }
}
