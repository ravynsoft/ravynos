/**************************************************************************
 *
 * Copyright 2010 VMware.
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


#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/os_time.h"
#include "gallivm/lp_bld_arit.h"
#include "gallivm/lp_bld_bitarit.h"
#include "gallivm/lp_bld_const.h"
#include "gallivm/lp_bld_debug.h"
#include "gallivm/lp_bld_init.h"
#include "gallivm/lp_bld_logic.h"
#include "gallivm/lp_bld_intr.h"
#include "gallivm/lp_bld_flow.h"
#include "gallivm/lp_bld_type.h"

#include "lp_perf.h"
#include "lp_debug.h"
#include "lp_flush.h"
#include "lp_screen.h"
#include "lp_context.h"
#include "lp_state.h"
#include "lp_state_fs.h"
#include "lp_state_setup.h"

#include "nir.h"

/** Setup shader number (for debugging) */
static unsigned setup_no = 0;


/* currently organized to interpolate full float[4] attributes even
 * when some elements are unused.  Later, can pack vertex data more
 * closely.
 */


struct lp_setup_args
{
   /* Function arguments:
    */
   LLVMValueRef v0;
   LLVMValueRef v1;
   LLVMValueRef v2;
   LLVMValueRef facing;		/* boolean */
   LLVMValueRef a0;
   LLVMValueRef dadx;
   LLVMValueRef dady;
   LLVMValueRef key;
   LLVMTypeRef vec4f_type;

   /* Derived:
    */
   LLVMValueRef x0_center;
   LLVMValueRef y0_center;
   LLVMValueRef dy20_ooa;
   LLVMValueRef dy01_ooa;
   LLVMValueRef dx20_ooa;
   LLVMValueRef dx01_ooa;
   struct lp_build_context bld;
};


static void
store_coef(struct gallivm_state *gallivm,
           const struct lp_setup_args *args,
           unsigned slot,
           LLVMValueRef a0,
           LLVMValueRef dadx,
           LLVMValueRef dady)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef idx = lp_build_const_int32(gallivm, slot);

   LLVMBuildStore(builder,
                  a0,
                  LLVMBuildGEP2(builder, args->vec4f_type, args->a0, &idx, 1, ""));

   LLVMBuildStore(builder,
                  dadx,
                  LLVMBuildGEP2(builder, args->vec4f_type, args->dadx, &idx, 1, ""));

   LLVMBuildStore(builder,
                  dady,
                  LLVMBuildGEP2(builder, args->vec4f_type, args->dady, &idx, 1, ""));
}


static void
emit_constant_coef4(struct gallivm_state *gallivm,
                    const struct lp_setup_args *args,
                    unsigned slot,
                    LLVMValueRef vert)
{
   store_coef(gallivm, args, slot, vert, args->bld.zero, args->bld.zero);
}


/**
 * Setup the fragment input attribute with the front-facing value.
 * \param frontface  is the triangle front facing?
 */
static void
emit_facing_coef(struct gallivm_state *gallivm,
                 struct lp_setup_args *args,
                 unsigned slot)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMTypeRef float_type = LLVMFloatTypeInContext(gallivm->context);
   LLVMValueRef a0_0 = args->facing;
   LLVMValueRef a0_0f = LLVMBuildSIToFP(builder, a0_0, float_type, "");
   LLVMValueRef a0, face_val;
   const unsigned char swizzles[4] = { PIPE_SWIZZLE_X, PIPE_SWIZZLE_0,
                                       PIPE_SWIZZLE_0, PIPE_SWIZZLE_0 };
   /* Our face val is either 1 or 0 so we do
    * face = (val * 2) - 1
    * to make it 1 or -1
    */
   face_val =
      LLVMBuildFAdd(builder,
                    LLVMBuildFMul(builder, a0_0f,
                                  lp_build_const_float(gallivm, 2.0),
                                  ""),
                    lp_build_const_float(gallivm, -1.0),
                    "facing");
   face_val = lp_build_broadcast_scalar(&args->bld, face_val);
   a0 = lp_build_swizzle_aos(&args->bld, face_val, swizzles);

   store_coef(gallivm, args, slot, a0, args->bld.zero, args->bld.zero);
}


static LLVMValueRef
vert_attrib(struct gallivm_state *gallivm,
            LLVMTypeRef vert_type,
            LLVMValueRef vert,
            int attr,
            int elem,
            const char *name)
{
   LLVMBuilderRef b = gallivm->builder;
   LLVMValueRef idx[2];
   LLVMTypeRef v_type = LLVMFloatTypeInContext(gallivm->context);

   idx[0] = lp_build_const_int32(gallivm, attr);
   idx[1] = lp_build_const_int32(gallivm, elem);
   return LLVMBuildLoad2(b, v_type, LLVMBuildGEP2(b, vert_type, vert, idx, 2, ""), name);
}


