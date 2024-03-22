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

/**
 * @file vc4_opt_algebraic.c
 *
 * This is the optimization pass for miscellaneous changes to instructions
 * where we can simplify the operation by some knowledge about the specific
 * operations.
 *
 * Mostly this will be a matter of turning things into MOVs so that they can
 * later be copy-propagated out.
 */

#include "vc4_qir.h"
#include "util/u_math.h"

static bool debug;

static void
dump_from(struct vc4_compile *c, struct qinst *inst)
{
        if (!debug)
                return;

        fprintf(stderr, "optimizing: ");
        qir_dump_inst(c, inst);
        fprintf(stderr, "\n");
}

static void
dump_to(struct vc4_compile *c, struct qinst *inst)
{
        if (!debug)
                return;

        fprintf(stderr, "to: ");
        qir_dump_inst(c, inst);
        fprintf(stderr, "\n");
}

static bool
is_constant_value(struct vc4_compile *c, struct qreg reg,
                  uint32_t val)
{
        if (reg.file == QFILE_UNIF &&
            !reg.pack &&
            c->uniform_contents[reg.index] == QUNIFORM_CONSTANT &&
            c->uniform_data[reg.index] == val) {
                return true;
        }

        if (reg.file == QFILE_SMALL_IMM && reg.index == val)
                return true;

        return false;
}

static bool
is_zero(struct vc4_compile *c, struct qreg reg)
{
        reg = qir_follow_movs(c, reg);
        return is_constant_value(c, reg, 0);
}

static bool
is_1f(struct vc4_compile *c, struct qreg reg)
{
        reg = qir_follow_movs(c, reg);
        return is_constant_value(c, reg, fui(1.0));
}

static void
replace_with_mov(struct vc4_compile *c, struct qinst *inst, struct qreg arg)
{
        dump_from(c, inst);

        inst->src[0] = arg;
        if (qir_has_implicit_tex_uniform(inst))
                inst->src[1] = inst->src[qir_get_tex_uniform_src(inst)];

        if (qir_is_mul(inst))
                inst->op = QOP_MMOV;
        else if (qir_is_float_input(inst))
                inst->op = QOP_FMOV;
        else
                inst->op = QOP_MOV;
        dump_to(c, inst);
}

static bool
replace_x_0_with_x(struct vc4_compile *c,
                 struct qinst *inst,
                 int arg)
{
        if (!is_zero(c, inst->src[arg]))
                return false;
        replace_with_mov(c, inst, inst->src[1 - arg]);
        return true;
}

static bool
replace_x_0_with_0(struct vc4_compile *c,
                  struct qinst *inst,
                  int arg)
{
        if (!is_zero(c, inst->src[arg]))
                return false;
        replace_with_mov(c, inst, inst->src[arg]);
        return true;
}

static bool
fmul_replace_one(struct vc4_compile *c,
                 struct qinst *inst,
                 int arg)
{
        if (!is_1f(c, inst->src[arg]))
                return false;
        replace_with_mov(c, inst, inst->src[1 - arg]);
        return true;
}

bool
qir_opt_algebraic(struct vc4_compile *c)
{
        bool progress = false;

        qir_for_each_inst_inorder(inst, c) {
                switch (inst->op) {
                case QOP_FMIN:
                        if (is_1f(c, inst->src[1]) &&
                            inst->src[0].pack >= QPU_UNPACK_8D_REP &&
                            inst->src[0].pack <= QPU_UNPACK_8D) {
                                replace_with_mov(c, inst, inst->src[0]);
                                progress = true;
                        }
                        break;

                case QOP_FMAX:
                        if (is_zero(c, inst->src[1]) &&
                            inst->src[0].pack >= QPU_UNPACK_8D_REP &&
                            inst->src[0].pack <= QPU_UNPACK_8D) {
                                replace_with_mov(c, inst, inst->src[0]);
                                progress = true;
                        }
                        break;

                case QOP_FSUB:
                case QOP_SUB:
                        if (is_zero(c, inst->src[1])) {
                                replace_with_mov(c, inst, inst->src[0]);
                                progress = true;
                        }
                        break;

                case QOP_ADD:
                        /* Kernel validation requires that we use an actual
                         * add instruction.
                         */
                        if (inst->dst.file != QFILE_TEX_S_DIRECT &&
                            (replace_x_0_with_x(c, inst, 0) ||
                             replace_x_0_with_x(c, inst, 1))) {
                                progress = true;
                                break;
                        }
                        break;

                case QOP_FADD:
                        if (replace_x_0_with_x(c, inst, 0) ||
                            replace_x_0_with_x(c, inst, 1)) {
                                progress = true;
                                break;
                        }

                        /* FADD(a, FSUB(0, b)) -> FSUB(a, b) */
                        if (inst->src[1].file == QFILE_TEMP &&
                            c->defs[inst->src[1].index] &&
                            c->defs[inst->src[1].index]->op == QOP_FSUB) {
                                struct qinst *fsub = c->defs[inst->src[1].index];
                                if (is_zero(c, fsub->src[0])) {
                                        dump_from(c, inst);
                                        inst->op = QOP_FSUB;
                                        inst->src[1] = fsub->src[1];
                                        progress = true;
                                        dump_to(c, inst);
                                        break;
                                }
                        }

                        /* FADD(FSUB(0, b), a) -> FSUB(a, b) */
                        if (inst->src[0].file == QFILE_TEMP &&
                            c->defs[inst->src[0].index] &&
                            c->defs[inst->src[0].index]->op == QOP_FSUB) {
                                struct qinst *fsub = c->defs[inst->src[0].index];
                                if (is_zero(c, fsub->src[0])) {
                                        dump_from(c, inst);
                                        inst->op = QOP_FSUB;
                                        inst->src[0] = inst->src[1];
                                        inst->src[1] = fsub->src[1];
                                        dump_to(c, inst);
                                        progress = true;
                                        break;
                                }
                        }
                        break;

                case QOP_FMUL:
                        if (!inst->dst.pack &&
                            (replace_x_0_with_0(c, inst, 0) ||
                             replace_x_0_with_0(c, inst, 1) ||
                             fmul_replace_one(c, inst, 0) ||
                             fmul_replace_one(c, inst, 1))) {
                                progress = true;
                                break;
                        }
                        break;

                case QOP_MUL24:
                        if (!inst->dst.pack &&
                            (replace_x_0_with_0(c, inst, 0) ||
                             replace_x_0_with_0(c, inst, 1))) {
                                progress = true;
                                break;
                        }
                        break;

                case QOP_AND:
                        if (replace_x_0_with_0(c, inst, 0) ||
                            replace_x_0_with_0(c, inst, 1)) {
                                progress = true;
                                break;
                        }

                        if (is_constant_value(c, inst->src[0], ~0)) {
                                replace_with_mov(c, inst, inst->src[1]);
                                progress = true;
                                break;
                        }
                        if (is_constant_value(c, inst->src[1], ~0)) {
                                replace_with_mov(c, inst, inst->src[0]);
                                progress = true;
                                break;
                        }
                        break;

                case QOP_OR:
                        if (replace_x_0_with_x(c, inst, 0) ||
                            replace_x_0_with_x(c, inst, 1)) {
                                progress = true;
                                break;
                        }
                        break;

                case QOP_RCP:
                        if (is_1f(c, inst->src[0])) {
                                replace_with_mov(c, inst, inst->src[0]);
                                progress = true;
                                break;
                        }
                        break;

                default:
                        break;
                }
        }

        return progress;
}
