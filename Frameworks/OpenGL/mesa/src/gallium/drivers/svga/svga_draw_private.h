/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#ifndef SVGA_DRAW_H_
#define SVGA_DRAW_H_

#include "util/compiler.h"
#include "pipe/p_defines.h"
#include "indices/u_indices.h"
#include "util/u_prim.h"
#include "svga_context.h"
#include "svga_hw_reg.h"
#include "svga3d_shaderdefs.h"

struct svga_context;
struct u_upload_mgr;

/**
 * Mask indicating which types of gallium primitives are actually
 * handled by the svga device.  Other types will be converted to
 * these types by the index/translation code.
 */
static const unsigned svga_hw_prims =
   ((1 << MESA_PRIM_POINTS) |
    (1 << MESA_PRIM_LINES) |
    (1 << MESA_PRIM_LINE_STRIP) |
    (1 << MESA_PRIM_TRIANGLES) |
    (1 << MESA_PRIM_TRIANGLE_STRIP) |
    (1 << MESA_PRIM_TRIANGLE_FAN) |
    (1 << MESA_PRIM_LINES_ADJACENCY) |
    (1 << MESA_PRIM_LINE_STRIP_ADJACENCY) |
    (1 << MESA_PRIM_TRIANGLES_ADJACENCY) |
    (1 << MESA_PRIM_TRIANGLE_STRIP_ADJACENCY) |
    (1 << MESA_PRIM_PATCHES));


/**
 * Translate a gallium MESA_PRIM_x value to an SVGA3D_PRIMITIVE_x value.
 * Also, compute the number of primitives that'll be drawn given a
 * vertex count.
 * Note that this function doesn't have to handle MESA_PRIM_LINE_LOOP,
 * MESA_PRIM_QUADS, MESA_PRIM_QUAD_STRIP or MESA_PRIM_POLYGON.  We convert
 * those to other types of primitives with index/translation code.
 */
static inline SVGA3dPrimitiveType
svga_translate_prim(unsigned mode, unsigned vcount, unsigned *prim_count,
                    uint8_t vertices_per_patch)
{
   switch (mode) {
   case MESA_PRIM_POINTS:
      *prim_count = vcount;
      return SVGA3D_PRIMITIVE_POINTLIST;

   case MESA_PRIM_LINES:
      *prim_count = vcount / 2;
      return SVGA3D_PRIMITIVE_LINELIST;

   case MESA_PRIM_LINE_STRIP:
      *prim_count = vcount - 1;
      return SVGA3D_PRIMITIVE_LINESTRIP;

   case MESA_PRIM_TRIANGLES:
      *prim_count = vcount / 3;
      return SVGA3D_PRIMITIVE_TRIANGLELIST;

   case MESA_PRIM_TRIANGLE_STRIP:
      *prim_count = vcount - 2;
      return SVGA3D_PRIMITIVE_TRIANGLESTRIP;

   case MESA_PRIM_TRIANGLE_FAN:
      *prim_count = vcount - 2;
      return SVGA3D_PRIMITIVE_TRIANGLEFAN;

   case MESA_PRIM_LINES_ADJACENCY:
      *prim_count = vcount / 4;
      return SVGA3D_PRIMITIVE_LINELIST_ADJ;

   case MESA_PRIM_LINE_STRIP_ADJACENCY:
      *prim_count = vcount - 3;
      return SVGA3D_PRIMITIVE_LINESTRIP_ADJ;

   case MESA_PRIM_TRIANGLES_ADJACENCY:
      *prim_count = vcount / 6;
      return SVGA3D_PRIMITIVE_TRIANGLELIST_ADJ;

   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
      *prim_count = vcount / 2 - 2 ;
      return SVGA3D_PRIMITIVE_TRIANGLESTRIP_ADJ;

   case MESA_PRIM_PATCHES:
      *prim_count = vcount / vertices_per_patch ;
      assert(vertices_per_patch >= 1);
      assert(vertices_per_patch <= 32);
      return (SVGA3D_PRIMITIVE_1_CONTROL_POINT_PATCH - 1)
             + vertices_per_patch;

   default:
      assert(0);
      *prim_count = 0;
      return 0;
   }
}


