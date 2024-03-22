/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2004-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009-2010  VMware, Inc.  All Rights Reserved.
 * Copyright © 2010 Intel Corporation
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

/**
 * \file uniforms.c
 * Functions related to GLSL uniform variables.
 * \author Brian Paul
 */

/**
 * XXX things to do:
 * 1. Check that the right error code is generated for all _mesa_error() calls.
 * 2. Insert FLUSH_VERTICES calls in various places
 */

#include "util/glheader.h"
#include "main/context.h"
#include "main/shaderapi.h"
#include "main/shaderobj.h"
#include "main/uniforms.h"
#include "main/enums.h"
#include "compiler/glsl/ir_uniform.h"
#include "compiler/glsl_types.h"
#include "program/program.h"
#include "util/bitscan.h"
#include "api_exec_decl.h"

#include "state_tracker/st_context.h"

/**
 * Update the vertex/fragment program's TexturesUsed array.
 *
 * This needs to be called after glUniform(set sampler var) is called.
 * A call to glUniform(samplerVar, value) causes a sampler to point to a
 * particular texture unit.  We know the sampler's texture target
 * (1D/2D/3D/etc) from compile time but the sampler's texture unit is
 * set by glUniform() calls.
 *
 * So, scan the program->SamplerUnits[] and program->SamplerTargets[]
 * information to update the prog->TexturesUsed[] values.
 * Each value of TexturesUsed[unit] is one of zero, TEXTURE_1D_INDEX,
 * TEXTURE_2D_INDEX, TEXTURE_3D_INDEX, etc.
 * We'll use that info for state validation before rendering.
 */
static inline void
update_single_shader_texture_used(struct gl_shader_program *shProg,
                                  struct gl_program *prog,
                                  GLuint unit, GLuint target)
{
   gl_shader_stage prog_stage =
      _mesa_program_enum_to_shader_stage(prog->Target);

   assert(unit < ARRAY_SIZE(prog->TexturesUsed));
   assert(target < NUM_TEXTURE_TARGETS);

   /* From section 7.10 (Samplers) of the OpenGL 4.5 spec:
    *
    * "It is not allowed to have variables of different sampler types pointing
    *  to the same texture image unit within a program object."
    */
   unsigned stages_mask = shProg->data->linked_stages;
   while (stages_mask) {
      const int stage = u_bit_scan(&stages_mask);

      /* Skip validation if we are yet to update textures used in this
       * stage.
       */
      if (prog_stage < stage)
         break;

      struct gl_program *glprog = shProg->_LinkedShaders[stage]->Program;
      if (glprog->TexturesUsed[unit] & ~(1 << target))
         shProg->SamplersValidated = GL_FALSE;
   }

   prog->TexturesUsed[unit] |= (1 << target);
}

void
_mesa_update_shader_textures_used(struct gl_shader_program *shProg,
                                  struct gl_program *prog)
{
   GLbitfield mask = prog->SamplersUsed;
   ASSERTED gl_shader_stage prog_stage =
      _mesa_program_enum_to_shader_stage(prog->Target);
   GLuint s;

   assert(shProg->_LinkedShaders[prog_stage]);

   memset(prog->TexturesUsed, 0, sizeof(prog->TexturesUsed));
   prog->ShadowSamplers = prog->shader_program->_LinkedShaders[prog_stage]->shadow_samplers;

   while (mask) {
      s = u_bit_scan(&mask);

      update_single_shader_texture_used(shProg, prog,
                                        prog->SamplerUnits[s],
                                        prog->sh.SamplerTargets[s]);
   }

   if (unlikely(prog->sh.HasBoundBindlessSampler)) {
      /* Loop over bindless samplers bound to texture units.
       */
      for (s = 0; s < prog->sh.NumBindlessSamplers; s++) {
         struct gl_bindless_sampler *sampler = &prog->sh.BindlessSamplers[s];

         if (!sampler->bound)
            continue;

         update_single_shader_texture_used(shProg, prog, sampler->unit,
                                           sampler->target);
      }
   }
}

/**
 * Connect a piece of driver storage with a part of a uniform
 *
 * \param uni            The uniform with which the storage will be associated
 * \param element_stride Byte-stride between array elements.
 *                       \sa gl_uniform_driver_storage::element_stride.
 * \param vector_stride  Byte-stride between vectors (in a matrix).
 *                       \sa gl_uniform_driver_storage::vector_stride.
 * \param format         Conversion from native format to driver format
 *                       required by the driver.
 * \param data           Location to dump the data.
 */
void
_mesa_uniform_attach_driver_storage(struct gl_uniform_storage *uni,
				    unsigned element_stride,
				    unsigned vector_stride,
				    enum gl_uniform_driver_format format,
				    void *data)
{
   uni->driver_storage =
      realloc(uni->driver_storage,
	      sizeof(struct gl_uniform_driver_storage)
	      * (uni->num_driver_storage + 1));

   uni->driver_storage[uni->num_driver_storage].element_stride = element_stride;
   uni->driver_storage[uni->num_driver_storage].vector_stride = vector_stride;
   uni->driver_storage[uni->num_driver_storage].format = format;
   uni->driver_storage[uni->num_driver_storage].data = data;

   uni->num_driver_storage++;
}

