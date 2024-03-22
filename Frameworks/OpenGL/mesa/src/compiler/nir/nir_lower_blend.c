/*
 * Copyright (C) 2019-2021 Collabora, Ltd.
 * Copyright (C) 2019 Alyssa Rosenzweig
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

/**
 * @file
 *
 * Implements the fragment pipeline (blending and writeout) in software, to be
 * run as a dedicated "blend shader" stage on Midgard/Bifrost, or as a fragment
 * shader variant on typical GPUs. This pass is useful if hardware lacks
 * fixed-function blending in part or in full.
 */

#include "nir_lower_blend.h"
#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_format_convert.h"
#include "util/blend.h"

struct ctx {
   const nir_lower_blend_options *options;
   nir_def *src1[8];
};

/* Given processed factors, combine them per a blend function */

static nir_def *
nir_blend_func(
   nir_builder *b,
   enum pipe_blend_func func,
   nir_def *src, nir_def *dst)
{
   switch (func) {
   case PIPE_BLEND_ADD:
      return nir_fadd(b, src, dst);
   case PIPE_BLEND_SUBTRACT:
      return nir_fsub(b, src, dst);
   case PIPE_BLEND_REVERSE_SUBTRACT:
      return nir_fsub(b, dst, src);
   case PIPE_BLEND_MIN:
      return nir_fmin(b, src, dst);
   case PIPE_BLEND_MAX:
      return nir_fmax(b, src, dst);
   }

   unreachable("Invalid blend function");
}

/* Does this blend function multiply by a blend factor? */

static bool
nir_blend_factored(enum pipe_blend_func func)
{
   switch (func) {
   case PIPE_BLEND_ADD:
   case PIPE_BLEND_SUBTRACT:
   case PIPE_BLEND_REVERSE_SUBTRACT:
      return true;
   default:
      return false;
   }
}

/* Compute a src_alpha_saturate factor */
static nir_def *
nir_alpha_saturate(
   nir_builder *b,
   nir_def *src, nir_def *dst,
   unsigned chan)
{
   nir_def *Asrc = nir_channel(b, src, 3);
   nir_def *Adst = nir_channel(b, dst, 3);
   nir_def *one = nir_imm_floatN_t(b, 1.0, src->bit_size);
   nir_def *Adsti = nir_fsub(b, one, Adst);

   return (chan < 3) ? nir_fmin(b, Asrc, Adsti) : one;
}

/* Returns a scalar single factor, unmultiplied */

static nir_def *
nir_blend_factor_value(
   nir_builder *b,
   nir_def *src, nir_def *src1, nir_def *dst, nir_def *bconst,
   unsigned chan,
   enum pipe_blendfactor factor_without_invert)
{
   switch (factor_without_invert) {
   case PIPE_BLENDFACTOR_ONE:
      return nir_imm_floatN_t(b, 1.0, src->bit_size);
   case PIPE_BLENDFACTOR_SRC_COLOR:
      return nir_channel(b, src, chan);
   case PIPE_BLENDFACTOR_SRC1_COLOR:
      return nir_channel(b, src1, chan);
   case PIPE_BLENDFACTOR_DST_COLOR:
      return nir_channel(b, dst, chan);
   case PIPE_BLENDFACTOR_SRC_ALPHA:
      return nir_channel(b, src, 3);
   case PIPE_BLENDFACTOR_SRC1_ALPHA:
      return nir_channel(b, src1, 3);
   case PIPE_BLENDFACTOR_DST_ALPHA:
      return nir_channel(b, dst, 3);
   case PIPE_BLENDFACTOR_CONST_COLOR:
      return nir_channel(b, bconst, chan);
   case PIPE_BLENDFACTOR_CONST_ALPHA:
      return nir_channel(b, bconst, 3);
   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
      return nir_alpha_saturate(b, src, dst, chan);
   default:
      assert(util_blendfactor_is_inverted(factor_without_invert));
      unreachable("Unexpected inverted factor");
   }
}

static nir_def *
nir_fsat_signed(nir_builder *b, nir_def *x)
{
   return nir_fclamp(b, x, nir_imm_floatN_t(b, -1.0, x->bit_size),
                     nir_imm_floatN_t(b, +1.0, x->bit_size));
}

static nir_def *
nir_fsat_to_format(nir_builder *b, nir_def *x, enum pipe_format format)
{
   if (util_format_is_unorm(format))
      return nir_fsat(b, x);
   else if (util_format_is_snorm(format))
      return nir_fsat_signed(b, x);
   else
      return x;
}

