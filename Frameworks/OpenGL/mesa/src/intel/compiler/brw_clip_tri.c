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

static void release_tmps( struct brw_clip_compile *c )
{
   c->last_tmp = c->first_tmp;
}


void brw_clip_tri_alloc_regs( struct brw_clip_compile *c,
			      GLuint nr_verts )
{
   const struct intel_device_info *devinfo = c->func.devinfo;
   GLuint i = 0,j;

   /* Register usage is static, precompute here:
    */
   c->reg.R0 = retype(brw_vec8_grf(i, 0), BRW_REGISTER_TYPE_UD); i++;

   if (c->key.nr_userclip) {
      c->reg.fixed_planes = brw_vec4_grf(i, 0);
      i += (6 + c->key.nr_userclip + 1) / 2;

      c->prog_data.curb_read_length = (6 + c->key.nr_userclip + 1) / 2;
   }
   else
      c->prog_data.curb_read_length = 0;


   /* Payload vertices plus space for more generated vertices:
    */
   for (j = 0; j < nr_verts; j++) {
      c->reg.vertex[j] = brw_vec4_grf(i, 0);
      i += c->nr_regs;
   }

   if (c->vue_map.num_slots % 2 && nr_verts > 0) {
      /* The VUE has an odd number of slots so the last register is only half
       * used.  Fill the second half with zero.
       */
      for (j = 0; j < 3; j++) {
	 GLuint delta = brw_vue_slot_to_offset(c->vue_map.num_slots);

	 brw_MOV(&c->func, byte_offset(c->reg.vertex[j], delta), brw_imm_f(0));
      }
   }

   c->reg.t          = brw_vec1_grf(i, 0);
   c->reg.loopcount  = retype(brw_vec1_grf(i, 1), BRW_REGISTER_TYPE_D);
   c->reg.nr_verts   = retype(brw_vec1_grf(i, 2), BRW_REGISTER_TYPE_UD);
   c->reg.planemask  = retype(brw_vec1_grf(i, 3), BRW_REGISTER_TYPE_UD);
   c->reg.plane_equation = brw_vec4_grf(i, 4);
   i++;

   c->reg.dpPrev     = brw_vec1_grf(i, 0); /* fixme - dp4 will clobber r.1,2,3 */
   c->reg.dp         = brw_vec1_grf(i, 4);
   i++;

   c->reg.inlist     = brw_uw16_reg(BRW_GENERAL_REGISTER_FILE, i, 0);
   i++;

   c->reg.outlist    = brw_uw16_reg(BRW_GENERAL_REGISTER_FILE, i, 0);
   i++;

   c->reg.freelist   = brw_uw16_reg(BRW_GENERAL_REGISTER_FILE, i, 0);
   i++;

   if (!c->key.nr_userclip) {
      c->reg.fixed_planes = brw_vec8_grf(i, 0);
      i++;
   }

   if (c->key.do_unfilled) {
      c->reg.dir     = brw_vec4_grf(i, 0);
      c->reg.offset  = brw_vec4_grf(i, 4);
      i++;
      c->reg.tmp0    = brw_vec4_grf(i, 0);
      c->reg.tmp1    = brw_vec4_grf(i, 4);
      i++;
   }

   c->reg.vertex_src_mask = retype(brw_vec1_grf(i, 0), BRW_REGISTER_TYPE_UD);
   c->reg.clipdistance_offset = retype(brw_vec1_grf(i, 1), BRW_REGISTER_TYPE_W);
   i++;

   if (devinfo->ver == 5) {
      c->reg.ff_sync = retype(brw_vec1_grf(i, 0), BRW_REGISTER_TYPE_UD);
      i++;
   }

   c->first_tmp = i;
   c->last_tmp = i;

   c->prog_data.urb_read_length = c->nr_regs; /* ? */
   c->prog_data.total_grf = i;
}



