// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_SHADERS_PUSH_H_
#define SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_SHADERS_PUSH_H_

//
// There is a limit to the maximum number of keyvals that can be sorted because
// the top 2 bits in the atomic lookback counters are used as tag bits.
//
#define RS_MAX_KEYVALS ((1 << 30) - 1)

//
// Right now, the entire implementation is very much dependent on an 8-bit radix
// size.  Most of the shaders attempt to honor this defined size but there are
// still a number of places where 256 is assumed.
//
#define RS_RADIX_LOG2 8
#define RS_RADIX_SIZE (1 << RS_RADIX_LOG2)

//
// LOOKBACK STATUS FLAGS
//
// The decoupled lookback status flags are stored in the two
// high bits of the count:
//
//   0                                   31
//   | REDUCTION OR PREFIX COUNT | STATUS |
//   +---------------------------+--------+
//   |             30            |    2   |
//
// This limits the keyval extent size to (2^30-1).
//
// Valid status flags are:
//
//   EVEN PASS                 ODD PASS
//   -----------------------   -----------------------
//   0 : invalid               0 : prefix available
//   1 : reduction available   1 : ---
//   2 : prefix available      2 : invalid
//   3 : ---                   3 : reduction available
//
// Atomically adding +1 to a "reduction available" status results in a "prefix
// available" status.
//
// clang-format off
#define RS_PARTITION_STATUS_EVEN_INVALID    (0u)
#define RS_PARTITION_STATUS_EVEN_REDUCTION  (1u)
#define RS_PARTITION_STATUS_EVEN_PREFIX     (2u)

#define RS_PARTITION_STATUS_ODD_INVALID     (2u)
#define RS_PARTITION_STATUS_ODD_REDUCTION   (3u)
#define RS_PARTITION_STATUS_ODD_PREFIX      (0u)
// clang-format on

//
// Arguments to indirectly launched shaders.
//
//   struct rs_indirect_info_dispatch
//   {
//     u32vec4 pad;
//     u32vec4 zero;
//     u32vec4 histogram;
//     u32vec4 scatter;
//   };
//
//   struct rs_indirect_info_fill
//   {
//     uint32_t block_offset;
//     uint32_t dword_offset_min;
//     uint32_t dword_offset_max_minus_min;
//     uint32_t reserved; // padding for 16 bytes
//   };
//
//   struct rs_indirect_info
//   {
//     rs_indirect_info_fill     pad;
//     rs_indirect_info_fill     zero;
//     rs_indirect_info_dispatch dispatch;
//   };
//
#define RS_STRUCT_INDIRECT_INFO_DISPATCH()                                                         \
  struct rs_indirect_info_dispatch                                                                 \
  {                                                                                                \
    RS_STRUCT_MEMBER_STRUCT(u32vec4, pad)                                                          \
    RS_STRUCT_MEMBER_STRUCT(u32vec4, zero)                                                         \
    RS_STRUCT_MEMBER_STRUCT(u32vec4, histogram)                                                    \
    RS_STRUCT_MEMBER_STRUCT(u32vec4, scatter)                                                      \
  }

#define RS_STRUCT_INDIRECT_INFO_FILL()                                                             \
  struct rs_indirect_info_fill                                                                     \
  {                                                                                                \
    RS_STRUCT_MEMBER(uint32_t, block_offset)                                                       \
    RS_STRUCT_MEMBER(uint32_t, dword_offset_min)                                                   \
    RS_STRUCT_MEMBER(uint32_t, dword_offset_max_minus_min)                                         \
    RS_STRUCT_MEMBER(uint32_t, reserved)                                                           \
  }

#define RS_STRUCT_INDIRECT_INFO()                                                                  \
  RS_STRUCT_INDIRECT_INFO_DISPATCH();                                                              \
  RS_STRUCT_INDIRECT_INFO_FILL();                                                                  \
  struct rs_indirect_info                                                                          \
  {                                                                                                \
    RS_STRUCT_MEMBER_STRUCT(rs_indirect_info_fill, pad)                                            \
    RS_STRUCT_MEMBER_STRUCT(rs_indirect_info_fill, zero)                                           \
    RS_STRUCT_MEMBER_STRUCT(rs_indirect_info_dispatch, dispatch)                                   \
  }

