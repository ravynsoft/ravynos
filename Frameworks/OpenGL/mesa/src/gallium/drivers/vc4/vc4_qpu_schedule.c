/*
 * Copyright © 2010 Intel Corporation
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
 * @file vc4_qpu_schedule.c
 *
 * The basic model of the list scheduler is to take a basic block, compute a
 * DAG of the dependencies, and make a list of the DAG heads.  Heuristically
 * pick a DAG head, then put all the children that are now DAG heads into the
 * list of things to schedule.
 *
 * The goal of scheduling here is to pack pairs of operations together in a
 * single QPU instruction.
 */

#include "vc4_qir.h"
#include "vc4_qpu.h"
#include "util/ralloc.h"
#include "util/dag.h"

static bool debug;

struct schedule_node_child;

struct schedule_node {
        struct dag_node dag;
        struct list_head link;
        struct queued_qpu_inst *inst;

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

        /**
         * Which uniform from uniform_data[] this instruction read, or -1 if
         * not reading a uniform.
         */
        int uniform;
};

/* When walking the instructions in reverse, we need to swap before/after in
 * add_dep().
 */
enum direction { F, R };

struct schedule_state {
        struct dag *dag;
        struct schedule_node *last_r[6];
        struct schedule_node *last_ra[32];
        struct schedule_node *last_rb[32];
        struct schedule_node *last_sf;
        struct schedule_node *last_vpm_read;
        struct schedule_node *last_tmu_write;
        struct schedule_node *last_tlb;
        struct schedule_node *last_vpm;
        struct schedule_node *last_uniforms_reset;
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
qpu_writes_r4(uint64_t inst)
{
        uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);

        switch(sig) {
        case QPU_SIG_COLOR_LOAD:
        case QPU_SIG_LOAD_TMU0:
        case QPU_SIG_LOAD_TMU1:
        case QPU_SIG_ALPHA_MASK_LOAD:
                return true;
        default:
                return false;
        }
}

static void
process_raddr_deps(struct schedule_state *state, struct schedule_node *n,
                   uint32_t raddr, bool is_a)
{
        switch (raddr) {
        case QPU_R_VARY:
                add_write_dep(state, &state->last_r[5], n);
                break;

        case QPU_R_VPM:
                add_write_dep(state, &state->last_vpm_read, n);
                break;

        case QPU_R_UNIF:
                add_read_dep(state, state->last_uniforms_reset, n);
                break;

        case QPU_R_NOP:
        case QPU_R_ELEM_QPU:
        case QPU_R_XY_PIXEL_COORD:
        case QPU_R_MS_REV_FLAGS:
                break;

        default:
                if (raddr < 32) {
                        if (is_a)
                                add_read_dep(state, state->last_ra[raddr], n);
                        else
                                add_read_dep(state, state->last_rb[raddr], n);
                } else {
                        fprintf(stderr, "unknown raddr %d\n", raddr);
                        abort();
                }
                break;
        }
}

static bool
is_tmu_write(uint32_t waddr)
{
        switch (waddr) {
        case QPU_W_TMU0_S:
        case QPU_W_TMU0_T:
        case QPU_W_TMU0_R:
        case QPU_W_TMU0_B:
        case QPU_W_TMU1_S:
        case QPU_W_TMU1_T:
        case QPU_W_TMU1_R:
        case QPU_W_TMU1_B:
                return true;
        default:
                return false;
        }
}

static bool
reads_uniform(uint64_t inst)
{
        if (QPU_GET_FIELD(inst, QPU_SIG) == QPU_SIG_LOAD_IMM)
                return false;

        return (QPU_GET_FIELD(inst, QPU_RADDR_A) == QPU_R_UNIF ||
                (QPU_GET_FIELD(inst, QPU_RADDR_B) == QPU_R_UNIF &&
                 QPU_GET_FIELD(inst, QPU_SIG) != QPU_SIG_SMALL_IMM) ||
                is_tmu_write(QPU_GET_FIELD(inst, QPU_WADDR_ADD)) ||
                is_tmu_write(QPU_GET_FIELD(inst, QPU_WADDR_MUL)));
}

