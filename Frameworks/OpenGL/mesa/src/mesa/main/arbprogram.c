/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
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
 * \file arbprogram.c
 * ARB_vertex/fragment_program state management functions.
 * \author Brian Paul
 */


#include "util/glheader.h"
#include "main/context.h"
#include "main/draw_validate.h"
#include "main/hash.h"

#include "main/macros.h"
#include "main/mtypes.h"
#include "main/shaderapi.h"
#include "main/state.h"
#include "program/arbprogparse.h"
#include "program/program.h"
#include "program/prog_print.h"
#include "api_exec_decl.h"

#include "state_tracker/st_program.h"

static void
flush_vertices_for_program_constants(struct gl_context *ctx, GLenum target)
{
   uint64_t new_driver_state;

   if (target == GL_FRAGMENT_PROGRAM_ARB) {
      new_driver_state =
         ctx->DriverFlags.NewShaderConstants[MESA_SHADER_FRAGMENT];
   } else {
      new_driver_state =
         ctx->DriverFlags.NewShaderConstants[MESA_SHADER_VERTEX];
   }

   FLUSH_VERTICES(ctx, new_driver_state ? 0 : _NEW_PROGRAM_CONSTANTS, 0);
   ctx->NewDriverState |= new_driver_state;
}

static struct gl_program*
lookup_or_create_program(GLuint id, GLenum target, const char* caller)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_program* newProg;

   if (id == 0) {
      /* Bind a default program */
      if (target == GL_VERTEX_PROGRAM_ARB)
         newProg = ctx->Shared->DefaultVertexProgram;
      else
         newProg = ctx->Shared->DefaultFragmentProgram;
   }
   else {
      /* Bind a user program */
      newProg = _mesa_lookup_program(ctx, id);
      if (!newProg || newProg == &_mesa_DummyProgram) {
         bool isGenName = newProg != NULL;
         /* allocate a new program now */
         newProg = ctx->Driver.NewProgram(ctx, _mesa_program_enum_to_shader_stage(target),
                                          id, true);
         if (!newProg) {
            _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", caller);
            return NULL;
         }
         _mesa_HashInsert(ctx->Shared->Programs, id, newProg, isGenName);
      }
      else if (newProg->Target != target) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(target mismatch)", caller);
         return NULL;
      }
   }
   return newProg;
}

/**
 * Bind a program (make it current)
 * \note Called from the GL API dispatcher by both glBindProgramNV
 * and glBindProgramARB.
 */
void GLAPIENTRY
_mesa_BindProgramARB(GLenum target, GLuint id)
{
   struct gl_program *curProg, *newProg;
   GET_CURRENT_CONTEXT(ctx);

   /* Error-check target and get curProg */
   if (target == GL_VERTEX_PROGRAM_ARB && ctx->Extensions.ARB_vertex_program) {
      curProg = ctx->VertexProgram.Current;
   }
   else if (target == GL_FRAGMENT_PROGRAM_ARB
            && ctx->Extensions.ARB_fragment_program) {
      curProg = ctx->FragmentProgram.Current;
   }
   else {
      _mesa_error(ctx, GL_INVALID_ENUM, "glBindProgramARB(target)");
      return;
   }

   /*
    * Get pointer to new program to bind.
    * NOTE: binding to a non-existant program is not an error.
    * That's supposed to be caught in glBegin.
    */
   newProg = lookup_or_create_program(id, target, "glBindProgram");
   if (!newProg)
      return;

   /** All error checking is complete now **/

   if (curProg->Id == id) {
      /* binding same program - no change */
      return;
   }

   /* signal new program (and its new constants) */
   FLUSH_VERTICES(ctx, _NEW_PROGRAM, 0);
   flush_vertices_for_program_constants(ctx, target);

   /* bind newProg */
   if (target == GL_VERTEX_PROGRAM_ARB) {
      _mesa_reference_program(ctx, &ctx->VertexProgram.Current, newProg);
   }
   else if (target == GL_FRAGMENT_PROGRAM_ARB) {
      _mesa_reference_program(ctx, &ctx->FragmentProgram.Current, newProg);
   }

   _mesa_update_vertex_processing_mode(ctx);
   _mesa_update_valid_to_render_state(ctx);

   /* Never null pointers */
   assert(ctx->VertexProgram.Current);
   assert(ctx->FragmentProgram.Current);
}


