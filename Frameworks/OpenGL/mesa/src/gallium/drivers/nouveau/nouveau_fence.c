/*
 * Copyright 2010 Christoph Bumiller
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
 */

#include "nouveau_screen.h"
#include "nouveau_context.h"
#include "nouveau_winsys.h"
#include "nouveau_fence.h"
#include "util/os_time.h"

#if DETECT_OS_UNIX
#include <sched.h>
#endif

static bool
_nouveau_fence_wait(struct nouveau_fence *fence, struct util_debug_callback *debug);

bool
nouveau_fence_new(struct nouveau_context *nv, struct nouveau_fence **fence)
{
   *fence = CALLOC_STRUCT(nouveau_fence);
   if (!*fence)
      return false;

   int ret = nouveau_bo_new(nv->screen->device, NOUVEAU_BO_GART, 0x1000, 0x1000, NULL, &(*fence)->bo);
   if (ret) {
      FREE(*fence);
      return false;
   }

   (*fence)->screen = nv->screen;
   (*fence)->context = nv;
   (*fence)->ref = 1;
   list_inithead(&(*fence)->work);

   return true;
}

static void
nouveau_fence_trigger_work(struct nouveau_fence *fence)
{
   simple_mtx_assert_locked(&fence->screen->fence.lock);

   struct nouveau_fence_work *work, *tmp;

   LIST_FOR_EACH_ENTRY_SAFE(work, tmp, &fence->work, list) {
      work->func(work->data);
      list_del(&work->list);
      FREE(work);
   }
}

static void
_nouveau_fence_emit(struct nouveau_fence *fence)
{
   struct nouveau_screen *screen = fence->screen;
   struct nouveau_fence_list *fence_list = &screen->fence;

   simple_mtx_assert_locked(&fence_list->lock);

   assert(fence->state != NOUVEAU_FENCE_STATE_EMITTING);
   if (fence->state >= NOUVEAU_FENCE_STATE_EMITTED)
      return;

   /* set this now, so that if fence.emit triggers a flush we don't recurse */
   fence->state = NOUVEAU_FENCE_STATE_EMITTING;

   p_atomic_inc(&fence->ref);

   if (fence_list->tail)
      fence_list->tail->next = fence;
   else
      fence_list->head = fence;

   fence_list->tail = fence;

   fence_list->emit(&fence->context->pipe, &fence->sequence, fence->bo);

   assert(fence->state == NOUVEAU_FENCE_STATE_EMITTING);
   fence->state = NOUVEAU_FENCE_STATE_EMITTED;
}

static void
nouveau_fence_del(struct nouveau_fence *fence)
{
   struct nouveau_fence *it;
   struct nouveau_fence_list *fence_list = &fence->screen->fence;

   simple_mtx_assert_locked(&fence_list->lock);

   if (fence->state == NOUVEAU_FENCE_STATE_EMITTED ||
       fence->state == NOUVEAU_FENCE_STATE_FLUSHED) {
      if (fence == fence_list->head) {
         fence_list->head = fence->next;
         if (!fence_list->head)
            fence_list->tail = NULL;
      } else {
         for (it = fence_list->head; it && it->next != fence; it = it->next);
         it->next = fence->next;
         if (fence_list->tail == fence)
            fence_list->tail = it;
      }
   }

   if (!list_is_empty(&fence->work)) {
      debug_printf("WARNING: deleting fence with work still pending !\n");
      nouveau_fence_trigger_work(fence);
   }

   nouveau_bo_ref(NULL, &fence->bo);
   FREE(fence);
}

void
nouveau_fence_cleanup(struct nouveau_context *nv)
{
   if (nv->fence) {
      struct nouveau_fence_list *fence_list = &nv->screen->fence;
      struct nouveau_fence *current = NULL;

      /* nouveau_fence_wait will create a new current fence, so wait on the
       * _current_ one, and remove both.
       */
      simple_mtx_lock(&fence_list->lock);
      _nouveau_fence_ref(nv->fence, &current);
      _nouveau_fence_wait(current, NULL);
      _nouveau_fence_ref(NULL, &current);
      _nouveau_fence_ref(NULL, &nv->fence);
      simple_mtx_unlock(&fence_list->lock);
   }
}

