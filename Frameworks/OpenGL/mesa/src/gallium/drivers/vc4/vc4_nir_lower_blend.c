/*
 * Copyright © 2015 Broadcom
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
 * Implements most of the fixed function fragment pipeline in shader code.
 *
 * VC4 doesn't have any hardware support for blending, alpha test, logic ops,
 * or color mask.  Instead, you read the current contents of the destination
 * from the tile buffer after having waited for the scoreboard (which is
 * handled by vc4_qpu_emit.c), then do math using your output color and that
 * destination value, and update the output color appropriately.
 *
 * Once this pass is done, the color write will either have one component (for
 * single sample) with packed argb8888, or 4 components with the per-sample
 * argb8888 result.
 */

/**
 * Lowers fixed-function blending to a load of the destination color and a
 * series of ALU operations before the store of the output.
 */
#include "util/format/u_format.h"
#include "vc4_qir.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_format_convert.h"
#include "vc4_context.h"

static bool
blend_depends_on_dst_color(struct vc4_compile *c)
{
        return (c->fs_key->blend.blend_enable ||
                c->fs_key->blend.colormask != PIPE_MASK_RGBA ||
                c->fs_key->logicop_func != PIPE_LOGICOP_COPY);
}

/** Emits a load of the previous fragment color from the tile buffer. */
static nir_def *
vc4_nir_get_dst_color(nir_builder *b, int sample)
{
        return nir_load_input(b, 1, 32, nir_imm_int(b, 0),
                              .base = VC4_NIR_TLB_COLOR_READ_INPUT + sample);
}

static nir_def *
vc4_blend_channel_f(nir_builder *b,
                    nir_def **src,
                    nir_def **dst,
                    unsigned factor,
                    int channel)
{
        switch(factor) {
        case PIPE_BLENDFACTOR_ONE:
                return nir_imm_float(b, 1.0);
        case PIPE_BLENDFACTOR_SRC_COLOR:
                return src[channel];
        case PIPE_BLENDFACTOR_SRC_ALPHA:
                return src[3];
        case PIPE_BLENDFACTOR_DST_ALPHA:
                return dst[3];
        case PIPE_BLENDFACTOR_DST_COLOR:
                return dst[channel];
        case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
                if (channel != 3) {
                        return nir_fmin(b,
                                        src[3],
                                        nir_fsub_imm(b, 1.0,
                                                     dst[3]));
                } else {
                        return nir_imm_float(b, 1.0);
                }
        case PIPE_BLENDFACTOR_CONST_COLOR:
                return nir_load_system_value(b,
                                             nir_intrinsic_load_blend_const_color_r_float +
                                             channel,
                                             0, 1, 32);
        case PIPE_BLENDFACTOR_CONST_ALPHA:
                return nir_load_blend_const_color_a_float(b);
        case PIPE_BLENDFACTOR_ZERO:
                return nir_imm_float(b, 0.0);
        case PIPE_BLENDFACTOR_INV_SRC_COLOR:
                return nir_fsub_imm(b, 1.0, src[channel]);
        case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
                return nir_fsub_imm(b, 1.0, src[3]);
        case PIPE_BLENDFACTOR_INV_DST_ALPHA:
                return nir_fsub_imm(b, 1.0, dst[3]);
        case PIPE_BLENDFACTOR_INV_DST_COLOR:
                return nir_fsub_imm(b, 1.0, dst[channel]);
        case PIPE_BLENDFACTOR_INV_CONST_COLOR:
                return nir_fsub_imm(b, 1.0,
                                    nir_load_system_value(b,
                                                          nir_intrinsic_load_blend_const_color_r_float +
                                                          channel,
                                                          0, 1, 32));
        case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
                return nir_fsub_imm(b, 1.0,
                                    nir_load_blend_const_color_a_float(b));

        default:
        case PIPE_BLENDFACTOR_SRC1_COLOR:
        case PIPE_BLENDFACTOR_SRC1_ALPHA:
        case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
        case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
                /* Unsupported. */
                fprintf(stderr, "Unknown blend factor %d\n", factor);
                return nir_imm_float(b, 1.0);
        }
}

static nir_def *
vc4_nir_set_packed_chan(nir_builder *b, nir_def *src0, nir_def *src1,
                        int chan)
{
        unsigned chan_mask = 0xff << (chan * 8);
        return nir_ior(b,
                       nir_iand_imm(b, src0, ~chan_mask),
                       nir_iand_imm(b, src1, chan_mask));
}

