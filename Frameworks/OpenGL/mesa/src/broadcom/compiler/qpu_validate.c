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
 * @file
 *
 * Validates the QPU instruction sequence after register allocation and
 * scheduling.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "v3d_compiler.h"
#include "qpu/qpu_disasm.h"

struct v3d_qpu_validate_state {
        struct v3d_compile *c;
        const struct v3d_qpu_instr *last;
        int ip;
        int last_sfu_write;
        int last_branch_ip;
        int last_thrsw_ip;
        int first_tlb_z_write;

        /* Set when we've found the last-THRSW signal, or if we were started
         * in single-segment mode.
         */
        bool last_thrsw_found;

        /* Set when we've found the THRSW after the last THRSW */
        bool thrend_found;

        int thrsw_count;
};

static void
fail_instr(struct v3d_qpu_validate_state *state, const char *msg)
{
        struct v3d_compile *c = state->c;

        fprintf(stderr, "v3d_qpu_validate at ip %d: %s:\n", state->ip, msg);

        int dump_ip = 0;
        vir_for_each_inst_inorder(inst, c) {
                v3d_qpu_dump(c->devinfo, &inst->qpu);

                if (dump_ip++ == state->ip)
                        fprintf(stderr, " *** ERROR ***");

                fprintf(stderr, "\n");
        }

        fprintf(stderr, "\n");
        abort();
}

static bool
in_branch_delay_slots(struct v3d_qpu_validate_state *state)
{
        return (state->ip - state->last_branch_ip) < 3;
}

static bool
in_thrsw_delay_slots(struct v3d_qpu_validate_state *state)
{
        return (state->ip - state->last_thrsw_ip) < 3;
}

static bool
qpu_magic_waddr_matches(const struct v3d_qpu_instr *inst,
                        bool (*predicate)(enum v3d_qpu_waddr waddr))
{
        if (inst->type == V3D_QPU_INSTR_TYPE_ALU)
                return false;

        if (inst->alu.add.op != V3D_QPU_A_NOP &&
            inst->alu.add.magic_write &&
            predicate(inst->alu.add.waddr))
                return true;

        if (inst->alu.mul.op != V3D_QPU_M_NOP &&
            inst->alu.mul.magic_write &&
            predicate(inst->alu.mul.waddr))
                return true;

        return false;
}

