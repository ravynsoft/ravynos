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
 * @file qpu_instr.h
 *
 * Definitions of the unpacked form of QPU instructions.  Assembly and
 * disassembly will use this for talking about instructions, with qpu_encode.c
 * and qpu_decode.c handling the pack and unpack of the actual 64-bit QPU
 * instruction.
 */

#ifndef QPU_INSTR_H
#define QPU_INSTR_H

#include <stdbool.h>
#include <stdint.h>
#include "util/macros.h"

struct v3d_device_info;

struct v3d_qpu_sig {
        bool thrsw:1;
        bool ldunif:1;
        bool ldunifa:1;
        bool ldunifrf:1;
        bool ldunifarf:1;
        bool ldtmu:1;
        bool ldvary:1;
        bool ldvpm:1;
        bool ldtlb:1;
        bool ldtlbu:1;
        bool ucb:1;
        bool rotate:1;
        bool wrtmuc:1;
        bool small_imm_a:1; /* raddr_a (add a), since V3D 7.x */
        bool small_imm_b:1; /* raddr_b (add b) */
        bool small_imm_c:1; /* raddr_c (mul a), since V3D 7.x */
        bool small_imm_d:1; /* raddr_d (mul b), since V3D 7.x */
};

enum v3d_qpu_cond {
        V3D_QPU_COND_NONE,
        V3D_QPU_COND_IFA,
        V3D_QPU_COND_IFB,
        V3D_QPU_COND_IFNA,
        V3D_QPU_COND_IFNB,
};

enum v3d_qpu_pf {
        V3D_QPU_PF_NONE,
        V3D_QPU_PF_PUSHZ,
        V3D_QPU_PF_PUSHN,
        V3D_QPU_PF_PUSHC,
};

enum v3d_qpu_uf {
        V3D_QPU_UF_NONE,
        V3D_QPU_UF_ANDZ,
        V3D_QPU_UF_ANDNZ,
        V3D_QPU_UF_NORNZ,
        V3D_QPU_UF_NORZ,
        V3D_QPU_UF_ANDN,
        V3D_QPU_UF_ANDNN,
        V3D_QPU_UF_NORNN,
        V3D_QPU_UF_NORN,
        V3D_QPU_UF_ANDC,
        V3D_QPU_UF_ANDNC,
        V3D_QPU_UF_NORNC,
        V3D_QPU_UF_NORC,
};

enum v3d_qpu_waddr {
        V3D_QPU_WADDR_R0 = 0,    /* Reserved on V3D 7.x */
        V3D_QPU_WADDR_R1 = 1,    /* Reserved on V3D 7.x */
        V3D_QPU_WADDR_R2 = 2,    /* Reserved on V3D 7.x */
        V3D_QPU_WADDR_R3 = 3,    /* Reserved on V3D 7.x */
        V3D_QPU_WADDR_R4 = 4,    /* Reserved on V3D 7.x */
        V3D_QPU_WADDR_R5 = 5,    /* V3D 4.x */
        V3D_QPU_WADDR_QUAD = 5,  /* V3D 7.x */
        V3D_QPU_WADDR_NOP = 6,
        V3D_QPU_WADDR_TLB = 7,
        V3D_QPU_WADDR_TLBU = 8,
        V3D_QPU_WADDR_TMU = 9,   /* V3D 3.x */
        V3D_QPU_WADDR_UNIFA = 9, /* V3D 4.x */
        V3D_QPU_WADDR_TMUL = 10,
        V3D_QPU_WADDR_TMUD = 11,
        V3D_QPU_WADDR_TMUA = 12,
        V3D_QPU_WADDR_TMUAU = 13,
        V3D_QPU_WADDR_VPM = 14,
        V3D_QPU_WADDR_VPMU = 15,
        V3D_QPU_WADDR_SYNC = 16,
        V3D_QPU_WADDR_SYNCU = 17,
        V3D_QPU_WADDR_SYNCB = 18,
        V3D_QPU_WADDR_RECIP = 19,  /* Reserved on V3D 7.x */
        V3D_QPU_WADDR_RSQRT = 20,  /* Reserved on V3D 7.x */
        V3D_QPU_WADDR_EXP = 21,    /* Reserved on V3D 7.x */
        V3D_QPU_WADDR_LOG = 22,    /* Reserved on V3D 7.x */
        V3D_QPU_WADDR_SIN = 23,    /* Reserved on V3D 7.x */
        V3D_QPU_WADDR_RSQRT2 = 24, /* Reserved on V3D 7.x */
        V3D_QPU_WADDR_TMUC = 32,
        V3D_QPU_WADDR_TMUS = 33,
        V3D_QPU_WADDR_TMUT = 34,
        V3D_QPU_WADDR_TMUR = 35,
        V3D_QPU_WADDR_TMUI = 36,
        V3D_QPU_WADDR_TMUB = 37,
        V3D_QPU_WADDR_TMUDREF = 38,
        V3D_QPU_WADDR_TMUOFF = 39,
        V3D_QPU_WADDR_TMUSCM = 40,
        V3D_QPU_WADDR_TMUSF = 41,
        V3D_QPU_WADDR_TMUSLOD = 42,
        V3D_QPU_WADDR_TMUHS = 43,
        V3D_QPU_WADDR_TMUHSCM = 44,
        V3D_QPU_WADDR_TMUHSF = 45,
        V3D_QPU_WADDR_TMUHSLOD = 46,
        V3D_QPU_WADDR_R5REP = 55, /* V3D 4.x */
        V3D_QPU_WADDR_REP = 55,   /* V3D 7.x */
};