static nir_def *
vc4_blend_channel_i(nir_builder *b,
                    nir_def *src,
                    nir_def *dst,
                    nir_def *src_a,
                    nir_def *dst_a,
                    unsigned factor,
                    int a_chan)
{
        switch (factor) {
        case PIPE_BLENDFACTOR_ONE:
                return nir_imm_int(b, ~0);
        case PIPE_BLENDFACTOR_SRC_COLOR:
                return src;
        case PIPE_BLENDFACTOR_SRC_ALPHA:
                return src_a;
        case PIPE_BLENDFACTOR_DST_ALPHA:
                return dst_a;
        case PIPE_BLENDFACTOR_DST_COLOR:
                return dst;
        case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
                return vc4_nir_set_packed_chan(b,
                                               nir_umin_4x8_vc4(b,
                                                            src_a,
                                                            nir_inot(b, dst_a)),
                                               nir_imm_int(b, ~0),
                                               a_chan);
        case PIPE_BLENDFACTOR_CONST_COLOR:
                return nir_load_blend_const_color_rgba8888_unorm(b);
        case PIPE_BLENDFACTOR_CONST_ALPHA:
                return nir_load_blend_const_color_aaaa8888_unorm(b);
        case PIPE_BLENDFACTOR_ZERO:
                return nir_imm_int(b, 0);
        case PIPE_BLENDFACTOR_INV_SRC_COLOR:
                return nir_inot(b, src);
        case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
                return nir_inot(b, src_a);
        case PIPE_BLENDFACTOR_INV_DST_ALPHA:
                return nir_inot(b, dst_a);
        case PIPE_BLENDFACTOR_INV_DST_COLOR:
                return nir_inot(b, dst);
        case PIPE_BLENDFACTOR_INV_CONST_COLOR:
                return nir_inot(b,
                                nir_load_blend_const_color_rgba8888_unorm(b));
        case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
                return nir_inot(b,
                                nir_load_blend_const_color_aaaa8888_unorm(b));

        default:
        case PIPE_BLENDFACTOR_SRC1_COLOR:
        case PIPE_BLENDFACTOR_SRC1_ALPHA:
        case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
        case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
                /* Unsupported. */
                fprintf(stderr, "Unknown blend factor %d\n", factor);
                return nir_imm_int(b, ~0);
        }
}

static nir_def *
vc4_blend_func_f(nir_builder *b, nir_def *src, nir_def *dst,
                 unsigned func)
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

        default:
                /* Unsupported. */
                fprintf(stderr, "Unknown blend func %d\n", func);
                return src;

        }
}

static nir_def *
vc4_blend_func_i(nir_builder *b, nir_def *src, nir_def *dst,
                 unsigned func)
{
        switch (func) {
        case PIPE_BLEND_ADD:
                return nir_usadd_4x8_vc4(b, src, dst);
        case PIPE_BLEND_SUBTRACT:
                return nir_ussub_4x8_vc4(b, src, dst);
        case PIPE_BLEND_REVERSE_SUBTRACT:
                return nir_ussub_4x8_vc4(b, dst, src);
        case PIPE_BLEND_MIN:
                return nir_umin_4x8_vc4(b, src, dst);
        case PIPE_BLEND_MAX:
                return nir_umax_4x8_vc4(b, src, dst);

        default:
                /* Unsupported. */
                fprintf(stderr, "Unknown blend func %d\n", func);
                return src;

        }
}

static void
vc4_do_blending_f(struct vc4_compile *c, nir_builder *b, nir_def **result,
                  nir_def **src_color, nir_def **dst_color)
{
        struct pipe_rt_blend_state *blend = &c->fs_key->blend;

        if (!blend->blend_enable) {
                for (int i = 0; i < 4; i++)
                        result[i] = src_color[i];
                return;
        }

        /* Clamp the src color to [0, 1].  Dest is already clamped. */
        for (int i = 0; i < 4; i++)
                src_color[i] = nir_fsat(b, src_color[i]);

        nir_def *src_blend[4], *dst_blend[4];
        for (int i = 0; i < 4; i++) {
                int src_factor = ((i != 3) ? blend->rgb_src_factor :
                                  blend->alpha_src_factor);
                int dst_factor = ((i != 3) ? blend->rgb_dst_factor :
                                  blend->alpha_dst_factor);
                src_blend[i] = nir_fmul(b, src_color[i],
                                        vc4_blend_channel_f(b,
                                                            src_color, dst_color,
                                                            src_factor, i));
                dst_blend[i] = nir_fmul(b, dst_color[i],
                                        vc4_blend_channel_f(b,
                                                            src_color, dst_color,
                                                            dst_factor, i));
        }

        for (int i = 0; i < 4; i++) {
                result[i] = vc4_blend_func_f(b, src_blend[i], dst_blend[i],
                                             ((i != 3) ? blend->rgb_func :
                                              blend->alpha_func));
        }
}

