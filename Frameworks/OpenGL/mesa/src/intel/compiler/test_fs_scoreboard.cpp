/*
 * Copyright © 2019 Intel Corporation
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

class scoreboard_test : public ::testing::Test {
protected:
   scoreboard_test();
   ~scoreboard_test() override;

   struct brw_compiler *compiler;
   struct brw_compile_params params;
   struct intel_device_info *devinfo;
   void *ctx;
   struct brw_wm_prog_data *prog_data;
   struct gl_shader_program *shader_prog;
   fs_visitor *v;
   fs_builder bld;
};

scoreboard_test::scoreboard_test()
   : bld(NULL, 0)
{
   ctx = ralloc_context(NULL);
   compiler = rzalloc(ctx, struct brw_compiler);
   devinfo = rzalloc(ctx, struct intel_device_info);
   devinfo->ver = 12;
   devinfo->verx10 = devinfo->ver * 10;

   compiler->devinfo = devinfo;
   brw_init_isa_info(&compiler->isa, devinfo);

   params = {};
   params.mem_ctx = ctx;

   prog_data = ralloc(ctx, struct brw_wm_prog_data);
   nir_shader *shader =
      nir_shader_create(ctx, MESA_SHADER_FRAGMENT, NULL, NULL);

   v = new fs_visitor(compiler, &params, NULL, &prog_data->base, shader, 8,
                      false, false);

   bld = fs_builder(v).at_end();
}

scoreboard_test::~scoreboard_test()
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

static void
lower_scoreboard(fs_visitor *v)
{
   const bool print = getenv("TEST_DEBUG");

   if (print) {
      fprintf(stderr, "= Before =\n");
      v->cfg->dump();
   }

   v->lower_scoreboard();

   if (print) {
      fprintf(stderr, "\n= After =\n");
      v->cfg->dump();
   }
}

fs_inst *
emit_SEND(const fs_builder &bld, const fs_reg &dst,
          const fs_reg &desc, const fs_reg &payload)
{
   fs_inst *inst = bld.emit(SHADER_OPCODE_SEND, dst, desc, desc, payload);
   inst->mlen = 1;
   return inst;
}

static tgl_swsb
tgl_swsb_testcase(unsigned regdist, unsigned sbid, enum tgl_sbid_mode mode)
{
   tgl_swsb swsb = tgl_swsb_sbid(mode, sbid);
   swsb.regdist = regdist;
   return swsb;
}

bool operator ==(const tgl_swsb &a, const tgl_swsb &b)
{
   return a.mode == b.mode &&
          a.regdist == b.regdist &&
          (a.mode == TGL_SBID_NULL || a.sbid == b.sbid);
}

std::ostream &operator<<(std::ostream &os, const tgl_swsb &swsb) {
   if (swsb.regdist)
      os << "@" << swsb.regdist;

   if (swsb.mode) {
      if (swsb.regdist)
         os << " ";
      os << "$" << swsb.sbid;
      if (swsb.mode & TGL_SBID_DST)
         os << ".dst";
      if (swsb.mode & TGL_SBID_SRC)
         os << ".src";
   }

   return os;
}

TEST_F(scoreboard_test, RAW_inorder_inorder)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   fs_reg y = v->vgrf(glsl_int_type());
   bld.ADD(   x, g[1], g[2]);
   bld.MUL(   y, g[3], g[4]);
   bld.AND(g[5],    x,    y);

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 1)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_regdist(1));
}

TEST_F(scoreboard_test, RAW_inorder_outoforder)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.ADD(          x, g[1], g[2]);
   bld.MUL(       g[3], g[4], g[5]);
   emit_SEND(bld, g[6], g[7],    x);

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 1)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_testcase(2, 0, TGL_SBID_SET));
}

TEST_F(scoreboard_test, RAW_outoforder_inorder)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   fs_reg y = v->vgrf(glsl_int_type());
   emit_SEND(bld,    x, g[1], g[2]);
   bld.MUL(          y, g[3], g[4]);
   bld.AND(       g[5],    x,    y);

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_sbid(TGL_SBID_SET, 0));
   EXPECT_EQ(instruction(block0, 1)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_testcase(1, 0, TGL_SBID_DST));
}

TEST_F(scoreboard_test, RAW_outoforder_outoforder)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   /* The second SEND depends on the first, and would need to refer to two
    * SBIDs.  Since it is not possible we expect a SYNC instruction to be
    * added.
    */
   fs_reg x = v->vgrf(glsl_int_type());
   emit_SEND(bld,    x, g[1], g[2]);
   emit_SEND(bld, g[3],    x, g[4])->sfid++;

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_sbid(TGL_SBID_SET, 0));

   fs_inst *sync = instruction(block0, 1);
   EXPECT_EQ(sync->opcode, BRW_OPCODE_SYNC);
   EXPECT_EQ(sync->sched, tgl_swsb_sbid(TGL_SBID_DST, 0));

   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_sbid(TGL_SBID_SET, 1));
}

