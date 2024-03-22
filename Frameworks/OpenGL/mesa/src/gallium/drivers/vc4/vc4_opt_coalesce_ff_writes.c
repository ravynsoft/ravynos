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
 * @file vc4_opt_coalesce_ff_writes.c
 *
 * This modifies instructions that generate the value consumed by a VPM or TMU
 * coordinate write to write directly into the VPM or TMU.
 */

#include "vc4_qir.h"

bool
qir_opt_coalesce_ff_writes(struct vc4_compile *c)
{
        /* For now, only do this pass when we don't have control flow. */
        struct qblock *block = qir_entry_block(c);
        if (block != qir_exit_block(c))
                return false;

        bool progress = false;
        uint32_t use_count[c->num_temps];
        memset(&use_count, 0, sizeof(use_count));

        qir_for_each_inst_inorder(inst, c) {
                for (int i = 0; i < qir_get_nsrc(inst); i++) {
                        if (inst->src[i].file == QFILE_TEMP) {
                                uint32_t temp = inst->src[i].index;
                                use_count[temp]++;
                        }
                }
        }

        qir_for_each_inst_inorder(mov_inst, c) {
                if (!qir_is_raw_mov(mov_inst) || mov_inst->sf)
                        continue;
                if (mov_inst->src[0].file != QFILE_TEMP)
                        continue;

                if (!(mov_inst->dst.file == QFILE_VPM ||
                      mov_inst->dst.file == QFILE_TLB_COLOR_WRITE ||
                      mov_inst->dst.file == QFILE_TLB_COLOR_WRITE_MS ||
                      qir_is_tex(mov_inst)))
                        continue;

                uint32_t temp = mov_inst->src[0].index;
                if (use_count[temp] != 1)
                        continue;

                struct qinst *inst = c->defs[temp];
                if (!inst)
                        continue;

                /* Don't bother trying to fold in an ALU op using a uniform to
                 * a texture op, as we'll just have to lower the uniform back
                 * out.
                 */
                if (qir_is_tex(mov_inst) && qir_has_uniform_read(inst))
                        continue;

                if (qir_depends_on_flags(inst) || inst->sf)
                        continue;

                if (qir_has_side_effects(c, inst) ||
                    qir_has_side_effect_reads(c, inst) ||
                    inst->op == QOP_TLB_COLOR_READ ||
                    inst->op == QOP_VARY_ADD_C) {
                        continue;
                }

                /* Move the generating instruction into the position of the FF
                 * write.
                 */
                c->defs[inst->dst.index] = NULL;
                inst->dst.file = mov_inst->dst.file;
                inst->dst.index = mov_inst->dst.index;
                if (qir_has_implicit_tex_uniform(mov_inst)) {
                        inst->src[qir_get_tex_uniform_src(inst)] =
                                mov_inst->src[qir_get_tex_uniform_src(mov_inst)];
                }

                list_del(&inst->link);
                list_addtail(&inst->link, &mov_inst->link);

                qir_remove_instruction(c, mov_inst);

                progress = true;
        }

        return progress;
}