/*
 * The spec says we need to clamp blend factors. However, we don't want to clamp
 * unnecessarily, as the clamp might not be optimized out. Check whether
 * clamping a blend factor is needed.
 */
static bool
should_clamp_factor(enum pipe_blendfactor factor, bool snorm)
{
   switch (util_blendfactor_without_invert(factor)) {
   case PIPE_BLENDFACTOR_ONE:
      /* 0, 1 are in [0, 1] and [-1, 1] */
      return false;

   case PIPE_BLENDFACTOR_SRC_COLOR:
   case PIPE_BLENDFACTOR_SRC1_COLOR:
   case PIPE_BLENDFACTOR_DST_COLOR:
   case PIPE_BLENDFACTOR_SRC_ALPHA:
   case PIPE_BLENDFACTOR_SRC1_ALPHA:
   case PIPE_BLENDFACTOR_DST_ALPHA:
      /* Colours are already clamped. For unorm, the complement of something
       * clamped is still clamped. But for snorm, this is not true. Clamp for
       * snorm only.
       */
      return util_blendfactor_is_inverted(factor) && snorm;

   case PIPE_BLENDFACTOR_CONST_COLOR:
   case PIPE_BLENDFACTOR_CONST_ALPHA:
      /* Constant colours are not yet clamped */
      return true;

   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
      /* For unorm, this is in bounds (and hence so is its complement). For
       * snorm, it may not be.
       */
      return snorm;

   default:
      unreachable("invalid blend factor");
   }
}

static bool
channel_uses_dest(nir_lower_blend_channel chan)
{
   /* If blend factors are ignored, dest is used (min/max) */
   if (!nir_blend_factored(chan.func))
      return true;

   /* If dest has a nonzero factor, it is used */
   if (chan.dst_factor != PIPE_BLENDFACTOR_ZERO)
      return true;

   /* Else, check the source factor */
   switch (util_blendfactor_without_invert(chan.src_factor)) {
   case PIPE_BLENDFACTOR_DST_COLOR:
   case PIPE_BLENDFACTOR_DST_ALPHA:
   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
      return true;
   default:
      return false;
   }
}

static nir_def *
nir_blend_factor(
   nir_builder *b,
   nir_def *raw_scalar,
   nir_def *src, nir_def *src1, nir_def *dst, nir_def *bconst,
   unsigned chan,
   enum pipe_blendfactor factor,
   enum pipe_format format)
{
   nir_def *f =
      nir_blend_factor_value(b, src, src1, dst, bconst, chan,
                             util_blendfactor_without_invert(factor));

   if (util_blendfactor_is_inverted(factor))
      f = nir_fadd_imm(b, nir_fneg(b, f), 1.0);

   if (should_clamp_factor(factor, util_format_is_snorm(format)))
      f = nir_fsat_to_format(b, f, format);

   return nir_fmul(b, raw_scalar, f);
}

/* Given a colormask, "blend" with the destination */

static nir_def *
nir_color_mask(
   nir_builder *b,
   unsigned mask,
   nir_def *src,
   nir_def *dst)
{
   return nir_vec4(b,
                   nir_channel(b, (mask & (1 << 0)) ? src : dst, 0),
                   nir_channel(b, (mask & (1 << 1)) ? src : dst, 1),
                   nir_channel(b, (mask & (1 << 2)) ? src : dst, 2),
                   nir_channel(b, (mask & (1 << 3)) ? src : dst, 3));
}

static nir_def *
nir_logicop_func(
   nir_builder *b,
   enum pipe_logicop func,
   nir_def *src, nir_def *dst, nir_def *bitmask)
{
   switch (func) {
   case PIPE_LOGICOP_CLEAR:
      return nir_imm_ivec4(b, 0, 0, 0, 0);
   case PIPE_LOGICOP_NOR:
      return nir_ixor(b, nir_ior(b, src, dst), bitmask);
   case PIPE_LOGICOP_AND_INVERTED:
      return nir_iand(b, nir_ixor(b, src, bitmask), dst);
   case PIPE_LOGICOP_COPY_INVERTED:
      return nir_ixor(b, src, bitmask);
   case PIPE_LOGICOP_AND_REVERSE:
      return nir_iand(b, src, nir_ixor(b, dst, bitmask));
   case PIPE_LOGICOP_INVERT:
      return nir_ixor(b, dst, bitmask);
   case PIPE_LOGICOP_XOR:
      return nir_ixor(b, src, dst);
   case PIPE_LOGICOP_NAND:
      return nir_ixor(b, nir_iand(b, src, dst), bitmask);
   case PIPE_LOGICOP_AND:
      return nir_iand(b, src, dst);
   case PIPE_LOGICOP_EQUIV:
      return nir_ixor(b, nir_ixor(b, src, dst), bitmask);
   case PIPE_LOGICOP_NOOP:
      unreachable("optimized out");
   case PIPE_LOGICOP_OR_INVERTED:
      return nir_ior(b, nir_ixor(b, src, bitmask), dst);
   case PIPE_LOGICOP_COPY:
      return src;
   case PIPE_LOGICOP_OR_REVERSE:
      return nir_ior(b, src, nir_ixor(b, dst, bitmask));
   case PIPE_LOGICOP_OR:
      return nir_ior(b, src, dst);
   case PIPE_LOGICOP_SET:
      return nir_imm_ivec4(b, ~0, ~0, ~0, ~0);
   }

   unreachable("Invalid logciop function");
}

