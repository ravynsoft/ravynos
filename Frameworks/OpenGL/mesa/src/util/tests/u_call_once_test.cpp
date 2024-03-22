/*
 * Copyright 2022 Yonggang Luo
 * SPDX-License-Identifier: MIT
 *
 * Testing u_call_once.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>

#include "c11/threads.h"
#include "util/u_atomic.h"
#include "util/u_call_once.h"

#define NUM_DEBUG_TEST_THREAD 8

static void update_x(int *x) {
   p_atomic_inc(x);
}

static int xC;
static void update_x_global(void)
{
   update_x(&xC);
}

static int test_call_once(void *_state)
{
   static int xA = 0;
   {
      static once_flag once = ONCE_FLAG_INIT;
      for (int i = 0; i < 100; i += 1) {
         util_call_once_data_slow(&once, (util_call_once_data_func)update_x, &xA);
      }
   }

   static int xB = 0;
   {
      static util_once_flag once = UTIL_ONCE_FLAG_INIT;
      for (int i = 0; i < 100; i += 1) {
         util_call_once_data(&once, (util_call_once_data_func)update_x, &xB);
      }
   }
   {
      static util_once_flag once = UTIL_ONCE_FLAG_INIT;
      for (int i = 0; i < 100; i += 1) {
         util_call_once(&once, update_x_global);
      }
   }
   EXPECT_EQ(xA, 1);
   EXPECT_EQ(xB, 1);
   EXPECT_EQ(xC, 1);
   return 0;
}

TEST(UtilCallOnce, Multithread)
{
   thrd_t threads[NUM_DEBUG_TEST_THREAD];
   for (unsigned i = 0; i < NUM_DEBUG_TEST_THREAD; i++) {
        thrd_create(&threads[i], test_call_once, NULL);
   }
   for (unsigned i = 0; i < NUM_DEBUG_TEST_THREAD; i++) {
      int ret;
      thrd_join(threads[i], &ret);
   }
}
