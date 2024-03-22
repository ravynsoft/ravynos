/*
 * Copyright © 2011 Intel Corporation
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

/**
 * \file shader_query.cpp
 * C-to-C++ bridge functions to query GLSL shader data
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "main/context.h"
#include "main/enums.h"
#include "main/shaderapi.h"
#include "main/shaderobj.h"
#include "main/uniforms.h"
#include "compiler/glsl/glsl_symbol_table.h"
#include "compiler/glsl/ir.h"
#include "compiler/glsl/linker_util.h"
#include "compiler/glsl/string_to_uint_map.h"
#include "util/mesa-sha1.h"
#include "c99_alloca.h"
#include "api_exec_decl.h"

static GLint
program_resource_location(struct gl_program_resource *res,
                          unsigned array_index);

/**
 * Declare convenience functions to return resource data in a given type.
 * Warning! this is not type safe so be *very* careful when using these.
 */
#define DECL_RESOURCE_FUNC(name, type) \
const type * RESOURCE_ ## name (gl_program_resource *res) { \
   assert(res->Data); \
   return (type *) res->Data; \
}

DECL_RESOURCE_FUNC(VAR, gl_shader_variable);
DECL_RESOURCE_FUNC(UBO, gl_uniform_block);
DECL_RESOURCE_FUNC(UNI, gl_uniform_storage);
DECL_RESOURCE_FUNC(ATC, gl_active_atomic_buffer);
DECL_RESOURCE_FUNC(XFV, gl_transform_feedback_varying_info);
DECL_RESOURCE_FUNC(XFB, gl_transform_feedback_buffer);
DECL_RESOURCE_FUNC(SUB, gl_subroutine_function);

static GLenum
mediump_to_highp_type(GLenum type)
{
   switch (type) {
   case GL_FLOAT16_NV:
      return GL_FLOAT;
   case GL_FLOAT16_VEC2_NV:
      return GL_FLOAT_VEC2;
   case GL_FLOAT16_VEC3_NV:
      return GL_FLOAT_VEC3;
   case GL_FLOAT16_VEC4_NV:
      return GL_FLOAT_VEC4;
   case GL_FLOAT16_MAT2_AMD:
      return GL_FLOAT_MAT2;
   case GL_FLOAT16_MAT3_AMD:
      return GL_FLOAT_MAT3;
   case GL_FLOAT16_MAT4_AMD:
      return GL_FLOAT_MAT4;
   case GL_FLOAT16_MAT2x3_AMD:
      return GL_FLOAT_MAT2x3;
   case GL_FLOAT16_MAT2x4_AMD:
      return GL_FLOAT_MAT2x4;
   case GL_FLOAT16_MAT3x2_AMD:
      return GL_FLOAT_MAT3x2;
   case GL_FLOAT16_MAT3x4_AMD:
      return GL_FLOAT_MAT3x4;
   case GL_FLOAT16_MAT4x2_AMD:
      return GL_FLOAT_MAT4x2;
   case GL_FLOAT16_MAT4x3_AMD:
      return GL_FLOAT_MAT4x3;
   default:
      return type;
   }
}

static void
bind_attrib_location(struct gl_context *ctx,
                     struct gl_shader_program *const shProg, GLuint index,
                     const GLchar *name, bool no_error)
{
   if (!name)
      return;

   if (!no_error) {
      if (strncmp(name, "gl_", 3) == 0) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glBindAttribLocation(illegal name)");
         return;
      }

      if (index >= ctx->Const.Program[MESA_SHADER_VERTEX].MaxAttribs) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glBindAttribLocation(%u >= %u)",
                     index, ctx->Const.Program[MESA_SHADER_VERTEX].MaxAttribs);
         return;
      }
   }

   /* Replace the current value if it's already in the list.  Add
    * VERT_ATTRIB_GENERIC0 because that's how the linker differentiates
    * between built-in attributes and user-defined attributes.
    */
   shProg->AttributeBindings->put(index + VERT_ATTRIB_GENERIC0, name);

   /*
    * Note that this attribute binding won't go into effect until
    * glLinkProgram is called again.
    */
}

void GLAPIENTRY
_mesa_BindAttribLocation_no_error(GLuint program, GLuint index,
                                  const GLchar *name)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_shader_program *const shProg =
      _mesa_lookup_shader_program(ctx, program);
   bind_attrib_location(ctx, shProg, index, name, true);
}

void GLAPIENTRY
_mesa_BindAttribLocation(GLuint program, GLuint index,
                         const GLchar *name)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_shader_program *const shProg =
      _mesa_lookup_shader_program_err(ctx, program, "glBindAttribLocation");
   if (!shProg)
      return;

   bind_attrib_location(ctx, shProg, index, name, false);
}

void GLAPIENTRY
_mesa_GetActiveAttrib(GLuint program, GLuint desired_index,
                      GLsizei maxLength, GLsizei * length, GLint * size,
                      GLenum * type, GLchar * name)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;

   if (maxLength < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGetActiveAttrib(maxLength < 0)");
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program, "glGetActiveAttrib");
   if (!shProg)
      return;

   if (!shProg->data->LinkStatus) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetActiveAttrib(program not linked)");
      return;
   }

   if (shProg->_LinkedShaders[MESA_SHADER_VERTEX] == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGetActiveAttrib(no vertex shader)");
      return;
   }

   struct gl_program_resource *res =
      _mesa_program_resource_find_index(shProg, GL_PROGRAM_INPUT,
                                        desired_index);

   /* User asked for index that does not exist. */
   if (!res) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGetActiveAttrib(index)");
      return;
   }

   const gl_shader_variable *const var = RESOURCE_VAR(res);

   const char *var_name = var->name.string;

   _mesa_copy_string(name, maxLength, length, var_name);

   if (size)
      _mesa_program_resource_prop(shProg, res, desired_index, GL_ARRAY_SIZE,
                                  size, false, "glGetActiveAttrib");

   if (type)
      _mesa_program_resource_prop(shProg, res, desired_index, GL_TYPE,
                                  (GLint *) type, false, "glGetActiveAttrib");
}

GLint GLAPIENTRY
_mesa_GetAttribLocation(GLuint program, const GLchar * name)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *const shProg =
      _mesa_lookup_shader_program_err(ctx, program, "glGetAttribLocation");

   if (!shProg) {
      return -1;
   }

   if (!shProg->data->LinkStatus) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetAttribLocation(program not linked)");
      return -1;
   }

   if (!name)
      return -1;

   /* Not having a vertex shader is not an error.
    */
   if (shProg->_LinkedShaders[MESA_SHADER_VERTEX] == NULL)
      return -1;

   unsigned array_index = 0;
   struct gl_program_resource *res =
      _mesa_program_resource_find_name(shProg, GL_PROGRAM_INPUT, name,
                                       &array_index);

   if (!res)
      return -1;

   return program_resource_location(res, array_index);
}

unsigned
_mesa_count_active_attribs(struct gl_shader_program *shProg)
{
   if (!shProg->data->LinkStatus
       || shProg->_LinkedShaders[MESA_SHADER_VERTEX] == NULL) {
      return 0;
   }

   struct gl_program_resource *res = shProg->data->ProgramResourceList;
   unsigned count = 0;
   for (unsigned j = 0; j < shProg->data->NumProgramResourceList;
        j++, res++) {
      if (res->Type == GL_PROGRAM_INPUT &&
          res->StageReferences & (1 << MESA_SHADER_VERTEX))
         count++;
   }
   return count;
}


size_t
_mesa_longest_attribute_name_length(struct gl_shader_program *shProg)
{
   if (!shProg->data->LinkStatus
       || shProg->_LinkedShaders[MESA_SHADER_VERTEX] == NULL) {
      return 0;
   }

   struct gl_program_resource *res = shProg->data->ProgramResourceList;
   size_t longest = 0;
   for (unsigned j = 0; j < shProg->data->NumProgramResourceList;
        j++, res++) {
      if (res->Type == GL_PROGRAM_INPUT &&
          res->StageReferences & (1 << MESA_SHADER_VERTEX)) {

         /* From the ARB_gl_spirv spec:
          *
          *   "If pname is ACTIVE_ATTRIBUTE_MAX_LENGTH, the length of the
          *    longest active attribute name, including a null terminator, is
          *    returned.  If no active attributes exist, zero is returned. If
          *    no name reflection information is available, one is returned."
          */
         const size_t length = RESOURCE_VAR(res)->name.length;

         if (length >= longest)
            longest = length + 1;
      }
   }

   return longest;
}

void static
bind_frag_data_location(struct gl_shader_program *const shProg,
                        const char *name, unsigned colorNumber,
                        unsigned index)
{
   /* Replace the current value if it's already in the list.  Add
    * FRAG_RESULT_DATA0 because that's how the linker differentiates
    * between built-in attributes and user-defined attributes.
    */
   shProg->FragDataBindings->put(colorNumber + FRAG_RESULT_DATA0, name);
   shProg->FragDataIndexBindings->put(index, name);

   /*
    * Note that this binding won't go into effect until
    * glLinkProgram is called again.
    */
}

