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

#undef NDEBUG

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "intel_decoder.h"

static bool quiet = false;

struct test_address {
   uint64_t offset;
};

__attribute__((unused)) static uint64_t
_test_combine_address(void *data, void *location,
                      struct test_address address, uint32_t delta)
{
   return address.offset + delta;
}

#define __gen_user_data void
#define __gen_combine_address _test_combine_address
#define __gen_address_type struct test_address

#include "gentest_pack.h"

static void
test_struct(struct intel_spec *spec) {
   /* Fill struct fields and <group> tag */
   struct GFX9_TEST_STRUCT test1 = {
      .number1 = 5,
      .number2 = 1234,
   };

   for (int i = 0; i < 4; i++) {
      test1.byte[i] = i * 10 + 5;
   }

   /* Pack struct into a dw array */
   uint32_t dw[GFX9_TEST_STRUCT_length];
   GFX9_TEST_STRUCT_pack(NULL, dw, &test1);

   /* Now decode the packed struct, and make sure it matches the original */
   struct intel_group *group;
   group = intel_spec_find_struct(spec, "TEST_STRUCT");

   assert(group != NULL);

   if (!quiet) {
      printf("\nTEST_STRUCT:\n");
      intel_print_group(stdout, group, 0, dw, 0, false);
   }

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, group, dw, 0, false);

   while (intel_field_iterator_next(&iter)) {
      int idx;
      if (strcmp(iter.name, "number1") == 0) {
         uint16_t number = iter.raw_value;
         assert(number == test1.number1);
      } else if (strcmp(iter.name, "number2") == 0) {
         uint16_t number = iter.raw_value;
         assert(number == test1.number2);
      } else if (sscanf(iter.name, "byte[%d]", &idx) == 1) {
         uint8_t number = iter.raw_value;
         assert(number == test1.byte[idx]);
      }
   }
}

static void
test_two_levels(struct intel_spec *spec) {
   struct GFX9_STRUCT_TWO_LEVELS test;

   for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 8; j++) {
         test.byte[i][j] = (i * 10 + j) % 256;
      }
   }

   uint32_t dw[GFX9_STRUCT_TWO_LEVELS_length];
   GFX9_STRUCT_TWO_LEVELS_pack(NULL, dw, &test);

   struct intel_group *group;
   group = intel_spec_find_struct(spec, "STRUCT_TWO_LEVELS");

   assert(group != NULL);

   if (!quiet) {
      printf("\nSTRUCT_TWO_LEVELS\n");
      intel_print_group(stdout, group, 0, dw, 0, false);
   }

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, group, dw, 0, false);

   while (intel_field_iterator_next(&iter)) {
      int i, j;

      assert(sscanf(iter.name, "byte[%d][%d]", &i, &j) == 2);
      uint8_t number = iter.raw_value;
      assert(number == test.byte[i][j]);
   }
}

int main(int argc, char **argv)
{
   struct intel_spec *spec = intel_spec_load_filename(GENXML_DIR, GENXML_FILE);

   if (argc > 1 && strcmp(argv[1], "-quiet") == 0)
      quiet = true;

   test_struct(spec);
   test_two_levels(spec);

   intel_spec_destroy(spec);

   return 0;
}
