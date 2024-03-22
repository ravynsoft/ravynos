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


#include "bufferobj.h"
#include "util/glheader.h"
#include "context.h"
#include "enums.h"
#include "hash.h"

#include "queryobj.h"
#include "mtypes.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/u_memory.h"

#include "api_exec_decl.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "state_tracker/st_context.h"
#include "state_tracker/st_cb_bitmap.h"


static struct gl_query_object *
new_query_object(struct gl_context *ctx, GLuint id)
{
   struct gl_query_object *q = CALLOC_STRUCT(gl_query_object);
   if (q) {
      q->Id = id;
      q->Ready = GL_TRUE;
      q->pq = NULL;
      q->type = PIPE_QUERY_TYPES; /* an invalid value */
      return q;
   }
   return NULL;
}


static void
free_queries(struct pipe_context *pipe, struct gl_query_object *q)
{
   if (q->pq) {
      pipe->destroy_query(pipe, q->pq);
      q->pq = NULL;
   }

   if (q->pq_begin) {
      pipe->destroy_query(pipe, q->pq_begin);
      q->pq_begin = NULL;
   }
}


static void
delete_query(struct gl_context *ctx, struct gl_query_object *q)
{
   struct pipe_context *pipe = ctx->pipe;

   free_queries(pipe, q);
   free(q->Label);
   FREE(q);
}

static int
target_to_index(const struct gl_query_object *q)
{
   if (q->Target == GL_PRIMITIVES_GENERATED ||
       q->Target == GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN ||
       q->Target == GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB)
      return q->Stream;

   /* Drivers with PIPE_CAP_QUERY_PIPELINE_STATISTICS_SINGLE = 0 ignore the
    * index param so it should be useless; but radeonsi needs it in some cases,
    * so pass the correct value.
    */
   switch (q->Target) {
      case GL_VERTICES_SUBMITTED_ARB:
         return PIPE_STAT_QUERY_IA_VERTICES;
      case GL_PRIMITIVES_SUBMITTED_ARB:
         return PIPE_STAT_QUERY_IA_PRIMITIVES;
      case GL_VERTEX_SHADER_INVOCATIONS_ARB:
         return PIPE_STAT_QUERY_VS_INVOCATIONS;
      case GL_GEOMETRY_SHADER_INVOCATIONS:
         return PIPE_STAT_QUERY_GS_INVOCATIONS;
      case GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB:
         return PIPE_STAT_QUERY_GS_PRIMITIVES;
      case GL_CLIPPING_INPUT_PRIMITIVES_ARB:
         return PIPE_STAT_QUERY_C_INVOCATIONS;
      case GL_CLIPPING_OUTPUT_PRIMITIVES_ARB:
         return PIPE_STAT_QUERY_C_PRIMITIVES;
      case GL_FRAGMENT_SHADER_INVOCATIONS_ARB:
         return PIPE_STAT_QUERY_PS_INVOCATIONS;
      case GL_TESS_CONTROL_SHADER_PATCHES_ARB:
         return PIPE_STAT_QUERY_HS_INVOCATIONS;
      case GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB:
         return PIPE_STAT_QUERY_DS_INVOCATIONS;
      case GL_COMPUTE_SHADER_INVOCATIONS_ARB:
         return PIPE_STAT_QUERY_CS_INVOCATIONS;
      default:
         break;
   }

   return 0;
}

static bool
query_type_is_dummy(struct gl_context *ctx, unsigned type)
{
   struct st_context *st = st_context(ctx);
   switch (type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      return !st->has_occlusion_query;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      return !st->has_pipeline_stat;
   case PIPE_QUERY_PIPELINE_STATISTICS_SINGLE:
      return !st->has_single_pipe_stat;
   default:
      break;
   }
   return false;
}

