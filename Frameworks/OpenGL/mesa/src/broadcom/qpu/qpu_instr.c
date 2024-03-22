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

#include <stdlib.h>
#include <string.h>
#include "util/macros.h"
#include "broadcom/common/v3d_device_info.h"
#include "qpu_instr.h"

const char *
v3d_qpu_magic_waddr_name(const struct v3d_device_info *devinfo,
                         enum v3d_qpu_waddr waddr)
{
        /* V3D 4.x UNIFA aliases TMU in V3D 3.x in the table below */
        if (devinfo->ver < 40 && waddr == V3D_QPU_WADDR_TMU)
                return "tmu";

        /* V3D 7.x QUAD and REP aliases R5 and R5REPT in the table below
         */
        if (devinfo->ver >= 71 && waddr == V3D_QPU_WADDR_QUAD)
                return "quad";

        if (devinfo->ver >= 71 && waddr == V3D_QPU_WADDR_REP)
                return "rep";

        static const char *waddr_magic[] = {
                [V3D_QPU_WADDR_R0] = "r0",
                [V3D_QPU_WADDR_R1] = "r1",
                [V3D_QPU_WADDR_R2] = "r2",
                [V3D_QPU_WADDR_R3] = "r3",
                [V3D_QPU_WADDR_R4] = "r4",
                [V3D_QPU_WADDR_R5] = "r5",
                [V3D_QPU_WADDR_NOP] = "-",
                [V3D_QPU_WADDR_TLB] = "tlb",
                [V3D_QPU_WADDR_TLBU] = "tlbu",
                [V3D_QPU_WADDR_UNIFA] = "unifa",
                [V3D_QPU_WADDR_TMUL] = "tmul",
                [V3D_QPU_WADDR_TMUD] = "tmud",
                [V3D_QPU_WADDR_TMUA] = "tmua",
                [V3D_QPU_WADDR_TMUAU] = "tmuau",
                [V3D_QPU_WADDR_VPM] = "vpm",
                [V3D_QPU_WADDR_VPMU] = "vpmu",
                [V3D_QPU_WADDR_SYNC] = "sync",
                [V3D_QPU_WADDR_SYNCU] = "syncu",
                [V3D_QPU_WADDR_SYNCB] = "syncb",
                [V3D_QPU_WADDR_RECIP] = "recip",
                [V3D_QPU_WADDR_RSQRT] = "rsqrt",
                [V3D_QPU_WADDR_EXP] = "exp",
                [V3D_QPU_WADDR_LOG] = "log",
                [V3D_QPU_WADDR_SIN] = "sin",
                [V3D_QPU_WADDR_RSQRT2] = "rsqrt2",
                [V3D_QPU_WADDR_TMUC] = "tmuc",
                [V3D_QPU_WADDR_TMUS] = "tmus",
                [V3D_QPU_WADDR_TMUT] = "tmut",
                [V3D_QPU_WADDR_TMUR] = "tmur",
                [V3D_QPU_WADDR_TMUI] = "tmui",
                [V3D_QPU_WADDR_TMUB] = "tmub",
                [V3D_QPU_WADDR_TMUDREF] = "tmudref",
                [V3D_QPU_WADDR_TMUOFF] = "tmuoff",
                [V3D_QPU_WADDR_TMUSCM] = "tmuscm",
                [V3D_QPU_WADDR_TMUSF] = "tmusf",
                [V3D_QPU_WADDR_TMUSLOD] = "tmuslod",
                [V3D_QPU_WADDR_TMUHS] = "tmuhs",
                [V3D_QPU_WADDR_TMUHSCM] = "tmuscm",
                [V3D_QPU_WADDR_TMUHSF] = "tmuhsf",
                [V3D_QPU_WADDR_TMUHSLOD] = "tmuhslod",
                [V3D_QPU_WADDR_R5REP] = "r5rep",
        };

        return waddr_magic[waddr];
}

