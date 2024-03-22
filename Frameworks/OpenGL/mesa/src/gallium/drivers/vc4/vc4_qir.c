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

#include "util/u_memory.h"
#include "util/ralloc.h"

#include "vc4_qir.h"
#include "vc4_qpu.h"

struct qir_op_info {
        const char *name;
        uint8_t ndst, nsrc;
        bool has_side_effects;
};

static const struct qir_op_info qir_op_info[] = {
        [QOP_MOV] = { "mov", 1, 1 },
        [QOP_FMOV] = { "fmov", 1, 1 },
        [QOP_MMOV] = { "mmov", 1, 1 },
        [QOP_FADD] = { "fadd", 1, 2 },
        [QOP_FSUB] = { "fsub", 1, 2 },
        [QOP_FMUL] = { "fmul", 1, 2 },
        [QOP_MUL24] = { "mul24", 1, 2 },
        [QOP_V8MULD] = {"v8muld", 1, 2 },
        [QOP_V8MIN] = {"v8min", 1, 2 },
        [QOP_V8MAX] = {"v8max", 1, 2 },
        [QOP_V8ADDS] = {"v8adds", 1, 2 },
        [QOP_V8SUBS] = {"v8subs", 1, 2 },
        [QOP_FMIN] = { "fmin", 1, 2 },
        [QOP_FMAX] = { "fmax", 1, 2 },
        [QOP_FMINABS] = { "fminabs", 1, 2 },
        [QOP_FMAXABS] = { "fmaxabs", 1, 2 },
        [QOP_FTOI] = { "ftoi", 1, 1 },
        [QOP_ITOF] = { "itof", 1, 1 },
        [QOP_ADD] = { "add", 1, 2 },
        [QOP_SUB] = { "sub", 1, 2 },
        [QOP_SHR] = { "shr", 1, 2 },
        [QOP_ASR] = { "asr", 1, 2 },
        [QOP_SHL] = { "shl", 1, 2 },
        [QOP_MIN] = { "min", 1, 2 },
        [QOP_MIN_NOIMM] = { "min_noimm", 1, 2 },
        [QOP_MAX] = { "max", 1, 2 },
        [QOP_AND] = { "and", 1, 2 },
        [QOP_OR] = { "or", 1, 2 },
        [QOP_XOR] = { "xor", 1, 2 },
        [QOP_NOT] = { "not", 1, 1 },

        [QOP_RCP] = { "rcp", 1, 1 },
        [QOP_RSQ] = { "rsq", 1, 1 },
        [QOP_EXP2] = { "exp2", 1, 1 },
        [QOP_LOG2] = { "log2", 1, 1 },
        [QOP_TLB_COLOR_READ] = { "tlb_color_read", 1, 0 },
        [QOP_MS_MASK] = { "ms_mask", 0, 1, true },
        [QOP_VARY_ADD_C] = { "vary_add_c", 1, 1 },

        [QOP_FRAG_Z] = { "frag_z", 1, 0 },
        [QOP_FRAG_W] = { "frag_w", 1, 0 },

        [QOP_TEX_RESULT] = { "tex_result", 1, 0, true },

        [QOP_THRSW] = { "thrsw", 0, 0, true },

        [QOP_LOAD_IMM] = { "load_imm", 0, 1 },
        [QOP_LOAD_IMM_U2] = { "load_imm_u2", 0, 1 },
        [QOP_LOAD_IMM_I2] = { "load_imm_i2", 0, 1 },

        [QOP_ROT_MUL] = { "rot_mul", 0, 2 },

        [QOP_BRANCH] = { "branch", 0, 0, true },
        [QOP_UNIFORMS_RESET] = { "uniforms_reset", 0, 2, true },
};

static const char *
qir_get_op_name(enum qop qop)
{
        if (qop < ARRAY_SIZE(qir_op_info) && qir_op_info[qop].name)
                return qir_op_info[qop].name;
        else
                return "???";
}

int
qir_get_non_sideband_nsrc(struct qinst *inst)
{
        assert(qir_op_info[inst->op].name);
        return qir_op_info[inst->op].nsrc;
}

int
qir_get_nsrc(struct qinst *inst)
{
        assert(qir_op_info[inst->op].name);

        int nsrc = qir_get_non_sideband_nsrc(inst);

        /* Normal (non-direct) texture coordinate writes also implicitly load
         * a uniform for the texture parameters.
         */
        if (qir_is_tex(inst) && inst->dst.file != QFILE_TEX_S_DIRECT)
                nsrc++;

        return nsrc;
}

