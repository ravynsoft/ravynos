/*
 * Copyright © 2018 Intel Corporation
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

#include "nir.h"
#include "nir_builder.h"
#include "gl_nir.h"
#include "gl_nir_linker.h"
#include "gl_nir_link_varyings.h"
#include "linker_util.h"
#include "main/shader_types.h"
#include "main/consts_exts.h"
#include "main/shaderobj.h"
#include "ir_uniform.h" /* for gl_uniform_storage */
#include "util/glheader.h"
#include "util/perf/cpu_trace.h"

/**
 * This file included general link methods, using NIR, instead of IR as
 * the counter-part glsl/linker.cpp
 */

void
gl_nir_opts(nir_shader *nir)
{
   bool progress;

   MESA_TRACE_FUNC();

   do {
      progress = false;

      NIR_PASS_V(nir, nir_lower_vars_to_ssa);

      /* Linking deals with unused inputs/outputs, but here we can remove
       * things local to the shader in the hopes that we can cleanup other
       * things. This pass will also remove variables with only stores, so we
       * might be able to make progress after it.
       */
      NIR_PASS(progress, nir, nir_remove_dead_variables,
               nir_var_function_temp | nir_var_shader_temp |
               nir_var_mem_shared,
               NULL);

      NIR_PASS(progress, nir, nir_opt_find_array_copies);
      NIR_PASS(progress, nir, nir_opt_copy_prop_vars);
      NIR_PASS(progress, nir, nir_opt_dead_write_vars);

      if (nir->options->lower_to_scalar) {
         NIR_PASS_V(nir, nir_lower_alu_to_scalar,
                    nir->options->lower_to_scalar_filter, NULL);
         NIR_PASS_V(nir, nir_lower_phis_to_scalar, false);
      }

      NIR_PASS_V(nir, nir_lower_alu);
      NIR_PASS_V(nir, nir_lower_pack);
      NIR_PASS(progress, nir, nir_copy_prop);
      NIR_PASS(progress, nir, nir_opt_remove_phis);
      NIR_PASS(progress, nir, nir_opt_dce);
      if (nir_opt_loop(nir)) {
         progress = true;
         NIR_PASS(progress, nir, nir_copy_prop);
         NIR_PASS(progress, nir, nir_opt_dce);
      }
      NIR_PASS(progress, nir, nir_opt_if, 0);
      NIR_PASS(progress, nir, nir_opt_dead_cf);
      NIR_PASS(progress, nir, nir_opt_cse);
      NIR_PASS(progress, nir, nir_opt_peephole_select, 8, true, true);

      NIR_PASS(progress, nir, nir_opt_phi_precision);
      NIR_PASS(progress, nir, nir_opt_algebraic);
      NIR_PASS(progress, nir, nir_opt_constant_folding);

      if (!nir->info.flrp_lowered) {
         unsigned lower_flrp =
            (nir->options->lower_flrp16 ? 16 : 0) |
            (nir->options->lower_flrp32 ? 32 : 0) |
            (nir->options->lower_flrp64 ? 64 : 0);

         if (lower_flrp) {
            bool lower_flrp_progress = false;

            NIR_PASS(lower_flrp_progress, nir, nir_lower_flrp,
                     lower_flrp,
                     false /* always_precise */);
            if (lower_flrp_progress) {
               NIR_PASS(progress, nir,
                        nir_opt_constant_folding);
               progress = true;
            }
         }

         /* Nothing should rematerialize any flrps, so we only need to do this
          * lowering once.
          */
         nir->info.flrp_lowered = true;
      }

      NIR_PASS(progress, nir, nir_opt_undef);
      NIR_PASS(progress, nir, nir_opt_conditional_discard);
      if (nir->options->max_unroll_iterations ||
            (nir->options->max_unroll_iterations_fp64 &&
               (nir->options->lower_doubles_options & nir_lower_fp64_full_software))) {
         NIR_PASS(progress, nir, nir_opt_loop_unroll);
      }
   } while (progress);

   NIR_PASS_V(nir, nir_lower_var_copies);
}

bool
gl_nir_can_add_pointsize_to_program(const struct gl_constants *consts,
                                    struct gl_program *prog)
{
   nir_shader *nir = prog->nir;
   if (!nir)
      return true; /* fixedfunction */

   assert(nir->info.stage == MESA_SHADER_VERTEX ||
          nir->info.stage == MESA_SHADER_TESS_EVAL ||
          nir->info.stage == MESA_SHADER_GEOMETRY);
   if (nir->info.outputs_written & VARYING_BIT_PSIZ)
      return false;

   unsigned max_components = nir->info.stage == MESA_SHADER_GEOMETRY ?
                             consts->MaxGeometryTotalOutputComponents :
                             consts->Program[nir->info.stage].MaxOutputComponents;
   unsigned num_components = 0;
   unsigned needed_components = nir->info.stage == MESA_SHADER_GEOMETRY ? nir->info.gs.vertices_out : 1;
   nir_foreach_shader_out_variable(var, nir) {
      num_components += glsl_count_dword_slots(var->type, false);
   }

   /* Ensure that there is enough attribute space to emit at least one primitive */
   if (num_components && nir->info.stage == MESA_SHADER_GEOMETRY) {
      if (num_components + needed_components > consts->Program[nir->info.stage].MaxOutputComponents)
         return false;
      num_components *= nir->info.gs.vertices_out;
   }

   return num_components + needed_components <= max_components;
}

static void
gl_nir_link_opts(nir_shader *producer, nir_shader *consumer)
{
   MESA_TRACE_FUNC();

   if (producer->options->lower_to_scalar) {
      NIR_PASS_V(producer, nir_lower_io_to_scalar_early, nir_var_shader_out);
      NIR_PASS_V(consumer, nir_lower_io_to_scalar_early, nir_var_shader_in);
   }

   nir_lower_io_arrays_to_elements(producer, consumer);

   gl_nir_opts(producer);
   gl_nir_opts(consumer);

   if (nir_link_opt_varyings(producer, consumer))
      gl_nir_opts(consumer);

   NIR_PASS_V(producer, nir_remove_dead_variables, nir_var_shader_out, NULL);
   NIR_PASS_V(consumer, nir_remove_dead_variables, nir_var_shader_in, NULL);

   if (nir_remove_unused_varyings(producer, consumer)) {
      NIR_PASS_V(producer, nir_lower_global_vars_to_local);
      NIR_PASS_V(consumer, nir_lower_global_vars_to_local);

      gl_nir_opts(producer);
      gl_nir_opts(consumer);

      /* Optimizations can cause varyings to become unused.
       * nir_compact_varyings() depends on all dead varyings being removed so
       * we need to call nir_remove_dead_variables() again here.
       */
      NIR_PASS_V(producer, nir_remove_dead_variables, nir_var_shader_out,
                 NULL);
      NIR_PASS_V(consumer, nir_remove_dead_variables, nir_var_shader_in,
                 NULL);
   }

   nir_link_varying_precision(producer, consumer);
}