static void
lp_twoside(struct gallivm_state *gallivm,
           struct lp_setup_args *args,
           const struct lp_setup_variant_key *key,
           int bcolor_slot,
           LLVMValueRef attribv[3])
{
   LLVMBuilderRef b = gallivm->builder;
   LLVMValueRef a0_back, a1_back, a2_back;
   LLVMValueRef idx2 = lp_build_const_int32(gallivm, bcolor_slot);

   LLVMValueRef facing = args->facing;
   LLVMValueRef front_facing = LLVMBuildICmp(b, LLVMIntEQ, facing,
                                             lp_build_const_int32(gallivm, 0), ""); /** need i1 for if condition */

   a0_back = LLVMBuildLoad2(b, args->vec4f_type, LLVMBuildGEP2(b, args->vec4f_type, args->v0, &idx2, 1, ""), "v0a_back");
   a1_back = LLVMBuildLoad2(b, args->vec4f_type, LLVMBuildGEP2(b, args->vec4f_type, args->v1, &idx2, 1, ""), "v1a_back");
   a2_back = LLVMBuildLoad2(b, args->vec4f_type, LLVMBuildGEP2(b, args->vec4f_type, args->v2, &idx2, 1, ""), "v2a_back");

   /* Possibly swap the front and back attrib values,
    *
    * Prefer select to if so we don't have to worry about phis or
    * allocas.
    */
   attribv[0] = LLVMBuildSelect(b, front_facing, a0_back, attribv[0], "");
   attribv[1] = LLVMBuildSelect(b, front_facing, a1_back, attribv[1], "");
   attribv[2] = LLVMBuildSelect(b, front_facing, a2_back, attribv[2], "");
}


