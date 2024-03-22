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

#include "brw_clip.h"
#include "brw_prim.h"


/* This is performed against the original triangles, so no indirection
 * required:
BZZZT!
 */
static void compute_tri_direction( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;
   struct brw_reg e = c->reg.tmp0;
   struct brw_reg f = c->reg.tmp1;
   GLuint hpos_offset = brw_varying_to_offset(&c->vue_map, VARYING_SLOT_POS);
   struct brw_reg v0 = byte_offset(c->reg.vertex[0], hpos_offset);
   struct brw_reg v1 = byte_offset(c->reg.vertex[1], hpos_offset);
   struct brw_reg v2 = byte_offset(c->reg.vertex[2], hpos_offset);


   struct brw_reg v0n = get_tmp(c);
   struct brw_reg v1n = get_tmp(c);
   struct brw_reg v2n = get_tmp(c);

   /* Convert to NDC.
    * NOTE: We can't modify the original vertex coordinates,
    * as it may impact further operations.
    * So, we have to keep normalized coordinates in temp registers.
    *
    * TBD-KC
    * Try to optimize unnecessary MOV's.
    */
   brw_MOV(p, v0n, v0);
   brw_MOV(p, v1n, v1);
   brw_MOV(p, v2n, v2);

   brw_clip_project_position(c, v0n);
   brw_clip_project_position(c, v1n);
   brw_clip_project_position(c, v2n);

   /* Calculate the vectors of two edges of the triangle:
    */
   brw_ADD(p, e, v0n, negate(v2n));
   brw_ADD(p, f, v1n, negate(v2n));

   /* Take their crossproduct:
    */
   brw_set_default_access_mode(p, BRW_ALIGN_16);
   brw_MUL(p, vec4(brw_null_reg()), brw_swizzle(e, BRW_SWIZZLE_YZXW),
           brw_swizzle(f, BRW_SWIZZLE_ZXYW));
   brw_MAC(p, vec4(e),  negate(brw_swizzle(e, BRW_SWIZZLE_ZXYW)),
           brw_swizzle(f, BRW_SWIZZLE_YZXW));
   brw_set_default_access_mode(p, BRW_ALIGN_1);

   brw_MUL(p, c->reg.dir, c->reg.dir, vec4(e));
}


static void cull_direction( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;
   GLuint conditional;

   assert (!(c->key.fill_ccw == BRW_CLIP_FILL_MODE_CULL &&
	     c->key.fill_cw == BRW_CLIP_FILL_MODE_CULL));

   if (c->key.fill_ccw == BRW_CLIP_FILL_MODE_CULL)
      conditional = BRW_CONDITIONAL_GE;
   else
      conditional = BRW_CONDITIONAL_L;

   brw_CMP(p,
	   vec1(brw_null_reg()),
	   conditional,
	   get_element(c->reg.dir, 2),
	   brw_imm_f(0));

   brw_IF(p, BRW_EXECUTE_1);
   {
      brw_clip_kill_thread(c);
   }
   brw_ENDIF(p);
}



static void copy_bfc( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;
   GLuint conditional;

   /* Do we have any colors to copy?
    */
   if (!(brw_clip_have_varying(c, VARYING_SLOT_COL0) &&
         brw_clip_have_varying(c, VARYING_SLOT_BFC0)) &&
       !(brw_clip_have_varying(c, VARYING_SLOT_COL1) &&
         brw_clip_have_varying(c, VARYING_SLOT_BFC1)))
      return;

   /* In some weird degenerate cases we can end up testing the
    * direction twice, once for culling and once for bfc copying.  Oh
    * well, that's what you get for setting weird GL state.
    */
   if (c->key.copy_bfc_ccw)
      conditional = BRW_CONDITIONAL_GE;
   else
      conditional = BRW_CONDITIONAL_L;

   brw_CMP(p,
	   vec1(brw_null_reg()),
	   conditional,
	   get_element(c->reg.dir, 2),
	   brw_imm_f(0));

   brw_IF(p, BRW_EXECUTE_1);
   {
      GLuint i;

      for (i = 0; i < 3; i++) {
	 if (brw_clip_have_varying(c, VARYING_SLOT_COL0) &&
             brw_clip_have_varying(c, VARYING_SLOT_BFC0))
	    brw_MOV(p,
		    byte_offset(c->reg.vertex[i],
                                brw_varying_to_offset(&c->vue_map,
                                                      VARYING_SLOT_COL0)),
		    byte_offset(c->reg.vertex[i],
                                brw_varying_to_offset(&c->vue_map,
                                                      VARYING_SLOT_BFC0)));

	 if (brw_clip_have_varying(c, VARYING_SLOT_COL1) &&
             brw_clip_have_varying(c, VARYING_SLOT_BFC1))
	    brw_MOV(p,
		    byte_offset(c->reg.vertex[i],
                                brw_varying_to_offset(&c->vue_map,
                                                      VARYING_SLOT_COL1)),
		    byte_offset(c->reg.vertex[i],
                                brw_varying_to_offset(&c->vue_map,
                                                      VARYING_SLOT_BFC1)));
      }
   }
   brw_ENDIF(p);
}




