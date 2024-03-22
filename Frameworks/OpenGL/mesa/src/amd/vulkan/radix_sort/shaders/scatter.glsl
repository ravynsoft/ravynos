// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// #pragma use_vulkan_memory_model  // results in spirv-remap validation error

//
// Each "pass" scatters the keyvals to their new destinations.
//
// clang-format off
#extension GL_EXT_control_flow_attributes          : require
#extension GL_KHR_shader_subgroup_basic            : require
#extension GL_KHR_shader_subgroup_arithmetic       : require
#extension GL_KHR_memory_scope_semantics           : require
#extension GL_KHR_shader_subgroup_ballot           : require
// clang-format on

//
// Load arch/keyval configuration
//
#include "config.h"

//
// Optional switches:
//
//   #define RS_SCATTER_DISABLE_REORDER
//   #define RS_SCATTER_ENABLE_BITFIELD_EXTRACT
//   #define RS_SCATTER_ENABLE_NV_MATCH
//   #define RS_SCATTER_ENABLE_BROADCAST_MATCH
//   #define RS_SCATTER_DISABLE_COMPONENTS_IN_REGISTERS
//

//
// Use NVIDIA Turing/Volta+ partitioning operator (`match_any()`)?
//
#ifdef RS_SCATTER_ENABLE_NV_MATCH
#extension GL_NV_shader_subgroup_partitioned : require
#endif

//
// Store prefix intermediates in registers?
//
#ifdef RS_SCATTER_DISABLE_COMPONENTS_IN_REGISTERS
#define RS_PREFIX_DISABLE_COMPONENTS_IN_REGISTERS
#endif

//
// Buffer reference macros and push constants
//
#include "bufref.h"
#include "push.h"

//
// Push constants for scatter shader
//
RS_STRUCT_PUSH_SCATTER();

layout(push_constant) uniform block_push
{
  rs_push_scatter push;
};

//
// Subgroup uniform support
//
#if defined(RS_SCATTER_SUBGROUP_UNIFORM_DISABLE) && defined(GL_EXT_subgroupuniform_qualifier)
#extension GL_EXT_subgroupuniform_qualifier : required
#define RS_SUBGROUP_UNIFORM subgroupuniformEXT
#else
#define RS_SUBGROUP_UNIFORM
#endif

//
// Check all mandatory switches are defined
//

// What's the size of the keyval?
#ifndef RS_KEYVAL_DWORDS
#error "Undefined: RS_KEYVAL_DWORDS"
#endif

// Which keyval dword does this shader bitfieldExtract() bits?
#ifndef RS_SCATTER_KEYVAL_DWORD_BASE
#error "Undefined: RS_SCATTER_KEYVAL_DWORD_BASE"
#endif

//
#ifndef RS_SCATTER_BLOCK_ROWS
#error "Undefined: RS_SCATTER_BLOCK_ROWS"
#endif

//
#ifndef RS_SCATTER_SUBGROUP_SIZE_LOG2
#error "Undefined: RS_SCATTER_SUBGROUP_SIZE_LOG2"
#endif

//
#ifndef RS_SCATTER_WORKGROUP_SIZE_LOG2
#error "Undefined: RS_SCATTER_WORKGROUP_SIZE_LOG2"
#endif

//
// Status masks are defined differently for the scatter_even and
// scatter_odd shaders.
//
#ifndef RS_PARTITION_STATUS_INVALID
#error "Undefined: RS_PARTITION_STATUS_INVALID"
#endif

#ifndef RS_PARTITION_STATUS_REDUCTION
#error "Undefined: RS_PARTITION_STATUS_REDUCTION"
#endif

#ifndef RS_PARTITION_STATUS_PREFIX
#error "Undefined: RS_PARTITION_STATUS_PREFIX"
#endif

//
// Assumes (RS_RADIX_LOG2 == 8)
//
// Error if this ever changes!
//
#if (RS_RADIX_LOG2 != 8)
#error "Error: (RS_RADIX_LOG2 != 8)"
#endif

//
// Masks are different for scatter_even/odd.
//
// clang-format off
#define RS_PARTITION_MASK_INVALID    (RS_PARTITION_STATUS_INVALID   << 30)
#define RS_PARTITION_MASK_REDUCTION  (RS_PARTITION_STATUS_REDUCTION << 30)
#define RS_PARTITION_MASK_PREFIX     (RS_PARTITION_STATUS_PREFIX    << 30)
#define RS_PARTITION_MASK_STATUS     0xC0000000
#define RS_PARTITION_MASK_COUNT      0x3FFFFFFF
// clang-format on

//
// Local macros
//
// clang-format off
#define RS_KEYVAL_SIZE               (RS_KEYVAL_DWORDS * 4)
#define RS_WORKGROUP_SIZE            (1 << RS_SCATTER_WORKGROUP_SIZE_LOG2)
#define RS_SUBGROUP_SIZE             (1 << RS_SCATTER_SUBGROUP_SIZE_LOG2)
#define RS_WORKGROUP_SUBGROUPS       (RS_WORKGROUP_SIZE / RS_SUBGROUP_SIZE)
#define RS_SUBGROUP_KEYVALS          (RS_SCATTER_BLOCK_ROWS * RS_SUBGROUP_SIZE)
#define RS_BLOCK_KEYVALS             (RS_SCATTER_BLOCK_ROWS * RS_WORKGROUP_SIZE)
#define RS_RADIX_MASK                ((1 << RS_RADIX_LOG2) - 1)
// clang-format on

//
// Validate number of keyvals fit in a uint16_t.
//
#if (RS_BLOCK_KEYVALS >= 65536)
#error "Error: (RS_BLOCK_KEYVALS >= 65536)"
#endif

//
// Keyval type
//
#if (RS_KEYVAL_DWORDS == 1)
#define RS_KEYVAL_TYPE uint32_t
#elif (RS_KEYVAL_DWORDS == 2)
#define RS_KEYVAL_TYPE u32vec2
#else
#error "Error: Unsupported RS_KEYVAL_DWORDS"
#endif

//
// Set up match mask
//
#if (RS_SUBGROUP_SIZE <= 32)
#if (RS_SUBGROUP_SIZE == 32)
#define RS_SUBGROUP_MASK 0xFFFFFFFF
#else
#define RS_SUBGROUP_MASK ((1 << RS_SUBGROUP_SIZE) - 1)
#endif
#endif

//
// Determine at compile time the base of the final iteration for
// workgroups smaller than RS_RADIX_SIZE.
//
#if (RS_WORKGROUP_SIZE < RS_RADIX_SIZE)
#define RS_WORKGROUP_BASE_FINAL ((RS_RADIX_SIZE / RS_WORKGROUP_SIZE) * RS_WORKGROUP_SIZE)
#endif

//
// Max macro
//
#define RS_MAX_2(a_, b_) (((a_) >= (b_)) ? (a_) : (b_))

//
// Select a keyval dword
//
#if (RS_KEYVAL_DWORDS == 1)
#define RS_KV_DWORD(kv_, dword_) (kv_)
#else
#define RS_KV_DWORD(kv_, dword_) (kv_)[dword_]
#endif

//
// Is bitfield extract faster?
//
#ifdef RS_SCATTER_ENABLE_BITFIELD_EXTRACT
//----------------------------------------------------------------------
//
// Test a bit in a radix digit
//
#define RS_BIT_IS_ONE(val_, bit_) (bitfieldExtract(val_, bit_, 1) != 0)

