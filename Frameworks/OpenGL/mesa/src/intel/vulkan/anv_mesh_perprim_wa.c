/*
 * Copyright © 2022 Intel Corporation
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

#include "anv_private.h"
#include "nir_builder.h"

/*
 * Wa_18019110168 for gfx 12.5.
 *
 * This file implements workaround for HW bug, which leads to fragment shader
 * reading incorrect per-primitive data if mesh shader, in addition to writing
 * per-primitive data, also writes to gl_ClipDistance.
 *
 * The suggested solution to that bug is to not use per-primitive data by:
 * - creating new vertices for provoking vertices shared by multiple primitives
 * - converting per-primitive attributes read by fragment shader to flat
 *   per-vertex attributes for the provoking vertex
 * - modifying fragment shader to read those per-vertex attributes
 *
 * There are at least 2 type of failures not handled very well:
 * - if the number of varying slots overflows, than only some attributes will
 *   be converted, leading to corruption of those unconverted attributes
 * - if the overall MUE size is so large it doesn't fit in URB, then URB
 *   allocation will fail in some way; unfortunately there's no good way to
 *   say how big MUE will be at this moment and back out
 *
 * This workaround needs to be applied before linking, so that unused outputs
 * created by this code are removed at link time.
 *
 * This workaround can be controlled by a driconf option to either disable it,
 * lower its scope or force enable it.
 *
 * Option "anv_mesh_conv_prim_attrs_to_vert_attrs" is evaluated like this:
 *  value == 0 - disable workaround
 *  value < 0 - enable ONLY if workaround is required
 *  value > 0 - enable ALWAYS, even if it's not required
 *  abs(value) >= 1 - attribute conversion
 *  abs(value) >= 2 - attribute conversion and vertex duplication
 *
 *  Default: -2 (both parts of the work around, ONLY if it's required)
 *
 */