void GLAPIENTRY
_mesa_BindFragDataLocation(GLuint program, GLuint colorNumber,
			   const GLchar *name)
{
   _mesa_BindFragDataLocationIndexed(program, colorNumber, 0, name);
}

void GLAPIENTRY
_mesa_BindFragDataLocation_no_error(GLuint program, GLuint colorNumber,
                                    const GLchar *name)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!name)
      return;

   struct gl_shader_program *const shProg =
      _mesa_lookup_shader_program(ctx, program);

   bind_frag_data_location(shProg, name, colorNumber, 0);
}

void GLAPIENTRY
_mesa_BindFragDataLocationIndexed(GLuint program, GLuint colorNumber,
                                  GLuint index, const GLchar *name)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_shader_program *const shProg =
      _mesa_lookup_shader_program_err(ctx, program, "glBindFragDataLocationIndexed");
   if (!shProg)
      return;

   if (!name)
      return;

   if (strncmp(name, "gl_", 3) == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glBindFragDataLocationIndexed(illegal name)");
      return;
   }

   if (index > 1) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBindFragDataLocationIndexed(index)");
      return;
   }

   if (index == 0 && colorNumber >= ctx->Const.MaxDrawBuffers) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBindFragDataLocationIndexed(colorNumber)");
      return;
   }

   if (index == 1 && colorNumber >= ctx->Const.MaxDualSourceDrawBuffers) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBindFragDataLocationIndexed(colorNumber)");
      return;
   }

   bind_frag_data_location(shProg, name, colorNumber, index);
}

void GLAPIENTRY
_mesa_BindFragDataLocationIndexed_no_error(GLuint program, GLuint colorNumber,
                                           GLuint index, const GLchar *name)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!name)
      return;

   struct gl_shader_program *const shProg =
      _mesa_lookup_shader_program(ctx, program);

   bind_frag_data_location(shProg, name, colorNumber, index);
}

GLint GLAPIENTRY
_mesa_GetFragDataIndex(GLuint program, const GLchar *name)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *const shProg =
      _mesa_lookup_shader_program_err(ctx, program, "glGetFragDataIndex");

   if (!shProg) {
      return -1;
   }

   if (!shProg->data->LinkStatus) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetFragDataIndex(program not linked)");
      return -1;
   }

   if (!name)
      return -1;

   /* Not having a fragment shader is not an error.
    */
   if (shProg->_LinkedShaders[MESA_SHADER_FRAGMENT] == NULL)
      return -1;

   return _mesa_program_resource_location_index(shProg, GL_PROGRAM_OUTPUT,
                                                name);
}

GLint GLAPIENTRY
_mesa_GetFragDataLocation(GLuint program, const GLchar *name)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *const shProg =
      _mesa_lookup_shader_program_err(ctx, program, "glGetFragDataLocation");

   if (!shProg) {
      return -1;
   }

   if (!shProg->data->LinkStatus) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetFragDataLocation(program not linked)");
      return -1;
   }

   if (!name)
      return -1;

   /* Not having a fragment shader is not an error.
    */
   if (shProg->_LinkedShaders[MESA_SHADER_FRAGMENT] == NULL)
      return -1;

   unsigned array_index = 0;
   struct gl_program_resource *res =
      _mesa_program_resource_find_name(shProg, GL_PROGRAM_OUTPUT, name,
                                       &array_index);

   if (!res)
      return -1;

   return program_resource_location(res, array_index);
}

const char*
_mesa_program_resource_name(struct gl_program_resource *res)
{
   switch (res->Type) {
   case GL_UNIFORM_BLOCK:
   case GL_SHADER_STORAGE_BLOCK:
      return RESOURCE_UBO(res)->name.string;
   case GL_TRANSFORM_FEEDBACK_VARYING:
      return RESOURCE_XFV(res)->name.string;
   case GL_PROGRAM_INPUT:
   case GL_PROGRAM_OUTPUT:
      return RESOURCE_VAR(res)->name.string;
   case GL_UNIFORM:
   case GL_BUFFER_VARIABLE:
      return RESOURCE_UNI(res)->name.string;
   case GL_VERTEX_SUBROUTINE_UNIFORM:
   case GL_GEOMETRY_SUBROUTINE_UNIFORM:
   case GL_FRAGMENT_SUBROUTINE_UNIFORM:
   case GL_COMPUTE_SUBROUTINE_UNIFORM:
   case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
   case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM:
      return RESOURCE_UNI(res)->name.string + MESA_SUBROUTINE_PREFIX_LEN;
   case GL_VERTEX_SUBROUTINE:
   case GL_GEOMETRY_SUBROUTINE:
   case GL_FRAGMENT_SUBROUTINE:
   case GL_COMPUTE_SUBROUTINE:
   case GL_TESS_CONTROL_SUBROUTINE:
   case GL_TESS_EVALUATION_SUBROUTINE:
      return RESOURCE_SUB(res)->name.string;
   default:
      break;
   }
   return NULL;
}

int
_mesa_program_resource_name_length(struct gl_program_resource *res)
{
   switch (res->Type) {
   case GL_UNIFORM_BLOCK:
   case GL_SHADER_STORAGE_BLOCK:
      return RESOURCE_UBO(res)->name.length;
   case GL_TRANSFORM_FEEDBACK_VARYING:
      return RESOURCE_XFV(res)->name.length;
   case GL_PROGRAM_INPUT:
   case GL_PROGRAM_OUTPUT:
      return RESOURCE_VAR(res)->name.length;
   case GL_UNIFORM:
   case GL_BUFFER_VARIABLE:
      return RESOURCE_UNI(res)->name.length;
   case GL_VERTEX_SUBROUTINE_UNIFORM:
   case GL_GEOMETRY_SUBROUTINE_UNIFORM:
   case GL_FRAGMENT_SUBROUTINE_UNIFORM:
   case GL_COMPUTE_SUBROUTINE_UNIFORM:
   case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
   case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM:
      return RESOURCE_UNI(res)->name.length - MESA_SUBROUTINE_PREFIX_LEN;
   case GL_VERTEX_SUBROUTINE:
   case GL_GEOMETRY_SUBROUTINE:
   case GL_FRAGMENT_SUBROUTINE:
   case GL_COMPUTE_SUBROUTINE:
   case GL_TESS_CONTROL_SUBROUTINE:
   case GL_TESS_EVALUATION_SUBROUTINE:
      return RESOURCE_SUB(res)->name.length;
   default:
      break;
   }
   return 0;
}

bool
_mesa_program_get_resource_name(struct gl_program_resource *res,
                                struct gl_resource_name *out)
{
   switch (res->Type) {
   case GL_UNIFORM_BLOCK:
   case GL_SHADER_STORAGE_BLOCK:
      *out = RESOURCE_UBO(res)->name;
      return out->string != NULL;
   case GL_TRANSFORM_FEEDBACK_VARYING:
      *out = RESOURCE_XFV(res)->name;
      return out->string != NULL;
   case GL_PROGRAM_INPUT:
   case GL_PROGRAM_OUTPUT:
      *out = RESOURCE_VAR(res)->name;
      return out->string != NULL;
   case GL_UNIFORM:
   case GL_BUFFER_VARIABLE:
      *out = RESOURCE_UNI(res)->name;
      return out->string != NULL;
   case GL_VERTEX_SUBROUTINE_UNIFORM:
   case GL_GEOMETRY_SUBROUTINE_UNIFORM:
   case GL_FRAGMENT_SUBROUTINE_UNIFORM:
   case GL_COMPUTE_SUBROUTINE_UNIFORM:
   case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
   case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM:
      *out = RESOURCE_UNI(res)->name;
      out->string += MESA_SUBROUTINE_PREFIX_LEN;
      out->length -= MESA_SUBROUTINE_PREFIX_LEN;
      assert(out->string); /* always non-NULL */
      return true;
   case GL_VERTEX_SUBROUTINE:
   case GL_GEOMETRY_SUBROUTINE:
   case GL_FRAGMENT_SUBROUTINE:
   case GL_COMPUTE_SUBROUTINE:
   case GL_TESS_CONTROL_SUBROUTINE:
   case GL_TESS_EVALUATION_SUBROUTINE:
      *out = RESOURCE_SUB(res)->name;
      return out->string != NULL;
   default:
      return false;
   }
}

unsigned
_mesa_program_resource_array_size(struct gl_program_resource *res)
{
   switch (res->Type) {
   case GL_TRANSFORM_FEEDBACK_VARYING:
      return RESOURCE_XFV(res)->Size > 1 ?
             RESOURCE_XFV(res)->Size : 0;
   case GL_PROGRAM_INPUT:
   case GL_PROGRAM_OUTPUT:
      return RESOURCE_VAR(res)->type->length;
   case GL_UNIFORM:
   case GL_VERTEX_SUBROUTINE_UNIFORM:
   case GL_GEOMETRY_SUBROUTINE_UNIFORM:
   case GL_FRAGMENT_SUBROUTINE_UNIFORM:
   case GL_COMPUTE_SUBROUTINE_UNIFORM:
   case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
   case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM:
      return RESOURCE_UNI(res)->array_elements;
   case GL_BUFFER_VARIABLE:
      /* Unsized arrays */
      if (RESOURCE_UNI(res)->array_stride > 0 &&
          RESOURCE_UNI(res)->array_elements == 0)
         return 1;
      else
         return RESOURCE_UNI(res)->array_elements;
   case GL_VERTEX_SUBROUTINE:
   case GL_GEOMETRY_SUBROUTINE:
   case GL_FRAGMENT_SUBROUTINE:
   case GL_COMPUTE_SUBROUTINE:
   case GL_TESS_CONTROL_SUBROUTINE:
   case GL_TESS_EVALUATION_SUBROUTINE:
   case GL_ATOMIC_COUNTER_BUFFER:
   case GL_UNIFORM_BLOCK:
   case GL_SHADER_STORAGE_BLOCK:
      return 0;
   default:
      assert(!"support for resource type not implemented");
   }
   return 0;
}

