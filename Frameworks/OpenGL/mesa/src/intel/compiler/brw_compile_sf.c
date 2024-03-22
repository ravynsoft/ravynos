/*
 * Copyright © 2006 - 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "brw_compiler.h"
#include "brw_eu.h"
#include "brw_prim.h"

#include "dev/intel_debug.h"

struct brw_sf_compile {
   struct brw_codegen func;
   struct brw_sf_prog_key key;
   struct brw_sf_prog_data prog_data;

   struct brw_reg pv;
   struct brw_reg det;
   struct brw_reg dx0;
   struct brw_reg dx2;
   struct brw_reg dy0;
   struct brw_reg dy2;

   /* z and 1/w passed in separately:
    */
   struct brw_reg z[3];
   struct brw_reg inv_w[3];

   /* The vertices:
    */
   struct brw_reg vert[3];

    /* Temporaries, allocated after last vertex reg.
    */
   struct brw_reg inv_det;
   struct brw_reg a1_sub_a0;
   struct brw_reg a2_sub_a0;
   struct brw_reg tmp;

   struct brw_reg m1Cx;
   struct brw_reg m2Cy;
   struct brw_reg m3C0;

   GLuint nr_verts;
   GLuint nr_attr_regs;
   GLuint nr_setup_regs;
   int urb_entry_read_offset;

   /** The last known value of the f0.0 flag register. */
   unsigned flag_value;

   struct brw_vue_map vue_map;
};

/**
 * Determine the vue slot corresponding to the given half of the given register.
 */
static inline int vert_reg_to_vue_slot(struct brw_sf_compile *c, GLuint reg,
                                       int half)
{
   return (reg + c->urb_entry_read_offset) * 2 + half;
}

/**
 * Determine the varying corresponding to the given half of the given
 * register.  half=0 means the first half of a register, half=1 means the
 * second half.
 */
static inline int vert_reg_to_varying(struct brw_sf_compile *c, GLuint reg,
                                      int half)
{
   int vue_slot = vert_reg_to_vue_slot(c, reg, half);
   return c->vue_map.slot_to_varying[vue_slot];
}

/**
 * Determine the register corresponding to the given vue slot
 */
static struct brw_reg get_vue_slot(struct brw_sf_compile *c,
                                   struct brw_reg vert,
                                   int vue_slot)
{
   GLuint off = vue_slot / 2 - c->urb_entry_read_offset;
   GLuint sub = vue_slot % 2;

   return brw_vec4_grf(vert.nr + off, sub * 4);
}

/**
 * Determine the register corresponding to the given varying.
 */
static struct brw_reg get_varying(struct brw_sf_compile *c,
                                  struct brw_reg vert,
                                  GLuint varying)
{
   int vue_slot = c->vue_map.varying_to_slot[varying];
   assert (vue_slot >= c->urb_entry_read_offset);
   return get_vue_slot(c, vert, vue_slot);
}

static bool
have_attr(struct brw_sf_compile *c, GLuint attr)
{
   return (c->key.attrs & BITFIELD64_BIT(attr)) ? 1 : 0;
}

/***********************************************************************
 * Twoside lighting
 */
static void copy_bfc( struct brw_sf_compile *c,
		      struct brw_reg vert )
{
   struct brw_codegen *p = &c->func;
   GLuint i;

   for (i = 0; i < 2; i++) {
      if (have_attr(c, VARYING_SLOT_COL0+i) &&
	  have_attr(c, VARYING_SLOT_BFC0+i))
	 brw_MOV(p,
		 get_varying(c, vert, VARYING_SLOT_COL0+i),
		 get_varying(c, vert, VARYING_SLOT_BFC0+i));
   }
}


