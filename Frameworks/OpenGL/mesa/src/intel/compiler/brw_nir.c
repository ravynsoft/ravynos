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
 */

#include "brw_nir.h"
#include "brw_nir_rt.h"
#include "brw_shader.h"
#include "dev/intel_debug.h"
#include "compiler/glsl_types.h"
#include "compiler/nir/nir_builder.h"
#include "util/u_math.h"

static bool
remap_tess_levels(nir_builder *b, nir_intrinsic_instr *intr,
                  enum tess_primitive_mode _primitive_mode)
{
   const int location = nir_intrinsic_base(intr);
   const unsigned component = nir_intrinsic_component(intr);
   bool out_of_bounds = false;
   bool write = !nir_intrinsic_infos[intr->intrinsic].has_dest;
   unsigned mask = write ? nir_intrinsic_write_mask(intr) : 0;
   nir_def *src = NULL, *dest = NULL;

   if (write) {
      assert(intr->num_components == intr->src[0].ssa->num_components);
   } else {
      assert(intr->num_components == intr->def.num_components);
   }

   if (location == VARYING_SLOT_TESS_LEVEL_INNER) {
      b->cursor = write ? nir_before_instr(&intr->instr)
                        : nir_after_instr(&intr->instr);

      switch (_primitive_mode) {
      case TESS_PRIMITIVE_QUADS:
         /* gl_TessLevelInner[0..1] lives at DWords 3-2 (reversed). */
         nir_intrinsic_set_base(intr, 0);

         if (write) {
            assert(intr->src[0].ssa->num_components == 2);

            intr->num_components = 4;

            nir_def *undef = nir_undef(b, 1, 32);
            nir_def *x = nir_channel(b, intr->src[0].ssa, 0);
            nir_def *y = nir_channel(b, intr->src[0].ssa, 1);
            src = nir_vec4(b, undef, undef, y, x);
            mask = !!(mask & WRITEMASK_X) << 3 | !!(mask & WRITEMASK_Y) << 2;
         } else if (intr->def.num_components > 1) {
            assert(intr->def.num_components == 2);

            intr->num_components = 4;
            intr->def.num_components = 4;

            unsigned wz[2] = { 3, 2 };
            dest = nir_swizzle(b, &intr->def, wz, 2);
         } else {
            nir_intrinsic_set_component(intr, 3 - component);
         }
         break;
      case TESS_PRIMITIVE_TRIANGLES:
         /* gl_TessLevelInner[0] lives at DWord 4. */
         nir_intrinsic_set_base(intr, 1);
         mask &= WRITEMASK_X;
         out_of_bounds = component > 0;
         break;
      case TESS_PRIMITIVE_ISOLINES:
         out_of_bounds = true;
         break;
      default:
         unreachable("Bogus tessellation domain");
      }
   } else if (location == VARYING_SLOT_TESS_LEVEL_OUTER) {
      b->cursor = write ? nir_before_instr(&intr->instr)
                        : nir_after_instr(&intr->instr);

      nir_intrinsic_set_base(intr, 1);

      switch (_primitive_mode) {
      case TESS_PRIMITIVE_QUADS:
      case TESS_PRIMITIVE_TRIANGLES:
         /* Quads:     gl_TessLevelOuter[0..3] lives at DWords 7-4 (reversed).
          * Triangles: gl_TessLevelOuter[0..2] lives at DWords 7-5 (reversed).
          */
         if (write) {
            assert(intr->src[0].ssa->num_components == 4);

            unsigned wzyx[4] = { 3, 2, 1, 0 };
            src = nir_swizzle(b, intr->src[0].ssa, wzyx, 4);
            mask = !!(mask & WRITEMASK_X) << 3 | !!(mask & WRITEMASK_Y) << 2 |
                   !!(mask & WRITEMASK_Z) << 1 | !!(mask & WRITEMASK_W) << 0;

            /* Don't overwrite the inner factor at DWord 4 for triangles */
            if (_primitive_mode == TESS_PRIMITIVE_TRIANGLES)
               mask &= ~WRITEMASK_X;
         } else if (intr->def.num_components > 1) {
            assert(intr->def.num_components == 4);

            unsigned wzyx[4] = { 3, 2, 1, 0 };
            dest = nir_swizzle(b, &intr->def, wzyx, 4);
         } else {
            nir_intrinsic_set_component(intr, 3 - component);
            out_of_bounds = component == 3 &&
                            _primitive_mode == TESS_PRIMITIVE_TRIANGLES;
         }
         break;
      case TESS_PRIMITIVE_ISOLINES:
         /* gl_TessLevelOuter[0..1] lives at DWords 6-7 (in order). */
         if (write) {
            assert(intr->src[0].ssa->num_components == 4);

            nir_def *undef = nir_undef(b, 1, 32);
            nir_def *x = nir_channel(b, intr->src[0].ssa, 0);
            nir_def *y = nir_channel(b, intr->src[0].ssa, 1);
            src = nir_vec4(b, undef, undef, x, y);
            mask = !!(mask & WRITEMASK_X) << 2 | !!(mask & WRITEMASK_Y) << 3;
         } else {
            nir_intrinsic_set_component(intr, 2 + component);
            out_of_bounds = component > 1;
         }
         break;
      default:
         unreachable("Bogus tessellation domain");
      }
   } else {
      return false;
   }

   if (out_of_bounds) {
      if (!write)
         nir_def_rewrite_uses(&intr->def, nir_undef(b, 1, 32));
      nir_instr_remove(&intr->instr);
   } else if (write) {
      nir_intrinsic_set_write_mask(intr, mask);

      if (src) {
         nir_src_rewrite(&intr->src[0], src);
      }
   } else if (dest) {
      nir_def_rewrite_uses_after(&intr->def, dest,
                                     dest->parent_instr);
   }

   return true;
}

static bool
is_input(nir_intrinsic_instr *intrin)
{
   return intrin->intrinsic == nir_intrinsic_load_input ||
          intrin->intrinsic == nir_intrinsic_load_per_vertex_input ||
          intrin->intrinsic == nir_intrinsic_load_interpolated_input;
}

static bool
is_output(nir_intrinsic_instr *intrin)
{
   return intrin->intrinsic == nir_intrinsic_load_output ||
          intrin->intrinsic == nir_intrinsic_load_per_vertex_output ||
          intrin->intrinsic == nir_intrinsic_store_output ||
          intrin->intrinsic == nir_intrinsic_store_per_vertex_output;
}


static bool
remap_patch_urb_offsets(nir_block *block, nir_builder *b,
                        const struct brw_vue_map *vue_map,
                        enum tess_primitive_mode tes_primitive_mode)
{
   nir_foreach_instr_safe(instr, block) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

      gl_shader_stage stage = b->shader->info.stage;

      if ((stage == MESA_SHADER_TESS_CTRL && is_output(intrin)) ||
          (stage == MESA_SHADER_TESS_EVAL && is_input(intrin))) {

         if (remap_tess_levels(b, intrin, tes_primitive_mode))
            continue;

         int vue_slot = vue_map->varying_to_slot[intrin->const_index[0]];
         assert(vue_slot != -1);
         intrin->const_index[0] = vue_slot;

         nir_src *vertex = nir_get_io_arrayed_index_src(intrin);
         if (vertex) {
            if (nir_src_is_const(*vertex)) {
               intrin->const_index[0] += nir_src_as_uint(*vertex) *
                                         vue_map->num_per_vertex_slots;
            } else {
               b->cursor = nir_before_instr(&intrin->instr);

               /* Multiply by the number of per-vertex slots. */
               nir_def *vertex_offset =
                  nir_imul(b,
                           vertex->ssa,
                           nir_imm_int(b,
                                       vue_map->num_per_vertex_slots));

               /* Add it to the existing offset */
               nir_src *offset = nir_get_io_offset_src(intrin);
               nir_def *total_offset =
                  nir_iadd(b, vertex_offset,
                           offset->ssa);

               nir_src_rewrite(offset, total_offset);
            }
         }
      }
   }
   return true;
}

