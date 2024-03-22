/* Copyright (c) 2018-2019 Alyssa Rosenzweig (alyssa@rosenzweig.io)
 * Copyright (C) 2019-2020 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "midgard.h"

/* Include the definitions of the macros and such */

#define MIDGARD_OPS_TABLE
#include "helpers.h"
#undef MIDGARD_OPS_TABLE

#include "midgard_ops.h"

/*
 * Table of mapping opcodes to accompanying properties. This is used for both
 * the disassembler and the compiler. It is placed in a .c file like this to
 * avoid duplications in the binary.
 */

/* clang-format off */
struct mir_op_props alu_opcode_props[256] = {
   [midgard_alu_op_fadd]            = {"FADD", UNITS_ADD | OP_COMMUTES},
   [midgard_alu_op_fadd_rtz]        = {"FADD.rtz", UNITS_ADD | OP_COMMUTES},
   [midgard_alu_op_fadd_rtn]        = {"FADD.rtn", UNITS_ADD | OP_COMMUTES},
   [midgard_alu_op_fadd_rtp]        = {"FADD.rtp", UNITS_ADD | OP_COMMUTES},
   [midgard_alu_op_fmul]            = {"FMUL", UNITS_MUL | UNIT_VLUT | OP_COMMUTES},
   [midgard_alu_op_fmul_rtz]        = {"FMUL.rtz", UNITS_MUL | UNIT_VLUT | OP_COMMUTES},
   [midgard_alu_op_fmul_rtn]        = {"FMUL.rtn", UNITS_MUL | UNIT_VLUT | OP_COMMUTES},
   [midgard_alu_op_fmul_rtp]        = {"FMUL.rtp", UNITS_MUL | UNIT_VLUT | OP_COMMUTES},
   [midgard_alu_op_fmin]            = {"FMIN", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_fmin_nan]        = {"FMIN.nan", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_fabsmin]         = {"FABSMIN", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_fabsmin_nan]     = {"FABSMIN.nan", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_fmax]            = {"FMAX", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_fmax_nan]        = {"FMAX.nan", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_fabsmax]         = {"FABSMAX", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_fabsmax_nan]     = {"FABSMAX.nan", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_imin]            = {"MIN", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_imax]            = {"MAX", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_umin]            = {"MIN", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_umax]            = {"MAX", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_iavg]            = {"AVG.rtz", UNITS_ADD | OP_COMMUTES},
   [midgard_alu_op_uavg]            = {"AVG.rtz", UNITS_ADD | OP_COMMUTES},
   [midgard_alu_op_iravg]           = {"AVG.round", UNITS_ADD | OP_COMMUTES},
   [midgard_alu_op_uravg]           = {"AVG.round", UNITS_ADD | OP_COMMUTES},

   [midgard_alu_op_fmov]            = {"FMOV", UNITS_ALL | QUIRK_FLIPPED_R24},
   [midgard_alu_op_fmov_rtz]        = {"FMOV.rtz", UNITS_ALL | QUIRK_FLIPPED_R24},
   [midgard_alu_op_fmov_rtn]        = {"FMOV.rtn", UNITS_ALL | QUIRK_FLIPPED_R24},
   [midgard_alu_op_fmov_rtp]        = {"FMOV.rtp", UNITS_ALL | QUIRK_FLIPPED_R24},
   [midgard_alu_op_froundaway]      = {"FROUNDAWAY", UNITS_ADD},
   [midgard_alu_op_froundeven]      = {"FROUNDEVEN", UNITS_ADD},
   [midgard_alu_op_ftrunc]          = {"FTRUNC", UNITS_ADD},
   [midgard_alu_op_ffloor]          = {"FFLOOR", UNITS_ADD},
   [midgard_alu_op_fceil]           = {"FCEIL", UNITS_ADD},

   /* Multiplies the X/Y components of the first arg and adds the second
    * arg. Like other LUTs, it must be scalarized. */
   [midgard_alu_op_ffma]            = {"FMA", UNIT_VLUT},
   [midgard_alu_op_ffma_rtz]        = {"FMA.rtz", UNIT_VLUT},
   [midgard_alu_op_ffma_rtn]        = {"FMA.rtn", UNIT_VLUT},
   [midgard_alu_op_ffma_rtp]        = {"FMA.rtp", UNIT_VLUT},

