/*
 * Copyright © 2010 Intel Corporation
 * Copyright © 2014-2017 Broadcom
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
 * The basic model of the list scheduler is to take a basic block, compute a
 * DAG of the dependencies, and make a list of the DAG heads.  Heuristically
 * pick a DAG head, then put all the children that are now DAG heads into the
 * list of things to schedule.
 *
 * The goal of scheduling here is to pack pairs of operations together in a
 * single QPU instruction.
 */

#include "qpu/qpu_disasm.h"
#include "v3d_compiler.h"
#include "util/ralloc.h"
#include "util/dag.h"

static bool debug;

struct schedule_node_child;

struct schedule_node {
        struct dag_node dag;
        struct list_head link;
        struct qinst *inst;

        /* Longest cycles + instruction_latency() of any parent of this node. */
        uint32_t unblocked_time;

        /**
         * Minimum number of cycles from scheduling this instruction until the
         * end of the program, based on the slowest dependency chain through
         * the children.
         */
        uint32_t delay;

        /**
         * cycles between this instruction being scheduled and when its result
         * can be consumed.
         */
        uint32_t latency;
};

/* When walking the instructions in reverse, we need to swap before/after in
 * add_dep().
 */
enum direction { F, R };

struct schedule_state {
        const struct v3d_device_info *devinfo;
        struct dag *dag;
        struct schedule_node *last_r[6];
        struct schedule_node *last_rf[64];
        struct schedule_node *last_sf;
        struct schedule_node *last_vpm_read;
        struct schedule_node *last_tmu_write;
        struct schedule_node *last_tmu_config;
        struct schedule_node *last_tmu_read;
        struct schedule_node *last_tlb;
        struct schedule_node *last_vpm;
        struct schedule_node *last_unif;
        struct schedule_node *last_rtop;
        struct schedule_node *last_unifa;
        enum direction dir;
        /* Estimated cycle when the current instruction would start. */
        uint32_t time;
};

static void
add_dep(struct schedule_state *state,
        struct schedule_node *before,
        struct schedule_node *after,
        bool write)
{
        bool write_after_read = !write && state->dir == R;
        uintptr_t edge_data = write_after_read;

        if (!before || !after)
                return;

        assert(before != after);

        if (state->dir == F)
                dag_add_edge(&before->dag, &after->dag, edge_data);
        else
                dag_add_edge(&after->dag, &before->dag, edge_data);
}

static void
add_read_dep(struct schedule_state *state,
              struct schedule_node *before,
              struct schedule_node *after)
{
        add_dep(state, before, after, false);
}

static void
add_write_dep(struct schedule_state *state,
              struct schedule_node **before,
              struct schedule_node *after)
{
        add_dep(state, *before, after, true);
        *before = after;
}

static bool
qpu_inst_is_tlb(const struct v3d_qpu_instr *inst)
{
        if (inst->sig.ldtlb || inst->sig.ldtlbu)
                return true;

        if (inst->type != V3D_QPU_INSTR_TYPE_ALU)
                return false;

        if (inst->alu.add.op != V3D_QPU_A_NOP &&
            inst->alu.add.magic_write &&
            (inst->alu.add.waddr == V3D_QPU_WADDR_TLB ||
             inst->alu.add.waddr == V3D_QPU_WADDR_TLBU))
                return true;

        if (inst->alu.mul.op != V3D_QPU_M_NOP &&
            inst->alu.mul.magic_write &&
            (inst->alu.mul.waddr == V3D_QPU_WADDR_TLB ||
             inst->alu.mul.waddr == V3D_QPU_WADDR_TLBU))
                return true;

        return false;
}

static void
process_mux_deps(struct schedule_state *state, struct schedule_node *n,
                 enum v3d_qpu_mux mux)
{
        assert(state->devinfo->ver < 71);
        switch (mux) {
        case V3D_QPU_MUX_A:
                add_read_dep(state, state->last_rf[n->inst->qpu.raddr_a], n);
                break;
        case V3D_QPU_MUX_B:
                if (!n->inst->qpu.sig.small_imm_b) {
                        add_read_dep(state,
                                     state->last_rf[n->inst->qpu.raddr_b], n);
                }
                break;
        default:
                add_read_dep(state, state->last_r[mux - V3D_QPU_MUX_R0], n);
                break;
        }
}


static void
process_raddr_deps(struct schedule_state *state, struct schedule_node *n,
                   uint8_t raddr, bool is_small_imm)
{
        assert(state->devinfo->ver >= 71);

        if (!is_small_imm)
                add_read_dep(state, state->last_rf[raddr], n);
}

static bool
tmu_write_is_sequence_terminator(uint32_t waddr)
{
        switch (waddr) {
        case V3D_QPU_WADDR_TMUS:
        case V3D_QPU_WADDR_TMUSCM:
        case V3D_QPU_WADDR_TMUSF:
        case V3D_QPU_WADDR_TMUSLOD:
        case V3D_QPU_WADDR_TMUA:
        case V3D_QPU_WADDR_TMUAU:
                return true;
        default:
                return false;
        }
}

static bool
can_reorder_tmu_write(const struct v3d_device_info *devinfo, uint32_t waddr)
{
        if (tmu_write_is_sequence_terminator(waddr))
                return false;

        if (waddr == V3D_QPU_WADDR_TMUD)
                return false;

        return true;
}

static void
process_waddr_deps(struct schedule_state *state, struct schedule_node *n,
                   uint32_t waddr, bool magic)
{
        if (!magic) {
                add_write_dep(state, &state->last_rf[waddr], n);
        } else if (v3d_qpu_magic_waddr_is_tmu(state->devinfo, waddr)) {
                if (can_reorder_tmu_write(state->devinfo, waddr))
                        add_read_dep(state, state->last_tmu_write, n);
                else
                        add_write_dep(state, &state->last_tmu_write, n);

                if (tmu_write_is_sequence_terminator(waddr))
                        add_write_dep(state, &state->last_tmu_config, n);
        } else if (v3d_qpu_magic_waddr_is_sfu(waddr)) {
                /* Handled by v3d_qpu_writes_r4() check. */
        } else {
                switch (waddr) {
                case V3D_QPU_WADDR_R0:
                case V3D_QPU_WADDR_R1:
                case V3D_QPU_WADDR_R2:
                        add_write_dep(state,
                                      &state->last_r[waddr - V3D_QPU_WADDR_R0],
                                      n);
                        break;
                case V3D_QPU_WADDR_R3:
                case V3D_QPU_WADDR_R4:
                case V3D_QPU_WADDR_R5:
                        /* Handled by v3d_qpu_writes_r*() checks below. */
                        break;

                case V3D_QPU_WADDR_VPM:
                case V3D_QPU_WADDR_VPMU:
                        add_write_dep(state, &state->last_vpm, n);
                        break;

                case V3D_QPU_WADDR_TLB:
                case V3D_QPU_WADDR_TLBU:
                        add_write_dep(state, &state->last_tlb, n);
                        break;

                case V3D_QPU_WADDR_SYNC:
                case V3D_QPU_WADDR_SYNCB:
                case V3D_QPU_WADDR_SYNCU:
                        /* For CS barrier(): Sync against any other memory
                         * accesses.  There doesn't appear to be any need for
                         * barriers to affect ALU operations.
                         */
                        add_write_dep(state, &state->last_tmu_write, n);
                        add_write_dep(state, &state->last_tmu_read, n);
                        break;

                case V3D_QPU_WADDR_UNIFA:
                        add_write_dep(state, &state->last_unifa, n);
                        break;

                case V3D_QPU_WADDR_NOP:
                        break;

                default:
                        fprintf(stderr, "Unknown waddr %d\n", waddr);
                        abort();
                }
        }
}

/**
 * Common code for dependencies that need to be tracked both forward and
 * backward.
 *
 * This is for things like "all reads of r4 have to happen between the r4
 * writes that surround them".
 */
static void
calculate_deps(struct schedule_state *state, struct schedule_node *n)
{
        const struct v3d_device_info *devinfo = state->devinfo;
        struct qinst *qinst = n->inst;
        struct v3d_qpu_instr *inst = &qinst->qpu;
        /* If the input and output segments are shared, then all VPM reads to
         * a location need to happen before all writes.  We handle this by
         * serializing all VPM operations for now.
         *
         * FIXME: we are assuming that the segments are shared. That is
         * correct right now as we are only using shared, but technically you
         * can choose.
         */
        bool separate_vpm_segment = false;

        if (inst->type == V3D_QPU_INSTR_TYPE_BRANCH) {
                if (inst->branch.cond != V3D_QPU_BRANCH_COND_ALWAYS)
                        add_read_dep(state, state->last_sf, n);

                /* XXX: BDI */
                /* XXX: BDU */
                /* XXX: ub */
                /* XXX: raddr_a */

                add_write_dep(state, &state->last_unif, n);
                return;
        }

        assert(inst->type == V3D_QPU_INSTR_TYPE_ALU);

        /* XXX: LOAD_IMM */

        if (v3d_qpu_add_op_num_src(inst->alu.add.op) > 0) {
                if (devinfo->ver < 71) {
                        process_mux_deps(state, n, inst->alu.add.a.mux);
                } else {
                        process_raddr_deps(state, n, inst->alu.add.a.raddr,
                                           inst->sig.small_imm_a);
                }
        }
        if (v3d_qpu_add_op_num_src(inst->alu.add.op) > 1) {
                if (devinfo->ver < 71) {
                        process_mux_deps(state, n, inst->alu.add.b.mux);
                } else {
                        process_raddr_deps(state, n, inst->alu.add.b.raddr,
                                           inst->sig.small_imm_b);
                }
        }

        if (v3d_qpu_mul_op_num_src(inst->alu.mul.op) > 0) {
                if (devinfo->ver < 71) {
                        process_mux_deps(state, n, inst->alu.mul.a.mux);
                } else {
                        process_raddr_deps(state, n, inst->alu.mul.a.raddr,
                                           inst->sig.small_imm_c);
                }
        }
        if (v3d_qpu_mul_op_num_src(inst->alu.mul.op) > 1) {
                if (devinfo->ver < 71) {
                        process_mux_deps(state, n, inst->alu.mul.b.mux);
                } else {
                        process_raddr_deps(state, n, inst->alu.mul.b.raddr,
                                           inst->sig.small_imm_d);
                }
        }

        switch (inst->alu.add.op) {
        case V3D_QPU_A_VPMSETUP:
                /* Could distinguish read/write by unpacking the uniform. */
                add_write_dep(state, &state->last_vpm, n);
                add_write_dep(state, &state->last_vpm_read, n);
                break;

        case V3D_QPU_A_STVPMV:
        case V3D_QPU_A_STVPMD:
        case V3D_QPU_A_STVPMP:
                add_write_dep(state, &state->last_vpm, n);
                break;

        case V3D_QPU_A_LDVPMV_IN:
        case V3D_QPU_A_LDVPMD_IN:
        case V3D_QPU_A_LDVPMG_IN:
        case V3D_QPU_A_LDVPMP:
                if (!separate_vpm_segment)
                        add_write_dep(state, &state->last_vpm, n);
                break;

        case V3D_QPU_A_VPMWT:
                add_read_dep(state, state->last_vpm, n);
                break;

        case V3D_QPU_A_MSF:
                add_read_dep(state, state->last_tlb, n);
                break;

        case V3D_QPU_A_SETMSF:
                add_write_dep(state, &state->last_tmu_write, n);
                FALLTHROUGH;
        case V3D_QPU_A_SETREVF:
                add_write_dep(state, &state->last_tlb, n);
                break;

        default:
                break;
        }

        switch (inst->alu.mul.op) {
        case V3D_QPU_M_MULTOP:
        case V3D_QPU_M_UMUL24:
                /* MULTOP sets rtop, and UMUL24 implicitly reads rtop and
                 * resets it to 0.  We could possibly reorder umul24s relative
                 * to each other, but for now just keep all the MUL parts in
                 * order.
                 */
                add_write_dep(state, &state->last_rtop, n);
                break;
        default:
                break;
        }

        if (inst->alu.add.op != V3D_QPU_A_NOP) {
                process_waddr_deps(state, n, inst->alu.add.waddr,
                                   inst->alu.add.magic_write);
        }
        if (inst->alu.mul.op != V3D_QPU_M_NOP) {
                process_waddr_deps(state, n, inst->alu.mul.waddr,
                                   inst->alu.mul.magic_write);
        }
        if (v3d_qpu_sig_writes_address(devinfo, &inst->sig)) {
                process_waddr_deps(state, n, inst->sig_addr,
                                   inst->sig_magic);
        }

        if (v3d_qpu_writes_r3(devinfo, inst))
                add_write_dep(state, &state->last_r[3], n);
        if (v3d_qpu_writes_r4(devinfo, inst))
                add_write_dep(state, &state->last_r[4], n);
        if (v3d_qpu_writes_r5(devinfo, inst))
                add_write_dep(state, &state->last_r[5], n);
        if (v3d_qpu_writes_rf0_implicitly(devinfo, inst))
                add_write_dep(state, &state->last_rf[0], n);

        /* If we add any more dependencies here we should consider whether we
         * also need to update qpu_inst_after_thrsw_valid_in_delay_slot.
         */
        if (inst->sig.thrsw) {
                /* All accumulator contents and flags are undefined after the
                 * switch.
                 */
                for (int i = 0; i < ARRAY_SIZE(state->last_r); i++)
                        add_write_dep(state, &state->last_r[i], n);
                add_write_dep(state, &state->last_sf, n);
                add_write_dep(state, &state->last_rtop, n);

                /* Scoreboard-locking operations have to stay after the last
                 * thread switch.
                 */
                add_write_dep(state, &state->last_tlb, n);

                add_write_dep(state, &state->last_tmu_write, n);
                add_write_dep(state, &state->last_tmu_config, n);
        }

        if (v3d_qpu_waits_on_tmu(inst)) {
                /* TMU loads are coming from a FIFO, so ordering is important.
                 */
                add_write_dep(state, &state->last_tmu_read, n);
                /* Keep TMU loads after their TMU lookup terminator */
                add_read_dep(state, state->last_tmu_config, n);
        }

        /* Allow wrtmuc to be reordered with other instructions in the
         * same TMU sequence by using a read dependency on the last TMU
         * sequence terminator.
         */
        if (inst->sig.wrtmuc)
                add_read_dep(state, state->last_tmu_config, n);

        if (inst->sig.ldtlb | inst->sig.ldtlbu)
                add_write_dep(state, &state->last_tlb, n);

        if (inst->sig.ldvpm) {
                add_write_dep(state, &state->last_vpm_read, n);

                /* At least for now, we're doing shared I/O segments, so queue
                 * all writes after all reads.
                 */
                if (!separate_vpm_segment)
                        add_write_dep(state, &state->last_vpm, n);
        }

        /* inst->sig.ldunif or sideband uniform read */
        if (vir_has_uniform(qinst))
                add_write_dep(state, &state->last_unif, n);

        /* Both unifa and ldunifa must preserve ordering */
        if (inst->sig.ldunifa || inst->sig.ldunifarf)
                add_write_dep(state, &state->last_unifa, n);

        if (v3d_qpu_reads_flags(inst))
                add_read_dep(state, state->last_sf, n);
        if (v3d_qpu_writes_flags(inst))
                add_write_dep(state, &state->last_sf, n);
}