static void
process_mux_deps(struct schedule_state *state, struct schedule_node *n,
                 uint32_t mux)
{
        if (mux != QPU_MUX_A && mux != QPU_MUX_B)
                add_read_dep(state, state->last_r[mux], n);
}


static void
process_waddr_deps(struct schedule_state *state, struct schedule_node *n,
                   uint32_t waddr, bool is_add)
{
        uint64_t inst = n->inst->inst;
        bool is_a = is_add ^ ((inst & QPU_WS) != 0);

        if (waddr < 32) {
                if (is_a) {
                        add_write_dep(state, &state->last_ra[waddr], n);
                } else {
                        add_write_dep(state, &state->last_rb[waddr], n);
                }
        } else if (is_tmu_write(waddr)) {
                add_write_dep(state, &state->last_tmu_write, n);
                add_read_dep(state, state->last_uniforms_reset, n);
        } else if (qpu_waddr_is_tlb(waddr) ||
                   waddr == QPU_W_MS_FLAGS) {
                add_write_dep(state, &state->last_tlb, n);
        } else {
                switch (waddr) {
                case QPU_W_ACC0:
                case QPU_W_ACC1:
                case QPU_W_ACC2:
                case QPU_W_ACC3:
                case QPU_W_ACC5:
                        add_write_dep(state, &state->last_r[waddr - QPU_W_ACC0],
                                      n);
                        break;

                case QPU_W_VPM:
                        add_write_dep(state, &state->last_vpm, n);
                        break;

                case QPU_W_VPMVCD_SETUP:
                        if (is_a)
                                add_write_dep(state, &state->last_vpm_read, n);
                        else
                                add_write_dep(state, &state->last_vpm, n);
                        break;

                case QPU_W_SFU_RECIP:
                case QPU_W_SFU_RECIPSQRT:
                case QPU_W_SFU_EXP:
                case QPU_W_SFU_LOG:
                        add_write_dep(state, &state->last_r[4], n);
                        break;

                case QPU_W_TLB_STENCIL_SETUP:
                        /* This isn't a TLB operation that does things like
                         * implicitly lock the scoreboard, but it does have to
                         * appear before TLB_Z, and each of the TLB_STENCILs
                         * have to schedule in the same order relative to each
                         * other.
                         */
                        add_write_dep(state, &state->last_tlb, n);
                        break;

                case QPU_W_MS_FLAGS:
                        add_write_dep(state, &state->last_tlb, n);
                        break;

                case QPU_W_UNIFORMS_ADDRESS:
                        add_write_dep(state, &state->last_uniforms_reset, n);
                        break;

                case QPU_W_NOP:
                        break;

                default:
                        fprintf(stderr, "Unknown waddr %d\n", waddr);
                        abort();
                }
        }
}

