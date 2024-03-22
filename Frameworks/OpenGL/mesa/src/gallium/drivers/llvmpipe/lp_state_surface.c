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

/* Authors:  Keith Whitwell <keithw@vmware.com>
 */

#include "pipe/p_state.h"
#include "util/u_inlines.h"
#include "util/u_framebuffer.h"
#include "util/u_surface.h"
#include "lp_context.h"
#include "lp_scene.h"
#include "lp_state.h"
#include "lp_setup.h"

#include "draw/draw_context.h"

#include "util/format/u_format.h"


/**
 * Set the framebuffer surface info: color buffers, zbuffer, stencil buffer.
 */
void
llvmpipe_set_framebuffer_state(struct pipe_context *pipe,
                               const struct pipe_framebuffer_state *fb)
{
   struct llvmpipe_context *lp = llvmpipe_context(pipe);

   bool changed = !util_framebuffer_state_equal(&lp->framebuffer, fb);

   assert(fb->width <= LP_MAX_WIDTH);
   assert(fb->height <= LP_MAX_HEIGHT);

   if (changed) {
      /*
       * If no depth buffer is bound, send the utility function the default
       * format for no bound depth (PIPE_FORMAT_NONE).
       */
      enum pipe_format depth_format = fb->zsbuf && !(LP_PERF & PERF_NO_DEPTH) ?
         fb->zsbuf->format : PIPE_FORMAT_NONE;
      const struct util_format_description *depth_desc =
         util_format_description(depth_format);

      if (fb->zsbuf && fb->zsbuf->context != pipe) {
         debug_printf("Illegal setting of fb state with zsbuf created in "
                       "another context\n");
      }
      for (unsigned i = 0; i < fb->nr_cbufs; i++) {
         if (fb->cbufs[i] &&
             fb->cbufs[i]->context != pipe) {
            debug_printf("Illegal setting of fb state with cbuf %d created in "
                          "another context\n", i);
         }
      }

      util_copy_framebuffer_state(&lp->framebuffer, fb);

      if (LP_PERF & PERF_NO_DEPTH) {
         pipe_surface_reference(&lp->framebuffer.zsbuf, NULL);
      }

      /*
       * Calculate the floating point depth sense and Minimum Resolvable Depth
       * value for the llvmpipe module. This is separate from the draw module.
       */
      lp->floating_point_depth =
         (util_get_depth_format_type(depth_desc) == UTIL_FORMAT_TYPE_FLOAT);

      lp->mrd = util_get_depth_format_mrd(depth_desc);

      /* Tell the draw module how deep the Z/depth buffer is. */
      draw_set_zs_format(lp->draw, depth_format);

      lp_setup_bind_framebuffer(lp->setup, &lp->framebuffer);

      lp->dirty |= LP_NEW_FRAMEBUFFER;
   }
}
