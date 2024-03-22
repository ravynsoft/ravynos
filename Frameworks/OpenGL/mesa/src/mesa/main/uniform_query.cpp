/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2004-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009-2010  VMware, Inc.  All Rights Reserved.
 * Copyright © 2010, 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <inttypes.h>  /* for PRIx64 macro */
#include <math.h>

#include "main/context.h"
#include "main/draw_validate.h"
#include "main/shaderapi.h"
#include "main/shaderobj.h"
#include "main/uniforms.h"
#include "compiler/glsl/ir.h"
#include "compiler/glsl/ir_uniform.h"
#include "compiler/glsl/glsl_parser_extras.h"
#include "compiler/glsl/program.h"
#include "util/bitscan.h"

#include "state_tracker/st_context.h"

/* This is one of the few glGet that can be called from the app thread safely.
 * Only these conditions must be met:
 * - There are no unfinished glLinkProgram and glDeleteProgram calls
 *   for the program object. This assures that the program object is immutable.
 * - glthread=true for GL errors to be passed to the driver thread safely
 *
 * Program objects can be looked up from any thread because they are part
 * of the multi-context shared state.
 */
extern "C" void
_mesa_GetActiveUniform_impl(GLuint program, GLuint index,
                            GLsizei maxLength, GLsizei *length, GLint *size,
                            GLenum *type, GLcharARB *nameOut, bool glthread)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;
   struct gl_program_resource *res;

   if (maxLength < 0) {
      _mesa_error_glthread_safe(ctx, GL_INVALID_VALUE, glthread,
                                "glGetActiveUniform(maxLength < 0)");
      return;
   }

   shProg = _mesa_lookup_shader_program_err_glthread(ctx, program, glthread,
                                                     "glGetActiveUniform");
   if (!shProg)
      return;

   res = _mesa_program_resource_find_index((struct gl_shader_program *) shProg,
                                           GL_UNIFORM, index);

   if (!res) {
      _mesa_error_glthread_safe(ctx, GL_INVALID_VALUE, glthread,
                                "glGetActiveUniform(index)");
      return;
   }

   if (nameOut)
      _mesa_get_program_resource_name(shProg, GL_UNIFORM, index, maxLength,
                                      length, nameOut, glthread,
                                      "glGetActiveUniform");
   if (type)
      _mesa_program_resource_prop((struct gl_shader_program *) shProg,
                                  res, index, GL_TYPE, (GLint*) type,
                                  glthread, "glGetActiveUniform");
   if (size)
      _mesa_program_resource_prop((struct gl_shader_program *) shProg,
                                  res, index, GL_ARRAY_SIZE, (GLint*) size,
                                  glthread, "glGetActiveUniform");
}

extern "C" void GLAPIENTRY
_mesa_GetActiveUniform(GLuint program, GLuint index,
                       GLsizei maxLength, GLsizei *length, GLint *size,
                       GLenum *type, GLcharARB *nameOut)
{
   _mesa_GetActiveUniform_impl(program, index, maxLength, length, size,
                               type, nameOut, false);
}

static GLenum
resource_prop_from_uniform_prop(GLenum uni_prop)
{
   switch (uni_prop) {
   case GL_UNIFORM_TYPE:
      return GL_TYPE;
   case GL_UNIFORM_SIZE:
      return GL_ARRAY_SIZE;
   case GL_UNIFORM_NAME_LENGTH:
      return GL_NAME_LENGTH;
   case GL_UNIFORM_BLOCK_INDEX:
      return GL_BLOCK_INDEX;
   case GL_UNIFORM_OFFSET:
      return GL_OFFSET;
   case GL_UNIFORM_ARRAY_STRIDE:
      return GL_ARRAY_STRIDE;
   case GL_UNIFORM_MATRIX_STRIDE:
      return GL_MATRIX_STRIDE;
   case GL_UNIFORM_IS_ROW_MAJOR:
      return GL_IS_ROW_MAJOR;
   case GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX:
      return GL_ATOMIC_COUNTER_BUFFER_INDEX;
   default:
      return 0;
   }
}

extern "C" void GLAPIENTRY
_mesa_GetActiveUniformsiv(GLuint program,
			  GLsizei uniformCount,
			  const GLuint *uniformIndices,
			  GLenum pname,
			  GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;
   struct gl_program_resource *res;
   GLenum res_prop;

   if (uniformCount < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glGetActiveUniformsiv(uniformCount < 0)");
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program, "glGetActiveUniform");
   if (!shProg)
      return;

   res_prop = resource_prop_from_uniform_prop(pname);

   /* We need to first verify that each entry exists as active uniform. If
    * not, generate error and do not cause any other side effects.
    *
    * In the case of and error condition, Page 16 (section 2.3.1 Errors)
    * of the OpenGL 4.5 spec says:
    *
    *     "If the generating command modifies values through a pointer argu-
    *     ment, no change is made to these values."
    */
   for (int i = 0; i < uniformCount; i++) {
      if (!_mesa_program_resource_find_index(shProg, GL_UNIFORM,
                                              uniformIndices[i])) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glGetActiveUniformsiv(index)");
         return;
      }
   }

   for (int i = 0; i < uniformCount; i++) {
      res = _mesa_program_resource_find_index(shProg, GL_UNIFORM,
                                              uniformIndices[i]);
      if (!_mesa_program_resource_prop(shProg, res, uniformIndices[i],
                                       res_prop, &params[i],
                                       false, "glGetActiveUniformsiv"))
         break;
   }
}

static struct gl_uniform_storage *
validate_uniform_parameters(GLint location, GLsizei count,
                            unsigned *array_index,
                            struct gl_context *ctx,
                            struct gl_shader_program *shProg,
                            const char *caller)
{
   if (shProg == NULL) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(program not linked)", caller);
      return NULL;
   }

   /* From page 12 (page 26 of the PDF) of the OpenGL 2.1 spec:
    *
    *     "If a negative number is provided where an argument of type sizei or
    *     sizeiptr is specified, the error INVALID_VALUE is generated."
    */
   if (count < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(count < 0)", caller);
      return NULL;
   }

   /* Check that the given location is in bounds of uniform remap table.
    * Unlinked programs will have NumUniformRemapTable == 0, so we can take
    * the shProg->data->LinkStatus check out of the main path.
    */
   if (unlikely(location >= (GLint) shProg->NumUniformRemapTable)) {
      if (!shProg->data->LinkStatus)
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(program not linked)",
                     caller);
      else
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(location=%d)",
                     caller, location);

      return NULL;
   }

   if (location == -1) {
      if (!shProg->data->LinkStatus)
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(program not linked)",
                     caller);

      return NULL;
   }

   /* Page 82 (page 96 of the PDF) of the OpenGL 2.1 spec says:
    *
    *     "If any of the following conditions occur, an INVALID_OPERATION
    *     error is generated by the Uniform* commands, and no uniform values
    *     are changed:
    *
    *     ...
    *
    *         - if no variable with a location of location exists in the
    *           program object currently in use and location is not -1,
    *         - if count is greater than one, and the uniform declared in the
    *           shader is not an array variable,
    */
   if (location < -1 || !shProg->UniformRemapTable[location]) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(location=%d)",
                  caller, location);
      return NULL;
   }

   /* If the driver storage pointer in remap table is -1, we ignore silently.
    *
    * GL_ARB_explicit_uniform_location spec says:
    *     "What happens if Uniform* is called with an explicitly defined
    *     uniform location, but that uniform is deemed inactive by the
    *     linker?
    *
    *     RESOLVED: The call is ignored for inactive uniform variables and
    *     no error is generated."
    *
    */
   if (shProg->UniformRemapTable[location] ==
       INACTIVE_UNIFORM_EXPLICIT_LOCATION)
      return NULL;

   struct gl_uniform_storage *const uni = shProg->UniformRemapTable[location];

   /* Even though no location is assigned to a built-in uniform and this
    * function should already have returned NULL, this test makes it explicit
    * that we are not allowing to update the value of a built-in.
    */
   if (uni->builtin)
      return NULL;

   if (uni->array_elements == 0) {
      if (count > 1) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(count = %u for non-array \"%s\"@%d)",
                     caller, count, uni->name.string, location);
         return NULL;
      }

      assert((location - uni->remap_location) == 0);
      *array_index = 0;
   } else {
      /* The array index specified by the uniform location is just the uniform
       * location minus the base location of of the uniform.
       */
      *array_index = location - uni->remap_location;

      /* If the uniform is an array, check that array_index is in bounds.
       * array_index is unsigned so no need to check for less than zero.
       */
      if (*array_index >= uni->array_elements) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(location=%d)",
                     caller, location);
         return NULL;
      }
   }
   return uni;
}

/**
 * Called via glGetUniform[fiui]v() to get the current value of a uniform.
 */
