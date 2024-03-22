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

#ifndef _U_TRACE_GALLIUM_H
#define _U_TRACE_GALLIUM_H

#include "util/perf/u_trace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Gallium specific u_trace helpers */

struct pipe_context;
struct pipe_framebuffer_state;

void
u_trace_pipe_context_init(struct u_trace_context *utctx,
                          struct pipe_context *pctx,
                          u_trace_record_ts record_timestamp,
                          u_trace_read_ts read_timestamp,
                          u_trace_delete_flush_data delete_flush_data);

/*
 * In some cases it is useful to have composite tracepoints like this,
 * to log more complex data structures.
 */

void
trace_framebuffer_state(struct u_trace *ut, void *cs, const struct pipe_framebuffer_state *pfb);

#ifdef __cplusplus
}
#endif

#endif  /* _U_TRACE_GALLIUM_H */