/**
 * Sever all connections with all pieces of driver storage for all uniforms
 *
 * \warning
 * This function does \b not release any of the \c data pointers
 * previously passed in to \c _mesa_uniform_attach_driver_stoarge.
 */
void
_mesa_uniform_detach_all_driver_storage(struct gl_uniform_storage *uni)
{
   free(uni->driver_storage);
   uni->driver_storage = NULL;
   uni->num_driver_storage = 0;
}

void GLAPIENTRY
_mesa_Uniform1f(GLint location, GLfloat v0)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, 1, &v0, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_FLOAT, 1);
}

void GLAPIENTRY
_mesa_Uniform2f(GLint location, GLfloat v0, GLfloat v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat v[2];
   v[0] = v0;
   v[1] = v1;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_FLOAT, 2);
}

void GLAPIENTRY
_mesa_Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat v[3];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_FLOAT, 3);
}

void GLAPIENTRY
_mesa_Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2,
                   GLfloat v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat v[4];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_FLOAT, 4);
}

void GLAPIENTRY
_mesa_Uniform1i(GLint location, GLint v0)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, 1, &v0, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT, 1);
}

void GLAPIENTRY
_mesa_Uniform2i(GLint location, GLint v0, GLint v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint v[2];
   v[0] = v0;
   v[1] = v1;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT, 2);
}

void GLAPIENTRY
_mesa_Uniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint v[3];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT, 3);
}

void GLAPIENTRY
_mesa_Uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint v[4];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT, 4);
}

void GLAPIENTRY
_mesa_Uniform1fv(GLint location, GLsizei count, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_FLOAT, 1);
}

void GLAPIENTRY
_mesa_Uniform2fv(GLint location, GLsizei count, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_FLOAT, 2);
}

void GLAPIENTRY
_mesa_Uniform3fv(GLint location, GLsizei count, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_FLOAT, 3);
}

void GLAPIENTRY
_mesa_Uniform4fv(GLint location, GLsizei count, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_FLOAT, 4);
}

void GLAPIENTRY
_mesa_Uniform1iv(GLint location, GLsizei count, const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT, 1);
}

void GLAPIENTRY
_mesa_Uniform2iv(GLint location, GLsizei count, const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT, 2);
}

void GLAPIENTRY
_mesa_Uniform3iv(GLint location, GLsizei count, const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT, 3);
}

void GLAPIENTRY
_mesa_Uniform4iv(GLint location, GLsizei count, const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT, 4);
}

void GLAPIENTRY
_mesa_UniformHandleui64ARB(GLint location, GLuint64 value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_handle(location, 1, &value, ctx, ctx->_Shader->ActiveProgram);
}

void GLAPIENTRY
_mesa_UniformHandleui64vARB(GLint location, GLsizei count,
                            const GLuint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_handle(location, count, value, ctx,
                        ctx->_Shader->ActiveProgram);
}


/** Same as above with direct state access **/
void GLAPIENTRY
_mesa_ProgramUniform1f(GLuint program, GLint location, GLfloat v0)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1f");
   _mesa_uniform(location, 1, &v0, ctx, shProg, GLSL_TYPE_FLOAT, 1);
}

void GLAPIENTRY
_mesa_ProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat v[2];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform2f");
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_FLOAT, 2);
}

void GLAPIENTRY
_mesa_ProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1,
                       GLfloat v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat v[3];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform3f");
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_FLOAT, 3);
}

void GLAPIENTRY
_mesa_ProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1,
                       GLfloat v2, GLfloat v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat v[4];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform4f");
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_FLOAT, 4);
}

void GLAPIENTRY
_mesa_ProgramUniform1i(GLuint program, GLint location, GLint v0)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1i");
   _mesa_uniform(location, 1, &v0, ctx, shProg, GLSL_TYPE_INT, 1);
}

void GLAPIENTRY
_mesa_ProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint v[2];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform2i");
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_INT, 2);
}

void GLAPIENTRY
_mesa_ProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1,
                       GLint v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint v[3];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform3i");
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_INT, 3);
}

void GLAPIENTRY
_mesa_ProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1,
                       GLint v2, GLint v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint v[4];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform4i");
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_INT, 4);
}

void GLAPIENTRY
_mesa_ProgramUniform1fv(GLuint program, GLint location, GLsizei count,
                        const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1fv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_FLOAT, 1);
}

void GLAPIENTRY
_mesa_ProgramUniform2fv(GLuint program, GLint location, GLsizei count,
                        const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform2fv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_FLOAT, 2);
}

void GLAPIENTRY
_mesa_ProgramUniform3fv(GLuint program, GLint location, GLsizei count,
                        const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform3fv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_FLOAT, 3);
}

void GLAPIENTRY
_mesa_ProgramUniform4fv(GLuint program, GLint location, GLsizei count,
                        const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform4fv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_FLOAT, 4);
}

void GLAPIENTRY
_mesa_ProgramUniform1iv(GLuint program, GLint location, GLsizei count,
                        const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1iv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_INT, 1);
}

void GLAPIENTRY
_mesa_ProgramUniform2iv(GLuint program, GLint location, GLsizei count,
                        const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform2iv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_INT, 2);
}

