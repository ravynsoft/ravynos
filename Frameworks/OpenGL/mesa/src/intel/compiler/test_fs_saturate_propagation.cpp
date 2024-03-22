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
 */

#include <gtest/gtest.h>
#include "brw_fs.h"
#include "brw_fs_builder.h"
#include "brw_cfg.h"

using namespace brw;

class saturate_propagation_test : public ::testing::Test {
protected:
   saturate_propagation_test();
   ~saturate_propagation_test() override;

   struct brw_compiler *compiler;
   struct brw_compile_params params;
   struct intel_device_info *devinfo;
   void *ctx;
   struct brw_wm_prog_data *prog_data;
   struct gl_shader_program *shader_prog;
   fs_visitor *v;
   fs_builder bld;
};

class saturate_propagation_fs_visitor : public fs_visitor
{
public:
   saturate_propagation_fs_visitor(struct brw_compiler *compiler,
                                   struct brw_compile_params *params,
                                   struct brw_wm_prog_data *prog_data,
                                   nir_shader *shader)
      : fs_visitor(compiler, params, NULL,
                   &prog_data->base, shader, 16, false, false) {}
};


saturate_propagation_test::saturate_propagation_test()
   : bld(NULL, 0)
{
   ctx = ralloc_context(NULL);
   compiler = rzalloc(ctx, struct brw_compiler);
   devinfo = rzalloc(ctx, struct intel_device_info);
   compiler->devinfo = devinfo;

   params = {};
   params.mem_ctx = ctx;

   prog_data = ralloc(ctx, struct brw_wm_prog_data);
   nir_shader *shader =
      nir_shader_create(ctx, MESA_SHADER_FRAGMENT, NULL, NULL);

   v = new saturate_propagation_fs_visitor(compiler, &params, prog_data, shader);

   bld = fs_builder(v).at_end();

   devinfo->ver = 6;
   devinfo->verx10 = devinfo->ver * 10;
}

saturate_propagation_test::~saturate_propagation_test()
{
   delete v;
   v = NULL;

   ralloc_free(ctx);
   ctx = NULL;
}


static fs_inst *
instruction(bblock_t *block, int num)
{
   fs_inst *inst = (fs_inst *)block->start();
   for (int i = 0; i < num; i++) {
      inst = (fs_inst *)inst->next;
   }
   return inst;
}

static bool
saturate_propagation(fs_visitor *v)
{
   const bool print = false;

   if (print) {
      fprintf(stderr, "= Before =\n");
      v->cfg->dump();
   }

   bool ret = v->opt_saturate_propagation();

   if (print) {
      fprintf(stderr, "\n= After =\n");
      v->cfg->dump();
   }

   return ret;
}

