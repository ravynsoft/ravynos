/*
 * Copyright © 2016-2018 Broadcom
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

#include "v3d_compiler.h"

/* We don't do any address packing. */
#define __gen_user_data void
#define __gen_address_type uint32_t
#define __gen_address_offset(reloc) (*reloc)
#define __gen_emit_reloc(cl, reloc)
#include "cle/v3d_packet_v42_pack.h"

static inline struct qinst *
vir_TMU_WRITE(struct v3d_compile *c, enum v3d_qpu_waddr waddr, struct qreg val)
{
        /* XXX perf: We should figure out how to merge ALU operations
         * producing the val with this MOV, when possible.
         */
        return vir_MOV_dest(c, vir_reg(QFILE_MAGIC, waddr), val);
}

static inline struct qinst *
vir_TMU_WRITE_or_count(struct v3d_compile *c,
                       enum v3d_qpu_waddr waddr,
                       struct qreg val,
                       uint32_t *tmu_writes)
{
        if (tmu_writes) {
                (*tmu_writes)++;
                return NULL;
        } else {
                return vir_TMU_WRITE(c, waddr, val);
        }
}

static void
vir_WRTMUC(struct v3d_compile *c, enum quniform_contents contents, uint32_t data)
{
        struct qinst *inst = vir_NOP(c);
        inst->qpu.sig.wrtmuc = true;
        inst->uniform = vir_get_uniform_index(c, contents, data);
}

static const struct V3D42_TMU_CONFIG_PARAMETER_1 p1_unpacked_default = {
        .per_pixel_mask_enable = true,
};

static const struct V3D42_TMU_CONFIG_PARAMETER_2 p2_unpacked_default = {
        .op = V3D_TMU_OP_REGULAR,
};

/**
 * If 'tmu_writes' is not NULL, then it just counts required register writes,
 * otherwise, it emits the actual register writes.
 *
 * It is important to notice that emitting register writes for the current
 * TMU operation may trigger a TMU flush, since it is possible that any
 * of the inputs required for the register writes is the result of a pending
 * TMU operation. If that happens we need to make sure that it doesn't happen
 * in the middle of the TMU register writes for the current TMU operation,
 * which is why we always call ntq_get_src() even if we are only interested in
 * register write counts.
 */