/**
 * Checks if array subscript is valid and if so sets array_index.
 */
static bool
valid_array_index(const GLchar *name, int len, unsigned *array_index)
{
   long idx = 0;
   const GLchar *out_base_name_end;

   idx = link_util_parse_program_resource_name(name, len, &out_base_name_end);
   if (idx < 0)
      return false;

   if (array_index)
      *array_index = idx;

   return true;
}

static struct gl_program_resource *
search_resource_hash(struct gl_shader_program *shProg,
                     GLenum programInterface, const char *name, int len,
                     unsigned *array_index)
{
   unsigned type = GET_PROGRAM_RESOURCE_TYPE_FROM_GLENUM(programInterface);
   assert(type < ARRAY_SIZE(shProg->data->ProgramResourceHash));

   if (!shProg->data->ProgramResourceHash[type])
      return NULL;

   const char *base_name_end;
   long index = link_util_parse_program_resource_name(name, len, &base_name_end);
   char *name_copy;

   /* If dealing with array, we need to get the basename. */
   if (index >= 0) {
      name_copy = (char *) alloca(base_name_end - name + 1);
      memcpy(name_copy, name, base_name_end - name);
      name_copy[base_name_end - name] = '\0';
      len = base_name_end - name;
   } else {
      name_copy = (char*) name;
   }

   uint32_t hash = _mesa_hash_string_with_length(name_copy, len);
   struct hash_entry *entry =
      _mesa_hash_table_search_pre_hashed(shProg->data->ProgramResourceHash[type],
                                         hash, name_copy);
   if (!entry)
      return NULL;

   if (array_index)
      *array_index = index >= 0 ? index : 0;

   return (struct gl_program_resource *)entry->data;
}

/* Find a program resource with specific name in given interface.
 */
struct gl_program_resource *
_mesa_program_resource_find_name(struct gl_shader_program *shProg,
                                 GLenum programInterface, const char *name,
                                 unsigned *array_index)
{
   if (name == NULL)
      return NULL;

   int len = strlen(name);

   /* If we have a name, try the ProgramResourceHash first. */
   struct gl_program_resource *res =
      search_resource_hash(shProg, programInterface, name, len, array_index);

   if (res)
      return res;

   res = shProg->data->ProgramResourceList;
   for (unsigned i = 0; i < shProg->data->NumProgramResourceList; i++, res++) {
      if (res->Type != programInterface)
         continue;

      struct gl_resource_name rname;

      /* Since ARB_gl_spirv lack of name reflections is a possibility */
      if (!_mesa_program_get_resource_name(res, &rname))
         continue;

      bool found = false;

      /* From ARB_program_interface_query spec:
       *
       * "uint GetProgramResourceIndex(uint program, enum programInterface,
       *                               const char *name);
       *  [...]
       *  If <name> exactly matches the name string of one of the active
       *  resources for <programInterface>, the index of the matched resource is
       *  returned. Additionally, if <name> would exactly match the name string
       *  of an active resource if "[0]" were appended to <name>, the index of
       *  the matched resource is returned. [...]"
       *
       * "A string provided to GetProgramResourceLocation or
       * GetProgramResourceLocationIndex is considered to match an active variable
       * if:
       *
       *  * the string exactly matches the name of the active variable;
       *
       *  * if the string identifies the base name of an active array, where the
       *    string would exactly match the name of the variable if the suffix
       *    "[0]" were appended to the string; [...]"
       */
      /* Remove array's index from interface block name comparison only if
       * array's index is zero and the resulting string length is the same
       * than the provided name's length.
       */
      int length_without_array_index =
         rname.last_square_bracket >= 0 ? rname.last_square_bracket : rname.length;
      bool rname_has_array_index_zero = rname.suffix_is_zero_square_bracketed &&
                                        rname.last_square_bracket == len;

      if (len >= rname.length && strncmp(rname.string, name, rname.length) == 0)
         found = true;
      else if (rname_has_array_index_zero &&
               strncmp(rname.string, name, length_without_array_index) == 0)
         found = true;

      if (found) {
         switch (programInterface) {
         case GL_UNIFORM_BLOCK:
         case GL_SHADER_STORAGE_BLOCK:
            /* Basename match, check if array or struct. */
            if (rname_has_array_index_zero ||
                name[rname.length] == '\0' ||
                name[rname.length] == '[' ||
                name[rname.length] == '.') {
               return res;
            }
            break;
         case GL_TRANSFORM_FEEDBACK_VARYING:
         case GL_BUFFER_VARIABLE:
         case GL_UNIFORM:
         case GL_VERTEX_SUBROUTINE_UNIFORM:
         case GL_GEOMETRY_SUBROUTINE_UNIFORM:
         case GL_FRAGMENT_SUBROUTINE_UNIFORM:
         case GL_COMPUTE_SUBROUTINE_UNIFORM:
         case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
         case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM:
         case GL_VERTEX_SUBROUTINE:
         case GL_GEOMETRY_SUBROUTINE:
         case GL_FRAGMENT_SUBROUTINE:
         case GL_COMPUTE_SUBROUTINE:
         case GL_TESS_CONTROL_SUBROUTINE:
         case GL_TESS_EVALUATION_SUBROUTINE:
            if (name[rname.length] == '.') {
               return res;
            }
            FALLTHROUGH;
         case GL_PROGRAM_INPUT:
         case GL_PROGRAM_OUTPUT:
            if (name[rname.length] == '\0') {
               return res;
            } else if (name[rname.length] == '[' &&
                valid_array_index(name, len, array_index)) {
               return res;
            }
            break;
         default:
            assert(!"not implemented for given interface");
         }
      }
   }
   return NULL;
}

/* Find an uniform or buffer variable program resource with an specific offset
 * inside a block with an specific binding.
 *
 * Valid interfaces are GL_BUFFER_VARIABLE and GL_UNIFORM.
 */
static struct gl_program_resource *
program_resource_find_binding_offset(struct gl_shader_program *shProg,
                                     GLenum programInterface,
                                     const GLuint binding,
                                     const GLint offset)
{

   /* First we need to get the BLOCK_INDEX from the BUFFER_BINDING */
   GLenum blockInterface;

   switch (programInterface) {
   case GL_BUFFER_VARIABLE:
      blockInterface = GL_SHADER_STORAGE_BLOCK;
      break;
   case GL_UNIFORM:
      blockInterface = GL_UNIFORM_BLOCK;
      break;
   default:
      assert("Invalid program interface");
      return NULL;
   }

   int block_index = -1;
   int starting_index = -1;
   struct gl_program_resource *res = shProg->data->ProgramResourceList;

   /* Blocks are added to the resource list in the same order that they are
    * added to UniformBlocks/ShaderStorageBlocks. Furthermore, all the blocks
    * of each type (UBO/SSBO) are contiguous, so we can infer block_index from
    * the resource list.
    */
   for (unsigned i = 0; i < shProg->data->NumProgramResourceList; i++, res++) {
      if (res->Type != blockInterface)
         continue;

      /* Store the first index where a resource of the specific interface is. */
      if (starting_index == -1)
         starting_index = i;

      const struct gl_uniform_block *block = RESOURCE_UBO(res);

      if (block->Binding == binding) {
         /* For arrays, or arrays of arrays of blocks, we want the resource
          * for the block with base index. Most properties for members of each
          * block are inherited from the block with the base index, including
          * a uniform being active or not.
          */
         block_index = i - starting_index - block->linearized_array_index;
         break;
      }
   }

   if (block_index == -1)
      return NULL;

   /* We now look for the resource corresponding to the uniform or buffer
    * variable using the BLOCK_INDEX and OFFSET.
    */
   res = shProg->data->ProgramResourceList;
   for (unsigned i = 0; i < shProg->data->NumProgramResourceList; i++, res++) {
      if (res->Type != programInterface)
         continue;

      const struct gl_uniform_storage *uniform = RESOURCE_UNI(res);

      if (uniform->block_index == block_index && uniform->offset == offset) {
         return res;
      }
   }

   return NULL;
}

/* Checks if an uniform or buffer variable is in the active program resource
 * list.
 *
 * It takes into accout that for variables coming from SPIR-V binaries their
 * names could not be available (ARB_gl_spirv). In that case, it will use the
 * the offset and the block binding to locate the resource.
 *
 * Valid interfaces are GL_BUFFER_VARIABLE and GL_UNIFORM.
 */
