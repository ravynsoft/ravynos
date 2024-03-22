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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "nir_test.h"

/* This is a macro so you get good line numbers */
#define EXPECT_INSTR_SWIZZLES(instr, load, expected_swizzle)    \
   EXPECT_EQ((instr)->src[0].src.ssa, &(load)->def);       \
   EXPECT_EQ(swizzle(instr, 0), expected_swizzle);

namespace {

class nir_load_store_vectorize_test : public nir_test {
protected:
   nir_load_store_vectorize_test()
      : nir_test::nir_test("nir_load_store_vectorize_test")
   {
   }

   unsigned count_intrinsics(nir_intrinsic_op intrinsic);

   nir_intrinsic_instr *get_intrinsic(nir_intrinsic_op intrinsic,
                                      unsigned index);

   bool run_vectorizer(nir_variable_mode modes, bool cse=false,
                       nir_variable_mode robust_modes = (nir_variable_mode)0);

   nir_def *get_resource(uint32_t binding, bool ssbo);

   nir_intrinsic_instr *create_indirect_load(nir_variable_mode mode, uint32_t binding, nir_def *offset,
                                             uint32_t id, unsigned bit_size=32, unsigned components=1,
                                             unsigned access=0);
   void create_indirect_store(nir_variable_mode mode, uint32_t binding, nir_def *offset,
                              uint32_t id, unsigned bit_size=32, unsigned components=1,
                              unsigned wrmask=0xf, unsigned access=0);

   nir_intrinsic_instr *create_load(nir_variable_mode mode, uint32_t binding, uint32_t offset,
                                    uint32_t id, unsigned bit_size=32, unsigned components=1,
                                    unsigned access=0);
   void create_store(nir_variable_mode mode, uint32_t binding, uint32_t offset,
                     uint32_t id, unsigned bit_size=32, unsigned components=1, unsigned wrmask=0xf,
                     unsigned access=0);

   void create_shared_load(nir_deref_instr *deref, uint32_t id,
                           unsigned bit_size=32, unsigned components=1);
   void create_shared_store(nir_deref_instr *deref, uint32_t id,
                            unsigned bit_size=32, unsigned components=1, unsigned wrmask=0xf);

   bool test_alu(nir_instr *instr, nir_op op);
   bool test_alu_def(nir_instr *instr, unsigned index, nir_def *def, unsigned swizzle=0);

   static bool mem_vectorize_callback(unsigned align_mul, unsigned align_offset,
                                      unsigned bit_size,
                                      unsigned num_components,
                                      nir_intrinsic_instr *low, nir_intrinsic_instr *high,
                                      void *data);
   static void shared_type_info(const struct glsl_type *type, unsigned *size, unsigned *align);

   std::string swizzle(nir_alu_instr *instr, int src);

   std::map<unsigned, nir_alu_instr*> movs;
   std::map<unsigned, nir_alu_src*> loads;
   std::map<unsigned, nir_def*> res_map;
};

std::string
nir_load_store_vectorize_test::swizzle(nir_alu_instr *instr, int src)
{
   std::string swizzle;
   for (unsigned i = 0; i < nir_ssa_alu_instr_src_components(instr, src); i++) {
      swizzle += "xyzw"[instr->src[src].swizzle[i]];
   }

   return swizzle;
}

unsigned
nir_load_store_vectorize_test::count_intrinsics(nir_intrinsic_op intrinsic)
{
   unsigned count = 0;
   nir_foreach_block(block, b->impl) {
      nir_foreach_instr(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;
         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic == intrinsic)
            count++;
      }
   }
   return count;
}

nir_intrinsic_instr *
nir_load_store_vectorize_test::get_intrinsic(nir_intrinsic_op intrinsic,
                             unsigned index)
{
   nir_foreach_block(block, b->impl) {
      nir_foreach_instr(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;
         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic == intrinsic) {
            if (index == 0)
               return intrin;
            index--;
         }
      }
   }
   return NULL;
}

bool
nir_load_store_vectorize_test::run_vectorizer(nir_variable_mode modes,
                                              bool cse,
                                              nir_variable_mode robust_modes)
{
   if (modes & nir_var_mem_shared)
      nir_lower_vars_to_explicit_types(b->shader, nir_var_mem_shared, shared_type_info);

   nir_load_store_vectorize_options opts = { };
   opts.callback = mem_vectorize_callback;
   opts.modes = modes;
   opts.robust_modes = robust_modes;
   bool progress = nir_opt_load_store_vectorize(b->shader, &opts);

   if (progress) {
      nir_validate_shader(b->shader, NULL);
      if (cse)
         nir_opt_cse(b->shader);
      nir_copy_prop(b->shader);
      nir_opt_algebraic(b->shader);
      nir_opt_constant_folding(b->shader);
   }
   return progress;
}

nir_def *
nir_load_store_vectorize_test::get_resource(uint32_t binding, bool ssbo)
{
   if (res_map.count(binding))
      return res_map[binding];

   nir_intrinsic_instr *res = nir_intrinsic_instr_create(
      b->shader, nir_intrinsic_vulkan_resource_index);
   nir_def_init(&res->instr, &res->def, 1, 32);
   res->num_components = 1;
   res->src[0] = nir_src_for_ssa(nir_imm_zero(b, 1, 32));
   nir_intrinsic_set_desc_type(
      res, ssbo ? 7/*VK_DESCRIPTOR_TYPE_STORAGE_BUFFER*/ : 6/*VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER*/);
   nir_intrinsic_set_desc_set(res, 0);
   nir_intrinsic_set_binding(res, binding);
   nir_builder_instr_insert(b, &res->instr);
   res_map[binding] = &res->def;
   return &res->def;
}

nir_intrinsic_instr *
nir_load_store_vectorize_test::create_indirect_load(
   nir_variable_mode mode, uint32_t binding, nir_def *offset, uint32_t id,
   unsigned bit_size, unsigned components, unsigned access)
{
   nir_intrinsic_op intrinsic;
   nir_def *res = NULL;
   switch (mode) {
   case nir_var_mem_ubo:
      intrinsic = nir_intrinsic_load_ubo;
      res = get_resource(binding, false);
      break;
   case nir_var_mem_ssbo:
      intrinsic = nir_intrinsic_load_ssbo;
      res = get_resource(binding, true);
      break;
   case nir_var_mem_push_const:
      intrinsic = nir_intrinsic_load_push_constant;
      break;
   default:
      return NULL;
   }
   nir_intrinsic_instr *load = nir_intrinsic_instr_create(b->shader, intrinsic);
   nir_def_init(&load->instr, &load->def, components, bit_size);
   load->num_components = components;
   if (res) {
      load->src[0] = nir_src_for_ssa(res);
      load->src[1] = nir_src_for_ssa(offset);
   } else {
      load->src[0] = nir_src_for_ssa(offset);
   }
   int byte_size = (bit_size == 1 ? 32 : bit_size) / 8;

   nir_intrinsic_set_align(load, byte_size, 0);
   if (mode != nir_var_mem_push_const) {
      nir_intrinsic_set_access(load, (gl_access_qualifier)access);
   }

   if (nir_intrinsic_has_range_base(load)) {
      uint32_t range = byte_size * components;
      int offset_src = res ? 1 : 0;

      if (nir_src_is_const(load->src[offset_src])) {
         nir_intrinsic_set_range_base(load, nir_src_as_uint(load->src[offset_src]));
         nir_intrinsic_set_range(load, range);
      } else {
         /* Unknown range */
         nir_intrinsic_set_range_base(load, 0);
         nir_intrinsic_set_range(load, ~0);
      }
   }

   nir_builder_instr_insert(b, &load->instr);
   nir_alu_instr *mov = nir_instr_as_alu(nir_mov(b, &load->def)->parent_instr);
   movs[id] = mov;
   loads[id] = &mov->src[0];

   return load;
}

