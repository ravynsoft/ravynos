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
 * @file v3d_opt_copy_propagation.c
 *
 * This implements simple copy propagation for VIR without control flow.
 *
 * For each temp, it keeps a qreg of which source it was MOVed from, if it
 * was.  If we see that used later, we can just reuse the source value, since
 * we know we don't have control flow, and we have SSA for our values so
 * there's no killing to worry about.
 */

#include "v3d_compiler.h"

static bool
is_copy_mov(const struct v3d_device_info *devinfo, struct qinst *inst)
{
        if (!inst)
                return false;

        if (inst->qpu.type != V3D_QPU_INSTR_TYPE_ALU ||
            (inst->qpu.alu.mul.op != V3D_QPU_M_FMOV &&
             inst->qpu.alu.mul.op != V3D_QPU_M_MOV)) {
                return false;
        }

        if (inst->dst.file != QFILE_TEMP)
                return false;

        if (inst->src[0].file != QFILE_TEMP)
                return false;

        if (inst->qpu.alu.add.output_pack != V3D_QPU_PACK_NONE ||
            inst->qpu.alu.mul.output_pack != V3D_QPU_PACK_NONE) {
                return false;
        }

        if (inst->qpu.flags.ac != V3D_QPU_COND_NONE ||
            inst->qpu.flags.mc != V3D_QPU_COND_NONE) {
                return false;
        }

        if (devinfo->ver == 42) {
                switch (inst->src[0].file) {
                case QFILE_MAGIC:
                        /* No copy propagating from R3/R4/R5 -- the MOVs from
                         * those are there to register allocate values produced
                         * into R3/4/5 to other regs (though hopefully r3/4/5).
                         */
                        switch (inst->src[0].index) {
                        case V3D_QPU_WADDR_R3:
                        case V3D_QPU_WADDR_R4:
                        case V3D_QPU_WADDR_R5:
                                return false;
                        default:
                                break;
                        }
                        break;

                case QFILE_REG:
                        switch (inst->src[0].index) {
                        case 0:
                        case 1:
                        case 2:
                                /* MOVs from rf0/1/2 are only to track the live
                                 * intervals for W/centroid W/Z.
                                 */
                                return false;
                        }
                        break;

                default:
                        break;
                }
        } else {
                assert(devinfo->ver >= 71);
                switch (inst->src[0].file) {
                case QFILE_REG:
                        switch (inst->src[0].index) {
                        /* MOVs from rf1/2/3 are only to track the live
                         * intervals for W/centroid W/Z.
                         *
                         * Note: rf0 can be implicitly written by ldvary
                         * (no temp involved), so it is not an SSA value and
                         * could clash with writes to other temps that are
                         * also allocated to rf0. In theory, that would mean
                         * that we can't copy propagate from it, but we handle
                         * this at register allocation time, preventing temps
                         * from being allocated to rf0 while the rf0 value from
                         * ldvary is still live.
                         */
                        case 1:
                        case 2:
                        case 3:
                                return false;
                        }
                        break;

                default:
                        break;
                }
        }

        return true;
}

static bool
vir_has_unpack(struct qinst *inst, int chan)
{
        assert(chan == 0 || chan == 1);

        if (vir_is_add(inst)) {
                if (chan == 0)
                        return inst->qpu.alu.add.a.unpack != V3D_QPU_UNPACK_NONE;
                else
                        return inst->qpu.alu.add.b.unpack != V3D_QPU_UNPACK_NONE;
        } else {
                if (chan == 0)
                        return inst->qpu.alu.mul.a.unpack != V3D_QPU_UNPACK_NONE;
                else
                        return inst->qpu.alu.mul.b.unpack != V3D_QPU_UNPACK_NONE;
        }
}

static bool
try_copy_prop(struct v3d_compile *c, struct qinst *inst, struct qinst **movs)
{
        bool debug = false;
        bool progress = false;

        for (int i = 0; i < vir_get_nsrc(inst); i++) {
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
                        if (!is_copy_mov(c->devinfo, c->defs[inst->src[i].index]))
                                continue;
                        mov = c->defs[inst->src[i].index];

                        if (mov->src[0].file == QFILE_TEMP &&
                            !c->defs[mov->src[0].index])
                                continue;
                }

                if (vir_has_unpack(mov, 0)) {
                        /* Make sure that the meaning of the unpack
                         * would be the same between the two
                         * instructions.
                         */
                        if (v3d_qpu_unpacks_f32(&inst->qpu) !=
                            v3d_qpu_unpacks_f32(&mov->qpu) ||
                            v3d_qpu_unpacks_f16(&inst->qpu) !=
                            v3d_qpu_unpacks_f16(&mov->qpu)) {
                                continue;
                        }

                        /* No composing the unpacks. */
                        if (vir_has_unpack(inst, i))
                                continue;

                        /* these ops can't represent abs. */
                        if (mov->qpu.alu.mul.a.unpack == V3D_QPU_UNPACK_ABS) {
                                switch (inst->qpu.alu.add.op) {
                                case V3D_QPU_A_VFPACK:
                                case V3D_QPU_A_FROUND:
                                case V3D_QPU_A_FTRUNC:
                                case V3D_QPU_A_FFLOOR:
                                case V3D_QPU_A_FCEIL:
                                case V3D_QPU_A_FDX:
                                case V3D_QPU_A_FDY:
                                case V3D_QPU_A_FTOIN:
                                case V3D_QPU_A_FTOIZ:
                                case V3D_QPU_A_FTOUZ:
                                case V3D_QPU_A_FTOC:
                                        continue;
                                default:
                                        break;
                                }
                        }
                }

                if (debug) {
                        fprintf(stderr, "Copy propagate: ");
                        vir_dump_inst(c, inst);
                        fprintf(stderr, "\n");
                }

                inst->src[i] = mov->src[0];
                if (vir_has_unpack(mov, 0)) {
                        enum v3d_qpu_input_unpack unpack = mov->qpu.alu.mul.a.unpack;

                        vir_set_unpack(inst, i, unpack);
                }

                if (debug) {
                        fprintf(stderr, "to: ");
                        vir_dump_inst(c, inst);
                        fprintf(stderr, "\n");
                }

                progress = true;
        }

        return progress;
}

static void
apply_kills(struct v3d_compile *c, struct qinst **movs, struct qinst *inst)
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
vir_opt_copy_propagate(struct v3d_compile *c)
{
        bool progress = false;
        struct qinst **movs;

        movs = ralloc_array(c, struct qinst *, c->num_temps);
        if (!movs)
                return false;

        vir_for_each_block(block, c) {
                /* The MOVs array tracks only available movs within the
                 * block.
                 */
                memset(movs, 0, sizeof(struct qinst *) * c->num_temps);

                c->cur_block = block;
                vir_for_each_inst(inst, block) {

                        progress = try_copy_prop(c, inst, movs) || progress;

                        apply_kills(c, movs, inst);

                        if (is_copy_mov(c->devinfo, inst))
                                movs[inst->dst.index] = inst;
                }
        }

        ralloc_free(movs);

        return progress;
}