static bool
can_remove_var(nir_variable *var, UNUSED void *data)
{
   /* Section 2.11.6 (Uniform Variables) of the OpenGL ES 3.0.3 spec
    * says:
    *
    *     "All members of a named uniform block declared with a shared or
    *     std140 layout qualifier are considered active, even if they are not
    *     referenced in any shader in the program. The uniform block itself is
    *     also considered active, even if no member of the block is
    *     referenced."
    *
    * Although the spec doesn't state it std430 layouts are expect to behave
    * the same way. If the variable is in a uniform block with one of those
    * layouts, do not eliminate it.
    */
   if (nir_variable_is_in_block(var) &&
       (glsl_get_ifc_packing(var->interface_type) !=
        GLSL_INTERFACE_PACKING_PACKED))
      return false;

   if (glsl_get_base_type(glsl_without_array(var->type)) ==
       GLSL_TYPE_SUBROUTINE)
      return false;

   /* Uniform initializers could get used by another stage. However if its a
    * hidden uniform then it should be safe to remove as this was a constant
    * variable that has been lowered to a uniform.
    */
   if (var->constant_initializer && var->data.how_declared != nir_var_hidden)
      return false;

   return true;
}

static void
set_always_active_io(nir_shader *shader, nir_variable_mode io_mode)
{
   assert(io_mode == nir_var_shader_in || io_mode == nir_var_shader_out);

   nir_foreach_variable_with_modes(var, shader, io_mode) {
      /* Don't set always active on builtins that haven't been redeclared */
      if (var->data.how_declared == nir_var_declared_implicitly)
         continue;

      var->data.always_active_io = true;
   }
}

/**
 * When separate shader programs are enabled, only input/outputs between
 * the stages of a multi-stage separate program can be safely removed
 * from the shader interface. Other inputs/outputs must remain active.
 */
static void
disable_varying_optimizations_for_sso(struct gl_shader_program *prog)
{
   unsigned first, last;
   assert(prog->SeparateShader);

   first = MESA_SHADER_STAGES;
   last = 0;

   /* Determine first and last stage. Excluding the compute stage */
   for (unsigned i = 0; i < MESA_SHADER_COMPUTE; i++) {
      if (!prog->_LinkedShaders[i])
         continue;
      if (first == MESA_SHADER_STAGES)
         first = i;
      last = i;
   }

   if (first == MESA_SHADER_STAGES)
      return;

   for (unsigned stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      if (!prog->_LinkedShaders[stage])
         continue;

      /* Prevent the removal of inputs to the first and outputs from the last
       * stage, unless they are the initial pipeline inputs or final pipeline
       * outputs, respectively.
       *
       * The removal of IO between shaders in the same program is always
       * allowed.
       */
      if (stage == first && stage != MESA_SHADER_VERTEX) {
         set_always_active_io(prog->_LinkedShaders[stage]->Program->nir,
                              nir_var_shader_in);
      }

      if (stage == last && stage != MESA_SHADER_FRAGMENT) {
         set_always_active_io(prog->_LinkedShaders[stage]->Program->nir,
                              nir_var_shader_out);
      }
   }
}

static bool
inout_has_same_location(const nir_variable *var, unsigned stage)
{
   if (!var->data.patch &&
       ((var->data.mode == nir_var_shader_out &&
         stage == MESA_SHADER_TESS_CTRL) ||
        (var->data.mode == nir_var_shader_in &&
         (stage == MESA_SHADER_TESS_CTRL || stage == MESA_SHADER_TESS_EVAL ||
          stage == MESA_SHADER_GEOMETRY))))
      return true;
   else
      return false;
}

/**
 * Create gl_shader_variable from nir_variable.
 */
static struct gl_shader_variable *
create_shader_variable(struct gl_shader_program *shProg,
                       const nir_variable *in,
                       const char *name, const struct glsl_type *type,
                       const struct glsl_type *interface_type,
                       bool use_implicit_location, int location,
                       const struct glsl_type *outermost_struct_type)
{
   /* Allocate zero-initialized memory to ensure that bitfield padding
    * is zero.
    */
   struct gl_shader_variable *out = rzalloc(shProg,
                                            struct gl_shader_variable);
   if (!out)
      return NULL;

   /* Since gl_VertexID may be lowered to gl_VertexIDMESA, but applications
    * expect to see gl_VertexID in the program resource list.  Pretend.
    */
   if (in->data.mode == nir_var_system_value &&
       in->data.location == SYSTEM_VALUE_VERTEX_ID_ZERO_BASE) {
      out->name.string = ralloc_strdup(shProg, "gl_VertexID");
   } else if ((in->data.mode == nir_var_shader_out &&
               in->data.location == VARYING_SLOT_TESS_LEVEL_OUTER) ||
              (in->data.mode == nir_var_system_value &&
               in->data.location == SYSTEM_VALUE_TESS_LEVEL_OUTER)) {
      out->name.string = ralloc_strdup(shProg, "gl_TessLevelOuter");
      type = glsl_array_type(glsl_float_type(), 4, 0);
   } else if ((in->data.mode == nir_var_shader_out &&
               in->data.location == VARYING_SLOT_TESS_LEVEL_INNER) ||
              (in->data.mode == nir_var_system_value &&
               in->data.location == SYSTEM_VALUE_TESS_LEVEL_INNER)) {
      out->name.string = ralloc_strdup(shProg, "gl_TessLevelInner");
      type = glsl_array_type(glsl_float_type(), 2, 0);
   } else {
      out->name.string = ralloc_strdup(shProg, name);
   }

   resource_name_updated(&out->name);

   if (!out->name.string)
      return NULL;

   /* The ARB_program_interface_query spec says:
    *
    *     "Not all active variables are assigned valid locations; the
    *     following variables will have an effective location of -1:
    *
    *      * uniforms declared as atomic counters;
    *
    *      * members of a uniform block;
    *
    *      * built-in inputs, outputs, and uniforms (starting with "gl_"); and
    *
    *      * inputs or outputs not declared with a "location" layout
    *        qualifier, except for vertex shader inputs and fragment shader
    *        outputs."
    */
   if (glsl_get_base_type(in->type) == GLSL_TYPE_ATOMIC_UINT ||
       is_gl_identifier(in->name) ||
       !(in->data.explicit_location || use_implicit_location)) {
      out->location = -1;
   } else {
      out->location = location;
   }

   out->type = type;
   out->outermost_struct_type = outermost_struct_type;
   out->interface_type = interface_type;
   out->component = in->data.location_frac;
   out->index = in->data.index;
   out->patch = in->data.patch;
   out->mode = in->data.mode;
   out->interpolation = in->data.interpolation;
   out->precision = in->data.precision;
   out->explicit_location = in->data.explicit_location;

   return out;
}

