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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __NEEDS_TRACE_PRIV
#error "Do not use this header!"
#endif

#ifndef _U_TRACE_PRIV_H
#define _U_TRACE_PRIV_H

#include "u_trace.h"
#include <stdio.h>

/*
 * Internal interface used by generated tracepoints
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Tracepoint descriptor.
 */
struct u_tracepoint {
   unsigned payload_sz;
   const char *name;
   /**
    * Whether this tracepoint's timestamp must be recorded with as an
    * end-of-pipe timestamp (for some GPUs the recording timestamp instruction
    * might be different for top/end of pipe).
    */
   bool end_of_pipe:1;
   /**
    * Index of this tracepoint in <basename>_tracepoint_names in the generated
    * u_trace perfetto header. By associating these names with iids in setup,
    * tracepoints can be presented with with their own names by passing that
    * to event->set_stage_iid().
    */
   uint16_t tp_idx;
   void (*print)(FILE *out, const void *payload);
   void (*print_json)(FILE *out, const void *payload);
#ifdef HAVE_PERFETTO
   /**
    * Callback to emit a perfetto event, such as render-stage trace
    */
   void (*perfetto)(void *pctx,
                    uint64_t ts_ns,
                    uint16_t tp_idx,
                    const void *flush_data,
                    const void *payload);
#endif
};

/**
 * Append a tracepoint followed by some amount of memory specified by
 * variable_sz, returning pointer that can be filled with trace payload.
 */
void *u_trace_appendv(struct u_trace *ut,
                      void *cs,
                      const struct u_tracepoint *tp,
                      unsigned variable_sz);

/**
 * Append a trace event, returning pointer to buffer of tp->payload_sz
 * to be filled in with trace payload.  Called by generated tracepoint
 * functions.
 */
static inline void *
u_trace_append(struct u_trace *ut, void *cs, const struct u_tracepoint *tp)
{
   return u_trace_appendv(ut, cs, tp, 0);
}

#ifdef __cplusplus
}
#endif

#endif /* _U_TRACE_PRIV_H */