/* The sideband uniform for textures gets stored after the normal ALU
 * arguments.
 */
int
qir_get_tex_uniform_src(struct qinst *inst)
{
        return qir_get_nsrc(inst) - 1;
}

/**
 * Returns whether the instruction has any side effects that must be
 * preserved.
 */
bool
qir_has_side_effects(struct vc4_compile *c, struct qinst *inst)
{
        switch (inst->dst.file) {
        case QFILE_TLB_Z_WRITE:
        case QFILE_TLB_COLOR_WRITE:
        case QFILE_TLB_COLOR_WRITE_MS:
        case QFILE_TLB_STENCIL_SETUP:
        case QFILE_TEX_S_DIRECT:
        case QFILE_TEX_S:
        case QFILE_TEX_T:
        case QFILE_TEX_R:
        case QFILE_TEX_B:
                return true;
        default:
                break;
        }

        return qir_op_info[inst->op].has_side_effects;
}

bool
qir_has_side_effect_reads(struct vc4_compile *c, struct qinst *inst)
{
        /* We can dead-code eliminate varyings, because we only tell the VS
         * about the live ones at the end.  But we have to preserve the
         * point/line coordinates reads, because they're generated by
         * fixed-function hardware.
         */
        for (int i = 0; i < qir_get_nsrc(inst); i++) {
                if (inst->src[i].file == QFILE_VARY &&
                    c->input_slots[inst->src[i].index].slot == 0xff) {
                        return true;
                }

                if (inst->src[i].file == QFILE_VPM)
                        return true;
        }

        if (inst->dst.file == QFILE_VPM)
                return true;

        return false;
}

bool
qir_has_uniform_read(struct qinst *inst)
{
        for (int i = 0; i < qir_get_nsrc(inst); i++) {
                if (inst->src[i].file == QFILE_UNIF)
                        return true;
        }

        return false;
}

bool
qir_is_mul(struct qinst *inst)
{
        switch (inst->op) {
        case QOP_MMOV:
        case QOP_FMUL:
        case QOP_MUL24:
        case QOP_V8MULD:
        case QOP_V8MIN:
        case QOP_V8MAX:
        case QOP_V8ADDS:
        case QOP_V8SUBS:
        case QOP_ROT_MUL:
                return true;
        default:
                return false;
        }
}

bool
qir_is_float_input(struct qinst *inst)
{
        switch (inst->op) {
        case QOP_FMOV:
        case QOP_FMUL:
        case QOP_FADD:
        case QOP_FSUB:
        case QOP_FMIN:
        case QOP_FMAX:
        case QOP_FMINABS:
        case QOP_FMAXABS:
        case QOP_FTOI:
                return true;
        default:
                return false;
        }
}

bool
qir_is_raw_mov(struct qinst *inst)
{
        return ((inst->op == QOP_MOV ||
                 inst->op == QOP_FMOV ||
                 inst->op == QOP_MMOV) &&
                inst->cond == QPU_COND_ALWAYS &&
                !inst->dst.pack &&
                !inst->src[0].pack);
}

bool
qir_is_tex(struct qinst *inst)
{
        switch (inst->dst.file) {
        case QFILE_TEX_S_DIRECT:
        case QFILE_TEX_S:
        case QFILE_TEX_T:
        case QFILE_TEX_R:
        case QFILE_TEX_B:
                return true;
        default:
                return false;
        }
}

bool
qir_has_implicit_tex_uniform(struct qinst *inst)
{
        switch (inst->dst.file) {
        case QFILE_TEX_S:
        case QFILE_TEX_T:
        case QFILE_TEX_R:
        case QFILE_TEX_B:
                return true;
        default:
                return false;
        }
}

bool
qir_depends_on_flags(struct qinst *inst)
{
        if (inst->op == QOP_BRANCH) {
                return inst->cond != QPU_COND_BRANCH_ALWAYS;
        } else {
                return (inst->cond != QPU_COND_ALWAYS &&
                        inst->cond != QPU_COND_NEVER);
        }
}

