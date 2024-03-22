/*
 * Copyright © 2015 Intel Corporation
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
 *
 * Based on test_fs_cmod_propagation.cpp
 */

#include <gtest/gtest.h>
#include "brw_vec4.h"
#include "brw_vec4_builder.h"
#include "brw_cfg.h"

using namespace brw;

class cmod_propagation_vec4_test : public ::testing::Test {
   virtual void SetUp();
   virtual void TearDown();

public:
   struct brw_compiler *compiler;
   struct brw_compile_params params;
   struct intel_device_info *devinfo;
   void *ctx;
   struct gl_shader_program *shader_prog;
   struct brw_vue_prog_data *prog_data;
   vec4_visitor *v;
};

class cmod_propagation_vec4_visitor : public vec4_visitor
{
public:
   cmod_propagation_vec4_visitor(struct brw_compiler *compiler,
                                 struct brw_compile_params *params,
                                 nir_shader *shader,
                                 struct brw_vue_prog_data *prog_data)
      : vec4_visitor(compiler, params, NULL, prog_data, shader,
                     false, false)
      {
         prog_data->dispatch_mode = DISPATCH_MODE_4X2_DUAL_OBJECT;
      }

protected:
   /* Dummy implementation for pure virtual methods */
   virtual dst_reg *make_reg_for_system_value(int /* location */)
   {
      unreachable("Not reached");
   }

   virtual void setup_payload()
   {
      unreachable("Not reached");
   }

   virtual void emit_prolog()
   {
      unreachable("Not reached");
   }

   virtual void emit_program_code()
   {
      unreachable("Not reached");
   }

   virtual void emit_thread_end()
   {
      unreachable("Not reached");
   }

   virtual void emit_urb_write_header(int /* mrf */)
   {
      unreachable("Not reached");
   }

   virtual vec4_instruction *emit_urb_write_opcode(bool /* complete */)
   {
      unreachable("Not reached");
   }
};


void cmod_propagation_vec4_test::SetUp()
{
   ctx = ralloc_context(NULL);
   compiler = rzalloc(ctx, struct brw_compiler);
   devinfo = rzalloc(ctx, struct intel_device_info);
   compiler->devinfo = devinfo;

   params = {};
   params.mem_ctx = ctx;

   prog_data = ralloc(ctx, struct brw_vue_prog_data);
   nir_shader *shader =
      nir_shader_create(ctx, MESA_SHADER_VERTEX, NULL, NULL);

   v = new cmod_propagation_vec4_visitor(compiler, &params, shader, prog_data);

   devinfo->ver = 7;
   devinfo->verx10 = devinfo->ver * 10;
}

void cmod_propagation_vec4_test::TearDown()
{
   delete v;
   v = NULL;

   ralloc_free(ctx);
   ctx = NULL;
}

static vec4_instruction *
instruction(bblock_t *block, int num)
{
   vec4_instruction *inst = (vec4_instruction *)block->start();
   for (int i = 0; i < num; i++) {
      inst = (vec4_instruction *)inst->next;
   }
   return inst;
}

static bool
cmod_propagation(vec4_visitor *v)
{
   const bool print = getenv("TEST_DEBUG");

   if (print) {
      fprintf(stderr, "= Before =\n");
      v->dump_instructions();
   }

   bool ret = v->opt_cmod_propagation();

   if (print) {
      fprintf(stderr, "\n= After =\n");
      v->dump_instructions();
   }

   return ret;
}

