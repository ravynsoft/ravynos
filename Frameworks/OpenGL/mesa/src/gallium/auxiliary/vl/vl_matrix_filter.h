/**************************************************************************
 *
 * Copyright 2012 Christian König.
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

/* implementation of all matrix based filters like
   gaussian, mean, laplacian, emboss, sharpness etc.. */

#ifndef vl_matrix_filter_h
#define vl_matrix_filter_h

#include "pipe/p_state.h"

struct vl_matrix_filter
{
   struct pipe_context *pipe;
   struct pipe_vertex_buffer quad;

   void *rs_state;
   void *blend;
   void *sampler;
   void *ves;
   void *vs, *fs;
};

bool
vl_matrix_filter_init(struct vl_matrix_filter *filter, struct pipe_context *pipe,
                      unsigned video_width, unsigned video_height,
                      unsigned matrix_width, unsigned matrix_height,
                      const float *matrix_values);

void
vl_matrix_filter_cleanup(struct vl_matrix_filter *filter);


void
vl_matrix_filter_render(struct vl_matrix_filter *filter,
                        struct pipe_sampler_view *src,
                        struct pipe_surface *dst);


#endif /* vl_matrix_filter_h */