static void
handle_tex_src(struct v3d_compile *c,
               nir_tex_instr *instr,
               unsigned src_idx,
               unsigned non_array_components,
               struct V3D42_TMU_CONFIG_PARAMETER_2 *p2_unpacked,
               struct qreg *s_out,
               unsigned *tmu_writes)
{
        /* Either we are calling this just to count required TMU writes, or we
         * are calling this to emit the actual TMU writes.
         */
        assert(tmu_writes || (s_out && p2_unpacked));

        struct qreg s;
        switch (instr->src[src_idx].src_type) {
        case nir_tex_src_coord:
                /* S triggers the lookup, so save it for the end. */
                s = ntq_get_src(c, instr->src[src_idx].src, 0);
                if (tmu_writes)
                        (*tmu_writes)++;
                else
                        *s_out = s;

                if (non_array_components > 1) {
                        struct qreg src =
                                ntq_get_src(c, instr->src[src_idx].src, 1);
                        vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUT, src,
                                                tmu_writes);
                }

                if (non_array_components > 2) {
                        struct qreg src =
                                ntq_get_src(c, instr->src[src_idx].src, 2);
                        vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUR, src,
                                               tmu_writes);
                }

                if (instr->is_array) {
                        struct qreg src =
                                ntq_get_src(c, instr->src[src_idx].src,
                                            instr->coord_components - 1);
                        vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUI, src,
                                               tmu_writes);
                }
                break;

        case nir_tex_src_bias: {
                struct qreg src = ntq_get_src(c, instr->src[src_idx].src, 0);
                vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUB, src, tmu_writes);
                break;
        }

        case nir_tex_src_lod: {
                struct qreg src = ntq_get_src(c, instr->src[src_idx].src, 0);
                vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUB, src, tmu_writes);
                if (!tmu_writes) {
                        /* With texel fetch automatic LOD is already disabled,
                         * and disable_autolod must not be enabled. For
                         * non-cubes we can use the register TMUSLOD, that
                         * implicitly sets disable_autolod.
                         */
                        assert(p2_unpacked);
                        if (instr->op != nir_texop_txf &&
                            instr->sampler_dim == GLSL_SAMPLER_DIM_CUBE) {
                                    p2_unpacked->disable_autolod = true;
                        }
               }
               break;
        }

        case nir_tex_src_comparator: {
                struct qreg src = ntq_get_src(c, instr->src[src_idx].src, 0);
                vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUDREF, src, tmu_writes);
                break;
        }

        case nir_tex_src_offset: {
                bool is_const_offset = nir_src_is_const(instr->src[src_idx].src);
                if (is_const_offset) {
                        if (!tmu_writes) {
                                p2_unpacked->offset_s =
                                        nir_src_comp_as_int(instr->src[src_idx].src, 0);
                                if (non_array_components >= 2)
                                        p2_unpacked->offset_t =
                                                nir_src_comp_as_int(instr->src[src_idx].src, 1);
                                if (non_array_components >= 3)
                                        p2_unpacked->offset_r =
                                                nir_src_comp_as_int(instr->src[src_idx].src, 2);
                        }
                } else {
                        struct qreg src_0 =
                                ntq_get_src(c, instr->src[src_idx].src, 0);
                        struct qreg src_1 =
                                ntq_get_src(c, instr->src[src_idx].src, 1);
                        if (!tmu_writes) {
                                struct qreg mask = vir_uniform_ui(c, 0xf);
                                struct qreg x, y, offset;

                                x = vir_AND(c, src_0, mask);
                                y = vir_AND(c, src_1, mask);
                                offset = vir_OR(c, x,
                                                vir_SHL(c, y, vir_uniform_ui(c, 4)));

                                vir_TMU_WRITE(c, V3D_QPU_WADDR_TMUOFF, offset);
                        } else {
                                (*tmu_writes)++;
                        }
                }
                break;
        }

        default:
                unreachable("unknown texture source");
        }
}

static void
vir_tex_handle_srcs(struct v3d_compile *c,
                    nir_tex_instr *instr,
                    struct V3D42_TMU_CONFIG_PARAMETER_2 *p2_unpacked,
                    struct qreg *s,
                    unsigned *tmu_writes)
{
        unsigned non_array_components = instr->op != nir_texop_lod ?
                instr->coord_components - instr->is_array :
                instr->coord_components;

        for (unsigned i = 0; i < instr->num_srcs; i++) {
                handle_tex_src(c, instr, i, non_array_components,
                               p2_unpacked, s, tmu_writes);
        }
}

static unsigned
get_required_tex_tmu_writes(struct v3d_compile *c, nir_tex_instr *instr)
{
        unsigned tmu_writes = 0;
        vir_tex_handle_srcs(c, instr, NULL, NULL, &tmu_writes);
        return tmu_writes;
}

