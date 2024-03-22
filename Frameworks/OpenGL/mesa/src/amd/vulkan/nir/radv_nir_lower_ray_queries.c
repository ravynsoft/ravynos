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

#include "nir/nir.h"
#include "nir/nir_builder.h"

#include "util/hash_table.h"

#include "bvh/bvh.h"
#include "nir/radv_nir_rt_common.h"
#include "radv_debug.h"
#include "radv_nir.h"
#include "radv_private.h"
#include "radv_shader.h"

/* Traversal stack size. Traversal supports backtracking so we can go deeper than this size if
 * needed. However, we keep a large stack size to avoid it being put into registers, which hurts
 * occupancy. */
#define MAX_SCRATCH_STACK_ENTRY_COUNT 76

typedef struct {
   nir_variable *variable;
   unsigned array_length;
} rq_variable;

static rq_variable *
rq_variable_create(void *ctx, nir_shader *shader, unsigned array_length, const struct glsl_type *type, const char *name)
{
   rq_variable *result = ralloc(ctx, rq_variable);
   result->array_length = array_length;

   const struct glsl_type *variable_type = type;
   if (array_length != 1)
      variable_type = glsl_array_type(type, array_length, glsl_get_explicit_stride(type));

   result->variable = nir_variable_create(shader, nir_var_shader_temp, variable_type, name);

   return result;
}

static nir_def *
nir_load_array(nir_builder *b, nir_variable *array, nir_def *index)
{
   return nir_load_deref(b, nir_build_deref_array(b, nir_build_deref_var(b, array), index));
}

static void
nir_store_array(nir_builder *b, nir_variable *array, nir_def *index, nir_def *value, unsigned writemask)
{
   nir_store_deref(b, nir_build_deref_array(b, nir_build_deref_var(b, array), index), value, writemask);
}

static nir_deref_instr *
rq_deref_var(nir_builder *b, nir_def *index, rq_variable *var)
{
   if (var->array_length == 1)
      return nir_build_deref_var(b, var->variable);

   return nir_build_deref_array(b, nir_build_deref_var(b, var->variable), index);
}

static nir_def *
rq_load_var(nir_builder *b, nir_def *index, rq_variable *var)
{
   if (var->array_length == 1)
      return nir_load_var(b, var->variable);

   return nir_load_array(b, var->variable, index);
}

static void
rq_store_var(nir_builder *b, nir_def *index, rq_variable *var, nir_def *value, unsigned writemask)
{
   if (var->array_length == 1) {
      nir_store_var(b, var->variable, value, writemask);
   } else {
      nir_store_array(b, var->variable, index, value, writemask);
   }
}

static void
rq_copy_var(nir_builder *b, nir_def *index, rq_variable *dst, rq_variable *src, unsigned mask)
{
   rq_store_var(b, index, dst, rq_load_var(b, index, src), mask);
}

static nir_def *
rq_load_array(nir_builder *b, nir_def *index, rq_variable *var, nir_def *array_index)
{
   if (var->array_length == 1)
      return nir_load_array(b, var->variable, array_index);

   return nir_load_deref(
      b, nir_build_deref_array(b, nir_build_deref_array(b, nir_build_deref_var(b, var->variable), index), array_index));
}

static void
rq_store_array(nir_builder *b, nir_def *index, rq_variable *var, nir_def *array_index, nir_def *value,
               unsigned writemask)
{
   if (var->array_length == 1) {
      nir_store_array(b, var->variable, array_index, value, writemask);
   } else {
      nir_store_deref(
         b,
         nir_build_deref_array(b, nir_build_deref_array(b, nir_build_deref_var(b, var->variable), index), array_index),
         value, writemask);
   }
}

struct ray_query_traversal_vars {
   rq_variable *origin;
   rq_variable *direction;

   rq_variable *bvh_base;
   rq_variable *stack;
   rq_variable *top_stack;
   rq_variable *stack_low_watermark;
   rq_variable *current_node;
   rq_variable *previous_node;
   rq_variable *instance_top_node;
   rq_variable *instance_bottom_node;
};

