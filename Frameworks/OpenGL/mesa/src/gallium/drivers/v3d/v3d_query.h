/*
 * Copyright © 2021 Raspberry Pi Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef V3D_QUERY_H
#define V3D_QUERY_H

#include "v3d_context.h"

struct v3d_query;

struct v3d_query_funcs {
        void (*destroy_query)(struct v3d_context *v3d, struct v3d_query *query);
        bool (*begin_query)(struct v3d_context *v3d, struct v3d_query *query);
        bool (*end_query)(struct v3d_context *v3d, struct v3d_query *query);
        bool (*get_query_result)(struct v3d_context *v3d, struct v3d_query *query,
                                 bool wait, union pipe_query_result *result);
};

struct v3d_query
{
        const struct v3d_query_funcs *funcs;
};

struct pipe_query *v3d_create_query_pipe(struct v3d_context *v3d, unsigned query_type, unsigned index);

#endif /* V3D_QUERY_H */