   /* Though they output a scalar, they need to run on a vector unit
    * since they process vectors */
   [midgard_alu_op_fdot3]           = {"FDOT3", UNIT_VMUL | OP_CHANNEL_COUNT(3) | OP_COMMUTES},
   [midgard_alu_op_fdot3r]          = {"FDOT3R", UNIT_VMUL | OP_CHANNEL_COUNT(3) | OP_COMMUTES},
   [midgard_alu_op_fdot4]           = {"FDOT4", UNIT_VMUL | OP_CHANNEL_COUNT(4) | OP_COMMUTES},

   /* Incredibly, iadd can run on vmul, etc */
   [midgard_alu_op_iadd]            = {"ADD", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_ishladd]         = {"ADD", UNITS_MUL},
   [midgard_alu_op_iaddsat]         = {"ADDSAT", UNITS_ADD | OP_COMMUTES},
   [midgard_alu_op_uaddsat]         = {"ADDSAT", UNITS_ADD | OP_COMMUTES},
   [midgard_alu_op_uabsdiff]        = {"ABSDIFF", UNITS_ADD},
   [midgard_alu_op_iabsdiff]        = {"ABSDIFF", UNITS_ADD},
   [midgard_alu_op_ichoose]         = {"CHOOSE", UNITS_ADD},
   [midgard_alu_op_isub]            = {"SUB", UNITS_MOST},
   [midgard_alu_op_ishlsub]         = {"SUB", UNITS_MUL},
   [midgard_alu_op_isubsat]         = {"SUBSAT", UNITS_ADD},
   [midgard_alu_op_usubsat]         = {"SUBSAT", UNITS_ADD},
   [midgard_alu_op_imul]            = {"MUL", UNITS_MUL | OP_COMMUTES},
   [midgard_alu_op_iwmul]           = {"WMUL.s", UNIT_VMUL | OP_COMMUTES},
   [midgard_alu_op_uwmul]           = {"WMUL.u", UNIT_VMUL | OP_COMMUTES},
   [midgard_alu_op_iuwmul]          = {"WMUL.su", UNIT_VMUL | OP_COMMUTES},
   [midgard_alu_op_imov]            = {"MOV", UNITS_ALL | QUIRK_FLIPPED_R24},

   /* For vector comparisons, use ball etc */
   [midgard_alu_op_feq]             = {"FCMP.eq", UNITS_MOST | OP_TYPE_CONVERT | OP_COMMUTES},
   [midgard_alu_op_fne]             = {"FCMP.ne", UNITS_MOST | OP_TYPE_CONVERT | OP_COMMUTES},
   [midgard_alu_op_fle]             = {"FCMP.le", UNITS_MOST | OP_TYPE_CONVERT},
   [midgard_alu_op_flt]             = {"FCMP.lt", UNITS_MOST | OP_TYPE_CONVERT},
   [midgard_alu_op_ieq]             = {"CMP.eq", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_ine]             = {"CMP.ne", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_ilt]             = {"CMP.lt", UNITS_MOST},
   [midgard_alu_op_ile]             = {"CMP.le", UNITS_MOST},
   [midgard_alu_op_ult]             = {"CMP.lt", UNITS_MOST},
   [midgard_alu_op_ule]             = {"CMP.le", UNITS_MOST},

   /* CSEL (MUX) runs in the second pipeline stage, sourcing its selector
    * the previous scalar or vector stage as indicated in the opcode. It
    * muxes individual bits based on the selector, implementing both
    * bit_select and bcsel (the latter because CMP returns 0/~0 booleans).
    *
    * It is legal to schedule (F)CSEL.vector to the scalar unit, but it
    * isn't usually useful. Our scheduler does not handle that case, so
    * don't try to and fall over.
    */
   [midgard_alu_op_icsel]           = {"CSEL.scalar", UNIT_VADD | UNIT_SMUL},
   [midgard_alu_op_icsel_v]         = {"CSEL.vector", UNIT_VADD},
   [midgard_alu_op_fcsel_v]         = {"FCSEL.vector", UNIT_VADD},
   [midgard_alu_op_fcsel]           = {"FCSEL.scalar", UNIT_VADD | UNIT_SMUL},