struct ray_query_intersection_vars {
   rq_variable *primitive_id;
   rq_variable *geometry_id_and_flags;
   rq_variable *instance_addr;
   rq_variable *intersection_type;
   rq_variable *opaque;
   rq_variable *frontface;
   rq_variable *sbt_offset_and_flags;
   rq_variable *barycentrics;
   rq_variable *t;
};

struct ray_query_vars {
   rq_variable *root_bvh_base;
   rq_variable *flags;
   rq_variable *cull_mask;
   rq_variable *origin;
   rq_variable *tmin;
   rq_variable *direction;

   rq_variable *incomplete;

   struct ray_query_intersection_vars closest;
   struct ray_query_intersection_vars candidate;

   struct ray_query_traversal_vars trav;

   rq_variable *stack;
   uint32_t shared_base;
   uint32_t stack_entries;

   nir_intrinsic_instr *initialize;
};

#define VAR_NAME(name) strcat(strcpy(ralloc_size(ctx, strlen(base_name) + strlen(name) + 1), base_name), name)

static struct ray_query_traversal_vars
init_ray_query_traversal_vars(void *ctx, nir_shader *shader, unsigned array_length, const char *base_name)
{
   struct ray_query_traversal_vars result;

   const struct glsl_type *vec3_type = glsl_vector_type(GLSL_TYPE_FLOAT, 3);

   result.origin = rq_variable_create(ctx, shader, array_length, vec3_type, VAR_NAME("_origin"));
   result.direction = rq_variable_create(ctx, shader, array_length, vec3_type, VAR_NAME("_direction"));

   result.bvh_base = rq_variable_create(ctx, shader, array_length, glsl_uint64_t_type(), VAR_NAME("_bvh_base"));
   result.stack = rq_variable_create(ctx, shader, array_length, glsl_uint_type(), VAR_NAME("_stack"));
   result.top_stack = rq_variable_create(ctx, shader, array_length, glsl_uint_type(), VAR_NAME("_top_stack"));
   result.stack_low_watermark =
      rq_variable_create(ctx, shader, array_length, glsl_uint_type(), VAR_NAME("_stack_low_watermark"));
   result.current_node = rq_variable_create(ctx, shader, array_length, glsl_uint_type(), VAR_NAME("_current_node"));
   result.previous_node = rq_variable_create(ctx, shader, array_length, glsl_uint_type(), VAR_NAME("_previous_node"));
   result.instance_top_node =
      rq_variable_create(ctx, shader, array_length, glsl_uint_type(), VAR_NAME("_instance_top_node"));
   result.instance_bottom_node =
      rq_variable_create(ctx, shader, array_length, glsl_uint_type(), VAR_NAME("_instance_bottom_node"));
   return result;
}

static struct ray_query_intersection_vars
init_ray_query_intersection_vars(void *ctx, nir_shader *shader, unsigned array_length, const char *base_name)
{
   struct ray_query_intersection_vars result;

   const struct glsl_type *vec2_type = glsl_vector_type(GLSL_TYPE_FLOAT, 2);

   result.primitive_id = rq_variable_create(ctx, shader, array_length, glsl_uint_type(), VAR_NAME("_primitive_id"));
   result.geometry_id_and_flags =
      rq_variable_create(ctx, shader, array_length, glsl_uint_type(), VAR_NAME("_geometry_id_and_flags"));
   result.instance_addr =
      rq_variable_create(ctx, shader, array_length, glsl_uint64_t_type(), VAR_NAME("_instance_addr"));
   result.intersection_type =
      rq_variable_create(ctx, shader, array_length, glsl_uint_type(), VAR_NAME("_intersection_type"));
   result.opaque = rq_variable_create(ctx, shader, array_length, glsl_bool_type(), VAR_NAME("_opaque"));
   result.frontface = rq_variable_create(ctx, shader, array_length, glsl_bool_type(), VAR_NAME("_frontface"));
   result.sbt_offset_and_flags =
      rq_variable_create(ctx, shader, array_length, glsl_uint_type(), VAR_NAME("_sbt_offset_and_flags"));
   result.barycentrics = rq_variable_create(ctx, shader, array_length, vec2_type, VAR_NAME("_barycentrics"));
   result.t = rq_variable_create(ctx, shader, array_length, glsl_float_type(), VAR_NAME("_t"));

   return result;
}