static LLVMValueRef
lp_do_offset_tri(struct gallivm_state *gallivm,
                 struct lp_setup_args *args,
                 const struct lp_setup_variant_key *key,
                 LLVMValueRef inv_det,
                 LLVMValueRef dxyz01,
                 LLVMValueRef dxyz20,
                 LLVMValueRef attribv[3])
{
   LLVMBuilderRef b = gallivm->builder;
   struct lp_build_context flt_scalar_bld;
   struct lp_build_context int_scalar_bld;
   struct lp_build_context *bld = &args->bld;
   LLVMValueRef zoffset, mult;
   LLVMValueRef dzdxdzdy, dzdx, dzdy, dzxyz20, dyzzx01, dyzzx01_dzxyz20, dzx01_dyz20;
   LLVMValueRef max, max_value, res12;
   LLVMValueRef shuffles[4];
   LLVMTypeRef shuf_type = LLVMInt32TypeInContext(gallivm->context);
   LLVMValueRef onei = lp_build_const_int32(gallivm, 1);
   LLVMValueRef zeroi = lp_build_const_int32(gallivm, 0);
   LLVMValueRef twoi = lp_build_const_int32(gallivm, 2);
   LLVMValueRef threei  = lp_build_const_int32(gallivm, 3);

   /* (res12) = cross(e,f).xy */
   shuffles[0] = twoi;
   shuffles[1] = zeroi;
   shuffles[2] = onei;
   shuffles[3] = twoi;
   dzxyz20 = LLVMBuildShuffleVector(b, dxyz20, dxyz20, LLVMConstVector(shuffles, 4), "");

   shuffles[0] = onei;
   shuffles[1] = twoi;
   shuffles[2] = twoi;
   shuffles[3] = zeroi;
   dyzzx01 = LLVMBuildShuffleVector(b, dxyz01, dxyz01, LLVMConstVector(shuffles, 4), "");

   dyzzx01_dzxyz20 = LLVMBuildFMul(b, dzxyz20, dyzzx01, "dyzzx01_dzxyz20");

   shuffles[0] = twoi;
   shuffles[1] = threei;
   shuffles[2] = LLVMGetUndef(shuf_type);
   shuffles[3] = LLVMGetUndef(shuf_type);
   dzx01_dyz20 = LLVMBuildShuffleVector(b, dyzzx01_dzxyz20, dyzzx01_dzxyz20,
                                        LLVMConstVector(shuffles, 4), "");

   res12 = LLVMBuildFSub(b, dyzzx01_dzxyz20, dzx01_dyz20, "res12");

   /* dzdx = fabsf(res1 * inv_det), dydx = fabsf(res2 * inv_det)*/
   dzdxdzdy = LLVMBuildFMul(b, res12, inv_det, "dzdxdzdy");
   dzdxdzdy = lp_build_abs(bld, dzdxdzdy);

   dzdx = LLVMBuildExtractElement(b, dzdxdzdy, zeroi, "");
   dzdy = LLVMBuildExtractElement(b, dzdxdzdy, onei, "");

   /* mult = MAX2(dzdx, dzdy) * pgon_offset_scale */
   max = LLVMBuildFCmp(b, LLVMRealUGT, dzdx, dzdy, "");
   max_value = LLVMBuildSelect(b, max, dzdx, dzdy, "max");

   mult = LLVMBuildFMul(b, max_value,
                        lp_build_const_float(gallivm,
                                             key->pgon_offset_scale), "");

   lp_build_context_init(&flt_scalar_bld, gallivm, lp_type_float_vec(32, 32));

   if (key->floating_point_depth) {
      /*
       * bias = pgon_offset_units * 2^(exponent(max(abs(z0), abs(z1), abs(z2))) -
       *           mantissa_bits) + MAX2(dzdx, dzdy) * pgon_offset_scale
       *
       * NOTE: Assumes IEEE float32.
       */
      LLVMValueRef c23_shifted, exp_mask, bias, exp;
      LLVMValueRef maxz_value, maxz0z1_value;

      lp_build_context_init(&int_scalar_bld, gallivm, lp_type_int_vec(32, 32));

      c23_shifted = lp_build_const_int32(gallivm, 23 << 23);
      exp_mask = lp_build_const_int32(gallivm, 0xff << 23);

      maxz0z1_value = lp_build_max(&flt_scalar_bld,
                         lp_build_abs(&flt_scalar_bld,
                            LLVMBuildExtractElement(b, attribv[0], twoi, "")),
                         lp_build_abs(&flt_scalar_bld,
                            LLVMBuildExtractElement(b, attribv[1], twoi, "")));

      maxz_value = lp_build_max(&flt_scalar_bld,
                      lp_build_abs(&flt_scalar_bld,
                         LLVMBuildExtractElement(b, attribv[2], twoi, "")),
                      maxz0z1_value);

      exp = LLVMBuildBitCast(b, maxz_value, int_scalar_bld.vec_type, "");
      exp = lp_build_and(&int_scalar_bld, exp, exp_mask);
      exp = lp_build_sub(&int_scalar_bld, exp, c23_shifted);
      /* Clamping to zero means mrd will be zero for very small numbers,
       * but specs do not indicate this should be prevented by clamping
       * mrd to smallest normal number instead. */
      exp = lp_build_max(&int_scalar_bld, exp, int_scalar_bld.zero);
      exp = LLVMBuildBitCast(b, exp, flt_scalar_bld.vec_type, "");

      bias = LLVMBuildFMul(b, exp,
                           lp_build_const_float(gallivm, key->pgon_offset_units),
                           "bias");

      zoffset = LLVMBuildFAdd(b, bias, mult, "zoffset");
   } else {
      /*
       * bias = pgon_offset_units + MAX2(dzdx, dzdy) * pgon_offset_scale
       */
      zoffset = LLVMBuildFAdd(b,
                              lp_build_const_float(gallivm, key->pgon_offset_units),
                              mult, "zoffset");
   }

   if (key->pgon_offset_clamp > 0) {
      zoffset = lp_build_min(&flt_scalar_bld,
                             lp_build_const_float(gallivm, key->pgon_offset_clamp),
                             zoffset);
   } else if (key->pgon_offset_clamp < 0) {
      zoffset = lp_build_max(&flt_scalar_bld,
                             lp_build_const_float(gallivm, key->pgon_offset_clamp),
                             zoffset);
   }

   return zoffset;
}


static void
load_attribute(struct gallivm_state *gallivm,
               struct lp_setup_args *args,
               const struct lp_setup_variant_key *key,
               unsigned vert_attr,
               LLVMValueRef attribv[3])
{
   LLVMBuilderRef b = gallivm->builder;
   LLVMValueRef idx = lp_build_const_int32(gallivm, vert_attr);

   /* Load the vertex data
    */
   attribv[0] = LLVMBuildLoad2(b, args->vec4f_type, LLVMBuildGEP2(b, args->vec4f_type, args->v0, &idx, 1, ""), "v0a");
   attribv[1] = LLVMBuildLoad2(b, args->vec4f_type, LLVMBuildGEP2(b, args->vec4f_type, args->v1, &idx, 1, ""), "v1a");
   attribv[2] = LLVMBuildLoad2(b, args->vec4f_type, LLVMBuildGEP2(b, args->vec4f_type, args->v2, &idx, 1, ""), "v2a");

   /* Potentially modify it according to twoside, etc:
    */
   if (key->twoside) {
      if (vert_attr == key->color_slot && key->bcolor_slot >= 0)
         lp_twoside(gallivm, args, key, key->bcolor_slot, attribv);
      else if (vert_attr == key->spec_slot && key->bspec_slot >= 0)
         lp_twoside(gallivm, args, key, key->bspec_slot, attribv);
   }
}


