/*
 * Copyright 2014 VMware, Inc.
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
 */


#ifndef U_PRIM_RESTART_H
#define U_PRIM_RESTART_H


#include "pipe/p_defines.h"


#ifdef __cplusplus
extern "C" {
#endif


struct pipe_context;
struct pipe_draw_info;
union pipe_index_binding;
struct pipe_resource;

void
util_translate_prim_restart_data(unsigned index_size,
                                 void *src_map, void *dst_map,
                                 unsigned count, unsigned restart_index);

struct pipe_draw_start_count_bias *
util_prim_restart_convert_to_direct(const void *index_map,
                                    const struct pipe_draw_info *info,
                                    const struct pipe_draw_start_count_bias *draw,
                                    unsigned *num_draws,
                                    unsigned *min_index,
                                    unsigned *max_index,
                                    unsigned *total_index_count);

enum pipe_error
util_draw_vbo_without_prim_restart(struct pipe_context *context,
                                   const struct pipe_draw_info *info,
                                   unsigned drawid_offset,
                                   const struct pipe_draw_indirect_info *indirect,
                                   const struct pipe_draw_start_count_bias *draw);

static inline unsigned
util_prim_restart_index_from_size(unsigned index_size)
{
   /* 1 -> 0xff, 2 -> 0xffff, 4 -> 0xffffffff */
   return 0xffffffffu >> 8 * (4 - index_size);
}

#ifdef __cplusplus
}
#endif

#endif