struct gl_program_resource *
_mesa_program_resource_find_active_variable(struct gl_shader_program *shProg,
                                            GLenum programInterface,
                                            const gl_uniform_block *block,
                                            unsigned index)
{
   struct gl_program_resource *res;
   struct gl_uniform_buffer_variable uni = block->Uniforms[index];

   assert(programInterface == GL_UNIFORM ||
          programInterface == GL_BUFFER_VARIABLE);

   if (uni.IndexName) {
      res = _mesa_program_resource_find_name(shProg, programInterface, uni.IndexName,
                                             NULL);
   } else {
      /* As the resource has no associated name (ARB_gl_spirv),
       * we can use the UBO/SSBO binding and offset to find it.
       */
      res = program_resource_find_binding_offset(shProg, programInterface,
                                                 block->Binding, uni.Offset);
   }

   return res;
}

static GLuint
calc_resource_index(struct gl_shader_program *shProg,
                    struct gl_program_resource *res)
{
   unsigned i;
   GLuint index = 0;
   for (i = 0; i < shProg->data->NumProgramResourceList; i++) {
      if (&shProg->data->ProgramResourceList[i] == res)
         return index;
      if (shProg->data->ProgramResourceList[i].Type == res->Type)
         index++;
   }
   return GL_INVALID_INDEX;
}

/**
 * Calculate index for the given resource.
 */
GLuint
_mesa_program_resource_index(struct gl_shader_program *shProg,
                             struct gl_program_resource *res)
{
   if (!res)
      return GL_INVALID_INDEX;

   switch (res->Type) {
   case GL_ATOMIC_COUNTER_BUFFER:
      return RESOURCE_ATC(res) - shProg->data->AtomicBuffers;
   case GL_VERTEX_SUBROUTINE:
   case GL_GEOMETRY_SUBROUTINE:
   case GL_FRAGMENT_SUBROUTINE:
   case GL_COMPUTE_SUBROUTINE:
   case GL_TESS_CONTROL_SUBROUTINE:
   case GL_TESS_EVALUATION_SUBROUTINE:
      return RESOURCE_SUB(res)->index;
   case GL_UNIFORM_BLOCK:
   case GL_SHADER_STORAGE_BLOCK:
   case GL_TRANSFORM_FEEDBACK_BUFFER:
   case GL_TRANSFORM_FEEDBACK_VARYING:
   default:
      return calc_resource_index(shProg, res);
   }
}

/**
 * Find a program resource that points to given data.
 */
static struct gl_program_resource*
program_resource_find_data(struct gl_shader_program *shProg, void *data)
{
   struct gl_program_resource *res = shProg->data->ProgramResourceList;
   for (unsigned i = 0; i < shProg->data->NumProgramResourceList;
        i++, res++) {
      if (res->Data == data)
         return res;
   }
   return NULL;
}

/* Find a program resource with specific index in given interface.
 */
struct gl_program_resource *
_mesa_program_resource_find_index(struct gl_shader_program *shProg,
                                  GLenum programInterface, GLuint index)
{
   struct gl_program_resource *res = shProg->data->ProgramResourceList;
   int idx = -1;

   for (unsigned i = 0; i < shProg->data->NumProgramResourceList;
        i++, res++) {
      if (res->Type != programInterface)
         continue;

      switch (res->Type) {
      case GL_UNIFORM_BLOCK:
      case GL_ATOMIC_COUNTER_BUFFER:
      case GL_SHADER_STORAGE_BLOCK:
      case GL_TRANSFORM_FEEDBACK_BUFFER:
         if (_mesa_program_resource_index(shProg, res) == index)
            return res;
         break;
      case GL_TRANSFORM_FEEDBACK_VARYING:
      case GL_PROGRAM_INPUT:
      case GL_PROGRAM_OUTPUT:
      case GL_UNIFORM:
      case GL_VERTEX_SUBROUTINE_UNIFORM:
      case GL_GEOMETRY_SUBROUTINE_UNIFORM:
      case GL_FRAGMENT_SUBROUTINE_UNIFORM:
      case GL_COMPUTE_SUBROUTINE_UNIFORM:
      case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
      case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM:
      case GL_VERTEX_SUBROUTINE:
      case GL_GEOMETRY_SUBROUTINE:
      case GL_FRAGMENT_SUBROUTINE:
      case GL_COMPUTE_SUBROUTINE:
      case GL_TESS_CONTROL_SUBROUTINE:
      case GL_TESS_EVALUATION_SUBROUTINE:
      case GL_BUFFER_VARIABLE:
         if (++idx == (int) index)
            return res;
         break;
      default:
         assert(!"not implemented for given interface");
      }
   }
   return NULL;
}

/* Function returns if resource name is expected to have index
 * appended into it.
 *
 *
 * Page 61 (page 73 of the PDF) in section 2.11 of the OpenGL ES 3.0
 * spec says:
 *
 *     "If the active uniform is an array, the uniform name returned in
 *     name will always be the name of the uniform array appended with
 *     "[0]"."
 *
 * The same text also appears in the OpenGL 4.2 spec.  It does not,
 * however, appear in any previous spec.  Previous specifications are
 * ambiguous in this regard.  However, either name can later be passed
 * to glGetUniformLocation (and related APIs), so there shouldn't be any
 * harm in always appending "[0]" to uniform array names.
 */
static bool
add_index_to_name(struct gl_program_resource *res)
{
   /* Transform feedback varyings have array index already appended
    * in their names.
    */
   return res->Type != GL_TRANSFORM_FEEDBACK_VARYING;
}

/* Get name length of a program resource. This consists of
 * base name + 3 for '[0]' if resource is an array.
 */
extern unsigned
_mesa_program_resource_name_length_array(struct gl_program_resource *res)
{
   int length = _mesa_program_resource_name_length(res);

   /* For shaders constructed from SPIR-V binaries, variables may not
    * have names associated with them.
    */
   if (!length)
      return 0;

   if (_mesa_program_resource_array_size(res) && add_index_to_name(res))
      length += 3;
   return length;
}

/* Get full name of a program resource.
 */
bool
_mesa_get_program_resource_name(struct gl_shader_program *shProg,
                                GLenum programInterface, GLuint index,
                                GLsizei bufSize, GLsizei *length,
                                GLchar *name, bool glthread,
                                const char *caller)
{
   GET_CURRENT_CONTEXT(ctx);

   /* Find resource with given interface and index. */
   struct gl_program_resource *res =
      _mesa_program_resource_find_index(shProg, programInterface, index);

   /* The error INVALID_VALUE is generated if <index> is greater than
   * or equal to the number of entries in the active resource list for
   * <programInterface>.
   */
   if (!res) {
      _mesa_error_glthread_safe(ctx, GL_INVALID_VALUE, glthread,
                                "%s(index %u)", caller, index);
      return false;
   }

   if (bufSize < 0) {
      _mesa_error_glthread_safe(ctx, GL_INVALID_VALUE, glthread,
                                "%s(bufSize %d)", caller, bufSize);
      return false;
   }

   GLsizei localLength;

   if (length == NULL)
      length = &localLength;

   _mesa_copy_string(name, bufSize, length, _mesa_program_resource_name(res));

   /* The resource name can be NULL for shaders constructed from SPIR-V
    * binaries. In that case, we do not add the '[0]'.
    */
   if (name && name[0] != '\0' &&
       _mesa_program_resource_array_size(res) && add_index_to_name(res)) {
      int i;

      /* The comparison is strange because *length does *NOT* include the
       * terminating NUL, but maxLength does.
       */
      for (i = 0; i < 3 && (*length + i + 1) < bufSize; i++)
         name[*length + i] = "[0]"[i];

      name[*length + i] = '\0';
      *length += i;
   }
   return true;
}

static GLint
program_resource_location(struct gl_program_resource *res, unsigned array_index)
{
   switch (res->Type) {
   case GL_PROGRAM_INPUT: {
      const gl_shader_variable *var = RESOURCE_VAR(res);

      if (var->location == -1)
         return -1;

      /* If the input is an array, fail if the index is out of bounds. */
      if (array_index > 0
          && array_index >= var->type->length) {
         return -1;
      }
      return var->location +
	     (array_index * glsl_without_array(var->type)->matrix_columns);
   }
   case GL_PROGRAM_OUTPUT:
      if (RESOURCE_VAR(res)->location == -1)
         return -1;

      /* If the output is an array, fail if the index is out of bounds. */
      if (array_index > 0
          && array_index >= RESOURCE_VAR(res)->type->length) {
         return -1;
      }
      return RESOURCE_VAR(res)->location + array_index;
   case GL_UNIFORM:
      /* If the uniform is built-in, fail. */
      if (RESOURCE_UNI(res)->builtin)
         return -1;

     /* From page 79 of the OpenGL 4.2 spec:
      *
      *     "A valid name cannot be a structure, an array of structures, or any
      *     portion of a single vector or a matrix."
      */
      if (glsl_type_is_struct(glsl_without_array(RESOURCE_UNI(res)->type)))
         return -1;

      /* From the GL_ARB_uniform_buffer_object spec:
       *
       *     "The value -1 will be returned if <name> does not correspond to an
       *     active uniform variable name in <program>, if <name> is associated
       *     with a named uniform block, or if <name> starts with the reserved
       *     prefix "gl_"."
       */
      if (RESOURCE_UNI(res)->block_index != -1 ||
          RESOURCE_UNI(res)->atomic_buffer_index != -1)
         return -1;

      FALLTHROUGH;
   case GL_VERTEX_SUBROUTINE_UNIFORM:
   case GL_GEOMETRY_SUBROUTINE_UNIFORM:
   case GL_FRAGMENT_SUBROUTINE_UNIFORM:
   case GL_COMPUTE_SUBROUTINE_UNIFORM:
   case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
   case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM:
      /* If the uniform is an array, fail if the index is out of bounds. */
      if (array_index > 0
          && array_index >= RESOURCE_UNI(res)->array_elements) {
         return -1;
      }

      /* location in remap table + array element offset */
      return RESOURCE_UNI(res)->remap_location + array_index;
   default:
      return -1;
   }
}

