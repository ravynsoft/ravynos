/*
 * Copyright © 2015 Red Hat
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "st_nir.h"

#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"

#include "program/program.h"
#include "program/prog_statevars.h"
#include "program/prog_parameter.h"
#include "main/context.h"
#include "main/mtypes.h"
#include "main/errors.h"
#include "main/glspirv.h"
#include "main/shaderapi.h"
#include "main/uniforms.h"

#include "main/shaderobj.h"
#include "st_context.h"
#include "st_program.h"
#include "st_shader_cache.h"

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/glsl_types.h"
#include "compiler/glsl/glsl_to_nir.h"
#include "compiler/glsl/gl_nir.h"
#include "compiler/glsl/gl_nir_linker.h"
#include "compiler/glsl/ir.h"
#include "compiler/glsl/ir_optimization.h"
#include "compiler/glsl/linker_util.h"
#include "compiler/glsl/program.h"
#include "compiler/glsl/shader_cache.h"
#include "compiler/glsl/string_to_uint_map.h"

static int
type_size(const struct glsl_type *type)
{
   return glsl_count_attribute_slots(type, false);
}

/* Depending on PIPE_CAP_TGSI_TEXCOORD (st->needs_texcoord_semantic) we
 * may need to fix up varying slots so the glsl->nir path is aligned
 * with the anything->tgsi->nir path.
 */
static void
st_nir_fixup_varying_slots(struct st_context *st, nir_shader *shader,
                           nir_variable_mode mode)
{
   if (st->needs_texcoord_semantic)
      return;

   /* This is called from finalize, but we don't want to do this adjustment twice. */
   assert(!st->allow_st_finalize_nir_twice);

   nir_foreach_variable_with_modes(var, shader, mode) {
      if (var->data.location >= VARYING_SLOT_VAR0 && var->data.location < VARYING_SLOT_PATCH0) {
         var->data.location += 9;
      } else if (var->data.location == VARYING_SLOT_PNTC) {
         var->data.location = VARYING_SLOT_VAR8;
      } else if ((var->data.location >= VARYING_SLOT_TEX0) &&
               (var->data.location <= VARYING_SLOT_TEX7)) {
         var->data.location += VARYING_SLOT_VAR0 - VARYING_SLOT_TEX0;
      }
   }
}

/* input location assignment for VS inputs must be handled specially, so
 * that it is aligned w/ st's vbo state.
 * (This isn't the case with, for ex, FS inputs, which only need to agree
 * on varying-slot w/ the VS outputs)
 */
void
st_nir_assign_vs_in_locations(struct nir_shader *nir)
{
   if (nir->info.stage != MESA_SHADER_VERTEX || nir->info.io_lowered)
      return;

   nir->num_inputs = util_bitcount64(nir->info.inputs_read);

   bool removed_inputs = false;

   nir_foreach_shader_in_variable_safe(var, nir) {
      /* NIR already assigns dual-slot inputs to two locations so all we have
       * to do is compact everything down.
       */
      if (nir->info.inputs_read & BITFIELD64_BIT(var->data.location)) {
         var->data.driver_location =
            util_bitcount64(nir->info.inputs_read &
                              BITFIELD64_MASK(var->data.location));
      } else {
         /* Convert unused input variables to shader_temp (with no
          * initialization), to avoid confusing drivers looking through the
          * inputs array and expecting to find inputs with a driver_location
          * set.
          */
         var->data.mode = nir_var_shader_temp;
         removed_inputs = true;
      }
   }

   /* Re-lower global vars, to deal with any dead VS inputs. */
   if (removed_inputs)
      NIR_PASS_V(nir, nir_lower_global_vars_to_local);
}

static int
st_nir_lookup_parameter_index(struct gl_program *prog, nir_variable *var)
{
   struct gl_program_parameter_list *params = prog->Parameters;

   /* Lookup the first parameter that the uniform storage that match the
    * variable location.
    */
   for (unsigned i = 0; i < params->NumParameters; i++) {
      int index = params->Parameters[i].MainUniformStorageIndex;
      if (index == var->data.location)
         return i;
   }

   /* TODO: Handle this fallback for SPIR-V.  We need this for GLSL e.g. in
    * dEQP-GLES2.functional.uniform_api.random.3
    */

   /* is there a better way to do this?  If we have something like:
    *
    *    struct S {
    *           float f;
    *           vec4 v;
    *    };
    *    uniform S color;
    *
    * Then what we get in prog->Parameters looks like:
    *
    *    0: Name=color.f, Type=6, DataType=1406, Size=1
    *    1: Name=color.v, Type=6, DataType=8b52, Size=4
    *
    * So the name doesn't match up and _mesa_lookup_parameter_index()
    * fails.  In this case just find the first matching "color.*"..
    *
    * Note for arrays you could end up w/ color[n].f, for example.
    */
   if (!prog->sh.data->spirv) {
      int namelen = strlen(var->name);
      for (unsigned i = 0; i < params->NumParameters; i++) {
         struct gl_program_parameter *p = &params->Parameters[i];
         if ((strncmp(p->Name, var->name, namelen) == 0) &&
             ((p->Name[namelen] == '.') || (p->Name[namelen] == '['))) {
            return i;
         }
      }
   }

   return -1;
}

