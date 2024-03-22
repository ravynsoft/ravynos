/*
 * Copyright (c) 2021 Intel Corporation
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

#include "brw_nir_rt.h"
#include "brw_nir_rt_builder.h"

#include "nir_deref.h"

#include "util/macros.h"

struct lowering_state {
   const struct intel_device_info *devinfo;

   nir_function_impl *impl;

   struct hash_table *queries;
   uint32_t n_queries;

   struct brw_nir_rt_globals_defs globals;
   nir_def *rq_globals;
};

struct brw_ray_query {
   nir_variable *opaque_var;
   nir_variable *internal_var;
   uint32_t id;
};

#define SIZEOF_QUERY_STATE (sizeof(uint32_t))

static bool
need_spill_fill(struct lowering_state *state)
{
   return state->n_queries > 1;
}

/**
 * This pass converts opaque RayQuery structures from SPIRV into a vec3 where
 * the first 2 elements store a global address for the query and the third
 * element is an incremented counter on the number of executed
 * nir_intrinsic_rq_proceed.
 */

static void
register_opaque_var(nir_variable *opaque_var, struct lowering_state *state)
{
   struct hash_entry *entry = _mesa_hash_table_search(state->queries, opaque_var);
   assert(entry == NULL);

   struct brw_ray_query *rq = rzalloc(state->queries, struct brw_ray_query);
   rq->opaque_var = opaque_var;
   rq->id = state->n_queries;

   unsigned aoa_size = glsl_get_aoa_size(opaque_var->type);
   state->n_queries += MAX2(1, aoa_size);

   _mesa_hash_table_insert(state->queries, opaque_var, rq);
}

static void
create_internal_var(struct brw_ray_query *rq, struct lowering_state *state)
{
   const struct glsl_type *opaque_type = rq->opaque_var->type;
   const struct glsl_type *internal_type = glsl_uint16_t_type();

   while (glsl_type_is_array(opaque_type)) {
      assert(!glsl_type_is_unsized_array(opaque_type));
      internal_type = glsl_array_type(internal_type,
                                      glsl_array_size(opaque_type),
                                      0);
      opaque_type = glsl_get_array_element(opaque_type);
   }

   rq->internal_var = nir_local_variable_create(state->impl,
                                                internal_type,
                                                NULL);
}



static nir_def *
get_ray_query_shadow_addr(nir_builder *b,
                          nir_deref_instr *deref,
                          struct lowering_state *state,
                          nir_deref_instr **out_state_deref)
{
   nir_deref_path path;
   nir_deref_path_init(&path, deref, NULL);
   assert(path.path[0]->deref_type == nir_deref_type_var);

   nir_variable *opaque_var = nir_deref_instr_get_variable(path.path[0]);
   struct hash_entry *entry = _mesa_hash_table_search(state->queries, opaque_var);
   assert(entry);

   struct brw_ray_query *rq = entry->data;

   /* Base address in the shadow memory of the variable associated with this
    * ray query variable.
    */
   nir_def *base_addr =
      nir_iadd_imm(b, state->globals.resume_sbt_addr,
                   brw_rt_ray_queries_shadow_stack_size(state->devinfo) * rq->id);

   bool spill_fill = need_spill_fill(state);
   *out_state_deref = nir_build_deref_var(b, rq->internal_var);

   if (!spill_fill)
      return NULL;

   /* Just emit code and let constant-folding go to town */
   nir_deref_instr **p = &path.path[1];
   for (; *p; p++) {
      if ((*p)->deref_type == nir_deref_type_array) {
         nir_def *index = (*p)->arr.index.ssa;

         /**/
         *out_state_deref = nir_build_deref_array(b, *out_state_deref, index);

         /**/
         uint64_t size = MAX2(1, glsl_get_aoa_size((*p)->type)) *
            brw_rt_ray_queries_shadow_stack_size(state->devinfo);

         nir_def *mul = nir_amul_imm(b, nir_i2i64(b, index), size);

         base_addr = nir_iadd(b, base_addr, mul);
      } else {
         unreachable("Unsupported deref type");
      }
   }

   nir_deref_path_finish(&path);

   /* Add the lane offset to the shadow memory address */
   nir_def *lane_offset =
      nir_imul_imm(
         b,
         nir_iadd(
            b,
            nir_imul(
               b,
               brw_load_btd_dss_id(b),
               brw_nir_rt_load_num_simd_lanes_per_dss(b, state->devinfo)),
            brw_nir_rt_sync_stack_id(b)),
         BRW_RT_SIZEOF_SHADOW_RAY_QUERY);

   return nir_iadd(b, base_addr, nir_i2i64(b, lane_offset));
}

