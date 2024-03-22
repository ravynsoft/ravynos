/*
 * Copyright © 2016 Broadcom Limited
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

#include "vc4_qir.h"
#include "vc4_qpu.h"

static void
fail_instr(struct vc4_compile *c, struct qinst *inst, const char *msg)
{
        fprintf(stderr, "qir_validate: %s: ", msg);
        qir_dump_inst(c, inst);
        fprintf(stderr, "\n");
        abort();
}

void qir_validate(struct vc4_compile *c)
{
        bool already_assigned[c->num_temps];
        memset(&already_assigned, 0, sizeof(already_assigned));

        /* We don't want to do validation in release builds, but we want to
         * keep compiling the validation code to make sure it doesn't get
         * broken.
         */
#ifndef DEBUG
        return;
#endif

        for (int i = 0; i < c->num_temps; i++) {
                struct qinst *def = c->defs[i];

                if (def && def->cond != QPU_COND_ALWAYS)
                        fail_instr(c, def, "SSA def with condition");
        }

        qir_for_each_inst_inorder(inst, c) {
                switch (inst->dst.file) {
                case QFILE_TEMP:
                        if (inst->dst.index >= c->num_temps)
                                fail_instr(c, inst, "bad temp index");

                        if (c->defs[inst->dst.index] &&
                            already_assigned[inst->dst.index]) {
                                fail_instr(c, inst, "Re-assignment of SSA value");
                        }
                        already_assigned[inst->dst.index] = true;
                        break;

                case QFILE_NULL:
                case QFILE_VPM:
                case QFILE_TLB_COLOR_WRITE:
                case QFILE_TLB_COLOR_WRITE_MS:
                case QFILE_TLB_Z_WRITE:
                case QFILE_TLB_STENCIL_SETUP:
                        break;

                case QFILE_VARY:
                case QFILE_UNIF:
                case QFILE_FRAG_X:
                case QFILE_FRAG_Y:
                case QFILE_FRAG_REV_FLAG:
                case QFILE_QPU_ELEMENT:
                case QFILE_SMALL_IMM:
                case QFILE_LOAD_IMM:
                        fail_instr(c, inst, "Bad dest file");
                        break;

                case QFILE_TEX_S:
                case QFILE_TEX_T:
                case QFILE_TEX_R:
                case QFILE_TEX_B:
                        if (inst->src[qir_get_tex_uniform_src(inst)].file !=
                            QFILE_UNIF) {
                                fail_instr(c, inst,
                                           "tex op missing implicit uniform");
                        }
                        break;

                case QFILE_TEX_S_DIRECT:
                        if (inst->op != QOP_ADD) {
                                fail_instr(c, inst,
                                           "kernel validation requires that "
                                           "direct texture lookups use an ADD");
                        }
                        break;
                }

                for (int i = 0; i < qir_get_nsrc(inst); i++) {
                        struct qreg src = inst->src[i];

                        switch (src.file) {
                        case QFILE_TEMP:
                                if (src.index >= c->num_temps)
                                        fail_instr(c, inst, "bad temp index");
                                break;

                        case QFILE_VARY:
                        case QFILE_UNIF:
                        case QFILE_VPM:
                        case QFILE_LOAD_IMM:
                        case QFILE_QPU_ELEMENT:
                                break;

                        case QFILE_SMALL_IMM:
                                if (qpu_encode_small_immediate(src.index) == ~0)
                                        fail_instr(c, inst, "bad small immediate");
                                break;

                        case QFILE_FRAG_X:
                        case QFILE_FRAG_Y:
                        case QFILE_FRAG_REV_FLAG:
                                if (c->stage != QSTAGE_FRAG)
                                        fail_instr(c, inst, "frag access in VS/CS");
                                break;

                        case QFILE_NULL:
                        case QFILE_TLB_COLOR_WRITE:
                        case QFILE_TLB_COLOR_WRITE_MS:
                        case QFILE_TLB_Z_WRITE:
                        case QFILE_TLB_STENCIL_SETUP:
                        case QFILE_TEX_S_DIRECT:
                        case QFILE_TEX_S:
                        case QFILE_TEX_T:
                        case QFILE_TEX_R:
                        case QFILE_TEX_B:
                                fail_instr(c, inst, "Bad src file");
                                break;
                        }
                }
        }
}