void GLAPIENTRY
_mesa_ProgramUniform3iv(GLuint program, GLint location, GLsizei count,
                        const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform3iv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_INT, 3);
}

void GLAPIENTRY
_mesa_ProgramUniform4iv(GLuint program, GLint location, GLsizei count,
                        const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform4iv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_INT, 4);
}

void GLAPIENTRY
_mesa_ProgramUniformHandleui64ARB(GLuint program, GLint location,
                                  GLuint64 value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformHandleui64ARB");
   _mesa_uniform_handle(location, 1, &value, ctx, shProg);
}

void GLAPIENTRY
_mesa_ProgramUniformHandleui64vARB(GLuint program, GLint location,
                                   GLsizei count, const GLuint64 *values)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformHandleui64vARB");
   _mesa_uniform_handle(location, count, values, ctx, shProg);
}


/** OpenGL 3.0 GLuint-valued functions **/
void GLAPIENTRY
_mesa_Uniform1ui(GLint location, GLuint v0)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, 1, &v0, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT, 1);
}

void GLAPIENTRY
_mesa_Uniform2ui(GLint location, GLuint v0, GLuint v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint v[2];
   v[0] = v0;
   v[1] = v1;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT, 2);
}

void GLAPIENTRY
_mesa_Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint v[3];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT, 3);
}

void GLAPIENTRY
_mesa_Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint v[4];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT, 4);
}

void GLAPIENTRY
_mesa_Uniform1uiv(GLint location, GLsizei count, const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT, 1);
}

void GLAPIENTRY
_mesa_Uniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT, 2);
}

void GLAPIENTRY
_mesa_Uniform3uiv(GLint location, GLsizei count, const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT, 3);
}

void GLAPIENTRY
_mesa_Uniform4uiv(GLint location, GLsizei count, const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT, 4);
}



void GLAPIENTRY
_mesa_UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose,
                          const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 2, 2, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose,
                          const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 3, 3, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose,
                          const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 4, 4, GLSL_TYPE_FLOAT);
}

/** Same as above with direct state access **/

void GLAPIENTRY
_mesa_ProgramUniform1ui(GLuint program, GLint location, GLuint v0)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1ui");
   _mesa_uniform(location, 1, &v0, ctx, shProg, GLSL_TYPE_UINT, 1);
}

void GLAPIENTRY
_mesa_ProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint v[2];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   shProg = _mesa_lookup_shader_program_err(ctx, program,
                                            "glProgramUniform2ui");
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_UINT, 2);
}

void GLAPIENTRY
_mesa_ProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1,
                        GLuint v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint v[3];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   shProg = _mesa_lookup_shader_program_err(ctx, program,
                                            "glProgramUniform3ui");
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_UINT, 3);
}

void GLAPIENTRY
_mesa_ProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1,
                        GLuint v2, GLuint v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint v[4];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform4ui");
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_UINT, 4);
}

void GLAPIENTRY
_mesa_ProgramUniform1uiv(GLuint program, GLint location, GLsizei count,
                         const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1uiv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_UINT, 1);
}

void GLAPIENTRY
_mesa_ProgramUniform2uiv(GLuint program, GLint location, GLsizei count,
                         const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform2uiv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_UINT, 2);
}

void GLAPIENTRY
_mesa_ProgramUniform3uiv(GLuint program, GLint location, GLsizei count,
                         const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform3uiv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_UINT, 3);
}

void GLAPIENTRY
_mesa_ProgramUniform4uiv(GLuint program, GLint location, GLsizei count,
                         const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform4uiv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_UINT, 4);
}



void GLAPIENTRY
_mesa_ProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count,
                              GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix2fv");
   _mesa_uniform_matrix(location, count, transpose, value, ctx, shProg, 2, 2, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count,
                              GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix3fv");
   _mesa_uniform_matrix(location, count, transpose, value, ctx, shProg, 3, 3, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count,
                              GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix4fv");
   _mesa_uniform_matrix(location, count, transpose, value, ctx, shProg, 4, 4, GLSL_TYPE_FLOAT);
}


/**
 * Non-square UniformMatrix are OpenGL 2.1
 */
void GLAPIENTRY
_mesa_UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose,
                         const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 2, 3, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose,
                         const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 3, 2, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose,
                         const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 2, 4, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose,
                         const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 4, 2, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose,
                         const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 3, 4, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose,
                         const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 4, 3, GLSL_TYPE_FLOAT);
}

/** Same as above with direct state access **/