static void
calculate_forward_deps(struct v3d_compile *c, struct dag *dag,
                       struct list_head *schedule_list)
{
        struct schedule_state state;

        memset(&state, 0, sizeof(state));
        state.dag = dag;
        state.devinfo = c->devinfo;
        state.dir = F;

        list_for_each_entry(struct schedule_node, node, schedule_list, link)
                calculate_deps(&state, node);
}

static void
calculate_reverse_deps(struct v3d_compile *c, struct dag *dag,
                       struct list_head *schedule_list)
{
        struct schedule_state state;

        memset(&state, 0, sizeof(state));
        state.dag = dag;
        state.devinfo = c->devinfo;
        state.dir = R;

        list_for_each_entry_rev(struct schedule_node, node, schedule_list,
                                link) {
                calculate_deps(&state, (struct schedule_node *)node);
        }
}

struct choose_scoreboard {
        struct dag *dag;
        int tick;
        int last_magic_sfu_write_tick;
        int last_stallable_sfu_reg;
        int last_stallable_sfu_tick;
        int last_ldvary_tick;
        int last_unifa_write_tick;
        int last_uniforms_reset_tick;
        int last_thrsw_tick;
        int last_branch_tick;
        int last_setmsf_tick;
        bool first_thrsw_emitted;
        bool last_thrsw_emitted;
        bool fixup_ldvary;
        int ldvary_count;
        int pending_ldtmu_count;
        bool first_ldtmu_after_thrsw;

        /* V3D 7.x */
        int last_implicit_rf0_write_tick;
        bool has_rf0_flops_conflict;
};

static bool
mux_reads_too_soon(struct choose_scoreboard *scoreboard,
                   const struct v3d_qpu_instr *inst, enum v3d_qpu_mux mux)
{
        switch (mux) {
        case V3D_QPU_MUX_R4:
                if (scoreboard->tick - scoreboard->last_magic_sfu_write_tick <= 2)
                        return true;
                break;

        case V3D_QPU_MUX_R5:
                if (scoreboard->tick - scoreboard->last_ldvary_tick <= 1)
                        return true;
                break;
        default:
                break;
        }

        return false;
}

static bool
reads_too_soon(struct choose_scoreboard *scoreboard,
               const struct v3d_qpu_instr *inst, uint8_t raddr)
{
        switch (raddr) {
        case 0: /* ldvary delayed write of C coefficient to rf0 */
                if (scoreboard->tick - scoreboard->last_ldvary_tick <= 1)
                        return true;
                break;
        default:
                break;
        }

        return false;
}

static bool
reads_too_soon_after_write(const struct v3d_device_info *devinfo,
                           struct choose_scoreboard *scoreboard,
                           struct qinst *qinst)
{
        const struct v3d_qpu_instr *inst = &qinst->qpu;

        /* XXX: Branching off of raddr. */
        if (inst->type == V3D_QPU_INSTR_TYPE_BRANCH)
                return false;

        assert(inst->type == V3D_QPU_INSTR_TYPE_ALU);

        if (inst->alu.add.op != V3D_QPU_A_NOP) {
                if (v3d_qpu_add_op_num_src(inst->alu.add.op) > 0) {
                        if (devinfo->ver < 71) {
                                if (mux_reads_too_soon(scoreboard, inst, inst->alu.add.a.mux))
                                        return true;
                        } else {
                                if (reads_too_soon(scoreboard, inst, inst->alu.add.a.raddr))
                                        return true;
                        }
                }
                if (v3d_qpu_add_op_num_src(inst->alu.add.op) > 1) {
                        if (devinfo->ver < 71) {
                                if (mux_reads_too_soon(scoreboard, inst, inst->alu.add.b.mux))
                                        return true;
                        } else {
                                if (reads_too_soon(scoreboard, inst, inst->alu.add.b.raddr))
                                        return true;
                        }
                }
        }

        if (inst->alu.mul.op != V3D_QPU_M_NOP) {
                if (v3d_qpu_mul_op_num_src(inst->alu.mul.op) > 0) {
                        if (devinfo->ver < 71) {
                                if (mux_reads_too_soon(scoreboard, inst, inst->alu.mul.a.mux))
                                        return true;
                        } else {
                                if (reads_too_soon(scoreboard, inst, inst->alu.mul.a.raddr))
                                        return true;
                        }
                }
                if (v3d_qpu_mul_op_num_src(inst->alu.mul.op) > 1) {
                        if (devinfo->ver < 71) {
                                if (mux_reads_too_soon(scoreboard, inst, inst->alu.mul.b.mux))
                                        return true;
                        } else {
                                if (reads_too_soon(scoreboard, inst, inst->alu.mul.b.raddr))
                                        return true;
                        }
                }
        }

        /* XXX: imm */

        return false;
}

static bool
writes_too_soon_after_write(const struct v3d_device_info *devinfo,
                            struct choose_scoreboard *scoreboard,
                            struct qinst *qinst)
{
        const struct v3d_qpu_instr *inst = &qinst->qpu;

        /* Don't schedule any other r4 write too soon after an SFU write.
         * This would normally be prevented by dependency tracking, but might
         * occur if a dead SFU computation makes it to scheduling.
         */
        if (scoreboard->tick - scoreboard->last_magic_sfu_write_tick < 2 &&
            v3d_qpu_writes_r4(devinfo, inst))
                return true;

        if (devinfo->ver == 42)
           return false;

        /* Don't schedule anything that writes rf0 right after ldvary, since
         * that would clash with the ldvary's delayed rf0 write (the exception
         * is another ldvary, since its implicit rf0 write would also have
         * one cycle of delay and would not clash).
         */
        if (scoreboard->last_ldvary_tick + 1 == scoreboard->tick &&
            (v3d71_qpu_writes_waddr_explicitly(devinfo, inst, 0) ||
             (v3d_qpu_writes_rf0_implicitly(devinfo, inst) &&
              !inst->sig.ldvary))) {
            return true;
       }

        return false;
}

static bool
scoreboard_is_locked(struct choose_scoreboard *scoreboard,
                     bool lock_scoreboard_on_first_thrsw)
{
        if (lock_scoreboard_on_first_thrsw) {
                return scoreboard->first_thrsw_emitted &&
                       scoreboard->tick - scoreboard->last_thrsw_tick >= 3;
        }

        return scoreboard->last_thrsw_emitted &&
               scoreboard->tick - scoreboard->last_thrsw_tick >= 3;
}

static bool
pixel_scoreboard_too_soon(struct v3d_compile *c,
                          struct choose_scoreboard *scoreboard,
                          const struct v3d_qpu_instr *inst)
{
        return qpu_inst_is_tlb(inst) &&
               !scoreboard_is_locked(scoreboard,
                                     c->lock_scoreboard_on_first_thrsw);
}

static bool
qpu_instruction_uses_rf(const struct v3d_device_info *devinfo,
                        const struct v3d_qpu_instr *inst,
                        uint32_t waddr) {

        if (inst->type != V3D_QPU_INSTR_TYPE_ALU)
           return false;

        if (devinfo->ver < 71) {
                if (v3d_qpu_uses_mux(inst, V3D_QPU_MUX_A) &&
                    inst->raddr_a == waddr)
                        return true;

                if (v3d_qpu_uses_mux(inst, V3D_QPU_MUX_B) &&
                    !inst->sig.small_imm_b && (inst->raddr_b == waddr))
                        return true;
        } else {
                if (v3d71_qpu_reads_raddr(inst, waddr))
                        return true;
        }

        return false;
}

static bool
read_stalls(const struct v3d_device_info *devinfo,
            struct choose_scoreboard *scoreboard,
            const struct v3d_qpu_instr *inst)
{
        return scoreboard->tick == scoreboard->last_stallable_sfu_tick + 1 &&
                qpu_instruction_uses_rf(devinfo, inst,
                                        scoreboard->last_stallable_sfu_reg);
}

/* We define a max schedule priority to allow negative priorities as result of
 * subtracting this max when an instruction stalls. So instructions that
 * stall have lower priority than regular instructions. */
#define MAX_SCHEDULE_PRIORITY 16

static int
get_instruction_priority(const struct v3d_device_info *devinfo,
                         const struct v3d_qpu_instr *inst)
{
        uint32_t baseline_score;
        uint32_t next_score = 0;

        /* Schedule TLB operations as late as possible, to get more
         * parallelism between shaders.
         */
        if (qpu_inst_is_tlb(inst))
                return next_score;
        next_score++;

        /* Empirical testing shows that using priorities to hide latency of
         * TMU operations when scheduling QPU leads to slightly worse
         * performance, even at 2 threads. We think this is because the thread
         * switching is already quite effective at hiding latency and NIR
         * scheduling (and possibly TMU pipelining too) are sufficient to hide
         * TMU latency, so piling up on that here doesn't provide any benefits
         * and instead may cause us to postpone critical paths that depend on
         * the TMU results.
         */
#if 0
        /* Schedule texture read results collection late to hide latency. */
        if (v3d_qpu_waits_on_tmu(inst))
                return next_score;
        next_score++;
#endif

        /* Default score for things that aren't otherwise special. */
        baseline_score = next_score;
        next_score++;

#if 0
        /* Schedule texture read setup early to hide their latency better. */
        if (v3d_qpu_writes_tmu(devinfo, inst))
                return next_score;
        next_score++;
#endif

        /* We should increase the maximum if we assert here */
        assert(next_score < MAX_SCHEDULE_PRIORITY);

        return baseline_score;
}

enum {
        V3D_PERIPHERAL_VPM_READ           = (1 << 0),
        V3D_PERIPHERAL_VPM_WRITE          = (1 << 1),
        V3D_PERIPHERAL_VPM_WAIT           = (1 << 2),
        V3D_PERIPHERAL_SFU                = (1 << 3),
        V3D_PERIPHERAL_TMU_WRITE          = (1 << 4),
        V3D_PERIPHERAL_TMU_READ           = (1 << 5),
        V3D_PERIPHERAL_TMU_WAIT           = (1 << 6),
        V3D_PERIPHERAL_TMU_WRTMUC_SIG     = (1 << 7),
        V3D_PERIPHERAL_TSY                = (1 << 8),
        V3D_PERIPHERAL_TLB_READ           = (1 << 9),
        V3D_PERIPHERAL_TLB_WRITE          = (1 << 10),
};

static uint32_t
qpu_peripherals(const struct v3d_device_info *devinfo,
                const struct v3d_qpu_instr *inst)
{
        uint32_t result = 0;
        if (v3d_qpu_reads_vpm(inst))
                result |= V3D_PERIPHERAL_VPM_READ;
        if (v3d_qpu_writes_vpm(inst))
                result |= V3D_PERIPHERAL_VPM_WRITE;
        if (v3d_qpu_waits_vpm(inst))
                result |= V3D_PERIPHERAL_VPM_WAIT;

        if (v3d_qpu_writes_tmu(devinfo, inst))
                result |= V3D_PERIPHERAL_TMU_WRITE;
        if (inst->sig.ldtmu)
                result |= V3D_PERIPHERAL_TMU_READ;
        if (inst->sig.wrtmuc)
                result |= V3D_PERIPHERAL_TMU_WRTMUC_SIG;

        if (v3d_qpu_uses_sfu(inst))
                result |= V3D_PERIPHERAL_SFU;

        if (v3d_qpu_reads_tlb(inst))
                result |= V3D_PERIPHERAL_TLB_READ;
        if (v3d_qpu_writes_tlb(inst))
                result |= V3D_PERIPHERAL_TLB_WRITE;

        if (inst->type == V3D_QPU_INSTR_TYPE_ALU) {
                if (inst->alu.add.op != V3D_QPU_A_NOP &&
                    inst->alu.add.magic_write &&
                    v3d_qpu_magic_waddr_is_tsy(inst->alu.add.waddr)) {
                        result |= V3D_PERIPHERAL_TSY;
                }

                if (inst->alu.add.op == V3D_QPU_A_TMUWT)
                        result |= V3D_PERIPHERAL_TMU_WAIT;
        }

        return result;
}