/*
 * FIXME: interpolation is always done wrt fb origin (0/0).
 * However, if some (small) tri is far away from the origin and gradients
 * are large, this can lead to HUGE errors, since the a0 value calculated
 * here can get very large (with the actual values inside the triangle way
 * smaller), leading to complete loss of accuracy. This could be prevented
 * by using some point inside (or at corner) of the tri as interpolation
 * origin, or just use barycentric interpolation (which GL suggests and is
 * what real hw does - you can get the barycentric coordinates from the
 * edge functions in rasterization in principle (though we skip these
 * sometimes completely in case of tris covering a block fully,
 * which obviously wouldn't work)).
 */
static void
calc_coef4(struct gallivm_state *gallivm,
           struct lp_setup_args *args,
           LLVMValueRef a0,
           LLVMValueRef a1,
           LLVMValueRef a2,
           LLVMValueRef out[3])
{
   LLVMBuilderRef b = gallivm->builder;
   LLVMValueRef attr_0;
   LLVMValueRef dy20_ooa = args->dy20_ooa;
   LLVMValueRef dy01_ooa = args->dy01_ooa;
   LLVMValueRef dx20_ooa = args->dx20_ooa;
   LLVMValueRef dx01_ooa = args->dx01_ooa;
   LLVMValueRef x0_center = args->x0_center;
   LLVMValueRef y0_center = args->y0_center;
   LLVMValueRef da01 = LLVMBuildFSub(b, a0, a1, "da01");
   LLVMValueRef da20 = LLVMBuildFSub(b, a2, a0, "da20");

   /* Calculate dadx (vec4f)
    */
   LLVMValueRef da01_dy20_ooa = LLVMBuildFMul(b, da01, dy20_ooa, "da01_dy20_ooa");
   LLVMValueRef da20_dy01_ooa = LLVMBuildFMul(b, da20, dy01_ooa, "da20_dy01_ooa");
   LLVMValueRef dadx          = LLVMBuildFSub(b, da01_dy20_ooa, da20_dy01_ooa, "dadx");

   /* Calculate dady (vec4f)
    */
   LLVMValueRef da01_dx20_ooa = LLVMBuildFMul(b, da01, dx20_ooa, "da01_dx20_ooa");
   LLVMValueRef da20_dx01_ooa = LLVMBuildFMul(b, da20, dx01_ooa, "da20_dx01_ooa");
   LLVMValueRef dady          = LLVMBuildFSub(b, da20_dx01_ooa, da01_dx20_ooa, "dady");

   /* Calculate a0 - the attribute value at the origin
    */
   LLVMValueRef dadx_x0    = LLVMBuildFMul(b, dadx, x0_center, "dadx_x0");
   LLVMValueRef dady_y0    = LLVMBuildFMul(b, dady, y0_center, "dady_y0");
   LLVMValueRef attr_v0    = LLVMBuildFAdd(b, dadx_x0, dady_y0, "attr_v0");
   attr_0                  = LLVMBuildFSub(b, a0, attr_v0, "attr_0");

   out[0] = attr_0;
   out[1] = dadx;
   out[2] = dady;
}


static void
emit_coef4(struct gallivm_state *gallivm,
           struct lp_setup_args *args,
           unsigned slot,
           LLVMValueRef a0,
           LLVMValueRef a1,
           LLVMValueRef a2)
{
   LLVMValueRef coeffs[3];
   calc_coef4(gallivm, args, a0, a1, a2, coeffs);
   store_coef(gallivm, args, slot, coeffs[0], coeffs[1], coeffs[2]);
}


static void
emit_linear_coef(struct gallivm_state *gallivm,
                 struct lp_setup_args *args,
                 unsigned slot,
                 LLVMValueRef attribv[3])
{
   /* nothing to do anymore */
   emit_coef4(gallivm, args, slot, attribv[0], attribv[1], attribv[2]);
}


/**
 * Compute a0, dadx and dady for a perspective-corrected interpolant,
 * for a triangle.
 * We basically multiply the vertex value by 1/w before computing
 * the plane coefficients (a0, dadx, dady).
 * Later, when we compute the value at a particular fragment position we'll
 * divide the interpolated value by the interpolated W at that fragment.
 */
