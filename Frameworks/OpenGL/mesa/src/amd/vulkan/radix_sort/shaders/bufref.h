// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_SHADERS_BUFREF_H_
#define SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_SHADERS_BUFREF_H_

//
// GLSL
//

#ifdef VULKAN  // defined by GLSL/VK compiler

#extension GL_EXT_shader_explicit_arithmetic_types : require

//
// If the target does not support VkPhysicalDeviceFeatures.shaderInt64
// then:
//
//   #define RS_DISABLE_SHADER_INT64
//
// clang-format off
#ifdef RS_DISABLE_SHADER_INT64
#extension GL_EXT_buffer_reference_uvec2 : require
#else
#extension GL_EXT_buffer_reference2      : require
#endif
// clang-format on

//
// Restrict shouldn't have any noticeable impact on these kernels and
// benchmarks appear to prove that true but it's correct to include
// the qualifier.
//
#define RS_RESTRICT restrict

//
// If the device doesn't support .shaderInt64 then the buffer reference address
// is a uvec2.
//
#ifdef RS_DISABLE_SHADER_INT64
#define RS_DEVADDR u32vec2
#else
#define RS_DEVADDR uint64_t
#endif

//
// Define a buffer reference.
//
#define RS_BUFREF_DEFINE(_layout, _name, _devaddr) RS_RESTRICT _layout _name = _layout(_devaddr)

//
// Define a buffer reference at a UINT32 offset.
//
#ifdef RS_DISABLE_SHADER_INT64
#define RS_BUFREF_DEFINE_AT_OFFSET_UINT32(_layout, _name, _devaddr_u32vec2, _offset)               \
  RS_RESTRICT _layout _name;                                                                       \
  {                                                                                                \
    u32vec2  devaddr;                                                                              \
    uint32_t carry;                                                                                \
                                                                                                   \
    devaddr.x = uaddCarry(_devaddr_u32vec2.x, _offset, carry);                                     \
    devaddr.y = _devaddr_u32vec2.y + carry;                                                        \
                                                                                                   \
    _name = _layout(devaddr);                                                                      \
  }
#else
#define RS_BUFREF_DEFINE_AT_OFFSET_UINT32(_layout, _name, _devaddr, _offset)                       \
  RS_RESTRICT _layout _name = _layout(_devaddr + _offset)
#endif

//
// Define a buffer reference at a packed UINT64 offset.
//
#ifdef RS_DISABLE_SHADER_INT64
#define RS_BUFREF_DEFINE_AT_OFFSET_U32VEC2(_layout, _name, _devaddr_u32vec2, _offset_u32vec2)      \
  RS_RESTRICT _layout _name;                                                                       \
  {                                                                                                \
    u32vec2  devaddr;                                                                              \
    uint32_t carry;                                                                                \
                                                                                                   \
    devaddr.x = uaddCarry(_devaddr_u32vec2.x, _offset_u32vec2.x, carry);                           \
    devaddr.y = _devaddr_u32vec2.y + _offset_u32vec2.y + carry;                                    \
                                                                                                   \
    _name = _layout(devaddr);                                                                      \
  }
#else
#define RS_BUFREF_DEFINE_AT_OFFSET_U32VEC2(_layout, _name, _devaddr, _offset_u32vec2)              \
  RS_RESTRICT _layout _name = _layout(_devaddr + pack64(_offset_u32vec2))
#endif

//
// Increment the buffer reference by a UINT32 offset.
//
#ifdef RS_DISABLE_SHADER_INT64
#define RS_BUFREF_INC_UINT32(_layout, _name, _inc)                                                 \
  {                                                                                                \
    u32vec2  devaddr = u32vec2(_name);                                                             \
    uint32_t carry;                                                                                \
                                                                                                   \
    devaddr.x = uaddCarry(devaddr.x, _inc, carry);                                                 \
    devaddr.y = devaddr.y + carry;                                                                 \
                                                                                                   \
    _name = _layout(devaddr);                                                                      \
  }
#else
#define RS_BUFREF_INC_UINT32(_layout, _name, _inc) _name = _layout(uint64_t(_name) + _inc)
#endif

//
// Increment the buffer reference by a packed UINT64 offset.
//
#ifdef RS_DISABLE_SHADER_INT64
#define RS_BUFREF_INC_U32VEC2(_layout, _name, _inc_u32vec2)                                        \
  {                                                                                                \
    u32vec2  devaddr = u32vec2(_name);                                                             \
    uint32_t carry;                                                                                \
                                                                                                   \
    devaddr.x = uaddCarry(devaddr.x, _inc_u32vec2.x, carry);                                       \
    devaddr.y = devaddr.y + _inc_u32vec2.y + carry;                                                \
                                                                                                   \
    _name = _layout(devaddr);                                                                      \
  }
#else
#define RS_BUFREF_INC_U32VEC2(_layout, _name, _inc_u32vec2)                                        \
  _name = _layout(uint64_t(_name) + pack64(_inc_u32vec2))
#endif

//
// Increment the buffer reference by the product of two UINT32 factors.
//
#define RS_BUFREF_INC_UINT32_UINT32(_layout, _name, _inc_a, _inc_b)                                \
  {                                                                                                \
    u32vec2 inc;                                                                                   \
                                                                                                   \
    umulExtended(_inc_a, _inc_b, inc.y, inc.x);                                                    \
                                                                                                   \
    RS_BUFREF_INC_U32VEC2(_layout, _name, inc);                                                    \
  }

//
//
//

#endif

//
//
//

#endif  // SRC_GRAPHICS_LIB_COMPUTE_RADIX_SORT_PLATFORMS_VK_SHADERS_BUFREF_H_
