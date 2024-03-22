/*
 * Copyright © 2021 Bas Nieuwenhuizen
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

#ifndef BVH_BVH_H
#define BVH_BVH_H

#define radv_bvh_node_triangle 0
#define radv_bvh_node_box16    4
#define radv_bvh_node_box32    5
#define radv_bvh_node_instance 6
#define radv_bvh_node_aabb     7

#define radv_ir_node_triangle 0
#define radv_ir_node_internal 1
#define radv_ir_node_instance 2
#define radv_ir_node_aabb     3

#define RADV_GEOMETRY_OPAQUE (1u << 31)

#define RADV_INSTANCE_FORCE_OPAQUE                 (1u << 31)
#define RADV_INSTANCE_NO_FORCE_NOT_OPAQUE          (1u << 30)
#define RADV_INSTANCE_TRIANGLE_FACING_CULL_DISABLE (1u << 29)
#define RADV_INSTANCE_TRIANGLE_FLIP_FACING         (1u << 28)

#ifdef VULKAN
#define VK_UUID_SIZE 16
#else
#include <vulkan/vulkan.h>
typedef struct radv_ir_node radv_ir_node;
typedef struct radv_global_sync_data radv_global_sync_data;
typedef struct radv_bvh_geometry_data radv_bvh_geometry_data;

typedef uint16_t float16_t;

typedef struct {
   float values[3][4];
} mat3x4;

typedef struct {
   float x;
   float y;
   float z;
} vec3;

typedef struct radv_aabb radv_aabb;

#endif

struct radv_aabb {
   vec3 min;
   vec3 max;
};

struct radv_accel_struct_serialization_header {
   uint8_t driver_uuid[VK_UUID_SIZE];
   uint8_t accel_struct_compat[VK_UUID_SIZE];
   uint64_t serialization_size;
   uint64_t compacted_size;
   uint64_t instance_count;
#ifndef VULKAN
   uint64_t instances[];
#endif
};

struct radv_accel_struct_geometry_info {
   uint32_t primitive_count;
   uint32_t flags;
   uint32_t type;
};

struct radv_accel_struct_header {
   uint32_t bvh_offset;
   uint32_t reserved;
   radv_aabb aabb;

   /* Everything after this gets either updated/copied from the CPU or written by header.comp. */
   uint64_t compacted_size;
   uint64_t serialization_size;
   uint32_t copy_dispatch_size[3];
   uint64_t size;

   /* Everything after this gets updated/copied from the CPU. */
   uint32_t geometry_count;
   uint64_t instance_offset;
   uint64_t instance_count;
   uint32_t build_flags;
};

struct radv_ir_node {
   radv_aabb aabb;
};

#define RADV_UNKNOWN_BVH_OFFSET 0xFFFFFFFF
#define RADV_NULL_BVH_OFFSET    0xFFFFFFFE

struct radv_ir_box_node {
   radv_ir_node base;
   uint32_t children[2];
   uint32_t bvh_offset;
};

struct radv_global_sync_data {
   uint32_t task_counts[2];
   uint32_t task_started_counter;
   uint32_t task_done_counter;
   uint32_t current_phase_start_counter;
   uint32_t current_phase_end_counter;
   uint32_t phase_index;
   /* If this flag is set, the shader should exit
    * instead of executing another phase */
   uint32_t next_phase_exit_flag;
};

struct radv_ir_header {
   int32_t min_bounds[3];
   int32_t max_bounds[3];
   uint32_t active_leaf_count;
   /* Indirect dispatch dimensions for the encoder.
    * ir_internal_node_count is the thread count in the X dimension,
    * while Y and Z are always set to 1. */
   uint32_t ir_internal_node_count;
   uint32_t dispatch_size_y;
   uint32_t dispatch_size_z;
   radv_global_sync_data sync_data;
   uint32_t dst_node_offset;
};

struct radv_bvh_triangle_node {
   float coords[3][3];
   uint32_t reserved[3];
   uint32_t triangle_id;
   /* flags in upper 4 bits */
   uint32_t geometry_id_and_flags;
   uint32_t reserved2;
   uint32_t id;
};

struct radv_bvh_aabb_node {
   uint32_t primitive_id;
   /* flags in upper 4 bits */
   uint32_t geometry_id_and_flags;
   uint32_t reserved[14];
};

struct radv_bvh_instance_node {
   uint64_t bvh_ptr; /* pre-shifted/masked to serve as node base */

   /* lower 24 bits are the custom instance index, upper 8 bits are the visibility mask */
   uint32_t custom_instance_and_mask;
   /* lower 24 bits are the sbt offset, upper 8 bits are VkGeometryInstanceFlagsKHR */
   uint32_t sbt_offset_and_flags;

   mat3x4 wto_matrix;

   uint32_t instance_id;
   uint32_t bvh_offset;
   uint32_t reserved[2];

   /* Object to world matrix transposed from the initial transform. */
   mat3x4 otw_matrix;
};

struct radv_bvh_box16_node {
   uint32_t children[4];
   float16_t coords[4][2][3];
};

struct radv_bvh_box32_node {
   uint32_t children[4];
   radv_aabb coords[4];
   uint32_t reserved[4];
};

#define RADV_BVH_ROOT_NODE    radv_bvh_node_box32
#define RADV_BVH_INVALID_NODE 0xffffffffu

/* If the task index is set to this value, there is no
 * more work to do. */
#define TASK_INDEX_INVALID 0xFFFFFFFF

struct radv_bvh_geometry_data {
   uint64_t data;
   uint64_t indices;
   uint64_t transform;

   uint32_t geometry_id;
   uint32_t geometry_type;
   uint32_t first_id;
   uint32_t stride;
   uint32_t vertex_format;
   uint32_t index_format;
};

#endif