static void
init_ray_query_vars(nir_shader *shader, unsigned array_length, struct ray_query_vars *dst, const char *base_name,
                    uint32_t max_shared_size)
{
   void *ctx = dst;
   const struct glsl_type *vec3_type = glsl_vector_type(GLSL_TYPE_FLOAT, 3);

   dst->root_bvh_base = rq_variable_create(dst, shader, array_length, glsl_uint64_t_type(), VAR_NAME("_root_bvh_base"));
   dst->flags = rq_variable_create(dst, shader, array_length, glsl_uint_type(), VAR_NAME("_flags"));
   dst->cull_mask = rq_variable_create(dst, shader, array_length, glsl_uint_type(), VAR_NAME("_cull_mask"));
   dst->origin = rq_variable_create(dst, shader, array_length, vec3_type, VAR_NAME("_origin"));
   dst->tmin = rq_variable_create(dst, shader, array_length, glsl_float_type(), VAR_NAME("_tmin"));
   dst->direction = rq_variable_create(dst, shader, array_length, vec3_type, VAR_NAME("_direction"));

   dst->incomplete = rq_variable_create(dst, shader, array_length, glsl_bool_type(), VAR_NAME("_incomplete"));

   dst->closest = init_ray_query_intersection_vars(dst, shader, array_length, VAR_NAME("_closest"));
   dst->candidate = init_ray_query_intersection_vars(dst, shader, array_length, VAR_NAME("_candidate"));

   dst->trav = init_ray_query_traversal_vars(dst, shader, array_length, VAR_NAME("_top"));

   uint32_t workgroup_size =
      shader->info.workgroup_size[0] * shader->info.workgroup_size[1] * shader->info.workgroup_size[2];
   uint32_t shared_stack_entries = shader->info.ray_queries == 1 ? 16 : 8;
   uint32_t shared_stack_size = workgroup_size * shared_stack_entries * 4;
   uint32_t shared_offset = align(shader->info.shared_size, 4);
   if (shader->info.stage != MESA_SHADER_COMPUTE || array_length > 1 ||
       shared_offset + shared_stack_size > max_shared_size) {
      dst->stack =
         rq_variable_create(dst, shader, array_length,
                            glsl_array_type(glsl_uint_type(), MAX_SCRATCH_STACK_ENTRY_COUNT, 0), VAR_NAME("_stack"));
      dst->stack_entries = MAX_SCRATCH_STACK_ENTRY_COUNT;
   } else {
      dst->stack = NULL;
      dst->shared_base = shared_offset;
      dst->stack_entries = shared_stack_entries;

      shader->info.shared_size = shared_offset + shared_stack_size;
   }
}

#undef VAR_NAME

static void
lower_ray_query(nir_shader *shader, nir_variable *ray_query, struct hash_table *ht, uint32_t max_shared_size)
{
   struct ray_query_vars *vars = ralloc(ht, struct ray_query_vars);

   unsigned array_length = 1;
   if (glsl_type_is_array(ray_query->type))
      array_length = glsl_get_length(ray_query->type);

   init_ray_query_vars(shader, array_length, vars, ray_query->name == NULL ? "" : ray_query->name, max_shared_size);

   _mesa_hash_table_insert(ht, ray_query, vars);
}

static void
copy_candidate_to_closest(nir_builder *b, nir_def *index, struct ray_query_vars *vars)
{
   rq_copy_var(b, index, vars->closest.barycentrics, vars->candidate.barycentrics, 0x3);
   rq_copy_var(b, index, vars->closest.geometry_id_and_flags, vars->candidate.geometry_id_and_flags, 0x1);
   rq_copy_var(b, index, vars->closest.instance_addr, vars->candidate.instance_addr, 0x1);
   rq_copy_var(b, index, vars->closest.intersection_type, vars->candidate.intersection_type, 0x1);
   rq_copy_var(b, index, vars->closest.opaque, vars->candidate.opaque, 0x1);
   rq_copy_var(b, index, vars->closest.frontface, vars->candidate.frontface, 0x1);
   rq_copy_var(b, index, vars->closest.sbt_offset_and_flags, vars->candidate.sbt_offset_and_flags, 0x1);
   rq_copy_var(b, index, vars->closest.primitive_id, vars->candidate.primitive_id, 0x1);
   rq_copy_var(b, index, vars->closest.t, vars->candidate.t, 0x1);
}