static void
st_nir_assign_uniform_locations(struct gl_context *ctx,
                                struct gl_program *prog,
                                nir_shader *nir)
{
   int shaderidx = 0;
   int imageidx = 0;

   nir_foreach_variable_with_modes(uniform, nir, nir_var_uniform |
                                                 nir_var_image) {
      int loc;

      const struct glsl_type *type = glsl_without_array(uniform->type);
      if (!uniform->data.bindless && (glsl_type_is_sampler(type) || glsl_type_is_image(type))) {
         if (glsl_type_is_sampler(type)) {
            loc = shaderidx;
            shaderidx += type_size(uniform->type);
         } else {
            loc = imageidx;
            imageidx += type_size(uniform->type);
         }
      } else if (uniform->state_slots) {
         const gl_state_index16 *const stateTokens = uniform->state_slots[0].tokens;

         unsigned comps;
         if (glsl_type_is_struct_or_ifc(type)) {
            comps = 4;
         } else {
            comps = glsl_get_vector_elements(type);
         }

         if (ctx->Const.PackedDriverUniformStorage) {
            loc = _mesa_add_sized_state_reference(prog->Parameters,
                                                  stateTokens, comps, false);
            loc = prog->Parameters->Parameters[loc].ValueOffset;
         } else {
            loc = _mesa_add_state_reference(prog->Parameters, stateTokens);
         }
      } else {
         loc = st_nir_lookup_parameter_index(prog, uniform);

         /* We need to check that loc is not -1 here before accessing the
          * array. It can be negative for example when we have a struct that
          * only contains opaque types.
          */
         if (loc >= 0 && ctx->Const.PackedDriverUniformStorage) {
            loc = prog->Parameters->Parameters[loc].ValueOffset;
         }
      }

      uniform->data.driver_location = loc;
   }
}

static bool
def_is_64bit(nir_def *def, void *state)
{
   bool *lower = (bool *)state;
   if (def && (def->bit_size == 64)) {
      *lower = true;
      return false;
   }
   return true;
}

static bool
src_is_64bit(nir_src *src, void *state)
{
   bool *lower = (bool *)state;
   if (src && (nir_src_bit_size(*src) == 64)) {
      *lower = true;
      return false;
   }
   return true;
}

static bool
filter_64_bit_instr(const nir_instr *const_instr, UNUSED const void *data)
{
   bool lower = false;
   /* lower_alu_to_scalar required nir_instr to be const, but nir_foreach_*
    * doesn't have const variants, so do the ugly const_cast here. */
   nir_instr *instr = const_cast<nir_instr *>(const_instr);

   nir_foreach_def(instr, def_is_64bit, &lower);
   if (lower)
      return true;
   nir_foreach_src(instr, src_is_64bit, &lower);
   return lower;
}

/* Second third of converting glsl_to_nir. This creates uniforms, gathers
 * info on varyings, etc after NIR link time opts have been applied.
 */
static char *
st_glsl_to_nir_post_opts(struct st_context *st, struct gl_program *prog,
                         struct gl_shader_program *shader_program)
{
   nir_shader *nir = prog->nir;
   struct pipe_screen *screen = st->screen;

   /* Make a pass over the IR to add state references for any built-in
    * uniforms that are used.  This has to be done now (during linking).
    * Code generation doesn't happen until the first time this shader is
    * used for rendering.  Waiting until then to generate the parameters is
    * too late.  At that point, the values for the built-in uniforms won't
    * get sent to the shader.
    */
   nir_foreach_uniform_variable(var, nir) {
      const nir_state_slot *const slots = var->state_slots;
      if (slots != NULL) {
         const struct glsl_type *type = glsl_without_array(var->type);
         for (unsigned int i = 0; i < var->num_state_slots; i++) {
            unsigned comps;
            if (glsl_type_is_struct_or_ifc(type)) {
               comps = _mesa_program_state_value_size(slots[i].tokens);
            } else {
               comps = glsl_get_vector_elements(type);
            }

            if (st->ctx->Const.PackedDriverUniformStorage) {
               _mesa_add_sized_state_reference(prog->Parameters,
                                               slots[i].tokens,
                                               comps, false);
            } else {
               _mesa_add_state_reference(prog->Parameters,
                                         slots[i].tokens);
            }
         }
      }
   }

