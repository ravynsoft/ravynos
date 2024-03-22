// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_SHADERS_PREFIX_LIMITS_H_
#define SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_SHADERS_PREFIX_LIMITS_H_

//
// Define various prefix limits
//
#define RS_PREFIX_LIMITS

//
// Multi-subgroup prefix requires shared memory.
//
#if (RS_WORKGROUP_SUBGROUPS > 1)

// clang-format off
#define RS_H_COMPONENTS    (RS_RADIX_SIZE / RS_WORKGROUP_SIZE)

#define RS_SWEEP_0_SIZE    (RS_RADIX_SIZE   / RS_SUBGROUP_SIZE)
#define RS_SWEEP_1_SIZE    (RS_SWEEP_0_SIZE / RS_SUBGROUP_SIZE)
#define RS_SWEEP_2_SIZE    (RS_SWEEP_1_SIZE / RS_SUBGROUP_SIZE)

#define RS_SWEEP_SIZE      (RS_SWEEP_0_SIZE + RS_SWEEP_1_SIZE + RS_SWEEP_2_SIZE)

#define RS_S0_PASSES       (RS_SWEEP_0_SIZE / RS_WORKGROUP_SIZE)
#define RS_S1_PASSES       (RS_SWEEP_1_SIZE / RS_WORKGROUP_SIZE)

#define RS_SWEEP_0_OFFSET  0
#define RS_SWEEP_1_OFFSET  (RS_SWEEP_0_OFFSET + RS_SWEEP_0_SIZE)
#define RS_SWEEP_2_OFFSET  (RS_SWEEP_1_OFFSET + RS_SWEEP_1_SIZE)
// clang-format on

//
// Single subgroup prefix doesn't use shared memory.
//
#else

#define RS_SWEEP_SIZE 0

#endif

//
//
//

#endif  // SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_SHADERS_PREFIX_LIMITS_H_
