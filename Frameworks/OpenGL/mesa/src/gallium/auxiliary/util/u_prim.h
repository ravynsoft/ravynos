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


#ifndef U_PRIM_H
#define U_PRIM_H


#include "pipe/p_defines.h"
#include "util/compiler.h"
#include "util/u_debug.h"
#include "compiler/shader_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

struct u_prim_vertex_count {
   unsigned min;
   unsigned incr;
};

/**
 * Decompose a primitive that is a loop, a strip, or a fan.  Return the
 * original primitive if it is already decomposed.
 */
static inline enum mesa_prim
u_decomposed_prim(enum mesa_prim prim)
{
   switch (prim) {
   case MESA_PRIM_LINE_LOOP:
   case MESA_PRIM_LINE_STRIP:
      return MESA_PRIM_LINES;
   case MESA_PRIM_TRIANGLE_STRIP:
   case MESA_PRIM_TRIANGLE_FAN:
      return MESA_PRIM_TRIANGLES;
   case MESA_PRIM_QUAD_STRIP:
      return MESA_PRIM_QUADS;
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
      return MESA_PRIM_LINES_ADJACENCY;
   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
      return MESA_PRIM_TRIANGLES_ADJACENCY;
   default:
      return prim;
   }
}

/**
 * Reduce a primitive to one of MESA_PRIM_POINTS, MESA_PRIM_LINES, and
 * MESA_PRIM_TRIANGLES.
 */
static inline enum mesa_prim
u_reduced_prim(enum mesa_prim prim)
{
   switch (prim) {
   case MESA_PRIM_POINTS:
      return MESA_PRIM_POINTS;
   case MESA_PRIM_LINES:
   case MESA_PRIM_LINE_LOOP:
   case MESA_PRIM_LINE_STRIP:
   case MESA_PRIM_LINES_ADJACENCY:
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
      return MESA_PRIM_LINES;
   default:
      return MESA_PRIM_TRIANGLES;
   }
}

/**
 * Re-assemble a primitive to remove its adjacency.
 */
static inline enum mesa_prim
u_assembled_prim(enum mesa_prim prim)
{
   switch (prim) {
   case MESA_PRIM_LINES_ADJACENCY:
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
      return MESA_PRIM_LINES;
   case MESA_PRIM_TRIANGLES_ADJACENCY:
   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
      return MESA_PRIM_TRIANGLES;
   default:
      return prim;
   }
}

/**
 * Return the vertex count information for a primitive.
 *
 * Note that if this function is called directly or indirectly anywhere in a
 * source file, it will increase the size of the binary slightly more than
 * expected because of the use of a table.
 */
static inline const struct u_prim_vertex_count *
u_prim_vertex_count(enum mesa_prim prim)
{
   static const struct u_prim_vertex_count prim_table[MESA_PRIM_COUNT] = {
      { 1, 1 }, /* MESA_PRIM_POINTS */
      { 2, 2 }, /* MESA_PRIM_LINES */
      { 2, 1 }, /* MESA_PRIM_LINE_LOOP */
      { 2, 1 }, /* MESA_PRIM_LINE_STRIP */
      { 3, 3 }, /* MESA_PRIM_TRIANGLES */
      { 3, 1 }, /* MESA_PRIM_TRIANGLE_STRIP */
      { 3, 1 }, /* MESA_PRIM_TRIANGLE_FAN */
      { 4, 4 }, /* MESA_PRIM_QUADS */
      { 4, 2 }, /* MESA_PRIM_QUAD_STRIP */
      { 3, 1 }, /* MESA_PRIM_POLYGON */
      { 4, 4 }, /* MESA_PRIM_LINES_ADJACENCY */
      { 4, 1 }, /* MESA_PRIM_LINE_STRIP_ADJACENCY */
      { 6, 6 }, /* MESA_PRIM_TRIANGLES_ADJACENCY */
      { 6, 2 }, /* MESA_PRIM_TRIANGLE_STRIP_ADJACENCY */
   };

   return (likely(prim < MESA_PRIM_COUNT)) ? &prim_table[prim] : NULL;
}

/**
 * Given a vertex count, return the number of primitives.
 * For polygons, return the number of triangles.
 */
static inline unsigned
u_prims_for_vertices(enum mesa_prim prim, unsigned num)
{
   const struct u_prim_vertex_count *info = u_prim_vertex_count(prim);

   assert(info);
   assert(info->incr != 0);

   if (num < info->min)
      return 0;

   return 1 + ((num - info->min) / info->incr);
}

static inline bool
u_validate_pipe_prim(enum mesa_prim pipe_prim, unsigned nr)
{
   const struct u_prim_vertex_count *count = u_prim_vertex_count(pipe_prim);

   return (count && nr >= count->min);
}