static void
insert_terminate_on_first_hit(nir_builder *b, nir_def *index, struct ray_query_vars *vars,
                              const struct radv_ray_flags *ray_flags, bool break_on_terminate)
{
   nir_def *terminate_on_first_hit;
   if (ray_flags)
      terminate_on_first_hit = ray_flags->terminate_on_first_hit;
   else
      terminate_on_first_hit =
         nir_test_mask(b, rq_load_var(b, index, vars->flags), SpvRayFlagsTerminateOnFirstHitKHRMask);
   nir_push_if(b, terminate_on_first_hit);
   {
      rq_store_var(b, index, vars->incomplete, nir_imm_false(b), 0x1);
      if (break_on_terminate)
         nir_jump(b, nir_jump_break);
   }
   nir_pop_if(b, NULL);
}

static void
lower_rq_confirm_intersection(nir_builder *b, nir_def *index, nir_intrinsic_instr *instr, struct ray_query_vars *vars)
{
   copy_candidate_to_closest(b, index, vars);
   insert_terminate_on_first_hit(b, index, vars, NULL, false);
}

static void
lower_rq_generate_intersection(nir_builder *b, nir_def *index, nir_intrinsic_instr *instr, struct ray_query_vars *vars)
{
   nir_push_if(b, nir_iand(b, nir_fge(b, rq_load_var(b, index, vars->closest.t), instr->src[1].ssa),
                           nir_fge(b, instr->src[1].ssa, rq_load_var(b, index, vars->tmin))));
   {
      copy_candidate_to_closest(b, index, vars);
      insert_terminate_on_first_hit(b, index, vars, NULL, false);
      rq_store_var(b, index, vars->closest.t, instr->src[1].ssa, 0x1);
   }
   nir_pop_if(b, NULL);
}

enum rq_intersection_type { intersection_type_none, intersection_type_triangle, intersection_type_aabb };

static void
lower_rq_initialize(nir_builder *b, nir_def *index, nir_intrinsic_instr *instr, struct ray_query_vars *vars,
                    struct radv_instance *instance)
{
   rq_store_var(b, index, vars->flags, instr->src[2].ssa, 0x1);
   rq_store_var(b, index, vars->cull_mask, nir_ishl_imm(b, instr->src[3].ssa, 24), 0x1);

   rq_store_var(b, index, vars->origin, instr->src[4].ssa, 0x7);
   rq_store_var(b, index, vars->trav.origin, instr->src[4].ssa, 0x7);

   rq_store_var(b, index, vars->tmin, instr->src[5].ssa, 0x1);

   rq_store_var(b, index, vars->direction, instr->src[6].ssa, 0x7);
   rq_store_var(b, index, vars->trav.direction, instr->src[6].ssa, 0x7);

   rq_store_var(b, index, vars->closest.t, instr->src[7].ssa, 0x1);
   rq_store_var(b, index, vars->closest.intersection_type, nir_imm_int(b, intersection_type_none), 0x1);

   nir_def *accel_struct = instr->src[1].ssa;

   /* Make sure that instance data loads don't hang in case of a miss by setting a valid initial address. */
   rq_store_var(b, index, vars->closest.instance_addr, accel_struct, 1);
   rq_store_var(b, index, vars->candidate.instance_addr, accel_struct, 1);

   nir_def *bvh_offset = nir_build_load_global(
      b, 1, 32, nir_iadd_imm(b, accel_struct, offsetof(struct radv_accel_struct_header, bvh_offset)),
      .access = ACCESS_NON_WRITEABLE);
   nir_def *bvh_base = nir_iadd(b, accel_struct, nir_u2u64(b, bvh_offset));
   bvh_base = build_addr_to_node(b, bvh_base);

   rq_store_var(b, index, vars->root_bvh_base, bvh_base, 0x1);
   rq_store_var(b, index, vars->trav.bvh_base, bvh_base, 1);