//
// Extract a keyval digit
//
#if (RS_KEYVAL_DWORDS == 1)
#define RS_KV_EXTRACT_DIGIT(kv_) bitfieldExtract(kv_, int32_t(push.pass_offset), RS_RADIX_LOG2)
#else
#define RS_KV_EXTRACT_DIGIT(kv_)                                                                   \
  bitfieldExtract(kv_[RS_SCATTER_KEYVAL_DWORD_BASE], int32_t(push.pass_offset), RS_RADIX_LOG2)
#endif
//----------------------------------------------------------------------
#else
//----------------------------------------------------------------------
//
// Test a bit in a radix digit
//
#define RS_BIT_IS_ONE(val_, bit_) (((val_) & (1 << (bit_))) != 0)

//
// Extract a keyval digit
//
#if (RS_KEYVAL_DWORDS == 1)
#define RS_KV_EXTRACT_DIGIT(kv_) ((kv_ >> push.pass_offset) & RS_RADIX_MASK)
#else
#define RS_KV_EXTRACT_DIGIT(kv_)                                                                   \
  ((kv_[RS_SCATTER_KEYVAL_DWORD_BASE] >> push.pass_offset) & RS_RADIX_MASK)
#endif
//----------------------------------------------------------------------
#endif

//
// Load prefix limits before loading prefix function and before
// calculating SMEM limits.
//
#include "prefix_limits.h"

//
// - The lookback span is RS_RADIX_SIZE dwords and overwrites the
//   ballots span.
//
// - The histogram span is RS_RADIX_SIZE dwords
//
// - The keyvals span is at least one dword per keyval in the
//   workgroup.  This span overwrites anything past the lookback
//   radix span.
//
// Shared memory map phase 1:
//
//   < LOOKBACK > < HISTOGRAM > < PREFIX > ...
//
// Shared memory map phase 3:
//
//   < LOOKBACK > < REORDER > ...
//
// FIXME(allanmac): Create a spreadsheet showing the exact shared
// memory footprint (RS_SMEM_DWORDS) for a configuration.
//
//            | Dwords                                    | Bytes
//  ----------+-------------------------------------------+--------
//  Lookback  | 256                                       | 1 KB
//  Histogram | 256                                       | 1 KB
//  Prefix    | 4-84                                      | 16-336
//  Reorder   | RS_WORKGROUP_SIZE * RS_SCATTER_BLOCK_ROWS | 2-8 KB
//
// clang-format off
#define RS_SMEM_LOOKBACK_SIZE     RS_RADIX_SIZE
#define RS_SMEM_HISTOGRAM_SIZE    RS_RADIX_SIZE
#define RS_SMEM_REORDER_SIZE      (RS_SCATTER_BLOCK_ROWS * RS_WORKGROUP_SIZE)

#define RS_SMEM_DWORDS_PHASE_1    (RS_SMEM_LOOKBACK_SIZE + RS_SMEM_HISTOGRAM_SIZE + RS_SWEEP_SIZE)
#define RS_SMEM_DWORDS_PHASE_2    (RS_SMEM_LOOKBACK_SIZE + RS_SMEM_REORDER_SIZE)

#define RS_SMEM_DWORDS            RS_MAX_2(RS_SMEM_DWORDS_PHASE_1, RS_SMEM_DWORDS_PHASE_2)

#define RS_SMEM_LOOKBACK_OFFSET   0
#define RS_SMEM_HISTOGRAM_OFFSET  (RS_SMEM_LOOKBACK_OFFSET  + RS_SMEM_LOOKBACK_SIZE)
#define RS_SMEM_PREFIX_OFFSET     (RS_SMEM_HISTOGRAM_OFFSET + RS_SMEM_HISTOGRAM_SIZE)
#define RS_SMEM_REORDER_OFFSET    (RS_SMEM_LOOKBACK_OFFSET  + RS_SMEM_LOOKBACK_SIZE)
// clang-format on

//
//
//
layout(local_size_x = RS_WORKGROUP_SIZE) in;

//
//
//
layout(buffer_reference, std430) buffer buffer_rs_kv
{
  RS_KEYVAL_TYPE extent[];
};

layout(buffer_reference, std430) buffer buffer_rs_histogram  // single histogram
{
  uint32_t extent[];
};

layout(buffer_reference, std430) buffer buffer_rs_partitions
{
  uint32_t extent[];
};

//
// Declare shared memory
//
struct rs_scatter_smem
{
  uint32_t extent[RS_SMEM_DWORDS];
};

shared rs_scatter_smem smem;

//
// The shared memory barrier is either subgroup-wide or
// workgroup-wide.
//
#if (RS_WORKGROUP_SUBGROUPS == 1)
#define RS_BARRIER() subgroupBarrier()
#else
#define RS_BARRIER() barrier()
#endif

//
// If multi-subgroup then define shared memory
//
#if (RS_WORKGROUP_SUBGROUPS > 1)

//----------------------------------------
#define RS_PREFIX_SWEEP0(idx_) smem.extent[RS_SMEM_PREFIX_OFFSET + RS_SWEEP_0_OFFSET + (idx_)]
//----------------------------------------

#if (RS_SWEEP_1_SIZE > 0)
//----------------------------------------
#define RS_PREFIX_SWEEP1(idx_) smem.extent[RS_SMEM_PREFIX_OFFSET + RS_SWEEP_1_OFFSET + (idx_)]
//----------------------------------------
#endif

#if (RS_SWEEP_2_SIZE > 0)
//----------------------------------------
#define RS_PREFIX_SWEEP2(idx_) smem.extent[RS_SMEM_PREFIX_OFFSET + RS_SWEEP_2_OFFSET + (idx_)]
//----------------------------------------
#endif

#endif

//
// Define prefix load/store functions
//
// clang-format off
#if (RS_WORKGROUP_SUBGROUPS == 1)
#define RS_PREFIX_LOAD(idx_)   smem.extent[RS_SMEM_HISTOGRAM_OFFSET + gl_SubgroupInvocationID + (idx_)]
#define RS_PREFIX_STORE(idx_)  smem.extent[RS_SMEM_HISTOGRAM_OFFSET + gl_SubgroupInvocationID + (idx_)]
#else
#define RS_PREFIX_LOAD(idx_)   smem.extent[RS_SMEM_HISTOGRAM_OFFSET + gl_LocalInvocationID.x + (idx_)]
#define RS_PREFIX_STORE(idx_)  smem.extent[RS_SMEM_HISTOGRAM_OFFSET + gl_LocalInvocationID.x + (idx_)]
#endif
// clang-format on

//
// Load the prefix function
//
// The prefix function operates on shared memory so there are no
// arguments.
//
#define RS_PREFIX_ARGS  // EMPTY

#include "prefix.h"