TEST_F(cmod_propagation_vec4_test, basic)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_float_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg src1 = src_reg(v, glsl_float_type());
   src_reg zero(brw_imm_f(0.0f));
   dst_reg dest_null = bld.null_reg_f();
   dest_null.writemask = WRITEMASK_X;

   bld.ADD(dest, src0, src1);
   bld.CMP(dest_null, src_reg(dest), zero, BRW_CONDITIONAL_GE);

   /* = Before =
    *
    * 0: add        dest.x  src0.xxxx  src1.xxxx
    * 1: cmp.ge.f0  null.x  dest.xxxx  0.0f
    *
    * = After =
    * 0: add.ge.f0  dest.x  src0.xxxx  src1.xxxx
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(0, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 0)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, basic_different_dst_writemask)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_float_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg src1 = src_reg(v, glsl_float_type());
   src_reg zero(brw_imm_f(0.0f));
   dst_reg dest_null = bld.null_reg_f();

   bld.ADD(dest, src0, src1);
   bld.CMP(dest_null, src_reg(dest), zero, BRW_CONDITIONAL_GE);

   /* = Before =
    *
    * 0: add        dest.x     src0  src1
    * 1: cmp.ge.f0  null.xyzw  dest  0.0f
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_NONE, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 1)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, andz_one)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_int_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg zero(brw_imm_f(0.0f));
   src_reg one(brw_imm_d(1));

   bld.CMP(retype(dest, BRW_REGISTER_TYPE_F), src0, zero, BRW_CONDITIONAL_L);
   set_condmod(BRW_CONDITIONAL_Z,
               bld.AND(bld.null_reg_d(), src_reg(dest), one));

   /* = Before =
    * 0: cmp.l.f0     dest:F  src0:F  0F
    * 1: and.z.f0     null:D  dest:D  1D
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_L, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_AND, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_EQ, instruction(block0, 1)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, non_cmod_instruction)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_uint_type());
   src_reg src0 = src_reg(v, glsl_uint_type());
   src_reg zero(brw_imm_ud(0u));
   bld.FBL(dest, src0);
   bld.CMP(bld.null_reg_ud(), src_reg(dest), zero, BRW_CONDITIONAL_GE);

   /* = Before =
    *
    * 0: fbl        dest  src0
    * 1: cmp.ge.f0  null  dest  0u
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_FBL, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 1)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, intervening_flag_write)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_float_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg src1 = src_reg(v, glsl_float_type());
   src_reg src2 = src_reg(v, glsl_float_type());
   src_reg zero(brw_imm_f(0.0f));
   bld.ADD(dest, src0, src1);
   bld.CMP(bld.null_reg_f(), src2, zero, BRW_CONDITIONAL_GE);
   bld.CMP(bld.null_reg_f(), src_reg(dest), zero, BRW_CONDITIONAL_GE);

   /* = Before =
    *
    * 0: add        dest  src0  src1
    * 1: cmp.ge.f0  null  src2  0.0f
    * 2: cmp.ge.f0  null  dest  0.0f
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 1)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 2)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 2)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, intervening_flag_read)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest0 = dst_reg(v, glsl_float_type());
   dst_reg dest1 = dst_reg(v, glsl_float_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg src1 = src_reg(v, glsl_float_type());
   src_reg src2 = src_reg(v, glsl_float_type());
   src_reg zero(brw_imm_f(0.0f));
   bld.ADD(dest0, src0, src1);
   set_predicate(BRW_PREDICATE_NORMAL, bld.SEL(dest1, src2, zero));
   bld.CMP(bld.null_reg_f(), src_reg(dest0), zero, BRW_CONDITIONAL_GE);

   /* = Before =
    *
    * 0: add        dest0 src0  src1
    * 1: (+f0) sel  dest1 src2  0.0f
    * 2: cmp.ge.f0  null  dest0 0.0f
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_OPCODE_SEL, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_PREDICATE_NORMAL, instruction(block0, 1)->predicate);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 2)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 2)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, intervening_dest_write)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_vec4_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg src1 = src_reg(v, glsl_float_type());
   src_reg src2 = src_reg(v, glsl_vec2_type());
   src_reg zero(brw_imm_f(0.0f));
   bld.ADD(offset(dest, 8, 2), src0, src1);
   bld.emit(SHADER_OPCODE_TEX, dest, src2)
      ->size_written = 4 * REG_SIZE;
   bld.CMP(bld.null_reg_f(), offset(src_reg(dest), 8, 2), zero, BRW_CONDITIONAL_GE);

   /* = Before =
    *
    * 0: add        dest+2  src0    src1
    * 1: tex rlen 4 dest+0  src2
    * 2: cmp.ge.f0  null    dest+2  0.0f
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_NONE, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(SHADER_OPCODE_TEX, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_NONE, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 2)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 2)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, intervening_flag_read_same_value)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest0 = dst_reg(v, glsl_float_type());
   dst_reg dest1 = dst_reg(v, glsl_float_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg src1 = src_reg(v, glsl_float_type());
   src_reg src2 = src_reg(v, glsl_float_type());
   src_reg zero(brw_imm_f(0.0f));
   dst_reg dest_null = bld.null_reg_f();
   dest_null.writemask = WRITEMASK_X;

   set_condmod(BRW_CONDITIONAL_GE, bld.ADD(dest0, src0, src1));
   set_predicate(BRW_PREDICATE_NORMAL, bld.SEL(dest1, src2, zero));
   bld.CMP(dest_null, src_reg(dest0), zero, BRW_CONDITIONAL_GE);

   /* = Before =
    *
    * 0: add.ge.f0  dest0   src0  src1
    * 1: (+f0) sel  dest1   src2  0.0f
    * 2: cmp.ge.f0  null.x  dest0 0.0f
    *
    * = After =
    * 0: add.ge.f0  dest0 src0  src1
    * 1: (+f0) sel  dest1 src2  0.0f
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_TRUE(cmod_propagation(v));
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_SEL, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_PREDICATE_NORMAL, instruction(block0, 1)->predicate);
}

TEST_F(cmod_propagation_vec4_test, negate)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_float_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg src1 = src_reg(v, glsl_float_type());
   src_reg zero(brw_imm_f(0.0f));
   bld.ADD(dest, src0, src1);
   src_reg tmp_src = src_reg(dest);
   tmp_src.negate = true;
   dst_reg dest_null = bld.null_reg_f();
   dest_null.writemask = WRITEMASK_X;
   bld.CMP(dest_null, tmp_src, zero, BRW_CONDITIONAL_GE);

   /* = Before =
    *
    * 0: add        dest     src0  src1
    * 1: cmp.ge.f0  null.x  -dest 0.0f
    *
    * = After =
    * 0: add.le.f0  dest     src0  src1
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(cmod_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(0, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_LE, instruction(block0, 0)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, movnz)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_float_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg src1 = src_reg(v, glsl_float_type());
   dst_reg dest_null = bld.null_reg_f();
   dest_null.writemask = WRITEMASK_X;

   bld.CMP(dest, src0, src1, BRW_CONDITIONAL_L);
   set_condmod(BRW_CONDITIONAL_NZ,
               bld.MOV(dest_null, src_reg(dest)));

   /* = Before =
    *
    * 0: cmp.l.f0  dest:F  src0:F  src1:F
    * 1: mov.nz.f0 null.x  dest:F
    *
    * = After =
    * 0: cmp.l.f0  dest  src0:F  src1:F
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(0, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_L, instruction(block0, 0)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, different_types_cmod_with_zero)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_int_type());
   src_reg src0 = src_reg(v, glsl_int_type());
   src_reg src1 = src_reg(v, glsl_int_type());
   src_reg zero(brw_imm_f(0.0f));
   bld.ADD(dest, src0, src1);
   bld.CMP(bld.null_reg_f(), retype(src_reg(dest), BRW_REGISTER_TYPE_F), zero,
           BRW_CONDITIONAL_GE);

   /* = Before =
    *
    * 0: add        dest:D  src0:D  src1:D
    * 1: cmp.ge.f0  null:F  dest:F  0.0f
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 1)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, andnz_non_one)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_int_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg zero(brw_imm_f(0.0f));
   src_reg nonone(brw_imm_d(38));

   bld.CMP(retype(dest, BRW_REGISTER_TYPE_F), src0, zero, BRW_CONDITIONAL_L);
   set_condmod(BRW_CONDITIONAL_NZ,
               bld.AND(bld.null_reg_d(), src_reg(dest), nonone));

   /* = Before =
    * 0: cmp.l.f0     dest:F  src0:F  0F
    * 1: and.nz.f0    null:D  dest:D  38D
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_L, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_AND, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_NZ, instruction(block0, 1)->conditional_mod);
}

/* Note that basic is using glsl_type:float types, while this one is using
 * glsl_type::vec4 */
