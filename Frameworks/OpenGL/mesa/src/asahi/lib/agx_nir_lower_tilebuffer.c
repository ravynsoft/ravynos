/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include "compiler/agx_internal_formats.h"
#include "compiler/glsl_types.h"
#include "util/format/u_format.h"
#include "util/macros.h"
#include "agx_nir_format_helpers.h"
#include "agx_pack.h"
#include "agx_tilebuffer.h"
#include "nir.h"
#include "nir_builder.h"
#include "nir_builder_opcodes.h"

#define AGX_NUM_TEXTURE_STATE_REGS 16
#define ALL_SAMPLES                0xFF

struct ctx {
   struct agx_tilebuffer_layout *tib;
   uint8_t *colormasks;
   bool *translucent;
   unsigned bindless_base;
   bool any_memory_stores;
   bool layer_id_sr;
   uint8_t outputs_written;
};

static bool
tib_filter(const nir_instr *instr, UNUSED const void *_)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic != nir_intrinsic_store_output &&
       intr->intrinsic != nir_intrinsic_load_output)
      return false;

   nir_io_semantics sem = nir_intrinsic_io_semantics(intr);
   assert(sem.dual_source_blend_index == 0 && "dual source blending lowered");
   return (sem.location >= FRAG_RESULT_DATA0);
}

static void
store_tilebuffer(nir_builder *b, struct agx_tilebuffer_layout *tib,
                 enum pipe_format format, enum pipe_format logical_format,
                 unsigned rt, nir_def *value, unsigned write_mask)
{
   /* The hardware cannot extend for a 32-bit format. Extend ourselves. */
   if (format == PIPE_FORMAT_R32_UINT && value->bit_size == 16) {
      if (util_format_is_pure_sint(logical_format))
         value = nir_i2i32(b, value);
      else if (util_format_is_pure_uint(logical_format))
         value = nir_u2u32(b, value);
      else
         value = nir_f2f32(b, value);
   }

   /* Pure integer formatss need to be clamped in software, at least in some
    * cases. We do so on store. Piglit gl-3.0-render-integer checks this, as
    * does KHR-GL33.packed_pixels.*.
    */
   const struct util_format_description *desc =
      util_format_description(logical_format);
   unsigned c = util_format_get_first_non_void_channel(logical_format);

   if (desc->channel[c].size <= 16 &&
       util_format_is_pure_integer(logical_format)) {

      unsigned bits[4] = {
         desc->channel[0].size,
         desc->channel[1].size,
         desc->channel[2].size,
         desc->channel[3].size,
      };

      if (util_format_is_pure_sint(logical_format))
         value = nir_format_clamp_sint(b, value, bits);
      else
         value = nir_format_clamp_uint(b, value, bits);

      value = nir_u2u16(b, value);
   }

   uint8_t offset_B = agx_tilebuffer_offset_B(tib, rt);
   nir_store_local_pixel_agx(b, value, nir_imm_intN_t(b, ALL_SAMPLES, 16),
                             .base = offset_B, .write_mask = write_mask,
                             .format = format);
}

static nir_def *
load_tilebuffer(nir_builder *b, struct agx_tilebuffer_layout *tib,
                uint8_t load_comps, uint8_t bit_size, unsigned rt,
                enum pipe_format format, enum pipe_format logical_format)
{
   unsigned comps = util_format_get_nr_components(logical_format);
   bool f16 = (format == PIPE_FORMAT_R16_FLOAT);

   /* Don't load with F16 */
   if (f16)
      format = PIPE_FORMAT_R16_UINT;

   uint8_t offset_B = agx_tilebuffer_offset_B(tib, rt);
   nir_def *res = nir_load_local_pixel_agx(
      b, MIN2(load_comps, comps), f16 ? 16 : bit_size,
      nir_imm_intN_t(b, ALL_SAMPLES, 16), .base = offset_B, .format = format);

   /* Extend floats */
   if (f16 && bit_size != 16) {
      assert(bit_size == 32);
      res = nir_f2f32(b, res);
   }

   res = nir_sign_extend_if_sint(b, res, logical_format);
   return nir_pad_vector(b, res, load_comps);
}

/*
 * As a simple implementation, we use image load/store instructions to access
 * spilled render targets. The driver will supply corresponding texture and PBE
 * descriptors for each render target, accessed bindlessly
 *
 * Note that this lower happens after driver bindings are lowered, so the
 * bindless handle is in the AGX-specific format.
 */