static void do_twoside_color( struct brw_sf_compile *c )
{
   struct brw_codegen *p = &c->func;
   GLuint backface_conditional = c->key.frontface_ccw ? BRW_CONDITIONAL_G : BRW_CONDITIONAL_L;

   /* Already done in clip program:
    */
   if (c->key.primitive == BRW_SF_PRIM_UNFILLED_TRIS)
      return;

   /* If the vertex shader provides backface color, do the selection. The VS
    * promises to set up the front color if the backface color is provided, but
    * it may contain junk if never written to.
    */
   if (!(have_attr(c, VARYING_SLOT_COL0) && have_attr(c, VARYING_SLOT_BFC0)) &&
       !(have_attr(c, VARYING_SLOT_COL1) && have_attr(c, VARYING_SLOT_BFC1)))
      return;

   /* Need to use BRW_EXECUTE_4 and also do an 4-wide compare in order
    * to get all channels active inside the IF.  In the clipping code
    * we run with NoMask, so it's not an option and we can use
    * BRW_EXECUTE_1 for all comparisons.
    */
   brw_CMP(p, vec4(brw_null_reg()), backface_conditional, c->det, brw_imm_f(0));
   brw_IF(p, BRW_EXECUTE_4);
   {
      switch (c->nr_verts) {
      case 3: copy_bfc(c, c->vert[2]); FALLTHROUGH;
      case 2: copy_bfc(c, c->vert[1]); FALLTHROUGH;
      case 1: copy_bfc(c, c->vert[0]);
      }
   }
   brw_ENDIF(p);
}



/***********************************************************************
 * Flat shading
 */

static void copy_flatshaded_attributes(struct brw_sf_compile *c,
                                       struct brw_reg dst,
                                       struct brw_reg src)
{
   struct brw_codegen *p = &c->func;
   int i;

   for (i = 0; i < c->vue_map.num_slots; i++) {
      if (c->key.interp_mode[i] == INTERP_MODE_FLAT) {
         brw_MOV(p,
                 get_vue_slot(c, dst, i),
                 get_vue_slot(c, src, i));
      }
   }
}

static int count_flatshaded_attributes(struct brw_sf_compile *c)
{
   int i;
   int count = 0;

   for (i = 0; i < c->vue_map.num_slots; i++)
      if (c->key.interp_mode[i] == INTERP_MODE_FLAT)
         count++;

   return count;
}



/* Need to use a computed jump to copy flatshaded attributes as the
 * vertices are ordered according to y-coordinate before reaching this
 * point, so the PV could be anywhere.
 */
static void do_flatshade_triangle( struct brw_sf_compile *c )
{
   struct brw_codegen *p = &c->func;
   GLuint nr;
   GLuint jmpi = 1;

   /* Already done in clip program:
    */
   if (c->key.primitive == BRW_SF_PRIM_UNFILLED_TRIS)
      return;

   if (p->devinfo->ver == 5)
       jmpi = 2;

   nr = count_flatshaded_attributes(c);

   brw_MUL(p, c->pv, c->pv, brw_imm_d(jmpi*(nr*2+1)));
   brw_JMPI(p, c->pv, BRW_PREDICATE_NONE);

   copy_flatshaded_attributes(c, c->vert[1], c->vert[0]);
   copy_flatshaded_attributes(c, c->vert[2], c->vert[0]);
   brw_JMPI(p, brw_imm_d(jmpi*(nr*4+1)), BRW_PREDICATE_NONE);

   copy_flatshaded_attributes(c, c->vert[0], c->vert[1]);
   copy_flatshaded_attributes(c, c->vert[2], c->vert[1]);
   brw_JMPI(p, brw_imm_d(jmpi*nr*2), BRW_PREDICATE_NONE);

   copy_flatshaded_attributes(c, c->vert[0], c->vert[2]);
   copy_flatshaded_attributes(c, c->vert[1], c->vert[2]);
}


static void do_flatshade_line( struct brw_sf_compile *c )
{
   struct brw_codegen *p = &c->func;
   GLuint nr;
   GLuint jmpi = 1;

   /* Already done in clip program:
    */
   if (c->key.primitive == BRW_SF_PRIM_UNFILLED_TRIS)
      return;

   if (p->devinfo->ver == 5)
       jmpi = 2;

   nr = count_flatshaded_attributes(c);

   brw_MUL(p, c->pv, c->pv, brw_imm_d(jmpi*(nr+1)));
   brw_JMPI(p, c->pv, BRW_PREDICATE_NONE);
   copy_flatshaded_attributes(c, c->vert[1], c->vert[0]);

   brw_JMPI(p, brw_imm_ud(jmpi*nr), BRW_PREDICATE_NONE);
   copy_flatshaded_attributes(c, c->vert[0], c->vert[1]);
}


/***********************************************************************
 * Triangle setup.
 */


