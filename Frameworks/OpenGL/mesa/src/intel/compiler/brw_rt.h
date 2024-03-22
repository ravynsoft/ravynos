/*
 * Copyright © 2020 Intel Corporation
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

#ifndef BRW_RT_H
#define BRW_RT_H

#include <stdint.h>

#include "compiler/shader_enums.h"
#include "util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Vulkan defines shaderGroupHandleSize = 32 */
#define BRW_RT_SBT_HANDLE_SIZE 32

/** RT_DISPATCH_GLOBALS size (see gen_rt.xml) */
#define BRW_RT_DISPATCH_GLOBALS_SIZE 80

/** Offset after the RT dispatch globals at which "push" constants live */
#define BRW_RT_PUSH_CONST_OFFSET 128

/** Stride of the resume SBT */
#define BRW_BTD_RESUME_SBT_STRIDE 8

/* Vulkan always uses exactly two levels of BVH: world and object.  At the API
 * level, these are referred to as top and bottom.
 */
enum brw_rt_bvh_level {
   BRW_RT_BVH_LEVEL_WORLD = 0,
   BRW_RT_BVH_LEVEL_OBJECT = 1,
};
#define BRW_RT_MAX_BVH_LEVELS 2

enum brw_rt_bvh_node_type {
   BRW_RT_BVH_NODE_TYPE_INTERNAL = 0,
   BRW_RT_BVH_NODE_TYPE_INSTANCE = 1,
   BRW_RT_BVH_NODE_TYPE_PROCEDURAL = 3,
   BRW_RT_BVH_NODE_TYPE_QUAD = 4,
};

/** HitKind values returned for triangle geometry
 *
 * This enum must match the SPIR-V enum.
 */
enum brw_rt_hit_kind {
   BRW_RT_HIT_KIND_FRONT_FACE = 0xfe,
   BRW_RT_HIT_KIND_BACK_FACE = 0xff,
};

/** Ray flags
 *
 * This enum must match the SPIR-V RayFlags enum.
 */
enum brw_rt_ray_flags {
   BRW_RT_RAY_FLAG_FORCE_OPAQUE                    = 0x01,
   BRW_RT_RAY_FLAG_FORCE_NON_OPAQUE                = 0x02,
   BRW_RT_RAY_FLAG_TERMINATE_ON_FIRST_HIT          = 0x04,
   BRW_RT_RAY_FLAG_SKIP_CLOSEST_HIT_SHADER         = 0x08,
   BRW_RT_RAY_FLAG_CULL_BACK_FACING_TRIANGLES      = 0x10,
   BRW_RT_RAY_FLAG_CULL_FRONT_FACING_TRIANGLES     = 0x20,
   BRW_RT_RAY_FLAG_CULL_OPAQUE                     = 0x40,
   BRW_RT_RAY_FLAG_CULL_NON_OPAQUE                 = 0x80,
   BRW_RT_RAY_FLAG_SKIP_TRIANGLES                  = 0x100,
   BRW_RT_RAY_FLAG_SKIP_AABBS                      = 0x200,
};

struct brw_rt_scratch_layout {
   /** Number of stack IDs per DSS */
   uint32_t stack_ids_per_dss;

   /** Start offset (in bytes) of the hardware MemRay stack */
   uint32_t ray_stack_start;

   /** Stride (in bytes) of the hardware MemRay stack */
   uint32_t ray_stack_stride;

   /** Start offset (in bytes) of the SW stacks */
   uint64_t sw_stack_start;

   /** Size (in bytes) of the SW stack for a single shader invocation */
   uint32_t sw_stack_size;

   /** Total size (in bytes) of the RT scratch memory area */
   uint64_t total_size;
};

/** Parameters passed to the raygen trampoline shader
 *
 * This struct is carefully construected to be 32B and must be passed to the
 * raygen trampoline shader as as inline constant data.
 */
struct brw_rt_raygen_trampoline_params {
   /** The GPU address of the RT_DISPATCH_GLOBALS */
   uint64_t rt_disp_globals_addr;