   if (vars->stack) {
      rq_store_var(b, index, vars->trav.stack, nir_imm_int(b, 0), 0x1);
      rq_store_var(b, index, vars->trav.stack_low_watermark, nir_imm_int(b, 0), 0x1);
   } else {
      nir_def *base_offset = nir_imul_imm(b, nir_load_local_invocation_index(b), sizeof(uint32_t));
      base_offset = nir_iadd_imm(b, base_offset, vars->shared_base);
      rq_store_var(b, index, vars->trav.stack, base_offset, 0x1);
      rq_store_var(b, index, vars->trav.stack_low_watermark, base_offset, 0x1);
   }

   rq_store_var(b, index, vars->trav.current_node, nir_imm_int(b, RADV_BVH_ROOT_NODE), 0x1);
   rq_store_var(b, index, vars->trav.previous_node, nir_imm_int(b, RADV_BVH_INVALID_NODE), 0x1);
   rq_store_var(b, index, vars->trav.instance_top_node, nir_imm_int(b, RADV_BVH_INVALID_NODE), 0x1);
   rq_store_var(b, index, vars->trav.instance_bottom_node, nir_imm_int(b, RADV_BVH_NO_INSTANCE_ROOT), 0x1);

   rq_store_var(b, index, vars->trav.top_stack, nir_imm_int(b, -1), 1);

   rq_store_var(b, index, vars->incomplete, nir_imm_bool(b, !(instance->debug_flags & RADV_DEBUG_NO_RT)), 0x1);

   vars->initialize = instr;
}

static nir_def *
lower_rq_load(struct radv_device *device, nir_builder *b, nir_def *index, nir_intrinsic_instr *instr,
              struct ray_query_vars *vars)
{
   bool committed = nir_intrinsic_committed(instr);
   struct ray_query_intersection_vars *intersection = committed ? &vars->closest : &vars->candidate;

   uint32_t column = nir_intrinsic_column(instr);