static nir_def *
nir_blend_logicop(
   nir_builder *b,
   const nir_lower_blend_options *options,
   unsigned rt,
   nir_def *src, nir_def *dst)
{
   unsigned bit_size = src->bit_size;

   enum pipe_format format = options->format[rt];
   const struct util_format_description *format_desc =
      util_format_description(format);

   /* From section 17.3.9 ("Logical Operation") of the OpenGL 4.6 core spec:
    *
    *    Logical operation has no effect on a floating-point destination color
    *    buffer, or when FRAMEBUFFER_SRGB is enabled and the value of
    *    FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING for the framebuffer attachment
    *    corresponding to the destination buffer is SRGB (see section 9.2.3).
    *    However, if logical operation is enabled, blending is still disabled.
    */
   if (util_format_is_float(format) || util_format_is_srgb(format))
      return src;

   if (bit_size != 32) {
      src = nir_f2f32(b, src);
      dst = nir_f2f32(b, dst);
   }

   assert(src->num_components <= 4);
   assert(dst->num_components <= 4);

   unsigned bits[4];
   for (int i = 0; i < 4; ++i)
      bits[i] = format_desc->channel[i].size;

   if (util_format_is_unorm(format)) {
      src = nir_format_float_to_unorm(b, src, bits);
      dst = nir_format_float_to_unorm(b, dst, bits);
   } else if (util_format_is_snorm(format)) {
      src = nir_format_float_to_snorm(b, src, bits);
      dst = nir_format_float_to_snorm(b, dst, bits);
   } else {
      assert(util_format_is_pure_integer(format));
   }

   nir_const_value mask[4];
   for (int i = 0; i < 4; ++i)
      mask[i] = nir_const_value_for_uint(BITFIELD_MASK(bits[i]), 32);

   nir_def *out = nir_logicop_func(b, options->logicop_func, src, dst,
                                   nir_build_imm(b, 4, 32, mask));

   if (util_format_is_unorm(format)) {
      out = nir_format_unorm_to_float(b, out, bits);
   } else if (util_format_is_snorm(format)) {
      /* Sign extend before converting so the i2f in snorm_to_float works */
      out = nir_format_sign_extend_ivec(b, out, bits);
      out = nir_format_snorm_to_float(b, out, bits);
   } else {
      assert(util_format_is_pure_integer(format));
   }

   if (bit_size == 16)
      out = nir_f2f16(b, out);

   return out;
}

static bool
channel_exists(const struct util_format_description *desc, unsigned i)
{
   return (i < desc->nr_channels) &&
          desc->channel[i].type != UTIL_FORMAT_TYPE_VOID;
}

/* Given a blend state, the source color, and the destination color,
 * return the blended color
 */

