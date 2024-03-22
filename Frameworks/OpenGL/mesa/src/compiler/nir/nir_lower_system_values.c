/*
 * Copyright © 2014 Intel Corporation
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
 *
 * Authors:
 *    Connor Abbott (cwabbott0@gmail.com)
 *
 */

#include "util/set.h"
#include "util/u_math.h"
#include "nir.h"
#include "nir_builder.h"

struct lower_sysval_state {
   const nir_lower_compute_system_values_options *options;

   /* List of intrinsics that have already been lowered and shouldn't be
    * lowered again.
    */
   struct set *lower_once_list;
};

static nir_def *
sanitize_32bit_sysval(nir_builder *b, nir_intrinsic_instr *intrin)
{
   const unsigned bit_size = intrin->def.bit_size;
   if (bit_size == 32)
      return NULL;

   intrin->def.bit_size = 32;
   return nir_u2uN(b, &intrin->def, bit_size);
}

static nir_def *
build_global_group_size(nir_builder *b, unsigned bit_size)
{
   nir_def *group_size = nir_load_workgroup_size(b);
   nir_def *num_workgroups = nir_load_num_workgroups(b);
   return nir_imul(b, nir_u2uN(b, group_size, bit_size),
                   nir_u2uN(b, num_workgroups, bit_size));
}

static bool
lower_system_value_filter(const nir_instr *instr, const void *_state)
{
   return instr->type == nir_instr_type_intrinsic;
}

