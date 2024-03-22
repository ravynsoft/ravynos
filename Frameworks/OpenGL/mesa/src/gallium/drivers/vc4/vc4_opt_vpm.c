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
 * @file vc4_opt_vpm.c
 *
 * This modifies instructions that exclusively consume a value read from the
 * VPM to directly read the VPM if other operands allow it.
 */

#include "vc4_qir.h"

bool
qir_opt_vpm(struct vc4_compile *c)
{
        if (c->stage == QSTAGE_FRAG)
                return false;

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

        /* For instructions reading from a temporary that contains a VPM read
         * result, try to move the instruction up in place of the VPM read.
         */
        qir_for_each_inst_inorder(inst, c) {
                if (!inst)
                        continue;

                if (qir_depends_on_flags(inst) || inst->sf)
                        continue;

                if (qir_has_side_effects(c, inst) ||
                    qir_has_side_effect_reads(c, inst) ||
                    qir_is_tex(inst))
                        continue;

                for (int j = 0; j < qir_get_nsrc(inst); j++) {
                        if (inst->src[j].file != QFILE_TEMP ||
                            inst->src[j].pack)
                                continue;

                        uint32_t temp = inst->src[j].index;

                        /* Since VPM reads pull from a FIFO, we only get to
                         * read each VPM entry once (unless we reset the read
                         * pointer).  That means we can't copy-propagate a VPM
                         * read to multiple locations.
                         */
                        if (use_count[temp] != 1)
                                continue;

                        struct qinst *mov = c->defs[temp];
                        if (!mov ||
                            (mov->op != QOP_MOV &&
                             mov->op != QOP_FMOV &&
                             mov->op != QOP_MMOV) ||
                            mov->src[0].file != QFILE_VPM) {
                                continue;
                        }

                        uint32_t temps = 0;
                        for (int k = 0; k < qir_get_nsrc(inst); k++) {
                                if (inst->src[k].file == QFILE_TEMP)
                                        temps++;
                        }

                        /* The instruction is safe to reorder if its other
                         * sources are independent of previous instructions
                         */
                        if (temps == 1) {
                                inst->src[j] = mov->src[0];

                                list_del(&inst->link);
                                list_addtail(&inst->link, &mov->link);
                                qir_remove_instruction(c, mov);

                                progress = true;
                                break;
                        }
                }
        }

        return progress;
}