static void
apply_perspective_corr(struct gallivm_state *gallivm,
                       struct lp_setup_args *args,
                       unsigned slot,
                       LLVMValueRef attribv[3])
{
   LLVMBuilderRef b = gallivm->builder;

   /* premultiply by 1/w  (v[0][3] is always 1/w):
    */
   LLVMValueRef v0_oow = lp_build_broadcast_scalar(&args->bld,
                                                   vert_attrib(gallivm, args->vec4f_type,
                                                               args->v0, 0, 3, "v0_oow"));
   LLVMValueRef v1_oow = lp_build_broadcast_scalar(&args->bld,
                                                   vert_attrib(gallivm, args->vec4f_type,
                                                               args->v1, 0, 3, "v1_oow"));
   LLVMValueRef v2_oow = lp_build_broadcast_scalar(&args->bld,
                                                   vert_attrib(gallivm, args->vec4f_type,
                                                               args->v2, 0, 3, "v2_oow"));

   attribv[0] = LLVMBuildFMul(b, attribv[0], v0_oow, "v0_oow_v0a");
   attribv[1] = LLVMBuildFMul(b, attribv[1], v1_oow, "v1_oow_v1a");
   attribv[2] = LLVMBuildFMul(b, attribv[2], v2_oow, "v2_oow_v2a");
}


/**
 * Compute the inputs-> dadx, dady, a0 values.
 */
static void
emit_tri_coef(struct gallivm_state *gallivm,
              const struct lp_setup_variant_key *key,
              struct lp_setup_args *args)
{
   LLVMValueRef attribs[3];

   /* setup interpolation for all the remaining attributes */
   for (unsigned slot = 0; slot < key->num_inputs; slot++) {
      switch (key->inputs[slot].interp) {
      case LP_INTERP_CONSTANT:
         load_attribute(gallivm, args, key, key->inputs[slot].src_index, attribs);
         if (key->flatshade_first) {
            emit_constant_coef4(gallivm, args, slot+1, attribs[0]);
         } else {
            emit_constant_coef4(gallivm, args, slot+1, attribs[2]);
         }
         break;

      case LP_INTERP_LINEAR:
         load_attribute(gallivm, args, key, key->inputs[slot].src_index, attribs);
         emit_linear_coef(gallivm, args, slot+1, attribs);
         break;

      case LP_INTERP_PERSPECTIVE:
         load_attribute(gallivm, args, key, key->inputs[slot].src_index, attribs);
         apply_perspective_corr(gallivm, args, slot+1, attribs);
         emit_linear_coef(gallivm, args, slot+1, attribs);
         break;

      case LP_INTERP_POSITION:
         /*
          * The generated pixel interpolators will pick up the coeffs from
          * slot 0.
          */
         break;

      case LP_INTERP_FACING:
         emit_facing_coef(gallivm, args, slot+1);
         break;

      default:
         assert(0);
      }
   }
}


/* XXX: generic code:
 */
static void
set_noalias(LLVMBuilderRef builder,
            LLVMValueRef function,
            const LLVMTypeRef *arg_types,
            int nr_args)
{
   for (int i = 0; i < nr_args; ++i) {
      if (LLVMGetTypeKind(arg_types[i]) == LLVMPointerTypeKind) {
         lp_add_function_attr(function, i + 1, LP_FUNC_ATTR_NOALIAS);
      }
   }
}


static void
init_args(struct gallivm_state *gallivm,
          const struct lp_setup_variant_key *key,
          struct lp_setup_args *args)
{
   LLVMBuilderRef b = gallivm->builder;
   LLVMTypeRef shuf_type = LLVMInt32TypeInContext(gallivm->context);
   LLVMValueRef onef = lp_build_const_float(gallivm, 1.0);
   LLVMValueRef onei = lp_build_const_int32(gallivm, 1);
   LLVMValueRef zeroi = lp_build_const_int32(gallivm, 0);
   LLVMValueRef pixel_center, xy0_center, dxy01, dxy20, dyx20;
   LLVMValueRef e, f, ef, ooa;
   LLVMValueRef shuffles[4], shuf10;
   LLVMValueRef attr_pos[3];
   LLVMValueRef polygon_offset;
   struct lp_type typef4 = lp_type_float_vec(32, 128);
   struct lp_build_context bld;

   lp_build_context_init(&bld, gallivm, typef4);
   args->bld = bld;

   /* The internal position input is in slot zero:
    */
   load_attribute(gallivm, args, key, 0, attr_pos);

   pixel_center = lp_build_const_vec(gallivm, typef4,
                                     (!key->multisample && key->pixel_center_half) ? 0.5 : 0.0);

   /*
    * xy are first two elems in v0a/v1a/v2a but just use vec4 arit
    * also offset_tri uses actually xyz in them
    */
   xy0_center = LLVMBuildFSub(b, attr_pos[0], pixel_center, "xy0_center" );