static void
process_cond_deps(struct schedule_state *state, struct schedule_node *n,
                  uint32_t cond)
{
        switch (cond) {
        case QPU_COND_NEVER:
        case QPU_COND_ALWAYS:
                break;
        default:
                add_read_dep(state, state->last_sf, n);
                break;
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
        uint64_t inst = n->inst->inst;
        uint32_t add_op = QPU_GET_FIELD(inst, QPU_OP_ADD);
        uint32_t mul_op = QPU_GET_FIELD(inst, QPU_OP_MUL);
        uint32_t waddr_add = QPU_GET_FIELD(inst, QPU_WADDR_ADD);
        uint32_t waddr_mul = QPU_GET_FIELD(inst, QPU_WADDR_MUL);
        uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);
        uint32_t raddr_a = sig == QPU_SIG_BRANCH ?
                                QPU_GET_FIELD(inst, QPU_BRANCH_RADDR_A) :
                                QPU_GET_FIELD(inst, QPU_RADDR_A);
        uint32_t raddr_b = QPU_GET_FIELD(inst, QPU_RADDR_B);
        uint32_t add_a = QPU_GET_FIELD(inst, QPU_ADD_A);
        uint32_t add_b = QPU_GET_FIELD(inst, QPU_ADD_B);
        uint32_t mul_a = QPU_GET_FIELD(inst, QPU_MUL_A);
        uint32_t mul_b = QPU_GET_FIELD(inst, QPU_MUL_B);

        if (sig != QPU_SIG_LOAD_IMM) {
                process_raddr_deps(state, n, raddr_a, true);
                if (sig != QPU_SIG_SMALL_IMM &&
                    sig != QPU_SIG_BRANCH)
                        process_raddr_deps(state, n, raddr_b, false);
        }

        if (sig != QPU_SIG_LOAD_IMM && sig != QPU_SIG_BRANCH) {
                if (add_op != QPU_A_NOP) {
                        process_mux_deps(state, n, add_a);
                        process_mux_deps(state, n, add_b);
                }
                if (mul_op != QPU_M_NOP) {
                        process_mux_deps(state, n, mul_a);
                        process_mux_deps(state, n, mul_b);
                }
        }

        process_waddr_deps(state, n, waddr_add, true);
        process_waddr_deps(state, n, waddr_mul, false);
        if (qpu_writes_r4(inst))
                add_write_dep(state, &state->last_r[4], n);

        switch (sig) {
        case QPU_SIG_SW_BREAKPOINT:
        case QPU_SIG_NONE:
        case QPU_SIG_SMALL_IMM:
        case QPU_SIG_LOAD_IMM:
                break;

        case QPU_SIG_THREAD_SWITCH:
        case QPU_SIG_LAST_THREAD_SWITCH:
                /* All accumulator contents and flags are undefined after the
                 * switch.
                 */
                for (int i = 0; i < ARRAY_SIZE(state->last_r); i++)
                        add_write_dep(state, &state->last_r[i], n);
                add_write_dep(state, &state->last_sf, n);

                /* Scoreboard-locking operations have to stay after the last
                 * thread switch.
                 */
                add_write_dep(state, &state->last_tlb, n);

                add_write_dep(state, &state->last_tmu_write, n);
                break;

        case QPU_SIG_LOAD_TMU0:
        case QPU_SIG_LOAD_TMU1:
                /* TMU loads are coming from a FIFO, so ordering is important.
                 */
                add_write_dep(state, &state->last_tmu_write, n);
                break;

        case QPU_SIG_COLOR_LOAD:
                add_read_dep(state, state->last_tlb, n);
                break;

        case QPU_SIG_BRANCH:
                add_read_dep(state, state->last_sf, n);
                break;

        case QPU_SIG_PROG_END:
        case QPU_SIG_WAIT_FOR_SCOREBOARD:
        case QPU_SIG_SCOREBOARD_UNLOCK:
        case QPU_SIG_COVERAGE_LOAD:
        case QPU_SIG_COLOR_LOAD_END:
        case QPU_SIG_ALPHA_MASK_LOAD:
                fprintf(stderr, "Unhandled signal bits %d\n", sig);
                abort();
        }

        process_cond_deps(state, n, QPU_GET_FIELD(inst, QPU_COND_ADD));
        process_cond_deps(state, n, QPU_GET_FIELD(inst, QPU_COND_MUL));
        if ((inst & QPU_SF) && sig != QPU_SIG_BRANCH)
                add_write_dep(state, &state->last_sf, n);
}

static void
calculate_forward_deps(struct vc4_compile *c, struct dag *dag,
                       struct list_head *schedule_list)
{
        struct schedule_state state;

        memset(&state, 0, sizeof(state));
        state.dag = dag;
        state.dir = F;

        list_for_each_entry(struct schedule_node, node, schedule_list, link)
                calculate_deps(&state, node);
}

static void
calculate_reverse_deps(struct vc4_compile *c, struct dag *dag,
                       struct list_head *schedule_list)
{
        struct schedule_state state;

        memset(&state, 0, sizeof(state));
        state.dag = dag;
        state.dir = R;

        list_for_each_entry_rev(struct schedule_node, node, schedule_list,
                                link) {
                calculate_deps(&state, (struct schedule_node *)node);
        }
}

struct choose_scoreboard {
        struct dag *dag;
        int tick;
        int last_sfu_write_tick;
        int last_uniforms_reset_tick;
        uint32_t last_waddr_a, last_waddr_b;
        bool tlb_locked;
};

