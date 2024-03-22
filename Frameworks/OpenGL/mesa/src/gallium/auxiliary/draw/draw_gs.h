/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
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

#ifndef DRAW_GS_H
#define DRAW_GS_H

#include "draw_context.h"
#include "tgsi/tgsi_exec.h"
#include "tgsi/tgsi_scan.h"
#include "draw_private.h"

#define MAX_TGSI_PRIMITIVES 4

struct draw_context;

#if DRAW_LLVM_AVAILABLE
struct draw_gs_jit_context;
struct draw_gs_llvm_variant;

/**
 * Structure holding the inputs to the geometry shader. It uses SOA layout.
 * The dimensions are as follows:
 * - maximum number of vertices for a geometry shader input primitive
 *   (6 for triangle_adjacency)
 * - maximum number of attributes for each vertex
 * - four channels per each attribute (x,y,z,w)
 * - number of input primitives equal to the SOA vector length
 */
struct draw_gs_inputs {
   float data[6][PIPE_MAX_SHADER_INPUTS][TGSI_NUM_CHANNELS][TGSI_NUM_CHANNELS];
};
#endif

/**
 * Private version of the compiled geometry shader
 */
struct draw_vertex_stream {
   unsigned *primitive_lengths;
   unsigned emitted_vertices;
   unsigned emitted_primitives;
   float (*tmp_output)[4];
};

struct draw_geometry_shader {
   struct draw_context *draw;

   struct tgsi_exec_machine *machine;

   /* This member will disappear shortly:*/
   struct pipe_shader_state state;

   struct tgsi_shader_info info;
   unsigned position_output;
   unsigned viewport_index_output;
   unsigned clipvertex_output;
   unsigned ccdistance_output[PIPE_MAX_CLIP_OR_CULL_DISTANCE_ELEMENT_COUNT];

   unsigned max_output_vertices;
   unsigned primitive_boundary;
   unsigned input_primitive;
   unsigned output_primitive;
   unsigned vertex_size;

   struct draw_vertex_stream stream[TGSI_MAX_VERTEX_STREAMS];
   unsigned num_vertex_streams;

   unsigned in_prim_idx;
   unsigned input_vertex_stride;
   unsigned fetched_prim_count;
   const float (*input)[4];
   const struct tgsi_shader_info *input_info;
   unsigned vector_length;
   unsigned max_out_prims;

   unsigned num_invocations;
   unsigned invocation_id;
#if DRAW_LLVM_AVAILABLE
   struct draw_gs_inputs *gs_input;
   struct draw_gs_jit_context *jit_context;
   struct lp_jit_resources *jit_resources;
   struct draw_gs_llvm_variant *current_variant;
   struct vertex_header *gs_output[PIPE_MAX_VERTEX_STREAMS];

   int **llvm_prim_lengths;
   int *llvm_emitted_primitives;
   int *llvm_emitted_vertices;
   int *llvm_prim_ids;
#endif

   void (*fetch_inputs)(struct draw_geometry_shader *shader,
                        unsigned *indices,
                        unsigned num_vertices,
                        unsigned prim_idx);
   void (*fetch_outputs)(struct draw_geometry_shader *shader,
                         unsigned vertex_stream,
                         unsigned num_primitives,
                         float (**p_output)[4]);

   void (*prepare)(struct draw_geometry_shader *shader,
                   const struct draw_buffer_info *constants);
   void (*run)(struct draw_geometry_shader *shader,
               unsigned input_primitives, unsigned *out_prims);
};


void
draw_geometry_shader_new_instance(struct draw_geometry_shader *gs);


/*
 * Returns the number of vertices emitted.
 * The vertex shader can emit any number of vertices as long as it's
 * smaller than the GS_MAX_OUTPUT_VERTICES shader property.
 */
void
draw_geometry_shader_run(struct draw_geometry_shader *shader,
                         const struct draw_buffer_info *constants,
                         const struct draw_vertex_info *input_verts,
                         const struct draw_prim_info *input_prim,
                         const struct tgsi_shader_info *input_info,
                         struct draw_vertex_info *output_verts,
                         struct draw_prim_info *output_prims );

void
draw_geometry_shader_prepare(struct draw_geometry_shader *shader,
                             struct draw_context *draw);

int
draw_gs_max_output_vertices(struct draw_geometry_shader *shader,
                            unsigned pipe_prim);

#if DRAW_LLVM_AVAILABLE
void
draw_gs_set_current_variant(struct draw_geometry_shader *shader,
                            struct draw_gs_llvm_variant *variant);
#endif

#endif