static void alloc_regs( struct brw_sf_compile *c )
{
   GLuint reg, i;

   /* Values computed by fixed function unit:
    */
   c->pv  = retype(brw_vec1_grf(1, 1), BRW_REGISTER_TYPE_D);
   c->det = brw_vec1_grf(1, 2);
   c->dx0 = brw_vec1_grf(1, 3);
   c->dx2 = brw_vec1_grf(1, 4);
   c->dy0 = brw_vec1_grf(1, 5);
   c->dy2 = brw_vec1_grf(1, 6);

   /* z and 1/w passed in separately:
    */
   c->z[0]     = brw_vec1_grf(2, 0);
   c->inv_w[0] = brw_vec1_grf(2, 1);
   c->z[1]     = brw_vec1_grf(2, 2);
   c->inv_w[1] = brw_vec1_grf(2, 3);
   c->z[2]     = brw_vec1_grf(2, 4);
   c->inv_w[2] = brw_vec1_grf(2, 5);

   /* The vertices:
    */
   reg = 3;
   for (i = 0; i < c->nr_verts; i++) {
      c->vert[i] = brw_vec8_grf(reg, 0);
      reg += c->nr_attr_regs;
   }

   /* Temporaries, allocated after last vertex reg.
    */
   c->inv_det = brw_vec1_grf(reg, 0);  reg++;
   c->a1_sub_a0 = brw_vec8_grf(reg, 0);  reg++;
   c->a2_sub_a0 = brw_vec8_grf(reg, 0);  reg++;
   c->tmp = brw_vec8_grf(reg, 0);  reg++;

   /* Note grf allocation:
    */
   c->prog_data.total_grf = reg;


   /* Outputs of this program - interpolation coefficients for
    * rasterization:
    */
   c->m1Cx = brw_vec8_reg(BRW_MESSAGE_REGISTER_FILE, 1, 0);
   c->m2Cy = brw_vec8_reg(BRW_MESSAGE_REGISTER_FILE, 2, 0);
   c->m3C0 = brw_vec8_reg(BRW_MESSAGE_REGISTER_FILE, 3, 0);
}


static void copy_z_inv_w( struct brw_sf_compile *c )
{
   struct brw_codegen *p = &c->func;
   GLuint i;

   /* Copy both scalars with a single MOV:
    */
   for (i = 0; i < c->nr_verts; i++)
      brw_MOV(p, vec2(suboffset(c->vert[i], 2)), vec2(c->z[i]));
}


static void invert_det( struct brw_sf_compile *c)
{
   /* Looks like we invert all 8 elements just to get 1/det in
    * position 2 !?!
    */
   gfx4_math(&c->func,
	     c->inv_det,
	     BRW_MATH_FUNCTION_INV,
	     0,
	     c->det,
	     BRW_MATH_PRECISION_FULL);

}


static bool
calculate_masks(struct brw_sf_compile *c,
                GLuint reg,
                GLushort *pc,
                GLushort *pc_persp,
                GLushort *pc_linear)
{
   bool is_last_attr = (reg == c->nr_setup_regs - 1);
   enum glsl_interp_mode interp;

   *pc_persp = 0;
   *pc_linear = 0;
   *pc = 0xf;

   interp = c->key.interp_mode[vert_reg_to_vue_slot(c, reg, 0)];
   if (interp == INTERP_MODE_SMOOTH) {
      *pc_linear = 0xf;
      *pc_persp = 0xf;
   } else if (interp == INTERP_MODE_NOPERSPECTIVE)
      *pc_linear = 0xf;

   /* Maybe only process one attribute on the final round:
    */
   if (vert_reg_to_varying(c, reg, 1) != BRW_VARYING_SLOT_COUNT) {
      *pc |= 0xf0;

      interp = c->key.interp_mode[vert_reg_to_vue_slot(c, reg, 1)];
      if (interp == INTERP_MODE_SMOOTH) {
         *pc_linear |= 0xf0;
         *pc_persp |= 0xf0;
      } else if (interp == INTERP_MODE_NOPERSPECTIVE)
         *pc_linear |= 0xf0;
   }

   return is_last_attr;
}

/* Calculates the predicate control for which channels of a reg
 * (containing 2 attrs) to do point sprite coordinate replacement on.
 */