void
v3d_vir_emit_tex(struct v3d_compile *c, nir_tex_instr *instr)
{
        unsigned texture_idx = instr->texture_index;

        /* For instructions that don't have a sampler (i.e. txf) we bind
         * default sampler state via the backend_flags to handle precision.
         */
        unsigned sampler_idx = nir_tex_instr_need_sampler(instr) ?
                               instr->sampler_index : instr->backend_flags;

        /* Even if the texture operation doesn't need a sampler by
         * itself, we still need to add the sampler configuration
         * parameter if the output is 32 bit
         */
        assert(sampler_idx < c->key->num_samplers_used);
        bool output_type_32_bit =
                c->key->sampler[sampler_idx].return_size == 32;

        struct V3D42_TMU_CONFIG_PARAMETER_0 p0_unpacked = {
        };

        /* Limit the number of channels returned to both how many the NIR
         * instruction writes and how many the instruction could produce.
         */
        nir_intrinsic_instr *store = nir_store_reg_for_def(&instr->def);
        if (store == NULL) {
                p0_unpacked.return_words_of_texture_data =
                        nir_def_components_read(&instr->def);
        } else {
                nir_def *reg = store->src[1].ssa;
                nir_intrinsic_instr *decl = nir_reg_get_decl(reg);
                unsigned reg_num_components =
                        nir_intrinsic_num_components(decl);

                /* For the non-ssa case we don't have a full equivalent to
                 * nir_def_components_read. This is a problem for the 16
                 * bit case. nir_lower_tex will not change the destination as
                 * nir_tex_instr_dest_size will still return 4. The driver is
                 * just expected to not store on other channels, so we
                 * manually ensure that here.
                 */
                uint32_t num_components = output_type_32_bit ?
                        MIN2(reg_num_components, 4) :
                        MIN2(reg_num_components, 2);

                p0_unpacked.return_words_of_texture_data = (1 << num_components) - 1;
        }
        assert(p0_unpacked.return_words_of_texture_data != 0);

        struct V3D42_TMU_CONFIG_PARAMETER_2 p2_unpacked = {
                .op = V3D_TMU_OP_REGULAR,
                .gather_mode = instr->op == nir_texop_tg4,
                .gather_component = instr->component,
                .coefficient_mode = instr->op == nir_texop_txd,
                .disable_autolod = instr->op == nir_texop_tg4,
                .lod_query = instr->op == nir_texop_lod,
        };

        const unsigned tmu_writes = get_required_tex_tmu_writes(c, instr);

        /* The input FIFO has 16 slots across all threads so if we require
         * more than that we need to lower thread count.
         */
        while (tmu_writes > 16 / c->threads)
                c->threads /= 2;

       /* If pipelining this TMU operation would overflow TMU fifos, we need
        * to flush any outstanding TMU operations.
        */
        const unsigned dest_components =
           util_bitcount(p0_unpacked.return_words_of_texture_data);
        if (ntq_tmu_fifo_overflow(c, dest_components))
                ntq_flush_tmu(c);

        /* Process tex sources emitting corresponding TMU writes */
        struct qreg s = { };
        vir_tex_handle_srcs(c, instr, &p2_unpacked, &s, NULL);

        uint32_t p0_packed;
        V3D42_TMU_CONFIG_PARAMETER_0_pack(NULL,
                                          (uint8_t *)&p0_packed,
                                          &p0_unpacked);

        uint32_t p2_packed;
        V3D42_TMU_CONFIG_PARAMETER_2_pack(NULL,
                                          (uint8_t *)&p2_packed,
                                          &p2_unpacked);

        /* Load texture_idx number into the high bits of the texture address field,
         * which will be be used by the driver to decide which texture to put
         * in the actual address field.
         */
        p0_packed |= texture_idx << 24;

        vir_WRTMUC(c, QUNIFORM_TMU_CONFIG_P0, p0_packed);

        /* p1 is optional, but we can skip it only if p2 can be skipped too */
        bool needs_p2_config =
                (instr->op == nir_texop_lod ||
                 memcmp(&p2_unpacked, &p2_unpacked_default,
                        sizeof(p2_unpacked)) != 0);

        /* To handle the cases were we can't just use p1_unpacked_default */
        bool non_default_p1_config = nir_tex_instr_need_sampler(instr) ||
                output_type_32_bit;

        if (non_default_p1_config) {
                struct V3D42_TMU_CONFIG_PARAMETER_1 p1_unpacked = {
                        .output_type_32_bit = output_type_32_bit,

                        .unnormalized_coordinates = (instr->sampler_dim ==
                                                     GLSL_SAMPLER_DIM_RECT),
                };

                /* Word enables can't ask for more channels than the
                 * output type could provide (2 for f16, 4 for
                 * 32-bit).
                 */
                assert(!p1_unpacked.output_type_32_bit ||
                       p0_unpacked.return_words_of_texture_data < (1 << 4));
                assert(p1_unpacked.output_type_32_bit ||
                       p0_unpacked.return_words_of_texture_data < (1 << 2));

                uint32_t p1_packed;
                V3D42_TMU_CONFIG_PARAMETER_1_pack(NULL,
                                                  (uint8_t *)&p1_packed,
                                                  &p1_unpacked);

                if (nir_tex_instr_need_sampler(instr)) {
                        /* Load sampler_idx number into the high bits of the
                         * sampler address field, which will be be used by the
                         * driver to decide which sampler to put in the actual
                         * address field.
                         */
                        p1_packed |= sampler_idx << 24;

                        vir_WRTMUC(c, QUNIFORM_TMU_CONFIG_P1, p1_packed);
                } else {
                        /* In this case, we don't need to merge in any
                         * sampler state from the API and can just use
                         * our packed bits */
                        vir_WRTMUC(c, QUNIFORM_CONSTANT, p1_packed);
                }
        } else if (needs_p2_config) {
                /* Configuration parameters need to be set up in
                 * order, and if P2 is needed, you need to set up P1
                 * too even if sampler info is not needed by the
                 * texture operation. But we can set up default info,
                 * and avoid asking the driver for the sampler state
                 * address
                 */
                uint32_t p1_packed_default;
                V3D42_TMU_CONFIG_PARAMETER_1_pack(NULL,
                                                  (uint8_t *)&p1_packed_default,
                                                  &p1_unpacked_default);
                vir_WRTMUC(c, QUNIFORM_CONSTANT, p1_packed_default);
        }

        if (needs_p2_config)
                vir_WRTMUC(c, QUNIFORM_CONSTANT, p2_packed);

        /* Emit retiring TMU write */
        struct qinst *retiring;
        if (instr->op == nir_texop_txf) {
                assert(instr->sampler_dim != GLSL_SAMPLER_DIM_CUBE);
                retiring = vir_TMU_WRITE(c, V3D_QPU_WADDR_TMUSF, s);
        } else if (instr->sampler_dim == GLSL_SAMPLER_DIM_CUBE) {
                retiring = vir_TMU_WRITE(c, V3D_QPU_WADDR_TMUSCM, s);
        } else if (instr->op == nir_texop_txl) {
                retiring = vir_TMU_WRITE(c, V3D_QPU_WADDR_TMUSLOD, s);
        } else {
                retiring = vir_TMU_WRITE(c, V3D_QPU_WADDR_TMUS, s);
        }

        retiring->ldtmu_count = p0_unpacked.return_words_of_texture_data;
        ntq_add_pending_tmu_flush(c, &instr->def,
                                  p0_unpacked.return_words_of_texture_data);
}