static bool
reads_too_soon_after_write(struct choose_scoreboard *scoreboard, uint64_t inst)
{
        uint32_t raddr_a = QPU_GET_FIELD(inst, QPU_RADDR_A);
        uint32_t raddr_b = QPU_GET_FIELD(inst, QPU_RADDR_B);
        uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);

        /* Full immediate loads don't read any registers. */
        if (sig == QPU_SIG_LOAD_IMM)
                return false;

        uint32_t src_muxes[] = {
                QPU_GET_FIELD(inst, QPU_ADD_A),
                QPU_GET_FIELD(inst, QPU_ADD_B),
                QPU_GET_FIELD(inst, QPU_MUL_A),
                QPU_GET_FIELD(inst, QPU_MUL_B),
        };
        for (int i = 0; i < ARRAY_SIZE(src_muxes); i++) {
                if ((src_muxes[i] == QPU_MUX_A &&
                     raddr_a < 32 &&
                     scoreboard->last_waddr_a == raddr_a) ||
                    (src_muxes[i] == QPU_MUX_B &&
                     sig != QPU_SIG_SMALL_IMM &&
                     raddr_b < 32 &&
                     scoreboard->last_waddr_b == raddr_b)) {
                        return true;
                }

                if (src_muxes[i] == QPU_MUX_R4) {
                        if (scoreboard->tick -
                            scoreboard->last_sfu_write_tick <= 2) {
                                return true;
                        }
                }
        }

        if (sig == QPU_SIG_SMALL_IMM &&
            QPU_GET_FIELD(inst, QPU_SMALL_IMM) >= QPU_SMALL_IMM_MUL_ROT) {
                uint32_t mux_a = QPU_GET_FIELD(inst, QPU_MUL_A);
                uint32_t mux_b = QPU_GET_FIELD(inst, QPU_MUL_B);

                if (scoreboard->last_waddr_a == mux_a + QPU_W_ACC0 ||
                    scoreboard->last_waddr_a == mux_b + QPU_W_ACC0 ||
                    scoreboard->last_waddr_b == mux_a + QPU_W_ACC0 ||
                    scoreboard->last_waddr_b == mux_b + QPU_W_ACC0) {
                        return true;
                }
        }

        if (reads_uniform(inst) &&
            scoreboard->tick - scoreboard->last_uniforms_reset_tick <= 2) {
                return true;
        }

        return false;
}

static bool
pixel_scoreboard_too_soon(struct choose_scoreboard *scoreboard, uint64_t inst)
{
        return (scoreboard->tick < 2 && qpu_inst_is_tlb(inst));
}

static int
get_instruction_priority(uint64_t inst)
{
        uint32_t waddr_add = QPU_GET_FIELD(inst, QPU_WADDR_ADD);
        uint32_t waddr_mul = QPU_GET_FIELD(inst, QPU_WADDR_MUL);
        uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);
        uint32_t baseline_score;
        uint32_t next_score = 0;

        /* Schedule TLB operations as late as possible, to get more
         * parallelism between shaders.
         */
        if (qpu_inst_is_tlb(inst))
                return next_score;
        next_score++;

        /* Schedule texture read results collection late to hide latency. */
        if (sig == QPU_SIG_LOAD_TMU0 || sig == QPU_SIG_LOAD_TMU1)
                return next_score;
        next_score++;

        /* Default score for things that aren't otherwise special. */
        baseline_score = next_score;
        next_score++;

        /* Schedule texture read setup early to hide their latency better. */
        if (is_tmu_write(waddr_add) || is_tmu_write(waddr_mul))
                return next_score;
        next_score++;

        return baseline_score;
}

static struct schedule_node *
choose_instruction_to_schedule(struct choose_scoreboard *scoreboard,
                               struct list_head *schedule_list,
                               struct schedule_node *prev_inst)
{
        struct schedule_node *chosen = NULL;
        int chosen_prio = 0;

        /* Don't pair up anything with a thread switch signal -- emit_thrsw()
         * will handle pairing it along with filling the delay slots.
         */
        if (prev_inst) {
                uint32_t prev_sig = QPU_GET_FIELD(prev_inst->inst->inst,
                                                  QPU_SIG);
                if (prev_sig == QPU_SIG_THREAD_SWITCH ||
                    prev_sig == QPU_SIG_LAST_THREAD_SWITCH) {
                        return NULL;
                }
        }