static bool
anv_mesh_convert_attrs_prim_to_vert(struct nir_shader *nir,
                                    gl_varying_slot *wa_mapping,
                                    uint64_t fs_inputs,
                                    const VkGraphicsPipelineCreateInfo *pCreateInfo,
                                    void *mem_ctx,
                                    const bool dup_vertices,
                                    const bool force_conversion)
{
   uint64_t per_primitive_outputs = nir->info.per_primitive_outputs;
   per_primitive_outputs &= ~BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_INDICES);

   if (per_primitive_outputs == 0)
      return false;

   uint64_t outputs_written = nir->info.outputs_written;
   uint64_t other_outputs = outputs_written & ~per_primitive_outputs;

   if ((other_outputs & (VARYING_BIT_CLIP_DIST0 | VARYING_BIT_CLIP_DIST1)) == 0)
      if (!force_conversion)
         return false;

   uint64_t all_outputs = outputs_written;
   unsigned attrs = 0;

   uint64_t remapped_outputs = outputs_written & per_primitive_outputs;
   remapped_outputs &= ~BITFIELD64_BIT(VARYING_SLOT_CULL_PRIMITIVE);

   /* Skip locations not read by the fragment shader, because they will
    * be eliminated at linking time. Note that some fs inputs may be
    * removed only after optimizations, so it's possible that we will
    * create too many variables.
    */
   remapped_outputs &= fs_inputs;

   /* Figure out the mapping between per-primitive and new per-vertex outputs. */
   nir_foreach_shader_out_variable(var, nir) {
      int location = var->data.location;

      if (!(BITFIELD64_BIT(location) & remapped_outputs))
         continue;

      /* Although primitive shading rate, layer and viewport have predefined
       * place in MUE Primitive Header (so we can't really move them anywhere),
       * we have to copy them to per-vertex space if fragment shader reads them.
       */
      assert(location == VARYING_SLOT_PRIMITIVE_SHADING_RATE ||
             location == VARYING_SLOT_LAYER ||
             location == VARYING_SLOT_VIEWPORT ||
             location == VARYING_SLOT_PRIMITIVE_ID ||
             location >= VARYING_SLOT_VAR0);

      const struct glsl_type *type = var->type;
      if (nir_is_arrayed_io(var, MESA_SHADER_MESH) || var->data.per_view) {
         assert(glsl_type_is_array(type));
         type = glsl_get_array_element(type);
      }

      unsigned num_slots = glsl_count_attribute_slots(type, false);

      for (gl_varying_slot slot = VARYING_SLOT_VAR0; slot <= VARYING_SLOT_VAR31; slot++) {
         uint64_t mask = BITFIELD64_MASK(num_slots) << slot;
         if ((all_outputs & mask) == 0) {
            wa_mapping[location] = slot;
            all_outputs |= mask;
            attrs++;
            break;
         }
      }

      if (wa_mapping[location] == 0) {
         fprintf(stderr, "Not enough space for hardware per-primitive data corruption work around.\n");
         break;
      }
   }

   if (attrs == 0)
      if (!force_conversion)
         return false;

   unsigned provoking_vertex = 0;

   const VkPipelineRasterizationStateCreateInfo *rs_info = pCreateInfo->pRasterizationState;
   const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *rs_pv_info =
      vk_find_struct_const(rs_info, PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT);
   if (rs_pv_info && rs_pv_info->provokingVertexMode == VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT)
      provoking_vertex = 2;

   unsigned vertices_per_primitive =
         mesa_vertices_per_prim(nir->info.mesh.primitive_type);

   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   nir_builder b = nir_builder_at(nir_after_impl(impl));

   /* wait for all subgroups to finish */
   nir_barrier(&b, SCOPE_WORKGROUP);

   nir_def *zero = nir_imm_int(&b, 0);

   nir_def *local_invocation_index = nir_load_local_invocation_index(&b);

   nir_def *cmp = nir_ieq(&b, local_invocation_index, zero);
   nir_if *if_stmt = nir_push_if(&b, cmp);
   {
      nir_variable *primitive_count_var = NULL;
      nir_variable *primitive_indices_var = NULL;

      unsigned num_other_variables = 0;
      nir_foreach_shader_out_variable(var, b.shader) {
         if ((BITFIELD64_BIT(var->data.location) & other_outputs) == 0)
            continue;
         num_other_variables++;
      }

      nir_deref_instr **per_vertex_derefs =
            ralloc_array(mem_ctx, nir_deref_instr *, num_other_variables);

      unsigned num_per_vertex_variables = 0;

      unsigned processed = 0;
      nir_foreach_shader_out_variable(var, b.shader) {
         if ((BITFIELD64_BIT(var->data.location) & other_outputs) == 0)
            continue;

         switch (var->data.location) {
            case VARYING_SLOT_PRIMITIVE_COUNT:
               primitive_count_var = var;
               break;
            case VARYING_SLOT_PRIMITIVE_INDICES:
               primitive_indices_var = var;
               break;
            default: {
               const struct glsl_type *type = var->type;
               assert(glsl_type_is_array(type));
               const struct glsl_type *array_element_type =
                     glsl_get_array_element(type);

               if (dup_vertices) {
                  /*
                   * Resize type of array output to make space for one extra
                   * vertex attribute for each primitive, so we ensure that
                   * the provoking vertex is not shared between primitives.
                   */
                  const struct glsl_type *new_type =
                        glsl_array_type(array_element_type,
                                        glsl_get_length(type) +
                                        nir->info.mesh.max_primitives_out,
                                        0);

                  var->type = new_type;
               }

               per_vertex_derefs[num_per_vertex_variables++] =
                     nir_build_deref_var(&b, var);
               break;
            }
         }

         ++processed;
      }
      assert(processed == num_other_variables);

      assert(primitive_count_var != NULL);
      assert(primitive_indices_var != NULL);

      /* Update types of derefs to match type of variables they (de)reference. */
      if (dup_vertices) {
         nir_foreach_function_impl(impl, b.shader) {
            nir_foreach_block(block, impl) {
               nir_foreach_instr(instr, block) {
                  if (instr->type != nir_instr_type_deref)
                     continue;

                  nir_deref_instr *deref = nir_instr_as_deref(instr);
                  if (deref->deref_type != nir_deref_type_var)
                     continue;

                  if (deref->var->type != deref->type)
                     deref->type = deref->var->type;
               }
            }
         }
      }

      /* indexed by slot of per-prim attribute */
      struct {
         nir_deref_instr *per_prim_deref;
         nir_deref_instr *per_vert_deref;
      } mapping[VARYING_SLOT_MAX] = {{NULL, NULL}, };

      /* Create new per-vertex output variables mirroring per-primitive variables
       * and create derefs for both old and new variables.
       */
      nir_foreach_shader_out_variable(var, b.shader) {
         gl_varying_slot location = var->data.location;

         if ((BITFIELD64_BIT(location) & (outputs_written & per_primitive_outputs)) == 0)
            continue;
         if (wa_mapping[location] == 0)
            continue;

         const struct glsl_type *type = var->type;
         assert(glsl_type_is_array(type));
         const struct glsl_type *array_element_type = glsl_get_array_element(type);

         const struct glsl_type *new_type =
               glsl_array_type(array_element_type,
                               nir->info.mesh.max_vertices_out +
                               (dup_vertices ? nir->info.mesh.max_primitives_out : 0),
                               0);

         nir_variable *new_var =
               nir_variable_create(b.shader, nir_var_shader_out, new_type, var->name);
         assert(wa_mapping[location] >= VARYING_SLOT_VAR0);
         assert(wa_mapping[location] <= VARYING_SLOT_VAR31);
         new_var->data.location = wa_mapping[location];
         new_var->data.interpolation = INTERP_MODE_FLAT;

         mapping[location].per_vert_deref = nir_build_deref_var(&b, new_var);
         mapping[location].per_prim_deref = nir_build_deref_var(&b, var);
      }

      nir_def *trueconst = nir_imm_true(&b);

      /*
       * for each Primitive (0 : primitiveCount)
       *    if VertexUsed[PrimitiveIndices[Primitive][provoking vertex]]
       *       create 1 new vertex at offset "Vertex"
       *       copy per vert attributes of provoking vertex to the new one
       *       update PrimitiveIndices[Primitive][provoking vertex]
       *       Vertex++
       *    else
       *       VertexUsed[PrimitiveIndices[Primitive][provoking vertex]] := true
       *
       *    for each attribute : mapping
       *       copy per_prim_attr(Primitive) to per_vert_attr[Primitive][provoking vertex]
       */

      /* primitive count */
      nir_def *primitive_count = nir_load_var(&b, primitive_count_var);

      /* primitive index */
      nir_variable *primitive_var =
            nir_local_variable_create(impl, glsl_uint_type(), "Primitive");
      nir_deref_instr *primitive_deref = nir_build_deref_var(&b, primitive_var);
      nir_store_deref(&b, primitive_deref, zero, 1);

      /* vertex index */
      nir_variable *vertex_var =
            nir_local_variable_create(impl, glsl_uint_type(), "Vertex");
      nir_deref_instr *vertex_deref = nir_build_deref_var(&b, vertex_var);
      nir_store_deref(&b, vertex_deref, nir_imm_int(&b, nir->info.mesh.max_vertices_out), 1);

      /* used vertices bitvector */
      const struct glsl_type *used_vertex_type =
            glsl_array_type(glsl_bool_type(),
                            nir->info.mesh.max_vertices_out,
                            0);
      nir_variable *used_vertex_var =
            nir_local_variable_create(impl, used_vertex_type, "VertexUsed");
      nir_deref_instr *used_vertex_deref =
               nir_build_deref_var(&b, used_vertex_var);
      /* Initialize it as "not used" */
      for (unsigned i = 0; i < nir->info.mesh.max_vertices_out; ++i) {
         nir_deref_instr *indexed_used_vertex_deref =
                        nir_build_deref_array(&b, used_vertex_deref, nir_imm_int(&b, i));
         nir_store_deref(&b, indexed_used_vertex_deref, nir_imm_false(&b), 1);
      }

      nir_loop *loop = nir_push_loop(&b);
      {
         nir_def *primitive = nir_load_deref(&b, primitive_deref);
         nir_def *cmp = nir_ige(&b, primitive, primitive_count);

         nir_if *loop_check = nir_push_if(&b, cmp);
         nir_jump(&b, nir_jump_break);
         nir_pop_if(&b, loop_check);

         nir_deref_instr *primitive_indices_deref =
               nir_build_deref_var(&b, primitive_indices_var);
         nir_deref_instr *indexed_primitive_indices_deref;
         nir_def *src_vertex;
         nir_def *prim_indices;

         /* array of vectors, we have to extract index out of array deref */
         indexed_primitive_indices_deref = nir_build_deref_array(&b, primitive_indices_deref, primitive);
         prim_indices = nir_load_deref(&b, indexed_primitive_indices_deref);
         src_vertex = nir_channel(&b, prim_indices, provoking_vertex);

         nir_def *dst_vertex = nir_load_deref(&b, vertex_deref);

         nir_deref_instr *indexed_used_vertex_deref =
                        nir_build_deref_array(&b, used_vertex_deref, src_vertex);
         nir_def *used_vertex = nir_load_deref(&b, indexed_used_vertex_deref);
         if (!dup_vertices)
            used_vertex = nir_imm_false(&b);

         nir_if *vertex_used_check = nir_push_if(&b, used_vertex);
         {
            for (unsigned a = 0; a < num_per_vertex_variables; ++a) {
               nir_deref_instr *attr_arr = per_vertex_derefs[a];
               nir_deref_instr *src = nir_build_deref_array(&b, attr_arr, src_vertex);
               nir_deref_instr *dst = nir_build_deref_array(&b, attr_arr, dst_vertex);

               nir_copy_deref(&b, dst, src);
            }

            /* replace one component of primitive indices vector */
            nir_def *new_val =
                  nir_vector_insert_imm(&b, prim_indices, dst_vertex, provoking_vertex);

            /* and store complete vector */
            nir_store_deref(&b, indexed_primitive_indices_deref, new_val,
                            BITFIELD_MASK(vertices_per_primitive));

            nir_store_deref(&b, vertex_deref, nir_iadd_imm(&b, dst_vertex, 1), 1);

            for (unsigned i = 0; i < ARRAY_SIZE(mapping); ++i) {
               if (!mapping[i].per_vert_deref)
                  continue;

               nir_deref_instr *src =
                     nir_build_deref_array(&b, mapping[i].per_prim_deref, primitive);
               nir_deref_instr *dst =
                     nir_build_deref_array(&b, mapping[i].per_vert_deref, dst_vertex);

               nir_copy_deref(&b, dst, src);
            }
         }
         nir_push_else(&b, vertex_used_check);
         {
            nir_store_deref(&b, indexed_used_vertex_deref, trueconst, 1);

            for (unsigned i = 0; i < ARRAY_SIZE(mapping); ++i) {
               if (!mapping[i].per_vert_deref)
                  continue;

               nir_deref_instr *src =
                     nir_build_deref_array(&b, mapping[i].per_prim_deref, primitive);
               nir_deref_instr *dst =
                     nir_build_deref_array(&b, mapping[i].per_vert_deref, src_vertex);

               nir_copy_deref(&b, dst, src);
            }

         }
         nir_pop_if(&b, vertex_used_check);

         nir_store_deref(&b, primitive_deref, nir_iadd_imm(&b, primitive, 1), 1);
      }
      nir_pop_loop(&b, loop);
   }
   nir_pop_if(&b, if_stmt); /* local_invocation_index == 0 */

   if (dup_vertices)
      nir->info.mesh.max_vertices_out += nir->info.mesh.max_primitives_out;

   if (should_print_nir(nir)) {
      printf("%s\n", __func__);
      nir_print_shader(nir, stdout);
   }

   /* deal with copy_derefs */
   NIR_PASS(_, nir, nir_split_var_copies);
   NIR_PASS(_, nir, nir_lower_var_copies);

   nir_shader_gather_info(nir, impl);

   return true;
}