   nir_ray_query_value value = nir_intrinsic_ray_query_value(instr);
   switch (value) {
   case nir_ray_query_value_flags:
      return rq_load_var(b, index, vars->flags);
   case nir_ray_query_value_intersection_barycentrics:
      return rq_load_var(b, index, intersection->barycentrics);
   case nir_ray_query_value_intersection_candidate_aabb_opaque:
      return nir_iand(b, rq_load_var(b, index, vars->candidate.opaque),
                      nir_ieq_imm(b, rq_load_var(b, index, vars->candidate.intersection_type), intersection_type_aabb));
   case nir_ray_query_value_intersection_front_face:
      return rq_load_var(b, index, intersection->frontface);
   case nir_ray_query_value_intersection_geometry_index:
      return nir_iand_imm(b, rq_load_var(b, index, intersection->geometry_id_and_flags), 0xFFFFFF);
   case nir_ray_query_value_intersection_instance_custom_index: {
      nir_def *instance_node_addr = rq_load_var(b, index, intersection->instance_addr);
      return nir_iand_imm(
         b,
         nir_build_load_global(
            b, 1, 32,
            nir_iadd_imm(b, instance_node_addr, offsetof(struct radv_bvh_instance_node, custom_instance_and_mask))),
         0xFFFFFF);
   }
   case nir_ray_query_value_intersection_instance_id: {
      nir_def *instance_node_addr = rq_load_var(b, index, intersection->instance_addr);
      return nir_build_load_global(
         b, 1, 32, nir_iadd_imm(b, instance_node_addr, offsetof(struct radv_bvh_instance_node, instance_id)));
   }
   case nir_ray_query_value_intersection_instance_sbt_index:
      return nir_iand_imm(b, rq_load_var(b, index, intersection->sbt_offset_and_flags), 0xFFFFFF);
   case nir_ray_query_value_intersection_object_ray_direction: {
      nir_def *instance_node_addr = rq_load_var(b, index, intersection->instance_addr);
      nir_def *wto_matrix[3];
      nir_build_wto_matrix_load(b, instance_node_addr, wto_matrix);
      return nir_build_vec3_mat_mult(b, rq_load_var(b, index, vars->direction), wto_matrix, false);
   }
   case nir_ray_query_value_intersection_object_ray_origin: {
      nir_def *instance_node_addr = rq_load_var(b, index, intersection->instance_addr);
      nir_def *wto_matrix[3];
      nir_build_wto_matrix_load(b, instance_node_addr, wto_matrix);
      return nir_build_vec3_mat_mult(b, rq_load_var(b, index, vars->origin), wto_matrix, true);
   }
   case nir_ray_query_value_intersection_object_to_world: {
      nir_def *instance_node_addr = rq_load_var(b, index, intersection->instance_addr);
      nir_def *rows[3];
      for (unsigned r = 0; r < 3; ++r)
         rows[r] = nir_build_load_global(
            b, 4, 32,
            nir_iadd_imm(b, instance_node_addr, offsetof(struct radv_bvh_instance_node, otw_matrix) + r * 16));

      return nir_vec3(b, nir_channel(b, rows[0], column), nir_channel(b, rows[1], column),
                      nir_channel(b, rows[2], column));
   }
   case nir_ray_query_value_intersection_primitive_index:
      return rq_load_var(b, index, intersection->primitive_id);
   case nir_ray_query_value_intersection_t:
      return rq_load_var(b, index, intersection->t);
   case nir_ray_query_value_intersection_type: {
      nir_def *intersection_type = rq_load_var(b, index, intersection->intersection_type);
      if (!committed)
         intersection_type = nir_iadd_imm(b, intersection_type, -1);

      return intersection_type;
   }
   case nir_ray_query_value_intersection_world_to_object: {
      nir_def *instance_node_addr = rq_load_var(b, index, intersection->instance_addr);

      nir_def *wto_matrix[3];
      nir_build_wto_matrix_load(b, instance_node_addr, wto_matrix);

      nir_def *vals[3];
      for (unsigned i = 0; i < 3; ++i)
         vals[i] = nir_channel(b, wto_matrix[i], column);

      return nir_vec(b, vals, 3);
   }
   case nir_ray_query_value_tmin:
      return rq_load_var(b, index, vars->tmin);
   case nir_ray_query_value_world_ray_direction:
      return rq_load_var(b, index, vars->direction);
   case nir_ray_query_value_world_ray_origin:
      return rq_load_var(b, index, vars->origin);
   case nir_ray_query_value_intersection_triangle_vertex_positions: {
      nir_def *instance_node_addr = rq_load_var(b, index, intersection->instance_addr);
      nir_def *primitive_id = rq_load_var(b, index, intersection->primitive_id);
      return radv_load_vertex_position(device, b, instance_node_addr, primitive_id, nir_intrinsic_column(instr));
   }
   default:
      unreachable("Invalid nir_ray_query_value!");
   }

   return NULL;
}

struct traversal_data {
   struct ray_query_vars *vars;
   nir_def *index;
};

static void
handle_candidate_aabb(nir_builder *b, struct radv_leaf_intersection *intersection,
                      const struct radv_ray_traversal_args *args)
{
   struct traversal_data *data = args->data;
   struct ray_query_vars *vars = data->vars;
   nir_def *index = data->index;

   rq_store_var(b, index, vars->candidate.primitive_id, intersection->primitive_id, 1);
   rq_store_var(b, index, vars->candidate.geometry_id_and_flags, intersection->geometry_id_and_flags, 1);
   rq_store_var(b, index, vars->candidate.opaque, intersection->opaque, 0x1);
   rq_store_var(b, index, vars->candidate.intersection_type, nir_imm_int(b, intersection_type_aabb), 0x1);

   nir_jump(b, nir_jump_break);
}

static void
handle_candidate_triangle(nir_builder *b, struct radv_triangle_intersection *intersection,
                          const struct radv_ray_traversal_args *args, const struct radv_ray_flags *ray_flags)
{
   struct traversal_data *data = args->data;
   struct ray_query_vars *vars = data->vars;
   nir_def *index = data->index;

   rq_store_var(b, index, vars->candidate.barycentrics, intersection->barycentrics, 3);
   rq_store_var(b, index, vars->candidate.primitive_id, intersection->base.primitive_id, 1);
   rq_store_var(b, index, vars->candidate.geometry_id_and_flags, intersection->base.geometry_id_and_flags, 1);
   rq_store_var(b, index, vars->candidate.t, intersection->t, 0x1);
   rq_store_var(b, index, vars->candidate.opaque, intersection->base.opaque, 0x1);
   rq_store_var(b, index, vars->candidate.frontface, intersection->frontface, 0x1);
   rq_store_var(b, index, vars->candidate.intersection_type, nir_imm_int(b, intersection_type_triangle), 0x1);

   nir_push_if(b, intersection->base.opaque);
   {
      copy_candidate_to_closest(b, index, vars);
      insert_terminate_on_first_hit(b, index, vars, ray_flags, true);
   }
   nir_push_else(b, NULL);
   {
      nir_jump(b, nir_jump_break);
   }
   nir_pop_if(b, NULL);
}