   /* Avoid reallocation of the program parameter list, because the uniform
    * storage is only associated with the original parameter list.
    * This should be enough for Bitmap and DrawPixels constants.
    */
   _mesa_ensure_and_associate_uniform_storage(st->ctx, shader_program, prog, 28);

   /* None of the builtins being lowered here can be produced by SPIR-V.  See
    * _mesa_builtin_uniform_desc. Also drivers that support packed uniform
    * storage don't need to lower builtins.
    */
   if (!shader_program->data->spirv &&
       !st->ctx->Const.PackedDriverUniformStorage)
      NIR_PASS_V(nir, st_nir_lower_builtin);

   if (!screen->get_param(screen, PIPE_CAP_NIR_ATOMICS_AS_DEREF))
      NIR_PASS_V(nir, gl_nir_lower_atomics, shader_program, true);

   NIR_PASS_V(nir, nir_opt_intrinsics);
   NIR_PASS_V(nir, nir_opt_fragdepth);

   /* Lower 64-bit ops. */
   if (nir->options->lower_int64_options ||
       nir->options->lower_doubles_options) {
      bool lowered_64bit_ops = false;
      bool revectorize = false;

      if (nir->options->lower_doubles_options) {
         /* nir_lower_doubles is not prepared for vector ops, so if the backend doesn't
          * request lower_alu_to_scalar until now, lower all 64 bit ops, and try to
          * vectorize them afterwards again */
         if (!nir->options->lower_to_scalar) {
            NIR_PASS(revectorize, nir, nir_lower_alu_to_scalar, filter_64_bit_instr, nullptr);
            NIR_PASS(revectorize, nir, nir_lower_phis_to_scalar, false);
         }
         /* doubles lowering requires frexp to be lowered first if it will be,
          * since the pass generates other 64-bit ops.  Most backends lower
          * frexp, and using doubles is rare, and using frexp is even more rare
          * (no instances in shader-db), so we're not too worried about
          * accidentally lowering a 32-bit frexp here.
          */
         NIR_PASS(lowered_64bit_ops, nir, nir_lower_frexp);

         NIR_PASS(lowered_64bit_ops, nir, nir_lower_doubles,
                  st->ctx->SoftFP64, nir->options->lower_doubles_options);
      }
      if (nir->options->lower_int64_options)
         NIR_PASS(lowered_64bit_ops, nir, nir_lower_int64);

      if (revectorize && !nir->options->vectorize_vec2_16bit)
         NIR_PASS_V(nir, nir_opt_vectorize, nullptr, nullptr);

      if (revectorize || lowered_64bit_ops)
         gl_nir_opts(nir);
   }

   nir_variable_mode mask =
      nir_var_shader_in | nir_var_shader_out | nir_var_function_temp;
   nir_remove_dead_variables(nir, mask, NULL);

   if (!st->has_hw_atomics && !screen->get_param(screen, PIPE_CAP_NIR_ATOMICS_AS_DEREF)) {
      unsigned align_offset_state = 0;
      if (st->ctx->Const.ShaderStorageBufferOffsetAlignment > 4) {
         struct gl_program_parameter_list *params = prog->Parameters;
         for (unsigned i = 0; i < shader_program->data->NumAtomicBuffers; i++) {
            gl_state_index16 state[STATE_LENGTH] = { STATE_ATOMIC_COUNTER_OFFSET, (short)shader_program->data->AtomicBuffers[i].Binding };
            _mesa_add_state_reference(params, state);
         }
         align_offset_state = STATE_ATOMIC_COUNTER_OFFSET;
      }
      NIR_PASS_V(nir, nir_lower_atomics_to_ssbo, align_offset_state);
   }

   st_set_prog_affected_state_flags(prog);

   st_finalize_nir_before_variants(nir);

   char *msg = NULL;
   if (st->allow_st_finalize_nir_twice)
      msg = st_finalize_nir(st, prog, shader_program, nir, true, true);

   if (st->ctx->_Shader->Flags & GLSL_DUMP) {
      _mesa_log("\n");
      _mesa_log("NIR IR for linked %s program %d:\n",
             _mesa_shader_stage_to_string(prog->info.stage),
             shader_program->Name);
      nir_print_shader(nir, _mesa_get_log_file());
      _mesa_log("\n\n");
   }

   return msg;
}