//
// Zero the SMEM histogram
//
void
rs_histogram_zero()
{
#if (RS_WORKGROUP_SUBGROUPS == 1)

  const uint32_t smem_offset = RS_SMEM_HISTOGRAM_OFFSET + gl_SubgroupInvocationID;

  [[unroll]] for (uint32_t ii = 0; ii < RS_RADIX_SIZE; ii += RS_SUBGROUP_SIZE)
  {
    smem.extent[smem_offset + ii] = 0;
  }

#elif (RS_WORKGROUP_SIZE < RS_RADIX_SIZE)

  const uint32_t smem_offset = RS_SMEM_HISTOGRAM_OFFSET + gl_LocalInvocationID.x;

  [[unroll]] for (uint32_t ii = 0; ii < RS_RADIX_SIZE; ii += RS_WORKGROUP_SIZE)
  {
    smem.extent[smem_offset + ii] = 0;
  }

#if (RS_WORKGROUP_BASE_FINAL < RS_RADIX_SIZE)
  const uint32_t smem_offset_final = smem_offset + RS_WORKGROUP_BASE_FINAL;

  if (smem_offset_final < RS_RADIX_SIZE)
    {
      smem.histogram[smem_offset_final] = 0;
    }
#endif

#elif (RS_WORKGROUP_SIZE >= RS_RADIX_SIZE)

#if (RS_WORKGROUP_SIZE > RS_RADIX_SIZE)
  if (gl_LocalInvocationID.x < RS_RADIX_SIZE)
#endif
    {
      smem.extent[RS_SMEM_HISTOGRAM_OFFSET + gl_LocalInvocationID.x] = 0;
    }

#endif

  RS_BARRIER();
}

//
// Perform a workgroup-wide match operation that computes both a
// workgroup-wide index for each keyval and a workgroup-wide
// histogram.
//
// FIXME(allanmac): Special case (RS_WORKGROUP_SUBGROUPS==1)
//
void
rs_histogram_rank(const RS_KEYVAL_TYPE kv[RS_SCATTER_BLOCK_ROWS],
                  out uint32_t         kr[RS_SCATTER_BLOCK_ROWS])
{
  // clang-format off
#define RS_HISTOGRAM_LOAD(digit_)          smem.extent[RS_SMEM_HISTOGRAM_OFFSET + (digit_)]
#define RS_HISTOGRAM_STORE(digit_, count_) smem.extent[RS_SMEM_HISTOGRAM_OFFSET + (digit_)] = (count_)
  // clang-format on

  //----------------------------------------------------------------------
  //
  // Use the Volta/Turing `match.sync` instruction.
  //
  // Note that performance is quite poor and the break-even for
  // `match.sync` requires more bits.
  //
  //----------------------------------------------------------------------
#ifdef RS_SCATTER_ENABLE_NV_MATCH

  //
  // 32
  //
#if (RS_SUBGROUP_SIZE == 32)

  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    //
    // NOTE(allanmac): Unfortunately there is no `match.any.sync.b8`
    //
    // TODO(allanmac): Consider using the `atomicOr()` match approach
    // described by Adinets since Volta/Turing have extremely fast
    // atomic smem operations.
    //
    const uint32_t digit = RS_KV_EXTRACT_DIGIT(kv[ii]);
    const uint32_t match = subgroupPartitionNV(digit).x;

    kr[ii] = (bitCount(match) << 16) | bitCount(match & gl_SubgroupLeMask.x);
  }

  //
  // Undefined!
  //
#else
#error "Error: rs_histogram_rank() undefined for subgroup size"
#endif

  //----------------------------------------------------------------------
  //
  // Default is to emulate a `match` operation with ballots.
  //
  //----------------------------------------------------------------------
#elif !defined(RS_SCATTER_ENABLE_BROADCAST_MATCH)

  //
  // 64
  //
#if (RS_SUBGROUP_SIZE == 64)

  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    const uint32_t digit = RS_KV_EXTRACT_DIGIT(kv[ii]);

    u32vec2 match;

    {
      const bool     is_one = RS_BIT_IS_ONE(digit, 0);
      const u32vec2  ballot = subgroupBallot(is_one).xy;
      const uint32_t mask   = is_one ? 0 : 0xFFFFFFFF;

      match.x = (ballot.x ^ mask);
      match.y = (ballot.y ^ mask);
    }

    [[unroll]] for (int32_t bit = 1; bit < RS_RADIX_LOG2; bit++)
    {
      const bool     is_one = RS_BIT_IS_ONE(digit, bit);
      const u32vec2  ballot = subgroupBallot(is_one).xy;
      const uint32_t mask   = is_one ? 0 : 0xFFFFFFFF;

      match.x &= (ballot.x ^ mask);
      match.y &= (ballot.y ^ mask);
    }

    kr[ii] = ((bitCount(match.x) + bitCount(match.y)) << 16) |
             (bitCount(match.x & gl_SubgroupLeMask.x) +  //
              bitCount(match.y & gl_SubgroupLeMask.y));
  }

  //
  // <= 32
  //
#elif ((RS_SUBGROUP_SIZE <= 32) && !defined(RS_SCATTER_ENABLE_NV_MATCH))

  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    const uint32_t digit = RS_KV_EXTRACT_DIGIT(kv[ii]);

    uint32_t match;

    {
      const bool     is_one = RS_BIT_IS_ONE(digit, 0);
      const uint32_t ballot = subgroupBallot(is_one).x;
      const uint32_t mask   = is_one ? 0 : RS_SUBGROUP_MASK;

      match = (ballot ^ mask);
    }

    [[unroll]] for (int32_t bit = 1; bit < RS_RADIX_LOG2; bit++)
    {
      const bool     is_one = RS_BIT_IS_ONE(digit, bit);
      const uint32_t ballot = subgroupBallot(is_one).x;
      const uint32_t mask   = is_one ? 0 : RS_SUBGROUP_MASK;

      match &= (ballot ^ mask);
    }

    kr[ii] = (bitCount(match) << 16) | bitCount(match & gl_SubgroupLeMask.x);
  }

  //
  // Undefined!
  //
#else
#error "Error: rs_histogram_rank() undefined for subgroup size"
#endif

  //----------------------------------------------------------------------
  //
  // Emulate a `match` operation with broadcasts.
  //
  // In general, using broadcasts is a win for narrow subgroups.
  //
  //----------------------------------------------------------------------
#else

  //
  // 64
  //
#if (RS_SUBGROUP_SIZE == 64)

  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    const uint32_t digit = RS_KV_EXTRACT_DIGIT(kv[ii]);

    u32vec2 match;

    // subgroup invocation 0
    {
      match[0] = (subgroupBroadcast(digit, 0) == digit) ? (1u << 0) : 0;
    }

    // subgroup invocations 1-31
    [[unroll]] for (int32_t jj = 1; jj < 32; jj++)
    {
      match[0] |= (subgroupBroadcast(digit, jj) == digit) ? (1u << jj) : 0;
    }

    // subgroup invocation 32
    {
      match[1] = (subgroupBroadcast(digit, 32) == digit) ? (1u << 0) : 0;
    }

    // subgroup invocations 33-63
    [[unroll]] for (int32_t jj = 1; jj < 32; jj++)
    {
      match[1] |= (subgroupBroadcast(digit, jj) == digit) ? (1u << jj) : 0;
    }

    kr[ii] = ((bitCount(match.x) + bitCount(match.y)) << 16) |
             (bitCount(match.x & gl_SubgroupLeMask.x) +  //
              bitCount(match.y & gl_SubgroupLeMask.y));
  }

  //
  // <= 32
  //
