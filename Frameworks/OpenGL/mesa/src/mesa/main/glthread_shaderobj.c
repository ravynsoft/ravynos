/*
 * Copyright © 2012 Intel Corporation
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

#include "glthread_marshal.h"
#include "dispatch.h"
#include "uniforms.h"
#include "api_exec_decl.h"

void
_mesa_glthread_ProgramChanged(struct gl_context *ctx)
{
   struct glthread_state *glthread = &ctx->GLThread;

   /* Track the last change. */
   p_atomic_set(&glthread->LastProgramChangeBatch, glthread->next);
   _mesa_glthread_flush_batch(ctx);
}

uint32_t
_mesa_unmarshal_GetActiveUniform(struct gl_context *ctx,
                                 const struct marshal_cmd_GetActiveUniform *restrict cmd)
{
   unreachable("never executed");
   return 0;
}

static void
wait_for_glLinkProgram(struct gl_context *ctx)
{
   /* Wait for the last glLinkProgram call. */
   _mesa_glthread_wait_for_call(ctx, &ctx->GLThread.LastProgramChangeBatch);
}

void GLAPIENTRY
_mesa_marshal_GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize,
                               GLsizei *length, GLint *size, GLenum *type,
                               GLchar * name)
{
   GET_CURRENT_CONTEXT(ctx);

   /* This will generate GL_INVALID_OPERATION, as it should. */
   if (ctx->GLThread.inside_begin_end) {
      _mesa_glthread_finish_before(ctx, "GetActiveUniform");
      CALL_GetActiveUniform(ctx->Dispatch.Current,
                            (program, index, bufSize, length, size, type,
                             name));
      return;
   }

   wait_for_glLinkProgram(ctx);

   /* We can execute glGetActiveUniform without syncing if we are sync'd to
    * the last calls of glLinkProgram and glDeleteProgram because shader
    * object IDs and their contents are immutable after those calls and
    * also thread-safe because they are shared between contexts.
    * glCreateShaderProgram calls glLinkProgram internally and it always
    * syncs, so it doesn't need any handling.
    */
   _mesa_GetActiveUniform_impl(program, index, bufSize, length, size, type,
                               name, true);
}

uint32_t
_mesa_unmarshal_GetUniformLocation(struct gl_context *ctx,
                                   const struct marshal_cmd_GetUniformLocation *restrict cmd)
{
   unreachable("never executed");
   return 0;
}

GLint GLAPIENTRY
_mesa_marshal_GetUniformLocation(GLuint program, const GLchar *name)
{
   GET_CURRENT_CONTEXT(ctx);

   /* This will generate GL_INVALID_OPERATION, as it should. */
   if (ctx->GLThread.inside_begin_end) {
      _mesa_glthread_finish_before(ctx, "GetUniformLocation");
      return CALL_GetUniformLocation(ctx->Dispatch.Current, (program, name));
   }

   wait_for_glLinkProgram(ctx);

   /* This is thread-safe. See the comment in _mesa_marshal_GetActiveUniform. */
   return _mesa_GetUniformLocation_impl(program, name, true);
}
