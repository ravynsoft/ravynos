/*
 * Copyright © 2019 Broadcom
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
 * Implements lowering for logical operations.
 *
 * V3D doesn't have any hardware support for logic ops.  Instead, you read the
 * current contents of the destination from the tile buffer, then do math using
 * your output color and that destination value, and update the output color
 * appropriately.
 */

#include "util/format/u_format.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_format_convert.h"
#include "v3d_compiler.h"


typedef nir_def *(*nir_pack_func)(nir_builder *b, nir_def *c);
typedef nir_def *(*nir_unpack_func)(nir_builder *b, nir_def *c);

static bool
logicop_depends_on_dst_color(int logicop_func)
{
        switch (logicop_func) {
        case PIPE_LOGICOP_SET:
        case PIPE_LOGICOP_CLEAR:
        case PIPE_LOGICOP_COPY:
        case PIPE_LOGICOP_COPY_INVERTED:
                return false;
        default:
                return true;
        }
}

static nir_def *
v3d_logicop(nir_builder *b, int logicop_func,
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
v3d_nir_get_swizzled_channel(nir_builder *b, nir_def **srcs, int swiz)
{
        switch (swiz) {
        default:
        case PIPE_SWIZZLE_NONE:
                fprintf(stderr, "warning: unknown swizzle\n");
                FALLTHROUGH;
        case PIPE_SWIZZLE_0:
                return nir_imm_float(b, 0.0);
        case PIPE_SWIZZLE_1:
                return nir_imm_float(b, 1.0);
        case PIPE_SWIZZLE_X:
        case PIPE_SWIZZLE_Y:
        case PIPE_SWIZZLE_Z:
        case PIPE_SWIZZLE_W:
                return srcs[swiz];
        }
}

static nir_def *
v3d_nir_swizzle_and_pack(nir_builder *b, nir_def **chans,
                         const uint8_t *swiz, nir_pack_func pack_func)
{
        nir_def *c[4];
        for (int i = 0; i < 4; i++)
                c[i] = v3d_nir_get_swizzled_channel(b, chans, swiz[i]);

        return pack_func(b, nir_vec4(b, c[0], c[1], c[2], c[3]));
}

static nir_def *
v3d_nir_unpack_and_swizzle(nir_builder *b, nir_def *packed,
                           const uint8_t *swiz, nir_unpack_func unpack_func)
{
        nir_def *unpacked = unpack_func(b, packed);

        nir_def *unpacked_chans[4];
        for (int i = 0; i < 4; i++)
                unpacked_chans[i] = nir_channel(b, unpacked, i);

        nir_def *c[4];
        for (int i = 0; i < 4; i++)
                c[i] = v3d_nir_get_swizzled_channel(b, unpacked_chans, swiz[i]);

        return nir_vec4(b, c[0], c[1], c[2], c[3]);
}

static nir_def *
pack_unorm_rgb10a2(nir_builder *b, nir_def *c)
{
        static const unsigned bits[4] = { 10, 10, 10, 2 };
        nir_def *unorm = nir_format_float_to_unorm(b, c, bits);

        nir_def *chans[4];
        for (int i = 0; i < 4; i++)
                chans[i] = nir_channel(b, unorm, i);

        nir_def *result = nir_mov(b, chans[0]);
        int offset = bits[0];
        for (int i = 1; i < 4; i++) {
                nir_def *shifted_chan =
                        nir_ishl_imm(b, chans[i], offset);
                result = nir_ior(b, result, shifted_chan);
                offset += bits[i];
        }
        return result;
}

static nir_def *
unpack_unorm_rgb10a2(nir_builder *b, nir_def *c)
{
        static const unsigned bits[4] = { 10, 10, 10, 2 };
        const unsigned masks[4] = { BITFIELD_MASK(bits[0]),
                                    BITFIELD_MASK(bits[1]),
                                    BITFIELD_MASK(bits[2]),
                                    BITFIELD_MASK(bits[3]) };

        nir_def *chans[4];
        for (int i = 0; i < 4; i++) {
                nir_def *unorm = nir_iand_imm(b, c, masks[i]);
                chans[i] = nir_format_unorm_to_float(b, unorm, &bits[i]);
                c = nir_ushr_imm(b, c, bits[i]);
        }

        return nir_vec4(b, chans[0], chans[1], chans[2], chans[3]);
}

static const uint8_t *
v3d_get_format_swizzle_for_rt(struct v3d_compile *c, int rt)
{
        static const uint8_t ident[4] = { 0, 1, 2, 3 };

        /* We will automatically swap R and B channels for BGRA formats
         * on tile loads and stores (see 'swap_rb' field in v3d_resource) so
         * we want to treat these surfaces as if they were regular RGBA formats.
         */
        if (c->fs_key->color_fmt[rt].swizzle[0] == 2 &&
            c->fs_key->color_fmt[rt].format != PIPE_FORMAT_B5G6R5_UNORM) {
                return ident;
        } else {
                return  c->fs_key->color_fmt[rt].swizzle;
        }
}

static nir_def *
v3d_nir_get_tlb_color(nir_builder *b, struct v3d_compile *c, int rt, int sample)
{
        uint32_t num_components =
                util_format_get_nr_components(c->fs_key->color_fmt[rt].format);

        nir_def *color[4];
        for (int i = 0; i < 4; i++) {
                if (i < num_components) {
                        color[i] =
                                nir_load_tlb_color_v3d(b, 1, 32, nir_imm_int(b, rt),
                                                       .base = sample,
                                                       .component = i);
                } else {
                        /* These will be DCEd */
                        color[i] = nir_imm_int(b, 0);
                }
        }
        return nir_vec4(b, color[0], color[1], color[2], color[3]);
}

static nir_def *
v3d_emit_logic_op_raw(struct v3d_compile *c, nir_builder *b,
                      nir_def **src_chans, nir_def **dst_chans,
                      int rt, int sample)
{
        const uint8_t *fmt_swz = v3d_get_format_swizzle_for_rt(c, rt);

        nir_def *op_res[4];
        for (int i = 0; i < 4; i++) {
                nir_def *src = src_chans[i];
                nir_def *dst =
                        v3d_nir_get_swizzled_channel(b, dst_chans, fmt_swz[i]);
                op_res[i] = v3d_logicop(b, c->fs_key->logicop_func, src, dst);

                /* We configure our integer RTs to clamp, so we need to ignore
                 * result bits that don't fit in the destination RT component
                 * size.
                 */
                uint32_t bits =
                        util_format_get_component_bits(
                                c->fs_key->color_fmt[rt].format,
                                UTIL_FORMAT_COLORSPACE_RGB, i);
                if (bits > 0 && bits < 32) {
                        op_res[i] =
                                nir_iand_imm(b, op_res[i], (1u << bits) - 1);
                }
        }

        nir_def *r[4];
        for (int i = 0; i < 4; i++)
                r[i] = v3d_nir_get_swizzled_channel(b, op_res, fmt_swz[i]);

        return nir_vec4(b, r[0], r[1], r[2], r[3]);
}

static nir_def *
v3d_emit_logic_op_unorm(struct v3d_compile *c, nir_builder *b,
                        nir_def **src_chans, nir_def **dst_chans,
                        int rt, int sample,
                        nir_pack_func pack_func, nir_unpack_func unpack_func)
{
        static const uint8_t src_swz[4] = { 0, 1, 2, 3 };
        nir_def *packed_src =
                v3d_nir_swizzle_and_pack(b, src_chans, src_swz, pack_func);

        const uint8_t *fmt_swz = v3d_get_format_swizzle_for_rt(c, rt);
        nir_def *packed_dst =
                v3d_nir_swizzle_and_pack(b, dst_chans, fmt_swz, pack_func);

        nir_def *packed_result =
                v3d_logicop(b, c->fs_key->logicop_func, packed_src, packed_dst);

        return v3d_nir_unpack_and_swizzle(b, packed_result, fmt_swz, unpack_func);
}

static nir_def *
v3d_nir_emit_logic_op(struct v3d_compile *c, nir_builder *b,
                      nir_def *src, int rt, int sample)
{
        nir_def *dst = v3d_nir_get_tlb_color(b, c, rt, sample);

        nir_def *src_chans[4], *dst_chans[4];
        for (unsigned i = 0; i < 4; i++) {
                src_chans[i] = nir_channel(b, src, i);
                dst_chans[i] = nir_channel(b, dst, i);
        }

        if (c->fs_key->color_fmt[rt].format == PIPE_FORMAT_R10G10B10A2_UNORM) {
                return v3d_emit_logic_op_unorm(
                        c, b, src_chans, dst_chans, rt, 0,
                        pack_unorm_rgb10a2, unpack_unorm_rgb10a2);
        }

        if (util_format_is_unorm(c->fs_key->color_fmt[rt].format)) {
                return v3d_emit_logic_op_unorm(
                        c, b, src_chans, dst_chans, rt, 0,
                        nir_pack_unorm_4x8, nir_unpack_unorm_4x8);
        }

        return v3d_emit_logic_op_raw(c, b, src_chans, dst_chans, rt, 0);
}

static void
v3d_emit_ms_output(nir_builder *b,
                   nir_def *color, nir_src *offset,
                   nir_alu_type type, int rt, int sample)
{
        nir_store_tlb_sample_color_v3d(b, color, nir_imm_int(b, rt), .base = sample, .component = 0, .src_type = type);
}

static void
v3d_nir_lower_logic_op_instr(struct v3d_compile *c,
                             nir_builder *b,
                             nir_intrinsic_instr *intr,
                             int rt)
{
        nir_def *frag_color = intr->src[0].ssa;


        const int logic_op = c->fs_key->logicop_func;
        if (c->fs_key->msaa && logicop_depends_on_dst_color(logic_op)) {
                c->msaa_per_sample_output = true;

                nir_src *offset = &intr->src[1];
                nir_alu_type type = nir_intrinsic_src_type(intr);
                for (int i = 0; i < V3D_MAX_SAMPLES; i++) {
                        nir_def *sample =
                                v3d_nir_emit_logic_op(c, b, frag_color, rt, i);

                        v3d_emit_ms_output(b, sample, offset, type, rt, i);
                }

                nir_instr_remove(&intr->instr);
        } else {
                nir_def *result =
                        v3d_nir_emit_logic_op(c, b, frag_color, rt, 0);

                nir_src_rewrite(&intr->src[0], result);
                intr->num_components = result->num_components;
        }
}

static bool
v3d_nir_lower_logic_ops_block(nir_block *block, struct v3d_compile *c)
{
        bool progress = false;

        nir_foreach_instr_safe(instr, block) {
                if (instr->type != nir_instr_type_intrinsic)
                        continue;

                nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
                if (intr->intrinsic != nir_intrinsic_store_output)
                        continue;

                nir_foreach_shader_out_variable(var, c->s) {
                        const int driver_loc = var->data.driver_location;
                        if (driver_loc != nir_intrinsic_base(intr))
                                continue;

                        const int loc = var->data.location;
                        if (loc != FRAG_RESULT_COLOR &&
                            (loc < FRAG_RESULT_DATA0 ||
                             loc >= FRAG_RESULT_DATA0 + V3D_MAX_DRAW_BUFFERS)) {
                                continue;
                        }

                        /* Logic operations do not apply on floating point or
                         * sRGB enabled render targets.
                         */
                        const int rt = driver_loc;
                        assert(rt < V3D_MAX_DRAW_BUFFERS);

                        const enum pipe_format format =
                                c->fs_key->color_fmt[rt].format;
                        if (util_format_is_float(format) ||
                            util_format_is_srgb(format)) {
                                continue;
                        }

                        nir_builder b = nir_builder_at(nir_before_instr(&intr->instr));
                        v3d_nir_lower_logic_op_instr(c, &b, intr, rt);

                        progress = true;
                }
        }

        return progress;
}

bool
v3d_nir_lower_logic_ops(nir_shader *s, struct v3d_compile *c)
{
        bool progress = false;

        /* Nothing to do if logic op is 'copy src to dst' or if logic ops are
         * disabled (we set the logic op to copy in that case).
         */
        if (c->fs_key->logicop_func == PIPE_LOGICOP_COPY)
                return false;

        nir_foreach_function_impl(impl, s) {
                nir_foreach_block(block, impl)
                        progress |= v3d_nir_lower_logic_ops_block(block, c);

                if (progress) {
                        nir_metadata_preserve(impl,
                                              nir_metadata_block_index |
                                              nir_metadata_dominance);
                } else {
                        nir_metadata_preserve(impl,
                                              nir_metadata_all);
                }
        }

        return progress;
}
