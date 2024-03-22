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
 * @file vc4_opt_copy_propagation.c
 *
 * This implements simple copy propagation for QIR without control flow.
 *
 * For each temp, it keeps a qreg of which source it was MOVed from, if it
 * was.  If we see that used later, we can just reuse the source value, since
 * we know we don't have control flow, and we have SSA for our values so
 * there's no killing to worry about.
 */

#include "vc4_qir.h"

static bool
is_copy_mov(struct qinst *inst)
{
        if (!inst)
                return false;

        if (inst->op != QOP_MOV &&
            inst->op != QOP_FMOV &&
            inst->op != QOP_MMOV) {
                return false;
        }

        if (inst->dst.file != QFILE_TEMP)
                return false;

        if (inst->src[0].file != QFILE_TEMP &&
            inst->src[0].file != QFILE_UNIF) {
                return false;
        }

        if (inst->dst.pack || inst->cond != QPU_COND_ALWAYS)
                return false;

        return true;

}

static bool
try_copy_prop(struct vc4_compile *c, struct qinst *inst, struct qinst **movs)
{
        bool debug = false;
        bool progress = false;

	for (int i = 0; i < qir_get_nsrc(inst); i++) {
                if (inst->src[i].file != QFILE_TEMP)
                        continue;

                /* We have two ways of finding MOVs we can copy propagate
                 * from.  One is if it's an SSA def: then we can reuse it from
                 * any block in the program, as long as its source is also an
                 * SSA def.  Alternatively, if it's in the "movs" array
                 * tracked within the block, then we know the sources for it
                 * haven't been changed since we saw the instruction within
                 * our block.
                 */
                struct qinst *mov = movs[inst->src[i].index];
                if (!mov) {
                        if (!is_copy_mov(c->defs[inst->src[i].index]))
                                continue;
                        mov = c->defs[inst->src[i].index];

                        if (mov->src[0].file == QFILE_TEMP &&
                            !c->defs[mov->src[0].index])
                                continue;
                }

                /* Mul rotation's source needs to be in an r0-r3 accumulator,
                 * so no uniforms or regfile-a/r4 unpacking allowed.
                 */
                if (inst->op == QOP_ROT_MUL &&
                    (mov->src[0].file != QFILE_TEMP ||
                     mov->src[0].pack))
                        continue;

                uint8_t unpack;
                if (mov->src[0].pack) {
                        /* Make sure that the meaning of the unpack
                         * would be the same between the two
                         * instructions.
                         */
                        if (qir_is_float_input(inst) !=
                            qir_is_float_input(mov)) {
                                continue;
                        }

                        /* There's only one unpack field, so make sure
                         * this instruction doesn't already use it.
                         */
                        bool already_has_unpack = false;
                        for (int j = 0; j < qir_get_nsrc(inst); j++) {
                                if (inst->src[j].pack)
                                        already_has_unpack = true;
                        }
                        if (already_has_unpack)
                                continue;

                        /* A destination pack requires the PM bit to
                         * be set to a specific value already, which
                         * may be different from ours.
                         */
                        if (inst->dst.pack)
                                continue;

                        unpack = mov->src[0].pack;
                } else {
                        unpack = inst->src[i].pack;
                }

                if (debug) {
                        fprintf(stderr, "Copy propagate: ");
                        qir_dump_inst(c, inst);
                        fprintf(stderr, "\n");
                }

                inst->src[i] = mov->src[0];
                inst->src[i].pack = unpack;

                if (debug) {
                        fprintf(stderr, "to: ");
                        qir_dump_inst(c, inst);
                        fprintf(stderr, "\n");
                }

                progress = true;
        }

        return progress;
}

static void
apply_kills(struct vc4_compile *c, struct qinst **movs, struct qinst *inst)
{
        if (inst->dst.file != QFILE_TEMP)
                return;

        for (int i = 0; i < c->num_temps; i++) {
                if (movs[i] &&
                    (movs[i]->dst.index == inst->dst.index ||
                     (movs[i]->src[0].file == QFILE_TEMP &&
                      movs[i]->src[0].index == inst->dst.index))) {
                        movs[i] = NULL;
                }
        }
}

bool
qir_opt_copy_propagation(struct vc4_compile *c)
{
        bool progress = false;
        struct qinst **movs;

        movs = ralloc_array(c, struct qinst *, c->num_temps);
        if (!movs)
                return false;

        qir_for_each_block(block, c) {
                /* The MOVs array tracks only available movs within the
                 * block.
                 */
                memset(movs, 0, sizeof(struct qinst *) * c->num_temps);

                qir_for_each_inst(inst, block) {
                        progress = try_copy_prop(c, inst, movs) || progress;

                        apply_kills(c, movs, inst);

                        if (is_copy_mov(inst))
                                movs[inst->dst.index] = inst;
                }
        }

        ralloc_free(movs);

        return progress;
}