/**
 * Delete a list of programs.
 * \note Not compiled into display lists.
 * \note Called by both glDeleteProgramsNV and glDeleteProgramsARB.
 */
void GLAPIENTRY
_mesa_DeleteProgramsARB(GLsizei n, const GLuint *ids)
{
   GLint i;
   GET_CURRENT_CONTEXT(ctx);

   FLUSH_VERTICES(ctx, 0, 0);

   if (n < 0) {
      _mesa_error( ctx, GL_INVALID_VALUE, "glDeleteProgramsNV" );
      return;
   }

   for (i = 0; i < n; i++) {
      if (ids[i] != 0) {
         struct gl_program *prog = _mesa_lookup_program(ctx, ids[i]);
         if (prog == &_mesa_DummyProgram) {
            _mesa_HashRemove(ctx->Shared->Programs, ids[i]);
         }
         else if (prog) {
            /* Unbind program if necessary */
            switch (prog->Target) {
            case GL_VERTEX_PROGRAM_ARB:
               if (ctx->VertexProgram.Current &&
                   ctx->VertexProgram.Current->Id == ids[i]) {
                  /* unbind this currently bound program */
                  _mesa_BindProgramARB(prog->Target, 0);
               }
               break;
            case GL_FRAGMENT_PROGRAM_ARB:
               if (ctx->FragmentProgram.Current &&
                   ctx->FragmentProgram.Current->Id == ids[i]) {
                  /* unbind this currently bound program */
                  _mesa_BindProgramARB(prog->Target, 0);
               }
               break;
            default:
               _mesa_problem(ctx, "bad target in glDeleteProgramsNV");
               return;
            }
            /* The ID is immediately available for re-use now */
            _mesa_HashRemove(ctx->Shared->Programs, ids[i]);
            _mesa_reference_program(ctx, &prog, NULL);
         }
      }
   }
}


/**
 * Generate a list of new program identifiers.
 * \note Not compiled into display lists.
 * \note Called by both glGenProgramsNV and glGenProgramsARB.
 */
void GLAPIENTRY
_mesa_GenProgramsARB(GLsizei n, GLuint *ids)
{
   GLuint i;
   GET_CURRENT_CONTEXT(ctx);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGenPrograms");
      return;
   }

   if (!ids)
      return;

   _mesa_HashLockMutex(ctx->Shared->Programs);

   _mesa_HashFindFreeKeys(ctx->Shared->Programs, ids, n);

   /* Insert pointer to dummy program as placeholder */
   for (i = 0; i < (GLuint) n; i++) {
      _mesa_HashInsertLocked(ctx->Shared->Programs, ids[i],
                             &_mesa_DummyProgram, true);
   }

   _mesa_HashUnlockMutex(ctx->Shared->Programs);
}


/**
 * Determine if id names a vertex or fragment program.
 * \note Not compiled into display lists.
 * \note Called from both glIsProgramNV and glIsProgramARB.
 * \param id is the program identifier
 * \return GL_TRUE if id is a program, else GL_FALSE.
 */
GLboolean GLAPIENTRY
_mesa_IsProgramARB(GLuint id)
{
   struct gl_program *prog = NULL;
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_FALSE);

   if (id == 0)
      return GL_FALSE;

   prog = _mesa_lookup_program(ctx, id);
   if (prog && (prog != &_mesa_DummyProgram))
      return GL_TRUE;
   else
      return GL_FALSE;
}

static struct gl_program*
get_current_program(struct gl_context* ctx, GLenum target, const char* caller)
{
   if (target == GL_VERTEX_PROGRAM_ARB
       && ctx->Extensions.ARB_vertex_program) {
      return ctx->VertexProgram.Current;
   }
   else if (target == GL_FRAGMENT_PROGRAM_ARB
            && ctx->Extensions.ARB_fragment_program) {
      return ctx->FragmentProgram.Current;
   }
   else {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(target)", caller);
      return NULL;
   }
}