static bool
qpu_compatible_peripheral_access(const struct v3d_device_info *devinfo,
                                 const struct v3d_qpu_instr *a,
                                 const struct v3d_qpu_instr *b)
{
        const uint32_t a_peripherals = qpu_peripherals(devinfo, a);
        const uint32_t b_peripherals = qpu_peripherals(devinfo, b);

        /* We can always do one peripheral access per instruction. */
        if (util_bitcount(a_peripherals) + util_bitcount(b_peripherals) <= 1)
                return true;

        /* V3D 4.x can't do more than one peripheral access except in a
         * few cases:
         */
        if (devinfo->ver == 42) {
                /* WRTMUC signal with TMU register write (other than tmuc). */
                if (a_peripherals == V3D_PERIPHERAL_TMU_WRTMUC_SIG &&
                    b_peripherals == V3D_PERIPHERAL_TMU_WRITE) {
                        return v3d_qpu_writes_tmu_not_tmuc(devinfo, b);
                }
                if (b_peripherals == V3D_PERIPHERAL_TMU_WRTMUC_SIG &&
                    a_peripherals == V3D_PERIPHERAL_TMU_WRITE) {
                        return v3d_qpu_writes_tmu_not_tmuc(devinfo, a);
                }

                /* TMU read with VPM read/write. */
                if (a_peripherals == V3D_PERIPHERAL_TMU_READ &&
                    (b_peripherals == V3D_PERIPHERAL_VPM_READ ||
                     b_peripherals == V3D_PERIPHERAL_VPM_WRITE)) {
                        return true;
                }
                if (b_peripherals == V3D_PERIPHERAL_TMU_READ &&
                    (a_peripherals == V3D_PERIPHERAL_VPM_READ ||
                     a_peripherals == V3D_PERIPHERAL_VPM_WRITE)) {
                        return true;
                }

                return false;
        }

        /* V3D 7.x can't have more than one of these restricted peripherals */
        const uint32_t restricted = V3D_PERIPHERAL_TMU_WRITE |
                                    V3D_PERIPHERAL_TMU_WRTMUC_SIG |
                                    V3D_PERIPHERAL_TSY |
                                    V3D_PERIPHERAL_TLB_READ |
                                    V3D_PERIPHERAL_SFU |
                                    V3D_PERIPHERAL_VPM_READ |
                                    V3D_PERIPHERAL_VPM_WRITE;

        const uint32_t a_restricted = a_peripherals & restricted;
        const uint32_t b_restricted = b_peripherals & restricted;
        if (a_restricted && b_restricted) {
                /* WRTMUC signal with TMU register write (other than tmuc) is
                 * allowed though.
                 */
                if (!((a_restricted == V3D_PERIPHERAL_TMU_WRTMUC_SIG &&
                       b_restricted == V3D_PERIPHERAL_TMU_WRITE &&
                       v3d_qpu_writes_tmu_not_tmuc(devinfo, b)) ||
                      (b_restricted == V3D_PERIPHERAL_TMU_WRTMUC_SIG &&
                       a_restricted == V3D_PERIPHERAL_TMU_WRITE &&
                       v3d_qpu_writes_tmu_not_tmuc(devinfo, a)))) {
                        return false;
                }
        }

        /* Only one TMU read per instruction */
        if ((a_peripherals & V3D_PERIPHERAL_TMU_READ) &&
            (b_peripherals & V3D_PERIPHERAL_TMU_READ)) {
                return false;
        }

        /* Only one TLB access per instruction */
        if ((a_peripherals & (V3D_PERIPHERAL_TLB_WRITE |
                              V3D_PERIPHERAL_TLB_READ)) &&
            (b_peripherals & (V3D_PERIPHERAL_TLB_WRITE |
                              V3D_PERIPHERAL_TLB_READ))) {
                return false;
        }

        return true;
}

/* Compute a bitmask of which rf registers are used between
 * the two instructions.
 */
static uint64_t
qpu_raddrs_used(const struct v3d_qpu_instr *a,
                const struct v3d_qpu_instr *b)
{
        assert(a->type == V3D_QPU_INSTR_TYPE_ALU);
        assert(b->type == V3D_QPU_INSTR_TYPE_ALU);

        uint64_t raddrs_used = 0;
        if (v3d_qpu_uses_mux(a, V3D_QPU_MUX_A))
                raddrs_used |= (1ll << a->raddr_a);
        if (!a->sig.small_imm_b && v3d_qpu_uses_mux(a, V3D_QPU_MUX_B))
                raddrs_used |= (1ll << a->raddr_b);
        if (v3d_qpu_uses_mux(b, V3D_QPU_MUX_A))
                raddrs_used |= (1ll << b->raddr_a);
        if (!b->sig.small_imm_b && v3d_qpu_uses_mux(b, V3D_QPU_MUX_B))
                raddrs_used |= (1ll << b->raddr_b);

        return raddrs_used;
}

/* Takes two instructions and attempts to merge their raddr fields (including
 * small immediates) into one merged instruction. For V3D 4.x, returns false
 * if the two instructions access more than two different rf registers between
 * them, or more than one rf register and one small immediate. For 7.x returns
 * false if both instructions use small immediates.
 */
static bool
qpu_merge_raddrs(struct v3d_qpu_instr *result,
                 const struct v3d_qpu_instr *add_instr,
                 const struct v3d_qpu_instr *mul_instr,
                 const struct v3d_device_info *devinfo)
{
        if (devinfo->ver >= 71) {
                assert(add_instr->sig.small_imm_a +
                       add_instr->sig.small_imm_b <= 1);
                assert(add_instr->sig.small_imm_c +
                       add_instr->sig.small_imm_d == 0);
                assert(mul_instr->sig.small_imm_a +
                       mul_instr->sig.small_imm_b == 0);
                assert(mul_instr->sig.small_imm_c +
                       mul_instr->sig.small_imm_d <= 1);

                result->sig.small_imm_a = add_instr->sig.small_imm_a;
                result->sig.small_imm_b = add_instr->sig.small_imm_b;
                result->sig.small_imm_c = mul_instr->sig.small_imm_c;
                result->sig.small_imm_d = mul_instr->sig.small_imm_d;

                return (result->sig.small_imm_a +
                        result->sig.small_imm_b +
                        result->sig.small_imm_c +
                        result->sig.small_imm_d) <= 1;
        }

        assert(devinfo->ver == 42);

        uint64_t raddrs_used = qpu_raddrs_used(add_instr, mul_instr);
        int naddrs = util_bitcount64(raddrs_used);

        if (naddrs > 2)
                return false;

        if ((add_instr->sig.small_imm_b || mul_instr->sig.small_imm_b)) {
                if (naddrs > 1)
                        return false;

                if (add_instr->sig.small_imm_b && mul_instr->sig.small_imm_b)
                        if (add_instr->raddr_b != mul_instr->raddr_b)
                                return false;

                result->sig.small_imm_b = true;
                result->raddr_b = add_instr->sig.small_imm_b ?
                        add_instr->raddr_b : mul_instr->raddr_b;
        }

        if (naddrs == 0)
                return true;

        int raddr_a = ffsll(raddrs_used) - 1;
        raddrs_used &= ~(1ll << raddr_a);
        result->raddr_a = raddr_a;

        if (!result->sig.small_imm_b) {
                if (v3d_qpu_uses_mux(add_instr, V3D_QPU_MUX_B) &&
                    raddr_a == add_instr->raddr_b) {
                        if (add_instr->alu.add.a.mux == V3D_QPU_MUX_B)
                                result->alu.add.a.mux = V3D_QPU_MUX_A;
                        if (add_instr->alu.add.b.mux == V3D_QPU_MUX_B &&
                            v3d_qpu_add_op_num_src(add_instr->alu.add.op) > 1) {
                                result->alu.add.b.mux = V3D_QPU_MUX_A;
                        }
                }
                if (v3d_qpu_uses_mux(mul_instr, V3D_QPU_MUX_B) &&
                    raddr_a == mul_instr->raddr_b) {
                        if (mul_instr->alu.mul.a.mux == V3D_QPU_MUX_B)
                                result->alu.mul.a.mux = V3D_QPU_MUX_A;
                        if (mul_instr->alu.mul.b.mux == V3D_QPU_MUX_B &&
                            v3d_qpu_mul_op_num_src(mul_instr->alu.mul.op) > 1) {
                                result->alu.mul.b.mux = V3D_QPU_MUX_A;
                        }
                }
        }
        if (!raddrs_used)
                return true;

        int raddr_b = ffsll(raddrs_used) - 1;
        result->raddr_b = raddr_b;
        if (v3d_qpu_uses_mux(add_instr, V3D_QPU_MUX_A) &&
            raddr_b == add_instr->raddr_a) {
                if (add_instr->alu.add.a.mux == V3D_QPU_MUX_A)
                        result->alu.add.a.mux = V3D_QPU_MUX_B;
                if (add_instr->alu.add.b.mux == V3D_QPU_MUX_A &&
                    v3d_qpu_add_op_num_src(add_instr->alu.add.op) > 1) {
                        result->alu.add.b.mux = V3D_QPU_MUX_B;
                }
        }
        if (v3d_qpu_uses_mux(mul_instr, V3D_QPU_MUX_A) &&
            raddr_b == mul_instr->raddr_a) {
                if (mul_instr->alu.mul.a.mux == V3D_QPU_MUX_A)
                        result->alu.mul.a.mux = V3D_QPU_MUX_B;
                if (mul_instr->alu.mul.b.mux == V3D_QPU_MUX_A &&
                    v3d_qpu_mul_op_num_src(mul_instr->alu.mul.op) > 1) {
                        result->alu.mul.b.mux = V3D_QPU_MUX_B;
                }
        }

        return true;
}

static bool
can_do_add_as_mul(enum v3d_qpu_add_op op)
{
        switch (op) {
        case V3D_QPU_A_ADD:
        case V3D_QPU_A_SUB:
                return true;
        default:
                return false;
        }
}

static enum v3d_qpu_mul_op
add_op_as_mul_op(enum v3d_qpu_add_op op)
{
        switch (op) {
        case V3D_QPU_A_ADD:
                return V3D_QPU_M_ADD;
        case V3D_QPU_A_SUB:
                return V3D_QPU_M_SUB;
        default:
                unreachable("unexpected add opcode");
        }
}

static void
qpu_convert_add_to_mul(const struct v3d_device_info *devinfo,
                       struct v3d_qpu_instr *inst)
{
        STATIC_ASSERT(sizeof(inst->alu.mul) == sizeof(inst->alu.add));
        assert(inst->alu.add.op != V3D_QPU_A_NOP);
        assert(inst->alu.mul.op == V3D_QPU_M_NOP);

        memcpy(&inst->alu.mul, &inst->alu.add, sizeof(inst->alu.mul));
        inst->alu.mul.op = add_op_as_mul_op(inst->alu.add.op);
        inst->alu.add.op = V3D_QPU_A_NOP;

        inst->flags.mc = inst->flags.ac;
        inst->flags.mpf = inst->flags.apf;
        inst->flags.muf = inst->flags.auf;
        inst->flags.ac = V3D_QPU_COND_NONE;
        inst->flags.apf = V3D_QPU_PF_NONE;
        inst->flags.auf = V3D_QPU_UF_NONE;

        inst->alu.mul.output_pack = inst->alu.add.output_pack;

        inst->alu.mul.a.unpack = inst->alu.add.a.unpack;
        inst->alu.mul.b.unpack = inst->alu.add.b.unpack;
        inst->alu.add.output_pack = V3D_QPU_PACK_NONE;
        inst->alu.add.a.unpack = V3D_QPU_UNPACK_NONE;
        inst->alu.add.b.unpack = V3D_QPU_UNPACK_NONE;

        if (devinfo->ver >= 71) {
                assert(!inst->sig.small_imm_c && !inst->sig.small_imm_d);
                assert(inst->sig.small_imm_a + inst->sig.small_imm_b <= 1);
                if (inst->sig.small_imm_a) {
                        inst->sig.small_imm_c = true;
                        inst->sig.small_imm_a = false;
                } else if (inst->sig.small_imm_b) {
                        inst->sig.small_imm_d = true;
                        inst->sig.small_imm_b = false;
                }
        }
}

static bool
can_do_mul_as_add(const struct v3d_device_info *devinfo, enum v3d_qpu_mul_op op)
{
        switch (op) {
        case V3D_QPU_M_MOV:
        case V3D_QPU_M_FMOV:
                return devinfo->ver >= 71;
        default:
                return false;
        }
}