TEST_F(scoreboard_test, WAR_inorder_inorder)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.ADD(g[1],    x, g[2]);
   bld.MUL(g[3], g[4], g[5]);
   bld.AND(   x, g[6], g[7]);

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 1)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_null());
}

TEST_F(scoreboard_test, WAR_inorder_outoforder)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.ADD(       g[1],    x, g[2]);
   bld.MUL(       g[3], g[4], g[5]);
   emit_SEND(bld,    x, g[6], g[7]);

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 1)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_testcase(2, 0, TGL_SBID_SET));
}

TEST_F(scoreboard_test, WAR_outoforder_inorder)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   emit_SEND(bld, g[1], g[2],    x);
   bld.MUL(       g[4], g[5], g[6]);
   bld.AND(          x, g[7], g[8]);

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_sbid(TGL_SBID_SET, 0));
   EXPECT_EQ(instruction(block0, 1)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_sbid(TGL_SBID_SRC, 0));
}

TEST_F(scoreboard_test, WAR_outoforder_outoforder)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   emit_SEND(bld, g[1], g[2],    x);
   emit_SEND(bld,    x, g[3], g[4])->sfid++;

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_sbid(TGL_SBID_SET, 0));

   fs_inst *sync = instruction(block0, 1);
   EXPECT_EQ(sync->opcode, BRW_OPCODE_SYNC);
   EXPECT_EQ(sync->sched, tgl_swsb_sbid(TGL_SBID_SRC, 0));

   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_sbid(TGL_SBID_SET, 1));
}

TEST_F(scoreboard_test, WAW_inorder_inorder)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.ADD(   x, g[1], g[2]);
   bld.MUL(g[3], g[4], g[5]);
   bld.AND(   x, g[6], g[7]);

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 1)->sched, tgl_swsb_null());

   /* NOTE: We only need this RegDist if a long instruction is followed by a
    * short one.  The pass is currently conservative about this and adding the
    * annotation.
    */
   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_regdist(2));
}

TEST_F(scoreboard_test, WAW_inorder_outoforder)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.ADD(          x, g[1], g[2]);
   bld.MUL(       g[3], g[4], g[5]);
   emit_SEND(bld,    x, g[6], g[7]);

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 1)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_testcase(2, 0, TGL_SBID_SET));
}

TEST_F(scoreboard_test, WAW_outoforder_inorder)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   emit_SEND(bld,    x, g[1], g[2]);
   bld.MUL(       g[3], g[4], g[5]);
   bld.AND(          x, g[6], g[7]);

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_sbid(TGL_SBID_SET, 0));
   EXPECT_EQ(instruction(block0, 1)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_sbid(TGL_SBID_DST, 0));
}

