/**************************************************************************
 *
 * Copyright 2013 Grigori Goronzy <greg@chown.ath.cx>
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
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/* motion adaptive deinterlacer implementation */

#ifndef vl_deint_filter_h
#define vl_deint_filter_h

#include "pipe/p_state.h"

struct vl_deint_filter
{
   struct pipe_context *pipe;
   struct pipe_vertex_buffer quad;

   void *rs_state;
   void *blend[3];
   void *sampler[4];
   void *ves;
   void *vs;
   void *fs_copy_top, *fs_copy_bottom;
   void *fs_deint_top, *fs_deint_bottom;

   unsigned video_width, video_height;
   bool skip_chroma;
   bool interleaved;

   struct pipe_video_buffer *video_buffer;
};

bool
vl_deint_filter_init(struct vl_deint_filter *filter, struct pipe_context *pipe,
                     unsigned video_width, unsigned video_height,
                     bool skip_chroma, bool spatial_filter, bool interleaved);

void
vl_deint_filter_cleanup(struct vl_deint_filter *filter);

bool
vl_deint_filter_check_buffers(struct vl_deint_filter *filter,
                              struct pipe_video_buffer *prevprev,
                              struct pipe_video_buffer *prev,
                              struct pipe_video_buffer *cur,
                              struct pipe_video_buffer *next);

void
vl_deint_filter_render(struct vl_deint_filter *filter,
                       struct pipe_video_buffer *prevprev,
                       struct pipe_video_buffer *prev,
                       struct pipe_video_buffer *cur,
                       struct pipe_video_buffer *next,
                       unsigned field);

#endif /* vl_deint_filter_h */
