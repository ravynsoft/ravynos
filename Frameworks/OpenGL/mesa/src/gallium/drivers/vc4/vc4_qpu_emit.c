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

#include <inttypes.h>

#include "vc4_context.h"
#include "vc4_qir.h"
#include "vc4_qpu.h"
#include "util/ralloc.h"
#include "util/u_debug_cb.h"

static void
vc4_dump_program(struct vc4_compile *c)
{
        fprintf(stderr, "%s prog %d/%d QPU:\n",
                qir_get_stage_name(c->stage),
                c->program_id, c->variant_id);

        for (int i = 0; i < c->qpu_inst_count; i++) {
                fprintf(stderr, "0x%016"PRIx64" ", c->qpu_insts[i]);
                vc4_qpu_disasm(&c->qpu_insts[i], 1);
                fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
}

static void
queue(struct qblock *block, uint64_t inst)
{
        struct queued_qpu_inst *q = rzalloc(block, struct queued_qpu_inst);
        q->inst = inst;
        list_addtail(&q->link, &block->qpu_inst_list);
}

static uint64_t *
last_inst(struct qblock *block)
{
        struct queued_qpu_inst *q =
                (struct queued_qpu_inst *)block->qpu_inst_list.prev;
        return &q->inst;
}

static void
set_last_cond_add(struct qblock *block, uint32_t cond)
{
        *last_inst(block) = qpu_set_cond_add(*last_inst(block), cond);
}

static void
set_last_cond_mul(struct qblock *block, uint32_t cond)
{
        *last_inst(block) = qpu_set_cond_mul(*last_inst(block), cond);
}

/**
 * Some special registers can be read from either file, which lets us resolve
 * raddr conflicts without extra MOVs.
 */
static bool
swap_file(struct qpu_reg *src)
{
        switch (src->addr) {
        case QPU_R_UNIF:
        case QPU_R_VARY:
                if (src->mux == QPU_MUX_SMALL_IMM) {
                        return false;
                } else {
                        if (src->mux == QPU_MUX_A)
                                src->mux = QPU_MUX_B;
                        else
                                src->mux = QPU_MUX_A;
                        return true;
                }

        default:
                return false;
        }
}

/**
 * Sets up the VPM read FIFO before we do any VPM read.
 *
 * VPM reads (vertex attribute input) and VPM writes (varyings output) from
 * the QPU reuse the VRI (varying interpolation) block's FIFOs to talk to the
 * VPM block.  In the VS/CS (unlike in the FS), the block starts out
 * uninitialized, and you need to emit setup to the block before any VPM
 * reads/writes.
 *
 * VRI has a FIFO in each direction, with each FIFO able to hold four
 * 32-bit-per-vertex values.  VPM reads come through the read FIFO and VPM
 * writes go through the write FIFO.  The read/write setup values from QPU go
 * through the write FIFO as well, with a sideband signal indicating that
 * they're setup values.  Once a read setup reaches the other side of the
 * FIFO, the VPM block will start asynchronously reading vertex attributes and
 * filling the read FIFO -- that way hopefully the QPU doesn't have to block
 * on reads later.
 *
 * VPM read setup can configure 16 32-bit-per-vertex values to be read at a
 * time, which is 4 vec4s.  If more than that is being read (since we support
 * 8 vec4 vertex attributes), then multiple read setup writes need to be done.
 *
 * The existence of the FIFO makes it seem like you should be able to emit
 * both setups for the 5-8 attribute cases and then do all the attribute
 * reads.  However, once the setup value makes it to the other end of the
 * write FIFO, it will immediately update the VPM block's setup register.
 * That updated setup register would be used for read FIFO fills from then on,
 * breaking whatever remaining VPM values were supposed to be read into the
 * read FIFO from the previous attribute set.
 *
 * As a result, we need to emit the read setup, pull every VPM read value from
 * that setup, and only then emit the second setup if applicable.
 */
static void
setup_for_vpm_read(struct vc4_compile *c, struct qblock *block)
{
        if (c->num_inputs_in_fifo) {
                c->num_inputs_in_fifo--;
                return;
        }

        c->num_inputs_in_fifo = MIN2(c->num_inputs_remaining, 16);

        queue(block,
              qpu_load_imm_ui(qpu_vrsetup(),
                              c->vpm_read_offset |
                              0x00001a00 |
                              ((c->num_inputs_in_fifo & 0xf) << 20)));
        c->num_inputs_remaining -= c->num_inputs_in_fifo;
        c->vpm_read_offset += c->num_inputs_in_fifo;

        c->num_inputs_in_fifo--;
}

/**
 * This is used to resolve the fact that we might register-allocate two
 * different operands of an instruction to the same physical register file
 * even though instructions have only one field for the register file source
 * address.
 *
 * In that case, we need to move one to a temporary that can be used in the
 * instruction, instead.  We reserve ra14/rb14 for this purpose.
 */
static void
fixup_raddr_conflict(struct qblock *block,
                     struct qpu_reg dst,
                     struct qpu_reg *src0, struct qpu_reg *src1,
                     struct qinst *inst, uint64_t *unpack)
{
        uint32_t mux0 = src0->mux == QPU_MUX_SMALL_IMM ? QPU_MUX_B : src0->mux;
        uint32_t mux1 = src1->mux == QPU_MUX_SMALL_IMM ? QPU_MUX_B : src1->mux;

        if (mux0 <= QPU_MUX_R5 ||
            mux0 != mux1 ||
            (src0->addr == src1->addr &&
             src0->mux == src1->mux)) {
                return;
        }

        if (swap_file(src0) || swap_file(src1))
                return;

        if (mux0 == QPU_MUX_A) {
                /* Make sure we use the same type of MOV as the instruction,
                 * in case of unpacks.
                 */
                if (qir_is_float_input(inst))
                        queue(block, qpu_a_FMAX(qpu_rb(14), *src0, *src0));
                else
                        queue(block, qpu_a_MOV(qpu_rb(14), *src0));

                /* If we had an unpack on this A-file source, we need to put
                 * it into this MOV, not into the later move from regfile B.
                 */
                if (inst->src[0].pack) {
                        *last_inst(block) |= *unpack;
                        *unpack = 0;
                }
                *src0 = qpu_rb(14);
        } else {
                queue(block, qpu_a_MOV(qpu_ra(14), *src0));
                *src0 = qpu_ra(14);
        }
}

static void
set_last_dst_pack(struct qblock *block, struct qinst *inst)
{
        ASSERTED bool had_pm = *last_inst(block) & QPU_PM;
        ASSERTED bool had_ws = *last_inst(block) & QPU_WS;
        ASSERTED uint32_t unpack = QPU_GET_FIELD(*last_inst(block), QPU_UNPACK);

        if (!inst->dst.pack)
                return;

        *last_inst(block) |= QPU_SET_FIELD(inst->dst.pack, QPU_PACK);

        if (qir_is_mul(inst)) {
                assert(!unpack || had_pm);
                *last_inst(block) |= QPU_PM;
        } else {
                assert(!unpack || !had_pm);
                assert(!had_ws); /* dst must be a-file to pack. */
        }
}

static void
handle_r4_qpu_write(struct qblock *block, struct qinst *qinst,
                    struct qpu_reg dst)
{
        if (dst.mux != QPU_MUX_R4) {
                queue(block, qpu_a_MOV(dst, qpu_r4()));
                set_last_cond_add(block, qinst->cond);
        } else {
                assert(qinst->cond == QPU_COND_ALWAYS);
                if (qinst->sf)
                        queue(block, qpu_a_MOV(qpu_ra(QPU_W_NOP), qpu_r4()));
        }
}

static void
vc4_generate_code_block(struct vc4_compile *c,
                        struct qblock *block,
                        struct qpu_reg *temp_registers)
{
        int last_vpm_read_index = -1;

        qir_for_each_inst(qinst, block) {
#if 0
                fprintf(stderr, "translating qinst to qpu: ");
                qir_dump_inst(qinst);
                fprintf(stderr, "\n");
#endif

                static const struct {
                        uint32_t op;
                } translate[] = {
#define A(name) [QOP_##name] = {QPU_A_##name}
#define M(name) [QOP_##name] = {QPU_M_##name}
                        A(FADD),
                        A(FSUB),
                        A(FMIN),
                        A(FMAX),
                        A(FMINABS),
                        A(FMAXABS),
                        A(FTOI),
                        A(ITOF),
                        A(ADD),
                        A(SUB),
                        A(SHL),
                        A(SHR),
                        A(ASR),
                        A(MIN),
                        A(MAX),
                        A(AND),
                        A(OR),
                        A(XOR),
                        A(NOT),

                        M(FMUL),
                        M(V8MULD),
                        M(V8MIN),
                        M(V8MAX),
                        M(V8ADDS),
                        M(V8SUBS),
                        M(MUL24),

                        /* If we replicate src[0] out to src[1], this works
                         * out the same as a MOV.
                         */
                        [QOP_MOV] = { QPU_A_OR },
                        [QOP_FMOV] = { QPU_A_FMAX },
                        [QOP_MMOV] = { QPU_M_V8MIN },

                        [QOP_MIN_NOIMM] = { QPU_A_MIN },
                };

                uint64_t unpack = 0;
                struct qpu_reg src[ARRAY_SIZE(qinst->src)];
                for (int i = 0; i < qir_get_nsrc(qinst); i++) {
                        int index = qinst->src[i].index;
                        switch (qinst->src[i].file) {
                        case QFILE_NULL:
                        case QFILE_LOAD_IMM:
                                src[i] = qpu_rn(0);
                                break;
                        case QFILE_TEMP:
                                src[i] = temp_registers[index];
                                if (qinst->src[i].pack) {
                                        assert(!unpack ||
                                               unpack == qinst->src[i].pack);
                                        unpack = QPU_SET_FIELD(qinst->src[i].pack,
                                                               QPU_UNPACK);
                                        if (src[i].mux == QPU_MUX_R4)
                                                unpack |= QPU_PM;
                                }
                                break;
                        case QFILE_UNIF:
                                src[i] = qpu_unif();
                                break;
                        case QFILE_VARY:
                                src[i] = qpu_vary();
                                break;
                        case QFILE_SMALL_IMM:
                                src[i].mux = QPU_MUX_SMALL_IMM;
                                src[i].addr = qpu_encode_small_immediate(qinst->src[i].index);
                                /* This should only have returned a valid
                                 * small immediate field, not ~0 for failure.
                                 */
                                assert(src[i].addr <= 47);
                                break;
                        case QFILE_VPM:
                                setup_for_vpm_read(c, block);
                                assert((int)qinst->src[i].index >=
                                       last_vpm_read_index);
                                (void)last_vpm_read_index;
                                last_vpm_read_index = qinst->src[i].index;
                                src[i] = qpu_ra(QPU_R_VPM);
                                break;

                        case QFILE_FRAG_X:
                                src[i] = qpu_ra(QPU_R_XY_PIXEL_COORD);
                                break;
                        case QFILE_FRAG_Y:
                                src[i] = qpu_rb(QPU_R_XY_PIXEL_COORD);
                                break;
                        case QFILE_FRAG_REV_FLAG:
                                src[i] = qpu_rb(QPU_R_MS_REV_FLAGS);
                                break;
                        case QFILE_QPU_ELEMENT:
                                src[i] = qpu_ra(QPU_R_ELEM_QPU);
                                break;

                        case QFILE_TLB_COLOR_WRITE:
                        case QFILE_TLB_COLOR_WRITE_MS:
                        case QFILE_TLB_Z_WRITE:
                        case QFILE_TLB_STENCIL_SETUP:
                        case QFILE_TEX_S:
                        case QFILE_TEX_S_DIRECT:
                        case QFILE_TEX_T:
                        case QFILE_TEX_R:
                        case QFILE_TEX_B:
                                unreachable("bad qir src file");
                        }
                }

                struct qpu_reg dst;
                switch (qinst->dst.file) {
                case QFILE_NULL:
                        dst = qpu_ra(QPU_W_NOP);
                        break;
                case QFILE_TEMP:
                        dst = temp_registers[qinst->dst.index];
                        break;
                case QFILE_VPM:
                        dst = qpu_ra(QPU_W_VPM);
                        break;

                case QFILE_TLB_COLOR_WRITE:
                        dst = qpu_tlbc();
                        break;

                case QFILE_TLB_COLOR_WRITE_MS:
                        dst = qpu_tlbc_ms();
                        break;

                case QFILE_TLB_Z_WRITE:
                        dst = qpu_ra(QPU_W_TLB_Z);
                        break;

                case QFILE_TLB_STENCIL_SETUP:
                        dst = qpu_ra(QPU_W_TLB_STENCIL_SETUP);
                        break;

                case QFILE_TEX_S:
                case QFILE_TEX_S_DIRECT:
                        dst = qpu_rb(QPU_W_TMU0_S);
                        break;

                case QFILE_TEX_T:
                        dst = qpu_rb(QPU_W_TMU0_T);
                        break;

                case QFILE_TEX_R:
                        dst = qpu_rb(QPU_W_TMU0_R);
                        break;

                case QFILE_TEX_B:
                        dst = qpu_rb(QPU_W_TMU0_B);
                        break;

                case QFILE_VARY:
                case QFILE_UNIF:
                case QFILE_SMALL_IMM:
                case QFILE_LOAD_IMM:
                case QFILE_FRAG_X:
                case QFILE_FRAG_Y:
                case QFILE_FRAG_REV_FLAG:
                case QFILE_QPU_ELEMENT:
                        assert(!"not reached");
                        break;
                }

                ASSERTED bool handled_qinst_cond = false;

                switch (qinst->op) {
                case QOP_RCP:
                case QOP_RSQ:
                case QOP_EXP2:
                case QOP_LOG2:
                        switch (qinst->op) {
                        case QOP_RCP:
                                queue(block, qpu_a_MOV(qpu_rb(QPU_W_SFU_RECIP),
                                                       src[0]) | unpack);
                                break;
                        case QOP_RSQ:
                                queue(block, qpu_a_MOV(qpu_rb(QPU_W_SFU_RECIPSQRT),
                                                       src[0]) | unpack);
                                break;
                        case QOP_EXP2:
                                queue(block, qpu_a_MOV(qpu_rb(QPU_W_SFU_EXP),
                                                       src[0]) | unpack);
                                break;
                        case QOP_LOG2:
                                queue(block, qpu_a_MOV(qpu_rb(QPU_W_SFU_LOG),
                                                       src[0]) | unpack);
                                break;
                        default:
                                abort();
                        }

                        handle_r4_qpu_write(block, qinst, dst);
                        handled_qinst_cond = true;

                        break;

                case QOP_LOAD_IMM:
                        assert(qinst->src[0].file == QFILE_LOAD_IMM);
                        queue(block, qpu_load_imm_ui(dst, qinst->src[0].index));
                        break;

                case QOP_LOAD_IMM_U2:
                        queue(block, qpu_load_imm_u2(dst, qinst->src[0].index));
                        break;

                case QOP_LOAD_IMM_I2:
                        queue(block, qpu_load_imm_i2(dst, qinst->src[0].index));
                        break;

                case QOP_ROT_MUL:
                        /* Rotation at the hardware level occurs on the inputs
                         * to the MUL unit, and they must be accumulators in
                         * order to have the time necessary to move things.
                         */
                        assert(src[0].mux <= QPU_MUX_R3);

                        queue(block,
                              qpu_m_rot(dst, src[0], qinst->src[1].index -
                                        QPU_SMALL_IMM_MUL_ROT) | unpack);
                        set_last_cond_mul(block, qinst->cond);
                        handled_qinst_cond = true;
                        set_last_dst_pack(block, qinst);
                        break;

                case QOP_MS_MASK:
                        src[1] = qpu_ra(QPU_R_MS_REV_FLAGS);
                        fixup_raddr_conflict(block, dst, &src[0], &src[1],
                                             qinst, &unpack);
                        queue(block, qpu_a_AND(qpu_ra(QPU_W_MS_FLAGS),
                                               src[0], src[1]) | unpack);
                        break;

                case QOP_FRAG_Z:
                case QOP_FRAG_W:
                        /* QOP_FRAG_Z/W don't emit instructions, just allocate
                         * the register to the Z/W payload.
                         */
                        break;

                case QOP_TLB_COLOR_READ:
                        queue(block, qpu_NOP());
                        *last_inst(block) = qpu_set_sig(*last_inst(block),
                                                        QPU_SIG_COLOR_LOAD);
                        handle_r4_qpu_write(block, qinst, dst);
                        handled_qinst_cond = true;
                        break;

                case QOP_VARY_ADD_C:
                        queue(block, qpu_a_FADD(dst, src[0], qpu_r5()) | unpack);
                        break;


                case QOP_TEX_RESULT:
                        queue(block, qpu_NOP());
                        *last_inst(block) = qpu_set_sig(*last_inst(block),
                                                        QPU_SIG_LOAD_TMU0);
                        handle_r4_qpu_write(block, qinst, dst);
                        handled_qinst_cond = true;
                        break;

                case QOP_THRSW:
                        queue(block, qpu_NOP());
                        *last_inst(block) = qpu_set_sig(*last_inst(block),
                                                        QPU_SIG_THREAD_SWITCH);
                        c->last_thrsw = last_inst(block);
                        break;

                case QOP_BRANCH:
                        /* The branch target will be updated at QPU scheduling
                         * time.
                         */
                        queue(block, (qpu_branch(qinst->cond, 0) |
                                      QPU_BRANCH_REL));
                        handled_qinst_cond = true;
                        break;

                case QOP_UNIFORMS_RESET:
                        fixup_raddr_conflict(block, dst, &src[0], &src[1],
                                             qinst, &unpack);

                        queue(block, qpu_a_ADD(qpu_ra(QPU_W_UNIFORMS_ADDRESS),
                                               src[0], src[1]));
                        break;

                default:
                        assert(qinst->op < ARRAY_SIZE(translate));
                        assert(translate[qinst->op].op != 0); /* NOPs */

                        /* Skip emitting the MOV if it's a no-op. */
                        if (qir_is_raw_mov(qinst) &&
                            dst.mux == src[0].mux && dst.addr == src[0].addr) {
                                break;
                        }

                        /* If we have only one source, put it in the second
                         * argument slot as well so that we don't take up
                         * another raddr just to get unused data.
                         */
                        if (qir_get_non_sideband_nsrc(qinst) == 1)
                                src[1] = src[0];

                        fixup_raddr_conflict(block, dst, &src[0], &src[1],
                                             qinst, &unpack);

                        if (qir_is_mul(qinst)) {
                                queue(block, qpu_m_alu2(translate[qinst->op].op,
                                                        dst,
                                                        src[0], src[1]) | unpack);
                                set_last_cond_mul(block, qinst->cond);
                        } else {
                                queue(block, qpu_a_alu2(translate[qinst->op].op,
                                                        dst,
                                                        src[0], src[1]) | unpack);
                                set_last_cond_add(block, qinst->cond);
                        }
                        handled_qinst_cond = true;
                        set_last_dst_pack(block, qinst);

                        break;
                }

                assert(qinst->cond == QPU_COND_ALWAYS ||
                       handled_qinst_cond);

                if (qinst->sf)
                        *last_inst(block) |= QPU_SF;
        }
}

void
vc4_generate_code(struct vc4_context *vc4, struct vc4_compile *c)
{
        struct qblock *start_block = list_first_entry(&c->blocks,
                                                      struct qblock, link);

        struct qpu_reg *temp_registers = vc4_register_allocate(vc4, c);
        if (!temp_registers)
                return;

        switch (c->stage) {
        case QSTAGE_VERT:
        case QSTAGE_COORD:
                c->num_inputs_remaining = c->num_inputs;
                queue(start_block, qpu_load_imm_ui(qpu_vwsetup(), 0x00001a00));
                break;
        case QSTAGE_FRAG:
                break;
        }

        qir_for_each_block(block, c)
                vc4_generate_code_block(c, block, temp_registers);

        /* Switch the last SIG_THRSW instruction to SIG_LAST_THRSW.
         *
         * LAST_THRSW is a new signal in BCM2708B0 (including Raspberry Pi)
         * that ensures that a later thread doesn't try to lock the scoreboard
         * and terminate before an earlier-spawned thread on the same QPU, by
         * delaying switching back to the later shader until earlier has
         * finished.  Otherwise, if the earlier thread was hitting the same
         * quad, the scoreboard would deadlock.
         */
        if (c->last_thrsw) {
                assert(QPU_GET_FIELD(*c->last_thrsw, QPU_SIG) ==
                       QPU_SIG_THREAD_SWITCH);
                *c->last_thrsw = ((*c->last_thrsw & ~QPU_SIG_MASK) |
                                  QPU_SET_FIELD(QPU_SIG_LAST_THREAD_SWITCH,
                                                QPU_SIG));
        }

        uint32_t cycles = qpu_schedule_instructions(c);
        uint32_t inst_count_at_schedule_time = c->qpu_inst_count;

        /* thread end can't have VPM write or read */
        if (QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_WADDR_ADD) == QPU_W_VPM ||
            QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_WADDR_MUL) == QPU_W_VPM ||
            QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_RADDR_A) == QPU_R_VPM ||
            QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_RADDR_B) == QPU_R_VPM) {
                qpu_serialize_one_inst(c, qpu_NOP());
        }

        /* thread end can't have uniform read */
        if (QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_RADDR_A) == QPU_R_UNIF ||
            QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_RADDR_B) == QPU_R_UNIF) {
                qpu_serialize_one_inst(c, qpu_NOP());
        }

        /* thread end can't have TLB operations */
        if (qpu_inst_is_tlb(c->qpu_insts[c->qpu_inst_count - 1]))
                qpu_serialize_one_inst(c, qpu_NOP());

        /* Make sure there's no existing signal set (like for a small
         * immediate)
         */
        if (QPU_GET_FIELD(c->qpu_insts[c->qpu_inst_count - 1],
                          QPU_SIG) != QPU_SIG_NONE) {
                qpu_serialize_one_inst(c, qpu_NOP());
        }

        c->qpu_insts[c->qpu_inst_count - 1] =
                qpu_set_sig(c->qpu_insts[c->qpu_inst_count - 1],
                            QPU_SIG_PROG_END);
        qpu_serialize_one_inst(c, qpu_NOP());
        qpu_serialize_one_inst(c, qpu_NOP());

        switch (c->stage) {
        case QSTAGE_VERT:
        case QSTAGE_COORD:
                break;
        case QSTAGE_FRAG:
                c->qpu_insts[c->qpu_inst_count - 1] =
                        qpu_set_sig(c->qpu_insts[c->qpu_inst_count - 1],
                                    QPU_SIG_SCOREBOARD_UNLOCK);
                break;
        }

        cycles += c->qpu_inst_count - inst_count_at_schedule_time;

        if (VC4_DBG(SHADERDB)) {
                util_debug_message(&vc4->base.debug, SHADER_INFO,
                                   "%s shader: %d inst, %d threads, %d uniforms, %d max-temps, %d estimated-cycles",
                                   qir_get_stage_name(c->stage),
                                   c->qpu_inst_count,
                                   1 + c->fs_threaded,
                                   c->num_uniforms,
                                   c->max_reg_pressure,
                                   cycles);
        }

        if (VC4_DBG(QPU))
                vc4_dump_program(c);

        vc4_qpu_validate(c->qpu_insts, c->qpu_inst_count);

        free(temp_registers);
}