static nir_def *
vc4_nir_splat(nir_builder *b, nir_def *src)
{
        nir_def *or1 = nir_ior(b, src, nir_ishl_imm(b, src, 8));
        return nir_ior(b, or1, nir_ishl_imm(b, or1, 16));
}

static nir_def *
vc4_do_blending_i(struct vc4_compile *c, nir_builder *b,
                  nir_def *src_color, nir_def *dst_color,
                  nir_def *src_float_a)
{
        struct pipe_rt_blend_state *blend = &c->fs_key->blend;

        if (!blend->blend_enable)
                return src_color;

        enum pipe_format color_format = c->fs_key->color_format;
        const uint8_t *format_swiz = vc4_get_format_swizzle(color_format);
        nir_def *src_a = nir_pack_unorm_4x8(b, src_float_a);
        nir_def *dst_a;
        int alpha_chan;
        for (alpha_chan = 0; alpha_chan < 4; alpha_chan++) {
                if (format_swiz[alpha_chan] == 3)
                        break;
        }
        if (alpha_chan != 4) {
                dst_a = vc4_nir_splat(b, nir_iand_imm(b, nir_ushr_imm(b, dst_color,
                                                                     alpha_chan * 8),
                                                      0xff));
        } else {
                dst_a = nir_imm_int(b, ~0);
        }

        nir_def *src_factor = vc4_blend_channel_i(b,
                                                      src_color, dst_color,
                                                      src_a, dst_a,
                                                      blend->rgb_src_factor,
                                                      alpha_chan);
        nir_def *dst_factor = vc4_blend_channel_i(b,
                                                      src_color, dst_color,
                                                      src_a, dst_a,
                                                      blend->rgb_dst_factor,
                                                      alpha_chan);

        if (alpha_chan != 4 &&
            blend->alpha_src_factor != blend->rgb_src_factor) {
                nir_def *src_alpha_factor =
                        vc4_blend_channel_i(b,
                                            src_color, dst_color,
                                            src_a, dst_a,
                                            blend->alpha_src_factor,
                                            alpha_chan);
                src_factor = vc4_nir_set_packed_chan(b, src_factor,
                                                     src_alpha_factor,
                                                     alpha_chan);
        }
        if (alpha_chan != 4 &&
            blend->alpha_dst_factor != blend->rgb_dst_factor) {
                nir_def *dst_alpha_factor =
                        vc4_blend_channel_i(b,
                                            src_color, dst_color,
                                            src_a, dst_a,
                                            blend->alpha_dst_factor,
                                            alpha_chan);
                dst_factor = vc4_nir_set_packed_chan(b, dst_factor,
                                                     dst_alpha_factor,
                                                     alpha_chan);
        }
        nir_def *src_blend = nir_umul_unorm_4x8_vc4(b, src_color, src_factor);
        nir_def *dst_blend = nir_umul_unorm_4x8_vc4(b, dst_color, dst_factor);

        nir_def *result =
                vc4_blend_func_i(b, src_blend, dst_blend, blend->rgb_func);
        if (alpha_chan != 4 && blend->alpha_func != blend->rgb_func) {
                nir_def *result_a = vc4_blend_func_i(b,
                                                         src_blend,
                                                         dst_blend,
                                                         blend->alpha_func);
                result = vc4_nir_set_packed_chan(b, result, result_a,
                                                 alpha_chan);
        }
        return result;
}