const char *
v3d_qpu_add_op_name(enum v3d_qpu_add_op op)
{
        static const char *op_names[] = {
                [V3D_QPU_A_FADD] = "fadd",
                [V3D_QPU_A_FADDNF] = "faddnf",
                [V3D_QPU_A_VFPACK] = "vfpack",
                [V3D_QPU_A_ADD] = "add",
                [V3D_QPU_A_SUB] = "sub",
                [V3D_QPU_A_FSUB] = "fsub",
                [V3D_QPU_A_MIN] = "min",
                [V3D_QPU_A_MAX] = "max",
                [V3D_QPU_A_UMIN] = "umin",
                [V3D_QPU_A_UMAX] = "umax",
                [V3D_QPU_A_SHL] = "shl",
                [V3D_QPU_A_SHR] = "shr",
                [V3D_QPU_A_ASR] = "asr",
                [V3D_QPU_A_ROR] = "ror",
                [V3D_QPU_A_FMIN] = "fmin",
                [V3D_QPU_A_FMAX] = "fmax",
                [V3D_QPU_A_VFMIN] = "vfmin",
                [V3D_QPU_A_AND] = "and",
                [V3D_QPU_A_OR] = "or",
                [V3D_QPU_A_XOR] = "xor",
                [V3D_QPU_A_VADD] = "vadd",
                [V3D_QPU_A_VSUB] = "vsub",
                [V3D_QPU_A_NOT] = "not",
                [V3D_QPU_A_NEG] = "neg",
                [V3D_QPU_A_FLAPUSH] = "flapush",
                [V3D_QPU_A_FLBPUSH] = "flbpush",
                [V3D_QPU_A_FLPOP] = "flpop",
                [V3D_QPU_A_RECIP] = "recip",
                [V3D_QPU_A_SETMSF] = "setmsf",
                [V3D_QPU_A_SETREVF] = "setrevf",
                [V3D_QPU_A_NOP] = "nop",
                [V3D_QPU_A_TIDX] = "tidx",
                [V3D_QPU_A_EIDX] = "eidx",
                [V3D_QPU_A_LR] = "lr",
                [V3D_QPU_A_VFLA] = "vfla",
                [V3D_QPU_A_VFLNA] = "vflna",
                [V3D_QPU_A_VFLB] = "vflb",
                [V3D_QPU_A_VFLNB] = "vflnb",
                [V3D_QPU_A_FXCD] = "fxcd",
                [V3D_QPU_A_XCD] = "xcd",
                [V3D_QPU_A_FYCD] = "fycd",
                [V3D_QPU_A_YCD] = "ycd",
                [V3D_QPU_A_MSF] = "msf",
                [V3D_QPU_A_REVF] = "revf",
                [V3D_QPU_A_VDWWT] = "vdwwt",
                [V3D_QPU_A_IID] = "iid",
                [V3D_QPU_A_SAMPID] = "sampid",
                [V3D_QPU_A_BARRIERID] = "barrierid",
                [V3D_QPU_A_TMUWT] = "tmuwt",
                [V3D_QPU_A_VPMSETUP] = "vpmsetup",
                [V3D_QPU_A_VPMWT] = "vpmwt",
                [V3D_QPU_A_FLAFIRST] = "flafirst",
                [V3D_QPU_A_FLNAFIRST] = "flnafirst",
                [V3D_QPU_A_LDVPMV_IN] = "ldvpmv_in",
                [V3D_QPU_A_LDVPMV_OUT] = "ldvpmv_out",
                [V3D_QPU_A_LDVPMD_IN] = "ldvpmd_in",
                [V3D_QPU_A_LDVPMD_OUT] = "ldvpmd_out",
                [V3D_QPU_A_LDVPMP] = "ldvpmp",
                [V3D_QPU_A_RSQRT] = "rsqrt",
                [V3D_QPU_A_EXP] = "exp",
                [V3D_QPU_A_LOG] = "log",
                [V3D_QPU_A_SIN] = "sin",
                [V3D_QPU_A_RSQRT2] = "rsqrt2",
                [V3D_QPU_A_LDVPMG_IN] = "ldvpmg_in",
                [V3D_QPU_A_LDVPMG_OUT] = "ldvpmg_out",
                [V3D_QPU_A_FCMP] = "fcmp",
                [V3D_QPU_A_VFMAX] = "vfmax",
                [V3D_QPU_A_FROUND] = "fround",
                [V3D_QPU_A_FTOIN] = "ftoin",
                [V3D_QPU_A_FTRUNC] = "ftrunc",
                [V3D_QPU_A_FTOIZ] = "ftoiz",
                [V3D_QPU_A_FFLOOR] = "ffloor",
                [V3D_QPU_A_FTOUZ] = "ftouz",
                [V3D_QPU_A_FCEIL] = "fceil",
                [V3D_QPU_A_FTOC] = "ftoc",
                [V3D_QPU_A_FDX] = "fdx",
                [V3D_QPU_A_FDY] = "fdy",
                [V3D_QPU_A_STVPMV] = "stvpmv",
                [V3D_QPU_A_STVPMD] = "stvpmd",
                [V3D_QPU_A_STVPMP] = "stvpmp",
                [V3D_QPU_A_ITOF] = "itof",
                [V3D_QPU_A_CLZ] = "clz",
                [V3D_QPU_A_UTOF] = "utof",
                [V3D_QPU_A_MOV] = "mov",
                [V3D_QPU_A_FMOV] = "fmov",
                [V3D_QPU_A_VPACK] = "vpack",
                [V3D_QPU_A_V8PACK] = "v8pack",
                [V3D_QPU_A_V10PACK] = "v10pack",
                [V3D_QPU_A_V11FPACK] = "v11fpack",
        };

        if (op >= ARRAY_SIZE(op_names))
                return NULL;

        return op_names[op];
}

const char *
v3d_qpu_mul_op_name(enum v3d_qpu_mul_op op)
{
        static const char *op_names[] = {
                [V3D_QPU_M_ADD] = "add",
                [V3D_QPU_M_SUB] = "sub",
                [V3D_QPU_M_UMUL24] = "umul24",
                [V3D_QPU_M_VFMUL] = "vfmul",
                [V3D_QPU_M_SMUL24] = "smul24",
                [V3D_QPU_M_MULTOP] = "multop",
                [V3D_QPU_M_FMOV] = "fmov",
                [V3D_QPU_M_MOV] = "mov",
                [V3D_QPU_M_NOP] = "nop",
                [V3D_QPU_M_FMUL] = "fmul",
                [V3D_QPU_M_FTOUNORM16] = "ftounorm16",
                [V3D_QPU_M_FTOSNORM16] = "ftosnorm16",
                [V3D_QPU_M_VFTOUNORM8] = "vftounorm8",
                [V3D_QPU_M_VFTOSNORM8] = "vftosnorm8",
                [V3D_QPU_M_VFTOUNORM10LO] = "vftounorm10lo",
                [V3D_QPU_M_VFTOUNORM10HI] = "vftounorm10hi",
        };

        if (op >= ARRAY_SIZE(op_names))
                return NULL;

        return op_names[op];
}