static nir_def *
handle_for_rt(nir_builder *b, unsigned base, unsigned rt, bool pbe,
              bool *bindless)
{
   unsigned index = base + (2 * rt) + (pbe ? 1 : 0);
   *bindless = (*bindless) || (index >= AGX_NUM_TEXTURE_STATE_REGS);

   if (*bindless)
      return nir_load_texture_handle_agx(b, nir_imm_int(b, index));
   else
      return nir_imm_intN_t(b, index, 16);
}

static enum glsl_sampler_dim
dim_for_rt(nir_builder *b, unsigned nr_samples, nir_def **sample)
{
   if (nr_samples == 1) {
      *sample = nir_imm_intN_t(b, 0, 16);
      return GLSL_SAMPLER_DIM_2D;
   } else {
      *sample = nir_load_sample_id(b);
      b->shader->info.fs.uses_sample_shading = true;
      return GLSL_SAMPLER_DIM_MS;
   }
}

static nir_def *
image_coords(nir_builder *b, nir_def *layer_id)
{
   nir_def *xy = nir_u2u32(b, nir_load_pixel_coord(b));
   nir_def *vec = nir_pad_vector(b, xy, 4);

   if (layer_id)
      vec = nir_vector_insert_imm(b, vec, layer_id, 2);

   return vec;
}

static void
store_memory(nir_builder *b, unsigned bindless_base, unsigned nr_samples,
             nir_def *layer_id, enum pipe_format format, unsigned rt,
             nir_def *value)
{
   /* Force bindless for multisampled image writes since they will be lowered
    * with a descriptor crawl later.
    */
   bool bindless = (nr_samples > 1);
   nir_def *image = handle_for_rt(b, bindless_base, rt, true, &bindless);
   nir_def *zero = nir_imm_intN_t(b, 0, 16);
   nir_def *lod = zero;

   nir_def *sample;
   enum glsl_sampler_dim dim = dim_for_rt(b, nr_samples, &sample);
   nir_def *coords = image_coords(b, layer_id);

   nir_begin_invocation_interlock(b);

   if (nr_samples > 1) {
      nir_def *coverage = nir_load_sample_mask(b);
      nir_def *covered = nir_ubitfield_extract(
         b, coverage, nir_u2u32(b, sample), nir_imm_int(b, 1));

      nir_push_if(b, nir_ine_imm(b, covered, 0));
   }

   if (bindless) {
      nir_bindless_image_store(b, image, coords, sample, value, lod,
                               .image_dim = dim, .image_array = true,
                               .format = format);
   } else {
      nir_image_store(b, image, coords, sample, value, lod, .image_dim = dim,
                      .image_array = true, .format = format);
   }

   if (nr_samples > 1)
      nir_pop_if(b, NULL);

   b->shader->info.writes_memory = true;
}

static nir_def *
load_memory(nir_builder *b, unsigned bindless_base, unsigned nr_samples,
            nir_def *layer_id, uint8_t comps, uint8_t bit_size, unsigned rt,
            enum pipe_format format)
{
   bool bindless = false;
   nir_def *image = handle_for_rt(b, bindless_base, rt, false, &bindless);
   nir_def *zero = nir_imm_intN_t(b, 0, 16);
   nir_def *lod = zero;

   nir_def *sample;
   enum glsl_sampler_dim dim = dim_for_rt(b, nr_samples, &sample);
   nir_def *coords = image_coords(b, layer_id);

   /* Ensure pixels below this one have written out their results */
   nir_begin_invocation_interlock(b);

   if (bindless) {
      return nir_bindless_image_load(b, comps, bit_size, image, coords, sample,
                                     lod, .image_dim = dim, .image_array = true,
                                     .format = format);
   } else {
      return nir_image_load(b, comps, bit_size, image, coords, sample, lod,
                            .image_dim = dim, .image_array = true,
                            .format = format);
   }
}

nir_def *
agx_internal_layer_id(nir_builder *b)
{
   /* In the background and end-of-tile programs, the layer ID is available as
    * sr2, the Z component of the workgroup index.
    */
   return nir_channel(b, nir_load_workgroup_id(b), 2);
}

static nir_def *
tib_layer_id(nir_builder *b, struct ctx *ctx)
{
   if (ctx->layer_id_sr) {
      return agx_internal_layer_id(b);
   } else {
      /* Otherwise, the layer ID is loaded as a flat varying. */
      b->shader->info.inputs_read |= VARYING_BIT_LAYER;

      return nir_load_input(b, 1, 32, nir_imm_int(b, 0),
                            .io_semantics.location = VARYING_SLOT_LAYER);
   }
}

