/**************************************************************************
 *
 * Copyright 2016 Nayan Deshmukh.
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

/* implementation of bicubic interpolation filters */

#ifndef vl_bicubic_filter_h
#define vl_bicubic_filter_h

#include "pipe/p_state.h"

struct vl_bicubic_filter
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
vl_bicubic_filter_init(struct vl_bicubic_filter *filter, struct pipe_context *pipe,
                      unsigned width, unsigned height);

void
vl_bicubic_filter_cleanup(struct vl_bicubic_filter *filter);


void
vl_bicubic_filter_render(struct vl_bicubic_filter *filter,
                        struct pipe_sampler_view *src,
                        struct pipe_surface *dst,
                        struct u_rect *dst_area,
                        struct u_rect *dst_clip);


#endif /* vl_bicubic_filter_h */
