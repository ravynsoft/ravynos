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


#include "lp_context.h"
#include "lp_state.h"

#include "draw/draw_context.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_transfer.h"


static void *
llvmpipe_create_vertex_elements_state(struct pipe_context *pipe,
                                      unsigned count,
                                      const struct pipe_vertex_element *attribs)
{
   struct lp_velems_state *velems;
   assert(count <= PIPE_MAX_ATTRIBS);
   velems = (struct lp_velems_state *) MALLOC(sizeof(struct lp_velems_state));
   if (velems) {
      velems->count = count;
      memcpy(velems->velem, attribs, sizeof(*attribs) * count);
   }
   return velems;
}


static void
llvmpipe_bind_vertex_elements_state(struct pipe_context *pipe,
                                    void *velems)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);
   struct lp_velems_state *lp_velems = (struct lp_velems_state *) velems;

   llvmpipe->velems = lp_velems;

   llvmpipe->dirty |= LP_NEW_VERTEX;

   if (velems)
      draw_set_vertex_elements(llvmpipe->draw, lp_velems->count,
                               lp_velems->velem);
}


static void
llvmpipe_delete_vertex_elements_state(struct pipe_context *pipe, void *velems)
{
   FREE(velems);
}


static void
llvmpipe_set_vertex_buffers(struct pipe_context *pipe,
                            unsigned count,
                            unsigned unbind_num_trailing_slots,
                            bool take_ownership,
                            const struct pipe_vertex_buffer *buffers)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   assert(count <= PIPE_MAX_ATTRIBS);

   util_set_vertex_buffers_count(llvmpipe->vertex_buffer,
                                 &llvmpipe->num_vertex_buffers,
                                 buffers, count,
                                 unbind_num_trailing_slots,
                                 take_ownership);

   llvmpipe->dirty |= LP_NEW_VERTEX;

   draw_set_vertex_buffers(llvmpipe->draw, count,
                           unbind_num_trailing_slots, buffers);
}


void
llvmpipe_init_vertex_funcs(struct llvmpipe_context *llvmpipe)
{
   llvmpipe->pipe.create_vertex_elements_state = llvmpipe_create_vertex_elements_state;
   llvmpipe->pipe.bind_vertex_elements_state = llvmpipe_bind_vertex_elements_state;
   llvmpipe->pipe.delete_vertex_elements_state = llvmpipe_delete_vertex_elements_state;

   llvmpipe->pipe.set_vertex_buffers = llvmpipe_set_vertex_buffers;
}