struct v3d_qpu_flags {
        enum v3d_qpu_cond ac, mc;
        enum v3d_qpu_pf apf, mpf;
        enum v3d_qpu_uf auf, muf;
};

enum v3d_qpu_add_op {
        V3D_QPU_A_FADD,
        V3D_QPU_A_FADDNF,
        V3D_QPU_A_VFPACK,
        V3D_QPU_A_ADD,
        V3D_QPU_A_SUB,
        V3D_QPU_A_FSUB,
        V3D_QPU_A_MIN,
        V3D_QPU_A_MAX,
        V3D_QPU_A_UMIN,
        V3D_QPU_A_UMAX,
        V3D_QPU_A_SHL,
        V3D_QPU_A_SHR,
        V3D_QPU_A_ASR,
        V3D_QPU_A_ROR,
        V3D_QPU_A_FMIN,
        V3D_QPU_A_FMAX,
        V3D_QPU_A_VFMIN,
        V3D_QPU_A_AND,
        V3D_QPU_A_OR,
        V3D_QPU_A_XOR,
        V3D_QPU_A_VADD,
        V3D_QPU_A_VSUB,
        V3D_QPU_A_NOT,
        V3D_QPU_A_NEG,
        V3D_QPU_A_FLAPUSH,
        V3D_QPU_A_FLBPUSH,
        V3D_QPU_A_FLPOP,
        V3D_QPU_A_RECIP,
        V3D_QPU_A_SETMSF,
        V3D_QPU_A_SETREVF,
        V3D_QPU_A_NOP,
        V3D_QPU_A_TIDX,
        V3D_QPU_A_EIDX,
        V3D_QPU_A_LR,
        V3D_QPU_A_VFLA,
        V3D_QPU_A_VFLNA,
        V3D_QPU_A_VFLB,
        V3D_QPU_A_VFLNB,
        V3D_QPU_A_FXCD,
        V3D_QPU_A_XCD,
        V3D_QPU_A_FYCD,
        V3D_QPU_A_YCD,
        V3D_QPU_A_MSF,
        V3D_QPU_A_REVF,
        V3D_QPU_A_VDWWT,
        V3D_QPU_A_IID,
        V3D_QPU_A_SAMPID,
        V3D_QPU_A_BARRIERID,
        V3D_QPU_A_TMUWT,
        V3D_QPU_A_VPMSETUP,
        V3D_QPU_A_VPMWT,
        V3D_QPU_A_FLAFIRST,
        V3D_QPU_A_FLNAFIRST,
        V3D_QPU_A_LDVPMV_IN,
        V3D_QPU_A_LDVPMV_OUT,
        V3D_QPU_A_LDVPMD_IN,
        V3D_QPU_A_LDVPMD_OUT,
        V3D_QPU_A_LDVPMP,
        V3D_QPU_A_RSQRT,
        V3D_QPU_A_EXP,
        V3D_QPU_A_LOG,
        V3D_QPU_A_SIN,
        V3D_QPU_A_RSQRT2,
        V3D_QPU_A_LDVPMG_IN,
        V3D_QPU_A_LDVPMG_OUT,
        V3D_QPU_A_FCMP,
        V3D_QPU_A_VFMAX,
        V3D_QPU_A_FROUND,
        V3D_QPU_A_FTOIN,
        V3D_QPU_A_FTRUNC,
        V3D_QPU_A_FTOIZ,
        V3D_QPU_A_FFLOOR,
        V3D_QPU_A_FTOUZ,
        V3D_QPU_A_FCEIL,
        V3D_QPU_A_FTOC,
        V3D_QPU_A_FDX,
        V3D_QPU_A_FDY,
        V3D_QPU_A_STVPMV,
        V3D_QPU_A_STVPMD,
        V3D_QPU_A_STVPMP,
        V3D_QPU_A_ITOF,
        V3D_QPU_A_CLZ,
        V3D_QPU_A_UTOF,