bool
qir_writes_r4(struct qinst *inst)
{
        switch (inst->op) {
        case QOP_TEX_RESULT:
        case QOP_TLB_COLOR_READ:
        case QOP_RCP:
        case QOP_RSQ:
        case QOP_EXP2:
        case QOP_LOG2:
                return true;
        default:
                return false;
        }
}

uint8_t
qir_channels_written(struct qinst *inst)
{
        if (qir_is_mul(inst)) {
                switch (inst->dst.pack) {
                case QPU_PACK_MUL_NOP:
                case QPU_PACK_MUL_8888:
                        return 0xf;
                case QPU_PACK_MUL_8A:
                        return 0x1;
                case QPU_PACK_MUL_8B:
                        return 0x2;
                case QPU_PACK_MUL_8C:
                        return 0x4;
                case QPU_PACK_MUL_8D:
                        return 0x8;
                }
        } else {
                switch (inst->dst.pack) {
                case QPU_PACK_A_NOP:
                case QPU_PACK_A_8888:
                case QPU_PACK_A_8888_SAT:
                case QPU_PACK_A_32_SAT:
                        return 0xf;
                case QPU_PACK_A_8A:
                case QPU_PACK_A_8A_SAT:
                        return 0x1;
                case QPU_PACK_A_8B:
                case QPU_PACK_A_8B_SAT:
                        return 0x2;
                case QPU_PACK_A_8C:
                case QPU_PACK_A_8C_SAT:
                        return 0x4;
                case QPU_PACK_A_8D:
                case QPU_PACK_A_8D_SAT:
                        return 0x8;
                case QPU_PACK_A_16A:
                case QPU_PACK_A_16A_SAT:
                        return 0x3;
                case QPU_PACK_A_16B:
                case QPU_PACK_A_16B_SAT:
                        return 0xc;
                }
        }
        unreachable("Bad pack field");
}

char *
qir_describe_uniform(enum quniform_contents contents, uint32_t data,
                     const uint32_t *uniforms)
{
        static const char *quniform_names[] = {
                [QUNIFORM_VIEWPORT_X_SCALE] = "vp_x_scale",
                [QUNIFORM_VIEWPORT_Y_SCALE] = "vp_y_scale",
                [QUNIFORM_VIEWPORT_Z_OFFSET] = "vp_z_offset",
                [QUNIFORM_VIEWPORT_Z_SCALE] = "vp_z_scale",
                [QUNIFORM_TEXTURE_CONFIG_P0] = "tex_p0",
                [QUNIFORM_TEXTURE_CONFIG_P1] = "tex_p1",
                [QUNIFORM_TEXTURE_CONFIG_P2] = "tex_p2",
                [QUNIFORM_TEXTURE_FIRST_LEVEL] = "tex_first_level",
        };

        switch (contents) {
        case QUNIFORM_CONSTANT:
                return ralloc_asprintf(NULL, "0x%08x / %f", data, uif(data));
        case QUNIFORM_UNIFORM:
                if (uniforms) {
                        uint32_t unif = uniforms[data];
                        return ralloc_asprintf(NULL, "unif[%d] = 0x%08x / %f",
                                               data, unif, uif(unif));
                } else {
                        return ralloc_asprintf(NULL, "unif[%d]", data);
                }

        case QUNIFORM_TEXTURE_CONFIG_P0:
        case QUNIFORM_TEXTURE_CONFIG_P1:
        case QUNIFORM_TEXTURE_CONFIG_P2:
        case QUNIFORM_TEXTURE_FIRST_LEVEL:
                return ralloc_asprintf(NULL, "%s[%d]",
                                       quniform_names[contents], data);

        default:
                if (contents < ARRAY_SIZE(quniform_names) &&
                    quniform_names[contents]) {
                        return ralloc_asprintf(NULL, "%s",
                                               quniform_names[contents]);
                } else {
                        return ralloc_asprintf(NULL, "??? %d", contents);
                }
        }
}