void
brw_nir_lower_vs_inputs(nir_shader *nir,
                        bool edgeflag_is_last,
                        const uint8_t *vs_attrib_wa_flags)
{
   /* Start with the location of the variable's base. */
   nir_foreach_shader_in_variable(var, nir)
      var->data.driver_location = var->data.location;

   /* Now use nir_lower_io to walk dereference chains.  Attribute arrays are
    * loaded as one vec4 or dvec4 per element (or matrix column), depending on
    * whether it is a double-precision type or not.
    */
   nir_lower_io(nir, nir_var_shader_in, type_size_vec4,
                nir_lower_io_lower_64bit_to_32);

   /* This pass needs actual constants */
   nir_opt_constant_folding(nir);

   nir_io_add_const_offset_to_base(nir, nir_var_shader_in);

   brw_nir_apply_attribute_workarounds(nir, vs_attrib_wa_flags);

   /* The last step is to remap VERT_ATTRIB_* to actual registers */

   /* Whether or not we have any system generated values.  gl_DrawID is not
    * included here as it lives in its own vec4.
    */
   const bool has_sgvs =
      BITSET_TEST(nir->info.system_values_read, SYSTEM_VALUE_FIRST_VERTEX) ||
      BITSET_TEST(nir->info.system_values_read, SYSTEM_VALUE_BASE_INSTANCE) ||
      BITSET_TEST(nir->info.system_values_read, SYSTEM_VALUE_VERTEX_ID_ZERO_BASE) ||
      BITSET_TEST(nir->info.system_values_read, SYSTEM_VALUE_INSTANCE_ID);

   const unsigned num_inputs = util_bitcount64(nir->info.inputs_read);

   nir_foreach_function_impl(impl, nir) {
      nir_builder b = nir_builder_create(impl);

      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

            switch (intrin->intrinsic) {
            case nir_intrinsic_load_first_vertex:
            case nir_intrinsic_load_base_instance:
            case nir_intrinsic_load_vertex_id_zero_base:
            case nir_intrinsic_load_instance_id:
            case nir_intrinsic_load_is_indexed_draw:
            case nir_intrinsic_load_draw_id: {
               b.cursor = nir_after_instr(&intrin->instr);

               /* gl_VertexID and friends are stored by the VF as the last
                * vertex element.  We convert them to load_input intrinsics at
                * the right location.
                */
               nir_intrinsic_instr *load =
                  nir_intrinsic_instr_create(nir, nir_intrinsic_load_input);
               load->src[0] = nir_src_for_ssa(nir_imm_int(&b, 0));

               nir_intrinsic_set_base(load, num_inputs);
               switch (intrin->intrinsic) {
               case nir_intrinsic_load_first_vertex:
                  nir_intrinsic_set_component(load, 0);
                  break;
               case nir_intrinsic_load_base_instance:
                  nir_intrinsic_set_component(load, 1);
                  break;
               case nir_intrinsic_load_vertex_id_zero_base:
                  nir_intrinsic_set_component(load, 2);
                  break;
               case nir_intrinsic_load_instance_id:
                  nir_intrinsic_set_component(load, 3);
                  break;
               case nir_intrinsic_load_draw_id:
               case nir_intrinsic_load_is_indexed_draw:
                  /* gl_DrawID and IsIndexedDraw are stored right after
                   * gl_VertexID and friends if any of them exist.
                   */
                  nir_intrinsic_set_base(load, num_inputs + has_sgvs);
                  if (intrin->intrinsic == nir_intrinsic_load_draw_id)
                     nir_intrinsic_set_component(load, 0);
                  else
                     nir_intrinsic_set_component(load, 1);
                  break;
               default:
                  unreachable("Invalid system value intrinsic");
               }

               load->num_components = 1;
               nir_def_init(&load->instr, &load->def, 1, 32);
               nir_builder_instr_insert(&b, &load->instr);

               nir_def_rewrite_uses(&intrin->def,
                                        &load->def);
               nir_instr_remove(&intrin->instr);
               break;
            }

            case nir_intrinsic_load_input: {
               /* Attributes come in a contiguous block, ordered by their
                * gl_vert_attrib value.  That means we can compute the slot
                * number for an attribute by masking out the enabled attributes
                * before it and counting the bits.
                */
               int attr = nir_intrinsic_base(intrin);
               uint64_t inputs_read = nir->info.inputs_read;
               int slot = -1;
               if (edgeflag_is_last) {
                  inputs_read &= ~BITFIELD64_BIT(VERT_ATTRIB_EDGEFLAG);
                  if (attr == VERT_ATTRIB_EDGEFLAG)
                     slot = num_inputs - 1;
               }
               if (slot == -1)
                  slot = util_bitcount64(inputs_read &
                                         BITFIELD64_MASK(attr));
               nir_intrinsic_set_base(intrin, slot);
               break;
            }

            default:
               break; /* Nothing to do */
            }
         }
      }
   }
}

void
brw_nir_lower_vue_inputs(nir_shader *nir,
                         const struct brw_vue_map *vue_map)
{
   nir_foreach_shader_in_variable(var, nir)
      var->data.driver_location = var->data.location;

   /* Inputs are stored in vec4 slots, so use type_size_vec4(). */
   nir_lower_io(nir, nir_var_shader_in, type_size_vec4,
                nir_lower_io_lower_64bit_to_32);

   /* This pass needs actual constants */
   nir_opt_constant_folding(nir);

   nir_io_add_const_offset_to_base(nir, nir_var_shader_in);

   nir_foreach_function_impl(impl, nir) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

            if (intrin->intrinsic == nir_intrinsic_load_input ||
                intrin->intrinsic == nir_intrinsic_load_per_vertex_input) {
               /* Offset 0 is the VUE header, which contains
                * VARYING_SLOT_LAYER [.y], VARYING_SLOT_VIEWPORT [.z], and
                * VARYING_SLOT_PSIZ [.w].
                */
               int varying = nir_intrinsic_base(intrin);
               int vue_slot;
               switch (varying) {
               case VARYING_SLOT_PSIZ:
                  nir_intrinsic_set_base(intrin, 0);
                  nir_intrinsic_set_component(intrin, 3);
                  break;

               default:
                  vue_slot = vue_map->varying_to_slot[varying];
                  assert(vue_slot != -1);
                  nir_intrinsic_set_base(intrin, vue_slot);
                  break;
               }
            }
         }
      }
   }
}

void
brw_nir_lower_tes_inputs(nir_shader *nir, const struct brw_vue_map *vue_map)
{
   nir_foreach_shader_in_variable(var, nir)
      var->data.driver_location = var->data.location;

   nir_lower_io(nir, nir_var_shader_in, type_size_vec4,
                nir_lower_io_lower_64bit_to_32);

   /* This pass needs actual constants */
   nir_opt_constant_folding(nir);

   nir_io_add_const_offset_to_base(nir, nir_var_shader_in);

   nir_foreach_function_impl(impl, nir) {
      nir_builder b = nir_builder_create(impl);
      nir_foreach_block(block, impl) {
         remap_patch_urb_offsets(block, &b, vue_map,
                                 nir->info.tess._primitive_mode);
      }
   }
}

static bool
lower_barycentric_per_sample(nir_builder *b,
                             nir_intrinsic_instr *intrin,
                             UNUSED void *cb_data)
{
   if (intrin->intrinsic != nir_intrinsic_load_barycentric_pixel &&
       intrin->intrinsic != nir_intrinsic_load_barycentric_centroid)
      return false;

   b->cursor = nir_before_instr(&intrin->instr);
   nir_def *centroid =
      nir_load_barycentric(b, nir_intrinsic_load_barycentric_sample,
                           nir_intrinsic_interp_mode(intrin));
   nir_def_rewrite_uses(&intrin->def, centroid);
   nir_instr_remove(&intrin->instr);
   return true;
}

/**
 * Convert interpolateAtOffset() offsets from [-0.5, +0.5] floating point
 * offsets to integer [-8, +7] offsets (in units of 1/16th of a pixel).
 *
 * We clamp to +7/16 on the upper end of the range, since +0.5 isn't
 * representable in a S0.4 value; a naive conversion would give us -8/16,
 * which is the opposite of what was intended.
 *
 * This is allowed by GL_ARB_gpu_shader5's quantization rules:
 *
 *    "Not all values of <offset> may be supported; x and y offsets may
 *     be rounded to fixed-point values with the number of fraction bits
 *     given by the implementation-dependent constant
 *     FRAGMENT_INTERPOLATION_OFFSET_BITS."
 */
static bool
lower_barycentric_at_offset(nir_builder *b, nir_intrinsic_instr *intrin,
                            void *data)
{
   if (intrin->intrinsic != nir_intrinsic_load_barycentric_at_offset)
      return false;

   b->cursor = nir_before_instr(&intrin->instr);

   assert(intrin->src[0].ssa);
   nir_def *offset =
      nir_imin(b, nir_imm_int(b, 7),
               nir_f2i32(b, nir_fmul_imm(b, intrin->src[0].ssa, 16)));

   nir_src_rewrite(&intrin->src[0], offset);

   return true;
}

void
brw_nir_lower_fs_inputs(nir_shader *nir,
                        const struct intel_device_info *devinfo,
                        const struct brw_wm_prog_key *key)
{
   nir_foreach_shader_in_variable(var, nir) {
      var->data.driver_location = var->data.location;

      /* Apply default interpolation mode.
       *
       * Everything defaults to smooth except for the legacy GL color
       * built-in variables, which might be flat depending on API state.
       */
      if (var->data.interpolation == INTERP_MODE_NONE) {
         const bool flat = key->flat_shade &&
            (var->data.location == VARYING_SLOT_COL0 ||
             var->data.location == VARYING_SLOT_COL1);

         var->data.interpolation = flat ? INTERP_MODE_FLAT
                                        : INTERP_MODE_SMOOTH;
      }

      /* On Ironlake and below, there is only one interpolation mode.
       * Centroid interpolation doesn't mean anything on this hardware --
       * there is no multisampling.
       */
      if (devinfo->ver < 6) {
         var->data.centroid = false;
         var->data.sample = false;
      }
   }

   nir_lower_io(nir, nir_var_shader_in, type_size_vec4,
                nir_lower_io_lower_64bit_to_32);
   if (devinfo->ver >= 11)
      nir_lower_interpolation(nir, ~0);

   if (key->multisample_fbo == BRW_NEVER) {
      nir_lower_single_sampled(nir);
   } else if (key->persample_interp == BRW_ALWAYS) {
      nir_shader_intrinsics_pass(nir, lower_barycentric_per_sample,
                                   nir_metadata_block_index |
                                   nir_metadata_dominance,
                                   NULL);
   }

   nir_shader_intrinsics_pass(nir, lower_barycentric_at_offset,
                                nir_metadata_block_index |
                                nir_metadata_dominance,
                                NULL);

   /* This pass needs actual constants */
   nir_opt_constant_folding(nir);

   nir_io_add_const_offset_to_base(nir, nir_var_shader_in);
}

void
brw_nir_lower_vue_outputs(nir_shader *nir)
{
   nir_foreach_shader_out_variable(var, nir) {
      var->data.driver_location = var->data.location;
   }

   nir_lower_io(nir, nir_var_shader_out, type_size_vec4,
                nir_lower_io_lower_64bit_to_32);
}