        /* V3D 7.x */
        V3D_QPU_A_FMOV,
        V3D_QPU_A_MOV,
        V3D_QPU_A_VPACK,
        V3D_QPU_A_V8PACK,
        V3D_QPU_A_V10PACK,
        V3D_QPU_A_V11FPACK,
};

enum v3d_qpu_mul_op {
        V3D_QPU_M_ADD,
        V3D_QPU_M_SUB,
        V3D_QPU_M_UMUL24,
        V3D_QPU_M_VFMUL,
        V3D_QPU_M_SMUL24,
        V3D_QPU_M_MULTOP,
        V3D_QPU_M_FMOV,
        V3D_QPU_M_MOV,
        V3D_QPU_M_NOP,
        V3D_QPU_M_FMUL,

        /* V3D 7.x */
        V3D_QPU_M_FTOUNORM16,
        V3D_QPU_M_FTOSNORM16,
        V3D_QPU_M_VFTOUNORM8,
        V3D_QPU_M_VFTOSNORM8,
        V3D_QPU_M_VFTOUNORM10LO,
        V3D_QPU_M_VFTOUNORM10HI,
};

enum v3d_qpu_output_pack {
        V3D_QPU_PACK_NONE,
        /**
         * Convert to 16-bit float, put in low 16 bits of destination leaving
         * high unmodified.
         */
        V3D_QPU_PACK_L,
        /**
         * Convert to 16-bit float, put in high 16 bits of destination leaving
         * low unmodified.
         */
        V3D_QPU_PACK_H,
};

enum v3d_qpu_input_unpack {
        /**
         * No-op input unpacking.  Note that this enum's value doesn't match
         * the packed QPU instruction value of the field (we use 0 so that the
         * default on new instruction creation is no-op).
         */
        V3D_QPU_UNPACK_NONE,
        /** Absolute value.  Only available for some operations. */
        V3D_QPU_UNPACK_ABS,
        /** Convert low 16 bits from 16-bit float to 32-bit float. */
        V3D_QPU_UNPACK_L,
        /** Convert high 16 bits from 16-bit float to 32-bit float. */
        V3D_QPU_UNPACK_H,

        /** Convert to 16f and replicate it to the high bits. */
        V3D_QPU_UNPACK_REPLICATE_32F_16,

        /** Replicate low 16 bits to high */
        V3D_QPU_UNPACK_REPLICATE_L_16,

        /** Replicate high 16 bits to low */
        V3D_QPU_UNPACK_REPLICATE_H_16,

        /** Swap high and low 16 bits */
        V3D_QPU_UNPACK_SWAP_16,

        /** Convert low 16 bits from 16-bit integer to unsigned 32-bit int */
        V3D_QPU_UNPACK_UL,
        /** Convert high 16 bits from 16-bit integer to unsigned 32-bit int */
        V3D_QPU_UNPACK_UH,
        /** Convert low 16 bits from 16-bit integer to signed 32-bit int */
        V3D_QPU_UNPACK_IL,
        /** Convert high 16 bits from 16-bit integer to signed 32-bit int */
        V3D_QPU_UNPACK_IH,
};

enum v3d_qpu_mux {
        V3D_QPU_MUX_R0,
        V3D_QPU_MUX_R1,
        V3D_QPU_MUX_R2,
        V3D_QPU_MUX_R3,
        V3D_QPU_MUX_R4,
        V3D_QPU_MUX_R5,
        V3D_QPU_MUX_A,
        V3D_QPU_MUX_B,
};

struct v3d_qpu_input {
        union {
                enum v3d_qpu_mux mux; /* V3D 4.x */
                uint8_t raddr; /* V3D 7.x */
        };
        enum v3d_qpu_input_unpack unpack;
};