static GLboolean
get_local_param_pointer(struct gl_context *ctx, const char *func,
                        struct gl_program* prog, GLenum target,
                        GLuint index, unsigned count, GLfloat **param)
{
   if (unlikely(index + count > prog->arb.MaxLocalParams)) {
      /* If arb.MaxLocalParams == 0, we need to do initialization. */
      if (!prog->arb.MaxLocalParams) {
         unsigned max;

         if (target == GL_VERTEX_PROGRAM_ARB)
            max = ctx->Const.Program[MESA_SHADER_VERTEX].MaxLocalParams;
         else
            max = ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxLocalParams;

         /* Allocate LocalParams. */
         if (!prog->arb.LocalParams) {
            prog->arb.LocalParams = rzalloc_array_size(prog, sizeof(float[4]),
                                                       max);
            if (!prog->arb.LocalParams) {
               _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", func);
               return GL_FALSE;
            }
         }

         /* Initialize MaxLocalParams. */
         prog->arb.MaxLocalParams = max;
      }

      /* Check again after initializing MaxLocalParams. */
      if (index + count > prog->arb.MaxLocalParams) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(index)", func);
         return GL_FALSE;
      }
   }

   *param = prog->arb.LocalParams[index];
   return GL_TRUE;
}


static GLboolean
get_env_param_pointer(struct gl_context *ctx, const char *func,
		      GLenum target, GLuint index, GLfloat **param)
{
   if (target == GL_FRAGMENT_PROGRAM_ARB
       && ctx->Extensions.ARB_fragment_program) {
      if (index >= ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxEnvParams) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(index)", func);
         return GL_FALSE;
      }
      *param = ctx->FragmentProgram.Parameters[index];
      return GL_TRUE;
   }
   else if (target == GL_VERTEX_PROGRAM_ARB &&
            ctx->Extensions.ARB_vertex_program) {
      if (index >= ctx->Const.Program[MESA_SHADER_VERTEX].MaxEnvParams) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(index)", func);
         return GL_FALSE;
      }
      *param = ctx->VertexProgram.Parameters[index];
      return GL_TRUE;
   } else {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(target)", func);
      return GL_FALSE;
   }
}

static void
set_program_string(struct gl_program *prog, GLenum target, GLenum format, GLsizei len,
                       const GLvoid *string)
{
   bool failed;
   GET_CURRENT_CONTEXT(ctx);

   FLUSH_VERTICES(ctx, _NEW_PROGRAM, 0);

   if (!ctx->Extensions.ARB_vertex_program
       && !ctx->Extensions.ARB_fragment_program) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glProgramStringARB()");
      return;
   }

   if (format != GL_PROGRAM_FORMAT_ASCII_ARB) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glProgramStringARB(format)");
      return;
   }

#ifdef ENABLE_SHADER_CACHE
   GLcharARB *replacement;

   gl_shader_stage stage = _mesa_program_enum_to_shader_stage(target);

   uint8_t sha1[SHA1_DIGEST_LENGTH];
   _mesa_sha1_compute(string, len, sha1);

   /* Dump original shader source to MESA_SHADER_DUMP_PATH and replace
    * if corresponding entry found from MESA_SHADER_READ_PATH.
    */
   _mesa_dump_shader_source(stage, string, sha1);

   replacement = _mesa_read_shader_source(stage, string, sha1);
   if (replacement)
      string = replacement;