void
brw_nir_lower_tcs_outputs(nir_shader *nir, const struct brw_vue_map *vue_map,
                          enum tess_primitive_mode tes_primitive_mode)
{
   nir_foreach_shader_out_variable(var, nir) {
      var->data.driver_location = var->data.location;
   }

   nir_lower_io(nir, nir_var_shader_out, type_size_vec4,
                nir_lower_io_lower_64bit_to_32);

   /* This pass needs actual constants */
   nir_opt_constant_folding(nir);

   nir_io_add_const_offset_to_base(nir, nir_var_shader_out);

   nir_foreach_function_impl(impl, nir) {
      nir_builder b = nir_builder_create(impl);
      nir_foreach_block(block, impl) {
         remap_patch_urb_offsets(block, &b, vue_map, tes_primitive_mode);
      }
   }
}

void
brw_nir_lower_fs_outputs(nir_shader *nir)
{
   nir_foreach_shader_out_variable(var, nir) {
      var->data.driver_location =
         SET_FIELD(var->data.index, BRW_NIR_FRAG_OUTPUT_INDEX) |
         SET_FIELD(var->data.location, BRW_NIR_FRAG_OUTPUT_LOCATION);
   }

   nir_lower_io(nir, nir_var_shader_out, type_size_dvec4, 0);
}

#define OPT(pass, ...) ({                                  \
   bool this_progress = false;                             \
   NIR_PASS(this_progress, nir, pass, ##__VA_ARGS__);      \
   if (this_progress)                                      \
      progress = true;                                     \
   this_progress;                                          \
})

void
brw_nir_optimize(nir_shader *nir, bool is_scalar,
                 const struct intel_device_info *devinfo)
{
   bool progress;
   unsigned lower_flrp =
      (nir->options->lower_flrp16 ? 16 : 0) |
      (nir->options->lower_flrp32 ? 32 : 0) |
      (nir->options->lower_flrp64 ? 64 : 0);

   do {
      progress = false;
      /* This pass is causing problems with types used by OpenCL :
       *    https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/13955
       *
       * Running with it disabled made no difference in the resulting assembly
       * code.
       */
      if (nir->info.stage != MESA_SHADER_KERNEL)
         OPT(nir_split_array_vars, nir_var_function_temp);
      OPT(nir_shrink_vec_array_vars, nir_var_function_temp);
      OPT(nir_opt_deref);
      if (OPT(nir_opt_memcpy))
         OPT(nir_split_var_copies);
      OPT(nir_lower_vars_to_ssa);
      if (!nir->info.var_copies_lowered) {
         /* Only run this pass if nir_lower_var_copies was not called
          * yet. That would lower away any copy_deref instructions and we
          * don't want to introduce any more.
          */
         OPT(nir_opt_find_array_copies);
      }
      OPT(nir_opt_copy_prop_vars);
      OPT(nir_opt_dead_write_vars);
      OPT(nir_opt_combine_stores, nir_var_all);

      OPT(nir_opt_ray_queries);
      OPT(nir_opt_ray_query_ranges);

      if (is_scalar) {
         OPT(nir_lower_alu_to_scalar, NULL, NULL);
      } else {
         OPT(nir_opt_shrink_stores, true);
         OPT(nir_opt_shrink_vectors);
      }

      OPT(nir_copy_prop);

      if (is_scalar) {
         OPT(nir_lower_phis_to_scalar, false);
      }

      OPT(nir_copy_prop);
      OPT(nir_opt_dce);
      OPT(nir_opt_cse);
      OPT(nir_opt_combine_stores, nir_var_all);

      /* Passing 0 to the peephole select pass causes it to convert
       * if-statements that contain only move instructions in the branches
       * regardless of the count.
       *
       * Passing 1 to the peephole select pass causes it to convert
       * if-statements that contain at most a single ALU instruction (total)
       * in both branches.  Before Gfx6, some math instructions were
       * prohibitively expensive and the results of compare operations need an
       * extra resolve step.  For these reasons, this pass is more harmful
       * than good on those platforms.
       *
       * For indirect loads of uniforms (push constants), we assume that array
       * indices will nearly always be in bounds and the cost of the load is
       * low.  Therefore there shouldn't be a performance benefit to avoid it.
       * However, in vec4 tessellation shaders, these loads operate by
       * actually pulling from memory.
       */
      const bool is_vec4_tessellation = !is_scalar &&
         (nir->info.stage == MESA_SHADER_TESS_CTRL ||
          nir->info.stage == MESA_SHADER_TESS_EVAL);
      OPT(nir_opt_peephole_select, 0, !is_vec4_tessellation, false);
      OPT(nir_opt_peephole_select, 8, !is_vec4_tessellation,
          devinfo->ver >= 6);

      OPT(nir_opt_intrinsics);
      OPT(nir_opt_idiv_const, 32);
      OPT(nir_opt_algebraic);

      /* BFI2 did not exist until Gfx7, so there's no point in trying to
       * optimize an instruction that should not get generated.
       */
      if (devinfo->ver >= 7)
         OPT(nir_opt_reassociate_bfi);

      OPT(nir_lower_constant_convert_alu_types);
      OPT(nir_opt_constant_folding);

      if (lower_flrp != 0) {
         if (OPT(nir_lower_flrp,
                 lower_flrp,
                 false /* always_precise */)) {
            OPT(nir_opt_constant_folding);
         }

         /* Nothing should rematerialize any flrps, so we only need to do this
          * lowering once.
          */
         lower_flrp = 0;
      }

      OPT(nir_opt_dead_cf);
      if (OPT(nir_opt_loop)) {
         /* If nir_opt_loop makes progress, then we need to clean
          * things up if we want any hope of nir_opt_if or nir_opt_loop_unroll
          * to make progress.
          */
         OPT(nir_copy_prop);
         OPT(nir_opt_dce);
      }
      OPT(nir_opt_if, nir_opt_if_optimize_phi_true_false);
      OPT(nir_opt_conditional_discard);
      if (nir->options->max_unroll_iterations != 0) {
         OPT(nir_opt_loop_unroll);
      }
      OPT(nir_opt_remove_phis);
      OPT(nir_opt_gcm, false);
      OPT(nir_opt_undef);
      OPT(nir_lower_pack);
   } while (progress);

   /* Workaround Gfxbench unused local sampler variable which will trigger an
    * assert in the opt_large_constants pass.
    */
   OPT(nir_remove_dead_variables, nir_var_function_temp, NULL);
}

static unsigned
lower_bit_size_callback(const nir_instr *instr, UNUSED void *data)
{
   const struct brw_compiler *compiler = (const struct brw_compiler *) data;
   const struct intel_device_info *devinfo = compiler->devinfo;

   switch (instr->type) {
   case nir_instr_type_alu: {
      nir_alu_instr *alu = nir_instr_as_alu(instr);
      switch (alu->op) {
      case nir_op_bit_count:
      case nir_op_ufind_msb:
      case nir_op_ifind_msb:
      case nir_op_find_lsb:
         /* These are handled specially because the destination is always
          * 32-bit and so the bit size of the instruction is given by the
          * source.
          */
         return alu->src[0].src.ssa->bit_size >= 32 ? 0 : 32;
      default:
         break;
      }

      if (alu->def.bit_size >= 32)
         return 0;

      /* Note: nir_op_iabs and nir_op_ineg are not lowered here because the
       * 8-bit ABS or NEG instruction should eventually get copy propagated
       * into the MOV that does the type conversion.  This results in far
       * fewer MOV instructions.
       */
      switch (alu->op) {
      case nir_op_idiv:
      case nir_op_imod:
      case nir_op_irem:
      case nir_op_udiv:
      case nir_op_umod:
      case nir_op_fceil:
      case nir_op_ffloor:
      case nir_op_ffract:
      case nir_op_fround_even:
      case nir_op_ftrunc:
         return 32;
      case nir_op_frcp:
      case nir_op_frsq:
      case nir_op_fsqrt:
      case nir_op_fpow:
      case nir_op_fexp2:
      case nir_op_flog2:
      case nir_op_fsin:
      case nir_op_fcos:
         return devinfo->ver < 9 ? 32 : 0;
      case nir_op_isign:
         assert(!"Should have been lowered by nir_opt_algebraic.");
         return 0;
      default:
         if (nir_op_infos[alu->op].num_inputs >= 2 &&
             alu->def.bit_size == 8)
            return 16;

         if (nir_alu_instr_is_comparison(alu) &&
             alu->src[0].src.ssa->bit_size == 8)
            return 16;

         return 0;
      }
      break;
   }

   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      switch (intrin->intrinsic) {
      case nir_intrinsic_read_invocation:
      case nir_intrinsic_read_first_invocation:
      case nir_intrinsic_vote_feq:
      case nir_intrinsic_vote_ieq:
      case nir_intrinsic_shuffle:
      case nir_intrinsic_shuffle_xor:
      case nir_intrinsic_shuffle_up:
      case nir_intrinsic_shuffle_down:
      case nir_intrinsic_quad_broadcast:
      case nir_intrinsic_quad_swap_horizontal:
      case nir_intrinsic_quad_swap_vertical:
      case nir_intrinsic_quad_swap_diagonal:
         if (intrin->src[0].ssa->bit_size == 8)
            return 16;
         return 0;

      case nir_intrinsic_reduce:
      case nir_intrinsic_inclusive_scan:
      case nir_intrinsic_exclusive_scan:
         /* There are a couple of register region issues that make things
          * complicated for 8-bit types:
          *
          *    1. Only raw moves are allowed to write to a packed 8-bit
          *       destination.
          *    2. If we use a strided destination, the efficient way to do
          *       scan operations ends up using strides that are too big to
          *       encode in an instruction.
          *
          * To get around these issues, we just do all 8-bit scan operations
          * in 16 bits.  It's actually fewer instructions than what we'd have
          * to do if we were trying to do it in native 8-bit types and the
          * results are the same once we truncate to 8 bits at the end.
          */
         if (intrin->def.bit_size == 8)
            return 16;
         return 0;

      default:
         return 0;
      }
      break;
   }

   case nir_instr_type_phi: {
      nir_phi_instr *phi = nir_instr_as_phi(instr);
      if (phi->def.bit_size == 8)
         return 16;
      return 0;
   }

   default:
      return 0;
   }
}

