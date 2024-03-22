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

#include <stdio.h>
#include <string.h>
#include "util/macros.h"
#include "util/ralloc.h"
#include "broadcom/common/v3d_device_info.h"
#include "broadcom/qpu/qpu_disasm.h"
#include "broadcom/qpu/qpu_instr.h"

static const struct {
        int ver;
        uint64_t inst;
        const char *expected;
} tests[] = {
        { 33, 0x3d003186bb800000ull, "nop                           ; nop                         ; ldvary" },
        { 33, 0x3c20318105829000ull, "fadd r1, r1, r5               ; nop                         ; thrsw" },
        { 33, 0x3c403186bb81d000ull, "vpmsetup -, r5                ; nop                         ; ldunif" },
        { 33, 0x3f003186bb800000ull, "nop                           ; nop                         ; ldvpm" },
        { 33, 0x3c002380b6edb000ull, "or rf0, r3, r3                ; mov vpm, r3" },
        { 33, 0x57403006bbb80000ull, "nop                           ; fmul r0, rf0, r5            ; ldvpm; ldunif" },
        { 33, 0x9c094adef634b000ull, "ffloor.ifb rf30.l, r3         ; fmul.pushz rf43.l, r5, r1.h" },
        { 33, 0xb0044c56ba326840ull, "flpop rf22, rf33              ; fmul.pushz rf49.l, r4.h, r1.abs" },

        /* vfmul input packing */
        { 33, 0x101e8b6e8aad4000ull, "fmax.nornn rf46, r4.l, r2.l   ; vfmul.ifnb rf45, r3, r5" },
        { 33, 0x1857d3c219825000ull, "faddnf.norc r2.l, r5.l, r4    ; vfmul.ifb rf15, r0.ll, r4   ; ldunif" },
        { 33, 0x1c0a0dfde2294000ull, "fcmp.ifna rf61.h, r4.abs, r2.l; vfmul rf55, r2.hh, r1" },
        { 33, 0x2011c89b402cc000ull, "fsub.norz rf27, r4.abs, r1.abs; vfmul.ifa rf34, r3.swp, r1" },

        { 33, 0xe01b42ab3bb063c0ull, "vfpack.andnc rf43, rf15.l, r0.h; fmul.ifna rf10.h, r4.l, r5.abs" },
        { 33, 0x600b8b87fb4d1000ull, "fdx.ifnb rf7.h, r1.l          ; fmul.pushn rf46, r3.l, r2.abs" },

        /* small immediates */
        { 33, 0x5de24398bbdc6218ull, "vflb.andnn rf24               ; fmul rf14, -8, rf8.h" },
        { 33, 0x25ef83d8b166f00full, "vfmin.pushn rf24, 15.ff, r5   ; smul24.ifnb rf15, r1, r3" },
        { 33, 0xadedcdf70839f990ull, "faddnf.pushc rf55, -16.l, r3.abs; fmul.ifb rf55.l, rf38.l, r1.h" },
        { 33, 0x7dff89fa6a01f020ull, "fsub.nornc rf58.h, 0x3b800000.l, r3.l; fmul.ifnb rf39, r0.h, r0.h" },

        /* branch conditions */
        { 33, 0x02000006002034c0ull, "b.anyap  rf19" },
        { 33, 0x02679356b4201000ull, "b.anyap  -1268280496" },
        { 33, 0x02b76a2dd0400000ull, "b.anynaq  zero_addr+0xd0b76a28" },
        { 33, 0x0200000500402000ull, "b.anynaq  lri" },
        { 33, 0x0216fe167301c8c0ull, "bu.anya  zero_addr+0x7316fe10, rf35" },
        { 33, 0x020000050040e000ull, "bu.anynaq  lri, r:unif" },
        { 33, 0x0200000300006000ull, "bu.na0  lri, a:unif" },

        /* Special waddr names */
        { 33, 0x3c00318735808000ull, "vfpack tlb, r0, r1            ; nop" },
        { 33, 0xe0571c938e8d5000ull, "fmax.andc recip, r5.h, r2.l   ; fmul.ifb rf50.h, r3.l, r4.abs; ldunif" },
        { 33, 0xc04098d4382c9000ull, "add.pushn rsqrt, r1, r1       ; fmul rf35.h, r3.abs, r1.abs ; ldunif" },
        { 33, 0x481edcd6b3184500ull, "vfmin.norn log, r4.hh, r0     ; fmul.ifnb rf51, rf20.abs, r0.l" },
        { 33, 0x041618d57c453000ull, "shl.andn exp, r3, r2          ; add.ifb rf35, r1, r2" },
        { 33, 0x7048e5da49272800ull, "fsub.ifa rf26, r2.l, rf32     ; fmul.pushc sin, r1.h, r1.abs; ldunif" },

        /* v4.1 signals */
        { 41, 0x1f010520cf60a000ull, "fcmp.andz rf32, r2.h, r1.h    ; vfmul rf20, r0.hh, r3       ; ldunifa" },
        { 41, 0x932045e6c16ea000ull, "fcmp rf38, r2.abs, r5         ; fmul rf23.l, r3, r3.abs     ; ldunifarf.rf1" },
        { 41, 0xd72f0434e43ae5c0ull, "fcmp rf52.h, rf23, r5.abs     ; fmul rf16.h, rf23, r1       ; ldunifarf.rf60" },
        { 41, 0xdb3048eb9d533780ull, "fmax rf43.l, r3.h, rf30       ; fmul rf35.h, r4, r2.l       ; ldunifarf.r1" },
        { 41, 0x733620471e6ce700ull, "faddnf rf7.l, rf28.h, r1.l    ; fmul r1, r3.h, r3.abs       ; ldunifarf.rsqrt2" },
        { 41, 0x9c094adef634b000ull, "ffloor.ifb rf30.l, r3         ; fmul.pushz rf43.l, r5, r1.h" },

        /* v4.1 opcodes */
        { 41, 0x3de020c7bdfd200dull, "ldvpmg_in rf7, r2, r2         ; mov r3, 13" },
        { 41, 0x3de02040f8ff7201ull, "stvpmv 1, rf8                 ; mov r1, 1" },
        { 41, 0xd8000e50bb2d3000ull, "sampid rf16                   ; fmul rf57.h, r3, r1.l" },

        /* v4.1 SFU instructions. */
        { 41, 0xe98d60c1ba2aef80ull, "recip rf1, rf62               ; fmul r3.h, r2.l, r1.l       ; ldunifrf.rf53" },
        { 41, 0x7d87c2debc51c000ull, "rsqrt rf30, r4                ; fmul rf11, r4.h, r2.h       ; ldunifrf.rf31" },
        { 41, 0xb182475abc2bb000ull, "rsqrt2 rf26, r3               ; fmul rf29.l, r2.h, r1.abs   ; ldunifrf.rf9" },
        { 41, 0x79880808bc0b6900ull, "sin rf8, rf36                 ; fmul rf32, r2.h, r0.l       ; ldunifrf.rf32" },
        { 41, 0x04092094bc5a28c0ull, "exp.ifb rf20, r2              ; add r2, rf35, r2" },
        { 41, 0xe00648bfbc32a000ull, "log rf63, r2                  ; fmul.andnn rf34.h, r4.l, r1.abs" },

        /* v4.2 changes */
        { 42, 0x3c203192bb814000ull, "barrierid syncb               ; nop                         ; thrsw" },
};