#endif /* ENABLE_SHADER_CACHE */

   if (target == GL_VERTEX_PROGRAM_ARB && ctx->Extensions.ARB_vertex_program) {
      _mesa_parse_arb_vertex_program(ctx, target, string, len, prog);
   }
   else if (target == GL_FRAGMENT_PROGRAM_ARB
            && ctx->Extensions.ARB_fragment_program) {
      _mesa_parse_arb_fragment_program(ctx, target, string, len, prog);
   }
   else {
      _mesa_error(ctx, GL_INVALID_ENUM, "glProgramStringARB(target)");
      return;
   }

   failed = ctx->Program.ErrorPos != -1;

   if (!failed) {
      /* finally, give the program to the driver for translation/checking */
      if (!st_program_string_notify(ctx, target, prog)) {
         failed = true;
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glProgramStringARB(rejected by driver");
      }
   }

   _mesa_update_vertex_processing_mode(ctx);
   _mesa_update_valid_to_render_state(ctx);

   if (ctx->_Shader->Flags & GLSL_DUMP) {
      const char *shader_type =
         target == GL_FRAGMENT_PROGRAM_ARB ? "fragment" : "vertex";

      fprintf(stderr, "ARB_%s_program source for program %d:\n",
              shader_type, prog->Id);
      fprintf(stderr, "%s\n", (const char *) string);

      if (failed) {
         fprintf(stderr, "ARB_%s_program %d failed to compile.\n",
                 shader_type, prog->Id);
      } else {
         fprintf(stderr, "Mesa IR for ARB_%s_program %d:\n",
                 shader_type, prog->Id);
         _mesa_print_program(prog);
         fprintf(stderr, "\n");
      }
      fflush(stderr);
   }

   /* Capture vp-*.shader_test/fp-*.shader_test files. */
   const char *capture_path = _mesa_get_shader_capture_path();
   if (capture_path != NULL) {
      FILE *file;
      const char *shader_type =
         target == GL_FRAGMENT_PROGRAM_ARB ? "fragment" : "vertex";
      char *filename =
         ralloc_asprintf(NULL, "%s/%cp-%u.shader_test",
                         capture_path, shader_type[0], prog->Id);

      file = fopen(filename, "w");
      if (file) {
         fprintf(file,
                 "[require]\nGL_ARB_%s_program\n\n[%s program]\n%s\n",
                 shader_type, shader_type, (const char *) string);
         fclose(file);
      } else {
         _mesa_warning(ctx, "Failed to open %s", filename);
      }
      ralloc_free(filename);
   }
}

void GLAPIENTRY
_mesa_ProgramStringARB(GLenum target, GLenum format, GLsizei len,
                       const GLvoid *string)
{
   GET_CURRENT_CONTEXT(ctx);
   if (target == GL_VERTEX_PROGRAM_ARB && ctx->Extensions.ARB_vertex_program) {
      set_program_string(ctx->VertexProgram.Current, target, format, len, string);
   }
   else if (target == GL_FRAGMENT_PROGRAM_ARB
            && ctx->Extensions.ARB_fragment_program) {
      set_program_string(ctx->FragmentProgram.Current, target, format, len, string);
   }
   else {
      _mesa_error(ctx, GL_INVALID_ENUM, "glProgramStringARB(target)");
      return;
   }
}

void GLAPIENTRY
_mesa_NamedProgramStringEXT(GLuint program, GLenum target, GLenum format, GLsizei len,
                            const GLvoid *string)
{
   struct gl_program* prog = lookup_or_create_program(program, target, "glNamedProgramStringEXT");

   if (!prog) {
      return;
   }
   set_program_string(prog, target, format, len, string);
}


/**
 * Set a program env parameter register.
 * \note Called from the GL API dispatcher.
 */
void GLAPIENTRY
_mesa_ProgramEnvParameter4dARB(GLenum target, GLuint index,
                               GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
   _mesa_ProgramEnvParameter4fARB(target, index, (GLfloat) x, (GLfloat) y,
		                  (GLfloat) z, (GLfloat) w);
}


/**
 * Set a program env parameter register.
 * \note Called from the GL API dispatcher.
 */
void GLAPIENTRY
_mesa_ProgramEnvParameter4dvARB(GLenum target, GLuint index,
                                const GLdouble *params)
{
   _mesa_ProgramEnvParameter4fARB(target, index, (GLfloat) params[0],
	                          (GLfloat) params[1], (GLfloat) params[2],
				  (GLfloat) params[3]);
}


/**
 * Set a program env parameter register.
 * \note Called from the GL API dispatcher.
 */
void GLAPIENTRY
_mesa_ProgramEnvParameter4fARB(GLenum target, GLuint index,
                               GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GLfloat *param;

   GET_CURRENT_CONTEXT(ctx);

   flush_vertices_for_program_constants(ctx, target);

   if (get_env_param_pointer(ctx, "glProgramEnvParameter",
			     target, index, &param)) {
      ASSIGN_4V(param, x, y, z, w);
   }
}



/**
 * Set a program env parameter register.
 * \note Called from the GL API dispatcher.
 */
void GLAPIENTRY
_mesa_ProgramEnvParameter4fvARB(GLenum target, GLuint index,
                                const GLfloat *params)
{
   GLfloat *param;

   GET_CURRENT_CONTEXT(ctx);

   flush_vertices_for_program_constants(ctx, target);

   if (get_env_param_pointer(ctx, "glProgramEnvParameter4fv",
			      target, index, &param)) {
      memcpy(param, params, 4 * sizeof(GLfloat));
   }
}


