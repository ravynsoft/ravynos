/*
 Copyright (C) Intel Corp.  2006.  All Rights Reserved.
 Intel funded Tungsten Graphics to
 develop this 3D driver.

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice (including the
 next paragraph) shall be included in all copies or substantial
 portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 **********************************************************************/
 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  */

#ifndef BRW_CLIP_H
#define BRW_CLIP_H

#include "brw_compiler.h"
#include "brw_eu.h"

/* Initial 3 verts, plus at most 6 additional verts from intersections
 * with fixed planes, plus at most 8 additional verts from intersections
 * with user clip planes
 */
#define MAX_VERTS (3+6+8)

#define PRIM_MASK  (0x1f)

struct brw_clip_compile {
   struct brw_codegen func;
   struct brw_clip_prog_key key;
   struct brw_clip_prog_data prog_data;

   struct {
      struct brw_reg R0;
      struct brw_reg vertex[MAX_VERTS];

      struct brw_reg t;
      struct brw_reg t0, t1;
      struct brw_reg dp0, dp1;

      struct brw_reg dpPrev;
      struct brw_reg dp;
      struct brw_reg loopcount;
      struct brw_reg nr_verts;
      struct brw_reg planemask;

      struct brw_reg inlist;
      struct brw_reg outlist;
      struct brw_reg freelist;

      struct brw_reg dir;
      struct brw_reg tmp0, tmp1;
      struct brw_reg offset;

      struct brw_reg fixed_planes;
      struct brw_reg plane_equation;

      struct brw_reg ff_sync;

      /* Bitmask indicating which coordinate attribute should be used for
       * comparison to each clipping plane. A 0 indicates that VARYING_SLOT_POS
       * should be used, because it's one of the fixed +/- x/y/z planes that
       * constitute the bounds of the view volume. A 1 indicates that
       * VARYING_SLOT_CLIP_VERTEX should be used (if available) since it's a user-
       * defined clipping plane.
       */
      struct brw_reg vertex_src_mask;

      /* Offset into the vertex of the current plane's clipdistance value */
      struct brw_reg clipdistance_offset;
   } reg;

   /* Number of registers storing VUE data */
   GLuint nr_regs;

   GLuint first_tmp;
   GLuint last_tmp;

   bool need_direction;

   struct brw_vue_map vue_map;
};

/**
 * True if the given varying is one of the outputs of the vertex shader.
 */
static inline bool brw_clip_have_varying(struct brw_clip_compile *c,
                                         GLuint varying)
{
   return (c->key.attrs & BITFIELD64_BIT(varying)) ? 1 : 0;
}

/* Points are only culled, so no need for a clip routine, however it
 * works out easier to have a dummy one.
 */
void brw_emit_unfilled_clip( struct brw_clip_compile *c );
void brw_emit_tri_clip( struct brw_clip_compile *c );
void brw_emit_line_clip( struct brw_clip_compile *c );
void brw_emit_point_clip( struct brw_clip_compile *c );

/* brw_clip_tri.c, for use by the unfilled clip routine:
 */
void brw_clip_tri_init_vertices( struct brw_clip_compile *c );
void brw_clip_tri_flat_shade( struct brw_clip_compile *c );
void brw_clip_tri( struct brw_clip_compile *c );
void brw_clip_tri_emit_polygon( struct brw_clip_compile *c );
void brw_clip_tri_alloc_regs( struct brw_clip_compile *c,
			      GLuint nr_verts );


/* Utils:
 */

void brw_clip_interp_vertex( struct brw_clip_compile *c,
			     struct brw_indirect dest_ptr,
			     struct brw_indirect v0_ptr, /* from */
			     struct brw_indirect v1_ptr, /* to */
			     struct brw_reg t0,
			     bool force_edgeflag );

void brw_clip_init_planes( struct brw_clip_compile *c );

void brw_clip_emit_vue(struct brw_clip_compile *c,
		       struct brw_indirect vert,
                       enum brw_urb_write_flags flags,
		       GLuint header);

void brw_clip_kill_thread(struct brw_clip_compile *c);

struct brw_reg brw_clip_plane_stride( struct brw_clip_compile *c );
struct brw_reg brw_clip_plane0_address( struct brw_clip_compile *c );

void brw_clip_copy_flatshaded_attributes( struct brw_clip_compile *c,
                                          GLuint to, GLuint from );

void brw_clip_init_clipmask( struct brw_clip_compile *c );

struct brw_reg get_tmp( struct brw_clip_compile *c );

void brw_clip_project_position(struct brw_clip_compile *c,
             struct brw_reg pos );
void brw_clip_ff_sync(struct brw_clip_compile *c);
void brw_clip_init_ff_sync(struct brw_clip_compile *c);

#endif