const char *
v3d_qpu_cond_name(enum v3d_qpu_cond cond)
{
        switch (cond) {
        case V3D_QPU_COND_NONE:
                return "";
        case V3D_QPU_COND_IFA:
                return ".ifa";
        case V3D_QPU_COND_IFB:
                return ".ifb";
        case V3D_QPU_COND_IFNA:
                return ".ifna";
        case V3D_QPU_COND_IFNB:
                return ".ifnb";
        default:
                unreachable("bad cond value");
        }
}

const char *
v3d_qpu_branch_cond_name(enum v3d_qpu_branch_cond cond)
{
        switch (cond) {
        case V3D_QPU_BRANCH_COND_ALWAYS:
                return "";
        case V3D_QPU_BRANCH_COND_A0:
                return ".a0";
        case V3D_QPU_BRANCH_COND_NA0:
                return ".na0";
        case V3D_QPU_BRANCH_COND_ALLA:
                return ".alla";
        case V3D_QPU_BRANCH_COND_ANYNA:
                return ".anyna";
        case V3D_QPU_BRANCH_COND_ANYA:
                return ".anya";
        case V3D_QPU_BRANCH_COND_ALLNA:
                return ".allna";
        default:
                unreachable("bad branch cond value");
        }
}

const char *
v3d_qpu_msfign_name(enum v3d_qpu_msfign msfign)
{
        switch (msfign) {
        case V3D_QPU_MSFIGN_NONE:
                return "";
        case V3D_QPU_MSFIGN_P:
                return "p";
        case V3D_QPU_MSFIGN_Q:
                return "q";
        default:
                unreachable("bad branch cond value");
        }
}

const char *
v3d_qpu_pf_name(enum v3d_qpu_pf pf)
{
        switch (pf) {
        case V3D_QPU_PF_NONE:
                return "";
        case V3D_QPU_PF_PUSHZ:
                return ".pushz";
        case V3D_QPU_PF_PUSHN:
                return ".pushn";
        case V3D_QPU_PF_PUSHC:
                return ".pushc";
        default:
                unreachable("bad pf value");
        }
}

const char *
v3d_qpu_uf_name(enum v3d_qpu_uf uf)
{
        switch (uf) {
        case V3D_QPU_UF_NONE:
                return "";
        case V3D_QPU_UF_ANDZ:
                return ".andz";
        case V3D_QPU_UF_ANDNZ:
                return ".andnz";
        case V3D_QPU_UF_NORZ:
                return ".norz";
        case V3D_QPU_UF_NORNZ:
                return ".nornz";
        case V3D_QPU_UF_ANDN:
                return ".andn";
        case V3D_QPU_UF_ANDNN:
                return ".andnn";
        case V3D_QPU_UF_NORN:
                return ".norn";
        case V3D_QPU_UF_NORNN:
                return ".nornn";
        case V3D_QPU_UF_ANDC:
                return ".andc";
        case V3D_QPU_UF_ANDNC:
                return ".andnc";
        case V3D_QPU_UF_NORC:
                return ".norc";
        case V3D_QPU_UF_NORNC:
                return ".nornc";
        default:
                unreachable("bad pf value");
        }
}

const char *
v3d_qpu_pack_name(enum v3d_qpu_output_pack pack)
{
        switch (pack) {
        case V3D_QPU_PACK_NONE:
                return "";
        case V3D_QPU_PACK_L:
                return ".l";
        case V3D_QPU_PACK_H:
                return ".h";
        default:
                unreachable("bad pack value");
        }
}

const char *
v3d_qpu_unpack_name(enum v3d_qpu_input_unpack unpack)
{
        switch (unpack) {
        case V3D_QPU_UNPACK_NONE:
                return "";
        case V3D_QPU_UNPACK_L:
                return ".l";
        case V3D_QPU_UNPACK_H:
                return ".h";
        case V3D_QPU_UNPACK_ABS:
                return ".abs";
        case V3D_QPU_UNPACK_REPLICATE_32F_16:
                return ".ff";
        case V3D_QPU_UNPACK_REPLICATE_L_16:
                return ".ll";
        case V3D_QPU_UNPACK_REPLICATE_H_16:
                return ".hh";
        case V3D_QPU_UNPACK_SWAP_16:
                return ".swp";
        default:
                unreachable("bad unpack value");
        }
}