static bool
add_shader_variable(const struct gl_constants *consts,
                    struct gl_shader_program *shProg,
                    struct set *resource_set,
                    unsigned stage_mask,
                    GLenum programInterface, nir_variable *var,
                    const char *name, const struct glsl_type *type,
                    bool use_implicit_location, int location,
                    bool inouts_share_location,
                    const struct glsl_type *outermost_struct_type)
{
   const struct glsl_type *interface_type = var->interface_type;

   if (outermost_struct_type == NULL) {
      if (var->data.from_named_ifc_block) {
         const char *interface_name = glsl_get_type_name(interface_type);

         if (glsl_type_is_array(interface_type)) {
            /* Issue #16 of the ARB_program_interface_query spec says:
             *
             * "* If a variable is a member of an interface block without an
             *    instance name, it is enumerated using just the variable name.
             *
             *  * If a variable is a member of an interface block with an
             *    instance name, it is enumerated as "BlockName.Member", where
             *    "BlockName" is the name of the interface block (not the
             *    instance name) and "Member" is the name of the variable."
             *
             * In particular, it indicates that it should be "BlockName",
             * not "BlockName[array length]".  The conformance suite and
             * dEQP both require this behavior.
             *
             * Here, we unwrap the extra array level added by named interface
             * block array lowering so we have the correct variable type.  We
             * also unwrap the interface type when constructing the name.
             *
             * We leave interface_type the same so that ES 3.x SSO pipeline
             * validation can enforce the rules requiring array length to
             * match on interface blocks.
             */
            type = glsl_get_array_element(type);

            interface_name =
               glsl_get_type_name(glsl_get_array_element(interface_type));
         }

         name = ralloc_asprintf(shProg, "%s.%s", interface_name, name);
      }
   }

   switch (glsl_get_base_type(type)) {
   case GLSL_TYPE_STRUCT: {
      /* The ARB_program_interface_query spec says:
       *
       *     "For an active variable declared as a structure, a separate entry
       *     will be generated for each active structure member.  The name of
       *     each entry is formed by concatenating the name of the structure,
       *     the "."  character, and the name of the structure member.  If a
       *     structure member to enumerate is itself a structure or array,
       *     these enumeration rules are applied recursively."
       */
      if (outermost_struct_type == NULL)
         outermost_struct_type = type;

      unsigned field_location = location;
      for (unsigned i = 0; i < glsl_get_length(type); i++) {
         const struct glsl_type *field_type = glsl_get_struct_field(type, i);
         const struct glsl_struct_field *field =
            glsl_get_struct_field_data(type, i);

         char *field_name = ralloc_asprintf(shProg, "%s.%s", name, field->name);
         if (!add_shader_variable(consts, shProg, resource_set,
                                  stage_mask, programInterface,
                                  var, field_name, field_type,
                                  use_implicit_location, field_location,
                                  false, outermost_struct_type))
            return false;

         field_location += glsl_count_attribute_slots(field_type, false);
      }
      return true;
   }

   case GLSL_TYPE_ARRAY: {
      /* The ARB_program_interface_query spec says:
       *
       *     "For an active variable declared as an array of basic types, a
       *      single entry will be generated, with its name string formed by
       *      concatenating the name of the array and the string "[0]"."
       *
       *     "For an active variable declared as an array of an aggregate data
       *      type (structures or arrays), a separate entry will be generated
       *      for each active array element, unless noted immediately below.
       *      The name of each entry is formed by concatenating the name of
       *      the array, the "[" character, an integer identifying the element
       *      number, and the "]" character.  These enumeration rules are
       *      applied recursively, treating each enumerated array element as a
       *      separate active variable."
       */
      const struct glsl_type *array_type = glsl_get_array_element(type);
      if (glsl_get_base_type(array_type) == GLSL_TYPE_STRUCT ||
          glsl_get_base_type(array_type) == GLSL_TYPE_ARRAY) {
         unsigned elem_location = location;
         unsigned stride = inouts_share_location ? 0 :
                           glsl_count_attribute_slots(array_type, false);
         for (unsigned i = 0; i < glsl_get_length(type); i++) {
            char *elem = ralloc_asprintf(shProg, "%s[%d]", name, i);
            if (!add_shader_variable(consts, shProg, resource_set,
                                     stage_mask, programInterface,
                                     var, elem, array_type,
                                     use_implicit_location, elem_location,
                                     false, outermost_struct_type))
               return false;
            elem_location += stride;
         }
         return true;
      }
   }
   FALLTHROUGH;

   default: {
      /* The ARB_program_interface_query spec says:
       *
       *     "For an active variable declared as a single instance of a basic
       *     type, a single entry will be generated, using the variable name
       *     from the shader source."
       */
      struct gl_shader_variable *sha_v =
         create_shader_variable(shProg, var, name, type, interface_type,
                                use_implicit_location, location,
                                outermost_struct_type);
      if (!sha_v)
         return false;

      return link_util_add_program_resource(shProg, resource_set,
                                            programInterface, sha_v, stage_mask);
   }
   }
}

static bool
add_vars_with_modes(const struct gl_constants *consts,
                    struct gl_shader_program *prog, struct set *resource_set,
                    nir_shader *nir, nir_variable_mode modes,
                    unsigned stage, GLenum programInterface)
{
   nir_foreach_variable_with_modes(var, nir, modes) {
      if (var->data.how_declared == nir_var_hidden)
         continue;

      int loc_bias = 0;
      switch(var->data.mode) {
      case nir_var_system_value:
      case nir_var_shader_in:
         if (programInterface != GL_PROGRAM_INPUT)
            continue;
         loc_bias = (stage == MESA_SHADER_VERTEX) ? VERT_ATTRIB_GENERIC0
                                                  : VARYING_SLOT_VAR0;
         break;
      case nir_var_shader_out:
         if (programInterface != GL_PROGRAM_OUTPUT)
            continue;
         loc_bias = (stage == MESA_SHADER_FRAGMENT) ? FRAG_RESULT_DATA0
                                                    : VARYING_SLOT_VAR0;
         break;
      default:
         continue;
      }

      if (var->data.patch)
         loc_bias = VARYING_SLOT_PATCH0;

      if (prog->data->spirv) {
         struct gl_shader_variable *sh_var =
            rzalloc(prog, struct gl_shader_variable);

         /* In the ARB_gl_spirv spec, names are considered optional debug info, so
          * the linker needs to work without them. Returning them is optional.
          * For simplicity, we ignore names.
          */
         sh_var->name.string = NULL;
         resource_name_updated(&sh_var->name);
         sh_var->type = var->type;
         sh_var->location = var->data.location - loc_bias;
         sh_var->explicit_location = var->data.explicit_location;
         sh_var->index = var->data.index;

         if (!link_util_add_program_resource(prog, resource_set,
                                             programInterface,
                                             sh_var, 1 << stage)) {
           return false;
         }
      } else {
         /* Skip packed varyings, packed varyings are handled separately
          * by add_packed_varyings in the GLSL IR
          * build_program_resource_list() call.
          * TODO: handle packed varyings here instead. We likely want a NIR
          * based packing pass first.
          */
         if (strncmp(var->name, "packed:", 7) == 0)
            continue;

         const bool vs_input_or_fs_output =
            (stage == MESA_SHADER_VERTEX &&
             var->data.mode == nir_var_shader_in) ||
            (stage == MESA_SHADER_FRAGMENT &&
             var->data.mode == nir_var_shader_out);

         if (!add_shader_variable(consts, prog, resource_set,
                                  1 << stage, programInterface,
                                  var, var->name, var->type,
                                  vs_input_or_fs_output,
                                  var->data.location - loc_bias,
                                  inout_has_same_location(var, stage),
                                  NULL))
            return false;
      }
   }

   return true;
}

