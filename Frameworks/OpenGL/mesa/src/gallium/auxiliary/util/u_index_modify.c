/*
 * Copyright 2010 Marek Olšák <maraeo@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "pipe/p_context.h"
#include "util/u_index_modify.h"
#include "util/u_inlines.h"

/* Ubyte indices. */

void util_shorten_ubyte_elts_to_userptr(struct pipe_context *context,
					const struct pipe_draw_info *info,
                                        unsigned add_transfer_flags,
					int index_bias,
					unsigned start,
					unsigned count,
					void *out)
{
    struct pipe_transfer *src_transfer = NULL;
    const unsigned char *in_map;
    unsigned short *out_map = out;
    unsigned i;

    if (info->has_user_indices) {
       in_map = info->index.user;
    } else {
       in_map = pipe_buffer_map(context, info->index.resource,
                                PIPE_MAP_READ |
                                add_transfer_flags,
                                &src_transfer);
    }
    in_map += start;

    for (i = 0; i < count; i++) {
        *out_map = (unsigned short)(*in_map + index_bias);
        in_map++;
        out_map++;
    }

    if (src_transfer)
       pipe_buffer_unmap(context, src_transfer);
}

/* Ushort indices. */

void util_rebuild_ushort_elts_to_userptr(struct pipe_context *context,
					 const struct pipe_draw_info *info,
                                         unsigned add_transfer_flags,
					 int index_bias,
					 unsigned start, unsigned count,
					 void *out)
{
    struct pipe_transfer *in_transfer = NULL;
    const unsigned short *in_map;
    unsigned short *out_map = out;
    unsigned i;

    if (info->has_user_indices) {
       in_map = info->index.user;
    } else {
       in_map = pipe_buffer_map(context, info->index.resource,
                                PIPE_MAP_READ |
                                add_transfer_flags,
                                &in_transfer);
    }
    in_map += start;

    for (i = 0; i < count; i++) {
        *out_map = (unsigned short)(*in_map + index_bias);
        in_map++;
        out_map++;
    }

    if (in_transfer)
       pipe_buffer_unmap(context, in_transfer);
}

/* Uint indices. */

void util_rebuild_uint_elts_to_userptr(struct pipe_context *context,
				       const struct pipe_draw_info *info,
                                       unsigned add_transfer_flags,
				       int index_bias,
				       unsigned start, unsigned count,
				       void *out)
{
    struct pipe_transfer *in_transfer = NULL;
    const unsigned int *in_map;
    unsigned int *out_map = out;
    unsigned i;

    if (info->has_user_indices) {
       in_map = info->index.user;
    } else {
       in_map = pipe_buffer_map(context, info->index.resource,
                                PIPE_MAP_READ |
                                add_transfer_flags,
                                &in_transfer);
    }
    in_map += start;

    for (i = 0; i < count; i++) {
        *out_map = (unsigned int)(*in_map + index_bias);
        in_map++;
        out_map++;
    }

    if (in_transfer)
       pipe_buffer_unmap(context, in_transfer);
}
