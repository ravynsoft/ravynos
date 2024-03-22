/*
 * Copyright © 2016 Intel Corporation
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
#include <gtest/gtest.h>
#include "ir.h"
#include "ir_array_refcount.h"
#include "ir_builder.h"
#include "util/hash_table.h"

using namespace ir_builder;

class array_refcount_test : public ::testing::Test {
public:
   virtual void SetUp();
   virtual void TearDown();

   exec_list instructions;
   ir_factory *body;
   void *mem_ctx;

   /**
    * glsl_type for a vec4[3][4][5].
    *
    * The exceptionally verbose name is picked because it matches the syntax
    * of http://cdecl.org/.
    */
   const glsl_type *array_3_of_array_4_of_array_5_of_vec4;

   /**
    * glsl_type for a int[3].
    *
    * The exceptionally verbose name is picked because it matches the syntax
    * of http://cdecl.org/.
    */
   const glsl_type *array_3_of_int;

   /**
    * Wrapper to access private member "bits" of ir_array_refcount_entry
    *
    * The test class is a friend to ir_array_refcount_entry, but the
    * individual tests are not part of the class.  Since the friendliness of
    * the test class does not extend to the tests, provide a wrapper.
    */
   const BITSET_WORD *get_bits(const ir_array_refcount_entry &entry)
   {
      return entry.bits;
   }

   /**
    * Wrapper to access private member "num_bits" of ir_array_refcount_entry
    *
    * The test class is a friend to ir_array_refcount_entry, but the
    * individual tests are not part of the class.  Since the friendliness of
    * the test class does not extend to the tests, provide a wrapper.
    */
   unsigned get_num_bits(const ir_array_refcount_entry &entry)
   {
      return entry.num_bits;
   }

   /**
    * Wrapper to access private member "array_depth" of ir_array_refcount_entry
    *
    * The test class is a friend to ir_array_refcount_entry, but the
    * individual tests are not part of the class.  Since the friendliness of
    * the test class does not extend to the tests, provide a wrapper.
    */
   unsigned get_array_depth(const ir_array_refcount_entry &entry)
   {
      return entry.array_depth;
   }
};

void
array_refcount_test::SetUp()
{
   glsl_type_singleton_init_or_ref();

   mem_ctx = ralloc_context(NULL);

   instructions.make_empty();
   body = new ir_factory(&instructions, mem_ctx);

   /* The type of vec4 x[3][4][5]; */
   const glsl_type *const array_5_of_vec4 =
      glsl_array_type(&glsl_type_builtin_vec4, 5, 0);
   const glsl_type *const array_4_of_array_5_of_vec4 =
      glsl_array_type(array_5_of_vec4, 4, 0);
   array_3_of_array_4_of_array_5_of_vec4 =
      glsl_array_type(array_4_of_array_5_of_vec4, 3, 0);

   array_3_of_int = glsl_array_type(&glsl_type_builtin_int, 3, 0);
}

void
array_refcount_test::TearDown()
{
   delete body;
   body = NULL;

   ralloc_free(mem_ctx);
   mem_ctx = NULL;

   glsl_type_singleton_decref();
}

static operand
deref_array(operand array, operand index)
{
   void *mem_ctx = ralloc_parent(array.val);

   ir_rvalue *val = new(mem_ctx) ir_dereference_array(array.val, index.val);

   return operand(val);
}

static operand
deref_struct(operand s, const char *field)
{
   void *mem_ctx = ralloc_parent(s.val);

   ir_rvalue *val = new(mem_ctx) ir_dereference_record(s.val, field);

   return operand(val);
}

/**
 * Verify that only the specified set of ir_variables exists in the hash table
 */