static nir_def *
lower_system_value_instr(nir_builder *b, nir_instr *instr, void *_state)
{
   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   /* All the intrinsics we care about are loads */
   if (!nir_intrinsic_infos[intrin->intrinsic].has_dest)
      return NULL;

   const unsigned bit_size = intrin->def.bit_size;

   switch (intrin->intrinsic) {
   case nir_intrinsic_load_vertex_id:
      if (b->shader->options->vertex_id_zero_based) {
         return nir_iadd(b, nir_load_vertex_id_zero_base(b),
                         nir_load_first_vertex(b));
      } else {
         return NULL;
      }

   case nir_intrinsic_load_base_vertex:
      /**
       * From the OpenGL 4.6 (11.1.3.9 Shader Inputs) specification:
       *
       * "gl_BaseVertex holds the integer value passed to the baseVertex
       * parameter to the command that resulted in the current shader
       * invocation. In the case where the command has no baseVertex
       * parameter, the value of gl_BaseVertex is zero."
       */
      if (b->shader->options->lower_base_vertex) {
         return nir_iand(b, nir_load_is_indexed_draw(b),
                         nir_load_first_vertex(b));
      } else {
         return NULL;
      }

   case nir_intrinsic_load_helper_invocation:
      if (b->shader->options->lower_helper_invocation) {
         return nir_build_lowered_load_helper_invocation(b);
      } else {
         return NULL;
      }

   case nir_intrinsic_load_local_invocation_id:
   case nir_intrinsic_load_local_invocation_index:
   case nir_intrinsic_load_num_workgroups:
   case nir_intrinsic_load_workgroup_id:
   case nir_intrinsic_load_workgroup_size:
      return sanitize_32bit_sysval(b, intrin);

   case nir_intrinsic_interp_deref_at_centroid:
   case nir_intrinsic_interp_deref_at_sample:
   case nir_intrinsic_interp_deref_at_offset: {
      nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
      if (!nir_deref_mode_is(deref, nir_var_system_value))
         return NULL;

      nir_variable *var = deref->var;
      enum glsl_interp_mode interp_mode;

      if (var->data.location == SYSTEM_VALUE_BARYCENTRIC_PERSP_COORD) {
         interp_mode = INTERP_MODE_SMOOTH;
      } else {
         assert(var->data.location == SYSTEM_VALUE_BARYCENTRIC_LINEAR_COORD);
         interp_mode = INTERP_MODE_NOPERSPECTIVE;
      }

      switch (intrin->intrinsic) {
      case nir_intrinsic_interp_deref_at_centroid:
         return nir_load_barycentric_coord_centroid(b, 32, .interp_mode = interp_mode);
      case nir_intrinsic_interp_deref_at_sample:
         return nir_load_barycentric_coord_at_sample(b, 32, intrin->src[1].ssa,
                                                     .interp_mode = interp_mode);
      case nir_intrinsic_interp_deref_at_offset:
         return nir_load_barycentric_coord_at_offset(b, 32, intrin->src[1].ssa,
                                                     .interp_mode = interp_mode);
      default:
         unreachable("Bogus interpolateAt() intrinsic.");
      }
   }

   case nir_intrinsic_load_input:
      if (b->shader->options->lower_layer_fs_input_to_sysval &&
          b->shader->info.stage == MESA_SHADER_FRAGMENT &&
          nir_intrinsic_io_semantics(intrin).location == VARYING_SLOT_LAYER)
         return nir_load_layer_id(b);
      else
         return NULL;

   case nir_intrinsic_load_deref: {
      nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
      if (!nir_deref_mode_is(deref, nir_var_system_value))
         return NULL;

      nir_def *column = NULL;
      if (deref->deref_type != nir_deref_type_var) {
         /* The only one system values that aren't plane variables are
          * gl_SampleMask which is always an array of one element and a
          * couple of ray-tracing intrinsics which are matrices.
          */
         assert(deref->deref_type == nir_deref_type_array);
         column = deref->arr.index.ssa;
         nir_deref_instr *arr_deref = deref;
         deref = nir_deref_instr_parent(deref);
         assert(deref->deref_type == nir_deref_type_var);

         switch (deref->var->data.location) {
         case SYSTEM_VALUE_TESS_LEVEL_INNER:
         case SYSTEM_VALUE_TESS_LEVEL_OUTER: {
            nir_def *sysval = (deref->var->data.location ==
                               SYSTEM_VALUE_TESS_LEVEL_INNER)
                                 ? nir_load_tess_level_inner(b)
                                 : nir_load_tess_level_outer(b);
            return nir_vector_extract(b, sysval, arr_deref->arr.index.ssa);
         }

         case SYSTEM_VALUE_SAMPLE_MASK_IN:
         case SYSTEM_VALUE_RAY_OBJECT_TO_WORLD:
         case SYSTEM_VALUE_RAY_WORLD_TO_OBJECT:
         case SYSTEM_VALUE_MESH_VIEW_INDICES:
         case SYSTEM_VALUE_RAY_TRIANGLE_VERTEX_POSITIONS:
            /* These are all single-element arrays in our implementation, and
             * the sysval load below just drops the 0 array index.
             */
            break;

         default:
            unreachable("unsupported system value array deref");
         }
      }
      nir_variable *var = deref->var;

      switch (var->data.location) {
      case SYSTEM_VALUE_INSTANCE_INDEX:
         return nir_iadd(b, nir_load_instance_id(b),
                         nir_load_base_instance(b));

      case SYSTEM_VALUE_SUBGROUP_EQ_MASK:
      case SYSTEM_VALUE_SUBGROUP_GE_MASK:
      case SYSTEM_VALUE_SUBGROUP_GT_MASK:
      case SYSTEM_VALUE_SUBGROUP_LE_MASK:
      case SYSTEM_VALUE_SUBGROUP_LT_MASK: {
         nir_intrinsic_op op =
            nir_intrinsic_from_system_value(var->data.location);
         nir_intrinsic_instr *load = nir_intrinsic_instr_create(b->shader, op);
         nir_def_init_for_type(&load->instr, &load->def, var->type);
         load->num_components = load->def.num_components;
         nir_builder_instr_insert(b, &load->instr);
         return &load->def;
      }

      case SYSTEM_VALUE_DEVICE_INDEX:
         if (b->shader->options->lower_device_index_to_zero)
            return nir_imm_int(b, 0);
         break;

      case SYSTEM_VALUE_GLOBAL_GROUP_SIZE:
         return build_global_group_size(b, bit_size);

      case SYSTEM_VALUE_BARYCENTRIC_LINEAR_PIXEL:
         return nir_load_barycentric(b, nir_intrinsic_load_barycentric_pixel,
                                     INTERP_MODE_NOPERSPECTIVE);

      case SYSTEM_VALUE_BARYCENTRIC_LINEAR_CENTROID:
         return nir_load_barycentric(b, nir_intrinsic_load_barycentric_centroid,
                                     INTERP_MODE_NOPERSPECTIVE);

      case SYSTEM_VALUE_BARYCENTRIC_LINEAR_SAMPLE:
         return nir_load_barycentric(b, nir_intrinsic_load_barycentric_sample,
                                     INTERP_MODE_NOPERSPECTIVE);

      case SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL:
         return nir_load_barycentric(b, nir_intrinsic_load_barycentric_pixel,
                                     INTERP_MODE_SMOOTH);

      case SYSTEM_VALUE_BARYCENTRIC_PERSP_CENTROID:
         return nir_load_barycentric(b, nir_intrinsic_load_barycentric_centroid,
                                     INTERP_MODE_SMOOTH);

      case SYSTEM_VALUE_BARYCENTRIC_PERSP_SAMPLE:
         return nir_load_barycentric(b, nir_intrinsic_load_barycentric_sample,
                                     INTERP_MODE_SMOOTH);

      case SYSTEM_VALUE_BARYCENTRIC_PULL_MODEL:
         return nir_load_barycentric(b, nir_intrinsic_load_barycentric_model,
                                     INTERP_MODE_NONE);

      case SYSTEM_VALUE_BARYCENTRIC_LINEAR_COORD:
      case SYSTEM_VALUE_BARYCENTRIC_PERSP_COORD: {
         enum glsl_interp_mode interp_mode;

         if (var->data.location == SYSTEM_VALUE_BARYCENTRIC_PERSP_COORD) {
            interp_mode = INTERP_MODE_SMOOTH;
         } else {
            assert(var->data.location == SYSTEM_VALUE_BARYCENTRIC_LINEAR_COORD);
            interp_mode = INTERP_MODE_NOPERSPECTIVE;
         }

         if (var->data.sample) {
            return nir_load_barycentric_coord_sample(b, 32, .interp_mode = interp_mode);
         } else if (var->data.centroid) {
            return nir_load_barycentric_coord_centroid(b, 32, .interp_mode = interp_mode);
         } else {
            return nir_load_barycentric_coord_pixel(b, 32, .interp_mode = interp_mode);
         }
      }

      case SYSTEM_VALUE_HELPER_INVOCATION: {
         /* When demote operation is used, reading the HelperInvocation
          * needs to use Volatile memory access semantics to provide the
          * correct (dynamic) value.  See OpDemoteToHelperInvocation.
          */
         if (nir_intrinsic_access(intrin) & ACCESS_VOLATILE)
            return nir_is_helper_invocation(b, 1);
         break;
      }

      case SYSTEM_VALUE_MESH_VIEW_INDICES:
         return nir_load_mesh_view_indices(b, intrin->def.num_components,
                                           bit_size, column, .base = 0,
                                           .range = intrin->def.num_components * bit_size / 8);

      default:
         break;
      }

      nir_intrinsic_op sysval_op =
         nir_intrinsic_from_system_value(var->data.location);
      if (glsl_type_is_matrix(var->type)) {
         assert(nir_intrinsic_infos[sysval_op].index_map[NIR_INTRINSIC_COLUMN] > 0);
         unsigned num_cols = glsl_get_matrix_columns(var->type);
         ASSERTED unsigned num_rows = glsl_get_vector_elements(var->type);
         assert(num_rows == intrin->def.num_components);

         nir_def *cols[4];
         for (unsigned i = 0; i < num_cols; i++) {
            cols[i] = nir_load_system_value(b, sysval_op, i,
                                            intrin->def.num_components,
                                            intrin->def.bit_size);
            assert(cols[i]->num_components == num_rows);
         }
         return nir_select_from_ssa_def_array(b, cols, num_cols, column);
      } else if (glsl_type_is_array(var->type)) {
         unsigned num_elems = glsl_get_length(var->type);
         ASSERTED const struct glsl_type *elem_type = glsl_get_array_element(var->type);
         assert(glsl_get_components(elem_type) == intrin->def.num_components);

         nir_def *elems[4];
         assert(ARRAY_SIZE(elems) >= num_elems);
         for (unsigned i = 0; i < num_elems; i++) {
            elems[i] = nir_load_system_value(b, sysval_op, i,
                                             intrin->def.num_components,
                                             intrin->def.bit_size);
         }
         return nir_select_from_ssa_def_array(b, elems, num_elems, column);
      } else {
         return nir_load_system_value(b, sysval_op, 0,
                                      intrin->def.num_components,
                                      intrin->def.bit_size);
      }
   }

   default:
      return NULL;
   }
}

