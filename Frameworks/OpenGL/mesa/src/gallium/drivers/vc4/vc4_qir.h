/*
 * Copyright © 2014 Broadcom
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

#ifndef VC4_QIR_H
#define VC4_QIR_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "util/macros.h"
#include "compiler/nir/nir.h"
#include "util/list.h"
#include "util/u_math.h"

#include "vc4_screen.h"
#include "vc4_qpu_defines.h"
#include "vc4_qpu.h"
#include "kernel/vc4_packet.h"
#include "pipe/p_state.h"

struct nir_builder;

enum qfile {
        QFILE_NULL,
        QFILE_TEMP,
        QFILE_VARY,
        QFILE_UNIF,
        QFILE_VPM,
        QFILE_TLB_COLOR_WRITE,
        QFILE_TLB_COLOR_WRITE_MS,
        QFILE_TLB_Z_WRITE,
        QFILE_TLB_STENCIL_SETUP,

        /* If tex_s is written on its own without preceding t/r/b setup, it's
         * a direct memory access using the input value, without the sideband
         * uniform load.  We represent these in QIR as a separate write
         * destination so we can tell if the sideband uniform is present.
         */
        QFILE_TEX_S_DIRECT,

        QFILE_TEX_S,
        QFILE_TEX_T,
        QFILE_TEX_R,
        QFILE_TEX_B,

        /* Payload registers that aren't in the physical register file, so we
         * can just use the corresponding qpu_reg at qpu_emit time.
         */
        QFILE_FRAG_X,
        QFILE_FRAG_Y,
        QFILE_FRAG_REV_FLAG,
        QFILE_QPU_ELEMENT,

        /**
         * Stores an immediate value in the index field that will be used
         * directly by qpu_load_imm().
         */
        QFILE_LOAD_IMM,

        /**
         * Stores an immediate value in the index field that can be turned
         * into a small immediate field by qpu_encode_small_immediate().
         */
        QFILE_SMALL_IMM,
};

struct qreg {
        enum qfile file;
        uint32_t index;
        int pack;
};

static inline struct qreg qir_reg(enum qfile file, uint32_t index)
{
        return (struct qreg){file, index};
}

enum qop {
        QOP_UNDEF,
        QOP_MOV,
        QOP_FMOV,
        QOP_MMOV,
        QOP_FADD,
        QOP_FSUB,
        QOP_FMUL,
        QOP_V8MULD,
        QOP_V8MIN,
        QOP_V8MAX,
        QOP_V8ADDS,
        QOP_V8SUBS,
        QOP_MUL24,
        QOP_FMIN,
        QOP_FMAX,
        QOP_FMINABS,
        QOP_FMAXABS,
        QOP_ADD,
        QOP_SUB,
        QOP_SHL,
        QOP_SHR,
        QOP_ASR,
        QOP_MIN,
        QOP_MIN_NOIMM,
        QOP_MAX,
        QOP_AND,
        QOP_OR,
        QOP_XOR,
        QOP_NOT,

        QOP_FTOI,
        QOP_ITOF,
        QOP_RCP,
        QOP_RSQ,
        QOP_EXP2,
        QOP_LOG2,
        QOP_VW_SETUP,
        QOP_VR_SETUP,
        QOP_TLB_COLOR_READ,
        QOP_MS_MASK,
        QOP_VARY_ADD_C,

        QOP_FRAG_Z,
        QOP_FRAG_W,

        /**
         * Signal of texture read being necessary and then reading r4 into
         * the destination
         */
        QOP_TEX_RESULT,

        /**
         * Insert the signal for switching threads in a threaded fragment
         * shader.  No value can be live in an accumulator across a thrsw.
         *
         * At the QPU level, this will have several delay slots before the
         * switch happens.  Those slots are the responsibility of the
         * scheduler.
         */
        QOP_THRSW,

        /* 32-bit immediate loaded to each SIMD channel */
        QOP_LOAD_IMM,

        /* 32-bit immediate divided into 16 2-bit unsigned int values and
         * loaded to each corresponding SIMD channel.
         */
        QOP_LOAD_IMM_U2,
        /* 32-bit immediate divided into 16 2-bit signed int values and
         * loaded to each corresponding SIMD channel.
         */
        QOP_LOAD_IMM_I2,

