/*
 * Copyright © 2013 Intel Corporation
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
#include "standalone_scaffolding.h"
#include "util/compiler.h"
#include "main/mtypes.h"
#include "main/macros.h"
#include "ir.h"
#include "glsl_parser_extras.h"
#include "glsl_symbol_table.h"

class common_builtin : public ::testing::Test {
public:
   common_builtin(GLenum shader_type)
      : shader_type(shader_type)
   {
      /* empty */
   }

   virtual void SetUp();
   virtual void TearDown();

   void string_starts_with_prefix(const char *str, const char *prefix);
   void names_start_with_gl();
   void uniforms_and_system_values_dont_have_explicit_location();
   void constants_are_constant();
   void no_invalid_variable_modes();

   GLenum shader_type;
   struct _mesa_glsl_parse_state *state;
   struct gl_shader *shader;
   void *mem_ctx;
   gl_context ctx;
   exec_list ir;
};

void
common_builtin::SetUp()
{
   glsl_type_singleton_init_or_ref();

   this->mem_ctx = ralloc_context(NULL);
   this->ir.make_empty();

   initialize_context_to_defaults(&this->ctx, API_OPENGL_COMPAT);

   this->shader = rzalloc(this->mem_ctx, gl_shader);
   this->shader->Type = this->shader_type;
   this->shader->Stage = _mesa_shader_enum_to_shader_stage(this->shader_type);

   this->state =
      new(mem_ctx) _mesa_glsl_parse_state(&this->ctx, this->shader->Stage,
                                          this->shader);

   _mesa_glsl_initialize_types(this->state);
   _mesa_glsl_initialize_variables(&this->ir, this->state);
}

void
common_builtin::TearDown()
{
   ralloc_free(this->mem_ctx);
   this->mem_ctx = NULL;

   glsl_type_singleton_decref();
}

void
common_builtin::string_starts_with_prefix(const char *str, const char *prefix)
{
   const size_t len = strlen(prefix);
   char *const name_prefix = new char[len + 1];

   strncpy(name_prefix, str, len);
   name_prefix[len] = '\0';
   EXPECT_STREQ(prefix, name_prefix) << "Bad name " << str;

   delete [] name_prefix;
}

void
common_builtin::names_start_with_gl()
{
   foreach_in_list(ir_instruction, node, &this->ir) {
      ir_variable *const var = node->as_variable();

      string_starts_with_prefix(var->name, "gl_");
   }
}

void
common_builtin::uniforms_and_system_values_dont_have_explicit_location()
{
   foreach_in_list(ir_instruction, node, &this->ir) {
      ir_variable *const var = node->as_variable();

      if (var->data.mode != ir_var_uniform && var->data.mode != ir_var_system_value)
         continue;

      EXPECT_FALSE(var->data.explicit_location);
      EXPECT_EQ(-1, var->data.location);
   }
}

void
common_builtin::constants_are_constant()
{
   foreach_in_list(ir_instruction, node, &this->ir) {
      ir_variable *const var = node->as_variable();

      if (var->data.mode != ir_var_auto)
         continue;

      EXPECT_FALSE(var->data.explicit_location);
      EXPECT_EQ(-1, var->data.location);
      EXPECT_TRUE(var->data.read_only);
   }
}

void
common_builtin::no_invalid_variable_modes()
{
   foreach_in_list(ir_instruction, node, &this->ir) {
      ir_variable *const var = node->as_variable();

      switch (var->data.mode) {
      case ir_var_auto:
      case ir_var_uniform:
      case ir_var_shader_in:
      case ir_var_shader_out:
      case ir_var_system_value:
         break;

      default:
         ADD_FAILURE() << "Built-in variable " << var->name
                       << " has an invalid mode " << int(var->data.mode);
         break;
      }
   }
}

/************************************************************/

class vertex_builtin : public common_builtin {
public:
   vertex_builtin()
      : common_builtin(GL_VERTEX_SHADER)
   {
      /* empty */
   }
};

TEST_F(vertex_builtin, names_start_with_gl)
{
   common_builtin::names_start_with_gl();
}

TEST_F(vertex_builtin, inputs_have_explicit_location)
{
   foreach_in_list(ir_instruction, node, &this->ir) {
      ir_variable *const var = node->as_variable();

      if (var->data.mode != ir_var_shader_in)
         continue;

      EXPECT_TRUE(var->data.explicit_location);
      EXPECT_NE(-1, var->data.location);
      EXPECT_GT(VERT_ATTRIB_GENERIC0, var->data.location);
      EXPECT_EQ(0u, var->data.location_frac);
   }
}

TEST_F(vertex_builtin, outputs_have_explicit_location)
{
   foreach_in_list(ir_instruction, node, &this->ir) {
      ir_variable *const var = node->as_variable();

      if (var->data.mode != ir_var_shader_out)
         continue;

      EXPECT_TRUE(var->data.explicit_location);
      EXPECT_NE(-1, var->data.location);
      EXPECT_GT(VARYING_SLOT_VAR0, var->data.location);
      EXPECT_EQ(0u, var->data.location_frac);

      /* Several varyings only exist in the fragment shader.  Be sure that no
       * outputs with these locations exist.
       */
      EXPECT_NE(VARYING_SLOT_PNTC, var->data.location);
      EXPECT_NE(VARYING_SLOT_FACE, var->data.location);
      EXPECT_NE(VARYING_SLOT_PRIMITIVE_ID, var->data.location);
   }
}

TEST_F(vertex_builtin, uniforms_and_system_values_dont_have_explicit_location)
{
   common_builtin::uniforms_and_system_values_dont_have_explicit_location();
}

