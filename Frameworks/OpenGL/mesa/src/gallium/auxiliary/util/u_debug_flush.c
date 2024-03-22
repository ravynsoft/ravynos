/**************************************************************************
 *
 * Copyright 2012 VMware, Inc.
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
 * @file
 * u_debug_flush.c Debug flush and map-related issues:
 * - Flush while synchronously mapped.
 * - Command stream reference while synchronously mapped.
 * - Synchronous map while referenced on command stream.
 * - Recursive maps.
 * - Unmap while not mapped.
 *
 * @author Thomas Hellstrom <thellstrom@vmware.com>
 */

#ifdef DEBUG
#include "util/compiler.h"
#include "util/simple_mtx.h"
#include "util/u_debug_stack.h"
#include "util/u_debug.h"
#include "util/u_memory.h"
#include "util/u_debug_flush.h"
#include "util/u_hash_table.h"
#include "util/list.h"
#include "util/u_inlines.h"
#include "util/u_string.h"
#include "util/u_thread.h"
#include <stdio.h>

/* Future improvement: Use realloc instead? */
#define DEBUG_FLUSH_MAP_DEPTH 64

struct debug_map_item {
   struct debug_stack_frame *frame;
   bool persistent;
};

struct debug_flush_buf {
   /* Atomic */
   struct pipe_reference reference; /* Must be the first member. */
   mtx_t mutex;
   /* Immutable */
   bool supports_persistent;
   unsigned bt_depth;
   /* Protected by mutex */
   int map_count;
   bool has_sync_map;
   int last_sync_map;
   struct debug_map_item maps[DEBUG_FLUSH_MAP_DEPTH];
};

struct debug_flush_item {
   struct debug_flush_buf *fbuf;
   unsigned bt_depth;
   struct debug_stack_frame *ref_frame;
};

struct debug_flush_ctx {
   /* Contexts are used by a single thread at a time */
   unsigned bt_depth;
   bool catch_map_of_referenced;
   struct hash_table *ref_hash;
   struct list_head head;
};

static simple_mtx_t list_mutex = SIMPLE_MTX_INITIALIZER;
static struct list_head ctx_list = {&ctx_list, &ctx_list};

static struct debug_stack_frame *
debug_flush_capture_frame(int start, int depth)
{
   struct debug_stack_frame *frames;

   frames = CALLOC(depth, sizeof(*frames));
   if (!frames)
      return NULL;

   debug_backtrace_capture(frames, start, depth);
   return frames;
}

struct debug_flush_buf *
debug_flush_buf_create(bool supports_persistent, unsigned bt_depth)
{
   struct debug_flush_buf *fbuf = CALLOC_STRUCT(debug_flush_buf);

   if (!fbuf)
      goto out_no_buf;

   fbuf->supports_persistent = supports_persistent;
   fbuf->bt_depth = bt_depth;
   pipe_reference_init(&fbuf->reference, 1);
   (void) mtx_init(&fbuf->mutex, mtx_plain);

   return fbuf;
out_no_buf:
   debug_printf("Debug flush buffer creation failed.\n");
   debug_printf("Debug flush checking for this buffer will be incomplete.\n");
   return NULL;
}

void
debug_flush_buf_reference(struct debug_flush_buf **dst,
                          struct debug_flush_buf *src)
{
   struct debug_flush_buf *fbuf = *dst;

   if (pipe_reference(&(*dst)->reference, &src->reference)) {
      int i;

      for (i = 0; i < fbuf->map_count; ++i) {
         FREE(fbuf->maps[i].frame);
      }
      FREE(fbuf);
   }

   *dst = src;
}

static void
debug_flush_item_destroy(struct debug_flush_item *item)
{
   debug_flush_buf_reference(&item->fbuf, NULL);

   FREE(item->ref_frame);

   FREE(item);
}