#elif ((RS_SUBGROUP_SIZE <= 32) && !defined(RS_SCATTER_ENABLE_NV_MATCH))

  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    const uint32_t digit = RS_KV_EXTRACT_DIGIT(kv[ii]);

    // subgroup invocation 0
    uint32_t match = (subgroupBroadcast(digit, 0) == digit) ? (1u << 0) : 0;

    // subgroup invocations 1-(RS_SUBGROUP_SIZE-1)
    [[unroll]] for (int32_t jj = 1; jj < RS_SUBGROUP_SIZE; jj++)
    {
      match |= (subgroupBroadcast(digit, jj) == digit) ? (1u << jj) : 0;
    }

    kr[ii] = (bitCount(match) << 16) | bitCount(match & gl_SubgroupLeMask.x);
  }

  //
  // Undefined!
  //
#else
#error "Error: rs_histogram_rank() undefined for subgroup size"
#endif

#endif

  //
  // This is a little unconventional but cycling through a subgroup at
  // a time is a performance win on the tested architectures.
  //
  for (uint32_t ii = 0; ii < RS_WORKGROUP_SUBGROUPS; ii++)
    {
      if (gl_SubgroupID == ii)
        {
          [[unroll]] for (uint32_t jj = 0; jj < RS_SCATTER_BLOCK_ROWS; jj++)
          {
            const uint32_t digit = RS_KV_EXTRACT_DIGIT(kv[jj]);
            const uint32_t prev  = RS_HISTOGRAM_LOAD(digit);
            const uint32_t rank  = kr[jj] & 0xFFFF;
            const uint32_t count = kr[jj] >> 16;

            kr[jj] = prev + rank;

            if (rank == count)
              {
                RS_HISTOGRAM_STORE(digit, (prev + count));
              }

            subgroupMemoryBarrierShared();
          }
        }

      RS_BARRIER();
    }
}

//
// Other partitions may lookback on this partition.
//
// Load the global exclusive prefix and for each subgroup
// store the exclusive prefix to shared memory and store the
// final inclusive prefix to global memory.
//
void
rs_first_prefix_store(restrict buffer_rs_partitions rs_partitions)
{
  //
  // Define the histogram reference
  //
#if (RS_WORKGROUP_SUBGROUPS == 1)
  const uint32_t hist_offset = gl_SubgroupInvocationID * 4;
#else
  const uint32_t hist_offset = gl_LocalInvocationID.x * 4;
#endif

  readonly RS_BUFREF_DEFINE_AT_OFFSET_UINT32(buffer_rs_histogram,
                                             rs_histogram,
                                             push.devaddr_histograms,
                                             hist_offset);

#if (RS_WORKGROUP_SUBGROUPS == 1)
  ////////////////////////////////////////////////////////////////////////////
  //
  // (RS_WORKGROUP_SUBGROUPS == 1)
  //
  const uint32_t smem_offset_h = RS_SMEM_HISTOGRAM_OFFSET + gl_SubgroupInvocationID;
  const uint32_t smem_offset_l = RS_SMEM_LOOKBACK_OFFSET + gl_SubgroupInvocationID;

  [[unroll]] for (uint32_t ii = 0; ii < RS_RADIX_SIZE; ii += RS_SUBGROUP_SIZE)
  {
    const uint32_t exc = rs_histogram.extent[ii];
    const uint32_t red = smem.extent[smem_offset_h + ii];

    smem.extent[smem_offset_l + ii] = exc;

    const uint32_t inc = exc + red;

    atomicStore(rs_partitions.extent[ii],
                inc | RS_PARTITION_MASK_PREFIX,
                gl_ScopeQueueFamily,
                gl_StorageSemanticsBuffer,
                gl_SemanticsRelease);
  }

#elif (RS_WORKGROUP_SIZE < RS_RADIX_SIZE)
  ////////////////////////////////////////////////////////////////////////////
  //
  // (RS_WORKGROUP_SIZE < RS_RADIX_SIZE)
  //
  const uint32_t smem_offset_h = RS_SMEM_HISTOGRAM_OFFSET + gl_LocalInvocationID.x;
  const uint32_t smem_offset_l = RS_SMEM_LOOKBACK_OFFSET + gl_LocalInvocationID.x;

  [[unroll]] for (uint32_t ii = 0; ii < RS_RADIX_SIZE; ii += RS_WORKGROUP_SIZE)
  {
    const uint32_t exc = rs_histogram.extent[ii];
    const uint32_t red = smem.extent[smem_offset_h + ii];

    smem.extent[smem_offset_l + ii] = exc;

    const uint32_t inc = exc + red;

    atomicStore(rs_partitions.extent[ii],
                inc | RS_PARTITION_MASK_PREFIX,
                gl_ScopeQueueFamily,
                gl_StorageSemanticsBuffer,
                gl_SemanticsRelease);
  }

#if (RS_WORKGROUP_BASE_FINAL < RS_RADIX_SIZE)
  const uint32_t smem_offset_final_h = smem_offset_h + RS_WORKGROUP_BASE_FINAL;
  const uint32_t smem_offset_final_l = smem_offset_l + RS_WORKGROUP_BASE_FINAL;

  if (smem_offset_final < RS_RADIX_SIZE)
    {
      const uint32_t exc = rs_histogram.extent[RS_WORKGROUP_BASE_FINAL];
      const uint32_t red = smem.extent[smem_offset_final_h];

      smem.extent[smem_offset_final_l] = exc;

      const uint32_t inc = exc + red;

      atomicStore(rs_partitions.extent[RS_WORKGROUP_BASE_FINAL],
                  inc | RS_PARTITION_MASK_PREFIX,
                  gl_ScopeQueueFamily,
                  gl_StorageSemanticsBuffer,
                  gl_SemanticsRelease);
    }
#endif

#elif (RS_WORKGROUP_SIZE >= RS_RADIX_SIZE)
  ////////////////////////////////////////////////////////////////////////////
  //
  // (RS_WORKGROUP_SIZE >= RS_RADIX_SIZE)
  //
#if (RS_WORKGROUP_SIZE > RS_RADIX_SIZE)
  if (gl_LocalInvocationID.x < RS_RADIX_SIZE)
#endif
    {
      const uint32_t exc = rs_histogram.extent[0];
      const uint32_t red = smem.extent[RS_SMEM_HISTOGRAM_OFFSET + gl_LocalInvocationID.x];

      smem.extent[RS_SMEM_LOOKBACK_OFFSET + gl_LocalInvocationID.x] = exc;

      const uint32_t inc = exc + red;

      atomicStore(rs_partitions.extent[0],
                  inc | RS_PARTITION_MASK_PREFIX,
                  gl_ScopeQueueFamily,
                  gl_StorageSemanticsBuffer,
                  gl_SemanticsRelease);
    }

#endif
}

