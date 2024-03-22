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

#include "r300_context.h"
#include "util/u_index_modify.h"
#include "util/u_upload_mgr.h"


void r300_translate_index_buffer(struct r300_context *r300,
                                 const struct pipe_draw_info *info,
                                 struct pipe_resource **out_buffer,
                                 unsigned *index_size, unsigned index_offset,
                                 unsigned *start, unsigned count)
{
    unsigned out_offset;
    void *ptr;

    switch (*index_size) {
    case 1:
        *out_buffer = NULL;
        u_upload_alloc(r300->uploader, 0, count * 2, 4,
                       &out_offset, out_buffer, &ptr);

        util_shorten_ubyte_elts_to_userptr(
                &r300->context, info, PIPE_MAP_UNSYNCHRONIZED, index_offset,
                *start, count, ptr);

        *index_size = 2;
        *start = out_offset / 2;
        break;

    case 2:
        if (index_offset) {
            *out_buffer = NULL;
            u_upload_alloc(r300->uploader, 0, count * 2, 4,
                           &out_offset, out_buffer, &ptr);

            util_rebuild_ushort_elts_to_userptr(&r300->context, info,
                                                PIPE_MAP_UNSYNCHRONIZED,
                                                index_offset, *start,
                                                count, ptr);

            *start = out_offset / 2;
        }
        break;

    case 4:
        if (index_offset) {
            *out_buffer = NULL;
            u_upload_alloc(r300->uploader, 0, count * 4, 4,
                           &out_offset, out_buffer, &ptr);

            util_rebuild_uint_elts_to_userptr(&r300->context, info,
                                              PIPE_MAP_UNSYNCHRONIZED,
                                              index_offset, *start,
                                              count, ptr);

            *start = out_offset / 4;
        }
        break;
    }
}