static uint16_t
calculate_point_sprite_mask(struct brw_sf_compile *c, GLuint reg)
{
   int varying1, varying2;
   uint16_t pc = 0;

   varying1 = vert_reg_to_varying(c, reg, 0);
   if (varying1 >= VARYING_SLOT_TEX0 && varying1 <= VARYING_SLOT_TEX7) {
      if (c->key.point_sprite_coord_replace & (1 << (varying1 - VARYING_SLOT_TEX0)))
	 pc |= 0x0f;
   }
   if (varying1 == BRW_VARYING_SLOT_PNTC)
      pc |= 0x0f;

   varying2 = vert_reg_to_varying(c, reg, 1);
   if (varying2 >= VARYING_SLOT_TEX0 && varying2 <= VARYING_SLOT_TEX7) {
      if (c->key.point_sprite_coord_replace & (1 << (varying2 -
                                                     VARYING_SLOT_TEX0)))
         pc |= 0xf0;
   }
   if (varying2 == BRW_VARYING_SLOT_PNTC)
      pc |= 0xf0;

   return pc;
}

static void
set_predicate_control_flag_value(struct brw_codegen *p,
                                 struct brw_sf_compile *c,
                                 unsigned value)
{
   brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);

   if (value != 0xff) {
      if (value != c->flag_value) {
         brw_MOV(p, brw_flag_reg(0, 0), brw_imm_uw(value));
         c->flag_value = value;
      }

      brw_set_default_predicate_control(p, BRW_PREDICATE_NORMAL);
   }
}

static void brw_emit_tri_setup(struct brw_sf_compile *c, bool allocate)
{
   struct brw_codegen *p = &c->func;
   GLuint i;

   c->flag_value = 0xff;
   c->nr_verts = 3;

   if (allocate)
      alloc_regs(c);

   invert_det(c);
   copy_z_inv_w(c);

   if (c->key.do_twoside_color)
      do_twoside_color(c);

   if (c->key.contains_flat_varying)
      do_flatshade_triangle(c);


   for (i = 0; i < c->nr_setup_regs; i++)
   {
      /* Pair of incoming attributes:
       */
      struct brw_reg a0 = offset(c->vert[0], i);
      struct brw_reg a1 = offset(c->vert[1], i);
      struct brw_reg a2 = offset(c->vert[2], i);
      GLushort pc, pc_persp, pc_linear;
      bool last = calculate_masks(c, i, &pc, &pc_persp, &pc_linear);

      if (pc_persp)
      {
	 set_predicate_control_flag_value(p, c, pc_persp);
	 brw_MUL(p, a0, a0, c->inv_w[0]);
	 brw_MUL(p, a1, a1, c->inv_w[1]);
	 brw_MUL(p, a2, a2, c->inv_w[2]);
      }


      /* Calculate coefficients for interpolated values:
       */
      if (pc_linear)
      {
	 set_predicate_control_flag_value(p, c, pc_linear);

	 brw_ADD(p, c->a1_sub_a0, a1, negate(a0));
	 brw_ADD(p, c->a2_sub_a0, a2, negate(a0));

	 /* calculate dA/dx
	  */
	 brw_MUL(p, brw_null_reg(), c->a1_sub_a0, c->dy2);
	 brw_MAC(p, c->tmp, c->a2_sub_a0, negate(c->dy0));
	 brw_MUL(p, c->m1Cx, c->tmp, c->inv_det);

	 /* calculate dA/dy
	  */
	 brw_MUL(p, brw_null_reg(), c->a2_sub_a0, c->dx0);
	 brw_MAC(p, c->tmp, c->a1_sub_a0, negate(c->dx2));
	 brw_MUL(p, c->m2Cy, c->tmp, c->inv_det);
      }

      {
	 set_predicate_control_flag_value(p, c, pc);
	 /* start point for interpolation
	  */
	 brw_MOV(p, c->m3C0, a0);

	 /* Copy m0..m3 to URB.  m0 is implicitly copied from r0 in
	  * the send instruction:
	  */
	 brw_urb_WRITE(p,
		       brw_null_reg(),
		       0,
		       brw_vec8_grf(0, 0), /* r0, will be copied to m0 */
                       last ? BRW_URB_WRITE_EOT_COMPLETE
                       : BRW_URB_WRITE_NO_FLAGS,
		       4, 	/* msg len */
		       0,	/* response len */
		       i*4,	/* offset */
		       BRW_URB_SWIZZLE_TRANSPOSE); /* XXX: Swizzle control "SF to windower" */
      }
   }

   brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
}



