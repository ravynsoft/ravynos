/*
 * Mesa 3-D graphics library
 *
 * Copyright © 2013 Gregory Hainaut <gregory.hainaut@gmail.com>
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

/**
 * \file pipelineobj.c
 * \author Hainaut Gregory <gregory.hainaut@gmail.com>
 *
 * Implementation of pipeline object related API functions. Based on
 * GL_ARB_separate_shader_objects extension.
 */

#include <stdbool.h>
#include "util/glheader.h"
#include "main/context.h"
#include "main/draw_validate.h"
#include "main/enums.h"
#include "main/hash.h"
#include "main/mtypes.h"
#include "main/pipelineobj.h"
#include "main/shaderapi.h"
#include "main/shaderobj.h"
#include "main/state.h"
#include "main/transformfeedback.h"
#include "main/uniforms.h"
#include "compiler/glsl/glsl_parser_extras.h"
#include "compiler/glsl/ir_uniform.h"
#include "program/program.h"
#include "program/prog_parameter.h"
#include "util/ralloc.h"
#include "util/bitscan.h"
#include "api_exec_decl.h"

/**
 * Delete a pipeline object.
 */
void
_mesa_delete_pipeline_object(struct gl_context *ctx,
                             struct gl_pipeline_object *obj)
{
   unsigned i;

   for (i = 0; i < MESA_SHADER_STAGES; i++) {
      _mesa_reference_program(ctx, &obj->CurrentProgram[i], NULL);
      _mesa_reference_shader_program(ctx, &obj->ReferencedPrograms[i], NULL);
   }

   _mesa_reference_shader_program(ctx, &obj->ActiveProgram, NULL);
   free(obj->Label);
   ralloc_free(obj);
}

/**
 * Allocate and initialize a new pipeline object.
 */
static struct gl_pipeline_object *
_mesa_new_pipeline_object(struct gl_context *ctx, GLuint name)
{
   struct gl_pipeline_object *obj = rzalloc(NULL, struct gl_pipeline_object);
   if (obj) {
      obj->Name = name;
      obj->RefCount = 1;
      obj->Flags = _mesa_get_shader_flags();
      obj->InfoLog = NULL;
   }

   return obj;
}

/**
 * Initialize pipeline object state for given context.
 */
void
_mesa_init_pipeline(struct gl_context *ctx)
{
   ctx->Pipeline.Objects = _mesa_NewHashTable();

   ctx->Pipeline.Current = NULL;

   /* Install a default Pipeline */
   ctx->Pipeline.Default = _mesa_new_pipeline_object(ctx, 0);
   _mesa_reference_pipeline_object(ctx, &ctx->_Shader, ctx->Pipeline.Default);
}


/**
 * Callback for deleting a pipeline object.  Called by _mesa_HashDeleteAll().
 */
static void
delete_pipelineobj_cb(void *data, void *userData)
{
   struct gl_pipeline_object *obj = (struct gl_pipeline_object *) data;
   struct gl_context *ctx = (struct gl_context *) userData;
   _mesa_delete_pipeline_object(ctx, obj);
}


/**
 * Free pipeline state for given context.
 */
void
_mesa_free_pipeline_data(struct gl_context *ctx)
{
   _mesa_reference_pipeline_object(ctx, &ctx->_Shader, NULL);

   _mesa_HashDeleteAll(ctx->Pipeline.Objects, delete_pipelineobj_cb, ctx);
   _mesa_DeleteHashTable(ctx->Pipeline.Objects);

   _mesa_delete_pipeline_object(ctx, ctx->Pipeline.Default);
}

/**
 * Look up the pipeline object for the given ID.
 *
 * \returns
 * Either a pointer to the pipeline object with the specified ID or \c NULL for
 * a non-existent ID.  The spec defines ID 0 as being technically
 * non-existent.
 */
struct gl_pipeline_object *
_mesa_lookup_pipeline_object(struct gl_context *ctx, GLuint id)
{
   if (id == 0)
      return NULL;
   else
      return (struct gl_pipeline_object *)
         _mesa_HashLookupLocked(ctx->Pipeline.Objects, id);
}

/**
 * Add the given pipeline object to the pipeline object pool.
 */
