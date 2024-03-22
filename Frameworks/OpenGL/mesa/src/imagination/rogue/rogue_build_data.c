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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "compiler/shader_enums.h"
#include "nir/nir.h"
#include "rogue.h"
#include "util/macros.h"

/**
 * \file rogue_build_data.c
 *
 * \brief Contains functions to collect build data for the driver.
 */

/* N.B. This will all be hoisted into the driver. */

/**
 * \brief Allocates the coefficient registers that will contain the iterator
 * data for the fragment shader input varyings.
 *
 * \param[in] args The iterator argument data.
 * \return The total number of coefficient registers required by the iterators.
 */
static unsigned alloc_iterator_regs(struct rogue_iterator_args *args)
{
   unsigned coeffs = 0;

   for (unsigned u = 0; u < args->num_fpu_iterators; ++u) {
      /* Ensure there aren't any gaps. */
      assert(args->base[u] == ~0);

      args->base[u] = coeffs;
      coeffs += ROGUE_COEFF_ALIGN * args->components[u];
   }

   return coeffs;
}

/**
 * \brief Reserves an iterator for a fragment shader input varying,
 * and calculates its setup data.
 *
 * \param[in] args The iterator argument data.
 * \param[in] i The iterator index.
 * \param[in] type The interpolation type of the varying.
 * \param[in] f16 Whether the data type is F16 or F32.
 * \param[in] components The number of components in the varying.
 */
static void reserve_iterator(struct rogue_iterator_args *args,
                             unsigned i,
                             enum glsl_interp_mode type,
                             bool f16,
                             unsigned components)
{
   struct ROGUE_PDSINST_DOUT_FIELDS_DOUTI_SRC data = { 0 };

   assert(components >= 1 && components <= 4);

   /* The first iterator (W) *must* be INTERP_MODE_NOPERSPECTIVE. */
   assert(i > 0 || type == INTERP_MODE_NOPERSPECTIVE);
   assert(i < ARRAY_SIZE(args->fpu_iterators));

   switch (type) {
   /* Default interpolation is smooth. */
   case INTERP_MODE_NONE:
      data.shademodel = ROGUE_PDSINST_DOUTI_SHADEMODEL_GOURUAD;
      data.perspective = true;
      break;

   case INTERP_MODE_NOPERSPECTIVE:
      data.shademodel = ROGUE_PDSINST_DOUTI_SHADEMODEL_GOURUAD;
      data.perspective = false;
      break;

   default:
      unreachable("Unimplemented interpolation type.");
   }

   /* Number of components in this varying
    * (corresponds to ROGUE_PDSINST_DOUTI_SIZE_1..4D).
    */
   data.size = (components - 1);

   /* TODO: Investigate F16 support. */
   assert(!f16);
   data.f16 = f16;

   /* Offsets within the vertex. */
   data.f32_offset = 2 * i;
   data.f16_offset = data.f32_offset;

   ROGUE_PDSINST_DOUT_FIELDS_DOUTI_SRC_pack(&args->fpu_iterators[i], &data);
   args->destination[i] = i;
   args->base[i] = ~0;
   args->components[i] = components;
   ++args->num_fpu_iterators;
}

static inline unsigned nir_count_variables_with_modes(const nir_shader *nir,
                                                      nir_variable_mode mode)
{
   unsigned count = 0;

   nir_foreach_variable_with_modes (var, nir, mode) {
      ++count;
   }

   return count;
}

/**
 * \brief Collects the fragment shader I/O data to feed-back to the driver.
 *
 * \sa #collect_io_data()
 *
 * \param[in] common_data Common build data.
 * \param[in] fs_data Fragment-specific build data.
 * \param[in] nir NIR fragment shader.
 */