static void brw_emit_line_setup(struct brw_sf_compile *c, bool allocate)
{
   struct brw_codegen *p = &c->func;
   GLuint i;

   c->flag_value = 0xff;
   c->nr_verts = 2;

   if (allocate)
      alloc_regs(c);

   invert_det(c);
   copy_z_inv_w(c);

   if (c->key.contains_flat_varying)
      do_flatshade_line(c);

   for (i = 0; i < c->nr_setup_regs; i++)
   {
      /* Pair of incoming attributes:
       */
      struct brw_reg a0 = offset(c->vert[0], i);
      struct brw_reg a1 = offset(c->vert[1], i);
      GLushort pc, pc_persp, pc_linear;
      bool last = calculate_masks(c, i, &pc, &pc_persp, &pc_linear);

      if (pc_persp)
      {
	 set_predicate_control_flag_value(p, c, pc_persp);
	 brw_MUL(p, a0, a0, c->inv_w[0]);
	 brw_MUL(p, a1, a1, c->inv_w[1]);
      }

      /* Calculate coefficients for position, color:
       */
      if (pc_linear) {
	 set_predicate_control_flag_value(p, c, pc_linear);

	 brw_ADD(p, c->a1_sub_a0, a1, negate(a0));

	 brw_MUL(p, c->tmp, c->a1_sub_a0, c->dx0);
	 brw_MUL(p, c->m1Cx, c->tmp, c->inv_det);

	 brw_MUL(p, c->tmp, c->a1_sub_a0, c->dy0);
	 brw_MUL(p, c->m2Cy, c->tmp, c->inv_det);
      }

      {
	 set_predicate_control_flag_value(p, c, pc);

	 /* start point for interpolation
	  */
	 brw_MOV(p, c->m3C0, a0);

	 /* Copy m0..m3 to URB.
	  */
	 brw_urb_WRITE(p,
		       brw_null_reg(),
		       0,
		       brw_vec8_grf(0, 0),
                       last ? BRW_URB_WRITE_EOT_COMPLETE
                       : BRW_URB_WRITE_NO_FLAGS,
		       4, 	/* msg len */
		       0,	/* response len */
		       i*4,	/* urb destination offset */
		       BRW_URB_SWIZZLE_TRANSPOSE);
      }
   }

   brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
}

static void brw_emit_point_sprite_setup(struct brw_sf_compile *c, bool allocate)
{
   struct brw_codegen *p = &c->func;
   GLuint i;

   c->flag_value = 0xff;
   c->nr_verts = 1;

   if (allocate)
      alloc_regs(c);

   copy_z_inv_w(c);
   for (i = 0; i < c->nr_setup_regs; i++)
   {
      struct brw_reg a0 = offset(c->vert[0], i);
      GLushort pc, pc_persp, pc_linear, pc_coord_replace;
      bool last = calculate_masks(c, i, &pc, &pc_persp, &pc_linear);

      pc_coord_replace = calculate_point_sprite_mask(c, i);
      pc_persp &= ~pc_coord_replace;

      if (pc_persp) {
	 set_predicate_control_flag_value(p, c, pc_persp);
	 brw_MUL(p, a0, a0, c->inv_w[0]);
      }

      /* Point sprite coordinate replacement: A texcoord with this
       * enabled gets replaced with the value (x, y, 0, 1) where x and
       * y vary from 0 to 1 across the horizontal and vertical of the
       * point.
       */
      if (pc_coord_replace) {
	 set_predicate_control_flag_value(p, c, pc_coord_replace);
	 /* Calculate 1.0/PointWidth */
	 gfx4_math(&c->func,
		   c->tmp,
		   BRW_MATH_FUNCTION_INV,
		   0,
		   c->dx0,
		   BRW_MATH_PRECISION_FULL);

	 brw_set_default_access_mode(p, BRW_ALIGN_16);

	 /* dA/dx, dA/dy */
	 brw_MOV(p, c->m1Cx, brw_imm_f(0.0));
	 brw_MOV(p, c->m2Cy, brw_imm_f(0.0));
	 brw_MOV(p, brw_writemask(c->m1Cx, WRITEMASK_X), c->tmp);
	 if (c->key.sprite_origin_lower_left) {
	    brw_MOV(p, brw_writemask(c->m2Cy, WRITEMASK_Y), negate(c->tmp));
	 } else {
	    brw_MOV(p, brw_writemask(c->m2Cy, WRITEMASK_Y), c->tmp);
	 }

	 /* attribute constant offset */
	 brw_MOV(p, c->m3C0, brw_imm_f(0.0));
	 if (c->key.sprite_origin_lower_left) {
	    brw_MOV(p, brw_writemask(c->m3C0, WRITEMASK_YW), brw_imm_f(1.0));
	 } else {
	    brw_MOV(p, brw_writemask(c->m3C0, WRITEMASK_W), brw_imm_f(1.0));
	 }

	 brw_set_default_access_mode(p, BRW_ALIGN_1);
      }

      if (pc & ~pc_coord_replace) {
	 set_predicate_control_flag_value(p, c, pc & ~pc_coord_replace);
	 brw_MOV(p, c->m1Cx, brw_imm_ud(0));
	 brw_MOV(p, c->m2Cy, brw_imm_ud(0));
	 brw_MOV(p, c->m3C0, a0); /* constant value */
      }


      set_predicate_control_flag_value(p, c, pc);
      /* Copy m0..m3 to URB. */
      brw_urb_WRITE(p,
		    brw_null_reg(),
		    0,
		    brw_vec8_grf(0, 0),
                    last ? BRW_URB_WRITE_EOT_COMPLETE
                    : BRW_URB_WRITE_NO_FLAGS,
		    4, 	/* msg len */
		    0,	/* response len */
		    i*4,	/* urb destination offset */
		    BRW_URB_SWIZZLE_TRANSPOSE);
   }

   brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
}

