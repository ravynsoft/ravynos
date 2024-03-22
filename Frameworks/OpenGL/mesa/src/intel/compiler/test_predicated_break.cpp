/*
 * Copyright (c) 2023 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include "brw_fs.h"
#include "brw_fs_builder.h"
#include "brw_cfg.h"

using namespace brw;

class PredicatedBreakTest : public ::testing::Test {
   virtual void SetUp();
   virtual void TearDown();

public:
   bool debug;
   void *mem_ctx;
   brw_compiler compiler;
   brw_compile_params params;
   intel_device_info devinfo;
   struct brw_wm_prog_data prog_data;
   struct gl_shader_program *shader_prog;

   fs_visitor *shader_a;
   fs_visitor *shader_b;

   bool opt_predicated_break(fs_visitor *s);
};

void
PredicatedBreakTest::SetUp()
{
   debug = getenv("TEST_DEBUG");

   mem_ctx = ralloc_context(NULL);

   devinfo = {};
   devinfo.ver = 9;
   devinfo.verx10 = 90;

   compiler = {};
   compiler.devinfo = &devinfo;
   brw_init_isa_info(&compiler.isa, &devinfo);

   params = {};
   params.mem_ctx = mem_ctx;

   prog_data = {};
   nir_shader *nir =
      nir_shader_create(mem_ctx, MESA_SHADER_FRAGMENT, NULL, NULL);

   shader_a = new fs_visitor(&compiler, &params, NULL,
                             &prog_data.base, nir, 8, false, false);

   shader_b = new fs_visitor(&compiler, &params, NULL,
                             &prog_data.base, nir, 8, false, false);
}

void
PredicatedBreakTest::TearDown()
{
   delete shader_a;
   delete shader_b;
   ralloc_free(mem_ctx);
   mem_ctx = NULL;
}

bool
PredicatedBreakTest::opt_predicated_break(fs_visitor *s)
{
   const bool print = getenv("TEST_DEBUG");

   if (print) {
      fprintf(stderr, "= Before =\n");
      s->cfg->dump();
   }

   bool ret = ::opt_predicated_break(s);

   if (print) {
      fprintf(stderr, "\n= After =\n");
      s->cfg->dump();
   }

   return ret;
}

static fs_builder
make_builder(fs_visitor *s)
{
   return fs_builder(s, s->dispatch_width).at_end();
}

static testing::AssertionResult
shaders_match(const char *a_expr, const char *b_expr,
              fs_visitor *a, fs_visitor *b)
{
   /* Using the CFG string dump for this.  Not ideal but it is
    * convenient that covers some CFG information, helping to
    * check if the optimization is keeping the CFG valid.
    */

   char *a_str = NULL;
   size_t a_size = 0;
   FILE *a_file = open_memstream(&a_str, &a_size);
   a->cfg->dump(a_file);
   fclose(a_file);

   char *b_str = NULL;
   size_t b_size = 0;
   FILE *b_file = open_memstream(&b_str, &b_size);
   b->cfg->dump(b_file);
   fclose(b_file);

   if (a_size != b_size || strcmp(a_str, b_str) != 0) {
      std::stringstream result;

      result << "Shaders don't match.\n\n"
             << a_expr << " is:\n\n" << a_str
             << "\n---\n"
             << b_expr << " is:\n\n" << b_str
             << "\n";

      free(a_str);
      free(b_str);
      return testing::AssertionFailure() << result.str();
   }

   free(a_str);
   free(b_str);
   return testing::AssertionSuccess();
}

#define ASSERT_SHADERS_MATCH(a, b)  ASSERT_PRED_FORMAT2(shaders_match, a, b)