        QOP_ROT_MUL,

        /* Jumps to block->successor[0] if the qinst->cond (as a
         * QPU_COND_BRANCH_*) passes, or block->successor[1] if not.  Note
         * that block->successor[1] may be unset if the condition is ALWAYS.
         */
        QOP_BRANCH,

        /* Emits an ADD from src[0] to src[1], where src[0] must be a
         * QOP_LOAD_IMM result and src[1] is a QUNIFORM_UNIFORMS_ADDRESS,
         * required by the kernel as part of its branch validation.
         */
        QOP_UNIFORMS_RESET,
};

struct queued_qpu_inst {
        struct list_head link;
        uint64_t inst;
};

struct qinst {
        struct list_head link;

        enum qop op;
        struct qreg dst;
        struct qreg src[3];
        bool sf;
        bool cond_is_exec_mask;
        uint8_t cond;
};

enum qstage {
        /**
         * Coordinate shader, runs during binning, before the VS, and just
         * outputs position.
         */
        QSTAGE_COORD,
        QSTAGE_VERT,
        QSTAGE_FRAG,
};

enum quniform_contents {
        /**
         * Indicates that a constant 32-bit value is copied from the program's
         * uniform contents.
         */
        QUNIFORM_CONSTANT,
        /**
         * Indicates that the program's uniform contents are used as an index
         * into the GL uniform storage.
         */
        QUNIFORM_UNIFORM,

        /** @{
         * Scaling factors from clip coordinates to relative to the viewport
         * center.
         *
         * This is used by the coordinate and vertex shaders to produce the
         * 32-bit entry consisting of 2 16-bit fields with 12.4 signed fixed
         * point offsets from the viewport ccenter.
         */
        QUNIFORM_VIEWPORT_X_SCALE,
        QUNIFORM_VIEWPORT_Y_SCALE,
        /** @} */

        QUNIFORM_VIEWPORT_Z_OFFSET,
        QUNIFORM_VIEWPORT_Z_SCALE,

        QUNIFORM_USER_CLIP_PLANE,

        /**
         * A reference to a texture config parameter 0 uniform.
         *
         * This is a uniform implicitly loaded with a QPU_W_TMU* write, which
         * defines texture type, miplevels, and such.  It will be found as a
         * parameter to the first QOP_TEX_[STRB] instruction in a sequence.
         */
        QUNIFORM_TEXTURE_CONFIG_P0,

        /**
         * A reference to a texture config parameter 1 uniform.
         *
         * This is a uniform implicitly loaded with a QPU_W_TMU* write, which
         * defines texture width, height, filters, and wrap modes.  It will be
         * found as a parameter to the second QOP_TEX_[STRB] instruction in a
         * sequence.
         */
        QUNIFORM_TEXTURE_CONFIG_P1,

        /** A reference to a texture config parameter 2 cubemap stride uniform */
        QUNIFORM_TEXTURE_CONFIG_P2,

        QUNIFORM_TEXTURE_FIRST_LEVEL,

        QUNIFORM_TEXTURE_MSAA_ADDR,

        QUNIFORM_UBO0_ADDR,
        QUNIFORM_UBO1_ADDR,

        QUNIFORM_TEXRECT_SCALE_X,
        QUNIFORM_TEXRECT_SCALE_Y,

        QUNIFORM_TEXTURE_BORDER_COLOR,

        QUNIFORM_BLEND_CONST_COLOR_X,
        QUNIFORM_BLEND_CONST_COLOR_Y,
        QUNIFORM_BLEND_CONST_COLOR_Z,
        QUNIFORM_BLEND_CONST_COLOR_W,
        QUNIFORM_BLEND_CONST_COLOR_RGBA,
        QUNIFORM_BLEND_CONST_COLOR_AAAA,

        QUNIFORM_STENCIL,

        QUNIFORM_SAMPLE_MASK,