static uint32_t
v3d_image_atomic_tmu_op(nir_intrinsic_instr *instr)
{
        nir_atomic_op atomic_op = nir_intrinsic_atomic_op(instr);
        switch (atomic_op) {
        case nir_atomic_op_iadd:    return v3d_get_op_for_atomic_add(instr, 3);
        case nir_atomic_op_imin:    return V3D_TMU_OP_WRITE_SMIN;
        case nir_atomic_op_umin:    return V3D_TMU_OP_WRITE_UMIN_FULL_L1_CLEAR;
        case nir_atomic_op_imax:    return V3D_TMU_OP_WRITE_SMAX;
        case nir_atomic_op_umax:    return V3D_TMU_OP_WRITE_UMAX;
        case nir_atomic_op_iand:    return V3D_TMU_OP_WRITE_AND_READ_INC;
        case nir_atomic_op_ior:     return V3D_TMU_OP_WRITE_OR_READ_DEC;
        case nir_atomic_op_ixor:    return V3D_TMU_OP_WRITE_XOR_READ_NOT;
        case nir_atomic_op_xchg:    return V3D_TMU_OP_WRITE_XCHG_READ_FLUSH;
        case nir_atomic_op_cmpxchg: return V3D_TMU_OP_WRITE_CMPXCHG_READ_FLUSH;
        default:                    unreachable("unknown atomic op");
        }
}

static uint32_t
v3d_image_load_store_tmu_op(nir_intrinsic_instr *instr)
{
        switch (instr->intrinsic) {
        case nir_intrinsic_image_load:
        case nir_intrinsic_image_store:
                return V3D_TMU_OP_REGULAR;

        case nir_intrinsic_image_atomic:
        case nir_intrinsic_image_atomic_swap:
                return v3d_image_atomic_tmu_op(instr);

        default:
                unreachable("unknown image intrinsic");
        };
}

