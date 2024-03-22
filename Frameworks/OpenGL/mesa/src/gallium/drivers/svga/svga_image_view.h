/**********************************************************
 * Copyright 2022 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#ifndef SVGA_IMAGE_VIEW_H
#define SVGA_IMAGE_VIEW_H

struct svga_image_view {
   struct pipe_image_view desc;
   struct pipe_resource *resource;
   unsigned uav_index;
};

void
svga_init_shader_image_functions(struct svga_context *svga);

void
svga_cleanup_shader_image_state(struct svga_context *svga);

SVGA3dUAViewId
svga_create_uav(struct svga_context *svga,
                SVGA3dUAViewDesc *desc,
                SVGA3dSurfaceFormat svga_format,
                unsigned resourceDim,
                struct svga_winsys_surface *surf);

void
svga_destroy_uav(struct svga_context *svga);

enum pipe_error
svga_rebind_uav(struct svga_context *svga);

enum pipe_error
svga_validate_image_view_resources(struct svga_context *svga, unsigned count,
                                   struct svga_image_view *images,
                                   bool rebind);

SVGA3dUAViewId
svga_create_uav_image(struct svga_context *svga,
                      const struct pipe_image_view *image);

void
svga_uav_cache_purge_image_views(struct svga_context *svga);

#endif /* SVGA_IMAGE_VIEW_H */