/* On gfx12.5+, if the offsets are not both constant and in the {-8,7} range,
 * we will have nir_lower_tex() lower the source offset by returning true from
 * this filter function.
 */
static bool
lower_xehp_tg4_offset_filter(const nir_instr *instr, UNUSED const void *data)
{
   if (instr->type != nir_instr_type_tex)
      return false;

   nir_tex_instr *tex = nir_instr_as_tex(instr);

   if (tex->op != nir_texop_tg4)
      return false;

   int offset_index = nir_tex_instr_src_index(tex, nir_tex_src_offset);
   if (offset_index < 0)
      return false;

   if (!nir_src_is_const(tex->src[offset_index].src))
      return true;

   int64_t offset_x = nir_src_comp_as_int(tex->src[offset_index].src, 0);
   int64_t offset_y = nir_src_comp_as_int(tex->src[offset_index].src, 1);

   return offset_x < -8 || offset_x > 7 || offset_y < -8 || offset_y > 7;
}

/* Does some simple lowering and runs the standard suite of optimizations
 *
 * This is intended to be called more-or-less directly after you get the
 * shader out of GLSL or some other source.  While it is geared towards i965,
 * it is not at all generator-specific.
 */
void
brw_preprocess_nir(const struct brw_compiler *compiler, nir_shader *nir,
                   const struct brw_nir_compiler_opts *opts)
{
   const struct intel_device_info *devinfo = compiler->devinfo;
   UNUSED bool progress; /* Written by OPT */

   const bool is_scalar = compiler->scalar_stage[nir->info.stage];

   nir_validate_ssa_dominance(nir, "before brw_preprocess_nir");

   OPT(nir_lower_frexp);

   if (is_scalar) {
      OPT(nir_lower_alu_to_scalar, NULL, NULL);
   }

   if (nir->info.stage == MESA_SHADER_GEOMETRY)
      OPT(nir_lower_gs_intrinsics, 0);

   /* See also brw_nir_trig_workarounds.py */
   if (compiler->precise_trig &&
       !(devinfo->ver >= 10 || devinfo->platform == INTEL_PLATFORM_KBL))
      OPT(brw_nir_apply_trig_workarounds);

   /* This workaround existing for performance reasons. Since it requires not
    * setting RENDER_SURFACE_STATE::SurfaceArray when the array length is 1,
    * we're loosing the HW robustness feature in that case.
    *
    * So when robust image access is enabled, just avoid the workaround.
    */
   if (intel_needs_workaround(devinfo, 1806565034) && !opts->robust_image_access)
      OPT(brw_nir_clamp_image_1d_2d_array_sizes);

   const nir_lower_tex_options tex_options = {
      .lower_txp = ~0,
      .lower_txf_offset = true,
      .lower_rect_offset = true,
      .lower_txd_cube_map = true,
      /* For below, See bspec 45942, "Enable new message layout for cube array" */
      .lower_txd_3d = devinfo->verx10 >= 125,
      .lower_txd_array = devinfo->verx10 >= 125,
      .lower_txb_shadow_clamp = true,
      .lower_txd_shadow_clamp = true,
      .lower_txd_offset_clamp = true,
      .lower_tg4_offsets = true,
      .lower_txs_lod = true, /* Wa_14012320009 */
      .lower_offset_filter =
         devinfo->verx10 >= 125 ? lower_xehp_tg4_offset_filter : NULL,
      .lower_invalid_implicit_lod = true,
   };

   /* In the case where TG4 coords are lowered to offsets and we have a
    * lower_xehp_tg4_offset_filter lowering those offsets further, we need to
    * rerun the pass because the instructions inserted by the first lowering
    * are not visible during that first pass.
    */
   if (OPT(nir_lower_tex, &tex_options))
      OPT(nir_lower_tex, &tex_options);
   OPT(nir_normalize_cubemap_coords);

   OPT(nir_lower_global_vars_to_local);

   OPT(nir_split_var_copies);
   OPT(nir_split_struct_vars, nir_var_function_temp);

   brw_nir_optimize(nir, is_scalar, devinfo);

   OPT(nir_lower_doubles, opts->softfp64, nir->options->lower_doubles_options);
   if (OPT(nir_lower_int64_float_conversions)) {
      OPT(nir_opt_algebraic);
      OPT(nir_lower_doubles, opts->softfp64,
          nir->options->lower_doubles_options);
   }

   OPT(nir_lower_bit_size, lower_bit_size_callback, (void *)compiler);

   /* Lower a bunch of stuff */
   OPT(nir_lower_var_copies);

   /* This needs to be run after the first optimization pass but before we
    * lower indirect derefs away
    */
   if (compiler->supports_shader_constants) {
      OPT(nir_opt_large_constants, NULL, 32);
   }

   if (is_scalar) {
      OPT(nir_lower_load_const_to_scalar);
   }

   OPT(nir_lower_system_values);
   nir_lower_compute_system_values_options lower_csv_options = {
      .has_base_workgroup_id = nir->info.stage == MESA_SHADER_COMPUTE,
   };
   OPT(nir_lower_compute_system_values, &lower_csv_options);

   const nir_lower_subgroups_options subgroups_options = {
      .ballot_bit_size = 32,
      .ballot_components = 1,
      .lower_to_scalar = true,
      .lower_vote_trivial = !is_scalar,
      .lower_relative_shuffle = true,
      .lower_quad_broadcast_dynamic = true,
      .lower_elect = true,
      .lower_inverse_ballot = true,
   };
   OPT(nir_lower_subgroups, &subgroups_options);

   nir_variable_mode indirect_mask =
      brw_nir_no_indirect_mask(compiler, nir->info.stage);
   OPT(nir_lower_indirect_derefs, indirect_mask, UINT32_MAX);

   /* Even in cases where we can handle indirect temporaries via scratch, we
    * it can still be expensive.  Lower indirects on small arrays to
    * conditional load/stores.
    *
    * The threshold of 16 was chosen semi-arbitrarily.  The idea is that an
    * indirect on an array of 16 elements is about 30 instructions at which
    * point, you may be better off doing a send.  With a SIMD8 program, 16
    * floats is 1/8 of the entire register file.  Any array larger than that
    * is likely to cause pressure issues.  Also, this value is sufficiently
    * high that the benchmarks known to suffer from large temporary array
    * issues are helped but nothing else in shader-db is hurt except for maybe
    * that one kerbal space program shader.
    */
   if (is_scalar && !(indirect_mask & nir_var_function_temp))
      OPT(nir_lower_indirect_derefs, nir_var_function_temp, 16);

   /* Lower array derefs of vectors for SSBO and UBO loads.  For both UBOs and
    * SSBOs, our back-end is capable of loading an entire vec4 at a time and
    * we would like to take advantage of that whenever possible regardless of
    * whether or not the app gives us full loads.  This should allow the
    * optimizer to combine UBO and SSBO load operations and save us some send
    * messages.
    */
   OPT(nir_lower_array_deref_of_vec,
       nir_var_mem_ubo | nir_var_mem_ssbo,
       nir_lower_direct_array_deref_of_vec_load);

   /* Clamp load_per_vertex_input of the TCS stage so that we do not generate
    * loads reading out of bounds. We can do this here because we called
    * nir_lower_system_values above.
    */
   if (nir->info.stage == MESA_SHADER_TESS_CTRL &&
       compiler->use_tcs_multi_patch)
      OPT(brw_nir_clamp_per_vertex_loads);

   /* Get rid of split copies */
   brw_nir_optimize(nir, is_scalar, devinfo);
}

static bool
brw_nir_zero_inputs_instr(struct nir_builder *b, nir_intrinsic_instr *intrin,
                          void *data)
{
   if (intrin->intrinsic != nir_intrinsic_load_deref)
      return false;

   nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
   if (!nir_deref_mode_is(deref, nir_var_shader_in))
      return false;

   if (deref->deref_type != nir_deref_type_var)
      return false;

   nir_variable *var = deref->var;

   uint64_t zero_inputs = *(uint64_t *)data;
   if (!(BITFIELD64_BIT(var->data.location) & zero_inputs))
      return false;

   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *zero = nir_imm_zero(b, 1, 32);

   nir_def_rewrite_uses(&intrin->def, zero);

   nir_instr_remove(&intrin->instr);

   return true;
}

static bool
brw_nir_zero_inputs(nir_shader *shader, uint64_t *zero_inputs)
{
   return nir_shader_intrinsics_pass(shader, brw_nir_zero_inputs_instr,
                                     nir_metadata_block_index | nir_metadata_dominance,
                                     zero_inputs);
}

/* Code for Wa_18019110168 may have created input/output variables beyond
 * VARYING_SLOT_MAX and removed uses of variables below VARYING_SLOT_MAX.
 * Clean it up, so they all stay below VARYING_SLOT_MAX.
 */