TEST_F(scoreboard_test, WAW_outoforder_outoforder)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   emit_SEND(bld, x, g[1], g[2]);
   emit_SEND(bld, x, g[3], g[4])->sfid++;

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(1, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_sbid(TGL_SBID_SET, 0));

   fs_inst *sync = instruction(block0, 1);
   EXPECT_EQ(sync->opcode, BRW_OPCODE_SYNC);
   EXPECT_EQ(sync->sched, tgl_swsb_sbid(TGL_SBID_DST, 0));

   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_sbid(TGL_SBID_SET, 1));
}


TEST_F(scoreboard_test, loop1)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.XOR(   x, g[1], g[2]);

   bld.emit(BRW_OPCODE_DO);

   bld.ADD(   x, g[1], g[2]);
   bld.emit(BRW_OPCODE_WHILE)->predicate = BRW_PREDICATE_NORMAL;

   bld.MUL(   x, g[1], g[2]);

   v->calculate_cfg();
   lower_scoreboard(v);

   bblock_t *body = v->cfg->blocks[2];
   fs_inst *add = instruction(body, 0);
   EXPECT_EQ(add->opcode, BRW_OPCODE_ADD);
   EXPECT_EQ(add->sched, tgl_swsb_regdist(1));

   bblock_t *last_block = v->cfg->blocks[3];
   fs_inst *mul = instruction(last_block, 0);
   EXPECT_EQ(mul->opcode, BRW_OPCODE_MUL);
   EXPECT_EQ(mul->sched, tgl_swsb_regdist(1));
}

TEST_F(scoreboard_test, loop2)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.XOR(   x, g[1], g[2]);
   bld.XOR(g[3], g[1], g[2]);
   bld.XOR(g[4], g[1], g[2]);
   bld.XOR(g[5], g[1], g[2]);

   bld.emit(BRW_OPCODE_DO);

   bld.ADD(   x, g[1], g[2]);
   bld.emit(BRW_OPCODE_WHILE)->predicate = BRW_PREDICATE_NORMAL;

   bld.MUL(   x, g[1], g[2]);

   v->calculate_cfg();
   lower_scoreboard(v);

   /* Now the write in ADD has the tightest RegDist for both ADD and MUL. */

   bblock_t *body = v->cfg->blocks[2];
   fs_inst *add = instruction(body, 0);
   EXPECT_EQ(add->opcode, BRW_OPCODE_ADD);
   EXPECT_EQ(add->sched, tgl_swsb_regdist(2));

   bblock_t *last_block = v->cfg->blocks[3];
   fs_inst *mul = instruction(last_block, 0);
   EXPECT_EQ(mul->opcode, BRW_OPCODE_MUL);
   EXPECT_EQ(mul->sched, tgl_swsb_regdist(2));
}

TEST_F(scoreboard_test, loop3)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.XOR(   x, g[1], g[2]);

   bld.emit(BRW_OPCODE_DO);

   /* For the ADD in the loop body this extra distance will always apply. */
   bld.XOR(g[3], g[1], g[2]);
   bld.XOR(g[4], g[1], g[2]);
   bld.XOR(g[5], g[1], g[2]);
   bld.XOR(g[6], g[1], g[2]);

   bld.ADD(   x, g[1], g[2]);
   bld.emit(BRW_OPCODE_WHILE)->predicate = BRW_PREDICATE_NORMAL;

   bld.MUL(   x, g[1], g[2]);

   v->calculate_cfg();
   lower_scoreboard(v);

   bblock_t *body = v->cfg->blocks[2];
   fs_inst *add = instruction(body, 4);
   EXPECT_EQ(add->opcode, BRW_OPCODE_ADD);
   EXPECT_EQ(add->sched, tgl_swsb_regdist(5));

   bblock_t *last_block = v->cfg->blocks[3];
   fs_inst *mul = instruction(last_block, 0);
   EXPECT_EQ(mul->opcode, BRW_OPCODE_MUL);
   EXPECT_EQ(mul->sched, tgl_swsb_regdist(1));
}


