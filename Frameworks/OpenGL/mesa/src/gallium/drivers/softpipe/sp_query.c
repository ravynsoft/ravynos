/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/* Author:
 *    Keith Whitwell <keithw@vmware.com>
 */

#include "draw/draw_context.h"
#include "util/os_time.h"
#include "pipe/p_defines.h"
#include "util/u_memory.h"
#include "sp_context.h"
#include "sp_query.h"
#include "sp_state.h"

struct softpipe_query {
   unsigned type;
   unsigned index;
   uint64_t start;
   uint64_t end;
   struct pipe_query_data_so_statistics so[PIPE_MAX_VERTEX_STREAMS];
   struct pipe_query_data_pipeline_statistics stats;
};


static struct softpipe_query *softpipe_query( struct pipe_query *p )
{
   return (struct softpipe_query *)p;
}

static struct pipe_query *
softpipe_create_query(struct pipe_context *pipe, 
		      unsigned type,
		      unsigned index)
{
   struct softpipe_query* sq;

   assert(type == PIPE_QUERY_OCCLUSION_COUNTER ||
          type == PIPE_QUERY_OCCLUSION_PREDICATE ||
          type == PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE ||
          type == PIPE_QUERY_TIME_ELAPSED ||
          type == PIPE_QUERY_SO_STATISTICS ||
          type == PIPE_QUERY_SO_OVERFLOW_PREDICATE ||
          type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE ||
          type == PIPE_QUERY_PRIMITIVES_EMITTED ||
          type == PIPE_QUERY_PRIMITIVES_GENERATED || 
          type == PIPE_QUERY_PIPELINE_STATISTICS ||
          type == PIPE_QUERY_GPU_FINISHED ||
          type == PIPE_QUERY_TIMESTAMP ||
          type == PIPE_QUERY_TIMESTAMP_DISJOINT);
   sq = CALLOC_STRUCT( softpipe_query );
   sq->type = type;
   sq->index = index;
   return (struct pipe_query *)sq;
}


static void
softpipe_destroy_query(struct pipe_context *pipe, struct pipe_query *q)
{
   FREE(q);
}


static bool
softpipe_begin_query(struct pipe_context *pipe, struct pipe_query *q)
{
   struct softpipe_context *softpipe = softpipe_context( pipe );
   struct softpipe_query *sq = softpipe_query(q);

   switch (sq->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      sq->start = softpipe->occlusion_count;
      break;
   case PIPE_QUERY_TIME_ELAPSED:
      sq->start = os_time_get_nano();
      break;
   case PIPE_QUERY_SO_STATISTICS:
      sq->so[sq->index].num_primitives_written = softpipe->so_stats[sq->index].num_primitives_written;
      sq->so[sq->index].primitives_storage_needed = softpipe->so_stats[sq->index].primitives_storage_needed;
      break;
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      sq->so[sq->index].num_primitives_written = softpipe->so_stats[sq->index].num_primitives_written;
      sq->so[sq->index].primitives_storage_needed = softpipe->so_stats[sq->index].primitives_storage_needed;
      break;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      for (unsigned i = 0; i < PIPE_MAX_VERTEX_STREAMS; i++) {
         sq->so[i].num_primitives_written = softpipe->so_stats[i].num_primitives_written;
         sq->so[i].primitives_storage_needed = softpipe->so_stats[i].primitives_storage_needed;
      }
      break;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      sq->so[sq->index].num_primitives_written = softpipe->so_stats[sq->index].num_primitives_written;
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      sq->so[sq->index].primitives_storage_needed = softpipe->so_stats[sq->index].primitives_storage_needed;
      break;
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_GPU_FINISHED:
   case PIPE_QUERY_TIMESTAMP_DISJOINT:
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      /* reset our cache */
      if (softpipe->active_statistics_queries == 0) {
         memset(&softpipe->pipeline_statistics, 0,
                sizeof(softpipe->pipeline_statistics));
      }
      memcpy(&sq->stats, &softpipe->pipeline_statistics,
             sizeof(sq->stats));
      softpipe->active_statistics_queries++;
      break;
   default:
      assert(0);
      break;
   }
   softpipe->active_query_count++;
   softpipe->dirty |= SP_NEW_QUERY;
   return true;
}