static void
st_nir_vectorize_io(nir_shader *producer, nir_shader *consumer)
{
   if (consumer)
      NIR_PASS_V(consumer, nir_lower_io_to_vector, nir_var_shader_in);

   if (!producer)
      return;

   NIR_PASS_V(producer, nir_lower_io_to_vector, nir_var_shader_out);

   if (producer->info.stage == MESA_SHADER_TESS_CTRL &&
       producer->options->vectorize_tess_levels)
      NIR_PASS_V(producer, nir_vectorize_tess_levels);

   NIR_PASS_V(producer, nir_opt_combine_stores, nir_var_shader_out);

   if ((producer)->info.stage != MESA_SHADER_TESS_CTRL) {
      /* Calling lower_io_to_vector creates output variable writes with
       * write-masks.  We only support these for TCS outputs, so for other
       * stages, we need to call nir_lower_io_to_temporaries to get rid of
       * them.  This, in turn, creates temporary variables and extra
       * copy_deref intrinsics that we need to clean up.
       */
      NIR_PASS_V(producer, nir_lower_io_to_temporaries,
                 nir_shader_get_entrypoint(producer), true, false);
      NIR_PASS_V(producer, nir_lower_global_vars_to_local);
      NIR_PASS_V(producer, nir_split_var_copies);
      NIR_PASS_V(producer, nir_lower_var_copies);
   }

   /* Undef scalar store_deref intrinsics are not ignored by nir_lower_io,
    * so they must be removed before that. These passes remove them.
    */
   NIR_PASS_V(producer, nir_lower_vars_to_ssa);
   NIR_PASS_V(producer, nir_opt_undef);
   NIR_PASS_V(producer, nir_opt_dce);
}