static void
begin_query(struct gl_context *ctx, struct gl_query_object *q)
{
   struct st_context *st = st_context(ctx);
   struct pipe_context *pipe = ctx->pipe;
   unsigned type;
   bool ret = false;

   st_flush_bitmap_cache(st_context(ctx));

   /* convert GL query type to Gallium query type */
   switch (q->Target) {
   case GL_ANY_SAMPLES_PASSED:
      type = PIPE_QUERY_OCCLUSION_PREDICATE;
      break;
   case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
      type = PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE;
      break;
   case GL_SAMPLES_PASSED_ARB:
      type = PIPE_QUERY_OCCLUSION_COUNTER;
      break;
   case GL_PRIMITIVES_GENERATED:
      type = PIPE_QUERY_PRIMITIVES_GENERATED;
      break;
   case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
      type = PIPE_QUERY_PRIMITIVES_EMITTED;
      break;
   case GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB:
      type = PIPE_QUERY_SO_OVERFLOW_PREDICATE;
      break;
   case GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB:
      type = PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE;
      break;
   case GL_TIME_ELAPSED:
      if (st->has_time_elapsed)
         type = PIPE_QUERY_TIME_ELAPSED;
      else
         type = PIPE_QUERY_TIMESTAMP;
      break;
   case GL_VERTICES_SUBMITTED_ARB:
   case GL_PRIMITIVES_SUBMITTED_ARB:
   case GL_VERTEX_SHADER_INVOCATIONS_ARB:
   case GL_TESS_CONTROL_SHADER_PATCHES_ARB:
   case GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB:
   case GL_GEOMETRY_SHADER_INVOCATIONS:
   case GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB:
   case GL_FRAGMENT_SHADER_INVOCATIONS_ARB:
   case GL_COMPUTE_SHADER_INVOCATIONS_ARB:
   case GL_CLIPPING_INPUT_PRIMITIVES_ARB:
   case GL_CLIPPING_OUTPUT_PRIMITIVES_ARB:
      type = st->has_single_pipe_stat ? PIPE_QUERY_PIPELINE_STATISTICS_SINGLE
                                      : PIPE_QUERY_PIPELINE_STATISTICS;
      break;
   default:
      assert(0 && "unexpected query target in st_BeginQuery()");
      return;
   }

   if (q->type != type) {
      /* free old query of different type */
      free_queries(pipe, q);
      q->type = PIPE_QUERY_TYPES; /* an invalid value */
   }

   if (q->Target == GL_TIME_ELAPSED &&
       type == PIPE_QUERY_TIMESTAMP) {
      /* Determine time elapsed by emitting two timestamp queries. */
      if (!q->pq_begin) {
         q->pq_begin = pipe->create_query(pipe, type, 0);
         q->type = type;
      }
      if (q->pq_begin)
         ret = pipe->end_query(pipe, q->pq_begin);
   } else {
      if (query_type_is_dummy(ctx, type)) {
         /* starting a dummy-query; ignore */
         assert(!q->pq);
         q->type = type;
         ret = true;
      } else if (!q->pq) {
         q->pq = pipe->create_query(pipe, type, target_to_index(q));
         q->type = type;
      }
      if (q->pq)
         ret = pipe->begin_query(pipe, q->pq);
   }

   if (!ret) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glBeginQuery");

      free_queries(pipe, q);
      q->Active = GL_FALSE;
      return;
   }

   if (q->type != PIPE_QUERY_TIMESTAMP)
      st->active_queries++;

   assert(q->type == type);
}


static void
end_query(struct gl_context *ctx, struct gl_query_object *q)
{
   struct st_context *st = st_context(ctx);
   struct pipe_context *pipe = ctx->pipe;
   bool ret = false;

   st_flush_bitmap_cache(st_context(ctx));

   if ((q->Target == GL_TIMESTAMP ||
        q->Target == GL_TIME_ELAPSED) &&
       !q->pq) {
      q->pq = pipe->create_query(pipe, PIPE_QUERY_TIMESTAMP, 0);
      q->type = PIPE_QUERY_TIMESTAMP;
   }

   if (query_type_is_dummy(ctx, q->type)) {
      /* ending a dummy-query; ignore */
      ret = true;
   } else if (q->pq)
      ret = pipe->end_query(pipe, q->pq);

   if (!ret) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glEndQuery");
      return;
   }

   if (q->type != PIPE_QUERY_TIMESTAMP)
      st->active_queries--;
}


static bool
get_query_result(struct pipe_context *pipe,
                 struct gl_query_object *q,
                 bool wait)
{
   union pipe_query_result data;

   if (!q->pq) {
      /* Needed in case we failed to allocate the gallium query earlier, or
       * in the case of a dummy query.
       *
       * Return TRUE in either case so we don't spin on this forever.
       */
      return true;
   }

   if (!pipe->get_query_result(pipe, q->pq, wait, &data))
      return false;

   switch (q->type) {
   case PIPE_QUERY_PIPELINE_STATISTICS:
      switch (q->Target) {
      case GL_VERTICES_SUBMITTED_ARB:
         q->Result = data.pipeline_statistics.ia_vertices;
         break;
      case GL_PRIMITIVES_SUBMITTED_ARB:
         q->Result = data.pipeline_statistics.ia_primitives;
         break;
      case GL_VERTEX_SHADER_INVOCATIONS_ARB:
         q->Result = data.pipeline_statistics.vs_invocations;
         break;
      case GL_TESS_CONTROL_SHADER_PATCHES_ARB:
         q->Result = data.pipeline_statistics.hs_invocations;
         break;
      case GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB:
         q->Result = data.pipeline_statistics.ds_invocations;
         break;
      case GL_GEOMETRY_SHADER_INVOCATIONS:
         q->Result = data.pipeline_statistics.gs_invocations;
         break;
      case GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB:
         q->Result = data.pipeline_statistics.gs_primitives;
         break;
      case GL_FRAGMENT_SHADER_INVOCATIONS_ARB:
         q->Result = data.pipeline_statistics.ps_invocations;
         break;
      case GL_COMPUTE_SHADER_INVOCATIONS_ARB:
         q->Result = data.pipeline_statistics.cs_invocations;
         break;
      case GL_CLIPPING_INPUT_PRIMITIVES_ARB:
         q->Result = data.pipeline_statistics.c_invocations;
         break;
      case GL_CLIPPING_OUTPUT_PRIMITIVES_ARB:
         q->Result = data.pipeline_statistics.c_primitives;
         break;
      default:
         unreachable("invalid pipeline statistics counter");
      }
      break;
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      q->Result = !!data.b;
      break;
   default:
      q->Result = data.u64;
      break;
   }

   if (q->Target == GL_TIME_ELAPSED &&
       q->type == PIPE_QUERY_TIMESTAMP) {
      /* Calculate the elapsed time from the two timestamp queries */
      assert(q->pq_begin);
      pipe->get_query_result(pipe, q->pq_begin, true, &data);
      q->Result -= data.u64;
   } else {
      assert(!q->pq_begin);
   }

   return true;
}


