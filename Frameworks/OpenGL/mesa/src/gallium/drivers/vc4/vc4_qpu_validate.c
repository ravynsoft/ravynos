
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

#include <stdlib.h>

#include "vc4_qpu.h"

static void
fail_instr(uint64_t inst, const char *msg)
{
        fprintf(stderr, "vc4_qpu_validate: %s: ", msg);
        vc4_qpu_disasm(&inst, 1);
        fprintf(stderr, "\n");
        abort();
}

static bool
writes_reg(uint64_t inst, uint32_t w)
{
        return (QPU_GET_FIELD(inst, QPU_WADDR_ADD) == w ||
                QPU_GET_FIELD(inst, QPU_WADDR_MUL) == w);
}

static bool
_reads_reg(uint64_t inst, uint32_t r, bool ignore_a, bool ignore_b)
{
        struct {
                uint32_t mux, addr;
        } src_regs[] = {
                { QPU_GET_FIELD(inst, QPU_ADD_A) },
                { QPU_GET_FIELD(inst, QPU_ADD_B) },
                { QPU_GET_FIELD(inst, QPU_MUL_A) },
                { QPU_GET_FIELD(inst, QPU_MUL_B) },
        };

        /* Branches only reference raddr_a (no mux), and we don't use that
         * feature of branching.
         */
        if (QPU_GET_FIELD(inst, QPU_SIG) == QPU_SIG_BRANCH)
                return false;

        /* Load immediates don't read any registers. */
        if (QPU_GET_FIELD(inst, QPU_SIG) == QPU_SIG_LOAD_IMM)
                return false;

        for (int i = 0; i < ARRAY_SIZE(src_regs); i++) {
                if (!ignore_a &&
                    src_regs[i].mux == QPU_MUX_A &&
                    (QPU_GET_FIELD(inst, QPU_RADDR_A) == r))
                        return true;

                if (!ignore_b &&
                    QPU_GET_FIELD(inst, QPU_SIG) != QPU_SIG_SMALL_IMM &&
                    src_regs[i].mux == QPU_MUX_B &&
                    (QPU_GET_FIELD(inst, QPU_RADDR_B) == r))
                        return true;
        }

        return false;
}

static bool
reads_reg(uint64_t inst, uint32_t r)
{
        return _reads_reg(inst, r, false, false);
}

static bool
reads_a_reg(uint64_t inst, uint32_t r)
{
        return _reads_reg(inst, r, false, true);
}

static bool
reads_b_reg(uint64_t inst, uint32_t r)
{
        return _reads_reg(inst, r, true, false);
}

static bool
writes_sfu(uint64_t inst)
{
        return (writes_reg(inst, QPU_W_SFU_RECIP) ||
                writes_reg(inst, QPU_W_SFU_RECIPSQRT) ||
                writes_reg(inst, QPU_W_SFU_EXP) ||
                writes_reg(inst, QPU_W_SFU_LOG));
}

/**
 * Checks for the instruction restrictions from page 37 ("Summary of
 * Instruction Restrictions").
 */