/**
 * Function implements following location queries:
 *    glGetUniformLocation
 */
GLint
_mesa_program_resource_location(struct gl_shader_program *shProg,
                                GLenum programInterface, const char *name)
{
   unsigned array_index = 0;
   struct gl_program_resource *res =
      _mesa_program_resource_find_name(shProg, programInterface, name,
                                       &array_index);

   /* Resource not found. */
   if (!res)
      return -1;

   return program_resource_location(res, array_index);
}

static GLint
_get_resource_location_index(struct gl_program_resource *res)
{
   /* Non-existent variable or resource is not referenced by fragment stage. */
   if (!res || !(res->StageReferences & (1 << MESA_SHADER_FRAGMENT)))
      return -1;

   /* From OpenGL 4.5 spec, 7.3 Program Objects
    * "The value -1 will be returned by either command...
    *  ... or if name identifies an active variable that does not have a
    * valid location assigned.
    */
   if (RESOURCE_VAR(res)->location == -1)
      return -1;
   return RESOURCE_VAR(res)->index;
}

/**
 * Function implements following index queries:
 *    glGetFragDataIndex
 */
GLint
_mesa_program_resource_location_index(struct gl_shader_program *shProg,
                                      GLenum programInterface, const char *name)
{
   struct gl_program_resource *res =
      _mesa_program_resource_find_name(shProg, programInterface, name, NULL);

   return _get_resource_location_index(res);
}

static uint8_t
stage_from_enum(GLenum ref)
{
   switch (ref) {
   case GL_REFERENCED_BY_VERTEX_SHADER:
      return MESA_SHADER_VERTEX;
   case GL_REFERENCED_BY_TESS_CONTROL_SHADER:
      return MESA_SHADER_TESS_CTRL;
   case GL_REFERENCED_BY_TESS_EVALUATION_SHADER:
      return MESA_SHADER_TESS_EVAL;
   case GL_REFERENCED_BY_GEOMETRY_SHADER:
      return MESA_SHADER_GEOMETRY;
   case GL_REFERENCED_BY_FRAGMENT_SHADER:
      return MESA_SHADER_FRAGMENT;
   case GL_REFERENCED_BY_COMPUTE_SHADER:
      return MESA_SHADER_COMPUTE;
   default:
      assert(!"shader stage not supported");
      return MESA_SHADER_STAGES;
   }
}

/**
 * Check if resource is referenced by given 'referenced by' stage enum.
 * ATC and UBO resources hold stage references of their own.
 */
static bool
is_resource_referenced(struct gl_shader_program *shProg,
                       struct gl_program_resource *res,
                       GLuint index, uint8_t stage)
{
   /* First, check if we even have such a stage active. */
   if (!shProg->_LinkedShaders[stage])
      return false;

   if (res->Type == GL_ATOMIC_COUNTER_BUFFER)
      return RESOURCE_ATC(res)->StageReferences[stage];

   if (res->Type == GL_UNIFORM_BLOCK)
      return shProg->data->UniformBlocks[index].stageref & (1 << stage);

   if (res->Type == GL_SHADER_STORAGE_BLOCK)
      return shProg->data->ShaderStorageBlocks[index].stageref & (1 << stage);

   return res->StageReferences & (1 << stage);
}

static unsigned
get_buffer_property(struct gl_shader_program *shProg,
                    struct gl_program_resource *res, const GLenum prop,
                    GLint *val, bool glthread, const char *caller)
{
   GET_CURRENT_CONTEXT(ctx);
   if (res->Type != GL_UNIFORM_BLOCK &&
       res->Type != GL_ATOMIC_COUNTER_BUFFER &&
       res->Type != GL_SHADER_STORAGE_BLOCK &&
       res->Type != GL_TRANSFORM_FEEDBACK_BUFFER)
      goto invalid_operation;

   if (res->Type == GL_UNIFORM_BLOCK) {
      switch (prop) {
      case GL_BUFFER_BINDING:
         *val = RESOURCE_UBO(res)->Binding;
         return 1;
      case GL_BUFFER_DATA_SIZE:
         *val = RESOURCE_UBO(res)->UniformBufferSize;
         return 1;
      case GL_NUM_ACTIVE_VARIABLES:
         *val = 0;
         for (unsigned i = 0; i < RESOURCE_UBO(res)->NumUniforms; i++) {
            struct gl_program_resource *uni =
               _mesa_program_resource_find_active_variable(
                  shProg,
                  GL_UNIFORM,
                  RESOURCE_UBO(res),
                  i);

            if (!uni)
               continue;
            (*val)++;
         }
         return 1;
      case GL_ACTIVE_VARIABLES: {
         unsigned num_values = 0;
         for (unsigned i = 0; i < RESOURCE_UBO(res)->NumUniforms; i++) {
            struct gl_program_resource *uni =
               _mesa_program_resource_find_active_variable(
                  shProg,
                  GL_UNIFORM,
                  RESOURCE_UBO(res),
                  i);

            if (!uni)
               continue;
            *val++ =
               _mesa_program_resource_index(shProg, uni);
            num_values++;
         }
         return num_values;
      }
      }
   } else if (res->Type == GL_SHADER_STORAGE_BLOCK) {
      switch (prop) {
      case GL_BUFFER_BINDING:
         *val = RESOURCE_UBO(res)->Binding;
         return 1;
      case GL_BUFFER_DATA_SIZE:
         *val = RESOURCE_UBO(res)->UniformBufferSize;
         return 1;
      case GL_NUM_ACTIVE_VARIABLES:
         *val = 0;
         for (unsigned i = 0; i < RESOURCE_UBO(res)->NumUniforms; i++) {
            struct gl_program_resource *uni =
               _mesa_program_resource_find_active_variable(
                  shProg,
                  GL_BUFFER_VARIABLE,
                  RESOURCE_UBO(res),
                  i);

            if (!uni)
               continue;
            (*val)++;
         }
         return 1;
      case GL_ACTIVE_VARIABLES: {
         unsigned num_values = 0;
         for (unsigned i = 0; i < RESOURCE_UBO(res)->NumUniforms; i++) {
            struct gl_program_resource *uni =
               _mesa_program_resource_find_active_variable(
                  shProg,
                  GL_BUFFER_VARIABLE,
                  RESOURCE_UBO(res),
                  i);

            if (!uni)
               continue;
            *val++ =
               _mesa_program_resource_index(shProg, uni);
            num_values++;
         }
         return num_values;
      }
      }
   } else if (res->Type == GL_ATOMIC_COUNTER_BUFFER) {
      switch (prop) {
      case GL_BUFFER_BINDING:
         *val = RESOURCE_ATC(res)->Binding;
         return 1;
      case GL_BUFFER_DATA_SIZE:
         *val = RESOURCE_ATC(res)->MinimumSize;
         return 1;
      case GL_NUM_ACTIVE_VARIABLES:
         *val = RESOURCE_ATC(res)->NumUniforms;
         return 1;
      case GL_ACTIVE_VARIABLES:
         for (unsigned i = 0; i < RESOURCE_ATC(res)->NumUniforms; i++) {
            /* Active atomic buffer contains index to UniformStorage. Find
             * out gl_program_resource via data pointer and then calculate
             * index of that uniform.
             */
            unsigned idx = RESOURCE_ATC(res)->Uniforms[i];
            struct gl_program_resource *uni =
               program_resource_find_data(shProg,
                                          &shProg->data->UniformStorage[idx]);
            assert(uni);
            *val++ = _mesa_program_resource_index(shProg, uni);
         }
         return RESOURCE_ATC(res)->NumUniforms;
      }
   } else if (res->Type == GL_TRANSFORM_FEEDBACK_BUFFER) {
      switch (prop) {
      case GL_BUFFER_BINDING:
         *val = RESOURCE_XFB(res)->Binding;
         return 1;
      case GL_NUM_ACTIVE_VARIABLES:
         *val = RESOURCE_XFB(res)->NumVaryings;
         return 1;
      case GL_ACTIVE_VARIABLES:
         struct gl_transform_feedback_info *linked_xfb =
            shProg->last_vert_prog->sh.LinkedTransformFeedback;
         for (int i = 0; i < linked_xfb->NumVarying; i++) {
            unsigned index = linked_xfb->Varyings[i].BufferIndex;
            struct gl_program_resource *buf_res =
               _mesa_program_resource_find_index(shProg,
                                                 GL_TRANSFORM_FEEDBACK_BUFFER,
                                                 index);
            assert(buf_res);
            if (res == buf_res) {
               *val++ = i;
            }
         }
         return RESOURCE_XFB(res)->NumVaryings;
      }
   }
   assert(!"support for property type not implemented");

invalid_operation:
   _mesa_error_glthread_safe(ctx, GL_INVALID_OPERATION, glthread,
                             "%s(%s prop %s)", caller,
                             _mesa_enum_to_string(res->Type),
                             _mesa_enum_to_string(prop));

   return 0;
}

