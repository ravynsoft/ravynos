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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file performance_monitor.c
 * Core Mesa support for the AMD_performance_monitor extension.
 *
 * In order to implement this extension, start by defining two enums:
 * one for Groups, and one for Counters.  These will be used as indexes into
 * arrays, so they should start at 0 and increment from there.
 *
 * Counter IDs need to be globally unique.  That is, you can't have counter 7
 * in group A and counter 7 in group B.  A global enum of all available
 * counters is a convenient way to guarantee this.
 */

#include <stdbool.h>
#include "util/glheader.h"
#include "context.h"
#include "enums.h"
#include "hash.h"
#include "macros.h"
#include "mtypes.h"
#include "performance_monitor.h"
#include "util/bitset.h"
#include "util/ralloc.h"
#include "util/u_memory.h"
#include "api_exec_decl.h"

#include "state_tracker/st_cb_bitmap.h"
#include "state_tracker/st_context.h"
#include "state_tracker/st_debug.h"

#include "pipe/p_context.h"
#include "pipe/p_screen.h"

void
_mesa_init_performance_monitors(struct gl_context *ctx)
{
   ctx->PerfMonitor.Monitors = _mesa_NewHashTable();
   ctx->PerfMonitor.NumGroups = 0;
   ctx->PerfMonitor.Groups = NULL;
}


static bool
init_perf_monitor(struct gl_context *ctx, struct gl_perf_monitor_object *m)
{
   struct pipe_context *pipe = ctx->pipe;
   unsigned *batch = NULL;
   unsigned num_active_counters = 0;
   unsigned max_batch_counters = 0;
   unsigned num_batch_counters = 0;
   int gid, cid;

   st_flush_bitmap_cache(st_context(ctx));

   /* Determine the number of active counters. */
   for (gid = 0; gid < ctx->PerfMonitor.NumGroups; gid++) {
      const struct gl_perf_monitor_group *g = &ctx->PerfMonitor.Groups[gid];

      if (m->ActiveGroups[gid] > g->MaxActiveCounters) {
         /* Maximum number of counters reached. Cannot start the session. */
         if (ST_DEBUG & DEBUG_MESA) {
            debug_printf("Maximum number of counters reached. "
                         "Cannot start the session!\n");
         }
         return false;
      }

      num_active_counters += m->ActiveGroups[gid];
      if (g->has_batch)
         max_batch_counters += m->ActiveGroups[gid];
   }

   if (!num_active_counters)
      return true;

   m->active_counters = CALLOC(num_active_counters,
                                 sizeof(*m->active_counters));
   if (!m->active_counters)
      return false;

   if (max_batch_counters) {
      batch = CALLOC(max_batch_counters, sizeof(*batch));
      if (!batch)
         return false;
   }

   /* Create a query for each active counter. */
   for (gid = 0; gid < ctx->PerfMonitor.NumGroups; gid++) {
      const struct gl_perf_monitor_group *g = &ctx->PerfMonitor.Groups[gid];

      BITSET_FOREACH_SET(cid, m->ActiveCounters[gid], g->NumCounters) {
         const struct gl_perf_monitor_counter *c = &g->Counters[cid];
         struct gl_perf_counter_object *cntr =
            &m->active_counters[m->num_active_counters];

         cntr->id       = cid;
         cntr->group_id = gid;
         if (c->flags & PIPE_DRIVER_QUERY_FLAG_BATCH) {
            cntr->batch_index = num_batch_counters;
            batch[num_batch_counters++] = c->query_type;
         } else {
            cntr->query = pipe->create_query(pipe, c->query_type, 0);
            if (!cntr->query)
               goto fail;
         }
         ++m->num_active_counters;
      }
   }

   /* Create the batch query. */
   if (num_batch_counters) {
      m->batch_query = pipe->create_batch_query(pipe, num_batch_counters,
                                                  batch);
      m->batch_result = CALLOC(num_batch_counters, sizeof(m->batch_result->batch[0]));
      if (!m->batch_query || !m->batch_result)
         goto fail;
   }

   FREE(batch);
   return true;

fail:
   FREE(batch);
   return false;
}

static void
do_reset_perf_monitor(struct gl_perf_monitor_object *m,
                   struct pipe_context *pipe)
{
   unsigned i;

