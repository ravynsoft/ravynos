
#include "../sfn_instr_alugroup.h"
#include "../sfn_instr_export.h"
#include "../sfn_instr_fetch.h"
#include "../sfn_instr_lds.h"
#include "../sfn_instr_tex.h"

#include "gtest/gtest.h"

using namespace r600;

using std::vector;

class InstrTest : public ::testing::Test {
   void SetUp() override { init_pool(); }

   void TearDown() override { release_pool(); }

protected:
   void check(const Instr& lhs, const Instr& rhs) const { EXPECT_EQ(lhs, rhs); }
};

TEST_F(InstrTest, test_alu_barrier)
{
   AluInstr alu(op0_group_barrier);

   EXPECT_FALSE(alu.has_alu_flag(alu_write));
   EXPECT_EQ(alu.opcode(), op0_group_barrier);

   EXPECT_EQ(alu.dest_chan(), 0);

   EXPECT_EQ(alu, alu);
}

TEST_F(InstrTest, test_alu_uni_op_mov)
{
   AluInstr alu(op1_mov,
                new Register(128, 2, pin_none),
                new Register(129, 0, pin_chan),
                {alu_write});

   EXPECT_TRUE(alu.has_alu_flag(alu_write));

   EXPECT_FALSE(alu.has_alu_flag(alu_last_instr));
   EXPECT_FALSE(alu.end_group());
   EXPECT_FALSE(alu.has_alu_flag(alu_op3));
   EXPECT_FALSE(alu.has_source_mod(0, AluInstr::mod_abs));
   EXPECT_FALSE(alu.has_source_mod(0, AluInstr::mod_neg));

   EXPECT_EQ(alu.opcode(), op1_mov);

   EXPECT_EQ(alu.dest_chan(), 2);
   auto dest = alu.dest();

   ASSERT_TRUE(dest);
   EXPECT_EQ(dest->sel(), 128);
   EXPECT_EQ(dest->chan(), 2);
   EXPECT_EQ(dest->pin(), pin_none);

   auto src0 = alu.psrc(0);
   ASSERT_TRUE(src0);

   EXPECT_EQ(src0->sel(), 129);
   EXPECT_EQ(src0->chan(), 0);
   EXPECT_EQ(src0->pin(), pin_chan);

   EXPECT_EQ(alu.n_sources(), 1);

   EXPECT_FALSE(alu.psrc(1));
   EXPECT_FALSE(alu.psrc(2));

   alu.set_source_mod(0, AluInstr::mod_abs);;
   EXPECT_TRUE(alu.has_source_mod(0, AluInstr::mod_abs));

   alu.set_source_mod(0, AluInstr::mod_neg);
   EXPECT_TRUE(alu.has_source_mod(0, AluInstr::mod_neg));
}

TEST_F(InstrTest, test_alu_op2)
{
   AluInstr alu(op2_add,
                new Register(130, 1, pin_none),
                new Register(129, 2, pin_chan),
                new Register(129, 3, pin_none),
                {alu_write, alu_last_instr});

   EXPECT_TRUE(alu.has_alu_flag(alu_write));

   EXPECT_TRUE(alu.has_alu_flag(alu_last_instr));
   EXPECT_FALSE(alu.has_alu_flag(alu_op3));

   EXPECT_FALSE(alu.has_source_mod(0, AluInstr::mod_neg));
   EXPECT_FALSE(alu.has_source_mod(1, AluInstr::mod_neg));
   EXPECT_FALSE(alu.has_source_mod(2, AluInstr::mod_neg));

   EXPECT_FALSE(alu.has_alu_flag(alu_src0_rel));
   EXPECT_FALSE(alu.has_alu_flag(alu_src1_rel));
   EXPECT_FALSE(alu.has_alu_flag(alu_src2_rel));

   EXPECT_EQ(alu.opcode(), op2_add);

   EXPECT_EQ(alu.dest_chan(), 1);
   auto dest = alu.dest();

   ASSERT_TRUE(dest);
   EXPECT_EQ(dest->sel(), 130);
   EXPECT_EQ(dest->chan(), 1);
   EXPECT_EQ(dest->pin(), pin_none);

   EXPECT_EQ(alu.n_sources(), 2);

   auto src0 = alu.psrc(0);
   ASSERT_TRUE(src0);

   EXPECT_EQ(src0->sel(), 129);
   EXPECT_EQ(src0->chan(), 2);
   EXPECT_EQ(src0->pin(), pin_chan);

   auto src1 = alu.psrc(1);
   ASSERT_TRUE(src1);

   EXPECT_EQ(src1->sel(), 129);
   EXPECT_EQ(src1->chan(), 3);
   EXPECT_EQ(src1->pin(), pin_none);

   EXPECT_FALSE(alu.psrc(2));
   EXPECT_EQ(alu, alu);
}