static nir_def *
nir_blend(
   nir_builder *b,
   const nir_lower_blend_options *options,
   unsigned rt,
   nir_def *src, nir_def *src1, nir_def *dst)
{
   /* Don't crash if src1 isn't written. It doesn't matter what dual colour we
    * blend with in that case, as long as we don't dereference NULL.
    */
   if (!src1)
      src1 = nir_imm_zero(b, 4, src->bit_size);

   /* Grab the blend constant ahead of time */
   nir_def *bconst;
   if (options->scalar_blend_const) {
      bconst = nir_vec4(b,
                        nir_load_blend_const_color_r_float(b),
                        nir_load_blend_const_color_g_float(b),
                        nir_load_blend_const_color_b_float(b),
                        nir_load_blend_const_color_a_float(b));
   } else {
      bconst = nir_load_blend_const_color_rgba(b);
   }

   if (src->bit_size == 16) {
      bconst = nir_f2f16(b, bconst);
      src1 = nir_f2f16(b, src1);
   }

   /* Fixed-point framebuffers require their inputs clamped. */
   enum pipe_format format = options->format[rt];

   /* From section 17.3.6 "Blending" of the OpenGL 4.5 spec:
    *
    *     If the color buffer is fixed-point, the components of the source and
    *     destination values and blend factors are each clamped to [0, 1] or
    *     [-1, 1] respectively for an unsigned normalized or signed normalized
    *     color buffer prior to evaluating the blend equation. If the color
    *     buffer is floating-point, no clamping occurs.
    *
    * Blend factors are clamped at the time of their use to ensure we properly
    * clamp negative constant colours with signed normalized formats and
    * ONE_MINUS_CONSTANT_* factors. Notice that -1 is in [-1, 1] but 1 - (-1) =
    * 2 is not in [-1, 1] and should be clamped to 1.
    */
   src = nir_fsat_to_format(b, src, format);

   if (src1)
      src1 = nir_fsat_to_format(b, src1, format);

   /* DST_ALPHA reads back 1.0 if there is no alpha channel */
   const struct util_format_description *desc =
      util_format_description(format);

   nir_def *zero = nir_imm_floatN_t(b, 0.0, dst->bit_size);
   nir_def *one = nir_imm_floatN_t(b, 1.0, dst->bit_size);

   dst = nir_vec4(b,
                  channel_exists(desc, 0) ? nir_channel(b, dst, 0) : zero,
                  channel_exists(desc, 1) ? nir_channel(b, dst, 1) : zero,
                  channel_exists(desc, 2) ? nir_channel(b, dst, 2) : zero,
                  channel_exists(desc, 3) ? nir_channel(b, dst, 3) : one);

   /* We blend per channel and recombine later */
   nir_def *channels[4];

   for (unsigned c = 0; c < 4; ++c) {
      /* Decide properties based on channel */
      nir_lower_blend_channel chan =
         (c < 3) ? options->rt[rt].rgb : options->rt[rt].alpha;

      nir_def *psrc = nir_channel(b, src, c);
      nir_def *pdst = nir_channel(b, dst, c);

      if (nir_blend_factored(chan.func)) {
         psrc = nir_blend_factor(
            b, psrc,
            src, src1, dst, bconst, c,
            chan.src_factor, format);

         pdst = nir_blend_factor(
            b, pdst,
            src, src1, dst, bconst, c,
            chan.dst_factor, format);
      }

      channels[c] = nir_blend_func(b, chan.func, psrc, pdst);
   }

   return nir_vec(b, channels, 4);
}

static int
color_index_for_location(unsigned location)
{
   assert(location != FRAG_RESULT_COLOR &&
          "gl_FragColor must be lowered before nir_lower_blend");

   if (location < FRAG_RESULT_DATA0)
      return -1;
   else
      return location - FRAG_RESULT_DATA0;
}

/*
 * Test if the blending options for a given channel encode the "replace" blend
 * mode: dest = source. In this case, blending may be specially optimized.
 */
static bool
nir_blend_replace_channel(const nir_lower_blend_channel *c)
{
   return (c->func == PIPE_BLEND_ADD) &&
          (c->src_factor == PIPE_BLENDFACTOR_ONE) &&
          (c->dst_factor == PIPE_BLENDFACTOR_ZERO);
}

static bool
nir_blend_replace_rt(const nir_lower_blend_rt *rt)
{
   return nir_blend_replace_channel(&rt->rgb) &&
          nir_blend_replace_channel(&rt->alpha);
}