void GLAPIENTRY
_mesa_ProgramEnvParameters4fvEXT(GLenum target, GLuint index, GLsizei count,
				 const GLfloat *params)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat * dest;

   flush_vertices_for_program_constants(ctx, target);

   if (count <= 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glProgramEnvParameters4fv(count)");
   }

   if (target == GL_FRAGMENT_PROGRAM_ARB
       && ctx->Extensions.ARB_fragment_program) {
      if ((index + count) > ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxEnvParams) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glProgramEnvParameters4fv(index + count)");
         return;
      }
      dest = ctx->FragmentProgram.Parameters[index];
   }
   else if (target == GL_VERTEX_PROGRAM_ARB
       && ctx->Extensions.ARB_vertex_program) {
      if ((index + count) > ctx->Const.Program[MESA_SHADER_VERTEX].MaxEnvParams) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glProgramEnvParameters4fv(index + count)");
         return;
      }
      dest = ctx->VertexProgram.Parameters[index];
   }
   else {
      _mesa_error(ctx, GL_INVALID_ENUM, "glProgramEnvParameters4fv(target)");
      return;
   }

   memcpy(dest, params, count * 4 * sizeof(GLfloat));
}


void GLAPIENTRY
_mesa_GetProgramEnvParameterdvARB(GLenum target, GLuint index,
                                  GLdouble *params)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat *fparam;

   if (get_env_param_pointer(ctx, "glGetProgramEnvParameterdv",
			     target, index, &fparam)) {
      COPY_4V(params, fparam);
   }
}


void GLAPIENTRY
_mesa_GetProgramEnvParameterfvARB(GLenum target, GLuint index,
                                  GLfloat *params)
{
   GLfloat *param;

   GET_CURRENT_CONTEXT(ctx);

   if (get_env_param_pointer(ctx, "glGetProgramEnvParameterfv",
			      target, index, &param)) {
      COPY_4V(params, param);
   }
}


void GLAPIENTRY
_mesa_ProgramLocalParameter4fARB(GLenum target, GLuint index,
                                 GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat *param;
   struct gl_program* prog = get_current_program(ctx, target, "glProgramLocalParameterARB");
   if (!prog) {
      return;
   }

   flush_vertices_for_program_constants(ctx, target);

   if (get_local_param_pointer(ctx, "glProgramLocalParameterARB",
			       prog, target, index, 1, &param)) {
      assert(index < MAX_PROGRAM_LOCAL_PARAMS);
      ASSIGN_4V(param, x, y, z, w);
   }
}

void GLAPIENTRY
_mesa_NamedProgramLocalParameter4fEXT(GLuint program, GLenum target, GLuint index,
                                      GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat *param;
   struct gl_program* prog = lookup_or_create_program(program, target,
                                                      "glNamedProgramLocalParameter4fEXT");

   if (!prog) {
      return;
   }

   if ((target == GL_VERTEX_PROGRAM_ARB && prog == ctx->VertexProgram.Current) ||
       (target == GL_FRAGMENT_PROGRAM_ARB && prog == ctx->FragmentProgram.Current)) {
      flush_vertices_for_program_constants(ctx, target);
   }

   if (get_local_param_pointer(ctx, "glNamedProgramLocalParameter4fEXT",
                prog, target, index, 1, &param)) {
      assert(index < MAX_PROGRAM_LOCAL_PARAMS);
      ASSIGN_4V(param, x, y, z, w);
   }
}


void GLAPIENTRY
_mesa_ProgramLocalParameter4fvARB(GLenum target, GLuint index,
                                  const GLfloat *params)
{
   _mesa_ProgramLocalParameter4fARB(target, index, params[0], params[1],
                                    params[2], params[3]);
}


void GLAPIENTRY
_mesa_NamedProgramLocalParameter4fvEXT(GLuint program, GLenum target, GLuint index,
                                  const GLfloat *params)
{
   _mesa_NamedProgramLocalParameter4fEXT(program, target, index, params[0],
                                         params[1], params[2], params[3]);
}