void brw_clip_tri_init_vertices( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;
   struct brw_reg tmp0 = c->reg.loopcount; /* handy temporary */

   /* Initial list of indices for incoming vertices:
    */
   brw_AND(p, tmp0, get_element_ud(c->reg.R0, 2), brw_imm_ud(PRIM_MASK));
   brw_CMP(p,
	   vec1(brw_null_reg()),
	   BRW_CONDITIONAL_EQ,
	   tmp0,
	   brw_imm_ud(_3DPRIM_TRISTRIP_REVERSE));

   /* XXX: Is there an easier way to do this?  Need to reverse every
    * second tristrip element:  Can ignore sometimes?
    */
   brw_IF(p, BRW_EXECUTE_1);
   {
      brw_MOV(p, get_element(c->reg.inlist, 0),  brw_address(c->reg.vertex[1]) );
      brw_MOV(p, get_element(c->reg.inlist, 1),  brw_address(c->reg.vertex[0]) );
      if (c->need_direction)
	 brw_MOV(p, c->reg.dir, brw_imm_f(-1));
   }
   brw_ELSE(p);
   {
      brw_MOV(p, get_element(c->reg.inlist, 0),  brw_address(c->reg.vertex[0]) );
      brw_MOV(p, get_element(c->reg.inlist, 1),  brw_address(c->reg.vertex[1]) );
      if (c->need_direction)
	 brw_MOV(p, c->reg.dir, brw_imm_f(1));
   }
   brw_ENDIF(p);

   brw_MOV(p, get_element(c->reg.inlist, 2),  brw_address(c->reg.vertex[2]) );
   brw_MOV(p, brw_vec8_grf(c->reg.outlist.nr, 0), brw_imm_f(0));
   brw_MOV(p, c->reg.nr_verts, brw_imm_ud(3));
}



void brw_clip_tri_flat_shade( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;
   struct brw_reg tmp0 = c->reg.loopcount; /* handy temporary */

   brw_AND(p, tmp0, get_element_ud(c->reg.R0, 2), brw_imm_ud(PRIM_MASK));
   brw_CMP(p,
	   vec1(brw_null_reg()),
	   BRW_CONDITIONAL_EQ,
	   tmp0,
	   brw_imm_ud(_3DPRIM_POLYGON));

   brw_IF(p, BRW_EXECUTE_1);
   {
      brw_clip_copy_flatshaded_attributes(c, 1, 0);
      brw_clip_copy_flatshaded_attributes(c, 2, 0);
   }
   brw_ELSE(p);
   {
      if (c->key.pv_first) {
	 brw_CMP(p,
		 vec1(brw_null_reg()),
		 BRW_CONDITIONAL_EQ,
		 tmp0,
		 brw_imm_ud(_3DPRIM_TRIFAN));
	 brw_IF(p, BRW_EXECUTE_1);
	 {
	    brw_clip_copy_flatshaded_attributes(c, 0, 1);
	    brw_clip_copy_flatshaded_attributes(c, 2, 1);
	 }
	 brw_ELSE(p);
	 {
	    brw_clip_copy_flatshaded_attributes(c, 1, 0);
	    brw_clip_copy_flatshaded_attributes(c, 2, 0);
	 }
	 brw_ENDIF(p);
      }
      else {
         brw_clip_copy_flatshaded_attributes(c, 0, 2);
         brw_clip_copy_flatshaded_attributes(c, 1, 2);
      }
   }
   brw_ENDIF(p);
}


/**
 * Loads the clip distance for a vertex into `dst`, and ends with
 * a comparison of it to zero with the condition `cond`.
 *
 * - If using a fixed plane, the distance is dot(hpos, plane).
 * - If using a user clip plane, the distance is directly available in the vertex.
 */
static inline void
load_clip_distance(struct brw_clip_compile *c, struct brw_indirect vtx,
                struct brw_reg dst, GLuint hpos_offset, int cond)
{
   struct brw_codegen *p = &c->func;

   dst = vec4(dst);
   brw_AND(p, vec1(brw_null_reg()), c->reg.vertex_src_mask, brw_imm_ud(1));
   brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);
   brw_IF(p, BRW_EXECUTE_1);
   {
      struct brw_indirect temp_ptr = brw_indirect(7, 0);
      brw_ADD(p, get_addr_reg(temp_ptr), get_addr_reg(vtx), c->reg.clipdistance_offset);
      brw_MOV(p, vec1(dst), deref_1f(temp_ptr, 0));
   }
   brw_ELSE(p);
   {
      brw_MOV(p, dst, deref_4f(vtx, hpos_offset));
      brw_DP4(p, dst, dst, c->reg.plane_equation);
   }
   brw_ENDIF(p);

   brw_CMP(p, brw_null_reg(), cond, vec1(dst), brw_imm_f(0.0f));
}