struct debug_flush_ctx *
debug_flush_ctx_create(UNUSED bool catch_reference_of_mapped,
                       unsigned bt_depth)
{
   struct debug_flush_ctx *fctx = CALLOC_STRUCT(debug_flush_ctx);

   if (!fctx)
      goto out_no_ctx;

   fctx->ref_hash = util_hash_table_create_ptr_keys();

   if (!fctx->ref_hash)
      goto out_no_ref_hash;

   fctx->bt_depth = bt_depth;
   simple_mtx_lock(&list_mutex);
   list_addtail(&fctx->head, &ctx_list);
   simple_mtx_unlock(&list_mutex);

   return fctx;

 out_no_ref_hash:
   FREE(fctx);
out_no_ctx:
   debug_printf("Debug flush context creation failed.\n");
   debug_printf("Debug flush checking for this context will be incomplete.\n");
   return NULL;
}

static void
debug_flush_alert(const char *s, const char *op,
                  unsigned start, unsigned depth,
                  bool continued,
                  bool capture,
                  const struct debug_stack_frame *frame)
{
   if (capture)
      frame = debug_flush_capture_frame(start, depth);

   if (s)
      debug_printf("%s ", s);
   if (frame) {
      debug_printf("%s backtrace follows:\n", op);
      debug_backtrace_dump(frame, depth);
   } else
      debug_printf("No %s backtrace was captured.\n", op);

   if (continued)
      debug_printf("**********************************\n");
   else
      debug_printf("*********END OF MESSAGE***********\n\n\n");

   if (capture)
      FREE((void *)frame);
}


void
debug_flush_map(struct debug_flush_buf *fbuf, unsigned flags)
{
   bool map_sync, persistent;

   if (!fbuf)
      return;

   mtx_lock(&fbuf->mutex);
   map_sync = !(flags & PIPE_MAP_UNSYNCHRONIZED);
   persistent = !map_sync || fbuf->supports_persistent ||
      !!(flags & PIPE_MAP_PERSISTENT);

   /* Recursive maps are allowed if previous maps are persistent,
    * or if the current map is unsync. In other cases we might flush
    * with unpersistent maps.
    */
   if (fbuf->has_sync_map && !map_sync) {
      debug_flush_alert("Recursive sync map detected.", "Map",
                        2, fbuf->bt_depth, true, true, NULL);
      debug_flush_alert(NULL, "Previous map", 0, fbuf->bt_depth, false,
                        false, fbuf->maps[fbuf->last_sync_map].frame);
   }

   fbuf->maps[fbuf->map_count].frame =
      debug_flush_capture_frame(1, fbuf->bt_depth);
   fbuf->maps[fbuf->map_count].persistent = persistent;
   if (!persistent) {
      fbuf->has_sync_map = true;
      fbuf->last_sync_map = fbuf->map_count;
   }

   fbuf->map_count++;
   assert(fbuf->map_count < DEBUG_FLUSH_MAP_DEPTH);

   mtx_unlock(&fbuf->mutex);

   if (!persistent) {
      struct debug_flush_ctx *fctx;

      simple_mtx_lock(&list_mutex);
      LIST_FOR_EACH_ENTRY(fctx, &ctx_list, head) {
         struct debug_flush_item *item =
            util_hash_table_get(fctx->ref_hash, fbuf);

         if (item && fctx->catch_map_of_referenced) {
            debug_flush_alert("Already referenced map detected.",
                              "Map", 2, fbuf->bt_depth, true, true, NULL);
            debug_flush_alert(NULL, "Reference", 0, item->bt_depth,
                              false, false, item->ref_frame);
         }
      }
      simple_mtx_unlock(&list_mutex);
   }
}

void
debug_flush_unmap(struct debug_flush_buf *fbuf)
{
   if (!fbuf)
      return;

   mtx_lock(&fbuf->mutex);
   if (--fbuf->map_count < 0) {
      debug_flush_alert("Unmap not previously mapped detected.", "Map",
                        2, fbuf->bt_depth, false, true, NULL);
   } else {
      if (fbuf->has_sync_map && fbuf->last_sync_map == fbuf->map_count) {
         int i = fbuf->map_count;

         fbuf->has_sync_map = false;
         while (i-- && !fbuf->has_sync_map) {
            if (!fbuf->maps[i].persistent) {
               fbuf->has_sync_map = true;
               fbuf->last_sync_map = i;
            }
         }
         FREE(fbuf->maps[fbuf->map_count].frame);
         fbuf->maps[fbuf->map_count].frame = NULL;
      }
   }
   mtx_unlock(&fbuf->mutex);
}