void
_mesa_wait_query(struct gl_context *ctx, struct gl_query_object *q)
{
   struct pipe_context *pipe = ctx->pipe;

   /* this function should only be called if we don't have a ready result */
   assert(!q->Ready);

   while (!q->Ready &&
          !get_query_result(pipe, q, true))
   {
      /* nothing */
   }

   q->Ready = GL_TRUE;
}


void
_mesa_check_query(struct gl_context *ctx, struct gl_query_object *q)
{
   struct pipe_context *pipe = ctx->pipe;
   assert(!q->Ready);   /* we should not get called if Ready is TRUE */
   q->Ready = get_query_result(pipe, q, false);
}


uint64_t
_mesa_get_timestamp(struct gl_context *ctx)
{
   struct pipe_context *pipe = ctx->pipe;
   struct pipe_screen *screen = pipe->screen;

   /* Prefer the per-screen function */
   if (screen->get_timestamp) {
      return screen->get_timestamp(screen);
   }
   else {
      /* Fall back to the per-context function */
      assert(pipe->get_timestamp);
      return pipe->get_timestamp(pipe);
   }
}

static void
store_query_result(struct gl_context *ctx, struct gl_query_object *q,
                   struct gl_buffer_object *buf, intptr_t offset,
                   GLenum pname, GLenum ptype)
{
   struct pipe_context *pipe = ctx->pipe;
   enum pipe_query_flags flags = 0;
   enum pipe_query_value_type result_type;
   int index;

   if (pname == GL_QUERY_RESULT)
      flags |= PIPE_QUERY_WAIT;

   /* GL_QUERY_TARGET is a bit of an extension since it has nothing to
    * do with the GPU end of the query. Write it in "by hand".
    */
   if (pname == GL_QUERY_TARGET) {
      /* Assume that the data must be LE. The endianness situation wrt CPU and
       * GPU is incredibly confusing, but the vast majority of GPUs are
       * LE. When a BE one comes along, this needs some form of resolution.
       */
      unsigned data[2] = { CPU_TO_LE32(q->Target), 0 };
      pipe_buffer_write(pipe, buf->buffer, offset,
                        (ptype == GL_INT64_ARB ||
                         ptype == GL_UNSIGNED_INT64_ARB) ? 8 : 4,
                        data);
      return;
   }

   switch (ptype) {
   case GL_INT:
      result_type = PIPE_QUERY_TYPE_I32;
      break;
   case GL_UNSIGNED_INT:
      result_type = PIPE_QUERY_TYPE_U32;
      break;
   case GL_INT64_ARB:
      result_type = PIPE_QUERY_TYPE_I64;
      break;
   case GL_UNSIGNED_INT64_ARB:
      result_type = PIPE_QUERY_TYPE_U64;
      break;
   default:
      unreachable("Unexpected result type");
   }

   if (pname == GL_QUERY_RESULT_AVAILABLE) {
      index = -1;
   } else if (q->type == PIPE_QUERY_PIPELINE_STATISTICS) {
      index = target_to_index(q);
   } else {
      index = 0;
   }

   if (q->pq)
      pipe->get_query_result_resource(pipe, q->pq, flags, result_type, index,
                                      buf->buffer, offset);
}

static struct gl_query_object **
get_pipe_stats_binding_point(struct gl_context *ctx,
                             GLenum target)
{
   const int which = target - GL_VERTICES_SUBMITTED;
   assert(which < MAX_PIPELINE_STATISTICS);

   if (!_mesa_has_pipeline_statistics(ctx))
      return NULL;

   return &ctx->Query.pipeline_stats[which];
}

/**
 * Return pointer to the query object binding point for the given target and
 * index.
 * \return NULL if invalid target, else the address of binding point
 */
static struct gl_query_object **
get_query_binding_point(struct gl_context *ctx, GLenum target, GLuint index)
{
   switch (target) {
   case GL_SAMPLES_PASSED:
      if (_mesa_has_occlusion_query(ctx))
         return &ctx->Query.CurrentOcclusionObject;
      else
         return NULL;
   case GL_ANY_SAMPLES_PASSED:
      if (_mesa_has_occlusion_query_boolean(ctx))
         return &ctx->Query.CurrentOcclusionObject;
      else
         return NULL;
   case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
      if (_mesa_has_ARB_ES3_compatibility(ctx) ||
          _mesa_has_EXT_occlusion_query_boolean(ctx))
         return &ctx->Query.CurrentOcclusionObject;
      else
         return NULL;
   case GL_TIME_ELAPSED:
      if (_mesa_has_EXT_timer_query(ctx) ||
          _mesa_has_EXT_disjoint_timer_query(ctx))
         return &ctx->Query.CurrentTimerObject;
      else
         return NULL;
   case GL_PRIMITIVES_GENERATED:
      if (_mesa_has_EXT_transform_feedback(ctx) ||
          _mesa_has_EXT_tessellation_shader(ctx) ||
          _mesa_has_OES_geometry_shader(ctx))
         return &ctx->Query.PrimitivesGenerated[index];
      else
         return NULL;
   case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
      if (_mesa_has_EXT_transform_feedback(ctx) || _mesa_is_gles3(ctx))
         return &ctx->Query.PrimitivesWritten[index];
      else
         return NULL;
   case GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW:
      if (_mesa_has_ARB_transform_feedback_overflow_query(ctx))
         return &ctx->Query.TransformFeedbackOverflow[index];
      else
         return NULL;
   case GL_TRANSFORM_FEEDBACK_OVERFLOW:
      if (_mesa_has_ARB_transform_feedback_overflow_query(ctx))
         return &ctx->Query.TransformFeedbackOverflowAny;
      else
         return NULL;

   case GL_VERTICES_SUBMITTED:
   case GL_PRIMITIVES_SUBMITTED:
   case GL_VERTEX_SHADER_INVOCATIONS:
   case GL_FRAGMENT_SHADER_INVOCATIONS:
   case GL_CLIPPING_INPUT_PRIMITIVES:
   case GL_CLIPPING_OUTPUT_PRIMITIVES:
         return get_pipe_stats_binding_point(ctx, target);

   case GL_GEOMETRY_SHADER_INVOCATIONS:
      /* GL_GEOMETRY_SHADER_INVOCATIONS is defined in a non-sequential order */
      target = GL_VERTICES_SUBMITTED + MAX_PIPELINE_STATISTICS - 1;
      FALLTHROUGH;
   case GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED:
      if (_mesa_has_geometry_shaders(ctx))
         return get_pipe_stats_binding_point(ctx, target);
      else
         return NULL;

   case GL_TESS_CONTROL_SHADER_PATCHES:
   case GL_TESS_EVALUATION_SHADER_INVOCATIONS:
      if (_mesa_has_tessellation(ctx))
         return get_pipe_stats_binding_point(ctx, target);
      else
         return NULL;

   case GL_COMPUTE_SHADER_INVOCATIONS:
      if (_mesa_has_compute_shaders(ctx))
         return get_pipe_stats_binding_point(ctx, target);
      else
         return NULL;

   default:
      return NULL;
   }
}

