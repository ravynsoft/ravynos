
#include "../sfn_instr_alu.h"
#include "../sfn_instr_export.h"
#include "../sfn_instr_fetch.h"
#include "../sfn_instr_lds.h"
#include "../sfn_instr_mem.h"
#include "../sfn_instr_tex.h"
#include "../sfn_instrfactory.h"

#include "gtest/gtest.h"
#include <sstream>

namespace r600 {

using std::istringstream;
using std::ostringstream;
using std::string;

class TestInstrFromString : public ::testing::Test {
public:
   TestInstrFromString();

   PInst from_string(const std::string& s);

protected:
   void add_dest_from_string(const char *init);
   void add_dest_vec4_from_string(const char *init);

   void check(const Instr& eval, const Instr& expect);
   void check(const string& init, const Instr& expect);

   void SetUp() override;
   void TearDown() override;

   InstrFactory *m_instr_factory{nullptr};
};

TEST_F(TestInstrFromString, test_alu_mov)
{
   add_dest_from_string("R1999.x");

   AluInstr expect(op1_mov,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 0, pin_none),
                   {alu_write, alu_last_instr});

   check("ALU MOV R2000.y : R1999.x {WL}", expect);
}

TEST_F(TestInstrFromString, test_alu_lds_read_ret)
{
   add_dest_from_string("R1999.x");

   AluInstr expect(DS_OP_READ_RET, {new Register(1999, 0, pin_none)}, {});

   check("ALU LDS READ_RET __.x : R1999.x {}", expect);
}

TEST_F(TestInstrFromString, test_alu_mov_literal)
{
   AluInstr expect(op1_mov,
                   new Register(2000, 1, pin_none),
                   new LiteralConstant(0x10),
                   {alu_write, alu_last_instr});

   check("ALU MOV R2000.y : L[0x10] {WL}", expect);
}

TEST_F(TestInstrFromString, test_alu_mov_neg)
{
   add_dest_from_string("R1999.x");
   AluInstr expect(op1_mov,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 0, pin_none),
                   {alu_write, alu_last_instr});
   expect.set_source_mod(0, AluInstr::mod_neg);

   check("ALU MOV R2000.y : -R1999.x {WL}", expect);
}

TEST_F(TestInstrFromString, test_alu_mov_abs)
{
   add_dest_from_string("R1999.x");
   AluInstr expect(op1_mov,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 0, pin_none),
                   {alu_write, alu_last_instr});
   expect.set_source_mod(0, AluInstr::mod_abs);

   check("ALU MOV R2000.y : |R1999.x| {WL}", expect);
}

TEST_F(TestInstrFromString, test_alu_mov_neg_abs)
{
   add_dest_from_string("R1999.x");
   AluInstr expect(op1_mov,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 0, pin_none),
                   {alu_write});
   expect.set_source_mod(0, AluInstr::mod_abs);
   expect.set_source_mod(0, AluInstr::mod_neg);

   check("ALU MOV R2000.y : -|R1999.x| {W}", expect);
}

TEST_F(TestInstrFromString, test_alu_add)
{
   add_dest_from_string("R1998.z");
   add_dest_from_string("R1999.w");

   AluInstr expect(op2_add,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 3, pin_none),
                   new Register(1998, 2, pin_none),
                   {alu_last_instr});
   check("ALU ADD __.y : R1999.w R1998.z {L}", expect);
}

TEST_F(TestInstrFromString, test_alu_add_clmap)
{
   add_dest_from_string("R1998.z");
   add_dest_from_string("R1999.w");
   AluInstr expect(op2_add,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 3, pin_none),
                   new Register(1998, 2, pin_none),
                   {alu_last_instr, alu_dst_clamp});
   check("ALU ADD CLAMP __.y : R1999.w R1998.z {L}", expect);
}

TEST_F(TestInstrFromString, test_alu_add_neg2)
{
   add_dest_from_string("R1998.z");
   add_dest_from_string("R1999.w");
   AluInstr expect(op2_add,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 3, pin_none),
                   new Register(1998, 2, pin_none),
                   {alu_last_instr});
   expect.set_source_mod(1, AluInstr::mod_neg);

   check("ALU ADD __.y : R1999.w -R1998.z {L}", expect);
}

