/*
 * Copyright 2011 Nouveau Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Christoph Bumiller
 */

#define NVC0_PUSH_EXPLICIT_SPACE_CHECKING

#include "nvc0/nvc0_context.h"
#include "nvc0/nvc0_query.h"
#include "nvc0/nvc0_query_sw.h"
#include "nvc0/nvc0_query_hw.h"
#include "nvc0/nvc0_query_hw_metric.h"
#include "nvc0/nvc0_query_hw_sm.h"

static struct pipe_query *
nvc0_create_query(struct pipe_context *pipe, unsigned type, unsigned index)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);
   struct nvc0_query *q;

   q = nvc0_sw_create_query(nvc0, type, index);
   if (!q)
      q = nvc0_hw_create_query(nvc0, type, index);

   return (struct pipe_query *)q;
}

static void
nvc0_destroy_query(struct pipe_context *pipe, struct pipe_query *pq)
{
   struct nvc0_query *q = nvc0_query(pq);
   q->funcs->destroy_query(nvc0_context(pipe), q);
}

static bool
nvc0_begin_query(struct pipe_context *pipe, struct pipe_query *pq)
{
   struct nvc0_query *q = nvc0_query(pq);
   return q->funcs->begin_query(nvc0_context(pipe), q);
}

static bool
nvc0_end_query(struct pipe_context *pipe, struct pipe_query *pq)
{
   struct nvc0_query *q = nvc0_query(pq);
   q->funcs->end_query(nvc0_context(pipe), q);
   return true;
}

static bool
nvc0_get_query_result(struct pipe_context *pipe, struct pipe_query *pq,
                      bool wait, union pipe_query_result *result)
{
   struct nvc0_query *q = nvc0_query(pq);
   return q->funcs->get_query_result(nvc0_context(pipe), q, wait, result);
}

static void
nvc0_get_query_result_resource(struct pipe_context *pipe,
                               struct pipe_query *pq,
                               enum pipe_query_flags flags,
                               enum pipe_query_value_type result_type,
                               int index,
                               struct pipe_resource *resource,
                               unsigned offset)
{
   struct nvc0_query *q = nvc0_query(pq);
   if (!q->funcs->get_query_result_resource) {
      assert(!"Unexpected lack of get_query_result_resource");
      return;
   }
   q->funcs->get_query_result_resource(nvc0_context(pipe), q, flags, result_type,
                                       index, resource, offset);
}

static void
nvc0_render_condition(struct pipe_context *pipe,
                      struct pipe_query *pq,
                      bool condition, enum pipe_render_cond_flag mode)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_query *q = nvc0_query(pq);
   struct nvc0_hw_query *hq = nvc0_hw_query(q);
   uint32_t cond;
   bool wait =
      mode != PIPE_RENDER_COND_NO_WAIT &&
      mode != PIPE_RENDER_COND_BY_REGION_NO_WAIT;

   if (!pq) {
      cond = NVC0_3D_COND_MODE_ALWAYS;
   }
   else {
      /* NOTE: comparison of 2 queries only works if both have completed */
      switch (q->type) {
      case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
         cond = condition ? NVC0_3D_COND_MODE_EQUAL :
                            NVC0_3D_COND_MODE_NOT_EQUAL;
         wait = true;
         break;
      case PIPE_QUERY_OCCLUSION_COUNTER:
      case PIPE_QUERY_OCCLUSION_PREDICATE:
      case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
         if (hq->state == NVC0_HW_QUERY_STATE_READY)
            wait = true;
         if (likely(!condition)) {
            cond = wait ? NVC0_3D_COND_MODE_NOT_EQUAL : NVC0_3D_COND_MODE_ALWAYS;
         } else {
            cond = wait ? NVC0_3D_COND_MODE_EQUAL : NVC0_3D_COND_MODE_ALWAYS;
         }
         break;
      default:
         assert(!"render condition query not a predicate");
         cond = NVC0_3D_COND_MODE_ALWAYS;
         break;
      }
   }

   nvc0->cond_query = pq;
   nvc0->cond_cond = condition;
   nvc0->cond_condmode = cond;
   nvc0->cond_mode = mode;

   if (!pq) {
      PUSH_SPACE(push, 2);
      IMMED_NVC0(push, NVC0_3D(COND_MODE), cond);
      if (nvc0->screen->compute)
         IMMED_NVC0(push, NVC0_CP(COND_MODE), cond);
      return;
   }

   if (wait && hq->state != NVC0_HW_QUERY_STATE_READY)
      nvc0_hw_query_fifo_wait(nvc0, q);

   PUSH_SPACE(push, 10);
   PUSH_REF1 (push, hq->bo, NOUVEAU_BO_GART | NOUVEAU_BO_RD);
   BEGIN_NVC0(push, NVC0_3D(COND_ADDRESS_HIGH), 3);
   PUSH_DATAh(push, hq->bo->offset + hq->offset);
   PUSH_DATA (push, hq->bo->offset + hq->offset);
   PUSH_DATA (push, cond);
   BEGIN_NVC0(push, NVC0_2D(COND_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, hq->bo->offset + hq->offset);
   PUSH_DATA (push, hq->bo->offset + hq->offset);
   if (nvc0->screen->compute) {
      BEGIN_NVC0(push, NVC0_CP(COND_ADDRESS_HIGH), 3);
      PUSH_DATAh(push, hq->bo->offset + hq->offset);
      PUSH_DATA (push, hq->bo->offset + hq->offset);
      PUSH_DATA (push, cond);
   }
}