   dxy01 = LLVMBuildFSub(b, attr_pos[0], attr_pos[1], "dxy01");
   dxy20 = LLVMBuildFSub(b, attr_pos[2], attr_pos[0], "dxy20");

   shuffles[0] = onei;
   shuffles[1] = zeroi;
   shuffles[2] = LLVMGetUndef(shuf_type);
   shuffles[3] = LLVMGetUndef(shuf_type);
   shuf10 = LLVMConstVector(shuffles, 4);

   dyx20 = LLVMBuildShuffleVector(b, dxy20, dxy20, shuf10, "");

   ef = LLVMBuildFMul(b, dxy01, dyx20, "ef");
   e = LLVMBuildExtractElement(b, ef, zeroi, "");
   f = LLVMBuildExtractElement(b, ef, onei, "");

   ooa  = LLVMBuildFDiv(b, onef, LLVMBuildFSub(b, e, f, ""), "ooa");

   ooa = lp_build_broadcast_scalar(&bld, ooa);

   /* tri offset calc shares a lot of arithmetic, do it here */
   if (key->pgon_offset_scale != 0.0f || key->pgon_offset_units != 0.0f) {
      polygon_offset = lp_do_offset_tri(gallivm, args, key, ooa, dxy01, dxy20, attr_pos);
   } else {
      polygon_offset = lp_build_const_float(gallivm, 0.0f);
   }

   dxy20 = LLVMBuildFMul(b, dxy20, ooa, "");
   dxy01 = LLVMBuildFMul(b, dxy01, ooa, "");

   args->dy20_ooa  = lp_build_extract_broadcast(gallivm, typef4, typef4, dxy20, onei);
   args->dy01_ooa  = lp_build_extract_broadcast(gallivm, typef4, typef4, dxy01, onei);

   args->dx20_ooa  = lp_build_extract_broadcast(gallivm, typef4, typef4, dxy20, zeroi);
   args->dx01_ooa  = lp_build_extract_broadcast(gallivm, typef4, typef4, dxy01, zeroi);

   args->x0_center = lp_build_extract_broadcast(gallivm, typef4, typef4, xy0_center, zeroi);
   args->y0_center = lp_build_extract_broadcast(gallivm, typef4, typef4, xy0_center, onei);

   LLVMValueRef coeffs[3];
   calc_coef4(gallivm, args, attr_pos[0], attr_pos[1], attr_pos[2], coeffs);

   /* This is a bit sneaky:
    * Because we observe that the X component of A0 is otherwise unused,
    * we can overwrite it with the computed polygon-offset value, to make
    * sure it's available in the fragment shader without having to change
    * the interface (which is error-prone).
    */
   coeffs[0] = LLVMBuildInsertElement(b, coeffs[0], polygon_offset,
                                      lp_build_const_int32(gallivm, 0), "");

   store_coef(gallivm, args, 0, coeffs[0], coeffs[1], coeffs[2]);
}


/**
 * Generate the runtime callable function for the coefficient calculation.
 *
 */
static struct lp_setup_variant *
generate_setup_variant(struct lp_setup_variant_key *key,
                       struct llvmpipe_context *lp)
{
   int64_t t0 = 0, t1;

   if (0)
      goto fail;

   struct lp_setup_variant *variant = CALLOC_STRUCT(lp_setup_variant);
   if (!variant)
      goto fail;

   variant->no = setup_no++;

   char func_name[64];
   snprintf(func_name, sizeof(func_name), "setup_variant_%u",
            variant->no);

   struct gallivm_state *gallivm;
   variant->gallivm = gallivm = gallivm_create(func_name, lp->context, NULL);
   if (!variant->gallivm) {
      goto fail;
   }

   LLVMBuilderRef builder = gallivm->builder;

   if (LP_DEBUG & DEBUG_COUNTERS) {
      t0 = os_time_get();
   }

   memcpy(&variant->key, key, key->size);
   variant->list_item_global.base = variant;

   /* Currently always deal with full 4-wide vertex attributes from
    * the vertices.
    */

   LLVMTypeRef vec4f_type =
      LLVMVectorType(LLVMFloatTypeInContext(gallivm->context), 4);

   LLVMTypeRef arg_types[8];
   arg_types[0] = LLVMPointerType(vec4f_type, 0);        /* v0 */
   arg_types[1] = LLVMPointerType(vec4f_type, 0);        /* v1 */
   arg_types[2] = LLVMPointerType(vec4f_type, 0);        /* v2 */
   arg_types[3] = LLVMInt32TypeInContext(gallivm->context); /* facing */
   arg_types[4] = LLVMPointerType(vec4f_type, 0);	/* a0, aligned */
   arg_types[5] = LLVMPointerType(vec4f_type, 0);	/* dadx, aligned */
   arg_types[6] = LLVMPointerType(vec4f_type, 0);	/* dady, aligned */
   arg_types[7] = LLVMPointerType(vec4f_type, 0);	/* key (placeholder) */