void
_nouveau_fence_update(struct nouveau_screen *screen, bool flushed)
{
   struct nouveau_fence *fence;
   struct nouveau_fence *next = NULL;
   struct nouveau_fence_list *fence_list = &screen->fence;
   u32 sequence = fence_list->update(&screen->base);

   simple_mtx_assert_locked(&fence_list->lock);

   /* If running under drm-shim, let all fences be signalled so things run to
    * completion (avoids a hang at the end of shader-db).
    */
   if (unlikely(screen->disable_fences))
      sequence = screen->fence.sequence;

   if (fence_list->sequence_ack == sequence)
      return;
   fence_list->sequence_ack = sequence;

   for (fence = fence_list->head; fence; fence = next) {
      next = fence->next;
      sequence = fence->sequence;

      fence->state = NOUVEAU_FENCE_STATE_SIGNALLED;

      nouveau_fence_trigger_work(fence);
      _nouveau_fence_ref(NULL, &fence);

      if (sequence == fence_list->sequence_ack)
         break;
   }
   fence_list->head = next;
   if (!next)
      fence_list->tail = NULL;

   if (flushed) {
      for (fence = next; fence; fence = fence->next)
         if (fence->state == NOUVEAU_FENCE_STATE_EMITTED)
            fence->state = NOUVEAU_FENCE_STATE_FLUSHED;
   }
}

#define NOUVEAU_FENCE_MAX_SPINS (1 << 31)

static bool
_nouveau_fence_signalled(struct nouveau_fence *fence)
{
   struct nouveau_screen *screen = fence->screen;

   simple_mtx_assert_locked(&screen->fence.lock);

   if (fence->state == NOUVEAU_FENCE_STATE_SIGNALLED)
      return true;

   if (fence->state >= NOUVEAU_FENCE_STATE_EMITTED)
      _nouveau_fence_update(screen, false);

   return fence->state == NOUVEAU_FENCE_STATE_SIGNALLED;
}

static bool
nouveau_fence_kick(struct nouveau_fence *fence)
{
   struct nouveau_context *context = fence->context;
   struct nouveau_screen *screen = fence->screen;
   struct nouveau_fence_list *fence_list = &screen->fence;
   bool current = !fence->sequence;

   simple_mtx_assert_locked(&fence_list->lock);

   /* wtf, someone is waiting on a fence in flush_notify handler? */
   assert(fence->state != NOUVEAU_FENCE_STATE_EMITTING);

   if (fence->state < NOUVEAU_FENCE_STATE_EMITTED) {
      if (PUSH_AVAIL(context->pushbuf) < 16)
         nouveau_pushbuf_space(context->pushbuf, 16, 0, 0);
      _nouveau_fence_emit(fence);
   }

   if (fence->state < NOUVEAU_FENCE_STATE_FLUSHED) {
      if (nouveau_pushbuf_kick(context->pushbuf, context->pushbuf->channel))
         return false;
   }

   if (current)
      _nouveau_fence_next(fence->context);

   _nouveau_fence_update(screen, false);

   return true;
}

static bool
_nouveau_fence_wait(struct nouveau_fence *fence, struct util_debug_callback *debug)
{
   struct nouveau_screen *screen = fence->screen;
   struct nouveau_fence_list *fence_list = &screen->fence;
   int64_t start = 0;

   simple_mtx_assert_locked(&fence_list->lock);

   if (debug && debug->debug_message)
      start = os_time_get_nano();

   if (!nouveau_fence_kick(fence))
      return false;

   if (fence->state < NOUVEAU_FENCE_STATE_SIGNALLED) {
      NOUVEAU_DRV_STAT(screen, any_non_kernel_fence_sync_count, 1);
      int ret = nouveau_bo_wait(fence->bo, NOUVEAU_BO_RDWR, screen->client);
      if (ret) {
         debug_printf("Wait on fence %u (ack = %u, next = %u) errored with %s !\n",
                      fence->sequence,
                      fence_list->sequence_ack, fence_list->sequence, strerror(ret));
         return false;
      }

      _nouveau_fence_update(screen, false);
      if (fence->state != NOUVEAU_FENCE_STATE_SIGNALLED)
         return false;

      if (debug && debug->debug_message)
         util_debug_message(debug, PERF_INFO,
                            "stalled %.3f ms waiting for fence",
                            (os_time_get_nano() - start) / 1000000.f);
   }

   return true;
}