unsigned
_mesa_program_resource_prop(struct gl_shader_program *shProg,
                            struct gl_program_resource *res, GLuint index,
                            const GLenum prop, GLint *val, bool glthread,
                            const char *caller)
{
   GET_CURRENT_CONTEXT(ctx);

#define VALIDATE_TYPE(type)\
   if (res->Type != type)\
      goto invalid_operation;

#define VALIDATE_TYPE_2(type1, type2)\
   if (res->Type != type1 && res->Type != type2)\
      goto invalid_operation;

   switch(prop) {
   case GL_NAME_LENGTH:
      switch (res->Type) {
      case GL_ATOMIC_COUNTER_BUFFER:
      case GL_TRANSFORM_FEEDBACK_BUFFER:
         goto invalid_operation;
      default:
         /* Resource name length + terminator. */
         *val = _mesa_program_resource_name_length_array(res) + 1;
      }
      return 1;
   case GL_TYPE:
      switch (res->Type) {
      case GL_UNIFORM:
      case GL_BUFFER_VARIABLE:
         *val = RESOURCE_UNI(res)->type->gl_type;
         *val = mediump_to_highp_type(*val);
         return 1;
      case GL_PROGRAM_INPUT:
      case GL_PROGRAM_OUTPUT:
         *val = RESOURCE_VAR(res)->type->gl_type;
         *val = mediump_to_highp_type(*val);
         return 1;
      case GL_TRANSFORM_FEEDBACK_VARYING:
         *val = RESOURCE_XFV(res)->Type;
         *val = mediump_to_highp_type(*val);
         return 1;
      default:
         goto invalid_operation;
      }
   case GL_ARRAY_SIZE:
      switch (res->Type) {
      case GL_UNIFORM:
      case GL_BUFFER_VARIABLE:
      case GL_VERTEX_SUBROUTINE_UNIFORM:
      case GL_GEOMETRY_SUBROUTINE_UNIFORM:
      case GL_FRAGMENT_SUBROUTINE_UNIFORM:
      case GL_COMPUTE_SUBROUTINE_UNIFORM:
      case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
      case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM:

         /* Test if a buffer variable is an array or an unsized array.
          * Unsized arrays return zero as array size.
          */
         if (RESOURCE_UNI(res)->is_shader_storage &&
             RESOURCE_UNI(res)->array_stride > 0)
            *val = RESOURCE_UNI(res)->array_elements;
         else
            *val = MAX2(RESOURCE_UNI(res)->array_elements, 1);
         return 1;
      case GL_PROGRAM_INPUT:
      case GL_PROGRAM_OUTPUT:
         *val = MAX2(_mesa_program_resource_array_size(res), 1);
         return 1;
      case GL_TRANSFORM_FEEDBACK_VARYING:
         *val = RESOURCE_XFV(res)->Size;
         return 1;
      default:
         goto invalid_operation;
      }
   case GL_OFFSET:
      switch (res->Type) {
      case GL_UNIFORM:
      case GL_BUFFER_VARIABLE:
         *val = RESOURCE_UNI(res)->offset;
         return 1;
      case GL_TRANSFORM_FEEDBACK_VARYING:
         *val = RESOURCE_XFV(res)->Offset;
         return 1;
      default:
         goto invalid_operation;
      }
   case GL_BLOCK_INDEX:
      VALIDATE_TYPE_2(GL_UNIFORM, GL_BUFFER_VARIABLE);
      *val = RESOURCE_UNI(res)->block_index;
      return 1;
   case GL_ARRAY_STRIDE:
      VALIDATE_TYPE_2(GL_UNIFORM, GL_BUFFER_VARIABLE);
      *val = RESOURCE_UNI(res)->array_stride;
      return 1;
   case GL_MATRIX_STRIDE:
      VALIDATE_TYPE_2(GL_UNIFORM, GL_BUFFER_VARIABLE);
      *val = RESOURCE_UNI(res)->matrix_stride;
      return 1;
   case GL_IS_ROW_MAJOR:
      VALIDATE_TYPE_2(GL_UNIFORM, GL_BUFFER_VARIABLE);
      *val = RESOURCE_UNI(res)->row_major;
      return 1;
   case GL_ATOMIC_COUNTER_BUFFER_INDEX:
      VALIDATE_TYPE(GL_UNIFORM);
      *val = RESOURCE_UNI(res)->atomic_buffer_index;
      return 1;
   case GL_BUFFER_BINDING:
   case GL_BUFFER_DATA_SIZE:
   case GL_NUM_ACTIVE_VARIABLES:
   case GL_ACTIVE_VARIABLES:
      return get_buffer_property(shProg, res, prop, val, glthread, caller);
   case GL_REFERENCED_BY_COMPUTE_SHADER:
      if (!_mesa_has_compute_shaders(ctx))
         goto invalid_enum;
      FALLTHROUGH;
   case GL_REFERENCED_BY_VERTEX_SHADER:
   case GL_REFERENCED_BY_TESS_CONTROL_SHADER:
   case GL_REFERENCED_BY_TESS_EVALUATION_SHADER:
   case GL_REFERENCED_BY_GEOMETRY_SHADER:
   case GL_REFERENCED_BY_FRAGMENT_SHADER:
      switch (res->Type) {
      case GL_UNIFORM:
      case GL_PROGRAM_INPUT:
      case GL_PROGRAM_OUTPUT:
      case GL_UNIFORM_BLOCK:
      case GL_BUFFER_VARIABLE:
      case GL_SHADER_STORAGE_BLOCK:
      case GL_ATOMIC_COUNTER_BUFFER:
         *val = is_resource_referenced(shProg, res, index,
                                       stage_from_enum(prop));
         return 1;
      default:
         goto invalid_operation;
      }
   case GL_LOCATION:
      switch (res->Type) {
      case GL_UNIFORM:
      case GL_VERTEX_SUBROUTINE_UNIFORM:
      case GL_GEOMETRY_SUBROUTINE_UNIFORM:
      case GL_FRAGMENT_SUBROUTINE_UNIFORM:
      case GL_COMPUTE_SUBROUTINE_UNIFORM:
      case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
      case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM:
      case GL_PROGRAM_INPUT:
      case GL_PROGRAM_OUTPUT:
         *val = program_resource_location(res, 0);
         return 1;
      default:
         goto invalid_operation;
      }
   case GL_LOCATION_COMPONENT:
      switch (res->Type) {
      case GL_PROGRAM_INPUT:
      case GL_PROGRAM_OUTPUT:
         *val = RESOURCE_VAR(res)->component;
         return 1;
      default:
         goto invalid_operation;
      }
   case GL_LOCATION_INDEX: {
      int tmp;
      if (res->Type != GL_PROGRAM_OUTPUT)
         goto invalid_operation;
      tmp = program_resource_location(res, 0);
      if (tmp == -1)
         *val = -1;
      else
         *val = _get_resource_location_index(res);
      return 1;
   }
   case GL_NUM_COMPATIBLE_SUBROUTINES:
      if (res->Type != GL_VERTEX_SUBROUTINE_UNIFORM &&
          res->Type != GL_FRAGMENT_SUBROUTINE_UNIFORM &&
          res->Type != GL_GEOMETRY_SUBROUTINE_UNIFORM &&
          res->Type != GL_COMPUTE_SUBROUTINE_UNIFORM &&
          res->Type != GL_TESS_CONTROL_SUBROUTINE_UNIFORM &&
          res->Type != GL_TESS_EVALUATION_SUBROUTINE_UNIFORM)
         goto invalid_operation;
      *val = RESOURCE_UNI(res)->num_compatible_subroutines;
      return 1;
   case GL_COMPATIBLE_SUBROUTINES: {
      const struct gl_uniform_storage *uni;
      struct gl_program *p;
      unsigned count, i;
      int j;

      if (res->Type != GL_VERTEX_SUBROUTINE_UNIFORM &&
          res->Type != GL_FRAGMENT_SUBROUTINE_UNIFORM &&
          res->Type != GL_GEOMETRY_SUBROUTINE_UNIFORM &&
          res->Type != GL_COMPUTE_SUBROUTINE_UNIFORM &&
          res->Type != GL_TESS_CONTROL_SUBROUTINE_UNIFORM &&
          res->Type != GL_TESS_EVALUATION_SUBROUTINE_UNIFORM)
         goto invalid_operation;
      uni = RESOURCE_UNI(res);

      p = shProg->_LinkedShaders[_mesa_shader_stage_from_subroutine_uniform(res->Type)]->Program;
      count = 0;
      for (i = 0; i < p->sh.NumSubroutineFunctions; i++) {
         struct gl_subroutine_function *fn = &p->sh.SubroutineFunctions[i];
         for (j = 0; j < fn->num_compat_types; j++) {
            if (fn->types[j] == uni->type) {
               val[count++] = i;
               break;
            }
         }
      }
      return count;
   }

   case GL_TOP_LEVEL_ARRAY_SIZE:
      VALIDATE_TYPE(GL_BUFFER_VARIABLE);
      *val = RESOURCE_UNI(res)->top_level_array_size;
      return 1;

   case GL_TOP_LEVEL_ARRAY_STRIDE:
      VALIDATE_TYPE(GL_BUFFER_VARIABLE);
      *val = RESOURCE_UNI(res)->top_level_array_stride;
      return 1;

   /* GL_ARB_tessellation_shader */
   case GL_IS_PER_PATCH:
      switch (res->Type) {
      case GL_PROGRAM_INPUT:
      case GL_PROGRAM_OUTPUT:
         *val = RESOURCE_VAR(res)->patch;
         return 1;
      default:
         goto invalid_operation;
      }

   case GL_TRANSFORM_FEEDBACK_BUFFER_INDEX:
      VALIDATE_TYPE(GL_TRANSFORM_FEEDBACK_VARYING);
      *val = RESOURCE_XFV(res)->BufferIndex;
      return 1;
   case GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE:
      VALIDATE_TYPE(GL_TRANSFORM_FEEDBACK_BUFFER);
      *val = RESOURCE_XFB(res)->Stride * 4;
      return 1;

   default:
      goto invalid_enum;
   }

#undef VALIDATE_TYPE
#undef VALIDATE_TYPE_2

invalid_enum:
   _mesa_error_glthread_safe(ctx, GL_INVALID_ENUM, glthread,
                             "%s(%s prop %s)", caller,
                             _mesa_enum_to_string(res->Type),
                             _mesa_enum_to_string(prop));
   return 0;

invalid_operation:
   _mesa_error_glthread_safe(ctx, GL_INVALID_OPERATION, glthread,
                             "%s(%s prop %s)", caller,
                             _mesa_enum_to_string(res->Type),
                             _mesa_enum_to_string(prop));
   return 0;
}