nir_def *
nir_build_lowered_load_helper_invocation(nir_builder *b)
{
   nir_def *tmp;
   tmp = nir_ishl(b, nir_imm_int(b, 1),
                  nir_load_sample_id_no_per_sample(b));
   tmp = nir_iand(b, nir_load_sample_mask_in(b), tmp);
   return nir_inot(b, nir_i2b(b, tmp));
}

bool
nir_lower_system_values(nir_shader *shader)
{
   bool progress = nir_shader_lower_instructions(shader,
                                                 lower_system_value_filter,
                                                 lower_system_value_instr,
                                                 NULL);

   /* We're going to delete the variables so we need to clean up all those
    * derefs we left lying around.
    */
   if (progress)
      nir_remove_dead_derefs(shader);

   nir_foreach_variable_with_modes_safe(var, shader, nir_var_system_value)
      exec_node_remove(&var->node);

   return progress;
}

static nir_def *
id_to_index_no_umod_slow(nir_builder *b, nir_def *index,
                         nir_def *size_x, nir_def *size_y,
                         unsigned bit_size)
{
   /* We lower ID to Index with the following formula:
    *
    *    id.z = index / (size.x * size.y)
    *    id.y = (index - (id.z * (size.x * size.y))) / size.x
    *    id.x = index - ((id.z * (size.x * size.y)) + (id.y * size.x))
    *
    * This is more efficient on HW that doesn't have a
    * modulo division instruction and when the size is either
    * not compile time known or not a power of two.
    */

   nir_def *size_x_y = nir_imul(b, size_x, size_y);
   nir_def *id_z = nir_udiv(b, index, size_x_y);
   nir_def *z_portion = nir_imul(b, id_z, size_x_y);
   nir_def *id_y = nir_udiv(b, nir_isub(b, index, z_portion), size_x);
   nir_def *y_portion = nir_imul(b, id_y, size_x);
   nir_def *id_x = nir_isub(b, index, nir_iadd(b, z_portion, y_portion));

   return nir_u2uN(b, nir_vec3(b, id_x, id_y, id_z), bit_size);
}