TEST_F(InstrTest, test_alu_op3)
{
   AluInstr alu(op3_cnde,
                new Register(130, 1, pin_none),
                new Register(129, 2, pin_chan),
                new Register(129, 3, pin_none),
                new Register(131, 1, pin_none),
                {alu_write, alu_last_instr});

   EXPECT_TRUE(alu.has_alu_flag(alu_write));
   EXPECT_TRUE(alu.has_alu_flag(alu_last_instr));
   EXPECT_TRUE(alu.end_group());
   EXPECT_TRUE(alu.has_alu_flag(alu_op3));

   EXPECT_EQ(alu.opcode(), op3_cnde);

   EXPECT_EQ(alu.dest_chan(), 1);
   auto dest = alu.dest();

   ASSERT_TRUE(dest);
   EXPECT_EQ(dest->sel(), 130);
   EXPECT_EQ(dest->chan(), 1);
   EXPECT_EQ(dest->pin(), pin_none);

   EXPECT_EQ(alu.n_sources(), 3);

   auto src0 = alu.psrc(0);
   ASSERT_TRUE(src0);

   EXPECT_EQ(src0->sel(), 129);
   EXPECT_EQ(src0->chan(), 2);
   EXPECT_EQ(src0->pin(), pin_chan);

   auto src1 = alu.psrc(1);
   ASSERT_TRUE(src1);

   EXPECT_EQ(src1->sel(), 129);
   EXPECT_EQ(src1->chan(), 3);
   EXPECT_EQ(src1->pin(), pin_none);

   auto src2 = alu.psrc(2);
   ASSERT_TRUE(src2);

   EXPECT_EQ(src2->sel(), 131);
   EXPECT_EQ(src2->chan(), 1);
   EXPECT_EQ(src2->pin(), pin_none);

   EXPECT_EQ(alu, alu);
}

TEST_F(InstrTest, test_alu_op1_comp)
{
   auto r128z = new Register(128, 2, pin_none);
   auto r128zc = new Register(128, 2, pin_chan);
   auto r128y = new Register(128, 1, pin_none);
   auto r129x = new Register(129, 0, pin_none);
   auto r129xc = new Register(129, 0, pin_chan);
   auto r129y = new Register(129, 1, pin_none);
   auto r130x = new Register(130, 0, pin_none);

   AluInstr alu1(op1_mov, r128z, r129x, {alu_write});
   EXPECT_NE(alu1, AluInstr(op1_mov, r128y, r129x, {alu_write}));
   EXPECT_NE(alu1, AluInstr(op1_mov, r128z, r129xc, {alu_write}));
   EXPECT_NE(alu1, AluInstr(op1_mov, r128z, r129y, {alu_write}));
   EXPECT_NE(alu1, AluInstr(op1_mov, r128z, r130x, {alu_write}));
   EXPECT_NE(alu1, AluInstr(op1_mov, r128z, r129x, {alu_write, alu_last_instr}));
   EXPECT_NE(alu1, AluInstr(op1_flt_to_int, r128z, r129x, {alu_write}));
   EXPECT_NE(alu1, AluInstr(op1_mov, r128zc, r129x, {alu_write}));

   EXPECT_EQ(alu1, alu1);
}

TEST_F(InstrTest, test_alu_op2_comp)
{
   auto r128x = new Register(128, 0, pin_none);
   auto r128y = new Register(128, 1, pin_none);
   auto r128z = new Register(128, 2, pin_none);

   AluInstr alu1(op2_add, r128z, r128x, r128y, {alu_write});

   EXPECT_NE(
      alu1, AluInstr(op2_add, r128z, r128x, new Register(129, 2, pin_none), {alu_write}));
   EXPECT_NE(
      alu1, AluInstr(op2_add, r128z, r128x, new Register(128, 0, pin_none), {alu_write}));
   EXPECT_NE(
      alu1, AluInstr(op2_add, r128z, r128x, new Register(128, 1, pin_chan), {alu_write}));
}

TEST_F(InstrTest, test_alu_op3_comp)
{
   auto r128x = new Register(128, 0, pin_none);
   auto r128y = new Register(128, 1, pin_none);
   auto r128z = new Register(128, 2, pin_none);

   AluInstr alu1(op3_muladd, r128z, r128x, r128y, r128y, {alu_write});

   EXPECT_NE(
      alu1,
      AluInstr(
         op3_muladd, r128z, r128x, r128y, new Register(129, 2, pin_none), {alu_write}));
   EXPECT_NE(
      alu1,
      AluInstr(
         op3_muladd, r128z, r128x, r128y, new Register(128, 0, pin_none), {alu_write}));
   EXPECT_NE(
      alu1,
      AluInstr(
         op3_muladd, r128z, r128x, r128y, new Register(128, 1, pin_chan), {alu_write}));
}