extern "C" void
_mesa_get_uniform(struct gl_context *ctx, GLuint program, GLint location,
		  GLsizei bufSize, enum glsl_base_type returnType,
		  GLvoid *paramsOut)
{
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program, "glGetUniformfv");
   unsigned offset;

   struct gl_uniform_storage *const uni =
      validate_uniform_parameters(location, 1, &offset,
                                  ctx, shProg, "glGetUniform");
   if (uni == NULL) {
      /* For glGetUniform, page 264 (page 278 of the PDF) of the OpenGL 2.1
       * spec says:
       *
       *     "The error INVALID_OPERATION is generated if program has not been
       *     linked successfully, or if location is not a valid location for
       *     program."
       *
       * For glUniform, page 82 (page 96 of the PDF) of the OpenGL 2.1 spec
       * says:
       *
       *     "If the value of location is -1, the Uniform* commands will
       *     silently ignore the data passed in, and the current uniform
       *     values will not be changed."
       *
       * Allowing -1 for the location parameter of glUniform allows
       * applications to avoid error paths in the case that, for example, some
       * uniform variable is removed by the compiler / linker after
       * optimization.  In this case, the new value of the uniform is dropped
       * on the floor.  For the case of glGetUniform, there is nothing
       * sensible to do for a location of -1.
       *
       * If the location was -1, validate_unfirom_parameters will return NULL
       * without raising an error.  Raise the error here.
       */
      if (location == -1) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "glGetUniform(location=%d)",
                     location);
      }

      return;
   }

   {
      unsigned elements = glsl_get_components(uni->type);
      unsigned components = uni->type->vector_elements;

      const int rmul = glsl_base_type_is_64bit(returnType) ? 2 : 1;
      int dmul = (glsl_type_is_64bit(uni->type)) ? 2 : 1;

      if ((glsl_type_is_sampler(uni->type) || glsl_type_is_image(uni->type)) &&
          !uni->is_bindless) {
         /* Non-bindless samplers/images are represented using unsigned integer
          * 32-bit, while bindless handles are 64-bit.
          */
         dmul = 1;
      }

      /* Calculate the source base address *BEFORE* modifying elements to
       * account for the size of the user's buffer.
       */
      const union gl_constant_value *src;
      if (ctx->Const.PackedDriverUniformStorage &&
          (uni->is_bindless || !glsl_contains_opaque(uni->type))) {
         unsigned dword_elements = elements;

         /* 16-bit uniforms are packed. */
         if (glsl_base_type_is_16bit(uni->type->base_type)) {
            dword_elements = DIV_ROUND_UP(components, 2) *
                             uni->type->matrix_columns;
         }

         src = (gl_constant_value *) uni->driver_storage[0].data +
            (offset * dword_elements * dmul);
      } else {
         src = &uni->storage[offset * elements * dmul];
      }

      assert(returnType == GLSL_TYPE_FLOAT || returnType == GLSL_TYPE_INT ||
             returnType == GLSL_TYPE_UINT || returnType == GLSL_TYPE_DOUBLE ||
             returnType == GLSL_TYPE_UINT64 || returnType == GLSL_TYPE_INT64);

      /* doubles have a different size than the other 3 types */
      unsigned bytes = sizeof(src[0]) * elements * rmul;
      if (bufSize < 0 || bytes > (unsigned) bufSize) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glGetnUniform*vARB(out of bounds: bufSize is %d,"
                     " but %u bytes are required)", bufSize, bytes);
         return;
      }

      /* If the return type and the uniform's native type are "compatible,"
       * just memcpy the data.  If the types are not compatible, perform a
       * slower convert-and-copy process.
       */
      if (returnType == uni->type->base_type ||
          ((returnType == GLSL_TYPE_INT || returnType == GLSL_TYPE_UINT) &&
           (glsl_type_is_sampler(uni->type) || glsl_type_is_image(uni->type))) ||
          (returnType == GLSL_TYPE_UINT64 && uni->is_bindless)) {
         memcpy(paramsOut, src, bytes);
      } else {
         union gl_constant_value *const dst =
            (union gl_constant_value *) paramsOut;
         /* This code could be optimized by putting the loop inside the switch
          * statements.  However, this is not expected to be
          * performance-critical code.
          */
         for (unsigned i = 0; i < elements; i++) {
            int sidx = i * dmul;
            int didx = i * rmul;

            if (glsl_base_type_is_16bit(uni->type->base_type)) {
               unsigned column = i / components;
               unsigned row = i % components;
               sidx = column * align(components, 2) + row;
            }

            switch (returnType) {
            case GLSL_TYPE_FLOAT:
               switch (uni->type->base_type) {
               case GLSL_TYPE_FLOAT16:
                  dst[didx].f = _mesa_half_to_float(((uint16_t*)src)[sidx]);
                  break;
               case GLSL_TYPE_UINT:
                  dst[didx].f = (float) src[sidx].u;
                  break;
               case GLSL_TYPE_INT:
               case GLSL_TYPE_SAMPLER:
               case GLSL_TYPE_IMAGE:
                  dst[didx].f = (float) src[sidx].i;
                  break;
               case GLSL_TYPE_BOOL:
                  dst[didx].f = src[sidx].i ? 1.0f : 0.0f;
                  break;
               case GLSL_TYPE_DOUBLE: {
                  double tmp;
                  memcpy(&tmp, &src[sidx].f, sizeof(tmp));
                  dst[didx].f = tmp;
                  break;
               }
               case GLSL_TYPE_UINT64: {
                  uint64_t tmp;
                  memcpy(&tmp, &src[sidx].u, sizeof(tmp));
                  dst[didx].f = tmp;
                  break;
                }
               case GLSL_TYPE_INT64: {
                  uint64_t tmp;
                  memcpy(&tmp, &src[sidx].i, sizeof(tmp));
                  dst[didx].f = tmp;
                  break;
               }
               default:
                  assert(!"Should not get here.");
                  break;
               }
               break;

            case GLSL_TYPE_DOUBLE:
               switch (uni->type->base_type) {
               case GLSL_TYPE_FLOAT16: {
                  double f = _mesa_half_to_float(((uint16_t*)src)[sidx]);
                  memcpy(&dst[didx].f, &f, sizeof(f));
                  break;
               }
               case GLSL_TYPE_UINT: {
                  double tmp = src[sidx].u;
                  memcpy(&dst[didx].f, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_INT:
               case GLSL_TYPE_SAMPLER:
               case GLSL_TYPE_IMAGE: {
                  double tmp = src[sidx].i;
                  memcpy(&dst[didx].f, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_BOOL: {
                  double tmp = src[sidx].i ? 1.0 : 0.0;
                  memcpy(&dst[didx].f, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_FLOAT: {
                  double tmp = src[sidx].f;
                  memcpy(&dst[didx].f, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_UINT64: {
                  uint64_t tmpu;
                  double tmp;
                  memcpy(&tmpu, &src[sidx].u, sizeof(tmpu));
                  tmp = tmpu;
                  memcpy(&dst[didx].f, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_INT64: {
                  int64_t tmpi;
                  double tmp;
                  memcpy(&tmpi, &src[sidx].i, sizeof(tmpi));
                  tmp = tmpi;
                  memcpy(&dst[didx].f, &tmp, sizeof(tmp));
                  break;
               }
               default:
                  assert(!"Should not get here.");
                  break;
               }
               break;

            case GLSL_TYPE_INT:
               switch (uni->type->base_type) {
               case GLSL_TYPE_FLOAT:
                  /* While the GL 3.2 core spec doesn't explicitly
                   * state how conversion of float uniforms to integer
                   * values works, in section 6.2 "State Tables" on
                   * page 267 it says:
                   *
                   *     "Unless otherwise specified, when floating
                   *      point state is returned as integer values or
                   *      integer state is returned as floating-point
                   *      values it is converted in the fashion
                   *      described in section 6.1.2"
                   *
                   * That section, on page 248, says:
                   *
                   *     "If GetIntegerv or GetInteger64v are called,
                   *      a floating-point value is rounded to the
                   *      nearest integer..."
                   */
                  dst[didx].i = (int64_t) roundf(src[sidx].f);
                  break;
               case GLSL_TYPE_FLOAT16:
                  dst[didx].i =
                     (int64_t)roundf(_mesa_half_to_float(((uint16_t*)src)[sidx]));
                  break;
               case GLSL_TYPE_BOOL:
                  dst[didx].i = src[sidx].i ? 1 : 0;
                  break;
               case GLSL_TYPE_UINT:
                  dst[didx].i = MIN2(src[sidx].i, INT_MAX);
                  break;
               case GLSL_TYPE_DOUBLE: {
                  double tmp;
                  memcpy(&tmp, &src[sidx].f, sizeof(tmp));
                  dst[didx].i = (int64_t) round(tmp);
                  break;
               }
               case GLSL_TYPE_UINT64: {
                  uint64_t tmp;
                  memcpy(&tmp, &src[sidx].u, sizeof(tmp));
                  dst[didx].i = tmp;
                  break;
               }
               case GLSL_TYPE_INT64: {
                  int64_t tmp;
                  memcpy(&tmp, &src[sidx].i, sizeof(tmp));
                  dst[didx].i = tmp;
                  break;
               }
               default:
                  assert(!"Should not get here.");
                  break;
               }
               break;

            case GLSL_TYPE_UINT:
               switch (uni->type->base_type) {
               case GLSL_TYPE_FLOAT:
                  /* The spec isn't terribly clear how to handle negative
                   * values with an unsigned return type.
                   *
                   * GL 4.5 section 2.2.2 ("Data Conversions for State
                   * Query Commands") says:
                   *
                   * "If a value is so large in magnitude that it cannot be
                   *  represented by the returned data type, then the nearest
                   *  value representable using the requested type is
                   *  returned."
                   */
                  dst[didx].u = src[sidx].f < 0.0f ?
                     0u : (uint32_t) roundf(src[sidx].f);
                  break;
               case GLSL_TYPE_FLOAT16: {
                  float f = _mesa_half_to_float(((uint16_t*)src)[sidx]);
                  dst[didx].u = f < 0.0f ? 0u : (uint32_t)roundf(f);
                  break;
               }
               case GLSL_TYPE_BOOL:
                  dst[didx].i = src[sidx].i ? 1 : 0;
                  break;
               case GLSL_TYPE_INT:
                  dst[didx].i = MAX2(src[sidx].i, 0);
                  break;
               case GLSL_TYPE_DOUBLE: {
                  double tmp;
                  memcpy(&tmp, &src[sidx].f, sizeof(tmp));
                  dst[didx].u = tmp < 0.0 ? 0u : (uint32_t) round(tmp);
                  break;
               }
               case GLSL_TYPE_UINT64: {
                  uint64_t tmp;
                  memcpy(&tmp, &src[sidx].u, sizeof(tmp));
                  dst[didx].i = MIN2(tmp, INT_MAX);
                  break;
               }
               case GLSL_TYPE_INT64: {
                  int64_t tmp;
                  memcpy(&tmp, &src[sidx].i, sizeof(tmp));
                  dst[didx].i = MAX2(tmp, 0);
                  break;
               }
               default:
                  unreachable("invalid uniform type");
               }
               break;

            case GLSL_TYPE_INT64:
               switch (uni->type->base_type) {
               case GLSL_TYPE_UINT: {
                  uint64_t tmp = src[sidx].u;
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_INT:
               case GLSL_TYPE_SAMPLER:
               case GLSL_TYPE_IMAGE: {
                  int64_t tmp = src[sidx].i;
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_BOOL: {
                  int64_t tmp = src[sidx].i ? 1.0f : 0.0f;
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_UINT64: {
                  uint64_t u64;
                  memcpy(&u64, &src[sidx].u, sizeof(u64));
                  int64_t tmp = MIN2(u64, INT_MAX);
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_FLOAT: {
                  int64_t tmp = (int64_t) roundf(src[sidx].f);
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_FLOAT16: {
                  float f = _mesa_half_to_float(((uint16_t*)src)[sidx]);
                  int64_t tmp = (int64_t) roundf(f);
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_DOUBLE: {
                  double d;
                  memcpy(&d, &src[sidx].f, sizeof(d));
                  int64_t tmp = (int64_t) round(d);
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               default:
                  assert(!"Should not get here.");
                  break;
               }
               break;

            case GLSL_TYPE_UINT64:
               switch (uni->type->base_type) {
               case GLSL_TYPE_UINT: {
                  uint64_t tmp = src[sidx].u;
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_INT:
               case GLSL_TYPE_SAMPLER:
               case GLSL_TYPE_IMAGE: {
                  int64_t tmp = MAX2(src[sidx].i, 0);
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_BOOL: {
                  int64_t tmp = src[sidx].i ? 1.0f : 0.0f;
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_INT64: {
                  uint64_t i64;
                  memcpy(&i64, &src[sidx].i, sizeof(i64));
                  uint64_t tmp = MAX2(i64, 0);
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_FLOAT: {
                  uint64_t tmp = src[sidx].f < 0.0f ?
                     0ull : (uint64_t) roundf(src[sidx].f);
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_FLOAT16: {
                  float f = _mesa_half_to_float(((uint16_t*)src)[sidx]);
                  uint64_t tmp = f < 0.0f ? 0ull : (uint64_t) roundf(f);
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               case GLSL_TYPE_DOUBLE: {
                  double d;
                  memcpy(&d, &src[sidx].f, sizeof(d));
                  uint64_t tmp = (d < 0.0) ? 0ull : (uint64_t) round(d);
                  memcpy(&dst[didx].u, &tmp, sizeof(tmp));
                  break;
               }
               default:
                  assert(!"Should not get here.");
                  break;
               }
               break;

            default:
               assert(!"Should not get here.");
               break;
            }
         }
      }
   }
}

static void
log_uniform(const void *values, enum glsl_base_type basicType,
	    unsigned rows, unsigned cols, unsigned count,
	    bool transpose,
	    const struct gl_shader_program *shProg,
	    GLint location,
	    const struct gl_uniform_storage *uni)
{

   const union gl_constant_value *v = (const union gl_constant_value *) values;
   const unsigned elems = rows * cols * count;
   const char *const extra = (cols == 1) ? "uniform" : "uniform matrix";

   printf("Mesa: set program %u %s \"%s\" (loc %d, type \"%s\", "
	  "transpose = %s) to: ",
	  shProg->Name, extra, uni->name.string, location, glsl_get_type_name(uni->type),
	  transpose ? "true" : "false");
   for (unsigned i = 0; i < elems; i++) {
      if (i != 0 && ((i % rows) == 0))
	 printf(", ");

      switch (basicType) {
      case GLSL_TYPE_UINT:
	 printf("%u ", v[i].u);
	 break;
      case GLSL_TYPE_INT:
	 printf("%d ", v[i].i);
	 break;
      case GLSL_TYPE_UINT64: {
         uint64_t tmp;
         memcpy(&tmp, &v[i * 2].u, sizeof(tmp));
         printf("%" PRIu64 " ", tmp);
         break;
      }
      case GLSL_TYPE_INT64: {
         int64_t tmp;
         memcpy(&tmp, &v[i * 2].u, sizeof(tmp));
         printf("%" PRId64 " ", tmp);
         break;
      }
      case GLSL_TYPE_FLOAT:
	 printf("%g ", v[i].f);
	 break;
      case GLSL_TYPE_DOUBLE: {
         double tmp;
         memcpy(&tmp, &v[i * 2].f, sizeof(tmp));
         printf("%g ", tmp);
         break;
      }
      default:
	 assert(!"Should not get here.");
	 break;
      }
   }
   printf("\n");
   fflush(stdout);
}

#if 0
static void
log_program_parameters(const struct gl_shader_program *shProg)
{
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      if (shProg->_LinkedShaders[i] == NULL)
	 continue;

      const struct gl_program *const prog = shProg->_LinkedShaders[i]->Program;

      printf("Program %d %s shader parameters:\n",
             shProg->Name, _mesa_shader_stage_to_string(i));
      for (unsigned j = 0; j < prog->Parameters->NumParameters; j++) {
         unsigned pvo = prog->Parameters->ParameterValueOffset[j];
         printf("%s: %u %p %f %f %f %f\n",
		prog->Parameters->Parameters[j].Name,
                pvo,
                prog->Parameters->ParameterValues + pvo,
                prog->Parameters->ParameterValues[pvo].f,
                prog->Parameters->ParameterValues[pvo + 1].f,
                prog->Parameters->ParameterValues[pvo + 2].f,
                prog->Parameters->ParameterValues[pvo + 3].f);
      }
   }
   fflush(stdout);
}
#endif

/**
 * Propagate some values from uniform backing storage to driver storage
 *
 * Values propagated from uniform backing storage to driver storage
 * have all format / type conversions previously requested by the
 * driver applied.  This function is most often called by the
 * implementations of \c glUniform1f, etc. and \c glUniformMatrix2f,
 * etc.
 *
 * \param uni          Uniform whose data is to be propagated to driver storage
 * \param array_index  If \c uni is an array, this is the element of
 *                     the array to be propagated.
 * \param count        Number of array elements to propagate.
 */
extern "C" void
_mesa_propagate_uniforms_to_driver_storage(struct gl_uniform_storage *uni,
					   unsigned array_index,
					   unsigned count)
{
   unsigned i;

   const unsigned components = uni->type->vector_elements;
   const unsigned vectors = uni->type->matrix_columns;
   const int dmul = glsl_type_is_64bit(uni->type) ? 2 : 1;

   /* Store the data in the driver's requested type in the driver's storage
    * areas.
    */
   unsigned src_vector_byte_stride = components * 4 * dmul;

   for (i = 0; i < uni->num_driver_storage; i++) {
      struct gl_uniform_driver_storage *const store = &uni->driver_storage[i];
      uint8_t *dst = (uint8_t *) store->data;
      const unsigned extra_stride =
	 store->element_stride - (vectors * store->vector_stride);
      const uint8_t *src =
	 (uint8_t *) (&uni->storage[array_index * (dmul * components * vectors)].i);

#if 0
      printf("%s: %p[%d] components=%u vectors=%u count=%u vector_stride=%u "
	     "extra_stride=%u\n",
	     __func__, dst, array_index, components,
	     vectors, count, store->vector_stride, extra_stride);
#endif

      dst += array_index * store->element_stride;

      switch (store->format) {
      case uniform_native: {
	 unsigned j;
	 unsigned v;

	 if (src_vector_byte_stride == store->vector_stride) {
	    if (extra_stride) {
	       for (j = 0; j < count; j++) {
	          memcpy(dst, src, src_vector_byte_stride * vectors);
	          src += src_vector_byte_stride * vectors;
	          dst += store->vector_stride * vectors;

	          dst += extra_stride;
	       }
	    } else {
	       /* Unigine Heaven benchmark gets here */
	       memcpy(dst, src, src_vector_byte_stride * vectors * count);
	       src += src_vector_byte_stride * vectors * count;
	       dst += store->vector_stride * vectors * count;
	    }
	 } else {
	    for (j = 0; j < count; j++) {
	       for (v = 0; v < vectors; v++) {
	          memcpy(dst, src, src_vector_byte_stride);
	          src += src_vector_byte_stride;
	          dst += store->vector_stride;
	       }

	       dst += extra_stride;
	    }
	 }
	 break;
      }

      case uniform_int_float: {
	 const int *isrc = (const int *) src;
	 unsigned j;
	 unsigned v;
	 unsigned c;

	 for (j = 0; j < count; j++) {
	    for (v = 0; v < vectors; v++) {
	       for (c = 0; c < components; c++) {
		  ((float *) dst)[c] = (float) *isrc;
		  isrc++;
	       }

	       dst += store->vector_stride;
	    }

	    dst += extra_stride;
	 }
	 break;
      }

      default:
	 assert(!"Should not get here.");
	 break;
      }
   }
}


static void
associate_uniform_storage(struct gl_context *ctx,
                          struct gl_shader_program *shader_program,
                          struct gl_program *prog)
{
   struct gl_program_parameter_list *params = prog->Parameters;
   gl_shader_stage shader_type = prog->info.stage;

   _mesa_disallow_parameter_storage_realloc(params);

   /* After adding each uniform to the parameter list, connect the storage for
    * the parameter with the tracking structure used by the API for the
    * uniform.
    */
   unsigned last_location = unsigned(~0);
   for (unsigned i = 0; i < params->NumParameters; i++) {
      if (params->Parameters[i].Type != PROGRAM_UNIFORM)
         continue;

      unsigned location = params->Parameters[i].UniformStorageIndex;

      struct gl_uniform_storage *storage =
         &shader_program->data->UniformStorage[location];

      /* Do not associate any uniform storage to built-in uniforms */
      if (storage->builtin)
         continue;

      if (location != last_location) {
         enum gl_uniform_driver_format format = uniform_native;
         unsigned columns = 0;

         int dmul;
         if (ctx->Const.PackedDriverUniformStorage && !prog->info.use_legacy_math_rules) {
            dmul = storage->type->vector_elements * sizeof(float);
         } else {
            dmul = 4 * sizeof(float);
         }

         switch (storage->type->base_type) {
         case GLSL_TYPE_UINT64:
            if (storage->type->vector_elements > 2)
               dmul *= 2;
            FALLTHROUGH;
         case GLSL_TYPE_UINT:
         case GLSL_TYPE_UINT16:
         case GLSL_TYPE_UINT8:
            assert(ctx->Const.NativeIntegers);
            format = uniform_native;
            columns = 1;
            break;
         case GLSL_TYPE_INT64:
            if (storage->type->vector_elements > 2)
               dmul *= 2;
            FALLTHROUGH;
         case GLSL_TYPE_INT:
         case GLSL_TYPE_INT16:
         case GLSL_TYPE_INT8:
            format =
               (ctx->Const.NativeIntegers) ? uniform_native : uniform_int_float;
            columns = 1;
            break;
         case GLSL_TYPE_DOUBLE:
            if (storage->type->vector_elements > 2)
               dmul *= 2;
            FALLTHROUGH;
         case GLSL_TYPE_FLOAT:
         case GLSL_TYPE_FLOAT16:
            format = uniform_native;
            columns = storage->type->matrix_columns;
            break;
         case GLSL_TYPE_BOOL:
            format = uniform_native;
            columns = 1;
            break;
         case GLSL_TYPE_SAMPLER:
         case GLSL_TYPE_TEXTURE:
         case GLSL_TYPE_IMAGE:
         case GLSL_TYPE_SUBROUTINE:
            format = uniform_native;
            columns = 1;
            break;
         case GLSL_TYPE_ATOMIC_UINT:
         case GLSL_TYPE_ARRAY:
         case GLSL_TYPE_VOID:
         case GLSL_TYPE_STRUCT:
         case GLSL_TYPE_ERROR:
         case GLSL_TYPE_INTERFACE:
         case GLSL_TYPE_COOPERATIVE_MATRIX:
            assert(!"Should not get here.");
            break;
         }

         unsigned pvo = params->Parameters[i].ValueOffset;
         _mesa_uniform_attach_driver_storage(storage, dmul * columns, dmul,
                                             format,
                                             &params->ParameterValues[pvo]);

         /* When a bindless sampler/image is bound to a texture/image unit, we
          * have to overwrite the constant value by the resident handle
          * directly in the constant buffer before the next draw. One solution
          * is to keep track a pointer to the base of the data.
          */
         if (storage->is_bindless && (prog->sh.NumBindlessSamplers ||
                                      prog->sh.NumBindlessImages)) {
            unsigned array_elements = MAX2(1, storage->array_elements);

            for (unsigned j = 0; j < array_elements; ++j) {
               unsigned unit = storage->opaque[shader_type].index + j;

               if (glsl_type_is_sampler(glsl_without_array(storage->type))) {
                  assert(unit >= 0 && unit < prog->sh.NumBindlessSamplers);
                  prog->sh.BindlessSamplers[unit].data =
                     &params->ParameterValues[pvo] + 4 * j;
               } else if (glsl_type_is_image(glsl_without_array(storage->type))) {
                  assert(unit >= 0 && unit < prog->sh.NumBindlessImages);
                  prog->sh.BindlessImages[unit].data =
                     &params->ParameterValues[pvo] + 4 * j;
               }
            }
         }

         /* After attaching the driver's storage to the uniform, propagate any
          * data from the linker's backing store.  This will cause values from
          * initializers in the source code to be copied over.
          */
         unsigned array_elements = MAX2(1, storage->array_elements);
         if (ctx->Const.PackedDriverUniformStorage && !prog->info.use_legacy_math_rules &&
             (storage->is_bindless || !glsl_contains_opaque(storage->type))) {
            const int dmul = glsl_type_is_64bit(storage->type) ? 2 : 1;
            const unsigned components =
               storage->type->vector_elements *
               storage->type->matrix_columns;

            for (unsigned s = 0; s < storage->num_driver_storage; s++) {
               gl_constant_value *uni_storage = (gl_constant_value *)
                  storage->driver_storage[s].data;
               memcpy(uni_storage, storage->storage,
                      sizeof(storage->storage[0]) * components *
                      array_elements * dmul);
            }
         } else {
            _mesa_propagate_uniforms_to_driver_storage(storage, 0,
                                                       array_elements);
         }

	      last_location = location;
      }
   }
}


void
_mesa_ensure_and_associate_uniform_storage(struct gl_context *ctx,
                              struct gl_shader_program *shader_program,
                              struct gl_program *prog, unsigned required_space)
{
   /* Avoid reallocation of the program parameter list, because the uniform
    * storage is only associated with the original parameter list.
    */
   _mesa_reserve_parameter_storage(prog->Parameters, required_space,
                                   required_space);

   /* This has to be done last.  Any operation the can cause
    * prog->ParameterValues to get reallocated (e.g., anything that adds a
    * program constant) has to happen before creating this linkage.
    */
   associate_uniform_storage(ctx, shader_program, prog);
}


/**
 * Return printable string for a given GLSL_TYPE_x
 */
static const char *
glsl_type_name(enum glsl_base_type type)
{
   switch (type) {
   case GLSL_TYPE_UINT:
      return "uint";
   case GLSL_TYPE_INT:
      return "int";
   case GLSL_TYPE_FLOAT:
      return "float";
   case GLSL_TYPE_DOUBLE:
      return "double";
   case GLSL_TYPE_UINT64:
      return "uint64";
   case GLSL_TYPE_INT64:
      return "int64";
   case GLSL_TYPE_BOOL:
      return "bool";
   case GLSL_TYPE_SAMPLER:
      return "sampler";
   case GLSL_TYPE_IMAGE:
      return "image";
   case GLSL_TYPE_ATOMIC_UINT:
      return "atomic_uint";
   case GLSL_TYPE_STRUCT:
      return "struct";
   case GLSL_TYPE_INTERFACE:
      return "interface";
   case GLSL_TYPE_ARRAY:
      return "array";
   case GLSL_TYPE_VOID:
      return "void";
   case GLSL_TYPE_ERROR:
      return "error";
   default:
      return "other";
   }
}


static struct gl_uniform_storage *
validate_uniform(GLint location, GLsizei count, const GLvoid *values,
                 unsigned *offset, struct gl_context *ctx,
                 struct gl_shader_program *shProg,
                 enum glsl_base_type basicType, unsigned src_components)
{
   struct gl_uniform_storage *const uni =
      validate_uniform_parameters(location, count, offset,
                                  ctx, shProg, "glUniform");
   if (uni == NULL)
      return NULL;

   if (glsl_type_is_matrix(uni->type)) {
      /* Can't set matrix uniforms (like mat4) with glUniform */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glUniform%u(uniform \"%s\"@%d is matrix)",
                  src_components, uni->name.string, location);
      return NULL;
   }

   /* Verify that the types are compatible. */
   const unsigned components = uni->type->vector_elements;

   if (components != src_components) {
      /* glUniformN() must match float/vecN type */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glUniform%u(\"%s\"@%u has %u components, not %u)",
                  src_components, uni->name.string, location,
                  components, src_components);
      return NULL;
   }

   bool match;
   switch (uni->type->base_type) {
   case GLSL_TYPE_BOOL:
      match = (basicType != GLSL_TYPE_DOUBLE);
      break;
   case GLSL_TYPE_SAMPLER:
      match = (basicType == GLSL_TYPE_INT);
      break;
   case GLSL_TYPE_IMAGE:
      match = (basicType == GLSL_TYPE_INT && _mesa_is_desktop_gl(ctx));
      break;
   case GLSL_TYPE_FLOAT16:
      match = basicType == GLSL_TYPE_FLOAT;
      break;
   default:
      match = (basicType == uni->type->base_type);
      break;
   }

   if (!match) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glUniform%u(\"%s\"@%d is %s, not %s)",
                  src_components, uni->name.string, location,
                  glsl_type_name(uni->type->base_type),
                  glsl_type_name(basicType));
      return NULL;
   }

   if (unlikely(ctx->_Shader->Flags & GLSL_UNIFORMS)) {
      log_uniform(values, basicType, components, 1, count,
                  false, shProg, location, uni);
   }

   /* Page 100 (page 116 of the PDF) of the OpenGL 3.0 spec says:
    *
    *     "Setting a sampler's value to i selects texture image unit number
    *     i. The values of i range from zero to the implementation- dependent
    *     maximum supported number of texture image units."
    *
    * In addition, table 2.3, "Summary of GL errors," on page 17 (page 33 of
    * the PDF) says:
    *
    *     "Error         Description                    Offending command
    *                                                   ignored?
    *     ...
    *     INVALID_VALUE  Numeric argument out of range  Yes"
    *
    * Based on that, when an invalid sampler is specified, we generate a
    * GL_INVALID_VALUE error and ignore the command.
    */
   if (glsl_type_is_sampler(uni->type)) {
      for (int i = 0; i < count; i++) {
         const unsigned texUnit = ((unsigned *) values)[i];

         /* check that the sampler (tex unit index) is legal */
         if (texUnit >= ctx->Const.MaxCombinedTextureImageUnits) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glUniform1i(invalid sampler/tex unit index for "
                        "uniform %d)", location);
            return NULL;
         }
      }
      /* We need to reset the validate flag on changes to samplers in case
       * two different sampler types are set to the same texture unit.
       */
      ctx->_Shader->Validated = ctx->_Shader->UserValidated = GL_FALSE;
   }

   if (glsl_type_is_image(uni->type)) {
      for (int i = 0; i < count; i++) {
         const int unit = ((GLint *) values)[i];

         /* check that the image unit is legal */
         if (unit < 0 || unit >= (int)ctx->Const.MaxImageUnits) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glUniform1i(invalid image unit index for uniform %d)",
                        location);
            return NULL;
         }
      }
   }

   return uni;
}

void
_mesa_flush_vertices_for_uniforms(struct gl_context *ctx,
                                  const struct gl_uniform_storage *uni)
{
   /* Opaque uniforms have no storage unless they are bindless */
   if (!uni->is_bindless && glsl_contains_opaque(uni->type)) {
      /* Samplers flush on demand and ignore redundant updates. */
      if (!glsl_type_is_sampler(uni->type))
         FLUSH_VERTICES(ctx, 0, 0);
      return;
   }

   uint64_t new_driver_state = 0;
   unsigned mask = uni->active_shader_mask;

   while (mask) {
      unsigned index = u_bit_scan(&mask);

      assert(index < MESA_SHADER_STAGES);
      new_driver_state |= ctx->DriverFlags.NewShaderConstants[index];
   }

   FLUSH_VERTICES(ctx, new_driver_state ? 0 : _NEW_PROGRAM_CONSTANTS, 0);
   ctx->NewDriverState |= new_driver_state;
}

static bool
copy_uniforms_to_storage(gl_constant_value *storage,
                         struct gl_uniform_storage *uni,
                         struct gl_context *ctx, GLsizei count,
                         const GLvoid *values, const int size_mul,
                         const unsigned offset, const unsigned components,
                         enum glsl_base_type basicType, bool flush)
{
   const gl_constant_value *src = (const gl_constant_value*)values;
   bool copy_as_uint64 = uni->is_bindless &&
                         (glsl_type_is_sampler(uni->type) || glsl_type_is_image(uni->type));
   bool copy_to_float16 = uni->type->base_type == GLSL_TYPE_FLOAT16;

   if (!glsl_type_is_boolean(uni->type) && !copy_as_uint64 && !copy_to_float16) {
      unsigned size = sizeof(storage[0]) * components * count * size_mul;

      if (!memcmp(storage, values, size))
         return false;

      if (flush)
         _mesa_flush_vertices_for_uniforms(ctx, uni);

      memcpy(storage, values, size);
      return true;
   } else if (copy_to_float16) {
      assert(ctx->Const.PackedDriverUniformStorage);
      const unsigned dst_components = align(components, 2);
      uint16_t *dst = (uint16_t*)storage;

      int i = 0;
      unsigned c = 0;

      if (flush) {
         /* Find the first element that's different. */
         for (; i < count; i++) {
            for (; c < components; c++) {
               if (dst[c] != _mesa_float_to_half(src[c].f)) {
                  _mesa_flush_vertices_for_uniforms(ctx, uni);
                  flush = false;
                  goto break_loops;
               }
            }
            c = 0;
            dst += dst_components;
            src += components;
         }
      break_loops:
         if (flush)
            return false; /* No change. */
      }

      /* Set the remaining elements. We know that at least 1 element is
       * different and that we have flushed.
       */
      for (; i < count; i++) {
         for (; c < components; c++)
            dst[c] = _mesa_float_to_half(src[c].f);

         c = 0;
         dst += dst_components;
         src += components;
      }

      return true;
   } else if (copy_as_uint64) {
      const unsigned elems = components * count;
      uint64_t *dst = (uint64_t*)storage;
      unsigned i = 0;

      if (flush) {
         /* Find the first element that's different. */
         for (; i < elems; i++) {
            if (dst[i] != src[i].u) {
               _mesa_flush_vertices_for_uniforms(ctx, uni);
               flush = false;
               break;
            }
         }
         if (flush)
            return false; /* No change. */
      }

      /* Set the remaining elements. We know that at least 1 element is
       * different and that we have flushed.
       */
      for (; i < elems; i++)
         dst[i] = src[i].u;

      return true;
   } else {
      const unsigned elems = components * count;
      gl_constant_value *dst = storage;

      if (basicType == GLSL_TYPE_FLOAT) {
         unsigned i = 0;

         if (flush) {
            /* Find the first element that's different. */
            for (; i < elems; i++) {
               if (dst[i].u !=
                   (src[i].f != 0.0f ? ctx->Const.UniformBooleanTrue : 0)) {
                  _mesa_flush_vertices_for_uniforms(ctx, uni);
                  flush = false;
                  break;
               }
            }
            if (flush)
               return false; /* No change. */
         }

         /* Set the remaining elements. We know that at least 1 element is
          * different and that we have flushed.
          */
         for (; i < elems; i++)
            dst[i].u = src[i].f != 0.0f ? ctx->Const.UniformBooleanTrue : 0;

         return true;
      } else {
         unsigned i = 0;

         if (flush) {
            /* Find the first element that's different. */
            for (; i < elems; i++) {
               if (dst[i].u !=
                   (src[i].u ? ctx->Const.UniformBooleanTrue : 0)) {
                  _mesa_flush_vertices_for_uniforms(ctx, uni);
                  flush = false;
                  break;
               }
            }
            if (flush)
               return false; /* No change. */
         }

         /* Set the remaining elements. We know that at least 1 element is
          * different and that we have flushed.
          */
         for (; i < elems; i++)
            dst[i].u = src[i].u ? ctx->Const.UniformBooleanTrue : 0;

         return true;
      }
   }
}


/**
 * Called via glUniform*() functions.
 */
extern "C" void
_mesa_uniform(GLint location, GLsizei count, const GLvoid *values,
              struct gl_context *ctx, struct gl_shader_program *shProg,
              enum glsl_base_type basicType, unsigned src_components)
{
   unsigned offset;
   int size_mul = glsl_base_type_is_64bit(basicType) ? 2 : 1;

   struct gl_uniform_storage *uni;
   if (_mesa_is_no_error_enabled(ctx)) {
      /* From Seciton 7.6 (UNIFORM VARIABLES) of the OpenGL 4.5 spec:
       *
       *   "If the value of location is -1, the Uniform* commands will
       *   silently ignore the data passed in, and the current uniform values
       *   will not be changed.
       */
      if (location == -1)
         return;

      if (location >= (int)shProg->NumUniformRemapTable)
         return;

      uni = shProg->UniformRemapTable[location];
      if (!uni || uni == INACTIVE_UNIFORM_EXPLICIT_LOCATION)
         return;

      /* The array index specified by the uniform location is just the
       * uniform location minus the base location of of the uniform.
       */
      assert(uni->array_elements > 0 || location == (int)uni->remap_location);
      offset = location - uni->remap_location;
   } else {
      uni = validate_uniform(location, count, values, &offset, ctx, shProg,
                             basicType, src_components);
      if (!uni)
         return;
   }

   const unsigned components = uni->type->vector_elements;

   /* Page 82 (page 96 of the PDF) of the OpenGL 2.1 spec says:
    *
    *     "When loading N elements starting at an arbitrary position k in a
    *     uniform declared as an array, elements k through k + N - 1 in the
    *     array will be replaced with the new values. Values for any array
    *     element that exceeds the highest array element index used, as
    *     reported by GetActiveUniform, will be ignored by the GL."
    *
    * Clamp 'count' to a valid value.  Note that for non-arrays a count > 1
    * will have already generated an error.
    */
   if (uni->array_elements != 0) {
      count = MIN2(count, (int) (uni->array_elements - offset));
   }

   /* Store the data in the "actual type" backing storage for the uniform.
    */
   bool ctx_flushed = false;
   gl_constant_value *storage;
   if (ctx->Const.PackedDriverUniformStorage &&
       (uni->is_bindless || !glsl_contains_opaque(uni->type))) {
      for (unsigned s = 0; s < uni->num_driver_storage; s++) {
         unsigned dword_components = components;

         /* 16-bit uniforms are packed. */
         if (glsl_base_type_is_16bit(uni->type->base_type))
            dword_components = DIV_ROUND_UP(dword_components, 2);

         storage = (gl_constant_value *)
            uni->driver_storage[s].data + (size_mul * offset * dword_components);

         if (copy_uniforms_to_storage(storage, uni, ctx, count, values, size_mul,
                                      offset, components, basicType, !ctx_flushed))
            ctx_flushed = true;
      }
   } else {
      storage = &uni->storage[size_mul * components * offset];
      if (copy_uniforms_to_storage(storage, uni, ctx, count, values, size_mul,
                                   offset, components, basicType, !ctx_flushed)) {
         _mesa_propagate_uniforms_to_driver_storage(uni, offset, count);
         ctx_flushed = true;
      }
   }
   /* Return early if possible. Bindless samplers need to be processed
    * because of the !sampler->bound codepath below.
    */
   if (!ctx_flushed && !(glsl_type_is_sampler(uni->type) && uni->is_bindless))
      return; /* no change in uniform values */

   /* If the uniform is a sampler, do the extra magic necessary to propagate
    * the changes through.
    */
   if (glsl_type_is_sampler(uni->type)) {
      /* Note that samplers are the only uniforms that don't call
       * FLUSH_VERTICES above.
       */
      bool flushed = false;
      bool any_changed = false;
      bool samplers_validated = shProg->SamplersValidated;

      shProg->SamplersValidated = GL_TRUE;

      for (int i = 0; i < MESA_SHADER_STAGES; i++) {
         struct gl_linked_shader *const sh = shProg->_LinkedShaders[i];

         /* If the shader stage doesn't use the sampler uniform, skip this. */
         if (!uni->opaque[i].active)
            continue;

         bool changed = false;
         for (int j = 0; j < count; j++) {
            unsigned unit = uni->opaque[i].index + offset + j;
            unsigned value = ((unsigned *)values)[j];

            if (uni->is_bindless) {
               struct gl_bindless_sampler *sampler =
                  &sh->Program->sh.BindlessSamplers[unit];

               /* Mark this bindless sampler as bound to a texture unit.
                */
               if (sampler->unit != value || !sampler->bound) {
                  if (!flushed) {
                     FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, 0);
                     flushed = true;
                  }
                  sampler->unit = value;
                  changed = true;
               }
               sampler->bound = true;
               sh->Program->sh.HasBoundBindlessSampler = true;
            } else {
               if (sh->Program->SamplerUnits[unit] != value) {
                  if (!flushed) {
                     FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, 0);
                     flushed = true;
                  }
                  sh->Program->SamplerUnits[unit] = value;
                  changed = true;
               }
            }
         }

         if (changed) {
            struct gl_program *const prog = sh->Program;
            _mesa_update_shader_textures_used(shProg, prog);
            any_changed = true;
         }
      }

      if (any_changed)
         _mesa_update_valid_to_render_state(ctx);
      else
         shProg->SamplersValidated = samplers_validated;
   }

   /* If the uniform is an image, update the mapping from image
    * uniforms to image units present in the shader data structure.
    */
   if (glsl_type_is_image(uni->type)) {
      for (int i = 0; i < MESA_SHADER_STAGES; i++) {
         struct gl_linked_shader *sh = shProg->_LinkedShaders[i];

         /* If the shader stage doesn't use the image uniform, skip this. */
         if (!uni->opaque[i].active)
            continue;

         for (int j = 0; j < count; j++) {
            unsigned unit = uni->opaque[i].index + offset + j;
            unsigned value = ((unsigned *)values)[j];

            if (uni->is_bindless) {
               struct gl_bindless_image *image =
                  &sh->Program->sh.BindlessImages[unit];

               /* Mark this bindless image as bound to an image unit.
                */
               image->unit = value;
               image->bound = true;
               sh->Program->sh.HasBoundBindlessImage = true;
            } else {
               sh->Program->sh.ImageUnits[unit] = value;
            }
         }
      }

      ctx->NewDriverState |= ST_NEW_IMAGE_UNITS;
   }
}


static bool
copy_uniform_matrix_to_storage(struct gl_context *ctx,
                               gl_constant_value *storage,
                               struct gl_uniform_storage *const uni,
                               unsigned count, const void *values,
                               const unsigned size_mul, const unsigned offset,
                               const unsigned components,
                               const unsigned vectors, bool transpose,
                               unsigned cols, unsigned rows,
                               enum glsl_base_type basicType, bool flush)
{
   const unsigned elements = components * vectors;
   const unsigned size = sizeof(storage[0]) * elements * count * size_mul;

   if (uni->type->base_type == GLSL_TYPE_FLOAT16) {
      assert(ctx->Const.PackedDriverUniformStorage);
      const unsigned dst_components = align(components, 2);
      const unsigned dst_elements = dst_components * vectors;

      if (!transpose) {
         const float *src = (const float *)values;
         uint16_t *dst = (uint16_t*)storage;

         unsigned i = 0, r = 0, c = 0;

         if (flush) {
            /* Find the first element that's different. */
            for (; i < count; i++) {
               for (; c < cols; c++) {
                  for (; r < rows; r++) {
                     if (dst[(c * dst_components) + r] !=
                         _mesa_float_to_half(src[(c * components) + r])) {
                        _mesa_flush_vertices_for_uniforms(ctx, uni);
                        flush = false;
                        goto break_loops_16bit;
                     }
                  }
                  r = 0;
               }
               c = 0;
               dst += dst_elements;
               src += elements;
            }

         break_loops_16bit:
            if (flush)
               return false; /* No change. */
         }

         /* Set the remaining elements. We know that at least 1 element is
          * different and that we have flushed.
          */
         for (; i < count; i++) {
            for (; c < cols; c++) {
               for (; r < rows; r++) {
                  dst[(c * dst_components) + r] =
                     _mesa_float_to_half(src[(c * components) + r]);
               }
               r = 0;
            }
            c = 0;
            dst += dst_elements;
            src += elements;
         }
         return true;
      } else {
         /* Transpose the matrix. */
         const float *src = (const float *)values;
         uint16_t *dst = (uint16_t*)storage;

         unsigned i = 0, r = 0, c = 0;

         if (flush) {
            /* Find the first element that's different. */
            for (; i < count; i++) {
               for (; r < rows; r++) {
                  for (; c < cols; c++) {
                     if (dst[(c * dst_components) + r] !=
                         _mesa_float_to_half(src[c + (r * vectors)])) {
                        _mesa_flush_vertices_for_uniforms(ctx, uni);
                        flush = false;
                        goto break_loops_16bit_transpose;
                     }
                  }
                  c = 0;
               }
               r = 0;
               dst += elements;
               src += elements;
            }

         break_loops_16bit_transpose:
            if (flush)
               return false; /* No change. */
         }

         /* Set the remaining elements. We know that at least 1 element is
          * different and that we have flushed.
          */
         for (; i < count; i++) {
            for (; r < rows; r++) {
               for (; c < cols; c++) {
                  dst[(c * dst_components) + r] =
                     _mesa_float_to_half(src[c + (r * vectors)]);
               }
               c = 0;
            }
            r = 0;
            dst += elements;
            src += elements;
         }
         return true;
      }
   } else if (!transpose) {
      if (!memcmp(storage, values, size))
         return false;

      if (flush)
         _mesa_flush_vertices_for_uniforms(ctx, uni);

      memcpy(storage, values, size);
      return true;
   } else if (basicType == GLSL_TYPE_FLOAT) {
      /* Transpose the matrix. */
      const float *src = (const float *)values;
      float *dst = (float*)storage;

      unsigned i = 0, r = 0, c = 0;

      if (flush) {
         /* Find the first element that's different. */
         for (; i < count; i++) {
            for (; r < rows; r++) {
               for (; c < cols; c++) {
                  if (dst[(c * components) + r] != src[c + (r * vectors)]) {
                     _mesa_flush_vertices_for_uniforms(ctx, uni);
                     flush = false;
                     goto break_loops;
                  }
               }
               c = 0;
            }
            r = 0;
            dst += elements;
            src += elements;
         }

      break_loops:
         if (flush)
            return false; /* No change. */
      }

      /* Set the remaining elements. We know that at least 1 element is
       * different and that we have flushed.
       */
      for (; i < count; i++) {
         for (; r < rows; r++) {
            for (; c < cols; c++)
               dst[(c * components) + r] = src[c + (r * vectors)];
            c = 0;
         }
         r = 0;
         dst += elements;
         src += elements;
      }
      return true;
   } else {
      assert(basicType == GLSL_TYPE_DOUBLE);
      const double *src = (const double *)values;
      double *dst = (double*)storage;

      unsigned i = 0, r = 0, c = 0;

      if (flush) {
         /* Find the first element that's different. */
         for (; i < count; i++) {
            for (; r < rows; r++) {
               for (; c < cols; c++) {
                  if (dst[(c * components) + r] != src[c + (r * vectors)]) {
                     _mesa_flush_vertices_for_uniforms(ctx, uni);
                     flush = false;
                     goto break_loops2;
                  }
               }
               c = 0;
            }
            r = 0;
            dst += elements;
            src += elements;
         }

      break_loops2:
         if (flush)
            return false; /* No change. */
      }

      /* Set the remaining elements. We know that at least 1 element is
       * different and that we have flushed.
       */
      for (; i < count; i++) {
         for (; r < rows; r++) {
            for (; c < cols; c++)
               dst[(c * components) + r] = src[c + (r * vectors)];
            c = 0;
         }
         r = 0;
         dst += elements;
         src += elements;
      }
      return true;
   }
}


/**
 * Called by glUniformMatrix*() functions.
 * Note: cols=2, rows=4  ==>  array[2] of vec4
 */
extern "C" void
_mesa_uniform_matrix(GLint location, GLsizei count,
                     GLboolean transpose, const void *values,
                     struct gl_context *ctx, struct gl_shader_program *shProg,
                     GLuint cols, GLuint rows, enum glsl_base_type basicType)
{
   unsigned offset;
   struct gl_uniform_storage *const uni =
      validate_uniform_parameters(location, count, &offset,
                                  ctx, shProg, "glUniformMatrix");
   if (uni == NULL)
      return;

   /* GL_INVALID_VALUE is generated if `transpose' is not GL_FALSE.
    * http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUniform.xml
    */
   if (transpose) {
      if (_mesa_is_gles2(ctx) && ctx->Version < 30) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glUniformMatrix(matrix transpose is not GL_FALSE)");
         return;
      }
   }

   if (!glsl_type_is_matrix(uni->type)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
		  "glUniformMatrix(non-matrix uniform)");
      return;
   }

   assert(basicType == GLSL_TYPE_FLOAT || basicType == GLSL_TYPE_DOUBLE);
   const unsigned size_mul = basicType == GLSL_TYPE_DOUBLE ? 2 : 1;

   assert(!glsl_type_is_sampler(uni->type));
   const unsigned vectors = uni->type->matrix_columns;
   const unsigned components = uni->type->vector_elements;

   /* Verify that the types are compatible.  This is greatly simplified for
    * matrices because they can only have a float base type.
    */
   if (vectors != cols || components != rows) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
		  "glUniformMatrix(matrix size mismatch)");
      return;
   }

   /* Section 2.11.7 (Uniform Variables) of the OpenGL 4.2 Core Profile spec
    * says:
    *
    *     "If any of the following conditions occur, an INVALID_OPERATION
    *     error is generated by the Uniform* commands, and no uniform values
    *     are changed:
    *
    *     ...
    *
    *     - if the uniform declared in the shader is not of type boolean and
    *       the type indicated in the name of the Uniform* command used does
    *       not match the type of the uniform"
    *
    * There are no Boolean matrix types, so we do not need to allow
    * GLSL_TYPE_BOOL here (as _mesa_uniform does).
    */
   if (uni->type->base_type != basicType &&
       !(uni->type->base_type == GLSL_TYPE_FLOAT16 &&
         basicType == GLSL_TYPE_FLOAT)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glUniformMatrix%ux%u(\"%s\"@%d is %s, not %s)",
                  cols, rows, uni->name.string, location,
                  glsl_type_name(uni->type->base_type),
                  glsl_type_name(basicType));
      return;
   }

   if (unlikely(ctx->_Shader->Flags & GLSL_UNIFORMS)) {
      log_uniform(values, uni->type->base_type, components, vectors, count,
		  bool(transpose), shProg, location, uni);
   }

   /* Page 82 (page 96 of the PDF) of the OpenGL 2.1 spec says:
    *
    *     "When loading N elements starting at an arbitrary position k in a
    *     uniform declared as an array, elements k through k + N - 1 in the
    *     array will be replaced with the new values. Values for any array
    *     element that exceeds the highest array element index used, as
    *     reported by GetActiveUniform, will be ignored by the GL."
    *
    * Clamp 'count' to a valid value.  Note that for non-arrays a count > 1
    * will have already generated an error.
    */
   if (uni->array_elements != 0) {
      count = MIN2(count, (int) (uni->array_elements - offset));
   }

   /* Store the data in the "actual type" backing storage for the uniform.
    */
   gl_constant_value *storage;
   const unsigned elements = components * vectors;
   if (ctx->Const.PackedDriverUniformStorage) {
      bool flushed = false;

      for (unsigned s = 0; s < uni->num_driver_storage; s++) {
         unsigned dword_components = components;

         /* 16-bit uniforms are packed. */
         if (glsl_base_type_is_16bit(uni->type->base_type))
            dword_components = DIV_ROUND_UP(dword_components, 2);

         storage = (gl_constant_value *)
            uni->driver_storage[s].data +
            (size_mul * offset * dword_components * vectors);

         if (copy_uniform_matrix_to_storage(ctx, storage, uni, count, values,
                                            size_mul, offset, components,
                                            vectors, transpose, cols, rows,
                                            basicType, !flushed))
            flushed = true;
      }
   } else {
      storage =  &uni->storage[size_mul * elements * offset];
      if (copy_uniform_matrix_to_storage(ctx, storage, uni, count, values,
                                         size_mul, offset, components, vectors,
                                         transpose, cols, rows, basicType,
                                         true))
         _mesa_propagate_uniforms_to_driver_storage(uni, offset, count);
   }
}

static void
update_bound_bindless_sampler_flag(struct gl_program *prog)
{
   unsigned i;

   if (likely(!prog->sh.HasBoundBindlessSampler))
      return;

   for (i = 0; i < prog->sh.NumBindlessSamplers; i++) {
      struct gl_bindless_sampler *sampler = &prog->sh.BindlessSamplers[i];

      if (sampler->bound)
         return;
   }
   prog->sh.HasBoundBindlessSampler = false;
}

static void
update_bound_bindless_image_flag(struct gl_program *prog)
{
   unsigned i;

   if (likely(!prog->sh.HasBoundBindlessImage))
      return;

   for (i = 0; i < prog->sh.NumBindlessImages; i++) {
      struct gl_bindless_image *image = &prog->sh.BindlessImages[i];

      if (image->bound)
         return;
   }
   prog->sh.HasBoundBindlessImage = false;
}

/**
 * Called via glUniformHandleui64*ARB() functions.
 */
extern "C" void
_mesa_uniform_handle(GLint location, GLsizei count, const GLvoid *values,
                     struct gl_context *ctx, struct gl_shader_program *shProg)
{
   unsigned offset;
   struct gl_uniform_storage *uni;

   if (_mesa_is_no_error_enabled(ctx)) {
      /* From Section 7.6 (UNIFORM VARIABLES) of the OpenGL 4.5 spec:
       *
       *   "If the value of location is -1, the Uniform* commands will
       *   silently ignore the data passed in, and the current uniform values
       *   will not be changed.
       */
      if (location == -1)
         return;

      uni = shProg->UniformRemapTable[location];
      if (!uni || uni == INACTIVE_UNIFORM_EXPLICIT_LOCATION)
         return;

      /* The array index specified by the uniform location is just the
       * uniform location minus the base location of of the uniform.
       */
      assert(uni->array_elements > 0 || location == (int)uni->remap_location);
      offset = location - uni->remap_location;
   } else {
      uni = validate_uniform_parameters(location, count, &offset,
                                        ctx, shProg, "glUniformHandleui64*ARB");
      if (!uni)
         return;

      if (!uni->is_bindless) {
         /* From section "Errors" of the ARB_bindless_texture spec:
          *
          * "The error INVALID_OPERATION is generated by
          *  UniformHandleui64{v}ARB if the sampler or image uniform being
          *  updated has the "bound_sampler" or "bound_image" layout qualifier."
          *
          * From section 4.4.6 of the ARB_bindless_texture spec:
          *
          * "In the absence of these qualifiers, sampler and image uniforms are
          *  considered "bound". Additionally, if GL_ARB_bindless_texture is
          *  not enabled, these uniforms are considered "bound"."
          */
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glUniformHandleui64*ARB(non-bindless sampler/image uniform)");
         return;
      }
   }

   const unsigned components = uni->type->vector_elements;
   const int size_mul = 2;

   if (unlikely(ctx->_Shader->Flags & GLSL_UNIFORMS)) {
      log_uniform(values, GLSL_TYPE_UINT64, components, 1, count,
                  false, shProg, location, uni);
   }

   /* Page 82 (page 96 of the PDF) of the OpenGL 2.1 spec says:
    *
    *     "When loading N elements starting at an arbitrary position k in a
    *     uniform declared as an array, elements k through k + N - 1 in the
    *     array will be replaced with the new values. Values for any array
    *     element that exceeds the highest array element index used, as
    *     reported by GetActiveUniform, will be ignored by the GL."
    *
    * Clamp 'count' to a valid value.  Note that for non-arrays a count > 1
    * will have already generated an error.
    */
   if (uni->array_elements != 0) {
      count = MIN2(count, (int) (uni->array_elements - offset));
   }


   /* Store the data in the "actual type" backing storage for the uniform.
    */
   if (ctx->Const.PackedDriverUniformStorage) {
      bool flushed = false;

      for (unsigned s = 0; s < uni->num_driver_storage; s++) {
         void *storage = (gl_constant_value *)
            uni->driver_storage[s].data + (size_mul * offset * components);
         unsigned size = sizeof(uni->storage[0]) * components * count * size_mul;

         if (!memcmp(storage, values, size))
            continue;

         if (!flushed) {
            _mesa_flush_vertices_for_uniforms(ctx, uni);
            flushed = true;
         }
         memcpy(storage, values, size);
      }
      if (!flushed)
         return;
   } else {
      void *storage = &uni->storage[size_mul * components * offset];
      unsigned size = sizeof(uni->storage[0]) * components * count * size_mul;

      if (!memcmp(storage, values, size))
         return;

      _mesa_flush_vertices_for_uniforms(ctx, uni);
      memcpy(storage, values, size);
      _mesa_propagate_uniforms_to_driver_storage(uni, offset, count);
   }

   if (glsl_type_is_sampler(uni->type)) {
      /* Mark this bindless sampler as not bound to a texture unit because
       * it refers to a texture handle.
       */
      for (int i = 0; i < MESA_SHADER_STAGES; i++) {
         struct gl_linked_shader *const sh = shProg->_LinkedShaders[i];

         /* If the shader stage doesn't use the sampler uniform, skip this. */
         if (!uni->opaque[i].active)
            continue;

         for (int j = 0; j < count; j++) {
            unsigned unit = uni->opaque[i].index + offset + j;
            struct gl_bindless_sampler *sampler =
               &sh->Program->sh.BindlessSamplers[unit];

            sampler->bound = false;
         }

         update_bound_bindless_sampler_flag(sh->Program);
      }
   }

   if (glsl_type_is_image(uni->type)) {
      /* Mark this bindless image as not bound to an image unit because it
       * refers to a texture handle.
       */
      for (int i = 0; i < MESA_SHADER_STAGES; i++) {
         struct gl_linked_shader *sh = shProg->_LinkedShaders[i];

         /* If the shader stage doesn't use the sampler uniform, skip this. */
         if (!uni->opaque[i].active)
            continue;

         for (int j = 0; j < count; j++) {
            unsigned unit = uni->opaque[i].index + offset + j;
            struct gl_bindless_image *image =
               &sh->Program->sh.BindlessImages[unit];

            image->bound = false;
         }

         update_bound_bindless_image_flag(sh->Program);
      }
   }
}

extern "C" bool
_mesa_sampler_uniforms_are_valid(const struct gl_shader_program *shProg,
				 char *errMsg, size_t errMsgLength)
{
   /* Shader does not have samplers. */
   if (shProg->data->NumUniformStorage == 0)
      return true;

   if (!shProg->SamplersValidated) {
      snprintf(errMsg, errMsgLength,
                     "active samplers with a different type "
                     "refer to the same texture image unit");
      return false;
   }
   return true;
}

extern "C" bool
_mesa_sampler_uniforms_pipeline_are_valid(struct gl_pipeline_object *pipeline)
{
   /* Section 2.11.11 (Shader Execution), subheading "Validation," of the
    * OpenGL 4.1 spec says:
    *
    *     "[INVALID_OPERATION] is generated by any command that transfers
    *     vertices to the GL if:
    *
    *         ...
    *
    *         - Any two active samplers in the current program object are of
    *           different types, but refer to the same texture image unit.
    *
    *         - The number of active samplers in the program exceeds the
    *           maximum number of texture image units allowed."
    */

   GLbitfield mask;
   GLbitfield TexturesUsed[MAX_COMBINED_TEXTURE_IMAGE_UNITS];
   unsigned active_samplers = 0;
   const struct gl_program **prog =
      (const struct gl_program **) pipeline->CurrentProgram;


   memset(TexturesUsed, 0, sizeof(TexturesUsed));

   for (unsigned idx = 0; idx < ARRAY_SIZE(pipeline->CurrentProgram); idx++) {
      if (!prog[idx])
         continue;

      mask = prog[idx]->SamplersUsed;
      while (mask) {
         const int s = u_bit_scan(&mask);
         GLuint unit = prog[idx]->SamplerUnits[s];
         GLuint tgt = prog[idx]->sh.SamplerTargets[s];

         /* FIXME: Samplers are initialized to 0 and Mesa doesn't do a
          * great job of eliminating unused uniforms currently so for now
          * don't throw an error if two sampler types both point to 0.
          */
         if (unit == 0)
            continue;

         if (TexturesUsed[unit] & ~(1 << tgt)) {
            pipeline->InfoLog =
               ralloc_asprintf(pipeline,
                     "Program %d: "
                     "Texture unit %d is accessed with 2 different types",
                     prog[idx]->Id, unit);
            return false;
         }

         TexturesUsed[unit] |= (1 << tgt);
      }

      active_samplers += prog[idx]->info.num_textures;
   }

   if (active_samplers > MAX_COMBINED_TEXTURE_IMAGE_UNITS) {
      pipeline->InfoLog =
         ralloc_asprintf(pipeline,
                         "the number of active samplers %d exceed the "
                         "maximum %d",
                         active_samplers, MAX_COMBINED_TEXTURE_IMAGE_UNITS);
      return false;
   }

   return true;
}