static enum v3d_qpu_mul_op
mul_op_as_add_op(enum v3d_qpu_mul_op op)
{
        switch (op) {
        case V3D_QPU_M_MOV:
                return V3D_QPU_A_MOV;
        case V3D_QPU_M_FMOV:
                return V3D_QPU_A_FMOV;
        default:
                unreachable("unexpected mov opcode");
        }
}

static void
qpu_convert_mul_to_add(struct v3d_qpu_instr *inst)
{
        STATIC_ASSERT(sizeof(inst->alu.add) == sizeof(inst->alu.mul));
        assert(inst->alu.mul.op != V3D_QPU_M_NOP);
        assert(inst->alu.add.op == V3D_QPU_A_NOP);

        memcpy(&inst->alu.add, &inst->alu.mul, sizeof(inst->alu.add));
        inst->alu.add.op = mul_op_as_add_op(inst->alu.mul.op);
        inst->alu.mul.op = V3D_QPU_M_NOP;

        inst->flags.ac = inst->flags.mc;
        inst->flags.apf = inst->flags.mpf;
        inst->flags.auf = inst->flags.muf;
        inst->flags.mc = V3D_QPU_COND_NONE;
        inst->flags.mpf = V3D_QPU_PF_NONE;
        inst->flags.muf = V3D_QPU_UF_NONE;

        inst->alu.add.output_pack = inst->alu.mul.output_pack;
        inst->alu.add.a.unpack = inst->alu.mul.a.unpack;
        inst->alu.add.b.unpack = inst->alu.mul.b.unpack;
        inst->alu.mul.output_pack = V3D_QPU_PACK_NONE;
        inst->alu.mul.a.unpack = V3D_QPU_UNPACK_NONE;
        inst->alu.mul.b.unpack = V3D_QPU_UNPACK_NONE;

        assert(!inst->sig.small_imm_a && !inst->sig.small_imm_b);
        assert(inst->sig.small_imm_c + inst->sig.small_imm_d <= 1);
        if (inst->sig.small_imm_c) {
                inst->sig.small_imm_a = true;
                inst->sig.small_imm_c = false;
        } else if (inst->sig.small_imm_d) {
                inst->sig.small_imm_b = true;
                inst->sig.small_imm_d = false;
        }
}

static bool
qpu_merge_inst(const struct v3d_device_info *devinfo,
               struct v3d_qpu_instr *result,
               const struct v3d_qpu_instr *a,
               const struct v3d_qpu_instr *b)
{
        if (a->type != V3D_QPU_INSTR_TYPE_ALU ||
            b->type != V3D_QPU_INSTR_TYPE_ALU) {
                return false;
        }

        if (!qpu_compatible_peripheral_access(devinfo, a, b))
                return false;

        struct v3d_qpu_instr merge = *a;
        const struct v3d_qpu_instr *add_instr = NULL, *mul_instr = NULL;

        struct v3d_qpu_instr mul_inst;
        if (b->alu.add.op != V3D_QPU_A_NOP) {
                if (a->alu.add.op == V3D_QPU_A_NOP) {
                        merge.alu.add = b->alu.add;

                        merge.flags.ac = b->flags.ac;
                        merge.flags.apf = b->flags.apf;
                        merge.flags.auf = b->flags.auf;

                        add_instr = b;
                        mul_instr = a;
                }
                /* If a's add op is used but its mul op is not, then see if we
                 * can convert either a's add op or b's add op to a mul op
                 * so we can merge.
                 */
                else if (a->alu.mul.op == V3D_QPU_M_NOP &&
                         can_do_add_as_mul(b->alu.add.op)) {
                        mul_inst = *b;
                        qpu_convert_add_to_mul(devinfo, &mul_inst);

                        merge.alu.mul = mul_inst.alu.mul;

                        merge.flags.mc = mul_inst.flags.mc;
                        merge.flags.mpf = mul_inst.flags.mpf;
                        merge.flags.muf = mul_inst.flags.muf;

                        add_instr = a;
                        mul_instr = &mul_inst;
                } else if (a->alu.mul.op == V3D_QPU_M_NOP &&
                           can_do_add_as_mul(a->alu.add.op)) {
                        mul_inst = *a;
                        qpu_convert_add_to_mul(devinfo, &mul_inst);

                        merge = mul_inst;
                        merge.alu.add = b->alu.add;

                        merge.flags.ac = b->flags.ac;
                        merge.flags.apf = b->flags.apf;
                        merge.flags.auf = b->flags.auf;

                        add_instr = b;
                        mul_instr = &mul_inst;
                } else {
                        return false;
                }
        }

        struct v3d_qpu_instr add_inst;
        if (b->alu.mul.op != V3D_QPU_M_NOP) {
                if (a->alu.mul.op == V3D_QPU_M_NOP) {
                        merge.alu.mul = b->alu.mul;

                        merge.flags.mc = b->flags.mc;
                        merge.flags.mpf = b->flags.mpf;
                        merge.flags.muf = b->flags.muf;

                        mul_instr = b;
                        add_instr = a;
                }
                /* If a's mul op is used but its add op is not, then see if we
                 * can convert either a's mul op or b's mul op to an add op
                 * so we can merge.
                 */
                else if (a->alu.add.op == V3D_QPU_A_NOP &&
                         can_do_mul_as_add(devinfo, b->alu.mul.op)) {
                        add_inst = *b;
                        qpu_convert_mul_to_add(&add_inst);

                        merge.alu.add = add_inst.alu.add;

                        merge.flags.ac = add_inst.flags.ac;
                        merge.flags.apf = add_inst.flags.apf;
                        merge.flags.auf = add_inst.flags.auf;

                        mul_instr = a;
                        add_instr = &add_inst;
                } else if (a->alu.add.op == V3D_QPU_A_NOP &&
                           can_do_mul_as_add(devinfo, a->alu.mul.op)) {
                        add_inst = *a;
                        qpu_convert_mul_to_add(&add_inst);

                        merge = add_inst;
                        merge.alu.mul = b->alu.mul;

                        merge.flags.mc = b->flags.mc;
                        merge.flags.mpf = b->flags.mpf;
                        merge.flags.muf = b->flags.muf;

                        mul_instr = b;
                        add_instr = &add_inst;
                } else {
                        return false;
                }
        }

        /* V3D 4.x and earlier use muxes to select the inputs for the ALUs and
         * they have restrictions on the number of raddrs that can be adressed
         * in a single instruction. In V3D 7.x, we don't have that restriction,
         * but we are still limited to a single small immediate per instruction.
         */
        if (add_instr && mul_instr &&
            !qpu_merge_raddrs(&merge, add_instr, mul_instr, devinfo)) {
                return false;
        }

        merge.sig.thrsw |= b->sig.thrsw;
        merge.sig.ldunif |= b->sig.ldunif;
        merge.sig.ldunifrf |= b->sig.ldunifrf;
        merge.sig.ldunifa |= b->sig.ldunifa;
        merge.sig.ldunifarf |= b->sig.ldunifarf;
        merge.sig.ldtmu |= b->sig.ldtmu;
        merge.sig.ldvary |= b->sig.ldvary;
        merge.sig.ldvpm |= b->sig.ldvpm;
        merge.sig.ldtlb |= b->sig.ldtlb;
        merge.sig.ldtlbu |= b->sig.ldtlbu;
        merge.sig.ucb |= b->sig.ucb;
        merge.sig.rotate |= b->sig.rotate;
        merge.sig.wrtmuc |= b->sig.wrtmuc;

        if (v3d_qpu_sig_writes_address(devinfo, &a->sig) &&
            v3d_qpu_sig_writes_address(devinfo, &b->sig))
                return false;
        merge.sig_addr |= b->sig_addr;
        merge.sig_magic |= b->sig_magic;

        uint64_t packed;
        bool ok = v3d_qpu_instr_pack(devinfo, &merge, &packed);

        *result = merge;
        /* No modifying the real instructions on failure. */
        assert(ok || (a != result && b != result));

        return ok;
}

static inline bool
try_skip_for_ldvary_pipelining(const struct v3d_qpu_instr *inst)
{
        return inst->sig.ldunif || inst->sig.ldunifrf;
}

static bool
qpu_inst_after_thrsw_valid_in_delay_slot(struct v3d_compile *c,
                                         struct choose_scoreboard *scoreboard,
                                         const struct qinst *qinst);

static struct schedule_node *
choose_instruction_to_schedule(struct v3d_compile *c,
                               struct choose_scoreboard *scoreboard,
                               struct schedule_node *prev_inst)
{
        struct schedule_node *chosen = NULL;
        int chosen_prio = 0;

        /* Don't pair up anything with a thread switch signal -- emit_thrsw()
         * will handle pairing it along with filling the delay slots.
         */
        if (prev_inst) {
                if (prev_inst->inst->qpu.sig.thrsw)
                        return NULL;
        }

        bool ldvary_pipelining = c->s->info.stage == MESA_SHADER_FRAGMENT &&
                                 scoreboard->ldvary_count < c->num_inputs;
        bool skipped_insts_for_ldvary_pipelining = false;
retry:
        list_for_each_entry(struct schedule_node, n, &scoreboard->dag->heads,
                            dag.link) {
                const struct v3d_qpu_instr *inst = &n->inst->qpu;

                if (ldvary_pipelining && try_skip_for_ldvary_pipelining(inst)) {
                        skipped_insts_for_ldvary_pipelining = true;
                        continue;
                }

                /* Don't choose the branch instruction until it's the last one
                 * left.  We'll move it up to fit its delay slots after we
                 * choose it.
                 */
                if (inst->type == V3D_QPU_INSTR_TYPE_BRANCH &&
                    !list_is_singular(&scoreboard->dag->heads)) {
                        continue;
                }

                /* We need to have 3 delay slots between a write to unifa and
                 * a follow-up ldunifa.
                 */
                if ((inst->sig.ldunifa || inst->sig.ldunifarf) &&
                    scoreboard->tick - scoreboard->last_unifa_write_tick <= 3)
                        continue;

                /* "An instruction must not read from a location in physical
                 *  regfile A or B that was written to by the previous
                 *  instruction."
                 */
                if (reads_too_soon_after_write(c->devinfo, scoreboard, n->inst))
                        continue;

                if (writes_too_soon_after_write(c->devinfo, scoreboard, n->inst))
                        continue;

                /* "Before doing a TLB access a scoreboard wait must have been
                 *  done. This happens either on the first or last thread
                 *  switch, depending on a setting (scb_wait_on_first_thrsw) in
                 *  the shader state."
                 */
                if (pixel_scoreboard_too_soon(c, scoreboard, inst))
                        continue;

                /* ldunif and ldvary both write the same register (r5 for v42
                 * and below, rf0 for v71), but ldunif does so a tick sooner.
                 * If the ldvary's register wasn't used, then ldunif might
                 * otherwise get scheduled so ldunif and ldvary try to update
                 * the register in the same tick.
                 */
                if ((inst->sig.ldunif || inst->sig.ldunifa) &&
                    scoreboard->tick == scoreboard->last_ldvary_tick + 1) {
                        continue;
                }

                /* If we are in a thrsw delay slot check that this instruction
                 * is valid for that.
                 */
                if (scoreboard->last_thrsw_tick + 2 >= scoreboard->tick &&
                    !qpu_inst_after_thrsw_valid_in_delay_slot(c, scoreboard,
                                                              n->inst)) {
                        continue;
                }

                if (inst->type == V3D_QPU_INSTR_TYPE_BRANCH) {
                        /* Don't try to put a branch in the delay slots of another
                         * branch or a unifa write.
                         */
                        if (scoreboard->last_branch_tick + 3 >= scoreboard->tick)
                                continue;
                        if (scoreboard->last_unifa_write_tick + 3 >= scoreboard->tick)
                                continue;

                        /* No branch with cond != 0,2,3 and msfign != 0 after
                         * setmsf.
                         */
                        if (scoreboard->last_setmsf_tick == scoreboard->tick - 1 &&
                            inst->branch.msfign != V3D_QPU_MSFIGN_NONE &&
                            inst->branch.cond != V3D_QPU_BRANCH_COND_ALWAYS &&
                            inst->branch.cond != V3D_QPU_BRANCH_COND_A0 &&
                            inst->branch.cond != V3D_QPU_BRANCH_COND_NA0) {
                                continue;
                        }
                }

                /* If we're trying to pair with another instruction, check
                 * that they're compatible.
                 */
                if (prev_inst) {
                        /* Don't pair up a thread switch signal -- we'll
                         * handle pairing it when we pick it on its own.
                         */
                        if (inst->sig.thrsw)
                                continue;

                        if (prev_inst->inst->uniform != -1 &&
                            n->inst->uniform != -1)
                                continue;

                       /* Simulator complains if we have two uniforms loaded in
                        * the the same instruction, which could happen if we
                        * have a ldunif or sideband uniform and we pair that
                        * with ldunifa.
                        */
                        if (vir_has_uniform(prev_inst->inst) &&
                            (inst->sig.ldunifa || inst->sig.ldunifarf)) {
                                continue;
                        }

                        if ((prev_inst->inst->qpu.sig.ldunifa ||
                             prev_inst->inst->qpu.sig.ldunifarf) &&
                            vir_has_uniform(n->inst)) {
                                continue;
                        }

                        /* Don't merge TLB instructions before we have acquired
                         * the scoreboard lock.
                         */
                        if (pixel_scoreboard_too_soon(c, scoreboard, inst))
                                continue;

                        /* When we successfully pair up an ldvary we then try
                         * to merge it into the previous instruction if
                         * possible to improve pipelining. Don't pick up the
                         * ldvary now if the follow-up fixup would place
                         * it in the delay slots of a thrsw, which is not
                         * allowed and would prevent the fixup from being
                         * successful. In V3D 7.x we can allow this to happen
                         * as long as it is not the last delay slot.
                         */
                        if (inst->sig.ldvary) {
                                if (c->devinfo->ver == 42 &&
                                    scoreboard->last_thrsw_tick + 2 >=
                                    scoreboard->tick - 1) {
                                        continue;
                                }
                                if (c->devinfo->ver >= 71 &&
                                    scoreboard->last_thrsw_tick + 2 ==
                                    scoreboard->tick - 1) {
                                        continue;
                                }
                        }

                        /* We can emit a new tmu lookup with a previous ldtmu
                         * if doing this would free just enough space in the
                         * TMU output fifo so we don't overflow, however, this
                         * is only safe if the ldtmu cannot stall.
                         *
                         * A ldtmu can stall if it is not the first following a
                         * thread switch and corresponds to the first word of a
                         * read request.
                         *
                         * FIXME: For now we forbid pairing up a new lookup
                         * with a previous ldtmu that is not the first after a
                         * thrsw if that could overflow the TMU output fifo
                         * regardless of whether the ldtmu is reading the first
                         * word of a TMU result or not, since we don't track
                         * this aspect in the compiler yet.
                         */
                        if (prev_inst->inst->qpu.sig.ldtmu &&
                            !scoreboard->first_ldtmu_after_thrsw &&
                            (scoreboard->pending_ldtmu_count +
                             n->inst->ldtmu_count > 16 / c->threads)) {
                                continue;
                        }

                        struct v3d_qpu_instr merged_inst;
                        if (!qpu_merge_inst(c->devinfo, &merged_inst,
                                            &prev_inst->inst->qpu, inst)) {
                                continue;
                        }
                }

                int prio = get_instruction_priority(c->devinfo, inst);

                if (read_stalls(c->devinfo, scoreboard, inst)) {
                        /* Don't merge an instruction that stalls */
                        if (prev_inst)
                                continue;
                        else {
                                /* Any instruction that don't stall will have
                                 * higher scheduling priority */
                                prio -= MAX_SCHEDULE_PRIORITY;
                                assert(prio < 0);
                        }
                }

                /* Found a valid instruction.  If nothing better comes along,
                 * this one works.
                 */
                if (!chosen) {
                        chosen = n;
                        chosen_prio = prio;
                        continue;
                }

                if (prio > chosen_prio) {
                        chosen = n;
                        chosen_prio = prio;
                } else if (prio < chosen_prio) {
                        continue;
                }

                if (n->delay > chosen->delay) {
                        chosen = n;
                        chosen_prio = prio;
                } else if (n->delay < chosen->delay) {
                        continue;
                }
        }