   [midgard_alu_op_frcp]            = {"FRCP", UNIT_VLUT},
   [midgard_alu_op_frsqrt]          = {"FRSQRT", UNIT_VLUT},
   [midgard_alu_op_fsqrt]           = {"FSQRT", UNIT_VLUT},
   [midgard_alu_op_fpow_pt1]        = {"FPOW_PT1", UNIT_VLUT},
   [midgard_alu_op_fpown_pt1]       = {"FPOWN_PT1", UNIT_VLUT},
   [midgard_alu_op_fpowr_pt1]       = {"FPOWR_PT1", UNIT_VLUT},
   [midgard_alu_op_fexp2]           = {"FEXP2", UNIT_VLUT},
   [midgard_alu_op_flog2]           = {"FLOG2", UNIT_VLUT},

   [midgard_alu_op_f2i_rte]         = {"F2I", UNITS_ADD | OP_TYPE_CONVERT | MIDGARD_ROUNDS},
   [midgard_alu_op_f2i_rtz]         = {"F2I.rtz", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_f2i_rtn]         = {"F2I.rtn", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_f2i_rtp]         = {"F2I.rtp", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_f2u_rte]         = {"F2U", UNITS_ADD | OP_TYPE_CONVERT | MIDGARD_ROUNDS},
   [midgard_alu_op_f2u_rtz]         = {"F2U.rtz", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_f2u_rtn]         = {"F2U.rtn", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_f2u_rtp]         = {"F2U.rtp", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_i2f_rte]         = {"I2F", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_i2f_rtz]         = {"I2F.rtz", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_i2f_rtn]         = {"I2F.rtn", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_i2f_rtp]         = {"I2F.rtp", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_u2f_rte]         = {"U2F", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_u2f_rtz]         = {"U2F.rtz", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_u2f_rtn]         = {"U2F.rtn", UNITS_ADD | OP_TYPE_CONVERT},
   [midgard_alu_op_u2f_rtp]         = {"U2F.rtp", UNITS_ADD | OP_TYPE_CONVERT},

   [midgard_alu_op_fsinpi]          = {"FSINPI", UNIT_VLUT},
   [midgard_alu_op_fcospi]          = {"FCOSPI", UNIT_VLUT},

   [midgard_alu_op_iand]            = {"AND", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_iandnot]         = {"ANDNOT", UNITS_MOST},

   [midgard_alu_op_ior]             = {"OR", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_iornot]          = {"ORNOT", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_inor]            = {"NOR", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_ixor]            = {"XOR", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_inxor]           = {"NXOR", UNITS_MOST | OP_COMMUTES},
   [midgard_alu_op_iclz]            = {"CLZ", UNITS_ADD},
   [midgard_alu_op_ipopcnt]         = {"POPCNT", UNIT_VADD},
   [midgard_alu_op_inand]           = {"NAND", UNITS_MOST},
   [midgard_alu_op_ishl]            = {"SHL", UNITS_ADD},
   [midgard_alu_op_ishlsat]         = {"SHL.sat", UNITS_ADD},
   [midgard_alu_op_ushlsat]         = {"SHL.sat", UNITS_ADD},
   [midgard_alu_op_iasr]            = {"ASR", UNITS_ADD},
   [midgard_alu_op_ilsr]            = {"LSR", UNITS_ADD},

   [midgard_alu_op_fball_eq]        = {"FCMP.all.eq",  UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES | OP_TYPE_CONVERT},
   [midgard_alu_op_fball_neq]       = {"FCMP.all.ne", UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES | OP_TYPE_CONVERT},
   [midgard_alu_op_fball_lt]        = {"FCMP.all.lt",  UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES | OP_TYPE_CONVERT},
   [midgard_alu_op_fball_lte]       = {"FCMP.all.le", UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES | OP_TYPE_CONVERT},

