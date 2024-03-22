/*
 * Copyright 2023 Alyssa Rosenzweig
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "compiler/shader_enums.h"
#include "libagx.h"

#ifndef __OPENCL_VERSION__
#include "util/macros.h"
#define GLOBAL(type_) uint64_t
#define CONST(type_)  uint64_t
#else
#define PACKED
#define GLOBAL(type_) global type_ *
#define CONST(type_)  constant type_ *
#endif

#ifndef LIBAGX_GEOMETRY_H
#define LIBAGX_GEOMETRY_H

#define MAX_SO_BUFFERS     4
#define MAX_VERTEX_STREAMS 4

struct agx_ia_key {
   /* The index size (1, 2, 4) or 0 if drawing without an index buffer. */
   uint8_t index_size;

   /* The primitive mode for unrolling the vertex ID */
   enum mesa_prim mode;

   /* Use first vertex as the provoking vertex for flat shading */
   bool flatshade_first;

   /* Whether we are doing input assembly for an indirect multidraw that is
    * implemented by a single superdraw with a prefix sum of vertex counts per
    * draw. This requires lowering lots of sysvals to index into the draw
    * descriptors according to the associated dynamic multidraw state.
    */
   bool indirect_multidraw;
};

/* Packed geometry state buffer */
struct agx_geometry_state {
   /* Heap to allocate from, in either direction. By convention, the top is used
    * for intra-draw allocations and the bottom is used for full-batch
    * allocations. In the future we could use kernel support to improve this.
    */
   GLOBAL(uchar) heap;
   uint32_t heap_bottom, heap_top, heap_size, padding;
} PACKED;

struct agx_ia_state {
   /* Heap to allocate from across draws */
   GLOBAL(struct agx_geometry_state) heap;

   /* Input: index buffer if present. */
   CONST(uchar) index_buffer;

   /* Input: draw count */
   CONST(uint) count;

   /* Input: indirect draw descriptor. Raw pointer since it's strided. */
   uint64_t draws;

   /* For the geom/tess path, this is the temporary prefix sum buffer.
    * Caller-allocated. For regular MDI, this is ok since the CPU knows the
    * worst-case draw count.
    */
   GLOBAL(uint) prefix_sums;

   /* When unrolling primitive restart, output draw descriptors */
   GLOBAL(uint) out_draws;

   /* Input: maximum draw count, count is clamped to this */
   uint32_t max_draws;

   /* Primitive restart index, if unrolling */
   uint32_t restart_index;

   /* Input index buffer size in bytes, if unrolling */
   uint32_t index_buffer_size_B;

   /* Stride for the draw descrptor array */
   uint32_t draw_stride;

   /* When unrolling primitive restart, use first vertex as the provoking vertex
    * for flat shading. We could stick this in the key, but meh, you're already
    * hosed for perf on the unroll path.
    */
   uint32_t flatshade_first;

   /* The index size (1, 2, 4) or 0 if drawing without an index buffer. */
   uint32_t index_size_B;
} PACKED;

struct agx_geometry_params {
   /* Persistent (cross-draw) geometry state */
   GLOBAL(struct agx_geometry_state) state;

   /* Address of associated indirect draw buffer */
   GLOBAL(uint) indirect_desc;

   /* Address of count buffer. For an indirect draw, this will be written by the
    * indirect setup kernel.
    */
   GLOBAL(uint) count_buffer;

   /* Address of the primitives generated counters */
   GLOBAL(uint) prims_generated_counter[MAX_VERTEX_STREAMS];
   GLOBAL(uint) xfb_prims_generated_counter[MAX_VERTEX_STREAMS];
   GLOBAL(uint) xfb_overflow[MAX_VERTEX_STREAMS];
   GLOBAL(uint) xfb_any_overflow;

   /* Pointers to transform feedback buffer offsets in bytes */
   GLOBAL(uint) xfb_offs_ptrs[MAX_SO_BUFFERS];

   /* Output (vertex) buffer, allocated by pre-GS. */
   GLOBAL(uint) output_buffer;

   /* Output index buffer, allocated by pre-GS. */
   GLOBAL(uint) output_index_buffer;

   /* Address of transform feedback buffer in general, supplied by the CPU. */
   GLOBAL(uchar) xfb_base_original[MAX_SO_BUFFERS];
   uint32_t xfb_size[MAX_SO_BUFFERS];

   /* Address of transform feedback for the current primitive. Written by pre-GS
    * program.
    */
   GLOBAL(uchar) xfb_base[MAX_SO_BUFFERS];

   /* Number of primitives emitted by transform feedback per stream. Written by
    * the pre-GS program.
    */
   uint32_t xfb_prims[MAX_VERTEX_STREAMS];

   /* Location-indexed mask of flat outputs, used for lowering GL edge flags. */
   uint64_t flat_outputs;

   /* Within an indirect GS draw, the grid used to dispatch the GS written out
    * by the GS indirect setup kernel. Unused for direct GS draws.
    */
   uint32_t gs_grid[3];

   /* Number of input primitives, calculated by the CPU for a direct draw or the
    * GS indirect setup kernel for an indirect draw.
    */
   uint32_t input_primitives;

   /* Number of bytes output by the GS count shader per input primitive (may be
    * 0), written by CPU and consumed by indirect draw setup shader for
    * allocating counts.
    */
   uint32_t count_buffer_stride;
} PACKED;

#endif