TEST_F(InstrTest, test_alu_op3_ne)
{
   auto R130x = new Register(130, 0, pin_none);
   auto R130y = new Register(130, 1, pin_none);
   auto R130z = new Register(130, 2, pin_none);
   auto R131z = new Register(131, 2, pin_none);
   auto R131w = new Register(131, 3, pin_none);

   AluInstr alu(op3_cnde, R130x, R130y, R131z, R131w, {alu_write, alu_last_instr});

   EXPECT_NE(
      alu, AluInstr(op3_muladd, R130x, R130y, R131z, R131w, {alu_write, alu_last_instr}));

   EXPECT_NE(alu,
             AluInstr(op3_cnde, R130z, R130y, R131z, R131w, {alu_write, alu_last_instr}));
   EXPECT_NE(alu,
             AluInstr(op3_cnde, R130x, R130z, R131z, R131w, {alu_write, alu_last_instr}));
   EXPECT_NE(alu,
             AluInstr(op3_cnde, R130x, R130y, R130z, R131w, {alu_write, alu_last_instr}));
   EXPECT_NE(alu,
             AluInstr(op3_cnde, R130x, R130y, R131z, R130z, {alu_write, alu_last_instr}));
   EXPECT_NE(alu, AluInstr(op3_cnde, R130x, R130y, R131z, R131w, {alu_write}));

   AluInstr alu_cf_changes = alu;
   alu_cf_changes.set_cf_type(cf_alu_push_before);

   EXPECT_NE(alu, alu_cf_changes);

   AluInstr alu_bs_changes = alu;
   alu_bs_changes.set_bank_swizzle(alu_vec_021);

   EXPECT_NE(alu, alu_bs_changes);
};

TEST_F(InstrTest, test_alu_op1_ne)
{
   auto R130x = new Register(130, 0, pin_none);
   auto R130y = new Register(130, 1, pin_none);
   auto R130z = new Register(130, 2, pin_none);

   AluInstr alu(op1_mov, R130x, R130y, {alu_write, alu_last_instr});

   EXPECT_NE(alu, AluInstr(op1_cos, R130x, R130y, {alu_write, alu_last_instr}));

   EXPECT_NE(alu, AluInstr(op1_mov, R130z, R130y, {alu_write, alu_last_instr}));
   EXPECT_NE(alu, AluInstr(op1_mov, R130x, R130z, {alu_write, alu_last_instr}));
   EXPECT_NE(alu, AluInstr(op1_mov, R130x, R130y, {alu_last_instr}));

   AluInstr alu_cf_changes = alu;
   alu_cf_changes.set_cf_type(cf_alu_push_before);

   EXPECT_NE(alu, alu_cf_changes);

   AluInstr alu_bs_changes = alu;
   alu_bs_changes.set_bank_swizzle(alu_vec_021);

   EXPECT_NE(alu, alu_bs_changes);
};

TEST_F(InstrTest, test_alu_dot4_grouped)
{
   auto R130x = new Register(130, 0, pin_none);
   auto R130y = new Register(130, 1, pin_none);
   auto R130z = new Register(130, 2, pin_none);
   auto R130w = new Register(130, 3, pin_none);

   auto R131x = new Register(131, 0, pin_none);
   auto R131y = new Register(131, 1, pin_none);
   auto R131z = new Register(131, 2, pin_none);
   auto R131w = new Register(131, 3, pin_none);

   auto R132x = new Register(132, 0, pin_chan);
   auto R132y = new Register(132, 1, pin_chan);
   auto R132z = new Register(132, 2, pin_chan);
   auto R132w = new Register(132, 3, pin_chan);

   AluInstr::SrcValues src({R130x, R130y, R130z, R130w, R131x, R131y, R131z, R131w});

   AluInstr alu(op2_dot4_ieee, R132x, src, {alu_write, alu_last_instr}, 4);

   EXPECT_NE(alu, AluInstr(op1_cos, R130x, R130y, {alu_write, alu_last_instr}));
   EXPECT_EQ(alu, alu);

   ValueFactory vf;
   auto group = alu.split(vf);
   group->fix_last_flag();
   ASSERT_TRUE(group);

   auto i = group->begin();
   EXPECT_NE(i, group->end());
   ASSERT_TRUE(*i);
   check(**i, AluInstr(op2_dot4_ieee, R132x, R130x, R130y, {alu_write}));
   ++i;
   EXPECT_NE(i, group->end());
   ASSERT_TRUE(*i);
   check(**i, AluInstr(op2_dot4_ieee, R132y, R130z, R130w, {}));
   ++i;
   EXPECT_NE(i, group->end());
   ASSERT_TRUE(*i);
   check(**i, AluInstr(op2_dot4_ieee, R132z, R131x, R131y, {}));
   ++i;
   EXPECT_NE(i, group->end());
   ASSERT_TRUE(*i);
   check(**i, AluInstr(op2_dot4_ieee, R132w, R131z, R131w, {alu_last_instr}));
   ++i;
   EXPECT_NE(i, group->end());
   ASSERT_FALSE(*i);
   ++i;
   EXPECT_EQ(i, group->end());
};