void
nir_load_store_vectorize_test::create_indirect_store(
   nir_variable_mode mode, uint32_t binding, nir_def *offset, uint32_t id,
   unsigned bit_size, unsigned components, unsigned wrmask, unsigned access)
{
   nir_const_value values[NIR_MAX_VEC_COMPONENTS];
   for (unsigned i = 0; i < components; i++)
      values[i] = nir_const_value_for_raw_uint((id << 4) | i, bit_size);
   nir_def *value = nir_build_imm(b, components, bit_size, values);

   nir_intrinsic_op intrinsic;
   nir_def *res = NULL;
   switch (mode) {
   case nir_var_mem_ssbo:
      intrinsic = nir_intrinsic_store_ssbo;
      res = get_resource(binding, true);
      break;
   case nir_var_mem_shared:
      intrinsic = nir_intrinsic_store_shared;
      break;
   default:
      return;
   }
   nir_intrinsic_instr *store = nir_intrinsic_instr_create(b->shader, intrinsic);
   nir_def_init(&store->instr, &store->def, components, bit_size);
   store->num_components = components;
   if (res) {
      store->src[0] = nir_src_for_ssa(value);
      store->src[1] = nir_src_for_ssa(res);
      store->src[2] = nir_src_for_ssa(offset);
   } else {
      store->src[0] = nir_src_for_ssa(value);
      store->src[1] = nir_src_for_ssa(offset);
   }
   nir_intrinsic_set_align(store, (bit_size == 1 ? 32 : bit_size) / 8, 0);
   nir_intrinsic_set_access(store, (gl_access_qualifier)access);
   nir_intrinsic_set_write_mask(store, wrmask & ((1 << components) - 1));
   nir_builder_instr_insert(b, &store->instr);
}

nir_intrinsic_instr *
nir_load_store_vectorize_test::create_load(
   nir_variable_mode mode, uint32_t binding, uint32_t offset, uint32_t id,
   unsigned bit_size, unsigned components, unsigned access)
{
   return create_indirect_load(mode, binding, nir_imm_int(b, offset), id, bit_size, components, access);
}

void
nir_load_store_vectorize_test::create_store(
   nir_variable_mode mode, uint32_t binding, uint32_t offset, uint32_t id,
   unsigned bit_size, unsigned components, unsigned wrmask, unsigned access)
{
   create_indirect_store(mode, binding, nir_imm_int(b, offset), id, bit_size, components, wrmask, access);
}

void nir_load_store_vectorize_test::create_shared_load(
   nir_deref_instr *deref, uint32_t id, unsigned bit_size, unsigned components)
{
   nir_def *load = nir_load_deref(b, deref);
   nir_alu_instr *mov = nir_instr_as_alu(nir_mov(b, load)->parent_instr);
   movs[id] = mov;
   loads[id] = &mov->src[0];
}

void nir_load_store_vectorize_test::create_shared_store(
   nir_deref_instr *deref, uint32_t id,
   unsigned bit_size, unsigned components, unsigned wrmask)
{
   nir_const_value values[NIR_MAX_VEC_COMPONENTS];
   for (unsigned i = 0; i < components; i++)
      values[i] = nir_const_value_for_raw_uint((id << 4) | i, bit_size);
   nir_def *value = nir_build_imm(b, components, bit_size, values);

   nir_store_deref(b, deref, value, wrmask & ((1 << components) - 1));
}

bool nir_load_store_vectorize_test::test_alu(nir_instr *instr, nir_op op)
{
   return instr->type == nir_instr_type_alu && nir_instr_as_alu(instr)->op == op;
}

bool nir_load_store_vectorize_test::test_alu_def(
   nir_instr *instr, unsigned index, nir_def *def, unsigned swizzle)
{
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(instr);

   if (index >= nir_op_infos[alu->op].num_inputs)
      return false;
   if (alu->src[index].src.ssa != def)
      return false;
   if (alu->src[index].swizzle[0] != swizzle)
      return false;

   return true;
}

bool nir_load_store_vectorize_test::mem_vectorize_callback(
   unsigned align_mul, unsigned align_offset, unsigned bit_size,
   unsigned num_components,
   nir_intrinsic_instr *low, nir_intrinsic_instr *high,
   void *data)
{
   /* Calculate a simple alignment, like how nir_intrinsic_align() does. */
   uint32_t align = align_mul;
   if (align_offset)
      align = 1 << (ffs(align_offset) - 1);

   /* Require scalar alignment and less than 5 components. */
   return align % (bit_size / 8) == 0 &&
          num_components <= 4;
}

void nir_load_store_vectorize_test::shared_type_info(
   const struct glsl_type *type, unsigned *size, unsigned *align)
{
   assert(glsl_type_is_vector_or_scalar(type));

   uint32_t comp_size = glsl_type_is_boolean(type)
      ? 4 : glsl_get_bit_size(type) / 8;
   unsigned length = glsl_get_vector_elements(type);
   *size = comp_size * length,
   *align = comp_size;
}
} // namespace

TEST_F(nir_load_store_vectorize_test, ubo_load_adjacent)
{
   create_load(nir_var_mem_ubo, 0, 0, 0x1);
   create_load(nir_var_mem_ubo, 0, 4, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ubo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ubo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ubo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ubo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);
   ASSERT_EQ(nir_intrinsic_range_base(load), 0);
   ASSERT_EQ(nir_intrinsic_range(load), 8);
   ASSERT_EQ(nir_src_as_uint(load->src[1]), 0);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");
}

TEST_F(nir_load_store_vectorize_test, ubo_load_intersecting)
{
   create_load(nir_var_mem_ubo, 0, 0, 0x1, 32, 2);
   create_load(nir_var_mem_ubo, 0, 4, 0x2, 32, 2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ubo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ubo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ubo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ubo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 3);
   ASSERT_EQ(nir_intrinsic_range_base(load), 0);
   ASSERT_EQ(nir_intrinsic_range(load), 12);
   ASSERT_EQ(nir_src_as_uint(load->src[1]), 0);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "xy");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "yz");
}

/* Test for a bug in range handling */
TEST_F(nir_load_store_vectorize_test, ubo_load_intersecting_range)
{
   create_load(nir_var_mem_ubo, 0, 0, 0x1, 32, 4);
   create_load(nir_var_mem_ubo, 0, 4, 0x2, 32, 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ubo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ubo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ubo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ubo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 4);
   ASSERT_EQ(nir_intrinsic_range_base(load), 0);
   ASSERT_EQ(nir_intrinsic_range(load), 16);
   ASSERT_EQ(nir_src_as_uint(load->src[1]), 0);
   ASSERT_EQ(loads[0x1]->src.ssa, &load->def);
   ASSERT_EQ(loads[0x2]->src.ssa, &load->def);
   ASSERT_EQ(loads[0x1]->swizzle[0], 0);
   ASSERT_EQ(loads[0x1]->swizzle[1], 1);
   ASSERT_EQ(loads[0x1]->swizzle[2], 2);
   ASSERT_EQ(loads[0x1]->swizzle[3], 3);
   ASSERT_EQ(loads[0x2]->swizzle[0], 1);
}