/**
 * Create $n query objects and store them in *ids. Make them of type $target
 * if dsa is set. Called from _mesa_GenQueries() and _mesa_CreateQueries().
 */
static void
create_queries(struct gl_context *ctx, GLenum target, GLsizei n, GLuint *ids,
               bool dsa)
{
   const char *func = dsa ? "glGenQueries" : "glCreateQueries";

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "%s(%d)\n", func, n);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(n < 0)", func);
      return;
   }

   if (_mesa_HashFindFreeKeys(ctx->Query.QueryObjects, ids, n)) {
      GLsizei i;
      for (i = 0; i < n; i++) {
         struct gl_query_object *q
            = new_query_object(ctx, ids[i]);
         if (!q) {
            _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", func);
            return;
         } else if (dsa) {
            /* Do the equivalent of binding the buffer with a target */
            q->Target = target;
            q->EverBound = GL_TRUE;
         }
         _mesa_HashInsertLocked(ctx->Query.QueryObjects, ids[i], q, true);
      }
   }
}

void GLAPIENTRY
_mesa_GenQueries(GLsizei n, GLuint *ids)
{
   GET_CURRENT_CONTEXT(ctx);
   create_queries(ctx, 0, n, ids, false);
}

void GLAPIENTRY
_mesa_CreateQueries(GLenum target, GLsizei n, GLuint *ids)
{
   GET_CURRENT_CONTEXT(ctx);

   switch (target) {
   case GL_SAMPLES_PASSED:
   case GL_ANY_SAMPLES_PASSED:
   case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
   case GL_TIME_ELAPSED:
   case GL_TIMESTAMP:
   case GL_PRIMITIVES_GENERATED:
   case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
   case GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW:
   case GL_TRANSFORM_FEEDBACK_OVERFLOW:
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glCreateQueries(invalid target = %s)",
                  _mesa_enum_to_string(target));
      return;
   }

   create_queries(ctx, target, n, ids, true);
}


void GLAPIENTRY
_mesa_DeleteQueries(GLsizei n, const GLuint *ids)
{
   GLint i;
   GET_CURRENT_CONTEXT(ctx);
   FLUSH_VERTICES(ctx, 0, 0);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glDeleteQueries(%d)\n", n);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDeleteQueriesARB(n < 0)");
      return;
   }

   for (i = 0; i < n; i++) {
      if (ids[i] > 0) {
         struct gl_query_object *q = _mesa_lookup_query_object(ctx, ids[i]);
         if (q) {
            if (q->Active) {
               struct gl_query_object **bindpt;
               bindpt = get_query_binding_point(ctx, q->Target, q->Stream);
               assert(bindpt); /* Should be non-null for active q. */
               if (bindpt) {
                  *bindpt = NULL;
               }
               q->Active = GL_FALSE;
               end_query(ctx, q);
            }
            _mesa_HashRemoveLocked(ctx->Query.QueryObjects, ids[i]);
            delete_query(ctx, q);
         }
      }
   }
}


GLboolean GLAPIENTRY
_mesa_IsQuery(GLuint id)
{
   struct gl_query_object *q;

   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_FALSE);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glIsQuery(%u)\n", id);

   if (id == 0)
      return GL_FALSE;

   q = _mesa_lookup_query_object(ctx, id);
   if (q == NULL)
      return GL_FALSE;

   return q->EverBound;
}

static GLboolean
query_error_check_index(struct gl_context *ctx, GLenum target, GLuint index)
{
   switch (target) {
   case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
   case GL_PRIMITIVES_GENERATED:
   case GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW:
      if (index >= ctx->Const.MaxVertexStreams) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glBeginQueryIndexed(index>=MaxVertexStreams)");
         return GL_FALSE;
      }
      break;
   default:
      if (index > 0) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glBeginQueryIndexed(index>0)");
         return GL_FALSE;
      }
   }
   return GL_TRUE;
}