static void
save_pipeline_object(struct gl_context *ctx, struct gl_pipeline_object *obj)
{
   if (obj->Name > 0) {
      _mesa_HashInsertLocked(ctx->Pipeline.Objects, obj->Name, obj, true);
   }
}

/**
 * Remove the given pipeline object from the pipeline object pool.
 * Do not deallocate the pipeline object though.
 */
static void
remove_pipeline_object(struct gl_context *ctx, struct gl_pipeline_object *obj)
{
   if (obj->Name > 0) {
      _mesa_HashRemoveLocked(ctx->Pipeline.Objects, obj->Name);
   }
}

/**
 * Set ptr to obj w/ reference counting.
 * Note: this should only be called from the _mesa_reference_pipeline_object()
 * inline function.
 */
void
_mesa_reference_pipeline_object_(struct gl_context *ctx,
                                 struct gl_pipeline_object **ptr,
                                 struct gl_pipeline_object *obj)
{
   assert(*ptr != obj);

   if (*ptr) {
      /* Unreference the old pipeline object */
      struct gl_pipeline_object *oldObj = *ptr;

      assert(oldObj->RefCount > 0);
      oldObj->RefCount--;

      if (oldObj->RefCount == 0) {
         _mesa_delete_pipeline_object(ctx, oldObj);
      }

      *ptr = NULL;
   }
   assert(!*ptr);

   if (obj) {
      /* reference new pipeline object */
      assert(obj->RefCount > 0);

      obj->RefCount++;
      *ptr = obj;
   }
}

static void
use_program_stage(struct gl_context *ctx, GLenum type,
                  struct gl_shader_program *shProg,
                  struct gl_pipeline_object *pipe) {
   gl_shader_stage stage = _mesa_shader_enum_to_shader_stage(type);
   struct gl_program *prog = NULL;
   if (shProg && shProg->_LinkedShaders[stage])
      prog = shProg->_LinkedShaders[stage]->Program;

   _mesa_use_program(ctx, stage, shProg, prog, pipe);
}

static void
use_program_stages(struct gl_context *ctx, struct gl_shader_program *shProg,
                   GLbitfield stages, struct gl_pipeline_object *pipe) {

   /* Enable individual stages from the program as requested by the
    * application.  If there is no shader for a requested stage in the
    * program, _mesa_use_shader_program will enable fixed-function processing
    * as dictated by the spec.
    *
    * Section 2.11.4 (Program Pipeline Objects) of the OpenGL 4.1 spec
    * says:
    *
    *     "If UseProgramStages is called with program set to zero or with a
    *     program object that contains no executable code for the given
    *     stages, it is as if the pipeline object has no programmable stage
    *     configured for the indicated shader stages."
    */
   if ((stages & GL_VERTEX_SHADER_BIT) != 0)
      use_program_stage(ctx, GL_VERTEX_SHADER, shProg, pipe);

   if ((stages & GL_FRAGMENT_SHADER_BIT) != 0)
      use_program_stage(ctx, GL_FRAGMENT_SHADER, shProg, pipe);

   if ((stages & GL_GEOMETRY_SHADER_BIT) != 0)
      use_program_stage(ctx, GL_GEOMETRY_SHADER, shProg, pipe);

   if ((stages & GL_TESS_CONTROL_SHADER_BIT) != 0)
      use_program_stage(ctx, GL_TESS_CONTROL_SHADER, shProg, pipe);

   if ((stages & GL_TESS_EVALUATION_SHADER_BIT) != 0)
      use_program_stage(ctx, GL_TESS_EVALUATION_SHADER, shProg, pipe);

   if ((stages & GL_COMPUTE_SHADER_BIT) != 0)
      use_program_stage(ctx, GL_COMPUTE_SHADER, shProg, pipe);

   pipe->Validated = pipe->UserValidated = false;

   if (pipe == ctx->_Shader)
      _mesa_update_valid_to_render_state(ctx);
}

void GLAPIENTRY
_mesa_UseProgramStages_no_error(GLuint pipeline, GLbitfield stages,
                                GLuint prog)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_pipeline_object *pipe =
      _mesa_lookup_pipeline_object(ctx, pipeline);
   struct gl_shader_program *shProg = NULL;

   if (prog)
      shProg = _mesa_lookup_shader_program(ctx, prog);

   /* Object is created by any Pipeline call but glGenProgramPipelines,
    * glIsProgramPipeline and GetProgramPipelineInfoLog
    */
   pipe->EverBound = GL_TRUE;

   use_program_stages(ctx, shProg, stages, pipe);
}

