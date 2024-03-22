// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_TARGETS_VENDORS_AMD_GCN3_U64_CONFIG_H_
#define SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_TARGETS_VENDORS_AMD_GCN3_U64_CONFIG_H_

//
//
//

// clang-format off
#define RS_KEYVAL_DWORDS                   2

#define RS_FILL_WORKGROUP_SIZE_LOG2        7
#define RS_FILL_BLOCK_ROWS                 8

#define RS_HISTOGRAM_WORKGROUP_SIZE_LOG2   8
#define RS_HISTOGRAM_SUBGROUP_SIZE_LOG2    6
#define RS_HISTOGRAM_BLOCK_ROWS            14

#define RS_PREFIX_WORKGROUP_SIZE_LOG2      8
#define RS_PREFIX_SUBGROUP_SIZE_LOG2       6

#define RS_SCATTER_WORKGROUP_SIZE_LOG2     8
#define RS_SCATTER_SUBGROUP_SIZE_LOG2      6
#define RS_SCATTER_BLOCK_ROWS              14
// clang-format on

//
//
//

#endif  // SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_TARGETS_VENDORS_AMD_GCN3_U64_CONFIG_H_