        /* If we did not find any instruction to schedule but we discarded
         * some of them to prioritize ldvary pipelining, try again.
         */
        if (!chosen && !prev_inst && skipped_insts_for_ldvary_pipelining) {
                skipped_insts_for_ldvary_pipelining = false;
                ldvary_pipelining = false;
                goto retry;
        }

        if (chosen && chosen->inst->qpu.sig.ldvary) {
                scoreboard->ldvary_count++;
                /* If we are pairing an ldvary, flag it so we can fix it up for
                 * optimal pipelining of ldvary sequences.
                 */
                if (prev_inst)
                        scoreboard->fixup_ldvary = true;
        }

        return chosen;
}

static void
update_scoreboard_for_magic_waddr(struct choose_scoreboard *scoreboard,
                                  enum v3d_qpu_waddr waddr,
                                  const struct v3d_device_info *devinfo)
{
        if (v3d_qpu_magic_waddr_is_sfu(waddr))
                scoreboard->last_magic_sfu_write_tick = scoreboard->tick;
        else if (waddr == V3D_QPU_WADDR_UNIFA)
                scoreboard->last_unifa_write_tick = scoreboard->tick;
}

static void
update_scoreboard_for_sfu_stall_waddr(struct choose_scoreboard *scoreboard,
                                      const struct v3d_qpu_instr *inst)
{
        if (v3d_qpu_instr_is_sfu(inst)) {
                scoreboard->last_stallable_sfu_reg = inst->alu.add.waddr;
                scoreboard->last_stallable_sfu_tick = scoreboard->tick;
        }
}

static void
update_scoreboard_tmu_tracking(struct choose_scoreboard *scoreboard,
                               const struct qinst *inst)
{
        /* Track if the have seen any ldtmu after the last thread switch */
        if (scoreboard->tick == scoreboard->last_thrsw_tick + 2)
                scoreboard->first_ldtmu_after_thrsw = true;

        /* Track the number of pending ldtmu instructions for outstanding
         * TMU lookups.
         */
        scoreboard->pending_ldtmu_count += inst->ldtmu_count;
        if (inst->qpu.sig.ldtmu) {
                assert(scoreboard->pending_ldtmu_count > 0);
                scoreboard->pending_ldtmu_count--;
                scoreboard->first_ldtmu_after_thrsw = false;
        }
}

static void
set_has_rf0_flops_conflict(struct choose_scoreboard *scoreboard,
                           const struct v3d_qpu_instr *inst,
                           const struct v3d_device_info *devinfo)
{
        if (scoreboard->last_implicit_rf0_write_tick == scoreboard->tick &&
            v3d_qpu_sig_writes_address(devinfo, &inst->sig) &&
            !inst->sig_magic) {
                scoreboard->has_rf0_flops_conflict = true;
        }
}

static void
update_scoreboard_for_rf0_flops(struct choose_scoreboard *scoreboard,
                                const struct v3d_qpu_instr *inst,
                                const struct v3d_device_info *devinfo)
{
        if (devinfo->ver < 71)
                return;

        /* Thread switch restrictions:
         *
         * At the point of a thread switch or thread end (when the actual
         * thread switch or thread end happens, not when the signalling
         * instruction is processed):
         *
         *    - If the most recent write to rf0 was from a ldunif, ldunifa, or
         *      ldvary instruction in which another signal also wrote to the
         *      register file, and the final instruction of the thread section
         *      contained a signal which wrote to the register file, then the
         *      value of rf0 is undefined at the start of the new section
         *
         * Here we use the scoreboard to track if our last rf0 implicit write
         * happens at the same time that another signal writes the register
         * file (has_rf0_flops_conflict). We will use that information when
         * scheduling thrsw instructions to avoid putting anything in their
         * last delay slot which has a signal that writes to the register file.
         */

        /* Reset tracking if we have an explicit rf0 write or we are starting
         * a new thread section.
         */
        if (v3d71_qpu_writes_waddr_explicitly(devinfo, inst, 0) ||
            scoreboard->tick - scoreboard->last_thrsw_tick == 3) {
                scoreboard->last_implicit_rf0_write_tick = -10;
                scoreboard->has_rf0_flops_conflict = false;
        }

        if (v3d_qpu_writes_rf0_implicitly(devinfo, inst)) {
                scoreboard->last_implicit_rf0_write_tick = inst->sig.ldvary ?
                        scoreboard->tick + 1 : scoreboard->tick;
        }

        set_has_rf0_flops_conflict(scoreboard, inst, devinfo);
}

static void
update_scoreboard_for_chosen(struct choose_scoreboard *scoreboard,
                             const struct qinst *qinst,
                             const struct v3d_device_info *devinfo)
{
        const struct v3d_qpu_instr *inst = &qinst->qpu;

        if (inst->type == V3D_QPU_INSTR_TYPE_BRANCH)
                return;

        assert(inst->type == V3D_QPU_INSTR_TYPE_ALU);

        if (inst->alu.add.op != V3D_QPU_A_NOP)  {
                if (inst->alu.add.magic_write) {
                        update_scoreboard_for_magic_waddr(scoreboard,
                                                          inst->alu.add.waddr,
                                                          devinfo);
                } else {
                        update_scoreboard_for_sfu_stall_waddr(scoreboard,
                                                              inst);
                }

                if (inst->alu.add.op == V3D_QPU_A_SETMSF)
                        scoreboard->last_setmsf_tick = scoreboard->tick;
        }

        if (inst->alu.mul.op != V3D_QPU_M_NOP) {
                if (inst->alu.mul.magic_write) {
                        update_scoreboard_for_magic_waddr(scoreboard,
                                                          inst->alu.mul.waddr,
                                                          devinfo);
                }
        }

        if (v3d_qpu_sig_writes_address(devinfo, &inst->sig) && inst->sig_magic) {
                update_scoreboard_for_magic_waddr(scoreboard,
                                                  inst->sig_addr,
                                                  devinfo);
        }

        if (inst->sig.ldvary)
                scoreboard->last_ldvary_tick = scoreboard->tick;

        update_scoreboard_for_rf0_flops(scoreboard, inst, devinfo);

        update_scoreboard_tmu_tracking(scoreboard, qinst);
}

static void
dump_state(const struct v3d_device_info *devinfo, struct dag *dag)
{
        list_for_each_entry(struct schedule_node, n, &dag->heads, dag.link) {
                fprintf(stderr, "         t=%4d: ", n->unblocked_time);
                v3d_qpu_dump(devinfo, &n->inst->qpu);
                fprintf(stderr, "\n");

                util_dynarray_foreach(&n->dag.edges, struct dag_edge, edge) {
                        struct schedule_node *child =
                                (struct schedule_node *)edge->child;
                        if (!child)
                                continue;

                        fprintf(stderr, "                 - ");
                        v3d_qpu_dump(devinfo, &child->inst->qpu);
                        fprintf(stderr, " (%d parents, %c)\n",
                                child->dag.parent_count,
                                edge->data ? 'w' : 'r');
                }
        }
}

static uint32_t magic_waddr_latency(const struct v3d_device_info *devinfo,
                                    enum v3d_qpu_waddr waddr,
                                    const struct v3d_qpu_instr *after)
{
        /* Apply some huge latency between texture fetch requests and getting
         * their results back.
         *
         * FIXME: This is actually pretty bogus.  If we do:
         *
         * mov tmu0_s, a
         * <a bit of math>
         * mov tmu0_s, b
         * load_tmu0
         * <more math>
         * load_tmu0
         *
         * we count that as worse than
         *
         * mov tmu0_s, a
         * mov tmu0_s, b
         * <lots of math>
         * load_tmu0
         * <more math>
         * load_tmu0
         *
         * because we associate the first load_tmu0 with the *second* tmu0_s.
         */
        if (v3d_qpu_magic_waddr_is_tmu(devinfo, waddr) &&
            v3d_qpu_waits_on_tmu(after)) {
                return 100;
        }

        /* Assume that anything depending on us is consuming the SFU result. */
        if (v3d_qpu_magic_waddr_is_sfu(waddr))
                return 3;

        return 1;
}

static uint32_t
instruction_latency(const struct v3d_device_info *devinfo,
                    struct schedule_node *before, struct schedule_node *after)
{
        const struct v3d_qpu_instr *before_inst = &before->inst->qpu;
        const struct v3d_qpu_instr *after_inst = &after->inst->qpu;
        uint32_t latency = 1;

        if (before_inst->type != V3D_QPU_INSTR_TYPE_ALU ||
            after_inst->type != V3D_QPU_INSTR_TYPE_ALU)
                return latency;

        if (v3d_qpu_instr_is_sfu(before_inst))
                return 2;

        if (before_inst->alu.add.op != V3D_QPU_A_NOP &&
            before_inst->alu.add.magic_write) {
                latency = MAX2(latency,
                               magic_waddr_latency(devinfo,
                                                   before_inst->alu.add.waddr,
                                                   after_inst));
        }

        if (before_inst->alu.mul.op != V3D_QPU_M_NOP &&
            before_inst->alu.mul.magic_write) {
                latency = MAX2(latency,
                               magic_waddr_latency(devinfo,
                                                   before_inst->alu.mul.waddr,
                                                   after_inst));
        }

        return latency;
}

/** Recursive computation of the delay member of a node. */
static void
compute_delay(struct dag_node *node, void *state)
{
        struct schedule_node *n = (struct schedule_node *)node;
        struct v3d_compile *c = (struct v3d_compile *) state;

        n->delay = 1;

        util_dynarray_foreach(&n->dag.edges, struct dag_edge, edge) {
                struct schedule_node *child =
                        (struct schedule_node *)edge->child;

                n->delay = MAX2(n->delay, (child->delay +
                                           instruction_latency(c->devinfo, n,
                                                               child)));
        }
}

/* Removes a DAG head, but removing only the WAR edges. (dag_prune_head()
 * should be called on it later to finish pruning the other edges).
 */
static void
pre_remove_head(struct dag *dag, struct schedule_node *n)
{
        list_delinit(&n->dag.link);

        util_dynarray_foreach(&n->dag.edges, struct dag_edge, edge) {
                if (edge->data)
                        dag_remove_edge(dag, edge);
        }
}