static nir_def *
vc4_logicop(nir_builder *b, int logicop_func,
            nir_def *src, nir_def *dst)
{
        switch (logicop_func) {
        case PIPE_LOGICOP_CLEAR:
                return nir_imm_int(b, 0);
        case PIPE_LOGICOP_NOR:
                return nir_inot(b, nir_ior(b, src, dst));
        case PIPE_LOGICOP_AND_INVERTED:
                return nir_iand(b, nir_inot(b, src), dst);
        case PIPE_LOGICOP_COPY_INVERTED:
                return nir_inot(b, src);
        case PIPE_LOGICOP_AND_REVERSE:
                return nir_iand(b, src, nir_inot(b, dst));
        case PIPE_LOGICOP_INVERT:
                return nir_inot(b, dst);
        case PIPE_LOGICOP_XOR:
                return nir_ixor(b, src, dst);
        case PIPE_LOGICOP_NAND:
                return nir_inot(b, nir_iand(b, src, dst));
        case PIPE_LOGICOP_AND:
                return nir_iand(b, src, dst);
        case PIPE_LOGICOP_EQUIV:
                return nir_inot(b, nir_ixor(b, src, dst));
        case PIPE_LOGICOP_NOOP:
                return dst;
        case PIPE_LOGICOP_OR_INVERTED:
                return nir_ior(b, nir_inot(b, src), dst);
        case PIPE_LOGICOP_OR_REVERSE:
                return nir_ior(b, src, nir_inot(b, dst));
        case PIPE_LOGICOP_OR:
                return nir_ior(b, src, dst);
        case PIPE_LOGICOP_SET:
                return nir_imm_int(b, ~0);
        default:
                fprintf(stderr, "Unknown logic op %d\n", logicop_func);
                FALLTHROUGH;
        case PIPE_LOGICOP_COPY:
                return src;
        }
}

static nir_def *
vc4_nir_swizzle_and_pack(struct vc4_compile *c, nir_builder *b,
                         nir_def **colors)
{
        enum pipe_format color_format = c->fs_key->color_format;
        const uint8_t *format_swiz = vc4_get_format_swizzle(color_format);

        nir_def *swizzled[4];
        for (int i = 0; i < 4; i++) {
                swizzled[i] = vc4_nir_get_swizzled_channel(b, colors,
                                                           format_swiz[i]);
        }

        return nir_pack_unorm_4x8(b,
                                  nir_vec4(b,
                                           swizzled[0], swizzled[1],
                                           swizzled[2], swizzled[3]));

}

static nir_def *
vc4_nir_blend_pipeline(struct vc4_compile *c, nir_builder *b, nir_def *src,
                       int sample)
{
        enum pipe_format color_format = c->fs_key->color_format;
        const uint8_t *format_swiz = vc4_get_format_swizzle(color_format);
        bool srgb = util_format_is_srgb(color_format);

        /* Pull out the float src/dst color components. */
        nir_def *packed_dst_color = vc4_nir_get_dst_color(b, sample);
        nir_def *dst_vec4 = nir_unpack_unorm_4x8(b, packed_dst_color);
        nir_def *src_color[4], *unpacked_dst_color[4];
        for (unsigned i = 0; i < 4; i++) {
                src_color[i] = nir_channel(b, src, i);
                unpacked_dst_color[i] = nir_channel(b, dst_vec4, i);
        }

        if (c->fs_key->sample_alpha_to_one && c->fs_key->msaa)
                src_color[3] = nir_imm_float(b, 1.0);

        nir_def *packed_color;
        if (srgb) {
                /* Unswizzle the destination color. */
                nir_def *dst_color[4];
                for (unsigned i = 0; i < 4; i++) {
                        dst_color[i] = vc4_nir_get_swizzled_channel(b,
                                                                    unpacked_dst_color,
                                                                    format_swiz[i]);
                }

                /* Turn dst color to linear. */
                for (int i = 0; i < 3; i++)
                        dst_color[i] = nir_format_srgb_to_linear(b, dst_color[i]);

                nir_def *blend_color[4];
                vc4_do_blending_f(c, b, blend_color, src_color, dst_color);

                /* sRGB encode the output color */
                for (int i = 0; i < 3; i++)
                        blend_color[i] = nir_format_linear_to_srgb(b, blend_color[i]);

                packed_color = vc4_nir_swizzle_and_pack(c, b, blend_color);
        } else {
                nir_def *packed_src_color =
                        vc4_nir_swizzle_and_pack(c, b, src_color);

                packed_color =
                        vc4_do_blending_i(c, b,
                                          packed_src_color, packed_dst_color,
                                          src_color[3]);
        }

        packed_color = vc4_logicop(b, c->fs_key->logicop_func,
                                   packed_color, packed_dst_color);

        /* If the bit isn't set in the color mask, then just return the
         * original dst color, instead.
         */
        uint32_t colormask = 0xffffffff;
        for (int i = 0; i < 4; i++) {
                if (format_swiz[i] < 4 &&
                    !(c->fs_key->blend.colormask & (1 << format_swiz[i]))) {
                        colormask &= ~(0xff << (i * 8));
                }
        }

        return nir_ior(b,
                       nir_iand_imm(b, packed_color, colormask),
                       nir_iand_imm(b, packed_dst_color, ~colormask));
}