TEST_F(nir_load_store_vectorize_test, ubo_load_identical)
{
   create_load(nir_var_mem_ubo, 0, 0, 0x1);
   create_load(nir_var_mem_ubo, 0, 0, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ubo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ubo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ubo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ubo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 1);
   ASSERT_EQ(nir_intrinsic_range_base(load), 0);
   ASSERT_EQ(nir_intrinsic_range(load), 4);
   ASSERT_EQ(nir_src_as_uint(load->src[1]), 0);
   ASSERT_EQ(loads[0x1]->src.ssa, &load->def);
   ASSERT_EQ(loads[0x2]->src.ssa, &load->def);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "x");
}

TEST_F(nir_load_store_vectorize_test, ubo_load_large)
{
   create_load(nir_var_mem_ubo, 0, 0, 0x1, 32, 2);
   create_load(nir_var_mem_ubo, 0, 8, 0x2, 32, 3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ubo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ubo));

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ubo), 2);
}

TEST_F(nir_load_store_vectorize_test, push_const_load_adjacent)
{
   create_load(nir_var_mem_push_const, 0, 0, 0x1);
   create_load(nir_var_mem_push_const, 0, 4, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_push_const));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_push_constant, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);
   ASSERT_EQ(nir_src_as_uint(load->src[0]), 0);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");
}

TEST_F(nir_load_store_vectorize_test, push_const_load_adjacent_base)
{
   create_load(nir_var_mem_push_const, 0, 0, 0x1);
   nir_intrinsic_set_base(create_load(nir_var_mem_push_const, 0, 0, 0x2), 4);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_push_const));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_push_constant, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);
   ASSERT_EQ(nir_src_as_uint(load->src[0]), 0);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1);
   create_load(nir_var_mem_ssbo, 0, 4, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);
   ASSERT_EQ(nir_src_as_uint(load->src[1]), 0);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent_indirect)
{
   nir_def *index_base = nir_load_local_invocation_index(b);
   create_indirect_load(nir_var_mem_ssbo, 0, index_base, 0x1);
   create_indirect_load(nir_var_mem_ssbo, 0, nir_iadd_imm(b, index_base, 4), 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);
   ASSERT_EQ(load->src[1].ssa, index_base);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent_indirect_sub)
{
   nir_def *index_base = nir_load_local_invocation_index(b);
   nir_def *index_base_prev = nir_iadd_imm(b, index_base, 0xfffffffc);
   create_indirect_load(nir_var_mem_ssbo, 0, index_base_prev, 0x1);
   create_indirect_load(nir_var_mem_ssbo, 0, index_base, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);
   ASSERT_EQ(load->src[1].ssa, index_base_prev);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent_indirect_neg_stride)
{
   nir_def *inv = nir_load_local_invocation_index(b);
   nir_def *inv_plus_one = nir_iadd_imm(b, inv, 1);
   nir_def *index_base = nir_imul_imm(b, inv, 0xfffffffc);
   nir_def *index_base_prev = nir_imul_imm(b, inv_plus_one, 0xfffffffc);
   create_indirect_load(nir_var_mem_ssbo, 0, index_base_prev, 0x1);
   create_indirect_load(nir_var_mem_ssbo, 0, index_base, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");

   /* nir_opt_algebraic optimizes the imul */
   ASSERT_TRUE(test_alu(load->src[1].ssa->parent_instr, nir_op_ineg));
   nir_def *offset = nir_instr_as_alu(load->src[1].ssa->parent_instr)->src[0].src.ssa;
   ASSERT_TRUE(test_alu(offset->parent_instr, nir_op_ishl));
   nir_alu_instr *shl = nir_instr_as_alu(offset->parent_instr);
   ASSERT_EQ(shl->src[0].src.ssa, inv_plus_one);
   ASSERT_EQ(nir_src_as_uint(shl->src[1].src), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_identical_store_adjacent)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1);
   create_store(nir_var_mem_ssbo, 0, 4, 0x2);
   create_load(nir_var_mem_ssbo, 0, 0, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 1);
   ASSERT_EQ(nir_src_as_uint(load->src[1]), 0);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x3], load, "x");
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_identical_store_intersecting)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1, 32, 2);
   create_store(nir_var_mem_ssbo, 0, 4, 0x2);
   create_load(nir_var_mem_ssbo, 0, 0, 0x3, 32, 2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_identical_store_identical)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1);
   create_store(nir_var_mem_ssbo, 0, 0, 0x2);
   create_load(nir_var_mem_ssbo, 0, 0, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_identical_load_identical)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1);
   create_load(nir_var_mem_ssbo, 0, 0, 0x2);
   create_store(nir_var_mem_ssbo, 0, 0, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);
}

/* if nir_opt_load_store_vectorize were implemented like many load/store
 * optimization passes are (for example, nir_opt_combine_stores and
 * nir_opt_copy_prop_vars) and stopped tracking a load when an aliasing store is
 * encountered, this case wouldn't be optimized.
 * A similar test for derefs is shared_load_adjacent_store_identical. */
TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent_store_identical)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1);
   create_store(nir_var_mem_ssbo, 0, 0, 0x2);
   create_load(nir_var_mem_ssbo, 0, 4, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 1);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);
   ASSERT_EQ(nir_src_as_uint(load->src[1]), 0);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x3], load, "y");
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_adjacent)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1);
   create_store(nir_var_mem_ssbo, 0, 4, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 1);

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_ssbo, 0);
   ASSERT_EQ(nir_src_as_uint(store->src[2]), 0);
   ASSERT_EQ(nir_intrinsic_write_mask(store), 0x3);
   nir_def *val = store->src[0].ssa;
   ASSERT_EQ(val->bit_size, 32);
   ASSERT_EQ(val->num_components, 2);
   nir_const_value *cv = nir_instr_as_load_const(val->parent_instr)->value;
   ASSERT_EQ(nir_const_value_as_uint(cv[0], 32), 0x10);
   ASSERT_EQ(nir_const_value_as_uint(cv[1], 32), 0x20);
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_intersecting)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1, 32, 2);
   create_store(nir_var_mem_ssbo, 0, 4, 0x2, 32, 2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 1);

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_ssbo, 0);
   ASSERT_EQ(nir_src_as_uint(store->src[2]), 0);
   ASSERT_EQ(nir_intrinsic_write_mask(store), 0x7);
   nir_def *val = store->src[0].ssa;
   ASSERT_EQ(val->bit_size, 32);
   ASSERT_EQ(val->num_components, 3);
   nir_const_value *cv = nir_instr_as_load_const(val->parent_instr)->value;
   ASSERT_EQ(nir_const_value_as_uint(cv[0], 32), 0x10);
   ASSERT_EQ(nir_const_value_as_uint(cv[1], 32), 0x20);
   ASSERT_EQ(nir_const_value_as_uint(cv[2], 32), 0x21);
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_identical)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1);
   create_store(nir_var_mem_ssbo, 0, 0, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 1);

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_ssbo, 0);
   ASSERT_EQ(nir_src_as_uint(store->src[2]), 0);
   ASSERT_EQ(nir_intrinsic_write_mask(store), 0x1);
   nir_def *val = store->src[0].ssa;
   ASSERT_EQ(val->bit_size, 32);
   ASSERT_EQ(val->num_components, 1);
   ASSERT_EQ(nir_src_as_uint(store->src[0]), 0x20);
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_large)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1, 32, 2);
   create_store(nir_var_mem_ssbo, 0, 8, 0x2, 32, 3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ubo_load_adjacent_memory_barrier)
{
   create_load(nir_var_mem_ubo, 0, 0, 0x1);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_ACQ_REL,
                             nir_var_mem_ssbo);

   create_load(nir_var_mem_ubo, 0, 4, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ubo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ubo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ubo), 1);
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent_memory_barrier)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_ACQ_REL,
                             nir_var_mem_ssbo);

   create_load(nir_var_mem_ssbo, 0, 4, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
}