#ifdef __cpp_exceptions
TEST_F(InstrTest, test_alu_wrong_source_count)
{
   EXPECT_THROW(AluInstr(op3_cnde,
                         new Register(130, 1, pin_none),
                         new Register(129, 2, pin_chan),
                         new Register(129, 3, pin_none),
                         {alu_write, alu_last_instr}),
                std::invalid_argument);

   EXPECT_THROW(AluInstr(op3_cnde,
                         new Register(130, 1, pin_none),
                         new Register(129, 2, pin_chan),
                         {alu_write, alu_last_instr}),
                std::invalid_argument);

   EXPECT_THROW(AluInstr(op1_mov,
                         new Register(130, 1, pin_none),
                         new Register(129, 2, pin_chan),
                         new Register(129, 2, pin_chan),
                         {alu_write, alu_last_instr}),
                std::invalid_argument);

   EXPECT_THROW(AluInstr(op2_add,
                         new Register(130, 1, pin_none),
                         new Register(129, 2, pin_chan),
                         {alu_write, alu_last_instr}),
                std::invalid_argument);

   EXPECT_THROW(AluInstr(op2_add,
                         new Register(130, 1, pin_none),
                         new Register(129, 2, pin_chan),
                         new Register(129, 2, pin_chan),
                         new Register(129, 2, pin_chan),
                         {alu_write, alu_last_instr}),
                std::invalid_argument);
}

TEST_F(InstrTest, test_alu_write_no_dest)
{
   EXPECT_THROW(AluInstr(op2_add,
                         nullptr,
                         new Register(129, 2, pin_chan),
                         new Register(129, 2, pin_chan),
                         {alu_write, alu_last_instr}),
                std::invalid_argument);
}

#endif

TEST_F(InstrTest, test_tex_basic)
{
   TexInstr tex(
      TexInstr::sample, RegisterVec4(129), {0, 1, 2, 3}, RegisterVec4(130), 17, nullptr, 1);

   EXPECT_EQ(tex.opcode(), TexInstr::sample);

   auto& dst = tex.dst();
   auto& src = tex.src();

   for (int i = 0; i < 4; ++i) {
      EXPECT_EQ(*dst[i], Register(129, i, pin_group));
      EXPECT_EQ(*src[i], Register(130, i, pin_group));
      EXPECT_EQ(tex.dest_swizzle(i), i);
   }

   EXPECT_EQ(tex.resource_id(), 17);
   EXPECT_EQ(tex.sampler_id(), 1);

   EXPECT_TRUE(tex.end_group());

   for (int i = 0; i < 3; ++i)
      EXPECT_EQ(tex.get_offset(i), 0);

   EXPECT_FALSE(tex.has_tex_flag(TexInstr::x_unnormalized));
   EXPECT_FALSE(tex.has_tex_flag(TexInstr::y_unnormalized));
   EXPECT_FALSE(tex.has_tex_flag(TexInstr::z_unnormalized));
   EXPECT_FALSE(tex.has_tex_flag(TexInstr::w_unnormalized));

   tex.set_tex_flag(TexInstr::x_unnormalized);
   EXPECT_TRUE(tex.has_tex_flag(TexInstr::x_unnormalized));
   EXPECT_FALSE(tex.has_tex_flag(TexInstr::y_unnormalized));
   EXPECT_FALSE(tex.has_tex_flag(TexInstr::z_unnormalized));
   EXPECT_FALSE(tex.has_tex_flag(TexInstr::w_unnormalized));

   tex.set_tex_flag(TexInstr::y_unnormalized);
   EXPECT_TRUE(tex.has_tex_flag(TexInstr::x_unnormalized));
   EXPECT_TRUE(tex.has_tex_flag(TexInstr::y_unnormalized));
   EXPECT_FALSE(tex.has_tex_flag(TexInstr::z_unnormalized));
   EXPECT_FALSE(tex.has_tex_flag(TexInstr::w_unnormalized));

   tex.set_tex_flag(TexInstr::z_unnormalized);
   tex.set_tex_flag(TexInstr::w_unnormalized);
   EXPECT_TRUE(tex.has_tex_flag(TexInstr::x_unnormalized));
   EXPECT_TRUE(tex.has_tex_flag(TexInstr::y_unnormalized));
   EXPECT_TRUE(tex.has_tex_flag(TexInstr::z_unnormalized));
   EXPECT_TRUE(tex.has_tex_flag(TexInstr::w_unnormalized));

   EXPECT_EQ(tex.inst_mode(), 0);

   EXPECT_FALSE(tex.resource_offset());

   tex.set_dest_swizzle({4, 7, 0, 1});
   EXPECT_EQ(tex.dest_swizzle(0), 4);
   EXPECT_EQ(tex.dest_swizzle(1), 7);
   EXPECT_EQ(tex.dest_swizzle(2), 0);
   EXPECT_EQ(tex.dest_swizzle(3), 1);

   tex.set_dest_swizzle({7, 2, 5, 0});
   EXPECT_EQ(tex.dest_swizzle(0), 7);
   EXPECT_EQ(tex.dest_swizzle(1), 2);
   EXPECT_EQ(tex.dest_swizzle(2), 5);
   EXPECT_EQ(tex.dest_swizzle(3), 0);

   tex.set_offset(0, 2);
   tex.set_offset(1, -1);
   tex.set_offset(2, 3);

   EXPECT_EQ(tex.get_offset(0), 4);
   EXPECT_EQ(tex.get_offset(1), -2);
   EXPECT_EQ(tex.get_offset(2), 6);
}