static void
brw_mesh_compact_io(nir_shader *mesh, nir_shader *frag)
{
   gl_varying_slot mapping[VARYING_SLOT_MAX] = {0, };
   gl_varying_slot cur = VARYING_SLOT_VAR0;
   bool compact = false;

   nir_foreach_shader_out_variable(var, mesh) {
      gl_varying_slot location = var->data.location;
      if (location < VARYING_SLOT_VAR0)
         continue;
      assert(location < ARRAY_SIZE(mapping));

      const struct glsl_type *type = var->type;
      if (nir_is_arrayed_io(var, MESA_SHADER_MESH) || var->data.per_view) {
         assert(glsl_type_is_array(type));
         type = glsl_get_array_element(type);
      }

      if (mapping[location])
         continue;

      unsigned num_slots = glsl_count_attribute_slots(type, false);

      compact |= location + num_slots > VARYING_SLOT_MAX;

      mapping[location] = cur;
      cur += num_slots;
   }

   if (!compact)
      return;

   /* The rest of this function should be hit only for Wa_18019110168. */

   nir_foreach_shader_out_variable(var, mesh) {
      gl_varying_slot location = var->data.location;
      if (location < VARYING_SLOT_VAR0)
         continue;
      location = mapping[location];
      if (location == 0)
         continue;
      var->data.location = location;
   }

   nir_foreach_shader_in_variable(var, frag) {
      gl_varying_slot location = var->data.location;
      if (location < VARYING_SLOT_VAR0)
         continue;
      location = mapping[location];
      if (location == 0)
         continue;
      var->data.location = location;
   }

   nir_shader_gather_info(mesh, nir_shader_get_entrypoint(mesh));
   nir_shader_gather_info(frag, nir_shader_get_entrypoint(frag));

   if (should_print_nir(mesh)) {
      printf("%s\n", __func__);
      nir_print_shader(mesh, stdout);
   }
   if (should_print_nir(frag)) {
      printf("%s\n", __func__);
      nir_print_shader(frag, stdout);
   }
}

void
brw_nir_link_shaders(const struct brw_compiler *compiler,
                     nir_shader *producer, nir_shader *consumer)
{
   const struct intel_device_info *devinfo = compiler->devinfo;

   if (producer->info.stage == MESA_SHADER_MESH &&
       consumer->info.stage == MESA_SHADER_FRAGMENT) {
      uint64_t fs_inputs = 0, ms_outputs = 0;
      /* gl_MeshPerPrimitiveEXT[].gl_ViewportIndex, gl_PrimitiveID and gl_Layer
       * are per primitive, but fragment shader does not have them marked as
       * such. Add the annotation here.
       */
      nir_foreach_shader_in_variable(var, consumer) {
         fs_inputs |= BITFIELD64_BIT(var->data.location);

         switch (var->data.location) {
            case VARYING_SLOT_LAYER:
            case VARYING_SLOT_PRIMITIVE_ID:
            case VARYING_SLOT_VIEWPORT:
               var->data.per_primitive = 1;
               break;
            default:
               continue;
         }
      }

      nir_foreach_shader_out_variable(var, producer)
         ms_outputs |= BITFIELD64_BIT(var->data.location);

      uint64_t zero_inputs = ~ms_outputs & fs_inputs;
      zero_inputs &= BITFIELD64_BIT(VARYING_SLOT_LAYER) |
                     BITFIELD64_BIT(VARYING_SLOT_VIEWPORT);

      if (zero_inputs)
         NIR_PASS(_, consumer, brw_nir_zero_inputs, &zero_inputs);
   }

   nir_lower_io_arrays_to_elements(producer, consumer);
   nir_validate_shader(producer, "after nir_lower_io_arrays_to_elements");
   nir_validate_shader(consumer, "after nir_lower_io_arrays_to_elements");

   const bool p_is_scalar = compiler->scalar_stage[producer->info.stage];
   const bool c_is_scalar = compiler->scalar_stage[consumer->info.stage];

   if (p_is_scalar && c_is_scalar) {
      NIR_PASS(_, producer, nir_lower_io_to_scalar_early, nir_var_shader_out);
      NIR_PASS(_, consumer, nir_lower_io_to_scalar_early, nir_var_shader_in);
      brw_nir_optimize(producer, p_is_scalar, devinfo);
      brw_nir_optimize(consumer, c_is_scalar, devinfo);
   }

   if (nir_link_opt_varyings(producer, consumer))
      brw_nir_optimize(consumer, c_is_scalar, devinfo);

   NIR_PASS(_, producer, nir_remove_dead_variables, nir_var_shader_out, NULL);
   NIR_PASS(_, consumer, nir_remove_dead_variables, nir_var_shader_in, NULL);

   if (nir_remove_unused_varyings(producer, consumer)) {
      if (should_print_nir(producer)) {
         printf("nir_remove_unused_varyings\n");
         nir_print_shader(producer, stdout);
      }
      if (should_print_nir(consumer)) {
         printf("nir_remove_unused_varyings\n");
         nir_print_shader(consumer, stdout);
      }

      NIR_PASS(_, producer, nir_lower_global_vars_to_local);
      NIR_PASS(_, consumer, nir_lower_global_vars_to_local);

      /* The backend might not be able to handle indirects on
       * temporaries so we need to lower indirects on any of the
       * varyings we have demoted here.
       */
      NIR_PASS(_, producer, nir_lower_indirect_derefs,
                  brw_nir_no_indirect_mask(compiler, producer->info.stage),
                  UINT32_MAX);
      NIR_PASS(_, consumer, nir_lower_indirect_derefs,
                  brw_nir_no_indirect_mask(compiler, consumer->info.stage),
                  UINT32_MAX);

      brw_nir_optimize(producer, p_is_scalar, devinfo);
      brw_nir_optimize(consumer, c_is_scalar, devinfo);

      if (producer->info.stage == MESA_SHADER_MESH &&
            consumer->info.stage == MESA_SHADER_FRAGMENT) {
         brw_mesh_compact_io(producer, consumer);
      }
   }

   NIR_PASS(_, producer, nir_lower_io_to_vector, nir_var_shader_out);

   if (producer->info.stage == MESA_SHADER_TESS_CTRL &&
       producer->options->vectorize_tess_levels)
   NIR_PASS_V(producer, nir_vectorize_tess_levels);

   NIR_PASS(_, producer, nir_opt_combine_stores, nir_var_shader_out);
   NIR_PASS(_, consumer, nir_lower_io_to_vector, nir_var_shader_in);

   if (producer->info.stage != MESA_SHADER_TESS_CTRL &&
       producer->info.stage != MESA_SHADER_MESH &&
       producer->info.stage != MESA_SHADER_TASK) {
      /* Calling lower_io_to_vector creates output variable writes with
       * write-masks.  On non-TCS outputs, the back-end can't handle it and we
       * need to call nir_lower_io_to_temporaries to get rid of them.  This,
       * in turn, creates temporary variables and extra copy_deref intrinsics
       * that we need to clean up.
       *
       * Note Mesh/Task don't support I/O as temporaries (I/O is shared
       * between whole workgroup, possibly using multiple HW threads). For
       * those write-mask in output is handled by I/O lowering.
       */
      NIR_PASS_V(producer, nir_lower_io_to_temporaries,
                 nir_shader_get_entrypoint(producer), true, false);
      NIR_PASS(_, producer, nir_lower_global_vars_to_local);
      NIR_PASS(_, producer, nir_split_var_copies);
      NIR_PASS(_, producer, nir_lower_var_copies);
   }

   if (producer->info.stage == MESA_SHADER_TASK &&
         consumer->info.stage == MESA_SHADER_MESH) {

      for (unsigned i = 0; i < 3; ++i)
         assert(producer->info.mesh.ts_mesh_dispatch_dimensions[i] <= UINT16_MAX);

      nir_lower_compute_system_values_options options = {
            .lower_workgroup_id_to_index = true,
            .num_workgroups[0] = producer->info.mesh.ts_mesh_dispatch_dimensions[0],
            .num_workgroups[1] = producer->info.mesh.ts_mesh_dispatch_dimensions[1],
            .num_workgroups[2] = producer->info.mesh.ts_mesh_dispatch_dimensions[2],
            /* nir_lower_idiv generates expensive code */
            .shortcut_1d_workgroup_id = compiler->devinfo->verx10 >= 125,
      };

      NIR_PASS(_, consumer, nir_lower_compute_system_values, &options);
   }
}

bool
brw_nir_should_vectorize_mem(unsigned align_mul, unsigned align_offset,
                             unsigned bit_size,
                             unsigned num_components,
                             nir_intrinsic_instr *low,
                             nir_intrinsic_instr *high,
                             void *data)
{
   /* Don't combine things to generate 64-bit loads/stores.  We have to split
    * those back into 32-bit ones anyway and UBO loads aren't split in NIR so
    * we don't want to make a mess for the back-end.
    */
   if (bit_size > 32)
      return false;

   if (low->intrinsic == nir_intrinsic_load_global_const_block_intel ||
       low->intrinsic == nir_intrinsic_load_ubo_uniform_block_intel ||
       low->intrinsic == nir_intrinsic_load_ssbo_uniform_block_intel ||
       low->intrinsic == nir_intrinsic_load_shared_uniform_block_intel ||
       low->intrinsic == nir_intrinsic_load_global_constant_uniform_block_intel) {
      if (num_components > 4) {
         if (!util_is_power_of_two_nonzero(num_components))
            return false;

         if (bit_size != 32)
            return false;

         if (num_components > 32)
            return false;
      }
   } else {
      /* We can handle at most a vec4 right now.  Anything bigger would get
       * immediately split by brw_nir_lower_mem_access_bit_sizes anyway.
       */
      if (num_components > 4)
         return false;
   }


   uint32_t align;
   if (align_offset)
      align = 1 << (ffs(align_offset) - 1);
   else
      align = align_mul;

   if (align < bit_size / 8)
      return false;

   return true;
}