/* A control barrier may only sync invocations in a workgroup, it doesn't
 * require that loads/stores complete.
 */
TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent_barrier)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1);
   nir_barrier(b, SCOPE_WORKGROUP, SCOPE_NONE,
                      (nir_memory_semantics)0, (nir_variable_mode)0);
   create_load(nir_var_mem_ssbo, 0, 4, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent_memory_barrier_shared)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1);

   nir_scoped_memory_barrier(b, SCOPE_WORKGROUP, NIR_MEMORY_ACQ_REL,
                             nir_var_mem_shared);

   create_load(nir_var_mem_ssbo, 0, 4, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_adjacent_discard)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1);
   nir_discard(b);
   create_store(nir_var_mem_ssbo, 0, 4, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_adjacent_demote)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1);
   nir_demote(b);
   create_store(nir_var_mem_ssbo, 0, 4, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent_discard)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1);
   nir_discard(b);
   create_load(nir_var_mem_ssbo, 0, 4, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent_demote)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1);
   nir_demote(b);
   create_load(nir_var_mem_ssbo, 0, 4, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent_8_8_16)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1, 8);
   create_load(nir_var_mem_ssbo, 0, 1, 0x2, 8);
   create_load(nir_var_mem_ssbo, 0, 2, 0x3, 16);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 3);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 8);
   ASSERT_EQ(load->def.num_components, 4);
   ASSERT_EQ(nir_src_as_uint(load->src[1]), 0);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");

   nir_def *val = loads[0x3]->src.ssa;
   ASSERT_EQ(val->bit_size, 16);
   ASSERT_EQ(val->num_components, 1);
   ASSERT_TRUE(test_alu(val->parent_instr, nir_op_ior));
   nir_def *low = nir_instr_as_alu(val->parent_instr)->src[0].src.ssa;
   nir_def *high = nir_instr_as_alu(val->parent_instr)->src[1].src.ssa;
   ASSERT_TRUE(test_alu(high->parent_instr, nir_op_ishl));
   high = nir_instr_as_alu(high->parent_instr)->src[0].src.ssa;
   ASSERT_TRUE(test_alu(low->parent_instr, nir_op_u2u16));
   ASSERT_TRUE(test_alu(high->parent_instr, nir_op_u2u16));
   ASSERT_TRUE(test_alu_def(low->parent_instr, 0, &load->def, 2));
   ASSERT_TRUE(test_alu_def(high->parent_instr, 0, &load->def, 3));
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent_32_32_64)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1, 32, 2);
   create_load(nir_var_mem_ssbo, 0, 8, 0x2, 64);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 4);
   ASSERT_EQ(nir_src_as_uint(load->src[1]), 0);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "xy");

   nir_def *val = loads[0x2]->src.ssa;
   ASSERT_EQ(val->bit_size, 64);
   ASSERT_EQ(val->num_components, 1);
   ASSERT_TRUE(test_alu(val->parent_instr, nir_op_pack_64_2x32));
   nir_alu_instr *pack = nir_instr_as_alu(val->parent_instr);
   EXPECT_INSTR_SWIZZLES(pack, load, "zw");
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_adjacent_32_32_64_64)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1, 32, 2);
   create_load(nir_var_mem_ssbo, 0, 8, 0x2, 64);
   create_load(nir_var_mem_ssbo, 0, 16, 0x3, 64);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 3);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo, true));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 64);
   ASSERT_EQ(load->def.num_components, 3);
   ASSERT_EQ(nir_src_as_uint(load->src[1]), 0);
   EXPECT_INSTR_SWIZZLES(movs[0x3], load, "z");

   nir_def *val = loads[0x2]->src.ssa;
   ASSERT_EQ(val->bit_size, 64);
   ASSERT_EQ(val->num_components, 1);
   ASSERT_TRUE(test_alu(val->parent_instr, nir_op_mov));
   nir_alu_instr *mov = nir_instr_as_alu(val->parent_instr);
   EXPECT_INSTR_SWIZZLES(mov, load, "y");

   val = loads[0x1]->src.ssa;
   ASSERT_EQ(val->bit_size, 32);
   ASSERT_EQ(val->num_components, 2);
   ASSERT_TRUE(test_alu(val->parent_instr, nir_op_unpack_64_2x32));
   nir_alu_instr *unpack = nir_instr_as_alu(val->parent_instr);
   EXPECT_INSTR_SWIZZLES(unpack, load, "x");
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_intersecting_32_32_64)
{
   create_load(nir_var_mem_ssbo, 0, 4, 0x1, 32, 2);
   create_load(nir_var_mem_ssbo, 0, 8, 0x2, 64);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 3);
   ASSERT_EQ(nir_src_as_uint(load->src[1]), 4);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "xy");

   nir_def *val = loads[0x2]->src.ssa;
   ASSERT_EQ(val->bit_size, 64);
   ASSERT_EQ(val->num_components, 1);
   ASSERT_TRUE(test_alu(val->parent_instr, nir_op_pack_64_2x32));
   nir_alu_instr *pack = nir_instr_as_alu(val->parent_instr);
   EXPECT_INSTR_SWIZZLES(pack, load, "yz");
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_adjacent_8_8_16)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1, 8);
   create_store(nir_var_mem_ssbo, 0, 1, 0x2, 8);
   create_store(nir_var_mem_ssbo, 0, 2, 0x3, 16);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 3);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 1);

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_ssbo, 0);
   ASSERT_EQ(nir_src_as_uint(store->src[2]), 0);
   ASSERT_EQ(nir_intrinsic_write_mask(store), 0xf);
   nir_def *val = store->src[0].ssa;
   ASSERT_EQ(val->bit_size, 8);
   ASSERT_EQ(val->num_components, 4);
   nir_const_value *cv = nir_instr_as_load_const(val->parent_instr)->value;
   ASSERT_EQ(nir_const_value_as_uint(cv[0], 8), 0x10);
   ASSERT_EQ(nir_const_value_as_uint(cv[1], 8), 0x20);
   ASSERT_EQ(nir_const_value_as_uint(cv[2], 8), 0x30);
   ASSERT_EQ(nir_const_value_as_uint(cv[3], 8), 0x0);
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_adjacent_32_32_64)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1, 32, 2);
   create_store(nir_var_mem_ssbo, 0, 8, 0x2, 64);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 1);

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_ssbo, 0);
   ASSERT_EQ(nir_src_as_uint(store->src[2]), 0);
   ASSERT_EQ(nir_intrinsic_write_mask(store), 0xf);
   nir_def *val = store->src[0].ssa;
   ASSERT_EQ(val->bit_size, 32);
   ASSERT_EQ(val->num_components, 4);
   nir_const_value *cv = nir_instr_as_load_const(val->parent_instr)->value;
   ASSERT_EQ(nir_const_value_as_uint(cv[0], 32), 0x10);
   ASSERT_EQ(nir_const_value_as_uint(cv[1], 32), 0x11);
   ASSERT_EQ(nir_const_value_as_uint(cv[2], 32), 0x20);
   ASSERT_EQ(nir_const_value_as_uint(cv[3], 32), 0x0);
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_adjacent_32_32_64_64)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1, 32, 2);
   create_store(nir_var_mem_ssbo, 0, 8, 0x2, 64);
   create_store(nir_var_mem_ssbo, 0, 16, 0x3, 64);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 3);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 1);

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_ssbo, 0);
   ASSERT_EQ(nir_src_as_uint(store->src[2]), 0);
   ASSERT_EQ(nir_intrinsic_write_mask(store), 0x7);
   nir_def *val = store->src[0].ssa;
   ASSERT_EQ(val->bit_size, 64);
   ASSERT_EQ(val->num_components, 3);
   nir_const_value *cv = nir_instr_as_load_const(val->parent_instr)->value;
   ASSERT_EQ(nir_const_value_as_uint(cv[0], 64), 0x1100000010ull);
   ASSERT_EQ(nir_const_value_as_uint(cv[1], 64), 0x20);
   ASSERT_EQ(nir_const_value_as_uint(cv[2], 64), 0x30);
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_intersecting_32_32_64)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1, 32, 2);
   create_store(nir_var_mem_ssbo, 0, 4, 0x2, 64);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 1);

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_ssbo, 0);
   ASSERT_EQ(nir_src_as_uint(store->src[2]), 0);
   ASSERT_EQ(nir_intrinsic_write_mask(store), 0x7);
   nir_def *val = store->src[0].ssa;
   ASSERT_EQ(val->bit_size, 32);
   ASSERT_EQ(val->num_components, 3);
   nir_const_value *cv = nir_instr_as_load_const(val->parent_instr)->value;
   ASSERT_EQ(nir_const_value_as_uint(cv[0], 32), 0x10);
   ASSERT_EQ(nir_const_value_as_uint(cv[1], 32), 0x20);
   ASSERT_EQ(nir_const_value_as_uint(cv[2], 32), 0x0);
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_adjacent_32_64)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1, 32);
   create_store(nir_var_mem_ssbo, 0, 4, 0x2, 64, 2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_store_identical_wrmask)
{
   create_store(nir_var_mem_ssbo, 0, 0, 0x1, 32, 4, 1 | 4);
   create_store(nir_var_mem_ssbo, 0, 0, 0x2, 32, 4, 2 | 4 | 8);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_ssbo), 1);

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_ssbo, 0);
   ASSERT_EQ(nir_src_as_uint(store->src[2]), 0);
   ASSERT_EQ(nir_intrinsic_write_mask(store), 0xf);
   nir_def *val = store->src[0].ssa;
   ASSERT_EQ(val->bit_size, 32);
   ASSERT_EQ(val->num_components, 4);
   nir_const_value *cv = nir_instr_as_load_const(val->parent_instr)->value;
   ASSERT_EQ(nir_const_value_as_uint(cv[0], 32), 0x10);
   ASSERT_EQ(nir_const_value_as_uint(cv[1], 32), 0x21);
   ASSERT_EQ(nir_const_value_as_uint(cv[2], 32), 0x22);
   ASSERT_EQ(nir_const_value_as_uint(cv[3], 32), 0x23);
}