/* Use mesa's clipping algorithms, translated to GFX4 assembly.
 */
void brw_clip_tri( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;
   struct brw_indirect vtx = brw_indirect(0, 0);
   struct brw_indirect vtxPrev = brw_indirect(1, 0);
   struct brw_indirect vtxOut = brw_indirect(2, 0);
   struct brw_indirect plane_ptr = brw_indirect(3, 0);
   struct brw_indirect inlist_ptr = brw_indirect(4, 0);
   struct brw_indirect outlist_ptr = brw_indirect(5, 0);
   struct brw_indirect freelist_ptr = brw_indirect(6, 0);
   GLuint hpos_offset = brw_varying_to_offset(&c->vue_map, VARYING_SLOT_POS);
   GLint clipdist0_offset = c->key.nr_userclip
      ? brw_varying_to_offset(&c->vue_map, VARYING_SLOT_CLIP_DIST0)
      : 0;

   brw_MOV(p, get_addr_reg(vtxPrev),     brw_address(c->reg.vertex[2]) );
   brw_MOV(p, get_addr_reg(plane_ptr),   brw_clip_plane0_address(c));
   brw_MOV(p, get_addr_reg(inlist_ptr),  brw_address(c->reg.inlist));
   brw_MOV(p, get_addr_reg(outlist_ptr), brw_address(c->reg.outlist));

   brw_MOV(p, get_addr_reg(freelist_ptr), brw_address(c->reg.vertex[3]) );

   /* Set the initial vertex source mask: The first 6 planes are the bounds
    * of the view volume; the next 8 planes are the user clipping planes.
    */
   brw_MOV(p, c->reg.vertex_src_mask, brw_imm_ud(0x3fc0));

   /* Set the initial clipdistance offset to be 6 floats before gl_ClipDistance[0].
    * We'll increment 6 times before we start hitting actual user clipping. */
   brw_MOV(p, c->reg.clipdistance_offset, brw_imm_d(clipdist0_offset - 6*sizeof(float)));

   brw_DO(p, BRW_EXECUTE_1);
   {
      /* if (planemask & 1)
       */
      brw_AND(p, vec1(brw_null_reg()), c->reg.planemask, brw_imm_ud(1));
      brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);

      brw_IF(p, BRW_EXECUTE_1);
      {
	 /* vtxOut = freelist_ptr++
	  */
	 brw_MOV(p, get_addr_reg(vtxOut),       get_addr_reg(freelist_ptr) );
	 brw_ADD(p, get_addr_reg(freelist_ptr), get_addr_reg(freelist_ptr), brw_imm_uw(c->nr_regs * REG_SIZE));

	 if (c->key.nr_userclip)
	    brw_MOV(p, c->reg.plane_equation, deref_4f(plane_ptr, 0));
	 else
	    brw_MOV(p, c->reg.plane_equation, deref_4b(plane_ptr, 0));

	 brw_MOV(p, c->reg.loopcount, c->reg.nr_verts);
	 brw_MOV(p, c->reg.nr_verts, brw_imm_ud(0));

	 brw_DO(p, BRW_EXECUTE_1);
	 {
	    /* vtx = *input_ptr;
	     */
	    brw_MOV(p, get_addr_reg(vtx), deref_1uw(inlist_ptr, 0));

            load_clip_distance(c, vtxPrev, c->reg.dpPrev, hpos_offset, BRW_CONDITIONAL_L);
	    /* (prev < 0.0f) */
	    brw_IF(p, BRW_EXECUTE_1);
	    {
               load_clip_distance(c, vtx, c->reg.dp, hpos_offset, BRW_CONDITIONAL_GE);
	       /* IS_POSITIVE(next)
		*/
	       brw_IF(p, BRW_EXECUTE_1);
	       {

		  /* Coming back in.
		   */
		  brw_ADD(p, c->reg.t, c->reg.dpPrev, negate(c->reg.dp));
		  brw_math_invert(p, c->reg.t, c->reg.t);
		  brw_MUL(p, c->reg.t, c->reg.t, c->reg.dpPrev);

		  /* If (vtxOut == 0) vtxOut = vtxPrev
		   */
		  brw_CMP(p, vec1(brw_null_reg()), BRW_CONDITIONAL_EQ, get_addr_reg(vtxOut), brw_imm_uw(0) );
                  brw_MOV(p, get_addr_reg(vtxOut), get_addr_reg(vtxPrev));
                  brw_inst_set_pred_control(p->devinfo, brw_last_inst,
                                            BRW_PREDICATE_NORMAL);

		  brw_clip_interp_vertex(c, vtxOut, vtxPrev, vtx, c->reg.t, false);

		  /* *outlist_ptr++ = vtxOut;
		   * nr_verts++;
		   * vtxOut = 0;
		   */
		  brw_MOV(p, deref_1uw(outlist_ptr, 0), get_addr_reg(vtxOut));
		  brw_ADD(p, get_addr_reg(outlist_ptr), get_addr_reg(outlist_ptr), brw_imm_uw(sizeof(short)));
		  brw_ADD(p, c->reg.nr_verts, c->reg.nr_verts, brw_imm_ud(1));
		  brw_MOV(p, get_addr_reg(vtxOut), brw_imm_uw(0) );
	       }
	       brw_ENDIF(p);

	    }
	    brw_ELSE(p);
	    {
	       /* *outlist_ptr++ = vtxPrev;
		* nr_verts++;
		*/
	       brw_MOV(p, deref_1uw(outlist_ptr, 0), get_addr_reg(vtxPrev));
	       brw_ADD(p, get_addr_reg(outlist_ptr), get_addr_reg(outlist_ptr), brw_imm_uw(sizeof(short)));
	       brw_ADD(p, c->reg.nr_verts, c->reg.nr_verts, brw_imm_ud(1));

               load_clip_distance(c, vtx, c->reg.dp, hpos_offset, BRW_CONDITIONAL_L);
	       /* (next < 0.0f)
		*/
	       brw_IF(p, BRW_EXECUTE_1);
	       {
		  /* Going out of bounds.  Avoid division by zero as we
		   * know dp != dpPrev from DIFFERENT_SIGNS, above.
		   */
		  brw_ADD(p, c->reg.t, c->reg.dp, negate(c->reg.dpPrev));
		  brw_math_invert(p, c->reg.t, c->reg.t);
		  brw_MUL(p, c->reg.t, c->reg.t, c->reg.dp);

		  /* If (vtxOut == 0) vtxOut = vtx
		   */
		  brw_CMP(p, vec1(brw_null_reg()), BRW_CONDITIONAL_EQ, get_addr_reg(vtxOut), brw_imm_uw(0) );
                  brw_MOV(p, get_addr_reg(vtxOut), get_addr_reg(vtx));
                  brw_inst_set_pred_control(p->devinfo, brw_last_inst,
                                            BRW_PREDICATE_NORMAL);

		  brw_clip_interp_vertex(c, vtxOut, vtx, vtxPrev, c->reg.t, true);

		  /* *outlist_ptr++ = vtxOut;
		   * nr_verts++;
		   * vtxOut = 0;
		   */
		  brw_MOV(p, deref_1uw(outlist_ptr, 0), get_addr_reg(vtxOut));
		  brw_ADD(p, get_addr_reg(outlist_ptr), get_addr_reg(outlist_ptr), brw_imm_uw(sizeof(short)));
		  brw_ADD(p, c->reg.nr_verts, c->reg.nr_verts, brw_imm_ud(1));
		  brw_MOV(p, get_addr_reg(vtxOut), brw_imm_uw(0) );
	       }
	       brw_ENDIF(p);
	    }
	    brw_ENDIF(p);

	    /* vtxPrev = vtx;
	     * inlist_ptr++;
	     */
	    brw_MOV(p, get_addr_reg(vtxPrev), get_addr_reg(vtx));
	    brw_ADD(p, get_addr_reg(inlist_ptr), get_addr_reg(inlist_ptr), brw_imm_uw(sizeof(short)));

	    /* while (--loopcount != 0)
	     */
	    brw_ADD(p, c->reg.loopcount, c->reg.loopcount, brw_imm_d(-1));
            brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);
	 }
	 brw_WHILE(p);
         brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);

	 /* vtxPrev = *(outlist_ptr-1)  OR: outlist[nr_verts-1]
	  * inlist = outlist
	  * inlist_ptr = &inlist[0]
	  * outlist_ptr = &outlist[0]
	  */
	 brw_ADD(p, get_addr_reg(outlist_ptr), get_addr_reg(outlist_ptr), brw_imm_w(-2));
	 brw_MOV(p, get_addr_reg(vtxPrev), deref_1uw(outlist_ptr, 0));
	 brw_MOV(p, brw_vec8_grf(c->reg.inlist.nr, 0), brw_vec8_grf(c->reg.outlist.nr, 0));
	 brw_MOV(p, get_addr_reg(inlist_ptr), brw_address(c->reg.inlist));
	 brw_MOV(p, get_addr_reg(outlist_ptr), brw_address(c->reg.outlist));
      }
      brw_ENDIF(p);

      /* plane_ptr++;
       */
      brw_ADD(p, get_addr_reg(plane_ptr), get_addr_reg(plane_ptr), brw_clip_plane_stride(c));

      /* nr_verts >= 3
       */
      brw_CMP(p,
	      vec1(brw_null_reg()),
	      BRW_CONDITIONAL_GE,
	      c->reg.nr_verts,
	      brw_imm_ud(3));
      brw_set_default_predicate_control(p, BRW_PREDICATE_NORMAL);

      /* && (planemask>>=1) != 0
       */
      brw_SHR(p, c->reg.planemask, c->reg.planemask, brw_imm_ud(1));
      brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);
      brw_SHR(p, c->reg.vertex_src_mask, c->reg.vertex_src_mask, brw_imm_ud(1));
      brw_ADD(p, c->reg.clipdistance_offset, c->reg.clipdistance_offset, brw_imm_w(sizeof(float)));
   }
   brw_WHILE(p);
   brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
}