void GLAPIENTRY
_mesa_BeginQueryIndexed(GLenum target, GLuint index, GLuint id)
{
   struct gl_query_object *q, **bindpt;
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glBeginQueryIndexed(%s, %u, %u)\n",
                  _mesa_enum_to_string(target), index, id);

   if (!query_error_check_index(ctx, target, index))
      return;

   FLUSH_VERTICES(ctx, 0, 0);

   bindpt = get_query_binding_point(ctx, target, index);
   if (!bindpt) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glBeginQuery{Indexed}(target)");
      return;
   }

   /* From the GL_ARB_occlusion_query spec:
    *
    *     "If BeginQueryARB is called while another query is already in
    *      progress with the same target, an INVALID_OPERATION error is
    *      generated."
    */
   if (*bindpt) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBeginQuery{Indexed}(target=%s is active)",
                  _mesa_enum_to_string(target));
      return;
   }

   if (id == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glBeginQuery{Indexed}(id==0)");
      return;
   }

   q = _mesa_lookup_query_object(ctx, id);
   if (!q) {
      if (ctx->API != API_OPENGL_COMPAT) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glBeginQuery{Indexed}(non-gen name)");
         return;
      } else {
         /* create new object */
         q = new_query_object(ctx, id);
         if (!q) {
            _mesa_error(ctx, GL_OUT_OF_MEMORY, "glBeginQuery{Indexed}");
            return;
         }
         _mesa_HashInsertLocked(ctx->Query.QueryObjects, id, q, false);
      }
   }
   else {
      /* pre-existing object */
      if (q->Active) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glBeginQuery{Indexed}(query already active)");
         return;
      }

      /* Section 2.14 Asynchronous Queries, page 84 of the OpenGL ES 3.0.4
       * spec states:
       *
       *     "BeginQuery generates an INVALID_OPERATION error if any of the
       *      following conditions hold: [...] id is the name of an
       *      existing query object whose type does not match target; [...]
       *
       * Similar wording exists in the OpenGL 4.5 spec, section 4.2. QUERY
       * OBJECTS AND ASYNCHRONOUS QUERIES, page 43.
       */
      if (q->EverBound && q->Target != target) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glBeginQuery{Indexed}(target mismatch)");
         return;
      }
   }

   /* This possibly changes the target of a buffer allocated by
    * CreateQueries. Issue 39) in the ARB_direct_state_access extension states
    * the following:
    *
    * "CreateQueries adds a <target>, so strictly speaking the <target>
    * command isn't needed for BeginQuery/EndQuery, but in the end, this also
    * isn't a selector, so we decided not to change it."
    *
    * Updating the target of the query object should be acceptable, so let's
    * do that.
    */

   q->Target = target;
   q->Active = GL_TRUE;
   q->Result = 0;
   q->Ready = GL_FALSE;
   q->EverBound = GL_TRUE;
   q->Stream = index;

   /* XXX should probably refcount query objects */
   *bindpt = q;

   begin_query(ctx, q);
}


void GLAPIENTRY
_mesa_EndQueryIndexed(GLenum target, GLuint index)
{
   struct gl_query_object *q, **bindpt;
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glEndQueryIndexed(%s, %u)\n",
                  _mesa_enum_to_string(target), index);

   if (!query_error_check_index(ctx, target, index))
      return;

   FLUSH_VERTICES(ctx, 0, 0);

   bindpt = get_query_binding_point(ctx, target, index);
   if (!bindpt) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glEndQuery{Indexed}(target)");
      return;
   }

   /* XXX should probably refcount query objects */
   q = *bindpt;

   /* Check for GL_ANY_SAMPLES_PASSED vs GL_SAMPLES_PASSED. */
   if (q && q->Target != target) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glEndQuery(target=%s with active query of target %s)",
                  _mesa_enum_to_string(target),
                  _mesa_enum_to_string(q->Target));
      return;
   }

   *bindpt = NULL;

   if (!q || !q->Active) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glEndQuery{Indexed}(no matching glBeginQuery{Indexed})");
      return;
   }

   q->Active = GL_FALSE;
   end_query(ctx, q);
}

void GLAPIENTRY
_mesa_BeginQuery(GLenum target, GLuint id)
{
   _mesa_BeginQueryIndexed(target, 0, id);
}

void GLAPIENTRY
_mesa_EndQuery(GLenum target)
{
   _mesa_EndQueryIndexed(target, 0);
}

void GLAPIENTRY
_mesa_QueryCounter(GLuint id, GLenum target)
{
   struct gl_query_object *q;
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glQueryCounter(%u, %s)\n", id,
                  _mesa_enum_to_string(target));

   /* error checking */
   if (target != GL_TIMESTAMP) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glQueryCounter(target)");
      return;
   }

   if (id == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glQueryCounter(id==0)");
      return;
   }

   q = _mesa_lookup_query_object(ctx, id);
   if (!q) {
      /* XXX the Core profile should throw INVALID_OPERATION here */

      /* create new object */
      q = new_query_object(ctx, id);
      if (!q) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glQueryCounter");
         return;
      }
      _mesa_HashInsertLocked(ctx->Query.QueryObjects, id, q, false);
   }
   else {
      if (q->Target && q->Target != GL_TIMESTAMP) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glQueryCounter(id has an invalid target)");
         return;
      }
   }

   if (q->Active) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glQueryCounter(id is active)");
      return;
   }

   /* This possibly changes the target of a buffer allocated by
    * CreateQueries. Issue 39) in the ARB_direct_state_access extension states
    * the following:
    *
    * "CreateQueries adds a <target>, so strictly speaking the <target>
    * command isn't needed for BeginQuery/EndQuery, but in the end, this also
    * isn't a selector, so we decided not to change it."
    *
    * Updating the target of the query object should be acceptable, so let's
    * do that.
    */

   q->Target = target;
   q->Result = 0;
   q->Ready = GL_FALSE;
   q->EverBound = GL_TRUE;

   /* QueryCounter is implemented using EndQuery without BeginQuery
    * in drivers. This is actually Direct3D and Gallium convention.
    */
   end_query(ctx, q);
}