#define D	1
#define A	2
#define B	4
static const uint8_t add_op_args[] = {
        [V3D_QPU_A_FADD] = D | A | B,
        [V3D_QPU_A_FADDNF] = D | A | B,
        [V3D_QPU_A_VFPACK] = D | A | B,
        [V3D_QPU_A_ADD] = D | A | B,
        [V3D_QPU_A_VFPACK] = D | A | B,
        [V3D_QPU_A_SUB] = D | A | B,
        [V3D_QPU_A_VFPACK] = D | A | B,
        [V3D_QPU_A_FSUB] = D | A | B,
        [V3D_QPU_A_MIN] = D | A | B,
        [V3D_QPU_A_MAX] = D | A | B,
        [V3D_QPU_A_UMIN] = D | A | B,
        [V3D_QPU_A_UMAX] = D | A | B,
        [V3D_QPU_A_SHL] = D | A | B,
        [V3D_QPU_A_SHR] = D | A | B,
        [V3D_QPU_A_ASR] = D | A | B,
        [V3D_QPU_A_ROR] = D | A | B,
        [V3D_QPU_A_FMIN] = D | A | B,
        [V3D_QPU_A_FMAX] = D | A | B,
        [V3D_QPU_A_VFMIN] = D | A | B,

        [V3D_QPU_A_AND] = D | A | B,
        [V3D_QPU_A_OR] = D | A | B,
        [V3D_QPU_A_XOR] = D | A | B,

        [V3D_QPU_A_VADD] = D | A | B,
        [V3D_QPU_A_VSUB] = D | A | B,
        [V3D_QPU_A_NOT] = D | A,
        [V3D_QPU_A_NEG] = D | A,
        [V3D_QPU_A_FLAPUSH] = D | A,
        [V3D_QPU_A_FLBPUSH] = D | A,
        [V3D_QPU_A_FLPOP] = D | A,
        [V3D_QPU_A_RECIP] = D | A,
        [V3D_QPU_A_SETMSF] = D | A,
        [V3D_QPU_A_SETREVF] = D | A,
        [V3D_QPU_A_NOP] = 0,
        [V3D_QPU_A_TIDX] = D,
        [V3D_QPU_A_EIDX] = D,
        [V3D_QPU_A_LR] = D,
        [V3D_QPU_A_VFLA] = D,
        [V3D_QPU_A_VFLNA] = D,
        [V3D_QPU_A_VFLB] = D,
        [V3D_QPU_A_VFLNB] = D,

        [V3D_QPU_A_FXCD] = D,
        [V3D_QPU_A_XCD] = D,
        [V3D_QPU_A_FYCD] = D,
        [V3D_QPU_A_YCD] = D,

        [V3D_QPU_A_MSF] = D,
        [V3D_QPU_A_REVF] = D,
        [V3D_QPU_A_VDWWT] = D,
        [V3D_QPU_A_IID] = D,
        [V3D_QPU_A_SAMPID] = D,
        [V3D_QPU_A_BARRIERID] = D,
        [V3D_QPU_A_TMUWT] = D,
        [V3D_QPU_A_VPMWT] = D,
        [V3D_QPU_A_FLAFIRST] = D,
        [V3D_QPU_A_FLNAFIRST] = D,

        [V3D_QPU_A_VPMSETUP] = D | A,

        [V3D_QPU_A_LDVPMV_IN] = D | A,
        [V3D_QPU_A_LDVPMV_OUT] = D | A,
        [V3D_QPU_A_LDVPMD_IN] = D | A,
        [V3D_QPU_A_LDVPMD_OUT] = D | A,
        [V3D_QPU_A_LDVPMP] = D | A,
        [V3D_QPU_A_RSQRT] = D | A,
        [V3D_QPU_A_EXP] = D | A,
        [V3D_QPU_A_LOG] = D | A,
        [V3D_QPU_A_SIN] = D | A,
        [V3D_QPU_A_RSQRT2] = D | A,
        [V3D_QPU_A_LDVPMG_IN] = D | A | B,
        [V3D_QPU_A_LDVPMG_OUT] = D | A | B,

        /* FIXME: MOVABSNEG */

        [V3D_QPU_A_FCMP] = D | A | B,
        [V3D_QPU_A_VFMAX] = D | A | B,

        [V3D_QPU_A_FROUND] = D | A,
        [V3D_QPU_A_FTOIN] = D | A,
        [V3D_QPU_A_FTRUNC] = D | A,
        [V3D_QPU_A_FTOIZ] = D | A,
        [V3D_QPU_A_FFLOOR] = D | A,
        [V3D_QPU_A_FTOUZ] = D | A,
        [V3D_QPU_A_FCEIL] = D | A,
        [V3D_QPU_A_FTOC] = D | A,

        [V3D_QPU_A_FDX] = D | A,
        [V3D_QPU_A_FDY] = D | A,

        [V3D_QPU_A_STVPMV] = A | B,
        [V3D_QPU_A_STVPMD] = A | B,
        [V3D_QPU_A_STVPMP] = A | B,

        [V3D_QPU_A_ITOF] = D | A,
        [V3D_QPU_A_CLZ] = D | A,
        [V3D_QPU_A_UTOF] = D | A,

        [V3D_QPU_A_MOV] = D | A,
        [V3D_QPU_A_FMOV] = D | A,
        [V3D_QPU_A_VPACK] = D | A | B,
        [V3D_QPU_A_V8PACK] = D | A | B,
        [V3D_QPU_A_V10PACK] = D | A | B,
        [V3D_QPU_A_V11FPACK] = D | A | B,
};

static const uint8_t mul_op_args[] = {
        [V3D_QPU_M_ADD] = D | A | B,
        [V3D_QPU_M_SUB] = D | A | B,
        [V3D_QPU_M_UMUL24] = D | A | B,
        [V3D_QPU_M_VFMUL] = D | A | B,
        [V3D_QPU_M_SMUL24] = D | A | B,
        [V3D_QPU_M_MULTOP] = D | A | B,
        [V3D_QPU_M_FMOV] = D | A,
        [V3D_QPU_M_NOP] = 0,
        [V3D_QPU_M_MOV] = D | A,
        [V3D_QPU_M_FMUL] = D | A | B,
        [V3D_QPU_M_FTOUNORM16] = D | A,
        [V3D_QPU_M_FTOSNORM16] = D | A,
        [V3D_QPU_M_VFTOUNORM8] = D | A,
        [V3D_QPU_M_VFTOSNORM8] = D | A,
        [V3D_QPU_M_VFTOUNORM10LO] = D | A,
        [V3D_QPU_M_VFTOUNORM10HI] = D | A,
};