static bool
softpipe_end_query(struct pipe_context *pipe, struct pipe_query *q)
{
   struct softpipe_context *softpipe = softpipe_context( pipe );
   struct softpipe_query *sq = softpipe_query(q);

   softpipe->active_query_count--;
   switch (sq->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      sq->end = softpipe->occlusion_count;
      break;
   case PIPE_QUERY_TIMESTAMP:
      sq->start = 0;
      FALLTHROUGH;
   case PIPE_QUERY_TIME_ELAPSED:
      sq->end = os_time_get_nano();
      break;
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      sq->so[sq->index].num_primitives_written =
         softpipe->so_stats[sq->index].num_primitives_written - sq->so[sq->index].num_primitives_written;
      sq->so[sq->index].primitives_storage_needed =
         softpipe->so_stats[sq->index].primitives_storage_needed - sq->so[sq->index].primitives_storage_needed;
      sq->end = sq->so[sq->index].primitives_storage_needed > sq->so[sq->index].num_primitives_written;
      break;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      sq->end = 0;
      for (unsigned i = 0; i < PIPE_MAX_VERTEX_STREAMS; i++) {
         sq->so[i].num_primitives_written =
            softpipe->so_stats[i].num_primitives_written - sq->so[i].num_primitives_written;
         sq->so[i].primitives_storage_needed =
            softpipe->so_stats[i].primitives_storage_needed - sq->so[i].primitives_storage_needed;
         sq->end |= sq->so[i].primitives_storage_needed > sq->so[i].num_primitives_written;
      }
      break;
   case PIPE_QUERY_SO_STATISTICS:
      sq->so[sq->index].num_primitives_written =
         softpipe->so_stats[sq->index].num_primitives_written - sq->so[sq->index].num_primitives_written;
      sq->so[sq->index].primitives_storage_needed =
         softpipe->so_stats[sq->index].primitives_storage_needed - sq->so[sq->index].primitives_storage_needed;
      break;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      sq->so[sq->index].num_primitives_written =
         softpipe->so_stats[sq->index].num_primitives_written - sq->so[sq->index].num_primitives_written;
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      sq->so[sq->index].primitives_storage_needed =
         softpipe->so_stats[sq->index].primitives_storage_needed - sq->so[sq->index].primitives_storage_needed;
      break;
   case PIPE_QUERY_GPU_FINISHED:
   case PIPE_QUERY_TIMESTAMP_DISJOINT:
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      sq->stats.ia_vertices =
         softpipe->pipeline_statistics.ia_vertices - sq->stats.ia_vertices;
      sq->stats.ia_primitives =
         softpipe->pipeline_statistics.ia_primitives - sq->stats.ia_primitives;
      sq->stats.vs_invocations =
         softpipe->pipeline_statistics.vs_invocations - sq->stats.vs_invocations;
      sq->stats.gs_invocations =
         softpipe->pipeline_statistics.gs_invocations - sq->stats.gs_invocations;
      sq->stats.gs_primitives =
         softpipe->pipeline_statistics.gs_primitives - sq->stats.gs_primitives;
      sq->stats.c_invocations =
         softpipe->pipeline_statistics.c_invocations - sq->stats.c_invocations;
      sq->stats.c_primitives =
         softpipe->pipeline_statistics.c_primitives - sq->stats.c_primitives;
      sq->stats.ps_invocations =
         softpipe->pipeline_statistics.ps_invocations - sq->stats.ps_invocations;
      sq->stats.cs_invocations =
         softpipe->pipeline_statistics.cs_invocations - sq->stats.cs_invocations;

      softpipe->active_statistics_queries--;
      break;
   default:
      assert(0);
      break;
   }
   softpipe->dirty |= SP_NEW_QUERY;
   return true;
}


static bool
softpipe_get_query_result(struct pipe_context *pipe, 
                          struct pipe_query *q,
                          bool wait,
                          union pipe_query_result *vresult)
{
   struct softpipe_query *sq = softpipe_query(q);
   uint64_t *result = (uint64_t*)vresult;

   switch (sq->type) {
   case PIPE_QUERY_SO_STATISTICS: {
      struct pipe_query_data_so_statistics *stats =
         (struct pipe_query_data_so_statistics *)vresult;
      stats->num_primitives_written = sq->so[sq->index].num_primitives_written;
      stats->primitives_storage_needed = sq->so[sq->index].primitives_storage_needed;
   }
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      memcpy(vresult, &sq->stats,
             sizeof(struct pipe_query_data_pipeline_statistics));
      break;
   case PIPE_QUERY_GPU_FINISHED:
      vresult->b = true;
      break;
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      vresult->b = sq->end != 0;
      break;
   case PIPE_QUERY_TIMESTAMP_DISJOINT: {
      struct pipe_query_data_timestamp_disjoint *td =
          (struct pipe_query_data_timestamp_disjoint *)vresult;
      /* os_get_time_nano return nanoseconds */
      td->frequency = UINT64_C(1000000000);
      td->disjoint = false;
   }
      break;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      *result = sq->so[sq->index].num_primitives_written;
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      *result = sq->so[sq->index].primitives_storage_needed;
      break;
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      vresult->b = sq->end - sq->start != 0;
      break;
   default:
      *result = sq->end - sq->start;
      break;
   }
   return true;
}

static bool
is_result_nonzero(struct pipe_query *q,
                  union pipe_query_result *vresult)
{
   struct softpipe_query *sq = softpipe_query(q);

   switch (sq->type) {
   case PIPE_QUERY_TIMESTAMP_DISJOINT:
   case PIPE_QUERY_SO_STATISTICS:
   case PIPE_QUERY_PIPELINE_STATISTICS:
      unreachable("unpossible");
      break;
   case PIPE_QUERY_GPU_FINISHED:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      return vresult->b;
   default:
      return !!vresult->u64;
   }
   return false;
}

/**
 * Called by rendering function to check rendering is conditional.
 * \return TRUE if we should render, FALSE if we should skip rendering
 */
bool
softpipe_check_render_cond(struct softpipe_context *sp)
{
   struct pipe_context *pipe = &sp->pipe;
   bool b, wait;
   union pipe_query_result result;
   memset(&result, 0, sizeof(union pipe_query_result));

   if (!sp->render_cond_query) {
      return true;  /* no query predicate, draw normally */
   }

   wait = (sp->render_cond_mode == PIPE_RENDER_COND_WAIT ||
           sp->render_cond_mode == PIPE_RENDER_COND_BY_REGION_WAIT);

   b = pipe->get_query_result(pipe, sp->render_cond_query, wait,
                              &result);
   if (b)
      return !is_result_nonzero(sp->render_cond_query, &result) == sp->render_cond_cond;
   else
      return true;
}


static void
softpipe_set_active_query_state(struct pipe_context *pipe, bool enable)
{
}


void softpipe_init_query_funcs(struct softpipe_context *softpipe )
{
   softpipe->pipe.create_query = softpipe_create_query;
   softpipe->pipe.destroy_query = softpipe_destroy_query;
   softpipe->pipe.begin_query = softpipe_begin_query;
   softpipe->pipe.end_query = softpipe_end_query;
   softpipe->pipe.get_query_result = softpipe_get_query_result;
   softpipe->pipe.set_active_query_state = softpipe_set_active_query_state;
}