void GLAPIENTRY
_mesa_GetQueryIndexediv(GLenum target, GLuint index, GLenum pname,
                        GLint *params)
{
   struct gl_query_object *q = NULL, **bindpt = NULL;
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glGetQueryIndexediv(%s, %u, %s)\n",
                  _mesa_enum_to_string(target),
                  index,
                  _mesa_enum_to_string(pname));

   if (!query_error_check_index(ctx, target, index))
      return;

   /* From the GL_EXT_occlusion_query_boolean spec:
    *
    * "The error INVALID_ENUM is generated if GetQueryivEXT is called where
    * <pname> is not CURRENT_QUERY_EXT."
    *
    * Same rule is present also in ES 3.2 spec.
    *
    * EXT_disjoint_timer_query extends this with GL_QUERY_COUNTER_BITS.
    */
   if (_mesa_is_gles(ctx)) {
      switch (pname) {
      case GL_CURRENT_QUERY:
         break;
      case GL_QUERY_COUNTER_BITS:
         if (_mesa_has_EXT_disjoint_timer_query(ctx))
            break;
         FALLTHROUGH;
      default:
         _mesa_error(ctx, GL_INVALID_ENUM, "glGetQueryivEXT(%s)",
                     _mesa_enum_to_string(pname));
      }
   }

   if (target == GL_TIMESTAMP) {
      if (!_mesa_has_ARB_timer_query(ctx) &&
          !_mesa_has_EXT_disjoint_timer_query(ctx)) {
         _mesa_error(ctx, GL_INVALID_ENUM, "glGetQueryARB(target)");
         return;
      }
   }
   else {
      bindpt = get_query_binding_point(ctx, target, index);
      if (!bindpt) {
         _mesa_error(ctx, GL_INVALID_ENUM, "glGetQuery{Indexed}iv(target)");
         return;
      }

      q = *bindpt;
   }

   switch (pname) {
      case GL_QUERY_COUNTER_BITS:
         switch (target) {
         case GL_SAMPLES_PASSED:
            *params = ctx->Const.QueryCounterBits.SamplesPassed;
            break;
         case GL_ANY_SAMPLES_PASSED:
         case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
            /* The minimum value of this is 1 if it's nonzero, and the value
             * is only ever GL_TRUE or GL_FALSE, so no sense in reporting more
             * bits.
             */
            *params = 1;
            break;
         case GL_TIME_ELAPSED:
            *params = ctx->Const.QueryCounterBits.TimeElapsed;
            break;
         case GL_TIMESTAMP:
            *params = ctx->Const.QueryCounterBits.Timestamp;
            break;
         case GL_PRIMITIVES_GENERATED:
            *params = ctx->Const.QueryCounterBits.PrimitivesGenerated;
            break;
         case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
            *params = ctx->Const.QueryCounterBits.PrimitivesWritten;
            break;
         case GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW:
         case GL_TRANSFORM_FEEDBACK_OVERFLOW:
            /* The minimum value of this is 1 if it's nonzero, and the value
             * is only ever GL_TRUE or GL_FALSE, so no sense in reporting more
             * bits.
             */
            *params = 1;
            break;
         case GL_VERTICES_SUBMITTED:
            *params = ctx->Const.QueryCounterBits.VerticesSubmitted;
            break;
         case GL_PRIMITIVES_SUBMITTED:
            *params = ctx->Const.QueryCounterBits.PrimitivesSubmitted;
            break;
         case GL_VERTEX_SHADER_INVOCATIONS:
            *params = ctx->Const.QueryCounterBits.VsInvocations;
            break;
         case GL_TESS_CONTROL_SHADER_PATCHES:
            *params = ctx->Const.QueryCounterBits.TessPatches;
            break;
         case GL_TESS_EVALUATION_SHADER_INVOCATIONS:
            *params = ctx->Const.QueryCounterBits.TessInvocations;
            break;
         case GL_GEOMETRY_SHADER_INVOCATIONS:
            *params = ctx->Const.QueryCounterBits.GsInvocations;
            break;
         case GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED:
            *params = ctx->Const.QueryCounterBits.GsPrimitives;
            break;
         case GL_FRAGMENT_SHADER_INVOCATIONS:
            *params = ctx->Const.QueryCounterBits.FsInvocations;
            break;
         case GL_COMPUTE_SHADER_INVOCATIONS:
            *params = ctx->Const.QueryCounterBits.ComputeInvocations;
            break;
         case GL_CLIPPING_INPUT_PRIMITIVES:
            *params = ctx->Const.QueryCounterBits.ClInPrimitives;
            break;
         case GL_CLIPPING_OUTPUT_PRIMITIVES:
            *params = ctx->Const.QueryCounterBits.ClOutPrimitives;
            break;
         default:
            _mesa_problem(ctx,
                          "Unknown target in glGetQueryIndexediv(target = %s)",
                          _mesa_enum_to_string(target));
            *params = 0;
            break;
         }
         break;
      case GL_CURRENT_QUERY:
         *params = (q && q->Target == target) ? q->Id : 0;
         break;
      default:
         _mesa_error(ctx, GL_INVALID_ENUM, "glGetQuery{Indexed}iv(pname)");
         return;
   }
}