extern "C" {

void
st_nir_lower_wpos_ytransform(struct nir_shader *nir,
                             struct gl_program *prog,
                             struct pipe_screen *pscreen)
{
   if (nir->info.stage != MESA_SHADER_FRAGMENT)
      return;

   static const gl_state_index16 wposTransformState[STATE_LENGTH] = {
      STATE_FB_WPOS_Y_TRANSFORM
   };
   nir_lower_wpos_ytransform_options wpos_options = { { 0 } };

   memcpy(wpos_options.state_tokens, wposTransformState,
          sizeof(wpos_options.state_tokens));
   wpos_options.fs_coord_origin_upper_left =
      pscreen->get_param(pscreen,
                         PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT);
   wpos_options.fs_coord_origin_lower_left =
      pscreen->get_param(pscreen,
                         PIPE_CAP_FS_COORD_ORIGIN_LOWER_LEFT);
   wpos_options.fs_coord_pixel_center_integer =
      pscreen->get_param(pscreen,
                         PIPE_CAP_FS_COORD_PIXEL_CENTER_INTEGER);
   wpos_options.fs_coord_pixel_center_half_integer =
      pscreen->get_param(pscreen,
                         PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER);

   if (nir_lower_wpos_ytransform(nir, &wpos_options)) {
      nir_validate_shader(nir, "after nir_lower_wpos_ytransform");
      _mesa_add_state_reference(prog->Parameters, wposTransformState);
   }

   static const gl_state_index16 pntcTransformState[STATE_LENGTH] = {
      STATE_FB_PNTC_Y_TRANSFORM
   };

   if (nir_lower_pntc_ytransform(nir, &pntcTransformState)) {
      _mesa_add_state_reference(prog->Parameters, pntcTransformState);
   }
}

static bool
st_link_glsl_to_nir(struct gl_context *ctx,
                    struct gl_shader_program *shader_program)
{
   struct st_context *st = st_context(ctx);
   struct gl_linked_shader *linked_shader[MESA_SHADER_STAGES];
   unsigned num_shaders = 0;

   /* Return early if we are loading the shader from on-disk cache */
   if (st_load_nir_from_disk_cache(ctx, shader_program)) {
      return GL_TRUE;
   }

   MESA_TRACE_FUNC();

   assert(shader_program->data->LinkStatus);

   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      if (shader_program->_LinkedShaders[i])
         linked_shader[num_shaders++] = shader_program->_LinkedShaders[i];
   }

   for (unsigned i = 0; i < num_shaders; i++) {
      struct gl_linked_shader *shader = linked_shader[i];
      const nir_shader_compiler_options *options =
         st->ctx->Const.ShaderCompilerOptions[shader->Stage].NirOptions;
      struct gl_program *prog = shader->Program;

      _mesa_copy_linked_program_data(shader_program, shader);

      assert(!prog->nir);
      prog->shader_program = shader_program;
      prog->state.type = PIPE_SHADER_IR_NIR;

      /* Parameters will be filled during NIR linking. */
      prog->Parameters = _mesa_new_parameter_list();

      if (shader_program->data->spirv) {
         prog->nir = _mesa_spirv_to_nir(ctx, shader_program, shader->Stage, options);
      } else {
         if (ctx->_Shader->Flags & GLSL_DUMP) {
            _mesa_log("\n");
            _mesa_log("GLSL IR for linked %s program %d:\n",
                      _mesa_shader_stage_to_string(shader->Stage),
                      shader_program->Name);
            _mesa_print_ir(_mesa_get_log_file(), shader->ir, NULL);
            _mesa_log("\n\n");
         }

         prog->nir = glsl_to_nir(&st->ctx->Const, shader_program, shader->Stage, options);
      }

      memcpy(prog->nir->info.source_sha1, shader->linked_source_sha1,
             SHA1_DIGEST_LENGTH);

      nir_shader_gather_info(prog->nir, nir_shader_get_entrypoint(prog->nir));
      if (!st->ctx->SoftFP64 && ((prog->nir->info.bit_sizes_int | prog->nir->info.bit_sizes_float) & 64) &&
          (options->lower_doubles_options & nir_lower_fp64_full_software) != 0) {

         /* It's not possible to use float64 on GLSL ES, so don't bother trying to
          * build the support code.  The support code depends on higher versions of
          * desktop GLSL, so it will fail to compile (below) anyway.
          */
         if (_mesa_is_desktop_gl(st->ctx) && st->ctx->Const.GLSLVersion >= 400)
            st->ctx->SoftFP64 = glsl_float64_funcs_to_nir(st->ctx, options);
      }
   }

   if (shader_program->data->spirv) {
      static const gl_nir_linker_options opts = {
         true /*fill_parameters */
      };
      if (!gl_nir_link_spirv(&ctx->Const, &ctx->Extensions, shader_program,
                             &opts))
         return GL_FALSE;
   } else {
      if (!gl_nir_link_glsl(&ctx->Const, &ctx->Extensions, ctx->API,
                            shader_program))
         return GL_FALSE;
   }

   for (unsigned i = 0; i < num_shaders; i++) {
      struct gl_program *prog = linked_shader[i]->Program;
      prog->ExternalSamplersUsed = gl_external_samplers(prog);
      _mesa_update_shader_textures_used(shader_program, prog);
   }

   nir_build_program_resource_list(&ctx->Const, shader_program,
                                   shader_program->data->spirv);

   for (unsigned i = 0; i < num_shaders; i++) {
      struct gl_linked_shader *shader = linked_shader[i];
      nir_shader *nir = shader->Program->nir;
      gl_shader_stage stage = shader->Stage;
      const struct gl_shader_compiler_options *options =
            &ctx->Const.ShaderCompilerOptions[stage];

      /* If there are forms of indirect addressing that the driver
       * cannot handle, perform the lowering pass.
       */
      if (options->EmitNoIndirectInput || options->EmitNoIndirectOutput ||
          options->EmitNoIndirectTemp || options->EmitNoIndirectUniform) {
         nir_variable_mode mode = options->EmitNoIndirectInput ?
            nir_var_shader_in : (nir_variable_mode)0;
         mode |= options->EmitNoIndirectOutput ?
            nir_var_shader_out : (nir_variable_mode)0;
         mode |= options->EmitNoIndirectTemp ?
            nir_var_function_temp : (nir_variable_mode)0;
         mode |= options->EmitNoIndirectUniform ?
            nir_var_uniform | nir_var_mem_ubo | nir_var_mem_ssbo :
            (nir_variable_mode)0;

         nir_lower_indirect_derefs(nir, mode, UINT32_MAX);
      }

      /* This needs to run after the initial pass of nir_lower_vars_to_ssa, so
       * that the buffer indices are constants in nir where they where
       * constants in GLSL. */
      NIR_PASS_V(nir, gl_nir_lower_buffers, shader_program);

      /* Remap the locations to slots so those requiring two slots will occupy
       * two locations. For instance, if we have in the IR code a dvec3 attr0 in
       * location 0 and vec4 attr1 in location 1, in NIR attr0 will use
       * locations/slots 0 and 1, and attr1 will use location/slot 2
       */
      if (nir->info.stage == MESA_SHADER_VERTEX && !shader_program->data->spirv)
         nir_remap_dual_slot_attributes(nir, &shader->Program->DualSlotInputs);

      NIR_PASS_V(nir, st_nir_lower_wpos_ytransform, shader->Program,
                 st->screen);

      NIR_PASS_V(nir, nir_lower_system_values);
      NIR_PASS_V(nir, nir_lower_compute_system_values, NULL);

      if (i >= 1) {
         struct gl_program *prev_shader = linked_shader[i - 1]->Program;

         /* We can't use nir_compact_varyings with transform feedback, since
          * the pipe_stream_output->output_register field is based on the
          * pre-compacted driver_locations.
          */
         if (!(prev_shader->sh.LinkedTransformFeedback &&
               prev_shader->sh.LinkedTransformFeedback->NumVarying > 0))
            nir_compact_varyings(prev_shader->nir,
                                 nir, ctx->API != API_OPENGL_COMPAT);

         if (ctx->Const.ShaderCompilerOptions[shader->Stage].NirOptions->vectorize_io)
            st_nir_vectorize_io(prev_shader->nir, nir);
      }
   }

   /* If the program is a separate shader program check if we need to vectorise
    * the first and last program interfaces too.
    */
   if (shader_program->SeparateShader && num_shaders > 0) {
      struct gl_linked_shader *first_shader = linked_shader[0];
      struct gl_linked_shader *last_shader = linked_shader[num_shaders - 1];
      if (first_shader->Stage != MESA_SHADER_COMPUTE) {
         if (ctx->Const.ShaderCompilerOptions[first_shader->Stage].NirOptions->vectorize_io &&
             first_shader->Stage > MESA_SHADER_VERTEX)
            st_nir_vectorize_io(NULL, first_shader->Program->nir);

         if (ctx->Const.ShaderCompilerOptions[last_shader->Stage].NirOptions->vectorize_io &&
             last_shader->Stage < MESA_SHADER_FRAGMENT)
            st_nir_vectorize_io(last_shader->Program->nir, NULL);
      }
   }

   struct shader_info *prev_info = NULL;

   for (unsigned i = 0; i < num_shaders; i++) {
      struct gl_linked_shader *shader = linked_shader[i];
      struct shader_info *info = &shader->Program->nir->info;

      char *msg = st_glsl_to_nir_post_opts(st, shader->Program, shader_program);
      if (msg) {
         linker_error(shader_program, msg);
         return false;
      }

      if (prev_info &&
          ctx->Const.ShaderCompilerOptions[shader->Stage].NirOptions->unify_interfaces) {
         prev_info->outputs_written |= info->inputs_read &
            ~(VARYING_BIT_TESS_LEVEL_INNER | VARYING_BIT_TESS_LEVEL_OUTER);
         info->inputs_read |= prev_info->outputs_written &
            ~(VARYING_BIT_TESS_LEVEL_INNER | VARYING_BIT_TESS_LEVEL_OUTER);

         prev_info->patch_outputs_written |= info->patch_inputs_read;
         info->patch_inputs_read |= prev_info->patch_outputs_written;
      }
      prev_info = info;
   }

   for (unsigned i = 0; i < num_shaders; i++) {
      struct gl_linked_shader *shader = linked_shader[i];
      struct gl_program *prog = shader->Program;

      /* Make sure that prog->info is in sync with nir->info, but st/mesa
       * expects some of the values to be from before lowering.
       */
      shader_info old_info = prog->info;
      prog->info = prog->nir->info;
      prog->info.name = old_info.name;
      prog->info.label = old_info.label;
      prog->info.num_ssbos = old_info.num_ssbos;
      prog->info.num_ubos = old_info.num_ubos;
      prog->info.num_abos = old_info.num_abos;

      if (prog->info.stage == MESA_SHADER_VERTEX) {
         /* NIR expands dual-slot inputs out to two locations.  We need to
          * compact things back down GL-style single-slot inputs to avoid
          * confusing the state tracker.
          */
         prog->info.inputs_read =
            nir_get_single_slot_attribs_mask(prog->nir->info.inputs_read,
                                             prog->DualSlotInputs);

         /* Initialize st_vertex_program members. */
         st_prepare_vertex_program(prog);
      }

      /* Get pipe_stream_output_info. */
      if (shader->Stage == MESA_SHADER_VERTEX ||
          shader->Stage == MESA_SHADER_TESS_EVAL ||
          shader->Stage == MESA_SHADER_GEOMETRY)
         st_translate_stream_output_info(prog);

      st_store_nir_in_disk_cache(st, prog);

      st_release_variants(st, prog);
      st_finalize_program(st, prog);
   }

   struct pipe_context *pctx = st_context(ctx)->pipe;
   if (pctx->link_shader) {
      void *driver_handles[PIPE_SHADER_TYPES];
      memset(driver_handles, 0, sizeof(driver_handles));

      for (uint32_t i = 0; i < MESA_SHADER_STAGES; ++i) {
         struct gl_linked_shader *shader = shader_program->_LinkedShaders[i];
         if (shader) {
            struct gl_program *p = shader->Program;
            if (p && p->variants) {
               enum pipe_shader_type type = pipe_shader_type_from_mesa(shader->Stage);
               driver_handles[type] = p->variants->driver_shader;
            }
         }
      }

      pctx->link_shader(pctx, driver_handles);
   }

   return true;
}

