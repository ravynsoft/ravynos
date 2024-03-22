/**************************************************************************
 *
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


#ifndef U_SURFACE_H
#define U_SURFACE_H


#include "util/compiler.h"
#include "pipe/p_state.h"

#include "util/u_pack_color.h"


#ifdef __cplusplus
extern "C" {
#endif


extern void
u_surface_default_template(struct pipe_surface *view,
                           const struct pipe_resource *texture);

extern void
util_copy_box(uint8_t * dst,
              enum pipe_format format,
              unsigned dst_stride, uint64_t dst_slice_stride,
              unsigned dst_x, unsigned dst_y, unsigned dst_z,
              unsigned width, unsigned height, unsigned depth,
              const uint8_t * src,
              int src_stride, uint64_t src_slice_stride,
              unsigned src_x, unsigned src_y, unsigned src_z);

extern void
util_fill_rect(uint8_t * dst, enum pipe_format format,
               unsigned dst_stride, unsigned dst_x, unsigned dst_y,
               unsigned width, unsigned height, union util_color *uc);

extern void
util_fill_box(uint8_t * dst, enum pipe_format format,
              unsigned stride, uintptr_t layer_stride,
              unsigned x, unsigned y, unsigned z,
              unsigned width, unsigned height, unsigned depth,
              union util_color *uc);

extern void
util_fill_zs_box(uint8_t *dst, enum pipe_format format,
                 bool need_rmw, unsigned clear_flags, unsigned stride,
                 unsigned layer_stride, unsigned width,
                 unsigned height, unsigned depth,
                 uint64_t zstencil);

extern void
util_resource_copy_region(struct pipe_context *pipe,
                          struct pipe_resource *dst,
                          unsigned dst_level,
                          unsigned dst_x, unsigned dst_y, unsigned dst_z,
                          struct pipe_resource *src,
                          unsigned src_level,
                          const struct pipe_box *src_box);

extern void
u_default_clear_texture(struct pipe_context *pipe,
                        struct pipe_resource *tex,
                        unsigned level,
                        const struct pipe_box *box,
                        const void *data);

extern void
util_clear_texture_sw(struct pipe_context *pipe,
                      struct pipe_resource *tex,
                      unsigned level,
                      const struct pipe_box *box,
                      const void *data);

extern void
util_clear_render_target(struct pipe_context *pipe,
                         struct pipe_surface *dst,
                         const union pipe_color_union *color,
                         unsigned dstx, unsigned dsty,
                         unsigned width, unsigned height);

extern void
util_clear_depth_stencil(struct pipe_context *pipe,
                         struct pipe_surface *dst,
                         unsigned clear_flags,
                         double depth,
                         unsigned stencil,
                         unsigned dstx, unsigned dsty,
                         unsigned width, unsigned height);

bool
util_can_blit_via_copy_region(const struct pipe_blit_info *blit,
                              bool tight_format_check,
                              bool render_condition_bound);

extern bool
util_try_blit_via_copy_region(struct pipe_context *ctx,
                              const struct pipe_blit_info *blit,
                              bool render_condition_bound);


#ifdef __cplusplus
}
#endif


#endif /* U_SURFACE_H */