TEST_F(TestInstrFromString, test_alu_sete_update_pref)
{
   add_dest_from_string("R1998.z");
   add_dest_from_string("R1999.w");
   AluInstr expect(op2_sete,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 3, pin_none),
                   new Register(1998, 2, pin_none),
                   {alu_last_instr, alu_update_pred});
   expect.set_source_mod(1, AluInstr::mod_neg);
   check("ALU SETE __.y : R1999.w -R1998.z {LP}", expect);
}

TEST_F(TestInstrFromString, test_alu_sete_update_pref_empty_dest)
{
   add_dest_from_string("R1998.z");
   add_dest_from_string("R1999.w");
   AluInstr expect(op2_sete,
                   new Register(2000, 0, pin_none),
                   new Register(1999, 3, pin_none),
                   new Register(1998, 2, pin_none),
                   {alu_last_instr, alu_update_pred});
   check("ALU SETE __.x : R1999.w R1998.z {LP}", expect);
}

TEST_F(TestInstrFromString, test_alu_setne_update_exec)
{
   add_dest_from_string("R1998.z");
   add_dest_from_string("R1999.w");
   AluInstr expect(op2_setne,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 3, pin_none),
                   new Register(1998, 2, pin_none),
                   {alu_last_instr, alu_update_exec});
   expect.set_source_mod(1, AluInstr::mod_neg);
   check("ALU SETNE __.y : R1999.w -R1998.z {LE}", expect);
}

TEST_F(TestInstrFromString, test_alu_add_abs2)
{
   add_dest_from_string("R1998.z");
   add_dest_from_string("R1999.w");
   AluInstr expect(op2_add,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 3, pin_none),
                   new Register(1998, 2, pin_none),
                   {alu_write, alu_last_instr});
   expect.set_source_mod(1, AluInstr::mod_abs);
   check("ALU ADD R2000.y : R1999.w |R1998.z| {WL}", expect);
}

TEST_F(TestInstrFromString, test_alu_add_abs2_neg2)
{
   add_dest_from_string("R1998.z");
   add_dest_from_string("R1999.w");
   AluInstr expect(op2_add,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 3, pin_none),
                   new Register(1998, 2, pin_none),
                   {alu_write, alu_last_instr});
   expect.set_source_mod(1, AluInstr::mod_neg);
   expect.set_source_mod(1, AluInstr::mod_abs);

   check("ALU ADD R2000.y : R1999.w -|R1998.z| {WL}", expect);
}

TEST_F(TestInstrFromString, test_alu_muladd)
{
   add_dest_from_string("R1998.z");
   add_dest_from_string("R1999.w");
   add_dest_from_string("R2000.y");
   AluInstr expect(op3_muladd_ieee,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 3, pin_none),
                   new Register(1998, 2, pin_none),
                   new Register(2000, 1, pin_none),
                   {alu_write, alu_last_instr});
   check("ALU MULADD_IEEE R2000.y : R1999.w R1998.z R2000.y {WL}", expect);
}

TEST_F(TestInstrFromString, test_alu_muladd_neg3)
{
   add_dest_from_string("R1998.z");
   add_dest_from_string("R1999.w");
   add_dest_from_string("R2000.y");
   AluInstr expect(op3_muladd_ieee,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 3, pin_none),
                   new Register(1998, 2, pin_none),
                   new Register(2000, 1, pin_none),
                   {alu_last_instr});
   check("ALU MULADD_IEEE __.y : R1999.w R1998.z -R2000.y {L}", expect);
}

TEST_F(TestInstrFromString, test_alu_mov_bs)
{
   add_dest_from_string("R1999.x");
   for (auto& [expect_bs, str] : AluInstr::bank_swizzle_map) {
      auto init = std::string("ALU MOV R2000.y : R1999.x {WL} ") + str;

      AluInstr expect(op1_mov,
                      new Register(2000, 1, pin_none),
                      new Register(1999, 0, pin_none),
                      {alu_write, alu_last_instr});
      expect.set_bank_swizzle(expect_bs);

      check(init, expect);
   }
}