static void
qir_print_reg(struct vc4_compile *c, struct qreg reg, bool write)
{
        static const char *files[] = {
                [QFILE_TEMP] = "t",
                [QFILE_VARY] = "v",
                [QFILE_TLB_COLOR_WRITE] = "tlb_c",
                [QFILE_TLB_COLOR_WRITE_MS] = "tlb_c_ms",
                [QFILE_TLB_Z_WRITE] = "tlb_z",
                [QFILE_TLB_STENCIL_SETUP] = "tlb_stencil",
                [QFILE_FRAG_X] = "frag_x",
                [QFILE_FRAG_Y] = "frag_y",
                [QFILE_FRAG_REV_FLAG] = "frag_rev_flag",
                [QFILE_QPU_ELEMENT] = "elem",
                [QFILE_TEX_S_DIRECT] = "tex_s_direct",
                [QFILE_TEX_S] = "tex_s",
                [QFILE_TEX_T] = "tex_t",
                [QFILE_TEX_R] = "tex_r",
                [QFILE_TEX_B] = "tex_b",
        };

        switch (reg.file) {

        case QFILE_NULL:
                fprintf(stderr, "null");
                break;

        case QFILE_LOAD_IMM:
                fprintf(stderr, "0x%08x (%f)", reg.index, uif(reg.index));
                break;

        case QFILE_SMALL_IMM:
                if ((int)reg.index >= -16 && (int)reg.index <= 15)
                        fprintf(stderr, "%d", reg.index);
                else
                        fprintf(stderr, "%f", uif(reg.index));
                break;

        case QFILE_VPM:
                if (write) {
                        fprintf(stderr, "vpm");
                } else {
                        fprintf(stderr, "vpm%d.%d",
                                reg.index / 4, reg.index % 4);
                }
                break;

        case QFILE_TLB_COLOR_WRITE:
        case QFILE_TLB_COLOR_WRITE_MS:
        case QFILE_TLB_Z_WRITE:
        case QFILE_TLB_STENCIL_SETUP:
        case QFILE_TEX_S_DIRECT:
        case QFILE_TEX_S:
        case QFILE_TEX_T:
        case QFILE_TEX_R:
        case QFILE_TEX_B:
                fprintf(stderr, "%s", files[reg.file]);
                break;

        case QFILE_UNIF: {
                char *desc = qir_describe_uniform(c->uniform_contents[reg.index],
                                                  c->uniform_data[reg.index],
                                                  NULL);
                fprintf(stderr, "u%d (%s)", reg.index, desc);
                ralloc_free(desc);
                break;
        }

        default:
                fprintf(stderr, "%s%d", files[reg.file], reg.index);
                break;
        }
}

void
qir_dump_inst(struct vc4_compile *c, struct qinst *inst)
{
        fprintf(stderr, "%s", qir_get_op_name(inst->op));
        if (inst->op == QOP_BRANCH)
                vc4_qpu_disasm_cond_branch(stderr, inst->cond);
        else
                vc4_qpu_disasm_cond(stderr, inst->cond);
        if (inst->sf)
                fprintf(stderr, ".sf");
        fprintf(stderr, " ");

        if (inst->op != QOP_BRANCH) {
                qir_print_reg(c, inst->dst, true);
                if (inst->dst.pack) {
                        if (inst->dst.pack) {
                                if (qir_is_mul(inst))
                                        vc4_qpu_disasm_pack_mul(stderr, inst->dst.pack);
                                else
                                        vc4_qpu_disasm_pack_a(stderr, inst->dst.pack);
                        }
                }
        }

        for (int i = 0; i < qir_get_nsrc(inst); i++) {
                fprintf(stderr, ", ");
                qir_print_reg(c, inst->src[i], false);
                vc4_qpu_disasm_unpack(stderr, inst->src[i].pack);
        }
}

void
qir_dump(struct vc4_compile *c)
{
        int ip = 0;
        int pressure = 0;

        qir_for_each_block(block, c) {
                fprintf(stderr, "BLOCK %d:\n", block->index);
                qir_for_each_inst(inst, block) {
                        if (c->temp_start) {
                                bool first = true;

                                fprintf(stderr, "%3d ", pressure);

                                for (int i = 0; i < c->num_temps; i++) {
                                        if (c->temp_start[i] != ip)
                                                continue;

                                        if (first) {
                                                first = false;
                                        } else {
                                                fprintf(stderr, ", ");
                                        }
                                        fprintf(stderr, "S%4d", i);
                                        pressure++;
                                }

                                if (first)
                                        fprintf(stderr, "      ");
                                else
                                        fprintf(stderr, " ");
                        }

                        if (c->temp_end) {
                                bool first = true;

                                for (int i = 0; i < c->num_temps; i++) {
                                        if (c->temp_end[i] != ip)
                                                continue;

                                        if (first) {
                                                first = false;
                                        } else {
                                                fprintf(stderr, ", ");
                                        }
                                        fprintf(stderr, "E%4d", i);
                                        pressure--;
                                }

                                if (first)
                                        fprintf(stderr, "      ");
                                else
                                        fprintf(stderr, " ");
                        }

                        qir_dump_inst(c, inst);
                        fprintf(stderr, "\n");
                        ip++;
                }
                if (block->successors[1]) {
                        fprintf(stderr, "-> BLOCK %d, %d\n",
                                block->successors[0]->index,
                                block->successors[1]->index);
                } else if (block->successors[0]) {
                        fprintf(stderr, "-> BLOCK %d\n",
                                block->successors[0]->index);
                }
        }
}