/*
  GLfloat iz	= 1.0 / dir.z;
  GLfloat ac	= dir.x * iz;
  GLfloat bc	= dir.y * iz;
  offset = ctx->Polygon.OffsetUnits * DEPTH_SCALE;
  offset += MAX2( abs(ac), abs(bc) ) * ctx->Polygon.OffsetFactor;
  if (ctx->Polygon.OffsetClamp && isfinite(ctx->Polygon.OffsetClamp)) {
    if (ctx->Polygon.OffsetClamp < 0)
      offset = MAX2( offset, ctx->Polygon.OffsetClamp );
    else
      offset = MIN2( offset, ctx->Polygon.OffsetClamp );
  }
  offset *= MRD;
*/
static void compute_offset( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;
   struct brw_reg off = c->reg.offset;
   struct brw_reg dir = c->reg.dir;

   brw_math_invert(p, get_element(off, 2), get_element(dir, 2));
   brw_MUL(p, vec2(off), vec2(dir), get_element(off, 2));

   brw_CMP(p,
	   vec1(brw_null_reg()),
	   BRW_CONDITIONAL_GE,
	   brw_abs(get_element(off, 0)),
	   brw_abs(get_element(off, 1)));

   brw_SEL(p, vec1(off),
           brw_abs(get_element(off, 0)), brw_abs(get_element(off, 1)));
   brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);

   brw_MUL(p, vec1(off), vec1(off), brw_imm_f(c->key.offset_factor));
   brw_ADD(p, vec1(off), vec1(off), brw_imm_f(c->key.offset_units));
   if (c->key.offset_clamp && isfinite(c->key.offset_clamp)) {
      brw_CMP(p,
              vec1(brw_null_reg()),
              c->key.offset_clamp < 0 ? BRW_CONDITIONAL_GE : BRW_CONDITIONAL_L,
              vec1(off),
              brw_imm_f(c->key.offset_clamp));
      brw_SEL(p, vec1(off), vec1(off), brw_imm_f(c->key.offset_clamp));
   }
}


static void merge_edgeflags( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;
   struct brw_reg tmp0 = get_element_ud(c->reg.tmp0, 0);

   brw_AND(p, tmp0, get_element_ud(c->reg.R0, 2), brw_imm_ud(PRIM_MASK));
   brw_CMP(p,
	   vec1(brw_null_reg()),
	   BRW_CONDITIONAL_EQ,
	   tmp0,
	   brw_imm_ud(_3DPRIM_POLYGON));

   /* Get away with using reg.vertex because we know that this is not
    * a _3DPRIM_TRISTRIP_REVERSE:
    */
   brw_IF(p, BRW_EXECUTE_1);
   {
      brw_AND(p, vec1(brw_null_reg()), get_element_ud(c->reg.R0, 2), brw_imm_ud(1<<8));
      brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_EQ);
      brw_MOV(p, byte_offset(c->reg.vertex[0],
                             brw_varying_to_offset(&c->vue_map,
                                                   VARYING_SLOT_EDGE)),
              brw_imm_f(0));
      brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);

      brw_AND(p, vec1(brw_null_reg()), get_element_ud(c->reg.R0, 2), brw_imm_ud(1<<9));
      brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_EQ);
      brw_MOV(p, byte_offset(c->reg.vertex[2],
                             brw_varying_to_offset(&c->vue_map,
                                                   VARYING_SLOT_EDGE)),
              brw_imm_f(0));
      brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);
   }
   brw_ENDIF(p);
}



static void apply_one_offset( struct brw_clip_compile *c,
			  struct brw_indirect vert )
{
   struct brw_codegen *p = &c->func;
   GLuint ndc_offset = brw_varying_to_offset(&c->vue_map,
                                             BRW_VARYING_SLOT_NDC);
   struct brw_reg z = deref_1f(vert, ndc_offset +
			       2 * type_sz(BRW_REGISTER_TYPE_F));

   brw_ADD(p, z, z, vec1(c->reg.offset));
}



/***********************************************************************
 * Output clipped polygon as an unfilled primitive:
 */