TEST_F(TestInstrFromString, test_alu_dot4_ieee)
{
   add_dest_from_string("R199.x");
   add_dest_from_string("R199.y");
   add_dest_from_string("R199.z");
   add_dest_from_string("R199.w");
   add_dest_from_string("R198.x");
   add_dest_from_string("R198.y");
   add_dest_from_string("R198.z");
   add_dest_from_string("R198.w");
   auto init = std::string("ALU DOT4_IEEE R2000.y : R199.x R198.w + R199.y R198.z + "
                           "R199.z R198.y + R199.w R198.x {WL}");

   AluInstr expect(op2_dot4_ieee,
                   new Register(2000, 1, pin_none),
                   {new Register(199, 0, pin_none),
                    new Register(198, 3, pin_none),
                    new Register(199, 1, pin_none),
                    new Register(198, 2, pin_none),
                    new Register(199, 2, pin_none),
                    new Register(198, 1, pin_none),
                    new Register(199, 3, pin_none),
                    new Register(198, 0, pin_none)},
                   {alu_write, alu_last_instr},
                   4);

   check(init, expect);
}

TEST_F(TestInstrFromString, test_alu_dot4_with_mods)
{
   add_dest_from_string("R199.x");
   add_dest_from_string("R199.y");
   add_dest_from_string("R199.z");
   add_dest_from_string("R199.w");
   add_dest_from_string("R198.x");
   add_dest_from_string("R198.y");
   add_dest_from_string("R198.z");
   add_dest_from_string("R198.w");
   auto init = std::string("ALU DOT4_IEEE R2000.y : -R199.x R198.w + R199.y |R198.z| + "
                           "-|R199.z| R198.y + -R199.w R198.x {WL}");

   AluInstr expect(op2_dot4_ieee,
                   new Register(2000, 1, pin_none),
                   {new Register(199, 0, pin_none),
                    new Register(198, 3, pin_none),
                    new Register(199, 1, pin_none),
                    new Register(198, 2, pin_none),
                    new Register(199, 2, pin_none),
                    new Register(198, 1, pin_none),
                    new Register(199, 3, pin_none),
                    new Register(198, 0, pin_none)},
                   {alu_write, alu_last_instr},
                   4);

   expect.set_source_mod(0, AluInstr::mod_neg);
   expect.set_source_mod(3, AluInstr::mod_abs);
   expect.set_source_mod(4, AluInstr::mod_neg);
   expect.set_source_mod(4, AluInstr::mod_abs);
   expect.set_source_mod(7, AluInstr::mod_neg);

   check(init, expect);
   auto instr = from_string(init);

   std::ostringstream print_str;
   print_str << *instr;
   EXPECT_EQ(print_str.str(), init);

}


TEST_F(TestInstrFromString, test_alu_mov_cf)
{
   add_dest_from_string("R1999.x");
   for (auto& [expect_cf, str] : AluInstr::cf_map) {
      auto init = std::string("ALU MOV R2000.y : R1999.x {WL} ") + str;

      AluInstr expect(op1_mov,
                      new Register(2000, 1, pin_none),
                      new Register(1999, 0, pin_none),
                      {alu_write, alu_last_instr});
      expect.set_cf_type(expect_cf);

      check(init, expect);
   }
}

TEST_F(TestInstrFromString, test_alu_interp_xy)
{
   add_dest_from_string("R0.y@fully");
   auto init =
      std::string("ALU INTERP_ZW R1024.z@chan : R0.y@fully Param0.z {W} VEC_210");

   auto r0y = new Register(0, 1, pin_fully);
   r0y->set_flag(Register::pin_start);
   AluInstr expect(op2_interp_zw,
                   new Register(1024, 2, pin_chan),
                   r0y,
                   new InlineConstant(ALU_SRC_PARAM_BASE, 2),
                   {alu_write});
   expect.set_bank_swizzle(alu_vec_210);

   check(init, expect);
}