        /* Placeholder uniform that will be updated by the kernel when used by
         * an instruction writing to QPU_W_UNIFORMS_ADDRESS.
         */
        QUNIFORM_UNIFORMS_ADDRESS,
};

struct vc4_varying_slot {
        uint8_t slot;
        uint8_t swizzle;
};

struct vc4_key {
        struct vc4_uncompiled_shader *shader_state;
        struct {
                enum pipe_format format;
                uint8_t swizzle[4];
                union {
                        struct {
                                unsigned compare_mode:1;
                                unsigned compare_func:3;
                                unsigned wrap_s:3;
                                unsigned wrap_t:3;
                                bool force_first_level:1;
                        };
                        struct {
                                uint16_t msaa_width, msaa_height;
                        };
                };
        } tex[VC4_MAX_TEXTURE_SAMPLERS];
        uint8_t ucp_enables;
};

struct vc4_fs_key {
        struct vc4_key base;
        enum pipe_format color_format;
        bool depth_enabled;
        bool stencil_enabled;
        bool stencil_twoside;
        bool stencil_full_writemasks;
        bool is_points;
        bool is_lines;
        bool point_coord_upper_left;
        bool msaa;
        bool sample_coverage;
        bool sample_alpha_to_coverage;
        bool sample_alpha_to_one;
        uint8_t logicop_func;
        uint32_t point_sprite_mask;
        uint32_t ubo_1_size;

        struct pipe_rt_blend_state blend;
};

struct vc4_vs_key {
        struct vc4_key base;

        const struct vc4_fs_inputs *fs_inputs;
        enum pipe_format attr_formats[8];
        bool is_coord;
        bool per_vertex_point_size;
};

/** A basic block of QIR instructions. */
struct qblock {
        struct list_head link;

        struct list_head instructions;
        struct list_head qpu_inst_list;

        struct set *predecessors;
        struct qblock *successors[2];

        int index;

        /* Instruction IPs for the first and last instruction of the block.
         * Set by vc4_qpu_schedule.c.
         */
        uint32_t start_qpu_ip;
        uint32_t end_qpu_ip;

        /* Instruction IP for the branch instruction of the block.  Set by
         * vc4_qpu_schedule.c.
         */
        uint32_t branch_qpu_ip;

        /** @{ used by vc4_qir_live_variables.c */
        BITSET_WORD *def;
        BITSET_WORD *use;
        BITSET_WORD *live_in;
        BITSET_WORD *live_out;
        int start_ip, end_ip;
        /** @} */
};

struct vc4_compile {
        struct vc4_context *vc4;
        nir_shader *s;
        nir_function_impl *impl;
        struct exec_list *cf_node_list;

        /**
         * Mapping from nir_register * or nir_def * to array of struct
         * qreg for the values.
         */
        struct hash_table *def_ht;

        /* For each temp, the instruction generating its value. */
        struct qinst **defs;
        uint32_t defs_array_size;

        /**
         * Inputs to the shader, arranged by TGSI declaration order.
         *
         * Not all fragment shader QFILE_VARY reads are present in this array.
         */
        struct qreg *inputs;
        struct qreg *outputs;
        bool msaa_per_sample_output;
        struct qreg color_reads[VC4_MAX_SAMPLES];
        struct qreg sample_colors[VC4_MAX_SAMPLES];
        uint32_t inputs_array_size;
        uint32_t outputs_array_size;
        uint32_t uniforms_array_size;

        /* State for whether we're executing on each channel currently.  0 if
         * yes, otherwise a block number + 1 that the channel jumped to.
         */
        struct qreg execute;

        struct qreg line_x, point_x, point_y;
        /** boolean (~0 -> true) if the fragment has been discarded. */
        struct qreg discard;
        struct qreg payload_FRAG_Z;
        struct qreg payload_FRAG_W;

        uint8_t vattr_sizes[8];

        /**
         * Array of the VARYING_SLOT_* of all FS QFILE_VARY reads.
         *
         * This includes those that aren't part of the VPM varyings, like
         * point/line coordinates.
         */
        struct vc4_varying_slot *input_slots;
        uint32_t num_input_slots;
        uint32_t input_slots_array_size;

