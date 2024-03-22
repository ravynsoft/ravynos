/*
 * Copyright © 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#ifndef NIR_TESTS_NIR_TEST_H
#define NIR_TESTS_NIR_TEST_H

#include <gtest/gtest.h>

#include "nir.h"
#include "nir_builder.h"

class nir_test : public ::testing::Test {
 protected:
   nir_test(const char *name)
   {
      glsl_type_singleton_init_or_ref();

      _b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, &options, "%s", name);
      b = &_b;
   }

   virtual ~nir_test()
   {
      if (HasFailure()) {
         printf("\nShader from the failed test:\n\n");
         nir_print_shader(b->shader, stdout);
      }

      ralloc_free(b->shader);

      glsl_type_singleton_decref();
   }

   nir_shader_compiler_options options = {};
   nir_builder _b;
   nir_builder *b;
};

#endif
