/**************************************************************************
 *
 * Copyright 2013 Marek Olšák <maraeo@gmail.com>
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
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/* This file contains code for reading values from pipe queries
 * for displaying on the HUD. To prevent stalls when reading queries, we
 * keep a list of busy queries in a ring. We read only those queries which
 * are idle.
 */

#include "hud/hud_private.h"
#include "pipe/p_screen.h"
#include "util/os_time.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include <stdio.h>

// Must be a power of two
#define NUM_QUERIES 8

struct hud_batch_query_context {
   unsigned num_query_types;
   unsigned allocated_query_types;
   unsigned *query_types;

   bool failed;
   struct pipe_query *query[NUM_QUERIES];
   union pipe_query_result *result[NUM_QUERIES];
   unsigned head, pending, results;
};

void
hud_batch_query_update(struct hud_batch_query_context *bq,
                       struct pipe_context *pipe)
{
   if (!bq || bq->failed)
      return;

   if (bq->query[bq->head])
      pipe->end_query(pipe, bq->query[bq->head]);

   bq->results = 0;

   while (bq->pending) {
      unsigned idx = (bq->head - bq->pending + 1) % NUM_QUERIES;
      struct pipe_query *query = bq->query[idx];

      if (!bq->result[idx])
         bq->result[idx] = MALLOC(sizeof(bq->result[idx]->batch[0]) *
                                  bq->num_query_types);
      if (!bq->result[idx]) {
         fprintf(stderr, "gallium_hud: out of memory.\n");
         bq->failed = true;
         return;
      }

      if (!pipe->get_query_result(pipe, query, false, bq->result[idx]))
         break;

      ++bq->results;
      --bq->pending;
   }

   bq->head = (bq->head + 1) % NUM_QUERIES;

   if (bq->pending == NUM_QUERIES) {
      fprintf(stderr,
              "gallium_hud: all queries busy after %i frames, dropping data.\n",
              NUM_QUERIES);

      assert(bq->query[bq->head]);

      pipe->destroy_query(pipe, bq->query[bq->head]);
      bq->query[bq->head] = NULL;
   }

   ++bq->pending;

   if (!bq->query[bq->head]) {
      bq->query[bq->head] = pipe->create_batch_query(pipe,
                                                     bq->num_query_types,
                                                     bq->query_types);

      if (!bq->query[bq->head]) {
         fprintf(stderr,
                 "gallium_hud: create_batch_query failed. You may have "
                 "selected too many or incompatible queries.\n");
         bq->failed = true;
         return;
      }
   }
}

void
hud_batch_query_begin(struct hud_batch_query_context *bq,
                      struct pipe_context *pipe)
{
   if (!bq || bq->failed || !bq->query[bq->head])
      return;

   if (!pipe->begin_query(pipe, bq->query[bq->head])) {
      fprintf(stderr,
              "gallium_hud: could not begin batch query. You may have "
              "selected too many or incompatible queries.\n");
      bq->failed = true;
   }
}

static bool
batch_query_add(struct hud_batch_query_context **pbq,
                unsigned query_type, unsigned *result_index)
{
   struct hud_batch_query_context *bq = *pbq;
   unsigned i;

   if (!bq) {
      bq = CALLOC_STRUCT(hud_batch_query_context);
      if (!bq)
         return false;
      *pbq = bq;
   }

   for (i = 0; i < bq->num_query_types; ++i) {
      if (bq->query_types[i] == query_type) {
         *result_index = i;
         return true;
      }
   }

   if (bq->num_query_types == bq->allocated_query_types) {
      unsigned new_alloc = MAX2(16, bq->allocated_query_types * 2);
      unsigned *new_query_types
         = REALLOC(bq->query_types,
                   bq->allocated_query_types * sizeof(unsigned),
                   new_alloc * sizeof(unsigned));
      if (!new_query_types)
         return false;
      bq->query_types = new_query_types;
      bq->allocated_query_types = new_alloc;
   }

   bq->query_types[bq->num_query_types] = query_type;
   *result_index = bq->num_query_types++;
   return true;
}