   for (i = 0; i < m->num_active_counters; ++i) {
      struct pipe_query *query = m->active_counters[i].query;
      if (query)
         pipe->destroy_query(pipe, query);
   }
   FREE(m->active_counters);
   m->active_counters = NULL;
   m->num_active_counters = 0;

   if (m->batch_query) {
      pipe->destroy_query(pipe, m->batch_query);
      m->batch_query = NULL;
   }
   FREE(m->batch_result);
   m->batch_result = NULL;
}

static void
delete_perf_monitor(struct gl_context *ctx, struct gl_perf_monitor_object *m)
{
   struct pipe_context *pipe = st_context(ctx)->pipe;

   do_reset_perf_monitor(m, pipe);
   FREE(m);
}

static GLboolean
begin_perf_monitor(struct gl_context *ctx, struct gl_perf_monitor_object *m)
{
   struct pipe_context *pipe = st_context(ctx)->pipe;
   unsigned i;

   if (!m->num_active_counters) {
      /* Create a query for each active counter before starting
       * a new monitoring session. */
      if (!init_perf_monitor(ctx, m))
         goto fail;
   }

   /* Start the query for each active counter. */
   for (i = 0; i < m->num_active_counters; ++i) {
      struct pipe_query *query = m->active_counters[i].query;
      if (query && !pipe->begin_query(pipe, query))
          goto fail;
   }

   if (m->batch_query && !pipe->begin_query(pipe, m->batch_query))
      goto fail;

   return true;

fail:
   /* Failed to start the monitoring session. */
   do_reset_perf_monitor(m, pipe);
   return false;
}

static void
end_perf_monitor(struct gl_context *ctx, struct gl_perf_monitor_object *m)
{
   struct pipe_context *pipe = st_context(ctx)->pipe;
   unsigned i;

   /* Stop the query for each active counter. */
   for (i = 0; i < m->num_active_counters; ++i) {
      struct pipe_query *query = m->active_counters[i].query;
      if (query)
         pipe->end_query(pipe, query);
   }

   if (m->batch_query)
      pipe->end_query(pipe, m->batch_query);
}

static void
reset_perf_monitor(struct gl_context *ctx, struct gl_perf_monitor_object *m)
{
   struct pipe_context *pipe = st_context(ctx)->pipe;

   if (!m->Ended)
      end_perf_monitor(ctx, m);

   do_reset_perf_monitor(m, pipe);

   if (m->Active)
      begin_perf_monitor(ctx, m);
}

static GLboolean
is_perf_monitor_result_available(struct gl_context *ctx,
                                 struct gl_perf_monitor_object *m)
{
   struct pipe_context *pipe = st_context(ctx)->pipe;
   unsigned i;

   if (!m->num_active_counters)
      return false;

   /* The result of a monitoring session is only available if the query of
    * each active counter is idle. */
   for (i = 0; i < m->num_active_counters; ++i) {
      struct pipe_query *query = m->active_counters[i].query;
      union pipe_query_result result;
      if (query && !pipe->get_query_result(pipe, query, false, &result)) {
         /* The query is busy. */
         return false;
      }
   }

   if (m->batch_query &&
       !pipe->get_query_result(pipe, m->batch_query, false, m->batch_result))
      return false;

   return true;
}

static void
get_perf_monitor_result(struct gl_context *ctx,
                        struct gl_perf_monitor_object *m,
                        GLsizei dataSize,
                        GLuint *data,
                        GLint *bytesWritten)
{
   struct pipe_context *pipe = st_context(ctx)->pipe;
   unsigned i;

   /* Copy data to the supplied array (data).
    *
    * The output data format is: <group ID, counter ID, value> for each
    * active counter. The API allows counters to appear in any order.
    */
   GLsizei offset = 0;
   bool have_batch_query = false;

   if (m->batch_query)
      have_batch_query = pipe->get_query_result(pipe, m->batch_query, true,
                                                m->batch_result);

   /* Read query results for each active counter. */
   for (i = 0; i < m->num_active_counters; ++i) {
      struct gl_perf_counter_object *cntr = &m->active_counters[i];
      union pipe_query_result result = { 0 };
      int gid, cid;
      GLenum type;

      cid  = cntr->id;
      gid  = cntr->group_id;
      type = ctx->PerfMonitor.Groups[gid].Counters[cid].Type;

      if (cntr->query) {
         if (!pipe->get_query_result(pipe, cntr->query, true, &result))
            continue;
      } else {
         if (!have_batch_query)
            continue;
         result.batch[0] = m->batch_result->batch[cntr->batch_index];
      }

      data[offset++] = gid;
      data[offset++] = cid;
      switch (type) {
      case GL_UNSIGNED_INT64_AMD:
         memcpy(&data[offset], &result.u64, sizeof(uint64_t));
         offset += sizeof(uint64_t) / sizeof(GLuint);
         break;
      case GL_UNSIGNED_INT:
         memcpy(&data[offset], &result.u32, sizeof(uint32_t));
         offset += sizeof(uint32_t) / sizeof(GLuint);
         break;
      case GL_FLOAT:
      case GL_PERCENTAGE_AMD:
         memcpy(&data[offset], &result.f, sizeof(GLfloat));
         offset += sizeof(GLfloat) / sizeof(GLuint);
         break;
      }
   }

   if (bytesWritten)
      *bytesWritten = offset * sizeof(GLuint);
}

