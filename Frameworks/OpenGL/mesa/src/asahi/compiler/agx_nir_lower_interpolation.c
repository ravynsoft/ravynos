/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "compiler/shader_enums.h"
#include "agx_nir.h"
#include "nir.h"
#include "nir_builder.h"
#include "nir_builder_opcodes.h"
#include "nir_intrinsics.h"
#include "nir_intrinsics_indices.h"

/*
 * In AGX, the values of fragment shader inputs are represented as coefficient
 * vectors <A, B, C>, which are dotted with <x, y, 1> to perform interpolation.
 * x and y are relative to the tile. In other words, A and B are the
 * screen-space partial derivatives of the input, and C is the value at the
 * corner of the tile.
 *
 * For some interpolation modes, the dot product happens in the iterator
 * hardware. Other modes are implemented in this file, by lowering to math on
 * the coefficient vectors.
 */

/* XXX: It's not clear what this is for, but seems necessary */
static nir_def *
cf_valid(nir_builder *b, nir_def *cf)
{
   nir_def *bit = nir_ieq_imm(b, nir_iand_imm(b, nir_channel(b, cf, 0), 1), 0);

   /* XXX: Apple's compiler actually checks that the significand is nonzero and
    * the exponent is 0 or 1. This is probably a typo -- it doesn't make any
    * logical sense.  Presumably they just meant to check for denorms, so let's
    * do that. Either way the tests pass.
    */
   nir_def *cf01 = nir_trim_vector(b, cf, 2);
   return nir_ior(b, bit, nir_fisnormal(b, cf01));
}

static nir_def *
interpolate_at_offset(nir_builder *b, nir_def *cf, nir_def *offset,
                      bool perspective)
{
   /* Get the coordinate of the pixel within the tile */
   nir_def *pixel_coords = nir_load_pixel_coord(b);
   nir_def *tile_offs = nir_umod_imm(b, pixel_coords, 32);

   /* Convert to float, getting the center of the pixel */
   nir_def *center = nir_fadd_imm(b, nir_u2f32(b, tile_offs), 0.5);

   /* Calculate the location to interpolate. offset is defined relative to the
    * center of the pixel and is a float.
    */
   nir_def *pos = nir_fadd(b, center, nir_f2f32(b, offset));

   /* Interpolate with the given coefficients */
   nir_def *interp = nir_ffma(b, nir_channel(b, pos, 1), nir_channel(b, cf, 1),
                              nir_channel(b, cf, 2));

   interp = nir_ffma(b, nir_channel(b, pos, 0), nir_channel(b, cf, 0), interp);

   /* Divide by RHW. This load will be lowered recursively. */
   if (perspective) {
      nir_def *bary = nir_load_barycentric_at_offset(
         b, 32, offset, .interp_mode = INTERP_MODE_NOPERSPECTIVE);

      nir_def *rhw = nir_load_interpolated_input(
         b, 1, 32, bary, nir_imm_int(b, 0), .component = 3,
         .io_semantics = {
            .location = VARYING_SLOT_POS,
            .num_slots = 1,
         });

      interp = nir_fdiv(b, interp, rhw);
   }

   /* Replace invalid interpolations with the constant channel */
   return nir_bcsel(b, cf_valid(b, cf), interp, nir_channel(b, cf, 2));
}

static nir_def *
interpolate_flat(nir_builder *b, nir_def *coefficients)
{
   /* Same value anywhere, so just take the constant (affine) component */
   return nir_channel(b, coefficients, 2);
}

static enum glsl_interp_mode
interp_mode_for_load(nir_intrinsic_instr *load)
{
   if (load->intrinsic == nir_intrinsic_load_input)
      return INTERP_MODE_FLAT;
   else
      return nir_intrinsic_interp_mode(nir_src_as_intrinsic(load->src[0]));
}

static bool
needs_lower(const nir_instr *instr, UNUSED const void *_)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   const nir_intrinsic_instr *load = nir_instr_as_intrinsic(instr);

   /* at_offset barycentrics need to be lowered */
   if (load->intrinsic == nir_intrinsic_load_interpolated_input) {
      return (nir_src_as_intrinsic(load->src[0])->intrinsic ==
              nir_intrinsic_load_barycentric_at_offset);
   }

   /* Flat shading always lowered */
   return (load->intrinsic == nir_intrinsic_load_input);
}

static nir_def *
interpolate_channel(nir_builder *b, nir_intrinsic_instr *load, unsigned channel)
{
   nir_io_semantics sem = nir_intrinsic_io_semantics(load);

   /* Indirect varyings not supported, just bias the location */
   sem.location += nir_src_as_uint(*nir_get_io_offset_src(load));
   sem.num_slots = 1;

   nir_def *coefficients = nir_load_coefficients_agx(
      b, .component = nir_intrinsic_component(load) + channel,
      .interp_mode = interp_mode_for_load(load), .io_semantics = sem);

   if (load->intrinsic == nir_intrinsic_load_input) {
      assert(load->def.bit_size == 32);
      return interpolate_flat(b, coefficients);
   } else {
      nir_intrinsic_instr *bary = nir_src_as_intrinsic(load->src[0]);

      nir_def *interp = interpolate_at_offset(
         b, coefficients, bary->src[0].ssa,
         nir_intrinsic_interp_mode(bary) != INTERP_MODE_NOPERSPECTIVE);

      return nir_f2fN(b, interp, load->def.bit_size);
   }
}

static nir_def *
lower(nir_builder *b, nir_instr *instr, void *data)
{
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   /* Each component is loaded separated */
   nir_def *values[NIR_MAX_VEC_COMPONENTS] = {NULL};
   for (unsigned i = 0; i < intr->def.num_components; ++i) {
      values[i] = interpolate_channel(b, intr, i);
   }

   return nir_vec(b, values, intr->def.num_components);
}

bool
agx_nir_lower_interpolation(nir_shader *s)
{
   assert(s->info.stage == MESA_SHADER_FRAGMENT);

   return nir_shader_lower_instructions(s, needs_lower, lower, NULL);
}