TEST_F(vertex_builtin, constants_are_constant)
{
   common_builtin::constants_are_constant();
}

TEST_F(vertex_builtin, no_invalid_variable_modes)
{
   common_builtin::no_invalid_variable_modes();
}

/********************************************************************/

class fragment_builtin : public common_builtin {
public:
   fragment_builtin()
      : common_builtin(GL_FRAGMENT_SHADER)
   {
      /* empty */
   }
};

TEST_F(fragment_builtin, names_start_with_gl)
{
   common_builtin::names_start_with_gl();
}

TEST_F(fragment_builtin, inputs_have_explicit_location)
{
   foreach_in_list(ir_instruction, node, &this->ir) {
      ir_variable *const var = node->as_variable();

      if (var->data.mode != ir_var_shader_in)
	 continue;

      EXPECT_TRUE(var->data.explicit_location);
      EXPECT_NE(-1, var->data.location);
      EXPECT_GT(VARYING_SLOT_VAR0, var->data.location);
      EXPECT_EQ(0u, var->data.location_frac);

      /* Several varyings only exist in the vertex / geometry shader.  Be sure
       * that no inputs with these locations exist.
       */
      EXPECT_TRUE(_mesa_varying_slot_in_fs((gl_varying_slot) var->data.location));
   }
}

TEST_F(fragment_builtin, outputs_have_explicit_location)
{
   foreach_in_list(ir_instruction, node, &this->ir) {
      ir_variable *const var = node->as_variable();

      if (var->data.mode != ir_var_shader_out)
	 continue;

      EXPECT_TRUE(var->data.explicit_location);
      EXPECT_NE(-1, var->data.location);

      /* gl_FragData[] has location FRAG_RESULT_DATA0.  Locations beyond that
       * are invalid.
       */
      EXPECT_GE(FRAG_RESULT_DATA0, var->data.location);

      EXPECT_EQ(0u, var->data.location_frac);
   }
}

TEST_F(fragment_builtin, uniforms_and_system_values_dont_have_explicit_location)
{
   common_builtin::uniforms_and_system_values_dont_have_explicit_location();
}

TEST_F(fragment_builtin, constants_are_constant)
{
   common_builtin::constants_are_constant();
}

TEST_F(fragment_builtin, no_invalid_variable_modes)
{
   common_builtin::no_invalid_variable_modes();
}

/********************************************************************/

class geometry_builtin : public common_builtin {
public:
   geometry_builtin()
      : common_builtin(GL_GEOMETRY_SHADER)
   {
      /* empty */
   }
};

TEST_F(geometry_builtin, names_start_with_gl)
{
   common_builtin::names_start_with_gl();
}

TEST_F(geometry_builtin, inputs_have_explicit_location)
{
   foreach_in_list(ir_instruction, node, &this->ir) {
      ir_variable *const var = node->as_variable();

      if (var->data.mode != ir_var_shader_in)
	 continue;

      if (var->is_interface_instance()) {
         EXPECT_STREQ("gl_in", var->name);
         EXPECT_FALSE(var->data.explicit_location);
         EXPECT_EQ(-1, var->data.location);

         ASSERT_TRUE(glsl_type_is_array(var->type));

         const glsl_type *const instance_type = var->type->fields.array;

         for (unsigned i = 0; i < instance_type->length; i++) {
            const glsl_struct_field *const input =
               &instance_type->fields.structure[i];

            string_starts_with_prefix(input->name, "gl_");
            EXPECT_NE(-1, input->location);
            EXPECT_GT(VARYING_SLOT_VAR0, input->location);

            /* Several varyings only exist in the fragment shader.  Be sure
             * that no inputs with these locations exist.
             */
            EXPECT_NE(VARYING_SLOT_PNTC, input->location);
            EXPECT_NE(VARYING_SLOT_FACE, input->location);
         }
      } else {
         EXPECT_TRUE(var->data.explicit_location);
         EXPECT_NE(-1, var->data.location);
         EXPECT_GT(VARYING_SLOT_VAR0, var->data.location);
         EXPECT_EQ(0u, var->data.location_frac);
      }

      /* Several varyings only exist in the fragment shader.  Be sure that no
       * inputs with these locations exist.
       */
      EXPECT_NE(VARYING_SLOT_PNTC, var->data.location);
      EXPECT_NE(VARYING_SLOT_FACE, var->data.location);
   }
}

TEST_F(geometry_builtin, outputs_have_explicit_location)
{
   foreach_in_list(ir_instruction, node, &this->ir) {
      ir_variable *const var = node->as_variable();

      if (var->data.mode != ir_var_shader_out)
	 continue;

      EXPECT_TRUE(var->data.explicit_location);
      EXPECT_NE(-1, var->data.location);
      EXPECT_GT(VARYING_SLOT_VAR0, var->data.location);
      EXPECT_EQ(0u, var->data.location_frac);

      /* Several varyings only exist in the fragment shader.  Be sure that no
       * outputs with these locations exist.
       */
      EXPECT_NE(VARYING_SLOT_PNTC, var->data.location);
      EXPECT_NE(VARYING_SLOT_FACE, var->data.location);
   }
}

TEST_F(geometry_builtin, uniforms_and_system_values_dont_have_explicit_location)
{
   common_builtin::uniforms_and_system_values_dont_have_explicit_location();
}

TEST_F(geometry_builtin, constants_are_constant)
{
   common_builtin::constants_are_constant();
}

TEST_F(geometry_builtin, no_invalid_variable_modes)
{
   common_builtin::no_invalid_variable_modes();
}