   [midgard_alu_op_fbany_eq]        = {"FCMP.any.eq",  UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES | OP_TYPE_CONVERT},
   [midgard_alu_op_fbany_neq]       = {"FCMP.any.ne", UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES | OP_TYPE_CONVERT},
   [midgard_alu_op_fbany_lt]        = {"FCMP.any.lt",  UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES | OP_TYPE_CONVERT},
   [midgard_alu_op_fbany_lte]       = {"FCMP.any.le", UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES | OP_TYPE_CONVERT},

   [midgard_alu_op_iball_eq]        = {"CMP.all.eq",  UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES},
   [midgard_alu_op_iball_neq]       = {"CMP.all.ne", UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES},
   [midgard_alu_op_iball_lt]        = {"CMP.all.lt",  UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES},
   [midgard_alu_op_iball_lte]       = {"CMP.all.le", UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES},
   [midgard_alu_op_uball_lt]        = {"CMP.all.lt",  UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES},
   [midgard_alu_op_uball_lte]       = {"CMP.all.le", UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES},

   [midgard_alu_op_ibany_eq]        = {"CMP.any.eq",  UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES},
   [midgard_alu_op_ibany_neq]       = {"CMP.any.ne", UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES},
   [midgard_alu_op_ibany_lt]        = {"CMP.any.lt",  UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES},
   [midgard_alu_op_ibany_lte]       = {"CMP.any.le", UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES},
   [midgard_alu_op_ubany_lt]        = {"CMP.any.lt",  UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES},
   [midgard_alu_op_ubany_lte]       = {"CMP.any.le", UNITS_VECTOR | OP_CHANNEL_COUNT(4) | OP_COMMUTES},

   [midgard_alu_op_fatan2_pt1]      = {"FATAN2_PT1", UNIT_VLUT},
   [midgard_alu_op_fatan2_pt2]      = {"FATAN2_PT2", UNIT_VLUT},

   /* Haven't seen in a while */
   [midgard_alu_op_freduce]         = {"FREDUCE", 0},
};

/* Define shorthands */

#define M8  midgard_reg_mode_8
#define M16 midgard_reg_mode_16
#define M32 midgard_reg_mode_32
#define M64 midgard_reg_mode_64

struct mir_ldst_op_props load_store_opcode_props[256] = {
   [midgard_op_unpack_colour_f32] = {"UNPACK.f32", M32},
   [midgard_op_unpack_colour_f16] = {"UNPACK.f16", M32},
   [midgard_op_unpack_colour_u32] = {"UNPACK.u32", M32},
   [midgard_op_unpack_colour_s32] = {"UNPACK.s32", M32},
   [midgard_op_pack_colour_f32] = {"PACK.f32", M32},
   [midgard_op_pack_colour_f16] = {"PACK.f16", M32},
   [midgard_op_pack_colour_u32] = {"PACK.u32", M32},
   [midgard_op_pack_colour_s32] = {"PACK.s32", M32},
   [midgard_op_lea] = {"LEA", M32 | LDST_ADDRESS },
   [midgard_op_lea_image] = {"LEA_IMAGE", M32 | LDST_ATTRIB },
   [midgard_op_ld_cubemap_coords] = {"CUBEMAP", M32},
   [midgard_op_ldst_mov] = {"LDST_MOV", M32},
   [midgard_op_ldst_perspective_div_y] = {"LDST_PERSPECTIVE_DIV_Y", M32},
   [midgard_op_ldst_perspective_div_z] = {"LDST_PERSPECTIVE_DIV_Z", M32},
   [midgard_op_ldst_perspective_div_w] = {"LDST_PERSPECTIVE_DIV_W", M32},

   [midgard_op_atomic_add]     = {"AADD.32",    M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_and]     = {"AAND.32",    M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_or]      = {"AOR.32",     M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_xor]     = {"AXOR.32",    M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_imin]    = {"AMIN.s32",   M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_umin]    = {"AMIN.u32",   M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_imax]    = {"AMAX.s32",   M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_umax]    = {"AMAX.u32",   M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_xchg]    = {"XCHG.32",    M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_cmpxchg] = {"CMPXCHG.32", M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},