static void
program_local_parameters4fv(struct gl_program* prog, GLuint index, GLsizei count,
                            const GLfloat *params, const char* caller)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat *dest;
   flush_vertices_for_program_constants(ctx, prog->Target);

   if (count <= 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(count)", caller);
   }

   if (get_local_param_pointer(ctx, caller,
                               prog, prog->Target, index, count, &dest))
      memcpy(dest, params, count * 4 * sizeof(GLfloat));
}


void GLAPIENTRY
_mesa_ProgramLocalParameters4fvEXT(GLenum target, GLuint index, GLsizei count,
				   const GLfloat *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_program* prog = get_current_program(ctx, target,
                                                 "glProgramLocalParameters4fv");
   if (!prog) {
      return;
   }

   program_local_parameters4fv(prog, index, count, params,
                               "glProgramLocalParameters4fv");
}

void GLAPIENTRY
_mesa_NamedProgramLocalParameters4fvEXT(GLuint program, GLenum target, GLuint index,
                                        GLsizei count, const GLfloat *params)
{
   struct gl_program* prog =
      lookup_or_create_program(program, target,
                               "glNamedProgramLocalParameters4fvEXT");
   if (!prog) {
      return;
   }

   program_local_parameters4fv(prog, index, count, params,
                               "glNamedProgramLocalParameters4fvEXT");
}


void GLAPIENTRY
_mesa_ProgramLocalParameter4dARB(GLenum target, GLuint index,
                                 GLdouble x, GLdouble y,
                                 GLdouble z, GLdouble w)
{
   _mesa_ProgramLocalParameter4fARB(target, index, (GLfloat) x, (GLfloat) y,
                                    (GLfloat) z, (GLfloat) w);
}


void GLAPIENTRY
_mesa_NamedProgramLocalParameter4dEXT(GLuint program, GLenum target, GLuint index,
                                      GLdouble x, GLdouble y,
                                      GLdouble z, GLdouble w)
{
   _mesa_NamedProgramLocalParameter4fEXT(program, target, index, (GLfloat) x, (GLfloat) y,
                                         (GLfloat) z, (GLfloat) w);
}


void GLAPIENTRY
_mesa_ProgramLocalParameter4dvARB(GLenum target, GLuint index,
                                  const GLdouble *params)
{
   _mesa_ProgramLocalParameter4fARB(target, index,
                                    (GLfloat) params[0], (GLfloat) params[1],
                                    (GLfloat) params[2], (GLfloat) params[3]);
}


void GLAPIENTRY
_mesa_NamedProgramLocalParameter4dvEXT(GLuint program, GLenum target, GLuint index,
                                       const GLdouble *params)
{
   _mesa_NamedProgramLocalParameter4fEXT(program, target, index,
                                         (GLfloat) params[0], (GLfloat) params[1],
                                         (GLfloat) params[2], (GLfloat) params[3]);
}


void GLAPIENTRY
_mesa_GetProgramLocalParameterfvARB(GLenum target, GLuint index,
                                    GLfloat *params)
{
   GLfloat *param;
   GET_CURRENT_CONTEXT(ctx);
   struct gl_program* prog = get_current_program(ctx, target, "glGetProgramLocalParameterfvARB");
   if (!prog) {
      return;
   }

   if (get_local_param_pointer(ctx, "glProgramLocalParameters4fvEXT",
				prog, target, index, 1, &param)) {
      COPY_4V(params, param);
   }
}


void GLAPIENTRY
_mesa_GetNamedProgramLocalParameterfvEXT(GLuint program, GLenum target, GLuint index,
                                         GLfloat *params)
{
   GLfloat *param;
   GET_CURRENT_CONTEXT(ctx);
   struct gl_program* prog = lookup_or_create_program(program, target,
                                                      "glGetNamedProgramLocalParameterfvEXT");
   if (!prog) {
      return;
   }

   if (get_local_param_pointer(ctx, "glGetNamedProgramLocalParameterfvEXT",
            prog, target, index, 1, &param)) {
      COPY_4V(params, param);
   }
}


void GLAPIENTRY
_mesa_GetProgramLocalParameterdvARB(GLenum target, GLuint index,
                                    GLdouble *params)
{
   GLfloat *param;
   GET_CURRENT_CONTEXT(ctx);
   struct gl_program* prog = get_current_program(ctx, target, "glGetProgramLocalParameterdvARB");
   if (!prog) {
      return;
   }

   if (get_local_param_pointer(ctx, "glProgramLocalParameters4fvEXT",
				prog, target, index, 1, &param)) {
      COPY_4V(params, param);
   }
}


