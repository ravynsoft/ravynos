/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
 * Copyright 2010 VMware, Inc.
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

/* Authors:
 *    Keith Whitwell, Qicheng Christopher Li, Brian Paul
 */

#ifndef LP_QUERY_H
#define LP_QUERY_H

#include <limits.h>
#include "util/u_thread.h"
#include "lp_limits.h"


struct llvmpipe_context;


struct llvmpipe_query {
   uint64_t start[LP_MAX_THREADS];  /* start count value for each thread */
   uint64_t end[LP_MAX_THREADS];    /* end count value for each thread */
   struct lp_fence *fence;          /* fence from last scene this was binned in */
   enum pipe_query_type type;
   unsigned index;
   unsigned num_primitives_generated[PIPE_MAX_VERTEX_STREAMS];
   unsigned num_primitives_written[PIPE_MAX_VERTEX_STREAMS];

   struct pipe_query_data_pipeline_statistics stats;
};


extern void llvmpipe_init_query_funcs(struct llvmpipe_context * );

extern bool llvmpipe_check_render_cond(struct llvmpipe_context *);

#endif /* LP_QUERY_H */