/* Points setup - several simplifications as all attributes are
 * constant across the face of the point (point sprites excluded!)
 */
static void brw_emit_point_setup(struct brw_sf_compile *c, bool allocate)
{
   struct brw_codegen *p = &c->func;
   GLuint i;

   c->flag_value = 0xff;
   c->nr_verts = 1;

   if (allocate)
      alloc_regs(c);

   copy_z_inv_w(c);

   brw_MOV(p, c->m1Cx, brw_imm_ud(0)); /* zero - move out of loop */
   brw_MOV(p, c->m2Cy, brw_imm_ud(0)); /* zero - move out of loop */

   for (i = 0; i < c->nr_setup_regs; i++)
   {
      struct brw_reg a0 = offset(c->vert[0], i);
      GLushort pc, pc_persp, pc_linear;
      bool last = calculate_masks(c, i, &pc, &pc_persp, &pc_linear);

      if (pc_persp)
      {
	 /* This seems odd as the values are all constant, but the
	  * fragment shader will be expecting it:
	  */
	 set_predicate_control_flag_value(p, c, pc_persp);
	 brw_MUL(p, a0, a0, c->inv_w[0]);
      }


      /* The delta values are always zero, just send the starting
       * coordinate.  Again, this is to fit in with the interpolation
       * code in the fragment shader.
       */
      {
	 set_predicate_control_flag_value(p, c, pc);

	 brw_MOV(p, c->m3C0, a0); /* constant value */

	 /* Copy m0..m3 to URB.
	  */
	 brw_urb_WRITE(p,
		       brw_null_reg(),
		       0,
		       brw_vec8_grf(0, 0),
                       last ? BRW_URB_WRITE_EOT_COMPLETE
                       : BRW_URB_WRITE_NO_FLAGS,
		       4, 	/* msg len */
		       0,	/* response len */
		       i*4,	/* urb destination offset */
		       BRW_URB_SWIZZLE_TRANSPOSE);
      }
   }

   brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
}