static void
store_stack_entry(nir_builder *b, nir_def *index, nir_def *value, const struct radv_ray_traversal_args *args)
{
   struct traversal_data *data = args->data;
   if (data->vars->stack)
      rq_store_array(b, data->index, data->vars->stack, index, value, 1);
   else
      nir_store_shared(b, value, index, .base = 0, .align_mul = 4);
}

static nir_def *
load_stack_entry(nir_builder *b, nir_def *index, const struct radv_ray_traversal_args *args)
{
   struct traversal_data *data = args->data;
   if (data->vars->stack)
      return rq_load_array(b, data->index, data->vars->stack, index);
   else
      return nir_load_shared(b, 1, 32, index, .base = 0, .align_mul = 4);
}

static nir_def *
lower_rq_proceed(nir_builder *b, nir_def *index, nir_intrinsic_instr *instr, struct ray_query_vars *vars,
                 struct radv_device *device)
{
   nir_metadata_require(nir_cf_node_get_function(&instr->instr.block->cf_node), nir_metadata_dominance);

   bool ignore_cull_mask = false;
   if (nir_block_dominates(vars->initialize->instr.block, instr->instr.block)) {
      nir_src cull_mask = vars->initialize->src[3];
      if (nir_src_is_const(cull_mask) && nir_src_as_uint(cull_mask) == 0xFF)
         ignore_cull_mask = true;
   }

   nir_variable *inv_dir = nir_local_variable_create(b->impl, glsl_vector_type(GLSL_TYPE_FLOAT, 3), "inv_dir");
   nir_store_var(b, inv_dir, nir_frcp(b, rq_load_var(b, index, vars->trav.direction)), 0x7);

   struct radv_ray_traversal_vars trav_vars = {
      .tmax = rq_deref_var(b, index, vars->closest.t),
      .origin = rq_deref_var(b, index, vars->trav.origin),
      .dir = rq_deref_var(b, index, vars->trav.direction),
      .inv_dir = nir_build_deref_var(b, inv_dir),
      .bvh_base = rq_deref_var(b, index, vars->trav.bvh_base),
      .stack = rq_deref_var(b, index, vars->trav.stack),
      .top_stack = rq_deref_var(b, index, vars->trav.top_stack),
      .stack_low_watermark = rq_deref_var(b, index, vars->trav.stack_low_watermark),
      .current_node = rq_deref_var(b, index, vars->trav.current_node),
      .previous_node = rq_deref_var(b, index, vars->trav.previous_node),
      .instance_top_node = rq_deref_var(b, index, vars->trav.instance_top_node),
      .instance_bottom_node = rq_deref_var(b, index, vars->trav.instance_bottom_node),
      .instance_addr = rq_deref_var(b, index, vars->candidate.instance_addr),
      .sbt_offset_and_flags = rq_deref_var(b, index, vars->candidate.sbt_offset_and_flags),
   };

   struct traversal_data data = {
      .vars = vars,
      .index = index,
   };

   struct radv_ray_traversal_args args = {
      .root_bvh_base = rq_load_var(b, index, vars->root_bvh_base),
      .flags = rq_load_var(b, index, vars->flags),
      .cull_mask = rq_load_var(b, index, vars->cull_mask),
      .origin = rq_load_var(b, index, vars->origin),
      .tmin = rq_load_var(b, index, vars->tmin),
      .dir = rq_load_var(b, index, vars->direction),
      .vars = trav_vars,
      .stack_entries = vars->stack_entries,
      .ignore_cull_mask = ignore_cull_mask,
      .stack_store_cb = store_stack_entry,
      .stack_load_cb = load_stack_entry,
      .aabb_cb = handle_candidate_aabb,
      .triangle_cb = handle_candidate_triangle,
      .data = &data,
   };