static inline bool
u_trim_pipe_prim(enum mesa_prim pipe_prim, unsigned *nr)
{
   const struct u_prim_vertex_count *count = u_prim_vertex_count(pipe_prim);

   if (count && *nr >= count->min) {
      if (count->incr > 1)
         *nr -= (*nr % count->incr);
      return true;
   }
   else {
      *nr = 0;
      return false;
   }
}


/**
 * Returns the number of reduced/tessellated primitives for the given vertex
 * count.  Each quad is treated as two triangles.  Polygons are treated as
 * triangle fans.
 */
static inline unsigned
u_reduced_prims_for_vertices(enum mesa_prim primitive, int vertices)
{
   switch (primitive) {
   case MESA_PRIM_QUADS:
   case MESA_PRIM_QUAD_STRIP:
      return u_decomposed_prims_for_vertices(primitive, vertices) * 2;
   case MESA_PRIM_POLYGON:
      primitive = MESA_PRIM_TRIANGLE_FAN;
      FALLTHROUGH;
   default:
      return u_decomposed_prims_for_vertices(primitive, vertices);
   }
}

static inline enum mesa_prim
u_base_prim_type(enum mesa_prim prim_type)
{
   switch(prim_type) {
      case MESA_PRIM_POINTS:
         return MESA_PRIM_POINTS;
      case MESA_PRIM_LINES:
      case MESA_PRIM_LINE_LOOP:
      case MESA_PRIM_LINE_STRIP:
      case MESA_PRIM_LINES_ADJACENCY:
      case MESA_PRIM_LINE_STRIP_ADJACENCY:
         return MESA_PRIM_LINES;
      case MESA_PRIM_TRIANGLES:
      case MESA_PRIM_TRIANGLE_STRIP:
      case MESA_PRIM_TRIANGLE_FAN:
      case MESA_PRIM_TRIANGLES_ADJACENCY:
      case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
         return MESA_PRIM_TRIANGLES;
      case MESA_PRIM_QUADS:
      case MESA_PRIM_QUAD_STRIP:
         return MESA_PRIM_QUADS;
      default:
         return prim_type;
   }
}

static inline enum mesa_prim
u_tess_prim_from_shader(enum tess_primitive_mode shader_mode)
{
   switch (shader_mode) {
   case TESS_PRIMITIVE_TRIANGLES:
      return MESA_PRIM_TRIANGLES;
   case TESS_PRIMITIVE_QUADS:
      return MESA_PRIM_QUADS;
   case TESS_PRIMITIVE_ISOLINES:
      return MESA_PRIM_LINES;
   default:
      return MESA_PRIM_POINTS;
   }
}

static inline unsigned
u_vertices_for_prims(enum mesa_prim prim_type, int count)
{
   if (count <= 0)
      return 0;

   /* We can only figure out the number of vertices from a number of primitives
    * if we are using basic primitives (so no loops, strips, fans, etc).
    */
   assert(prim_type == u_base_prim_type(prim_type) &&
          prim_type != MESA_PRIM_PATCHES && prim_type != MESA_PRIM_POLYGON);

   const struct u_prim_vertex_count *info = u_prim_vertex_count(prim_type);
   assert(info);

   return info->min + (count - 1) * info->incr;
}

/**
 * Returns the number of stream out outputs for a given number of vertices and
 * primitive type.
 */

static inline unsigned
u_stream_outputs_for_vertices(enum mesa_prim primitive, unsigned nr)
{
   /* Extraneous vertices don't contribute to stream outputs */
   u_trim_pipe_prim(primitive, &nr);

   /* Polygons are special, since they are a single primitive with many
    * vertices. In this case, we just have an output for each vertex (after
    * trimming) */

   if (primitive == MESA_PRIM_POLYGON)
      return nr;

   /* Normally, consider how many primitives are actually generated */
   unsigned prims = u_decomposed_prims_for_vertices(primitive, nr);

   /* One output per vertex after decomposition */
   enum mesa_prim base = u_base_prim_type(primitive);

   /* The GL 4.6 compatibility spec says
    *
    *    When quads and polygons are provided to transform feedback with a
    *    primitive mode of TRIANGLES, they will be tessellated and recorded as
    *    triangles (the order of tessellation within a primitive is undefined)
    *
    * Further, quads and polygons are always provided as TRIANGLES. So
    * tessellate quads into triangles.
    */
   if (base == MESA_PRIM_QUADS) {
      base = MESA_PRIM_TRIANGLES;
      prims *= 2;
   }

   return u_vertices_for_prims(base, prims);
}

const char *u_prim_name(enum mesa_prim pipe_prim);


#ifdef __cplusplus
}
#endif


#endif
