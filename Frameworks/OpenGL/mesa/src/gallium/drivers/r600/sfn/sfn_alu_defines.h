/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2018-2019 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef r600_sfn_alu_defines_h
#define r600_sfn_alu_defines_h

#include "../r600_isa.h"

#include <bitset>
#include <map>

namespace r600 {

// We sacrifice 123 for dummy dests
static const int g_registers_end = 123;
static const int g_clause_local_start = 124;
static const int g_clause_local_end = 128;

/* ALU op2 instructions 17:7 top three bits always zero. */
enum EAluOp {
   op2_add = 0,
   op2_mul = 1,
   op2_mul_ieee = 2,
   op2_max = 3,
   op2_min = 4,
   op2_max_dx10 = 5,
   op2_min_dx10 = 6,
   op2_sete = 8,
   op2_setgt = 9,
   op2_setge = 10,
   op2_setne = 11,
   op2_sete_dx10 = 12,
   op2_setgt_dx10 = 13,
   op2_setge_dx10 = 14,
   op2_setne_dx10 = 15,
   op1_fract = 16,
   op1_trunc = 17,
   op1_ceil = 18,
   op1_rndne = 19,
   op1_floor = 20,
   op2_ashr_int = 21,
   op2_lshr_int = 22,
   op2_lshl_int = 23,
   op1_mov = 25,
   op0_nop = 26,
   op2_mul_64 = 27,
   op1_flt64_to_flt32 = 28,
   op1_flt32_to_flt64 = 29,
   op2_pred_setgt_uint = 30,
   op2_pred_setge_uint = 31,
   op2_pred_sete = 32,
   op2_pred_setgt = 33,
   op2_pred_setge = 34,
   op2_pred_setne = 35,
   op1_pred_set_inv = 36,
   op2_pred_set_pop = 37,
   op0_pred_set_clr = 38,
   op1_pred_set_restore = 39,
   op2_pred_sete_push = 40,
   op2_pred_setgt_push = 41,
   op2_pred_setge_push = 42,
   op2_pred_setne_push = 43,
   op2_kille = 44,
   op2_killgt = 45,
   op2_killge = 46,
   op2_killne = 47,
   op2_and_int = 48,
   op2_or_int = 49,
   op2_xor_int = 50,
   op1_not_int = 51,
   op2_add_int = 52,
   op2_sub_int = 53,
   op2_max_int = 54,
   op2_min_int = 55,
   op2_max_uint = 56,
   op2_min_uint = 57,
   op2_sete_int = 58,
   op2_setgt_int = 59,
   op2_setge_int = 60,
   op2_setne_int = 61,
   op2_setgt_uint = 62,
   op2_setge_uint = 63,
   op2_killgt_uint = 64,
   op2_killge_uint = 65,
   op2_prede_int = 66,
   op2_pred_setgt_int = 67,
   op2_pred_setge_int = 68,
   op2_pred_setne_int = 69,
   op2_kille_int = 70,
   op2_killgt_int = 71,
   op2_killge_int = 72,
   op2_killne_int = 73,
   op2_pred_sete_push_int = 74,
   op2_pred_setgt_push_int = 75,
   op2_pred_setge_push_int = 76,
   op2_pred_setne_push_int = 77,
   op2_pred_setlt_push_int = 78,
   op2_pred_setle_push_int = 79,
   op1_flt_to_int = 80,
   op1_bfrev_int = 81,
   op2_addc_uint = 82,
   op2_subb_uint = 83,
   op0_group_barrier = 84,
   op0_group_seq_begin = 85,
   op0_group_seq_end = 86,
   op2_set_mode = 87,
   op1_set_cf_idx0 = 88,
   op1_set_cf_idx1 = 89,
   op2_set_lds_size = 90,
   op1_exp_ieee = 129,
   op1_log_clamped = 130,
   op1_log_ieee = 131,
   op1_recip_clamped = 132,
   op1_recip_ff = 133,
   op1_recip_ieee = 134,
   op1_recipsqrt_clamped = 135,
   op1_recipsqrt_ff = 136,
   op1_recipsqrt_ieee1 = 137,
   op1_sqrt_ieee = 138,
   op1_sin = 141,
   op1_cos = 142,
   op2_mullo_int = 143,
   op2_mulhi_int = 144,
   op2_mullo_uint = 145,
   op2_mulhi_uint = 146,
   op1_recip_int = 147,
   op1_recip_uint = 148,
   op1_recip_64 = 149,
   op1_recip_clamped_64 = 150,
   op1_recipsqrt_64 = 151,
   op1_recipsqrt_clamped_64 = 152,
   op1_sqrt_64 = 153,
   op1_flt_to_uint = 154,
   op1_int_to_flt = 155,
   op1_uint_to_flt = 156,
   op2_bfm_int = 160,
   op1_flt32_to_flt16 = 162,
   op1_flt16_to_flt32 = 163,
   op1_ubyte0_flt = 164,
   op1_ubyte1_flt = 165,
   op1_ubyte2_flt = 166,
   op1_ubyte3_flt = 167,
   op1_bcnt_int = 170,
   op1_ffbh_uint = 171,
   op1_ffbl_int = 172,
   op1_ffbh_int = 173,
   op1_flt_to_uint4 = 174,
   op2_dot_ieee = 175,
   op1_flt_to_int_rpi = 176,
   op1_flt_to_int_floor = 177,
   op2_mulhi_uint24 = 178,
   op1_mbcnt_32hi_int = 179,
   op1_offset_to_flt = 180,
   op2_mul_uint24 = 181,
   op1_bcnt_accum_prev_int = 182,
   op1_mbcnt_32lo_accum_prev_int = 183,
   op2_sete_64 = 184,
   op2_setne_64 = 185,
   op2_setgt_64 = 186,
   op2_setge_64 = 187,
   op2_min_64 = 188,
   op2_max_64 = 189,
   op2_dot4 = 190,
   op2_dot4_ieee = 191,
   op2_cube = 192,
   op1_max4 = 193,
   op1_frexp_64 = 196,
   op1_ldexp_64 = 197,
   op1_fract_64 = 198,
   op2_pred_setgt_64 = 199,
   op2_pred_sete_64 = 198,
   op2_pred_setge_64 = 201,
   OP2V_MUL_64 = 202,
   op2_add_64 = 203,
   op1_mova_int = 204,
   op1v_flt64_to_flt32 = 205,
   op1v_flt32_to_flt64 = 206,
   op2_sad_accum_prev_uint = 207,
   op2_dot = 208,
   op1_mul_prev = 209,
   op1_mul_ieee_prev = 210,
   op1_add_prev = 211,
   op2_muladd_prev = 212,
   op2_muladd_ieee_prev = 213,
   op2_interp_xy = 214,
   op2_interp_zw = 215,
   op2_interp_x = 216,
   op2_interp_z = 217,
   op0_store_flags = 218,
   op1_load_store_flags = 219,
   op0_lds_1a = 220,
   op0_lds_1a1d = 221,
   op0_lds_2a = 223,
   op1_interp_load_p0 = 224,
   op1_interp_load_p10 = 125,
   op1_interp_load_p20 = 126,
   // op 3 all left shift 6
   op3_bfe_uint = 4 << 6,
   op3_bfe_int = 5 << 6,
   op3_bfi_int = 6 << 6,
   op3_fma = 7 << 6,
   op3_cndne_64 = 9 << 6,
   op3_fma_64 = 10 << 6,
   op3_lerp_uint = 11 << 6,
   op3_bit_align_int = 12 << 6,
   op3_byte_align_int = 13 << 6,
   op3_sad_accum_uint = 14 << 6,
   op3_sad_accum_hi_uint = 15 << 6,
   op3_muladd_uint24 = 16 << 6,
   op3_lds_idx_op = 17 << 6,
   op3_muladd = 20 << 6,
   op3_muladd_m2 = 21 << 6,
   op3_muladd_m4 = 22 << 6,
   op3_muladd_d2 = 23 << 6,
   op3_muladd_ieee = 24 << 6,
   op3_cnde = 25 << 6,
   op3_cndgt = 26 << 6,
   op3_cndge = 27 << 6,
   op3_cnde_int = 28 << 6,
   op3_cndgt_int = 29 << 6,
   op3_cndge_int = 30 << 6,
   op3_mul_lit = 31 << 6,
   op_invalid = 0xffff
};

enum AluModifiers {
   alu_src0_rel,
   alu_src1_rel,
   alu_src2_rel,
   alu_dst_clamp,
   alu_dst_rel,
   alu_last_instr,
   alu_update_exec,
   alu_update_pred,
   alu_write,
   alu_op3,
   alu_is_trans,
   alu_is_cayman_trans,
   alu_is_lds,
   alu_lds_group_start,
   alu_lds_group_end,
   alu_lds_address,
   alu_no_schedule_bias,
   alu_64bit_op,
   alu_flag_none,
   alu_flag_count
};

enum AluDstModifiers {
   omod_off = 0,
   omod_mul2 = 1,
   omod_mul4 = 2,
   omod_divl2 = 3
};

enum AluPredSel {
   pred_off = 0,
   pred_zero = 2,
   pred_one = 3
};

enum AluBankSwizzle {
   alu_vec_012 = 0,
   sq_alu_scl_201 = 0,
   alu_vec_021 = 1,
   sq_alu_scl_122 = 1,
   alu_vec_120 = 2,
   sq_alu_scl_212 = 2,
   alu_vec_102 = 3,
   sq_alu_scl_221 = 3,
   alu_vec_201 = 4,
   sq_alu_scl_unknown = 4,
   alu_vec_210 = 5,
   alu_vec_unknown = 6
};

inline AluBankSwizzle
operator++(AluBankSwizzle& x)
{
   x = static_cast<AluBankSwizzle>(x + 1);
   return x;
}

using AluOpFlags = std::bitset<alu_flag_count>;

struct AluOp {
   static constexpr int x = 1;
   static constexpr int y = 2;
   static constexpr int z = 4;
   static constexpr int w = 8;
   static constexpr int v = 15;
   static constexpr int t = 16;
   static constexpr int a = 31;