void
_mesa_free_perfomance_monitor_groups(struct gl_context *ctx)
{
   struct gl_perf_monitor_state *perfmon = &ctx->PerfMonitor;
   int gid;

   for (gid = 0; gid < perfmon->NumGroups; gid++) {
      FREE((void *)perfmon->Groups[gid].Counters);
   }
   FREE((void *)perfmon->Groups);
}

static inline void
init_groups(struct gl_context *ctx)
{
   if (likely(ctx->PerfMonitor.Groups))
      return;

   struct gl_perf_monitor_state *perfmon = &ctx->PerfMonitor;
   struct pipe_screen *screen = ctx->pipe->screen;
   struct gl_perf_monitor_group *groups = NULL;
   int num_counters, num_groups;
   int gid, cid;

   /* Get the number of available queries. */
   num_counters = screen->get_driver_query_info(screen, 0, NULL);

   /* Get the number of available groups. */
   num_groups = screen->get_driver_query_group_info(screen, 0, NULL);
   groups = CALLOC(num_groups, sizeof(*groups));
   if (!groups)
      return;

   for (gid = 0; gid < num_groups; gid++) {
      struct gl_perf_monitor_group *g = &groups[perfmon->NumGroups];
      struct pipe_driver_query_group_info group_info;
      struct gl_perf_monitor_counter *counters = NULL;

      if (!screen->get_driver_query_group_info(screen, gid, &group_info))
         continue;

      g->Name = group_info.name;
      g->MaxActiveCounters = group_info.max_active_queries;

      if (group_info.num_queries)
         counters = CALLOC(group_info.num_queries, sizeof(*counters));
      if (!counters)
         goto fail;
      g->Counters = counters;

      for (cid = 0; cid < num_counters; cid++) {
         struct gl_perf_monitor_counter *c = &counters[g->NumCounters];
         struct pipe_driver_query_info info;

         if (!screen->get_driver_query_info(screen, cid, &info))
            continue;
         if (info.group_id != gid)
            continue;

         c->Name = info.name;
         switch (info.type) {
            case PIPE_DRIVER_QUERY_TYPE_UINT64:
            case PIPE_DRIVER_QUERY_TYPE_BYTES:
            case PIPE_DRIVER_QUERY_TYPE_MICROSECONDS:
            case PIPE_DRIVER_QUERY_TYPE_HZ:
               c->Minimum.u64 = 0;
               c->Maximum.u64 = info.max_value.u64 ? info.max_value.u64 : UINT64_MAX;
               c->Type = GL_UNSIGNED_INT64_AMD;
               break;
            case PIPE_DRIVER_QUERY_TYPE_UINT:
               c->Minimum.u32 = 0;
               c->Maximum.u32 = info.max_value.u32 ? info.max_value.u32 : UINT32_MAX;
               c->Type = GL_UNSIGNED_INT;
               break;
            case PIPE_DRIVER_QUERY_TYPE_FLOAT:
               c->Minimum.f = 0.0;
               c->Maximum.f = info.max_value.f ? info.max_value.f : FLT_MAX;
               c->Type = GL_FLOAT;
               break;
            case PIPE_DRIVER_QUERY_TYPE_PERCENTAGE:
               c->Minimum.f = 0.0f;
               c->Maximum.f = 100.0f;
               c->Type = GL_PERCENTAGE_AMD;
               break;
            default:
               unreachable("Invalid driver query type!");
         }

         c->query_type = info.query_type;
         c->flags = info.flags;
         if (c->flags & PIPE_DRIVER_QUERY_FLAG_BATCH)
            g->has_batch = true;

         g->NumCounters++;
      }
      perfmon->NumGroups++;
   }
   perfmon->Groups = groups;

   return;

fail:
   for (gid = 0; gid < num_groups; gid++) {
      FREE((void *)groups[gid].Counters);
   }
   FREE(groups);
}