void GLAPIENTRY
_mesa_GetNamedProgramLocalParameterdvEXT(GLuint program, GLenum target, GLuint index,
                                         GLdouble *params)
{
   GLfloat *param;
   GET_CURRENT_CONTEXT(ctx);
   struct gl_program* prog = lookup_or_create_program(program, target,
                                                      "glGetNamedProgramLocalParameterdvEXT");
   if (!prog) {
      return;
   }

   if (get_local_param_pointer(ctx, "glGetNamedProgramLocalParameterdvEXT",
            prog, target, index, 1, &param)) {
      COPY_4V(params, param);
   }
}


static void
get_program_iv(struct gl_program *prog, GLenum target, GLenum pname,
               GLint *params)
{
   const struct gl_program_constants *limits;

   GET_CURRENT_CONTEXT(ctx);

   if (target == GL_VERTEX_PROGRAM_ARB) {
      limits = &ctx->Const.Program[MESA_SHADER_VERTEX];
   }
   else {
      limits = &ctx->Const.Program[MESA_SHADER_FRAGMENT];
   }

   assert(prog);
   assert(limits);

   /* Queries supported for both vertex and fragment programs */
   switch (pname) {
      case GL_PROGRAM_LENGTH_ARB:
         *params
            = prog->String ? (GLint) strlen((char *) prog->String) : 0;
         return;
      case GL_PROGRAM_FORMAT_ARB:
         *params = prog->Format;
         return;
      case GL_PROGRAM_BINDING_ARB:
         *params = prog->Id;
         return;
      case GL_PROGRAM_INSTRUCTIONS_ARB:
         *params = prog->arb.NumInstructions;
         return;
      case GL_MAX_PROGRAM_INSTRUCTIONS_ARB:
         *params = limits->MaxInstructions;
         return;
      case GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB:
         *params = prog->arb.NumNativeInstructions;
         return;
      case GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB:
         *params = limits->MaxNativeInstructions;
         return;
      case GL_PROGRAM_TEMPORARIES_ARB:
         *params = prog->arb.NumTemporaries;
         return;
      case GL_MAX_PROGRAM_TEMPORARIES_ARB:
         *params = limits->MaxTemps;
         return;
      case GL_PROGRAM_NATIVE_TEMPORARIES_ARB:
         *params = prog->arb.NumNativeTemporaries;
         return;
      case GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB:
         *params = limits->MaxNativeTemps;
         return;
      case GL_PROGRAM_PARAMETERS_ARB:
         *params = prog->arb.NumParameters;
         return;
      case GL_MAX_PROGRAM_PARAMETERS_ARB:
         *params = limits->MaxParameters;
         return;
      case GL_PROGRAM_NATIVE_PARAMETERS_ARB:
         *params = prog->arb.NumNativeParameters;
         return;
      case GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB:
         *params = limits->MaxNativeParameters;
         return;
      case GL_PROGRAM_ATTRIBS_ARB:
         *params = prog->arb.NumAttributes;
         return;
      case GL_MAX_PROGRAM_ATTRIBS_ARB:
         *params = limits->MaxAttribs;
         return;
      case GL_PROGRAM_NATIVE_ATTRIBS_ARB:
         *params = prog->arb.NumNativeAttributes;
         return;
      case GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB:
         *params = limits->MaxNativeAttribs;
         return;
      case GL_PROGRAM_ADDRESS_REGISTERS_ARB:
         *params = prog->arb.NumAddressRegs;
         return;
      case GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB:
         *params = limits->MaxAddressRegs;
         return;
      case GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB:
         *params = prog->arb.NumNativeAddressRegs;
         return;
      case GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB:
         *params = limits->MaxNativeAddressRegs;
         return;
      case GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB:
         *params = limits->MaxLocalParams;
         return;
      case GL_MAX_PROGRAM_ENV_PARAMETERS_ARB:
         *params = limits->MaxEnvParams;
         return;
      case GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB:
         /*
          * XXX we may not really need a driver callback here.
          * If the number of native instructions, registers, etc. used
          * are all below the maximums, we could return true.
          * The spec says that even if this query returns true, there's
          * no guarantee that the program will run in hardware.
          */
         if (prog->Id == 0) {
            /* default/null program */
            *params = GL_FALSE;
         }
	 else {
            /* probably running in software */
	    *params = GL_TRUE;
         }
         return;
      default:
         /* continue with fragment-program only queries below */
         break;
   }

   /*
    * The following apply to fragment programs only (at this time)
    */
   if (target == GL_FRAGMENT_PROGRAM_ARB) {
      const struct gl_program *fp = ctx->FragmentProgram.Current;
      switch (pname) {
         case GL_PROGRAM_ALU_INSTRUCTIONS_ARB:
            *params = fp->arb.NumNativeAluInstructions;
            return;
         case GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB:
            *params = fp->arb.NumAluInstructions;
            return;
         case GL_PROGRAM_TEX_INSTRUCTIONS_ARB:
            *params = fp->arb.NumTexInstructions;
            return;
         case GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB:
            *params = fp->arb.NumNativeTexInstructions;
            return;
         case GL_PROGRAM_TEX_INDIRECTIONS_ARB:
            *params = fp->arb.NumTexIndirections;
            return;
         case GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB:
            *params = fp->arb.NumNativeTexIndirections;
            return;
         case GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB:
            *params = limits->MaxAluInstructions;
            return;
         case GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB:
            *params = limits->MaxNativeAluInstructions;
            return;
         case GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB:
            *params = limits->MaxTexInstructions;
            return;
         case GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB:
            *params = limits->MaxNativeTexInstructions;
            return;
         case GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB:
            *params = limits->MaxTexIndirections;
            return;
         case GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB:
            *params = limits->MaxNativeTexIndirections;
            return;
         default:
            _mesa_error(ctx, GL_INVALID_ENUM, "glGetProgramivARB(pname)");
            return;
      }
   } else {
      _mesa_error(ctx, GL_INVALID_ENUM, "glGetProgramivARB(pname)");
      return;
   }
}