void GLAPIENTRY
_mesa_GetQueryiv(GLenum target, GLenum pname, GLint *params)
{
   _mesa_GetQueryIndexediv(target, 0, pname, params);
}

static void
get_query_object(struct gl_context *ctx, const char *func,
                 GLuint id, GLenum pname, GLenum ptype,
                 struct gl_buffer_object *buf, intptr_t offset)
{
   struct gl_query_object *q = NULL;
   uint64_t value;

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "%s(%u, %s)\n", func, id,
                  _mesa_enum_to_string(pname));

   if (id)
      q = _mesa_lookup_query_object(ctx, id);

   if (!q || q->Active || !q->EverBound) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(id=%d is invalid or active)", func, id);
      return;
   }

   /* From GL_EXT_occlusion_query_boolean spec:
    *
    *    "Accepted by the <pname> parameter of GetQueryObjectivEXT and
    *    GetQueryObjectuivEXT:
    *
    *    QUERY_RESULT_EXT                               0x8866
    *    QUERY_RESULT_AVAILABLE_EXT                     0x8867"
    *
    * Same rule is present also in ES 3.2 spec.
    */
   if (_mesa_is_gles(ctx) &&
       (pname != GL_QUERY_RESULT && pname != GL_QUERY_RESULT_AVAILABLE)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(%s)", func,
                  _mesa_enum_to_string(pname));
      return;
   }

   if (buf) {
      bool is_64bit = ptype == GL_INT64_ARB ||
         ptype == GL_UNSIGNED_INT64_ARB;
      if (!_mesa_has_ARB_query_buffer_object(ctx)) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(not supported)", func);
         return;
      }
      if (buf->Size < offset + 4 * (is_64bit ? 2 : 1)) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(out of bounds)", func);
         return;
      }

      if (offset < 0) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(offset is negative)", func);
         return;
      }

      switch (pname) {
      case GL_QUERY_RESULT:
      case GL_QUERY_RESULT_NO_WAIT:
      case GL_QUERY_RESULT_AVAILABLE:
      case GL_QUERY_TARGET:
         store_query_result(ctx, q, buf, offset, pname, ptype);
         return;
      }

      /* fall through to get error below */
   }

   switch (pname) {
   case GL_QUERY_RESULT:
      if (!q->Ready)
         _mesa_wait_query(ctx, q);
      value = q->Result;
      break;
   case GL_QUERY_RESULT_NO_WAIT:
      if (!_mesa_has_ARB_query_buffer_object(ctx))
         goto invalid_enum;
      _mesa_check_query(ctx, q);
      if (!q->Ready)
         return;
      value = q->Result;
      break;
   case GL_QUERY_RESULT_AVAILABLE:
      if (!q->Ready)
         _mesa_check_query(ctx, q);
      value = q->Ready;
      break;
   case GL_QUERY_TARGET:
      value = q->Target;
      break;
   default:
invalid_enum:
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(pname=%s)",
                  func, _mesa_enum_to_string(pname));
      return;
   }

   switch (ptype) {
   case GL_INT: {
      GLint *param = (GLint *)offset;
      if (value > 0x7fffffff)
         *param = 0x7fffffff;
      else
         *param = value;
      break;
   }
   case GL_UNSIGNED_INT: {
      GLuint *param = (GLuint *)offset;
      if (value > 0xffffffff)
         *param = 0xffffffff;
      else
         *param = value;
      break;
   }
   case GL_INT64_ARB:
   case GL_UNSIGNED_INT64_ARB: {
      GLuint64EXT *param = (GLuint64EXT *)offset;
      *param = value;
      break;
   }
   default:
      unreachable("unexpected ptype");
   }
}

void GLAPIENTRY
_mesa_GetQueryObjectiv(GLuint id, GLenum pname, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);

   get_query_object(ctx, "glGetQueryObjectiv",
                    id, pname, GL_INT, ctx->QueryBuffer, (intptr_t)params);
}


void GLAPIENTRY
_mesa_GetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
   GET_CURRENT_CONTEXT(ctx);

   get_query_object(ctx, "glGetQueryObjectuiv",
                    id, pname, GL_UNSIGNED_INT,
                    ctx->QueryBuffer, (intptr_t)params);
}


/**
 * New with GL_EXT_timer_query
 */
void GLAPIENTRY
_mesa_GetQueryObjecti64v(GLuint id, GLenum pname, GLint64EXT *params)
{
   GET_CURRENT_CONTEXT(ctx);

   get_query_object(ctx, "glGetQueryObjecti64v",
                    id, pname, GL_INT64_ARB,
                    ctx->QueryBuffer, (intptr_t)params);
}


/**
 * New with GL_EXT_timer_query
 */
void GLAPIENTRY
_mesa_GetQueryObjectui64v(GLuint id, GLenum pname, GLuint64EXT *params)
{
   GET_CURRENT_CONTEXT(ctx);

   get_query_object(ctx, "glGetQueryObjectui64v",
                    id, pname, GL_UNSIGNED_INT64_ARB,
                    ctx->QueryBuffer, (intptr_t)params);
}

/**
 * New with GL_ARB_query_buffer_object
 */
