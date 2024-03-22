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
 * @file v3d_opt_dead_code.c
 *
 * This is a simple dead code eliminator for SSA values in VIR.
 *
 * It walks all the instructions finding what temps are used, then walks again
 * to remove instructions writing unused temps.
 *
 * This is an inefficient implementation if you have long chains of
 * instructions where the entire chain is dead, but we expect those to have
 * been eliminated at the NIR level, and here we're just cleaning up small
 * problems produced by NIR->VIR.
 */

#include "v3d_compiler.h"

static bool debug;

static void
dce(struct v3d_compile *c, struct qinst *inst)
{
        if (debug) {
                fprintf(stderr, "Removing: ");
                vir_dump_inst(c, inst);
                fprintf(stderr, "\n");
        }
        assert(!v3d_qpu_writes_flags(&inst->qpu));
        vir_remove_instruction(c, inst);
}

static bool
can_write_to_null(struct v3d_compile *c, struct qinst *inst)
{
        /* The SFU instructions must write to a physical register. */
        if (v3d_qpu_uses_sfu(&inst->qpu))
                return false;

        return true;
}

static void
vir_dce_flags(struct v3d_compile *c, struct qinst *inst)
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
        inst->qpu.flags.auf = V3D_QPU_UF_NONE;
        inst->qpu.flags.muf = V3D_QPU_UF_NONE;
}

static bool
check_last_ldunifa(struct v3d_compile *c,
                   struct qinst *inst,
                   struct qblock *block)
{
        if (!inst->qpu.sig.ldunifa && !inst->qpu.sig.ldunifarf)
                return false;

        list_for_each_entry_from(struct qinst, scan_inst, inst->link.next,
                                 &block->instructions, link) {
                /* If we find a new write to unifa, then this was the last
                 * ldunifa in its sequence and is safe to remove.
                 */
                if (scan_inst->dst.file == QFILE_MAGIC &&
                    scan_inst->dst.index == V3D_QPU_WADDR_UNIFA) {
                        return true;
                }

                /* If we find another ldunifa in the same sequence then we
                 * can't remove it.
                 */
                if (scan_inst->qpu.sig.ldunifa || scan_inst->qpu.sig.ldunifarf)
                        return false;
        }

        return true;
}

static bool
check_first_ldunifa(struct v3d_compile *c,
                    struct qinst *inst,
                    struct qblock *block,
                    struct qinst **unifa)
{
        if (!inst->qpu.sig.ldunifa && !inst->qpu.sig.ldunifarf)
                return false;

        list_for_each_entry_from_rev(struct qinst, scan_inst, inst->link.prev,
                                     &block->instructions, link) {
                /* If we find a write to unifa, then this was the first
                 * ldunifa in its sequence and is safe to remove.
                 */
                if (scan_inst->dst.file == QFILE_MAGIC &&
                    scan_inst->dst.index == V3D_QPU_WADDR_UNIFA) {
                        *unifa = scan_inst;
                        return true;
                }

                /* If we find another ldunifa in the same sequence then we
                 * can't remove it.
                 */
                if (scan_inst->qpu.sig.ldunifa || scan_inst->qpu.sig.ldunifarf)
                        return false;
        }

        unreachable("could not find starting unifa for ldunifa sequence");
}

static bool
increment_unifa_address(struct v3d_compile *c, struct qinst *unifa)
{
        if (unifa->qpu.type == V3D_QPU_INSTR_TYPE_ALU &&
            unifa->qpu.alu.mul.op == V3D_QPU_M_MOV) {
                c->cursor = vir_after_inst(unifa);
                struct qreg unifa_reg = vir_reg(QFILE_MAGIC, V3D_QPU_WADDR_UNIFA);
                vir_ADD_dest(c, unifa_reg, unifa->src[0], vir_uniform_ui(c, 4u));
                vir_remove_instruction(c, unifa);
                return true;
        }

        if (unifa->qpu.type == V3D_QPU_INSTR_TYPE_ALU &&
            unifa->qpu.alu.add.op == V3D_QPU_A_ADD) {
                c->cursor = vir_after_inst(unifa);
                struct qreg unifa_reg = vir_reg(QFILE_MAGIC, V3D_QPU_WADDR_UNIFA);
                struct qreg tmp =
                        vir_ADD(c, unifa->src[1], vir_uniform_ui(c, 4u));
                vir_ADD_dest(c, unifa_reg, unifa->src[0], tmp);
                vir_remove_instruction(c, unifa);
                return true;
        }

        return false;
}

