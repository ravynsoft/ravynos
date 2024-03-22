/*
 * Copyright (C) 2021 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "bi_builder.h"
#include "bi_test.h"
#include "va_compiler.h"

#include <gtest/gtest.h>

#define CASE(instr, expected)                                                  \
   do {                                                                        \
      uint64_t _value = va_pack_instr(instr);                                  \
      if (_value != expected) {                                                \
         fprintf(stderr, "Got %" PRIx64 ", expected %" PRIx64 "\n", _value,    \
                 (uint64_t)expected);                                          \
         bi_print_instr(instr, stderr);                                        \
         fprintf(stderr, "\n");                                                \
         ADD_FAILURE();                                                        \
      }                                                                        \
   } while (0)

class ValhallPacking : public testing::Test {
 protected:
   ValhallPacking()
   {
      mem_ctx = ralloc_context(NULL);
      b = bit_builder(mem_ctx);

      zero = bi_fau((enum bir_fau)(BIR_FAU_IMMEDIATE | 0), false);
      one = bi_fau((enum bir_fau)(BIR_FAU_IMMEDIATE | 8), false);
      n4567 = bi_fau((enum bir_fau)(BIR_FAU_IMMEDIATE | 4), true);
   }

   ~ValhallPacking()
   {
      ralloc_free(mem_ctx);
   }

   void *mem_ctx;
   bi_builder *b;
   bi_index zero, one, n4567;
};

TEST_F(ValhallPacking, Moves)
{
   CASE(bi_mov_i32_to(b, bi_register(1), bi_register(2)),
        0x0091c10000000002ULL);
   CASE(bi_mov_i32_to(b, bi_register(1),
                      bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 5), false)),
        0x0091c1000000008aULL);
}

TEST_F(ValhallPacking, Fadd)
{
   CASE(bi_fadd_f32_to(b, bi_register(0), bi_register(1), bi_register(2)),
        0x00a4c00000000201ULL);
   CASE(
      bi_fadd_f32_to(b, bi_register(0), bi_register(1), bi_abs(bi_register(2))),
      0x00a4c02000000201ULL);
   CASE(
      bi_fadd_f32_to(b, bi_register(0), bi_register(1), bi_neg(bi_register(2))),
      0x00a4c01000000201ULL);

   CASE(bi_fadd_v2f16_to(b, bi_register(0),
                         bi_swz_16(bi_register(1), false, false),
                         bi_swz_16(bi_register(0), true, true)),
        0x00a5c0000c000001ULL);

   CASE(bi_fadd_v2f16_to(b, bi_register(0), bi_register(1), bi_register(0)),
        0x00a5c00028000001ULL);

   CASE(bi_fadd_v2f16_to(b, bi_register(0), bi_register(1),
                         bi_swz_16(bi_register(0), true, false)),
        0x00a5c00024000001ULL);

   CASE(bi_fadd_v2f16_to(b, bi_register(0), bi_discard(bi_abs(bi_register(0))),
                         bi_neg(zero)),
        0x00a5c0902800c040ULL);

   CASE(bi_fadd_f32_to(b, bi_register(0), bi_register(1), zero),
        0x00a4c0000000c001ULL);

   CASE(bi_fadd_f32_to(b, bi_register(0), bi_register(1), bi_neg(zero)),
        0x00a4c0100000c001ULL);

   CASE(bi_fadd_f32_to(b, bi_register(0), bi_register(1),
                       bi_half(bi_register(0), true)),
        0x00a4c00008000001ULL);

   CASE(bi_fadd_f32_to(b, bi_register(0), bi_register(1),
                       bi_half(bi_register(0), false)),
        0x00a4c00004000001ULL);
}

TEST_F(ValhallPacking, Clper)
{
   CASE(bi_clper_i32_to(b, bi_register(0), bi_register(0), bi_byte(n4567, 0),
                        BI_INACTIVE_RESULT_F1, BI_LANE_OP_NONE,
                        BI_SUBGROUP_SUBGROUP16),
        0x00a0c030128fc900);
}

TEST_F(ValhallPacking, Clamps)
{
   bi_instr *I = bi_fadd_f32_to(b, bi_register(0), bi_register(1),
                                bi_neg(bi_abs(bi_register(2))));
   CASE(I, 0x00a4c03000000201ULL);

   I->clamp = BI_CLAMP_CLAMP_M1_1;
   CASE(I, 0x00a4c03200000201ULL);
}

TEST_F(ValhallPacking, Misc)
{
   CASE(bi_fma_f32_to(b, bi_register(1), bi_discard(bi_register(1)),
                      bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 4), false),
                      bi_neg(zero)),
        0x00b2c10400c08841ULL);

   CASE(bi_fround_f32_to(b, bi_register(2), bi_discard(bi_neg(bi_register(2))),
                         BI_ROUND_RTN),
        0x0090c240800d0042ULL);

   CASE(bi_fround_v2f16_to(b, bi_half(bi_register(0), false), bi_register(0),
                           BI_ROUND_RTN),
        0x00904000a00f0000ULL);

   CASE(
      bi_fround_v2f16_to(b, bi_half(bi_register(0), false),
                         bi_swz_16(bi_register(1), true, false), BI_ROUND_RTN),
      0x00904000900f0001ULL);
}

TEST_F(ValhallPacking, FaddImm)
{
   CASE(bi_fadd_imm_f32_to(b, bi_register(2), bi_discard(bi_register(2)),
                           0x4847C6C0),
        0x0114C24847C6C042ULL);

   CASE(bi_fadd_imm_v2f16_to(b, bi_register(2), bi_discard(bi_register(2)),
                             0x70AC6784),
        0x0115C270AC678442ULL);
}

TEST_F(ValhallPacking, Comparions)
{
   CASE(bi_icmp_or_v2s16_to(b, bi_register(2),
                            bi_discard(bi_swz_16(bi_register(3), true, false)),
                            bi_discard(bi_swz_16(bi_register(2), true, false)),
                            zero, BI_CMPF_GT, BI_RESULT_TYPE_M1),
        0x00f9c21184c04243);

   CASE(bi_fcmp_or_v2f16_to(b, bi_register(2),
                            bi_discard(bi_swz_16(bi_register(3), true, false)),
                            bi_discard(bi_swz_16(bi_register(2), false, false)),
                            zero, BI_CMPF_GT, BI_RESULT_TYPE_M1),
        0x00f5c20190c04243);
}

TEST_F(ValhallPacking, Conversions)
{
   CASE(bi_v2s16_to_v2f16_to(b, bi_register(2), bi_discard(bi_register(2))),
        0x0090c22000070042);
}

TEST_F(ValhallPacking, BranchzI16)
{
   bi_instr *I =
      bi_branchz_i16(b, bi_half(bi_register(2), false), bi_null(), BI_CMPF_EQ);
   I->branch_offset = 1;
   CASE(I, 0x001fc03000000102);
}

TEST_F(ValhallPacking, BranchzI16Backwards)
{
   bi_instr *I = bi_branchz_i16(b, zero, bi_null(), BI_CMPF_EQ);
   I->branch_offset = -8;
   CASE(I, 0x001fc017fffff8c0);
}

TEST_F(ValhallPacking, Blend)
{
   CASE(
      bi_blend_to(b, bi_null(), bi_register(0), bi_register(60),
                  bi_fau(BIR_FAU_BLEND_0, false), bi_fau(BIR_FAU_BLEND_0, true),
                  bi_null(), BI_REGISTER_FORMAT_F16, 2, 0),
      0x007f4004333c00f0);
}

TEST_F(ValhallPacking, Mux)
{
   CASE(bi_mux_i32_to(b, bi_register(0), bi_discard(bi_register(0)),
                      bi_discard(bi_register(4)),
                      bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 0), false),
                      BI_MUX_BIT),
        0x00b8c00300804440ull);
}

TEST_F(ValhallPacking, AtestFP16)
{
   CASE(bi_atest_to(b, bi_register(60), bi_register(60),
                    bi_half(bi_register(1), true),
                    bi_fau(BIR_FAU_ATEST_PARAM, false)),
        0x007dbc0208ea013c);
}

TEST_F(ValhallPacking, AtestFP32)
{
   CASE(bi_atest_to(b, bi_register(60), bi_register(60), one,
                    bi_fau(BIR_FAU_ATEST_PARAM, false)),
        0x007dbc0200ead03c);
}

TEST_F(ValhallPacking, Transcendentals)
{
   CASE(bi_frexpm_f32_to(b, bi_register(1), bi_register(0), false, true),
        0x0099c10001000000);

   CASE(bi_frexpe_f32_to(b, bi_register(0), bi_discard(bi_register(0)), false,
                         true),
        0x0099c00001020040);

   CASE(bi_frsq_f32_to(b, bi_register(2), bi_register(1)), 0x009cc20000020001);

   CASE(bi_fma_rscale_f32_to(b, bi_register(0), bi_discard(bi_register(1)),
                             bi_discard(bi_register(2)), bi_neg(zero),
                             bi_discard(bi_register(0)), BI_SPECIAL_LEFT),
        0x0162c00440c04241);
}

TEST_F(ValhallPacking, Csel)
{
   CASE(bi_csel_u32_to(b, bi_register(1), bi_discard(bi_register(2)),
                       bi_discard(bi_register(3)),
                       bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 2), false),
                       bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 2), true),
                       BI_CMPF_EQ),
        0x0150c10085844342);

   CASE(bi_csel_u32_to(b, bi_register(1), bi_discard(bi_register(2)),
                       bi_discard(bi_register(3)),
                       bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 2), false),
                       bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 2), true),
                       BI_CMPF_LT),
        0x0150c10485844342);

   CASE(bi_csel_s32_to(b, bi_register(1), bi_discard(bi_register(2)),
                       bi_discard(bi_register(3)),
                       bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 2), false),
                       bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 2), true),
                       BI_CMPF_LT),
        0x0158c10485844342);
}

TEST_F(ValhallPacking, LdAttrImm)
{
   bi_instr *I = bi_ld_attr_imm_to(
      b, bi_register(0), bi_discard(bi_register(60)),
      bi_discard(bi_register(61)), BI_REGISTER_FORMAT_F16, BI_VECSIZE_V4, 1);
   I->table = 1;

   CASE(I, 0x0066800433117d7c);
}

TEST_F(ValhallPacking, LdVarBufImmF16)
{
   CASE(bi_ld_var_buf_imm_f16_to(b, bi_register(2), bi_register(61),
                                 BI_REGISTER_FORMAT_F16, BI_SAMPLE_CENTER,
                                 BI_SOURCE_FORMAT_F16, BI_UPDATE_RETRIEVE,
                                 BI_VECSIZE_V4, 0),
        0x005d82143300003d);

   CASE(bi_ld_var_buf_imm_f16_to(b, bi_register(0), bi_register(61),
                                 BI_REGISTER_FORMAT_F16, BI_SAMPLE_SAMPLE,
                                 BI_SOURCE_FORMAT_F16, BI_UPDATE_STORE,
                                 BI_VECSIZE_V4, 0),
        0x005d80843300003d);

   CASE(bi_ld_var_buf_imm_f16_to(b, bi_register(0), bi_register(61),
                                 BI_REGISTER_FORMAT_F16, BI_SAMPLE_CENTROID,
                                 BI_SOURCE_FORMAT_F16, BI_UPDATE_STORE,
                                 BI_VECSIZE_V4, 8),
        0x005d80443308003d);
}

TEST_F(ValhallPacking, LeaBufImm)
{
   CASE(bi_lea_buf_imm_to(b, bi_register(4), bi_discard(bi_register(59))),
        0x005e840400000d7b);
}

TEST_F(ValhallPacking, StoreSegment)
{
   CASE(bi_store_i96(b, bi_register(0), bi_discard(bi_register(4)),
                     bi_discard(bi_register(5)), BI_SEG_VARY, 0),
        0x0061400632000044);
}

TEST_F(ValhallPacking, Convert16To32)
{
   CASE(bi_u16_to_u32_to(b, bi_register(2),
                         bi_discard(bi_swz_16(bi_register(55), false, false))),
        0x0090c20000140077);

   CASE(bi_u16_to_u32_to(b, bi_register(2),
                         bi_discard(bi_swz_16(bi_register(55), true, false))),
        0x0090c20010140077);

   CASE(bi_u16_to_f32_to(b, bi_register(2),
                         bi_discard(bi_swz_16(bi_register(55), false, false))),
        0x0090c20000150077);

   CASE(bi_u16_to_f32_to(b, bi_register(2),
                         bi_discard(bi_swz_16(bi_register(55), true, false))),
        0x0090c20010150077);

   CASE(bi_s16_to_s32_to(b, bi_register(2),
                         bi_discard(bi_swz_16(bi_register(55), false, false))),
        0x0090c20000040077);

   CASE(bi_s16_to_s32_to(b, bi_register(2),
                         bi_discard(bi_swz_16(bi_register(55), true, false))),
        0x0090c20010040077);
}

TEST_F(ValhallPacking, Swizzle8)
{
   CASE(bi_icmp_or_v4u8_to(b, bi_register(1), bi_byte(bi_register(0), 0), zero,
                           zero, BI_CMPF_NE, BI_RESULT_TYPE_I1),
        0x00f2c14300c0c000);
}

TEST_F(ValhallPacking, FauPage1)
{
   CASE(bi_mov_i32_to(b, bi_register(1),
                      bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 32), false)),
        0x0291c10000000080ULL);
}

TEST_F(ValhallPacking, LdTileV3F16)
{
   CASE(bi_ld_tile_to(b, bi_register(4), bi_discard(bi_register(0)),
                      bi_register(60), bi_register(3), BI_REGISTER_FORMAT_F16,
                      BI_VECSIZE_V3),
        0x0078840423033c40);
}

TEST_F(ValhallPacking, Rhadd8)
{
   CASE(bi_hadd_v4s8_to(b, bi_register(0), bi_discard(bi_register(1)),
                        bi_discard(bi_register(0)), BI_ROUND_RTP),
        0x00aac000400b4041);
}
