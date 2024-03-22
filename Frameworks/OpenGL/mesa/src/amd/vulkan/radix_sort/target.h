// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_TARGET_H_
#define SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_TARGET_H_

//
//
//

#include <stdint.h>

//
// This structure packages target-specific configuration parameters.
//

struct radix_sort_vk_target_config
{
  uint32_t keyval_dwords;

  struct
  {
    uint32_t workgroup_size_log2;
  } init;

  struct
  {
    uint32_t workgroup_size_log2;
  } fill;

  struct
  {
    uint32_t workgroup_size_log2;
    uint32_t subgroup_size_log2;
    uint32_t block_rows;
  } histogram;

  struct
  {
    uint32_t workgroup_size_log2;
    uint32_t subgroup_size_log2;
  } prefix;

  struct
  {
    uint32_t workgroup_size_log2;
    uint32_t subgroup_size_log2;
    uint32_t block_rows;
  } scatter;
};

//
//
//

#endif  // SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_TARGET_H_