void GLAPIENTRY
_mesa_GetProgramivARB(GLenum target, GLenum pname, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_program* prog = get_current_program(ctx, target,
                                                 "glGetProgramivARB");
   if (!prog) {
      return;
   }
   get_program_iv(prog, target, pname, params);
}

void GLAPIENTRY
_mesa_GetNamedProgramivEXT(GLuint program, GLenum target, GLenum pname,
                           GLint *params)
{
   struct gl_program* prog;
   if (pname == GL_PROGRAM_BINDING_ARB) {
      _mesa_GetProgramivARB(target, pname, params);
      return;
   }
   prog = lookup_or_create_program(program, target,
                                                      "glGetNamedProgramivEXT");
   if (!prog) {
      return;
   }
   get_program_iv(prog, target, pname, params);
}


void GLAPIENTRY
_mesa_GetProgramStringARB(GLenum target, GLenum pname, GLvoid *string)
{
   const struct gl_program *prog;
   char *dst = (char *) string;
   GET_CURRENT_CONTEXT(ctx);

   if (target == GL_VERTEX_PROGRAM_ARB) {
      prog = ctx->VertexProgram.Current;
   }
   else if (target == GL_FRAGMENT_PROGRAM_ARB) {
      prog = ctx->FragmentProgram.Current;
   }
   else {
      _mesa_error(ctx, GL_INVALID_ENUM, "glGetProgramStringARB(target)");
      return;
   }

   assert(prog);

   if (pname != GL_PROGRAM_STRING_ARB) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glGetProgramStringARB(pname)");
      return;
   }

   if (prog->String)
      memcpy(dst, prog->String, strlen((char *) prog->String));
   else
      *dst = '\0';
}


void GLAPIENTRY
_mesa_GetNamedProgramStringEXT(GLuint program, GLenum target,
                               GLenum pname, GLvoid *string) {
   char *dst = (char *) string;
   GET_CURRENT_CONTEXT(ctx);
   struct gl_program* prog = lookup_or_create_program(program, target,
                                                      "glGetNamedProgramStringEXT");
   if (!prog)
      return;

   if (pname != GL_PROGRAM_STRING_ARB) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glGetNamedProgramStringEXT(pname)");
      return;
   }

   if (prog->String)
      memcpy(dst, prog->String, strlen((char *) prog->String));
   else
      *dst = '\0';
}