static
bool combine_all_memory_barriers(nir_intrinsic_instr *a,
                                 nir_intrinsic_instr *b,
                                 void *data)
{
   /* Combine control barriers with identical memory semantics. This prevents
    * the second barrier generating a spurious, identical fence message as the
    * first barrier.
    */
   if (nir_intrinsic_memory_modes(a) == nir_intrinsic_memory_modes(b) &&
       nir_intrinsic_memory_semantics(a) == nir_intrinsic_memory_semantics(b) &&
       nir_intrinsic_memory_scope(a) == nir_intrinsic_memory_scope(b)) {
      nir_intrinsic_set_execution_scope(a, MAX2(nir_intrinsic_execution_scope(a),
                                                nir_intrinsic_execution_scope(b)));
      return true;
   }

   /* Only combine pure memory barriers */
   if ((nir_intrinsic_execution_scope(a) != SCOPE_NONE) ||
       (nir_intrinsic_execution_scope(b) != SCOPE_NONE))
      return false;

   /* Translation to backend IR will get rid of modes we don't care about, so
    * no harm in always combining them.
    *
    * TODO: While HW has only ACQUIRE|RELEASE fences, we could improve the
    * scheduling so that it can take advantage of the different semantics.
    */
   nir_intrinsic_set_memory_modes(a, nir_intrinsic_memory_modes(a) |
                                     nir_intrinsic_memory_modes(b));
   nir_intrinsic_set_memory_semantics(a, nir_intrinsic_memory_semantics(a) |
                                         nir_intrinsic_memory_semantics(b));
   nir_intrinsic_set_memory_scope(a, MAX2(nir_intrinsic_memory_scope(a),
                                          nir_intrinsic_memory_scope(b)));
   return true;
}

static nir_mem_access_size_align
get_mem_access_size_align(nir_intrinsic_op intrin, uint8_t bytes,
                          uint8_t bit_size, uint32_t align_mul, uint32_t align_offset,
                          bool offset_is_const, const void *cb_data)
{
   const uint32_t align = nir_combined_align(align_mul, align_offset);

   switch (intrin) {
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_load_shared:
   case nir_intrinsic_load_scratch:
      /* The offset is constant so we can use a 32-bit load and just shift it
       * around as needed.
       */
      if (align < 4 && offset_is_const) {
         assert(util_is_power_of_two_nonzero(align_mul) && align_mul >= 4);
         const unsigned pad = align_offset % 4;
         const unsigned comps32 = MIN2(DIV_ROUND_UP(bytes + pad, 4), 4);
         return (nir_mem_access_size_align) {
            .bit_size = 32,
            .num_components = comps32,
            .align = 4,
         };
      }
      break;

   case nir_intrinsic_load_task_payload:
      if (bytes < 4 || align < 4) {
         return (nir_mem_access_size_align) {
            .bit_size = 32,
            .num_components = 1,
            .align = 4,
         };
      }
      break;

   default:
      break;
   }

   const bool is_load = nir_intrinsic_infos[intrin].has_dest;
   const bool is_scratch = intrin == nir_intrinsic_load_scratch ||
                           intrin == nir_intrinsic_store_scratch;

   if (align < 4 || bytes < 4) {
      /* Choose a byte, word, or dword */
      bytes = MIN2(bytes, 4);
      if (bytes == 3)
         bytes = is_load ? 4 : 2;

      if (is_scratch) {
         /* The way scratch address swizzling works in the back-end, it
          * happens at a DWORD granularity so we can't have a single load
          * or store cross a DWORD boundary.
          */
         if ((align_offset % 4) + bytes > MIN2(align_mul, 4))
            bytes = MIN2(align_mul, 4) - (align_offset % 4);

         /* Must be a power of two */
         if (bytes == 3)
            bytes = 2;
      }

      return (nir_mem_access_size_align) {
         .bit_size = bytes * 8,
         .num_components = 1,
         .align = 1,
      };
   } else {
      bytes = MIN2(bytes, 16);
      return (nir_mem_access_size_align) {
         .bit_size = 32,
         .num_components = is_scratch ? 1 :
                           is_load ? DIV_ROUND_UP(bytes, 4) : bytes / 4,
         .align = 4,
      };
   }
}

static void
brw_vectorize_lower_mem_access(nir_shader *nir,
                               const struct brw_compiler *compiler,
                               enum brw_robustness_flags robust_flags)
{
   bool progress = false;
   const bool is_scalar = compiler->scalar_stage[nir->info.stage];

   if (is_scalar) {
      nir_load_store_vectorize_options options = {
         .modes = nir_var_mem_ubo | nir_var_mem_ssbo |
                  nir_var_mem_global | nir_var_mem_shared |
                  nir_var_mem_task_payload,
         .callback = brw_nir_should_vectorize_mem,
         .robust_modes = (nir_variable_mode)0,
      };

      if (robust_flags & BRW_ROBUSTNESS_UBO)
         options.robust_modes |= nir_var_mem_ubo | nir_var_mem_global;
      if (robust_flags & BRW_ROBUSTNESS_SSBO)
         options.robust_modes |= nir_var_mem_ssbo | nir_var_mem_global;

      OPT(nir_opt_load_store_vectorize, &options);

      /* Only run the blockify optimization on Gfx9+ because although prior HW
       * versions have support for block loads, they do have limitations on
       * alignment as well as requiring split sends which are not supported
       * there.
       */
      if (compiler->devinfo->ver >= 9) {
         /* Required for nir_divergence_analysis() */
         OPT(nir_convert_to_lcssa, true, true);

         /* When HW supports block loads, using the divergence analysis, try
          * to find uniform SSBO loads and turn them into block loads.
          *
          * Rerun the vectorizer after that to make the largest possible block
          * loads.
          *
          * This is a win on 2 fronts :
          *   - fewer send messages
          *   - reduced register pressure
          */
         nir_divergence_analysis(nir);
         if (OPT(brw_nir_blockify_uniform_loads, compiler->devinfo))
            OPT(nir_opt_load_store_vectorize, &options);
         OPT(nir_opt_remove_phis);
      }
   }

   nir_lower_mem_access_bit_sizes_options mem_access_options = {
      .modes = nir_var_mem_ssbo |
               nir_var_mem_constant |
               nir_var_mem_task_payload |
               nir_var_shader_temp |
               nir_var_function_temp |
               nir_var_mem_global |
               nir_var_mem_shared,
      .callback = get_mem_access_size_align,
   };
   OPT(nir_lower_mem_access_bit_sizes, &mem_access_options);

   while (progress) {
      progress = false;

      OPT(nir_lower_pack);
      OPT(nir_copy_prop);
      OPT(nir_opt_dce);
      OPT(nir_opt_cse);
      OPT(nir_opt_algebraic);
      OPT(nir_opt_constant_folding);
   }
}

static bool
nir_shader_has_local_variables(const nir_shader *nir)
{
   nir_foreach_function_impl(impl, nir) {
      if (!exec_list_is_empty(&impl->locals))
         return true;
   }

   return false;
}

/* Prepare the given shader for codegen
 *
 * This function is intended to be called right before going into the actual
 * backend and is highly backend-specific.  Also, once this function has been
 * called on a shader, it will no longer be in SSA form so most optimizations
 * will not work.
 */
void
brw_postprocess_nir(nir_shader *nir, const struct brw_compiler *compiler,
                    bool debug_enabled,
                    enum brw_robustness_flags robust_flags)
{
   const struct intel_device_info *devinfo = compiler->devinfo;
   const bool is_scalar = compiler->scalar_stage[nir->info.stage];

   UNUSED bool progress; /* Written by OPT */

   OPT(brw_nir_lower_sparse_intrinsics);

   OPT(nir_lower_bit_size, lower_bit_size_callback, (void *)compiler);

   OPT(nir_opt_combine_barriers, combine_all_memory_barriers, NULL);

   do {
      progress = false;
      OPT(nir_opt_algebraic_before_ffma);
   } while (progress);

   if (devinfo->verx10 >= 125) {
      /* Lower integer division by constants before nir_lower_idiv. */
      OPT(nir_opt_idiv_const, 32);
      const nir_lower_idiv_options options = {
         .allow_fp16 = false
      };
      OPT(nir_lower_idiv, &options);
   }

   if (gl_shader_stage_can_set_fragment_shading_rate(nir->info.stage))
      NIR_PASS(_, nir, brw_nir_lower_shading_rate_output);

   brw_nir_optimize(nir, is_scalar, devinfo);

   if (is_scalar && nir_shader_has_local_variables(nir)) {
      OPT(nir_lower_vars_to_explicit_types, nir_var_function_temp,
          glsl_get_natural_size_align_bytes);
      OPT(nir_lower_explicit_io, nir_var_function_temp,
          nir_address_format_32bit_offset);
      brw_nir_optimize(nir, is_scalar, devinfo);
   }

   brw_vectorize_lower_mem_access(nir, compiler, robust_flags);

   if (OPT(nir_lower_int64))
      brw_nir_optimize(nir, is_scalar, devinfo);

   if (devinfo->ver >= 6) {
      /* Try and fuse multiply-adds, if successful, run shrink_vectors to
       * avoid peephole_ffma to generate things like this :
       *    vec16 ssa_0 = ...
       *    vec16 ssa_1 = fneg ssa_0
       *    vec1  ssa_2 = ffma ssa_1, ...
       *
       * We want this instead :
       *    vec16 ssa_0 = ...
       *    vec1  ssa_1 = fneg ssa_0.x
       *    vec1  ssa_2 = ffma ssa_1, ...
       */
      if (OPT(brw_nir_opt_peephole_ffma))
         OPT(nir_opt_shrink_vectors);
   }

   if (is_scalar)
      OPT(brw_nir_opt_peephole_imul32x16);