TEST_F(InstrTest, test_tex_gather4)
{
   TexInstr tex(
      TexInstr::gather4, RegisterVec4(131), {0, 1, 2, 3}, RegisterVec4(132), 2, nullptr, 19);

   EXPECT_EQ(tex.opcode(), TexInstr::gather4);

   auto& dst = tex.dst();
   auto& src = tex.src();

   for (int i = 0; i < 4; ++i) {
      EXPECT_EQ(*dst[i], Register(131, i, pin_group));
      EXPECT_EQ(*src[i], Register(132, i, pin_group));
      EXPECT_EQ(tex.dest_swizzle(i), i);
   }

   EXPECT_EQ(tex.resource_id(), 2);
   EXPECT_EQ(tex.sampler_id(), 19);

   for (int i = 0; i < 3; ++i)
      EXPECT_EQ(tex.get_offset(i), 0);

   EXPECT_FALSE(tex.has_tex_flag(TexInstr::x_unnormalized));
   EXPECT_FALSE(tex.has_tex_flag(TexInstr::y_unnormalized));
   EXPECT_FALSE(tex.has_tex_flag(TexInstr::z_unnormalized));
   EXPECT_FALSE(tex.has_tex_flag(TexInstr::w_unnormalized));

   tex.set_gather_comp(2);
   EXPECT_EQ(tex.inst_mode(), 2);
}