bool
v3d_qpu_add_op_has_dst(enum v3d_qpu_add_op op)
{
        assert(op < ARRAY_SIZE(add_op_args));

        return add_op_args[op] & D;
}

bool
v3d_qpu_mul_op_has_dst(enum v3d_qpu_mul_op op)
{
        assert(op < ARRAY_SIZE(mul_op_args));

        return mul_op_args[op] & D;
}

int
v3d_qpu_add_op_num_src(enum v3d_qpu_add_op op)
{
        assert(op < ARRAY_SIZE(add_op_args));

        uint8_t args = add_op_args[op];
        if (args & B)
                return 2;
        else if (args & A)
                return 1;
        else
                return 0;
}

int
v3d_qpu_mul_op_num_src(enum v3d_qpu_mul_op op)
{
        assert(op < ARRAY_SIZE(mul_op_args));

        uint8_t args = mul_op_args[op];
        if (args & B)
                return 2;
        else if (args & A)
                return 1;
        else
                return 0;
}

enum v3d_qpu_cond
v3d_qpu_cond_invert(enum v3d_qpu_cond cond)
{
        switch (cond) {
        case V3D_QPU_COND_IFA:
                return V3D_QPU_COND_IFNA;
        case V3D_QPU_COND_IFNA:
                return V3D_QPU_COND_IFA;
        case V3D_QPU_COND_IFB:
                return V3D_QPU_COND_IFNB;
        case V3D_QPU_COND_IFNB:
                return V3D_QPU_COND_IFB;
        default:
                unreachable("Non-invertible cond");
        }
}

bool
v3d_qpu_magic_waddr_is_sfu(enum v3d_qpu_waddr waddr)
{
        switch (waddr) {
        case V3D_QPU_WADDR_RECIP:
        case V3D_QPU_WADDR_RSQRT:
        case V3D_QPU_WADDR_EXP:
        case V3D_QPU_WADDR_LOG:
        case V3D_QPU_WADDR_SIN:
        case V3D_QPU_WADDR_RSQRT2:
                return true;
        default:
                return false;
        }
}

bool
v3d_qpu_magic_waddr_is_tmu(const struct v3d_device_info *devinfo,
                           enum v3d_qpu_waddr waddr)
{
        if (devinfo->ver >= 40) {
                return ((waddr >= V3D_QPU_WADDR_TMUD &&
                         waddr <= V3D_QPU_WADDR_TMUAU) ||
                       (waddr >= V3D_QPU_WADDR_TMUC &&
                        waddr <= V3D_QPU_WADDR_TMUHSLOD));
        } else {
                return ((waddr >= V3D_QPU_WADDR_TMU &&
                         waddr <= V3D_QPU_WADDR_TMUAU) ||
                       (waddr >= V3D_QPU_WADDR_TMUC &&
                        waddr <= V3D_QPU_WADDR_TMUHSLOD));
        }
}

bool
v3d_qpu_waits_on_tmu(const struct v3d_qpu_instr *inst)
{
        return (inst->sig.ldtmu ||
                (inst->type == V3D_QPU_INSTR_TYPE_ALU &&
                 inst->alu.add.op == V3D_QPU_A_TMUWT));
}

bool
v3d_qpu_magic_waddr_is_tlb(enum v3d_qpu_waddr waddr)
{
        return (waddr == V3D_QPU_WADDR_TLB ||
                waddr == V3D_QPU_WADDR_TLBU);
}

bool
v3d_qpu_magic_waddr_is_vpm(enum v3d_qpu_waddr waddr)
{
        return (waddr == V3D_QPU_WADDR_VPM ||
                waddr == V3D_QPU_WADDR_VPMU);
}

bool
v3d_qpu_magic_waddr_is_tsy(enum v3d_qpu_waddr waddr)
{
        return (waddr == V3D_QPU_WADDR_SYNC ||
                waddr == V3D_QPU_WADDR_SYNCB ||
                waddr == V3D_QPU_WADDR_SYNCU);
}

bool
v3d_qpu_magic_waddr_loads_unif(enum v3d_qpu_waddr waddr)
{
        switch (waddr) {
        case V3D_QPU_WADDR_VPMU:
        case V3D_QPU_WADDR_TLBU:
        case V3D_QPU_WADDR_TMUAU:
        case V3D_QPU_WADDR_SYNCU:
                return true;
        default:
                return false;
        }
}

static bool
v3d_qpu_add_op_reads_vpm(enum  v3d_qpu_add_op op)
{
        switch (op) {
        case V3D_QPU_A_VPMSETUP:
        case V3D_QPU_A_LDVPMV_IN:
        case V3D_QPU_A_LDVPMV_OUT:
        case V3D_QPU_A_LDVPMD_IN:
        case V3D_QPU_A_LDVPMD_OUT:
        case V3D_QPU_A_LDVPMP:
        case V3D_QPU_A_LDVPMG_IN:
        case V3D_QPU_A_LDVPMG_OUT:
                return true;
        default:
                return false;
        }
}

static bool
v3d_qpu_add_op_writes_vpm(enum  v3d_qpu_add_op op)
{
        switch (op) {
        case V3D_QPU_A_VPMSETUP:
        case V3D_QPU_A_STVPMV:
        case V3D_QPU_A_STVPMD:
        case V3D_QPU_A_STVPMP:
                return true;
        default:
                return false;
        }
}

bool
v3d_qpu_reads_tlb(const struct v3d_qpu_instr *inst)
{
        return inst->sig.ldtlb || inst->sig.ldtlbu;
}