   if (OPT(nir_opt_comparison_pre)) {
      OPT(nir_copy_prop);
      OPT(nir_opt_dce);
      OPT(nir_opt_cse);

      /* Do the select peepehole again.  nir_opt_comparison_pre (combined with
       * the other optimization passes) will have removed at least one
       * instruction from one of the branches of the if-statement, so now it
       * might be under the threshold of conversion to bcsel.
       *
       * See brw_nir_optimize for the explanation of is_vec4_tessellation.
       */
      const bool is_vec4_tessellation = !is_scalar &&
         (nir->info.stage == MESA_SHADER_TESS_CTRL ||
          nir->info.stage == MESA_SHADER_TESS_EVAL);
      OPT(nir_opt_peephole_select, 0, is_vec4_tessellation, false);
      OPT(nir_opt_peephole_select, 1, is_vec4_tessellation,
          compiler->devinfo->ver >= 6);
   }

   do {
      progress = false;
      if (OPT(nir_opt_algebraic_late)) {
         /* At this late stage, anything that makes more constants will wreak
          * havok on the vec4 backend.  The handling of constants in the vec4
          * backend is not good.
          */
         if (is_scalar)
            OPT(nir_opt_constant_folding);

         OPT(nir_copy_prop);
         OPT(nir_opt_dce);
         OPT(nir_opt_cse);
      }
   } while (progress);


   if (OPT(nir_lower_fp16_casts, nir_lower_fp16_split_fp64)) {
      if (OPT(nir_lower_int64)) {
         brw_nir_optimize(nir, is_scalar, devinfo);
      }
   }

   OPT(brw_nir_lower_conversions);

   if (is_scalar)
      OPT(nir_lower_alu_to_scalar, NULL, NULL);

   while (OPT(nir_opt_algebraic_distribute_src_mods)) {
      if (is_scalar)
         OPT(nir_opt_constant_folding);

      OPT(nir_copy_prop);
      OPT(nir_opt_dce);
      OPT(nir_opt_cse);
   }

   OPT(nir_copy_prop);
   OPT(nir_opt_dce);
   OPT(nir_opt_move, nir_move_comparisons);
   OPT(nir_opt_dead_cf);

   bool divergence_analysis_dirty = false;
   NIR_PASS(_, nir, nir_convert_to_lcssa, true, true);
   NIR_PASS_V(nir, nir_divergence_analysis);

   /* TODO: Enable nir_opt_uniform_atomics on Gfx7.x too.
    * It currently fails Vulkan tests on Haswell for an unknown reason.
    *
    * TODO: Using this optimization on RT/OpenCL kernels also seems to cause
    *       issues. Until we can understand those issues, disable it.
    */
   bool opt_uniform_atomic_stage_allowed =
      devinfo->ver >= 8 &&
      nir->info.stage != MESA_SHADER_KERNEL &&
      nir->info.stage != MESA_SHADER_RAYGEN &&
      !gl_shader_stage_is_callable(nir->info.stage);

   if (opt_uniform_atomic_stage_allowed && OPT(nir_opt_uniform_atomics)) {
      const nir_lower_subgroups_options subgroups_options = {
         .ballot_bit_size = 32,
         .ballot_components = 1,
         .lower_elect = true,
      };
      OPT(nir_lower_subgroups, &subgroups_options);

      if (OPT(nir_lower_int64))
         brw_nir_optimize(nir, is_scalar, devinfo);

      divergence_analysis_dirty = true;
   }

   /* Do this only after the last opt_gcm. GCM will undo this lowering. */
   if (nir->info.stage == MESA_SHADER_FRAGMENT) {
      if (divergence_analysis_dirty) {
         NIR_PASS(_, nir, nir_convert_to_lcssa, true, true);
         NIR_PASS_V(nir, nir_divergence_analysis);
      }

      OPT(brw_nir_lower_non_uniform_barycentric_at_sample);
   }

   /* Clean up LCSSA phis */
   OPT(nir_opt_remove_phis);

   OPT(nir_lower_bool_to_int32);
   OPT(nir_copy_prop);
   OPT(nir_opt_dce);

   OPT(nir_lower_locals_to_regs, 32);

   if (unlikely(debug_enabled)) {
      /* Re-index SSA defs so we print more sensible numbers. */
      nir_foreach_function_impl(impl, nir) {
         nir_index_ssa_defs(impl);
      }

      fprintf(stderr, "NIR (SSA form) for %s shader:\n",
              _mesa_shader_stage_to_string(nir->info.stage));
      nir_print_shader(nir, stderr);
   }

   nir_validate_ssa_dominance(nir, "before nir_convert_from_ssa");

   /* Rerun the divergence analysis before convert_from_ssa as this pass has
    * some assert on consistent divergence flags.
    */
   NIR_PASS(_, nir, nir_convert_to_lcssa, true, true);
   NIR_PASS_V(nir, nir_divergence_analysis);
   OPT(nir_opt_remove_phis);

   OPT(nir_convert_from_ssa, true);

   if (!is_scalar) {
      OPT(nir_move_vec_src_uses_to_dest, true);
      OPT(nir_lower_vec_to_regs, NULL, NULL);
   }

   OPT(nir_opt_dce);

   if (OPT(nir_opt_rematerialize_compares))
      OPT(nir_opt_dce);

   OPT(nir_opt_dce);

   /* The mesh stages require this pass to be called at the last minute,
    * but if anything is done by it, it will also constant fold, and that
    * undoes the work done by nir_trivialize_registers, so call it right
    * before that one instead.
    */
   if (nir->info.stage == MESA_SHADER_MESH ||
       nir->info.stage == MESA_SHADER_TASK)
      brw_nir_adjust_payload(nir);

   nir_trivialize_registers(nir);

   /* This is the last pass we run before we start emitting stuff.  It
    * determines when we need to insert boolean resolves on Gen <= 5.  We
    * run it last because it stashes data in instr->pass_flags and we don't
    * want that to be squashed by other NIR passes.
    */
   if (devinfo->ver <= 5)
      brw_nir_analyze_boolean_resolves(nir);

   nir_sweep(nir);

   if (unlikely(debug_enabled)) {
      fprintf(stderr, "NIR (final form) for %s shader:\n",
              _mesa_shader_stage_to_string(nir->info.stage));
      nir_print_shader(nir, stderr);
   }
}

static bool
brw_nir_apply_sampler_key(nir_shader *nir,
                          const struct brw_compiler *compiler,
                          const struct brw_sampler_prog_key_data *key_tex)
{
   const struct intel_device_info *devinfo = compiler->devinfo;
   nir_lower_tex_options tex_options = {
      .lower_txd_clamp_bindless_sampler = true,
      .lower_txd_clamp_if_sampler_index_not_lt_16 = true,
      .lower_invalid_implicit_lod = true,
      .lower_index_to_offset = true,
   };

   /* Iron Lake and prior require lowering of all rectangle textures */
   if (devinfo->ver < 6)
      tex_options.lower_rect = true;

   /* Prior to Broadwell, our hardware can't actually do GL_CLAMP */
   if (devinfo->ver < 8) {
      tex_options.saturate_s = key_tex->gl_clamp_mask[0];
      tex_options.saturate_t = key_tex->gl_clamp_mask[1];
      tex_options.saturate_r = key_tex->gl_clamp_mask[2];
   }

   /* Prior to Haswell, we have to lower gradients on shadow samplers */
   tex_options.lower_txd_shadow = devinfo->verx10 <= 70;

   return nir_lower_tex(nir, &tex_options);
}

static unsigned
get_subgroup_size(const struct shader_info *info, unsigned max_subgroup_size)
{
   switch (info->subgroup_size) {
   case SUBGROUP_SIZE_API_CONSTANT:
      /* We have to use the global constant size. */
      return BRW_SUBGROUP_SIZE;

   case SUBGROUP_SIZE_UNIFORM:
      /* It has to be uniform across all invocations but can vary per stage
       * if we want.  This gives us a bit more freedom.
       *
       * For compute, brw_nir_apply_key is called per-dispatch-width so this
       * is the actual subgroup size and not a maximum.  However, we only
       * invoke one size of any given compute shader so it's still guaranteed
       * to be uniform across invocations.
       */
      return max_subgroup_size;

   case SUBGROUP_SIZE_VARYING:
      /* The subgroup size is allowed to be fully varying.  For geometry
       * stages, we know it's always 8 which is max_subgroup_size so we can
       * return that.  For compute, brw_nir_apply_key is called once per
       * dispatch-width so max_subgroup_size is the real subgroup size.
       *
       * For fragment, we return 0 and let it fall through to the back-end
       * compiler.  This means we can't optimize based on subgroup size but
       * that's a risk the client took when it asked for a varying subgroup
       * size.
       */
      return info->stage == MESA_SHADER_FRAGMENT ? 0 : max_subgroup_size;

   case SUBGROUP_SIZE_REQUIRE_8:
   case SUBGROUP_SIZE_REQUIRE_16:
   case SUBGROUP_SIZE_REQUIRE_32:
      assert(gl_shader_stage_uses_workgroup(info->stage) ||
             (info->stage >= MESA_SHADER_RAYGEN && info->stage <= MESA_SHADER_CALLABLE));
      /* These enum values are expressly chosen to be equal to the subgroup
       * size that they require.
       */
      return info->subgroup_size;

   case SUBGROUP_SIZE_FULL_SUBGROUPS:
   case SUBGROUP_SIZE_REQUIRE_64:
   case SUBGROUP_SIZE_REQUIRE_128:
      break;
   }

   unreachable("Invalid subgroup size type");
}

unsigned
brw_nir_api_subgroup_size(const nir_shader *nir,
                          unsigned hw_subgroup_size)
{
   return get_subgroup_size(&nir->info, hw_subgroup_size);
}