void brw_clip_tri_emit_polygon(struct brw_clip_compile *c)
{
   struct brw_codegen *p = &c->func;

   /* for (loopcount = nr_verts-2; loopcount > 0; loopcount--)
    */
   brw_ADD(p,
	   c->reg.loopcount,
	   c->reg.nr_verts,
	   brw_imm_d(-2));
   brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_G);

   brw_IF(p, BRW_EXECUTE_1);
   {
      struct brw_indirect v0 = brw_indirect(0, 0);
      struct brw_indirect vptr = brw_indirect(1, 0);

      brw_MOV(p, get_addr_reg(vptr), brw_address(c->reg.inlist));
      brw_MOV(p, get_addr_reg(v0), deref_1uw(vptr, 0));

      brw_clip_emit_vue(c, v0, BRW_URB_WRITE_ALLOCATE_COMPLETE,
                        ((_3DPRIM_TRIFAN << URB_WRITE_PRIM_TYPE_SHIFT)
                         | URB_WRITE_PRIM_START));

      brw_ADD(p, get_addr_reg(vptr), get_addr_reg(vptr), brw_imm_uw(2));
      brw_MOV(p, get_addr_reg(v0), deref_1uw(vptr, 0));

      brw_DO(p, BRW_EXECUTE_1);
      {
	 brw_clip_emit_vue(c, v0, BRW_URB_WRITE_ALLOCATE_COMPLETE,
                           (_3DPRIM_TRIFAN << URB_WRITE_PRIM_TYPE_SHIFT));

	 brw_ADD(p, get_addr_reg(vptr), get_addr_reg(vptr), brw_imm_uw(2));
	 brw_MOV(p, get_addr_reg(v0), deref_1uw(vptr, 0));

	 brw_ADD(p, c->reg.loopcount, c->reg.loopcount, brw_imm_d(-1));
         brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);
      }
      brw_WHILE(p);
      brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);

      brw_clip_emit_vue(c, v0, BRW_URB_WRITE_EOT_COMPLETE,
                        ((_3DPRIM_TRIFAN << URB_WRITE_PRIM_TYPE_SHIFT)
                         | URB_WRITE_PRIM_END));
   }
   brw_ENDIF(p);
}