        list_for_each_entry(struct schedule_node, n, &scoreboard->dag->heads,
                            dag.link) {
                uint64_t inst = n->inst->inst;
                uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);

                /* Don't choose the branch instruction until it's the last one
                 * left.  XXX: We could potentially choose it before it's the
                 * last one, if the remaining instructions fit in the delay
                 * slots.
                 */
                if (sig == QPU_SIG_BRANCH &&
                    !list_is_singular(&scoreboard->dag->heads)) {
                        continue;
                }

                /* "An instruction must not read from a location in physical
                 *  regfile A or B that was written to by the previous
                 *  instruction."
                 */
                if (reads_too_soon_after_write(scoreboard, inst))
                        continue;

                /* "A scoreboard wait must not occur in the first two
                 *  instructions of a fragment shader. This is either the
                 *  explicit Wait for Scoreboard signal or an implicit wait
                 *  with the first tile-buffer read or write instruction."
                 */
                if (pixel_scoreboard_too_soon(scoreboard, inst))
                        continue;

                /* If we're trying to pair with another instruction, check
                 * that they're compatible.
                 */
                if (prev_inst) {
                        /* Don't pair up a thread switch signal -- we'll
                         * handle pairing it when we pick it on its own.
                         */
                        if (sig == QPU_SIG_THREAD_SWITCH ||
                            sig == QPU_SIG_LAST_THREAD_SWITCH) {
                                continue;
                        }

                        if (prev_inst->uniform != -1 && n->uniform != -1)
                                continue;

                        /* Don't merge in something that will lock the TLB.
                         * Hopefully what we have in inst will release some
                         * other instructions, allowing us to delay the
                         * TLB-locking instruction until later.
                         */
                        if (!scoreboard->tlb_locked && qpu_inst_is_tlb(inst))
                                continue;

                        inst = qpu_merge_inst(prev_inst->inst->inst, inst);
                        if (!inst)
                                continue;
                }

                int prio = get_instruction_priority(inst);

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

        return chosen;
}

static void
update_scoreboard_for_chosen(struct choose_scoreboard *scoreboard,
                             uint64_t inst)
{
        uint32_t waddr_add = QPU_GET_FIELD(inst, QPU_WADDR_ADD);
        uint32_t waddr_mul = QPU_GET_FIELD(inst, QPU_WADDR_MUL);

        if (!(inst & QPU_WS)) {
                scoreboard->last_waddr_a = waddr_add;
                scoreboard->last_waddr_b = waddr_mul;
        } else {
                scoreboard->last_waddr_b = waddr_add;
                scoreboard->last_waddr_a = waddr_mul;
        }

        if ((waddr_add >= QPU_W_SFU_RECIP && waddr_add <= QPU_W_SFU_LOG) ||
            (waddr_mul >= QPU_W_SFU_RECIP && waddr_mul <= QPU_W_SFU_LOG)) {
                scoreboard->last_sfu_write_tick = scoreboard->tick;
        }

        if (waddr_add == QPU_W_UNIFORMS_ADDRESS ||
            waddr_mul == QPU_W_UNIFORMS_ADDRESS) {
                scoreboard->last_uniforms_reset_tick = scoreboard->tick;
        }

        if (qpu_inst_is_tlb(inst))
                scoreboard->tlb_locked = true;
}

static void
dump_state(struct dag *dag)
{
        list_for_each_entry(struct schedule_node, n, &dag->heads, dag.link) {
                fprintf(stderr, "         t=%4d: ", n->unblocked_time);
                vc4_qpu_disasm(&n->inst->inst, 1);
                fprintf(stderr, "\n");

                util_dynarray_foreach(&n->dag.edges, struct dag_edge, edge) {
                        struct schedule_node *child =
                                (struct schedule_node *)edge->child;
                        if (!child)
                                continue;

                        fprintf(stderr, "                 - ");
                        vc4_qpu_disasm(&child->inst->inst, 1);
                        fprintf(stderr, " (%d parents, %c)\n",
                                child->dag.parent_count,
                                edge->data ? 'w' : 'r');
                }
        }
}