static void
validate_variables_in_hash_table(struct hash_table *ht,
                                 unsigned count,
                                 ...)
{
   ir_variable **vars = new ir_variable *[count];
   va_list args;

   /* Make a copy of the list of expected ir_variables.  The copied list can
    * be modified during the checking.
    */
   va_start(args, count);

   for (unsigned i = 0; i < count; i++)
      vars[i] = va_arg(args, ir_variable *);

   va_end(args);

   hash_table_foreach(ht, entry) {
      const ir_instruction *const ir = (ir_instruction *) entry->key;
      const ir_variable *const v = ir->as_variable();

      if (v == NULL) {
         ADD_FAILURE() << "Invalid junk in hash table: ir_type = "
                       << ir->ir_type << ", address = "
                       << (void *) ir;
         continue;
      }

      unsigned i;
      for (i = 0; i < count; i++) {
         if (vars[i] == NULL)
            continue;

         if (vars[i] == v)
            break;
      }

      if (i == count) {
            ADD_FAILURE() << "Invalid variable in hash table: \""
                          << v->name << "\"";
      } else {
         /* As each variable is encountered, remove it from the set.  Don't
          * bother compacting the set because we don't care about
          * performance here.
          */
         vars[i] = NULL;
      }
   }

   /* Check that there's nothing left in the set. */
   for (unsigned i = 0; i < count; i++) {
      if (vars[i] != NULL) {
         ADD_FAILURE() << "Variable was not in the hash table: \""
                          << vars[i]->name << "\"";
      }
   }

   delete [] vars;
}

TEST_F(array_refcount_test, ir_array_refcount_entry_initial_state_for_scalar)
{
   ir_variable *const var =
      new(mem_ctx) ir_variable(&glsl_type_builtin_int, "a", ir_var_auto);

   ir_array_refcount_entry entry(var);

   ASSERT_NE((void *)0, get_bits(entry));
   EXPECT_FALSE(entry.is_referenced);
   EXPECT_EQ(1, get_num_bits(entry));
   EXPECT_EQ(0, get_array_depth(entry));
   EXPECT_FALSE(entry.is_linearized_index_referenced(0));
}

TEST_F(array_refcount_test, ir_array_refcount_entry_initial_state_for_vector)
{
   ir_variable *const var =
      new(mem_ctx) ir_variable(&glsl_type_builtin_vec4, "a", ir_var_auto);

   ir_array_refcount_entry entry(var);

   ASSERT_NE((void *)0, get_bits(entry));
   EXPECT_FALSE(entry.is_referenced);
   EXPECT_EQ(1, get_num_bits(entry));
   EXPECT_EQ(0, get_array_depth(entry));
   EXPECT_FALSE(entry.is_linearized_index_referenced(0));
}

TEST_F(array_refcount_test, ir_array_refcount_entry_initial_state_for_matrix)
{
   ir_variable *const var =
      new(mem_ctx) ir_variable(&glsl_type_builtin_mat4, "a", ir_var_auto);

   ir_array_refcount_entry entry(var);

   ASSERT_NE((void *)0, get_bits(entry));
   EXPECT_FALSE(entry.is_referenced);
   EXPECT_EQ(1, get_num_bits(entry));
   EXPECT_EQ(0, get_array_depth(entry));
   EXPECT_FALSE(entry.is_linearized_index_referenced(0));
}

TEST_F(array_refcount_test, ir_array_refcount_entry_initial_state_for_array)
{
   ir_variable *const var =
      new(mem_ctx) ir_variable(array_3_of_array_4_of_array_5_of_vec4,
                               "a",
                               ir_var_auto);
   const unsigned total_elements = glsl_get_aoa_size(var->type);

   ir_array_refcount_entry entry(var);

   ASSERT_NE((void *)0, get_bits(entry));
   EXPECT_FALSE(entry.is_referenced);
   EXPECT_EQ(total_elements, get_num_bits(entry));
   EXPECT_EQ(3, get_array_depth(entry));

   for (unsigned i = 0; i < total_elements; i++)
      EXPECT_FALSE(entry.is_linearized_index_referenced(i)) << "index = " << i;
}

