/*
 * Copyright © 2020 Google, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "u_trace_gallium.h"
#include "u_inlines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"

#include "u_tracepoints.h"

#ifdef __cplusplus
extern "C" {
#endif

static void *
u_trace_pipe_create_ts_buffer(struct u_trace_context *utctx, uint32_t size)
{
   struct pipe_context *ctx = utctx->pctx;

   struct pipe_resource tmpl = {
      .target     = PIPE_BUFFER,
      .format     = PIPE_FORMAT_R8_UNORM,
      .bind       = PIPE_BIND_QUERY_BUFFER | PIPE_BIND_LINEAR,
      .width0     = size,
      .height0    = 1,
      .depth0     = 1,
      .array_size = 1,
   };

   return ctx->screen->resource_create(ctx->screen, &tmpl);
}

static void
u_trace_pipe_delete_ts_buffer(struct u_trace_context *utctx, void *timestamps)
{
   struct pipe_resource *buffer = timestamps;
   pipe_resource_reference(&buffer, NULL);
}

void
u_trace_pipe_context_init(struct u_trace_context *utctx,
                          struct pipe_context *pctx,
                          u_trace_record_ts record_timestamp,
                          u_trace_read_ts read_timestamp,
                          u_trace_delete_flush_data delete_flush_data)
{
   u_trace_context_init(utctx, pctx,
                        u_trace_pipe_create_ts_buffer,
                        u_trace_pipe_delete_ts_buffer,
                        record_timestamp,
                        read_timestamp,
                        delete_flush_data);
}

inline void
trace_framebuffer_state(struct u_trace *ut, void *cs, const struct pipe_framebuffer_state *pfb)
{
   if (likely(!u_trace_enabled(ut->utctx)))
      return;

   trace_framebuffer(ut, cs, pfb);

   for (unsigned i = 0; i < pfb->nr_cbufs; i++) {
      if (pfb->cbufs[i]) {
         trace_surface(ut, cs, pfb->cbufs[i]);
      }
   }
   if (pfb->zsbuf) {
      trace_surface(ut, cs, pfb->zsbuf);
   }
}

#ifdef __cplusplus
}
#endif
