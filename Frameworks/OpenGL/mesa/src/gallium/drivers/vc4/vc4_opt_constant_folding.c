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
 * @file vc4_opt_constant_folding.c
 *
 * Simple constant folding pass to clean up operations on only constants,
 * which we might have generated within vc4_program.c.
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
constant_fold(struct vc4_compile *c, struct qinst *inst)
{
        int nsrc = qir_get_nsrc(inst);
        uint32_t ui[nsrc];

        for (int i = 0; i < nsrc; i++) {
                struct qreg reg = inst->src[i];
                if (reg.file == QFILE_UNIF &&
                    c->uniform_contents[reg.index] == QUNIFORM_CONSTANT) {
                        ui[i] = c->uniform_data[reg.index];
                } else if (reg.file == QFILE_SMALL_IMM) {
                        ui[i] = reg.index;
                } else {
                        return false;
                }
        }

        uint32_t result = 0;
        switch (inst->op) {
        case QOP_SHR:
                result = ui[0] >> ui[1];
                break;

        default:
                return false;
        }

        dump_from(c, inst);

        inst->src[0] = qir_uniform_ui(c, result);
        for (int i = 1; i < nsrc; i++)
                inst->src[i] = c->undef;
        inst->op = QOP_MOV;

        dump_to(c, inst);
        return true;
}

bool
qir_opt_constant_folding(struct vc4_compile *c)
{
        bool progress = false;

        qir_for_each_inst_inorder(inst, c) {
                if (constant_fold(c, inst))
                        progress = true;
        }

        return progress;
}