void GLAPIENTRY
_mesa_ProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix2x3fv");
   _mesa_uniform_matrix(location, count, transpose, value, ctx, shProg, 2, 3, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix3x2fv");
   _mesa_uniform_matrix(location, count, transpose, value, ctx, shProg, 3, 2, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix2x4fv");
   _mesa_uniform_matrix(location, count, transpose, value, ctx, shProg, 2, 4, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix4x2fv");
   _mesa_uniform_matrix(location, count, transpose, value, ctx, shProg, 4, 2, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix3x4fv");
   _mesa_uniform_matrix(location, count, transpose, value, ctx, shProg, 3, 4, GLSL_TYPE_FLOAT);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix4x3fv");
   _mesa_uniform_matrix(location, count, transpose, value, ctx, shProg, 4, 3, GLSL_TYPE_FLOAT);
}


void GLAPIENTRY
_mesa_GetnUniformfvARB(GLuint program, GLint location,
                       GLsizei bufSize, GLfloat *params)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_get_uniform(ctx, program, location, bufSize, GLSL_TYPE_FLOAT, params);
}

void GLAPIENTRY
_mesa_GetUniformfv(GLuint program, GLint location, GLfloat *params)
{
   _mesa_GetnUniformfvARB(program, location, INT_MAX, params);
}


void GLAPIENTRY
_mesa_GetnUniformivARB(GLuint program, GLint location,
                       GLsizei bufSize, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_get_uniform(ctx, program, location, bufSize, GLSL_TYPE_INT, params);
}

void GLAPIENTRY
_mesa_GetUniformiv(GLuint program, GLint location, GLint *params)
{
   _mesa_GetnUniformivARB(program, location, INT_MAX, params);
}


/* GL3 */
void GLAPIENTRY
_mesa_GetnUniformuivARB(GLuint program, GLint location,
                        GLsizei bufSize, GLuint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_get_uniform(ctx, program, location, bufSize, GLSL_TYPE_UINT, params);
}

void GLAPIENTRY
_mesa_GetUniformuiv(GLuint program, GLint location, GLuint *params)
{
   _mesa_GetnUniformuivARB(program, location, INT_MAX, params);
}


/* GL4 */
void GLAPIENTRY
_mesa_GetnUniformdvARB(GLuint program, GLint location,
                       GLsizei bufSize, GLdouble *params)
{
   GET_CURRENT_CONTEXT(ctx);

   _mesa_get_uniform(ctx, program, location, bufSize, GLSL_TYPE_DOUBLE, params);
}

void GLAPIENTRY
_mesa_GetUniformdv(GLuint program, GLint location, GLdouble *params)
{
   _mesa_GetnUniformdvARB(program, location, INT_MAX, params);
}

void GLAPIENTRY
_mesa_GetnUniformi64vARB(GLuint program, GLint location,
                         GLsizei bufSize, GLint64 *params)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_get_uniform(ctx, program, location, bufSize, GLSL_TYPE_INT64, params);
}
void GLAPIENTRY
_mesa_GetUniformi64vARB(GLuint program, GLint location, GLint64 *params)
{
   _mesa_GetnUniformi64vARB(program, location, INT_MAX, params);
}

void GLAPIENTRY
_mesa_GetnUniformui64vARB(GLuint program, GLint location,
                         GLsizei bufSize, GLuint64 *params)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_get_uniform(ctx, program, location, bufSize, GLSL_TYPE_UINT64, params);
}

void GLAPIENTRY
_mesa_GetUniformui64vARB(GLuint program, GLint location, GLuint64 *params)
{
   _mesa_GetnUniformui64vARB(program, location, INT_MAX, params);
}


GLint
_mesa_GetUniformLocation_impl(GLuint programObj, const GLcharARB *name,
                              bool glthread)
{
   struct gl_shader_program *shProg;

   GET_CURRENT_CONTEXT(ctx);

   shProg = _mesa_lookup_shader_program_err_glthread(ctx, programObj, glthread,
                                                     "glGetUniformLocation");
   if (!shProg || !name)
      return -1;

   /* Page 80 (page 94 of the PDF) of the OpenGL 2.1 spec says:
    *
    *     "If program has not been successfully linked, the error
    *     INVALID_OPERATION is generated."
    */
   if (shProg->data->LinkStatus == LINKING_FAILURE) {
      _mesa_error_glthread_safe(ctx, GL_INVALID_OPERATION, glthread,
                                "glGetUniformLocation(program not linked)");
      return -1;
   }

   return _mesa_program_resource_location(shProg, GL_UNIFORM, name);
}

GLint GLAPIENTRY
_mesa_GetUniformLocation(GLuint programObj, const GLcharARB *name)
{
   return _mesa_GetUniformLocation_impl(programObj, name, false);
}

GLint GLAPIENTRY
_mesa_GetUniformLocation_no_error(GLuint programObj, const GLcharARB *name)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program(ctx, programObj);

   return _mesa_program_resource_location(shProg, GL_UNIFORM, name);
}

GLuint GLAPIENTRY
_mesa_GetUniformBlockIndex(GLuint program,
			   const GLchar *uniformBlockName)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;

   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetUniformBlockIndex");
      return GL_INVALID_INDEX;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
					    "glGetUniformBlockIndex");
   if (!shProg)
      return GL_INVALID_INDEX;

   struct gl_program_resource *res =
      _mesa_program_resource_find_name(shProg, GL_UNIFORM_BLOCK,
                                       uniformBlockName, NULL);
   if (!res)
      return GL_INVALID_INDEX;

   return _mesa_program_resource_index(shProg, res);
}

void GLAPIENTRY
_mesa_GetUniformIndices(GLuint program,
			GLsizei uniformCount,
			const GLchar * const *uniformNames,
			GLuint *uniformIndices)
{
   GET_CURRENT_CONTEXT(ctx);
   GLsizei i;
   struct gl_shader_program *shProg;

   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetUniformIndices");
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
					    "glGetUniformIndices");
   if (!shProg)
      return;

   if (uniformCount < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glGetUniformIndices(uniformCount < 0)");
      return;
   }

   for (i = 0; i < uniformCount; i++) {
      struct gl_program_resource *res =
         _mesa_program_resource_find_name(shProg, GL_UNIFORM, uniformNames[i],
                                          NULL);
      uniformIndices[i] = _mesa_program_resource_index(shProg, res);
   }
}