static void
mark_instruction_scheduled(const struct v3d_device_info *devinfo,
                           struct dag *dag,
                           uint32_t time,
                           struct schedule_node *node)
{
        if (!node)
                return;

        util_dynarray_foreach(&node->dag.edges, struct dag_edge, edge) {
                struct schedule_node *child =
                        (struct schedule_node *)edge->child;

                if (!child)
                        continue;

                uint32_t latency = instruction_latency(devinfo, node, child);

                child->unblocked_time = MAX2(child->unblocked_time,
                                             time + latency);
        }
        dag_prune_head(dag, &node->dag);
}

static void
insert_scheduled_instruction(struct v3d_compile *c,
                             struct qblock *block,
                             struct choose_scoreboard *scoreboard,
                             struct qinst *inst)
{
        list_addtail(&inst->link, &block->instructions);

        update_scoreboard_for_chosen(scoreboard, inst, c->devinfo);
        c->qpu_inst_count++;
        scoreboard->tick++;
}

static struct qinst *
vir_nop()
{
        struct qreg undef = vir_nop_reg();
        struct qinst *qinst = vir_add_inst(V3D_QPU_A_NOP, undef, undef, undef);

        return qinst;
}

static void
emit_nop(struct v3d_compile *c, struct qblock *block,
         struct choose_scoreboard *scoreboard)
{
        insert_scheduled_instruction(c, block, scoreboard, vir_nop());
}

static bool
qpu_inst_valid_in_thrend_slot(struct v3d_compile *c,
                              const struct qinst *qinst, int slot)
{
        const struct v3d_qpu_instr *inst = &qinst->qpu;

        if (slot == 2 && qinst->is_tlb_z_write)
                return false;

        if (slot > 0 && qinst->uniform != ~0)
                return false;

        if (c->devinfo->ver == 42 && v3d_qpu_waits_vpm(inst))
                return false;

        if (inst->sig.ldvary)
                return false;

        if (inst->type == V3D_QPU_INSTR_TYPE_ALU) {
                /* GFXH-1625: TMUWT not allowed in the final instruction. */
                if (c->devinfo->ver == 42 && slot == 2 &&
                    inst->alu.add.op == V3D_QPU_A_TMUWT) {
                        return false;
                }

                if (c->devinfo->ver == 42) {
                        /* No writing physical registers at the end. */
                        bool add_is_nop = inst->alu.add.op == V3D_QPU_A_NOP;
                        bool mul_is_nop = inst->alu.mul.op == V3D_QPU_M_NOP;
                        if ((!add_is_nop && !inst->alu.add.magic_write) ||
                            (!mul_is_nop && !inst->alu.mul.magic_write)) {
                                return false;
                        }

                        if (v3d_qpu_sig_writes_address(c->devinfo, &inst->sig) &&
                            !inst->sig_magic) {
                                return false;
                        }
                }

                if (c->devinfo->ver >= 71) {
                        /* The thread end instruction must not write to the
                         * register file via the add/mul ALUs.
                         */
                        if (slot == 0 &&
                            (!inst->alu.add.magic_write ||
                             !inst->alu.mul.magic_write)) {
                                return false;
                        }
                }

                if (c->devinfo->ver == 42) {
                        /* RF0-2 might be overwritten during the delay slots by
                         * fragment shader setup.
                         */
                        if (inst->raddr_a < 3 && v3d_qpu_uses_mux(inst, V3D_QPU_MUX_A))
                                return false;

                        if (inst->raddr_b < 3 &&
                            !inst->sig.small_imm_b &&
                            v3d_qpu_uses_mux(inst, V3D_QPU_MUX_B)) {
                                return false;
                        }
                }

                if (c->devinfo->ver >= 71) {
                        /* RF2-3 might be overwritten during the delay slots by
                         * fragment shader setup.
                         */
                        if (v3d71_qpu_reads_raddr(inst, 2) ||
                            v3d71_qpu_reads_raddr(inst, 3)) {
                                return false;
                        }

                        if (v3d71_qpu_writes_waddr_explicitly(c->devinfo, inst, 2) ||
                            v3d71_qpu_writes_waddr_explicitly(c->devinfo, inst, 3)) {
                                return false;
                        }
                }
        }

        return true;
}

/**
 * This is called when trying to merge a thrsw back into the instruction stream
 * of instructions that were scheduled *before* the thrsw signal to fill its
 * delay slots. Because the actual execution of the thrsw happens after the
 * delay slots, it is usually safe to do this, but there are some cases that
 * need special care.
 */
static bool
qpu_inst_before_thrsw_valid_in_delay_slot(struct v3d_compile *c,
                                          struct choose_scoreboard *scoreboard,
                                          const struct qinst *qinst,
                                          uint32_t slot)
{
        /* No scheduling SFU when the result would land in the other
         * thread.  The simulator complains for safety, though it
         * would only occur for dead code in our case.
         */
        if (slot > 0 && v3d_qpu_instr_is_legacy_sfu(&qinst->qpu))
                return false;

        if (qinst->qpu.sig.ldvary) {
                if (c->devinfo->ver == 42 && slot > 0)
                        return false;
                if (c->devinfo->ver >= 71 && slot == 2)
                        return false;
        }

        /* unifa and the following 3 instructions can't overlap a
         * thread switch/end. The docs further clarify that this means
         * the cycle at which the actual thread switch/end happens
         * and not when the thrsw instruction is processed, which would
         * be after the 2 delay slots following the thrsw instruction.
         * This means that we can move up a thrsw up to the instruction
         * right after unifa:
         *
         * unifa, r5
         * thrsw
         * delay slot 1
         * delay slot 2
         * Thread switch happens here, 4 instructions away from unifa
         */
        if (v3d_qpu_writes_unifa(c->devinfo, &qinst->qpu))
                return false;

        /* See comment when we set has_rf0_flops_conflict for details */
        if (c->devinfo->ver >= 71 &&
            slot == 2 &&
            v3d_qpu_sig_writes_address(c->devinfo, &qinst->qpu.sig) &&
            !qinst->qpu.sig_magic) {
                if (scoreboard->has_rf0_flops_conflict)
                        return false;
                if (scoreboard->last_implicit_rf0_write_tick == scoreboard->tick)
                        return false;
        }

        return true;
}

/**
 * This is called for instructions scheduled *after* a thrsw signal that may
 * land in the delay slots of the thrsw. Because these instructions were
 * scheduled after the thrsw, we need to be careful when placing them into
 * the delay slots, since that means that we are moving them ahead of the
 * thread switch and we need to ensure that is not a problem.
 */
static bool
qpu_inst_after_thrsw_valid_in_delay_slot(struct v3d_compile *c,
                                         struct choose_scoreboard *scoreboard,
                                         const struct qinst *qinst)
{
        const uint32_t slot = scoreboard->tick - scoreboard->last_thrsw_tick;
        assert(slot <= 2);

        /* We merge thrsw instructions back into the instruction stream
         * manually, so any instructions scheduled after a thrsw should be
         * in the actual delay slots and not in the same slot as the thrsw.
         */
        assert(slot >= 1);

        /* No emitting a thrsw while the previous thrsw hasn't happened yet. */
        if (qinst->qpu.sig.thrsw)
                return false;

        /* The restrictions for instructions scheduled before the the thrsw
         * also apply to instructions scheduled after the thrsw that we want
         * to place in its delay slots.
         */
        if (!qpu_inst_before_thrsw_valid_in_delay_slot(c, scoreboard, qinst, slot))
                return false;

        /* TLB access is disallowed until scoreboard wait is executed, which
         * we do on the last thread switch.
         */
        if (qpu_inst_is_tlb(&qinst->qpu))
                return false;

        /* Instruction sequence restrictions: Branch is not allowed in delay
         * slots of a thrsw.
         */
        if (qinst->qpu.type == V3D_QPU_INSTR_TYPE_BRANCH)
                return false;

        /* Miscellaneous restrictions: At the point of a thrsw we need to have
         * at least one outstanding lookup or TSY wait.
         *
         * So avoid placing TMU instructions scheduled after the thrsw into
         * its delay slots or we may be compromising the integrity of our TMU
         * sequences. Also, notice that if we moved these instructions into
         * the delay slots of a previous thrsw we could overflow our TMU output
         * fifo, since we could be effectively pipelining a lookup scheduled
         * after the thrsw into the sequence before the thrsw.
         */
        if (v3d_qpu_writes_tmu(c->devinfo, &qinst->qpu) ||
            qinst->qpu.sig.wrtmuc) {
                return false;
        }

        /* Don't move instructions that wait on the TMU before the thread switch
         * happens since that would make the current thread stall before the
         * switch, which is exactly what we want to avoid with the thrsw
         * instruction.
         */
        if (v3d_qpu_waits_on_tmu(&qinst->qpu))
                return false;

        /* A thread switch invalidates all accumulators, so don't place any
         * instructions that write accumulators into the delay slots.
         */
        if (v3d_qpu_writes_accum(c->devinfo, &qinst->qpu))
                return false;

        /* Multop has an implicit write to the rtop register which is an
         * specialized accumulator that is only used with this instruction.
         */
        if (qinst->qpu.alu.mul.op == V3D_QPU_M_MULTOP)
                return false;

        /* Flags are invalidated across a thread switch, so dont' place
         * instructions that write flags into delay slots.
         */
        if (v3d_qpu_writes_flags(&qinst->qpu))
                return false;

        /* TSY sync ops materialize at the point of the next thread switch,
         * therefore, if we have a TSY sync right after a thread switch, we
         * cannot place it in its delay slots, or we would be moving the sync
         * to the thrsw before it instead.
         */
        if (qinst->qpu.alu.add.op == V3D_QPU_A_BARRIERID)
                return false;

        return true;
}

static bool
valid_thrsw_sequence(struct v3d_compile *c, struct choose_scoreboard *scoreboard,
                     struct qinst *qinst, int instructions_in_sequence,
                     bool is_thrend)
{
        for (int slot = 0; slot < instructions_in_sequence; slot++) {
                if (!qpu_inst_before_thrsw_valid_in_delay_slot(c, scoreboard,
                                                               qinst, slot)) {
                        return false;
                }

                if (is_thrend &&
                    !qpu_inst_valid_in_thrend_slot(c, qinst, slot)) {
                        return false;
                }

                /* Note that the list is circular, so we can only do this up
                 * to instructions_in_sequence.
                 */
                qinst = (struct qinst *)qinst->link.next;
        }

        return true;
}

/**
 * Emits a THRSW signal in the stream, trying to move it up to pair with
 * another instruction.
 */
static int
emit_thrsw(struct v3d_compile *c,
           struct qblock *block,
           struct choose_scoreboard *scoreboard,
           struct qinst *inst,
           bool is_thrend)
{
        int time = 0;

        /* There should be nothing in a thrsw inst being scheduled other than
         * the signal bits.
         */
        assert(inst->qpu.type == V3D_QPU_INSTR_TYPE_ALU);
        assert(inst->qpu.alu.add.op == V3D_QPU_A_NOP);
        assert(inst->qpu.alu.mul.op == V3D_QPU_M_NOP);

        /* Don't try to emit a thrsw in the delay slots of a previous thrsw
         * or branch.
         */
        while (scoreboard->last_thrsw_tick + 2 >= scoreboard->tick) {
                emit_nop(c, block, scoreboard);
                time++;
        }
        while (scoreboard->last_branch_tick + 3 >= scoreboard->tick) {
                emit_nop(c, block, scoreboard);
                time++;
        }

        /* Find how far back into previous instructions we can put the THRSW. */
        int slots_filled = 0;
        int invalid_sig_count = 0;
        int invalid_seq_count = 0;
        bool last_thrsw_after_invalid_ok = false;
        struct qinst *merge_inst = NULL;
        vir_for_each_inst_rev(prev_inst, block) {
                /* No emitting our thrsw while the previous thrsw hasn't
                 * happened yet.
                 */
                if (scoreboard->last_thrsw_tick + 3 >
                    scoreboard->tick - (slots_filled + 1)) {
                        break;
                }


                if (!valid_thrsw_sequence(c, scoreboard,
                                          prev_inst, slots_filled + 1,
                                          is_thrend)) {
                        /* Even if the current sequence isn't valid, we may
                         * be able to get a valid sequence by trying to move the
                         * thrsw earlier, so keep going.
                         */
                        invalid_seq_count++;
                        goto cont_block;
                }

                struct v3d_qpu_sig sig = prev_inst->qpu.sig;
                sig.thrsw = true;
                uint32_t packed_sig;
                if (!v3d_qpu_sig_pack(c->devinfo, &sig, &packed_sig)) {
                        /* If we can't merge the thrsw here because of signal
                         * incompatibility, keep going, we might be able to
                         * merge it in an earlier instruction.
                         */
                        invalid_sig_count++;
                        goto cont_block;
                }

                /* For last thrsw we need 2 consecutive slots that are
                 * thrsw compatible, so if we have previously jumped over
                 * an incompatible signal, flag that we have found the first
                 * valid slot here and keep going.
                 */
                if (inst->is_last_thrsw && invalid_sig_count > 0 &&
                    !last_thrsw_after_invalid_ok) {
                        last_thrsw_after_invalid_ok = true;
                        invalid_sig_count++;
                        goto cont_block;
                }

                /* We can merge the thrsw in this instruction */
                last_thrsw_after_invalid_ok = false;
                invalid_sig_count = 0;
                invalid_seq_count = 0;
                merge_inst = prev_inst;

cont_block:
                if (++slots_filled == 3)
                        break;
        }

        /* If we jumped over a signal incompatibility and did not manage to
         * merge the thrsw in the end, we need to adjust slots filled to match
         * the last valid merge point.
         */
        assert((invalid_sig_count == 0 && invalid_seq_count == 0) ||
                slots_filled >= invalid_sig_count + invalid_seq_count);
        if (invalid_sig_count > 0)
                slots_filled -= invalid_sig_count;
        if (invalid_seq_count > 0)
                slots_filled -= invalid_seq_count;

        bool needs_free = false;
        if (merge_inst) {
                merge_inst->qpu.sig.thrsw = true;
                needs_free = true;
                scoreboard->last_thrsw_tick = scoreboard->tick - slots_filled;
        } else {
                scoreboard->last_thrsw_tick = scoreboard->tick;
                insert_scheduled_instruction(c, block, scoreboard, inst);
                time++;
                slots_filled++;
                merge_inst = inst;
        }

        scoreboard->first_thrsw_emitted = true;

        /* If we're emitting the last THRSW (other than program end), then
         * signal that to the HW by emitting two THRSWs in a row.
         */
        if (inst->is_last_thrsw) {
                if (slots_filled <= 1) {
                        emit_nop(c, block, scoreboard);
                        time++;
                }
                struct qinst *second_inst =
                        (struct qinst *)merge_inst->link.next;
                second_inst->qpu.sig.thrsw = true;
                scoreboard->last_thrsw_emitted = true;
        }

        /* Make sure the thread end executes within the program lifespan */
        if (is_thrend) {
                for (int i = 0; i < 3 - slots_filled; i++) {
                        emit_nop(c, block, scoreboard);
                        time++;
                }
        }

        /* If we put our THRSW into another instruction, free up the
         * instruction that didn't end up scheduled into the list.
         */
        if (needs_free)
                free(inst);

        return time;
}