static uint32_t waddr_latency(uint32_t waddr, uint64_t after)
{
        if (waddr < 32)
                return 2;

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
        if (waddr == QPU_W_TMU0_S) {
                if (QPU_GET_FIELD(after, QPU_SIG) == QPU_SIG_LOAD_TMU0)
                        return 100;
        }
        if (waddr == QPU_W_TMU1_S) {
                if (QPU_GET_FIELD(after, QPU_SIG) == QPU_SIG_LOAD_TMU1)
                        return 100;
        }

        switch(waddr) {
        case QPU_W_SFU_RECIP:
        case QPU_W_SFU_RECIPSQRT:
        case QPU_W_SFU_EXP:
        case QPU_W_SFU_LOG:
                return 3;
        default:
                return 1;
        }
}

static uint32_t
instruction_latency(struct schedule_node *before, struct schedule_node *after)
{
        uint64_t before_inst = before->inst->inst;
        uint64_t after_inst = after->inst->inst;

        return MAX2(waddr_latency(QPU_GET_FIELD(before_inst, QPU_WADDR_ADD),
                                  after_inst),
                    waddr_latency(QPU_GET_FIELD(before_inst, QPU_WADDR_MUL),
                                  after_inst));
}

/** Recursive computation of the delay member of a node. */
static void
compute_delay(struct dag_node *node, void *state)
{
        struct schedule_node *n = (struct schedule_node *)node;

        n->delay = 1;

        util_dynarray_foreach(&n->dag.edges, struct dag_edge, edge) {
                struct schedule_node *child =
                        (struct schedule_node *)edge->child;
                n->delay = MAX2(n->delay, (child->delay +
                                           instruction_latency(n, child)));
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
mark_instruction_scheduled(struct dag *dag,
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

                uint32_t latency = instruction_latency(node, child);

                child->unblocked_time = MAX2(child->unblocked_time,
                                             time + latency);
        }
        dag_prune_head(dag, &node->dag);
}

/**
 * Emits a THRSW/LTHRSW signal in the stream, trying to move it up to pair
 * with another instruction.
 */
static void
emit_thrsw(struct vc4_compile *c,
           struct choose_scoreboard *scoreboard,
           uint64_t inst)
{
        uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);

        /* There should be nothing in a thrsw inst being scheduled other than
         * the signal bits.
         */
        assert(QPU_GET_FIELD(inst, QPU_OP_ADD) == QPU_A_NOP);
        assert(QPU_GET_FIELD(inst, QPU_OP_MUL) == QPU_M_NOP);

        /* Try to find an earlier scheduled instruction that we can merge the
         * thrsw into.
         */
        int thrsw_ip = c->qpu_inst_count;
        for (int i = 1; i <= MIN2(c->qpu_inst_count, 3); i++) {
                uint64_t prev_instr = c->qpu_insts[c->qpu_inst_count - i];
                uint32_t prev_sig = QPU_GET_FIELD(prev_instr, QPU_SIG);

                if (prev_sig == QPU_SIG_NONE)
                        thrsw_ip = c->qpu_inst_count - i;
        }

        if (thrsw_ip != c->qpu_inst_count) {
                /* Merge the thrsw into the existing instruction. */
                c->qpu_insts[thrsw_ip] =
                        QPU_UPDATE_FIELD(c->qpu_insts[thrsw_ip], sig, QPU_SIG);
        } else {
                qpu_serialize_one_inst(c, inst);
                update_scoreboard_for_chosen(scoreboard, inst);
        }

        /* Fill the delay slots. */
        while (c->qpu_inst_count < thrsw_ip + 3) {
                update_scoreboard_for_chosen(scoreboard, qpu_NOP());
                qpu_serialize_one_inst(c, qpu_NOP());
        }
}