/**
 * Bound program to severals stages of the pipeline
 */
void GLAPIENTRY
_mesa_UseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_pipeline_object *pipe = _mesa_lookup_pipeline_object(ctx, pipeline);
   struct gl_shader_program *shProg = NULL;
   GLbitfield any_valid_stages;

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glUseProgramStages(%u, 0x%x, %u)\n",
                  pipeline, stages, program);

   if (!pipe) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glUseProgramStages(pipeline)");
      return;
   }

   /* Object is created by any Pipeline call but glGenProgramPipelines,
    * glIsProgramPipeline and GetProgramPipelineInfoLog
    */
   pipe->EverBound = GL_TRUE;

   /* Section 2.11.4 (Program Pipeline Objects) of the OpenGL 4.1 spec says:
    *
    *     "If stages is not the special value ALL_SHADER_BITS, and has a bit
    *     set that is not recognized, the error INVALID_VALUE is generated."
    */
   any_valid_stages = GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT;
   if (_mesa_has_geometry_shaders(ctx))
      any_valid_stages |= GL_GEOMETRY_SHADER_BIT;
   if (_mesa_has_tessellation(ctx))
      any_valid_stages |= GL_TESS_CONTROL_SHADER_BIT |
                          GL_TESS_EVALUATION_SHADER_BIT;
   if (_mesa_has_compute_shaders(ctx))
      any_valid_stages |= GL_COMPUTE_SHADER_BIT;

   if (stages != GL_ALL_SHADER_BITS && (stages & ~any_valid_stages) != 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glUseProgramStages(Stages)");
      return;
   }

   /* Section 2.17.2 (Transform Feedback Primitive Capture) of the OpenGL 4.1
    * spec says:
    *
    *     "The error INVALID_OPERATION is generated:
    *
    *      ...
    *
    *         - by UseProgramStages if the program pipeline object it refers
    *           to is current and the current transform feedback object is
    *           active and not paused;
    */
   if (ctx->_Shader == pipe) {
      if (_mesa_is_xfb_active_and_unpaused(ctx)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
               "glUseProgramStages(transform feedback active)");
         return;
      }
   }

   if (program) {
      shProg = _mesa_lookup_shader_program_err(ctx, program,
                                               "glUseProgramStages");
      if (shProg == NULL)
         return;

      /* Section 2.11.4 (Program Pipeline Objects) of the OpenGL 4.1 spec
       * says:
       *
       *     "If the program object named by program was linked without the
       *     PROGRAM_SEPARABLE parameter set, or was not linked successfully,
       *     the error INVALID_OPERATION is generated and the corresponding
       *     shader stages in the pipeline program pipeline object are not
       *     modified."
       */
      if (!shProg->data->LinkStatus) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glUseProgramStages(program not linked)");
         return;
      }

      if (!shProg->SeparateShader) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glUseProgramStages(program wasn't linked with the "
                     "PROGRAM_SEPARABLE flag)");
         return;
      }
   }

   use_program_stages(ctx, shProg, stages, pipe);
}

static ALWAYS_INLINE void
active_shader_program(struct gl_context *ctx, GLuint pipeline, GLuint program,
                      bool no_error)
{
   struct gl_shader_program *shProg = NULL;
   struct gl_pipeline_object *pipe = _mesa_lookup_pipeline_object(ctx, pipeline);

   if (program) {
      if (no_error) {
         shProg = _mesa_lookup_shader_program(ctx, program);
      } else {
         shProg = _mesa_lookup_shader_program_err(ctx, program,
                                                  "glActiveShaderProgram(program)");
         if (shProg == NULL)
            return;
      }
   }

   if (!no_error && !pipe) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glActiveShaderProgram(pipeline)");
      return;
   }

   /* Object is created by any Pipeline call but glGenProgramPipelines,
    * glIsProgramPipeline and GetProgramPipelineInfoLog
    */
   pipe->EverBound = GL_TRUE;

   if (!no_error && shProg != NULL && !shProg->data->LinkStatus) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
            "glActiveShaderProgram(program %u not linked)", shProg->Name);
      return;
   }

   _mesa_reference_shader_program(ctx, &pipe->ActiveProgram, shProg);
   if (pipe == ctx->_Shader)
      _mesa_update_valid_to_render_state(ctx);
}