TEST_F(nir_load_store_vectorize_test, shared_load_adjacent)
{
   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared, glsl_array_type(glsl_uint_type(), 4, 0), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);

   create_shared_load(nir_build_deref_array_imm(b, deref, 0), 0x1);
   create_shared_load(nir_build_deref_array_imm(b, deref, 1), 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);

   deref = nir_src_as_deref(load->src[0]);
   ASSERT_EQ(deref->deref_type, nir_deref_type_cast);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_array);
   ASSERT_EQ(nir_src_as_uint(deref->arr.index), 0);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_var);
   ASSERT_EQ(deref->var, var);

   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");
}

TEST_F(nir_load_store_vectorize_test, shared_load_distant_64bit)
{
   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared, glsl_array_type(glsl_uint_type(), 4, 0), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);
   nir_def_init(&deref->instr, &deref->def, 1, 64);

   create_shared_load(nir_build_deref_array_imm(b, deref, 0x100000000), 0x1);
   create_shared_load(nir_build_deref_array_imm(b, deref, 0x200000001), 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   EXPECT_FALSE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);
}

TEST_F(nir_load_store_vectorize_test, shared_load_adjacent_indirect)
{
   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared, glsl_array_type(glsl_uint_type(), 4, 0), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);
   nir_def *index_base = nir_load_local_invocation_index(b);

   create_shared_load(nir_build_deref_array(b, deref, index_base), 0x1);
   create_shared_load(nir_build_deref_array(b, deref, nir_iadd_imm(b, index_base, 1)), 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);

   deref = nir_src_as_deref(load->src[0]);
   ASSERT_EQ(deref->deref_type, nir_deref_type_cast);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_array);
   ASSERT_EQ(deref->arr.index.ssa, index_base);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_var);
   ASSERT_EQ(deref->var, var);

   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");
}

TEST_F(nir_load_store_vectorize_test, shared_load_adjacent_indirect_sub)
{
   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared, glsl_array_type(glsl_uint_type(), 4, 0), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);
   nir_def *index_base = nir_load_local_invocation_index(b);
   nir_def *index_base_prev = nir_iadd_imm(b, index_base, 0xffffffff);

   create_shared_load(nir_build_deref_array(b, deref, index_base_prev), 0x1);
   create_shared_load(nir_build_deref_array(b, deref, index_base), 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);

   deref = nir_src_as_deref(load->src[0]);
   ASSERT_EQ(deref->deref_type, nir_deref_type_cast);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_array);
   ASSERT_EQ(deref->arr.index.ssa, index_base_prev);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_var);
   ASSERT_EQ(deref->var, var);

   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");
}

TEST_F(nir_load_store_vectorize_test, shared_load_struct)
{
   glsl_struct_field fields[2] = {glsl_struct_field(glsl_uint_type(), "field0"),
                                  glsl_struct_field(glsl_array_type(glsl_uint_type(), 4, 0), "field1")};

   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared, glsl_struct_type(fields, 2, "Struct", false), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);

   create_shared_load(nir_build_deref_struct(b, deref, 0), 0x1);
   create_shared_load(nir_build_deref_array_imm(b, nir_build_deref_struct(b, deref, 1), 0), 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);

   deref = nir_src_as_deref(load->src[0]);
   ASSERT_EQ(deref->deref_type, nir_deref_type_cast);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_struct);
   ASSERT_EQ(deref->strct.index, 0);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_var);
   ASSERT_EQ(deref->var, var);

   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");
}

