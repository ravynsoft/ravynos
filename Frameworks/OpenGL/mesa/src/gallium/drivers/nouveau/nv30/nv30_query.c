/*
 * Copyright 2012 Red Hat Inc.
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
 * Authors: Ben Skeggs
 *
 */

#include "nv_object.xml.h"
#include "nv30/nv30-40_3d.xml.h"
#include "nv30/nv30_screen.h"
#include "nv30/nv30_context.h"
#include "nv30/nv30_winsys.h"

struct nv30_query_object {
   struct list_head list;
   struct nouveau_heap *hw;
};

static volatile void *
nv30_ntfy(struct nv30_screen *screen, struct nv30_query_object *qo)
{
   struct nv04_notify *query = screen->query->data;
   struct nouveau_bo *notify = screen->notify;
   volatile void *ntfy = NULL;

   if (qo && qo->hw)
      ntfy = (char *)notify->map + query->offset + qo->hw->start;

   return ntfy;
}

static void
nv30_query_object_del(struct nv30_screen *screen, struct nv30_query_object **po)
{
   struct nv30_query_object *qo = *po; *po = NULL;
   if (qo) {
      volatile uint32_t *ntfy = nv30_ntfy(screen, qo);
      while (ntfy[3] & 0xff000000) {
      }
      nouveau_heap_free(&qo->hw);
      list_del(&qo->list);
      FREE(qo);
   }
}

static struct nv30_query_object *
nv30_query_object_new(struct nv30_screen *screen)
{
   struct nv30_query_object *oq, *qo = CALLOC_STRUCT(nv30_query_object);
   volatile uint32_t *ntfy;

   if (!qo)
      return NULL;

   /* allocate a new hw query object, if no hw objects left we need to
    * spin waiting for one to become free
    */
   while (nouveau_heap_alloc(screen->query_heap, 32, NULL, &qo->hw)) {
      oq = list_first_entry(&screen->queries, struct nv30_query_object, list);
      nv30_query_object_del(screen, &oq);
   }

   list_addtail(&qo->list, &screen->queries);

   ntfy = nv30_ntfy(screen, qo);
   ntfy[0] = 0x00000000;
   ntfy[1] = 0x00000000;
   ntfy[2] = 0x00000000;
   ntfy[3] = 0x01000000;
   return qo;
}

struct nv30_query {
   struct nv30_query_object *qo[2];
   unsigned type;
   uint32_t report;
   uint32_t enable;
   uint64_t result;
};

static inline struct nv30_query *
nv30_query(struct pipe_query *pipe)
{
   return (struct nv30_query *)pipe;
}

static struct pipe_query *
nv30_query_create(struct pipe_context *pipe, unsigned type, unsigned index)
{
   struct nv30_query *q = CALLOC_STRUCT(nv30_query);
   if (!q)
      return NULL;

   q->type = type;

   switch (q->type) {
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIME_ELAPSED:
      q->enable = 0x0000;
      q->report = 1;
      break;
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      q->enable = NV30_3D_QUERY_ENABLE;
      q->report = 1;
      break;
   case NV30_QUERY_ZCULL_0:
   case NV30_QUERY_ZCULL_1:
   case NV30_QUERY_ZCULL_2:
   case NV30_QUERY_ZCULL_3:
      q->enable = 0x1804;
      q->report = 2 + (q->type - NV30_QUERY_ZCULL_0);
      break;
   default:
      FREE(q);
      return NULL;
   }

   return (struct pipe_query *)q;
}

static void
nv30_query_destroy(struct pipe_context *pipe, struct pipe_query *pq)
{
   FREE(pq);
}

static bool
nv30_query_begin(struct pipe_context *pipe, struct pipe_query *pq)
{
   struct nv30_context *nv30 = nv30_context(pipe);
   struct nv30_query *q = nv30_query(pq);
   struct nouveau_pushbuf *push = nv30->base.pushbuf;

   switch (q->type) {
   case PIPE_QUERY_TIME_ELAPSED:
      q->qo[0] = nv30_query_object_new(nv30->screen);
      if (q->qo[0]) {
         BEGIN_NV04(push, NV30_3D(QUERY_GET), 1);
         PUSH_DATA (push, (q->report << 24) | q->qo[0]->hw->start);
      }
      break;
   case PIPE_QUERY_TIMESTAMP:
      return true;
   default:
      BEGIN_NV04(push, NV30_3D(QUERY_RESET), 1);
      PUSH_DATA (push, q->report);
      break;
   }

   if (q->enable) {
      BEGIN_NV04(push, SUBC_3D(q->enable), 1);
      PUSH_DATA (push, 1);
   }
   return true;
}