void GLAPIENTRY
_mesa_ActiveShaderProgram_no_error(GLuint pipeline, GLuint program)
{
   GET_CURRENT_CONTEXT(ctx);
   active_shader_program(ctx, pipeline, program, true);
}

/**
 * Use the named shader program for subsequent glUniform calls (if pipeline
 * bound)
 */
void GLAPIENTRY
_mesa_ActiveShaderProgram(GLuint pipeline, GLuint program)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glActiveShaderProgram(%u, %u)\n", pipeline, program);

   active_shader_program(ctx, pipeline, program, false);
}

static ALWAYS_INLINE void
bind_program_pipeline(struct gl_context *ctx, GLuint pipeline, bool no_error)
{
   struct gl_pipeline_object *newObj = NULL;

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glBindProgramPipeline(%u)\n", pipeline);

   /* Rebinding the same pipeline object: no change.
    */
   if (ctx->_Shader->Name == pipeline)
      return;

   /* Section 2.17.2 (Transform Feedback Primitive Capture) of the OpenGL 4.1
    * spec says:
    *
    *     "The error INVALID_OPERATION is generated:
    *
    *      ...
    *
    *         - by BindProgramPipeline if the current transform feedback
    *           object is active and not paused;
    */
   if (!no_error && _mesa_is_xfb_active_and_unpaused(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
            "glBindProgramPipeline(transform feedback active)");
      return;
   }

   /* Get pointer to new pipeline object (newObj)
    */
   if (pipeline) {
      /* non-default pipeline object */
      newObj = _mesa_lookup_pipeline_object(ctx, pipeline);
      if (!no_error && !newObj) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glBindProgramPipeline(non-gen name)");
         return;
      }

      /* Object is created by any Pipeline call but glGenProgramPipelines,
       * glIsProgramPipeline and GetProgramPipelineInfoLog
       */
      newObj->EverBound = GL_TRUE;
   }

   _mesa_bind_pipeline(ctx, newObj);
}

void GLAPIENTRY
_mesa_BindProgramPipeline_no_error(GLuint pipeline)
{
   GET_CURRENT_CONTEXT(ctx);
   bind_program_pipeline(ctx, pipeline, true);
}

/**
 * Make program of the pipeline current
 */
void GLAPIENTRY
_mesa_BindProgramPipeline(GLuint pipeline)
{
   GET_CURRENT_CONTEXT(ctx);
   bind_program_pipeline(ctx, pipeline, false);
}

void
_mesa_bind_pipeline(struct gl_context *ctx,
                    struct gl_pipeline_object *pipe)
{
   int i;
   /* First bind the Pipeline to pipeline binding point */
   _mesa_reference_pipeline_object(ctx, &ctx->Pipeline.Current, pipe);

   /* Section 2.11.3 (Program Objects) of the OpenGL 4.1 spec says:
    *
    *     "If there is a current program object established by UseProgram,
    *     that program is considered current for all stages. Otherwise, if
    *     there is a bound program pipeline object (see section 2.11.4), the
    *     program bound to the appropriate stage of the pipeline object is
    *     considered current."
    */
   if (&ctx->Shader != ctx->_Shader) {
      FLUSH_VERTICES(ctx, _NEW_PROGRAM | _NEW_PROGRAM_CONSTANTS, 0);

      if (pipe != NULL) {
         /* Bound the pipeline to the current program and
          * restore the pipeline state
          */
         _mesa_reference_pipeline_object(ctx, &ctx->_Shader, pipe);
      } else {
         /* Unbind the pipeline */
         _mesa_reference_pipeline_object(ctx, &ctx->_Shader,
                                         ctx->Pipeline.Default);
      }

      for (i = 0; i < MESA_SHADER_STAGES; i++) {
         struct gl_program *prog = ctx->_Shader->CurrentProgram[i];
         if (prog) {
            _mesa_program_init_subroutine_defaults(ctx, prog);
         }
      }

      _mesa_update_vertex_processing_mode(ctx);
      _mesa_update_allow_draw_out_of_order(ctx);
      _mesa_update_valid_to_render_state(ctx);
   }
}