TEST_F(nir_load_store_vectorize_test, shared_load_identical_store_adjacent)
{
   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared, glsl_array_type(glsl_uint_type(), 4, 0), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);

   create_shared_load(nir_build_deref_array_imm(b, deref, 0), 0x1);
   create_shared_store(nir_build_deref_array_imm(b, deref, 1), 0x2);
   create_shared_load(nir_build_deref_array_imm(b, deref, 0), 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 1);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 1);

   deref = nir_src_as_deref(load->src[0]);
   ASSERT_EQ(deref->deref_type, nir_deref_type_array);
   ASSERT_EQ(nir_src_as_uint(deref->arr.index), 0);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_var);
   ASSERT_EQ(deref->var, var);

   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x3], load, "x");
}

TEST_F(nir_load_store_vectorize_test, shared_load_identical_store_identical)
{
   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared, glsl_array_type(glsl_uint_type(), 4, 0), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);

   create_shared_load(nir_build_deref_array_imm(b, deref, 0), 0x1);
   create_shared_store(nir_build_deref_array_imm(b, deref, 0), 0x2);
   create_shared_load(nir_build_deref_array_imm(b, deref, 0), 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   EXPECT_FALSE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);
}

TEST_F(nir_load_store_vectorize_test, shared_load_adjacent_store_identical)
{
   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared, glsl_array_type(glsl_uint_type(), 4, 0), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);

   create_shared_load(nir_build_deref_array_imm(b, deref, 0), 0x1);
   create_shared_store(nir_build_deref_array_imm(b, deref, 0), 0x2);
   create_shared_load(nir_build_deref_array_imm(b, deref, 1), 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 1);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);

   deref = nir_src_as_deref(load->src[0]);
   ASSERT_EQ(deref->deref_type, nir_deref_type_cast);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_array);
   ASSERT_EQ(nir_src_as_uint(deref->arr.index), 0);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_var);
   ASSERT_EQ(deref->var, var);

   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x3], load, "y");
}

TEST_F(nir_load_store_vectorize_test, shared_load_bool)
{
   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared, glsl_array_type(glsl_bool_type(), 4, 0), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);

   create_shared_load(nir_build_deref_array_imm(b, deref, 0), 0x1, 1);
   create_shared_load(nir_build_deref_array_imm(b, deref, 1), 0x2, 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);

   deref = nir_src_as_deref(load->src[0]);
   ASSERT_EQ(deref->deref_type, nir_deref_type_cast);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_array);
   ASSERT_EQ(nir_src_as_uint(deref->arr.index), 0);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_var);
   ASSERT_EQ(deref->var, var);

   /* The loaded value is converted to Boolean by (loaded != 0). */
   ASSERT_TRUE(test_alu(loads[0x1]->src.ssa->parent_instr, nir_op_ine));
   ASSERT_TRUE(test_alu(loads[0x2]->src.ssa->parent_instr, nir_op_ine));
   ASSERT_TRUE(test_alu_def(loads[0x1]->src.ssa->parent_instr, 0, &load->def, 0));
   ASSERT_TRUE(test_alu_def(loads[0x2]->src.ssa->parent_instr, 0, &load->def, 1));
}

TEST_F(nir_load_store_vectorize_test, shared_load_bool_mixed)
{
   glsl_struct_field fields[2] = {glsl_struct_field(glsl_bool_type(), "field0"),
                                  glsl_struct_field(glsl_array_type(glsl_uint_type(), 4, 0), "field1")};

   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared, glsl_struct_type(fields, 2, "Struct", false), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);

   create_shared_load(nir_build_deref_struct(b, deref, 0), 0x1, 1);
   create_shared_load(nir_build_deref_array_imm(b, nir_build_deref_struct(b, deref, 1), 0), 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);

   deref = nir_src_as_deref(load->src[0]);
   ASSERT_EQ(deref->deref_type, nir_deref_type_cast);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_struct);
   ASSERT_EQ(deref->strct.index, 0);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_var);
   ASSERT_EQ(deref->var, var);

   /* The loaded value is converted to Boolean by (loaded != 0). */
   ASSERT_TRUE(test_alu(loads[0x1]->src.ssa->parent_instr, nir_op_ine));
   ASSERT_TRUE(test_alu_def(loads[0x1]->src.ssa->parent_instr, 0, &load->def, 0));

   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");
}

TEST_F(nir_load_store_vectorize_test, shared_store_adjacent)
{
   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared, glsl_array_type(glsl_uint_type(), 4, 0), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);

   create_shared_store(nir_build_deref_array_imm(b, deref, 0), 0x1);
   create_shared_store(nir_build_deref_array_imm(b, deref, 1), 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 1);

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_deref, 0);
   ASSERT_EQ(nir_intrinsic_write_mask(store), 0x3);
   nir_def *val = store->src[1].ssa;
   ASSERT_EQ(val->bit_size, 32);
   ASSERT_EQ(val->num_components, 2);
   nir_const_value *cv = nir_instr_as_load_const(val->parent_instr)->value;
   ASSERT_EQ(nir_const_value_as_uint(cv[0], 32), 0x10);
   ASSERT_EQ(nir_const_value_as_uint(cv[1], 32), 0x20);

   deref = nir_src_as_deref(store->src[0]);
   ASSERT_EQ(deref->deref_type, nir_deref_type_cast);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_array);
   ASSERT_EQ(nir_src_as_uint(deref->arr.index), 0);

   deref = nir_deref_instr_parent(deref);
   ASSERT_EQ(deref->deref_type, nir_deref_type_var);
   ASSERT_EQ(deref->var, var);
}

TEST_F(nir_load_store_vectorize_test, push_const_load_separate_base)
{
   create_load(nir_var_mem_push_const, 0, 0, 0x1);
   nir_intrinsic_set_base(create_load(nir_var_mem_push_const, 0, 4, 0x2), 4);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_push_const));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 2);
}

TEST_F(nir_load_store_vectorize_test, push_const_load_separate_direct_direct)
{
   create_load(nir_var_mem_push_const, 0, 0, 0x1);
   create_load(nir_var_mem_push_const, 0, 8, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_push_const));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 2);
}

TEST_F(nir_load_store_vectorize_test, push_const_load_separate_direct_indirect)
{
   nir_def *index_base = nir_load_local_invocation_index(b);
   create_load(nir_var_mem_push_const, 0, 0, 0x1);
   create_indirect_load(nir_var_mem_push_const, 0, index_base, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_push_const));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 2);
}

TEST_F(nir_load_store_vectorize_test, push_const_load_separate_indirect_indirect)
{
   nir_def *index_base = nir_load_local_invocation_index(b);
   create_indirect_load(nir_var_mem_push_const, 0,
      nir_iadd_imm(b, nir_imul_imm(b, nir_iadd_imm(b, index_base, 2), 16), 32), 0x1);
   create_indirect_load(nir_var_mem_push_const, 0,
      nir_iadd_imm(b, nir_imul_imm(b, nir_iadd_imm(b, index_base, 3), 16), 32), 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_push_const));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 2);
}