TEST_F(scoreboard_test, conditional1)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.XOR(   x, g[1], g[2]);
   bld.emit(BRW_OPCODE_IF);

   bld.ADD(   x, g[1], g[2]);

   bld.emit(BRW_OPCODE_ENDIF);
   bld.MUL(   x, g[1], g[2]);

   v->calculate_cfg();
   lower_scoreboard(v);

   bblock_t *body = v->cfg->blocks[1];
   fs_inst *add = instruction(body, 0);
   EXPECT_EQ(add->opcode, BRW_OPCODE_ADD);
   EXPECT_EQ(add->sched, tgl_swsb_regdist(2));

   bblock_t *last_block = v->cfg->blocks[2];
   fs_inst *mul = instruction(last_block, 1);
   EXPECT_EQ(mul->opcode, BRW_OPCODE_MUL);
   EXPECT_EQ(mul->sched, tgl_swsb_regdist(2));
}

TEST_F(scoreboard_test, conditional2)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.XOR(   x, g[1], g[2]);
   bld.XOR(g[3], g[1], g[2]);
   bld.XOR(g[4], g[1], g[2]);
   bld.XOR(g[5], g[1], g[2]);
   bld.emit(BRW_OPCODE_IF);

   bld.ADD(   x, g[1], g[2]);

   bld.emit(BRW_OPCODE_ENDIF);
   bld.MUL(   x, g[1], g[2]);

   v->calculate_cfg();
   lower_scoreboard(v);

   bblock_t *body = v->cfg->blocks[1];
   fs_inst *add = instruction(body, 0);
   EXPECT_EQ(add->opcode, BRW_OPCODE_ADD);
   EXPECT_EQ(add->sched, tgl_swsb_regdist(5));

   bblock_t *last_block = v->cfg->blocks[2];
   fs_inst *mul = instruction(last_block, 1);
   EXPECT_EQ(mul->opcode, BRW_OPCODE_MUL);
   EXPECT_EQ(mul->sched, tgl_swsb_regdist(2));
}

TEST_F(scoreboard_test, conditional3)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.XOR(   x, g[1], g[2]);
   bld.emit(BRW_OPCODE_IF);

   bld.XOR(g[3], g[1], g[2]);
   bld.XOR(g[4], g[1], g[2]);
   bld.XOR(g[5], g[1], g[2]);
   bld.ADD(   x, g[1], g[2]);

   bld.emit(BRW_OPCODE_ENDIF);
   bld.MUL(   x, g[1], g[2]);

   v->calculate_cfg();
   lower_scoreboard(v);

   bblock_t *body = v->cfg->blocks[1];
   fs_inst *add = instruction(body, 3);
   EXPECT_EQ(add->opcode, BRW_OPCODE_ADD);
   EXPECT_EQ(add->sched, tgl_swsb_regdist(5));

   bblock_t *last_block = v->cfg->blocks[2];
   fs_inst *mul = instruction(last_block, 1);
   EXPECT_EQ(mul->opcode, BRW_OPCODE_MUL);
   EXPECT_EQ(mul->sched, tgl_swsb_regdist(2));
}

TEST_F(scoreboard_test, conditional4)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.XOR(   x, g[1], g[2]);
   bld.emit(BRW_OPCODE_IF);

   bld.ADD(   x, g[1], g[2]);
   bld.XOR(g[3], g[1], g[2]);
   bld.XOR(g[4], g[1], g[2]);
   bld.XOR(g[5], g[1], g[2]);

   bld.emit(BRW_OPCODE_ENDIF);
   bld.MUL(   x, g[1], g[2]);

   v->calculate_cfg();
   lower_scoreboard(v);

   bblock_t *body = v->cfg->blocks[1];
   fs_inst *add = instruction(body, 0);
   EXPECT_EQ(add->opcode, BRW_OPCODE_ADD);
   EXPECT_EQ(add->sched, tgl_swsb_regdist(2));

   bblock_t *last_block = v->cfg->blocks[2];
   fs_inst *mul = instruction(last_block, 1);
   EXPECT_EQ(mul->opcode, BRW_OPCODE_MUL);
   EXPECT_EQ(mul->sched, tgl_swsb_regdist(3));
}