//
// Define the push constant structures shared by the host and device.
//
//   INIT
//   ----
//   struct rs_push_init
//   {
//     uint64_t devaddr_count;         // address of count buffer
//     uint64_t devaddr_indirect;      // address of indirect info buffer
//   };
//
//   FILL
//   ----
//   struct rs_push_fill
//   {
//     uint64_t devaddr_info;          // address of indirect info for fill shader
//     uint64_t devaddr_dwords;        // address of dwords extent
//     uint32_t dword;                 // dword value used to fill the dwords extent
//   };
//
//   HISTOGRAM
//   ---------
//   struct rs_push_histogram
//   {
//     uint64_t devaddr_histograms;    // address of histograms extent
//     uint64_t devaddr_keyvals;       // address of keyvals extent
//     uint32_t passes;                // number of passes
//   };
//
//   PREFIX
//   ------
//   struct rs_push_prefix
//   {
//     uint64_t devaddr_histograms;    // address of histograms extent
//   };
//
//   SCATTER
//   -------
//   struct rs_push_scatter
//   {
//     uint64_t devaddr_keyvals_in;    // address of input keyvals
//     uint64_t devaddr_keyvals_out;   // address of output keyvals
//     uint64_t devaddr_partitions     // address of partitions
//     uint64_t devaddr_histogram;     // address of pass histogram
//     uint32_t pass_offset;           // keyval pass offset
//   };
//
#define RS_STRUCT_PUSH_INIT()                                                                      \
  struct rs_push_init                                                                              \
  {                                                                                                \
    RS_STRUCT_MEMBER(RS_DEVADDR, devaddr_info)                                                     \
    RS_STRUCT_MEMBER(RS_DEVADDR, devaddr_count)                                                    \
    RS_STRUCT_MEMBER(uint32_t, passes)                                                             \
  }

#define RS_STRUCT_PUSH_FILL()                                                                      \
  struct rs_push_fill                                                                              \
  {                                                                                                \
    RS_STRUCT_MEMBER(RS_DEVADDR, devaddr_info)                                                     \
    RS_STRUCT_MEMBER(RS_DEVADDR, devaddr_dwords)                                                   \
    RS_STRUCT_MEMBER(uint32_t, dword)                                                              \
  }

#define RS_STRUCT_PUSH_HISTOGRAM()                                                                 \
  struct rs_push_histogram                                                                         \
  {                                                                                                \
    RS_STRUCT_MEMBER(RS_DEVADDR, devaddr_histograms)                                               \
    RS_STRUCT_MEMBER(RS_DEVADDR, devaddr_keyvals)                                                  \
    RS_STRUCT_MEMBER(uint32_t, passes)                                                             \
  }

#define RS_STRUCT_PUSH_PREFIX()                                                                    \
  struct rs_push_prefix                                                                            \
  {                                                                                                \
    RS_STRUCT_MEMBER(RS_DEVADDR, devaddr_histograms)                                               \
  }

#define RS_STRUCT_PUSH_SCATTER()                                                                   \
  struct rs_push_scatter                                                                           \
  {                                                                                                \
    RS_STRUCT_MEMBER(RS_DEVADDR, devaddr_keyvals_even)                                             \
    RS_STRUCT_MEMBER(RS_DEVADDR, devaddr_keyvals_odd)                                              \
    RS_STRUCT_MEMBER(RS_DEVADDR, devaddr_partitions)                                               \
    RS_STRUCT_MEMBER(RS_DEVADDR, devaddr_histograms)                                               \
    RS_STRUCT_MEMBER(uint32_t, pass_offset)                                                        \
  }

////////////////////////////////////////////////////////////////////
//
// GLSL
//
#ifdef VULKAN  // defined by GLSL/VK compiler

// clang-format off
#define RS_STRUCT_MEMBER(type_, name_)               type_ name_;
#define RS_STRUCT_MEMBER_FARRAY(type_, len_, name_)  type_ name_[len_];
#define RS_STRUCT_MEMBER_STRUCT(type_, name_)        type_ name_;
// clang-format on

////////////////////////////////////////////////////////////////////
//
// C/C++
//
#else

#ifdef __cplusplus
extern "C" {
#endif

//
//
//

#include <stdint.h>

struct u32vec4
{
  uint32_t x;
  uint32_t y;
  uint32_t z;
  uint32_t w;
};

// clang-format off
#define RS_DEVADDR                                   uint64_t
#define RS_STRUCT_MEMBER(type_, name_)               type_ name_;
#define RS_STRUCT_MEMBER_FARRAY(type_, len_, name_)  type_ name_[len_];
#define RS_STRUCT_MEMBER_STRUCT(type_, name_)        struct type_ name_;
// clang-format on

RS_STRUCT_PUSH_INIT();
RS_STRUCT_PUSH_FILL();
RS_STRUCT_PUSH_HISTOGRAM();
RS_STRUCT_PUSH_PREFIX();
RS_STRUCT_PUSH_SCATTER();

RS_STRUCT_INDIRECT_INFO();

//
//
//

#ifdef __cplusplus
}
#endif

#endif

//
//
//

#endif  // SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_SHADERS_PUSH_H_