TEST_F(array_refcount_test, mark_array_elements_referenced_simple)
{
   ir_variable *const var =
      new(mem_ctx) ir_variable(array_3_of_array_4_of_array_5_of_vec4,
                               "a",
                               ir_var_auto);
   const unsigned total_elements = glsl_get_aoa_size(var->type);

   ir_array_refcount_entry entry(var);

   static const array_deref_range dr[] = {
      { 0, 5 }, { 1, 4 }, { 2, 3 }
   };
   const unsigned accessed_element = 0 + (1 * 5) + (2 * 4 * 5);

   link_util_mark_array_elements_referenced(dr, 3, entry.array_depth,
                                            entry.bits);

   for (unsigned i = 0; i < total_elements; i++)
      EXPECT_EQ(i == accessed_element, entry.is_linearized_index_referenced(i));
}

TEST_F(array_refcount_test, mark_array_elements_referenced_whole_first_array)
{
   ir_variable *const var =
      new(mem_ctx) ir_variable(array_3_of_array_4_of_array_5_of_vec4,
                               "a",
                               ir_var_auto);

   ir_array_refcount_entry entry(var);

   static const array_deref_range dr[] = {
      { 0, 5 }, { 1, 4 }, { 3, 3 }
   };

   link_util_mark_array_elements_referenced(dr, 3, entry.array_depth,
                                            entry.bits);

   for (unsigned i = 0; i < 3; i++) {
      for (unsigned j = 0; j < 4; j++) {
         for (unsigned k = 0; k < 5; k++) {
            const bool accessed = (j == 1) && (k == 0);
            const unsigned linearized_index = k + (j * 5) + (i * 4 * 5);

            EXPECT_EQ(accessed,
                      entry.is_linearized_index_referenced(linearized_index));
         }
      }
   }
}

TEST_F(array_refcount_test, mark_array_elements_referenced_whole_second_array)
{
   ir_variable *const var =
      new(mem_ctx) ir_variable(array_3_of_array_4_of_array_5_of_vec4,
                               "a",
                               ir_var_auto);

   ir_array_refcount_entry entry(var);

   static const array_deref_range dr[] = {
      { 0, 5 }, { 4, 4 }, { 1, 3 }
   };

   link_util_mark_array_elements_referenced(dr, 3, entry.array_depth,
                                            entry.bits);

   for (unsigned i = 0; i < 3; i++) {
      for (unsigned j = 0; j < 4; j++) {
         for (unsigned k = 0; k < 5; k++) {
            const bool accessed = (i == 1) && (k == 0);
            const unsigned linearized_index = k + (j * 5) + (i * 4 * 5);

            EXPECT_EQ(accessed,
                      entry.is_linearized_index_referenced(linearized_index));
         }
      }
   }
}

TEST_F(array_refcount_test, mark_array_elements_referenced_whole_third_array)
{
   ir_variable *const var =
      new(mem_ctx) ir_variable(array_3_of_array_4_of_array_5_of_vec4,
                               "a",
                               ir_var_auto);

   ir_array_refcount_entry entry(var);

   static const array_deref_range dr[] = {
      { 5, 5 }, { 2, 4 }, { 1, 3 }
   };

   link_util_mark_array_elements_referenced(dr, 3, entry.array_depth,
                                            entry.bits);

   for (unsigned i = 0; i < 3; i++) {
      for (unsigned j = 0; j < 4; j++) {
         for (unsigned k = 0; k < 5; k++) {
            const bool accessed = (i == 1) && (j == 2);
            const unsigned linearized_index = k + (j * 5) + (i * 4 * 5);

            EXPECT_EQ(accessed,
                      entry.is_linearized_index_referenced(linearized_index));
         }
      }
   }
}

TEST_F(array_refcount_test, mark_array_elements_referenced_whole_first_and_third_arrays)
{
   ir_variable *const var =
      new(mem_ctx) ir_variable(array_3_of_array_4_of_array_5_of_vec4,
                               "a",
                               ir_var_auto);

   ir_array_refcount_entry entry(var);

   static const array_deref_range dr[] = {
      { 5, 5 }, { 3, 4 }, { 3, 3 }
   };

   link_util_mark_array_elements_referenced(dr, 3, entry.array_depth,
                                            entry.bits);

   for (unsigned i = 0; i < 3; i++) {
      for (unsigned j = 0; j < 4; j++) {
         for (unsigned k = 0; k < 5; k++) {
            const bool accessed = (j == 3);
            const unsigned linearized_index = k + (j * 5) + (i * 4 * 5);

            EXPECT_EQ(accessed,
                      entry.is_linearized_index_referenced(linearized_index));
         }
      }
   }
}