        /**
         * An entry per outputs[] in the VS indicating what the VARYING_SLOT_*
         * of the output is.  Used to emit from the VS in the order that the
         * FS needs.
         */
        struct vc4_varying_slot *output_slots;

        struct pipe_shader_state *shader_state;
        struct vc4_key *key;
        struct vc4_fs_key *fs_key;
        struct vc4_vs_key *vs_key;

        /* Live ranges of temps. */
        int *temp_start, *temp_end;

        uint32_t *uniform_data;
        enum quniform_contents *uniform_contents;
        uint32_t uniform_array_size;
        uint32_t num_uniforms;
        uint32_t num_outputs;
        uint32_t num_texture_samples;
        uint32_t output_position_index;
        uint32_t output_color_index;
        uint32_t output_point_size_index;
        uint32_t output_sample_mask_index;

        struct qreg undef;
        enum qstage stage;
        uint32_t num_temps;
        uint32_t max_reg_pressure;

        struct list_head blocks;
        int next_block_index;
        struct qblock *cur_block;
        struct qblock *loop_cont_block;
        struct qblock *loop_break_block;
        struct qblock *last_top_block;

        struct list_head qpu_inst_list;

        /* Pre-QPU-scheduled instruction containing the last THRSW */
        uint64_t *last_thrsw;

        uint64_t *qpu_insts;
        uint32_t qpu_inst_count;
        uint32_t qpu_inst_size;
        uint32_t num_inputs;

        /**
         * Number of inputs from num_inputs remaining to be queued to the read
         * FIFO in the VS/CS.
         */
        uint32_t num_inputs_remaining;

        /* Number of inputs currently in the read FIFO for the VS/CS */
        uint32_t num_inputs_in_fifo;

        /** Next offset in the VPM to read from in the VS/CS */
        uint32_t vpm_read_offset;

        uint32_t program_id;
        uint32_t variant_id;

        /* Set to compile program in threaded FS mode, where SIG_THREAD_SWITCH
         * is used to hide texturing latency at the cost of limiting ourselves
         * to the bottom half of physical reg space.
         */
        bool fs_threaded;

        bool last_thrsw_at_top_level;

        bool failed;
};

/* Special nir_load_input intrinsic index for loading the current TLB
 * destination color.
 */
#define VC4_NIR_TLB_COLOR_READ_INPUT		2000000000

#define VC4_NIR_MS_MASK_OUTPUT			2000000000

struct vc4_compile *qir_compile_init(void);
void qir_compile_destroy(struct vc4_compile *c);
struct qblock *qir_new_block(struct vc4_compile *c);
void qir_set_emit_block(struct vc4_compile *c, struct qblock *block);
void qir_link_blocks(struct qblock *predecessor, struct qblock *successor);
struct qblock *qir_entry_block(struct vc4_compile *c);
struct qblock *qir_exit_block(struct vc4_compile *c);
struct qinst *qir_inst(enum qop op, struct qreg dst,
                       struct qreg src0, struct qreg src1);
void qir_remove_instruction(struct vc4_compile *c, struct qinst *qinst);
struct qreg qir_uniform(struct vc4_compile *c,
                        enum quniform_contents contents,
                        uint32_t data);
void qir_schedule_instructions(struct vc4_compile *c);
void qir_reorder_uniforms(struct vc4_compile *c);
void qir_emit_uniform_stream_resets(struct vc4_compile *c);

struct qreg qir_emit_def(struct vc4_compile *c, struct qinst *inst);
struct qinst *qir_emit_nondef(struct vc4_compile *c, struct qinst *inst);