void
vc4_qpu_validate(uint64_t *insts, uint32_t num_inst)
{
        bool scoreboard_locked = false;
        bool threaded = false;

        /* We don't want to do validation in release builds, but we want to
         * keep compiling the validation code to make sure it doesn't get
         * broken.
         */
#ifndef DEBUG
        return;
#endif

        for (int i = 0; i < num_inst; i++) {
                uint64_t inst = insts[i];
                uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);

                if (sig != QPU_SIG_PROG_END) {
                        if (qpu_inst_is_tlb(inst))
                                scoreboard_locked = true;

                        if (sig == QPU_SIG_THREAD_SWITCH ||
                            sig == QPU_SIG_LAST_THREAD_SWITCH) {
                                threaded = true;
                        }

                        continue;
                }

                /* "The Thread End instruction must not write to either physical
                 *  regfile A or B."
                 */
                if (QPU_GET_FIELD(inst, QPU_WADDR_ADD) < 32 ||
                    QPU_GET_FIELD(inst, QPU_WADDR_MUL) < 32) {
                        fail_instr(inst, "write to phys reg in thread end");
                }

                /* Can't trigger an implicit wait on scoreboard in the program
                 * end instruction.
                 */
                if (qpu_inst_is_tlb(inst) && !scoreboard_locked)
                        fail_instr(inst, "implicit sb wait in program end");

                /* Two delay slots will be executed. */
                assert(i + 2 <= num_inst);

                 for (int j = i; j < i + 2; j++) {
                         /* "The last three instructions of any program
                          *  (Thread End plus the following two delay-slot
                          *  instructions) must not do varyings read, uniforms
                          *  read or any kind of VPM, VDR, or VDW read or
                          *  write."
                          */
                         if (writes_reg(insts[j], QPU_W_VPM) ||
                             reads_reg(insts[j], QPU_R_VARY) ||
                             reads_reg(insts[j], QPU_R_UNIF) ||
                             reads_reg(insts[j], QPU_R_VPM)) {
                                 fail_instr(insts[j], "last 3 instructions "
                                            "using fixed functions");
                         }

                         /* "The Thread End instruction and the following two
                          *  delay slot instructions must not write or read
                          *  address 14 in either regfile A or B."
                          */
                         if (writes_reg(insts[j], 14) ||
                             reads_reg(insts[j], 14)) {
                                 fail_instr(insts[j], "last 3 instructions "
                                            "must not use r14");
                         }
                 }

                 /* "The final program instruction (the second delay slot
                  *  instruction) must not do a TLB Z write."
                  */
                 if (writes_reg(insts[i + 2], QPU_W_TLB_Z)) {
                         fail_instr(insts[i + 2], "final instruction doing "
                                    "Z write");
                 }
        }

        /* "A scoreboard wait must not occur in the first two instructions of
         *  a fragment shader. This is either the explicit Wait for Scoreboard
         *  signal or an implicit wait with the first tile-buffer read or
         *  write instruction."
         */
        for (int i = 0; i < 2; i++) {
                uint64_t inst = insts[i];

                if (qpu_inst_is_tlb(inst))
                        fail_instr(inst, "sb wait in first two insts");
        }

        /* "If TMU_NOSWAP is written, the write must be three instructions
         *  before the first TMU write instruction.  For example, if
         *  TMU_NOSWAP is written in the first shader instruction, the first
         *  TMU write cannot occur before the 4th shader instruction."
         */
        int last_tmu_noswap = -10;
        for (int i = 0; i < num_inst; i++) {
                uint64_t inst = insts[i];

                if ((i - last_tmu_noswap) <= 3 &&
                    (writes_reg(inst, QPU_W_TMU0_S) ||
                     writes_reg(inst, QPU_W_TMU1_S))) {
                        fail_instr(inst, "TMU write too soon after TMU_NOSWAP");
                }

                if (writes_reg(inst, QPU_W_TMU_NOSWAP))
                    last_tmu_noswap = i;
        }

        /* "An instruction must not read from a location in physical regfile A
         *  or B that was written to by the previous instruction."
         */
        for (int i = 0; i < num_inst - 1; i++) {
                uint64_t inst = insts[i];
                uint32_t add_waddr = QPU_GET_FIELD(inst, QPU_WADDR_ADD);
                uint32_t mul_waddr = QPU_GET_FIELD(inst, QPU_WADDR_MUL);
                uint32_t waddr_a, waddr_b;

                if (inst & QPU_WS) {
                        waddr_b = add_waddr;
                        waddr_a = mul_waddr;
                } else {
                        waddr_a = add_waddr;
                        waddr_b = mul_waddr;
                }

                if ((waddr_a < 32 && reads_a_reg(insts[i + 1], waddr_a)) ||
                    (waddr_b < 32 && reads_b_reg(insts[i + 1], waddr_b))) {
                        fail_instr(insts[i + 1],
                                   "Reads physical reg too soon after write");
                }
        }

        /* "After an SFU lookup instruction, accumulator r4 must not be read
         *  in the following two instructions. Any other instruction that
         *  results in r4 being written (that is, TMU read, TLB read, SFU
         *  lookup) cannot occur in the two instructions following an SFU
         *  lookup."
         */
        int last_sfu_inst = -10;
        for (int i = 0; i < num_inst - 1; i++) {
                uint64_t inst = insts[i];
                uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);

                if (i - last_sfu_inst <= 2 &&
                    (writes_sfu(inst) ||
                     sig == QPU_SIG_LOAD_TMU0 ||
                     sig == QPU_SIG_LOAD_TMU1 ||
                     sig == QPU_SIG_COLOR_LOAD)) {
                        fail_instr(inst, "R4 write too soon after SFU write");
                }

                if (writes_sfu(inst))
                        last_sfu_inst = i;
        }

        for (int i = 0; i < num_inst - 1; i++) {
                uint64_t inst = insts[i];

                if (QPU_GET_FIELD(inst, QPU_SIG) == QPU_SIG_SMALL_IMM &&
                    QPU_GET_FIELD(inst, QPU_SMALL_IMM) >=
                    QPU_SMALL_IMM_MUL_ROT) {
                        uint32_t mux_a = QPU_GET_FIELD(inst, QPU_MUL_A);
                        uint32_t mux_b = QPU_GET_FIELD(inst, QPU_MUL_B);

                        /* "The full horizontal vector rotate is only
                         *  available when both of the mul ALU input arguments
                         *  are taken from accumulators r0-r3."
                         */
                        if (mux_a > QPU_MUX_R3 || mux_b > QPU_MUX_R3) {
                                fail_instr(inst,
                                           "MUL rotate using non-accumulator "
                                           "input");
                        }

                        if (QPU_GET_FIELD(inst, QPU_SMALL_IMM) ==
                            QPU_SMALL_IMM_MUL_ROT) {
                                /* "An instruction that does a vector rotate
                                 *  by r5 must not immediately follow an
                                 *  instruction that writes to r5."
                                 */
                                if (writes_reg(insts[i - 1], QPU_W_ACC5)) {
                                        fail_instr(inst,
                                                   "vector rotate by r5 "
                                                   "immediately after r5 write");
                                }
                        }

                        /* "An instruction that does a vector rotate must not
                         *  immediately follow an instruction that writes to the
                         *  accumulator that is being rotated."
                         */
                        if (writes_reg(insts[i - 1], QPU_W_ACC0 + mux_a) ||
                            writes_reg(insts[i - 1], QPU_W_ACC0 + mux_b)) {
                                fail_instr(inst,
                                           "vector rotate of value "
                                           "written in previous instruction");
                        }
                }
        }

        /* "An instruction that does a vector rotate must not immediately
         *  follow an instruction that writes to the accumulator that is being
         *  rotated.
         *
         * XXX: TODO.
         */

        /* "After an instruction that does a TLB Z write, the multisample mask
         *  must not be read as an instruction input argument in the following
         *  two instruction. The TLB Z write instruction can, however, be
         *  followed immediately by a TLB color write."
         */
        for (int i = 0; i < num_inst - 1; i++) {
                uint64_t inst = insts[i];
                if (writes_reg(inst, QPU_W_TLB_Z) &&
                    (reads_a_reg(insts[i + 1], QPU_R_MS_REV_FLAGS) ||
                     reads_a_reg(insts[i + 2], QPU_R_MS_REV_FLAGS))) {
                        fail_instr(inst, "TLB Z write followed by MS mask read");
                }
        }

        /*
         * "A single instruction can only perform a maximum of one of the
         *  following closely coupled peripheral accesses in a single
         *  instruction: TMU write, TMU read, TLB write, TLB read, TLB
         *  combined color read and write, SFU write, Mutex read or Semaphore
         *  access."
         */
        for (int i = 0; i < num_inst - 1; i++) {
                uint64_t inst = insts[i];

                if (qpu_num_sf_accesses(inst) > 1)
                        fail_instr(inst, "Single instruction writes SFU twice");
        }

        /* "The uniform base pointer can be written (from SIMD element 0) by
         *  the processor to reset the stream, there must be at least two
         *  nonuniform-accessing instructions following a pointer change
         *  before uniforms can be accessed once more."
         */
        int last_unif_pointer_update = -3;
        for (int i = 0; i < num_inst; i++) {
                uint64_t inst = insts[i];
                uint32_t waddr_add = QPU_GET_FIELD(inst, QPU_WADDR_ADD);
                uint32_t waddr_mul = QPU_GET_FIELD(inst, QPU_WADDR_MUL);

                if (reads_reg(inst, QPU_R_UNIF) &&
                    i - last_unif_pointer_update <= 2) {
                        fail_instr(inst,
                                   "uniform read too soon after pointer update");
                }

                if (waddr_add == QPU_W_UNIFORMS_ADDRESS ||
                    waddr_mul == QPU_W_UNIFORMS_ADDRESS)
                        last_unif_pointer_update = i;
        }

        if (threaded) {
                bool last_thrsw_found = false;
                bool scoreboard_locked = false;
                int tex_samples_outstanding = 0;
                int last_tex_samples_outstanding = 0;
                int thrsw_ip = -1;

                for (int i = 0; i < num_inst; i++) {
                        uint64_t inst = insts[i];
                        uint32_t sig = QPU_GET_FIELD(inst, QPU_SIG);

                        if (i == thrsw_ip) {
                                /* In order to get texture results back in the
                                 * correct order, before a new thrsw we have
                                 * to read all the texture results from before
                                 * the previous thrsw.
                                 *
                                 * FIXME: Is collecting the remaining results
                                 * during the delay slots OK, or should we do
                                 * this at THRSW signal time?
                                 */
                                if (last_tex_samples_outstanding != 0) {
                                        fail_instr(inst, "THRSW with texture "
                                                   "results from the previous "
                                                   "THRSW still in the FIFO.");
                                }

                                last_tex_samples_outstanding =
                                        tex_samples_outstanding;
                                tex_samples_outstanding = 0;
                        }

                        if (qpu_inst_is_tlb(inst))
                                scoreboard_locked = true;

                        switch (sig) {
                        case QPU_SIG_THREAD_SWITCH:
                        case QPU_SIG_LAST_THREAD_SWITCH:
                                /* No thread switching with the scoreboard
                                 * locked.  Doing so means we may deadlock
                                 * when the other thread tries to lock
                                 * scoreboard.
                                 */
                                if (scoreboard_locked) {
                                        fail_instr(inst, "THRSW with the "
                                                   "scoreboard locked.");
                                }

                                /* No thread switching after lthrsw, since
                                 * lthrsw means that we get delayed until the
                                 * other shader is ready for us to terminate.
                                 */
                                if (last_thrsw_found) {
                                        fail_instr(inst, "THRSW after a "
                                                   "previous LTHRSW");
                                }

                                if (sig == QPU_SIG_LAST_THREAD_SWITCH)
                                        last_thrsw_found = true;

                                /* No THRSW while we already have a THRSW
                                 * queued.
                                 */
                                if (i < thrsw_ip) {
                                        fail_instr(inst,
                                                   "THRSW with a THRSW queued.");
                                }

                                thrsw_ip = i + 3;
                                break;

                        case QPU_SIG_LOAD_TMU0:
                        case QPU_SIG_LOAD_TMU1:
                                if (last_tex_samples_outstanding == 0) {
                                        fail_instr(inst, "TMU load with nothing "
                                                   "in the results fifo from "
                                                   "the previous THRSW.");
                                }

                                last_tex_samples_outstanding--;
                                break;
                        }

                        uint32_t waddr_add = QPU_GET_FIELD(inst, QPU_WADDR_ADD);
                        uint32_t waddr_mul = QPU_GET_FIELD(inst, QPU_WADDR_MUL);
                        if (waddr_add == QPU_W_TMU0_S ||
                            waddr_add == QPU_W_TMU1_S ||
                            waddr_mul == QPU_W_TMU0_S ||
                            waddr_mul == QPU_W_TMU1_S) {
                                tex_samples_outstanding++;
                        }
                }
        }
}