static void brw_emit_anyprim_setup( struct brw_sf_compile *c )
{
   struct brw_codegen *p = &c->func;
   struct brw_reg payload_prim = brw_uw1_reg(BRW_GENERAL_REGISTER_FILE, 1, 0);
   struct brw_reg payload_attr = get_element_ud(brw_vec1_reg(BRW_GENERAL_REGISTER_FILE, 1, 0), 0);
   struct brw_reg primmask;
   int jmp;
   struct brw_reg v1_null_ud = vec1(retype(brw_null_reg(), BRW_REGISTER_TYPE_UD));

   c->nr_verts = 3;
   alloc_regs(c);

   primmask = retype(get_element(c->tmp, 0), BRW_REGISTER_TYPE_UD);

   brw_MOV(p, primmask, brw_imm_ud(1));
   brw_SHL(p, primmask, primmask, payload_prim);

   brw_AND(p, v1_null_ud, primmask, brw_imm_ud((1<<_3DPRIM_TRILIST) |
					       (1<<_3DPRIM_TRISTRIP) |
					       (1<<_3DPRIM_TRIFAN) |
					       (1<<_3DPRIM_TRISTRIP_REVERSE) |
					       (1<<_3DPRIM_POLYGON) |
					       (1<<_3DPRIM_RECTLIST) |
					       (1<<_3DPRIM_TRIFAN_NOSTIPPLE)));
   brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_Z);
   jmp = brw_JMPI(p, brw_imm_d(0), BRW_PREDICATE_NORMAL) - p->store;
   brw_emit_tri_setup(c, false);
   brw_land_fwd_jump(p, jmp);

   brw_AND(p, v1_null_ud, primmask, brw_imm_ud((1<<_3DPRIM_LINELIST) |
					       (1<<_3DPRIM_LINESTRIP) |
					       (1<<_3DPRIM_LINELOOP) |
					       (1<<_3DPRIM_LINESTRIP_CONT) |
					       (1<<_3DPRIM_LINESTRIP_BF) |
					       (1<<_3DPRIM_LINESTRIP_CONT_BF)));
   brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_Z);
   jmp = brw_JMPI(p, brw_imm_d(0), BRW_PREDICATE_NORMAL) - p->store;
   brw_emit_line_setup(c, false);
   brw_land_fwd_jump(p, jmp);

   brw_AND(p, v1_null_ud, payload_attr, brw_imm_ud(1<<BRW_SPRITE_POINT_ENABLE));
   brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_Z);
   jmp = brw_JMPI(p, brw_imm_d(0), BRW_PREDICATE_NORMAL) - p->store;
   brw_emit_point_sprite_setup(c, false);
   brw_land_fwd_jump(p, jmp);

   brw_emit_point_setup( c, false );
}

const unsigned *
brw_compile_sf(const struct brw_compiler *compiler,
               void *mem_ctx,
               const struct brw_sf_prog_key *key,
               struct brw_sf_prog_data *prog_data,
               struct brw_vue_map *vue_map,
               unsigned *final_assembly_size)
{
   struct brw_sf_compile c;
   memset(&c, 0, sizeof(c));

   /* Begin the compilation:
    */
   brw_init_codegen(&compiler->isa, &c.func, mem_ctx);

   c.key = *key;
   c.vue_map = *vue_map;
   if (c.key.do_point_coord) {
      /*
       * gl_PointCoord is a FS instead of VS builtin variable, thus it's
       * not included in c.vue_map generated in VS stage. Here we add
       * it manually to let SF shader generate the needed interpolation
       * coefficient for FS shader.
       */
      c.vue_map.varying_to_slot[BRW_VARYING_SLOT_PNTC] = c.vue_map.num_slots;
      c.vue_map.slot_to_varying[c.vue_map.num_slots++] = BRW_VARYING_SLOT_PNTC;
   }
   c.urb_entry_read_offset = BRW_SF_URB_ENTRY_READ_OFFSET;
   c.nr_attr_regs = (c.vue_map.num_slots + 1)/2 - c.urb_entry_read_offset;
   c.nr_setup_regs = c.nr_attr_regs;

   c.prog_data.urb_read_length = c.nr_attr_regs;
   c.prog_data.urb_entry_size = c.nr_setup_regs * 2;

   /* Which primitive?  Or all three?
    */
   switch (key->primitive) {
   case BRW_SF_PRIM_TRIANGLES:
      c.nr_verts = 3;
      brw_emit_tri_setup( &c, true );
      break;
   case BRW_SF_PRIM_LINES:
      c.nr_verts = 2;
      brw_emit_line_setup( &c, true );
      break;
   case BRW_SF_PRIM_POINTS:
      c.nr_verts = 1;
      if (key->do_point_sprite)
	  brw_emit_point_sprite_setup( &c, true );
      else
	  brw_emit_point_setup( &c, true );
      break;
   case BRW_SF_PRIM_UNFILLED_TRIS:
      c.nr_verts = 3;
      brw_emit_anyprim_setup( &c );
      break;
   default:
      unreachable("not reached");
   }

   /* FINISHME: SF programs use calculated jumps (i.e., JMPI with a register
    * source). Compacting would be difficult.
    */
   /* brw_compact_instructions(&c.func, 0, 0, NULL); */

   *prog_data = c.prog_data;

   const unsigned *program = brw_get_program(&c.func, final_assembly_size);

   if (INTEL_DEBUG(DEBUG_SF)) {
      fprintf(stderr, "sf:\n");
      brw_disassemble_with_labels(&compiler->isa,
                                  program, 0, *final_assembly_size, stderr);
      fprintf(stderr, "\n");
   }

   return program;
}