static void
uniform_block_binding(struct gl_context *ctx, struct gl_shader_program *shProg,
                      GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
   if (shProg->data->UniformBlocks[uniformBlockIndex].Binding !=
       uniformBlockBinding) {

      FLUSH_VERTICES(ctx, 0, 0);
      ctx->NewDriverState |= ST_NEW_UNIFORM_BUFFER;

      shProg->data->UniformBlocks[uniformBlockIndex].Binding =
         uniformBlockBinding;
   }
}

void GLAPIENTRY
_mesa_UniformBlockBinding_no_error(GLuint program, GLuint uniformBlockIndex,
                                   GLuint uniformBlockBinding)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_shader_program *shProg = _mesa_lookup_shader_program(ctx, program);
   uniform_block_binding(ctx, shProg, uniformBlockIndex, uniformBlockBinding);
}

void GLAPIENTRY
_mesa_UniformBlockBinding(GLuint program,
			  GLuint uniformBlockIndex,
			  GLuint uniformBlockBinding)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;

   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glUniformBlockBinding");
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
					    "glUniformBlockBinding");
   if (!shProg)
      return;

   if (uniformBlockIndex >= shProg->data->NumUniformBlocks) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glUniformBlockBinding(block index %u >= %u)",
                  uniformBlockIndex, shProg->data->NumUniformBlocks);
      return;
   }

   if (uniformBlockBinding >= ctx->Const.MaxUniformBufferBindings) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glUniformBlockBinding(block binding %u >= %u)",
		  uniformBlockBinding, ctx->Const.MaxUniformBufferBindings);
      return;
   }

   uniform_block_binding(ctx, shProg, uniformBlockIndex, uniformBlockBinding);
}

static void
shader_storage_block_binding(struct gl_context *ctx,
                             struct gl_shader_program *shProg,
                             GLuint shaderStorageBlockIndex,
                             GLuint shaderStorageBlockBinding)
{
   if (shProg->data->ShaderStorageBlocks[shaderStorageBlockIndex].Binding !=
       shaderStorageBlockBinding) {

      FLUSH_VERTICES(ctx, 0, 0);
      ctx->NewDriverState |= ST_NEW_STORAGE_BUFFER;

      shProg->data->ShaderStorageBlocks[shaderStorageBlockIndex].Binding =
         shaderStorageBlockBinding;
   }
}

void GLAPIENTRY
_mesa_ShaderStorageBlockBinding_no_error(GLuint program,
                                         GLuint shaderStorageBlockIndex,
                                         GLuint shaderStorageBlockBinding)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_shader_program *shProg = _mesa_lookup_shader_program(ctx, program);
   shader_storage_block_binding(ctx, shProg, shaderStorageBlockIndex,
                                shaderStorageBlockBinding);
}

void GLAPIENTRY
_mesa_ShaderStorageBlockBinding(GLuint program,
			        GLuint shaderStorageBlockIndex,
			        GLuint shaderStorageBlockBinding)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;

   if (!ctx->Extensions.ARB_shader_storage_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glShaderStorageBlockBinding");
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
					    "glShaderStorageBlockBinding");
   if (!shProg)
      return;

   if (shaderStorageBlockIndex >= shProg->data->NumShaderStorageBlocks) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glShaderStorageBlockBinding(block index %u >= %u)",
                  shaderStorageBlockIndex,
                  shProg->data->NumShaderStorageBlocks);
      return;
   }

   if (shaderStorageBlockBinding >= ctx->Const.MaxShaderStorageBufferBindings) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glShaderStorageBlockBinding(block binding %u >= %u)",
		  shaderStorageBlockBinding,
                  ctx->Const.MaxShaderStorageBufferBindings);
      return;
   }

   shader_storage_block_binding(ctx, shProg, shaderStorageBlockIndex,
                                shaderStorageBlockBinding);
}

/**
 * Generic program resource property query.
 */