static void
qpu_validate_inst(struct v3d_qpu_validate_state *state, struct qinst *qinst)
{
        const struct v3d_device_info *devinfo = state->c->devinfo;

        if (qinst->is_tlb_z_write && state->ip < state->first_tlb_z_write)
                state->first_tlb_z_write = state->ip;

        const struct v3d_qpu_instr *inst = &qinst->qpu;

        if (inst->type == V3D_QPU_INSTR_TYPE_BRANCH &&
            state->first_tlb_z_write >= 0 &&
            state->ip > state->first_tlb_z_write &&
            inst->branch.msfign != V3D_QPU_MSFIGN_NONE &&
            inst->branch.cond != V3D_QPU_BRANCH_COND_ALWAYS &&
            inst->branch.cond != V3D_QPU_BRANCH_COND_A0 &&
            inst->branch.cond != V3D_QPU_BRANCH_COND_NA0) {
                fail_instr(state, "Implicit branch MSF read after TLB Z write");
        }

        if (inst->type != V3D_QPU_INSTR_TYPE_ALU)
                return;

        if (inst->alu.add.op == V3D_QPU_A_SETMSF &&
            state->first_tlb_z_write >= 0 &&
            state->ip > state->first_tlb_z_write) {
                fail_instr(state, "SETMSF after TLB Z write");
        }

        if (state->first_tlb_z_write >= 0 &&
            state->ip > state->first_tlb_z_write &&
            inst->alu.add.op == V3D_QPU_A_MSF) {
                fail_instr(state, "MSF read after TLB Z write");
        }

        if (devinfo->ver < 71) {
                if (inst->sig.small_imm_a || inst->sig.small_imm_c ||
                    inst->sig.small_imm_d) {
                        fail_instr(state, "small imm a/c/d added after V3D 7.1");
                }
        } else {
                if ((inst->sig.small_imm_a || inst->sig.small_imm_b) &&
                    !vir_is_add(qinst)) {
                        fail_instr(state, "small imm a/b used but no ADD inst");
                }
                if ((inst->sig.small_imm_c || inst->sig.small_imm_d) &&
                    !vir_is_mul(qinst)) {
                        fail_instr(state, "small imm c/d used but no MUL inst");
                }
                if (inst->sig.small_imm_a + inst->sig.small_imm_b +
                    inst->sig.small_imm_c + inst->sig.small_imm_d > 1) {
                        fail_instr(state, "only one small immediate can be "
                                   "enabled per instruction");
                }
        }

        /* LDVARY writes r5 two instructions later and LDUNIF writes
         * r5 one instruction later, which is illegal to have
         * together.
         */
        if (state->last && state->last->sig.ldvary &&
            (inst->sig.ldunif || inst->sig.ldunifa)) {
                fail_instr(state, "LDUNIF after a LDVARY");
        }

        /* GFXH-1633 (fixed since V3D 4.2.14, which is Rpi4)
         *
         * FIXME: This would not check correctly for V3D 4.2 versions lower
         * than V3D 4.2.14, but that is not a real issue because the simulator
         * will still catch this, and we are not really targeting any such
         * versions anyway.
         */
        if (state->c->devinfo->ver < 42) {
                bool last_reads_ldunif = (state->last && (state->last->sig.ldunif ||
                                                          state->last->sig.ldunifrf));
                bool last_reads_ldunifa = (state->last && (state->last->sig.ldunifa ||
                                                           state->last->sig.ldunifarf));
                bool reads_ldunif = inst->sig.ldunif || inst->sig.ldunifrf;
                bool reads_ldunifa = inst->sig.ldunifa || inst->sig.ldunifarf;
                if ((last_reads_ldunif && reads_ldunifa) ||
                    (last_reads_ldunifa && reads_ldunif)) {
                        fail_instr(state,
                                   "LDUNIF and LDUNIFA can't be next to each other");
                }
        }

        int tmu_writes = 0;
        int sfu_writes = 0;
        int vpm_writes = 0;
        int tlb_writes = 0;
        int tsy_writes = 0;

        if (inst->alu.add.op != V3D_QPU_A_NOP) {
                if (inst->alu.add.magic_write) {
                        if (v3d_qpu_magic_waddr_is_tmu(state->c->devinfo,
                                                       inst->alu.add.waddr)) {
                                tmu_writes++;
                        }
                        if (v3d_qpu_magic_waddr_is_sfu(inst->alu.add.waddr))
                                sfu_writes++;
                        if (v3d_qpu_magic_waddr_is_vpm(inst->alu.add.waddr))
                                vpm_writes++;
                        if (v3d_qpu_magic_waddr_is_tlb(inst->alu.add.waddr))
                                tlb_writes++;
                        if (v3d_qpu_magic_waddr_is_tsy(inst->alu.add.waddr))
                                tsy_writes++;
                }
        }

        if (inst->alu.mul.op != V3D_QPU_M_NOP) {
                if (inst->alu.mul.magic_write) {
                        if (v3d_qpu_magic_waddr_is_tmu(state->c->devinfo,
                                                       inst->alu.mul.waddr)) {
                                tmu_writes++;
                        }
                        if (v3d_qpu_magic_waddr_is_sfu(inst->alu.mul.waddr))
                                sfu_writes++;
                        if (v3d_qpu_magic_waddr_is_vpm(inst->alu.mul.waddr))
                                vpm_writes++;
                        if (v3d_qpu_magic_waddr_is_tlb(inst->alu.mul.waddr))
                                tlb_writes++;
                        if (v3d_qpu_magic_waddr_is_tsy(inst->alu.mul.waddr))
                                tsy_writes++;
                }
        }

        if (in_thrsw_delay_slots(state)) {
                /* There's no way you want to start SFU during the THRSW delay
                 * slots, since the result would land in the other thread.
                 */
                if (sfu_writes) {
                        fail_instr(state,
                                   "SFU write started during THRSW delay slots ");
                }

                if (inst->sig.ldvary) {
                        if (devinfo->ver == 42)
                                fail_instr(state, "LDVARY during THRSW delay slots");
                        if (devinfo->ver >= 71 &&
                            state->ip - state->last_thrsw_ip == 2) {
                                fail_instr(state, "LDVARY in 2nd THRSW delay slot");
                        }
                }
        }

        (void)qpu_magic_waddr_matches; /* XXX */

        /* SFU r4 results come back two instructions later.  No doing
         * r4 read/writes or other SFU lookups until it's done.
         */
        if (state->ip - state->last_sfu_write < 2) {
                if (v3d_qpu_uses_mux(inst, V3D_QPU_MUX_R4))
                        fail_instr(state, "R4 read too soon after SFU");

                if (v3d_qpu_writes_r4(devinfo, inst))
                        fail_instr(state, "R4 write too soon after SFU");

                if (sfu_writes)
                        fail_instr(state, "SFU write too soon after SFU");
        }

        /* XXX: The docs say VPM can happen with the others, but the simulator
         * disagrees.
         */
        if (tmu_writes +
            sfu_writes +
            vpm_writes +
            tlb_writes +
            tsy_writes +
            (devinfo->ver == 42 ? inst->sig.ldtmu : 0) +
            inst->sig.ldtlb +
            inst->sig.ldvpm +
            inst->sig.ldtlbu > 1) {
                fail_instr(state,
                           "Only one of [TMU, SFU, TSY, TLB read, VPM] allowed");
        }

        if (sfu_writes)
                state->last_sfu_write = state->ip;

        if (inst->sig.thrsw) {
                if (in_branch_delay_slots(state))
                        fail_instr(state, "THRSW in a branch delay slot.");

                if (state->last_thrsw_found)
                        state->thrend_found = true;

                if (state->last_thrsw_ip == state->ip - 1) {
                        /* If it's the second THRSW in a row, then it's just a
                         * last-thrsw signal.
                         */
                        if (state->last_thrsw_found)
                                fail_instr(state, "Two last-THRSW signals");
                        state->last_thrsw_found = true;
                } else {
                        if (in_thrsw_delay_slots(state)) {
                                fail_instr(state,
                                           "THRSW too close to another THRSW.");
                        }
                        state->thrsw_count++;
                        state->last_thrsw_ip = state->ip;
                }
        }

        if (state->thrend_found &&
            state->last_thrsw_ip - state->ip <= 2 &&
            inst->type == V3D_QPU_INSTR_TYPE_ALU) {
                if ((inst->alu.add.op != V3D_QPU_A_NOP &&
                     !inst->alu.add.magic_write)) {
                        if (devinfo->ver == 42) {
                                fail_instr(state, "RF write after THREND");
                        } else if (devinfo->ver >= 71) {
                                if (state->last_thrsw_ip - state->ip == 0) {
                                        fail_instr(state,
                                                   "ADD RF write at THREND");
                                }
                                if (inst->alu.add.waddr == 2 ||
                                    inst->alu.add.waddr == 3) {
                                        fail_instr(state,
                                                   "RF2-3 write after THREND");
                                }
                        }
                }

                if ((inst->alu.mul.op != V3D_QPU_M_NOP &&
                     !inst->alu.mul.magic_write)) {
                        if (devinfo->ver == 42) {
                                fail_instr(state, "RF write after THREND");
                        } else if (devinfo->ver >= 71) {
                                if (state->last_thrsw_ip - state->ip == 0) {
                                        fail_instr(state,
                                                   "MUL RF write at THREND");
                                }

                                if (inst->alu.mul.waddr == 2 ||
                                    inst->alu.mul.waddr == 3) {
                                        fail_instr(state,
                                                   "RF2-3 write after THREND");
                                }
                        }
                }

                if (v3d_qpu_sig_writes_address(devinfo, &inst->sig) &&
                    !inst->sig_magic) {
                        if (devinfo->ver == 42) {
                                fail_instr(state, "RF write after THREND");
                        } else if (devinfo->ver >= 71 &&
                                   (inst->sig_addr == 2 ||
                                    inst->sig_addr == 3)) {
                                fail_instr(state, "RF2-3 write after THREND");
                        }
                }

                /* GFXH-1625: No TMUWT in the last instruction */
                if (state->last_thrsw_ip - state->ip == 2 &&
                    inst->alu.add.op == V3D_QPU_A_TMUWT)
                        fail_instr(state, "TMUWT in last instruction");
        }

        if (inst->type == V3D_QPU_INSTR_TYPE_BRANCH) {
                if (in_branch_delay_slots(state))
                        fail_instr(state, "branch in a branch delay slot.");
                if (in_thrsw_delay_slots(state))
                        fail_instr(state, "branch in a THRSW delay slot.");
                state->last_branch_ip = state->ip;
        }
}

static void
qpu_validate_block(struct v3d_qpu_validate_state *state, struct qblock *block)
{
        vir_for_each_inst(qinst, block) {
                qpu_validate_inst(state, qinst);

                state->last = &qinst->qpu;
                state->ip++;
        }
}

/**
 * Checks for the instruction restrictions from page 37 ("Summary of
 * Instruction Restrictions").
 */
void
qpu_validate(struct v3d_compile *c)
{
        /* We don't want to do validation in release builds, but we want to
         * keep compiling the validation code to make sure it doesn't get
         * broken.
         */
#ifndef DEBUG
        return;
#endif

        struct v3d_qpu_validate_state state = {
                .c = c,
                .last_sfu_write = -10,
                .last_thrsw_ip = -10,
                .last_branch_ip = -10,
                .first_tlb_z_write = INT_MAX,
                .ip = 0,

                .last_thrsw_found = !c->last_thrsw,
        };

        vir_for_each_block(block, c) {
                qpu_validate_block(&state, block);
        }

        if (state.thrsw_count > 1 && !state.last_thrsw_found) {
                fail_instr(&state,
                           "thread switch found without last-THRSW in program");
        }

        if (!state.thrend_found)
                fail_instr(&state, "No program-end THRSW found");
}