static bool
add_interface_variables(const struct gl_constants *consts,
                        struct gl_shader_program *prog,
                        struct set *resource_set,
                        unsigned stage, GLenum programInterface)
{
   struct gl_linked_shader *sh = prog->_LinkedShaders[stage];
   if (!sh)
      return true;

   nir_shader *nir = sh->Program->nir;
   assert(nir);

   switch (programInterface) {
   case GL_PROGRAM_INPUT: {
      return add_vars_with_modes(consts, prog, resource_set,
                                 nir, nir_var_shader_in | nir_var_system_value,
                                 stage, programInterface);
   }
   case GL_PROGRAM_OUTPUT:
      return add_vars_with_modes(consts, prog, resource_set,
                                 nir, nir_var_shader_out,
                                 stage, programInterface);
   default:
      assert("!Should not get here");
      break;
   }

   return false;
}

bool
nir_add_packed_var_to_resource_list(const struct gl_constants *consts,
                                    struct gl_shader_program *shProg,
                                    struct set *resource_set,
                                    nir_variable *var,
                                    unsigned stage, GLenum type)
{
   if (!add_shader_variable(consts, shProg, resource_set, 1 << stage,
                            type, var, var->name, var->type, false,
                            var->data.location - VARYING_SLOT_VAR0,
                            inout_has_same_location(var, stage), NULL))
      return false;

   return true;
}

/**
 * Initilise list of program resources that point to resource data.
 */
void
init_program_resource_list(struct gl_shader_program *prog)
{
   /* Rebuild resource list. */
   if (prog->data->ProgramResourceList) {
      ralloc_free(prog->data->ProgramResourceList);
      prog->data->ProgramResourceList = NULL;
      prog->data->NumProgramResourceList = 0;
   }
}

/* TODO: as we keep adding features, this method is becoming more and more
 * similar to its GLSL counterpart at linker.cpp. Eventually it would be good
 * to check if they could be refactored, and reduce code duplication somehow
 */
void
nir_build_program_resource_list(const struct gl_constants *consts,
                                struct gl_shader_program *prog,
                                bool rebuild_resourse_list)
{
   /* Rebuild resource list. */
   if (rebuild_resourse_list)
      init_program_resource_list(prog);

   int input_stage = MESA_SHADER_STAGES, output_stage = 0;

   /* Determine first input and final output stage. These are used to
    * detect which variables should be enumerated in the resource list
    * for GL_PROGRAM_INPUT and GL_PROGRAM_OUTPUT.
    */
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      if (!prog->_LinkedShaders[i])
         continue;
      if (input_stage == MESA_SHADER_STAGES)
         input_stage = i;
      output_stage = i;
   }

   /* Empty shader, no resources. */
   if (input_stage == MESA_SHADER_STAGES && output_stage == 0)
      return;

   struct set *resource_set = _mesa_pointer_set_create(NULL);

   /* Add inputs and outputs to the resource list. */
   if (!add_interface_variables(consts, prog, resource_set, input_stage,
                                GL_PROGRAM_INPUT)) {
      return;
   }

   if (!add_interface_variables(consts, prog, resource_set, output_stage,
                                GL_PROGRAM_OUTPUT)) {
      return;
   }

   /* Add transform feedback varyings and buffers. */
   if (prog->last_vert_prog) {
      struct gl_transform_feedback_info *linked_xfb =
         prog->last_vert_prog->sh.LinkedTransformFeedback;

      /* Add varyings. */
      if (linked_xfb->NumVarying > 0) {
         for (int i = 0; i < linked_xfb->NumVarying; i++) {
            if (!link_util_add_program_resource(prog, resource_set,
                                                GL_TRANSFORM_FEEDBACK_VARYING,
                                                &linked_xfb->Varyings[i], 0))
            return;
         }
      }

      /* Add buffers. */
      for (unsigned i = 0; i < consts->MaxTransformFeedbackBuffers; i++) {
         if ((linked_xfb->ActiveBuffers >> i) & 1) {
            linked_xfb->Buffers[i].Binding = i;
            if (!link_util_add_program_resource(prog, resource_set,
                                                GL_TRANSFORM_FEEDBACK_BUFFER,
                                                &linked_xfb->Buffers[i], 0))
            return;
         }
      }
   }

   /* Add uniforms
    *
    * Here, it is expected that nir_link_uniforms() has already been
    * called, so that UniformStorage table is already available.
    */
   int top_level_array_base_offset = -1;
   int top_level_array_size_in_bytes = -1;
   int second_element_offset = -1;
   int block_index = -1;
   for (unsigned i = 0; i < prog->data->NumUniformStorage; i++) {
      struct gl_uniform_storage *uniform = &prog->data->UniformStorage[i];

      if (uniform->hidden) {
         for (int j = MESA_SHADER_VERTEX; j < MESA_SHADER_STAGES; j++) {
            if (!uniform->opaque[j].active ||
                glsl_get_base_type(uniform->type) != GLSL_TYPE_SUBROUTINE)
               continue;

            GLenum type =
               _mesa_shader_stage_to_subroutine_uniform((gl_shader_stage)j);
            /* add shader subroutines */
            if (!link_util_add_program_resource(prog, resource_set,
                                                type, uniform, 0))
               return;
         }

         continue;
      }

      if (!link_util_should_add_buffer_variable(prog, uniform,
                                                top_level_array_base_offset,
                                                top_level_array_size_in_bytes,
                                                second_element_offset, block_index))
         continue;


      if (prog->data->UniformStorage[i].offset >= second_element_offset) {
         top_level_array_base_offset =
            prog->data->UniformStorage[i].offset;

         top_level_array_size_in_bytes =
            prog->data->UniformStorage[i].top_level_array_size *
            prog->data->UniformStorage[i].top_level_array_stride;

         /* Set or reset the second element offset. For non arrays this
          * will be set to -1.
          */
         second_element_offset = top_level_array_size_in_bytes ?
            top_level_array_base_offset +
            prog->data->UniformStorage[i].top_level_array_stride : -1;
      }
      block_index = uniform->block_index;


      GLenum interface = uniform->is_shader_storage ? GL_BUFFER_VARIABLE : GL_UNIFORM;
      if (!link_util_add_program_resource(prog, resource_set, interface, uniform,
                                          uniform->active_shader_mask)) {
         return;
      }
   }


   for (unsigned i = 0; i < prog->data->NumUniformBlocks; i++) {
      if (!link_util_add_program_resource(prog, resource_set, GL_UNIFORM_BLOCK,
                                          &prog->data->UniformBlocks[i],
                                          prog->data->UniformBlocks[i].stageref))
         return;
   }

   for (unsigned i = 0; i < prog->data->NumShaderStorageBlocks; i++) {
      if (!link_util_add_program_resource(prog, resource_set, GL_SHADER_STORAGE_BLOCK,
                                          &prog->data->ShaderStorageBlocks[i],
                                          prog->data->ShaderStorageBlocks[i].stageref))
         return;
   }

   /* Add atomic counter buffers. */
   for (unsigned i = 0; i < prog->data->NumAtomicBuffers; i++) {
      if (!link_util_add_program_resource(prog, resource_set, GL_ATOMIC_COUNTER_BUFFER,
                                          &prog->data->AtomicBuffers[i], 0))
         return;
   }

   unsigned mask = prog->data->linked_stages;
   while (mask) {
      const int i = u_bit_scan(&mask);
      struct gl_program *p = prog->_LinkedShaders[i]->Program;

      GLuint type = _mesa_shader_stage_to_subroutine((gl_shader_stage)i);
      for (unsigned j = 0; j < p->sh.NumSubroutineFunctions; j++) {
         if (!link_util_add_program_resource(prog, resource_set,
                                             type,
                                             &p->sh.SubroutineFunctions[j],
                                             0))
            return;
      }
   }

   _mesa_set_destroy(resource_set, NULL);
}