TEST_F(nir_load_store_vectorize_test, push_const_load_adjacent_complex_indirect)
{
   nir_def *index_base = nir_load_local_invocation_index(b);
   //vec4 pc[]; pc[gl_LocalInvocationIndex].w; pc[gl_LocalInvocationIndex+1].x;
   nir_def *low = nir_iadd_imm(b, nir_imul_imm(b, index_base, 16), 12);
   nir_def *high = nir_imul_imm(b, nir_iadd_imm(b, index_base, 1), 16);
   create_indirect_load(nir_var_mem_push_const, 0, low, 0x1);
   create_indirect_load(nir_var_mem_push_const, 0, high, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_push_const));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_push_constant), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_push_constant, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);
   ASSERT_EQ(load->src[0].ssa, low);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "y");
}

TEST_F(nir_load_store_vectorize_test, ssbo_alias0)
{
   nir_def *index_base = nir_load_local_invocation_index(b);
   create_load(nir_var_mem_ssbo, 0, 0, 0x1);
   create_indirect_store(nir_var_mem_ssbo, 0, index_base, 0x2);
   create_load(nir_var_mem_ssbo, 0, 0, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_alias1)
{
   nir_def *load_base = nir_load_global_invocation_index(b, 32);
   nir_def *store_base = nir_load_local_invocation_index(b);
   create_indirect_load(nir_var_mem_ssbo, 0, load_base, 0x1);
   create_indirect_store(nir_var_mem_ssbo, 0, store_base, 0x2);
   create_indirect_load(nir_var_mem_ssbo, 0, load_base, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_FALSE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, DISABLED_ssbo_alias2)
{
   /* TODO: try to combine these loads */
   nir_def *index_base = nir_load_local_invocation_index(b);
   nir_def *offset = nir_iadd_imm(b, nir_imul_imm(b, index_base, 16), 4);
   create_indirect_load(nir_var_mem_ssbo, 0, offset, 0x1);
   create_store(nir_var_mem_ssbo, 0, 0, 0x2);
   create_indirect_load(nir_var_mem_ssbo, 0, offset, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 1);
   ASSERT_EQ(load->src[1].ssa, offset);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x3], load, "x");
}

TEST_F(nir_load_store_vectorize_test, ssbo_alias3)
{
   /* these loads can be combined if nir_alu_instr::no_unsigned_wrap is set.
    * these loads can't be combined because if index_base == 268435455, then
    * offset == 0 because the addition would wrap around */
   nir_def *index_base = nir_load_local_invocation_index(b);
   nir_def *offset = nir_iadd_imm(b, nir_imul_imm(b, index_base, 16), 16);
   create_indirect_load(nir_var_mem_ssbo, 0, offset, 0x1);
   create_store(nir_var_mem_ssbo, 0, 0, 0x2);
   create_indirect_load(nir_var_mem_ssbo, 0, offset, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, DISABLED_ssbo_alias4)
{
   /* TODO: try to combine these loads */
   nir_def *index_base = nir_load_local_invocation_index(b);
   nir_def *offset = nir_iadd_imm(b, nir_imul_imm(b, index_base, 16), 16);
   nir_instr_as_alu(offset->parent_instr)->no_unsigned_wrap = true;
   create_indirect_load(nir_var_mem_ssbo, 0, offset, 0x1);
   create_store(nir_var_mem_ssbo, 0, 0, 0x2);
   create_indirect_load(nir_var_mem_ssbo, 0, offset, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 1);
   ASSERT_EQ(load->src[1].ssa, offset);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x3], load, "x");
}

TEST_F(nir_load_store_vectorize_test, ssbo_alias5)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1);
   create_store(nir_var_mem_ssbo, 1, 0, 0x2);
   create_load(nir_var_mem_ssbo, 0, 0, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_alias6)
{
   create_load(nir_var_mem_ssbo, 0, 0, 0x1, 32, 1, ACCESS_RESTRICT);
   create_store(nir_var_mem_ssbo, 1, 0, 0x2, 32, 1, 0xf, ACCESS_RESTRICT);
   create_load(nir_var_mem_ssbo, 0, 0, 0x3, 32, 1, ACCESS_RESTRICT);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 1);
   ASSERT_EQ(nir_src_as_uint(load->src[1]), 0);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x3], load, "x");
}

TEST_F(nir_load_store_vectorize_test, DISABLED_shared_alias0)
{
   /* TODO: implement type-based alias analysis so that these loads can be
    * combined. this is made a bit more difficult than simply using
    * nir_compare_derefs() because the vectorizer creates loads/stores with
    * casted derefs. The solution would probably be to keep multiple derefs for
    * an entry (one for each load/store combined into it). */
   glsl_struct_field fields[2] = {glsl_struct_field(glsl_array_type(glsl_uint_type(), 4, 0), "field0"),
                                  glsl_struct_field(glsl_array_type(glsl_uint_type(), 4, 0), "field1")};

   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared, glsl_struct_type(fields, 2, "Struct", false), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);

   nir_def *index0 = nir_load_local_invocation_index(b);
   nir_def *index1 = nir_load_global_invocation_index(b, 32);
   nir_deref_instr *load_deref = nir_build_deref_array(b, nir_build_deref_struct(b, deref, 0), index0);

   create_shared_load(load_deref, 0x1);
   create_shared_store(nir_build_deref_array(b, nir_build_deref_struct(b, deref, 1), index1), 0x2);
   create_shared_load(load_deref, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 1);
   ASSERT_EQ(load->src[0].ssa, &load_deref->def);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x3], load, "x");
}