/**
 * Delete a set of pipeline objects.
 *
 * \param n      Number of pipeline objects to delete.
 * \param ids    pipeline of \c n pipeline object IDs.
 */
void GLAPIENTRY
_mesa_DeleteProgramPipelines(GLsizei n, const GLuint *pipelines)
{
   GET_CURRENT_CONTEXT(ctx);
   GLsizei i;

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glDeleteProgramPipelines(%d, %p)\n", n, pipelines);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDeleteProgramPipelines(n<0)");
      return;
   }

   for (i = 0; i < n; i++) {
      struct gl_pipeline_object *obj =
         _mesa_lookup_pipeline_object(ctx, pipelines[i]);

      if (obj) {
         assert(obj->Name == pipelines[i]);

         /* If the pipeline object is currently bound, the spec says "If an
          * object that is currently bound is deleted, the binding for that
          * object reverts to zero and no program pipeline object becomes
          * current."
          */
         if (obj == ctx->Pipeline.Current) {
            _mesa_BindProgramPipeline(0);
         }

         /* The ID is immediately freed for re-use */
         remove_pipeline_object(ctx, obj);

         /* Unreference the pipeline object.
          * If refcount hits zero, the object will be deleted.
          */
         _mesa_reference_pipeline_object(ctx, &obj, NULL);
      }
   }
}

/**
 * Generate a set of unique pipeline object IDs and store them in \c pipelines.
 * \param n       Number of IDs to generate.
 * \param pipelines  pipeline of \c n locations to store the IDs.
 */
static void
create_program_pipelines(struct gl_context *ctx, GLsizei n, GLuint *pipelines,
                         bool dsa)
{
   const char *func = dsa ? "glCreateProgramPipelines" : "glGenProgramPipelines";
   GLint i;

   if (!pipelines)
      return;

   _mesa_HashFindFreeKeys(ctx->Pipeline.Objects, pipelines, n);

   for (i = 0; i < n; i++) {
      struct gl_pipeline_object *obj;

      obj = _mesa_new_pipeline_object(ctx, pipelines[i]);
      if (!obj) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", func);
         return;
      }

      if (dsa) {
         /* make dsa-allocated objects behave like program objects */
         obj->EverBound = GL_TRUE;
      }

      save_pipeline_object(ctx, obj);
   }
}

static void
create_program_pipelines_err(struct gl_context *ctx, GLsizei n,
                             GLuint *pipelines, bool dsa)
{
   const char *func = dsa ? "glCreateProgramPipelines" : "glGenProgramPipelines";

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s (n < 0)", func);
      return;
   }

   create_program_pipelines(ctx, n, pipelines, dsa);
}

void GLAPIENTRY
_mesa_GenProgramPipelines_no_error(GLsizei n, GLuint *pipelines)
{
   GET_CURRENT_CONTEXT(ctx);
   create_program_pipelines(ctx, n, pipelines, false);
}

void GLAPIENTRY
_mesa_GenProgramPipelines(GLsizei n, GLuint *pipelines)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glGenProgramPipelines(%d, %p)\n", n, pipelines);

   create_program_pipelines_err(ctx, n, pipelines, false);
}

void GLAPIENTRY
_mesa_CreateProgramPipelines_no_error(GLsizei n, GLuint *pipelines)
{
   GET_CURRENT_CONTEXT(ctx);
   create_program_pipelines(ctx, n, pipelines, true);
}

void GLAPIENTRY
_mesa_CreateProgramPipelines(GLsizei n, GLuint *pipelines)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glCreateProgramPipelines(%d, %p)\n", n, pipelines);

   create_program_pipelines_err(ctx, n, pipelines, true);
}

/**
 * Determine if ID is the name of an pipeline object.
 *
 * \param id  ID of the potential pipeline object.
 * \return  \c GL_TRUE if \c id is the name of a pipeline object,
 *          \c GL_FALSE otherwise.
 */