TEST_F(TestInstrFromString, test_alu_interp_xy_no_write)
{
   add_dest_from_string("R0.x@fully");
   auto init = std::string("ALU INTERP_XY __.x@chan : R0.x@fully Param0.z {} VEC_210");

   auto r0x = new Register(0, 0, pin_fully);
   r0x->set_flag(Register::pin_start);

   AluInstr expect(op2_interp_xy,
                   new Register(1024, 0, pin_chan),
                   r0x,
                   new InlineConstant(ALU_SRC_PARAM_BASE, 2),
                   {});
   expect.set_bank_swizzle(alu_vec_210);

   check(init, expect);
}

TEST_F(TestInstrFromString, test_alu_mov_cf_bs)
{
   add_dest_from_string("R1999.x");
   auto init = std::string("ALU MOV R2000.y : R1999.x {WL} VEC_210 POP_AFTER");
   AluInstr expect(op1_mov,
                   new Register(2000, 1, pin_none),
                   new Register(1999, 0, pin_none),
                   {alu_write, alu_last_instr});
   expect.set_cf_type(cf_alu_pop_after);
   expect.set_bank_swizzle(alu_vec_210);
   check(init, expect);
}

TEST_F(TestInstrFromString, test_tex_sample_basic)
{
   add_dest_vec4_from_string("R2000.xyzw");
   auto init = std::string("TEX SAMPLE R1000.xyzw : R2000.xyzw RID:10 SID:1 NNNN");
   TexInstr expect(
      TexInstr::sample, RegisterVec4(1000), {0, 1, 2, 3}, RegisterVec4(2000), 10, nullptr, 1);
   check(init, expect);
}

TEST_F(TestInstrFromString, test_tex_ld_basic)
{
   add_dest_vec4_from_string("R2002.xyzw");
   auto init = std::string("TEX LD R1001.xyzw : R2002.xyzw RID:27 SID:7 NNNN");
   TexInstr expect(
      TexInstr::ld, RegisterVec4(1001), {0, 1, 2, 3}, RegisterVec4(2002), 27, nullptr, 7);
   check(init, expect);
}

TEST_F(TestInstrFromString, test_tex_sample_with_offset)
{
   add_dest_vec4_from_string("R2002.xyzw");
   auto init =
      std::string("TEX SAMPLE R1001.xyzw : R2002.xyzw RID:27 SID:2 OX:1 OY:-2 OZ:5 NNNN");

   TexInstr expect(
      TexInstr::sample, RegisterVec4(1001), {0, 1, 2, 3}, RegisterVec4(2002), 27, nullptr, 2);
   expect.set_offset(0, 1);
   expect.set_offset(1, -2);
   expect.set_offset(2, 5);

   check(init, expect);
}

TEST_F(TestInstrFromString, test_tex_gather4_x)
{
   add_dest_vec4_from_string("R2002.xyzw");
   auto init =
      std::string("TEX GATHER4 R1001.xyzw : R2002.xyzw RID:7 SID:27 MODE:0 NNNN");
   TexInstr expect(
      TexInstr::gather4, RegisterVec4(1001), {0, 1, 2, 3}, RegisterVec4(2002), 7, nullptr, 27);
   check(init, expect);
}

TEST_F(TestInstrFromString, test_tex_gather4_y)
{
   add_dest_vec4_from_string("R2002.xyzw");
   auto init =
      std::string("TEX GATHER4 R1001.xyzw : R2002.xyzw RID:7 SID:27 MODE:1 NNNN");
   TexInstr expect(
      TexInstr::gather4, RegisterVec4(1001), {0, 1, 2, 3}, RegisterVec4(2002), 7, nullptr, 27);
   expect.set_gather_comp(1);
   check(init, expect);
}

TEST_F(TestInstrFromString, test_tex_sampler_with_offset)
{
   add_dest_vec4_from_string("R2002.xyzw");
   auto init =
      std::string("TEX SAMPLE R1001.xyzw : R2002.xyzw RID:7 SID:27 SO:R200.z NNNN");
   TexInstr expect(TexInstr::sample,
                   RegisterVec4(1001),
                   {0, 1, 2, 3},
                   RegisterVec4(2002),
                   7,
                   nullptr,
                   27,
                   new Register(200, 2, pin_none));
   check(init, expect);
}

TEST_F(TestInstrFromString, test_export_param_60)
{
   add_dest_vec4_from_string("R1001.xyzw");

   ExportInstr expect(ExportInstr::param, 60, RegisterVec4(1001));
   check("EXPORT PARAM 60 R1001.xyzw", expect);
}

