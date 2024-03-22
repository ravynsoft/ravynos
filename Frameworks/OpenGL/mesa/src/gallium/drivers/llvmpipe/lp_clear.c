/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 * Copyright 2009 VMware, Inc.  All Rights Reserved.
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

/* Author:
 *    Brian Paul
 *    Michel Dänzer
 */


#include "pipe/p_defines.h"
#include "lp_clear.h"
#include "lp_context.h"
#include "lp_setup.h"
#include "lp_query.h"
#include "lp_debug.h"
#include "lp_state.h"


/**
 * Clear the given buffers to the specified values.
 * No masking, no scissor (clear entire buffer).
 */
void
llvmpipe_clear(struct pipe_context *pipe, 
               unsigned buffers,
               const struct pipe_scissor_state *scissor_state,
               const union pipe_color_union *color,
               double depth,
               unsigned stencil)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   if (!llvmpipe_check_render_cond(llvmpipe))
      return;

   llvmpipe_update_derived_clear(llvmpipe);

   if (LP_PERF & PERF_NO_DEPTH)
      buffers &= ~PIPE_CLEAR_DEPTHSTENCIL;

   lp_setup_clear( llvmpipe->setup, color, depth, stencil, buffers );
}