   /** The GPU address of the BINDLESS_SHADER_RECORD for the raygen shader */
   uint64_t raygen_bsr_addr;

   /** 1 if this is an indirect dispatch, 0 otherwise */
   uint8_t is_indirect;

   /** The integer log2 of the local group size
    *
    * Ray-tracing shaders don't have a concept of local vs. global workgroup
    * size.  They only have a single 3D launch size.  The raygen trampoline
    * shader is always dispatched with a local workgroup size equal to the
    * SIMD width but the shape of the local workgroup is determined at
    * dispatch time based on the shape of the launch and passed to the
    * trampoline via this field.  (There's no sense having a Z dimension on
    * the local workgroup if the launch is 2D.)
    *
    * We use the integer log2 of the size because there's no point in
    * non-power-of-two sizes and  shifts are cheaper than division.
    */
   uint8_t local_group_size_log2[3];

   uint32_t pad[3];
};

/** Size of the "hot zone" in bytes
 *
 * The hot zone is a SW-defined data structure which is a single uvec4
 * containing two bits of information:
 *
 *  - hotzone.x: Stack offset (in bytes)
 *
 *    This is the offset (in bytes) into the per-thread scratch space at which
 *    the current shader's stack starts.  This is incremented by the calling
 *    shader prior to any shader call type instructions and gets decremented
 *    by the resume shader as part of completing the return operation.
 *
 *
 *  - hotzone.yzw: The launch ID associated with the current thread
 *
 *    Inside a bindless shader, the only information we have is the DSS ID
 *    from the hardware EU and a per-DSS stack ID.  In particular, the three-
 *    dimensional launch ID is lost the moment we leave the raygen trampoline.
 */
#define BRW_RT_SIZEOF_HOTZONE 16

/* From the BSpec "Address Computation for Memory Based Data Structures:
 * Ray and TraversalStack (Async Ray Tracing)":
 *
 *    sizeof(Ray) = 64B, sizeof(HitInfo) = 32B, sizeof(TravStack) = 32B.
 */
#define BRW_RT_SIZEOF_RAY 64
#define BRW_RT_SIZEOF_HIT_INFO 32
#define BRW_RT_SIZEOF_TRAV_STACK 32

/* From the BSpec:
 *
 *    syncStackSize = (maxBVHLevels % 2 == 1) ?
 *       (sizeof(HitInfo) * 2 +
 *          (sizeof(Ray) + sizeof(TravStack)) * maxBVHLevels + 32B) :
 *       (sizeof(HitInfo) * 2 +
 *          (sizeof(Ray) + sizeof(TravStack)) * maxBVHLevels);
 *
 * The select is just to align to 64B.
 */
#define BRW_RT_SIZEOF_RAY_QUERY \
   (BRW_RT_SIZEOF_HIT_INFO * 2 + \
    (BRW_RT_SIZEOF_RAY + BRW_RT_SIZEOF_TRAV_STACK) * BRW_RT_MAX_BVH_LEVELS + \
    (BRW_RT_MAX_BVH_LEVELS % 2 ? 32 : 0))

#define BRW_RT_SIZEOF_SHADOW_RAY_QUERY  \
   (BRW_RT_SIZEOF_HIT_INFO * 2 + \
    (BRW_RT_SIZEOF_RAY + BRW_RT_SIZEOF_TRAV_STACK) * BRW_RT_MAX_BVH_LEVELS)

#define BRW_RT_SIZEOF_HW_STACK \
   (BRW_RT_SIZEOF_HIT_INFO * 2 + \
    BRW_RT_SIZEOF_RAY * BRW_RT_MAX_BVH_LEVELS + \
    BRW_RT_SIZEOF_TRAV_STACK * BRW_RT_MAX_BVH_LEVELS)

/* This is a mesa-defined region for hit attribute data */
#define BRW_RT_SIZEOF_HIT_ATTRIB_DATA 64
#define BRW_RT_OFFSETOF_HIT_ATTRIB_DATA BRW_RT_SIZEOF_HW_STACK