static bool
nv30_query_end(struct pipe_context *pipe, struct pipe_query *pq)
{
   struct nv30_context *nv30 = nv30_context(pipe);
   struct nv30_screen *screen = nv30->screen;
   struct nv30_query *q = nv30_query(pq);
   struct nouveau_pushbuf *push = nv30->base.pushbuf;

   q->qo[1] = nv30_query_object_new(screen);
   if (q->qo[1]) {
      BEGIN_NV04(push, NV30_3D(QUERY_GET), 1);
      PUSH_DATA (push, (q->report << 24) | q->qo[1]->hw->start);
   }

   if (q->enable) {
      BEGIN_NV04(push, SUBC_3D(q->enable), 1);
      PUSH_DATA (push, 0);
   }
   PUSH_KICK (push);
   return true;
}

static bool
nv30_query_result(struct pipe_context *pipe, struct pipe_query *pq,
                  bool wait, union pipe_query_result *result)
{
   struct nv30_screen *screen = nv30_screen(pipe->screen);
   struct nv30_query *q = nv30_query(pq);
   volatile uint32_t *ntfy0 = nv30_ntfy(screen, q->qo[0]);
   volatile uint32_t *ntfy1 = nv30_ntfy(screen, q->qo[1]);

   if (ntfy1) {
      while (ntfy1[3] & 0xff000000) {
         if (!wait)
            return false;
      }

      switch (q->type) {
      case PIPE_QUERY_TIMESTAMP:
         q->result = *(uint64_t *)&ntfy1[0];
         break;
      case PIPE_QUERY_TIME_ELAPSED:
         q->result = *(uint64_t *)&ntfy1[0] - *(uint64_t *)&ntfy0[0];
         break;
      default:
         q->result = ntfy1[2];
         break;
      }

      nv30_query_object_del(screen, &q->qo[0]);
      nv30_query_object_del(screen, &q->qo[1]);
   }

   if (q->type == PIPE_QUERY_OCCLUSION_PREDICATE ||
       q->type == PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE)
      result->b = !!q->result;
   else
      result->u64 = q->result;
   return true;
}

static void
nv40_query_render_condition(struct pipe_context *pipe,
                            struct pipe_query *pq,
                            bool condition, enum pipe_render_cond_flag mode)
{
   struct nv30_context *nv30 = nv30_context(pipe);
   struct nv30_query *q = nv30_query(pq);
   struct nouveau_pushbuf *push = nv30->base.pushbuf;

   nv30->render_cond_query = pq;
   nv30->render_cond_mode = mode;
   nv30->render_cond_cond = condition;

   if (!pq) {
      BEGIN_NV04(push, SUBC_3D(0x1e98), 1);
      PUSH_DATA (push, 0x01000000);
      return;
   }

   if (mode == PIPE_RENDER_COND_WAIT ||
       mode == PIPE_RENDER_COND_BY_REGION_WAIT) {
      BEGIN_NV04(push, SUBC_3D(0x0110), 1);
      PUSH_DATA (push, 0);
   }

   BEGIN_NV04(push, SUBC_3D(0x1e98), 1);
   PUSH_DATA (push, 0x02000000 | q->qo[1]->hw->start);
}

static void
nv30_set_active_query_state(struct pipe_context *pipe, bool enable)
{
}

void
nv30_query_init(struct pipe_context *pipe)
{
   struct nouveau_object *eng3d = nv30_context(pipe)->screen->eng3d;

   pipe->create_query = nv30_query_create;
   pipe->destroy_query = nv30_query_destroy;
   pipe->begin_query = nv30_query_begin;
   pipe->end_query = nv30_query_end;
   pipe->get_query_result = nv30_query_result;
   pipe->set_active_query_state = nv30_set_active_query_state;
   if (eng3d->oclass >= NV40_3D_CLASS)
      pipe->render_condition = nv40_query_render_condition;
}