static void collect_io_data_fs(struct rogue_common_build_data *common_data,
                               struct rogue_fs_build_data *fs_data,
                               nir_shader *nir)
{
   unsigned num_inputs = nir_count_variables_with_modes(nir, nir_var_shader_in);
   assert(num_inputs < (ARRAY_SIZE(fs_data->iterator_args.fpu_iterators) - 1));

   /* Process inputs (if present). */
   if (num_inputs) {
      /* If the fragment shader has inputs, the first iterator
       * must be used for the W component.
       */
      reserve_iterator(&fs_data->iterator_args,
                       0,
                       INTERP_MODE_NOPERSPECTIVE,
                       false,
                       1);

      nir_foreach_shader_in_variable (var, nir) {
         unsigned i = (var->data.location - VARYING_SLOT_VAR0) + 1;
         unsigned components = glsl_get_components(var->type);
         enum glsl_interp_mode interp = var->data.interpolation;
         bool f16 = glsl_type_is_16bit(var->type);

         /* Check that arguments are either F16 or F32. */
         assert(glsl_get_base_type(var->type) == GLSL_TYPE_FLOAT);
         assert(f16 || glsl_type_is_32bit(var->type));

         /* Check input location. */
         assert(var->data.location >= VARYING_SLOT_VAR0 &&
                var->data.location <= VARYING_SLOT_VAR31);

         reserve_iterator(&fs_data->iterator_args, i, interp, f16, components);
      }

      common_data->coeffs = alloc_iterator_regs(&fs_data->iterator_args);
      assert(common_data->coeffs);
      assert(common_data->coeffs <= rogue_reg_infos[ROGUE_REG_CLASS_COEFF].num);
   }

   /* TODO: Process outputs. */
}

/**
 * \brief Allocates the vertex shader outputs.
 *
 * \param[in] outputs The vertex shader output data.
 * \return The total number of vertex outputs required.
 */
static unsigned alloc_vs_outputs(struct rogue_vertex_outputs *outputs)
{
   unsigned vs_outputs = 0;

   for (unsigned u = 0; u < outputs->num_output_vars; ++u) {
      /* Ensure there aren't any gaps. */
      assert(outputs->base[u] == ~0);

      outputs->base[u] = vs_outputs;
      vs_outputs += outputs->components[u];
   }

   return vs_outputs;
}

/**
 * \brief Counts the varyings used by the vertex shader.
 *
 * \param[in] outputs The vertex shader output data.
 * \return The number of varyings used.
 */
static unsigned count_vs_varyings(struct rogue_vertex_outputs *outputs)
{
   unsigned varyings = 0;

   /* Skip the position. */
   for (unsigned u = 1; u < outputs->num_output_vars; ++u)
      varyings += outputs->components[u];

   return varyings;
}

/**
 * \brief Reserves space for a vertex shader input.
 *
 * \param[in] inputs The vertex input data.
 * \param[in] i The vertex input index.
 * \param[in] components The number of components in the input.
 */
static void reserve_vs_input(struct rogue_vertex_inputs *inputs,
                             unsigned i,
                             unsigned components)
{
   assert(components >= 1 && components <= 4);

   assert(i < ARRAY_SIZE(inputs->base));

   inputs->base[i] = ~0;
   inputs->components[i] = components;
   ++inputs->num_input_vars;
}

/**
 * \brief Reserves space for a vertex shader output.
 *
 * \param[in] outputs The vertex output data.
 * \param[in] i The vertex output index.
 * \param[in] components The number of components in the output.
 */
static void reserve_vs_output(struct rogue_vertex_outputs *outputs,
                              unsigned i,
                              unsigned components)
{
   assert(components >= 1 && components <= 4);

   assert(i < ARRAY_SIZE(outputs->base));

   outputs->base[i] = ~0;
   outputs->components[i] = components;
   ++outputs->num_output_vars;
}

/**
 * \brief Collects the vertex shader I/O data to feed-back to the driver.
 *
 * \sa #collect_io_data()
 *
 * \param[in] common_data Common build data.
 * \param[in] vs_data Vertex-specific build data.
 * \param[in] nir NIR vertex shader.
 */