static void
mesa_bufferiv(struct gl_shader_program *shProg, GLenum type,
              GLuint index, GLenum pname, GLint *params, const char *caller)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_program_resource *res =
      _mesa_program_resource_find_index(shProg, type, index);

   if (!res) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(bufferindex %d)", caller, index);
      return;
   }

   switch (pname) {
   case GL_UNIFORM_BLOCK_BINDING:
   case GL_ATOMIC_COUNTER_BUFFER_BINDING:
      _mesa_program_resource_prop(shProg, res, index, GL_BUFFER_BINDING,
                                  params, false, caller);
      return;
   case GL_UNIFORM_BLOCK_DATA_SIZE:
   case GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE:
      _mesa_program_resource_prop(shProg, res, index, GL_BUFFER_DATA_SIZE,
                                  params, false, caller);
      return;
   case GL_UNIFORM_BLOCK_NAME_LENGTH:
      _mesa_program_resource_prop(shProg, res, index, GL_NAME_LENGTH,
                                  params, false, caller);
      return;
   case GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS:
   case GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS:
      _mesa_program_resource_prop(shProg, res, index, GL_NUM_ACTIVE_VARIABLES,
                                  params, false, caller);
      return;
   case GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES:
   case GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES:
      _mesa_program_resource_prop(shProg, res, index, GL_ACTIVE_VARIABLES,
                                  params, false, caller);
      return;
   case GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER:
   case GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER:
      _mesa_program_resource_prop(shProg, res, index,
                                  GL_REFERENCED_BY_VERTEX_SHADER, params,
                                  false, caller);
      return;

   case GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER:
   case GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER:
      _mesa_program_resource_prop(shProg, res, index,
                                  GL_REFERENCED_BY_TESS_CONTROL_SHADER, params,
                                  false, caller);
      return;

   case GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER:
   case GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER:
      _mesa_program_resource_prop(shProg, res, index,
                                  GL_REFERENCED_BY_TESS_EVALUATION_SHADER, params,
                                  false, caller);
      return;

   case GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER:
   case GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER:
      _mesa_program_resource_prop(shProg, res, index,
                                  GL_REFERENCED_BY_GEOMETRY_SHADER, params,
                                  false, caller);
      return;
   case GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER:
   case GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER:
      _mesa_program_resource_prop(shProg, res, index,
                                  GL_REFERENCED_BY_FRAGMENT_SHADER, params,
                                  false, caller);
      return;
   case GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER:
   case GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER:
      _mesa_program_resource_prop(shProg, res, index,
                                  GL_REFERENCED_BY_COMPUTE_SHADER, params,
                                  false, caller);
      return;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(pname 0x%x (%s))", caller, pname,
                  _mesa_enum_to_string(pname));
      return;
   }
}


void GLAPIENTRY
_mesa_GetActiveUniformBlockiv(GLuint program,
			      GLuint uniformBlockIndex,
			      GLenum pname,
			      GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;

   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetActiveUniformBlockiv");
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
					    "glGetActiveUniformBlockiv");
   if (!shProg)
      return;

   mesa_bufferiv(shProg, GL_UNIFORM_BLOCK, uniformBlockIndex, pname, params,
                 "glGetActiveUniformBlockiv");
}

void GLAPIENTRY
_mesa_GetActiveUniformBlockName(GLuint program,
				GLuint uniformBlockIndex,
				GLsizei bufSize,
				GLsizei *length,
				GLchar *uniformBlockName)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;

   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetActiveUniformBlockiv");
      return;
   }

   if (bufSize < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glGetActiveUniformBlockName(bufSize %d < 0)",
		  bufSize);
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
					    "glGetActiveUniformBlockiv");
   if (!shProg)
      return;

   if (uniformBlockName)
      _mesa_get_program_resource_name(shProg, GL_UNIFORM_BLOCK,
                                      uniformBlockIndex, bufSize, length,
                                      uniformBlockName, false,
                                      "glGetActiveUniformBlockName");
}

void GLAPIENTRY
_mesa_GetActiveUniformName(GLuint program, GLuint uniformIndex,
			   GLsizei bufSize, GLsizei *length,
			   GLchar *uniformName)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;

   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetActiveUniformName");
      return;
   }

   if (bufSize < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glGetActiveUniformName(bufSize %d < 0)",
		  bufSize);
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program, "glGetActiveUniformName");

   if (!shProg)
      return;

   _mesa_get_program_resource_name(shProg, GL_UNIFORM, uniformIndex, bufSize,
                                   length, uniformName, false,
                                   "glGetActiveUniformName");
}

void GLAPIENTRY
_mesa_GetActiveAtomicCounterBufferiv(GLuint program, GLuint bufferIndex,
                                     GLenum pname, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;

   if (!ctx->Extensions.ARB_shader_atomic_counters) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetActiveAtomicCounterBufferiv");
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
                                            "glGetActiveAtomicCounterBufferiv");
   if (!shProg)
      return;

   mesa_bufferiv(shProg, GL_ATOMIC_COUNTER_BUFFER, bufferIndex, pname, params,
                 "glGetActiveAtomicCounterBufferiv");
}

void GLAPIENTRY
_mesa_Uniform1d(GLint location, GLdouble v0)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, 1, &v0, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_DOUBLE, 1);
}

void GLAPIENTRY
_mesa_Uniform2d(GLint location, GLdouble v0, GLdouble v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLdouble v[2];
   v[0] = v0;
   v[1] = v1;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_DOUBLE, 2);
}

void GLAPIENTRY
_mesa_Uniform3d(GLint location, GLdouble v0, GLdouble v1, GLdouble v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLdouble v[3];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_DOUBLE, 3);
}

void GLAPIENTRY
_mesa_Uniform4d(GLint location, GLdouble v0, GLdouble v1, GLdouble v2,
                GLdouble v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLdouble v[4];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_DOUBLE, 4);
}

void GLAPIENTRY
_mesa_Uniform1dv(GLint location, GLsizei count, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_DOUBLE, 1);
}

void GLAPIENTRY
_mesa_Uniform2dv(GLint location, GLsizei count, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_DOUBLE, 2);
}