struct index_cache {
   u_generate_func generate;
   unsigned gen_nr;

   /* If non-null, this buffer is filled by calling generate(nr, map(buffer))
    */
   struct pipe_resource *buffer;
};


/** Max number of primitives per draw call */
#define QSZ SVGA3D_MAX_DRAW_PRIMITIVE_RANGES

struct draw_cmd {
   struct svga_winsys_context *swc;

   /* vertex layout info */
   SVGA3dVertexDecl vdecl[SVGA3D_INPUTREG_MAX];
   unsigned vdecl_count;
   SVGA3dElementLayoutId vdecl_layout_id;
   unsigned vdecl_buffer_index[SVGA3D_INPUTREG_MAX];

   /* vertex buffer info */
   struct pipe_vertex_buffer vbufs[SVGA3D_INPUTREG_MAX];
   unsigned vbuf_count;

   SVGA3dPrimitiveRange prim[QSZ];
   struct pipe_resource *prim_ib[QSZ];
   unsigned prim_count;   /**< number of primitives for this draw */
   unsigned min_index[QSZ];
   unsigned max_index[QSZ];
};

#define IDX_CACHE_MAX  8

struct svga_hwtnl {
   struct svga_context *svga;
   struct u_upload_mgr *upload_ib;

   /* Additional negative index bias due to partial buffer uploads
    * This is compensated for in the offset associated with all
    * vertex buffers.
    */
   int index_bias;

   /* Provoking vertex information (for flat shading). */
   unsigned api_pv;  /**< app-requested PV mode (PV_FIRST or PV_LAST) */
   unsigned hw_pv;   /**< device-supported PV mode (PV_FIRST or PV_LAST) */

   /* The triangle fillmode for the device (one of PIPE_POLYGON_MODE_{FILL,
    * LINE,POINT}).  If the polygon front mode matches the back mode,
    * api_fillmode will be that mode.  Otherwise, api_fillmode will be
    * PIPE_POLYGON_MODE_FILL.
    */
   unsigned api_fillmode;

   /* Cache the results of running a particular generate func on each
    * primitive type.
    */
   struct index_cache index_cache[MESA_PRIM_COUNT][IDX_CACHE_MAX];

   /* Try to build the maximal draw command packet before emitting:
    */
   struct draw_cmd cmd;
};



/**
 * Do we need to use the gallium 'indices' helper to render unfilled
 * triangles?
 */
static inline bool
svga_need_unfilled_fallback(const struct svga_hwtnl *hwtnl,
                            enum mesa_prim prim)
{
   if (u_reduced_prim(prim) != MESA_PRIM_TRIANGLES) {
      /* if we're drawing points or lines, no fallback needed */
      return false;
   }

   if ((prim == MESA_PRIM_QUADS ||
        prim == MESA_PRIM_QUAD_STRIP ||
        prim == MESA_PRIM_POLYGON) &&
       hwtnl->api_fillmode == PIPE_POLYGON_MODE_LINE) {
      /* We can't directly render quads or polygons.  They're
       * converted to triangles.  If we let the device draw the triangle
       * outlines we'll get an extra, stray lines in the interiors.
       * So, to draw unfilled quads correctly, we need the fallback.
       */
      return true;
   }
   return false;
}


enum pipe_error
svga_hwtnl_prim(struct svga_hwtnl *hwtnl,
                const SVGA3dPrimitiveRange *range,
                unsigned vcount,
                unsigned min_index,
                unsigned max_index,
                struct pipe_resource *ib,
                unsigned start_instance, unsigned instance_count,
                const struct pipe_draw_indirect_info *indirect,
                const struct pipe_stream_output_target *so_vertex_count);

enum pipe_error
svga_hwtnl_simple_draw_range_elements(struct svga_hwtnl *hwtnl,
                                      struct pipe_resource *indexBuffer,
                                      unsigned index_size,
                                      int index_bias,
                                      unsigned min_index,
                                      unsigned max_index,
                                      enum mesa_prim prim,
                                      unsigned start,
                                      unsigned count,
                                      unsigned start_instance,
                                      unsigned instance_count,
                                      uint8_t vertices_per_patch);

#endif