struct qreg
qir_get_temp(struct vc4_compile *c)
{
        struct qreg reg;

        reg.file = QFILE_TEMP;
        reg.index = c->num_temps++;
        reg.pack = 0;

        if (c->num_temps > c->defs_array_size) {
                uint32_t old_size = c->defs_array_size;
                c->defs_array_size = MAX2(old_size * 2, 16);
                c->defs = reralloc(c, c->defs, struct qinst *,
                                   c->defs_array_size);
                memset(&c->defs[old_size], 0,
                       sizeof(c->defs[0]) * (c->defs_array_size - old_size));
        }

        return reg;
}

struct qinst *
qir_inst(enum qop op, struct qreg dst, struct qreg src0, struct qreg src1)
{
        struct qinst *inst = CALLOC_STRUCT(qinst);

        inst->op = op;
        inst->dst = dst;
        inst->src[0] = src0;
        inst->src[1] = src1;
        inst->cond = QPU_COND_ALWAYS;

        return inst;
}

static void
qir_emit(struct vc4_compile *c, struct qinst *inst)
{
        list_addtail(&inst->link, &c->cur_block->instructions);
}

/* Updates inst to write to a new temporary, emits it, and notes the def. */
struct qreg
qir_emit_def(struct vc4_compile *c, struct qinst *inst)
{
        assert(inst->dst.file == QFILE_NULL);

        inst->dst = qir_get_temp(c);

        if (inst->dst.file == QFILE_TEMP)
                c->defs[inst->dst.index] = inst;

        qir_emit(c, inst);

        return inst->dst;
}

struct qinst *
qir_emit_nondef(struct vc4_compile *c, struct qinst *inst)
{
        if (inst->dst.file == QFILE_TEMP)
                c->defs[inst->dst.index] = NULL;

        qir_emit(c, inst);

        return inst;
}

bool
qir_reg_equals(struct qreg a, struct qreg b)
{
        return a.file == b.file && a.index == b.index && a.pack == b.pack;
}

struct qblock *
qir_new_block(struct vc4_compile *c)
{
        struct qblock *block = rzalloc(c, struct qblock);

        list_inithead(&block->instructions);
        list_inithead(&block->qpu_inst_list);

        block->predecessors = _mesa_set_create(block,
                                               _mesa_hash_pointer,
                                               _mesa_key_pointer_equal);

        block->index = c->next_block_index++;

        return block;
}

void
qir_set_emit_block(struct vc4_compile *c, struct qblock *block)
{
        c->cur_block = block;
        list_addtail(&block->link, &c->blocks);
}

struct qblock *
qir_entry_block(struct vc4_compile *c)
{
        return list_first_entry(&c->blocks, struct qblock, link);
}

struct qblock *
qir_exit_block(struct vc4_compile *c)
{
        return list_last_entry(&c->blocks, struct qblock, link);
}

void
qir_link_blocks(struct qblock *predecessor, struct qblock *successor)
{
        _mesa_set_add(successor->predecessors, predecessor);
        if (predecessor->successors[0]) {
                assert(!predecessor->successors[1]);
                predecessor->successors[1] = successor;
        } else {
                predecessor->successors[0] = successor;
        }
}

struct vc4_compile *
qir_compile_init(void)
{
        struct vc4_compile *c = rzalloc(NULL, struct vc4_compile);

        list_inithead(&c->blocks);
        qir_set_emit_block(c, qir_new_block(c));
        c->last_top_block = c->cur_block;

        c->output_position_index = -1;
        c->output_color_index = -1;
        c->output_point_size_index = -1;
        c->output_sample_mask_index = -1;

        c->def_ht = _mesa_hash_table_create(c, _mesa_hash_pointer,
                                            _mesa_key_pointer_equal);

        return c;
}