//
// Atomically store the reduction to the global partition.
//
void
rs_reduction_store(restrict buffer_rs_partitions      rs_partitions,
                   RS_SUBGROUP_UNIFORM const uint32_t partition_base)
{
#if (RS_WORKGROUP_SUBGROUPS == 1)
  ////////////////////////////////////////////////////////////////////////////
  //
  // (RS_WORKGROUP_SUBGROUPS == 1)
  //
  const uint32_t smem_offset = RS_SMEM_HISTOGRAM_OFFSET + gl_SubgroupInvocationID;

  [[unroll]] for (uint32_t ii = 0; ii < RS_RADIX_SIZE; ii += RS_SUBGROUP_SIZE)
  {
    const uint32_t red = smem.extent[smem_offset + ii];

    atomicStore(rs_partitions.extent[partition_base + ii],
                red | RS_PARTITION_MASK_REDUCTION,
                gl_ScopeQueueFamily,
                gl_StorageSemanticsBuffer,
                gl_SemanticsRelease);
  }

#elif (RS_WORKGROUP_SIZE < RS_RADIX_SIZE)
  ////////////////////////////////////////////////////////////////////////////
  //
  // (RS_WORKGROUP_SIZE < RS_RADIX_SIZE)
  //
  const uint32_t smem_offset = RS_SMEM_HISTOGRAM_OFFSET + gl_LocalInvocationID.x;

  [[unroll]] for (uint32_t ii = 0; ii < RS_RADIX_SIZE; ii += RS_WORKGROUP_SIZE)
  {
    const uint32_t red = smem.extent[smem_offset + ii];

    atomicStore(rs_partitions.extent[partition_base + ii],
                red | RS_PARTITION_MASK_REDUCTION,
                gl_ScopeQueueFamily,
                gl_StorageSemanticsBuffer,
                gl_SemanticsRelease);
  }

#if (RS_WORKGROUP_BASE_FINAL < RS_RADIX_SIZE)
  const uint32_t smem_offset_final = smem_offset + RS_WORKGROUP_BASE_FINAL;

  if (smem_offset_final < RS_RADIX_SIZE)
    {
      const uint32_t red = smem.extent[smem_offset_final];

      atomicStore(rs_partitions.extent[partition_base + RS_WORKGROUP_BASE_FINAL],
                  red | RS_PARTITION_MASK_REDUCTION,
                  gl_ScopeQueueFamily,
                  gl_StorageSemanticsBuffer,
                  gl_SemanticsRelease);
    }
#endif

#elif (RS_WORKGROUP_SIZE >= RS_RADIX_SIZE)
  ////////////////////////////////////////////////////////////////////////////
  //
  // (RS_WORKGROUP_SIZE >= RS_RADIX_SIZE)
  //
#if (RS_WORKGROUP_SIZE > RS_RADIX_SIZE)
  if (gl_LocalInvocationID.x < RS_RADIX_SIZE)
#endif
    {
      const uint32_t red = smem.extent[RS_SMEM_HISTOGRAM_OFFSET + gl_LocalInvocationID.x];

      atomicStore(rs_partitions.extent[partition_base],
                  red | RS_PARTITION_MASK_REDUCTION,
                  gl_ScopeQueueFamily,
                  gl_StorageSemanticsBuffer,
                  gl_SemanticsRelease);
    }

#endif
}

//
// Lookback and accumulate reductions until a PREFIX partition is
// reached and then update this workgroup's partition and local
// histogram prefix.
//
// TODO(allanmac): Consider reenabling the cyclic/ring buffer of
// partitions in order to save memory.  It actually adds complexity
// but reduces the amount of pre-scatter buffer zeroing.
//
void
rs_lookback_store(restrict buffer_rs_partitions      rs_partitions,
                  RS_SUBGROUP_UNIFORM const uint32_t partition_base)
{
#if (RS_WORKGROUP_SUBGROUPS == 1)
  ////////////////////////////////////////////////////////////////////////////
  //
  // (RS_WORKGROUP_SUBGROUPS == 1)
  //
  const uint32_t smem_offset = RS_SMEM_LOOKBACK_OFFSET + gl_SubgroupInvocationID;

  [[unroll]] for (uint32_t ii = 0; ii < RS_RADIX_SIZE; ii += RS_SUBGROUP_SIZE)
  {
    uint32_t partition_base_prev = partition_base - RS_RADIX_SIZE;
    uint32_t exc                 = 0;

    //
    // NOTE: Each workgroup invocation can proceed independently.
    // Subgroups and workgroups do NOT have to coordinate.
    //
    while (true)
      {
        const uint32_t prev = atomicLoad(rs_partitions.extent[partition_base_prev + ii],
                                         gl_ScopeQueueFamily,
                                         gl_StorageSemanticsBuffer,
                                         gl_SemanticsAcquire);

        // spin until valid
        if ((prev & RS_PARTITION_MASK_STATUS) == RS_PARTITION_MASK_INVALID)
          {
            continue;
          }

        exc += (prev & RS_PARTITION_MASK_COUNT);

        if ((prev & RS_PARTITION_MASK_STATUS) != RS_PARTITION_MASK_PREFIX)
          {
            // continue accumulating reductions
            partition_base_prev -= RS_RADIX_SIZE;
            continue;
          }

        //
        // Otherwise, save the exclusive scan and atomically transform
        // the reduction into an inclusive prefix status math:
        //
        //   reduction + 1 = prefix
        //
        smem.extent[smem_offset + ii] = exc;

        atomicAdd(rs_partitions.extent[partition_base + ii],
                  exc | (1 << 30),
                  gl_ScopeQueueFamily,
                  gl_StorageSemanticsBuffer,
                  gl_SemanticsAcquireRelease);
        break;
      }
  }

#elif (RS_WORKGROUP_SIZE < RS_RADIX_SIZE)
  ////////////////////////////////////////////////////////////////////////////
  //
  // (RS_WORKGROUP_SIZE < RS_RADIX_SIZE)
  //
  const uint32_t smem_offset = RS_SMEM_LOOKBACK_OFFSET + gl_LocalInvocationID.x;

  [[unroll]] for (uint32_t ii = 0; ii < RS_RADIX_SIZE; ii += RS_WORKGROUP_SIZE)
  {
    uint32_t partition_base_prev = partition_base - RS_RADIX_SIZE;
    uint32_t exc                 = 0;

    //
    // NOTE: Each workgroup invocation can proceed independently.
    // Subgroups and workgroups do NOT have to coordinate.
    //
    while (true)
      {
        const uint32_t prev = atomicLoad(rs_partitions.extent[partition_base_prev + ii],
                                         gl_ScopeQueueFamily,
                                         gl_StorageSemanticsBuffer,
                                         gl_SemanticsAcquire);

        // spin until valid
        if ((prev & RS_PARTITION_MASK_STATUS) == RS_PARTITION_MASK_INVALID)
          {
            continue;
          }

        exc += (prev & RS_PARTITION_MASK_COUNT);

        if ((prev & RS_PARTITION_MASK_STATUS) != RS_PARTITION_MASK_PREFIX)
          {
            // continue accumulating reductions
            partition_base_prev -= RS_RADIX_SIZE;
            continue;
          }

        //
        // Otherwise, save the exclusive scan and atomically transform
        // the reduction into an inclusive prefix status math:
        //
        //   reduction + 1 = prefix
        //
        smem.extent[smem_offset + ii] = exc;

        atomicAdd(rs_partitions.extent[partition_base + ii],
                  exc | (1 << 30),
                  gl_ScopeQueueFamily,
                  gl_StorageSemanticsBuffer,
                  gl_SemanticsAcquireRelease);
        break;
      }
  }

#if (RS_WORKGROUP_BASE_FINAL < RS_RADIX_SIZE)
  const uint32_t smem_offset_final = smem_offset + RS_WORKGROUP_BASE_FINAL;

  if (smem_offset_final < RS_SMEM_LOOKBACK_OFFSET + RS_RADIX_SIZE)
    {
      uint32_t partition_base_prev = partition_base - RS_RADIX_SIZE;
      uint32_t exc                 = 0;

      //
      // NOTE: Each workgroup invocation can proceed independently.
      // Subgroups and workgroups do NOT have to coordinate.
      //
      while (true)
        {
          const uint32_t prev = atomicLoad(rs_partitions.extent[partition_base_prev + ii],
                                           gl_ScopeQueueFamily,
                                           gl_StorageSemanticsBuffer,
                                           gl_SemanticsAcquire);

          // spin until valid
          if ((prev & RS_PARTITION_MASK_STATUS) == RS_PARTITION_MASK_INVALID)
            {
              continue;
            }

          exc += (prev & RS_PARTITION_MASK_COUNT);

          if ((prev & RS_PARTITION_MASK_STATUS) != RS_PARTITION_MASK_PREFIX)
            {
              // continue accumulating reductions
              partition_base_prev -= RS_RADIX_SIZE;
              continue;
            }

          //
          // Otherwise, save the exclusive scan and atomically transform
          // the reduction into an inclusive prefix status math:
          //
          //   reduction + 1 = prefix
          //
          smem.extent[smem_offset + ii] = exc;

          atomicAdd(rs_partitions.extent[partition_base + RS_WORKGROUP_BASE_FINAL],
                    exc | (1 << 30),
                    gl_ScopeQueueFamily,
                    gl_StorageSemanticsBuffer,
                    gl_SemanticsAcquireRelease);
          break;
        }
    }
#endif

#elif (RS_WORKGROUP_SIZE >= RS_RADIX_SIZE)
  ////////////////////////////////////////////////////////////////////////////
  //
  // (RS_WORKGROUP_SIZE >= RS_RADIX_SIZE)
  //
#if (RS_WORKGROUP_SIZE > RS_RADIX_SIZE)
  if (gl_LocalInvocationID.x < RS_RADIX_SIZE)
#endif
    {
      uint32_t partition_base_prev = partition_base - RS_RADIX_SIZE;
      uint32_t exc                 = 0;

      //
      // NOTE: Each workgroup invocation can proceed independently.
      // Subgroups and workgroups do NOT have to coordinate.
      //
      while (true)
        {
          const uint32_t prev = atomicLoad(rs_partitions.extent[partition_base_prev],
                                           gl_ScopeQueueFamily,
                                           gl_StorageSemanticsBuffer,
                                           gl_SemanticsAcquire);

          // spin until valid
          if ((prev & RS_PARTITION_MASK_STATUS) == RS_PARTITION_MASK_INVALID)
            {
              continue;
            }

          exc += (prev & RS_PARTITION_MASK_COUNT);

          if ((prev & RS_PARTITION_MASK_STATUS) != RS_PARTITION_MASK_PREFIX)
            {
              // continue accumulating reductions
              partition_base_prev -= RS_RADIX_SIZE;
              continue;
            }

          //
          // Otherwise, save the exclusive scan and atomically transform
          // the reduction into an inclusive prefix status math:
          //
          //   reduction + 1 = prefix
          //
          smem.extent[RS_SMEM_LOOKBACK_OFFSET + gl_LocalInvocationID.x] = exc;

          atomicAdd(rs_partitions.extent[partition_base],
                    exc | (1 << 30),
                    gl_ScopeQueueFamily,
                    gl_StorageSemanticsBuffer,
                    gl_SemanticsAcquireRelease);
          break;
        }
    }

#endif
}