static void
shared_type_info(const struct glsl_type *type, unsigned *size, unsigned *align)
{
   assert(glsl_type_is_vector_or_scalar(type));

   uint32_t comp_size = glsl_type_is_boolean(type)
      ? 4 : glsl_get_bit_size(type) / 8;
   unsigned length = glsl_get_vector_elements(type);
   *size = comp_size * length,
   *align = comp_size * (length == 3 ? 4 : length);
}

static bool
can_remove_varying_before_linking(nir_variable *var, void *data)
{
   bool *is_sso = (bool *) data;
   if (*is_sso) {
      /* Allow the removal of unused builtins in SSO */
      return var->data.location > -1 && var->data.location < VARYING_SLOT_VAR0;
   } else
      return true;
}

static void
remove_dead_varyings_pre_linking(nir_shader *nir)
{
   struct nir_remove_dead_variables_options opts;
   bool is_sso = nir->info.separate_shader;
   opts.can_remove_var_data = &is_sso;
   opts.can_remove_var = &can_remove_varying_before_linking;
   nir_variable_mode mask = nir_var_shader_in | nir_var_shader_out;
   nir_remove_dead_variables(nir, mask, &opts);
}

/* - create a gl_PointSize variable
 * - find every gl_Position write
 * - store 1.0 to gl_PointSize after every gl_Position write
 */
void
gl_nir_add_point_size(nir_shader *nir)
{
   nir_variable *psiz = nir_create_variable_with_location(nir, nir_var_shader_out,
                                                          VARYING_SLOT_PSIZ, glsl_float_type());
   psiz->data.how_declared = nir_var_hidden;

   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   nir_builder b = nir_builder_create(impl);
   bool found = false;
   nir_foreach_block_safe(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type == nir_instr_type_intrinsic) {
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic == nir_intrinsic_store_deref ||
                intr->intrinsic == nir_intrinsic_copy_deref) {
               nir_variable *var = nir_intrinsic_get_var(intr, 0);
               if (var->data.location == VARYING_SLOT_POS) {
                  b.cursor = nir_after_instr(instr);
                  nir_deref_instr *deref = nir_build_deref_var(&b, psiz);
                  nir_store_deref(&b, deref, nir_imm_float(&b, 1.0), BITFIELD_BIT(0));
                  found = true;
               }
            }
         }
      }
   }
   if (!found) {
      b.cursor = nir_before_impl(impl);
      nir_deref_instr *deref = nir_build_deref_var(&b, psiz);
      nir_store_deref(&b, deref, nir_imm_float(&b, 1.0), BITFIELD_BIT(0));
   }
}

static void
zero_array_members(nir_builder *b, nir_variable *var)
{
   nir_deref_instr *deref = nir_build_deref_var(b, var);
   nir_def *zero = nir_imm_zero(b, 4, 32);
   for (int i = 0; i < glsl_array_size(var->type); i++) {
      nir_deref_instr *arr = nir_build_deref_array_imm(b, deref, i);
      uint32_t mask = BITFIELD_MASK(glsl_get_vector_elements(arr->type));
      nir_store_deref(b, arr, nir_channels(b, zero, mask), mask);
   }
}

/* GL has an implicit default of 0 for unwritten gl_ClipDistance members;
 * to achieve this, write 0 to all members at the start of the shader and
 * let them be naturally overwritten later
 */
static bool
gl_nir_zero_initialize_clip_distance(nir_shader *nir)
{
   nir_variable *clip_dist0 = nir_find_variable_with_location(nir, nir_var_shader_out, VARYING_SLOT_CLIP_DIST0);
   nir_variable *clip_dist1 = nir_find_variable_with_location(nir, nir_var_shader_out, VARYING_SLOT_CLIP_DIST1);
   if (!clip_dist0 && !clip_dist1)
      return false;

   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   nir_builder b = nir_builder_at(nir_before_impl(impl));
   if (clip_dist0)
      zero_array_members(&b, clip_dist0);

   if (clip_dist1)
      zero_array_members(&b, clip_dist1);

   return true;
}

static void
lower_patch_vertices_in(struct gl_shader_program *shader_prog)
{
   struct gl_linked_shader *linked_tcs =
      shader_prog->_LinkedShaders[MESA_SHADER_TESS_CTRL];
   struct gl_linked_shader *linked_tes =
      shader_prog->_LinkedShaders[MESA_SHADER_TESS_EVAL];

   /* If we have a TCS and TES linked together, lower TES patch vertices. */
   if (linked_tcs && linked_tes) {
      nir_shader *tcs_nir = linked_tcs->Program->nir;
      nir_shader *tes_nir = linked_tes->Program->nir;

      /* The TES input vertex count is the TCS output vertex count,
       * lower TES gl_PatchVerticesIn to a constant.
       */
      uint32_t tes_patch_verts = tcs_nir->info.tess.tcs_vertices_out;
      NIR_PASS_V(tes_nir, nir_lower_patch_vertices, tes_patch_verts, NULL);
   }
}