   [midgard_op_atomic_add64]     = {"AADD.64",    M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_and64]     = {"AAND.64",    M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_or64]      = {"AOR.64",     M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_xor64]     = {"AXOR.64",    M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_imin64]    = {"AMIN.s64",   M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_umin64]    = {"AMIN.u64",   M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_imax64]    = {"AMAX.s64",   M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_umax64]    = {"AMAX.u64",   M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_xchg64]    = {"XCHG.64",    M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_cmpxchg64] = {"CMPXCHG.64", M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},

   [midgard_op_atomic_add_be]     = {"AADD.32.be",    M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_and_be]     = {"AAND.32.be",    M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_or_be]      = {"AOR.32.be",     M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_xor_be]     = {"AXOR.32.be",    M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_imin_be]    = {"AMIN.s32.be",   M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_umin_be]    = {"AMIN.u32.be",   M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_imax_be]    = {"AMAX.s32.be",   M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_umax_be]    = {"AMAX.u32.be",   M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_xchg_be]    = {"XCHG.32.be",    M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_cmpxchg_be] = {"CMPXCHG.32.be", M32 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},

   [midgard_op_atomic_add64]     = {"AADD.64.be",    M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_and64]     = {"AAND.64.be",    M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_or64]      = {"AOR.64.be",     M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_xor64]     = {"AXOR.64.be",    M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_imin64]    = {"AMIN.s64.be",   M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_umin64]    = {"AMIN.u64.be",   M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_imax64]    = {"AMAX.s64.be",   M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_umax64]    = {"AMAX.u64.be",   M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_xchg64]    = {"XCHG.64.be",    M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},
   [midgard_op_atomic_cmpxchg64] = {"CMPXCHG.64.be", M64 | LDST_SIDE_FX | LDST_ADDRESS | LDST_ATOMIC},

   [midgard_op_ld_u8]         = {"LD.u8",         M32 | LDST_ADDRESS},
   [midgard_op_ld_i8]         = {"LD.s8",         M32 | LDST_ADDRESS},
   [midgard_op_ld_u16]        = {"LD.u16",        M32 | LDST_ADDRESS},
   [midgard_op_ld_i16]        = {"LD.s16",        M32 | LDST_ADDRESS},
   [midgard_op_ld_u16_be]     = {"LD.u16.be",     M32 | LDST_ADDRESS},
   [midgard_op_ld_i16_be]     = {"LD.s16.be",     M32 | LDST_ADDRESS},
   [midgard_op_ld_32]         = {"LD.32",         M32 | LDST_ADDRESS},
   [midgard_op_ld_32_bswap2]  = {"LD.32.bswap2",  M32 | LDST_ADDRESS},
   [midgard_op_ld_32_bswap4]  = {"LD.32.bswap4",  M32 | LDST_ADDRESS},
   [midgard_op_ld_64]         = {"LD.64",         M32 | LDST_ADDRESS},
   [midgard_op_ld_64_bswap2]  = {"LD.64.bswap2",  M32 | LDST_ADDRESS},
   [midgard_op_ld_64_bswap4]  = {"LD.64.bswap4",  M32 | LDST_ADDRESS},
   [midgard_op_ld_64_bswap8]  = {"LD.64.bswap8",  M32 | LDST_ADDRESS},
   [midgard_op_ld_128]        = {"LD.128",        M32 | LDST_ADDRESS},
   [midgard_op_ld_128_bswap2] = {"LD.128.bswap2", M32 | LDST_ADDRESS},
   [midgard_op_ld_128_bswap4] = {"LD.128.bswap4", M32 | LDST_ADDRESS},
   [midgard_op_ld_128_bswap8] = {"LD.128.bswap8", M32 | LDST_ADDRESS},

   [midgard_op_ld_attr_32]  = {"LD_ATTR.f32", M32 | LDST_ATTRIB},
   [midgard_op_ld_attr_32i] = {"LD_ATTR.s32", M32 | LDST_ATTRIB},
   [midgard_op_ld_attr_32u] = {"LD_ATTR.u32", M32 | LDST_ATTRIB},
   [midgard_op_ld_attr_16]  = {"LD_ATTR.f16", M32 | LDST_ATTRIB},