void GLAPIENTRY
_mesa_Uniform3dv(GLint location, GLsizei count, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_DOUBLE, 3);
}

void GLAPIENTRY
_mesa_Uniform4dv(GLint location, GLsizei count, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_DOUBLE, 4);
}

void GLAPIENTRY
_mesa_UniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose,
                       const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 2, 2, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_UniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose,
                       const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 3, 3, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_UniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose,
                       const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 4, 4, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_UniformMatrix2x3dv(GLint location, GLsizei count, GLboolean transpose,
                         const GLdouble *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 2, 3, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_UniformMatrix3x2dv(GLint location, GLsizei count, GLboolean transpose,
                         const GLdouble *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 3, 2, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_UniformMatrix2x4dv(GLint location, GLsizei count, GLboolean transpose,
                         const GLdouble *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 2, 4, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_UniformMatrix4x2dv(GLint location, GLsizei count, GLboolean transpose,
                         const GLdouble *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 4, 2, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_UniformMatrix3x4dv(GLint location, GLsizei count, GLboolean transpose,
                         const GLdouble *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 3, 4, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_UniformMatrix4x3dv(GLint location, GLsizei count, GLboolean transpose,
                         const GLdouble *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, ctx->_Shader->ActiveProgram, 4, 3, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_ProgramUniform1d(GLuint program, GLint location, GLdouble v0)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1d");
   _mesa_uniform(location, 1, &v0, ctx, shProg, GLSL_TYPE_DOUBLE, 1);
}

void GLAPIENTRY
_mesa_ProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLdouble v[2];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform2d");
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_DOUBLE, 2);
}

void GLAPIENTRY
_mesa_ProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1,
                       GLdouble v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLdouble v[3];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform3d");
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_DOUBLE, 3);
}

void GLAPIENTRY
_mesa_ProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1,
                       GLdouble v2, GLdouble v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLdouble v[4];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform4d");
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_DOUBLE, 4);
}

void GLAPIENTRY
_mesa_ProgramUniform1dv(GLuint program, GLint location, GLsizei count,
                        const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1dv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_DOUBLE, 1);
}

void GLAPIENTRY
_mesa_ProgramUniform2dv(GLuint program, GLint location, GLsizei count,
                        const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform2dv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_DOUBLE, 2);
}

void GLAPIENTRY
_mesa_ProgramUniform3dv(GLuint program, GLint location, GLsizei count,
                        const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform3dv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_DOUBLE, 3);
}

void GLAPIENTRY
_mesa_ProgramUniform4dv(GLuint program, GLint location, GLsizei count,
                        const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform4dv");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_DOUBLE, 4);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count,
                              GLboolean transpose, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix2dv");
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, shProg, 2, 2, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count,
                              GLboolean transpose, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix3dv");
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, shProg, 3, 3, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count,
                              GLboolean transpose, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix4dv");
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, shProg, 4, 4, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix2x3dv");
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, shProg, 2, 3, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix3x2dv");
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, shProg, 3, 2, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix2x4dv");
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, shProg, 2, 4, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix4x2dv");
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, shProg, 4, 2, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix3x4dv");
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, shProg, 3, 4, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLdouble * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix4x3dv");
   _mesa_uniform_matrix(location, count, transpose, value,
                        ctx, shProg, 4, 3, GLSL_TYPE_DOUBLE);
}

void GLAPIENTRY
_mesa_Uniform1i64ARB(GLint location, GLint64 v0)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, 1, &v0, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT64, 1);
}

void GLAPIENTRY
_mesa_Uniform2i64ARB(GLint location, GLint64 v0, GLint64 v1)
{
   GET_CURRENT_CONTEXT(ctx);
   int64_t v[2];
   v[0] = v0;
   v[1] = v1;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT64, 2);
}

void GLAPIENTRY
_mesa_Uniform3i64ARB(GLint location, GLint64 v0, GLint64 v1, GLint64 v2)
{
   GET_CURRENT_CONTEXT(ctx);
   int64_t v[3];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT64, 3);
}

void GLAPIENTRY
_mesa_Uniform4i64ARB(GLint location,  GLint64 v0, GLint64 v1, GLint64 v2, GLint64 v3)
{
   GET_CURRENT_CONTEXT(ctx);
   int64_t v[4];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT64, 4);
}

void GLAPIENTRY
_mesa_Uniform1i64vARB(GLint location, GLsizei count, const GLint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT64, 1);
}

void GLAPIENTRY
_mesa_Uniform2i64vARB(GLint location,  GLsizei count, const GLint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT64, 2);
}

void GLAPIENTRY
_mesa_Uniform3i64vARB(GLint location,  GLsizei count, const GLint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT64, 3);
}

void GLAPIENTRY
_mesa_Uniform4i64vARB(GLint location,  GLsizei count, const GLint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_INT64, 4);
}

void GLAPIENTRY
_mesa_Uniform1ui64ARB(GLint location,  GLuint64 v0)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, 1, &v0, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT64, 1);
}

void GLAPIENTRY
_mesa_Uniform2ui64ARB(GLint location,  GLuint64 v0, GLuint64 v1)
{
   GET_CURRENT_CONTEXT(ctx);
   uint64_t v[2];
   v[0] = v0;
   v[1] = v1;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT64, 2);
}