TEST_F(scoreboard_test, conditional5)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.XOR(   x, g[1], g[2]);
   bld.emit(BRW_OPCODE_IF);

   bld.ADD(   x, g[1], g[2]);
   bld.emit(BRW_OPCODE_ELSE);

   bld.ROL(   x, g[1], g[2]);

   bld.emit(BRW_OPCODE_ENDIF);
   bld.MUL(   x, g[1], g[2]);

   v->calculate_cfg();
   lower_scoreboard(v);

   bblock_t *then_body = v->cfg->blocks[1];
   fs_inst *add = instruction(then_body, 0);
   EXPECT_EQ(add->opcode, BRW_OPCODE_ADD);
   EXPECT_EQ(add->sched, tgl_swsb_regdist(2));

   bblock_t *else_body = v->cfg->blocks[2];
   fs_inst *rol = instruction(else_body, 0);
   EXPECT_EQ(rol->opcode, BRW_OPCODE_ROL);
   EXPECT_EQ(rol->sched, tgl_swsb_regdist(2));

   bblock_t *last_block = v->cfg->blocks[3];
   fs_inst *mul = instruction(last_block, 1);
   EXPECT_EQ(mul->opcode, BRW_OPCODE_MUL);
   EXPECT_EQ(mul->sched, tgl_swsb_regdist(2));
}

TEST_F(scoreboard_test, conditional6)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.XOR(   x, g[1], g[2]);
   bld.emit(BRW_OPCODE_IF);

   bld.XOR(g[3], g[1], g[2]);
   bld.XOR(g[4], g[1], g[2]);
   bld.XOR(g[5], g[1], g[2]);
   bld.ADD(   x, g[1], g[2]);
   bld.emit(BRW_OPCODE_ELSE);

   bld.XOR(g[6], g[1], g[2]);
   bld.XOR(g[7], g[1], g[2]);
   bld.XOR(g[8], g[1], g[2]);
   bld.XOR(g[9], g[1], g[2]);
   bld.ROL(   x, g[1], g[2]);

   bld.emit(BRW_OPCODE_ENDIF);
   bld.MUL(   x, g[1], g[2]);

   v->calculate_cfg();
   lower_scoreboard(v);

   bblock_t *then_body = v->cfg->blocks[1];
   fs_inst *add = instruction(then_body, 3);
   EXPECT_EQ(add->opcode, BRW_OPCODE_ADD);
   EXPECT_EQ(add->sched, tgl_swsb_regdist(5));

   bblock_t *else_body = v->cfg->blocks[2];
   fs_inst *rol = instruction(else_body, 4);
   EXPECT_EQ(rol->opcode, BRW_OPCODE_ROL);
   EXPECT_EQ(rol->sched, tgl_swsb_regdist(6));

   bblock_t *last_block = v->cfg->blocks[3];
   fs_inst *mul = instruction(last_block, 1);
   EXPECT_EQ(mul->opcode, BRW_OPCODE_MUL);
   EXPECT_EQ(mul->sched, tgl_swsb_regdist(2));
}