TEST_F(PredicatedBreakTest, TopBreakWithoutContinue)
{
   fs_builder a = make_builder(shader_a);
   fs_builder b = make_builder(shader_b);

   fs_reg r1 = brw_vec8_grf(1, 0);
   fs_reg r2 = brw_vec8_grf(2, 0);
   fs_reg r3 = brw_vec8_grf(3, 0);

   a.DO();
   a.CMP(r1, r2, r3, BRW_CONDITIONAL_NZ);
   a.IF(BRW_PREDICATE_NORMAL);
   a.BREAK();
   a.ENDIF();
   a.ADD(r1, r2, r3);
   a.WHILE();
   a.NOP();  /* There's always going to be something after a WHILE. */
   shader_a->calculate_cfg();

   /* The IF/ENDIF around the BREAK is expected to be removed. */
   bool progress = opt_predicated_break(shader_a);
   EXPECT_TRUE(progress);

   b.DO();
   b.CMP(r1, r2, r3, BRW_CONDITIONAL_NZ);
   b.BREAK()->predicate = BRW_PREDICATE_NORMAL;
   b.ADD(r1, r2, r3);
   b.WHILE();
   b.NOP();
   shader_b->calculate_cfg();

   ASSERT_SHADERS_MATCH(shader_a, shader_b);
}

TEST_F(PredicatedBreakTest, TopBreakWithContinue)
{
   fs_builder a = make_builder(shader_a);
   fs_builder b = make_builder(shader_b);

   fs_reg r1 = brw_vec8_grf(1, 0);
   fs_reg r2 = brw_vec8_grf(2, 0);
   fs_reg r3 = brw_vec8_grf(3, 0);

   a.DO();
   a.CMP(r1, r2, r3, BRW_CONDITIONAL_NZ);
   a.IF(BRW_PREDICATE_NORMAL);
   a.BREAK();
   a.ENDIF();
   a.ADD(r1, r2, r3);
   a.CMP(r1, r2, r3, BRW_CONDITIONAL_GE);
   a.IF(BRW_PREDICATE_NORMAL);
   a.CONTINUE();
   a.ENDIF();
   a.MUL(r1, r2, r3);
   a.WHILE();
   a.NOP();  /* There's always going to be something after a WHILE. */
   shader_a->calculate_cfg();

   /* The IF/ENDIF around the BREAK and the CONTINUE are expected to be
    * removed.
    */
   bool progress = opt_predicated_break(shader_a);
   EXPECT_TRUE(progress);

   b.DO();
   b.CMP(r1, r2, r3, BRW_CONDITIONAL_NZ);
   b.BREAK()->predicate = BRW_PREDICATE_NORMAL;
   b.ADD(r1, r2, r3);
   b.CMP(r1, r2, r3, BRW_CONDITIONAL_GE);
   b.CONTINUE()->predicate = BRW_PREDICATE_NORMAL;
   b.MUL(r1, r2, r3);
   b.WHILE();
   b.NOP();
   shader_b->calculate_cfg();

   ASSERT_SHADERS_MATCH(shader_a, shader_b);
}

TEST_F(PredicatedBreakTest, DISABLED_BottomBreakWithoutContinue)
{
   fs_builder a = make_builder(shader_a);
   fs_builder b = make_builder(shader_b);

   fs_reg r1 = brw_vec8_grf(1, 0);
   fs_reg r2 = brw_vec8_grf(2, 0);
   fs_reg r3 = brw_vec8_grf(3, 0);

   a.DO();
   a.ADD(r1, r2, r3);
   a.CMP(r1, r2, r3, BRW_CONDITIONAL_NZ);
   a.IF(BRW_PREDICATE_NORMAL);
   a.BREAK();
   a.ENDIF();
   a.WHILE();
   a.NOP();  /* There's always going to be something after a WHILE. */
   shader_a->calculate_cfg();

   /* BREAK is the only way to exit the loop, so expect to remove the
    * IF/BREAK/ENDIF and add a predicate to WHILE.
    */
   bool progress = opt_predicated_break(shader_a);
   EXPECT_TRUE(progress);

   b.DO();
   b.ADD(r1, r2, r3);
   b.CMP(r1, r2, r3, BRW_CONDITIONAL_NZ);
   auto w = b.WHILE();
   w->predicate = BRW_PREDICATE_NORMAL;
   w->predicate_inverse = true;
   b.NOP();
   shader_b->calculate_cfg();

   ASSERT_SHADERS_MATCH(shader_a, shader_b);
}