   [midgard_op_ld_vary_32]  = {"LD_VARY.f32", M32 | LDST_ATTRIB},
   [midgard_op_ld_vary_16]  = {"LD_VARY.f16", M32 | LDST_ATTRIB},
   [midgard_op_ld_vary_32i] = {"LD_VARY.s32", M32 | LDST_ATTRIB},
   [midgard_op_ld_vary_32u] = {"LD_VARY.u32", M32 | LDST_ATTRIB},

   [midgard_op_ld_special_32f] = {"LD_SPECIAL.f32", M32 | LDST_SPECIAL_MASK},
   [midgard_op_ld_special_16f] = {"LD_SPECIAL.f16", M16 | LDST_SPECIAL_MASK},
   [midgard_op_ld_special_32u] = {"LD_SPECIAL.u32", M32},
   [midgard_op_ld_special_32i] = {"LD_SPECIAL.s32", M32},

   [midgard_op_ld_tilebuffer_32f] = {"LD_TILEBUFFER.f32", M32},
   [midgard_op_ld_tilebuffer_16f] = {"LD_TILEBUFFER.f16", M16},
   [midgard_op_ld_tilebuffer_raw] = {"LD_TILEBUFFER.raw", M32},

   [midgard_op_ld_ubo_u8]         = {"LD_UBO.u8",         M32},
   [midgard_op_ld_ubo_i8]         = {"LD_UBO.s8",         M32},
   [midgard_op_ld_ubo_u16]        = {"LD_UBO.u16",        M16},
   [midgard_op_ld_ubo_i16]        = {"LD_UBO.s16",        M16},
   [midgard_op_ld_ubo_u16_be]     = {"LD_UBO.u16.be",     M16},
   [midgard_op_ld_ubo_i16_be]     = {"LD_UBO.s16.be",     M16},
   [midgard_op_ld_ubo_32]         = {"LD_UBO.32",         M32},
   [midgard_op_ld_ubo_32_bswap2]  = {"LD_UBO.32.bswap2",  M32},
   [midgard_op_ld_ubo_32_bswap4]  = {"LD_UBO.32.bswap4",  M32},
   [midgard_op_ld_ubo_64]         = {"LD_UBO.64",         M32},
   [midgard_op_ld_ubo_64_bswap2]  = {"LD_UBO.64.bswap2",  M32},
   [midgard_op_ld_ubo_64_bswap4]  = {"LD_UBO.64.bswap4",  M32},
   [midgard_op_ld_ubo_64_bswap8]  = {"LD_UBO.64.bswap8",  M32},
   [midgard_op_ld_ubo_128]        = {"LD_UBO.128",        M32},
   [midgard_op_ld_ubo_128_bswap2] = {"LD_UBO.128.bswap2", M32},
   [midgard_op_ld_ubo_128_bswap4] = {"LD_UBO.128.bswap4", M32},
   [midgard_op_ld_ubo_128_bswap8] = {"LD_UBO.128.bswap8", M32},

   [midgard_op_ld_image_32f] = {"LD_IMAGE.f32", M32 | LDST_ATTRIB},
   [midgard_op_ld_image_16f] = {"LD_IMAGE.f16", M16 | LDST_ATTRIB},
   [midgard_op_ld_image_32i] = {"LD_IMAGE.s32", M32 | LDST_ATTRIB},
   [midgard_op_ld_image_32u] = {"LD_IMAGE.u32", M32 | LDST_ATTRIB},

   [midgard_op_st_u8]         = {"ST.u8",         M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_i8]         = {"ST.s8",         M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_u16]        = {"ST.u16",        M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_i16]        = {"ST.s16",        M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_u16_be]     = {"ST.u16.be",     M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_i16_be]     = {"ST.s16.be",     M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_32]         = {"ST.32",         M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_32_bswap2]  = {"ST.32.bswap2",  M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_32_bswap4]  = {"ST.32.bswap4",  M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_64]         = {"ST.64",         M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_64_bswap2]  = {"ST.64.bswap2",  M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_64_bswap4]  = {"ST.64.bswap4",  M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_64_bswap8]  = {"ST.64.bswap8",  M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_128]        = {"ST.128",        M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_128_bswap2] = {"ST.128.bswap2", M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_128_bswap4] = {"ST.128.bswap4", M32 | LDST_STORE | LDST_ADDRESS},
   [midgard_op_st_128_bswap8] = {"ST.128.bswap8", M32 | LDST_STORE | LDST_ADDRESS},