static struct gl_perf_monitor_object *
new_performance_monitor(struct gl_context *ctx, GLuint index)
{
   unsigned i;
   struct gl_perf_monitor_object *m = CALLOC_STRUCT(gl_perf_monitor_object);

   if (m == NULL)
      return NULL;

   m->Name = index;

   m->Active = false;

   m->ActiveGroups =
      rzalloc_array(NULL, unsigned, ctx->PerfMonitor.NumGroups);

   m->ActiveCounters =
      ralloc_array(NULL, BITSET_WORD *, ctx->PerfMonitor.NumGroups);

   if (m->ActiveGroups == NULL || m->ActiveCounters == NULL)
      goto fail;

   for (i = 0; i < ctx->PerfMonitor.NumGroups; i++) {
      const struct gl_perf_monitor_group *g = &ctx->PerfMonitor.Groups[i];

      m->ActiveCounters[i] = rzalloc_array(m->ActiveCounters, BITSET_WORD,
                                           BITSET_WORDS(g->NumCounters));
      if (m->ActiveCounters[i] == NULL)
         goto fail;
   }

   return m;

fail:
   ralloc_free(m->ActiveGroups);
   ralloc_free(m->ActiveCounters);
   delete_perf_monitor(ctx, m);
   return NULL;
}

static void
free_performance_monitor(void *data, void *user)
{
   struct gl_perf_monitor_object *m = data;
   struct gl_context *ctx = user;

   ralloc_free(m->ActiveGroups);
   ralloc_free(m->ActiveCounters);
   delete_perf_monitor(ctx, m);
}

void
_mesa_free_performance_monitors(struct gl_context *ctx)
{
   _mesa_HashDeleteAll(ctx->PerfMonitor.Monitors,
                       free_performance_monitor, ctx);
   _mesa_DeleteHashTable(ctx->PerfMonitor.Monitors);
}

static inline struct gl_perf_monitor_object *
lookup_monitor(struct gl_context *ctx, GLuint id)
{
   return (struct gl_perf_monitor_object *)
      _mesa_HashLookup(ctx->PerfMonitor.Monitors, id);
}

static inline const struct gl_perf_monitor_group *
get_group(const struct gl_context *ctx, GLuint id)
{
   if (id >= ctx->PerfMonitor.NumGroups)
      return NULL;

   return &ctx->PerfMonitor.Groups[id];
}

static inline const struct gl_perf_monitor_counter *
get_counter(const struct gl_perf_monitor_group *group_obj, GLuint id)
{
   if (id >= group_obj->NumCounters)
      return NULL;

   return &group_obj->Counters[id];
}

/*****************************************************************************/

void GLAPIENTRY
_mesa_GetPerfMonitorGroupsAMD(GLint *numGroups, GLsizei groupsSize,
                              GLuint *groups)
{
   GET_CURRENT_CONTEXT(ctx);
   init_groups(ctx);

   if (numGroups != NULL)
      *numGroups = ctx->PerfMonitor.NumGroups;

   if (groupsSize > 0 && groups != NULL) {
      unsigned i;
      unsigned n = MIN2((GLuint) groupsSize, ctx->PerfMonitor.NumGroups);

      /* We just use the index in the Groups array as the ID. */
      for (i = 0; i < n; i++)
         groups[i] = i;
   }
}

void GLAPIENTRY
_mesa_GetPerfMonitorCountersAMD(GLuint group, GLint *numCounters,
                                GLint *maxActiveCounters,
                                GLsizei countersSize, GLuint *counters)
{
   GET_CURRENT_CONTEXT(ctx);
   const struct gl_perf_monitor_group *group_obj;

   init_groups(ctx);

   group_obj = get_group(ctx, group);
   if (group_obj == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetPerfMonitorCountersAMD(invalid group)");
      return;
   }

   if (maxActiveCounters != NULL)
      *maxActiveCounters = group_obj->MaxActiveCounters;

   if (numCounters != NULL)
      *numCounters = group_obj->NumCounters;

   if (counters != NULL) {
      unsigned i;
      unsigned n = MIN2(group_obj->NumCounters, (GLuint) countersSize);
      for (i = 0; i < n; i++) {
         /* We just use the index in the Counters array as the ID. */
         counters[i] = i;
      }
   }
}