struct v3d_qpu_alu_instr {
        struct {
                enum v3d_qpu_add_op op;
                struct v3d_qpu_input a, b;
                uint8_t waddr;
                bool magic_write;
                enum v3d_qpu_output_pack output_pack;
        } add;

        struct {
                enum v3d_qpu_mul_op op;
                struct v3d_qpu_input a, b;
                uint8_t waddr;
                bool magic_write;
                enum v3d_qpu_output_pack output_pack;
        } mul;
};

enum v3d_qpu_branch_cond {
        V3D_QPU_BRANCH_COND_ALWAYS,
        V3D_QPU_BRANCH_COND_A0,
        V3D_QPU_BRANCH_COND_NA0,
        V3D_QPU_BRANCH_COND_ALLA,
        V3D_QPU_BRANCH_COND_ANYNA,
        V3D_QPU_BRANCH_COND_ANYA,
        V3D_QPU_BRANCH_COND_ALLNA,
};

enum v3d_qpu_msfign {
        /** Ignore multisample flags when determining branch condition. */
        V3D_QPU_MSFIGN_NONE,
        /**
         * If no multisample flags are set in the lane (a pixel in the FS, a
         * vertex in the VS), ignore the lane's condition when computing the
         * branch condition.
         */
        V3D_QPU_MSFIGN_P,
        /**
         * If no multisample flags are set in a 2x2 quad in the FS, ignore the
         * quad's a/b conditions.
         */
        V3D_QPU_MSFIGN_Q,
};

enum v3d_qpu_branch_dest {
        V3D_QPU_BRANCH_DEST_ABS,
        V3D_QPU_BRANCH_DEST_REL,
        V3D_QPU_BRANCH_DEST_LINK_REG,
        V3D_QPU_BRANCH_DEST_REGFILE,
};

struct v3d_qpu_branch_instr {
        enum v3d_qpu_branch_cond cond;
        enum v3d_qpu_msfign msfign;

        /** Selects how to compute the new IP if the branch is taken. */
        enum v3d_qpu_branch_dest bdi;

        /**
         * Selects how to compute the new uniforms pointer if the branch is
         * taken.  (ABS/REL implicitly load a uniform and use that)
         */
        enum v3d_qpu_branch_dest bdu;

        /**
         * If set, then udest determines how the uniform stream will branch,
         * otherwise the uniform stream is left as is.
         */
        bool ub;

        uint8_t raddr_a;

        uint32_t offset;
};

enum v3d_qpu_instr_type {
        V3D_QPU_INSTR_TYPE_ALU,
        V3D_QPU_INSTR_TYPE_BRANCH,
};

struct v3d_qpu_instr {
        enum v3d_qpu_instr_type type;

        struct v3d_qpu_sig sig;
        uint8_t sig_addr;
        bool sig_magic; /* If the signal writes to a magic address */
        uint8_t raddr_a; /* V3D 4.x */
        uint8_t raddr_b; /* V3D 4.x (holds packed small immediate in 7.x too) */
        struct v3d_qpu_flags flags;

        union {
                struct v3d_qpu_alu_instr alu;
                struct v3d_qpu_branch_instr branch;
        };
};

const char *v3d_qpu_magic_waddr_name(const struct v3d_device_info *devinfo,
                                     enum v3d_qpu_waddr waddr);
const char *v3d_qpu_add_op_name(enum v3d_qpu_add_op op);
const char *v3d_qpu_mul_op_name(enum v3d_qpu_mul_op op);
const char *v3d_qpu_cond_name(enum v3d_qpu_cond cond);
const char *v3d_qpu_pf_name(enum v3d_qpu_pf pf);
const char *v3d_qpu_uf_name(enum v3d_qpu_uf uf);
const char *v3d_qpu_pack_name(enum v3d_qpu_output_pack pack);
const char *v3d_qpu_unpack_name(enum v3d_qpu_input_unpack unpack);
const char *v3d_qpu_branch_cond_name(enum v3d_qpu_branch_cond cond);
const char *v3d_qpu_msfign_name(enum v3d_qpu_msfign msfign);

enum v3d_qpu_cond v3d_qpu_cond_invert(enum v3d_qpu_cond cond) ATTRIBUTE_CONST;

bool v3d_qpu_add_op_has_dst(enum v3d_qpu_add_op op);
bool v3d_qpu_mul_op_has_dst(enum v3d_qpu_mul_op op);
int v3d_qpu_add_op_num_src(enum v3d_qpu_add_op op);
int v3d_qpu_mul_op_num_src(enum v3d_qpu_mul_op op);