   LLVMTypeRef func_type =
      LLVMFunctionType(LLVMVoidTypeInContext(gallivm->context),
                       arg_types, ARRAY_SIZE(arg_types), 0);

   variant->function = LLVMAddFunction(gallivm->module, func_name, func_type);
   if (!variant->function)
      goto fail;

   LLVMSetFunctionCallConv(variant->function, LLVMCCallConv);

   struct lp_setup_args args;
   args.vec4f_type = vec4f_type;
   args.v0       = LLVMGetParam(variant->function, 0);
   args.v1       = LLVMGetParam(variant->function, 1);
   args.v2       = LLVMGetParam(variant->function, 2);
   args.facing   = LLVMGetParam(variant->function, 3);
   args.a0       = LLVMGetParam(variant->function, 4);
   args.dadx     = LLVMGetParam(variant->function, 5);
   args.dady     = LLVMGetParam(variant->function, 6);
   args.key      = LLVMGetParam(variant->function, 7);

   lp_build_name(args.v0, "in_v0");
   lp_build_name(args.v1, "in_v1");
   lp_build_name(args.v2, "in_v2");
   lp_build_name(args.facing, "in_facing");
   lp_build_name(args.a0, "out_a0");
   lp_build_name(args.dadx, "out_dadx");
   lp_build_name(args.dady, "out_dady");
   lp_build_name(args.key, "key");

   /*
    * Function body
    */
   LLVMBasicBlockRef block =
      LLVMAppendBasicBlockInContext(gallivm->context,
                                    variant->function, "entry");
   LLVMPositionBuilderAtEnd(builder, block);

   set_noalias(builder, variant->function, arg_types, ARRAY_SIZE(arg_types));
   init_args(gallivm, &variant->key, &args);
   emit_tri_coef(gallivm, &variant->key, &args);

   LLVMBuildRetVoid(builder);

   gallivm_verify_function(gallivm, variant->function);

   gallivm_compile_module(gallivm);

   variant->jit_function = (lp_jit_setup_triangle)
      gallivm_jit_function(gallivm, variant->function);
   if (!variant->jit_function)
      goto fail;

   gallivm_free_ir(variant->gallivm);

   /*
    * Update timing information:
    */
   if (LP_DEBUG & DEBUG_COUNTERS) {
      t1 = os_time_get();
      LP_COUNT_ADD(llvm_compile_time, t1 - t0);
      LP_COUNT_ADD(nr_llvm_compiles, 1);
   }

   return variant;

fail:
   if (variant) {
      if (variant->gallivm) {
         gallivm_destroy(variant->gallivm);
      }
      FREE(variant);
   }

   return NULL;
}


static void
lp_make_setup_variant_key(const struct llvmpipe_context *lp,
                          struct lp_setup_variant_key *key)
{
   const struct lp_fragment_shader *fs = lp->fs;
   struct nir_shader *nir = fs->base.ir.nir;

   assert(sizeof key->inputs[0] == sizeof(uint));

   key->num_inputs = nir->num_inputs;
   key->flatshade_first = lp->rasterizer->flatshade_first;
   key->pixel_center_half = lp->rasterizer->half_pixel_center;
   key->multisample = lp->rasterizer->multisample;
   key->twoside = lp->rasterizer->light_twoside;
   key->size = offsetof(struct lp_setup_variant_key, inputs[key->num_inputs]);

   key->color_slot = lp->color_slot[0];
   key->bcolor_slot = lp->bcolor_slot[0];
   key->spec_slot = lp->color_slot[1];
   key->bspec_slot = lp->bcolor_slot[1];

   /*
    * If depth is floating point, depth bias is calculated with respect
    * to the primitive's maximum Z value. Retain the original depth bias
    * value until that stage.
    */
   key->floating_point_depth = lp->floating_point_depth;

   if (key->floating_point_depth) {
      key->pgon_offset_units = (float) lp->rasterizer->offset_units;
   } else {
      key->pgon_offset_units =
         (float) (lp->rasterizer->offset_units * lp->mrd * 2);
   }

   key->pgon_offset_scale = lp->rasterizer->offset_scale;
   key->pgon_offset_clamp = lp->rasterizer->offset_clamp;
   key->uses_constant_interp = 0;
   key->pad = 0;

