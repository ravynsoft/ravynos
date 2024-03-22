/*
 * Copyright © 2014 Intel Corporation
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

class copy_propagation_vec4_test : public ::testing::Test {
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

class copy_propagation_vec4_visitor : public vec4_visitor
{
public:
   copy_propagation_vec4_visitor(struct brw_compiler *compiler,
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


void copy_propagation_vec4_test::SetUp()
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

   v = new copy_propagation_vec4_visitor(compiler, &params, shader, prog_data);

   devinfo->ver = 4;
   devinfo->verx10 = devinfo->ver * 10;
}

void copy_propagation_vec4_test::TearDown()
{
   delete v;
   v = NULL;

   ralloc_free(ctx);
   ctx = NULL;
}


static void
copy_propagation(vec4_visitor *v)
{
   const bool print = getenv("TEST_DEBUG");

   if (print) {
      fprintf(stderr, "instructions before:\n");
      v->dump_instructions();
   }

   v->calculate_cfg();
   v->opt_copy_propagation();

   if (print) {
      fprintf(stderr, "instructions after:\n");
      v->dump_instructions();
   }
}

TEST_F(copy_propagation_vec4_test, test_swizzle_swizzle)
{
   dst_reg a = dst_reg(v, glsl_vec4_type());
   dst_reg b = dst_reg(v, glsl_vec4_type());
   dst_reg c = dst_reg(v, glsl_vec4_type());

   v->emit(v->ADD(a, src_reg(a), src_reg(a)));

   v->emit(v->MOV(b, swizzle(src_reg(a), BRW_SWIZZLE4(BRW_SWIZZLE_Y,
                                                      BRW_SWIZZLE_Z,
                                                      BRW_SWIZZLE_W,
                                                      BRW_SWIZZLE_X))));

   vec4_instruction *test_mov =
      v->MOV(c, swizzle(src_reg(b), BRW_SWIZZLE4(BRW_SWIZZLE_Y,
                                                 BRW_SWIZZLE_Z,
                                                 BRW_SWIZZLE_W,
                                                 BRW_SWIZZLE_X)));
   v->emit(test_mov);

   copy_propagation(v);

   EXPECT_EQ(test_mov->src[0].nr, a.nr);
   EXPECT_EQ(test_mov->src[0].swizzle, BRW_SWIZZLE4(BRW_SWIZZLE_Z,
                                                    BRW_SWIZZLE_W,
                                                    BRW_SWIZZLE_X,
                                                    BRW_SWIZZLE_Y));
}

TEST_F(copy_propagation_vec4_test, test_swizzle_writemask)
{
   dst_reg a = dst_reg(v, glsl_vec4_type());
   dst_reg b = dst_reg(v, glsl_vec4_type());
   dst_reg c = dst_reg(v, glsl_vec4_type());

   v->emit(v->MOV(b, swizzle(src_reg(a), BRW_SWIZZLE4(BRW_SWIZZLE_X,
                                                      BRW_SWIZZLE_Y,
                                                      BRW_SWIZZLE_X,
                                                      BRW_SWIZZLE_Z))));

   v->emit(v->MOV(writemask(a, WRITEMASK_XYZ), brw_imm_f(1.0f)));

   vec4_instruction *test_mov =
      v->MOV(c, swizzle(src_reg(b), BRW_SWIZZLE4(BRW_SWIZZLE_W,
                                                 BRW_SWIZZLE_W,
                                                 BRW_SWIZZLE_W,
                                                 BRW_SWIZZLE_W)));
   v->emit(test_mov);

   copy_propagation(v);

   /* should not copy propagate */
   EXPECT_EQ(test_mov->src[0].nr, b.nr);
   EXPECT_EQ(test_mov->src[0].swizzle, BRW_SWIZZLE4(BRW_SWIZZLE_W,
                                                    BRW_SWIZZLE_W,
                                                    BRW_SWIZZLE_W,
                                                    BRW_SWIZZLE_W));
}