static void
update_trace_ctrl_level(nir_builder *b,
                        nir_deref_instr *state_deref,
                        nir_def **out_old_ctrl,
                        nir_def **out_old_level,
                        nir_def *new_ctrl,
                        nir_def *new_level)
{
   nir_def *old_value = nir_load_deref(b, state_deref);
   nir_def *old_ctrl = nir_ishr_imm(b, old_value, 2);
   nir_def *old_level = nir_iand_imm(b, old_value, 0x3);

   if (out_old_ctrl)
      *out_old_ctrl = old_ctrl;
   if (out_old_level)
      *out_old_level = old_level;

   if (new_ctrl)
      new_ctrl = nir_i2i16(b, new_ctrl);
   if (new_level)
      new_level = nir_i2i16(b, new_level);

   if (new_ctrl || new_level) {
      if (!new_ctrl)
         new_ctrl = old_ctrl;
      if (!new_level)
         new_level = old_level;

      nir_def *new_value = nir_ior(b, nir_ishl_imm(b, new_ctrl, 2), new_level);
      nir_store_deref(b, state_deref, new_value, 0x1);
   }
}

static void
fill_query(nir_builder *b,
           nir_def *hw_stack_addr,
           nir_def *shadow_stack_addr,
           nir_def *ctrl)
{
   brw_nir_memcpy_global(b, hw_stack_addr, 64, shadow_stack_addr, 64,
                         BRW_RT_SIZEOF_RAY_QUERY);
}

static void
spill_query(nir_builder *b,
            nir_def *hw_stack_addr,
            nir_def *shadow_stack_addr)
{
   brw_nir_memcpy_global(b, shadow_stack_addr, 64, hw_stack_addr, 64,
                         BRW_RT_SIZEOF_RAY_QUERY);
}


