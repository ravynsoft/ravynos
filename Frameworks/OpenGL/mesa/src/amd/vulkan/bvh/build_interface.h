/*
 * Copyright © 2022 Konstantin Seurer
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

#ifndef BVH_BUILD_INTERFACE_H
#define BVH_BUILD_INTERFACE_H

#ifdef VULKAN
#include "build_helpers.h"
#else
#include <stdint.h>
#include "bvh.h"
#define REF(type) uint64_t
#define VOID_REF  uint64_t
#endif

struct leaf_args {
   VOID_REF ir;
   VOID_REF bvh;
   REF(radv_ir_header) header;
   REF(key_id_pair) ids;

   radv_bvh_geometry_data geom_data;
};

struct morton_args {
   VOID_REF bvh;
   REF(radv_ir_header) header;
   REF(key_id_pair) ids;
};

#define LBVH_RIGHT_CHILD_BIT_SHIFT 29
#define LBVH_RIGHT_CHILD_BIT       (1 << LBVH_RIGHT_CHILD_BIT_SHIFT)

struct lbvh_node_info {
   /* Number of children that have been processed (or are invalid/leaves) in
    * the lbvh_generate_ir pass.
    */
   uint32_t path_count;

   uint32_t children[2];
   uint32_t parent;
};

struct lbvh_main_args {
   VOID_REF bvh;
   REF(key_id_pair) src_ids;
   VOID_REF node_info;
   uint32_t id_count;
   uint32_t internal_node_base;
};

struct lbvh_generate_ir_args {
   VOID_REF bvh;
   VOID_REF node_info;
   VOID_REF header;
   uint32_t internal_node_base;
};

#define RADV_COPY_MODE_COPY        0
#define RADV_COPY_MODE_SERIALIZE   1
#define RADV_COPY_MODE_DESERIALIZE 2

struct copy_args {
   VOID_REF src_addr;
   VOID_REF dst_addr;
   uint32_t mode;
};

struct encode_args {
   VOID_REF intermediate_bvh;
   VOID_REF output_bvh;
   REF(radv_ir_header) header;
   uint32_t output_bvh_offset;
   uint32_t leaf_node_count;
   uint32_t geometry_type;
};

struct ploc_prefix_scan_partition {
   uint32_t aggregate;
   uint32_t inclusive_sum;
};

#define PLOC_WORKGROUP_SIZE 1024

struct ploc_args {
   VOID_REF bvh;
   VOID_REF prefix_scan_partitions;
   REF(radv_ir_header) header;
   VOID_REF ids_0;
   VOID_REF ids_1;
   uint32_t internal_node_offset;
};

struct header_args {
   REF(radv_ir_header) src;
   REF(radv_accel_struct_header) dst;
   uint32_t bvh_offset;
   uint32_t instance_count;
};

struct update_args {
   REF(radv_accel_struct_header) src;
   REF(radv_accel_struct_header) dst;
   REF(radv_aabb) leaf_bounds;
   REF(uint32_t) internal_ready_count;
   uint32_t leaf_node_count;

   radv_bvh_geometry_data geom_data;
};

#endif
