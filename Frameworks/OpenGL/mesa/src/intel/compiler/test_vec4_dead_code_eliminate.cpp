/*
 * Copyright © 2018 Intel Corporation
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
#include "brw_vec4.h"

using namespace brw;

class dead_code_eliminate_vec4_test : public ::testing::Test {
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

class dead_code_eliminate_vec4_visitor : public vec4_visitor
{
public:
   dead_code_eliminate_vec4_visitor(struct brw_compiler *compiler,
                                    struct brw_compile_params *params,
                                 nir_shader *shader,
                                 struct brw_vue_prog_data *prog_data)
      : vec4_visitor(compiler, params, NULL, prog_data, shader,
                     false /* no_spills */, false)
   {
      prog_data->dispatch_mode = DISPATCH_MODE_4X2_DUAL_OBJECT;
   }

protected:
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


void dead_code_eliminate_vec4_test::SetUp()
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

  v = new dead_code_eliminate_vec4_visitor(compiler, &params, shader, prog_data);

   devinfo->ver = 4;
   devinfo->verx10 = devinfo->ver * 10;
}

void dead_code_eliminate_vec4_test::TearDown()
{
   delete v;
   v = NULL;

   ralloc_free(ctx);
   ctx = NULL;
}

static void
dead_code_eliminate(vec4_visitor *v)
{
   const bool print = getenv("TEST_DEBUG");

   if (print) {
      fprintf(stderr, "instructions before:\n");
      v->dump_instructions();
   }

   v->calculate_cfg();
   v->dead_code_eliminate();

   if (print) {
      fprintf(stderr, "instructions after:\n");
      v->dump_instructions();
   }
}

TEST_F(dead_code_eliminate_vec4_test, some_dead_channels_all_flags_used)
{
   const vec4_builder bld = vec4_builder(v).at_end();
   src_reg r1 = src_reg(v, glsl_vec4_type());
   src_reg r2 = src_reg(v, glsl_vec4_type());
   src_reg r3 = src_reg(v, glsl_vec4_type());
   src_reg r4 = src_reg(v, glsl_vec4_type());
   src_reg r5 = src_reg(v, glsl_vec4_type());
   src_reg r6 = src_reg(v, glsl_vec4_type());

   /* Sequence like the following should not be modified by DCE.
    *
    *     cmp.l.f0(8)     g4<1>F         g2<4,4,1>.wF   g1<4,4,1>.xF
    *     mov(8)          g5<1>.xF       g4<4,4,1>.xF
    *     (+f0.x) sel(8)  g6<1>UD        g3<4>UD        g6<4>UD
    */
   vec4_instruction *test_cmp =
      bld.CMP(dst_reg(r4), r2, r1, BRW_CONDITIONAL_L);

   test_cmp->src[0].swizzle = BRW_SWIZZLE_WWWW;
   test_cmp->src[1].swizzle = BRW_SWIZZLE_XXXX;

   vec4_instruction *test_mov =
      bld.MOV(dst_reg(r5), r4);

   test_mov->dst.writemask = WRITEMASK_X;
   test_mov->src[0].swizzle = BRW_SWIZZLE_XXXX;

   vec4_instruction *test_sel =
      bld.SEL(dst_reg(r6), r3, r6);

   set_predicate(BRW_PREDICATE_NORMAL, test_sel);

   /* The scratch write is here just to make r5 and r6 be live so that the
    * whole program doesn't get eliminated by DCE.
    */
   v->emit(v->SCRATCH_WRITE(dst_reg(r4), r6, r5));

   dead_code_eliminate(v);

   EXPECT_EQ(test_cmp->dst.writemask, WRITEMASK_XYZW);
}