//
// Lookback and accumulate reductions until a PREFIX partition is
// reached and then update this workgroup's local histogram prefix.
//
// Skip updating this workgroup's partition because it's last.
//
void
rs_lookback_skip_store(restrict buffer_rs_partitions      rs_partitions,
                       RS_SUBGROUP_UNIFORM const uint32_t partition_base)
{
#if (RS_WORKGROUP_SUBGROUPS == 1)
  ////////////////////////////////////////////////////////////////////////////
  //
  // (RS_WORKGROUP_SUBGROUPS == 1)
  //
  const uint32_t smem_offset = RS_SMEM_LOOKBACK_OFFSET + gl_SubgroupInvocationID;

  [[unroll]] for (uint32_t ii = 0; ii < RS_RADIX_SIZE; ii += RS_SUBGROUP_SIZE)
  {
    uint32_t partition_base_prev = partition_base - RS_RADIX_SIZE;
    uint32_t exc                 = 0;

    //
    // NOTE: Each workgroup invocation can proceed independently.
    // Subgroups and workgroups do NOT have to coordinate.
    //
    while (true)
      {
        const uint32_t prev = atomicLoad(rs_partitions.extent[partition_base_prev + ii],
                                         gl_ScopeQueueFamily,
                                         gl_StorageSemanticsBuffer,
                                         gl_SemanticsAcquire);

        // spin until valid
        if ((prev & RS_PARTITION_MASK_STATUS) == RS_PARTITION_MASK_INVALID)
          {
            continue;
          }

        exc += (prev & RS_PARTITION_MASK_COUNT);

        if ((prev & RS_PARTITION_MASK_STATUS) != RS_PARTITION_MASK_PREFIX)
          {
            // continue accumulating reductions
            partition_base_prev -= RS_RADIX_SIZE;
            continue;
          }

        // Otherwise, save the exclusive scan.
        smem.extent[smem_offset + ii] = exc;
        break;
      }
  }

#elif (RS_WORKGROUP_SIZE < RS_RADIX_SIZE)
  ////////////////////////////////////////////////////////////////////////////
  //
  // (RS_WORKGROUP_SIZE < RS_RADIX_SIZE)
  //
  const uint32_t smem_offset = RS_SMEM_LOOKBACK_OFFSET + gl_LocalInvocationID.x;

  [[unroll]] for (uint32_t ii = 0; ii < RS_RADIX_SIZE; ii += RS_WORKGROUP_SIZE)
  {
    uint32_t partition_base_prev = partition_base - RS_RADIX_SIZE;
    uint32_t exc                 = 0;

    //
    // NOTE: Each workgroup invocation can proceed independently.
    // Subgroups and workgroups do NOT have to coordinate.
    //
    while (true)
      {
        const uint32_t prev = atomicLoad(rs_partitions.extent[partition_base_prev + ii],
                                         gl_ScopeQueueFamily,
                                         gl_StorageSemanticsBuffer,
                                         gl_SemanticsAcquire);

        // spin until valid
        if ((prev & RS_PARTITION_MASK_STATUS) == RS_PARTITION_MASK_INVALID)
          {
            continue;
          }

        exc += (prev & RS_PARTITION_MASK_COUNT);

        if ((prev & RS_PARTITION_MASK_STATUS) != RS_PARTITION_MASK_PREFIX)
          {
            // continue accumulating reductions
            partition_base_prev -= RS_RADIX_SIZE;
            continue;
          }

        // Otherwise, save the exclusive scan.
        smem.extent[smem_offset + ii] = exc;
        break;
      }
  }

#if (RS_WORKGROUP_BASE_FINAL < RS_RADIX_SIZE)
  const uint32_t smem_offset_final = smem_offset + RS_WORKGROUP_BASE_FINAL;

  if (smem_offset_final < RS_RADIX_SIZE)
    {
      uint32_t partition_base_prev = partition_base - RS_RADIX_SIZE;
      uint32_t exc                 = 0;

      //
      // NOTE: Each workgroup invocation can proceed independently.
      // Subgroups and workgroups do NOT have to coordinate.
      //
      while (true)
        {
          const uint32_t prev =
            atomicLoad(rs_partitions.extent[partition_base_prev + RS_WORKGROUP_BASE_FINAL],
                       gl_ScopeQueueFamily,
                       gl_StorageSemanticsBuffer,
                       gl_SemanticsAcquire);

          // spin until valid
          if ((prev & RS_PARTITION_MASK_STATUS) == RS_PARTITION_MASK_INVALID)
            {
              continue;
            }

          exc += (prev & RS_PARTITION_MASK_COUNT);

          if ((prev & RS_PARTITION_MASK_STATUS) != RS_PARTITION_MASK_PREFIX)
            {
              // continue accumulating reductions
              partition_base_prev -= RS_RADIX_SIZE;
              continue;
            }

          // Otherwise, save the exclusive scan.
          smem.extent[smem_offset_final] = exc;
          break;
        }
    }
#endif

#elif (RS_WORKGROUP_SIZE >= RS_RADIX_SIZE)
  ////////////////////////////////////////////////////////////////////////////
  //
  // (RS_WORKGROUP_SIZE >= RS_RADIX_SIZE)
  //
#if (RS_WORKGROUP_SIZE > RS_RADIX_SIZE)
  if (gl_LocalInvocationID.x < RS_RADIX_SIZE)
#endif
    {
      uint32_t partition_base_prev = partition_base - RS_RADIX_SIZE;
      uint32_t exc                 = 0;

      //
      // NOTE: Each workgroup invocation can proceed independently.
      // Subgroups and workgroups do NOT have to coordinate.
      //
      while (true)
        {
          const uint32_t prev = atomicLoad(rs_partitions.extent[partition_base_prev],
                                           gl_ScopeQueueFamily,
                                           gl_StorageSemanticsBuffer,
                                           gl_SemanticsAcquire);

          // spin until valid
          if ((prev & RS_PARTITION_MASK_STATUS) == RS_PARTITION_MASK_INVALID)
            {
              continue;
            }

          exc += (prev & RS_PARTITION_MASK_COUNT);

          if ((prev & RS_PARTITION_MASK_STATUS) != RS_PARTITION_MASK_PREFIX)
            {
              // continue accumulating reductions
              partition_base_prev -= RS_RADIX_SIZE;
              continue;
            }

          // Otherwise, save the exclusive scan.
          smem.extent[RS_SMEM_LOOKBACK_OFFSET + gl_LocalInvocationID.x] = exc;
          break;
        }
    }

#endif
}