   [midgard_op_st_vary_32]  = {"ST_VARY.f32", M32 | LDST_STORE | LDST_ATTRIB},
   [midgard_op_st_vary_32i] = {"ST_VARY.s32", M32 | LDST_STORE | LDST_ATTRIB},
   [midgard_op_st_vary_32u] = {"ST_VARY.u32", M32 | LDST_STORE | LDST_ATTRIB},
   [midgard_op_st_vary_16]  = {"ST_VARY.f16", M16 | LDST_STORE | LDST_ATTRIB},

   [midgard_op_st_image_32f] = {"ST_IMAGE.f32", M32 | LDST_STORE | LDST_ATTRIB},
   [midgard_op_st_image_16f] = {"ST_IMAGE.f16", M16 | LDST_STORE | LDST_ATTRIB},
   [midgard_op_st_image_32i] = {"ST_IMAGE.u32", M32 | LDST_STORE | LDST_ATTRIB},
   [midgard_op_st_image_32u] = {"ST_IMAGE.s32", M32 | LDST_STORE | LDST_ATTRIB},

   [midgard_op_st_special_32f] = {"ST_SPECIAL.f32", M32},
   [midgard_op_st_special_16f] = {"ST_SPECIAL.f16", M16},
   [midgard_op_st_special_32u] = {"ST_SPECIAL.u32", M32},
   [midgard_op_st_special_32i] = {"ST_SPECIAL.s32", M32},

   [midgard_op_st_tilebuffer_32f] = {"ST_TILEBUFFER.f32", M32},
   [midgard_op_st_tilebuffer_16f] = {"ST_TILEBUFFER.f16", M16},
   [midgard_op_st_tilebuffer_raw] = {"ST_TILEBUFFER.raw", M32},
};

struct mir_tex_op_props tex_opcode_props[16] = {
   [midgard_tex_op_normal] =                 {"TEX", M32},
   [midgard_tex_op_gradient] =               {"TEX_GRAD", M32},
   [midgard_tex_op_fetch] =                  {"TEX_FETCH", M32},
   [midgard_tex_op_grad_from_derivative] =   {"DER_TO_GRAD", M32},
   [midgard_tex_op_grad_from_coords] =       {"COORDS_TO_GRAD", M32},
   [midgard_tex_op_mov] =                    {"MOV", M32},
   [midgard_tex_op_barrier] =                {"BARRIER", M32},
   [midgard_tex_op_derivative] =             {"DERIVATIVE", M32}
};

#undef M8
#undef M16
#undef M32
#undef M64

struct mir_tag_props midgard_tag_props[16] = {
   [TAG_INVALID]           = {"invalid", 0},
   [TAG_BREAK]             = {"break", 0},
   [TAG_TEXTURE_4_VTX]     = {"tex/vt", 1},
   [TAG_TEXTURE_4]         = {"tex", 1},
   [TAG_TEXTURE_4_BARRIER] = {"tex/bar", 1},
   [TAG_LOAD_STORE_4]      = {"ldst", 1},
   [TAG_UNKNOWN_1]         = {"unk1", 1},
   [TAG_UNKNOWN_2]         = {"unk2", 1},
   [TAG_ALU_4]             = {"alu/4", 1},
   [TAG_ALU_8]             = {"alu/8", 2},
   [TAG_ALU_12]            = {"alu/12", 3},
   [TAG_ALU_16]            = {"alu/16", 4},
   [TAG_ALU_4_WRITEOUT]    = {"aluw/4", 1},
   [TAG_ALU_8_WRITEOUT]    = {"aluw/8", 2},
   [TAG_ALU_12_WRITEOUT]   = {"aluw/12", 3},
   [TAG_ALU_16_WRITEOUT]   = {"aluw/16", 4}
};
/* clang-format on */