struct qreg qir_get_temp(struct vc4_compile *c);
void qir_calculate_live_intervals(struct vc4_compile *c);
int qir_get_nsrc(struct qinst *inst);
int qir_get_non_sideband_nsrc(struct qinst *inst);
int qir_get_tex_uniform_src(struct qinst *inst);
bool qir_reg_equals(struct qreg a, struct qreg b);
bool qir_has_side_effects(struct vc4_compile *c, struct qinst *inst);
bool qir_has_side_effect_reads(struct vc4_compile *c, struct qinst *inst);
bool qir_has_uniform_read(struct qinst *inst);
bool qir_is_mul(struct qinst *inst);
bool qir_is_raw_mov(struct qinst *inst);
bool qir_is_tex(struct qinst *inst);
bool qir_has_implicit_tex_uniform(struct qinst *inst);
bool qir_is_float_input(struct qinst *inst);
bool qir_depends_on_flags(struct qinst *inst);
bool qir_writes_r4(struct qinst *inst);
struct qreg qir_follow_movs(struct vc4_compile *c, struct qreg reg);
uint8_t qir_channels_written(struct qinst *inst);

void qir_dump(struct vc4_compile *c);
void qir_dump_inst(struct vc4_compile *c, struct qinst *inst);
char *qir_describe_uniform(enum quniform_contents contents, uint32_t data,
                           const uint32_t *uniforms);
const char *qir_get_stage_name(enum qstage stage);

void qir_validate(struct vc4_compile *c);

void qir_optimize(struct vc4_compile *c);
bool qir_opt_algebraic(struct vc4_compile *c);
bool qir_opt_coalesce_ff_writes(struct vc4_compile *c);
bool qir_opt_constant_folding(struct vc4_compile *c);
bool qir_opt_copy_propagation(struct vc4_compile *c);
bool qir_opt_dead_code(struct vc4_compile *c);
bool qir_opt_peephole_sf(struct vc4_compile *c);
bool qir_opt_small_immediates(struct vc4_compile *c);
bool qir_opt_vpm(struct vc4_compile *c);
void vc4_nir_lower_blend(nir_shader *s, struct vc4_compile *c);
void vc4_nir_lower_io(nir_shader *s, struct vc4_compile *c);
nir_def *vc4_nir_get_swizzled_channel(struct nir_builder *b,
                                          nir_def **srcs, int swiz);
void vc4_nir_lower_txf_ms(nir_shader *s, struct vc4_compile *c);
void qir_lower_uniforms(struct vc4_compile *c);

uint32_t qpu_schedule_instructions(struct vc4_compile *c);

void qir_SF(struct vc4_compile *c, struct qreg src);

static inline struct qreg
qir_uniform_ui(struct vc4_compile *c, uint32_t ui)
{
        return qir_uniform(c, QUNIFORM_CONSTANT, ui);
}

static inline struct qreg
qir_uniform_f(struct vc4_compile *c, float f)
{
        return qir_uniform(c, QUNIFORM_CONSTANT, fui(f));
}

#define QIR_ALU0(name)                                                   \
static inline struct qreg                                                \
qir_##name(struct vc4_compile *c)                                        \
{                                                                        \
        return qir_emit_def(c, qir_inst(QOP_##name, c->undef,            \
                                        c->undef, c->undef));            \
}                                                                        \
static inline struct qinst *                                             \
qir_##name##_dest(struct vc4_compile *c, struct qreg dest)               \
{                                                                        \
        return qir_emit_nondef(c, qir_inst(QOP_##name, dest,             \
                                           c->undef, c->undef));         \
}

#define QIR_ALU1(name)                                                   \
static inline struct qreg                                                \
qir_##name(struct vc4_compile *c, struct qreg a)                         \
{                                                                        \
        return qir_emit_def(c, qir_inst(QOP_##name, c->undef,            \
                                        a, c->undef));                   \
}                                                                        \
static inline struct qinst *                                             \
qir_##name##_dest(struct vc4_compile *c, struct qreg dest,               \
                  struct qreg a)                                         \
{                                                                        \
        return qir_emit_nondef(c, qir_inst(QOP_##name, dest, a,          \
                                           c->undef));                   \
}