//
// Compute a 1-based local index for each keyval by adding the 1-based
// rank to the local histogram prefix.
//
void
rs_rank_to_local(const RS_KEYVAL_TYPE kv[RS_SCATTER_BLOCK_ROWS],
                 inout uint32_t       kr[RS_SCATTER_BLOCK_ROWS])
{
  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    const uint32_t digit = RS_KV_EXTRACT_DIGIT(kv[ii]);
    const uint32_t exc   = smem.extent[RS_SMEM_HISTOGRAM_OFFSET + digit];
    const uint32_t idx   = exc + kr[ii];

    kr[ii] |= (idx << 16);
  }

  //
  // Reordering phase will overwrite histogram span.
  //
  RS_BARRIER();
}

//
// Compute a 1-based local index for each keyval by adding the 1-based
// rank to the global histogram prefix.
//
void
rs_rank_to_global(const RS_KEYVAL_TYPE kv[RS_SCATTER_BLOCK_ROWS],
                  inout uint32_t       kr[RS_SCATTER_BLOCK_ROWS])
{
  //
  // Define the histogram reference
  //
  readonly RS_BUFREF_DEFINE(buffer_rs_histogram, rs_histogram, push.devaddr_histograms);

  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    const uint32_t digit = RS_KV_EXTRACT_DIGIT(kv[ii]);
    const uint32_t exc   = rs_histogram.extent[digit];

    kr[ii] += (exc - 1);
  }
}

//
// Using the local indices, rearrange the keyvals into sorted order.
//
void
rs_reorder(inout RS_KEYVAL_TYPE kv[RS_SCATTER_BLOCK_ROWS], inout uint32_t kr[RS_SCATTER_BLOCK_ROWS])
{
  // clang-format off
#if (RS_WORKGROUP_SUBGROUPS == 1)
  const uint32_t smem_base = RS_SMEM_REORDER_OFFSET + gl_SubgroupInvocationID;
#else
  const uint32_t smem_base = RS_SMEM_REORDER_OFFSET + gl_LocalInvocationID.x;
#endif
  // clang-format on

  [[unroll]] for (uint32_t ii = 0; ii < RS_KEYVAL_DWORDS; ii++)
  {
    //
    // Store keyval dword to sorted location
    //
    [[unroll]] for (uint32_t jj = 0; jj < RS_SCATTER_BLOCK_ROWS; jj++)
    {
      const uint32_t smem_idx = (RS_SMEM_REORDER_OFFSET - 1) + (kr[jj] >> 16);

      smem.extent[smem_idx] = RS_KV_DWORD(kv[jj], ii);
    }

    RS_BARRIER();

    //
    // Load keyval dword from sorted location
    //
    [[unroll]] for (uint32_t jj = 0; jj < RS_SCATTER_BLOCK_ROWS; jj++)
    {
      RS_KV_DWORD(kv[jj], ii) = smem.extent[smem_base + jj * RS_WORKGROUP_SIZE];
    }

    RS_BARRIER();
  }

  //
  // Store the digit-index to sorted location
  //
  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    const uint32_t smem_idx = (RS_SMEM_REORDER_OFFSET - 1) + (kr[ii] >> 16);

    smem.extent[smem_idx] = uint32_t(kr[ii]);
  }

  RS_BARRIER();

  //
  // Load kr[] from sorted location -- we only need the rank.
  //
  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    kr[ii] = smem.extent[smem_base + ii * RS_WORKGROUP_SIZE] & 0xFFFF;
  }
}

//
// Using the global/local indices obtained by a single workgroup,
// rearrange the keyvals into sorted order.
//
void
rs_reorder_1(inout RS_KEYVAL_TYPE kv[RS_SCATTER_BLOCK_ROWS],
             inout uint32_t       kr[RS_SCATTER_BLOCK_ROWS])
{
  // clang-format off
#if (RS_WORKGROUP_SUBGROUPS == 1)
  const uint32_t smem_base = RS_SMEM_REORDER_OFFSET + gl_SubgroupInvocationID;
#else
  const uint32_t smem_base = RS_SMEM_REORDER_OFFSET + gl_LocalInvocationID.x;
#endif
  // clang-format on

  [[unroll]] for (uint32_t ii = 0; ii < RS_KEYVAL_DWORDS; ii++)
  {
    //
    // Store keyval dword to sorted location
    //
    [[unroll]] for (uint32_t jj = 0; jj < RS_SCATTER_BLOCK_ROWS; jj++)
    {
      const uint32_t smem_idx = RS_SMEM_REORDER_OFFSET + kr[jj];

      smem.extent[smem_idx] = RS_KV_DWORD(kv[jj], ii);
    }

    RS_BARRIER();

    //
    // Load keyval dword from sorted location
    //
    [[unroll]] for (uint32_t jj = 0; jj < RS_SCATTER_BLOCK_ROWS; jj++)
    {
      RS_KV_DWORD(kv[jj], ii) = smem.extent[smem_base + jj * RS_WORKGROUP_SIZE];
    }

    RS_BARRIER();
  }

  //
  // Store the digit-index to sorted location
  //
  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    const uint32_t smem_idx = RS_SMEM_REORDER_OFFSET + kr[ii];

    smem.extent[smem_idx] = uint32_t(kr[ii]);
  }

  RS_BARRIER();

  //
  // Load kr[] from sorted location -- we only need the rank.
  //
  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    kr[ii] = smem.extent[smem_base + ii * RS_WORKGROUP_SIZE];
  }
}