static void
lower_ray_query_intrinsic(nir_builder *b,
                          nir_intrinsic_instr *intrin,
                          struct lowering_state *state)
{
   nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);

   b->cursor = nir_instr_remove(&intrin->instr);

   nir_deref_instr *ctrl_level_deref;
   nir_def *shadow_stack_addr =
      get_ray_query_shadow_addr(b, deref, state, &ctrl_level_deref);
   nir_def *hw_stack_addr =
      brw_nir_rt_sync_stack_addr(b, state->globals.base_mem_addr, state->devinfo);
   nir_def *stack_addr = shadow_stack_addr ? shadow_stack_addr : hw_stack_addr;

   switch (intrin->intrinsic) {
   case nir_intrinsic_rq_initialize: {
      nir_def *as_addr = intrin->src[1].ssa;
      nir_def *ray_flags = intrin->src[2].ssa;
      /* From the SPIR-V spec:
       *
       *    "Only the 8 least-significant bits of Cull Mask are used by
       *    this instruction - other bits are ignored.
       *
       *    Only the 16 least-significant bits of Miss Index are used by
       *    this instruction - other bits are ignored."
       */
      nir_def *cull_mask = nir_iand_imm(b, intrin->src[3].ssa, 0xff);
      nir_def *ray_orig = intrin->src[4].ssa;
      nir_def *ray_t_min = intrin->src[5].ssa;
      nir_def *ray_dir = intrin->src[6].ssa;
      nir_def *ray_t_max = intrin->src[7].ssa;

      nir_def *root_node_ptr =
         brw_nir_rt_acceleration_structure_to_root_node(b, as_addr);

      struct brw_nir_rt_mem_ray_defs ray_defs = {
         .root_node_ptr = root_node_ptr,
         .ray_flags = nir_u2u16(b, ray_flags),
         .ray_mask = cull_mask,
         .orig = ray_orig,
         .t_near = ray_t_min,
         .dir = ray_dir,
         .t_far = ray_t_max,
      };

      nir_def *ray_addr =
         brw_nir_rt_mem_ray_addr(b, stack_addr, BRW_RT_BVH_LEVEL_WORLD);

      brw_nir_rt_query_mark_init(b, stack_addr);
      brw_nir_rt_store_mem_ray_query_at_addr(b, ray_addr, &ray_defs);

      update_trace_ctrl_level(b, ctrl_level_deref,
                              NULL, NULL,
                              nir_imm_int(b, GEN_RT_TRACE_RAY_INITAL),
                              nir_imm_int(b, BRW_RT_BVH_LEVEL_WORLD));
      break;
   }

   case nir_intrinsic_rq_proceed: {
      nir_def *not_done =
         nir_inot(b, brw_nir_rt_query_done(b, stack_addr));
      nir_def *not_done_then, *not_done_else;

      nir_push_if(b, not_done);
      {
         nir_def *ctrl, *level;
         update_trace_ctrl_level(b, ctrl_level_deref,
                                 &ctrl, &level,
                                 NULL,
                                 NULL);

         /* Mark the query as done because handing it over to the HW for
          * processing. If the HW make any progress, it will write back some
          * data and as a side effect, clear the "done" bit. If no progress is
          * made, HW does not write anything back and we can use this bit to
          * detect that.
          */
         brw_nir_rt_query_mark_done(b, stack_addr);

         if (shadow_stack_addr)
            fill_query(b, hw_stack_addr, shadow_stack_addr, ctrl);

         nir_trace_ray_intel(b, state->rq_globals, level, ctrl, .synchronous = true);

         struct brw_nir_rt_mem_hit_defs hit_in = {};
         brw_nir_rt_load_mem_hit_from_addr(b, &hit_in, hw_stack_addr, false);

         if (shadow_stack_addr)
            spill_query(b, hw_stack_addr, shadow_stack_addr);

         update_trace_ctrl_level(b, ctrl_level_deref,
                                 NULL, NULL,
                                 nir_imm_int(b, GEN_RT_TRACE_RAY_CONTINUE),
                                 hit_in.bvh_level);

         not_done_then = nir_inot(b, hit_in.done);
      }
      nir_push_else(b, NULL);
      {
         not_done_else = nir_imm_false(b);
      }
      nir_pop_if(b, NULL);
      not_done = nir_if_phi(b, not_done_then, not_done_else);
      nir_def_rewrite_uses(&intrin->def, not_done);
      break;
   }

   case nir_intrinsic_rq_confirm_intersection: {
      brw_nir_memcpy_global(b,
                            brw_nir_rt_mem_hit_addr_from_addr(b, stack_addr, true), 16,
                            brw_nir_rt_mem_hit_addr_from_addr(b, stack_addr, false), 16,
                            BRW_RT_SIZEOF_HIT_INFO);
      update_trace_ctrl_level(b, ctrl_level_deref,
                              NULL, NULL,
                              nir_imm_int(b, GEN_RT_TRACE_RAY_COMMIT),
                              nir_imm_int(b, BRW_RT_BVH_LEVEL_OBJECT));
      break;
   }

   case nir_intrinsic_rq_generate_intersection: {
      brw_nir_rt_generate_hit_addr(b, stack_addr, intrin->src[1].ssa);
      update_trace_ctrl_level(b, ctrl_level_deref,
                              NULL, NULL,
                              nir_imm_int(b, GEN_RT_TRACE_RAY_COMMIT),
                              nir_imm_int(b, BRW_RT_BVH_LEVEL_OBJECT));
      break;
   }

   case nir_intrinsic_rq_terminate: {
      brw_nir_rt_query_mark_done(b, stack_addr);
      break;
   }

   case nir_intrinsic_rq_load: {
      const bool committed = nir_intrinsic_committed(intrin);

      struct brw_nir_rt_mem_ray_defs world_ray_in = {};
      struct brw_nir_rt_mem_ray_defs object_ray_in = {};
      struct brw_nir_rt_mem_hit_defs hit_in = {};
      brw_nir_rt_load_mem_ray_from_addr(b, &world_ray_in, stack_addr,
                                        BRW_RT_BVH_LEVEL_WORLD);
      brw_nir_rt_load_mem_ray_from_addr(b, &object_ray_in, stack_addr,
                                        BRW_RT_BVH_LEVEL_OBJECT);
      brw_nir_rt_load_mem_hit_from_addr(b, &hit_in, stack_addr, committed);

      nir_def *sysval = NULL;
      switch (nir_intrinsic_ray_query_value(intrin)) {
      case nir_ray_query_value_intersection_type:
         if (committed) {
            /* Values we want to generate :
             *
             * RayQueryCommittedIntersectionNoneEXT = 0U        <= hit_in.valid == false
             * RayQueryCommittedIntersectionTriangleEXT = 1U    <= hit_in.leaf_type == BRW_RT_BVH_NODE_TYPE_QUAD (4)
             * RayQueryCommittedIntersectionGeneratedEXT = 2U   <= hit_in.leaf_type == BRW_RT_BVH_NODE_TYPE_PROCEDURAL (3)
             */
            sysval =
               nir_bcsel(b, nir_ieq_imm(b, hit_in.leaf_type, 4),
                         nir_imm_int(b, 1), nir_imm_int(b, 2));
            sysval =
               nir_bcsel(b, hit_in.valid,
                         sysval, nir_imm_int(b, 0));
         } else {
            /* 0 -> triangle, 1 -> AABB */
            sysval =
               nir_b2i32(b,
                         nir_ieq_imm(b, hit_in.leaf_type,
                                     BRW_RT_BVH_NODE_TYPE_PROCEDURAL));
         }
         break;

      case nir_ray_query_value_intersection_t:
         sysval = hit_in.t;
         break;

      case nir_ray_query_value_intersection_instance_custom_index: {
         struct brw_nir_rt_bvh_instance_leaf_defs leaf;
         brw_nir_rt_load_bvh_instance_leaf(b, &leaf, hit_in.inst_leaf_ptr);
         sysval = leaf.instance_id;
         break;
      }

      case nir_ray_query_value_intersection_instance_id: {
         struct brw_nir_rt_bvh_instance_leaf_defs leaf;
         brw_nir_rt_load_bvh_instance_leaf(b, &leaf, hit_in.inst_leaf_ptr);
         sysval = leaf.instance_index;
         break;
      }

      case nir_ray_query_value_intersection_instance_sbt_index: {
         struct brw_nir_rt_bvh_instance_leaf_defs leaf;
         brw_nir_rt_load_bvh_instance_leaf(b, &leaf, hit_in.inst_leaf_ptr);
         sysval = leaf.contribution_to_hit_group_index;
         break;
      }

      case nir_ray_query_value_intersection_geometry_index: {
         nir_def *geometry_index_dw =
            nir_load_global(b, nir_iadd_imm(b, hit_in.prim_leaf_ptr, 4), 4,
                            1, 32);
         sysval = nir_iand_imm(b, geometry_index_dw, BITFIELD_MASK(29));
         break;
      }

      case nir_ray_query_value_intersection_primitive_index:
         sysval = brw_nir_rt_load_primitive_id_from_hit(b, NULL /* is_procedural */, &hit_in);
         break;

      case nir_ray_query_value_intersection_barycentrics:
         sysval = hit_in.tri_bary;
         break;

      case nir_ray_query_value_intersection_front_face:
         sysval = hit_in.front_face;
         break;

      case nir_ray_query_value_intersection_object_ray_direction:
         sysval = world_ray_in.dir;
         break;

      case nir_ray_query_value_intersection_object_ray_origin:
         sysval = world_ray_in.orig;
         break;

      case nir_ray_query_value_intersection_object_to_world: {
         struct brw_nir_rt_bvh_instance_leaf_defs leaf;
         brw_nir_rt_load_bvh_instance_leaf(b, &leaf, hit_in.inst_leaf_ptr);
         sysval = leaf.object_to_world[nir_intrinsic_column(intrin)];
         break;
      }

      case nir_ray_query_value_intersection_world_to_object: {
         struct brw_nir_rt_bvh_instance_leaf_defs leaf;
         brw_nir_rt_load_bvh_instance_leaf(b, &leaf, hit_in.inst_leaf_ptr);
         sysval = leaf.world_to_object[nir_intrinsic_column(intrin)];
         break;
      }

      case nir_ray_query_value_intersection_candidate_aabb_opaque:
         sysval = hit_in.front_face;
         break;

      case nir_ray_query_value_tmin:
         sysval = world_ray_in.t_near;
         break;

      case nir_ray_query_value_flags:
         sysval = nir_u2u32(b, world_ray_in.ray_flags);
         break;

      case nir_ray_query_value_world_ray_direction:
         sysval = world_ray_in.dir;
         break;

      case nir_ray_query_value_world_ray_origin:
         sysval = world_ray_in.orig;
         break;

      case nir_ray_query_value_intersection_triangle_vertex_positions: {
         struct brw_nir_rt_bvh_primitive_leaf_positions_defs pos;
         brw_nir_rt_load_bvh_primitive_leaf_positions(b, &pos, hit_in.prim_leaf_ptr);
         sysval = pos.positions[nir_intrinsic_column(intrin)];
         break;
      }

      default:
         unreachable("Invalid ray query");
      }

      assert(sysval);
      nir_def_rewrite_uses(&intrin->def, sysval);
      break;
   }

   default:
      unreachable("Invalid intrinsic");
   }
}