TEST_F(TestInstrFromString, test_export_pos_61)
{
   add_dest_from_string("R1002.y@group");

   ExportInstr expect(ExportInstr::pos, 61, RegisterVec4(1002, false, {1, 4, 5, 7}));
   check("EXPORT POS 61 R1002.y01_", expect);
}

TEST_F(TestInstrFromString, test_export_last_pixel_0)
{
   add_dest_vec4_from_string("R1002.xyzw");

   ExportInstr expect(ExportInstr::pixel, 0, RegisterVec4(1002, false, {2, 3, 0, 1}));
   expect.set_is_last_export(true);
   check("EXPORT_DONE PIXEL 0 R1002.zwxy", expect);
}

TEST_F(TestInstrFromString, test_fetch_basic)
{
   add_dest_from_string("R201.z");

   FetchInstr expect(vc_fetch,
                     RegisterVec4(1002),
                     {0, 4, 5, 1},
                     new Register(201, 2, pin_none),
                     0,
                     vertex_data,
                     fmt_8,
                     vtx_nf_norm,
                     vtx_es_none,
                     1,
                     nullptr);
   expect.set_mfc(31);
   expect.set_element_size(3);
   check("VFETCH R1002.x01y : R201.z RID:1 VERTEX FMT(8,UNORM) MFC:31 ES:3", expect);
}

TEST_F(TestInstrFromString, test_query_buffer_size)
{
   QueryBufferSizeInstr expect(RegisterVec4(1002),
                               RegisterVec4::Swizzle({0, 1, 2, 3}),
                               1);
   check("GET_BUF_RESINFO R1002.xyzw : RID:1", expect);

   FetchInstr expect_fetch(vc_get_buf_resinfo,
                           RegisterVec4(1002),
                           RegisterVec4::Swizzle({0, 1, 2, 3}),
                           new Register(0, 7, pin_fully),
                           0,
                           no_index_offset,
                           fmt_32_32_32_32,
                           vtx_nf_norm,
                           vtx_es_none,
                           1,
                           nullptr);
   expect_fetch.set_fetch_flag(FetchInstr::format_comp_signed);
   check("GET_BUF_RESINFO R1002.xyzw : RID:1", expect_fetch);
}

TEST_F(TestInstrFromString, test_load_from_buffer)
{
   add_dest_from_string("R201.x");
   add_dest_from_string("R202.x");
   string init = "LOAD_BUF R200.xzwy : R201.x + 16b RID:10 + R202.x";
   LoadFromBuffer expect(RegisterVec4(200),
                         RegisterVec4::Swizzle({0, 2, 3, 1}),
                         new Register(201, 0, pin_none),
                         16,
                         10,
                         new Register(202, 0, pin_none),
                         fmt_32_32_32_32_float);
   check(init, expect);

   auto instr = from_string(init);
   FetchInstr expect_fetch(vc_fetch,
                           RegisterVec4(200),
                           RegisterVec4::Swizzle({0, 2, 3, 1}),
                           new Register(201, 0, pin_none),
                           16,
                           no_index_offset,
                           fmt_32_32_32_32_float,
                           vtx_nf_scaled,
                           vtx_es_none,
                           10,
                           new Register(202, 0, pin_none));
   expect_fetch.set_fetch_flag(FetchInstr::format_comp_signed);
   expect_fetch.set_mfc(16);
   check(*instr, expect_fetch);
}