//
// Each subgroup loads RS_SCATTER_BLOCK_ROWS rows of keyvals into
// registers.
//
void
rs_load(out RS_KEYVAL_TYPE kv[RS_SCATTER_BLOCK_ROWS])
{
  //
  // Set up buffer reference
  //
  const uint32_t kv_in_offset_keys = gl_WorkGroupID.x * RS_BLOCK_KEYVALS +
                                     gl_SubgroupID * RS_SUBGROUP_KEYVALS + gl_SubgroupInvocationID;

  u32vec2 kv_in_offset;

  umulExtended(kv_in_offset_keys,
               RS_KEYVAL_SIZE,
               kv_in_offset.y,   // msb
               kv_in_offset.x);  // lsb

  readonly RS_BUFREF_DEFINE_AT_OFFSET_U32VEC2(buffer_rs_kv,
                                              rs_kv_in,
                                              RS_DEVADDR_KEYVALS_IN(push),
                                              kv_in_offset);

  //
  // Load keyvals
  //
  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    kv[ii] = rs_kv_in.extent[ii * RS_SUBGROUP_SIZE];
  }
}

//
// Convert local index to global
//
void
rs_local_to_global(const RS_KEYVAL_TYPE kv[RS_SCATTER_BLOCK_ROWS],
                   inout uint32_t       kr[RS_SCATTER_BLOCK_ROWS])
{
  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    const uint32_t digit = RS_KV_EXTRACT_DIGIT(kv[ii]);
    const uint32_t exc   = smem.extent[RS_SMEM_LOOKBACK_OFFSET + digit];

    kr[ii] += (exc - 1);
  }
}

//
// Store a single workgroup
//
void
rs_store(const RS_KEYVAL_TYPE kv[RS_SCATTER_BLOCK_ROWS], const uint32_t kr[RS_SCATTER_BLOCK_ROWS])
{
  //
  // Define kv_out bufref
  //
  writeonly RS_BUFREF_DEFINE(buffer_rs_kv, rs_kv_out, RS_DEVADDR_KEYVALS_OUT(push));

  //
  // Store keyval:
  //
  //   "out[ keyval.rank ] = keyval"
  //
  // FIXME(allanmac): Consider implementing an aligned writeout
  // strategy to avoid excess global memory transactions.
  //
  [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
  {
    rs_kv_out.extent[kr[ii]] = kv[ii];
  }
}

//
//
//
void
main()
{
  //
  // Load keyvals
  //
  RS_KEYVAL_TYPE kv[RS_SCATTER_BLOCK_ROWS];

  rs_load(kv);

  //
  // Zero shared histogram
  //
  // Ends with barrier.
  //
  rs_histogram_zero();

  //
  // Compute histogram and bin-relative keyval indices
  //
  // This histogram can immediately be used to update the partition
  // with either a PREFIX or REDUCTION flag.
  //
  // Ends with a barrier.
  //
  uint32_t kr[RS_SCATTER_BLOCK_ROWS];

  rs_histogram_rank(kv, kr);

//
// DEBUG
//
#if 0  // (RS_KEYVAL_DWORDS == 1)
  {
    writeonly RS_BUFREF_DEFINE_AT_OFFSET_UINT32(buffer_rs_kv,
                                                rs_kv_out,
                                                RS_DEVADDR_KEYVALS_OUT(push),
                                                gl_LocalInvocationID.x * 4);

    [[unroll]] for (uint32_t ii = 0; ii < RS_SCATTER_BLOCK_ROWS; ii++)
    {
      rs_kv_out.extent[gl_WorkGroupID.x * RS_BLOCK_KEYVALS + ii * RS_WORKGROUP_SIZE] = kr[ii];
    }

    return;
  }
#endif

  //
  // When there is a single workgroup then the local and global
  // exclusive scanned histograms are the same.
  //
  if (gl_NumWorkGroups.x == 1)
    {
      rs_rank_to_global(kv, kr);

#ifndef RS_SCATTER_DISABLE_REORDER
      rs_reorder_1(kv, kr);
#endif

      rs_store(kv, kr);
    }
  else
    {
      //
      // Define partitions bufref
      //
#if (RS_WORKGROUP_SUBGROUPS == 1)
      const uint32_t partition_offset = gl_SubgroupInvocationID * 4;
#else
      const uint32_t partition_offset = gl_LocalInvocationID.x * 4;
#endif

      RS_BUFREF_DEFINE_AT_OFFSET_UINT32(buffer_rs_partitions,
                                        rs_partitions,
                                        push.devaddr_partitions,
                                        partition_offset);

      //
      // The first partition is a special case.
      //
      if (gl_WorkGroupID.x == 0)
        {
          //
          // Other workgroups may lookback on this partition.
          //
          // Load the global histogram and local histogram and store
          // the exclusive prefix.
          //
          rs_first_prefix_store(rs_partitions);
        }
      else
        {
          //
          // Otherwise, this is not the first workgroup.
          //
          RS_SUBGROUP_UNIFORM const uint32_t partition_base = gl_WorkGroupID.x * RS_RADIX_SIZE;

          //
          // The last partition is a special case.
          //
          if (gl_WorkGroupID.x + 1 < gl_NumWorkGroups.x)
            {
              //
              // Atomically store the reduction to the global partition.
              //
              rs_reduction_store(rs_partitions, partition_base);

              //
              // Lookback and accumulate reductions until a PREFIX
              // partition is reached and then update this workgroup's
              // partition and local histogram prefix.
              //
              rs_lookback_store(rs_partitions, partition_base);
            }
          else
            {
              //
              // Lookback and accumulate reductions until a PREFIX
              // partition is reached and then update this workgroup's
              // local histogram prefix.
              //
              // Skip updating this workgroup's partition because it's
              // last.
              //
              rs_lookback_skip_store(rs_partitions, partition_base);
            }
        }

#ifndef RS_SCATTER_DISABLE_REORDER
      //
      // Compute exclusive prefix scan of histogram.
      //
      // No barrier.
      //
      rs_prefix();

      //
      // Barrier before reading prefix scanned histogram.
      //
      RS_BARRIER();

      //
      // Convert keyval's rank to a local index
      //
      // Ends with a barrier.
      //
      rs_rank_to_local(kv, kr);

      //
      // Reorder kv[] and kr[]
      //
      // Ends with a barrier.
      //
      rs_reorder(kv, kr);
#else
      //
      // Wait for lookback to complete.
      //
      RS_BARRIER();
#endif

      //
      // Convert local index to a global index.
      //
      rs_local_to_global(kv, kr);

      //
      // Store keyvals to their new locations
      //
      rs_store(kv, kr);
    }
}

//
//
//