void GLAPIENTRY
_mesa_Uniform3ui64ARB(GLint location, GLuint64 v0, GLuint64 v1, GLuint64 v2)
{
   GET_CURRENT_CONTEXT(ctx);
   uint64_t v[3];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT64, 3);
}

void GLAPIENTRY
_mesa_Uniform4ui64ARB(GLint location,  GLuint64 v0, GLuint64 v1, GLuint64 v2, GLuint64 v3)
{
   GET_CURRENT_CONTEXT(ctx);
   uint64_t v[4];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   _mesa_uniform(location, 1, v, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT64, 4);
}

void GLAPIENTRY
_mesa_Uniform1ui64vARB(GLint location,  GLsizei count, const GLuint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT64, 1);
}

void GLAPIENTRY
_mesa_Uniform2ui64vARB(GLint location,  GLsizei count, const GLuint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT64, 2);
}

void GLAPIENTRY
_mesa_Uniform3ui64vARB(GLint location,  GLsizei count, const GLuint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT64, 3);
}

void GLAPIENTRY
_mesa_Uniform4ui64vARB(GLint location,  GLsizei count, const GLuint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(location, count, value, ctx, ctx->_Shader->ActiveProgram, GLSL_TYPE_UINT64, 4);
}

/* DSA entrypoints */
void GLAPIENTRY
_mesa_ProgramUniform1i64ARB(GLuint program, GLint location, GLint64 v0)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1i64ARB");
   _mesa_uniform(location, 1, &v0, ctx, shProg, GLSL_TYPE_INT64, 1);
}

void GLAPIENTRY
_mesa_ProgramUniform2i64ARB(GLuint program, GLint location, GLint64 v0, GLint64 v1)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform2i64ARB");
   int64_t v[2];
   v[0] = v0;
   v[1] = v1;
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_INT64, 2);
}

void GLAPIENTRY
_mesa_ProgramUniform3i64ARB(GLuint program, GLint location, GLint64 v0, GLint64 v1, GLint64 v2)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform3i64ARB");
   int64_t v[3];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_INT64, 3);
}

void GLAPIENTRY
_mesa_ProgramUniform4i64ARB(GLuint program, GLint location,  GLint64 v0, GLint64 v1, GLint64 v2, GLint64 v3)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform4i64ARB");
   int64_t v[4];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_INT64, 4);
}

void GLAPIENTRY
_mesa_ProgramUniform1i64vARB(GLuint program, GLint location, GLsizei count, const GLint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform1i64vARB");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_INT64, 1);
}

void GLAPIENTRY
_mesa_ProgramUniform2i64vARB(GLuint program, GLint location,  GLsizei count, const GLint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform2i64vARB");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_INT64, 2);
}

void GLAPIENTRY
_mesa_ProgramUniform3i64vARB(GLuint program, GLint location,  GLsizei count, const GLint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform3i64vARB");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_INT64, 3);
}

void GLAPIENTRY
_mesa_ProgramUniform4i64vARB(GLuint program, GLint location,  GLsizei count, const GLint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform4i64vARB");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_INT64, 4);
}

void GLAPIENTRY
_mesa_ProgramUniform1ui64ARB(GLuint program, GLint location,  GLuint64 v0)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform1ui64ARB");
   _mesa_uniform(location, 1, &v0, ctx, shProg, GLSL_TYPE_UINT64, 1);
}

void GLAPIENTRY
_mesa_ProgramUniform2ui64ARB(GLuint program, GLint location,  GLuint64 v0, GLuint64 v1)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform2ui64ARB");
   uint64_t v[2];
   v[0] = v0;
   v[1] = v1;
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_UINT64, 2);
}

void GLAPIENTRY
_mesa_ProgramUniform3ui64ARB(GLuint program, GLint location, GLuint64 v0, GLuint64 v1, GLuint64 v2)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform3ui64ARB");
   uint64_t v[3];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_UINT64, 3);
}

void GLAPIENTRY
_mesa_ProgramUniform4ui64ARB(GLuint program, GLint location,  GLuint64 v0, GLuint64 v1, GLuint64 v2, GLuint64 v3)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform4ui64ARB");
   uint64_t v[4];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   _mesa_uniform(location, 1, v, ctx, shProg, GLSL_TYPE_UINT64, 4);
}

void GLAPIENTRY
_mesa_ProgramUniform1ui64vARB(GLuint program, GLint location,  GLsizei count, const GLuint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform1ui64vARB");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_UINT64, 1);
}

void GLAPIENTRY
_mesa_ProgramUniform2ui64vARB(GLuint program, GLint location,  GLsizei count, const GLuint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform2ui64vARB");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_UINT64, 2);
}

void GLAPIENTRY
_mesa_ProgramUniform3ui64vARB(GLuint program, GLint location,  GLsizei count, const GLuint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform3ui64vARB");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_UINT64, 3);
}

void GLAPIENTRY
_mesa_ProgramUniform4ui64vARB(GLuint program, GLint location,  GLsizei count, const GLuint64 *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
                                      "glProgramUniform4ui64vARB");
   _mesa_uniform(location, count, value, ctx, shProg, GLSL_TYPE_UINT64, 4);
}
