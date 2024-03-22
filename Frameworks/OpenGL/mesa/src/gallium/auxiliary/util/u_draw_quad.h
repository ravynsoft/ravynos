/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
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

#ifndef U_DRAWQUAD_H
#define U_DRAWQUAD_H


#include "util/compiler.h"
#include "pipe/p_context.h"

#include "util/u_draw.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pipe_resource;
struct cso_context;
struct cso_velems_state;

extern void
util_draw_vertex_buffer(struct pipe_context *pipe, struct cso_context *cso,
                        struct pipe_resource *vbuf, unsigned offset,
                        enum mesa_prim prim_type, unsigned num_attribs,
                        unsigned num_verts);

void
util_draw_user_vertex_buffer(struct cso_context *cso, void *buffer,
                             enum mesa_prim prim_type, unsigned num_verts,
                             unsigned num_attribs);

void
util_draw_user_vertices(struct cso_context *cso, struct cso_velems_state *ve,
                        void *buffer, enum mesa_prim prim_type,
                        unsigned num_verts);

#ifdef __cplusplus
}
#endif


#endif