bool
vir_opt_dead_code(struct v3d_compile *c)
{
        bool progress = false;
        bool *used = calloc(c->num_temps, sizeof(bool));

        /* Defuse the "are you removing the cursor?" assertion in the core.
         * You'll need to set up a new cursor for any new instructions after
         * doing DCE (which we would expect, anyway).
         */
        c->cursor.link = NULL;

        vir_for_each_inst_inorder(inst, c) {
                for (int i = 0; i < vir_get_nsrc(inst); i++) {
                        if (inst->src[i].file == QFILE_TEMP)
                                used[inst->src[i].index] = true;
                }
        }

        vir_for_each_block(block, c) {
                struct qinst *last_flags_write = NULL;
                c->cur_block = block;
                vir_for_each_inst_safe(inst, block) {
                        /* If this instruction reads the flags, we can't
                         * remove the flags generation for it.
                         */
                        if (v3d_qpu_reads_flags(&inst->qpu))
                                last_flags_write = NULL;

                        if (inst->dst.file != QFILE_NULL &&
                            !(inst->dst.file == QFILE_TEMP &&
                              !used[inst->dst.index])) {
                                continue;
                        }

                        const bool is_ldunifa = inst->qpu.sig.ldunifa ||
                                                inst->qpu.sig.ldunifarf;

                        if (vir_has_side_effects(c, inst) && !is_ldunifa)
                                continue;

                        bool is_first_ldunifa = false;
                        bool is_last_ldunifa = false;
                        struct qinst *unifa = NULL;
                        if (is_ldunifa) {
                                is_last_ldunifa =
                                        check_last_ldunifa(c, inst, block);

                                is_first_ldunifa =
                                        check_first_ldunifa(c, inst, block, &unifa);
                        }

                        if (v3d_qpu_writes_flags(&inst->qpu)) {
                                /* If we obscure a previous flags write,
                                 * drop it.
                                 */
                                if (last_flags_write &&
                                    (inst->qpu.flags.apf != V3D_QPU_PF_NONE ||
                                     inst->qpu.flags.mpf != V3D_QPU_PF_NONE)) {
                                        vir_dce_flags(c, last_flags_write);
                                        progress = true;
                                }

                                last_flags_write = inst;
                        }

                        if (v3d_qpu_writes_flags(&inst->qpu) ||
                            (is_ldunifa && !is_first_ldunifa && !is_last_ldunifa)) {
                                /* If we can't remove the instruction, but we
                                 * don't need its destination value, just
                                 * remove the destination.  The register
                                 * allocator would trivially color it and it
                                 * wouldn't cause any register pressure, but
                                 * it's nicer to read the VIR code without
                                 * unused destination regs.
                                 */
                                if (inst->dst.file == QFILE_TEMP &&
                                    can_write_to_null(c, inst)) {
                                        if (debug) {
                                                fprintf(stderr,
                                                        "Removing dst from: ");
                                                vir_dump_inst(c, inst);
                                                fprintf(stderr, "\n");
                                        }
                                        c->defs[inst->dst.index] = NULL;
                                        inst->dst.file = QFILE_NULL;
                                        progress = true;
                                }
                                continue;
                        }

                        /* If we are removing the first ldunifa in a sequence
                         * we need to update the unifa address.
                         */
                        if (is_first_ldunifa) {
                                assert(unifa);
                                if (!increment_unifa_address(c, unifa))
                                        continue;
                        }

                        assert(inst != last_flags_write);
                        dce(c, inst);
                        progress = true;
                        continue;
                }
        }

        free(used);

        return progress;
}