void
st_nir_assign_varying_locations(struct st_context *st, nir_shader *nir)
{
   if (nir->info.stage == MESA_SHADER_VERTEX) {
      nir_assign_io_var_locations(nir, nir_var_shader_out,
                                  &nir->num_outputs,
                                  nir->info.stage);
      st_nir_fixup_varying_slots(st, nir, nir_var_shader_out);
   } else if (nir->info.stage == MESA_SHADER_GEOMETRY ||
              nir->info.stage == MESA_SHADER_TESS_CTRL ||
              nir->info.stage == MESA_SHADER_TESS_EVAL) {
      nir_assign_io_var_locations(nir, nir_var_shader_in,
                                  &nir->num_inputs,
                                  nir->info.stage);
      st_nir_fixup_varying_slots(st, nir, nir_var_shader_in);

      nir_assign_io_var_locations(nir, nir_var_shader_out,
                                  &nir->num_outputs,
                                  nir->info.stage);
      st_nir_fixup_varying_slots(st, nir, nir_var_shader_out);
   } else if (nir->info.stage == MESA_SHADER_FRAGMENT) {
      nir_assign_io_var_locations(nir, nir_var_shader_in,
                                  &nir->num_inputs,
                                  nir->info.stage);
      st_nir_fixup_varying_slots(st, nir, nir_var_shader_in);
      nir_assign_io_var_locations(nir, nir_var_shader_out,
                                  &nir->num_outputs,
                                  nir->info.stage);
   } else if (nir->info.stage == MESA_SHADER_COMPUTE) {
       /* TODO? */
   } else {
      unreachable("invalid shader type");
   }
}