static void emit_lines(struct brw_clip_compile *c,
		       bool do_offset)
{
   struct brw_codegen *p = &c->func;
   struct brw_indirect v0 = brw_indirect(0, 0);
   struct brw_indirect v1 = brw_indirect(1, 0);
   struct brw_indirect v0ptr = brw_indirect(2, 0);
   struct brw_indirect v1ptr = brw_indirect(3, 0);

   /* Need a separate loop for offset:
    */
   if (do_offset) {
      brw_MOV(p, c->reg.loopcount, c->reg.nr_verts);
      brw_MOV(p, get_addr_reg(v0ptr), brw_address(c->reg.inlist));

      brw_DO(p, BRW_EXECUTE_1);
      {
	 brw_MOV(p, get_addr_reg(v0), deref_1uw(v0ptr, 0));
	 brw_ADD(p, get_addr_reg(v0ptr), get_addr_reg(v0ptr), brw_imm_uw(2));

	 apply_one_offset(c, v0);

	 brw_ADD(p, c->reg.loopcount, c->reg.loopcount, brw_imm_d(-1));
         brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_G);
      }
      brw_WHILE(p);
      brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);
   }

   /* v1ptr = &inlist[nr_verts]
    * *v1ptr = v0
    */
   brw_MOV(p, c->reg.loopcount, c->reg.nr_verts);
   brw_MOV(p, get_addr_reg(v0ptr), brw_address(c->reg.inlist));
   brw_ADD(p, get_addr_reg(v1ptr), get_addr_reg(v0ptr), retype(c->reg.nr_verts, BRW_REGISTER_TYPE_UW));
   brw_ADD(p, get_addr_reg(v1ptr), get_addr_reg(v1ptr), retype(c->reg.nr_verts, BRW_REGISTER_TYPE_UW));
   brw_MOV(p, deref_1uw(v1ptr, 0), deref_1uw(v0ptr, 0));

   brw_DO(p, BRW_EXECUTE_1);
   {
      brw_MOV(p, get_addr_reg(v0), deref_1uw(v0ptr, 0));
      brw_MOV(p, get_addr_reg(v1), deref_1uw(v0ptr, 2));
      brw_ADD(p, get_addr_reg(v0ptr), get_addr_reg(v0ptr), brw_imm_uw(2));

      /* draw edge if edgeflag != 0 */
      brw_CMP(p,
	      vec1(brw_null_reg()), BRW_CONDITIONAL_NZ,
	      deref_1f(v0, brw_varying_to_offset(&c->vue_map,
                                                 VARYING_SLOT_EDGE)),
	      brw_imm_f(0));
      brw_IF(p, BRW_EXECUTE_1);
      {
	 brw_clip_emit_vue(c, v0, BRW_URB_WRITE_ALLOCATE_COMPLETE,
                           (_3DPRIM_LINESTRIP << URB_WRITE_PRIM_TYPE_SHIFT)
                           | URB_WRITE_PRIM_START);
	 brw_clip_emit_vue(c, v1, BRW_URB_WRITE_ALLOCATE_COMPLETE,
                           (_3DPRIM_LINESTRIP << URB_WRITE_PRIM_TYPE_SHIFT)
                           | URB_WRITE_PRIM_END);
      }
      brw_ENDIF(p);

      brw_ADD(p, c->reg.loopcount, c->reg.loopcount, brw_imm_d(-1));
      brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);
   }
   brw_WHILE(p);
   brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);
}



static void emit_points(struct brw_clip_compile *c,
			bool do_offset )
{
   struct brw_codegen *p = &c->func;

   struct brw_indirect v0 = brw_indirect(0, 0);
   struct brw_indirect v0ptr = brw_indirect(2, 0);

   brw_MOV(p, c->reg.loopcount, c->reg.nr_verts);
   brw_MOV(p, get_addr_reg(v0ptr), brw_address(c->reg.inlist));

   brw_DO(p, BRW_EXECUTE_1);
   {
      brw_MOV(p, get_addr_reg(v0), deref_1uw(v0ptr, 0));
      brw_ADD(p, get_addr_reg(v0ptr), get_addr_reg(v0ptr), brw_imm_uw(2));

      /* draw if edgeflag != 0
       */
      brw_CMP(p,
	      vec1(brw_null_reg()), BRW_CONDITIONAL_NZ,
	      deref_1f(v0, brw_varying_to_offset(&c->vue_map,
                                                 VARYING_SLOT_EDGE)),
	      brw_imm_f(0));
      brw_IF(p, BRW_EXECUTE_1);
      {
	 if (do_offset)
	    apply_one_offset(c, v0);

	 brw_clip_emit_vue(c, v0, BRW_URB_WRITE_ALLOCATE_COMPLETE,
                           (_3DPRIM_POINTLIST << URB_WRITE_PRIM_TYPE_SHIFT)
                           | URB_WRITE_PRIM_START | URB_WRITE_PRIM_END);
      }
      brw_ENDIF(p);

      brw_ADD(p, c->reg.loopcount, c->reg.loopcount, brw_imm_d(-1));
      brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);
   }
   brw_WHILE(p);
   brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);
}