static bool
anv_frag_update_derefs_instr(struct nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type != nir_instr_type_deref)
      return false;

   nir_deref_instr *deref = nir_instr_as_deref(instr);
   if (deref->deref_type != nir_deref_type_var)
      return false;

   nir_variable *var = deref->var;
   if (!(var->data.mode & nir_var_shader_in))
      return false;

   int location = var->data.location;
   nir_deref_instr **new_derefs = (nir_deref_instr **)data;
   if (new_derefs[location] == NULL)
      return false;

   nir_instr_remove(&deref->instr);
   nir_def_rewrite_uses(&deref->def, &new_derefs[location]->def);

   return true;
}

static bool
anv_frag_update_derefs(nir_shader *shader, nir_deref_instr **mapping)
{
   return nir_shader_instructions_pass(shader, anv_frag_update_derefs_instr,
                                       nir_metadata_none, (void *)mapping);
}

/* Update fragment shader inputs with new ones. */
static void
anv_frag_convert_attrs_prim_to_vert(struct nir_shader *nir,
                                    gl_varying_slot *wa_mapping)
{
   /* indexed by slot of per-prim attribute */
   nir_deref_instr *new_derefs[VARYING_SLOT_MAX] = {NULL, };

   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   nir_builder b = nir_builder_at(nir_before_impl(impl));

   nir_foreach_shader_in_variable_safe(var, nir) {
      gl_varying_slot location = var->data.location;
      gl_varying_slot new_location = wa_mapping[location];
      if (new_location == 0)
         continue;

      assert(wa_mapping[new_location] == 0);

      nir_variable *new_var =
            nir_variable_create(b.shader, nir_var_shader_in, var->type, var->name);
      new_var->data.location = new_location;
      new_var->data.location_frac = var->data.location_frac;
      new_var->data.interpolation = INTERP_MODE_FLAT;

      new_derefs[location] = nir_build_deref_var(&b, new_var);
   }

   NIR_PASS(_, nir, anv_frag_update_derefs, new_derefs);

   nir_shader_gather_info(nir, impl);
}