bool
v3d_qpu_writes_tlb(const struct v3d_qpu_instr *inst)
{
        if (inst->type == V3D_QPU_INSTR_TYPE_ALU) {
                if (inst->alu.add.op != V3D_QPU_A_NOP &&
                    inst->alu.add.magic_write &&
                    v3d_qpu_magic_waddr_is_tlb(inst->alu.add.waddr)) {
                        return true;
                }

                if (inst->alu.mul.op != V3D_QPU_M_NOP &&
                    inst->alu.mul.magic_write &&
                    v3d_qpu_magic_waddr_is_tlb(inst->alu.mul.waddr)) {
                        return true;
                }
        }

        return false;
}

bool
v3d_qpu_uses_tlb(const struct v3d_qpu_instr *inst)
{
        return  v3d_qpu_writes_tlb(inst) || v3d_qpu_reads_tlb(inst);
}

bool
v3d_qpu_uses_sfu(const struct v3d_qpu_instr *inst)
{
        return v3d_qpu_instr_is_sfu(inst) || v3d_qpu_instr_is_legacy_sfu(inst);
}

/* Checks whether the instruction implements a SFU operation by the writing
 * to specific magic register addresses instead of using SFU ALU opcodes.
 */
bool
v3d_qpu_instr_is_legacy_sfu(const struct v3d_qpu_instr *inst)
{
        if (inst->type == V3D_QPU_INSTR_TYPE_ALU) {
                if (inst->alu.add.op != V3D_QPU_A_NOP &&
                    inst->alu.add.magic_write &&
                    v3d_qpu_magic_waddr_is_sfu(inst->alu.add.waddr)) {
                        return true;
                }

                if (inst->alu.mul.op != V3D_QPU_M_NOP &&
                    inst->alu.mul.magic_write &&
                    v3d_qpu_magic_waddr_is_sfu(inst->alu.mul.waddr)) {
                        return true;
                }
        }

        return false;
}

bool
v3d_qpu_instr_is_sfu(const struct v3d_qpu_instr *inst)
{
        if (inst->type == V3D_QPU_INSTR_TYPE_ALU) {
                switch (inst->alu.add.op) {
                case V3D_QPU_A_RECIP:
                case V3D_QPU_A_RSQRT:
                case V3D_QPU_A_EXP:
                case V3D_QPU_A_LOG:
                case V3D_QPU_A_SIN:
                case V3D_QPU_A_RSQRT2:
                        return true;
                default:
                        return false;
                }
        }
        return false;
}

bool
v3d_qpu_writes_tmu(const struct v3d_device_info *devinfo,
                   const struct v3d_qpu_instr *inst)
{
        return (inst->type == V3D_QPU_INSTR_TYPE_ALU &&
                ((inst->alu.add.op != V3D_QPU_A_NOP &&
                  inst->alu.add.magic_write &&
                  v3d_qpu_magic_waddr_is_tmu(devinfo, inst->alu.add.waddr)) ||
                 (inst->alu.mul.op != V3D_QPU_M_NOP &&
                  inst->alu.mul.magic_write &&
                  v3d_qpu_magic_waddr_is_tmu(devinfo, inst->alu.mul.waddr))));
}

bool
v3d_qpu_writes_tmu_not_tmuc(const struct v3d_device_info *devinfo,
                            const struct v3d_qpu_instr *inst)
{
        return v3d_qpu_writes_tmu(devinfo, inst) &&
               (!inst->alu.add.magic_write ||
                inst->alu.add.waddr != V3D_QPU_WADDR_TMUC) &&
               (!inst->alu.mul.magic_write ||
                inst->alu.mul.waddr != V3D_QPU_WADDR_TMUC);
}

bool
v3d_qpu_reads_vpm(const struct v3d_qpu_instr *inst)
{
        if (inst->sig.ldvpm)
                return true;

        if (inst->type == V3D_QPU_INSTR_TYPE_ALU) {
                if (v3d_qpu_add_op_reads_vpm(inst->alu.add.op))
                        return true;
        }

        return false;
}

bool
v3d_qpu_writes_vpm(const struct v3d_qpu_instr *inst)
{
        if (inst->type == V3D_QPU_INSTR_TYPE_ALU) {
                if (v3d_qpu_add_op_writes_vpm(inst->alu.add.op))
                        return true;

                if (inst->alu.add.op != V3D_QPU_A_NOP &&
                    inst->alu.add.magic_write &&
                    v3d_qpu_magic_waddr_is_vpm(inst->alu.add.waddr)) {
                        return true;
                }

                if (inst->alu.mul.op != V3D_QPU_M_NOP &&
                    inst->alu.mul.magic_write &&
                    v3d_qpu_magic_waddr_is_vpm(inst->alu.mul.waddr)) {
                        return true;
                }
        }

        return false;
}

bool
v3d_qpu_writes_unifa(const struct v3d_device_info *devinfo,
                     const struct v3d_qpu_instr *inst)
{
        if (devinfo->ver < 40)
                return false;

        if (inst->type == V3D_QPU_INSTR_TYPE_ALU) {
                if (inst->alu.add.op != V3D_QPU_A_NOP &&
                    inst->alu.add.magic_write &&
                    inst->alu.add.waddr == V3D_QPU_WADDR_UNIFA) {
                        return true;
                }

                if (inst->alu.mul.op != V3D_QPU_M_NOP &&
                    inst->alu.mul.magic_write &&
                    inst->alu.mul.waddr == V3D_QPU_WADDR_UNIFA) {
                        return true;
                }

                if (v3d_qpu_sig_writes_address(devinfo, &inst->sig) &&
                    inst->sig_magic &&
                    inst->sig_addr == V3D_QPU_WADDR_UNIFA) {
                        return true;
                }
        }

        return false;
}

