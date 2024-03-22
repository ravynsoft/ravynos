/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2009  VMware, Inc.   All Rights Reserved.
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
 * \file condrender.c
 * Conditional rendering functions
 *
 * \author Brian Paul
 */

#include "util/glheader.h"
#include "condrender.h"
#include "enums.h"
#include "mtypes.h"
#include "queryobj.h"

#include "api_exec_decl.h"
#include "state_tracker/st_cb_bitmap.h"
#include "state_tracker/st_context.h"

static void
BeginConditionalRender(struct gl_context *ctx, struct gl_query_object *q,
                       GLenum mode)
{
   struct st_context *st = st_context(ctx);
   uint m;
   /* Don't invert the condition for rendering by default */
   bool inverted = false;

   st_flush_bitmap_cache(st);

   switch (mode) {
   case GL_QUERY_WAIT:
      m = PIPE_RENDER_COND_WAIT;
      break;
   case GL_QUERY_NO_WAIT:
      m = PIPE_RENDER_COND_NO_WAIT;
      break;
   case GL_QUERY_BY_REGION_WAIT:
      m = PIPE_RENDER_COND_BY_REGION_WAIT;
      break;
   case GL_QUERY_BY_REGION_NO_WAIT:
      m = PIPE_RENDER_COND_BY_REGION_NO_WAIT;
      break;
   case GL_QUERY_WAIT_INVERTED:
      m = PIPE_RENDER_COND_WAIT;
      inverted = true;
      break;
   case GL_QUERY_NO_WAIT_INVERTED:
      m = PIPE_RENDER_COND_NO_WAIT;
      inverted = true;
      break;
   case GL_QUERY_BY_REGION_WAIT_INVERTED:
      m = PIPE_RENDER_COND_BY_REGION_WAIT;
      inverted = true;
      break;
   case GL_QUERY_BY_REGION_NO_WAIT_INVERTED:
      m = PIPE_RENDER_COND_BY_REGION_NO_WAIT;
      inverted = true;
      break;
   default:
      assert(0 && "bad mode in st_BeginConditionalRender");
      m = PIPE_RENDER_COND_WAIT;
   }

   cso_set_render_condition(st->cso_context, q->pq, inverted, m);
}

static void
EndConditionalRender(struct gl_context *ctx, struct gl_query_object *q)
{
   struct st_context *st = st_context(ctx);
   (void) q;

   st_flush_bitmap_cache(st);

   cso_set_render_condition(st->cso_context, NULL, false, 0);
}

static ALWAYS_INLINE void
begin_conditional_render(struct gl_context *ctx, GLuint queryId, GLenum mode,
                         bool no_error)
{
   struct gl_query_object *q = NULL;

   assert(ctx->Query.CondRenderMode == GL_NONE);

   if (queryId != 0)
      q = _mesa_lookup_query_object(ctx, queryId);

   if (!no_error) {
      /* Section 2.14 (Conditional Rendering) of the OpenGL 3.0 spec says:
       *
       *     "The error INVALID_VALUE is generated if <id> is not the name of an
       *     existing query object query."
       */
      if (!q) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glBeginConditionalRender(bad queryId=%u)", queryId);
         return;
      }
      assert(q->Id == queryId);

      switch (mode) {
      case GL_QUERY_WAIT:
      case GL_QUERY_NO_WAIT:
      case GL_QUERY_BY_REGION_WAIT:
      case GL_QUERY_BY_REGION_NO_WAIT:
         break; /* OK */
      case GL_QUERY_WAIT_INVERTED:
      case GL_QUERY_NO_WAIT_INVERTED:
      case GL_QUERY_BY_REGION_WAIT_INVERTED:
      case GL_QUERY_BY_REGION_NO_WAIT_INVERTED:
         if (ctx->Extensions.ARB_conditional_render_inverted)
            break; /* OK */
         FALLTHROUGH;
      default:
         _mesa_error(ctx, GL_INVALID_ENUM, "glBeginConditionalRender(mode=%s)",
                     _mesa_enum_to_string(mode));
         return;
      }

      /* Section 2.14 (Conditional Rendering) of the OpenGL 3.0 spec says:
       *
       *     "The error INVALID_OPERATION is generated if <id> is the name of a
       *     query object with a target other than SAMPLES_PASSED, or <id> is
       *     the name of a query currently in progress."
       */
      if ((q->Target != GL_SAMPLES_PASSED &&
           q->Target != GL_ANY_SAMPLES_PASSED &&
           q->Target != GL_ANY_SAMPLES_PASSED_CONSERVATIVE &&
           q->Target != GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB &&
           q->Target != GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB) || q->Active) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "glBeginConditionalRender()");
         return;
      }
   }

   ctx->Query.CondRenderQuery = q;
   ctx->Query.CondRenderMode = mode;

   BeginConditionalRender(ctx, q, mode);
}