static void
swap_mux(enum v3d_qpu_mux *a, enum v3d_qpu_mux *b)
{
        enum v3d_qpu_mux t = *a;
        *a = *b;
        *b = t;
}

static void
swap_pack(enum v3d_qpu_input_unpack *a, enum v3d_qpu_input_unpack *b)
{
        enum v3d_qpu_input_unpack t = *a;
        *a = *b;
        *b = t;
}

int
main(int argc, char **argv)
{
        struct v3d_device_info devinfo = { };
        int retval = 0;

        for (int i = 0; i < ARRAY_SIZE(tests); i++) {
                devinfo.ver = tests[i].ver;

                printf("Testing v%d.%d 0x%016llx (\"%s\")... ",
                       devinfo.ver / 10, devinfo.ver % 10,
                       (long long)tests[i].inst,
                        tests[i].expected);

                const char *disasm_output = v3d_qpu_disasm(&devinfo,
                                                           tests[i].inst);

                printf("%s\n", disasm_output);

                if (strcmp(disasm_output, tests[i].expected) != 0) {
                        printf("FAIL\n");
                        printf("  Expected: \"%s\"\n", tests[i].expected);
                        printf("  Got:      \"%s\"\n", disasm_output);
                        retval = 1;
                        continue;
                }
                ralloc_free((void *)disasm_output);

                struct v3d_qpu_instr instr;
                if (!v3d_qpu_instr_unpack(&devinfo, tests[i].inst, &instr)) {
                        printf("FAIL (unpack) %s\n", tests[i].expected);
                        retval = 1;
                        continue;
                }

                if (instr.type == V3D_QPU_INSTR_TYPE_ALU) {
                        switch (instr.alu.add.op) {
                        case V3D_QPU_A_FADD:
                        case V3D_QPU_A_FADDNF:
                        case V3D_QPU_A_FMIN:
                        case V3D_QPU_A_FMAX:
                                /* Swap the operands to be sure that we test
                                 * how the QPUs distinguish between these ops.
                                 */
                                swap_mux(&instr.alu.add.a.mux,
                                         &instr.alu.add.b.mux);
                                swap_pack(&instr.alu.add.a.unpack,
                                          &instr.alu.add.b.unpack);
                                break;
                        default:
                                break;
                        }
                }

                uint64_t repack;
                if (!v3d_qpu_instr_pack(&devinfo, &instr, &repack)) {
                        printf("FAIL (pack) %s\n", tests[i].expected);
                        retval = 1;
                        continue;
                }

                if (repack != tests[i].inst) {
                        printf("FAIL (repack) 0x%016llx\n", (long long)repack);
                        printf("  Expected: \"%s\"\n", tests[i].expected);
                        const char *redisasm = v3d_qpu_disasm(&devinfo, repack);
                        printf("  Got:      \"%s\"\n", redisasm);
                        retval = 1;
                        ralloc_free((void *)redisasm);
                }

                printf("PASS\n");
        }

        return retval;
}