static bool
qpu_inst_valid_in_branch_delay_slot(struct v3d_compile *c, struct qinst *inst)
{
        if (inst->qpu.type == V3D_QPU_INSTR_TYPE_BRANCH)
                return false;

        if (inst->qpu.sig.thrsw)
                return false;

        if (v3d_qpu_writes_unifa(c->devinfo, &inst->qpu))
                return false;

        if (vir_has_uniform(inst))
                return false;

        return true;
}

static void
emit_branch(struct v3d_compile *c,
           struct qblock *block,
           struct choose_scoreboard *scoreboard,
           struct qinst *inst)
{
        assert(inst->qpu.type == V3D_QPU_INSTR_TYPE_BRANCH);

        /* We should've not picked up a branch for the delay slots of a previous
         * thrsw, branch or unifa write instruction.
         */
        int branch_tick = scoreboard->tick;
        assert(scoreboard->last_thrsw_tick + 2 < branch_tick);
        assert(scoreboard->last_branch_tick + 3 < branch_tick);
        assert(scoreboard->last_unifa_write_tick + 3 < branch_tick);

        /* V3D 4.x can't place a branch with msfign != 0 and cond != 0,2,3 after
         * setmsf.
         */
        bool is_safe_msf_branch =
                c->devinfo->ver >= 71 ||
                inst->qpu.branch.msfign == V3D_QPU_MSFIGN_NONE ||
                inst->qpu.branch.cond == V3D_QPU_BRANCH_COND_ALWAYS ||
                inst->qpu.branch.cond == V3D_QPU_BRANCH_COND_A0 ||
                inst->qpu.branch.cond == V3D_QPU_BRANCH_COND_NA0;
        assert(scoreboard->last_setmsf_tick != branch_tick - 1 ||
               is_safe_msf_branch);

        /* Insert the branch instruction */
        insert_scheduled_instruction(c, block, scoreboard, inst);

        /* Now see if we can move the branch instruction back into the
         * instruction stream to fill its delay slots
         */
        int slots_filled = 0;
        while (slots_filled < 3 && block->instructions.next != &inst->link) {
                struct qinst *prev_inst = (struct qinst *) inst->link.prev;
                assert(prev_inst->qpu.type != V3D_QPU_INSTR_TYPE_BRANCH);

                /* Can't move the branch instruction if that would place it
                 * in the delay slots of other instructions.
                 */
                if (scoreboard->last_branch_tick + 3 >=
                    branch_tick - slots_filled - 1) {
                        break;
                }

                if (scoreboard->last_thrsw_tick + 2 >=
                    branch_tick - slots_filled - 1) {
                        break;
                }

                if (scoreboard->last_unifa_write_tick + 3 >=
                    branch_tick - slots_filled - 1) {
                        break;
                }

                /* Do not move up a branch if it can disrupt an ldvary sequence
                 * as that can cause stomping of the r5 register.
                 */
                if (scoreboard->last_ldvary_tick + 2 >=
                    branch_tick - slots_filled) {
                       break;
                }

                /* Can't move a conditional branch before the instruction
                 * that writes the flags for its condition.
                 */
                if (v3d_qpu_writes_flags(&prev_inst->qpu) &&
                    inst->qpu.branch.cond != V3D_QPU_BRANCH_COND_ALWAYS) {
                        break;
                }

                if (!qpu_inst_valid_in_branch_delay_slot(c, prev_inst))
                        break;

                if (!is_safe_msf_branch) {
                        struct qinst *prev_prev_inst =
                                (struct qinst *) prev_inst->link.prev;
                        if (prev_prev_inst->qpu.type == V3D_QPU_INSTR_TYPE_ALU &&
                            prev_prev_inst->qpu.alu.add.op == V3D_QPU_A_SETMSF) {
                                break;
                        }
                }

                list_del(&prev_inst->link);
                list_add(&prev_inst->link, &inst->link);
                slots_filled++;
        }

        block->branch_qpu_ip = c->qpu_inst_count - 1 - slots_filled;
        scoreboard->last_branch_tick = branch_tick - slots_filled;

        /* Fill any remaining delay slots.
         *
         * For unconditional branches we'll try to fill these with the
         * first instructions in the successor block after scheduling
         * all blocks when setting up branch targets.
         */
        for (int i = 0; i < 3 - slots_filled; i++)
                emit_nop(c, block, scoreboard);
}

static bool
alu_reads_register(const struct v3d_device_info *devinfo,
                   struct v3d_qpu_instr *inst,
                   bool add, bool magic, uint32_t index)
{
        uint32_t num_src;
        if (add)
                num_src = v3d_qpu_add_op_num_src(inst->alu.add.op);
        else
                num_src = v3d_qpu_mul_op_num_src(inst->alu.mul.op);

        if (devinfo->ver == 42) {
                enum v3d_qpu_mux mux_a, mux_b;
                if (add) {
                        mux_a = inst->alu.add.a.mux;
                        mux_b = inst->alu.add.b.mux;
                } else {
                        mux_a = inst->alu.mul.a.mux;
                        mux_b = inst->alu.mul.b.mux;
                }

                for (int i = 0; i < num_src; i++) {
                        if (magic) {
                                if (i == 0 && mux_a == index)
                                        return true;
                                if (i == 1 && mux_b == index)
                                        return true;
                        } else {
                                if (i == 0 && mux_a == V3D_QPU_MUX_A &&
                                    inst->raddr_a == index) {
                                        return true;
                                }
                                if (i == 0 && mux_a == V3D_QPU_MUX_B &&
                                    inst->raddr_b == index) {
                                        return true;
                                }
                                if (i == 1 && mux_b == V3D_QPU_MUX_A &&
                                    inst->raddr_a == index) {
                                        return true;
                                }
                                if (i == 1 && mux_b == V3D_QPU_MUX_B &&
                                    inst->raddr_b == index) {
                                        return true;
                                }
                        }
                }

                return false;
        }

        assert(devinfo->ver >= 71);
        assert(!magic);

        uint32_t raddr_a, raddr_b;
        if (add) {
                raddr_a = inst->alu.add.a.raddr;
                raddr_b = inst->alu.add.b.raddr;
        } else {
                raddr_a = inst->alu.mul.a.raddr;
                raddr_b = inst->alu.mul.b.raddr;
        }

        for (int i = 0; i < num_src; i++) {
                if (i == 0 && raddr_a == index)
                        return true;
                if (i == 1 && raddr_b == index)
                        return true;
        }

        return false;
}

/**
 * This takes and ldvary signal merged into 'inst' and tries to move it up to
 * the previous instruction to get good pipelining of ldvary sequences,
 * transforming this:
 *
 * nop                  ; nop               ; ldvary.r4
 * nop                  ; fmul  r0, r4, rf0 ;
 * fadd  rf13, r0, r5   ; nop;              ; ldvary.r1  <-- inst
 *
 * into:
 *
 * nop                  ; nop               ; ldvary.r4
 * nop                  ; fmul  r0, r4, rf0 ; ldvary.r1
 * fadd  rf13, r0, r5   ; nop;              ;            <-- inst
 *
 * If we manage to do this successfully (we return true here), then flagging
 * the ldvary as "scheduled" may promote the follow-up fmul to a DAG head that
 * we will be able to pick up to merge into 'inst', leading to code like this:
 *
 * nop                  ; nop               ; ldvary.r4
 * nop                  ; fmul  r0, r4, rf0 ; ldvary.r1
 * fadd  rf13, r0, r5   ; fmul  r2, r1, rf0 ;            <-- inst
 */
static bool
fixup_pipelined_ldvary(struct v3d_compile *c,
                       struct choose_scoreboard *scoreboard,
                       struct qblock *block,
                       struct v3d_qpu_instr *inst)
{
        const struct v3d_device_info *devinfo = c->devinfo;

        /* We only call this if we have successfully merged an ldvary into a
         * previous instruction.
         */
        assert(inst->type == V3D_QPU_INSTR_TYPE_ALU);
        assert(inst->sig.ldvary);
        uint32_t ldvary_magic = inst->sig_magic;
        uint32_t ldvary_index = inst->sig_addr;

        /* The instruction in which we merged the ldvary cannot read
         * the ldvary destination, if it does, then moving the ldvary before
         * it would overwrite it.
         */
        if (alu_reads_register(devinfo, inst, true, ldvary_magic, ldvary_index))
                return false;
        if (alu_reads_register(devinfo, inst, false, ldvary_magic, ldvary_index))
                return false;

        /* The implicit ldvary destination may not be written to by a signal
         * in the instruction following ldvary. Since we are planning to move
         * ldvary to the previous instruction, this means we need to check if
         * the current instruction has any other signal that could create this
         * conflict. The only other signal that can write to the implicit
         * ldvary destination that is compatible with ldvary in the same
         * instruction is ldunif.
         */
        if (inst->sig.ldunif)
                return false;

        /* The previous instruction can't write to the same destination as the
         * ldvary.
         */
        struct qinst *prev = (struct qinst *) block->instructions.prev;
        if (!prev || prev->qpu.type != V3D_QPU_INSTR_TYPE_ALU)
                return false;

        if (prev->qpu.alu.add.op != V3D_QPU_A_NOP) {
                if (prev->qpu.alu.add.magic_write == ldvary_magic &&
                    prev->qpu.alu.add.waddr == ldvary_index) {
                        return false;
                }
        }

        if (prev->qpu.alu.mul.op != V3D_QPU_M_NOP) {
                if (prev->qpu.alu.mul.magic_write == ldvary_magic &&
                    prev->qpu.alu.mul.waddr == ldvary_index) {
                        return false;
                }
        }

        /* The previous instruction cannot have a conflicting signal */
        if (v3d_qpu_sig_writes_address(devinfo, &prev->qpu.sig))
                return false;

        uint32_t sig;
        struct v3d_qpu_sig new_sig = prev->qpu.sig;
        new_sig.ldvary = true;
        if (!v3d_qpu_sig_pack(devinfo, &new_sig, &sig))
                return false;

        /* The previous instruction cannot use flags since ldvary uses the
         * 'cond' instruction field to store the destination.
         */
        if (v3d_qpu_writes_flags(&prev->qpu))
                return false;
        if (v3d_qpu_reads_flags(&prev->qpu))
                return false;

        /* We can't put an ldvary in the delay slots of a thrsw. We should've
         * prevented this when pairing up the ldvary with another instruction
         * and flagging it for a fixup. In V3D 7.x this is limited only to the
         * second delay slot.
         */
        assert((devinfo->ver == 42 &&
                scoreboard->last_thrsw_tick + 2 < scoreboard->tick - 1) ||
               (devinfo->ver >= 71 &&
                scoreboard->last_thrsw_tick + 2 != scoreboard->tick - 1));

        /* Move the ldvary to the previous instruction and remove it from the
         * current one.
         */
        prev->qpu.sig.ldvary = true;
        prev->qpu.sig_magic = ldvary_magic;
        prev->qpu.sig_addr = ldvary_index;
        scoreboard->last_ldvary_tick = scoreboard->tick - 1;

        inst->sig.ldvary = false;
        inst->sig_magic = false;
        inst->sig_addr = 0;