void
qir_remove_instruction(struct vc4_compile *c, struct qinst *qinst)
{
        if (qinst->dst.file == QFILE_TEMP)
                c->defs[qinst->dst.index] = NULL;

        list_del(&qinst->link);
        free(qinst);
}

struct qreg
qir_follow_movs(struct vc4_compile *c, struct qreg reg)
{
        int pack = reg.pack;

        while (reg.file == QFILE_TEMP &&
               c->defs[reg.index] &&
               (c->defs[reg.index]->op == QOP_MOV ||
                c->defs[reg.index]->op == QOP_FMOV ||
                c->defs[reg.index]->op == QOP_MMOV)&&
               !c->defs[reg.index]->dst.pack &&
               !c->defs[reg.index]->src[0].pack) {
                reg = c->defs[reg.index]->src[0];
        }

        reg.pack = pack;
        return reg;
}

void
qir_compile_destroy(struct vc4_compile *c)
{
        qir_for_each_block(block, c) {
                while (!list_is_empty(&block->instructions)) {
                        struct qinst *qinst =
                                list_first_entry(&block->instructions,
                                                 struct qinst, link);
                        qir_remove_instruction(c, qinst);
                }
        }

        ralloc_free(c);
}

const char *
qir_get_stage_name(enum qstage stage)
{
        static const char *names[] = {
                [QSTAGE_FRAG] = "MESA_SHADER_FRAGMENT",
                [QSTAGE_VERT] = "MESA_SHADER_VERTEX",
                [QSTAGE_COORD] = "MESA_SHADER_COORD",
        };

        return names[stage];
}

struct qreg
qir_uniform(struct vc4_compile *c,
            enum quniform_contents contents,
            uint32_t data)
{
        for (int i = 0; i < c->num_uniforms; i++) {
                if (c->uniform_contents[i] == contents &&
                    c->uniform_data[i] == data) {
                        return qir_reg(QFILE_UNIF, i);
                }
        }

        uint32_t uniform = c->num_uniforms++;

        if (uniform >= c->uniform_array_size) {
                c->uniform_array_size = MAX2(MAX2(16, uniform + 1),
                                             c->uniform_array_size * 2);

                c->uniform_data = reralloc(c, c->uniform_data,
                                           uint32_t,
                                           c->uniform_array_size);
                c->uniform_contents = reralloc(c, c->uniform_contents,
                                               enum quniform_contents,
                                               c->uniform_array_size);
        }

        c->uniform_contents[uniform] = contents;
        c->uniform_data[uniform] = data;

        return qir_reg(QFILE_UNIF, uniform);
}

void
qir_SF(struct vc4_compile *c, struct qreg src)
{
        struct qinst *last_inst = NULL;

        if (!list_is_empty(&c->cur_block->instructions))
                last_inst = (struct qinst *)c->cur_block->instructions.prev;

        /* We don't have any way to guess which kind of MOV is implied. */
        assert(!src.pack);

        if (src.file != QFILE_TEMP ||
            !c->defs[src.index] ||
            last_inst != c->defs[src.index]) {
                last_inst = qir_MOV_dest(c, qir_reg(QFILE_NULL, 0), src);
        }
        last_inst->sf = true;
}

#define OPTPASS(func)                                                   \
        do {                                                            \
                bool stage_progress = func(c);                          \
                if (stage_progress) {                                   \
                        progress = true;                                \
                        if (print_opt_debug) {                          \
                                fprintf(stderr,                         \
                                        "QIR opt pass %2d: %s progress\n", \
                                        pass, #func);                   \
                        }                                               \
                        qir_validate(c);                                \
                }                                                       \
        } while (0)

void
qir_optimize(struct vc4_compile *c)
{
        bool print_opt_debug = false;
        int pass = 1;

        while (true) {
                bool progress = false;

                OPTPASS(qir_opt_algebraic);
                OPTPASS(qir_opt_constant_folding);
                OPTPASS(qir_opt_copy_propagation);
                OPTPASS(qir_opt_peephole_sf);
                OPTPASS(qir_opt_dead_code);
                OPTPASS(qir_opt_small_immediates);
                OPTPASS(qir_opt_vpm);
                OPTPASS(qir_opt_coalesce_ff_writes);

                if (!progress)
                        break;

                pass++;
        }
}