static void do_clip_tri( struct brw_clip_compile *c )
{
   brw_clip_init_planes(c);

   brw_clip_tri(c);
}


static void maybe_do_clip_tri( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;

   brw_CMP(p, vec1(brw_null_reg()), BRW_CONDITIONAL_NZ, c->reg.planemask, brw_imm_ud(0));
   brw_IF(p, BRW_EXECUTE_1);
   {
      do_clip_tri(c);
   }
   brw_ENDIF(p);
}

static void brw_clip_test( struct brw_clip_compile *c )
{
    struct brw_reg t = retype(get_tmp(c), BRW_REGISTER_TYPE_UD);
    struct brw_reg t1 = retype(get_tmp(c), BRW_REGISTER_TYPE_UD);
    struct brw_reg t2 = retype(get_tmp(c), BRW_REGISTER_TYPE_UD);
    struct brw_reg t3 = retype(get_tmp(c), BRW_REGISTER_TYPE_UD);

    struct brw_reg v0 = get_tmp(c);
    struct brw_reg v1 = get_tmp(c);
    struct brw_reg v2 = get_tmp(c);

    struct brw_indirect vt0 = brw_indirect(0, 0);
    struct brw_indirect vt1 = brw_indirect(1, 0);
    struct brw_indirect vt2 = brw_indirect(2, 0);

    struct brw_codegen *p = &c->func;
    struct brw_reg tmp0 = c->reg.loopcount; /* handy temporary */

    GLuint hpos_offset = brw_varying_to_offset(&c->vue_map,
                                                   VARYING_SLOT_POS);

    brw_MOV(p, get_addr_reg(vt0), brw_address(c->reg.vertex[0]));
    brw_MOV(p, get_addr_reg(vt1), brw_address(c->reg.vertex[1]));
    brw_MOV(p, get_addr_reg(vt2), brw_address(c->reg.vertex[2]));
    brw_MOV(p, v0, deref_4f(vt0, hpos_offset));
    brw_MOV(p, v1, deref_4f(vt1, hpos_offset));
    brw_MOV(p, v2, deref_4f(vt2, hpos_offset));
    brw_AND(p, c->reg.planemask, c->reg.planemask, brw_imm_ud(~0x3f));

    /* test nearz, xmin, ymin plane */
    /* clip.xyz < -clip.w */
    brw_CMP(p, t1, BRW_CONDITIONAL_L, v0, negate(get_element(v0, 3)));
    brw_CMP(p, t2, BRW_CONDITIONAL_L, v1, negate(get_element(v1, 3)));
    brw_CMP(p, t3, BRW_CONDITIONAL_L, v2, negate(get_element(v2, 3)));

    /* All vertices are outside of a plane, rejected */
    brw_AND(p, t, t1, t2);
    brw_AND(p, t, t, t3);
    brw_OR(p, tmp0, get_element(t, 0), get_element(t, 1));
    brw_OR(p, tmp0, tmp0, get_element(t, 2));
    brw_AND(p, brw_null_reg(), tmp0, brw_imm_ud(0x1));
    brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);
    brw_IF(p, BRW_EXECUTE_1);
    {
        brw_clip_kill_thread(c);
    }
    brw_ENDIF(p);
    brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);

    /* some vertices are inside a plane, some are outside,need to clip */
    brw_XOR(p, t, t1, t2);
    brw_XOR(p, t1, t2, t3);
    brw_OR(p, t, t, t1);
    brw_AND(p, t, t, brw_imm_ud(0x1));
    brw_CMP(p, brw_null_reg(), BRW_CONDITIONAL_NZ,
            get_element(t, 0), brw_imm_ud(0));
    brw_OR(p, c->reg.planemask, c->reg.planemask, brw_imm_ud((1<<5)));
    brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);
    brw_CMP(p, brw_null_reg(), BRW_CONDITIONAL_NZ,
            get_element(t, 1), brw_imm_ud(0));
    brw_OR(p, c->reg.planemask, c->reg.planemask, brw_imm_ud((1<<3)));
    brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);
    brw_CMP(p, brw_null_reg(), BRW_CONDITIONAL_NZ,
            get_element(t, 2), brw_imm_ud(0));
    brw_OR(p, c->reg.planemask, c->reg.planemask, brw_imm_ud((1<<1)));
    brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);

    /* test farz, xmax, ymax plane */
    /* clip.xyz > clip.w */
    brw_CMP(p, t1, BRW_CONDITIONAL_G, v0, get_element(v0, 3));
    brw_CMP(p, t2, BRW_CONDITIONAL_G, v1, get_element(v1, 3));
    brw_CMP(p, t3, BRW_CONDITIONAL_G, v2, get_element(v2, 3));

    /* All vertices are outside of a plane, rejected */
    brw_AND(p, t, t1, t2);
    brw_AND(p, t, t, t3);
    brw_OR(p, tmp0, get_element(t, 0), get_element(t, 1));
    brw_OR(p, tmp0, tmp0, get_element(t, 2));
    brw_AND(p, brw_null_reg(), tmp0, brw_imm_ud(0x1));
    brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);
    brw_IF(p, BRW_EXECUTE_1);
    {
        brw_clip_kill_thread(c);
    }
    brw_ENDIF(p);
    brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);

    /* some vertices are inside a plane, some are outside,need to clip */
    brw_XOR(p, t, t1, t2);
    brw_XOR(p, t1, t2, t3);
    brw_OR(p, t, t, t1);
    brw_AND(p, t, t, brw_imm_ud(0x1));
    brw_CMP(p, brw_null_reg(), BRW_CONDITIONAL_NZ,
            get_element(t, 0), brw_imm_ud(0));
    brw_OR(p, c->reg.planemask, c->reg.planemask, brw_imm_ud((1<<4)));
    brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);
    brw_CMP(p, brw_null_reg(), BRW_CONDITIONAL_NZ,
            get_element(t, 1), brw_imm_ud(0));
    brw_OR(p, c->reg.planemask, c->reg.planemask, brw_imm_ud((1<<2)));
    brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);
    brw_CMP(p, brw_null_reg(), BRW_CONDITIONAL_NZ,
            get_element(t, 2), brw_imm_ud(0));
    brw_OR(p, c->reg.planemask, c->reg.planemask, brw_imm_ud((1<<0)));
    brw_inst_set_pred_control(p->devinfo, brw_last_inst, BRW_PREDICATE_NORMAL);

    release_tmps(c);
}


