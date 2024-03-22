/**************************************************************************
 *
 * Copyright 2009 Younes Manton.
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

#ifndef vl_compositor_gfx_h
#define vl_compositor_gfx_h

#include "vl_compositor.h"

/**
 * create vertex shader
 */
void *
create_vert_shader(struct vl_compositor *c);

/**
 * create YCbCr-to-RGB fragment
 */
void *
create_frag_shader_video_buffer(struct vl_compositor *c);

/**
 * create YCbCr-to-RGB weave fragment shader
 */
void *
create_frag_shader_weave_rgb(struct vl_compositor *c);

/**
 * create YCbCr i-to-YCbCr p deint fragment shader
 */
void *
create_frag_shader_deint_yuv(struct vl_compositor *c,
                             bool                  y,
                             bool                  w);

/**
 * create YUV/RGB-Palette-to-RGB fragment shader
 */
void *
create_frag_shader_palette(struct vl_compositor *c,
                           bool                  include_cc);

/**
 * create YCbCr RGB-to-RGB fragment shader
 */
void *
create_frag_shader_rgba(struct vl_compositor *c);

/**
 * create RGB-to-YUV fragment shader
 */
void *
create_frag_shader_rgb_yuv(struct vl_compositor *c,
                           bool                  y);

/**
 * render the layers to the frontbuffer with graphic shader
 */
void
vl_compositor_gfx_render(struct vl_compositor_state *s,
                         struct vl_compositor       *c,
                         struct pipe_surface        *dst_surface,
                         struct u_rect              *dirty_area,
                         bool                        clear_dirty);
#endif /* vl_compositor_gfx_h */