TEST_F(nir_load_store_vectorize_test, shared_alias1)
{
   nir_variable *var0 = nir_variable_create(b->shader, nir_var_mem_shared, glsl_uint_type(), "var0");
   nir_variable *var1 = nir_variable_create(b->shader, nir_var_mem_shared, glsl_uint_type(), "var1");
   nir_deref_instr *load_deref = nir_build_deref_var(b, var0);

   create_shared_load(load_deref, 0x1);
   create_shared_store(nir_build_deref_var(b, var1), 0x2);
   create_shared_load(load_deref, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 1);
   ASSERT_EQ(load->src[0].ssa, &load_deref->def);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x3], load, "x");
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_distant_64bit)
{
   create_indirect_load(nir_var_mem_ssbo, 0, nir_imm_int64(b, 0x100000000), 0x1);
   create_indirect_load(nir_var_mem_ssbo, 0, nir_imm_int64(b, 0x200000004), 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_load_distant_indirect_64bit)
{
   nir_def *index_base = nir_u2u64(b, nir_load_local_invocation_index(b));
   nir_def *first = nir_imul_imm(b, index_base, 0x100000000);
   nir_def *second = nir_imul_imm(b, index_base, 0x200000000);
   create_indirect_load(nir_var_mem_ssbo, 0, first, 0x1);
   create_indirect_load(nir_var_mem_ssbo, 0, second, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_offset_overflow_robust)
{
   create_load(nir_var_mem_ssbo, 0, 0xfffffffc, 0x1);
   create_load(nir_var_mem_ssbo, 0, 0x0, 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo, false, nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_offset_overflow_robust_indirect_stride1)
{
   nir_def *offset = nir_load_local_invocation_index(b);
   create_indirect_load(nir_var_mem_ssbo, 0, offset, 0x1);
   create_indirect_load(nir_var_mem_ssbo, 0, nir_iadd_imm(b, offset, 4), 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_FALSE(run_vectorizer(nir_var_mem_ssbo, false, nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);
}

TEST_F(nir_load_store_vectorize_test, ssbo_offset_overflow_robust_indirect_stride8)
{
   nir_def *offset = nir_load_local_invocation_index(b);
   offset = nir_imul_imm(b, offset, 8);
   create_indirect_load(nir_var_mem_ssbo, 0, offset, 0x1);
   create_indirect_load(nir_var_mem_ssbo, 0, nir_iadd_imm(b, offset, 4), 0x2);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo, false, nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);
}

TEST_F(nir_load_store_vectorize_test, ssbo_offset_overflow_robust_indirect_stride12)
{
   nir_def *offset = nir_load_local_invocation_index(b);
   offset = nir_imul_imm(b, offset, 12);
   create_indirect_load(nir_var_mem_ssbo, 0, offset, 0x1);
   nir_def *offset_4 = nir_iadd_imm(b, offset, 4);
   create_indirect_load(nir_var_mem_ssbo, 0, offset_4, 0x2);
   create_indirect_load(nir_var_mem_ssbo, 0, nir_iadd_imm(b, offset, 8), 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 3);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo, false, nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 2);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_ssbo, 0);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 1);
   ASSERT_EQ(load->src[1].ssa, offset);
   EXPECT_INSTR_SWIZZLES(movs[0x1], load, "x");

   load = get_intrinsic(nir_intrinsic_load_ssbo, 1);
   ASSERT_EQ(load->def.bit_size, 32);
   ASSERT_EQ(load->def.num_components, 2);
   ASSERT_EQ(load->src[1].ssa, offset_4);
   EXPECT_INSTR_SWIZZLES(movs[0x2], load, "x");
   EXPECT_INSTR_SWIZZLES(movs[0x3], load, "y");
}

TEST_F(nir_load_store_vectorize_test, ssbo_offset_overflow_robust_indirect_stride16)
{
   nir_def *offset = nir_load_local_invocation_index(b);
   offset = nir_imul_imm(b, offset, 16);
   create_indirect_load(nir_var_mem_ssbo, 0, offset, 0x1);
   create_indirect_load(nir_var_mem_ssbo, 0, nir_iadd_imm(b, offset, 4), 0x2);
   create_indirect_load(nir_var_mem_ssbo, 0, nir_iadd_imm(b, offset, 8), 0x3);
   create_indirect_load(nir_var_mem_ssbo, 0, nir_iadd_imm(b, offset, 12), 0x4);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 4);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ssbo, false, nir_var_mem_ssbo));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_ssbo), 1);
}

TEST_F(nir_load_store_vectorize_test, shared_offset_overflow_robust_indirect_stride12)
{
   nir_variable *var = nir_variable_create(b->shader, nir_var_mem_shared,
                                           glsl_array_type(glsl_uint_type(), 4, 0), "var");
   nir_deref_instr *deref = nir_build_deref_var(b, var);

   nir_def *index = nir_load_local_invocation_index(b);
   index = nir_imul_imm(b, index, 3);
   create_shared_load(nir_build_deref_array(b, deref, index), 0x1);
   create_shared_load(nir_build_deref_array(b, deref, nir_iadd_imm(b, index, 1)), 0x2);
   create_shared_load(nir_build_deref_array(b, deref, nir_iadd_imm(b, index, 2)), 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 3);

   EXPECT_FALSE(run_vectorizer(nir_var_mem_shared, false, nir_var_mem_shared));

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 3);
}

TEST_F(nir_load_store_vectorize_test, ubo_alignment_16_4)
{
   nir_def *offset = nir_load_local_invocation_index(b);
   offset = nir_imul_imm(b, offset, 16);
   offset = nir_iadd_imm(b, offset, 4);
   nir_intrinsic_instr *load = create_indirect_load(nir_var_mem_ubo, 0, offset,
                                                    0x1);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ubo));
   EXPECT_EQ(nir_intrinsic_align_mul(load), 16);
   EXPECT_EQ(nir_intrinsic_align_offset(load), 4);
}

TEST_F(nir_load_store_vectorize_test, ubo_alignment_16_4_swapped)
{
   nir_def *offset = nir_load_local_invocation_index(b);
   offset = nir_iadd_imm(b, offset, 1);
   offset = nir_imul_imm(b, offset, 16);
   offset = nir_iadd_imm(b, offset, 4);
   nir_intrinsic_instr *load =
      create_indirect_load(nir_var_mem_ubo, 0, offset, 0x1);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ubo));
   EXPECT_EQ(nir_intrinsic_align_mul(load), 16);
   EXPECT_EQ(nir_intrinsic_align_offset(load), 4);
}

/* Check offset % mul != 0 */
TEST_F(nir_load_store_vectorize_test, ubo_alignment_16_20)
{
   nir_def *offset = nir_load_local_invocation_index(b);
   offset = nir_imul_imm(b, offset, 16);
   offset = nir_iadd_imm(b, offset, 20);
   nir_intrinsic_instr *load = create_indirect_load(nir_var_mem_ubo, 0, offset,
                                                    0x1);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ubo));
   EXPECT_EQ(nir_intrinsic_align_mul(load), 16);
   EXPECT_EQ(nir_intrinsic_align_offset(load), 4);
}

/* Check that we don't upgrade to non-power-of-two alignments. */
TEST_F(nir_load_store_vectorize_test, ubo_alignment_24_4)
{
   nir_def *offset = nir_load_local_invocation_index(b);
   offset = nir_imul_imm(b, offset, 24);
   offset = nir_iadd_imm(b, offset, 4);
   nir_intrinsic_instr *load =
      create_indirect_load(nir_var_mem_ubo, 0, offset, 0x1);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ubo));
   EXPECT_EQ(nir_intrinsic_align_mul(load), 8);
   EXPECT_EQ(nir_intrinsic_align_offset(load), 4);
}

/* Check that we don't upgrade to non-power-of-two alignments. */
TEST_F(nir_load_store_vectorize_test, ubo_alignment_64_16_8)
{
   nir_def *x = nir_imul_imm(b, nir_load_local_invocation_index(b), 64);
   nir_def *y = nir_imul_imm(b, nir_load_instance_id(b), 16);
   nir_def *offset = nir_iadd(b, x, y);
   offset = nir_iadd_imm(b, offset, 8);
   nir_intrinsic_instr *load =
      create_indirect_load(nir_var_mem_ubo, 0, offset, 0x1);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ubo));
   EXPECT_EQ(nir_intrinsic_align_mul(load), 16);
   EXPECT_EQ(nir_intrinsic_align_offset(load), 8);
}

TEST_F(nir_load_store_vectorize_test, ubo_alignment_const_100)
{
   nir_intrinsic_instr *load =
      create_indirect_load(nir_var_mem_ubo, 0, nir_imm_int(b, 100), 0x1);

   EXPECT_TRUE(run_vectorizer(nir_var_mem_ubo));
   EXPECT_EQ(nir_intrinsic_align_mul(load), NIR_ALIGN_MUL_MAX);
   EXPECT_EQ(nir_intrinsic_align_offset(load), 100);
}