TEST_F(TestInstrFromString, test_load_from_scratch)
{

   add_dest_from_string("R201.x");
   string init = "READ_SCRATCH R200.xzwy : R201.x SIZE:20 ES:3";

   LoadFromScratch expect(RegisterVec4(200),
                          RegisterVec4::Swizzle({0, 2, 3, 1}),
                          new Register(201, 0, pin_none),
                          20);
   check(init, expect);

   FetchInstr expect_fetch(vc_read_scratch,
                           RegisterVec4(200),
                           RegisterVec4::Swizzle({0, 2, 3, 1}),
                           new Register(201, 0, pin_none),
                           0,
                           no_index_offset,
                           fmt_32_32_32_32,
                           vtx_nf_int,
                           vtx_es_none,
                           0,
                           nullptr);
   expect_fetch.set_element_size(3);
   expect_fetch.set_print_skip(FetchInstr::EPrintSkip::mfc);
   expect_fetch.set_print_skip(FetchInstr::EPrintSkip::fmt);
   expect_fetch.set_print_skip(FetchInstr::EPrintSkip::ftype);
   expect_fetch.set_fetch_flag(FetchInstr::EFlags::uncached);
   expect_fetch.set_fetch_flag(FetchInstr::EFlags::indexed);
   expect_fetch.set_fetch_flag(FetchInstr::EFlags::wait_ack);
   expect_fetch.set_array_size(19);

   check(init, expect_fetch);
}

TEST_F(TestInstrFromString, test_write_scratch_to_offset)
{
   add_dest_vec4_from_string("R1.xyzw");
   string init = "WRITE_SCRATCH 20 R1.xyzw AL:4 ALO:16";
   ScratchIOInstr expect(RegisterVec4(1), 20, 4, 16, 0xf);
   check(init, expect);

   add_dest_vec4_from_string("R2.xyzw");
   string init2 = "WRITE_SCRATCH 10 R2.xy_w AL:8 ALO:8";
   ScratchIOInstr expect2(RegisterVec4(2), 10, 8, 8, 0xb);
   check(init2, expect2);
}

TEST_F(TestInstrFromString, test_write_scratch_to_index)
{
   add_dest_vec4_from_string("R1.xyzw");
   add_dest_from_string("R3.x");
   string init = "WRITE_SCRATCH @R3.x[10] R1.xyzw AL:4 ALO:16";
   ScratchIOInstr expect(RegisterVec4(1), new Register(3, 0, pin_none), 4, 16, 0xf, 10);
   check(init, expect);

   add_dest_vec4_from_string("R2.xyzw");
   add_dest_from_string("R4.x");
   string init2 = "WRITE_SCRATCH @R4.x[20] R2.xy__ AL:4 ALO:16";
   ScratchIOInstr expect2(RegisterVec4(2), new Register(4, 0, pin_none), 4, 16, 0x3, 20);
   check(init2, expect2);
}

TEST_F(TestInstrFromString, test_load_from_scratch_fixed_offset)
{
   string init = "READ_SCRATCH R200.xzwy : L[0xA] SIZE:40 ES:3";

   LoadFromScratch expect(RegisterVec4(200),
                          RegisterVec4::Swizzle({0, 2, 3, 1}),
                          new LiteralConstant(10),
                          40);
   check(init, expect);

   FetchInstr expect_fetch(vc_read_scratch,
                           RegisterVec4(200),
                           RegisterVec4::Swizzle({0, 2, 3, 1}),
                           new Register(0, 7, pin_none),
                           0,
                           no_index_offset,
                           fmt_32_32_32_32,
                           vtx_nf_int,
                           vtx_es_none,
                           0,
                           nullptr);
   expect_fetch.set_element_size(3);
   expect_fetch.set_print_skip(FetchInstr::EPrintSkip::mfc);
   expect_fetch.set_print_skip(FetchInstr::EPrintSkip::fmt);
   expect_fetch.set_print_skip(FetchInstr::EPrintSkip::ftype);
   expect_fetch.set_fetch_flag(FetchInstr::EFlags::uncached);
   expect_fetch.set_fetch_flag(FetchInstr::EFlags::wait_ack);
   expect_fetch.set_array_base(10);
   expect_fetch.set_array_size(39);

   check(init, expect_fetch);
}

TEST_F(TestInstrFromString, test_lds_read_3_values)
{
   add_dest_from_string("R5.x@free");
   add_dest_from_string("R5.y@free");
   add_dest_from_string("R5.z@free");

   auto init =
      "LDS_READ [ R10.x@free R11.x@free R12.x@free ] : [ R5.x@free R5.y@free R5.z@free ]";

   std::vector<PRegister, Allocator<PRegister>> dests(3);
   std::vector<PVirtualValue, Allocator<PVirtualValue>> srcs(3);

   for (int i = 0; i < 3; ++i) {
      dests[i] = new Register(10 + i, 0, pin_free);
      srcs[i] = new Register(5, i, pin_free);
   }

   LDSReadInstr expect(dests, srcs);
   check(init, expect);
}