static void emit_primitives( struct brw_clip_compile *c,
			     GLuint mode,
			     bool do_offset )
{
   switch (mode) {
   case BRW_CLIP_FILL_MODE_FILL:
      brw_clip_tri_emit_polygon(c);
      break;

   case BRW_CLIP_FILL_MODE_LINE:
      emit_lines(c, do_offset);
      break;

   case BRW_CLIP_FILL_MODE_POINT:
      emit_points(c, do_offset);
      break;

   case BRW_CLIP_FILL_MODE_CULL:
      unreachable("not reached");
   }
}



static void emit_unfilled_primitives( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;

   /* Direction culling has already been done.
    */
   if (c->key.fill_ccw != c->key.fill_cw &&
       c->key.fill_ccw != BRW_CLIP_FILL_MODE_CULL &&
       c->key.fill_cw != BRW_CLIP_FILL_MODE_CULL)
   {
      brw_CMP(p,
	      vec1(brw_null_reg()),
	      BRW_CONDITIONAL_GE,
	      get_element(c->reg.dir, 2),
	      brw_imm_f(0));

      brw_IF(p, BRW_EXECUTE_1);
      {
	 emit_primitives(c, c->key.fill_ccw, c->key.offset_ccw);
      }
      brw_ELSE(p);
      {
	 emit_primitives(c, c->key.fill_cw, c->key.offset_cw);
      }
      brw_ENDIF(p);
   }
   else if (c->key.fill_cw != BRW_CLIP_FILL_MODE_CULL) {
      emit_primitives(c, c->key.fill_cw, c->key.offset_cw);
   }
   else if (c->key.fill_ccw != BRW_CLIP_FILL_MODE_CULL) {
      emit_primitives(c, c->key.fill_ccw, c->key.offset_ccw);
   }
}




static void check_nr_verts( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;

   brw_CMP(p, vec1(brw_null_reg()), BRW_CONDITIONAL_L, c->reg.nr_verts, brw_imm_d(3));
   brw_IF(p, BRW_EXECUTE_1);
   {
      brw_clip_kill_thread(c);
   }
   brw_ENDIF(p);
}


void brw_emit_unfilled_clip( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;

   c->need_direction = ((c->key.offset_ccw || c->key.offset_cw) ||
			(c->key.fill_ccw != c->key.fill_cw) ||
			c->key.fill_ccw == BRW_CLIP_FILL_MODE_CULL ||
			c->key.fill_cw == BRW_CLIP_FILL_MODE_CULL ||
			c->key.copy_bfc_cw ||
			c->key.copy_bfc_ccw);

   brw_clip_tri_alloc_regs(c, 3 + c->key.nr_userclip + 6);
   brw_clip_tri_init_vertices(c);
   brw_clip_init_ff_sync(c);

   assert(brw_clip_have_varying(c, VARYING_SLOT_EDGE));

   if (c->key.fill_ccw == BRW_CLIP_FILL_MODE_CULL &&
       c->key.fill_cw == BRW_CLIP_FILL_MODE_CULL) {
      brw_clip_kill_thread(c);
      return;
   }

   merge_edgeflags(c);

   /* Need to use the inlist indirection here:
    */
   if (c->need_direction)
      compute_tri_direction(c);

   if (c->key.fill_ccw == BRW_CLIP_FILL_MODE_CULL ||
       c->key.fill_cw == BRW_CLIP_FILL_MODE_CULL)
      cull_direction(c);

   if (c->key.offset_ccw ||
       c->key.offset_cw)
      compute_offset(c);

   if (c->key.copy_bfc_ccw ||
       c->key.copy_bfc_cw)
      copy_bfc(c);

   /* Need to do this whether we clip or not:
    */
   if (c->key.contains_flat_varying)
      brw_clip_tri_flat_shade(c);

   brw_clip_init_clipmask(c);
   brw_CMP(p, vec1(brw_null_reg()), BRW_CONDITIONAL_NZ, c->reg.planemask, brw_imm_ud(0));
   brw_IF(p, BRW_EXECUTE_1);
   {
      brw_clip_init_planes(c);
      brw_clip_tri(c);
      check_nr_verts(c);
   }
   brw_ENDIF(p);

   emit_unfilled_primitives(c);
   brw_clip_kill_thread(c);
}
