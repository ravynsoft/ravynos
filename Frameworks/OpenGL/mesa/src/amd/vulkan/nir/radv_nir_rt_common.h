/*
 * Copyright © 2021 Google
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

#ifndef RADV_RT_COMMON_H
#define RADV_RT_COMMON_H

#include "nir/nir.h"
#include "nir/nir_builder.h"
#include "vk_nir_convert_ycbcr.h"

#include "compiler/spirv/spirv.h"

#include "radv_private.h"

nir_def *build_addr_to_node(nir_builder *b, nir_def *addr);

nir_def *nir_build_vec3_mat_mult(nir_builder *b, nir_def *vec, nir_def *matrix[], bool translation);

void nir_build_wto_matrix_load(nir_builder *b, nir_def *instance_addr, nir_def **out);

nir_def *radv_load_vertex_position(struct radv_device *device, nir_builder *b, nir_def *instance_addr,
                                   nir_def *primitive_id, uint32_t index);

struct radv_ray_traversal_args;

struct radv_ray_flags {
   nir_def *force_opaque;
   nir_def *force_not_opaque;
   nir_def *terminate_on_first_hit;
   nir_def *no_cull_front;
   nir_def *no_cull_back;
   nir_def *no_cull_opaque;
   nir_def *no_cull_no_opaque;
   nir_def *no_skip_triangles;
   nir_def *no_skip_aabbs;
};

struct radv_leaf_intersection {
   nir_def *node_addr;
   nir_def *primitive_id;
   nir_def *geometry_id_and_flags;
   nir_def *opaque;
};

typedef void (*radv_aabb_intersection_cb)(nir_builder *b, struct radv_leaf_intersection *intersection,
                                          const struct radv_ray_traversal_args *args);

struct radv_triangle_intersection {
   struct radv_leaf_intersection base;

   nir_def *t;
   nir_def *frontface;
   nir_def *barycentrics;
};

typedef void (*radv_triangle_intersection_cb)(nir_builder *b, struct radv_triangle_intersection *intersection,
                                              const struct radv_ray_traversal_args *args,
                                              const struct radv_ray_flags *ray_flags);

typedef void (*radv_rt_stack_store_cb)(nir_builder *b, nir_def *index, nir_def *value,
                                       const struct radv_ray_traversal_args *args);

typedef nir_def *(*radv_rt_stack_load_cb)(nir_builder *b, nir_def *index, const struct radv_ray_traversal_args *args);

struct radv_ray_traversal_vars {
   /* For each accepted hit, tmax will be set to the t value. This allows for automatic intersection
    * culling.
    */
   nir_deref_instr *tmax;

   /* Those variables change when entering and exiting BLASes. */
   nir_deref_instr *origin;
   nir_deref_instr *dir;
   nir_deref_instr *inv_dir;

   /* The base address of the current TLAS/BLAS. */
   nir_deref_instr *bvh_base;

   /* stack is the current stack pointer/index. top_stack is the pointer/index that marks the end of
    * traversal for the current BLAS/TLAS. stack_low_watermark is the low watermark of the short
    * stack.
    */
   nir_deref_instr *stack;
   nir_deref_instr *top_stack;
   nir_deref_instr *stack_low_watermark;

   nir_deref_instr *current_node;

   /* The node visited in the previous iteration. This is used in backtracking to jump to its parent
    * and then find the child after the previously visited node.
    */
   nir_deref_instr *previous_node;

   /* When entering an instance these are the instance node and the root node of the BLAS */
   nir_deref_instr *instance_top_node;
   nir_deref_instr *instance_bottom_node;

   /* Information about the current instance used for culling. */
   nir_deref_instr *instance_addr;
   nir_deref_instr *sbt_offset_and_flags;
};

struct radv_ray_traversal_args {
   nir_def *root_bvh_base;
   nir_def *flags;
   nir_def *cull_mask;
   nir_def *origin;
   nir_def *tmin;
   nir_def *dir;

   struct radv_ray_traversal_vars vars;

   /* The increment/decrement used for radv_ray_traversal_vars::stack, and how many entries are
    * available. stack_base is the base address of the stack. */
   uint32_t stack_stride;
   uint32_t stack_entries;
   uint32_t stack_base;

   bool ignore_cull_mask;

   radv_rt_stack_store_cb stack_store_cb;
   radv_rt_stack_load_cb stack_load_cb;

   radv_aabb_intersection_cb aabb_cb;
   radv_triangle_intersection_cb triangle_cb;

   void *data;
};

/* For the initialization of instance_bottom_node. Explicitly different than RADV_BVH_INVALID_NODE
 * or any real node, to ensure we never exit an instance when we're not in one. */
#define RADV_BVH_NO_INSTANCE_ROOT 0xfffffffeu

/* Builds the ray traversal loop and returns whether traversal is incomplete, similar to
 * rayQueryProceedEXT. Traversal will only be considered incomplete, if one of the specified
 * callbacks breaks out of the traversal loop.
 */
nir_def *radv_build_ray_traversal(struct radv_device *device, nir_builder *b,
                                  const struct radv_ray_traversal_args *args);

#endif