bool
v3d_qpu_waits_vpm(const struct v3d_qpu_instr *inst)
{
        return inst->type == V3D_QPU_INSTR_TYPE_ALU &&
               inst->alu.add.op == V3D_QPU_A_VPMWT;
}

bool
v3d_qpu_reads_or_writes_vpm(const struct v3d_qpu_instr *inst)
{
        return v3d_qpu_reads_vpm(inst) || v3d_qpu_writes_vpm(inst);
}

bool
v3d_qpu_uses_vpm(const struct v3d_qpu_instr *inst)
{
        return v3d_qpu_reads_vpm(inst) ||
               v3d_qpu_writes_vpm(inst) ||
               v3d_qpu_waits_vpm(inst);
}

static bool
qpu_writes_magic_waddr_explicitly(const struct v3d_device_info *devinfo,
                                  const struct v3d_qpu_instr *inst,
                                  uint32_t waddr)
{
        if (inst->type == V3D_QPU_INSTR_TYPE_ALU) {
                if (inst->alu.add.op != V3D_QPU_A_NOP &&
                    inst->alu.add.magic_write && inst->alu.add.waddr == waddr)
                        return true;

                if (inst->alu.mul.op != V3D_QPU_M_NOP &&
                    inst->alu.mul.magic_write && inst->alu.mul.waddr == waddr)
                        return true;
        }

        if (v3d_qpu_sig_writes_address(devinfo, &inst->sig) &&
            inst->sig_magic && inst->sig_addr == waddr) {
                return true;
        }

        return false;
}

bool
v3d_qpu_writes_r3(const struct v3d_device_info *devinfo,
                  const struct v3d_qpu_instr *inst)
{
        if(!devinfo->has_accumulators)
                return false;

        if (qpu_writes_magic_waddr_explicitly(devinfo, inst, V3D_QPU_WADDR_R3))
                return true;

        return (devinfo->ver < 41 && inst->sig.ldvary) || inst->sig.ldvpm;
}

bool
v3d_qpu_writes_r4(const struct v3d_device_info *devinfo,
                  const struct v3d_qpu_instr *inst)
{
        if (!devinfo->has_accumulators)
                return false;

        if (inst->type == V3D_QPU_INSTR_TYPE_ALU) {
                if (inst->alu.add.op != V3D_QPU_A_NOP &&
                    inst->alu.add.magic_write &&
                    (inst->alu.add.waddr == V3D_QPU_WADDR_R4 ||
                     v3d_qpu_magic_waddr_is_sfu(inst->alu.add.waddr))) {
                        return true;
                }

                if (inst->alu.mul.op != V3D_QPU_M_NOP &&
                    inst->alu.mul.magic_write &&
                    (inst->alu.mul.waddr == V3D_QPU_WADDR_R4 ||
                     v3d_qpu_magic_waddr_is_sfu(inst->alu.mul.waddr))) {
                        return true;
                }
        }

        if (v3d_qpu_sig_writes_address(devinfo, &inst->sig)) {
                if (inst->sig_magic && inst->sig_addr == V3D_QPU_WADDR_R4)
                        return true;
        } else if (inst->sig.ldtmu) {
                return true;
        }

        return false;
}

bool
v3d_qpu_writes_r5(const struct v3d_device_info *devinfo,
                  const struct v3d_qpu_instr *inst)
{
        if (!devinfo->has_accumulators)
                return false;

        if (qpu_writes_magic_waddr_explicitly(devinfo, inst, V3D_QPU_WADDR_R5))
                return true;

        return inst->sig.ldvary || inst->sig.ldunif || inst->sig.ldunifa;
}

bool
v3d_qpu_writes_accum(const struct v3d_device_info *devinfo,
                     const struct v3d_qpu_instr *inst)
{
        if (!devinfo->has_accumulators)
                return false;

        if (v3d_qpu_writes_r5(devinfo, inst))
                return true;
        if (v3d_qpu_writes_r4(devinfo, inst))
                return true;
        if (v3d_qpu_writes_r3(devinfo, inst))
                return true;
        if (qpu_writes_magic_waddr_explicitly(devinfo, inst, V3D_QPU_WADDR_R2))
                return true;
        if (qpu_writes_magic_waddr_explicitly(devinfo, inst, V3D_QPU_WADDR_R1))
                return true;
        if (qpu_writes_magic_waddr_explicitly(devinfo, inst, V3D_QPU_WADDR_R0))
                return true;

        return false;
}

bool
v3d_qpu_writes_rf0_implicitly(const struct v3d_device_info *devinfo,
                              const struct v3d_qpu_instr *inst)
{
        if (devinfo->ver >= 71 &&
            (inst->sig.ldvary || inst->sig.ldunif || inst->sig.ldunifa)) {
                return true;
        }

        return false;
}

bool
v3d_qpu_uses_mux(const struct v3d_qpu_instr *inst, enum v3d_qpu_mux mux)
{
        int add_nsrc = v3d_qpu_add_op_num_src(inst->alu.add.op);
        int mul_nsrc = v3d_qpu_mul_op_num_src(inst->alu.mul.op);

        return ((add_nsrc > 0 && inst->alu.add.a.mux == mux) ||
                (add_nsrc > 1 && inst->alu.add.b.mux == mux) ||
                (mul_nsrc > 0 && inst->alu.mul.a.mux == mux) ||
                (mul_nsrc > 1 && inst->alu.mul.b.mux == mux));
}