TEST_F(TestInstrFromString, test_lds_read_2_values)
{
   add_dest_from_string("R5.x@free");
   add_dest_from_string("R5.y@free");

   auto init = "LDS_READ [ R11.x@free R12.x@free ] : [ R5.x@free R5.y@free ]";

   std::vector<PRegister, Allocator<PRegister>> dests(2);
   std::vector<PVirtualValue, Allocator<PVirtualValue>> srcs(2);

   for (int i = 0; i < 2; ++i) {
      dests[i] = new Register(11 + i, 0, pin_free);
      srcs[i] = new Register(5, i, pin_free);
   }

   LDSReadInstr expect(dests, srcs);
   check(init, expect);
}

TEST_F(TestInstrFromString, test_lds_write_1_value)
{
   auto init = "LDS WRITE __.x [ R1.x ] : R2.y";
   add_dest_from_string("R1.x");
   add_dest_from_string("R2.y");

   LDSAtomicInstr expect(DS_OP_WRITE,
                         nullptr,
                         new Register(1, 0, pin_none),
                         {new Register(2, 1, pin_none)});

   check(init, expect);
}

TEST_F(TestInstrFromString, test_lds_write_2_value)
{
   auto init = "LDS WRITE2 __.x [ R1.x ] : R2.y KC0[1].z";

   add_dest_from_string("R1.x");
   add_dest_from_string("R2.y");

   LDSAtomicInstr expect(DS_OP_WRITE2,
                         nullptr,
                         new Register(1, 0, pin_none),
                         {new Register(2, 1, pin_none), new UniformValue(513, 2, 0)});

   check(init, expect);
}

TEST_F(TestInstrFromString, test_lds_write_atomic_add_ret)
{
   auto init = "LDS ADD_RET R7.y [ R1.x ] : R2.y";

   add_dest_from_string("R1.x");
   add_dest_from_string("R2.y");

   LDSAtomicInstr expect(DS_OP_ADD_RET,
                         new Register(7, 1, pin_none),
                         new Register(1, 0, pin_none),
                         {new Register(2, 1, pin_none)});

   check(init, expect);
}

TEST_F(TestInstrFromString, test_lds_write_atomic_add)
{
   auto init = "LDS ADD __.x [ R1.x ] : R2.y";

   add_dest_from_string("R1.x");
   add_dest_from_string("R2.y");

   LDSAtomicInstr expect(DS_OP_ADD,
                         nullptr,
                         new Register(1, 0, pin_none),
                         {new Register(2, 1, pin_none)});

   check(init, expect);
}

TEST_F(TestInstrFromString, test_writeTF)
{
   auto init = "WRITE_TF R1.xyzw";

   add_dest_vec4_from_string("R1.xyzw");

   WriteTFInstr expect(RegisterVec4(1, false, {0, 1, 2, 3}, pin_group));

   check(init, expect);
}

TestInstrFromString::TestInstrFromString() {}

PInst
TestInstrFromString::from_string(const std::string& s)
{
   return m_instr_factory->from_string(s, 0, false);
}

void
TestInstrFromString::check(const Instr& eval, const Instr& expect)
{
   EXPECT_EQ(eval, expect);
}

void
TestInstrFromString::check(const string& init, const Instr& expect)
{
   auto instr = from_string(init);
   ASSERT_TRUE(instr);
   EXPECT_EQ(*instr, expect);
}

void
TestInstrFromString::add_dest_from_string(const char *init)
{
   m_instr_factory->value_factory().dest_from_string(init);
}

void
TestInstrFromString::add_dest_vec4_from_string(const char *init)
{
   RegisterVec4::Swizzle dummy;
   m_instr_factory->value_factory().dest_vec4_from_string(init, dummy);
}

void
TestInstrFromString::SetUp()
{
   MemoryPool::instance().initialize();
   m_instr_factory = new InstrFactory;
}

void
TestInstrFromString::TearDown()
{
   MemoryPool::instance().free();
}

} // namespace r600