void
hud_batch_query_cleanup(struct hud_batch_query_context **pbq,
                        struct pipe_context *pipe)
{
   struct hud_batch_query_context *bq = *pbq;
   unsigned idx;

   if (!bq)
      return;

   *pbq = NULL;

   if (bq->query[bq->head] && !bq->failed)
      pipe->end_query(pipe, bq->query[bq->head]);

   for (idx = 0; idx < NUM_QUERIES; ++idx) {
      if (bq->query[idx])
         pipe->destroy_query(pipe, bq->query[idx]);
      FREE(bq->result[idx]);
   }

   FREE(bq->query_types);
   FREE(bq);
}

struct query_info {
   struct hud_batch_query_context *batch;
   enum pipe_query_type query_type;

   /** index to choose fields in pipe_query_data_pipeline_statistics,
    * for example.
    */
   unsigned result_index;
   enum pipe_driver_query_result_type result_type;
   enum pipe_driver_query_type type;

   /* Ring of queries. If a query is busy, we use another slot. */
   struct pipe_query *query[NUM_QUERIES];
   unsigned head, tail;

   uint64_t last_time;
   uint64_t results_cumulative;
   unsigned num_results;
};

static void
query_new_value_batch(struct query_info *info)
{
   struct hud_batch_query_context *bq = info->batch;
   unsigned result_index = info->result_index;
   unsigned idx = (bq->head - bq->pending) % NUM_QUERIES;
   unsigned results = bq->results;

   while (results) {
      info->results_cumulative += bq->result[idx]->batch[result_index].u64;
      ++info->num_results;

      --results;
      idx = (idx - 1) % NUM_QUERIES;
   }
}

static void
query_new_value_normal(struct query_info *info, struct pipe_context *pipe)
{
   if (info->last_time) {
      if (info->query[info->head])
         pipe->end_query(pipe, info->query[info->head]);

      /* read query results */
      while (1) {
         struct pipe_query *query = info->query[info->tail];
         union pipe_query_result result;
         uint64_t *res64 = (uint64_t *)&result;

         if (query && pipe->get_query_result(pipe, query, false, &result)) {
            if (info->type == PIPE_DRIVER_QUERY_TYPE_FLOAT) {
               assert(info->result_index == 0);
               info->results_cumulative += (uint64_t) (result.f * 1000.0f);
            }
            else {
               info->results_cumulative += res64[info->result_index];
            }
            info->num_results++;

            if (info->tail == info->head)
               break;

            info->tail = (info->tail+1) % NUM_QUERIES;
         }
         else {
            /* the oldest query is busy */
            if ((info->head+1) % NUM_QUERIES == info->tail) {
               /* all queries are busy, throw away the last query and create
                * a new one */
               fprintf(stderr,
                       "gallium_hud: all queries are busy after %i frames, "
                       "can't add another query\n",
                       NUM_QUERIES);
               if (info->query[info->head])
                  pipe->destroy_query(pipe, info->query[info->head]);
               info->query[info->head] =
                     pipe->create_query(pipe, info->query_type, 0);
            }
            else {
               /* the last query is busy, we need to add a new one we can use
                * for this frame */
               info->head = (info->head+1) % NUM_QUERIES;
               if (!info->query[info->head]) {
                  info->query[info->head] =
                        pipe->create_query(pipe, info->query_type, 0);
               }
            }
            break;
         }
      }
   }
   else {
      /* initialize */
      info->query[info->head] = pipe->create_query(pipe, info->query_type, 0);
   }
}

static void
begin_query(struct hud_graph *gr, struct pipe_context *pipe)
{
   struct query_info *info = gr->query_data;

   assert(!info->batch);
   if (info->query[info->head])
      pipe->begin_query(pipe, info->query[info->head]);
}