bool v3d_qpu_sig_pack(const struct v3d_device_info *devinfo,
                      const struct v3d_qpu_sig *sig,
                      uint32_t *packed_sig);
bool v3d_qpu_sig_unpack(const struct v3d_device_info *devinfo,
                        uint32_t packed_sig,
                        struct v3d_qpu_sig *sig);

bool
v3d_qpu_flags_pack(const struct v3d_device_info *devinfo,
                   const struct v3d_qpu_flags *cond,
                   uint32_t *packed_cond);
bool
v3d_qpu_flags_unpack(const struct v3d_device_info *devinfo,
                     uint32_t packed_cond,
                     struct v3d_qpu_flags *cond);

bool
v3d_qpu_small_imm_pack(const struct v3d_device_info *devinfo,
                       uint32_t value,
                       uint32_t *packed_small_immediate);

bool
v3d_qpu_small_imm_unpack(const struct v3d_device_info *devinfo,
                         uint32_t packed_small_immediate,
                         uint32_t *small_immediate);

bool
v3d_qpu_instr_pack(const struct v3d_device_info *devinfo,
                   const struct v3d_qpu_instr *instr,
                   uint64_t *packed_instr);
bool
v3d_qpu_instr_unpack(const struct v3d_device_info *devinfo,
                     uint64_t packed_instr,
                     struct v3d_qpu_instr *instr);

bool v3d_qpu_magic_waddr_is_sfu(enum v3d_qpu_waddr waddr) ATTRIBUTE_CONST;
bool v3d_qpu_magic_waddr_is_tmu(const struct v3d_device_info *devinfo,
                                enum v3d_qpu_waddr waddr) ATTRIBUTE_CONST;
bool v3d_qpu_magic_waddr_is_tlb(enum v3d_qpu_waddr waddr) ATTRIBUTE_CONST;
bool v3d_qpu_magic_waddr_is_vpm(enum v3d_qpu_waddr waddr) ATTRIBUTE_CONST;
bool v3d_qpu_magic_waddr_is_tsy(enum v3d_qpu_waddr waddr) ATTRIBUTE_CONST;
bool v3d_qpu_magic_waddr_loads_unif(enum v3d_qpu_waddr waddr) ATTRIBUTE_CONST;
bool v3d_qpu_reads_tlb(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_writes_tlb(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_uses_tlb(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_instr_is_sfu(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_instr_is_legacy_sfu(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_uses_sfu(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_writes_tmu(const struct v3d_device_info *devinfo,
                        const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_writes_tmu_not_tmuc(const struct v3d_device_info *devinfo,
                                 const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_writes_r3(const struct v3d_device_info *devinfo,
                       const struct v3d_qpu_instr *instr) ATTRIBUTE_CONST;
bool v3d_qpu_writes_r4(const struct v3d_device_info *devinfo,
                       const struct v3d_qpu_instr *instr) ATTRIBUTE_CONST;
bool v3d_qpu_writes_r5(const struct v3d_device_info *devinfo,
                       const struct v3d_qpu_instr *instr) ATTRIBUTE_CONST;
bool v3d_qpu_writes_rf0_implicitly(const struct v3d_device_info *devinfo,
                                   const struct v3d_qpu_instr *instr) ATTRIBUTE_CONST;
bool v3d_qpu_writes_accum(const struct v3d_device_info *devinfo,
                          const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_waits_on_tmu(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_uses_mux(const struct v3d_qpu_instr *inst, enum v3d_qpu_mux mux);
bool v3d_qpu_uses_vpm(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_waits_vpm(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_reads_vpm(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_writes_vpm(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_reads_or_writes_vpm(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_reads_flags(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_writes_flags(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_writes_unifa(const struct v3d_device_info *devinfo,
                          const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_sig_writes_address(const struct v3d_device_info *devinfo,
                                const struct v3d_qpu_sig *sig) ATTRIBUTE_CONST;
bool v3d_qpu_unpacks_f32(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;
bool v3d_qpu_unpacks_f16(const struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;

bool v3d_qpu_is_nop(struct v3d_qpu_instr *inst) ATTRIBUTE_CONST;

bool v3d71_qpu_reads_raddr(const struct v3d_qpu_instr *inst, uint8_t raddr);
bool v3d71_qpu_writes_waddr_explicitly(const struct v3d_device_info *devinfo,
                                       const struct v3d_qpu_instr *inst,
                                       uint8_t waddr);
#endif