#define BRW_RT_ASYNC_STACK_STRIDE \
   ALIGN_POT(BRW_RT_OFFSETOF_HIT_ATTRIB_DATA + \
             BRW_RT_SIZEOF_HIT_ATTRIB_DATA, 64)

static inline void
brw_rt_compute_scratch_layout(struct brw_rt_scratch_layout *layout,
                              const struct intel_device_info *devinfo,
                              uint32_t stack_ids_per_dss,
                              uint32_t sw_stack_size)
{
   layout->stack_ids_per_dss = stack_ids_per_dss;

   const uint32_t dss_count = intel_device_info_dual_subslice_id_bound(devinfo);
   const uint32_t num_stack_ids = dss_count * stack_ids_per_dss;

   uint64_t size = 0;

   /* The first thing in our scratch area is an array of "hot zones" which
    * store the stack offset as well as the launch IDs for each active
    * invocation.
    */
   size += BRW_RT_SIZEOF_HOTZONE * num_stack_ids;

   /* Next, we place the HW ray stacks */
   assert(size % 64 == 0); /* Cache-line aligned */
   assert(size < UINT32_MAX);
   layout->ray_stack_start = size;
   layout->ray_stack_stride = BRW_RT_ASYNC_STACK_STRIDE;
   size += num_stack_ids * layout->ray_stack_stride;

   /* Finally, we place the SW stacks for the individual ray-tracing shader
    * invocations.  We align these to 64B to ensure that we don't have any
    * shared cache lines which could hurt performance.
    */
   assert(size % 64 == 0);
   layout->sw_stack_start = size;
   layout->sw_stack_size = ALIGN(sw_stack_size, 64);

   /* Currently it's always the case that sw_stack_size is a power of
    * two, but power-of-two SW stack sizes are prone to causing
    * collisions in the hashing function used by the L3 to map memory
    * addresses to banks, which can cause stack accesses from most
    * DSSes to bottleneck on a single L3 bank.  Fix it by padding the
    * SW stack by a single cacheline if it was a power of two.
    */
   if (layout->sw_stack_size > 64 &&
       util_is_power_of_two_nonzero(layout->sw_stack_size))
      layout->sw_stack_size += 64;

   size += num_stack_ids * layout->sw_stack_size;

   layout->total_size = size;
}

static inline uint32_t
brw_rt_ray_queries_hw_stacks_size(const struct intel_device_info *devinfo)
{
   /* Maximum slice/subslice/EU ID can be computed from the max_scratch_ids
    * which includes all the threads.
    */
   uint32_t max_eu_id = devinfo->max_scratch_ids[MESA_SHADER_COMPUTE];
   uint32_t max_simd_size = 16; /* Cannot run in SIMD32 with ray queries */
   return max_eu_id * max_simd_size * BRW_RT_SIZEOF_RAY_QUERY;
}

static inline uint32_t
brw_rt_ray_queries_shadow_stack_size(const struct intel_device_info *devinfo)
{
   /* Maximum slice/subslice/EU ID can be computed from the max_scratch_ids
    * which includes all the threads.
    */
   uint32_t max_eu_id = devinfo->max_scratch_ids[MESA_SHADER_COMPUTE];
   uint32_t max_simd_size = 16; /* Cannot run in SIMD32 with ray queries */
   return max_eu_id * max_simd_size * BRW_RT_SIZEOF_SHADOW_RAY_QUERY;
}

static inline uint32_t
brw_rt_ray_queries_shadow_stacks_size(const struct intel_device_info *devinfo,
                                      uint32_t ray_queries)
{
   /* Don't bother a shadow stack if we only have a single query. We can
    * directly write in the HW buffer.
    */
   return (ray_queries > 1 ? ray_queries : 0) * brw_rt_ray_queries_shadow_stack_size(devinfo) +
          ray_queries * 4; /* Ctrl + Level data */
}

#ifdef __cplusplus
}
#endif

#endif /* BRW_RT_H */
