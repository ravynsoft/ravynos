/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "compiler/spirv/nir_spirv.h"
#include "nir/nir.h"
#include "rogue.h"
#include "util/macros.h"

#include <stdbool.h>

/**
 * \file rogue_nir.c
 *
 * \brief Contains SPIR-V and NIR-specific functions.
 */

/**
 * \brief SPIR-V to NIR compilation options.
 */
static const struct spirv_to_nir_options spirv_options = {
   .environment = NIR_SPIRV_VULKAN,

   /* Buffer address: (descriptor_set, binding), offset. */
   .ubo_addr_format = nir_address_format_64bit_global,
};

static const nir_shader_compiler_options nir_options = {
   .fuse_ffma32 = true,
};

static int rogue_glsl_type_size(const struct glsl_type *type, bool bindless)
{
   return glsl_count_attribute_slots(type, false);
}

/**
 * \brief Applies optimizations and passes required to lower the NIR shader into
 * a form suitable for lowering to Rogue IR.
 *
 * \param[in] ctx Shared multi-stage build context.
 * \param[in] shader Rogue shader.
 * \param[in] stage Shader stage.
 */
static void rogue_nir_passes(struct rogue_build_ctx *ctx,
                             nir_shader *nir,
                             gl_shader_stage stage)
{
   bool progress;

#if !defined(NDEBUG)
   bool nir_debug_print_shader_prev = nir_debug_print_shader[nir->info.stage];
   nir_debug_print_shader[nir->info.stage] = ROGUE_DEBUG(NIR_PASSES);
#endif /* !defined(NDEBUG) */

   nir_validate_shader(nir, "after spirv_to_nir");

   NIR_PASS_V(nir, nir_lower_vars_to_ssa);

   /* Splitting. */
   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_split_per_member_structs);

   /* Replace references to I/O variables with intrinsics. */
   NIR_PASS_V(nir,
              nir_lower_io,
              nir_var_shader_in | nir_var_shader_out,
              rogue_glsl_type_size,
              (nir_lower_io_options)0);

   /* Load inputs to scalars (single registers later). */
   /* TODO: Fitrp can process multiple frag inputs at once, scalarise I/O. */
   NIR_PASS_V(nir, nir_lower_io_to_scalar, nir_var_shader_in, NULL, NULL);

   /* Optimize GL access qualifiers. */
   const nir_opt_access_options opt_access_options = {
      .is_vulkan = true,
   };
   NIR_PASS_V(nir, nir_opt_access, &opt_access_options);

   /* Apply PFO code to the fragment shader output. */
   if (nir->info.stage == MESA_SHADER_FRAGMENT)
      NIR_PASS_V(nir, rogue_nir_pfo);

   /* Load outputs to scalars (single registers later). */
   NIR_PASS_V(nir, nir_lower_io_to_scalar, nir_var_shader_out, NULL, NULL);

   /* Lower ALU operations to scalars. */
   NIR_PASS_V(nir, nir_lower_alu_to_scalar, NULL, NULL);

   /* Lower load_consts to scalars. */
   NIR_PASS_V(nir, nir_lower_load_const_to_scalar);

   /* Additional I/O lowering. */
   NIR_PASS_V(nir,
              nir_lower_explicit_io,
              nir_var_mem_ubo,
              spirv_options.ubo_addr_format);
   NIR_PASS_V(nir, nir_lower_io_to_scalar, nir_var_mem_ubo, NULL, NULL);
   NIR_PASS_V(nir, rogue_nir_lower_io);

   /* Algebraic opts. */
   do {
      progress = false;

      NIR_PASS(progress, nir, nir_copy_prop);
      NIR_PASS(progress, nir, nir_opt_cse);
      NIR_PASS(progress, nir, nir_opt_algebraic);
      NIR_PASS(progress, nir, nir_opt_constant_folding);
      NIR_PASS(progress, nir, nir_opt_dce);
      NIR_PASS_V(nir, nir_opt_gcm, false);
   } while (progress);

   /* Late algebraic opts. */
   do {
      progress = false;

      NIR_PASS(progress, nir, nir_opt_algebraic_late);
      NIR_PASS_V(nir, nir_opt_constant_folding);
      NIR_PASS_V(nir, nir_copy_prop);
      NIR_PASS_V(nir, nir_opt_dce);
      NIR_PASS_V(nir, nir_opt_cse);
   } while (progress);

   /* Remove unused constant registers. */
   NIR_PASS_V(nir, nir_opt_dce);

   /* Move loads to just before they're needed. */
   /* Disabled for now since we want to try and keep them vectorised and group
    * them. */
   /* TODO: Investigate this further. */
   /* NIR_PASS_V(nir, nir_opt_move, nir_move_load_ubo | nir_move_load_input); */

   /* TODO: Re-enable scheduling after register pressure tweaks. */
#if 0
	/* Instruction scheduling. */
	struct nir_schedule_options schedule_options = {
		.threshold = ROGUE_MAX_REG_TEMP / 2,
	};
	NIR_PASS_V(nir, nir_schedule, &schedule_options);
#endif

   /* Assign I/O locations. */
   nir_assign_io_var_locations(nir,
                               nir_var_shader_in,
                               &nir->num_inputs,
                               nir->info.stage);
   nir_assign_io_var_locations(nir,
                               nir_var_shader_out,
                               &nir->num_outputs,
                               nir->info.stage);

   /* Renumber SSA defs. */
   nir_index_ssa_defs(nir_shader_get_entrypoint(nir));

   /* Gather info into nir shader struct. */
   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));

   /* Clean-up after passes. */
   nir_sweep(nir);

   nir_validate_shader(nir, "after passes");
   if (ROGUE_DEBUG(NIR)) {
      fputs("after passes\n", stdout);
      nir_print_shader(nir, stdout);
   }

#if !defined(NDEBUG)
   nir_debug_print_shader[nir->info.stage] = nir_debug_print_shader_prev;
#endif /* !defined(NDEBUG) */
}

/**
 * \brief Converts a SPIR-V shader to NIR.
 *
 * \param[in] ctx Shared multi-stage build context.
 * \param[in] entry Shader entry-point function name.
 * \param[in] stage Shader stage.
 * \param[in] spirv_size SPIR-V data length in DWORDs.
 * \param[in] spirv_data SPIR-V data.
 * \param[in] num_spec Number of SPIR-V specializations.
 * \param[in] spec SPIR-V specializations.
 * \return A nir_shader* if successful, or NULL if unsuccessful.
 */
PUBLIC
nir_shader *rogue_spirv_to_nir(rogue_build_ctx *ctx,
                               gl_shader_stage stage,
                               const char *entry,
                               unsigned spirv_size,
                               const uint32_t *spirv_data,
                               unsigned num_spec,
                               struct nir_spirv_specialization *spec)
{
   nir_shader *nir;

   nir = spirv_to_nir(spirv_data,
                      spirv_size,
                      spec,
                      num_spec,
                      stage,
                      entry,
                      &spirv_options,
                      &nir_options);
   if (!nir)
      return NULL;

   ralloc_steal(ctx, nir);

   /* Apply passes. */
   rogue_nir_passes(ctx, nir, stage);

   /* Collect I/O data to pass back to the driver. */
   rogue_collect_io_data(ctx, nir);

   return nir;
}