bool
v3d71_qpu_reads_raddr(const struct v3d_qpu_instr *inst, uint8_t raddr)
{
        int add_nsrc = v3d_qpu_add_op_num_src(inst->alu.add.op);
        int mul_nsrc = v3d_qpu_mul_op_num_src(inst->alu.mul.op);

        return (add_nsrc > 0 && !inst->sig.small_imm_a && inst->alu.add.a.raddr == raddr) ||
               (add_nsrc > 1 && !inst->sig.small_imm_b && inst->alu.add.b.raddr == raddr) ||
               (mul_nsrc > 0 && !inst->sig.small_imm_c && inst->alu.mul.a.raddr == raddr) ||
               (mul_nsrc > 1 && !inst->sig.small_imm_d && inst->alu.mul.b.raddr == raddr);
}

bool
v3d71_qpu_writes_waddr_explicitly(const struct v3d_device_info *devinfo,
                                  const struct v3d_qpu_instr *inst,
                                  uint8_t waddr)
{
        if (inst->type != V3D_QPU_INSTR_TYPE_ALU)
                return false;

        if (v3d_qpu_add_op_has_dst(inst->alu.add.op) &&
            !inst->alu.add.magic_write &&
            inst->alu.add.waddr == waddr) {
                return true;
        }

        if (v3d_qpu_mul_op_has_dst(inst->alu.mul.op) &&
            !inst->alu.mul.magic_write &&
            inst->alu.mul.waddr == waddr) {
                return true;
        }

        if (v3d_qpu_sig_writes_address(devinfo, &inst->sig) &&
            !inst->sig_magic && inst->sig_addr == waddr) {
                return true;
        }

        return false;
}

bool
v3d_qpu_sig_writes_address(const struct v3d_device_info *devinfo,
                           const struct v3d_qpu_sig *sig)
{
        if (devinfo->ver < 41)
                return false;

        return (sig->ldunifrf ||
                sig->ldunifarf ||
                sig->ldvary ||
                sig->ldtmu ||
                sig->ldtlb ||
                sig->ldtlbu);
}

bool
v3d_qpu_reads_flags(const struct v3d_qpu_instr *inst)
{
        if (inst->type == V3D_QPU_INSTR_TYPE_BRANCH) {
                return inst->branch.cond != V3D_QPU_BRANCH_COND_ALWAYS;
        } else if (inst->type == V3D_QPU_INSTR_TYPE_ALU) {
                if (inst->flags.ac != V3D_QPU_COND_NONE ||
                    inst->flags.mc != V3D_QPU_COND_NONE ||
                    inst->flags.auf != V3D_QPU_UF_NONE ||
                    inst->flags.muf != V3D_QPU_UF_NONE)
                        return true;

                switch (inst->alu.add.op) {
                case V3D_QPU_A_VFLA:
                case V3D_QPU_A_VFLNA:
                case V3D_QPU_A_VFLB:
                case V3D_QPU_A_VFLNB:
                case V3D_QPU_A_FLAPUSH:
                case V3D_QPU_A_FLBPUSH:
                case V3D_QPU_A_FLAFIRST:
                case V3D_QPU_A_FLNAFIRST:
                        return true;
                default:
                        break;
                }
        }

        return false;
}

bool
v3d_qpu_writes_flags(const struct v3d_qpu_instr *inst)
{
        if (inst->flags.apf != V3D_QPU_PF_NONE ||
            inst->flags.mpf != V3D_QPU_PF_NONE ||
            inst->flags.auf != V3D_QPU_UF_NONE ||
            inst->flags.muf != V3D_QPU_UF_NONE) {
                return true;
        }

        return false;
}

bool
v3d_qpu_unpacks_f32(const struct v3d_qpu_instr *inst)
{
        if (inst->type != V3D_QPU_INSTR_TYPE_ALU)
                return false;

        switch (inst->alu.add.op) {
        case V3D_QPU_A_FADD:
        case V3D_QPU_A_FADDNF:
        case V3D_QPU_A_FSUB:
        case V3D_QPU_A_FMIN:
        case V3D_QPU_A_FMAX:
        case V3D_QPU_A_FCMP:
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
        case V3D_QPU_A_VFPACK:
                return true;
                break;
        default:
                break;
        }

        switch (inst->alu.mul.op) {
        case V3D_QPU_M_FMOV:
        case V3D_QPU_M_FMUL:
                return true;
                break;
        default:
                break;
        }

        return false;
}
bool
v3d_qpu_unpacks_f16(const struct v3d_qpu_instr *inst)
{
        if (inst->type != V3D_QPU_INSTR_TYPE_ALU)
                return false;

        switch (inst->alu.add.op) {
        case V3D_QPU_A_VFMIN:
        case V3D_QPU_A_VFMAX:
                return true;
                break;
        default:
                break;
        }

        switch (inst->alu.mul.op) {
        case V3D_QPU_M_VFMUL:
                return true;
                break;
        default:
                break;
        }

        return false;
}

bool
v3d_qpu_is_nop(struct v3d_qpu_instr *inst)
{
        static const struct v3d_qpu_sig nosig = { 0 };

        if (inst->type != V3D_QPU_INSTR_TYPE_ALU)
                return false;
        if (inst->alu.add.op != V3D_QPU_A_NOP)
                return false;
        if (inst->alu.mul.op != V3D_QPU_M_NOP)
                return false;
        if (memcmp(&inst->sig, &nosig, sizeof(nosig)))
                return false;
        return true;
}