extern void
_mesa_get_program_resourceiv(struct gl_shader_program *shProg,
                             GLenum programInterface, GLuint index, GLsizei propCount,
                             const GLenum *props, GLsizei bufSize,
                             GLsizei *length, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint *val = (GLint *) params;
   const GLenum *prop = props;
   GLsizei amount = 0;

   struct gl_program_resource *res =
      _mesa_program_resource_find_index(shProg, programInterface, index);

   /* No such resource found or bufSize negative. */
   if (!res || bufSize < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetProgramResourceiv(%s index %d bufSize %d)",
                  _mesa_enum_to_string(programInterface), index, bufSize);
      return;
   }

   /* Write propCount values until error occurs or bufSize reached. */
   for (int i = 0; i < propCount && i < bufSize; i++, val++, prop++) {
      int props_written =
         _mesa_program_resource_prop(shProg, res, index, *prop, val,
                                     false, "glGetProgramResourceiv");

      /* Error happened. */
      if (props_written == 0)
         return;

      amount += props_written;
   }

   /* If <length> is not NULL, the actual number of integer values
    * written to <params> will be written to <length>.
    */
   if (length)
      *length = amount;
}

extern void
_mesa_get_program_interfaceiv(struct gl_shader_program *shProg,
                              GLenum programInterface, GLenum pname,
                              GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   unsigned i;

   /* Validate pname against interface. */
   switch(pname) {
   case GL_ACTIVE_RESOURCES:
      for (i = 0, *params = 0; i < shProg->data->NumProgramResourceList; i++)
         if (shProg->data->ProgramResourceList[i].Type == programInterface)
            (*params)++;
      break;
   case GL_MAX_NAME_LENGTH:
      if (programInterface == GL_ATOMIC_COUNTER_BUFFER ||
          programInterface == GL_TRANSFORM_FEEDBACK_BUFFER) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glGetProgramInterfaceiv(%s pname %s)",
                     _mesa_enum_to_string(programInterface),
                     _mesa_enum_to_string(pname));
         return;
      }
      /* Name length consists of base name, 3 additional chars '[0]' if
       * resource is an array and finally 1 char for string terminator.
       */
      for (i = 0, *params = 0; i < shProg->data->NumProgramResourceList; i++) {
         if (shProg->data->ProgramResourceList[i].Type != programInterface)
            continue;
         unsigned len =
            _mesa_program_resource_name_length_array(&shProg->data->ProgramResourceList[i]);
         *params = MAX2((unsigned)*params, len + 1);
      }
      break;
   case GL_MAX_NUM_ACTIVE_VARIABLES:
      switch (programInterface) {
      case GL_UNIFORM_BLOCK:
         for (i = 0, *params = 0; i < shProg->data->NumProgramResourceList; i++) {
            if (shProg->data->ProgramResourceList[i].Type == programInterface) {
               struct gl_uniform_block *block =
                  (struct gl_uniform_block *)
                  shProg->data->ProgramResourceList[i].Data;
               *params = MAX2((unsigned)*params, block->NumUniforms);
            }
         }
         break;
      case GL_SHADER_STORAGE_BLOCK:
         for (i = 0, *params = 0; i < shProg->data->NumProgramResourceList; i++) {
            if (shProg->data->ProgramResourceList[i].Type == programInterface) {
               struct gl_uniform_block *block =
                  (struct gl_uniform_block *)
                  shProg->data->ProgramResourceList[i].Data;
               GLint block_params = 0;
               for (unsigned j = 0; j < block->NumUniforms; j++) {
                  struct gl_program_resource *uni =
                     _mesa_program_resource_find_active_variable(
                        shProg,
                        GL_BUFFER_VARIABLE,
                        block,
                        j);
                  if (!uni)
                     continue;
                  block_params++;
               }
               *params = MAX2(*params, block_params);
            }
         }
         break;
      case GL_ATOMIC_COUNTER_BUFFER:
         for (i = 0, *params = 0; i < shProg->data->NumProgramResourceList; i++) {
            if (shProg->data->ProgramResourceList[i].Type == programInterface) {
               struct gl_active_atomic_buffer *buffer =
                  (struct gl_active_atomic_buffer *)
                  shProg->data->ProgramResourceList[i].Data;
               *params = MAX2((unsigned)*params, buffer->NumUniforms);
            }
         }
         break;
      case GL_TRANSFORM_FEEDBACK_BUFFER:
         for (i = 0, *params = 0; i < shProg->data->NumProgramResourceList; i++) {
            if (shProg->data->ProgramResourceList[i].Type == programInterface) {
               struct gl_transform_feedback_buffer *buffer =
                  (struct gl_transform_feedback_buffer *)
                  shProg->data->ProgramResourceList[i].Data;
               *params = MAX2((unsigned)*params, buffer->NumVaryings);
            }
         }
         break;
      default:
        _mesa_error(ctx, GL_INVALID_OPERATION,
                    "glGetProgramInterfaceiv(%s pname %s)",
                    _mesa_enum_to_string(programInterface),
                    _mesa_enum_to_string(pname));
      }
      break;
   case GL_MAX_NUM_COMPATIBLE_SUBROUTINES:
      switch (programInterface) {
      case GL_VERTEX_SUBROUTINE_UNIFORM:
      case GL_FRAGMENT_SUBROUTINE_UNIFORM:
      case GL_GEOMETRY_SUBROUTINE_UNIFORM:
      case GL_COMPUTE_SUBROUTINE_UNIFORM:
      case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
      case GL_TESS_EVALUATION_SUBROUTINE_UNIFORM: {
         for (i = 0, *params = 0; i < shProg->data->NumProgramResourceList; i++) {
            if (shProg->data->ProgramResourceList[i].Type == programInterface) {
               struct gl_uniform_storage *uni =
                  (struct gl_uniform_storage *)
                  shProg->data->ProgramResourceList[i].Data;
               *params = MAX2((unsigned)*params, uni->num_compatible_subroutines);
            }
         }
         break;
      }

      default:
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glGetProgramInterfaceiv(%s pname %s)",
                     _mesa_enum_to_string(programInterface),
                     _mesa_enum_to_string(pname));
      }
      break;
   default:
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetProgramInterfaceiv(pname %s)",
                  _mesa_enum_to_string(pname));
   }
}