void GLAPIENTRY
_mesa_GetQueryBufferObjectiv(GLuint id, GLuint buffer, GLenum pname,
                             GLintptr offset)
{
   struct gl_buffer_object *buf;
   GET_CURRENT_CONTEXT(ctx);

   buf = _mesa_lookup_bufferobj_err(ctx, buffer, "glGetQueryBufferObjectiv");
   if (!buf)
      return;

   get_query_object(ctx, "glGetQueryBufferObjectiv",
                    id, pname, GL_INT, buf, offset);
}


void GLAPIENTRY
_mesa_GetQueryBufferObjectuiv(GLuint id, GLuint buffer, GLenum pname,
                              GLintptr offset)
{
   struct gl_buffer_object *buf;
   GET_CURRENT_CONTEXT(ctx);

   buf = _mesa_lookup_bufferobj_err(ctx, buffer, "glGetQueryBufferObjectuiv");
   if (!buf)
      return;

   get_query_object(ctx, "glGetQueryBufferObjectuiv",
                    id, pname, GL_UNSIGNED_INT, buf, offset);
}


void GLAPIENTRY
_mesa_GetQueryBufferObjecti64v(GLuint id, GLuint buffer, GLenum pname,
                               GLintptr offset)
{
   struct gl_buffer_object *buf;
   GET_CURRENT_CONTEXT(ctx);

   buf = _mesa_lookup_bufferobj_err(ctx, buffer, "glGetQueryBufferObjecti64v");
   if (!buf)
      return;

   get_query_object(ctx, "glGetQueryBufferObjecti64v",
                    id, pname, GL_INT64_ARB, buf, offset);
}


void GLAPIENTRY
_mesa_GetQueryBufferObjectui64v(GLuint id, GLuint buffer, GLenum pname,
                                GLintptr offset)
{
   struct gl_buffer_object *buf;
   GET_CURRENT_CONTEXT(ctx);

   buf = _mesa_lookup_bufferobj_err(ctx, buffer, "glGetQueryBufferObjectui64v");
   if (!buf)
      return;

   get_query_object(ctx, "glGetQueryBufferObjectui64v",
                    id, pname, GL_UNSIGNED_INT64_ARB, buf, offset);
}


/**
 * Allocate/init the context state related to query objects.
 */
void
_mesa_init_queryobj(struct gl_context *ctx)
{
   struct pipe_screen *screen = ctx->pipe->screen;

   ctx->Query.QueryObjects = _mesa_NewHashTable();
   ctx->Query.CurrentOcclusionObject = NULL;

   if (screen->get_param(screen, PIPE_CAP_OCCLUSION_QUERY))
      ctx->Const.QueryCounterBits.SamplesPassed = 64;
   else
      ctx->Const.QueryCounterBits.SamplesPassed = 0;

   ctx->Const.QueryCounterBits.TimeElapsed = 64;
   ctx->Const.QueryCounterBits.Timestamp = 64;
   ctx->Const.QueryCounterBits.PrimitivesGenerated = 64;
   ctx->Const.QueryCounterBits.PrimitivesWritten = 64;

   if (screen->get_param(screen, PIPE_CAP_QUERY_PIPELINE_STATISTICS) ||
       screen->get_param(screen, PIPE_CAP_QUERY_PIPELINE_STATISTICS_SINGLE)) {
      ctx->Const.QueryCounterBits.VerticesSubmitted = 64;
      ctx->Const.QueryCounterBits.PrimitivesSubmitted = 64;
      ctx->Const.QueryCounterBits.VsInvocations = 64;
      ctx->Const.QueryCounterBits.TessPatches = 64;
      ctx->Const.QueryCounterBits.TessInvocations = 64;
      ctx->Const.QueryCounterBits.GsInvocations = 64;
      ctx->Const.QueryCounterBits.GsPrimitives = 64;
      ctx->Const.QueryCounterBits.FsInvocations = 64;
      ctx->Const.QueryCounterBits.ComputeInvocations = 64;
      ctx->Const.QueryCounterBits.ClInPrimitives = 64;
      ctx->Const.QueryCounterBits.ClOutPrimitives = 64;
   } else {
      ctx->Const.QueryCounterBits.VerticesSubmitted = 0;
      ctx->Const.QueryCounterBits.PrimitivesSubmitted = 0;
      ctx->Const.QueryCounterBits.VsInvocations = 0;
      ctx->Const.QueryCounterBits.TessPatches = 0;
      ctx->Const.QueryCounterBits.TessInvocations = 0;
      ctx->Const.QueryCounterBits.GsInvocations = 0;
      ctx->Const.QueryCounterBits.GsPrimitives = 0;
      ctx->Const.QueryCounterBits.FsInvocations = 0;
      ctx->Const.QueryCounterBits.ComputeInvocations = 0;
      ctx->Const.QueryCounterBits.ClInPrimitives = 0;
      ctx->Const.QueryCounterBits.ClOutPrimitives = 0;
   }
}


/**
 * Callback for deleting a query object.  Called by _mesa_HashDeleteAll().
 */
static void
delete_queryobj_cb(void *data, void *userData)
{
   struct gl_query_object *q= (struct gl_query_object *) data;
   struct gl_context *ctx = (struct gl_context *)userData;
   delete_query(ctx, q);
}


/**
 * Free the context state related to query objects.
 */
void
_mesa_free_queryobj_data(struct gl_context *ctx)
{
   _mesa_HashDeleteAll(ctx->Query.QueryObjects, delete_queryobj_cb, ctx);
   _mesa_DeleteHashTable(ctx->Query.QueryObjects);
}
