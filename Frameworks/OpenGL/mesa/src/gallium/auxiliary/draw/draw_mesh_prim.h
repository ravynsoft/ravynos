/**************************************************************************
 *
 * Copyright 2023 Red Hat.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef DRAW_MESH_PRIM_H
#define DRAW_MESH_PRIM_H

struct draw_context;
struct draw_prim_info;
struct draw_vertex_info;

void
draw_mesh_prim_run(struct draw_context *draw,
                   unsigned num_per_prim_inputs,
                   void *per_prim_inputs,
                   int cull_prim_idx,
                   const struct draw_prim_info *in_prim_info,
                   const struct draw_vertex_info *in_vert_info,
                   struct draw_prim_info *out_prim_info,
                   struct draw_vertex_info *out_vert_info);
#endif