TEST_F(array_refcount_test, do_not_process_vector_indexing)
{
   /* Vectors and matrices can also be indexed in much the same manner as
    * arrays.  The visitor should not try to track per-element accesses to
    * these types.
    */
   ir_variable *var_a = new(mem_ctx) ir_variable(&glsl_type_builtin_float,
                                                 "a",
                                                 ir_var_auto);
   ir_variable *var_b = new(mem_ctx) ir_variable(&glsl_type_builtin_int,
                                                 "b",
                                                 ir_var_auto);
   ir_variable *var_c = new(mem_ctx) ir_variable(&glsl_type_builtin_vec4,
                                                 "c",
                                                 ir_var_auto);

   body->emit(assign(var_a, deref_array(var_c, var_b)));

   ir_array_refcount_visitor v;

   visit_list_elements(&v, &instructions);

   ir_array_refcount_entry *entry_a = v.get_variable_entry(var_a);
   ir_array_refcount_entry *entry_b = v.get_variable_entry(var_b);
   ir_array_refcount_entry *entry_c = v.get_variable_entry(var_c);

   EXPECT_TRUE(entry_a->is_referenced);
   EXPECT_TRUE(entry_b->is_referenced);
   EXPECT_TRUE(entry_c->is_referenced);

   /* As validated by previous tests, for non-array types, num_bits is 1. */
   ASSERT_EQ(1, get_num_bits(*entry_c));
   EXPECT_FALSE(entry_c->is_linearized_index_referenced(0));
}

TEST_F(array_refcount_test, do_not_process_matrix_indexing)
{
   /* Vectors and matrices can also be indexed in much the same manner as
    * arrays.  The visitor should not try to track per-element accesses to
    * these types.
    */
   ir_variable *var_a = new(mem_ctx) ir_variable(&glsl_type_builtin_vec4,
                                                 "a",
                                                 ir_var_auto);
   ir_variable *var_b = new(mem_ctx) ir_variable(&glsl_type_builtin_int,
                                                 "b",
                                                 ir_var_auto);
   ir_variable *var_c = new(mem_ctx) ir_variable(&glsl_type_builtin_mat4,
                                                 "c",
                                                 ir_var_auto);

   body->emit(assign(var_a, deref_array(var_c, var_b)));

   ir_array_refcount_visitor v;

   visit_list_elements(&v, &instructions);

   ir_array_refcount_entry *entry_a = v.get_variable_entry(var_a);
   ir_array_refcount_entry *entry_b = v.get_variable_entry(var_b);
   ir_array_refcount_entry *entry_c = v.get_variable_entry(var_c);

   EXPECT_TRUE(entry_a->is_referenced);
   EXPECT_TRUE(entry_b->is_referenced);
   EXPECT_TRUE(entry_c->is_referenced);

   /* As validated by previous tests, for non-array types, num_bits is 1. */
   ASSERT_EQ(1, get_num_bits(*entry_c));
   EXPECT_FALSE(entry_c->is_linearized_index_referenced(0));
}

TEST_F(array_refcount_test, do_not_process_array_inside_structure)
{
   /* Structures can contain arrays.  The visitor should not try to track
    * per-element accesses to arrays contained inside structures.
    */
   const glsl_struct_field fields[] = {
      glsl_struct_field(array_3_of_int, "i"),
   };

   const glsl_type *const record_of_array_3_of_int =
      glsl_struct_type(fields, ARRAY_SIZE(fields), "S", false /* packed */);

   ir_variable *var_a = new(mem_ctx) ir_variable(&glsl_type_builtin_int,
                                                 "a",
                                                 ir_var_auto);

   ir_variable *var_b = new(mem_ctx) ir_variable(record_of_array_3_of_int,
                                                 "b",
                                                 ir_var_auto);

   /* a = b.i[2] */
   body->emit(assign(var_a,
                     deref_array(
                        deref_struct(var_b, "i"),
                        body->constant(int(2)))));

   ir_array_refcount_visitor v;

   visit_list_elements(&v, &instructions);

   ir_array_refcount_entry *entry_a = v.get_variable_entry(var_a);
   ir_array_refcount_entry *entry_b = v.get_variable_entry(var_b);

   EXPECT_TRUE(entry_a->is_referenced);
   EXPECT_TRUE(entry_b->is_referenced);

   ASSERT_EQ(1, get_num_bits(*entry_b));
   EXPECT_FALSE(entry_b->is_linearized_index_referenced(0));

   validate_variables_in_hash_table(v.ht, 2, var_a, var_b);
}