static void
query_new_value(struct hud_graph *gr, struct pipe_context *pipe)
{
   struct query_info *info = gr->query_data;
   uint64_t now = os_time_get();

   if (info->batch) {
      query_new_value_batch(info);
   } else {
      query_new_value_normal(info, pipe);
   }

   if (!info->last_time) {
      info->last_time = now;
      return;
   }

   if (info->num_results && info->last_time + gr->pane->period <= now) {
      double value;

      switch (info->result_type) {
      default:
      case PIPE_DRIVER_QUERY_RESULT_TYPE_AVERAGE:
         value = info->results_cumulative / info->num_results;
         break;
      case PIPE_DRIVER_QUERY_RESULT_TYPE_CUMULATIVE:
         value = info->results_cumulative;
         break;
      }

      if (info->type == PIPE_DRIVER_QUERY_TYPE_FLOAT) {
         value /= 1000.0;
      }

      hud_graph_add_value(gr, value);

      info->last_time = now;
      info->results_cumulative = 0;
      info->num_results = 0;
   }
}

static void
free_query_info(void *ptr, struct pipe_context *pipe)
{
   struct query_info *info = ptr;

   if (!info->batch && info->last_time) {
      int i;

      pipe->end_query(pipe, info->query[info->head]);

      for (i = 0; i < ARRAY_SIZE(info->query); i++) {
         if (info->query[i]) {
            pipe->destroy_query(pipe, info->query[i]);
         }
      }
   }
   FREE(info);
}


/**
 * \param result_index  to select fields of pipe_query_data_pipeline_statistics,
 *                      for example.
 */
void
hud_pipe_query_install(struct hud_batch_query_context **pbq,
                       struct hud_pane *pane,
                       const char *name,
                       enum pipe_query_type query_type,
                       unsigned result_index,
                       uint64_t max_value, enum pipe_driver_query_type type,
                       enum pipe_driver_query_result_type result_type,
                       unsigned flags)
{
   struct hud_graph *gr;
   struct query_info *info;

   gr = CALLOC_STRUCT(hud_graph);
   if (!gr)
      return;

   strncpy(gr->name, name, sizeof(gr->name));
   gr->name[sizeof(gr->name) - 1] = '\0';
   gr->query_data = CALLOC_STRUCT(query_info);
   if (!gr->query_data)
      goto fail_gr;

   gr->query_new_value = query_new_value;
   gr->free_query_data = free_query_info;

   info = gr->query_data;
   info->result_type = result_type;
   info->type = type;

   if (flags & PIPE_DRIVER_QUERY_FLAG_BATCH) {
      if (!batch_query_add(pbq, query_type, &info->result_index))
         goto fail_info;
      info->batch = *pbq;
   } else {
      gr->begin_query = begin_query;
      info->query_type = query_type;
      info->result_index = result_index;
   }

   hud_pane_add_graph(pane, gr);
   pane->type = type; /* must be set before updating the max_value */

   if (pane->max_value < max_value)
      hud_pane_set_max_value(pane, max_value);
   return;

fail_info:
   FREE(info);
fail_gr:
   FREE(gr);
}

bool
hud_driver_query_install(struct hud_batch_query_context **pbq,
                         struct hud_pane *pane, struct pipe_screen *screen,
                         const char *name)
{
   struct pipe_driver_query_info query = { 0 };
   unsigned num_queries, i;
   bool found = false;

   if (!screen->get_driver_query_info)
      return false;

   num_queries = screen->get_driver_query_info(screen, 0, NULL);

   for (i = 0; i < num_queries; i++) {
      if (screen->get_driver_query_info(screen, i, &query) &&
          strcmp(query.name, name) == 0) {
         found = true;
         break;
      }
   }

   if (!found)
      return false;

   hud_pipe_query_install(pbq, pane, query.name, query.query_type, 0,
                          query.max_value.u64, query.type, query.result_type,
                          query.flags);

   return true;
}