   memcpy(key->inputs, fs->inputs, key->num_inputs * sizeof key->inputs[0]);

   for (unsigned i = 0; i < key->num_inputs; i++) {
      if (key->inputs[i].interp == LP_INTERP_COLOR) {
         if (lp->rasterizer->flatshade)
            key->inputs[i].interp = LP_INTERP_CONSTANT;
         else
            key->inputs[i].interp = LP_INTERP_PERSPECTIVE;
      }
      if (key->inputs[i].interp == LP_INTERP_CONSTANT) {
         key->uses_constant_interp = 1;
      }
   }
}


static void
remove_setup_variant(struct llvmpipe_context *lp,
                     struct lp_setup_variant *variant)
{
   if (gallivm_debug & GALLIVM_DEBUG_IR) {
      debug_printf("llvmpipe: del setup_variant #%u total %u\n",
                   variant->no, lp->nr_setup_variants);
   }

   if (variant->gallivm) {
      gallivm_destroy(variant->gallivm);
   }

   list_del(&variant->list_item_global.list);
   lp->nr_setup_variants--;
   FREE(variant);
}


/* When the number of setup variants exceeds a threshold, cull a
 * fraction (currently a quarter) of them.
 */
static void
cull_setup_variants(struct llvmpipe_context *lp)
{
   struct pipe_context *pipe = &lp->pipe;

   /*
    * XXX: we need to flush the context until we have some sort of reference
    * counting in fragment shaders as they may still be binned
    * Flushing alone might not be sufficient we need to wait on it too.
    */
   llvmpipe_finish(pipe, __func__);

   for (int i = 0; i < LP_MAX_SETUP_VARIANTS / 4; i++) {
      struct lp_setup_variant_list_item *item;
      if (list_is_empty(&lp->setup_variants_list.list)) {
         break;
      }
      item = list_last_entry(&lp->setup_variants_list.list,
                             struct lp_setup_variant_list_item, list);
      assert(item);
      assert(item->base);
      remove_setup_variant(lp, item->base);
   }
}


/**
 * Update fragment/vertex shader linkage state.  This is called just
 * prior to drawing something when some fragment-related state has
 * changed.
 */
void
llvmpipe_update_setup(struct llvmpipe_context *lp)
{
   struct lp_setup_variant_key *key = &lp->setup_variant.key;
   struct lp_setup_variant *variant = NULL;
   struct lp_setup_variant_list_item *li;

   lp_make_setup_variant_key(lp, key);

   LIST_FOR_EACH_ENTRY(li, &lp->setup_variants_list.list, list) {
      if (li->base->key.size == key->size &&
         memcmp(&li->base->key, key, key->size) == 0) {
         variant = li->base;
         break;
      }
   }

   if (variant) {
      list_move_to(&variant->list_item_global.list, &lp->setup_variants_list.list);
   } else {
      if (lp->nr_setup_variants >= LP_MAX_SETUP_VARIANTS) {
         cull_setup_variants(lp);
      }

      variant = generate_setup_variant(key, lp);
      if (variant) {
         list_add(&variant->list_item_global.list, &lp->setup_variants_list.list);
         lp->nr_setup_variants++;
      }
   }

   lp_setup_set_setup_variant(lp->setup, variant);
}


void
lp_delete_setup_variants(struct llvmpipe_context *lp)
{
   struct lp_setup_variant_list_item *li, *next;
   LIST_FOR_EACH_ENTRY_SAFE(li, next, &lp->setup_variants_list.list, list) {
      remove_setup_variant(lp, li->base);
   }
}


void
lp_dump_setup_coef(const struct lp_setup_variant_key *key,
                   const float (*sa0)[4],
                   const float (*sdadx)[4],
                   const float (*sdady)[4])
{
   for (int i = 0; i < TGSI_NUM_CHANNELS; i++) {
      float a0   = sa0  [0][i];
      float dadx = sdadx[0][i];
      float dady = sdady[0][i];

      debug_printf("POS.%c: a0 = %f, dadx = %f, dady = %f\n",
                   "xyzw"[i], a0, dadx, dady);
   }

   for (int slot = 0; slot < key->num_inputs; slot++) {
      unsigned usage_mask = key->inputs[slot].usage_mask;
      for (int i = 0; i < TGSI_NUM_CHANNELS; i++) {
         if (usage_mask & (1 << i)) {
            float a0   = sa0  [1 + slot][i];
            float dadx = sdadx[1 + slot][i];
            float dady = sdady[1 + slot][i];

            debug_printf("IN[%u].%c: a0 = %f, dadx = %f, dady = %f\n",
                         slot, "xyzw"[i], a0, dadx, dady);
         }
      }
   }
}