void GLAPIENTRY
_mesa_BeginConditionalRender_no_error(GLuint queryId, GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);
   begin_conditional_render(ctx, queryId, mode, true);
}


void GLAPIENTRY
_mesa_BeginConditionalRender(GLuint queryId, GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);

   /* Section 2.14 (Conditional Rendering) of the OpenGL 3.0 spec says:
    *
    *     "If BeginConditionalRender is called while conditional rendering is
    *     in progress, or if EndConditionalRender is called while conditional
    *     rendering is not in progress, the error INVALID_OPERATION is
    *     generated."
    */
   if (!ctx->Extensions.NV_conditional_render || ctx->Query.CondRenderQuery) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glBeginConditionalRender()");
      return;
   }

   begin_conditional_render(ctx, queryId, mode, false);
}


static void
end_conditional_render(struct gl_context *ctx)
{
   FLUSH_VERTICES(ctx, 0, 0);

   EndConditionalRender(ctx, ctx->Query.CondRenderQuery);

   ctx->Query.CondRenderQuery = NULL;
   ctx->Query.CondRenderMode = GL_NONE;
}


void GLAPIENTRY
_mesa_EndConditionalRender_no_error(void)
{
   GET_CURRENT_CONTEXT(ctx);
   end_conditional_render(ctx);
}


void GLAPIENTRY
_mesa_EndConditionalRender(void)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.NV_conditional_render || !ctx->Query.CondRenderQuery) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glEndConditionalRender()");
      return;
   }

   end_conditional_render(ctx);
}


/**
 * This function is called by software rendering commands (all point,
 * line triangle drawing, glClear, glDrawPixels, glCopyPixels, and
 * glBitmap, glBlitFramebuffer) to determine if subsequent drawing
 * commands should be
 * executed or discarded depending on the current conditional
 * rendering state.  Ideally, this check would be implemented by the
 * GPU when doing hardware rendering.  XXX should this function be
 * called via a new driver hook?
 *
 * \return GL_TRUE if we should render, GL_FALSE if we should discard
 */
GLboolean
_mesa_check_conditional_render(struct gl_context *ctx)
{
   struct gl_query_object *q = ctx->Query.CondRenderQuery;

   if (!q) {
      /* no query in progress - draw normally */
      return GL_TRUE;
   }

   switch (ctx->Query.CondRenderMode) {
   case GL_QUERY_BY_REGION_WAIT:
      FALLTHROUGH;
   case GL_QUERY_WAIT:
      if (!q->Ready) {
         _mesa_wait_query(ctx, q);
      }
      return q->Result > 0;
   case GL_QUERY_BY_REGION_WAIT_INVERTED:
      FALLTHROUGH;
   case GL_QUERY_WAIT_INVERTED:
      if (!q->Ready) {
         _mesa_wait_query(ctx, q);
      }
      return q->Result == 0;
   case GL_QUERY_BY_REGION_NO_WAIT:
      FALLTHROUGH;
   case GL_QUERY_NO_WAIT:
      if (!q->Ready)
         _mesa_check_query(ctx, q);
      return q->Ready ? (q->Result > 0) : GL_TRUE;
   case GL_QUERY_BY_REGION_NO_WAIT_INVERTED:
      FALLTHROUGH;
   case GL_QUERY_NO_WAIT_INVERTED:
      if (!q->Ready)
         _mesa_check_query(ctx, q);
      return q->Ready ? (q->Result == 0) : GL_TRUE;
   default:
      _mesa_problem(ctx, "Bad cond render mode %s in "
                    " _mesa_check_conditional_render()",
                    _mesa_enum_to_string(ctx->Query.CondRenderMode));
      return GL_TRUE;
   }
}
