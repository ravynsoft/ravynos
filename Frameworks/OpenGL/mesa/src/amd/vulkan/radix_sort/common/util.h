// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_GRAPHICS_LIB_COMPUTE_COMMON_UTIL_H_
#define SRC_GRAPHICS_LIB_COMPUTE_COMMON_UTIL_H_

//
//
//

#include <stdbool.h>
#include <stdint.h>

//
//
//

#ifdef __cplusplus
extern "C" {
#endif

//
//
//

// Return true iff |n| is a power of 2.
bool
is_pow2_u32(uint32_t n);

// Return |n| rounded-up to the nearest power of 2.
// If |n| is zero then return 0.
// REQUIRES: |n <= 0x80000000|.
uint32_t
pow2_ru_u32(uint32_t n);

// Return |n| rounded-down to the nearest power of 2.
// REQUIRES: |n > 0|.
uint32_t
pow2_rd_u32(uint32_t n);

// Return the most-significant bit position for |n|.
// REQUIRES: |n > 0|.
uint32_t
msb_idx_u32(uint32_t n);  // 0-based bit position

//
//
//

#ifdef __cplusplus
}
#endif

//
//
//

#endif  // SRC_GRAPHICS_LIB_COMPUTE_COMMON_UTIL_H_
