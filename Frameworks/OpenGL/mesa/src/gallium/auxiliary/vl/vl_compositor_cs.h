/**************************************************************************
 *
 * Copyright 2019 Advanced Micro Devices, Inc.
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
 * Authors: James Zhu <james.zhu<@amd.com>
 *
 **************************************************************************/

#ifndef vl_compositor_cs_h
#define vl_compositor_cs_h

#include "vl_compositor.h"

/**
 * render the layers to the frontbuffer with compute shader
 */
void
vl_compositor_cs_render(struct vl_compositor_state *s,
                        struct vl_compositor       *c,
                        struct pipe_surface        *dst_surface,
                        struct u_rect              *dirty_area,
                        bool                        clear_dirty);

bool vl_compositor_cs_init_shaders(struct vl_compositor *c);
void vl_compositor_cs_cleanup_shaders(struct vl_compositor *c);
#endif /* vl_compositor_cs_h */