static nir_def *
tib_impl(nir_builder *b, nir_instr *instr, void *data)
{
   struct ctx *ctx = data;
   struct agx_tilebuffer_layout *tib = ctx->tib;
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   nir_io_semantics sem = nir_intrinsic_io_semantics(intr);
   unsigned rt = sem.location - FRAG_RESULT_DATA0;
   assert(rt < ARRAY_SIZE(tib->logical_format));

   enum pipe_format logical_format = tib->logical_format[rt];
   enum pipe_format format = agx_tilebuffer_physical_format(tib, rt);
   unsigned comps = util_format_get_nr_components(logical_format);

   if (intr->intrinsic == nir_intrinsic_store_output) {
      ctx->outputs_written |= BITFIELD_BIT(rt);

      /* Only write components that actually exist */
      uint16_t write_mask = (uint16_t)BITFIELD_MASK(comps);

      /* Delete stores to nonexistent render targets */
      if (logical_format == PIPE_FORMAT_NONE)
         return NIR_LOWER_INSTR_PROGRESS_REPLACE;

      /* Only write colours masked by the blend state */
      if (ctx->colormasks)
         write_mask &= ctx->colormasks[rt];

      /* Masked stores require a translucent pass type */
      if (write_mask != BITFIELD_MASK(comps)) {
         assert(ctx->translucent != NULL &&
                "colour masking requires translucency");

         assert(agx_tilebuffer_supports_mask(tib, rt));
         *(ctx->translucent) = true;
      }

      /* But we ignore the NIR write mask for that, since it's basically an
       * optimization hint.
       */
      if (agx_tilebuffer_supports_mask(tib, rt))
         write_mask &= nir_intrinsic_write_mask(intr);

      /* Delete stores that are entirely masked out */
      if (!write_mask)
         return NIR_LOWER_INSTR_PROGRESS_REPLACE;

      nir_def *value = intr->src[0].ssa;

      /* Trim to format as required by hardware */
      value = nir_trim_vector(b, intr->src[0].ssa, comps);

      if (tib->spilled[rt]) {
         store_memory(b, ctx->bindless_base, tib->nr_samples,
                      tib_layer_id(b, ctx), logical_format, rt, value);
         ctx->any_memory_stores = true;
      } else {
         store_tilebuffer(b, tib, format, logical_format, rt, value,
                          write_mask);
      }

      return NIR_LOWER_INSTR_PROGRESS_REPLACE;
   } else {
      uint8_t bit_size = intr->def.bit_size;

      /* Loads from non-existent render targets are undefined in NIR but not
       * possible to encode in the hardware, delete them.
       */
      if (logical_format == PIPE_FORMAT_NONE) {
         return nir_undef(b, intr->num_components, bit_size);
      } else if (tib->spilled[rt]) {
         *(ctx->translucent) = true;

         return load_memory(b, ctx->bindless_base, tib->nr_samples,
                            tib_layer_id(b, ctx), intr->num_components,
                            bit_size, rt, logical_format);
      } else {
         return load_tilebuffer(b, tib, intr->num_components, bit_size, rt,
                                format, logical_format);
      }
   }
}

bool
agx_nir_lower_tilebuffer(nir_shader *shader, struct agx_tilebuffer_layout *tib,
                         uint8_t *colormasks, unsigned *bindless_base,
                         bool *translucent, bool layer_id_sr)
{
   assert(shader->info.stage == MESA_SHADER_FRAGMENT);

   struct ctx ctx = {
      .tib = tib,
      .colormasks = colormasks,
      .translucent = translucent,
      .layer_id_sr = layer_id_sr,
   };

   /* Allocate 1 texture + 1 PBE descriptor for each spilled descriptor */
   if (agx_tilebuffer_spills(tib)) {
      assert(bindless_base != NULL && "must be specified if spilling");
      ctx.bindless_base = *bindless_base;
      *bindless_base += (AGX_MAX_RENDER_TARGETS * 2);
   }

   bool progress =
      nir_shader_lower_instructions(shader, tib_filter, tib_impl, &ctx);

   /* Flush at end */
   if (ctx.any_memory_stores) {
      nir_function_impl *impl = nir_shader_get_entrypoint(shader);
      nir_builder b = nir_builder_at(nir_after_impl(impl));
      nir_fence_pbe_to_tex_pixel_agx(&b);
   }

   /* If there are any render targets bound to the framebuffer that aren't
    * statically written by the fragment shader, that acts as an implicit mask
    * and requires translucency.
    *
    * XXX: Could be optimized.
    */
   for (unsigned i = 0; i < ARRAY_SIZE(tib->logical_format); ++i) {
      bool exists = tib->logical_format[i] != PIPE_FORMAT_NONE;
      bool written = ctx.outputs_written & BITFIELD_BIT(i);

      if (translucent)
         *translucent |= (exists && !written);
   }

   return progress;
}