static void
lower_ray_query_impl(nir_function_impl *impl, struct lowering_state *state)
{
   nir_builder _b, *b = &_b;
   _b = nir_builder_at(nir_before_impl(impl));

   state->rq_globals = nir_load_ray_query_global_intel(b);

   brw_nir_rt_load_globals_addr(b, &state->globals, state->rq_globals);

   nir_foreach_block_safe(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic != nir_intrinsic_rq_initialize &&
             intrin->intrinsic != nir_intrinsic_rq_terminate &&
             intrin->intrinsic != nir_intrinsic_rq_proceed &&
             intrin->intrinsic != nir_intrinsic_rq_generate_intersection &&
             intrin->intrinsic != nir_intrinsic_rq_confirm_intersection &&
             intrin->intrinsic != nir_intrinsic_rq_load)
            continue;

         lower_ray_query_intrinsic(b, intrin, state);
      }
   }

   nir_metadata_preserve(impl, nir_metadata_none);
}

bool
brw_nir_lower_ray_queries(nir_shader *shader,
                          const struct intel_device_info *devinfo)
{
   assert(exec_list_length(&shader->functions) == 1);

   struct lowering_state state = {
      .devinfo = devinfo,
      .impl = nir_shader_get_entrypoint(shader),
      .queries = _mesa_pointer_hash_table_create(NULL),
   };

   /* Map all query variable to internal type variables */
   nir_foreach_function_temp_variable(var, state.impl) {
      if (!var->data.ray_query)
         continue;
      register_opaque_var(var, &state);
   }
   hash_table_foreach(state.queries, entry)
      create_internal_var(entry->data, &state);

   bool progress = state.n_queries > 0;

   if (progress) {
      lower_ray_query_impl(state.impl, &state);

      nir_remove_dead_derefs(shader);
      nir_remove_dead_variables(shader,
                                nir_var_shader_temp | nir_var_function_temp,
                                NULL);

      nir_metadata_preserve(state.impl, nir_metadata_none);
   }

   ralloc_free(state.queries);

   return progress;
}
