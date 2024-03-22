/*
 * Copyright © 2012 Intel Corporation
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

#define register_coalesce(v) _register_coalesce(v, __func__)

class register_coalesce_vec4_test : public ::testing::Test {
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


class register_coalesce_vec4_visitor : public vec4_visitor
{
public:
   register_coalesce_vec4_visitor(struct brw_compiler *compiler,
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


void register_coalesce_vec4_test::SetUp()
{
   ctx = ralloc_context(NULL);
   compiler = rzalloc(ctx, struct brw_compiler);
   devinfo = rzalloc(ctx, struct intel_device_info);
   compiler->devinfo = devinfo;

   prog_data = ralloc(ctx, struct brw_vue_prog_data);

   params = {};
   params.mem_ctx = ctx;

   nir_shader *shader =
      nir_shader_create(ctx, MESA_SHADER_VERTEX, NULL, NULL);

   v = new register_coalesce_vec4_visitor(compiler, &params, shader, prog_data);

   devinfo->ver = 4;
   devinfo->verx10 = devinfo->ver * 10;
}

void register_coalesce_vec4_test::TearDown()
{
   delete v;
   v = NULL;

   ralloc_free(ctx);
   ctx = NULL;
}

static void
_register_coalesce(vec4_visitor *v, const char *func)
{
   const bool print = getenv("TEST_DEBUG");

   if (print) {
      printf("%s: instructions before:\n", func);
      v->dump_instructions();
   }

   v->calculate_cfg();
   v->opt_register_coalesce();

   if (print) {
      printf("%s: instructions after:\n", func);
      v->dump_instructions();
   }
}

TEST_F(register_coalesce_vec4_test, test_compute_to_mrf)
{
   src_reg something = src_reg(v, glsl_float_type());
   dst_reg temp = dst_reg(v, glsl_float_type());
   dst_reg init;

   dst_reg m0 = dst_reg(MRF, 0);
   m0.writemask = WRITEMASK_X;
   m0.type = BRW_REGISTER_TYPE_F;

   vec4_instruction *mul = v->emit(v->MUL(temp, something, brw_imm_f(1.0f)));
   v->emit(v->MOV(m0, src_reg(temp)));

   register_coalesce(v);

   EXPECT_EQ(mul->dst.file, MRF);
}


TEST_F(register_coalesce_vec4_test, test_multiple_use)
{
   src_reg something = src_reg(v, glsl_float_type());
   dst_reg temp = dst_reg(v, glsl_vec4_type());
   dst_reg init;

   dst_reg m0 = dst_reg(MRF, 0);
   m0.writemask = WRITEMASK_X;
   m0.type = BRW_REGISTER_TYPE_F;

   dst_reg m1 = dst_reg(MRF, 1);
   m1.writemask = WRITEMASK_XYZW;
   m1.type = BRW_REGISTER_TYPE_F;

   src_reg src = src_reg(temp);
   vec4_instruction *mul = v->emit(v->MUL(temp, something, brw_imm_f(1.0f)));
   src.swizzle = BRW_SWIZZLE_XXXX;
   v->emit(v->MOV(m0, src));
   src.swizzle = BRW_SWIZZLE_XYZW;
   v->emit(v->MOV(m1, src));

   register_coalesce(v);

   EXPECT_NE(mul->dst.file, MRF);
}

TEST_F(register_coalesce_vec4_test, test_dp4_mrf)
{
   src_reg some_src_1 = src_reg(v, glsl_vec4_type());
   src_reg some_src_2 = src_reg(v, glsl_vec4_type());
   dst_reg init;

   dst_reg m0 = dst_reg(MRF, 0);
   m0.writemask = WRITEMASK_Y;
   m0.type = BRW_REGISTER_TYPE_F;

   dst_reg temp = dst_reg(v, glsl_float_type());

   vec4_instruction *dp4 = v->emit(v->DP4(temp, some_src_1, some_src_2));
   v->emit(v->MOV(m0, src_reg(temp)));

   register_coalesce(v);

   EXPECT_EQ(dp4->dst.file, MRF);
   EXPECT_EQ(dp4->dst.writemask, WRITEMASK_Y);
}

TEST_F(register_coalesce_vec4_test, test_dp4_grf)
{
   src_reg some_src_1 = src_reg(v, glsl_vec4_type());
   src_reg some_src_2 = src_reg(v, glsl_vec4_type());
   dst_reg init;

   dst_reg to = dst_reg(v, glsl_vec4_type());
   dst_reg temp = dst_reg(v, glsl_float_type());

   vec4_instruction *dp4 = v->emit(v->DP4(temp, some_src_1, some_src_2));
   to.writemask = WRITEMASK_Y;
   v->emit(v->MOV(to, src_reg(temp)));

   /* if we don't do something with the result, the automatic dead code
    * elimination will remove all our instructions.
    */
   src_reg src = src_reg(to);
   src.negate = true;
   v->emit(v->MOV(dst_reg(MRF, 0), src));

   register_coalesce(v);

   EXPECT_EQ(dp4->dst.nr, to.nr);
   EXPECT_EQ(dp4->dst.writemask, WRITEMASK_Y);
}

TEST_F(register_coalesce_vec4_test, test_channel_mul_grf)
{
   src_reg some_src_1 = src_reg(v, glsl_vec4_type());
   src_reg some_src_2 = src_reg(v, glsl_vec4_type());
   dst_reg init;

   dst_reg to = dst_reg(v, glsl_vec4_type());
   dst_reg temp = dst_reg(v, glsl_float_type());

   vec4_instruction *mul = v->emit(v->MUL(temp, some_src_1, some_src_2));
   to.writemask = WRITEMASK_Y;
   v->emit(v->MOV(to, src_reg(temp)));

   /* if we don't do something with the result, the automatic dead code
    * elimination will remove all our instructions.
    */
   src_reg src = src_reg(to);
   src.negate = true;
   v->emit(v->MOV(dst_reg(MRF, 0), src));

   register_coalesce(v);

   EXPECT_EQ(mul->dst.nr, to.nr);
}