        /* Update rf0 flops tracking for new ldvary delayed rf0 write tick */
        if (devinfo->ver >= 71) {
                scoreboard->last_implicit_rf0_write_tick = scoreboard->tick;
                set_has_rf0_flops_conflict(scoreboard, inst, devinfo);
        }

        /* By moving ldvary to the previous instruction we make it update r5
         * (rf0 for ver >= 71) in the current one, so nothing else in it
         * should write this register.
         *
         * This should've been prevented by our depedency tracking, which
         * would not allow ldvary to be paired up with an instruction that
         * writes r5/rf0 (since our dependency tracking doesn't know that the
         * ldvary write to r5/rf0 happens in the next instruction).
         */
        assert(!v3d_qpu_writes_r5(devinfo, inst));
        assert(devinfo->ver == 42 ||
               (!v3d_qpu_writes_rf0_implicitly(devinfo, inst) &&
                !v3d71_qpu_writes_waddr_explicitly(devinfo, inst, 0)));

        return true;
}

static uint32_t
schedule_instructions(struct v3d_compile *c,
                      struct choose_scoreboard *scoreboard,
                      struct qblock *block,
                      enum quniform_contents *orig_uniform_contents,
                      uint32_t *orig_uniform_data,
                      uint32_t *next_uniform)
{
        const struct v3d_device_info *devinfo = c->devinfo;
        uint32_t time = 0;

        while (!list_is_empty(&scoreboard->dag->heads)) {
                struct schedule_node *chosen =
                        choose_instruction_to_schedule(c, scoreboard, NULL);
                struct schedule_node *merge = NULL;

                /* If there are no valid instructions to schedule, drop a NOP
                 * in.
                 */
                struct qinst *qinst = chosen ? chosen->inst : vir_nop();
                struct v3d_qpu_instr *inst = &qinst->qpu;

                if (debug) {
                        fprintf(stderr, "t=%4d: current list:\n",
                                time);
                        dump_state(devinfo, scoreboard->dag);
                        fprintf(stderr, "t=%4d: chose:   ", time);
                        v3d_qpu_dump(devinfo, inst);
                        fprintf(stderr, "\n");
                }

                /* We can't mark_instruction_scheduled() the chosen inst until
                 * we're done identifying instructions to merge, so put the
                 * merged instructions on a list for a moment.
                 */
                struct list_head merged_list;
                list_inithead(&merged_list);

                /* Schedule this instruction onto the QPU list. Also try to
                 * find an instruction to pair with it.
                 */
                if (chosen) {
                        time = MAX2(chosen->unblocked_time, time);
                        pre_remove_head(scoreboard->dag, chosen);

                        while ((merge =
                                choose_instruction_to_schedule(c, scoreboard,
                                                               chosen))) {
                                time = MAX2(merge->unblocked_time, time);
                                pre_remove_head(scoreboard->dag, merge);
                                list_addtail(&merge->link, &merged_list);
                                (void)qpu_merge_inst(devinfo, inst,
                                                     inst, &merge->inst->qpu);
                                if (merge->inst->uniform != -1) {
                                        chosen->inst->uniform =
                                                merge->inst->uniform;
                                }

                                chosen->inst->ldtmu_count +=
                                        merge->inst->ldtmu_count;

                                if (debug) {
                                        fprintf(stderr, "t=%4d: merging: ",
                                                time);
                                        v3d_qpu_dump(devinfo, &merge->inst->qpu);
                                        fprintf(stderr, "\n");
                                        fprintf(stderr, "         result: ");
                                        v3d_qpu_dump(devinfo, inst);
                                        fprintf(stderr, "\n");
                                }

                                if (scoreboard->fixup_ldvary) {
                                        scoreboard->fixup_ldvary = false;
                                        if (fixup_pipelined_ldvary(c, scoreboard, block, inst)) {
                                                /* Flag the ldvary as scheduled
                                                 * now so we can try to merge the
                                                 * follow-up instruction in the
                                                 * the ldvary sequence into the
                                                 * current instruction.
                                                 */
                                                mark_instruction_scheduled(
                                                        devinfo, scoreboard->dag,
                                                        time, merge);
                                        }
                                }
                        }
                        if (read_stalls(c->devinfo, scoreboard, inst))
                                c->qpu_inst_stalled_count++;
                }

                /* Update the uniform index for the rewritten location --
                 * branch target updating will still need to change
                 * c->uniform_data[] using this index.
                 */
                if (qinst->uniform != -1) {
                        if (inst->type == V3D_QPU_INSTR_TYPE_BRANCH)
                                block->branch_uniform = *next_uniform;

                        c->uniform_data[*next_uniform] =
                                orig_uniform_data[qinst->uniform];
                        c->uniform_contents[*next_uniform] =
                                orig_uniform_contents[qinst->uniform];
                        qinst->uniform = *next_uniform;
                        (*next_uniform)++;
                }

                if (debug) {
                        fprintf(stderr, "\n");
                }

                /* Now that we've scheduled a new instruction, some of its
                 * children can be promoted to the list of instructions ready to
                 * be scheduled.  Update the children's unblocked time for this
                 * DAG edge as we do so.
                 */
                mark_instruction_scheduled(devinfo, scoreboard->dag, time, chosen);
                list_for_each_entry(struct schedule_node, merge, &merged_list,
                                    link) {
                        mark_instruction_scheduled(devinfo, scoreboard->dag, time, merge);

                        /* The merged VIR instruction doesn't get re-added to the
                         * block, so free it now.
                         */
                        free(merge->inst);
                }

                if (inst->sig.thrsw) {
                        time += emit_thrsw(c, block, scoreboard, qinst, false);
                } else if (inst->type == V3D_QPU_INSTR_TYPE_BRANCH) {
                        emit_branch(c, block, scoreboard, qinst);
                } else {
                        insert_scheduled_instruction(c, block,
                                                     scoreboard, qinst);
                }
        }

        return time;
}

static uint32_t
qpu_schedule_instructions_block(struct v3d_compile *c,
                                struct choose_scoreboard *scoreboard,
                                struct qblock *block,
                                enum quniform_contents *orig_uniform_contents,
                                uint32_t *orig_uniform_data,
                                uint32_t *next_uniform)
{
        void *mem_ctx = ralloc_context(NULL);
        scoreboard->dag = dag_create(mem_ctx);
        struct list_head setup_list;

        list_inithead(&setup_list);

        /* Wrap each instruction in a scheduler structure. */
        while (!list_is_empty(&block->instructions)) {
                struct qinst *qinst = (struct qinst *)block->instructions.next;
                struct schedule_node *n =
                        rzalloc(mem_ctx, struct schedule_node);

                dag_init_node(scoreboard->dag, &n->dag);
                n->inst = qinst;

                list_del(&qinst->link);
                list_addtail(&n->link, &setup_list);
        }

        calculate_forward_deps(c, scoreboard->dag, &setup_list);
        calculate_reverse_deps(c, scoreboard->dag, &setup_list);

        dag_traverse_bottom_up(scoreboard->dag, compute_delay, c);

        uint32_t cycles = schedule_instructions(c, scoreboard, block,
                                                orig_uniform_contents,
                                                orig_uniform_data,
                                                next_uniform);

        ralloc_free(mem_ctx);
        scoreboard->dag = NULL;

        return cycles;
}

static void
qpu_set_branch_targets(struct v3d_compile *c)
{
        vir_for_each_block(block, c) {
                /* The end block of the program has no branch. */
                if (!block->successors[0])
                        continue;

                /* If there was no branch instruction, then the successor
                 * block must follow immediately after this one.
                 */
                if (block->branch_qpu_ip == ~0) {
                        assert(block->end_qpu_ip + 1 ==
                               block->successors[0]->start_qpu_ip);
                        continue;
                }

                /* Walk back through the delay slots to find the branch
                 * instr.
                 */
                struct qinst *branch = NULL;
                struct list_head *entry = block->instructions.prev;
                int32_t delay_slot_count = -1;
                struct qinst *delay_slots_start = NULL;
                for (int i = 0; i < 3; i++) {
                        entry = entry->prev;
                        struct qinst *inst =
                                container_of(entry, struct qinst, link);

                        if (delay_slot_count == -1) {
                                if (!v3d_qpu_is_nop(&inst->qpu))
                                        delay_slot_count = i;
                                else
                                        delay_slots_start = inst;
                        }

                        if (inst->qpu.type == V3D_QPU_INSTR_TYPE_BRANCH) {
                                branch = inst;
                                break;
                        }
                }
                assert(branch && branch->qpu.type == V3D_QPU_INSTR_TYPE_BRANCH);
                assert(delay_slot_count >= 0 && delay_slot_count <= 3);
                assert(delay_slot_count == 0 || delay_slots_start != NULL);

                /* Make sure that the if-we-don't-jump
                 * successor was scheduled just after the
                 * delay slots.
                 */
                assert(!block->successors[1] ||
                       block->successors[1]->start_qpu_ip ==
                       block->branch_qpu_ip + 4);

                branch->qpu.branch.offset =
                        ((block->successors[0]->start_qpu_ip -
                          (block->branch_qpu_ip + 4)) *
                         sizeof(uint64_t));

                /* Set up the relative offset to jump in the
                 * uniform stream.
                 *
                 * Use a temporary here, because
                 * uniform_data[inst->uniform] may be shared
                 * between multiple instructions.
                 */
                assert(c->uniform_contents[branch->uniform] == QUNIFORM_CONSTANT);
                c->uniform_data[branch->uniform] =
                        (block->successors[0]->start_uniform -
                         (block->branch_uniform + 1)) * 4;

                /* If this is an unconditional branch, try to fill any remaining
                 * delay slots with the initial instructions of the successor
                 * block.
                 *
                 * FIXME: we can do the same for conditional branches if we
                 * predicate the instructions to match the branch condition.
                 */
                if (branch->qpu.branch.cond == V3D_QPU_BRANCH_COND_ALWAYS) {
                        struct list_head *successor_insts =
                                &block->successors[0]->instructions;
                        delay_slot_count = MIN2(delay_slot_count,
                                                list_length(successor_insts));
                        struct qinst *s_inst =
                                (struct qinst *) successor_insts->next;
                        struct qinst *slot = delay_slots_start;
                        int slots_filled = 0;
                        while (slots_filled < delay_slot_count &&
                               qpu_inst_valid_in_branch_delay_slot(c, s_inst)) {
                                memcpy(&slot->qpu, &s_inst->qpu,
                                       sizeof(slot->qpu));
                                s_inst = (struct qinst *) s_inst->link.next;
                                slot = (struct qinst *) slot->link.next;
                                slots_filled++;
                        }
                        branch->qpu.branch.offset +=
                                slots_filled * sizeof(uint64_t);
                }
        }
}

uint32_t
v3d_qpu_schedule_instructions(struct v3d_compile *c)
{
        const struct v3d_device_info *devinfo = c->devinfo;
        struct qblock *end_block = list_last_entry(&c->blocks,
                                                   struct qblock, link);

        /* We reorder the uniforms as we schedule instructions, so save the
         * old data off and replace it.
         */
        uint32_t *uniform_data = c->uniform_data;
        enum quniform_contents *uniform_contents = c->uniform_contents;
        c->uniform_contents = ralloc_array(c, enum quniform_contents,
                                           c->num_uniforms);
        c->uniform_data = ralloc_array(c, uint32_t, c->num_uniforms);
        c->uniform_array_size = c->num_uniforms;
        uint32_t next_uniform = 0;

        struct choose_scoreboard scoreboard;
        memset(&scoreboard, 0, sizeof(scoreboard));
        scoreboard.last_ldvary_tick = -10;
        scoreboard.last_unifa_write_tick = -10;
        scoreboard.last_magic_sfu_write_tick = -10;
        scoreboard.last_uniforms_reset_tick = -10;
        scoreboard.last_thrsw_tick = -10;
        scoreboard.last_branch_tick = -10;
        scoreboard.last_setmsf_tick = -10;
        scoreboard.last_stallable_sfu_tick = -10;
        scoreboard.first_ldtmu_after_thrsw = true;
        scoreboard.last_implicit_rf0_write_tick = - 10;

        if (debug) {
                fprintf(stderr, "Pre-schedule instructions\n");
                vir_for_each_block(block, c) {
                        fprintf(stderr, "BLOCK %d\n", block->index);
                        list_for_each_entry(struct qinst, qinst,
                                            &block->instructions, link) {
                                v3d_qpu_dump(devinfo, &qinst->qpu);
                                fprintf(stderr, "\n");
                        }
                }
                fprintf(stderr, "\n");
        }

        uint32_t cycles = 0;
        vir_for_each_block(block, c) {
                block->start_qpu_ip = c->qpu_inst_count;
                block->branch_qpu_ip = ~0;
                block->start_uniform = next_uniform;

                cycles += qpu_schedule_instructions_block(c,
                                                          &scoreboard,
                                                          block,
                                                          uniform_contents,
                                                          uniform_data,
                                                          &next_uniform);

                block->end_qpu_ip = c->qpu_inst_count - 1;
        }

        /* Emit the program-end THRSW instruction. */;
        struct qinst *thrsw = vir_nop();
        thrsw->qpu.sig.thrsw = true;
        emit_thrsw(c, end_block, &scoreboard, thrsw, true);

        qpu_set_branch_targets(c);

        assert(next_uniform == c->num_uniforms);

        return cycles;
}