TEST_F(array_refcount_test, visit_simple_indexing)
{
   ir_variable *var_a = new(mem_ctx) ir_variable(&glsl_type_builtin_vec4,
                                                 "a",
                                                 ir_var_auto);
   ir_variable *var_b = new(mem_ctx) ir_variable(array_3_of_array_4_of_array_5_of_vec4,
                                                 "b",
                                                 ir_var_auto);

   /* a = b[2][1][0] */
   body->emit(assign(var_a,
                     deref_array(
                        deref_array(
                           deref_array(var_b, body->constant(int(2))),
                           body->constant(int(1))),
                        body->constant(int(0)))));

   ir_array_refcount_visitor v;

   visit_list_elements(&v, &instructions);

   const unsigned accessed_element = 0 + (1 * 5) + (2 * 4 * 5);
   ir_array_refcount_entry *entry_b = v.get_variable_entry(var_b);
   const unsigned total_elements = glsl_get_aoa_size(var_b->type);

   for (unsigned i = 0; i < total_elements; i++)
      EXPECT_EQ(i == accessed_element, entry_b->is_linearized_index_referenced(i)) <<
         "i = " << i;

   validate_variables_in_hash_table(v.ht, 2, var_a, var_b);
}

TEST_F(array_refcount_test, visit_whole_second_array_indexing)
{
   ir_variable *var_a = new(mem_ctx) ir_variable(&glsl_type_builtin_vec4,
                                                 "a",
                                                 ir_var_auto);
   ir_variable *var_b = new(mem_ctx) ir_variable(array_3_of_array_4_of_array_5_of_vec4,
                                                 "b",
                                                 ir_var_auto);
   ir_variable *var_i = new(mem_ctx) ir_variable(&glsl_type_builtin_int,
                                                 "i",
                                                 ir_var_auto);

   /* a = b[2][i][1] */
   body->emit(assign(var_a,
                     deref_array(
                        deref_array(
                           deref_array(var_b, body->constant(int(2))),
                           var_i),
                        body->constant(int(1)))));

   ir_array_refcount_visitor v;

   visit_list_elements(&v, &instructions);

   ir_array_refcount_entry *const entry_b = v.get_variable_entry(var_b);
   for (unsigned i = 0; i < 3; i++) {
      for (unsigned j = 0; j < 4; j++) {
         for (unsigned k = 0; k < 5; k++) {
            const bool accessed = (i == 2) && (k == 1);
            const unsigned linearized_index = k + (j * 5) + (i * 4 * 5);

            EXPECT_EQ(accessed,
                      entry_b->is_linearized_index_referenced(linearized_index)) <<
               "i = " << i;
         }
      }
   }

   validate_variables_in_hash_table(v.ht, 3, var_a, var_b, var_i);
}