void brw_emit_tri_clip( struct brw_clip_compile *c )
{
   struct brw_codegen *p = &c->func;
   brw_clip_tri_alloc_regs(c, 3 + c->key.nr_userclip + 6);
   brw_clip_tri_init_vertices(c);
   brw_clip_init_clipmask(c);
   brw_clip_init_ff_sync(c);

   /* if -ve rhw workaround bit is set,
      do cliptest */
   if (p->devinfo->has_negative_rhw_bug) {
      brw_AND(p, brw_null_reg(), get_element_ud(c->reg.R0, 2),
              brw_imm_ud(1<<20));
      brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);
      brw_IF(p, BRW_EXECUTE_1);
      {
         brw_clip_test(c);
      }
      brw_ENDIF(p);
   }
   /* Can't push into do_clip_tri because with polygon (or quad)
    * flatshading, need to apply the flatshade here because we don't
    * respect the PV when converting to trifan for emit:
    */
   if (c->key.contains_flat_varying)
      brw_clip_tri_flat_shade(c);

   if ((c->key.clip_mode == BRW_CLIP_MODE_NORMAL) ||
       (c->key.clip_mode == BRW_CLIP_MODE_KERNEL_CLIP))
      do_clip_tri(c);
   else
      maybe_do_clip_tri(c);

   brw_clip_tri_emit_polygon(c);

   /* Send an empty message to kill the thread:
    */
   brw_clip_kill_thread(c);
}