static void
preprocess_shader(const struct gl_constants *consts,
                  const struct gl_extensions *exts,
                  struct gl_program *prog,
                  struct gl_shader_program *shader_program,
                  gl_shader_stage stage)
{
   const struct gl_shader_compiler_options *gl_options =
      &consts->ShaderCompilerOptions[prog->info.stage];
   const nir_shader_compiler_options *options = gl_options->NirOptions;
   assert(options);

   nir_shader *nir = prog->nir;

   if (prog->info.stage == MESA_SHADER_FRAGMENT && consts->HasFBFetch) {
      nir_shader_gather_info(prog->nir, nir_shader_get_entrypoint(prog->nir));
      NIR_PASS_V(prog->nir, gl_nir_lower_blend_equation_advanced,
                 exts->KHR_blend_equation_advanced_coherent);
      nir_lower_global_vars_to_local(prog->nir);
      NIR_PASS_V(prog->nir, nir_opt_combine_stores, nir_var_shader_out);
   }

   /* Set the next shader stage hint for VS and TES. */
   if (!nir->info.separate_shader &&
       (nir->info.stage == MESA_SHADER_VERTEX ||
        nir->info.stage == MESA_SHADER_TESS_EVAL)) {

      unsigned prev_stages = (1 << (prog->info.stage + 1)) - 1;
      unsigned stages_mask =
         ~prev_stages & shader_program->data->linked_stages;

      nir->info.next_stage = stages_mask ?
         (gl_shader_stage) u_bit_scan(&stages_mask) : MESA_SHADER_FRAGMENT;
   } else {
      nir->info.next_stage = MESA_SHADER_FRAGMENT;
   }

   prog->skip_pointsize_xfb = !(nir->info.outputs_written & VARYING_BIT_PSIZ);
   if (!consts->PointSizeFixed && prog->skip_pointsize_xfb &&
       stage < MESA_SHADER_FRAGMENT && stage != MESA_SHADER_TESS_CTRL &&
       gl_nir_can_add_pointsize_to_program(consts, prog)) {
      NIR_PASS_V(nir, gl_nir_add_point_size);
   }

   if (stage < MESA_SHADER_FRAGMENT && stage != MESA_SHADER_TESS_CTRL &&
       (nir->info.outputs_written & (VARYING_BIT_CLIP_DIST0 | VARYING_BIT_CLIP_DIST1)))
      NIR_PASS_V(nir, gl_nir_zero_initialize_clip_distance);

   if (options->lower_all_io_to_temps ||
       nir->info.stage == MESA_SHADER_VERTEX ||
       nir->info.stage == MESA_SHADER_GEOMETRY) {
      NIR_PASS_V(nir, nir_lower_io_to_temporaries,
                 nir_shader_get_entrypoint(nir),
                 true, true);
   } else if (nir->info.stage == MESA_SHADER_FRAGMENT ||
              !consts->SupportsReadingOutputs) {
      NIR_PASS_V(nir, nir_lower_io_to_temporaries,
                 nir_shader_get_entrypoint(nir),
                 true, false);
   }

   NIR_PASS_V(nir, nir_lower_global_vars_to_local);
   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_lower_var_copies);

   if (gl_options->LowerPrecisionFloat16 && gl_options->LowerPrecisionInt16) {
      NIR_PASS_V(nir, nir_lower_mediump_vars, nir_var_function_temp | nir_var_shader_temp | nir_var_mem_shared);
   }

   if (options->lower_to_scalar) {
      NIR_PASS_V(nir, nir_remove_dead_variables,
                 nir_var_function_temp | nir_var_shader_temp |
                 nir_var_mem_shared, NULL);
      NIR_PASS_V(nir, nir_opt_copy_prop_vars);
      NIR_PASS_V(nir, nir_lower_alu_to_scalar,
                 options->lower_to_scalar_filter, NULL);
   }

   NIR_PASS_V(nir, nir_opt_barrier_modes);

   /* before buffers and vars_to_ssa */
   NIR_PASS_V(nir, gl_nir_lower_images, true);

   if (prog->nir->info.stage == MESA_SHADER_COMPUTE) {
      NIR_PASS_V(prog->nir, nir_lower_vars_to_explicit_types,
                 nir_var_mem_shared, shared_type_info);
      NIR_PASS_V(prog->nir, nir_lower_explicit_io,
                 nir_var_mem_shared, nir_address_format_32bit_offset);
   }

   /* Do a round of constant folding to clean up address calculations */
   NIR_PASS_V(nir, nir_opt_constant_folding);
}

static bool
prelink_lowering(const struct gl_constants *consts,
                 const struct gl_extensions *exts,
                 struct gl_shader_program *shader_program,
                 struct gl_linked_shader **linked_shader, unsigned num_shaders)
{
   for (unsigned i = 0; i < num_shaders; i++) {
      struct gl_linked_shader *shader = linked_shader[i];
      const nir_shader_compiler_options *options =
         consts->ShaderCompilerOptions[shader->Stage].NirOptions;
      struct gl_program *prog = shader->Program;

      /* ES 3.0+ vertex shaders may still have dead varyings but its now safe
       * to remove them as validation is now done according to the spec.
       */
      if (shader_program->IsES && shader_program->GLSL_Version >= 300 &&
          i == MESA_SHADER_VERTEX)
         remove_dead_varyings_pre_linking(prog->nir);

      preprocess_shader(consts, exts, prog, shader_program, shader->Stage);

      if (prog->nir->info.shared_size > consts->MaxComputeSharedMemorySize) {
         linker_error(shader_program, "Too much shared memory used (%u/%u)\n",
                      prog->nir->info.shared_size,
                      consts->MaxComputeSharedMemorySize);
         return false;
      }

      if (options->lower_to_scalar) {
         NIR_PASS_V(shader->Program->nir, nir_lower_load_const_to_scalar);
      }
   }

   lower_patch_vertices_in(shader_program);

   /* Linking shaders also optimizes them. Separate shaders, compute shaders
    * and shaders with a fixed-func VS or FS that don't need linking are
    * optimized here.
    */
   if (num_shaders == 1)
      gl_nir_opts(linked_shader[0]->Program->nir);

   /* nir_opt_access() needs to run before linking so that ImageAccess[]
    * and BindlessImage[].access are filled out with the correct modes.
    */
   for (unsigned i = 0; i < num_shaders; i++) {
      nir_shader *nir = linked_shader[i]->Program->nir;

      nir_opt_access_options opt_access_options;
      opt_access_options.is_vulkan = false;
      NIR_PASS_V(nir, nir_opt_access, &opt_access_options);

      if (consts->ShaderCompilerOptions[i].LowerCombinedClipCullDistance) {
         NIR_PASS_V(nir, nir_lower_clip_cull_distance_to_vec4s);
      }

      /* Combine clip and cull outputs into one array and set:
       * - shader_info::clip_distance_array_size
       * - shader_info::cull_distance_array_size
       */
      if (consts->CombinedClipCullDistanceArrays)
         NIR_PASS_V(nir, nir_lower_clip_cull_distance_arrays);
   }