static uint32_t
schedule_instructions(struct vc4_compile *c,
                      struct choose_scoreboard *scoreboard,
                      struct qblock *block,
                      struct list_head *schedule_list,
                      enum quniform_contents *orig_uniform_contents,
                      uint32_t *orig_uniform_data,
                      uint32_t *next_uniform)
{
        uint32_t time = 0;

        while (!list_is_empty(&scoreboard->dag->heads)) {
                struct schedule_node *chosen =
                        choose_instruction_to_schedule(scoreboard,
                                                       schedule_list,
                                                       NULL);
                struct schedule_node *merge = NULL;

                /* If there are no valid instructions to schedule, drop a NOP
                 * in.
                 */
                uint64_t inst = chosen ? chosen->inst->inst : qpu_NOP();

                if (debug) {
                        fprintf(stderr, "t=%4d: current list:\n",
                                time);
                        dump_state(scoreboard->dag);
                        fprintf(stderr, "t=%4d: chose: ", time);
                        vc4_qpu_disasm(&inst, 1);
                        fprintf(stderr, "\n");
                }

                /* Schedule this instruction onto the QPU list. Also try to
                 * find an instruction to pair with it.
                 */
                if (chosen) {
                        time = MAX2(chosen->unblocked_time, time);
                        pre_remove_head(scoreboard->dag, chosen);
                        if (chosen->uniform != -1) {
                                c->uniform_data[*next_uniform] =
                                        orig_uniform_data[chosen->uniform];
                                c->uniform_contents[*next_uniform] =
                                        orig_uniform_contents[chosen->uniform];
                                (*next_uniform)++;
                        }

                        merge = choose_instruction_to_schedule(scoreboard,
                                                               schedule_list,
                                                               chosen);
                        if (merge) {
                                time = MAX2(merge->unblocked_time, time);
                                inst = qpu_merge_inst(inst, merge->inst->inst);
                                assert(inst != 0);
                                if (merge->uniform != -1) {
                                        c->uniform_data[*next_uniform] =
                                                orig_uniform_data[merge->uniform];
                                        c->uniform_contents[*next_uniform] =
                                                orig_uniform_contents[merge->uniform];
                                        (*next_uniform)++;
                                }

                                if (debug) {
                                        fprintf(stderr, "t=%4d: merging: ",
                                                time);
                                        vc4_qpu_disasm(&merge->inst->inst, 1);
                                        fprintf(stderr, "\n");
                                        fprintf(stderr, "            resulting in: ");
                                        vc4_qpu_disasm(&inst, 1);
                                        fprintf(stderr, "\n");
                                }
                        }
                }

                if (debug) {
                        fprintf(stderr, "\n");
                }

                /* Now that we've scheduled a new instruction, some of its
                 * children can be promoted to the list of instructions ready to
                 * be scheduled.  Update the children's unblocked time for this
                 * DAG edge as we do so.
                 */
                mark_instruction_scheduled(scoreboard->dag, time, chosen);
                mark_instruction_scheduled(scoreboard->dag, time, merge);

                if (QPU_GET_FIELD(inst, QPU_SIG) == QPU_SIG_THREAD_SWITCH ||
                    QPU_GET_FIELD(inst, QPU_SIG) == QPU_SIG_LAST_THREAD_SWITCH) {
                        emit_thrsw(c, scoreboard, inst);
                } else {
                        qpu_serialize_one_inst(c, inst);
                        update_scoreboard_for_chosen(scoreboard, inst);
                }

                scoreboard->tick++;
                time++;

                if (QPU_GET_FIELD(inst, QPU_SIG) == QPU_SIG_BRANCH) {
                        block->branch_qpu_ip = c->qpu_inst_count - 1;
                        /* Fill the delay slots.
                         *
                         * We should fill these with actual instructions,
                         * instead, but that will probably need to be done
                         * after this, once we know what the leading
                         * instructions of the successors are (so we can
                         * handle A/B register file write latency)
                        */
                        inst = qpu_NOP();
                        update_scoreboard_for_chosen(scoreboard, inst);
                        qpu_serialize_one_inst(c, inst);
                        qpu_serialize_one_inst(c, inst);
                        qpu_serialize_one_inst(c, inst);
                }
        }

        return time;
}