GLboolean GLAPIENTRY
_mesa_IsProgramPipeline(GLuint pipeline)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glIsProgramPipeline(%u)\n", pipeline);

   struct gl_pipeline_object *obj = _mesa_lookup_pipeline_object(ctx, pipeline);
   if (obj == NULL)
      return GL_FALSE;

   return obj->EverBound;
}

/**
 * glGetProgramPipelineiv() - get pipeline shader state.
 */
void GLAPIENTRY
_mesa_GetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_pipeline_object *pipe = _mesa_lookup_pipeline_object(ctx, pipeline);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glGetProgramPipelineiv(%u, %d, %p)\n",
                  pipeline, pname, params);

   /* Are geometry shaders available in this context?
    */
   const bool has_gs = _mesa_has_geometry_shaders(ctx);
   const bool has_tess = _mesa_has_tessellation(ctx);

   if (!pipe) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetProgramPipelineiv(pipeline)");
      return;
   }

   /* Object is created by any Pipeline call but glGenProgramPipelines,
    * glIsProgramPipeline and GetProgramPipelineInfoLog
    */
   pipe->EverBound = GL_TRUE;

   switch (pname) {
   case GL_ACTIVE_PROGRAM:
      *params = pipe->ActiveProgram ? pipe->ActiveProgram->Name : 0;
      return;
   case GL_INFO_LOG_LENGTH:
      *params = (pipe->InfoLog && pipe->InfoLog[0] != '\0') ?
         strlen(pipe->InfoLog) + 1 : 0;
      return;
   case GL_VALIDATE_STATUS:
      *params = pipe->UserValidated;
      return;
   case GL_VERTEX_SHADER:
      *params = pipe->CurrentProgram[MESA_SHADER_VERTEX]
         ? pipe->CurrentProgram[MESA_SHADER_VERTEX]->Id : 0;
      return;
   case GL_TESS_EVALUATION_SHADER:
      if (!has_tess)
         break;
      *params = pipe->CurrentProgram[MESA_SHADER_TESS_EVAL]
         ? pipe->CurrentProgram[MESA_SHADER_TESS_EVAL]->Id : 0;
      return;
   case GL_TESS_CONTROL_SHADER:
      if (!has_tess)
         break;
      *params = pipe->CurrentProgram[MESA_SHADER_TESS_CTRL]
         ? pipe->CurrentProgram[MESA_SHADER_TESS_CTRL]->Id : 0;
      return;
   case GL_GEOMETRY_SHADER:
      if (!has_gs)
         break;
      *params = pipe->CurrentProgram[MESA_SHADER_GEOMETRY]
         ? pipe->CurrentProgram[MESA_SHADER_GEOMETRY]->Id : 0;
      return;
   case GL_FRAGMENT_SHADER:
      *params = pipe->CurrentProgram[MESA_SHADER_FRAGMENT]
         ? pipe->CurrentProgram[MESA_SHADER_FRAGMENT]->Id : 0;
      return;
   case GL_COMPUTE_SHADER:
      if (!_mesa_has_compute_shaders(ctx))
         break;
      *params = pipe->CurrentProgram[MESA_SHADER_COMPUTE]
         ? pipe->CurrentProgram[MESA_SHADER_COMPUTE]->Id : 0;
      return;
   default:
      break;
   }

   _mesa_error(ctx, GL_INVALID_ENUM, "glGetProgramPipelineiv(pname=%s)",
               _mesa_enum_to_string(pname));
}

/**
 * Determines whether every stage in a linked program is active in the
 * specified pipeline.
 */
static bool
program_stages_all_active(struct gl_pipeline_object *pipe,
                          const struct gl_program *prog)
{
   bool status = true;

   if (!prog)
      return true;

   unsigned mask = prog->sh.data->linked_stages;
   while (mask) {
      const int i = u_bit_scan(&mask);
      if (pipe->CurrentProgram[i]) {
         if (prog->Id != pipe->CurrentProgram[i]->Id) {
            status = false;
         }
      } else {
         status = false;
      }
   }

   if (!status) {
      pipe->InfoLog = ralloc_asprintf(pipe,
                                      "Program %d is not active for all "
                                      "shaders that was linked",
                                      prog->Id);
   }

   return status;
}

