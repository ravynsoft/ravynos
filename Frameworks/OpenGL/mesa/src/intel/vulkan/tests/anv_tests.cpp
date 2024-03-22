/*
 * Copyright © 2023 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>

#include "test_common.h"

#define ANV_C_TEST(S, N, C) extern "C" void C(void); TEST(S, N) { C(); }

ANV_C_TEST(StatePool, Regular, state_pool_test);
ANV_C_TEST(StatePool, FreeListOnly, state_pool_free_list_only_test);
ANV_C_TEST(StatePool, MaxSizeOverLimit, state_pool_max_size_over_limit);
ANV_C_TEST(StatePool, MaxSizeWithinLimit, state_pool_max_size_within_limit);
ANV_C_TEST(StatePool, NoFree, state_pool_no_free_test);
ANV_C_TEST(StatePool, Padding, state_pool_padding_test);

ANV_C_TEST(BlockPool, NoFree, block_pool_no_free_test);
ANV_C_TEST(BlockPool, GrowFirst, block_pool_grow_first_test);
ANV_C_TEST(BlockPool, MaxSize, block_pool_max_size);

extern "C" void FAIL_IN_GTEST(const char *file_path, unsigned line_number, const char *msg) {
   GTEST_FAIL_AT(file_path, line_number) << msg;
}