   return true;
}

bool
gl_nir_link_spirv(const struct gl_constants *consts,
                  const struct gl_extensions *exts,
                  struct gl_shader_program *prog,
                  const struct gl_nir_linker_options *options)
{
   struct gl_linked_shader *linked_shader[MESA_SHADER_STAGES];
   unsigned num_shaders = 0;

   MESA_TRACE_FUNC();

   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      if (prog->_LinkedShaders[i]) {
         linked_shader[num_shaders++] = prog->_LinkedShaders[i];

         remove_dead_varyings_pre_linking(prog->_LinkedShaders[i]->Program->nir);
      }
   }

   if (!prelink_lowering(consts, exts, prog, linked_shader, num_shaders))
      return false;

   /* Linking the stages in the opposite order (from fragment to vertex)
    * ensures that inter-shader outputs written to in an earlier stage
    * are eliminated if they are (transitively) not used in a later
    * stage.
    */
   for (int i = num_shaders - 2; i >= 0; i--) {
      gl_nir_link_opts(linked_shader[i]->Program->nir,
                       linked_shader[i + 1]->Program->nir);
   }

   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      struct gl_linked_shader *shader = prog->_LinkedShaders[i];
      if (shader) {
         const nir_remove_dead_variables_options opts = {
            .can_remove_var = can_remove_var,
         };
         nir_remove_dead_variables(shader->Program->nir,
                                   nir_var_uniform | nir_var_image,
                                   &opts);
      }
   }

   if (!gl_nir_link_uniform_blocks(consts, prog))
      return false;

   if (!gl_nir_link_uniforms(consts, prog, options->fill_parameters))
      return false;

   gl_nir_link_assign_atomic_counter_resources(consts, prog);
   gl_nir_link_assign_xfb_resources(consts, prog);

   return true;
}

/**
 * Validate shader image resources.
 */
static void
check_image_resources(const struct gl_constants *consts,
                      const struct gl_extensions *exts,
                      struct gl_shader_program *prog)
{
   unsigned total_image_units = 0;
   unsigned fragment_outputs = 0;
   unsigned total_shader_storage_blocks = 0;

   if (!exts->ARB_shader_image_load_store)
      return;

   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      struct gl_linked_shader *sh = prog->_LinkedShaders[i];
      if (!sh)
         continue;

      total_image_units += sh->Program->info.num_images;
      total_shader_storage_blocks += sh->Program->info.num_ssbos;
   }

   if (total_image_units > consts->MaxCombinedImageUniforms)
      linker_error(prog, "Too many combined image uniforms\n");

   struct gl_linked_shader *frag_sh =
      prog->_LinkedShaders[MESA_SHADER_FRAGMENT];
   if (frag_sh) {
      uint64_t frag_outputs_written = frag_sh->Program->info.outputs_written;
      fragment_outputs = util_bitcount64(frag_outputs_written);
   }

   if (total_image_units + fragment_outputs + total_shader_storage_blocks >
       consts->MaxCombinedShaderOutputResources)
      linker_error(prog, "Too many combined image uniforms, shader storage "
                         " buffers and fragment outputs\n");
}

static bool
is_sampler_array_accessed_indirectly(nir_deref_instr *deref)
{
   for (nir_deref_instr *d = deref; d; d = nir_deref_instr_parent(d)) {
      if (d->deref_type != nir_deref_type_array)
         continue;

      if (nir_src_is_const(d->arr.index))
         continue;

      return true;
   }

   return false;
}

/**
 * This check is done to make sure we allow only constant expression
 * indexing and "constant-index-expression" (indexing with an expression
 * that includes loop induction variable).
 */
static bool
validate_sampler_array_indexing(const struct gl_constants *consts,
                                struct gl_shader_program *prog)
{
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      if (prog->_LinkedShaders[i] == NULL)
         continue;

      bool no_dynamic_indexing =
         consts->ShaderCompilerOptions[i].NirOptions->force_indirect_unrolling_sampler;

      bool uses_indirect_sampler_array_indexing = false;
      nir_foreach_function_impl(impl, prog->_LinkedShaders[i]->Program->nir) {
         nir_foreach_block(block, impl) {
            nir_foreach_instr(instr, block) {
               /* Check if a sampler array is accessed indirectly */
               if (instr->type == nir_instr_type_tex) {
                  nir_tex_instr *tex_instr = nir_instr_as_tex(instr);
                  int sampler_idx =
                     nir_tex_instr_src_index(tex_instr, nir_tex_src_sampler_deref);
                  if (sampler_idx >= 0) {
                     nir_deref_instr *deref =
                        nir_instr_as_deref(tex_instr->src[sampler_idx].src.ssa->parent_instr);
                     if (is_sampler_array_accessed_indirectly(deref)) {
                        uses_indirect_sampler_array_indexing = true;
                        break;
                     }
                  }
               }
            }

            if (uses_indirect_sampler_array_indexing)
               break;
         }
         if (uses_indirect_sampler_array_indexing)
            break;
      }

      if (uses_indirect_sampler_array_indexing) {
         const char *msg = "sampler arrays indexed with non-constant "
                           "expressions is forbidden in GLSL %s %u";
         /* Backend has indicated that it has no dynamic indexing support. */
         if (no_dynamic_indexing) {
            linker_error(prog, msg, prog->IsES ? "ES" : "", prog->GLSL_Version);
            return false;
         } else {
            linker_warning(prog, msg, prog->IsES ? "ES" : "",
                           prog->GLSL_Version);
         }
      }
   }

   return true;
}

bool
gl_nir_link_glsl(const struct gl_constants *consts,
                 const struct gl_extensions *exts,
                 gl_api api,
                 struct gl_shader_program *prog)
{
   if (prog->NumShaders == 0)
      return true;

   MESA_TRACE_FUNC();

   prog->last_vert_prog = NULL;
   for (int i = MESA_SHADER_GEOMETRY; i >= MESA_SHADER_VERTEX; i--) {
      if (prog->_LinkedShaders[i] == NULL)
         continue;

      prog->last_vert_prog = prog->_LinkedShaders[i]->Program;
      break;
   }

   unsigned first = MESA_SHADER_STAGES;
   unsigned last = 0;