static bool
program_stages_interleaved_illegally(const struct gl_pipeline_object *pipe)
{
   unsigned prev_linked_stages = 0;

   /* Look for programs bound to stages: A -> B -> A, with any intervening
    * sequence of unrelated programs or empty stages.
    */
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      struct gl_program *cur = pipe->CurrentProgram[i];

      /* Empty stages anywhere in the pipe are OK.  Also we can be confident
       * that if the linked_stages mask matches we are looking at the same
       * linked program because a previous validation call to
       * program_stages_all_active() will have already failed if two different
       * programs with the sames stages linked are not active for all linked
       * stages.
       */
      if (!cur || cur->sh.data->linked_stages == prev_linked_stages)
         continue;

      if (prev_linked_stages) {
         /* We've seen an A -> B transition; look at the rest of the pipe
          * to see if we ever see A again.
          */
         if (prev_linked_stages >> (i + 1))
            return true;
      }

      prev_linked_stages = cur->sh.data->linked_stages;
   }

   return false;
}

extern GLboolean
_mesa_validate_program_pipeline(struct gl_context* ctx,
                                struct gl_pipeline_object *pipe)
{
   unsigned i;
   bool program_empty = true;

   pipe->Validated = GL_FALSE;

   /* Release and reset the info log.
    */
   if (pipe->InfoLog != NULL)
      ralloc_free(pipe->InfoLog);

   pipe->InfoLog = NULL;

   /* Section 2.11.11 (Shader Execution), subheading "Validation," of the
    * OpenGL 4.1 spec says:
    *
    *     "[INVALID_OPERATION] is generated by any command that transfers
    *     vertices to the GL if:
    *
    *         - A program object is active for at least one, but not all of
    *           the shader stages that were present when the program was
    *           linked."
    *
    * For each possible program stage, verify that the program bound to that
    * stage has all of its stages active.  In other words, if the program
    * bound to the vertex stage also has a fragment shader, the fragment
    * shader must also be bound to the fragment stage.
    */
   for (i = 0; i < MESA_SHADER_STAGES; i++) {
      if (!program_stages_all_active(pipe, pipe->CurrentProgram[i])) {
         return GL_FALSE;
      }
   }

   /* Section 2.11.11 (Shader Execution), subheading "Validation," of the
    * OpenGL 4.1 spec says:
    *
    *     "[INVALID_OPERATION] is generated by any command that transfers
    *     vertices to the GL if:
    *
    *         ...
    *
    *         - One program object is active for at least two shader stages
    *           and a second program is active for a shader stage between two
    *           stages for which the first program was active."
    */
   if (program_stages_interleaved_illegally(pipe)) {
      pipe->InfoLog =
         ralloc_strdup(pipe,
                       "Program is active for multiple shader stages with an "
                       "intervening stage provided by another program");
      return GL_FALSE;
   }

   /* Section 2.11.11 (Shader Execution), subheading "Validation," of the
    * OpenGL 4.1 spec says:
    *
    *     "[INVALID_OPERATION] is generated by any command that transfers
    *     vertices to the GL if:
    *
    *         ...
    *
    *         - There is an active program for tessellation control,
    *           tessellation evaluation, or geometry stages with corresponding
    *           executable shader, but there is no active program with
    *           executable vertex shader."
    */
   if (!pipe->CurrentProgram[MESA_SHADER_VERTEX]
       && (pipe->CurrentProgram[MESA_SHADER_GEOMETRY] ||
           pipe->CurrentProgram[MESA_SHADER_TESS_CTRL] ||
           pipe->CurrentProgram[MESA_SHADER_TESS_EVAL])) {
      pipe->InfoLog = ralloc_strdup(pipe, "Program lacks a vertex shader");
      return GL_FALSE;
   }

   /* Section 2.11.11 (Shader Execution), subheading "Validation," of the
    * OpenGL 4.1 spec says:
    *
    *     "[INVALID_OPERATION] is generated by any command that transfers
    *     vertices to the GL if:
    *
    *         ...
    *
    *         - There is no current program object specified by UseProgram,
    *           there is a current program pipeline object, and the current
    *           program for any shader stage has been relinked since being
    *           applied to the pipeline object via UseProgramStages with the
    *           PROGRAM_SEPARABLE parameter set to FALSE.
    */
   for (i = 0; i < MESA_SHADER_STAGES; i++) {
      if (pipe->CurrentProgram[i] &&
          !pipe->CurrentProgram[i]->info.separate_shader) {
         pipe->InfoLog = ralloc_asprintf(pipe,
                                         "Program %d was relinked without "
                                         "PROGRAM_SEPARABLE state",
                                         pipe->CurrentProgram[i]->Id);
         return GL_FALSE;
      }
   }