TEST_F(scoreboard_test, conditional7)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.XOR(   x, g[1], g[2]);
   bld.emit(BRW_OPCODE_IF);

   bld.ADD(   x, g[1], g[2]);
   bld.XOR(g[3], g[1], g[2]);
   bld.XOR(g[4], g[1], g[2]);
   bld.XOR(g[5], g[1], g[2]);
   bld.emit(BRW_OPCODE_ELSE);

   bld.ROL(   x, g[1], g[2]);
   bld.XOR(g[6], g[1], g[2]);
   bld.XOR(g[7], g[1], g[2]);
   bld.XOR(g[8], g[1], g[2]);
   bld.XOR(g[9], g[1], g[2]);

   bld.emit(BRW_OPCODE_ENDIF);
   bld.MUL(   x, g[1], g[2]);

   v->calculate_cfg();
   lower_scoreboard(v);

   bblock_t *then_body = v->cfg->blocks[1];
   fs_inst *add = instruction(then_body, 0);
   EXPECT_EQ(add->opcode, BRW_OPCODE_ADD);
   EXPECT_EQ(add->sched, tgl_swsb_regdist(2));

   bblock_t *else_body = v->cfg->blocks[2];
   fs_inst *rol = instruction(else_body, 0);
   EXPECT_EQ(rol->opcode, BRW_OPCODE_ROL);
   EXPECT_EQ(rol->sched, tgl_swsb_regdist(2));

   bblock_t *last_block = v->cfg->blocks[3];
   fs_inst *mul = instruction(last_block, 1);
   EXPECT_EQ(mul->opcode, BRW_OPCODE_MUL);
   EXPECT_EQ(mul->sched, tgl_swsb_regdist(6));
}

TEST_F(scoreboard_test, conditional8)
{
   fs_reg g[16];
   for (unsigned i = 0; i < ARRAY_SIZE(g); i++)
      g[i] = v->vgrf(glsl_int_type());

   fs_reg x = v->vgrf(glsl_int_type());
   bld.XOR(   x, g[1], g[2]);
   bld.XOR(g[3], g[1], g[2]);
   bld.XOR(g[4], g[1], g[2]);
   bld.XOR(g[5], g[1], g[2]);
   bld.XOR(g[6], g[1], g[2]);
   bld.XOR(g[7], g[1], g[2]);
   bld.emit(BRW_OPCODE_IF);

   bld.ADD(   x, g[1], g[2]);
   bld.emit(BRW_OPCODE_ELSE);

   bld.ROL(   x, g[1], g[2]);

   bld.emit(BRW_OPCODE_ENDIF);
   bld.MUL(   x, g[1], g[2]);

   v->calculate_cfg();
   lower_scoreboard(v);

   bblock_t *then_body = v->cfg->blocks[1];
   fs_inst *add = instruction(then_body, 0);
   EXPECT_EQ(add->opcode, BRW_OPCODE_ADD);
   EXPECT_EQ(add->sched, tgl_swsb_regdist(7));

   /* Note that the ROL will have RegDist 2 and not 7, illustrating the
    * physical CFG edge between the then-block and the else-block.
    */
   bblock_t *else_body = v->cfg->blocks[2];
   fs_inst *rol = instruction(else_body, 0);
   EXPECT_EQ(rol->opcode, BRW_OPCODE_ROL);
   EXPECT_EQ(rol->sched, tgl_swsb_regdist(2));

   bblock_t *last_block = v->cfg->blocks[3];
   fs_inst *mul = instruction(last_block, 1);
   EXPECT_EQ(mul->opcode, BRW_OPCODE_MUL);
   EXPECT_EQ(mul->sched, tgl_swsb_regdist(2));
}

TEST_F(scoreboard_test, gfx125_RaR_over_different_pipes)
{
   devinfo->verx10 = 125;
   brw_init_isa_info(&compiler->isa, devinfo);

   fs_reg a = v->vgrf(glsl_int_type());
   fs_reg b = v->vgrf(glsl_int_type());
   fs_reg f = v->vgrf(glsl_float_type());
   fs_reg x = v->vgrf(glsl_int_type());

   bld.ADD(f, x, x);
   bld.ADD(a, x, x);
   bld.ADD(x, b, b);

   v->calculate_cfg();
   bblock_t *block0 = v->cfg->blocks[0];
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   lower_scoreboard(v);
   ASSERT_EQ(0, block0->start_ip);
   ASSERT_EQ(2, block0->end_ip);

   EXPECT_EQ(instruction(block0, 0)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 1)->sched, tgl_swsb_null());
   EXPECT_EQ(instruction(block0, 2)->sched, tgl_swsb_regdist(1));
}
