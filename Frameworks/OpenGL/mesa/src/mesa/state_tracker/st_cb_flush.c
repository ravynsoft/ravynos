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

 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  *   Brian Paul
  */

#include "util/glheader.h"
#include "main/macros.h"
#include "main/context.h"
#include "st_context.h"
#include "st_cb_bitmap.h"
#include "st_cb_flush.h"
#include "st_cb_clear.h"
#include "st_context.h"
#include "st_manager.h"
#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "util/u_gen_mipmap.h"
#include "util/perf/cpu_trace.h"


void
st_flush(struct st_context *st,
         struct pipe_fence_handle **fence,
         unsigned flags)
{
   MESA_TRACE_FUNC();

   /* We want to call this function periodically.
    * Typically, it has nothing to do so it shouldn't be expensive.
    */
   st_context_free_zombie_objects(st);

   st_flush_bitmap_cache(st);
   st->pipe->flush(st->pipe, fence, flags);
}


/**
 * Flush, and wait for completion.
 */
void
st_finish(struct st_context *st)
{
   struct pipe_fence_handle *fence = NULL;

   MESA_TRACE_FUNC();

   st_flush(st, &fence, PIPE_FLUSH_ASYNC | PIPE_FLUSH_HINT_FINISH);

   if (fence) {
      st->screen->fence_finish(st->screen, NULL, fence,
                               OS_TIMEOUT_INFINITE);
      st->screen->fence_reference(st->screen, &fence, NULL);
   }

   st_manager_flush_swapbuffers();
}


void
st_glFlush(struct gl_context *ctx, unsigned gallium_flush_flags)
{
   struct st_context *st = st_context(ctx);

   /* Don't call st_finish() here.  It is not the state tracker's
    * responsibilty to inject sleeps in the hope of avoiding buffer
    * synchronization issues.  Calling finish() here will just hide
    * problems that need to be fixed elsewhere.
    */
   st_flush(st, NULL, gallium_flush_flags);

   st_manager_flush_frontbuffer(st);
}

void
st_glFinish(struct gl_context *ctx)
{
   struct st_context *st = st_context(ctx);

   st_finish(st);

   st_manager_flush_frontbuffer(st);
}


static GLenum
gl_reset_status_from_pipe_reset_status(enum pipe_reset_status status)
{
   switch (status) {
   case PIPE_NO_RESET:
      return GL_NO_ERROR;
   case PIPE_GUILTY_CONTEXT_RESET:
      return GL_GUILTY_CONTEXT_RESET_ARB;
   case PIPE_INNOCENT_CONTEXT_RESET:
      return GL_INNOCENT_CONTEXT_RESET_ARB;
   case PIPE_UNKNOWN_CONTEXT_RESET:
      return GL_UNKNOWN_CONTEXT_RESET_ARB;
   default:
      assert(0);
      return GL_NO_ERROR;
   }
}


static void
st_device_reset_callback(void *data, enum pipe_reset_status status)
{
   struct st_context *st = data;

   assert(status != PIPE_NO_RESET);

   st->reset_status = status;
   _mesa_set_context_lost_dispatch(st->ctx);
}


/**
 * Query information about GPU resets observed by this context
 *
 * Called via \c dd_function_table::GetGraphicsResetStatus.
 */
static GLenum
st_get_graphics_reset_status(struct gl_context *ctx)
{
   struct st_context *st = st_context(ctx);
   enum pipe_reset_status status;

   if (st->reset_status != PIPE_NO_RESET) {
      status = st->reset_status;
      st->reset_status = PIPE_NO_RESET;
   } else {
      status = st->pipe->get_device_reset_status(st->pipe);
      if (status != PIPE_NO_RESET)
         st_device_reset_callback(st, status);
   }

   return gl_reset_status_from_pipe_reset_status(status);
}


void
st_install_device_reset_callback(struct st_context *st)
{
   if (st->pipe->set_device_reset_callback) {
      struct pipe_device_reset_callback cb;
      cb.reset = st_device_reset_callback;
      cb.data = st;
      st->pipe->set_device_reset_callback(st->pipe, &cb);
   }
}


void
st_init_flush_functions(struct pipe_screen *screen,
                        struct dd_function_table *functions)
{
   if (screen->get_param(screen, PIPE_CAP_DEVICE_RESET_STATUS_QUERY))
      functions->GetGraphicsResetStatus = st_get_graphics_reset_status;
}