TEST_F(InstrTest, test_tex_neq)
{
   TexInstr tex_ref(
      TexInstr::sample, RegisterVec4(129), {0, 1, 2, 3}, RegisterVec4(130), 1, nullptr, 17);
   EXPECT_EQ(tex_ref, tex_ref);

   EXPECT_NE(
      tex_ref,
      TexInstr(
         TexInstr::sample_c, RegisterVec4(129), {0, 1, 2, 3}, RegisterVec4(130), 1, nullptr, 17));
   EXPECT_NE(
      tex_ref,
      TexInstr(
         TexInstr::sample, RegisterVec4(130), {0, 1, 2, 3}, RegisterVec4(130), 1, nullptr, 17));
   EXPECT_NE(
      tex_ref,
      TexInstr(
         TexInstr::sample, RegisterVec4(130), {0, 1, 2, 3}, RegisterVec4(130), 1, nullptr, 17));

   EXPECT_NE(
      tex_ref,
      TexInstr(
         TexInstr::sample, RegisterVec4(129), {7, 1, 2, 3}, RegisterVec4(130), 1, nullptr, 17));
   EXPECT_NE(
      tex_ref,
      TexInstr(
         TexInstr::sample, RegisterVec4(129), {0, 7, 2, 3}, RegisterVec4(130), 1, nullptr, 17));
   EXPECT_NE(
      tex_ref,
      TexInstr(
         TexInstr::sample, RegisterVec4(129), {0, 1, 7, 3}, RegisterVec4(130), 1, nullptr, 17));
   EXPECT_NE(
      tex_ref,
      TexInstr(
         TexInstr::sample, RegisterVec4(129), {0, 1, 2, 7}, RegisterVec4(130), 1, nullptr, 17));

   EXPECT_NE(tex_ref,
             TexInstr(TexInstr::sample,
                      RegisterVec4(129),
                      {0, 1, 2, 3},
                      RegisterVec4(130, false, {7, 1, 2, 3}),
                      1,
                      nullptr,
                      17));
   EXPECT_NE(tex_ref,
             TexInstr(TexInstr::sample,
                      RegisterVec4(129),
                      {0, 1, 2, 3},
                      RegisterVec4(130, false, {0, 7, 2, 3}),
                      1,
                      nullptr,
                      17));
   EXPECT_NE(tex_ref,
             TexInstr(TexInstr::sample,
                      RegisterVec4(129),
                      {0, 1, 2, 3},
                      RegisterVec4(130, false, {0, 1, 7, 3}),
                      1,
                      nullptr,
                      17));
   EXPECT_NE(tex_ref,
             TexInstr(TexInstr::sample,
                      RegisterVec4(129),
                      {0, 1, 2, 3},
                      RegisterVec4(130, false, {0, 1, 2, 7}),
                      1,
                      nullptr,
                      17));

   EXPECT_NE(
      tex_ref,
      TexInstr(
         TexInstr::sample, RegisterVec4(129), {0, 1, 2, 3}, RegisterVec4(130), 2, nullptr, 17));
   EXPECT_NE(
      tex_ref,
      TexInstr(
         TexInstr::sample, RegisterVec4(129), {0, 1, 2, 3}, RegisterVec4(130), 1, nullptr, 18));

   /*
   auto tex_with_sampler_offset = tex_ref;
   tex_with_sampler_offset.set_sampler_offset(new LiteralConstant( 2));
   EXPECT_NE(tex_ref, tex_with_sampler_offset);

   auto tex_cmp1 = tex_ref;
   EXPECT_EQ(tex_ref, tex_cmp1);

   tex_cmp1.set_tex_flag(TexInstr::x_unnormalized); EXPECT_NE(tex_ref, tex_cmp1);
   auto tex_cmp2 = tex_ref; tex_cmp2.set_tex_flag(TexInstr::y_unnormalized);
   EXPECT_NE(tex_ref, tex_cmp2); auto tex_cmp3 = tex_ref;
   tex_cmp3.set_tex_flag(TexInstr::z_unnormalized); EXPECT_NE(tex_ref, tex_cmp3); auto
   tex_cmp4 = tex_ref; tex_cmp4.set_tex_flag(TexInstr::w_unnormalized); EXPECT_NE(tex_ref,
   tex_cmp4);

   for (int i = 0; i < 3; ++i) {
      auto tex_ofs = tex_ref;
      tex_ofs.set_offset(i, 1);
      EXPECT_NE(tex_ref, tex_ofs);
   }

   for (int i = 0; i < 4; ++i) {
      auto tex_swz = tex_ref;
      RegisterVec4::Swizzle dst_swz = {0,1,2,3};
      dst_swz[i] = 7;
      tex_swz.set_dest_swizzle(dst_swz);
      EXPECT_NE(tex_ref, tex_swz);
   }

   auto tex_cmp_mode = tex_ref;
   tex_cmp_mode.set_inst_mode(1);
   EXPECT_NE(tex_ref, tex_cmp_mode);*/
}

TEST_F(InstrTest, test_export_basic)
{
   ExportInstr exp0(ExportInstr::param, 60, RegisterVec4(200));

   EXPECT_EQ(exp0.export_type(), ExportInstr::param);
   EXPECT_EQ(exp0.location(), 60);
   EXPECT_EQ(exp0.value(), RegisterVec4(200));
   EXPECT_FALSE(exp0.is_last_export());

   ExportInstr exp1(ExportInstr::param, 60, RegisterVec4(200));
   exp1.set_is_last_export(true);
   EXPECT_TRUE(exp1.is_last_export());

   EXPECT_EQ(exp0, exp0);
   EXPECT_NE(exp0, exp1);

   ExportInstr exp2(ExportInstr::pos, 60, RegisterVec4(200));
   EXPECT_EQ(exp2.export_type(), ExportInstr::pos);
   EXPECT_NE(exp0, exp2);

   ExportInstr exp3(ExportInstr::param, 61, RegisterVec4(200));
   EXPECT_EQ(exp3.location(), 61);
   EXPECT_NE(exp0, exp3);

   ExportInstr exp4(ExportInstr::param, 60, RegisterVec4(201));
   EXPECT_EQ(exp4.value(), RegisterVec4(201));
   EXPECT_NE(exp0, exp4);

   EXPECT_NE(exp0,
             ExportInstr(ExportInstr::param, 60, RegisterVec4(200, false, {7, 1, 2, 3})));
   EXPECT_NE(exp0,
             ExportInstr(ExportInstr::param, 60, RegisterVec4(200, false, {0, 7, 2, 3})));
   EXPECT_NE(exp0,
             ExportInstr(ExportInstr::param, 60, RegisterVec4(200, false, {0, 1, 7, 3})));
   EXPECT_NE(exp0,
             ExportInstr(ExportInstr::param, 60, RegisterVec4(200, false, {0, 1, 2, 7})));
}

