/**************************************************************************
 *
 * Copyright 2013 VMware, Inc.
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

/*
 * Input assembler needs to be able to decompose adjacency primitives
 * into something that can be understood by the rest of the pipeline.
 * The specs say that the adjacency primitives are *only* visible
 * in the geometry shader, for everything else they need to be
 * decomposed. Which in most of the cases is not an issue, because the
 * geometry shader always decomposes them for us, but without geometry
 * shader we were passing unchanged adjacency primitives to the
 * rest of the pipeline and causing crashes everywhere.
 * If geometry shader is missing and the input primitive is one of
 * the adjacency primitives we use the code from this file to
 * decompose them into something that the rest of the pipeline can
 * understand.
 *
 */

#ifndef DRAW_PRIM_ASSEMBLER_H
#define DRAW_PRIM_ASSEMBLER_H

#include "draw/draw_private.h"

struct draw_assembler;

struct draw_assembler *
draw_prim_assembler_create(struct draw_context *draw);

void
draw_prim_assembler_destroy(struct draw_assembler *ia);

bool
draw_prim_assembler_is_required(const struct draw_context *draw,
                                const struct draw_prim_info *prim_info,
                                const struct draw_vertex_info *vert_info);

void
draw_prim_assembler_run(struct draw_context *draw,
                        const struct draw_prim_info *in_prim_info,
                        const struct draw_vertex_info *in_vert_info,
                        struct draw_prim_info *out_prim_info,
                        struct draw_vertex_info *out_vert_info);


void
draw_prim_assembler_prepare_outputs(struct draw_assembler *ia);

void
draw_prim_assembler_new_instance(struct draw_assembler *ia);


#endif