   /* Determine first and last stage. */
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      if (!prog->_LinkedShaders[i])
         continue;
      if (first == MESA_SHADER_STAGES)
         first = i;
      last = i;
   }

   gl_nir_lower_named_interface_blocks(prog);

   /* Validate the inputs of each stage with the output of the preceding
    * stage.
    */
   unsigned prev = first;
   for (unsigned i = prev + 1; i <= MESA_SHADER_FRAGMENT; i++) {
      if (prog->_LinkedShaders[i] == NULL)
         continue;

      gl_nir_cross_validate_outputs_to_inputs(consts, prog,
                                              prog->_LinkedShaders[prev],
                                              prog->_LinkedShaders[i]);
      if (!prog->data->LinkStatus)
         return false;

      prev = i;
   }

   /* The cross validation of outputs/inputs above validates interstage
    * explicit locations. We need to do this also for the inputs in the first
    * stage and outputs of the last stage included in the program, since there
    * is no cross validation for these.
    */
   gl_nir_validate_first_and_last_interface_explicit_locations(consts, prog,
                                                               (gl_shader_stage) first,
                                                               (gl_shader_stage) last);

   if (prog->SeparateShader)
      disable_varying_optimizations_for_sso(prog);

   struct gl_linked_shader *linked_shader[MESA_SHADER_STAGES];
   unsigned num_shaders = 0;

   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      if (prog->_LinkedShaders[i]) {
         linked_shader[num_shaders++] = prog->_LinkedShaders[i];

         /* Section 13.46 (Vertex Attribute Aliasing) of the OpenGL ES 3.2
          * specification says:
          *
          *    "In general, the behavior of GLSL ES should not depend on
          *    compiler optimizations which might be implementation-dependent.
          *    Name matching rules in most languages, including C++ from which
          *    GLSL ES is derived, are based on declarations rather than use.
          *
          *    RESOLUTION: The existence of aliasing is determined by
          *    declarations present after preprocessing."
          *
          * Because of this rule, we don't remove dead attributes before
          * attribute assignment for vertex shader inputs here.
          */
         if (!(prog->IsES && prog->GLSL_Version >= 300 && i == MESA_SHADER_VERTEX))
            remove_dead_varyings_pre_linking(prog->_LinkedShaders[i]->Program->nir);
      }
   }

   if (!gl_assign_attribute_or_color_locations(consts, prog))
      return false;

   if (!prelink_lowering(consts, exts, prog, linked_shader, num_shaders))
      return false;

   if (!gl_nir_link_varyings(consts, exts, api, prog))
      return false;

   /* Validation for special cases where we allow sampler array indexing
    * with loop induction variable. This check emits a warning or error
    * depending if backend can handle dynamic indexing.
    */
   if ((!prog->IsES && prog->GLSL_Version < 130) ||
       (prog->IsES && prog->GLSL_Version < 300)) {
      if (!validate_sampler_array_indexing(consts, prog))
         return false;
   }

   if (prog->data->LinkStatus == LINKING_FAILURE)
      return false;

   /* Linking the stages in the opposite order (from fragment to vertex)
    * ensures that inter-shader outputs written to in an earlier stage
    * are eliminated if they are (transitively) not used in a later
    * stage.
    */
   for (int i = num_shaders - 2; i >= 0; i--) {
      gl_nir_link_opts(linked_shader[i]->Program->nir,
                       linked_shader[i + 1]->Program->nir);
   }

   /* Tidy up any left overs from the linking process for single shaders.
    * For example varying arrays that get packed may have dead elements that
    * can be now be eliminated now that array access has been lowered.
    */
   if (num_shaders == 1)
      gl_nir_opts(linked_shader[0]->Program->nir);

   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      struct gl_linked_shader *shader = prog->_LinkedShaders[i];
      if (shader) {
         if (consts->GLSLLowerConstArrays) {
            nir_lower_const_arrays_to_uniforms(shader->Program->nir,
                                               consts->Program[i].MaxUniformComponents);
         }

         const nir_remove_dead_variables_options opts = {
            .can_remove_var = can_remove_var,
         };
         nir_remove_dead_variables(shader->Program->nir,
                                   nir_var_uniform | nir_var_image |
                                   nir_var_mem_ubo | nir_var_mem_ssbo |
                                   nir_var_system_value,
                                   &opts);

         if (shader->Program->info.stage == MESA_SHADER_FRAGMENT) {
            nir_shader *nir = shader->Program->nir;
            nir_foreach_variable_in_shader(var, nir) {
               if (var->data.mode == nir_var_system_value &&
                   (var->data.location == SYSTEM_VALUE_SAMPLE_ID ||
                    var->data.location == SYSTEM_VALUE_SAMPLE_POS))
                  nir->info.fs.uses_sample_shading = true;

               if (var->data.mode == nir_var_shader_in && var->data.sample)
                  nir->info.fs.uses_sample_shading = true;

               if (var->data.mode == nir_var_shader_out &&
                   var->data.fb_fetch_output)
                  nir->info.fs.uses_sample_shading = true;
            }
         }
      }
   }

   if (!gl_nir_link_uniform_blocks(consts, prog))
      return false;

   if (!gl_nir_link_uniforms(consts, prog, true))
      return false;

   link_util_calculate_subroutine_compat(prog);
   link_util_check_uniform_resources(consts, prog);
   link_util_check_subroutine_resources(prog);
   check_image_resources(consts, exts, prog);
   gl_nir_link_assign_atomic_counter_resources(consts, prog);
   gl_nir_link_check_atomic_counter_resources(consts, prog);

   /* OpenGL ES < 3.1 requires that a vertex shader and a fragment shader both
    * be present in a linked program. GL_ARB_ES2_compatibility doesn't say
    * anything about shader linking when one of the shaders (vertex or
    * fragment shader) is absent. So, the extension shouldn't change the
    * behavior specified in GLSL specification.
    *
    * From OpenGL ES 3.1 specification (7.3 Program Objects):
    *     "Linking can fail for a variety of reasons as specified in the
    *     OpenGL ES Shading Language Specification, as well as any of the
    *     following reasons:
    *
    *     ...
    *
    *     * program contains objects to form either a vertex shader or
    *       fragment shader, and program is not separable, and does not
    *       contain objects to form both a vertex shader and fragment
    *       shader."
    *
    * However, the only scenario in 3.1+ where we don't require them both is
    * when we have a compute shader. For example:
    *
    * - No shaders is a link error.
    * - Geom or Tess without a Vertex shader is a link error which means we
    *   always require a Vertex shader and hence a Fragment shader.
    * - Finally a Compute shader linked with any other stage is a link error.
    */
   if (!prog->SeparateShader && _mesa_is_api_gles2(api) &&
       !prog->_LinkedShaders[MESA_SHADER_COMPUTE]) {
      if (prog->_LinkedShaders[MESA_SHADER_VERTEX] == NULL) {
         linker_error(prog, "program lacks a vertex shader\n");
      } else if (prog->_LinkedShaders[MESA_SHADER_FRAGMENT] == NULL) {
         linker_error(prog, "program lacks a fragment shader\n");
      }
   }

   if (prog->data->LinkStatus == LINKING_FAILURE)
      return false;

   return true;
}