static void collect_io_data_vs(struct rogue_common_build_data *common_data,
                               struct rogue_vs_build_data *vs_data,
                               nir_shader *nir)
{
   ASSERTED bool out_pos_present = false;
   ASSERTED unsigned num_outputs =
      nir_count_variables_with_modes(nir, nir_var_shader_out);

   /* Process outputs. */

   /* We should always have at least a position variable. */
   assert(num_outputs > 0 && "Unsupported number of vertex shader outputs.");

   nir_foreach_shader_out_variable (var, nir) {
      unsigned components = glsl_get_components(var->type);

      /* Check that outputs are F32. */
      /* TODO: Support other types. */
      assert(glsl_get_base_type(var->type) == GLSL_TYPE_FLOAT);
      assert(glsl_type_is_32bit(var->type));

      if (var->data.location == VARYING_SLOT_POS) {
         assert(components == 4);
         out_pos_present = true;

         reserve_vs_output(&vs_data->outputs, 0, components);
      } else if ((var->data.location >= VARYING_SLOT_VAR0) &&
                 (var->data.location <= VARYING_SLOT_VAR31)) {
         unsigned i = (var->data.location - VARYING_SLOT_VAR0) + 1;
         reserve_vs_output(&vs_data->outputs, i, components);
      } else {
         unreachable("Unsupported vertex output type.");
      }
   }

   /* Always need the output position to be present. */
   assert(out_pos_present);

   vs_data->num_vertex_outputs = alloc_vs_outputs(&vs_data->outputs);
   assert(vs_data->num_vertex_outputs);
   assert(vs_data->num_vertex_outputs <
          rogue_reg_infos[ROGUE_REG_CLASS_VTXOUT].num);

   vs_data->num_varyings = count_vs_varyings(&vs_data->outputs);
}

/**
 * \brief Collects I/O data to feed-back to the driver.
 *
 * Collects the inputs/outputs/memory required, and feeds that back to the
 * driver. Done at this stage rather than at the start of rogue_to_binary, so
 * that all the I/O of all the shader stages is known before backend
 * compilation, which would let us do things like cull unused inputs.
 *
 * \param[in] ctx Shared multi-stage build context.
 * \param[in] nir NIR shader.
 */
PUBLIC
void rogue_collect_io_data(struct rogue_build_ctx *ctx, nir_shader *nir)
{
   gl_shader_stage stage = nir->info.stage;
   struct rogue_common_build_data *common_data = &ctx->common_data[stage];

   /* Collect stage-specific data. */
   switch (stage) {
   case MESA_SHADER_FRAGMENT:
      return collect_io_data_fs(common_data, &ctx->stage_data.fs, nir);

   case MESA_SHADER_VERTEX:
      return collect_io_data_vs(common_data, &ctx->stage_data.vs, nir);

   default:
      break;
   }

   unreachable("Unsupported stage.");
}

/**
 * \brief Returns the allocated coefficient register index for a component of an
 * input varying location.
 *
 * \param[in] args The allocated iterator argument data.
 * \param[in] location The input varying location, or ~0 for the W coefficient.
 * \param[in] component The requested component.
 * \return The coefficient register index.
 */
PUBLIC
unsigned rogue_coeff_index_fs(struct rogue_iterator_args *args,
                              gl_varying_slot location,
                              unsigned component)
{
   unsigned i;

   /* Special case: W coefficient. */
   if (location == ~0) {
      /* The W component shouldn't be the only one. */
      assert(args->num_fpu_iterators > 1);
      assert(args->destination[0] == 0);
      return 0;
   }

   i = (location - VARYING_SLOT_VAR0) + 1;
   assert(location >= VARYING_SLOT_VAR0 && location <= VARYING_SLOT_VAR31);
   assert(i < args->num_fpu_iterators);
   assert(component < args->components[i]);
   assert(args->base[i] != ~0);

   return args->base[i] + (ROGUE_COEFF_ALIGN * component);
}

/**
 * \brief Returns the allocated vertex output index for a component of an input
 * varying location.
 *
 * \param[in] outputs The vertex output data.
 * \param[in] location The output varying location.
 * \param[in] component The requested component.
 * \return The vertex output index.
 */
PUBLIC
unsigned rogue_output_index_vs(struct rogue_vertex_outputs *outputs,
                               gl_varying_slot location,
                               unsigned component)
{
   unsigned i;

   if (location == VARYING_SLOT_POS) {
      /* Always at location 0. */
      assert(outputs->base[0] == 0);
      i = 0;
   } else if ((location >= VARYING_SLOT_VAR0) &&
              (location <= VARYING_SLOT_VAR31)) {
      i = (location - VARYING_SLOT_VAR0) + 1;
   } else {
      unreachable("Unsupported vertex output type.");
   }

   assert(i < outputs->num_output_vars);
   assert(component < outputs->components[i]);
   assert(outputs->base[i] != ~0);

   return outputs->base[i] + component;
}
