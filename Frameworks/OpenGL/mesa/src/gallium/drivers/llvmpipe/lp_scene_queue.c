/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
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


/**
 * Scene queue.  We'll use two queues.  One contains "full" scenes which
 * are produced by the "setup" code.  The other contains "empty" scenes
 * which are produced by the "rast" code when it finishes rendering a scene.
 */

#include "util/u_thread.h"
#include "util/u_memory.h"
#include "lp_scene_queue.h"
#include "util/u_math.h"
#include "lp_setup_context.h"


#define SCENE_QUEUE_SIZE MAX_SCENES



/**
 * A queue of scenes
 */
struct lp_scene_queue
{
   struct lp_scene *scenes[SCENE_QUEUE_SIZE];

   mtx_t mutex;
   cnd_t change;

   /* These values wrap around, so that head == tail means empty.  When used
    * to index the array, we use them modulo the queue size.  This scheme
    * works because the queue size is a power of two.
    */
   unsigned head;
   unsigned tail;
};



/** Allocate a new scene queue */
struct lp_scene_queue *
lp_scene_queue_create(void)
{
   /* Circular queue behavior depends on size being a power of two. */
   STATIC_ASSERT(SCENE_QUEUE_SIZE > 0);
   STATIC_ASSERT((SCENE_QUEUE_SIZE & (SCENE_QUEUE_SIZE - 1)) == 0);

   struct lp_scene_queue *queue = CALLOC_STRUCT(lp_scene_queue);

   if (!queue)
      return NULL;

   (void) mtx_init(&queue->mutex, mtx_plain);
   cnd_init(&queue->change);

   return queue;
}


/** Delete a scene queue */
void
lp_scene_queue_destroy(struct lp_scene_queue *queue)
{
   cnd_destroy(&queue->change);
   mtx_destroy(&queue->mutex);
   FREE(queue);
}


/** Remove first lp_scene from head of queue */
struct lp_scene *
lp_scene_dequeue(struct lp_scene_queue *queue, bool wait)
{
   mtx_lock(&queue->mutex);

   if (wait) {
      /* Wait for queue to be not empty. */
      while (queue->head == queue->tail)
         cnd_wait(&queue->change, &queue->mutex);
   } else {
      if (queue->head == queue->tail) {
         mtx_unlock(&queue->mutex);
         return NULL;
      }
   }

   struct lp_scene *scene = queue->scenes[queue->head++ % SCENE_QUEUE_SIZE];

   cnd_signal(&queue->change);
   mtx_unlock(&queue->mutex);

   return scene;
}


/** Add an lp_scene to tail of queue */
void
lp_scene_enqueue(struct lp_scene_queue *queue, struct lp_scene *scene)
{
   mtx_lock(&queue->mutex);

   /* Wait for free space. */
   while (queue->tail - queue->head >= SCENE_QUEUE_SIZE)
      cnd_wait(&queue->change, &queue->mutex);

   queue->scenes[queue->tail++ % SCENE_QUEUE_SIZE] = scene;

   cnd_signal(&queue->change);
   mtx_unlock(&queue->mutex);
}