void
st_nir_lower_samplers(struct pipe_screen *screen, nir_shader *nir,
                      struct gl_shader_program *shader_program,
                      struct gl_program *prog)
{
   if (screen->get_param(screen, PIPE_CAP_NIR_SAMPLERS_AS_DEREF))
      NIR_PASS_V(nir, gl_nir_lower_samplers_as_deref, shader_program);
   else
      NIR_PASS_V(nir, gl_nir_lower_samplers, shader_program);

   if (prog) {
      BITSET_COPY(prog->info.textures_used, nir->info.textures_used);
      BITSET_COPY(prog->info.textures_used_by_txf, nir->info.textures_used_by_txf);
      BITSET_COPY(prog->info.samplers_used, nir->info.samplers_used);
      BITSET_COPY(prog->info.images_used, nir->info.images_used);
      BITSET_COPY(prog->info.image_buffers, nir->info.image_buffers);
      BITSET_COPY(prog->info.msaa_images, nir->info.msaa_images);
   }
}

static int
st_packed_uniforms_type_size(const struct glsl_type *type, bool bindless)
{
   return glsl_count_dword_slots(type, bindless);
}

static int
st_unpacked_uniforms_type_size(const struct glsl_type *type, bool bindless)
{
   return glsl_count_vec4_slots(type, false, bindless);
}

void
st_nir_lower_uniforms(struct st_context *st, nir_shader *nir)
{
   if (st->ctx->Const.PackedDriverUniformStorage) {
      NIR_PASS_V(nir, nir_lower_io, nir_var_uniform,
                 st_packed_uniforms_type_size,
                 (nir_lower_io_options)0);
   } else {
      NIR_PASS_V(nir, nir_lower_io, nir_var_uniform,
                 st_unpacked_uniforms_type_size,
                 (nir_lower_io_options)0);
   }

   if (nir->options->lower_uniforms_to_ubo)
      NIR_PASS_V(nir, nir_lower_uniforms_to_ubo,
                 st->ctx->Const.PackedDriverUniformStorage,
                 !st->ctx->Const.NativeIntegers);
}

/* Last third of preparing nir from glsl, which happens after shader
 * variant lowering.
 */