TEST_F(cmod_propagation_vec4_test, basic_vec4)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_vec4_type());
   src_reg src0 = src_reg(v, glsl_vec4_type());
   src_reg src1 = src_reg(v, glsl_vec4_type());
   src_reg zero(brw_imm_f(0.0f));

   bld.MUL(dest, src0, src1);
   bld.CMP(bld.null_reg_f(), src_reg(dest), zero, BRW_CONDITIONAL_NZ);

   /* = Before =
    * 0: mul         dest.xyzw  src0.xyzw  src1.xyzw
    * 1: cmp.nz.f0.0 null.xyzw  dest.xyzw  0.0f
    *
    * = After =
    * 0: mul.nz.f0.0 dest.xyzw  src0.xyzw  src1.xyzw
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(0, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_MUL, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_NZ, instruction(block0, 0)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, basic_vec4_different_dst_writemask)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_vec4_type());
   dest.writemask = WRITEMASK_X;
   src_reg src0 = src_reg(v, glsl_vec4_type());
   src_reg src1 = src_reg(v, glsl_vec4_type());
   src_reg zero(brw_imm_f(0.0f));
   dst_reg dest_null = bld.null_reg_f();

   bld.MUL(dest, src0, src1);
   bld.CMP(dest_null, src_reg(dest), zero, BRW_CONDITIONAL_NZ);

   /* = Before =
    * 0: mul         dest.x  src0  src1
    * 1: cmp.nz.f0.0 null    dest  0.0f
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_MUL, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_NONE, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_NZ, instruction(block0, 1)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, mad_one_component_vec4)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_vec4_type());
   dest.writemask = WRITEMASK_X;
   src_reg src0 = src_reg(v, glsl_vec4_type());
   src_reg src1 = src_reg(v, glsl_vec4_type());
   src_reg src2 = src_reg(v, glsl_vec4_type());
   src0.swizzle = src1.swizzle = src2.swizzle = BRW_SWIZZLE_XXXX;
   src2.negate = true;
   src_reg zero(brw_imm_f(0.0f));
   src_reg tmp(dest);
   tmp.swizzle = BRW_SWIZZLE_XXXX;
   dst_reg dest_null = bld.null_reg_f();
   dest_null.writemask = WRITEMASK_X;

   bld.MAD(dest, src0, src1, src2);
   bld.CMP(dest_null, tmp, zero, BRW_CONDITIONAL_L);

   /* = Before =
    *
    * 0: mad         dest.x:F  src0.xxxx:F  src10.xxxx:F  -src2.xxxx:F
    * 1: cmp.l.f0.0  null.x:F  dest.xxxx:F  0.0f
    *
    * = After =
    * 0: mad.l.f0    dest.x:F  src0.xxxx:F  src10.xxxx:F  -src2.xxxx:F
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(0, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_MAD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_L, instruction(block0, 0)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, mad_more_one_component_vec4)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_vec4_type());
   dest.writemask = WRITEMASK_XW;
   src_reg src0 = src_reg(v, glsl_vec4_type());
   src_reg src1 = src_reg(v, glsl_vec4_type());
   src_reg src2 = src_reg(v, glsl_vec4_type());
   src0.swizzle = src1.swizzle = src2.swizzle = BRW_SWIZZLE_XXXX;
   src2.negate = true;
   src_reg zero(brw_imm_f(0.0f));
   src_reg tmp(dest);
   tmp.swizzle = BRW_SWIZZLE_XXXX;
   dst_reg dest_null = bld.null_reg_f();

   bld.MAD(dest, src0, src1, src2);
   bld.CMP(dest_null, tmp, zero, BRW_CONDITIONAL_L);

   /* = Before =
    *
    * 0: mad         dest.xw:F  src0.xxxx:F  src10.xxxx:F  -src2.xxxx:F
    * 1: cmp.l.f0.0  null:F  dest.xxxx:F  zeroF
    *
    * = After =
    * (No changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_MAD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_NONE, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_L, instruction(block0, 1)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, cmp_mov_vec4)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_ivec4_type());
   dest.writemask = WRITEMASK_X;
   src_reg src0 = src_reg(v, glsl_ivec4_type());
   src0.swizzle = BRW_SWIZZLE_XXXX;
   src0.file = UNIFORM;
   src_reg nonone = retype(brw_imm_d(16), BRW_REGISTER_TYPE_D);
   src_reg mov_src = src_reg(dest);
   mov_src.swizzle = BRW_SWIZZLE_XXXX;
   dst_reg dest_null = bld.null_reg_d();
   dest_null.writemask = WRITEMASK_X;

   bld.CMP(dest, src0, nonone, BRW_CONDITIONAL_GE);
   set_condmod(BRW_CONDITIONAL_NZ,
               bld.MOV(dest_null, mov_src));

   /* = Before =
    *
    * 0: cmp.ge.f0  dest.x:D  u.xxxx:D  16D
    * 1: mov.nz.f0  null.x:D  dest.xxxx:D
    *
    * = After =
    * 0: cmp.ge.f0  dest.x:D  u.xxxx:D  16D
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(0, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 0)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, mul_cmp_different_channels_vec4)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_vec4_type());
   src_reg src0 = src_reg(v, glsl_vec4_type());
   src_reg src1 = src_reg(v, glsl_vec4_type());
   src_reg zero(brw_imm_f(0.0f));
   src_reg cmp_src = src_reg(dest);
   cmp_src.swizzle = BRW_SWIZZLE4(0,1,3,2);

   bld.MUL(dest, src0, src1);
   bld.CMP(bld.null_reg_f(), cmp_src, zero, BRW_CONDITIONAL_NZ);

   /* = Before =
    * 0: mul         dest  src0       src1
    * 1: cmp.nz.f0.0 null  dest.xywz  0.0f
    *
    * = After =
    * (No changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_MUL, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_NONE, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_NZ, instruction(block0, 1)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, add_cmp_same_dst_writemask)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_vec4_type());
   src_reg src0 = src_reg(v, glsl_vec4_type());
   src_reg src1 = src_reg(v, glsl_vec4_type());
   dst_reg dest_null = bld.null_reg_f();

   bld.ADD(dest, src0, src1);
   vec4_instruction *inst = bld.CMP(dest_null, src0, src1, BRW_CONDITIONAL_GE);
   inst->src[1].negate = true;

   /* = Before =
    *
    * 0: add        dest.xyzw  src0  src1
    * 1: cmp.ge.f0  null.xyzw  src0  -src1
    *
    * = After =
    * 0: add.ge.f0  dest.xyzw  src0  src1
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(0, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 0)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, add_cmp_different_dst_writemask)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_float_type());
   src_reg src0 = src_reg(v, glsl_vec4_type());
   src_reg src1 = src_reg(v, glsl_vec4_type());
   dst_reg dest_null = bld.null_reg_f();

   bld.ADD(dest, src0, src1);
   vec4_instruction *inst = bld.CMP(dest_null, src0, src1, BRW_CONDITIONAL_GE);
   inst->src[1].negate = true;

   /* = Before =
    *
    * 0: add        dest.x     src0  src1
    * 1: cmp.ge.f0  null.xyzw  src0  -src1
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_NONE, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 1)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, prop_across_sel_gfx7)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest1 = dst_reg(v, glsl_float_type());
   dst_reg dest2 = dst_reg(v, glsl_float_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg src1 = src_reg(v, glsl_float_type());
   src_reg src2 = src_reg(v, glsl_float_type());
   src_reg src3 = src_reg(v, glsl_float_type());
   src_reg zero(brw_imm_f(0.0f));
   dst_reg dest_null = bld.null_reg_f();
   dest_null.writemask = WRITEMASK_X;

   bld.ADD(dest1, src0, src1);
   bld.SEL(dest2, src2, src3)
      ->conditional_mod = BRW_CONDITIONAL_GE;
   bld.CMP(dest_null, src_reg(dest1), zero, BRW_CONDITIONAL_GE);

   /* = Before =
    *
    * 0: add        dest1.x src0.xxxx  src1.xxxx
    * 1: sel.ge.f0  dest2.x src2.xxxx  src3.xxxx
    * 2: cmp.ge.f0  null.x  dest.xxxx  0.0f
    *
    * = After =
    * 0: add.ge.f0  dest.x  src0.xxxx  src1.xxxx
    * 1: sel.ge.f0  dest2.x src2.xxxx  src3.xxxx
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_TRUE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_SEL, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 1)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, prop_across_sel_gfx5)
{
   devinfo->ver = 5;
   devinfo->verx10 = devinfo->ver * 10;

   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest1 = dst_reg(v, glsl_float_type());
   dst_reg dest2 = dst_reg(v, glsl_float_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg src1 = src_reg(v, glsl_float_type());
   src_reg src2 = src_reg(v, glsl_float_type());
   src_reg src3 = src_reg(v, glsl_float_type());
   src_reg zero(brw_imm_f(0.0f));
   dst_reg dest_null = bld.null_reg_f();
   dest_null.writemask = WRITEMASK_X;

   bld.ADD(dest1, src0, src1);
   bld.SEL(dest2, src2, src3)
      ->conditional_mod = BRW_CONDITIONAL_GE;
   bld.CMP(dest_null, src_reg(dest1), zero, BRW_CONDITIONAL_GE);

   /* = Before =
    *
    * 0: add        dest1.x src0.xxxx  src1.xxxx
    * 1: sel.ge.f0  dest2.x src2.xxxx  src3.xxxx
    * 2: cmp.ge.f0  null.x  dest.xxxx  0.0f
    *
    * = After =
    * (no changes)
    *
    * On Gfx4 and Gfx5, sel.l (for min) and sel.ge (for max) are implemented
    * using a separate cmpn and sel instruction.  This lowering occurs in
    * fs_vistor::lower_minmax which is called a long time after the first
    * calls to cmod_propagation.
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_NONE, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_SEL, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 1)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 2)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 2)->conditional_mod);
}

TEST_F(cmod_propagation_vec4_test, prop_into_sel_gfx5)
{
   devinfo->ver = 5;
   devinfo->verx10 = devinfo->ver * 10;

   const vec4_builder bld = vec4_builder(v).at_end();
   dst_reg dest = dst_reg(v, glsl_float_type());
   src_reg src0 = src_reg(v, glsl_float_type());
   src_reg src1 = src_reg(v, glsl_float_type());
   src_reg zero(brw_imm_f(0.0f));
   dst_reg dest_null = bld.null_reg_f();
   dest_null.writemask = WRITEMASK_X;

   bld.SEL(dest, src0, src1)
      ->conditional_mod = BRW_CONDITIONAL_GE;
   bld.CMP(dest_null, src_reg(dest), zero, BRW_CONDITIONAL_GE);

   /* = Before =
    *
    * 0: sel.ge.f0  dest.x  src2.xxxx  src3.xxxx
    * 1: cmp.ge.f0  null.x  dest.xxxx  0.0f
    *
    * = After =
    * (no changes)
    *
    * Do not copy propagate into a sel.cond instruction.  While it does modify
    * the flags, the flags are not based on the result compared with zero (as
    * with most other instructions).  The result is based on the sources
    * compared with each other (like cmp.cond).
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(cmod_propagation(v));

   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_SEL, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 0)->conditional_mod);
   EXPECT_EQ(BRW_OPCODE_CMP, instruction(block0, 1)->opcode);
   EXPECT_EQ(BRW_CONDITIONAL_GE, instruction(block0, 1)->conditional_mod);
}