void GLAPIENTRY
_mesa_GetPerfMonitorGroupStringAMD(GLuint group, GLsizei bufSize,
                                   GLsizei *length, GLchar *groupString)
{
   GET_CURRENT_CONTEXT(ctx);
   const struct gl_perf_monitor_group *group_obj;

   init_groups(ctx);

   group_obj = get_group(ctx, group);
   if (group_obj == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGetPerfMonitorGroupStringAMD");
      return;
   }

   if (bufSize == 0) {
      /* Return the number of characters that would be required to hold the
       * group string, excluding the null terminator.
       */
      if (length != NULL)
         *length = strlen(group_obj->Name);
   } else {
      if (length != NULL)
         *length = MIN2(strlen(group_obj->Name), bufSize);
      if (groupString != NULL)
         strncpy(groupString, group_obj->Name, bufSize);
   }
}

void GLAPIENTRY
_mesa_GetPerfMonitorCounterStringAMD(GLuint group, GLuint counter,
                                     GLsizei bufSize, GLsizei *length,
                                     GLchar *counterString)
{
   GET_CURRENT_CONTEXT(ctx);

   const struct gl_perf_monitor_group *group_obj;
   const struct gl_perf_monitor_counter *counter_obj;

   init_groups(ctx);

   group_obj = get_group(ctx, group);

   if (group_obj == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetPerfMonitorCounterStringAMD(invalid group)");
      return;
   }

   counter_obj = get_counter(group_obj, counter);

   if (counter_obj == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetPerfMonitorCounterStringAMD(invalid counter)");
      return;
   }

   if (bufSize == 0) {
      /* Return the number of characters that would be required to hold the
       * counter string, excluding the null terminator.
       */
      if (length != NULL)
         *length = strlen(counter_obj->Name);
   } else {
      if (length != NULL)
         *length = MIN2(strlen(counter_obj->Name), bufSize);
      if (counterString != NULL)
         strncpy(counterString, counter_obj->Name, bufSize);
   }
}

void GLAPIENTRY
_mesa_GetPerfMonitorCounterInfoAMD(GLuint group, GLuint counter, GLenum pname,
                                   GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);

   const struct gl_perf_monitor_group *group_obj;
   const struct gl_perf_monitor_counter *counter_obj;

   init_groups(ctx);

   group_obj = get_group(ctx, group);

   if (group_obj == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetPerfMonitorCounterInfoAMD(invalid group)");
      return;
   }

   counter_obj = get_counter(group_obj, counter);

   if (counter_obj == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetPerfMonitorCounterInfoAMD(invalid counter)");
      return;
   }

   switch (pname) {
   case GL_COUNTER_TYPE_AMD:
      *((GLenum *) data) = counter_obj->Type;
      break;

   case GL_COUNTER_RANGE_AMD:
      switch (counter_obj->Type) {
      case GL_FLOAT:
      case GL_PERCENTAGE_AMD: {
         float *f_data = data;
         f_data[0] = counter_obj->Minimum.f;
         f_data[1] = counter_obj->Maximum.f;
         break;
      }
      case GL_UNSIGNED_INT: {
         uint32_t *u32_data = data;
         u32_data[0] = counter_obj->Minimum.u32;
         u32_data[1] = counter_obj->Maximum.u32;
         break;
      }
      case GL_UNSIGNED_INT64_AMD: {
         uint64_t *u64_data = data;
         u64_data[0] = counter_obj->Minimum.u64;
         u64_data[1] = counter_obj->Maximum.u64;
         break;
      }
      default:
         assert(!"Should not get here: invalid counter type");
      }
      break;

   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetPerfMonitorCounterInfoAMD(pname)");
      return;
   }
}

void GLAPIENTRY
_mesa_GenPerfMonitorsAMD(GLsizei n, GLuint *monitors)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glGenPerfMonitorsAMD(%d)\n", n);

   init_groups(ctx);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGenPerfMonitorsAMD(n < 0)");
      return;
   }

   if (monitors == NULL)
      return;

   if (_mesa_HashFindFreeKeys(ctx->PerfMonitor.Monitors, monitors, n)) {
      GLsizei i;
      for (i = 0; i < n; i++) {
         struct gl_perf_monitor_object *m =
            new_performance_monitor(ctx, monitors[i]);
         if (!m) {
            _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGenPerfMonitorsAMD");
            return;
         }
         _mesa_HashInsert(ctx->PerfMonitor.Monitors, monitors[i], m, true);
      }
   } else {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGenPerfMonitorsAMD");
      return;
   }
}