static void
vc4_nir_store_sample_mask(struct vc4_compile *c, nir_builder *b,
                          nir_def *val)
{
        nir_variable *sample_mask = nir_variable_create(c->s, nir_var_shader_out,
                                                        glsl_uint_type(),
                                                        "sample_mask");
        sample_mask->data.driver_location = c->s->num_outputs++;
        sample_mask->data.location = FRAG_RESULT_SAMPLE_MASK;

        nir_store_output(b, val, nir_imm_int(b, 0),
                         .base = sample_mask->data.driver_location);
}

static void
vc4_nir_lower_blend_instr(struct vc4_compile *c, nir_builder *b,
                          nir_intrinsic_instr *intr)
{
        nir_def *frag_color = intr->src[0].ssa;

        if (c->fs_key->sample_alpha_to_coverage) {
                nir_def *a = nir_channel(b, frag_color, 3);

                /* XXX: We should do a nice dither based on the fragment
                 * coordinate, instead.
                 */
                nir_def *num_bits = nir_f2i32(b, nir_fmul_imm(b, a, VC4_MAX_SAMPLES));
                nir_def *bitmask = nir_iadd_imm(b,
                                                    nir_ishl(b,
                                                             nir_imm_int(b, 1),
                                                             num_bits),
                                                    -1);
                vc4_nir_store_sample_mask(c, b, bitmask);
        }

        /* The TLB color read returns each sample in turn, so if our blending
         * depends on the destination color, we're going to have to run the
         * blending function separately for each destination sample value, and
         * then output the per-sample color using TLB_COLOR_MS.
         */
        nir_def *blend_output;
        if (c->fs_key->msaa && blend_depends_on_dst_color(c)) {
                c->msaa_per_sample_output = true;

                nir_def *samples[4];
                for (int i = 0; i < VC4_MAX_SAMPLES; i++)
                        samples[i] = vc4_nir_blend_pipeline(c, b, frag_color, i);
                blend_output = nir_vec4(b,
                                        samples[0], samples[1],
                                        samples[2], samples[3]);
        } else {
                blend_output = vc4_nir_blend_pipeline(c, b, frag_color, 0);
        }

        nir_src_rewrite(&intr->src[0], blend_output);
        if (intr->num_components != blend_output->num_components) {
                unsigned component_mask = BITFIELD_MASK(blend_output->num_components);
                nir_intrinsic_set_write_mask(intr, component_mask);
                intr->num_components = blend_output->num_components;
        }
}

static bool
vc4_nir_lower_blend_block(nir_block *block, struct vc4_compile *c)
{
        nir_foreach_instr_safe(instr, block) {
                if (instr->type != nir_instr_type_intrinsic)
                        continue;
                nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
                if (intr->intrinsic != nir_intrinsic_store_output)
                        continue;

                nir_variable *output_var = NULL;
                nir_foreach_shader_out_variable(var, c->s) {
                        if (var->data.driver_location ==
                            nir_intrinsic_base(intr)) {
                                output_var = var;
                                break;
                        }
                }
                assert(output_var);

                if (output_var->data.location != FRAG_RESULT_COLOR &&
                    output_var->data.location != FRAG_RESULT_DATA0) {
                        continue;
                }

                nir_builder b = nir_builder_at(nir_before_instr(&intr->instr));
                vc4_nir_lower_blend_instr(c, &b, intr);
        }
        return true;
}

void
vc4_nir_lower_blend(nir_shader *s, struct vc4_compile *c)
{
        nir_foreach_function_impl(impl, s) {
                nir_foreach_block(block, impl) {
                        vc4_nir_lower_blend_block(block, c);
                }

                nir_metadata_preserve(impl,
                                      nir_metadata_block_index |
                                      nir_metadata_dominance);
        }

        /* If we didn't do alpha-to-coverage on the output color, we still
         * need to pass glSampleMask() through.
         */
        if (c->fs_key->sample_coverage && !c->fs_key->sample_alpha_to_coverage) {
                nir_function_impl *impl = nir_shader_get_entrypoint(s);
                nir_builder b = nir_builder_at(nir_after_impl(impl));

                vc4_nir_store_sample_mask(c, &b, nir_load_sample_mask_in(&b));
        }
}