   /* Section 11.1.3.11 (Validation) of the OpenGL 4.5 spec says:
    *
    *    "An INVALID_OPERATION error is generated by any command that trans-
    *    fers vertices to the GL or launches compute work if the current set
    *    of active program objects cannot be executed, for reasons including:
    *
    *       ...
    *
    *       - There is no current program object specified by UseProgram,
    *         there is a current program pipeline object, and that object is
    *         empty (no executable code is installed for any stage).
    */
   for (i = 0; i < MESA_SHADER_STAGES; i++) {
      if (pipe->CurrentProgram[i]) {
         program_empty = false;
         break;
      }
   }

   if (program_empty) {
      return GL_FALSE;
   }

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
   if (!_mesa_sampler_uniforms_pipeline_are_valid(pipe))
      return GL_FALSE;

   /* Validate inputs against outputs, this cannot be done during linking
    * since programs have been linked separately from each other.
    *
    * Section 11.1.3.11 (Validation) of the OpenGL 4.5 Core Profile spec says:
    *
    *     "Separable program objects may have validation failures that cannot be
    *     detected without the complete program pipeline. Mismatched interfaces,
    *     improper usage of program objects together, and the same
    *     state-dependent failures can result in validation errors for such
    *     program objects."
    *
    * OpenGL ES 3.1 specification has the same text.
    *
    * Section 11.1.3.11 (Validation) of the OpenGL ES spec also says:
    *
    *    An INVALID_OPERATION error is generated by any command that transfers
    *    vertices to the GL or launches compute work if the current set of
    *    active program objects cannot be executed, for reasons including:
    *
    *    * The current program pipeline object contains a shader interface
    *      that doesn't have an exact match (see section 7.4.1)
    *
    * Based on this, only perform the most-strict checking on ES or when the
    * application has created a debug context.
    */
   if ((_mesa_is_gles(ctx) || (ctx->Const.ContextFlags & GL_CONTEXT_FLAG_DEBUG_BIT)) &&
       !_mesa_validate_pipeline_io(pipe)) {
      if (_mesa_is_gles(ctx))
         return GL_FALSE;

      static GLuint msg_id = 0;

      _mesa_gl_debugf(ctx, &msg_id,
                      MESA_DEBUG_SOURCE_API,
                      MESA_DEBUG_TYPE_PORTABILITY,
                      MESA_DEBUG_SEVERITY_MEDIUM,
                      "glValidateProgramPipeline: pipeline %u does not meet "
                      "strict OpenGL ES 3.1 requirements and may not be "
                      "portable across desktop hardware\n",
                      pipe->Name);
   }

   pipe->Validated = GL_TRUE;
   return GL_TRUE;
}

/**
 * Check compatibility of pipeline's program
 */
void GLAPIENTRY
_mesa_ValidateProgramPipeline(GLuint pipeline)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glValidateProgramPipeline(%u)\n", pipeline);

   struct gl_pipeline_object *pipe = _mesa_lookup_pipeline_object(ctx, pipeline);

   if (!pipe) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glValidateProgramPipeline(pipeline)");
      return;
   }

   _mesa_validate_program_pipeline(ctx, pipe);
   pipe->UserValidated = pipe->Validated;
}

void GLAPIENTRY
_mesa_GetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize,
                                GLsizei *length, GLchar *infoLog)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glGetProgramPipelineInfoLog(%u, %d, %p, %p)\n",
                  pipeline, bufSize, length, infoLog);

   struct gl_pipeline_object *pipe = _mesa_lookup_pipeline_object(ctx, pipeline);

   if (!pipe) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetProgramPipelineInfoLog(pipeline)");
      return;
   }

   if (bufSize < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetProgramPipelineInfoLog(bufSize)");
      return;
   }

   _mesa_copy_string(infoLog, bufSize, length, pipe->InfoLog);
}