char *
st_finalize_nir(struct st_context *st, struct gl_program *prog,
                struct gl_shader_program *shader_program,
                nir_shader *nir, bool finalize_by_driver,
                bool is_before_variants)
{
   struct pipe_screen *screen = st->screen;

   MESA_TRACE_FUNC();

   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_lower_var_copies);

   const bool lower_tg4_offsets =
      !st->screen->get_param(screen, PIPE_CAP_TEXTURE_GATHER_OFFSETS);

   if (st->lower_rect_tex || lower_tg4_offsets) {
      struct nir_lower_tex_options opts = {0};
      opts.lower_rect = !!st->lower_rect_tex;
      opts.lower_tg4_offsets = lower_tg4_offsets;

      NIR_PASS_V(nir, nir_lower_tex, &opts);
   }

   st_nir_assign_varying_locations(st, nir);
   st_nir_assign_uniform_locations(st->ctx, prog, nir);

   /* Lower load_deref/store_deref of inputs and outputs.
    * This depends on st_nir_assign_varying_locations.
    */
   if (nir->options->lower_io_variables) {
      nir_lower_io_passes(nir, false);
      NIR_PASS_V(nir, nir_remove_dead_variables,
                 nir_var_shader_in | nir_var_shader_out, NULL);
   }

   /* Set num_uniforms in number of attribute slots (vec4s) */
   nir->num_uniforms = DIV_ROUND_UP(prog->Parameters->NumParameterValues, 4);

   st_nir_lower_uniforms(st, nir);

   if (is_before_variants && nir->options->lower_uniforms_to_ubo) {
      /* This must be done after uniforms are lowered to UBO and all
       * nir_var_uniform variables are removed from NIR to prevent conflicts
       * between state parameter merging and shader variant generation.
       */
      _mesa_optimize_state_parameters(&st->ctx->Const, prog->Parameters);
   }

   st_nir_lower_samplers(screen, nir, shader_program, prog);
   if (!screen->get_param(screen, PIPE_CAP_NIR_IMAGES_AS_DEREF))
      NIR_PASS_V(nir, gl_nir_lower_images, false);

   char *msg = NULL;
   if (finalize_by_driver && screen->finalize_nir)
      msg = screen->finalize_nir(screen, nir);

   return msg;
}

/**
 * Link a GLSL shader program.  Called via glLinkProgram().
 */
void
st_link_shader(struct gl_context *ctx, struct gl_shader_program *prog)
{
   unsigned int i;
   bool spirv = false;

   MESA_TRACE_FUNC();

   _mesa_clear_shader_program_data(ctx, prog);

   prog->data = _mesa_create_shader_program_data();

   prog->data->LinkStatus = LINKING_SUCCESS;

   for (i = 0; i < prog->NumShaders; i++) {
      if (!prog->Shaders[i]->CompileStatus) {
	 linker_error(prog, "linking with uncompiled/unspecialized shader");
      }

      if (!i) {
         spirv = (prog->Shaders[i]->spirv_data != NULL);
      } else if (spirv && !prog->Shaders[i]->spirv_data) {
         /* The GL_ARB_gl_spirv spec adds a new bullet point to the list of
          * reasons LinkProgram can fail:
          *
          *    "All the shader objects attached to <program> do not have the
          *     same value for the SPIR_V_BINARY_ARB state."
          */
         linker_error(prog,
                      "not all attached shaders have the same "
                      "SPIR_V_BINARY_ARB state");
      }
   }
   prog->data->spirv = spirv;

   if (prog->data->LinkStatus) {
      if (!spirv)
         link_shaders(ctx, prog);
      else
         _mesa_spirv_link_shaders(ctx, prog);
   }

   /* If LinkStatus is LINKING_SUCCESS, then reset sampler validated to true.
    * Validation happens via the LinkShader call below. If LinkStatus is
    * LINKING_SKIPPED, then SamplersValidated will have been restored from the
    * shader cache.
    */
   if (prog->data->LinkStatus == LINKING_SUCCESS) {
      prog->SamplersValidated = GL_TRUE;
   }

   if (prog->data->LinkStatus && !st_link_glsl_to_nir(ctx, prog)) {
      prog->data->LinkStatus = LINKING_FAILURE;
   }

   if (prog->data->LinkStatus != LINKING_FAILURE)
      _mesa_create_program_resource_hash(prog);

   /* Return early if we are loading the shader from on-disk cache */
   if (prog->data->LinkStatus == LINKING_SKIPPED)
      return;

   if (ctx->_Shader->Flags & GLSL_DUMP) {
      if (!prog->data->LinkStatus) {
	 fprintf(stderr, "GLSL shader program %d failed to link\n", prog->Name);
      }

      if (prog->data->InfoLog && prog->data->InfoLog[0] != 0) {
	 fprintf(stderr, "GLSL shader program %d info log:\n", prog->Name);
         fprintf(stderr, "%s\n", prog->data->InfoLog);
      }
   }

#ifdef ENABLE_SHADER_CACHE
   if (prog->data->LinkStatus)
      shader_cache_write_program_metadata(ctx, prog);
#endif
}

} /* extern "C" */