TEST_F(saturate_propagation_test, basic)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.ADD(dst0, src0, src1);
   set_saturate(true, bld.MOV(dst1, dst0));

   /* = Before =
    *
    * 0: add(16)       dst0  src0  src1
    * 1: mov.sat(16)   dst1  dst0
    *
    * = After =
    * 0: add.sat(16)   dst0  src0  src1
    * 1: mov(16)       dst1  dst0
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_TRUE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, other_non_saturated_use)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg dst2 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.ADD(dst0, src0, src1);
   set_saturate(true, bld.MOV(dst1, dst0));
   bld.ADD(dst2, dst0, src0);

   /* = Before =
    *
    * 0: add(16)       dst0  src0  src1
    * 1: mov.sat(16)   dst1  dst0
    * 2: add(16)       dst2  dst0  src0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 2)->opcode);
}

TEST_F(saturate_propagation_test, predicated_instruction)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.ADD(dst0, src0, src1)
      ->predicate = BRW_PREDICATE_NORMAL;
   set_saturate(true, bld.MOV(dst1, dst0));

   /* = Before =
    *
    * 0: (+f0) add(16) dst0  src0  src1
    * 1: mov.sat(16)   dst1  dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, neg_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   bld.RNDU(dst0, src0);
   dst0.negate = true;
   set_saturate(true, bld.MOV(dst1, dst0));

   /* = Before =
    *
    * 0: rndu(16)      dst0  src0
    * 1: mov.sat(16)   dst1  -dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_RNDU, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, add_neg_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.ADD(dst0, src0, src1);
   dst0.negate = true;
   set_saturate(true, bld.MOV(dst1, dst0));

   /* = Before =
    *
    * 0: add(16)       dst0  src0  src1
    * 1: mov.sat(16)   dst1  -dst0
    *
    * = After =
    * 0: add.sat(16)   dst0  -src0 -src1
    * 1: mov(16)       dst1  dst0
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_TRUE(instruction(block0, 0)->saturate);
   EXPECT_TRUE(instruction(block0, 0)->src[0].negate);
   EXPECT_TRUE(instruction(block0, 0)->src[1].negate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, add_imm_float_neg_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = brw_imm_f(1.0f);
   bld.ADD(dst0, src0, src1);
   dst0.negate = true;
   set_saturate(true, bld.MOV(dst1, dst0));

   /* = Before =
    *
    * 0: add(16)       dst0  src0  1.0f
    * 1: mov.sat(16)   dst1  -dst0
    *
    * = After =
    * 0: add.sat(16)   dst0  -src0 -1.0f
    * 1: mov(16)       dst1  dst0
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_TRUE(instruction(block0, 0)->saturate);
   EXPECT_TRUE(instruction(block0, 0)->src[0].negate);
   EXPECT_EQ(instruction(block0, 0)->src[1].f, -1.0f);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, mul_neg_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.MUL(dst0, src0, src1);
   dst0.negate = true;
   set_saturate(true, bld.MOV(dst1, dst0));

   /* = Before =
    *
    * 0: mul(16)       dst0  src0  src1
    * 1: mov.sat(16)   dst1  -dst0
    *
    * = After =
    * 0: mul.sat(16)   dst0  src0 -src1
    * 1: mov(16)       dst1  dst0
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_MUL, instruction(block0, 0)->opcode);
   EXPECT_TRUE(instruction(block0, 0)->saturate);
   EXPECT_TRUE(instruction(block0, 0)->src[0].negate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 1)->saturate);
   EXPECT_FALSE(instruction(block0, 1)->src[0].negate);
}

TEST_F(saturate_propagation_test, mad_neg_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   fs_reg src2 = v->vgrf(glsl_float_type());
   bld.MAD(dst0, src0, src1, src2);
   dst0.negate = true;
   set_saturate(true, bld.MOV(dst1, dst0));

   /* = Before =
    *
    * 0: mad(16)       dst0  src0  src1 src2
    * 1: mov.sat(16)   dst1  -dst0
    *
    * = After =
    * 0: mad.sat(16)   dst0  -src0 -src1 src2
    * 1: mov(16)       dst1  dst0
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_MAD, instruction(block0, 0)->opcode);
   EXPECT_TRUE(instruction(block0, 0)->saturate);
   EXPECT_TRUE(instruction(block0, 0)->src[0].negate);
   EXPECT_TRUE(instruction(block0, 0)->src[1].negate);
   EXPECT_FALSE(instruction(block0, 0)->src[2].negate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 1)->saturate);
   EXPECT_FALSE(instruction(block0, 1)->src[0].negate);
}

TEST_F(saturate_propagation_test, mad_imm_float_neg_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg src0 = brw_imm_f(1.0f);
   fs_reg src1 = brw_imm_f(-2.0f);
   fs_reg src2 = v->vgrf(glsl_float_type());
   /* The builder for MAD tries to be helpful and not put immediates as direct
    * sources. We want to test specifically that case.
    */
   fs_inst *mad = bld.MAD(dst0, src2, src2, src2);
   mad->src[0]= src0;
   mad->src[1] = src1;
   dst0.negate = true;
   set_saturate(true, bld.MOV(dst1, dst0));

   /* = Before =
    *
    * 0: mad(16)       dst0  1.0f -2.0f src2
    * 1: mov.sat(16)   dst1  -dst0
    *
    * = After =
    * 0: mad.sat(16)   dst0  -1.0f 2.0f src2
    * 1: mov(16)       dst1  dst0
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_TRUE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_MAD, instruction(block0, 0)->opcode);
   EXPECT_TRUE(instruction(block0, 0)->saturate);
   EXPECT_EQ(instruction(block0, 0)->src[0].f, -1.0f);
   EXPECT_EQ(instruction(block0, 0)->src[1].f, 2.0f);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 1)->saturate);
   EXPECT_FALSE(instruction(block0, 1)->src[0].negate);
}

TEST_F(saturate_propagation_test, mul_mov_sat_neg_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg dst2 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.MUL(dst0, src0, src1);
   set_saturate(true, bld.MOV(dst1, dst0));
   dst0.negate = true;
   set_saturate(true, bld.MOV(dst2, dst0));

   /* = Before =
    *
    * 0: mul(16)       dst0  src0  src1
    * 1: mov.sat(16)   dst1  dst0
    * 2: mov.sat(16)   dst2  -dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_MUL, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_FALSE(instruction(block0, 0)->src[1].negate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 2)->opcode);
   EXPECT_TRUE(instruction(block0, 2)->src[0].negate);
   EXPECT_TRUE(instruction(block0, 2)->saturate);
}

TEST_F(saturate_propagation_test, mul_neg_mov_sat_neg_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg dst2 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.MUL(dst0, src0, src1);
   dst0.negate = true;
   set_saturate(true, bld.MOV(dst1, dst0));
   set_saturate(true, bld.MOV(dst2, dst0));

   /* = Before =
    *
    * 0: mul(16)       dst0  src0  src1
    * 1: mov.sat(16)   dst1  -dst0
    * 2: mov.sat(16)   dst2  -dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_MUL, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_FALSE(instruction(block0, 0)->src[1].negate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->src[0].negate);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 2)->opcode);
   EXPECT_TRUE(instruction(block0, 2)->src[0].negate);
   EXPECT_TRUE(instruction(block0, 2)->saturate);
}

TEST_F(saturate_propagation_test, abs_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.ADD(dst0, src0, src1);
   dst0.abs = true;
   set_saturate(true, bld.MOV(dst1, dst0));

   /* = Before =
    *
    * 0: add(16)       dst0  src0  src1
    * 1: mov.sat(16)   dst1  (abs)dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, producer_saturates)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg dst2 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   set_saturate(true, bld.ADD(dst0, src0, src1));
   set_saturate(true, bld.MOV(dst1, dst0));
   bld.MOV(dst2, dst0);

   /* = Before =
    *
    * 0: add.sat(16)   dst0  src0  src1
    * 1: mov.sat(16)   dst1  dst0
    * 2: mov(16)       dst2  dst0
    *
    * = After =
    * 0: add.sat(16)   dst0  src0  src1
    * 1: mov(16)       dst1  dst0
    * 2: mov(16)       dst2  dst0
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_TRUE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_TRUE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, intervening_saturating_copy)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg dst2 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.ADD(dst0, src0, src1);
   set_saturate(true, bld.MOV(dst1, dst0));
   set_saturate(true, bld.MOV(dst2, dst0));

   /* = Before =
    *
    * 0: add(16)       dst0  src0  src1
    * 1: mov.sat(16)   dst1  dst0
    * 2: mov.sat(16)   dst2  dst0
    *
    * = After =
    * 0: add.sat(16)   dst0  src0  src1
    * 1: mov(16)       dst1  dst0
    * 2: mov(16)       dst2  dst0
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_TRUE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_TRUE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 1)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 2)->opcode);
   EXPECT_FALSE(instruction(block0, 2)->saturate);
}

TEST_F(saturate_propagation_test, intervening_dest_write)
{
   fs_reg dst0 = v->vgrf(glsl_vec4_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   fs_reg src2 = v->vgrf(glsl_vec2_type());
   bld.ADD(offset(dst0, bld, 2), src0, src1);
   bld.emit(SHADER_OPCODE_TEX, dst0, src2)
      ->size_written = 8 * REG_SIZE;
   set_saturate(true, bld.MOV(dst1, offset(dst0, bld, 2)));

   /* = Before =
    *
    * 0: add(16)        dst0+2  src0    src1
    * 1: tex(16) rlen 4 dst0+0  src2
    * 2: mov.sat(16)    dst1    dst0+2
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(SHADER_OPCODE_TEX, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 2)->opcode);
   EXPECT_TRUE(instruction(block0, 2)->saturate);
}

TEST_F(saturate_propagation_test, mul_neg_mov_sat_mov_sat)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg dst2 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.MUL(dst0, src0, src1);
   dst0.negate = true;
   set_saturate(true, bld.MOV(dst1, dst0));
   dst0.negate = false;
   set_saturate(true, bld.MOV(dst2, dst0));

   /* = Before =
    *
    * 0: mul(16)       dst0  src0  src1
    * 1: mov.sat(16)   dst1  -dst0
    * 2: mov.sat(16)   dst2  dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_MUL, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_FALSE(instruction(block0, 0)->src[1].negate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
   EXPECT_TRUE(instruction(block0, 1)->src[0].negate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 2)->opcode);
   EXPECT_TRUE(instruction(block0, 2)->saturate);
}

TEST_F(saturate_propagation_test, smaller_exec_size_consumer)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.ADD(dst0, src0, src1);
   set_saturate(true, bld.group(8, 0).MOV(dst1, dst0));

   /* = Before =
    *
    * 0: add(16)       dst0  src0  src1
    * 1: mov.sat(8)    dst1  dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, larger_exec_size_consumer)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.group(8, 0).ADD(dst0, src0, src1);
   set_saturate(true, bld.MOV(dst1, dst0));

   /* = Before =
    *
    * 0: add(8)        dst0  src0  src1
    * 1: mov.sat(16)   dst1  dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(1, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 1)->opcode);
   EXPECT_TRUE(instruction(block0, 1)->saturate);
}

TEST_F(saturate_propagation_test, offset_source_barrier)
{
   fs_reg dst0 = v->vgrf(glsl_float_type());
   fs_reg dst1 = v->vgrf(glsl_float_type());
   fs_reg dst2 = v->vgrf(glsl_float_type());
   fs_reg src0 = v->vgrf(glsl_float_type());
   fs_reg src1 = v->vgrf(glsl_float_type());
   bld.group(16, 0).ADD(dst0, src0, src1);
   bld.group(1, 0).ADD(dst1, component(dst0, 8), brw_imm_f(1.0f));
   set_saturate(true, bld.group(16, 0).MOV(dst2, dst0));

   /* = Before =
    *
    * 0: add(16)       dst0  src0   src1
    * 0: add(1)        dst1  dst0+8 1.0f
    * 1: mov.sat(16)   dst2  dst0
    *
    * = After =
    * (no changes)
    */

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];

   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);

   EXPECT_FALSE(saturate_propagation(v));
   EXPECT_EQ(0, block0->start_ip);
   EXPECT_EQ(2, block0->end_ip);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 0)->opcode);
   EXPECT_EQ(BRW_OPCODE_ADD, instruction(block0, 1)->opcode);
   EXPECT_FALSE(instruction(block0, 0)->saturate);
   EXPECT_FALSE(instruction(block0, 1)->saturate);
   EXPECT_EQ(BRW_OPCODE_MOV, instruction(block0, 2)->opcode);
   EXPECT_TRUE(instruction(block0, 2)->saturate);
}