TEST_F(InstrTest, test_fetch_basic)
{
   FetchInstr fetch(vc_fetch,
                    RegisterVec4(200),
                    {0, 2, 1, 3},
                    new Register(201, 2, pin_none),
                    0,
                    vertex_data,
                    fmt_8,
                    vtx_nf_norm,
                    vtx_es_none,
                    1,
                    nullptr);

   EXPECT_EQ(fetch.opcode(), vc_fetch);
   EXPECT_EQ(fetch.dst(), RegisterVec4(200));
   EXPECT_EQ(fetch.dest_swizzle(0), 0);
   EXPECT_EQ(fetch.dest_swizzle(1), 2);
   EXPECT_EQ(fetch.dest_swizzle(2), 1);
   EXPECT_EQ(fetch.dest_swizzle(3), 3);

   EXPECT_EQ(fetch.src(), Register(201, 2, pin_none));
   EXPECT_EQ(fetch.src_offset(), 0);

   EXPECT_EQ(fetch.resource_id(), 1);
   EXPECT_FALSE(fetch.resource_offset());

   EXPECT_EQ(fetch.fetch_type(), vertex_data);
   EXPECT_EQ(fetch.data_format(), fmt_8);
   EXPECT_EQ(fetch.num_format(), vtx_nf_norm);
   EXPECT_EQ(fetch.endian_swap(), vtx_es_none);

   EXPECT_EQ(fetch.mega_fetch_count(), 0);
   EXPECT_EQ(fetch.array_base(), 0);
   EXPECT_EQ(fetch.array_size(), 0);
   EXPECT_EQ(fetch.elm_size(), 0);

   for (int i = 0; i < FetchInstr::unknown; ++i) {
      EXPECT_FALSE(fetch.has_fetch_flag(static_cast<FetchInstr::EFlags>(i)));
   }

   EXPECT_NE(fetch,
             FetchInstr(vc_get_buf_resinfo,
                        RegisterVec4(200),
                        {0, 2, 1, 3},
                        new Register(201, 2, pin_none),
                        0,
                        vertex_data,
                        fmt_8,
                        vtx_nf_norm,
                        vtx_es_none,
                        1,
                        nullptr));

   EXPECT_NE(fetch,
             FetchInstr(vc_fetch,
                        RegisterVec4(201),
                        {0, 2, 1, 3},
                        new Register(201, 2, pin_none),
                        0,
                        vertex_data,
                        fmt_8,
                        vtx_nf_norm,
                        vtx_es_none,
                        1,
                        nullptr));

   EXPECT_NE(fetch,
             FetchInstr(vc_fetch,
                        RegisterVec4(200),
                        {1, 2, 0, 3},
                        new Register(201, 2, pin_none),
                        0,
                        vertex_data,
                        fmt_8,
                        vtx_nf_norm,
                        vtx_es_none,
                        1,
                        nullptr));

   EXPECT_NE(fetch,
             FetchInstr(vc_fetch,
                        RegisterVec4(200),
                        {0, 2, 1, 3},
                        new Register(200, 2, pin_none),
                        0,
                        vertex_data,
                        fmt_8,
                        vtx_nf_norm,
                        vtx_es_none,
                        1,
                        nullptr));

   EXPECT_NE(fetch,
             FetchInstr(vc_fetch,
                        RegisterVec4(200),
                        {0, 2, 1, 3},
                        new Register(201, 2, pin_none),
                        8,
                        vertex_data,
                        fmt_8,
                        vtx_nf_norm,
                        vtx_es_none,
                        1,
                        nullptr));

   EXPECT_NE(fetch,
             FetchInstr(vc_fetch,
                        RegisterVec4(200),
                        {0, 2, 1, 3},
                        new Register(201, 2, pin_none),
                        0,
                        instance_data,
                        fmt_8,
                        vtx_nf_norm,
                        vtx_es_none,
                        1,
                        nullptr));

   EXPECT_NE(fetch,
             FetchInstr(vc_fetch,
                        RegisterVec4(200),
                        {0, 2, 1, 3},
                        new Register(201, 2, pin_none),
                        0,
                        vertex_data,
                        fmt_8_8,
                        vtx_nf_norm,
                        vtx_es_none,
                        1,
                        nullptr));

   EXPECT_NE(fetch,
             FetchInstr(vc_fetch,
                        RegisterVec4(200),
                        {0, 2, 1, 3},
                        new Register(201, 2, pin_none),
                        0,
                        vertex_data,
                        fmt_8,
                        vtx_nf_int,
                        vtx_es_none,
                        1,
                        nullptr));

   EXPECT_NE(fetch,
             FetchInstr(vc_fetch,
                        RegisterVec4(200),
                        {0, 2, 1, 3},
                        new Register(201, 2, pin_none),
                        0,
                        vertex_data,
                        fmt_8,
                        vtx_nf_norm,
                        vtx_es_8in16,
                        1,
                        nullptr));

   EXPECT_NE(fetch,
             FetchInstr(vc_fetch,
                        RegisterVec4(200),
                        {0, 2, 1, 3},
                        new Register(201, 2, pin_none),
                        0,
                        vertex_data,
                        fmt_8,
                        vtx_nf_norm,
                        vtx_es_none,
                        2,
                        nullptr));

   EXPECT_NE(fetch,
             FetchInstr(vc_fetch,
                        RegisterVec4(200),
                        {0, 2, 1, 3},
                        new Register(201, 2, pin_none),
                        0,
                        vertex_data,
                        fmt_8,
                        vtx_nf_norm,
                        vtx_es_none,
                        1,
                        new Register(1000, 0, pin_none)));

   auto fetch1 = fetch;
   fetch1.set_mfc(31);
   EXPECT_NE(fetch1, fetch);
   EXPECT_EQ(fetch1.mega_fetch_count(), 31);
   EXPECT_TRUE(
      fetch1.has_fetch_flag(static_cast<FetchInstr::EFlags>(FetchInstr::is_mega_fetch)));

   auto fetch2 = fetch;
   fetch2.set_array_base(32);
   EXPECT_NE(fetch, fetch2);
   EXPECT_EQ(fetch2.array_base(), 32);

   auto fetch3 = fetch;
   fetch3.set_array_size(16);
   EXPECT_NE(fetch, fetch3);
   EXPECT_EQ(fetch3.array_size(), 16);

   auto fetch4 = fetch;
   fetch4.set_element_size(3);
   EXPECT_NE(fetch, fetch4);
   EXPECT_EQ(fetch4.elm_size(), 3);
}