static nir_def *
lower_id_to_index_no_umod(nir_builder *b, nir_def *index,
                          nir_def *size, unsigned bit_size,
                          const uint32_t *size_imm,
                          bool shortcut_1d)
{
   nir_def *size_x, *size_y;

   if (size_imm[0] > 0)
      size_x = nir_imm_int(b, size_imm[0]);
   else
      size_x = nir_channel(b, size, 0);

   if (size_imm[1] > 0)
      size_y = nir_imm_int(b, size_imm[1]);
   else
      size_y = nir_channel(b, size, 1);

   if (shortcut_1d) {
      /* if size.y + size.z == 2 (which means that both y and z are 1)
       *    id = vec3(index, 0, 0)
       * else
       *    id = id_to_index_no_umod_slow
       */

      nir_def *size_z = nir_channel(b, size, 2);
      nir_def *cond = nir_ieq(b, nir_iadd(b, size_y, size_z), nir_imm_int(b, 2));

      nir_def *val1, *val2;
      nir_if *if_opt = nir_push_if(b, cond);
      if_opt->control = nir_selection_control_dont_flatten;
      {
         nir_def *zero = nir_imm_int(b, 0);
         val1 = nir_u2uN(b, nir_vec3(b, index, zero, zero), bit_size);
      }
      nir_push_else(b, if_opt);
      {
         val2 = id_to_index_no_umod_slow(b, index, size_x, size_y, bit_size);
      }
      nir_pop_if(b, if_opt);

      return nir_if_phi(b, val1, val2);
   } else {
      return id_to_index_no_umod_slow(b, index, size_x, size_y, bit_size);
   }
}