/**
 * If 'tmu_writes' is not NULL, then it just counts required register writes,
 * otherwise, it emits the actual register writes.
 *
 * It is important to notice that emitting register writes for the current
 * TMU operation may trigger a TMU flush, since it is possible that any
 * of the inputs required for the register writes is the result of a pending
 * TMU operation. If that happens we need to make sure that it doesn't happen
 * in the middle of the TMU register writes for the current TMU operation,
 * which is why we always call ntq_get_src() even if we are only interested in
 * register write counts.
 */
static struct qinst *
vir_image_emit_register_writes(struct v3d_compile *c,
                               nir_intrinsic_instr *instr,
                               bool atomic_add_replaced,
                               uint32_t *tmu_writes)
{
        if (tmu_writes)
                *tmu_writes = 0;

        bool is_1d = false;
        switch (nir_intrinsic_image_dim(instr)) {
        case GLSL_SAMPLER_DIM_1D:
                is_1d = true;
                break;
        case GLSL_SAMPLER_DIM_BUF:
                break;
        case GLSL_SAMPLER_DIM_2D:
        case GLSL_SAMPLER_DIM_RECT:
        case GLSL_SAMPLER_DIM_CUBE: {
                struct qreg src = ntq_get_src(c, instr->src[1], 1);
                vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUT, src, tmu_writes);
                break;
        }
        case GLSL_SAMPLER_DIM_3D: {
                struct qreg src_1_1 = ntq_get_src(c, instr->src[1], 1);
                struct qreg src_1_2 = ntq_get_src(c, instr->src[1], 2);
                vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUT, src_1_1, tmu_writes);
                vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUR, src_1_2, tmu_writes);
                break;
        }
        default:
                unreachable("bad image sampler dim");
        }

        /* In order to fetch on a cube map, we need to interpret it as
         * 2D arrays, where the third coord would be the face index.
         */
        if (nir_intrinsic_image_dim(instr) == GLSL_SAMPLER_DIM_CUBE ||
            nir_intrinsic_image_array(instr)) {
                struct qreg src = ntq_get_src(c, instr->src[1], is_1d ? 1 : 2);
                vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUI, src, tmu_writes);
        }

        /* Emit the data writes for atomics or image store. */
        if (instr->intrinsic != nir_intrinsic_image_load &&
            !atomic_add_replaced) {
                for (int i = 0; i < nir_intrinsic_src_components(instr, 3); i++) {
                        struct qreg src_3_i = ntq_get_src(c, instr->src[3], i);
                        vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUD, src_3_i,
                                               tmu_writes);
                }

                /* Second atomic argument */
                if (instr->intrinsic == nir_intrinsic_image_atomic_swap &&
                    nir_intrinsic_atomic_op(instr) == nir_atomic_op_cmpxchg) {
                        struct qreg src_4_0 = ntq_get_src(c, instr->src[4], 0);
                        vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUD, src_4_0,
                                               tmu_writes);
                }
        }

        struct qreg src_1_0 = ntq_get_src(c, instr->src[1], 0);
        if (!tmu_writes && vir_in_nonuniform_control_flow(c) &&
            instr->intrinsic != nir_intrinsic_image_load) {
                vir_set_pf(c, vir_MOV_dest(c, vir_nop_reg(), c->execute),
                           V3D_QPU_PF_PUSHZ);
        }

        struct qinst *retiring =
                vir_TMU_WRITE_or_count(c, V3D_QPU_WADDR_TMUSF, src_1_0, tmu_writes);

        if (!tmu_writes && vir_in_nonuniform_control_flow(c) &&
            instr->intrinsic != nir_intrinsic_image_load) {
                struct qinst *last_inst =
                        (struct  qinst *)c->cur_block->instructions.prev;
                vir_set_cond(last_inst, V3D_QPU_COND_IFA);
        }

        return retiring;
}

static unsigned
get_required_image_tmu_writes(struct v3d_compile *c,
                              nir_intrinsic_instr *instr,
                              bool atomic_add_replaced)
{
        unsigned tmu_writes;
        vir_image_emit_register_writes(c, instr, atomic_add_replaced,
                                       &tmu_writes);
        return tmu_writes;
}