TEST_F(array_refcount_test, visit_array_indexing_an_array)
{
   ir_variable *var_a = new(mem_ctx) ir_variable(&glsl_type_builtin_vec4,
                                                 "a",
                                                 ir_var_auto);
   ir_variable *var_b = new(mem_ctx) ir_variable(array_3_of_array_4_of_array_5_of_vec4,
                                                 "b",
                                                 ir_var_auto);
   ir_variable *var_c = new(mem_ctx) ir_variable(array_3_of_int,
                                                 "c",
                                                 ir_var_auto);
   ir_variable *var_i = new(mem_ctx) ir_variable(&glsl_type_builtin_int,
                                                 "i",
                                                 ir_var_auto);

   /* a = b[2][3][c[i]] */
   body->emit(assign(var_a,
                     deref_array(
                        deref_array(
                           deref_array(var_b, body->constant(int(2))),
                           body->constant(int(3))),
                        deref_array(var_c, var_i))));

   ir_array_refcount_visitor v;

   visit_list_elements(&v, &instructions);

   ir_array_refcount_entry *const entry_b = v.get_variable_entry(var_b);

   for (unsigned i = 0; i < 3; i++) {
      for (unsigned j = 0; j < 4; j++) {
         for (unsigned k = 0; k < 5; k++) {
            const bool accessed = (i == 2) && (j == 3);
            const unsigned linearized_index = k + (j * 5) + (i * 4 * 5);

            EXPECT_EQ(accessed,
                      entry_b->is_linearized_index_referenced(linearized_index)) <<
               "array b[" << i << "][" << j << "][" << k << "], " <<
               "linear index = " << linearized_index;
         }
      }
   }

   ir_array_refcount_entry *const entry_c = v.get_variable_entry(var_c);

   for (int i = 0; i < glsl_array_size(var_c->type); i++) {
      EXPECT_EQ(true, entry_c->is_linearized_index_referenced(i)) <<
         "array c, i = " << i;
   }

   validate_variables_in_hash_table(v.ht, 4, var_a, var_b, var_c, var_i);
}

TEST_F(array_refcount_test, visit_array_indexing_with_itself)
{
   const glsl_type *const array_2_of_array_3_of_int =
      glsl_array_type(array_3_of_int, 2, 0);

   const glsl_type *const array_2_of_array_2_of_array_3_of_int =
      glsl_array_type(array_2_of_array_3_of_int, 2, 0);

   ir_variable *var_a = new(mem_ctx) ir_variable(&glsl_type_builtin_int,
                                                 "a",
                                                 ir_var_auto);
   ir_variable *var_b = new(mem_ctx) ir_variable(array_2_of_array_2_of_array_3_of_int,
                                                 "b",
                                                 ir_var_auto);

   /* Given GLSL code:
    *
    *    int b[2][2][3];
    *    a = b[ b[0][0][0] ][ b[ b[0][1][0] ][ b[1][0][0] ][1] ][2]
    *
    * b[0][0][0], b[0][1][0], and b[1][0][0] are trivially accessed.
    *
    * b[*][*][1] and b[*][*][2] are accessed.
    *
    * Only b[1][1][0] is not accessed.
    */
   operand b000 = deref_array(
      deref_array(
         deref_array(var_b, body->constant(int(0))),
         body->constant(int(0))),
      body->constant(int(0)));

   operand b010 = deref_array(
      deref_array(
         deref_array(var_b, body->constant(int(0))),
         body->constant(int(1))),
      body->constant(int(0)));

   operand b100 = deref_array(
      deref_array(
         deref_array(var_b, body->constant(int(1))),
         body->constant(int(0))),
      body->constant(int(0)));

   operand b_b010_b100_1 = deref_array(
      deref_array(
         deref_array(var_b, b010),
         b100),
      body->constant(int(1)));

   body->emit(assign(var_a,
                     deref_array(
                        deref_array(
                           deref_array(var_b, b000),
                           b_b010_b100_1),
                        body->constant(int(2)))));

   ir_array_refcount_visitor v;

   visit_list_elements(&v, &instructions);

   ir_array_refcount_entry *const entry_b = v.get_variable_entry(var_b);

   for (unsigned i = 0; i < 2; i++) {
      for (unsigned j = 0; j < 2; j++) {
         for (unsigned k = 0; k < 3; k++) {
            const bool accessed = !(i == 1 && j == 1 && k == 0);
            const unsigned linearized_index = k + (j * 3) + (i * 2 * 3);

            EXPECT_EQ(accessed,
                      entry_b->is_linearized_index_referenced(linearized_index)) <<
               "array b[" << i << "][" << j << "][" << k << "], " <<
               "linear index = " << linearized_index;
         }
      }
   }

   validate_variables_in_hash_table(v.ht, 2, var_a, var_b);
}