static uint32_t
qpu_schedule_instructions_block(struct vc4_compile *c,
                                struct choose_scoreboard *scoreboard,
                                struct qblock *block,
                                enum quniform_contents *orig_uniform_contents,
                                uint32_t *orig_uniform_data,
                                uint32_t *next_uniform)
{
        scoreboard->dag = dag_create(NULL);
        struct list_head setup_list;

        list_inithead(&setup_list);

        /* Wrap each instruction in a scheduler structure. */
        uint32_t next_sched_uniform = *next_uniform;
        while (!list_is_empty(&block->qpu_inst_list)) {
                struct queued_qpu_inst *inst =
                        (struct queued_qpu_inst *)block->qpu_inst_list.next;
                struct schedule_node *n = rzalloc(scoreboard->dag,
                                                  struct schedule_node);

                dag_init_node(scoreboard->dag, &n->dag);
                n->inst = inst;

                if (reads_uniform(inst->inst)) {
                        n->uniform = next_sched_uniform++;
                } else {
                        n->uniform = -1;
                }
                list_del(&inst->link);
                list_addtail(&n->link, &setup_list);
        }

        calculate_forward_deps(c, scoreboard->dag, &setup_list);
        calculate_reverse_deps(c, scoreboard->dag, &setup_list);

        dag_traverse_bottom_up(scoreboard->dag, compute_delay, NULL);

        uint32_t cycles = schedule_instructions(c, scoreboard, block,
                                                &setup_list,
                                                orig_uniform_contents,
                                                orig_uniform_data,
                                                next_uniform);

        ralloc_free(scoreboard->dag);
        scoreboard->dag = NULL;

        return cycles;
}

static void
qpu_set_branch_targets(struct vc4_compile *c)
{
        qir_for_each_block(block, c) {
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

                /* Set the branch target for the block that doesn't follow
                 * immediately after ours.
                 */
                uint64_t *branch_inst = &c->qpu_insts[block->branch_qpu_ip];
                assert(QPU_GET_FIELD(*branch_inst, QPU_SIG) == QPU_SIG_BRANCH);
                assert(QPU_GET_FIELD(*branch_inst, QPU_BRANCH_TARGET) == 0);

                uint32_t branch_target =
                        (block->successors[0]->start_qpu_ip -
                         (block->branch_qpu_ip + 4)) * sizeof(uint64_t);
                *branch_inst = (*branch_inst |
                                QPU_SET_FIELD(branch_target, QPU_BRANCH_TARGET));

                /* Make sure that the if-we-don't-jump successor was scheduled
                 * just after the delay slots.
                 */
                if (block->successors[1]) {
                        assert(block->successors[1]->start_qpu_ip ==
                               block->branch_qpu_ip + 4);
                }
        }
}

uint32_t
qpu_schedule_instructions(struct vc4_compile *c)
{
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
        scoreboard.last_waddr_a = ~0;
        scoreboard.last_waddr_b = ~0;
        scoreboard.last_sfu_write_tick = -10;
        scoreboard.last_uniforms_reset_tick = -10;

        if (debug) {
                fprintf(stderr, "Pre-schedule instructions\n");
                qir_for_each_block(block, c) {
                        fprintf(stderr, "BLOCK %d\n", block->index);
                        list_for_each_entry(struct queued_qpu_inst, q,
                                            &block->qpu_inst_list, link) {
                                vc4_qpu_disasm(&q->inst, 1);
                                fprintf(stderr, "\n");
                        }
                }
                fprintf(stderr, "\n");
        }

        uint32_t cycles = 0;
        qir_for_each_block(block, c) {
                block->start_qpu_ip = c->qpu_inst_count;
                block->branch_qpu_ip = ~0;

                cycles += qpu_schedule_instructions_block(c,
                                                          &scoreboard,
                                                          block,
                                                          uniform_contents,
                                                          uniform_data,
                                                          &next_uniform);

                block->end_qpu_ip = c->qpu_inst_count - 1;
        }

        qpu_set_branch_targets(c);

        assert(next_uniform == c->num_uniforms);

        if (debug) {
                fprintf(stderr, "Post-schedule instructions\n");
                vc4_qpu_disasm(c->qpu_insts, c->qpu_inst_count);
                fprintf(stderr, "\n");
        }

        return cycles;
}