void
_nouveau_fence_next(struct nouveau_context *nv)
{
   struct nouveau_fence_list *fence_list = &nv->screen->fence;

   simple_mtx_assert_locked(&fence_list->lock);

   if (nv->fence->state < NOUVEAU_FENCE_STATE_EMITTING) {
      if (p_atomic_read(&nv->fence->ref) > 1)
         _nouveau_fence_emit(nv->fence);
      else
         return;
   }

   _nouveau_fence_ref(NULL, &nv->fence);

   nouveau_fence_new(nv, &nv->fence);
}

void
nouveau_fence_unref_bo(void *data)
{
   struct nouveau_bo *bo = data;

   nouveau_bo_ref(NULL, &bo);
}

bool
nouveau_fence_work(struct nouveau_fence *fence,
                   void (*func)(void *), void *data)
{
   struct nouveau_fence_work *work;
   struct nouveau_screen *screen;

   if (!fence || fence->state == NOUVEAU_FENCE_STATE_SIGNALLED) {
      func(data);
      return true;
   }

   work = CALLOC_STRUCT(nouveau_fence_work);
   if (!work)
      return false;
   work->func = func;
   work->data = data;

   /* the fence might get deleted by fence_kick */
   screen = fence->screen;

   simple_mtx_lock(&screen->fence.lock);
   list_add(&work->list, &fence->work);
   if (++fence->work_count > 64)
      nouveau_fence_kick(fence);
   simple_mtx_unlock(&screen->fence.lock);
   return true;
}

void
_nouveau_fence_ref(struct nouveau_fence *fence, struct nouveau_fence **ref)
{
   if (fence)
      p_atomic_inc(&fence->ref);

   if (*ref) {
      simple_mtx_assert_locked(&(*ref)->screen->fence.lock);
      if (p_atomic_dec_zero(&(*ref)->ref))
         nouveau_fence_del(*ref);
   }

   *ref = fence;
}

void
nouveau_fence_ref(struct nouveau_fence *fence, struct nouveau_fence **ref)
{
   struct nouveau_fence_list *fence_list = NULL;
   if (ref && *ref)
      fence_list = &(*ref)->screen->fence;

   if (fence_list)
      simple_mtx_lock(&fence_list->lock);

   _nouveau_fence_ref(fence, ref);

   if (fence_list)
      simple_mtx_unlock(&fence_list->lock);
}

bool
nouveau_fence_wait(struct nouveau_fence *fence, struct util_debug_callback *debug)
{
   struct nouveau_fence_list *fence_list = &fence->screen->fence;
   simple_mtx_lock(&fence_list->lock);
   bool res = _nouveau_fence_wait(fence, debug);
   simple_mtx_unlock(&fence_list->lock);
   return res;
}

void
nouveau_fence_next_if_current(struct nouveau_context *nv, struct nouveau_fence *fence)
{
   simple_mtx_lock(&fence->screen->fence.lock);
   if (nv->fence == fence)
      _nouveau_fence_next(nv);
   simple_mtx_unlock(&fence->screen->fence.lock);
}

bool
nouveau_fence_signalled(struct nouveau_fence *fence)
{
   simple_mtx_lock(&fence->screen->fence.lock);
   bool ret = _nouveau_fence_signalled(fence);
   simple_mtx_unlock(&fence->screen->fence.lock);
   return ret;
}
