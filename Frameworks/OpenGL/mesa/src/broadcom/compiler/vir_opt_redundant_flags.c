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
 * @file v3d_opt_redundant_flags.c
 *
 * This eliminates the APF/MPF flags for redundant flags updates.  These are
 * often produced by our channel masking in nonuniform control flow.
 */

#include "v3d_compiler.h"

static bool debug;

static void
vir_dce_pf(struct v3d_compile *c, struct qinst *inst)
{
        if (debug) {
                fprintf(stderr,
                        "Removing flags write from: ");
                vir_dump_inst(c, inst);
                fprintf(stderr, "\n");
        }

        assert(inst->qpu.type == V3D_QPU_INSTR_TYPE_ALU);

        inst->qpu.flags.apf = V3D_QPU_PF_NONE;
        inst->qpu.flags.mpf = V3D_QPU_PF_NONE;
}

static bool
vir_sources_modified(struct qinst *srcs, struct qinst *write)
{
        for (int i = 0; i < vir_get_nsrc(srcs); i++) {
                if (write->dst.file == QFILE_TEMP &&
                    srcs->src[i].file == QFILE_TEMP &&
                    srcs->src[i].index == write->dst.index) {
                        return true;
                }

                /* assume magic regs may be modified by basically anything. */
                if (srcs->src[i].file != QFILE_TEMP &&
                    srcs->src[i].file != QFILE_SMALL_IMM)
                        return true;
        }

        return false;
}

static bool
vir_instr_flags_op_equal(struct qinst *a, struct qinst *b)
{
        for (int i = 0; i < vir_get_nsrc(a); i++) {
                if (a->src[i].file != b->src[i].file ||
                    a->src[i].index != b->src[i].index) {
                        return false;
                }
        }

        if (a->qpu.flags.apf != b->qpu.flags.apf ||
            a->qpu.flags.mpf != b->qpu.flags.mpf ||
            a->qpu.alu.add.op != b->qpu.alu.add.op ||
            a->qpu.alu.mul.op != b->qpu.alu.mul.op ||
            a->qpu.alu.add.a.unpack != b->qpu.alu.add.a.unpack ||
            a->qpu.alu.add.b.unpack != b->qpu.alu.add.b.unpack ||
            a->qpu.alu.add.output_pack != b->qpu.alu.add.output_pack ||
            a->qpu.alu.mul.a.unpack != b->qpu.alu.mul.a.unpack ||
            a->qpu.alu.mul.b.unpack != b->qpu.alu.mul.b.unpack ||
            a->qpu.alu.mul.output_pack != b->qpu.alu.mul.output_pack) {
                return false;
        }

        return true;
}

static bool
vir_opt_redundant_flags_block(struct v3d_compile *c, struct qblock *block)
{
        struct qinst *last_flags = NULL;
        bool progress = false;

        c->cur_block = block;
        vir_for_each_inst(inst, block) {
                if (inst->qpu.type != V3D_QPU_INSTR_TYPE_ALU ||
                    inst->qpu.flags.auf != V3D_QPU_UF_NONE ||
                    inst->qpu.flags.muf != V3D_QPU_UF_NONE) {
                        last_flags = NULL;
                        continue;
                }

                /* Flags aren't preserved across a thrsw.
                 *
                 * In V3D 4.2+ flags are preserved across thread switches.
                 */
                if (c->devinfo->ver < 42) {
                        if (inst->qpu.sig.thrsw)
                                last_flags = NULL;
                }

                if (inst->qpu.flags.apf != V3D_QPU_PF_NONE ||
                    inst->qpu.flags.mpf != V3D_QPU_PF_NONE) {
                        if (last_flags &&
                            vir_instr_flags_op_equal(inst, last_flags)) {
                                vir_dce_pf(c, inst);
                                progress = true;
                        } else {
                                last_flags = inst;
                        }
                }

                if (last_flags && vir_sources_modified(last_flags, inst)) {
                        last_flags = NULL;
                }
        }

        return progress;
}

bool
vir_opt_redundant_flags(struct v3d_compile *c)
{
        bool progress = false;

        vir_for_each_block(block, c) {
                progress = vir_opt_redundant_flags_block(c, block) || progress;
        }

        return progress;
}