int
nvc0_screen_get_driver_query_info(struct pipe_screen *pscreen,
                                  unsigned id,
                                  struct pipe_driver_query_info *info)
{
   struct nvc0_screen *screen = nvc0_screen(pscreen);
   int num_sw_queries = 0, num_hw_queries = 0;

   num_sw_queries = nvc0_sw_get_driver_query_info(screen, 0, NULL);
   num_hw_queries = nvc0_hw_get_driver_query_info(screen, 0, NULL);

   if (!info)
      return num_sw_queries + num_hw_queries;

   /* Init default values. */
   info->name = "this_is_not_the_query_you_are_looking_for";
   info->query_type = 0xdeadd01d;
   info->max_value.u64 = 0;
   info->type = PIPE_DRIVER_QUERY_TYPE_UINT64;
   info->group_id = -1;
   info->flags = 0;

#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS
   if (id < num_sw_queries)
      return nvc0_sw_get_driver_query_info(screen, id, info);
#endif

   return nvc0_hw_get_driver_query_info(screen, id - num_sw_queries, info);
}

int
nvc0_screen_get_driver_query_group_info(struct pipe_screen *pscreen,
                                        unsigned id,
                                        struct pipe_driver_query_group_info *info)
{
   struct nvc0_screen *screen = nvc0_screen(pscreen);
   int count = 0;
   int map[3] = {};

   if (screen->base.drm->version >= 0x01000101) {
      if (screen->compute) {
         if (screen->base.class_3d <= GM200_3D_CLASS) {
            map[count++] = NVC0_HW_SM_QUERY_GROUP;
            map[count++] = NVC0_HW_METRIC_QUERY_GROUP;
         }
      }
   }

#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS
   map[count++] = NVC0_SW_QUERY_DRV_STAT_GROUP;
#endif

   if (!info)
      return count;

   switch (map[id]) {
   case NVC0_HW_SM_QUERY_GROUP:
      if (screen->compute && screen->base.class_3d <= GM200_3D_CLASS) {
         info->name = "MP counters";

         /* Expose the maximum number of hardware counters available, although
          * some queries use more than one counter. Expect failures in that
          * case but as performance counters are for developers, this should
          * not have a real impact. */
         info->max_active_queries = 8;
         info->num_queries = nvc0_hw_sm_get_num_queries(screen);
         return 1;
      }
      break;
   case NVC0_HW_METRIC_QUERY_GROUP:
      if (screen->compute && screen->base.class_3d <= GM200_3D_CLASS) {
         info->name = "Performance metrics";
         info->max_active_queries = 4; /* A metric uses at least 2 queries */
         info->num_queries = nvc0_hw_metric_get_num_queries(screen);
         return 1;
      }
      break;
#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS
   case NVC0_SW_QUERY_DRV_STAT_GROUP:
      info->name = "Driver statistics";
      info->max_active_queries = NVC0_SW_QUERY_DRV_STAT_COUNT;
      info->num_queries = NVC0_SW_QUERY_DRV_STAT_COUNT;
      return 1;
#endif
   }

   /* user asked for info about non-existing query group */
   info->name = "this_is_not_the_query_group_you_are_looking_for";
   info->max_active_queries = 0;
   info->num_queries = 0;
   return 0;
}

static void
nvc0_set_active_query_state(struct pipe_context *pipe, bool enable)
{
}

void
nvc0_init_query_functions(struct nvc0_context *nvc0)
{
   struct pipe_context *pipe = &nvc0->base.pipe;

   pipe->create_query = nvc0_create_query;
   pipe->destroy_query = nvc0_destroy_query;
   pipe->begin_query = nvc0_begin_query;
   pipe->end_query = nvc0_end_query;
   pipe->get_query_result = nvc0_get_query_result;
   pipe->get_query_result_resource = nvc0_get_query_result_resource;
   pipe->set_active_query_state = nvc0_set_active_query_state;
   pipe->render_condition = nvc0_render_condition;
   nvc0->cond_condmode = NVC0_3D_COND_MODE_ALWAYS;
}