   AluOp(int ns, bool src_mod, bool clamp, bool fp64, uint8_t um_r600,
         uint8_t um_r700, uint8_t um_eg, const char *n):
       nsrc(ns),
       can_srcmod(src_mod),
       can_clamp(clamp),
       is_fp64(fp64),
       name(n)
   {
      unit_mask[0] = um_r600;
      unit_mask[1] = um_r700;
      unit_mask[2] = um_eg;
   }

   bool can_channel(int flags, r600_chip_class unit_type) const
   {
      assert(unit_type < 3);
      return flags & unit_mask[unit_type];
   }

   int nsrc : 4;
   int can_srcmod : 1;
   int can_clamp : 1;
   int is_fp64 : 1;
   uint8_t unit_mask[3];
   const char *name;
};

extern const std::map<EAluOp, AluOp> alu_ops;

enum AluInlineConstants {
   ALU_SRC_LDS_OQ_A = 219,
   ALU_SRC_LDS_OQ_B = 220,
   ALU_SRC_LDS_OQ_A_POP = 221,
   ALU_SRC_LDS_OQ_B_POP = 222,
   ALU_SRC_LDS_DIRECT_A = 223,
   ALU_SRC_LDS_DIRECT_B = 224,
   ALU_SRC_TIME_HI = 227,
   ALU_SRC_TIME_LO = 228,
   ALU_SRC_MASK_HI = 229,
   ALU_SRC_MASK_LO = 230,
   ALU_SRC_HW_WAVE_ID = 231,
   ALU_SRC_SIMD_ID = 232,
   ALU_SRC_SE_ID = 233,
   ALU_SRC_HW_THREADGRP_ID = 234,
   ALU_SRC_WAVE_ID_IN_GRP = 235,
   ALU_SRC_NUM_THREADGRP_WAVES = 236,
   ALU_SRC_HW_ALU_ODD = 237,
   ALU_SRC_LOOP_IDX = 238,
   ALU_SRC_PARAM_BASE_ADDR = 240,
   ALU_SRC_NEW_PRIM_MASK = 241,
   ALU_SRC_PRIM_MASK_HI = 242,
   ALU_SRC_PRIM_MASK_LO = 243,
   ALU_SRC_1_DBL_L = 244,
   ALU_SRC_1_DBL_M = 245,
   ALU_SRC_0_5_DBL_L = 246,
   ALU_SRC_0_5_DBL_M = 247,
   ALU_SRC_0 = 248,
   ALU_SRC_1 = 249,
   ALU_SRC_1_INT = 250,
   ALU_SRC_M_1_INT = 251,
   ALU_SRC_0_5 = 252,
   ALU_SRC_LITERAL = 253,
   ALU_SRC_PV = 254,
   ALU_SRC_PS = 255,
   ALU_SRC_PARAM_BASE = 0x1C0,
   ALU_SRC_UNKNOWN
};

struct AluInlineConstantDescr {
   bool use_chan;
   const char *descr;
};

extern const std::map<AluInlineConstants, AluInlineConstantDescr> alu_src_const;

#define LDSOP2(X) LDS_##X = LDS_OP2_LDS_##X

enum ESDOp {
   DS_OP_ADD = 0,
   DS_OP_SUB = 1,
   DS_OP_RSUB = 2,
   DS_OP_INC = 3,
   DS_OP_DEC = 4,
   DS_OP_MIN_INT = 5,
   DS_OP_MAX_INT = 6,
   DS_OP_MIN_UINT = 7,
   DS_OP_MAX_UINT = 8,
   DS_OP_AND = 9,
   DS_OP_OR = 10,
   DS_OP_XOR = 11,
   DS_OP_MSKOR = 12,
   DS_OP_WRITE = 13,
   DS_OP_WRITE_REL = 14,
   DS_OP_WRITE2 = 15,
   DS_OP_CMP_STORE = 16,
   DS_OP_CMP_STORE_SPF = 17,
   DS_OP_BYTE_WRITE = 18,
   DS_OP_SHORT_WRITE = 19,
   DS_OP_ADD_RET = 32,
   DS_OP_SUB_RET = 33,
   DS_OP_RSUB_RET = 34,
   DS_OP_INC_RET = 35,
   DS_OP_DEC_RET = 36,
   DS_OP_MIN_INT_RET = 37,
   DS_OP_MAX_INT_RET = 38,
   DS_OP_MIN_UINT_RET = 39,
   DS_OP_MAX_UINT_RET = 40,
   DS_OP_AND_RET = 41,
   DS_OP_OR_RET = 42,
   DS_OP_XOR_RET = 43,
   DS_OP_MSKOR_RET = 44,
   DS_OP_XCHG_RET = 45,
   DS_OP_XCHG_REL_RET = 46,
   DS_OP_XCHG2_RET = 47,
   DS_OP_CMP_XCHG_RET = 48,
   DS_OP_CMP_XCHG_SPF_RET = 49,
   DS_OP_READ_RET = 50,
   DS_OP_READ_REL_RET = 51,
   DS_OP_READ2_RET = 52,
   DS_OP_READWRITE_RET = 53,
   DS_OP_BYTE_READ_RET = 54,
   DS_OP_UBYTE_READ_RET = 55,
   DS_OP_SHORT_READ_RET = 56,
   DS_OP_USHORT_READ_RET = 57,
   DS_OP_ATOMIC_ORDERED_ALLOC_RET = 63,
   DS_OP_INVALID = 64,
   LDSOP2(ADD_RET),
   LDSOP2(ADD),
   LDSOP2(AND_RET),
   LDSOP2(AND),
   LDSOP2(WRITE),
   LDSOP2(OR_RET),
   LDSOP2(OR),
   LDSOP2(MAX_INT_RET),
   LDSOP2(MAX_INT),
   LDSOP2(MAX_UINT_RET),
   LDSOP2(MAX_UINT),
   LDSOP2(MIN_INT_RET),
   LDSOP2(MIN_INT),
   LDSOP2(MIN_UINT_RET),
   LDSOP2(MIN_UINT),
   LDSOP2(XOR_RET),
   LDSOP2(XOR),
   LDSOP2(XCHG_RET),
   LDS_CMP_XCHG_RET = LDS_OP3_LDS_CMP_XCHG_RET,
   LDS_WRITE_REL = LDS_OP3_LDS_WRITE_REL
};

#undef LDSOP2

struct LDSOp {
   int nsrc;
   const char *name;
};

extern const std::map<ESDOp, LDSOp> lds_ops;

struct KCacheLine {
   int bank{0};
   int addr{0};
   int len{0};
   int index_mode{0};
   enum KCacheLockMode {
      free,
      lock_1,
      lock_2
   } mode{free};
};

} // namespace r600

#endif // ALU_DEFINES_H