void
anv_apply_per_prim_attr_wa(struct nir_shader *ms_nir,
                           struct nir_shader *fs_nir,
                           struct anv_device *device,
                           const VkGraphicsPipelineCreateInfo *info)
{
   const struct intel_device_info *devinfo = device->info;

   int mesh_conv_prim_attrs_to_vert_attrs =
         device->physical->instance->mesh_conv_prim_attrs_to_vert_attrs;
   if (mesh_conv_prim_attrs_to_vert_attrs < 0 &&
         !intel_needs_workaround(devinfo, 18019110168))
      mesh_conv_prim_attrs_to_vert_attrs = 0;

   if (mesh_conv_prim_attrs_to_vert_attrs != 0) {
      uint64_t fs_inputs = 0;
      nir_foreach_shader_in_variable(var, fs_nir)
         fs_inputs |= BITFIELD64_BIT(var->data.location);

      void *stage_ctx = ralloc_context(NULL);

      gl_varying_slot wa_mapping[VARYING_SLOT_MAX] = { 0, };

      const bool dup_vertices = abs(mesh_conv_prim_attrs_to_vert_attrs) >= 2;
      const bool force_conversion = mesh_conv_prim_attrs_to_vert_attrs > 0;

      if (anv_mesh_convert_attrs_prim_to_vert(ms_nir, wa_mapping,
                                              fs_inputs, info, stage_ctx,
                                              dup_vertices, force_conversion))
         anv_frag_convert_attrs_prim_to_vert(fs_nir, wa_mapping);

      ralloc_free(stage_ctx);
   }
}