static bool
validate_io(struct gl_program *producer, struct gl_program *consumer)
{
   if (producer->sh.data->linked_stages == consumer->sh.data->linked_stages)
      return true;

   const bool producer_is_array_stage =
      producer->info.stage == MESA_SHADER_TESS_CTRL;
   const bool consumer_is_array_stage =
      consumer->info.stage == MESA_SHADER_GEOMETRY ||
      consumer->info.stage == MESA_SHADER_TESS_CTRL ||
      consumer->info.stage == MESA_SHADER_TESS_EVAL;

   bool valid = true;

   gl_shader_variable const **outputs =
      (gl_shader_variable const **) calloc(producer->sh.data->NumProgramResourceList,
                                           sizeof(gl_shader_variable *));
   if (outputs == NULL)
      return false;

   /* Section 7.4.1 (Shader Interface Matching) of the OpenGL ES 3.1 spec
    * says:
    *
    *    At an interface between program objects, the set of inputs and
    *    outputs are considered to match exactly if and only if:
    *
    *    - Every declared input variable has a matching output, as described
    *      above.
    *    - There are no user-defined output variables declared without a
    *      matching input variable declaration.
    *
    * Every input has an output, and every output has an input.  Scan the list
    * of producer resources once, and generate the list of outputs.  As inputs
    * and outputs are matched, remove the matched outputs from the set.  At
    * the end, the set must be empty.  If the set is not empty, then there is
    * some output that did not have an input.
    */
   unsigned num_outputs = 0;
   for (unsigned i = 0; i < producer->sh.data->NumProgramResourceList; i++) {
      struct gl_program_resource *res =
         &producer->sh.data->ProgramResourceList[i];

      if (res->Type != GL_PROGRAM_OUTPUT)
         continue;

      gl_shader_variable const *const var = RESOURCE_VAR(res);

      /* Section 7.4.1 (Shader Interface Matching) of the OpenGL ES 3.1 spec
       * says:
       *
       *    Built-in inputs or outputs do not affect interface matching.
       */
      if (is_gl_identifier(var->name.string))
         continue;

      outputs[num_outputs++] = var;
   }

   unsigned match_index = 0;
   for (unsigned i = 0; i < consumer->sh.data->NumProgramResourceList; i++) {
      struct gl_program_resource *res =
         &consumer->sh.data->ProgramResourceList[i];

      if (res->Type != GL_PROGRAM_INPUT)
         continue;

      gl_shader_variable const *const consumer_var = RESOURCE_VAR(res);
      gl_shader_variable const *producer_var = NULL;

      if (is_gl_identifier(consumer_var->name.string))
         continue;

      /* Inputs with explicit locations match other outputs with explicit
       * locations by location instead of by name.
       */
      if (consumer_var->explicit_location) {
         for (unsigned j = 0; j < num_outputs; j++) {
            const gl_shader_variable *const var = outputs[j];

            if (var->explicit_location &&
                consumer_var->location == var->location) {
               producer_var = var;
               match_index = j;
               break;
            }
         }
      } else {
         for (unsigned j = 0; j < num_outputs; j++) {
            const gl_shader_variable *const var = outputs[j];

            if (!var->explicit_location &&
                strcmp(consumer_var->name.string, var->name.string) == 0) {
               producer_var = var;
               match_index = j;
               break;
            }
         }
      }

      /* Section 7.4.1 (Shader Interface Matching) of the OpenGL ES 3.1 spec
       * says:
       *
       *    - An output variable is considered to match an input variable in
       *      the subsequent shader if:
       *
       *      - the two variables match in name, type, and qualification; or
       *
       *      - the two variables are declared with the same location
       *        qualifier and match in type and qualification.
       */
      if (producer_var == NULL) {
         valid = false;
         goto out;
      }

      /* An output cannot match more than one input, so remove the output from
       * the set of possible outputs.
       */
      outputs[match_index] = NULL;
      num_outputs--;
      if (match_index < num_outputs)
         outputs[match_index] = outputs[num_outputs];

      /* Section 7.4.1 (Shader Interface Matching) of the ES 3.2 spec says:
       *
       *    "Tessellation control shader per-vertex output variables and
       *     blocks and tessellation control, tessellation evaluation, and
       *     geometry shader per-vertex input variables and blocks are
       *     required to be declared as arrays, with each element representing
       *     input or output values for a single vertex of a multi-vertex
       *     primitive. For the purposes of interface matching, such variables
       *     and blocks are treated as though they were not declared as
       *     arrays."
       *
       * So we unwrap those types before matching.
       */
      const glsl_type *consumer_type = consumer_var->type;
      const glsl_type *consumer_interface_type = consumer_var->interface_type;
      const glsl_type *producer_type = producer_var->type;
      const glsl_type *producer_interface_type = producer_var->interface_type;

      if (consumer_is_array_stage) {
         if (consumer_interface_type) {
            /* the interface is the array; the underlying types should match */
            if (glsl_type_is_array(consumer_interface_type) && !consumer_var->patch)
               consumer_interface_type = consumer_interface_type->fields.array;
         } else {
            if (glsl_type_is_array(consumer_type) && !consumer_var->patch)
               consumer_type = consumer_type->fields.array;
         }
      }

      if (producer_is_array_stage) {
         if (producer_interface_type) {
            /* the interface is the array; the underlying types should match */
            if (glsl_type_is_array(producer_interface_type) && !producer_var->patch)
               producer_interface_type = producer_interface_type->fields.array;
         } else {
            if (glsl_type_is_array(producer_type) && !producer_var->patch)
               producer_type = producer_type->fields.array;
         }
      }

      if (producer_type != consumer_type) {
         valid = false;
         goto out;
      }

      if (producer_interface_type != consumer_interface_type) {
         valid = false;
         goto out;
      }

      /* Section 9.2.2 (Separable Programs) of the GLSL ES spec says:
       *
       *    Qualifier Class|  Qualifier  |in/out
       *    ---------------+-------------+------
       *    Storage        |     in      |
       *                   |     out     |  N/A
       *                   |   uniform   |
       *    ---------------+-------------+------
       *    Auxiliary      |   centroid  |   No
       *    ---------------+-------------+------
       *                   |   location  |  Yes
       *                   | Block layout|  N/A
       *                   |   binding   |  N/A
       *                   |   offset    |  N/A
       *                   |   format    |  N/A
       *    ---------------+-------------+------
       *    Interpolation  |   smooth    |
       *                   |    flat     |  Yes
       *    ---------------+-------------+------
       *                   |    lowp     |
       *    Precision      |   mediump   |  Yes
       *                   |    highp    |
       *    ---------------+-------------+------
       *    Variance       |  invariant  |   No
       *    ---------------+-------------+------
       *    Memory         |     all     |  N/A
       *
       * Note that location mismatches are detected by the loops above that
       * find the producer variable that goes with the consumer variable.
       */
      unsigned producer_interpolation = producer_var->interpolation;
      unsigned consumer_interpolation = consumer_var->interpolation;
      if (producer_interpolation == INTERP_MODE_NONE)
         producer_interpolation = INTERP_MODE_SMOOTH;
      if (consumer_interpolation == INTERP_MODE_NONE)
         consumer_interpolation = INTERP_MODE_SMOOTH;
      if (producer_interpolation != consumer_interpolation) {
         valid = false;
         goto out;
      }

      if (producer_var->precision != consumer_var->precision) {
         valid = false;
         goto out;
      }

      if (producer_var->outermost_struct_type != consumer_var->outermost_struct_type) {
         valid = false;
         goto out;
      }
   }

 out:
   free(outputs);
   return valid && num_outputs == 0;
}

/**
 * Validate inputs against outputs in a program pipeline.
 */
extern "C" bool
_mesa_validate_pipeline_io(struct gl_pipeline_object *pipeline)
{
   struct gl_program **prog = (struct gl_program **) pipeline->CurrentProgram;

   /* Find first active stage in pipeline. */
   unsigned idx, prev = 0;
   for (idx = 0; idx < ARRAY_SIZE(pipeline->CurrentProgram); idx++) {
      if (prog[idx]) {
         prev = idx;
         break;
      }
   }

   for (idx = prev + 1; idx < ARRAY_SIZE(pipeline->CurrentProgram); idx++) {
      if (prog[idx]) {
         /* Pipeline might include both non-compute and a compute program, do
          * not attempt to validate varyings between non-compute and compute
          * stage.
          */
         if (prog[idx]->info.stage == MESA_SHADER_COMPUTE)
            break;

         if (!validate_io(prog[prev], prog[idx]))
            return false;

         prev = idx;
      }
   }
   return true;
}

extern "C" void
_mesa_program_resource_hash_destroy(struct gl_shader_program *shProg)
{
   for (unsigned i = 0; i < ARRAY_SIZE(shProg->data->ProgramResourceHash); i++) {
      if (shProg->data->ProgramResourceHash[i]) {
         _mesa_hash_table_destroy(shProg->data->ProgramResourceHash[i], NULL);
         shProg->data->ProgramResourceHash[i] = NULL;
      }
   }
}

extern "C" void
_mesa_create_program_resource_hash(struct gl_shader_program *shProg)
{
   /* Rebuild resource hash. */
   _mesa_program_resource_hash_destroy(shProg);

   struct gl_program_resource *res = shProg->data->ProgramResourceList;
   for (unsigned i = 0; i < shProg->data->NumProgramResourceList; i++, res++) {
      struct gl_resource_name name;
      if (_mesa_program_get_resource_name(res, &name)) {
         unsigned type = GET_PROGRAM_RESOURCE_TYPE_FROM_GLENUM(res->Type);
         assert(type < ARRAY_SIZE(shProg->data->ProgramResourceHash));

         if (!shProg->data->ProgramResourceHash[type]) {
            shProg->data->ProgramResourceHash[type] =
               _mesa_hash_table_create(shProg, _mesa_hash_string,
                                       _mesa_key_string_equal);
         }

         _mesa_hash_table_insert(shProg->data->ProgramResourceHash[type],
                                 name.string, res);
      }
   }
}