TEST_F(InstrTest, test_fetch_basic2)
{
   FetchInstr fetch(vc_get_buf_resinfo,
                    RegisterVec4(201),
                    {0, 1, 3, 4},
                    new Register(202, 3, pin_none),
                    1,
                    no_index_offset,
                    fmt_32_32,
                    vtx_nf_int,
                    vtx_es_8in16,
                    3,
                    new Register(300, 1, pin_none));

   EXPECT_EQ(fetch.opcode(), vc_get_buf_resinfo);
   EXPECT_EQ(fetch.dst(), RegisterVec4(201));
   EXPECT_EQ(fetch.dest_swizzle(0), 0);
   EXPECT_EQ(fetch.dest_swizzle(1), 1);
   EXPECT_EQ(fetch.dest_swizzle(2), 3);
   EXPECT_EQ(fetch.dest_swizzle(3), 4);

   EXPECT_EQ(fetch.src(), Register(202, 3, pin_none));
   EXPECT_EQ(fetch.src_offset(), 1);

   EXPECT_EQ(fetch.resource_id(), 3);
   EXPECT_EQ(*fetch.resource_offset(), Register(300, 1, pin_none));

   EXPECT_EQ(fetch.fetch_type(), no_index_offset);
   EXPECT_EQ(fetch.data_format(), fmt_32_32);
   EXPECT_EQ(fetch.num_format(), vtx_nf_int);
   EXPECT_EQ(fetch.endian_swap(), vtx_es_8in16);

   EXPECT_EQ(fetch.mega_fetch_count(), 0);
   EXPECT_EQ(fetch.array_base(), 0);
   EXPECT_EQ(fetch.array_size(), 0);
   EXPECT_EQ(fetch.elm_size(), 0);

   for (int i = 0; i < FetchInstr::unknown; ++i) {
      EXPECT_FALSE(fetch.has_fetch_flag(static_cast<FetchInstr::EFlags>(i)));
   }

   auto fetch1 = fetch;
   fetch1.set_mfc(15);
   EXPECT_NE(fetch1, fetch);
   EXPECT_EQ(fetch1.mega_fetch_count(), 15);
   EXPECT_TRUE(
      fetch1.has_fetch_flag(static_cast<FetchInstr::EFlags>(FetchInstr::is_mega_fetch)));

   auto fetch2 = fetch;
   fetch2.set_array_base(128);
   EXPECT_NE(fetch, fetch2);
   EXPECT_EQ(fetch2.array_base(), 128);

   auto fetch3 = fetch;
   fetch3.set_array_size(8);
   EXPECT_NE(fetch, fetch3);
   EXPECT_EQ(fetch3.array_size(), 8);

   auto fetch4 = fetch;
   fetch4.set_element_size(1);
   EXPECT_NE(fetch, fetch4);
   EXPECT_EQ(fetch4.elm_size(), 1);
}