void
brw_nir_apply_key(nir_shader *nir,
                  const struct brw_compiler *compiler,
                  const struct brw_base_prog_key *key,
                  unsigned max_subgroup_size)
{
   bool progress = false;

   OPT(brw_nir_apply_sampler_key, compiler, &key->tex);

   const nir_lower_subgroups_options subgroups_options = {
      .subgroup_size = get_subgroup_size(&nir->info, max_subgroup_size),
      .ballot_bit_size = 32,
      .ballot_components = 1,
      .lower_subgroup_masks = true,
   };
   OPT(nir_lower_subgroups, &subgroups_options);

   if (key->limit_trig_input_range)
      OPT(brw_nir_limit_trig_input_range_workaround);

   if (progress) {
      const bool is_scalar = compiler->scalar_stage[nir->info.stage];
      brw_nir_optimize(nir, is_scalar, compiler->devinfo);
   }
}

enum brw_conditional_mod
brw_cmod_for_nir_comparison(nir_op op)
{
   switch (op) {
   case nir_op_flt:
   case nir_op_flt32:
   case nir_op_ilt:
   case nir_op_ilt32:
   case nir_op_ult:
   case nir_op_ult32:
      return BRW_CONDITIONAL_L;

   case nir_op_fge:
   case nir_op_fge32:
   case nir_op_ige:
   case nir_op_ige32:
   case nir_op_uge:
   case nir_op_uge32:
      return BRW_CONDITIONAL_GE;

   case nir_op_feq:
   case nir_op_feq32:
   case nir_op_ieq:
   case nir_op_ieq32:
   case nir_op_b32all_fequal2:
   case nir_op_b32all_iequal2:
   case nir_op_b32all_fequal3:
   case nir_op_b32all_iequal3:
   case nir_op_b32all_fequal4:
   case nir_op_b32all_iequal4:
      return BRW_CONDITIONAL_Z;

   case nir_op_fneu:
   case nir_op_fneu32:
   case nir_op_ine:
   case nir_op_ine32:
   case nir_op_b32any_fnequal2:
   case nir_op_b32any_inequal2:
   case nir_op_b32any_fnequal3:
   case nir_op_b32any_inequal3:
   case nir_op_b32any_fnequal4:
   case nir_op_b32any_inequal4:
      return BRW_CONDITIONAL_NZ;

   default:
      unreachable("Unsupported NIR comparison op");
   }
}

enum lsc_opcode
lsc_aop_for_nir_intrinsic(const nir_intrinsic_instr *atomic)
{
   switch (nir_intrinsic_atomic_op(atomic)) {
   case nir_atomic_op_iadd: {
      unsigned src_idx;
      switch (atomic->intrinsic) {
      case nir_intrinsic_image_atomic:
      case nir_intrinsic_bindless_image_atomic:
         src_idx = 3;
         break;
      case nir_intrinsic_ssbo_atomic:
         src_idx = 2;
         break;
      case nir_intrinsic_shared_atomic:
      case nir_intrinsic_global_atomic:
         src_idx = 1;
         break;
      default:
         unreachable("Invalid add atomic opcode");
      }

      if (nir_src_is_const(atomic->src[src_idx])) {
         int64_t add_val = nir_src_as_int(atomic->src[src_idx]);
         if (add_val == 1)
            return LSC_OP_ATOMIC_INC;
         else if (add_val == -1)
            return LSC_OP_ATOMIC_DEC;
      }
      return LSC_OP_ATOMIC_ADD;
   }

   case nir_atomic_op_imin: return LSC_OP_ATOMIC_MIN;
   case nir_atomic_op_umin: return LSC_OP_ATOMIC_UMIN;
   case nir_atomic_op_imax: return LSC_OP_ATOMIC_MAX;
   case nir_atomic_op_umax: return LSC_OP_ATOMIC_UMAX;
   case nir_atomic_op_iand: return LSC_OP_ATOMIC_AND;
   case nir_atomic_op_ior:  return LSC_OP_ATOMIC_OR;
   case nir_atomic_op_ixor: return LSC_OP_ATOMIC_XOR;
   case nir_atomic_op_xchg: return LSC_OP_ATOMIC_STORE;
   case nir_atomic_op_cmpxchg: return LSC_OP_ATOMIC_CMPXCHG;

   case nir_atomic_op_fmin: return LSC_OP_ATOMIC_FMIN;
   case nir_atomic_op_fmax: return LSC_OP_ATOMIC_FMAX;
   case nir_atomic_op_fcmpxchg: return LSC_OP_ATOMIC_FCMPXCHG;
   case nir_atomic_op_fadd: return LSC_OP_ATOMIC_FADD;

   default:
      unreachable("Unsupported NIR atomic intrinsic");
   }
}

enum brw_reg_type
brw_type_for_nir_type(const struct intel_device_info *devinfo,
                      nir_alu_type type)
{
   switch (type) {
   case nir_type_uint:
   case nir_type_uint32:
      return BRW_REGISTER_TYPE_UD;
   case nir_type_bool:
   case nir_type_int:
   case nir_type_bool32:
   case nir_type_int32:
      return BRW_REGISTER_TYPE_D;
   case nir_type_float:
   case nir_type_float32:
      return BRW_REGISTER_TYPE_F;
   case nir_type_float16:
      return BRW_REGISTER_TYPE_HF;
   case nir_type_float64:
      return BRW_REGISTER_TYPE_DF;
   case nir_type_int64:
      return devinfo->ver < 8 ? BRW_REGISTER_TYPE_DF : BRW_REGISTER_TYPE_Q;
   case nir_type_uint64:
      return devinfo->ver < 8 ? BRW_REGISTER_TYPE_DF : BRW_REGISTER_TYPE_UQ;
   case nir_type_int16:
      return BRW_REGISTER_TYPE_W;
   case nir_type_uint16:
      return BRW_REGISTER_TYPE_UW;
   case nir_type_int8:
      return BRW_REGISTER_TYPE_B;
   case nir_type_uint8:
      return BRW_REGISTER_TYPE_UB;
   default:
      unreachable("unknown type");
   }

   return BRW_REGISTER_TYPE_F;
}

nir_shader *
brw_nir_create_passthrough_tcs(void *mem_ctx, const struct brw_compiler *compiler,
                               const struct brw_tcs_prog_key *key)
{
   assert(key->input_vertices > 0);

   const nir_shader_compiler_options *options =
      compiler->nir_options[MESA_SHADER_TESS_CTRL];

   uint64_t inputs_read = key->outputs_written &
      ~(VARYING_BIT_TESS_LEVEL_INNER | VARYING_BIT_TESS_LEVEL_OUTER);

   unsigned locations[64];
   unsigned num_locations = 0;

   u_foreach_bit64(varying, inputs_read)
      locations[num_locations++] = varying;

   nir_shader *nir =
      nir_create_passthrough_tcs_impl(options, locations, num_locations,
                                      key->input_vertices);

   ralloc_steal(mem_ctx, nir);

   nir->info.inputs_read = inputs_read;
   nir->info.tess._primitive_mode = key->_tes_primitive_mode;
   nir_validate_shader(nir, "in brw_nir_create_passthrough_tcs");

   struct brw_nir_compiler_opts opts = {};
   brw_preprocess_nir(compiler, nir, &opts);

   return nir;
}

nir_def *
brw_nir_load_global_const(nir_builder *b, nir_intrinsic_instr *load_uniform,
      nir_def *base_addr, unsigned off)
{
   assert(load_uniform->intrinsic == nir_intrinsic_load_uniform);

   unsigned bit_size = load_uniform->def.bit_size;
   assert(bit_size >= 8 && bit_size % 8 == 0);
   unsigned byte_size = bit_size / 8;
   nir_def *sysval;

   if (nir_src_is_const(load_uniform->src[0])) {
      uint64_t offset = off +
                        nir_intrinsic_base(load_uniform) +
                        nir_src_as_uint(load_uniform->src[0]);

      /* Things should be component-aligned. */
      assert(offset % byte_size == 0);

      unsigned suboffset = offset % 64;
      uint64_t aligned_offset = offset - suboffset;

      /* Load two just in case we go over a 64B boundary */
      nir_def *data[2];
      for (unsigned i = 0; i < 2; i++) {
         nir_def *addr = nir_iadd_imm(b, base_addr, aligned_offset + i * 64);
         data[i] = nir_load_global_const_block_intel(b, 16, addr,
                                                     nir_imm_true(b));
      }

      sysval = nir_extract_bits(b, data, 2, suboffset * 8,
                                load_uniform->num_components, bit_size);
   } else {
      nir_def *offset32 =
         nir_iadd_imm(b, load_uniform->src[0].ssa,
                         off + nir_intrinsic_base(load_uniform));
      nir_def *addr = nir_iadd(b, base_addr, nir_u2u64(b, offset32));
      sysval = nir_load_global_constant(b, addr, byte_size,
                                        load_uniform->num_components, bit_size);
   }

   return sysval;
}

const struct glsl_type *
brw_nir_get_var_type(const struct nir_shader *nir, nir_variable *var)
{
   const struct glsl_type *type = var->interface_type;
   if (!type) {
      type = var->type;
      if (nir_is_arrayed_io(var, nir->info.stage) || var->data.per_view) {
         assert(glsl_type_is_array(type));
         type = glsl_get_array_element(type);
      }
   }

   return type;
}

bool
brw_nir_pulls_at_sample(nir_shader *shader)
{
   nir_foreach_function_impl(impl, shader) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

            if (intrin->intrinsic == nir_intrinsic_load_barycentric_at_sample)
               return true;
         }
      }
   }

   return false;
}