/**
 * Add the given buffer to the list of active buffers.  Active buffers
 * are those which are referenced by the command buffer currently being
 * constructed.
 */
void
debug_flush_cb_reference(struct debug_flush_ctx *fctx,
                         struct debug_flush_buf *fbuf)
{
   struct debug_flush_item *item;

   if (!fctx || !fbuf)
      return;

   item = util_hash_table_get(fctx->ref_hash, fbuf);

   mtx_lock(&fbuf->mutex);
   if (fbuf->map_count && fbuf->has_sync_map) {
      debug_flush_alert("Reference of mapped buffer detected.", "Reference",
                        2, fctx->bt_depth, true, true, NULL);
      debug_flush_alert(NULL, "Map", 0, fbuf->bt_depth, false,
                        false, fbuf->maps[fbuf->last_sync_map].frame);
   }
   mtx_unlock(&fbuf->mutex);

   if (!item) {
      item = CALLOC_STRUCT(debug_flush_item);
      if (item) {
         debug_flush_buf_reference(&item->fbuf, fbuf);
         item->bt_depth = fctx->bt_depth;
         item->ref_frame = debug_flush_capture_frame(2, item->bt_depth);
         _mesa_hash_table_insert(fctx->ref_hash, fbuf, item);
         return;
      }
      goto out_no_item;
   }
   return;

out_no_item:
   debug_printf("Debug flush command buffer reference creation failed.\n");
   debug_printf("Debug flush checking will be incomplete "
                "for this command batch.\n");
}

static int
debug_flush_might_flush_cb(UNUSED void *key, void *value, void *data)
{
   struct debug_flush_item *item =
      (struct debug_flush_item *) value;
   struct debug_flush_buf *fbuf = item->fbuf;

   mtx_lock(&fbuf->mutex);
   if (fbuf->map_count && fbuf->has_sync_map) {
      const char *reason = (const char *) data;
      char message[80];

      snprintf(message, sizeof(message),
               "%s referenced mapped buffer detected.", reason);

      debug_flush_alert(message, reason, 3, item->bt_depth, true, true, NULL);
      debug_flush_alert(NULL, "Map", 0, fbuf->bt_depth, true, false,
                        fbuf->maps[fbuf->last_sync_map].frame);
      debug_flush_alert(NULL, "First reference", 0, item->bt_depth, false,
                        false, item->ref_frame);
   }
   mtx_unlock(&fbuf->mutex);

   return 0;
}

/**
 * Called when we're about to possibly flush a command buffer.
 * We check if any active buffers are in a mapped state.  If so, print an alert.
 */
void
debug_flush_might_flush(struct debug_flush_ctx *fctx)
{
   if (!fctx)
      return;

   util_hash_table_foreach(fctx->ref_hash,
                           debug_flush_might_flush_cb,
                           "Might flush");
}

static int
debug_flush_flush_cb(UNUSED void *key, void *value, UNUSED void *data)
{
   struct debug_flush_item *item =
      (struct debug_flush_item *) value;

   debug_flush_item_destroy(item);

   return 0;
}


/**
 * Called when we flush a command buffer.  Two things are done:
 * 1. Check if any of the active buffers are currently mapped (alert if so).
 * 2. Discard/unreference all the active buffers.
 */
void
debug_flush_flush(struct debug_flush_ctx *fctx)
{
   if (!fctx)
      return;

   util_hash_table_foreach(fctx->ref_hash,
                           debug_flush_might_flush_cb,
                           "Flush");
   util_hash_table_foreach(fctx->ref_hash,
                           debug_flush_flush_cb,
                           NULL);
   _mesa_hash_table_clear(fctx->ref_hash, NULL);
}

void
debug_flush_ctx_destroy(struct debug_flush_ctx *fctx)
{
   if (!fctx)
      return;

   list_del(&fctx->head);
   util_hash_table_foreach(fctx->ref_hash,
                           debug_flush_flush_cb,
                           NULL);
   _mesa_hash_table_clear(fctx->ref_hash, NULL);
   _mesa_hash_table_destroy(fctx->ref_hash, NULL);
   FREE(fctx);
}
#endif