TEST_F(PredicatedBreakTest, BottomBreakWithContinue)
{
   fs_builder a = make_builder(shader_a);
   fs_builder b = make_builder(shader_b);

   fs_reg r1 = brw_vec8_grf(1, 0);
   fs_reg r2 = brw_vec8_grf(2, 0);
   fs_reg r3 = brw_vec8_grf(3, 0);

   a.DO();
   a.ADD(r1, r2, r3);
   a.CMP(r1, r2, r3, BRW_CONDITIONAL_GE);
   a.IF(BRW_PREDICATE_NORMAL);
   a.CONTINUE();
   a.ENDIF();
   a.MUL(r1, r2, r3);
   a.CMP(r1, r2, r3, BRW_CONDITIONAL_NZ);
   a.IF(BRW_PREDICATE_NORMAL);
   a.BREAK();
   a.ENDIF();
   a.WHILE();
   a.NOP();  /* There's always going to be something after a WHILE. */
   shader_a->calculate_cfg();

   /* With a CONTINUE, the BREAK can't be removed, but still remove the
    * IF/ENDIF around both of them.
    */
   bool progress = opt_predicated_break(shader_a);
   EXPECT_TRUE(progress);

   b.DO();
   b.ADD(r1, r2, r3);
   b.CMP(r1, r2, r3, BRW_CONDITIONAL_GE);
   b.CONTINUE()->predicate = BRW_PREDICATE_NORMAL;
   b.MUL(r1, r2, r3);
   b.CMP(r1, r2, r3, BRW_CONDITIONAL_NZ);
   b.BREAK()->predicate = BRW_PREDICATE_NORMAL;
   b.WHILE();
   b.NOP();
   shader_b->calculate_cfg();

   ASSERT_SHADERS_MATCH(shader_a, shader_b);
}

TEST_F(PredicatedBreakTest, TwoBreaks)
{
   fs_builder a = make_builder(shader_a);
   fs_builder b = make_builder(shader_b);

   fs_reg r1 = brw_vec8_grf(1, 0);
   fs_reg r2 = brw_vec8_grf(2, 0);
   fs_reg r3 = brw_vec8_grf(3, 0);

   a.DO();
   a.ADD(r1, r2, r3);
   a.CMP(r1, r2, r3, BRW_CONDITIONAL_NZ);
   a.IF(BRW_PREDICATE_NORMAL);
   a.BREAK();
   a.ENDIF();
   a.MUL(r1, r2, r3);
   a.CMP(r1, r2, r3, BRW_CONDITIONAL_GE);
   a.IF(BRW_PREDICATE_NORMAL);
   a.BREAK();
   a.ENDIF();
   a.AND(r1, r2, r3);
   a.WHILE();
   a.NOP();  /* There's always going to be something after a WHILE. */
   shader_a->calculate_cfg();

   /* The IF/ENDIF around the breaks are expected to be removed. */
   bool progress = opt_predicated_break(shader_a);
   EXPECT_TRUE(progress);

   b.DO();
   b.ADD(r1, r2, r3);
   b.CMP(r1, r2, r3, BRW_CONDITIONAL_NZ);
   b.BREAK()->predicate = BRW_PREDICATE_NORMAL;
   b.MUL(r1, r2, r3);
   b.CMP(r1, r2, r3, BRW_CONDITIONAL_GE);
   b.BREAK()->predicate = BRW_PREDICATE_NORMAL;
   b.AND(r1, r2, r3);
   b.WHILE();
   b.NOP();  /* There's always going to be something after a WHILE. */
   shader_b->calculate_cfg();

   ASSERT_SHADERS_MATCH(shader_a, shader_b);
}