void
v3d_vir_emit_image_load_store(struct v3d_compile *c,
                              nir_intrinsic_instr *instr)
{
        unsigned format = nir_intrinsic_format(instr);
        unsigned unit = nir_src_as_uint(instr->src[0]);

        struct V3D42_TMU_CONFIG_PARAMETER_0 p0_unpacked = {
        };

        struct V3D42_TMU_CONFIG_PARAMETER_1 p1_unpacked = {
                .per_pixel_mask_enable = true,
                .output_type_32_bit = v3d_gl_format_is_return_32(format),
        };

        struct V3D42_TMU_CONFIG_PARAMETER_2 p2_unpacked = { 0 };

        /* Limit the number of channels returned to both how many the NIR
         * instruction writes and how many the instruction could produce.
         */
        uint32_t instr_return_channels = nir_intrinsic_dest_components(instr);
        if (!p1_unpacked.output_type_32_bit)
                instr_return_channels = (instr_return_channels + 1) / 2;

        p0_unpacked.return_words_of_texture_data =
                (1 << instr_return_channels) - 1;

        p2_unpacked.op = v3d_image_load_store_tmu_op(instr);

        /* If we were able to replace atomic_add for an inc/dec, then we
         * need/can to do things slightly different, like not loading the
         * amount to add/sub, as that is implicit.
         */
        bool atomic_add_replaced =
                instr->intrinsic == nir_intrinsic_image_atomic &&
                nir_intrinsic_atomic_op(instr) == nir_atomic_op_iadd &&
                (p2_unpacked.op == V3D_TMU_OP_WRITE_AND_READ_INC ||
                 p2_unpacked.op == V3D_TMU_OP_WRITE_OR_READ_DEC);

        uint32_t p0_packed;
        V3D42_TMU_CONFIG_PARAMETER_0_pack(NULL,
                                          (uint8_t *)&p0_packed,
                                          &p0_unpacked);

        /* Load unit number into the high bits of the texture or sampler
         * address field, which will be be used by the driver to decide which
         * texture to put in the actual address field.
         */
        p0_packed |= unit << 24;

        uint32_t p1_packed;
        V3D42_TMU_CONFIG_PARAMETER_1_pack(NULL,
                                          (uint8_t *)&p1_packed,
                                          &p1_unpacked);

        uint32_t p2_packed;
        V3D42_TMU_CONFIG_PARAMETER_2_pack(NULL,
                                          (uint8_t *)&p2_packed,
                                          &p2_unpacked);

        if (instr->intrinsic != nir_intrinsic_image_load)
                c->tmu_dirty_rcl = true;


        const uint32_t tmu_writes =
                get_required_image_tmu_writes(c, instr, atomic_add_replaced);

        /* The input FIFO has 16 slots across all threads so if we require
         * more than that we need to lower thread count.
         */
        while (tmu_writes > 16 / c->threads)
                c->threads /= 2;

       /* If pipelining this TMU operation would overflow TMU fifos, we need
        * to flush any outstanding TMU operations.
        */
        if (ntq_tmu_fifo_overflow(c, instr_return_channels))
                ntq_flush_tmu(c);

        vir_WRTMUC(c, QUNIFORM_IMAGE_TMU_CONFIG_P0, p0_packed);
        if (memcmp(&p1_unpacked, &p1_unpacked_default, sizeof(p1_unpacked)))
                   vir_WRTMUC(c, QUNIFORM_CONSTANT, p1_packed);
        if (memcmp(&p2_unpacked, &p2_unpacked_default, sizeof(p2_unpacked)))
                   vir_WRTMUC(c, QUNIFORM_CONSTANT, p2_packed);

        struct qinst *retiring =
                vir_image_emit_register_writes(c, instr, atomic_add_replaced, NULL);
        retiring->ldtmu_count = p0_unpacked.return_words_of_texture_data;
        ntq_add_pending_tmu_flush(c, &instr->def,
                                  p0_unpacked.return_words_of_texture_data);
}