#define QIR_ALU2(name)                                                   \
static inline struct qreg                                                \
qir_##name(struct vc4_compile *c, struct qreg a, struct qreg b)          \
{                                                                        \
        return qir_emit_def(c, qir_inst(QOP_##name, c->undef, a, b));    \
}                                                                        \
static inline struct qinst *                                             \
qir_##name##_dest(struct vc4_compile *c, struct qreg dest,               \
                  struct qreg a, struct qreg b)                          \
{                                                                        \
        return qir_emit_nondef(c, qir_inst(QOP_##name, dest, a, b));     \
}

#define QIR_NODST_1(name)                                               \
static inline struct qinst *                                            \
qir_##name(struct vc4_compile *c, struct qreg a)                        \
{                                                                       \
        return qir_emit_nondef(c, qir_inst(QOP_##name, c->undef,        \
                                           a, c->undef));               \
}

#define QIR_NODST_2(name)                                               \
static inline struct qinst *                                            \
qir_##name(struct vc4_compile *c, struct qreg a, struct qreg b)         \
{                                                                       \
        return qir_emit_nondef(c, qir_inst(QOP_##name, c->undef,        \
                                           a, b));                      \
}

#define QIR_PAYLOAD(name)                                                \
static inline struct qreg                                                \
qir_##name(struct vc4_compile *c)                                        \
{                                                                        \
        struct qreg *payload = &c->payload_##name;                       \
        if (payload->file != QFILE_NULL)                                 \
                return *payload;                                         \
        *payload = qir_get_temp(c);                                      \
        struct qinst *inst = qir_inst(QOP_##name, *payload,              \
                                      c->undef, c->undef);               \
        struct qblock *entry = qir_entry_block(c);                       \
        list_add(&inst->link, &entry->instructions);                     \
        c->defs[payload->index] = inst;                                  \
        return *payload;                                                 \
}

QIR_ALU1(MOV)
QIR_ALU1(FMOV)
QIR_ALU1(MMOV)
QIR_ALU2(FADD)
QIR_ALU2(FSUB)
QIR_ALU2(FMUL)
QIR_ALU2(V8MULD)
QIR_ALU2(V8MIN)
QIR_ALU2(V8MAX)
QIR_ALU2(V8ADDS)
QIR_ALU2(V8SUBS)
QIR_ALU2(MUL24)
QIR_ALU2(FMIN)
QIR_ALU2(FMAX)
QIR_ALU2(FMINABS)
QIR_ALU2(FMAXABS)
QIR_ALU1(FTOI)
QIR_ALU1(ITOF)

QIR_ALU2(ADD)
QIR_ALU2(SUB)
QIR_ALU2(SHL)
QIR_ALU2(SHR)
QIR_ALU2(ASR)
QIR_ALU2(MIN)
QIR_ALU2(MIN_NOIMM)
QIR_ALU2(MAX)
QIR_ALU2(AND)
QIR_ALU2(OR)
QIR_ALU2(XOR)
QIR_ALU1(NOT)

QIR_ALU1(RCP)
QIR_ALU1(RSQ)
QIR_ALU1(EXP2)
QIR_ALU1(LOG2)
QIR_ALU1(VARY_ADD_C)
QIR_PAYLOAD(FRAG_Z)
QIR_PAYLOAD(FRAG_W)
QIR_ALU0(TEX_RESULT)
QIR_ALU0(TLB_COLOR_READ)
QIR_NODST_1(MS_MASK)

static inline struct qreg
qir_SEL(struct vc4_compile *c, uint8_t cond, struct qreg src0, struct qreg src1)
{
        struct qreg t = qir_get_temp(c);
        qir_MOV_dest(c, t, src1);
        qir_MOV_dest(c, t, src0)->cond = cond;
        return t;
}

static inline struct qreg
qir_UNPACK_8_F(struct vc4_compile *c, struct qreg src, int i)
{
        struct qreg t = qir_FMOV(c, src);
        c->defs[t.index]->src[0].pack = QPU_UNPACK_8A + i;
        return t;
}

static inline struct qreg
qir_UNPACK_8_I(struct vc4_compile *c, struct qreg src, int i)
{
        struct qreg t = qir_MOV(c, src);
        c->defs[t.index]->src[0].pack = QPU_UNPACK_8A + i;
        return t;
}

static inline struct qreg
qir_UNPACK_16_F(struct vc4_compile *c, struct qreg src, int i)
{
        struct qreg t = qir_FMOV(c, src);
        c->defs[t.index]->src[0].pack = QPU_UNPACK_16A + i;
        return t;
}

static inline struct qreg
qir_UNPACK_16_I(struct vc4_compile *c, struct qreg src, int i)
{
        struct qreg t = qir_MOV(c, src);
        c->defs[t.index]->src[0].pack = QPU_UNPACK_16A + i;
        return t;
}

static inline void
qir_PACK_8_F(struct vc4_compile *c, struct qreg dest, struct qreg val, int chan)
{
        assert(!dest.pack);
        dest.pack = QPU_PACK_MUL_8A + chan;
        qir_emit_nondef(c, qir_inst(QOP_MMOV, dest, val, c->undef));
}

static inline struct qreg
qir_PACK_8888_F(struct vc4_compile *c, struct qreg val)
{
        struct qreg dest = qir_MMOV(c, val);
        c->defs[dest.index]->dst.pack = QPU_PACK_MUL_8888;
        return dest;
}

static inline void
qir_VPM_WRITE(struct vc4_compile *c, struct qreg val)
{
        qir_MOV_dest(c, qir_reg(QFILE_VPM, 0), val);
}

static inline struct qreg
qir_LOAD_IMM(struct vc4_compile *c, uint32_t val)
{
        return qir_emit_def(c, qir_inst(QOP_LOAD_IMM, c->undef,
                                        qir_reg(QFILE_LOAD_IMM, val), c->undef));
}

static inline struct qreg
qir_LOAD_IMM_U2(struct vc4_compile *c, uint32_t val)
{
        return qir_emit_def(c, qir_inst(QOP_LOAD_IMM_U2, c->undef,
                                        qir_reg(QFILE_LOAD_IMM, val),
                                        c->undef));
}

static inline struct qreg
qir_LOAD_IMM_I2(struct vc4_compile *c, uint32_t val)
{
        return qir_emit_def(c, qir_inst(QOP_LOAD_IMM_I2, c->undef,
                                        qir_reg(QFILE_LOAD_IMM, val),
                                        c->undef));
}

/** Shifts the multiply output to the right by rot channels */
static inline struct qreg
qir_ROT_MUL(struct vc4_compile *c, struct qreg val, uint32_t rot)
{
        return qir_emit_def(c, qir_inst(QOP_ROT_MUL, c->undef,
                                        val,
                                        qir_reg(QFILE_LOAD_IMM,
                                                QPU_SMALL_IMM_MUL_ROT + rot)));
}

static inline struct qinst *
qir_MOV_cond(struct vc4_compile *c, uint8_t cond,
             struct qreg dest, struct qreg src)
{
        struct qinst *mov = qir_MOV_dest(c, dest, src);
        mov->cond = cond;
        return mov;
}

static inline struct qinst *
qir_BRANCH(struct vc4_compile *c, uint8_t cond)
{
        struct qinst *inst = qir_inst(QOP_BRANCH, c->undef, c->undef, c->undef);
        inst->cond = cond;
        qir_emit_nondef(c, inst);
        return inst;
}

#define qir_for_each_block(block, c)                                    \
        list_for_each_entry(struct qblock, block, &c->blocks, link)

#define qir_for_each_block_rev(block, c)                                \
        list_for_each_entry_rev(struct qblock, block, &c->blocks, link)

/* Loop over the non-NULL members of the successors array. */
#define qir_for_each_successor(succ, block)                             \
        for (struct qblock *succ = block->successors[0];                \
             succ != NULL;                                              \
             succ = (succ == block->successors[1] ? NULL :              \
                     block->successors[1]))

#define qir_for_each_inst(inst, block)                                  \
        list_for_each_entry(struct qinst, inst, &block->instructions, link)

#define qir_for_each_inst_rev(inst, block)                                  \
        list_for_each_entry_rev(struct qinst, inst, &block->instructions, link)

#define qir_for_each_inst_safe(inst, block)                             \
        list_for_each_entry_safe(struct qinst, inst, &block->instructions, link)

#define qir_for_each_inst_inorder(inst, c)                              \
        qir_for_each_block(_block, c)                                   \
                qir_for_each_inst_safe(inst, _block)

#endif /* VC4_QIR_H */