   if (vars->stack) {
      args.stack_stride = 1;
      args.stack_base = 0;
   } else {
      uint32_t workgroup_size =
         b->shader->info.workgroup_size[0] * b->shader->info.workgroup_size[1] * b->shader->info.workgroup_size[2];
      args.stack_stride = workgroup_size * 4;
      args.stack_base = vars->shared_base;
   }

   nir_push_if(b, rq_load_var(b, index, vars->incomplete));
   {
      nir_def *incomplete = radv_build_ray_traversal(device, b, &args);
      rq_store_var(b, index, vars->incomplete, nir_iand(b, rq_load_var(b, index, vars->incomplete), incomplete), 1);
   }
   nir_pop_if(b, NULL);

   return rq_load_var(b, index, vars->incomplete);
}

static void
lower_rq_terminate(nir_builder *b, nir_def *index, nir_intrinsic_instr *instr, struct ray_query_vars *vars)
{
   rq_store_var(b, index, vars->incomplete, nir_imm_false(b), 0x1);
}

bool
radv_nir_lower_ray_queries(struct nir_shader *shader, struct radv_device *device)
{
   bool progress = false;
   struct hash_table *query_ht = _mesa_pointer_hash_table_create(NULL);

   nir_foreach_variable_in_list (var, &shader->variables) {
      if (!var->data.ray_query)
         continue;

      lower_ray_query(shader, var, query_ht, device->physical_device->max_shared_size);

      progress = true;
   }

   nir_foreach_function (function, shader) {
      if (!function->impl)
         continue;

      nir_builder builder = nir_builder_create(function->impl);

      nir_foreach_variable_in_list (var, &function->impl->locals) {
         if (!var->data.ray_query)
            continue;

         lower_ray_query(shader, var, query_ht, device->physical_device->max_shared_size);

         progress = true;
      }

      nir_foreach_block (block, function->impl) {
         nir_foreach_instr_safe (instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrinsic = nir_instr_as_intrinsic(instr);

            if (!nir_intrinsic_is_ray_query(intrinsic->intrinsic))
               continue;

            nir_deref_instr *ray_query_deref = nir_instr_as_deref(intrinsic->src[0].ssa->parent_instr);
            nir_def *index = NULL;

            if (ray_query_deref->deref_type == nir_deref_type_array) {
               index = ray_query_deref->arr.index.ssa;
               ray_query_deref = nir_instr_as_deref(ray_query_deref->parent.ssa->parent_instr);
            }

            assert(ray_query_deref->deref_type == nir_deref_type_var);

            struct ray_query_vars *vars =
               (struct ray_query_vars *)_mesa_hash_table_search(query_ht, ray_query_deref->var)->data;

            builder.cursor = nir_before_instr(instr);

            nir_def *new_dest = NULL;

            switch (intrinsic->intrinsic) {
            case nir_intrinsic_rq_confirm_intersection:
               lower_rq_confirm_intersection(&builder, index, intrinsic, vars);
               break;
            case nir_intrinsic_rq_generate_intersection:
               lower_rq_generate_intersection(&builder, index, intrinsic, vars);
               break;
            case nir_intrinsic_rq_initialize:
               lower_rq_initialize(&builder, index, intrinsic, vars, device->instance);
               break;
            case nir_intrinsic_rq_load:
               new_dest = lower_rq_load(device, &builder, index, intrinsic, vars);
               break;
            case nir_intrinsic_rq_proceed:
               new_dest = lower_rq_proceed(&builder, index, intrinsic, vars, device);
               break;
            case nir_intrinsic_rq_terminate:
               lower_rq_terminate(&builder, index, intrinsic, vars);
               break;
            default:
               unreachable("Unsupported ray query intrinsic!");
            }

            if (new_dest)
               nir_def_rewrite_uses(&intrinsic->def, new_dest);

            nir_instr_remove(instr);
            nir_instr_free(instr);

            progress = true;
         }
      }

      nir_metadata_preserve(function->impl, nir_metadata_none);
   }

   ralloc_free(query_ht);

   return progress;
}
