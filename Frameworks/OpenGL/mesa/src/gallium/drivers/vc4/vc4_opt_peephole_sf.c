/*
 * Copyright © 2016 Broadcom
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
 * @file vc4_opt_peephole_sf.c
 *
 * Quick optimization to eliminate unused or identical SF updates.
 */

#include "vc4_qir.h"
#include "util/u_math.h"

static bool debug;

static void
dump_from(struct vc4_compile *c, struct qinst *inst, const char *type)
{
        if (!debug)
                return;

        fprintf(stderr, "optimizing %s: ", type);
        qir_dump_inst(c, inst);
        fprintf(stderr, "\n");
}

static void
dump_to(struct vc4_compile *c, struct qinst *inst)
{
        if (!debug)
                return;

        fprintf(stderr, "to: ");
        qir_dump_inst(c, inst);
        fprintf(stderr, "\n");
}

static bool
inst_srcs_updated(struct qinst *inst, struct qinst *writer)
{
        /* If the sources get overwritten, stop tracking the
         * last instruction writing SF.
         */
        switch (writer->dst.file) {
        case QFILE_TEMP:
                for (int i = 0; i < qir_get_nsrc(inst); i++) {
                        if (inst->src[i].file == QFILE_TEMP &&
                            inst->src[i].index == writer->dst.index) {
                                return true;
                        }
                }
                return false;
        default:
                return false;
        }
}

static bool
src_file_varies_on_reread(struct qreg reg)
{
        switch (reg.file) {
        case QFILE_VARY:
        case QFILE_VPM:
                return true;
        default:
                return false;
        }
}

static bool
inst_result_equals(struct qinst *a, struct qinst *b)
{
        if (a->op != b->op ||
            qir_depends_on_flags(a) ||
            qir_depends_on_flags(b)) {
                return false;
        }

        for (int i = 0; i < qir_get_nsrc(a); i++) {
                if (!qir_reg_equals(a->src[i], b->src[i]) ||
                    src_file_varies_on_reread(a->src[i]) ||
                    src_file_varies_on_reread(b->src[i])) {
                        return false;
                }
        }

        return true;
}

static bool
qir_opt_peephole_sf_block(struct vc4_compile *c, struct qblock *block)
{
        bool progress = false;
        /* We don't have liveness dataflow analysis for flags, but we also
         * never generate a use of flags across control flow, so just treat
         * them as unused at block exit.
         */
        bool sf_live = false;
        struct qinst *last_sf = NULL;

        /* Walk the block from bottom to top, tracking if the SF is used, and
         * removing unused or repeated ones.
         */
        qir_for_each_inst_rev(inst, block) {
                if (inst->sf) {
                        if (!sf_live) {
                                /* Our instruction's SF isn't read, so drop it.
                                 */
                                dump_from(c, inst, "dead SF");
                                inst->sf = false;
                                dump_to(c, inst);
                                progress = true;
                        } else if (last_sf &&
                                   inst_result_equals(last_sf, inst)) {
                                /* The last_sf sets up same value as inst, so
                                 * just drop the later one.
                                 */
                                dump_from(c, last_sf, "repeated SF");
                                last_sf->sf = false;
                                dump_to(c, last_sf);
                                progress = true;
                                last_sf = inst;
                        } else {
                                last_sf = inst;
                        }
                        sf_live = false;
                }

                if (last_sf) {
                        if (inst_srcs_updated(last_sf, inst))
                                last_sf = NULL;
                }

                if (qir_depends_on_flags(inst))
                        sf_live = true;
        }

        return progress;
}

bool
qir_opt_peephole_sf(struct vc4_compile *c)
{
        bool progress = false;

        qir_for_each_block(block, c)
                progress = qir_opt_peephole_sf_block(c, block) || progress;

        return progress;
}