static nir_def *
lower_id_to_index(nir_builder *b, nir_def *index, nir_def *size,
                  unsigned bit_size)
{
   /* We lower gl_LocalInvocationID to gl_LocalInvocationIndex based
    * on this formula:
    *
    *    id.x = index % size.x;
    *    id.y = (index / size.x) % gl_WorkGroupSize.y;
    *    id.z = (index / (size.x * size.y)) % size.z;
    *
    * However, the final % size.z does nothing unless we
    * accidentally end up with an index that is too
    * large so it can safely be omitted.
    *
    * Because no hardware supports a local workgroup size greater than
    * about 1K, this calculation can be done in 32-bit and can save some
    * 64-bit arithmetic.
    */

   nir_def *size_x = nir_channel(b, size, 0);
   nir_def *size_y = nir_channel(b, size, 1);

   nir_def *id_x = nir_umod(b, index, size_x);
   nir_def *id_y = nir_umod(b, nir_udiv(b, index, size_x), size_y);
   nir_def *id_z = nir_udiv(b, index, nir_imul(b, size_x, size_y));

   return nir_u2uN(b, nir_vec3(b, id_x, id_y, id_z), bit_size);
}

static bool
lower_compute_system_value_filter(const nir_instr *instr, const void *_state)
{
   return instr->type == nir_instr_type_intrinsic;
}

static nir_def *
try_lower_id_to_index_1d(nir_builder *b, nir_def *index, const uint32_t *size)
{
   /* size_x = 1, size_y = 1, therefore Z = local index */
   if (size[0] == 1 && size[1] == 1)
      return nir_vec3(b, nir_imm_int(b, 0), nir_imm_int(b, 0), index);

   /* size_x = 1, size_z = 1, therefore Y = local index */
   if (size[0] == 1 && size[2] == 1)
      return nir_vec3(b, nir_imm_int(b, 0), index, nir_imm_int(b, 0));

   /* size_y = 1, size_z = 1, therefore X = local index */
   if (size[1] == 1 && size[2] == 1)
      return nir_vec3(b, index, nir_imm_int(b, 0), nir_imm_int(b, 0));

   return NULL;
}