void GLAPIENTRY
_mesa_DeletePerfMonitorsAMD(GLsizei n, GLuint *monitors)
{
   GLint i;
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glDeletePerfMonitorsAMD(%d)\n", n);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDeletePerfMonitorsAMD(n < 0)");
      return;
   }

   if (monitors == NULL)
      return;

   for (i = 0; i < n; i++) {
      struct gl_perf_monitor_object *m = lookup_monitor(ctx, monitors[i]);

      if (m) {
         /* Give the driver a chance to stop the monitor if it's active. */
         if (m->Active) {
            reset_perf_monitor(ctx, m);
            m->Ended = false;
         }

         _mesa_HashRemove(ctx->PerfMonitor.Monitors, monitors[i]);
         ralloc_free(m->ActiveGroups);
         ralloc_free(m->ActiveCounters);
         delete_perf_monitor(ctx, m);
      } else {
         /* "INVALID_VALUE error will be generated if any of the monitor IDs
          *  in the <monitors> parameter to DeletePerfMonitorsAMD do not
          *  reference a valid generated monitor ID."
          */
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glDeletePerfMonitorsAMD(invalid monitor)");
      }
   }
}

void GLAPIENTRY
_mesa_SelectPerfMonitorCountersAMD(GLuint monitor, GLboolean enable,
                                   GLuint group, GLint numCounters,
                                   GLuint *counterList)
{
   GET_CURRENT_CONTEXT(ctx);
   int i;
   struct gl_perf_monitor_object *m;
   const struct gl_perf_monitor_group *group_obj;

   m = lookup_monitor(ctx, monitor);

   /* "INVALID_VALUE error will be generated if the <monitor> parameter to
    *  SelectPerfMonitorCountersAMD does not reference a monitor created by
    *  GenPerfMonitorsAMD."
    */
   if (m == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glSelectPerfMonitorCountersAMD(invalid monitor)");
      return;
   }

   group_obj = get_group(ctx, group);

   /* "INVALID_VALUE error will be generated if the <group> parameter to
    *  GetPerfMonitorCountersAMD, GetPerfMonitorCounterStringAMD,
    *  GetPerfMonitorCounterStringAMD, GetPerfMonitorCounterInfoAMD, or
    *  SelectPerfMonitorCountersAMD does not reference a valid group ID."
    */
   if (group_obj == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glSelectPerfMonitorCountersAMD(invalid group)");
      return;
   }

   /* "INVALID_VALUE error will be generated if the <numCounters> parameter to
    *  SelectPerfMonitorCountersAMD is less than 0."
    */
   if (numCounters < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glSelectPerfMonitorCountersAMD(numCounters < 0)");
      return;
   }

   /* "When SelectPerfMonitorCountersAMD is called on a monitor, any outstanding
    *  results for that monitor become invalidated and the result queries
    *  PERFMON_RESULT_SIZE_AMD and PERFMON_RESULT_AVAILABLE_AMD are reset to 0."
    */
   reset_perf_monitor(ctx, m);

   /* Sanity check the counter ID list. */
   for (i = 0; i < numCounters; i++) {
      if (counterList[i] >= group_obj->NumCounters) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glSelectPerfMonitorCountersAMD(invalid counter ID)");
         return;
      }
   }

   if (enable) {
      /* Enable the counters */
      for (i = 0; i < numCounters; i++) {
         if (!BITSET_TEST(m->ActiveCounters[group], counterList[i])) {
            ++m->ActiveGroups[group];
            BITSET_SET(m->ActiveCounters[group], counterList[i]);
         }
      }
   } else {
      /* Disable the counters */
      for (i = 0; i < numCounters; i++) {
         if (BITSET_TEST(m->ActiveCounters[group], counterList[i])) {
            --m->ActiveGroups[group];
            BITSET_CLEAR(m->ActiveCounters[group], counterList[i]);
         }
      }
   }
}