static bool
nir_lower_blend_instr(nir_builder *b, nir_intrinsic_instr *store, void *data)
{
   struct ctx *ctx = data;
   const nir_lower_blend_options *options = ctx->options;
   if (store->intrinsic != nir_intrinsic_store_output)
      return false;

   nir_io_semantics sem = nir_intrinsic_io_semantics(store);
   int rt = color_index_for_location(sem.location);

   /* No blend lowering requested on this RT */
   if (rt < 0 || options->format[rt] == PIPE_FORMAT_NONE)
      return false;

   /* Only process stores once. Pass flags are cleared by consume_dual_stores */
   if (store->instr.pass_flags)
      return false;

   store->instr.pass_flags = 1;

   /* Store are sunk to the bottom of the block to ensure that the dual
    * source colour is already written.
    */
   b->cursor = nir_after_block(store->instr.block);

   /* Don't bother copying the destination to the source for disabled RTs */
   if (options->rt[rt].colormask == 0 ||
       (options->logicop_enable && options->logicop_func == PIPE_LOGICOP_NOOP)) {

      nir_instr_remove(&store->instr);
      return true;
   }

   /* Grab the input color.  We always want 4 channels during blend.  Dead
    * code will clean up any channels we don't need.
    */
   nir_def *src = nir_pad_vector(b, store->src[0].ssa, 4);

   assert(nir_src_as_uint(store->src[1]) == 0 && "store_output invariant");

   /* Grab the previous fragment color if we need it */
   nir_def *dst;

   if (channel_uses_dest(options->rt[rt].rgb) ||
       channel_uses_dest(options->rt[rt].alpha) ||
       options->logicop_enable ||
       options->rt[rt].colormask != BITFIELD_MASK(4)) {

      b->shader->info.outputs_read |= BITFIELD64_BIT(sem.location);
      b->shader->info.fs.uses_fbfetch_output = true;
      b->shader->info.fs.uses_sample_shading = true;
      sem.fb_fetch_output = true;

      dst = nir_load_output(b, 4, nir_src_bit_size(store->src[0]),
                            nir_imm_int(b, 0),
                            .dest_type = nir_intrinsic_src_type(store),
                            .io_semantics = sem);
   } else {
      dst = nir_undef(b, 4, nir_src_bit_size(store->src[0]));
   }

   /* Blend the two colors per the passed options. We only call nir_blend if
    * blending is enabled with a blend mode other than replace (independent of
    * the color mask). That avoids unnecessary fsat instructions in the common
    * case where blending is disabled at an API level, but the driver calls
    * nir_blend (possibly for color masking).
    */
   nir_def *blended = src;

   if (options->logicop_enable) {
      blended = nir_blend_logicop(b, options, rt, src, dst);
   } else if (!util_format_is_pure_integer(options->format[rt]) &&
              !nir_blend_replace_rt(&options->rt[rt])) {
      assert(!util_format_is_scaled(options->format[rt]));
      blended = nir_blend(b, options, rt, src, ctx->src1[rt], dst);
   }

   /* Apply a colormask if necessary */
   if (options->rt[rt].colormask != BITFIELD_MASK(4))
      blended = nir_color_mask(b, options->rt[rt].colormask, blended, dst);

   const unsigned num_components =
      util_format_get_nr_components(options->format[rt]);

   /* Shave off any components we don't want to store */
   blended = nir_trim_vector(b, blended, num_components);

   /* Grow or shrink the store destination as needed */
   store->num_components = num_components;
   nir_intrinsic_set_write_mask(store, nir_intrinsic_write_mask(store) &
                                          nir_component_mask(num_components));

   /* Write out the final color instead of the input */
   nir_src_rewrite(&store->src[0], blended);

   /* Sink to bottom */
   nir_instr_remove(&store->instr);
   nir_builder_instr_insert(b, &store->instr);
   return true;
}

/*
 * Dual-source colours are only for blending, so when nir_lower_blend is used,
 * the dual source store_output is for us (only). Remove dual stores so the
 * backend doesn't have to deal with them, collecting the sources for blending.
 */
static bool
consume_dual_stores(nir_builder *b, nir_intrinsic_instr *store, void *data)
{
   nir_def **outputs = data;
   if (store->intrinsic != nir_intrinsic_store_output)
      return false;

   /* While we're here, clear the pass flags for store_outputs, since we'll set
    * them later.
    */
   store->instr.pass_flags = 0;

   nir_io_semantics sem = nir_intrinsic_io_semantics(store);
   if (sem.dual_source_blend_index == 0)
      return false;

   int rt = color_index_for_location(sem.location);
   assert(rt >= 0 && rt < 8 && "bounds for dual-source blending");

   outputs[rt] = store->src[0].ssa;
   nir_instr_remove(&store->instr);
   return true;
}

/** Lower blending to framebuffer fetch and some math
 *
 * This pass requires that shader I/O is lowered to explicit load/store
 * instructions using nir_lower_io.
 */
void
nir_lower_blend(nir_shader *shader, const nir_lower_blend_options *options)
{
   assert(shader->info.stage == MESA_SHADER_FRAGMENT);

   struct ctx ctx = { .options = options };
   nir_shader_intrinsics_pass(shader, consume_dual_stores,
                              nir_metadata_block_index |
                                 nir_metadata_dominance,
                              ctx.src1);

   nir_shader_intrinsics_pass(shader, nir_lower_blend_instr,
                              nir_metadata_block_index |
                                 nir_metadata_dominance,
                              &ctx);
}