static nir_def *
lower_compute_system_value_instr(nir_builder *b,
                                 nir_instr *instr, void *_state)
{
   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   struct lower_sysval_state *state = (struct lower_sysval_state *)_state;
   const nir_lower_compute_system_values_options *options = state->options;

   /* All the intrinsics we care about are loads */
   if (!nir_intrinsic_infos[intrin->intrinsic].has_dest)
      return NULL;

   const unsigned bit_size = intrin->def.bit_size;

   switch (intrin->intrinsic) {
   case nir_intrinsic_load_local_invocation_id:
      /* If lower_cs_local_id_to_index is true, then we replace
       * local_invocation_id with a formula based on local_invocation_index.
       */
      if (b->shader->options->lower_cs_local_id_to_index ||
          (options && options->lower_cs_local_id_to_index)) {
         nir_def *local_index = nir_load_local_invocation_index(b);

         if (!b->shader->info.workgroup_size_variable) {
            /* Shortcut for 1 dimensional workgroups:
             * Use local_invocation_index directly, which is better than
             * lower_id_to_index + constant folding, because
             * this way we don't leave behind extra ALU instrs.
             */

            uint32_t wg_size[3] = {b->shader->info.workgroup_size[0],
                                   b->shader->info.workgroup_size[1],
                                   b->shader->info.workgroup_size[2]};
            nir_def *val = try_lower_id_to_index_1d(b, local_index, wg_size);
            if (val)
               return val;
         }

         nir_def *local_size = nir_load_workgroup_size(b);
         return lower_id_to_index(b, local_index, local_size, bit_size);
      }
      if (options && options->shuffle_local_ids_for_quad_derivatives &&
          b->shader->info.cs.derivative_group == DERIVATIVE_GROUP_QUADS &&
          _mesa_set_search(state->lower_once_list, instr) == NULL) {
         nir_def *ids = nir_load_local_invocation_id(b);
         _mesa_set_add(state->lower_once_list, ids->parent_instr);

         nir_def *x = nir_channel(b, ids, 0);
         nir_def *y = nir_channel(b, ids, 1);
         nir_def *z = nir_channel(b, ids, 2);
         unsigned size_x = b->shader->info.workgroup_size[0];
         nir_def *size_x_imm;

         if (b->shader->info.workgroup_size_variable)
            size_x_imm = nir_channel(b, nir_load_workgroup_size(b), 0);
         else
            size_x_imm = nir_imm_int(b, size_x);

         /* Remap indices from:
          *    | 0| 1| 2| 3|
          *    | 4| 5| 6| 7|
          *    | 8| 9|10|11|
          *    |12|13|14|15|
          * to:
          *    | 0| 1| 4| 5|
          *    | 2| 3| 6| 7|
          *    | 8| 9|12|13|
          *    |10|11|14|15|
          *
          * That's the layout required by AMD hardware for derivatives to
          * work. Other hardware may work differently.
          *
          * It's a classic tiling pattern that can be implemented by inserting
          * bit y[0] between bits x[0] and x[1] like this:
          *
          *    x[0],y[0],x[1],...x[last],y[1],...,y[last]
          *
          * If the width is a power of two, use:
          *    i = ((x & 1) | ((y & 1) << 1) | ((x & ~1) << 1)) | ((y & ~1) << logbase2(size_x))
          *
          * If the width is not a power of two or the local size is variable, use:
          *    i = ((x & 1) | ((y & 1) << 1) | ((x & ~1) << 1)) + ((y & ~1) * size_x)
          *
          * GL_NV_compute_shader_derivatives requires that the width and height
          * are a multiple of two, which is also a requirement for the second
          * expression to work.
          *
          * The 2D result is: (x,y) = (i % w, i / w)
          */

         nir_def *one = nir_imm_int(b, 1);
         nir_def *inv_one = nir_imm_int(b, ~1);
         nir_def *x_bit0 = nir_iand(b, x, one);
         nir_def *y_bit0 = nir_iand(b, y, one);
         nir_def *x_bits_1n = nir_iand(b, x, inv_one);
         nir_def *y_bits_1n = nir_iand(b, y, inv_one);
         nir_def *bits_01 = nir_ior(b, x_bit0, nir_ishl(b, y_bit0, one));
         nir_def *bits_01x = nir_ior(b, bits_01,
                                     nir_ishl(b, x_bits_1n, one));
         nir_def *i;

         if (!b->shader->info.workgroup_size_variable &&
             util_is_power_of_two_nonzero(size_x)) {
            nir_def *log2_size_x = nir_imm_int(b, util_logbase2(size_x));
            i = nir_ior(b, bits_01x, nir_ishl(b, y_bits_1n, log2_size_x));
         } else {
            i = nir_iadd(b, bits_01x, nir_imul(b, y_bits_1n, size_x_imm));
         }

         /* This should be fast if size_x is an immediate or even a power
          * of two.
          */
         x = nir_umod(b, i, size_x_imm);
         y = nir_udiv(b, i, size_x_imm);

         return nir_vec3(b, x, y, z);
      }

      /* If a workgroup size dimension is 1, then the local invocation id must be zero. */
      nir_component_mask_t is_zero = 0;
      is_zero |= b->shader->info.workgroup_size[0] == 1 ? 0x1 : 0x0;
      is_zero |= b->shader->info.workgroup_size[1] == 1 ? 0x2 : 0x0;
      is_zero |= b->shader->info.workgroup_size[2] == 1 ? 0x4 : 0x0;
      if (!b->shader->info.workgroup_size_variable && is_zero) {
         nir_scalar defs[3];
         for (unsigned i = 0; i < 3; i++) {
            defs[i] = is_zero & (1 << i) ? nir_get_scalar(nir_imm_zero(b, 1, 32), 0) : nir_get_scalar(&intrin->def, i);
         }
         return nir_vec_scalars(b, defs, 3);
      }

      return NULL;

   case nir_intrinsic_load_local_invocation_index:
      /* If lower_cs_local_index_to_id is true, then we replace
       * local_invocation_index with a formula based on local_invocation_id.
       */
      if (b->shader->options->lower_cs_local_index_to_id ||
          (options && options->lower_local_invocation_index)) {
         /* From the GLSL man page for gl_LocalInvocationIndex:
          *
          *    "The value of gl_LocalInvocationIndex is equal to
          *    gl_LocalInvocationID.z * gl_WorkGroupSize.x *
          *    gl_WorkGroupSize.y + gl_LocalInvocationID.y *
          *    gl_WorkGroupSize.x + gl_LocalInvocationID.x"
          */
         nir_def *local_id = nir_load_local_invocation_id(b);
         nir_def *local_size = nir_load_workgroup_size(b);
         nir_def *size_x = nir_channel(b, local_size, 0);
         nir_def *size_y = nir_channel(b, local_size, 1);

         /* Because no hardware supports a local workgroup size greater than
          * about 1K, this calculation can be done in 32-bit and can save some
          * 64-bit arithmetic.
          */
         nir_def *index;
         index = nir_imul(b, nir_channel(b, local_id, 2),
                          nir_imul(b, size_x, size_y));
         index = nir_iadd(b, index,
                          nir_imul(b, nir_channel(b, local_id, 1), size_x));
         index = nir_iadd(b, index, nir_channel(b, local_id, 0));
         return nir_u2uN(b, index, bit_size);
      } else {
         return NULL;
      }

   case nir_intrinsic_load_workgroup_size:
      if (b->shader->info.workgroup_size_variable) {
         /* If the local work group size is variable it can't be lowered at
          * this point.  We do, however, have to make sure that the intrinsic
          * is only 32-bit.
          */
         return NULL;
      } else {
         /* using a 32 bit constant is safe here as no device/driver needs more
          * than 32 bits for the local size */
         nir_const_value workgroup_size_const[3];
         memset(workgroup_size_const, 0, sizeof(workgroup_size_const));
         workgroup_size_const[0].u32 = b->shader->info.workgroup_size[0];
         workgroup_size_const[1].u32 = b->shader->info.workgroup_size[1];
         workgroup_size_const[2].u32 = b->shader->info.workgroup_size[2];
         return nir_u2uN(b, nir_build_imm(b, 3, 32, workgroup_size_const), bit_size);
      }

   case nir_intrinsic_load_global_invocation_id_zero_base: {
      if ((options && options->has_base_workgroup_id) ||
          !b->shader->options->has_cs_global_id) {
         nir_def *group_size = nir_load_workgroup_size(b);
         nir_def *group_id = nir_load_workgroup_id(b);
         nir_def *local_id = nir_load_local_invocation_id(b);

         return nir_iadd(b, nir_imul(b, nir_u2uN(b, group_id, bit_size),
                         nir_u2uN(b, group_size, bit_size)),
                         nir_u2uN(b, local_id, bit_size));
      } else {
         return NULL;
      }
   }

   case nir_intrinsic_load_global_invocation_id: {
      if (options && options->has_base_global_invocation_id)
         return nir_iadd(b, nir_load_global_invocation_id_zero_base(b, bit_size),
                         nir_load_base_global_invocation_id(b, bit_size));
      else if ((options && options->has_base_workgroup_id) ||
               !b->shader->options->has_cs_global_id)
         return nir_load_global_invocation_id_zero_base(b, bit_size);
      else
         return NULL;
   }

   case nir_intrinsic_load_global_invocation_index: {
      /* OpenCL's global_linear_id explicitly removes the global offset before computing this */
      assert(b->shader->info.stage == MESA_SHADER_KERNEL);
      nir_def *global_base_id = nir_load_base_global_invocation_id(b, bit_size);
      nir_def *global_id = nir_isub(b, nir_load_global_invocation_id(b, bit_size), global_base_id);
      nir_def *global_size = build_global_group_size(b, bit_size);

      /* index = id.x + ((id.y + (id.z * size.y)) * size.x) */
      nir_def *index;
      index = nir_imul(b, nir_channel(b, global_id, 2),
                       nir_channel(b, global_size, 1));
      index = nir_iadd(b, nir_channel(b, global_id, 1), index);
      index = nir_imul(b, nir_channel(b, global_size, 0), index);
      index = nir_iadd(b, nir_channel(b, global_id, 0), index);
      return index;
   }

   case nir_intrinsic_load_workgroup_id: {
      if (options && options->has_base_workgroup_id)
         return nir_iadd(b, nir_u2uN(b, nir_load_workgroup_id_zero_base(b), bit_size),
                         nir_load_base_workgroup_id(b, bit_size));
      else if (options && options->lower_workgroup_id_to_index) {
         nir_def *wg_idx = nir_load_workgroup_index(b);

         nir_def *val =
            try_lower_id_to_index_1d(b, wg_idx, options->num_workgroups);
         if (val)
            return val;

         nir_def *num_workgroups = nir_load_num_workgroups(b);
         return lower_id_to_index_no_umod(b, wg_idx,
                                          nir_u2uN(b, num_workgroups, bit_size),
                                          bit_size,
                                          options->num_workgroups,
                                          options->shortcut_1d_workgroup_id);
      }

      return NULL;
   }

   case nir_intrinsic_load_num_workgroups: {
      if (!options)
         return NULL;

      const uint32_t *num_wgs_imm = options->num_workgroups;

      /* Exit early when none of the num workgroups components are known at
       * compile time.
       */
      if (num_wgs_imm[0] == 0 && num_wgs_imm[1] == 0 && num_wgs_imm[2] == 0)
         return NULL;

      b->cursor = nir_after_instr(instr);

      nir_def *num_wgs = &intrin->def;
      for (unsigned i = 0; i < 3; ++i) {
         if (num_wgs_imm[i])
            num_wgs = nir_vector_insert_imm(b, num_wgs, nir_imm_int(b, num_wgs_imm[i]), i);
      }

      return num_wgs;
   }

   case nir_intrinsic_load_shader_index:
      return nir_imm_int(b, b->shader->info.cs.shader_index);

   default:
      return NULL;
   }
}

bool
nir_lower_compute_system_values(nir_shader *shader,
                                const nir_lower_compute_system_values_options *options)
{
   if (!gl_shader_stage_uses_workgroup(shader->info.stage))
      return false;

   struct lower_sysval_state state;
   state.options = options;
   state.lower_once_list = _mesa_pointer_set_create(NULL);

   bool progress =
      nir_shader_lower_instructions(shader,
                                    lower_compute_system_value_filter,
                                    lower_compute_system_value_instr,
                                    (void *)&state);
   ralloc_free(state.lower_once_list);

   /* Update this so as not to lower it again. */
   if (options && options->shuffle_local_ids_for_quad_derivatives &&
       shader->info.cs.derivative_group == DERIVATIVE_GROUP_QUADS)
      shader->info.cs.derivative_group = DERIVATIVE_GROUP_LINEAR;

   return progress;
}