void GLAPIENTRY
_mesa_BeginPerfMonitorAMD(GLuint monitor)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_perf_monitor_object *m = lookup_monitor(ctx, monitor);

   if (m == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glBeginPerfMonitorAMD(invalid monitor)");
      return;
   }

   /* "INVALID_OPERATION error will be generated if BeginPerfMonitorAMD is
    *  called when a performance monitor is already active."
    */
   if (m->Active) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBeginPerfMonitor(already active)");
      return;
   }

   /* The driver is free to return false if it can't begin monitoring for
    * any reason.  This translates into an INVALID_OPERATION error.
    */
   if (begin_perf_monitor(ctx, m)) {
      m->Active = true;
      m->Ended = false;
   } else {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBeginPerfMonitor(driver unable to begin monitoring)");
   }
}

void GLAPIENTRY
_mesa_EndPerfMonitorAMD(GLuint monitor)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_perf_monitor_object *m = lookup_monitor(ctx, monitor);

   if (m == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glEndPerfMonitorAMD(invalid monitor)");
      return;
   }

   /* "INVALID_OPERATION error will be generated if EndPerfMonitorAMD is called
    *  when a performance monitor is not currently started."
    */
   if (!m->Active) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glEndPerfMonitor(not active)");
      return;
   }

   end_perf_monitor(ctx, m);

   m->Active = false;
   m->Ended = true;
}

/**
 * Return the number of bytes needed to store a monitor's result.
 */
static unsigned
perf_monitor_result_size(const struct gl_context *ctx,
                         const struct gl_perf_monitor_object *m)
{
   unsigned group, counter;
   unsigned size = 0;

   for (group = 0; group < ctx->PerfMonitor.NumGroups; group++) {
      const struct gl_perf_monitor_group *g = &ctx->PerfMonitor.Groups[group];

      BITSET_FOREACH_SET(counter, m->ActiveCounters[group], g->NumCounters) {
         const struct gl_perf_monitor_counter *c = &g->Counters[counter];

         size += sizeof(uint32_t); /* Group ID */
         size += sizeof(uint32_t); /* Counter ID */
         size += _mesa_perf_monitor_counter_size(c);
      }
   }
   return size;
}

void GLAPIENTRY
_mesa_GetPerfMonitorCounterDataAMD(GLuint monitor, GLenum pname,
                                   GLsizei dataSize, GLuint *data,
                                   GLint *bytesWritten)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_perf_monitor_object *m = lookup_monitor(ctx, monitor);
   bool result_available;

   if (m == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetPerfMonitorCounterDataAMD(invalid monitor)");
      return;
   }

   /* "It is an INVALID_OPERATION error for <data> to be NULL." */
   if (data == NULL) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetPerfMonitorCounterDataAMD(data == NULL)");
      return;
   }

   /* We need at least enough room for a single value. */
   if (dataSize < sizeof(GLuint)) {
      if (bytesWritten != NULL)
         *bytesWritten = 0;
      return;
   }

   /* If the monitor has never ended, there is no result. */
   result_available = m->Ended &&
      is_perf_monitor_result_available(ctx, m);

   /* AMD appears to return 0 for all queries unless a result is available. */
   if (!result_available) {
      *data = 0;
      if (bytesWritten != NULL)
         *bytesWritten = sizeof(GLuint);
      return;
   }

   switch (pname) {
   case GL_PERFMON_RESULT_AVAILABLE_AMD:
      *data = 1;
      if (bytesWritten != NULL)
         *bytesWritten = sizeof(GLuint);
      break;
   case GL_PERFMON_RESULT_SIZE_AMD:
      *data = perf_monitor_result_size(ctx, m);
      if (bytesWritten != NULL)
         *bytesWritten = sizeof(GLuint);
      break;
   case GL_PERFMON_RESULT_AMD:
      get_perf_monitor_result(ctx, m, dataSize, data, bytesWritten);
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetPerfMonitorCounterDataAMD(pname)");
   }
}

/**
 * Returns how many bytes a counter's value takes up.
 */
unsigned
_mesa_perf_monitor_counter_size(const struct gl_perf_monitor_counter *c)
{
   switch (c->Type) {
   case GL_FLOAT:
   case GL_PERCENTAGE_AMD:
      return sizeof(GLfloat);
   case GL_UNSIGNED_INT:
      return sizeof(GLuint);
   case GL_UNSIGNED_INT64_AMD:
      return sizeof(uint64_t);
   default:
      assert(!"Should not get here: invalid counter type");
      return 0;
   }
}
