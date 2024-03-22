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

/**
 * @file
 * Texture sampling -- common code.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_cpu_detect.h"
#include "lp_bld_arit.h"
#include "lp_bld_const.h"
#include "lp_bld_debug.h"
#include "lp_bld_printf.h"
#include "lp_bld_flow.h"
#include "lp_bld_sample.h"
#include "lp_bld_swizzle.h"
#include "lp_bld_type.h"
#include "lp_bld_logic.h"
#include "lp_bld_pack.h"
#include "lp_bld_quad.h"
#include "lp_bld_bitarit.h"


/*
 * Bri-linear factor. Should be greater than one.
 */
#define BRILINEAR_FACTOR 2


/**
 * Does the given texture wrap mode allow sampling the texture border color?
 * XXX maybe move this into gallium util code.
 */
bool
lp_sampler_wrap_mode_uses_border_color(enum pipe_tex_wrap mode,
                                       enum pipe_tex_filter min_img_filter,
                                       enum pipe_tex_filter mag_img_filter)
{
   switch (mode) {
   case PIPE_TEX_WRAP_REPEAT:
   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
   case PIPE_TEX_WRAP_MIRROR_REPEAT:
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
      return false;
   case PIPE_TEX_WRAP_CLAMP:
   case PIPE_TEX_WRAP_MIRROR_CLAMP:
      if (min_img_filter == PIPE_TEX_FILTER_NEAREST &&
          mag_img_filter == PIPE_TEX_FILTER_NEAREST) {
         return false;
      } else {
         return true;
      }
   case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
      return true;
   default:
      assert(0 && "unexpected wrap mode");
      return false;
   }
}


/**
 * Initialize lp_sampler_static_texture_state object with the gallium
 * texture/sampler_view state (this contains the parts which are
 * considered static).
 */
void
lp_sampler_static_texture_state(struct lp_static_texture_state *state,
                                const struct pipe_sampler_view *view)
{
   memset(state, 0, sizeof *state);

   if (!view || !view->texture)
      return;

   const struct pipe_resource *texture = view->texture;

   state->format = view->format;
   state->res_format = texture->format;
   state->swizzle_r = view->swizzle_r;
   state->swizzle_g = view->swizzle_g;
   state->swizzle_b = view->swizzle_b;
   state->swizzle_a = view->swizzle_a;
   assert(state->swizzle_r < PIPE_SWIZZLE_NONE);
   assert(state->swizzle_g < PIPE_SWIZZLE_NONE);
   assert(state->swizzle_b < PIPE_SWIZZLE_NONE);
   assert(state->swizzle_a < PIPE_SWIZZLE_NONE);

   /* check if it is a tex2d created from buf */
   if (view->is_tex2d_from_buf)
      state->target = PIPE_TEXTURE_2D;
   else
      state->target = view->target;

   state->pot_width = util_is_power_of_two_or_zero(texture->width0);
   state->pot_height = util_is_power_of_two_or_zero(texture->height0);
   state->pot_depth = util_is_power_of_two_or_zero(texture->depth0);
   state->level_zero_only = !view->u.tex.last_level;

   /*
    * the layer / element / level parameters are all either dynamic
    * state or handled transparently wrt execution.
    */
}


/**
 * Initialize lp_sampler_static_texture_state object with the gallium
 * texture/sampler_view state (this contains the parts which are
 * considered static).
 */
void
lp_sampler_static_texture_state_image(struct lp_static_texture_state *state,
                                      const struct pipe_image_view *view)
{
   memset(state, 0, sizeof *state);

   if (!view || !view->resource)
      return;

   const struct pipe_resource *resource = view->resource;

   state->format = view->format;
   state->res_format = resource->format;
   state->swizzle_r = PIPE_SWIZZLE_X;
   state->swizzle_g = PIPE_SWIZZLE_Y;
   state->swizzle_b = PIPE_SWIZZLE_Z;
   state->swizzle_a = PIPE_SWIZZLE_W;
   assert(state->swizzle_r < PIPE_SWIZZLE_NONE);
   assert(state->swizzle_g < PIPE_SWIZZLE_NONE);
   assert(state->swizzle_b < PIPE_SWIZZLE_NONE);
   assert(state->swizzle_a < PIPE_SWIZZLE_NONE);

   state->target = resource->target;
   state->pot_width = util_is_power_of_two_or_zero(resource->width0);
   state->pot_height = util_is_power_of_two_or_zero(resource->height0);
   state->pot_depth = util_is_power_of_two_or_zero(resource->depth0);
   state->level_zero_only = view->u.tex.level == 0;

   /*
    * the layer / element / level parameters are all either dynamic
    * state or handled transparently wrt execution.
    */
}


/**
 * Initialize lp_sampler_static_sampler_state object with the gallium sampler
 * state (this contains the parts which are considered static).
 */
void
lp_sampler_static_sampler_state(struct lp_static_sampler_state *state,
                                const struct pipe_sampler_state *sampler)
{
   memset(state, 0, sizeof *state);

   if (!sampler)
      return;

   /*
    * We don't copy sampler state over unless it is actually enabled, to avoid
    * spurious recompiles, as the sampler static state is part of the shader
    * key.
    *
    * Ideally gallium frontends or cso_cache module would make all state
    * canonical, but until that happens it's better to be safe than sorry here.
    *
    * XXX: Actually there's much more than can be done here, especially
    * regarding 1D/2D/3D/CUBE textures, wrap modes, etc.
    */

   state->wrap_s            = sampler->wrap_s;
   state->wrap_t            = sampler->wrap_t;
   state->wrap_r            = sampler->wrap_r;
   state->min_img_filter    = sampler->min_img_filter;
   state->mag_img_filter    = sampler->mag_img_filter;
   state->min_mip_filter    = sampler->min_mip_filter;
   state->seamless_cube_map = sampler->seamless_cube_map;
   state->reduction_mode    = sampler->reduction_mode;
   state->aniso = sampler->max_anisotropy > 1.0f;

   if (sampler->max_lod > 0.0f) {
      state->max_lod_pos = 1;
   }

   if (sampler->lod_bias != 0.0f) {
      state->lod_bias_non_zero = 1;
   }

   if (state->min_mip_filter != PIPE_TEX_MIPFILTER_NONE ||
       state->min_img_filter != state->mag_img_filter) {

      /* If min_lod == max_lod we can greatly simplify mipmap selection.
       * This is a case that occurs during automatic mipmap generation.
       */
      if (sampler->min_lod == sampler->max_lod) {
         state->min_max_lod_equal = 1;
      } else {
         if (sampler->min_lod > 0.0f) {
            state->apply_min_lod = 1;
         }

         /*
          * XXX this won't do anything with the mesa state tracker which always
          * sets max_lod to not more than actually present mip maps...
          */
         if (sampler->max_lod < (PIPE_MAX_TEXTURE_LEVELS - 1)) {
            state->apply_max_lod = 1;
         }
      }
   }

   state->compare_mode      = sampler->compare_mode;
   if (sampler->compare_mode != PIPE_TEX_COMPARE_NONE) {
      state->compare_func   = sampler->compare_func;
   }

   state->normalized_coords = !sampler->unnormalized_coords;
}


/* build aniso pmin value */
static LLVMValueRef
lp_build_pmin(struct lp_build_sample_context *bld,
              LLVMValueRef first_level,
              LLVMValueRef s,
              LLVMValueRef t,
              LLVMValueRef max_aniso)
{
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMBuilderRef builder = bld->gallivm->builder;
   struct lp_build_context *coord_bld = &bld->coord_bld;
   struct lp_build_context *int_size_bld = &bld->int_size_in_bld;
   struct lp_build_context *float_size_bld = &bld->float_size_in_bld;
   struct lp_build_context *pmin_bld = &bld->lodf_bld;
   LLVMTypeRef i32t = LLVMInt32TypeInContext(bld->gallivm->context);
   LLVMValueRef index0 = LLVMConstInt(i32t, 0, 0);
   LLVMValueRef index1 = LLVMConstInt(i32t, 1, 0);
   LLVMValueRef ddx_ddy = lp_build_packed_ddx_ddy_twocoord(coord_bld, s, t);
   LLVMValueRef int_size, float_size;
   const unsigned length = coord_bld->type.length;
   const unsigned num_quads = length / 4;
   const bool pmin_per_quad = pmin_bld->type.length != length;

   int_size = lp_build_minify(int_size_bld, bld->int_size, first_level, true);
   float_size = lp_build_int_to_float(float_size_bld, int_size);
   max_aniso = lp_build_broadcast_scalar(coord_bld, max_aniso);
   max_aniso = lp_build_mul(coord_bld, max_aniso, max_aniso);

   static const unsigned char swizzle01[] = { /* no-op swizzle */
      0, 1,
      LP_BLD_SWIZZLE_DONTCARE, LP_BLD_SWIZZLE_DONTCARE
   };
   static const unsigned char swizzle23[] = {
      2, 3,
      LP_BLD_SWIZZLE_DONTCARE, LP_BLD_SWIZZLE_DONTCARE
   };
   LLVMValueRef ddx_ddys, ddx_ddyt, floatdim, shuffles[LP_MAX_VECTOR_LENGTH / 4];

   for (unsigned i = 0; i < num_quads; i++) {
      shuffles[i*4+0] = shuffles[i*4+1] = index0;
      shuffles[i*4+2] = shuffles[i*4+3] = index1;
   }
   floatdim = LLVMBuildShuffleVector(builder, float_size, float_size,
                                     LLVMConstVector(shuffles, length), "");
   ddx_ddy = lp_build_mul(coord_bld, ddx_ddy, floatdim);

   ddx_ddy = lp_build_mul(coord_bld, ddx_ddy, ddx_ddy);

   ddx_ddys = lp_build_swizzle_aos(coord_bld, ddx_ddy, swizzle01);
   ddx_ddyt = lp_build_swizzle_aos(coord_bld, ddx_ddy, swizzle23);

   LLVMValueRef px2_py2 = lp_build_add(coord_bld, ddx_ddys, ddx_ddyt);

   static const unsigned char swizzle0[] = { /* no-op swizzle */
     0, LP_BLD_SWIZZLE_DONTCARE,
     LP_BLD_SWIZZLE_DONTCARE, LP_BLD_SWIZZLE_DONTCARE
   };
   static const unsigned char swizzle1[] = {
     1, LP_BLD_SWIZZLE_DONTCARE,
     LP_BLD_SWIZZLE_DONTCARE, LP_BLD_SWIZZLE_DONTCARE
   };
   LLVMValueRef px2 = lp_build_swizzle_aos(coord_bld, px2_py2, swizzle0);
   LLVMValueRef py2 = lp_build_swizzle_aos(coord_bld, px2_py2, swizzle1);

   LLVMValueRef pmax2 = lp_build_max(coord_bld, px2, py2);
   LLVMValueRef pmin2 = lp_build_min(coord_bld, px2, py2);

   LLVMValueRef temp = lp_build_mul(coord_bld, pmin2, max_aniso);

   LLVMValueRef comp = lp_build_compare(gallivm, coord_bld->type, PIPE_FUNC_GREATER,
                                        pmin2, temp);

   LLVMValueRef pmin2_alt = lp_build_div(coord_bld, pmax2, max_aniso);

   pmin2 = lp_build_select(coord_bld, comp, pmin2_alt, pmin2);

   if (pmin_per_quad)
      pmin2 = lp_build_pack_aos_scalars(bld->gallivm, coord_bld->type,
                                        pmin_bld->type, pmin2, 0);
   else
      pmin2 = lp_build_swizzle_scalar_aos(pmin_bld, pmin2, 0, 4);
   return pmin2;
}


/**
 * Generate code to compute coordinate gradient (rho).
 * \param derivs  partial derivatives of (s, t, r, q) with respect to X and Y
 *
 * The resulting rho has bld->levelf format (per quad or per element).
 */
static LLVMValueRef
lp_build_rho(struct lp_build_sample_context *bld,
             LLVMValueRef first_level,
             LLVMValueRef s,
             LLVMValueRef t,
             LLVMValueRef r,
             const struct lp_derivatives *derivs)
{
   struct gallivm_state *gallivm = bld->gallivm;
   struct lp_build_context *int_size_bld = &bld->int_size_in_bld;
   struct lp_build_context *float_size_bld = &bld->float_size_in_bld;
   struct lp_build_context *float_bld = &bld->float_bld;
   struct lp_build_context *coord_bld = &bld->coord_bld;
   struct lp_build_context *rho_bld = &bld->lodf_bld;
   const unsigned dims = bld->dims;
   LLVMValueRef ddx_ddy[2] = {NULL};
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMTypeRef i32t = LLVMInt32TypeInContext(bld->gallivm->context);
   LLVMValueRef index0 = LLVMConstInt(i32t, 0, 0);
   LLVMValueRef index1 = LLVMConstInt(i32t, 1, 0);
   LLVMValueRef index2 = LLVMConstInt(i32t, 2, 0);
   LLVMValueRef rho_vec;
   LLVMValueRef rho;
   unsigned length = coord_bld->type.length;
   unsigned num_quads = length / 4;
   bool rho_per_quad = rho_bld->type.length != length;
   bool no_rho_opt = bld->no_rho_approx && (dims > 1);
   LLVMValueRef i32undef = LLVMGetUndef(LLVMInt32TypeInContext(gallivm->context));
   LLVMValueRef rho_xvec, rho_yvec;

   /* Note that all simplified calculations will only work for isotropic
    * filtering
    */

   /*
    * rho calcs are always per quad except for explicit derivs (excluding
    * the messy cube maps for now) when requested.
    */

   LLVMValueRef int_size =
      lp_build_minify(int_size_bld, bld->int_size, first_level, true);
   LLVMValueRef float_size = lp_build_int_to_float(float_size_bld, int_size);

   if (derivs) {
      LLVMValueRef ddmax[3] = { NULL }, ddx[3] = { NULL }, ddy[3] = { NULL };
      for (unsigned i = 0; i < dims; i++) {
         LLVMValueRef indexi = lp_build_const_int32(gallivm, i);

         LLVMValueRef floatdim =
            lp_build_extract_broadcast(gallivm, bld->float_size_in_type,
                                       coord_bld->type, float_size, indexi);

         /*
          * note that for rho_per_quad case could reduce math (at some shuffle
          * cost), but for now use same code to per-pixel lod case.
          */
         if (no_rho_opt) {
            ddx[i] = lp_build_mul(coord_bld, floatdim, derivs->ddx[i]);
            ddy[i] = lp_build_mul(coord_bld, floatdim, derivs->ddy[i]);
            ddx[i] = lp_build_mul(coord_bld, ddx[i], ddx[i]);
            ddy[i] = lp_build_mul(coord_bld, ddy[i], ddy[i]);
         } else {
            LLVMValueRef tmpx = lp_build_abs(coord_bld, derivs->ddx[i]);
            LLVMValueRef tmpy = lp_build_abs(coord_bld, derivs->ddy[i]);
            ddmax[i] = lp_build_max(coord_bld, tmpx, tmpy);
            ddmax[i] = lp_build_mul(coord_bld, floatdim, ddmax[i]);
         }
      }
      if (no_rho_opt) {
         rho_xvec = lp_build_add(coord_bld, ddx[0], ddx[1]);
         rho_yvec = lp_build_add(coord_bld, ddy[0], ddy[1]);
         if (dims > 2) {
            rho_xvec = lp_build_add(coord_bld, rho_xvec, ddx[2]);
            rho_yvec = lp_build_add(coord_bld, rho_yvec, ddy[2]);
         }
         rho = lp_build_max(coord_bld, rho_xvec, rho_yvec);
         /* skipping sqrt hence returning rho squared */
      } else {
         rho = ddmax[0];
         if (dims > 1) {
            rho = lp_build_max(coord_bld, rho, ddmax[1]);
            if (dims > 2) {
               rho = lp_build_max(coord_bld, rho, ddmax[2]);
            }
         }
      }

      LLVMValueRef rho_is_inf = lp_build_is_inf_or_nan(gallivm,
                                                       coord_bld->type, rho);
      rho = lp_build_select(coord_bld, rho_is_inf, coord_bld->zero, rho);

      if (rho_per_quad) {
         /*
          * rho_vec contains per-pixel rho, convert to scalar per quad.
          */
         rho = lp_build_pack_aos_scalars(bld->gallivm, coord_bld->type,
                                         rho_bld->type, rho, 0);
      }
   } else {
      /*
       * This looks all a bit complex, but it's not that bad
       * (the shuffle code makes it look worse than it is).
       * Still, might not be ideal for all cases.
       */
      static const unsigned char swizzle0[] = { /* no-op swizzle */
         0, LP_BLD_SWIZZLE_DONTCARE,
         LP_BLD_SWIZZLE_DONTCARE, LP_BLD_SWIZZLE_DONTCARE
      };
      static const unsigned char swizzle1[] = {
         1, LP_BLD_SWIZZLE_DONTCARE,
         LP_BLD_SWIZZLE_DONTCARE, LP_BLD_SWIZZLE_DONTCARE
      };
      static const unsigned char swizzle2[] = {
         2, LP_BLD_SWIZZLE_DONTCARE,
         LP_BLD_SWIZZLE_DONTCARE, LP_BLD_SWIZZLE_DONTCARE
      };

      if (dims < 2) {
         ddx_ddy[0] = lp_build_packed_ddx_ddy_onecoord(coord_bld, s);
      } else if (dims >= 2) {
         ddx_ddy[0] = lp_build_packed_ddx_ddy_twocoord(coord_bld, s, t);
         if (dims > 2) {
            ddx_ddy[1] = lp_build_packed_ddx_ddy_onecoord(coord_bld, r);
         }
      }

      if (no_rho_opt) {
         static const unsigned char swizzle01[] = { /* no-op swizzle */
            0, 1,
            LP_BLD_SWIZZLE_DONTCARE, LP_BLD_SWIZZLE_DONTCARE
         };
         static const unsigned char swizzle23[] = {
            2, 3,
            LP_BLD_SWIZZLE_DONTCARE, LP_BLD_SWIZZLE_DONTCARE
         };
         LLVMValueRef ddx_ddys, ddx_ddyt, floatdim;
         LLVMValueRef shuffles[LP_MAX_VECTOR_LENGTH / 4];

         for (unsigned i = 0; i < num_quads; i++) {
            shuffles[i*4+0] = shuffles[i*4+1] = index0;
            shuffles[i*4+2] = shuffles[i*4+3] = index1;
         }
         floatdim = LLVMBuildShuffleVector(builder, float_size, float_size,
                                           LLVMConstVector(shuffles, length),
                                           "");
         ddx_ddy[0] = lp_build_mul(coord_bld, ddx_ddy[0], floatdim);
         ddx_ddy[0] = lp_build_mul(coord_bld, ddx_ddy[0], ddx_ddy[0]);
         ddx_ddys = lp_build_swizzle_aos(coord_bld, ddx_ddy[0], swizzle01);
         ddx_ddyt = lp_build_swizzle_aos(coord_bld, ddx_ddy[0], swizzle23);
         rho_vec = lp_build_add(coord_bld, ddx_ddys, ddx_ddyt);

         if (dims > 2) {
            static const unsigned char swizzle02[] = {
               0, 2,
               LP_BLD_SWIZZLE_DONTCARE, LP_BLD_SWIZZLE_DONTCARE
            };
            floatdim = lp_build_extract_broadcast(gallivm, bld->float_size_in_type,
                                                  coord_bld->type, float_size, index2);
            ddx_ddy[1] = lp_build_mul(coord_bld, ddx_ddy[1], floatdim);
            ddx_ddy[1] = lp_build_mul(coord_bld, ddx_ddy[1], ddx_ddy[1]);
            ddx_ddy[1] = lp_build_swizzle_aos(coord_bld, ddx_ddy[1], swizzle02);
            rho_vec = lp_build_add(coord_bld, rho_vec, ddx_ddy[1]);
         }

         rho_xvec = lp_build_swizzle_aos(coord_bld, rho_vec, swizzle0);
         rho_yvec = lp_build_swizzle_aos(coord_bld, rho_vec, swizzle1);
         rho = lp_build_max(coord_bld, rho_xvec, rho_yvec);

         if (rho_per_quad) {
            rho = lp_build_pack_aos_scalars(bld->gallivm, coord_bld->type,
                                            rho_bld->type, rho, 0);
         } else {
            rho = lp_build_swizzle_scalar_aos(coord_bld, rho, 0, 4);
         }
         /* skipping sqrt hence returning rho squared */
      } else {
         ddx_ddy[0] = lp_build_abs(coord_bld, ddx_ddy[0]);
         if (dims > 2) {
            ddx_ddy[1] = lp_build_abs(coord_bld, ddx_ddy[1]);
         } else {
            ddx_ddy[1] = NULL; /* silence compiler warning */
         }

         if (dims < 2) {
            rho_xvec = lp_build_swizzle_aos(coord_bld, ddx_ddy[0], swizzle0);
            rho_yvec = lp_build_swizzle_aos(coord_bld, ddx_ddy[0], swizzle2);
         } else if (dims == 2) {
            static const unsigned char swizzle02[] = {
               0, 2,
               LP_BLD_SWIZZLE_DONTCARE, LP_BLD_SWIZZLE_DONTCARE
            };
            static const unsigned char swizzle13[] = {
               1, 3,
               LP_BLD_SWIZZLE_DONTCARE, LP_BLD_SWIZZLE_DONTCARE
            };
            rho_xvec = lp_build_swizzle_aos(coord_bld, ddx_ddy[0], swizzle02);
            rho_yvec = lp_build_swizzle_aos(coord_bld, ddx_ddy[0], swizzle13);
         } else {
            LLVMValueRef shuffles1[LP_MAX_VECTOR_LENGTH];
            LLVMValueRef shuffles2[LP_MAX_VECTOR_LENGTH];
            assert(dims == 3);
            for (unsigned i = 0; i < num_quads; i++) {
               shuffles1[4*i + 0] = lp_build_const_int32(gallivm, 4*i);
               shuffles1[4*i + 1] = lp_build_const_int32(gallivm, 4*i + 2);
               shuffles1[4*i + 2] = lp_build_const_int32(gallivm, length + 4*i);
               shuffles1[4*i + 3] = i32undef;
               shuffles2[4*i + 0] = lp_build_const_int32(gallivm, 4*i + 1);
               shuffles2[4*i + 1] = lp_build_const_int32(gallivm, 4*i + 3);
               shuffles2[4*i + 2] = lp_build_const_int32(gallivm, length + 4*i + 2);
               shuffles2[4*i + 3] = i32undef;
            }
            rho_xvec = LLVMBuildShuffleVector(builder, ddx_ddy[0], ddx_ddy[1],
                                              LLVMConstVector(shuffles1, length), "");
            rho_yvec = LLVMBuildShuffleVector(builder, ddx_ddy[0], ddx_ddy[1],
                                              LLVMConstVector(shuffles2, length), "");
         }

         rho_vec = lp_build_max(coord_bld, rho_xvec, rho_yvec);

         if (bld->coord_type.length > 4) {
            /* expand size to each quad */
            if (dims > 1) {
               /* could use some broadcast_vector helper for this? */
               LLVMValueRef src[LP_MAX_VECTOR_LENGTH/4];
               for (unsigned i = 0; i < num_quads; i++) {
                  src[i] = float_size;
               }
               float_size = lp_build_concat(bld->gallivm, src,
                                            float_size_bld->type, num_quads);
            } else {
               float_size = lp_build_broadcast_scalar(coord_bld, float_size);
            }
            rho_vec = lp_build_mul(coord_bld, rho_vec, float_size);

            if (dims <= 1) {
               rho = rho_vec;
            } else {
               if (dims >= 2) {
                  LLVMValueRef rho_s, rho_t, rho_r;

                  rho_s = lp_build_swizzle_aos(coord_bld, rho_vec, swizzle0);
                  rho_t = lp_build_swizzle_aos(coord_bld, rho_vec, swizzle1);

                  rho = lp_build_max(coord_bld, rho_s, rho_t);

                  if (dims >= 3) {
                     rho_r = lp_build_swizzle_aos(coord_bld, rho_vec, swizzle2);
                     rho = lp_build_max(coord_bld, rho, rho_r);
                  }
               }
            }
            if (rho_per_quad) {
               rho = lp_build_pack_aos_scalars(bld->gallivm, coord_bld->type,
                                               rho_bld->type, rho, 0);
            } else {
               rho = lp_build_swizzle_scalar_aos(coord_bld, rho, 0, 4);
            }
         } else {
            if (dims <= 1) {
               rho_vec = LLVMBuildExtractElement(builder, rho_vec, index0, "");
            }
            rho_vec = lp_build_mul(float_size_bld, rho_vec, float_size);

            if (dims <= 1) {
               rho = rho_vec;
            } else {
               if (dims >= 2) {
                  LLVMValueRef rho_s, rho_t, rho_r;

                  rho_s = LLVMBuildExtractElement(builder, rho_vec, index0, "");
                  rho_t = LLVMBuildExtractElement(builder, rho_vec, index1, "");

                  rho = lp_build_max(float_bld, rho_s, rho_t);

                  if (dims >= 3) {
                     rho_r = LLVMBuildExtractElement(builder, rho_vec, index2, "");
                     rho = lp_build_max(float_bld, rho, rho_r);
                  }
               }
            }
            if (!rho_per_quad) {
               rho = lp_build_broadcast_scalar(rho_bld, rho);
            }
         }
      }
   }

   return rho;
}


/*
 * Bri-linear lod computation
 *
 * Use a piece-wise linear approximation of log2 such that:
 * - round to nearest, for values in the neighborhood of -1, 0, 1, 2, etc.
 * - linear approximation for values in the neighborhood of 0.5, 1.5., etc,
 *   with the steepness specified in 'factor'
 * - exact result for 0.5, 1.5, etc.
 *
 *
 *   1.0 -              /----*
 *                     /
 *                    /
 *                   /
 *   0.5 -          *
 *                 /
 *                /
 *               /
 *   0.0 - *----/
 *
 *         |                 |
 *        2^0               2^1
 *
 * This is a technique also commonly used in hardware:
 * - http://ixbtlabs.com/articles2/gffx/nv40-rx800-3.html
 *
 * TODO: For correctness, this should only be applied when texture is known to
 * have regular mipmaps, i.e., mipmaps derived from the base level.
 *
 * TODO: This could be done in fixed point, where applicable.
 */
static void
lp_build_brilinear_lod(struct lp_build_context *bld,
                       LLVMValueRef lod,
                       double factor,
                       LLVMValueRef *out_lod_ipart,
                       LLVMValueRef *out_lod_fpart)
{
   LLVMValueRef lod_fpart;
   double pre_offset = (factor - 0.5)/factor - 0.5;
   double post_offset = 1 - factor;

   if (0) {
      lp_build_printf(bld->gallivm, "lod = %f\n", lod);
   }

   lod = lp_build_add(bld, lod,
                      lp_build_const_vec(bld->gallivm, bld->type, pre_offset));

   lp_build_ifloor_fract(bld, lod, out_lod_ipart, &lod_fpart);

   lod_fpart = lp_build_mad(bld, lod_fpart,
                            lp_build_const_vec(bld->gallivm, bld->type, factor),
                            lp_build_const_vec(bld->gallivm, bld->type, post_offset));

   /*
    * It's not necessary to clamp lod_fpart since:
    * - the above expression will never produce numbers greater than one.
    * - the mip filtering branch is only taken if lod_fpart is positive
    */

   *out_lod_fpart = lod_fpart;

   if (0) {
      lp_build_printf(bld->gallivm, "lod_ipart = %i\n", *out_lod_ipart);
      lp_build_printf(bld->gallivm, "lod_fpart = %f\n\n", *out_lod_fpart);
   }
}


/*
 * Combined log2 and brilinear lod computation.
 *
 * It's in all identical to calling lp_build_fast_log2() and
 * lp_build_brilinear_lod() above, but by combining we can compute the integer
 * and fractional part independently.
 */
static void
lp_build_brilinear_rho(struct lp_build_context *bld,
                       LLVMValueRef rho,
                       double factor,
                       LLVMValueRef *out_lod_ipart,
                       LLVMValueRef *out_lod_fpart)
{
   const double pre_factor = (2*factor - 0.5)/(M_SQRT2*factor);
   const double post_offset = 1 - 2*factor;

   assert(bld->type.floating);

   assert(lp_check_value(bld->type, rho));

   /*
    * The pre factor will make the intersections with the exact powers of two
    * happen precisely where we want them to be, which means that the integer
    * part will not need any post adjustments.
    */
   rho = lp_build_mul(bld, rho,
                      lp_build_const_vec(bld->gallivm, bld->type, pre_factor));

   /* ipart = ifloor(log2(rho)) */
   LLVMValueRef lod_ipart = lp_build_extract_exponent(bld, rho, 0);

   /* fpart = rho / 2**ipart */
   LLVMValueRef lod_fpart = lp_build_extract_mantissa(bld, rho);

   lod_fpart =
      lp_build_mad(bld, lod_fpart,
                   lp_build_const_vec(bld->gallivm, bld->type, factor),
                   lp_build_const_vec(bld->gallivm, bld->type, post_offset));

   /*
    * Like lp_build_brilinear_lod, it's not necessary to clamp lod_fpart since:
    * - the above expression will never produce numbers greater than one.
    * - the mip filtering branch is only taken if lod_fpart is positive
    */

   *out_lod_ipart = lod_ipart;
   *out_lod_fpart = lod_fpart;
}


/**
 * Fast implementation of iround(log2(sqrt(x))), based on
 * log2(x^n) == n*log2(x).
 *
 * Gives accurate results all the time.
 * (Could be trivially extended to handle other power-of-two roots.)
 */
static LLVMValueRef
lp_build_ilog2_sqrt(struct lp_build_context *bld,
                    LLVMValueRef x)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   struct lp_type i_type = lp_int_type(bld->type);
   LLVMValueRef one = lp_build_const_int_vec(bld->gallivm, i_type, 1);

   assert(bld->type.floating);

   assert(lp_check_value(bld->type, x));

   /* ipart = log2(x) + 0.5 = 0.5*(log2(x^2) + 1.0) */
   LLVMValueRef ipart = lp_build_extract_exponent(bld, x, 1);
   ipart = LLVMBuildAShr(builder, ipart, one, "");

   return ipart;
}


/**
 * Generate code to compute texture level of detail (lambda).
 * \param derivs  partial derivatives of (s, t, r, q) with respect to X and Y
 * \param lod_bias  optional float vector with the shader lod bias
 * \param explicit_lod  optional float vector with the explicit lod
 * \param out_lod_ipart  integer part of lod
 * \param out_lod_fpart  float part of lod (never larger than 1 but may be negative)
 * \param out_lod_positive  (mask) if lod is positive (i.e. texture is minified)
 *
 * The resulting lod can be scalar per quad or be per element.
 */
void
lp_build_lod_selector(struct lp_build_sample_context *bld,
                      bool is_lodq,
                      unsigned sampler_unit,
                      LLVMValueRef first_level,
                      LLVMValueRef s,
                      LLVMValueRef t,
                      LLVMValueRef r,
                      const struct lp_derivatives *derivs,
                      LLVMValueRef lod_bias, /* optional */
                      LLVMValueRef explicit_lod, /* optional */
                      enum pipe_tex_mipfilter mip_filter,
                      LLVMValueRef max_aniso,
                      LLVMValueRef *out_lod,
                      LLVMValueRef *out_lod_ipart,
                      LLVMValueRef *out_lod_fpart,
                      LLVMValueRef *out_lod_positive)

{
   LLVMBuilderRef builder = bld->gallivm->builder;
   struct lp_sampler_dynamic_state *dynamic_state = bld->dynamic_state;
   struct lp_build_context *lodf_bld = &bld->lodf_bld;
   LLVMValueRef lod;

   *out_lod_ipart = bld->lodi_bld.zero;
   *out_lod_positive = bld->lodi_bld.zero;
   *out_lod_fpart = lodf_bld->zero;

   /*
    * For determining min/mag, we follow GL 4.1 spec, 3.9.12 Texture
    * Magnification: "Implementations may either unconditionally assume c = 0
    * for the minification vs. magnification switch-over point, or may choose
    * to make c depend on the combination of minification and magnification
    * modes as follows: if the magnification filter is given by LINEAR and the
    * minification filter is given by NEAREST_MIPMAP_NEAREST or
    * NEAREST_MIPMAP_LINEAR, then c = 0.5. This is done to ensure that a
    * minified texture does not appear "sharper" than a magnified
    * texture. Otherwise c = 0."  And 3.9.11 Texture Minification: "If lod is
    * less than or equal to the constant c (see section 3.9.12) the texture is
    * said to be magnified; if it is greater, the texture is minified."  So,
    * using 0 as switchover point always, and using magnification for lod ==
    * 0.  Note that the always c = 0 behavior is new (first appearing in GL
    * 3.1 spec), old GL versions required 0.5 for the modes listed above.  I
    * have no clue about the (undocumented) wishes of d3d9/d3d10 here!
    */

   if (bld->static_sampler_state->min_max_lod_equal && !is_lodq) {
      /* User is forcing sampling from a particular mipmap level.
       * This is hit during mipmap generation.
       */
      LLVMValueRef min_lod =
         dynamic_state->min_lod(bld->gallivm, bld->resources_type,
                                bld->resources_ptr, sampler_unit);

      lod = lp_build_broadcast_scalar(lodf_bld, min_lod);
   } else {
      if (explicit_lod) {
         if (bld->num_lods != bld->coord_type.length)
            lod = lp_build_pack_aos_scalars(bld->gallivm, bld->coord_bld.type,
                                            lodf_bld->type, explicit_lod, 0);
         else
            lod = explicit_lod;
      } else {
         LLVMValueRef rho;
         bool rho_squared = bld->no_rho_approx && (bld->dims > 1);

         if (bld->static_sampler_state->aniso &&
             !explicit_lod) {
            rho = lp_build_pmin(bld, first_level, s, t, max_aniso);
            rho_squared = true;
         } else {
            rho = lp_build_rho(bld, first_level, s, t, r, derivs);
         }

         /*
          * Compute lod = log2(rho)
          */

         if (!lod_bias && !is_lodq &&
             !bld->static_sampler_state->aniso &&
             !bld->static_sampler_state->lod_bias_non_zero &&
             !bld->static_sampler_state->apply_max_lod &&
             !bld->static_sampler_state->apply_min_lod) {
            /*
             * Special case when there are no post-log2 adjustments, which
             * saves instructions but keeping the integer and fractional lod
             * computations separate from the start.
             */

            if (mip_filter == PIPE_TEX_MIPFILTER_NONE ||
                mip_filter == PIPE_TEX_MIPFILTER_NEAREST) {
               /*
                * Don't actually need both values all the time, lod_ipart is
                * needed for nearest mipfilter, lod_positive if min != mag.
                */
               if (rho_squared) {
                  *out_lod_ipart = lp_build_ilog2_sqrt(lodf_bld, rho);
               } else {
                  *out_lod_ipart = lp_build_ilog2(lodf_bld, rho);
               }
               *out_lod_positive = lp_build_cmp(lodf_bld, PIPE_FUNC_GREATER,
                                                rho, lodf_bld->one);
               return;
            }
            if (mip_filter == PIPE_TEX_MIPFILTER_LINEAR &&
                !bld->no_brilinear && !rho_squared &&
                !bld->static_sampler_state->aniso) {
               /*
                * This can't work if rho is squared. Not sure if it could be
                * fixed while keeping it worthwile, could also do sqrt here
                * but brilinear and no_rho_opt seems like a combination not
                * making much sense anyway so just use ordinary path below.
                */
               lp_build_brilinear_rho(lodf_bld, rho, BRILINEAR_FACTOR,
                                      out_lod_ipart, out_lod_fpart);
               *out_lod_positive = lp_build_cmp(lodf_bld, PIPE_FUNC_GREATER,
                                                rho, lodf_bld->one);
               return;
            }
         }

         if (0) {
            lod = lp_build_log2(lodf_bld, rho);
         } else {
            /* get more accurate results if we just sqaure rho always */
            if (!rho_squared)
               rho = lp_build_mul(lodf_bld, rho, rho);
            lod = lp_build_fast_log2(lodf_bld, rho);
         }

         /* log2(x^2) == 0.5*log2(x) */
         lod = lp_build_mul(lodf_bld, lod,
                            lp_build_const_vec(bld->gallivm,
                                               lodf_bld->type, 0.5F));

         /* add shader lod bias */
         if (lod_bias) {
            if (bld->num_lods != bld->coord_type.length)
               lod_bias = lp_build_pack_aos_scalars(bld->gallivm,
                                                    bld->coord_bld.type,
                                                    lodf_bld->type,
                                                    lod_bias, 0);
            lod = LLVMBuildFAdd(builder, lod, lod_bias, "shader_lod_bias");
         }
      }

      /* add sampler lod bias */
      if (bld->static_sampler_state->lod_bias_non_zero) {
         LLVMValueRef sampler_lod_bias =
            dynamic_state->lod_bias(bld->gallivm, bld->resources_type,
                                    bld->resources_ptr, sampler_unit);
         sampler_lod_bias = lp_build_broadcast_scalar(lodf_bld,
                                                      sampler_lod_bias);
         lod = LLVMBuildFAdd(builder, lod, sampler_lod_bias, "sampler_lod_bias");
      }

      if (is_lodq) {
         *out_lod = lod;
      }

      /* clamp lod */
      if (bld->static_sampler_state->apply_max_lod) {
         LLVMValueRef max_lod =
            dynamic_state->max_lod(bld->gallivm, bld->resources_type,
                                   bld->resources_ptr, sampler_unit);
         max_lod = lp_build_broadcast_scalar(lodf_bld, max_lod);

         lod = lp_build_min(lodf_bld, lod, max_lod);
      }
      if (bld->static_sampler_state->apply_min_lod) {
         LLVMValueRef min_lod =
            dynamic_state->min_lod(bld->gallivm, bld->resources_type,
                                   bld->resources_ptr, sampler_unit);
         min_lod = lp_build_broadcast_scalar(lodf_bld, min_lod);

         lod = lp_build_max(lodf_bld, lod, min_lod);
      }

      if (is_lodq) {
         *out_lod_fpart = lod;
         return;
      }
   }

   *out_lod_positive = lp_build_cmp(lodf_bld, PIPE_FUNC_GREATER,
                                    lod, lodf_bld->zero);

   if (bld->static_sampler_state->aniso) {
      *out_lod_ipart = lp_build_itrunc(lodf_bld, lod);
   } else if (mip_filter == PIPE_TEX_MIPFILTER_LINEAR) {
      if (!bld->no_brilinear) {
         lp_build_brilinear_lod(lodf_bld, lod, BRILINEAR_FACTOR,
                                out_lod_ipart, out_lod_fpart);
      } else {
         lp_build_ifloor_fract(lodf_bld, lod, out_lod_ipart, out_lod_fpart);
      }

      lp_build_name(*out_lod_fpart, "lod_fpart");
   } else {
      *out_lod_ipart = lp_build_iround(lodf_bld, lod);
   }

   lp_build_name(*out_lod_ipart, "lod_ipart");

   return;
}


/**
 * For PIPE_TEX_MIPFILTER_NEAREST, convert int part of lod
 * to actual mip level.
 * Note: this is all scalar per quad code.
 * \param lod_ipart  int texture level of detail
 * \param level_out  returns integer
 * \param out_of_bounds returns per coord out_of_bounds mask if provided
 */
void
lp_build_nearest_mip_level(struct lp_build_sample_context *bld,
                           LLVMValueRef first_level,
                           LLVMValueRef last_level,
                           LLVMValueRef lod_ipart,
                           LLVMValueRef *level_out,
                           LLVMValueRef *out_of_bounds)
{
   struct lp_build_context *leveli_bld = &bld->leveli_bld;
   LLVMValueRef level = lp_build_add(leveli_bld, lod_ipart, first_level);

   if (out_of_bounds) {
      LLVMValueRef out, out1;
      out = lp_build_cmp(leveli_bld, PIPE_FUNC_LESS, level, first_level);
      out1 = lp_build_cmp(leveli_bld, PIPE_FUNC_GREATER, level, last_level);
      out = lp_build_or(leveli_bld, out, out1);
      if (bld->num_mips == bld->coord_bld.type.length) {
         *out_of_bounds = out;
      } else if (bld->num_mips == 1) {
         *out_of_bounds = lp_build_broadcast_scalar(&bld->int_coord_bld, out);
      } else {
         assert(bld->num_mips == bld->coord_bld.type.length / 4);
         *out_of_bounds =
            lp_build_unpack_broadcast_aos_scalars(bld->gallivm,
                                                  leveli_bld->type,
                                                  bld->int_coord_bld.type,
                                                  out);
      }
      level = lp_build_andnot(&bld->int_coord_bld, level, *out_of_bounds);
      *level_out = level;
   } else {
      /* clamp level to legal range of levels */
      *level_out = lp_build_clamp(leveli_bld, level, first_level, last_level);

   }
}


/**
 * For PIPE_TEX_MIPFILTER_LINEAR, convert per-quad (or per element) int LOD(s)
 * to two (per-quad) (adjacent) mipmap level indexes, and fix up float lod
 * part accordingly.
 * Later, we'll sample from those two mipmap levels and interpolate between
 * them.
 */
void
lp_build_linear_mip_levels(struct lp_build_sample_context *bld,
                           unsigned texture_unit,
                           LLVMValueRef first_level,
                           LLVMValueRef last_level,
                           LLVMValueRef lod_ipart,
                           LLVMValueRef *lod_fpart_inout,
                           LLVMValueRef *level0_out,
                           LLVMValueRef *level1_out)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   struct lp_build_context *leveli_bld = &bld->leveli_bld;
   struct lp_build_context *levelf_bld = &bld->levelf_bld;
   LLVMValueRef clamp_min;
   LLVMValueRef clamp_max;

   assert(bld->num_lods == bld->num_mips);

   *level0_out = lp_build_add(leveli_bld, lod_ipart, first_level);
   *level1_out = lp_build_add(leveli_bld, *level0_out, leveli_bld->one);

   /*
    * Clamp both *level0_out and *level1_out to [first_level, last_level],
    * with the minimum number of comparisons, and zeroing lod_fpart in the
    * extreme ends in the process.
    */

   /* *level0_out < first_level */
   clamp_min = LLVMBuildICmp(builder, LLVMIntSLT,
                             *level0_out, first_level,
                             "clamp_lod_to_first");

   *level0_out = LLVMBuildSelect(builder, clamp_min,
                                 first_level, *level0_out, "");

   *level1_out = LLVMBuildSelect(builder, clamp_min,
                                 first_level, *level1_out, "");

   *lod_fpart_inout = LLVMBuildSelect(builder, clamp_min,
                                      levelf_bld->zero, *lod_fpart_inout, "");

   /* *level0_out >= last_level */
   clamp_max = LLVMBuildICmp(builder, LLVMIntSGE,
                             *level0_out, last_level,
                             "clamp_lod_to_last");

   *level0_out = LLVMBuildSelect(builder, clamp_max,
                                 last_level, *level0_out, "");

   *level1_out = LLVMBuildSelect(builder, clamp_max,
                                 last_level, *level1_out, "");

   *lod_fpart_inout = LLVMBuildSelect(builder, clamp_max,
                                      levelf_bld->zero, *lod_fpart_inout, "");

   lp_build_name(*level0_out, "texture%u_miplevel0", texture_unit);
   lp_build_name(*level1_out, "texture%u_miplevel1", texture_unit);
   lp_build_name(*lod_fpart_inout, "texture%u_mipweight", texture_unit);
}


/**
 * A helper function that factorizes this common pattern.
 */
LLVMValueRef
lp_sample_load_mip_value(struct gallivm_state *gallivm,
                         LLVMTypeRef ptr_type,
                         LLVMValueRef offsets,
                         LLVMValueRef index1)
{
   LLVMValueRef zero = lp_build_const_int32(gallivm, 0);
   LLVMValueRef indexes[2] = {zero, index1};
   LLVMValueRef ptr = LLVMBuildGEP2(gallivm->builder, ptr_type, offsets,
                                    indexes, ARRAY_SIZE(indexes), "");
   return LLVMBuildLoad2(gallivm->builder,
                         LLVMInt32TypeInContext(gallivm->context), ptr, "");
}


/**
 * Return pointer to a single mipmap level.
 * \param level  integer mipmap level
 */
LLVMValueRef
lp_build_get_mipmap_level(struct lp_build_sample_context *bld,
                          LLVMValueRef level)
{
   LLVMValueRef mip_offset = lp_sample_load_mip_value(bld->gallivm, bld->mip_offsets_type,
                                                      bld->mip_offsets, level);
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef data_ptr =
      LLVMBuildGEP2(builder,
                    LLVMInt8TypeInContext(bld->gallivm->context),
                    bld->base_ptr, &mip_offset, 1, "");
   return data_ptr;
}


/**
 * Return (per-pixel) offsets to mip levels.
 * \param level  integer mipmap level
 */
LLVMValueRef
lp_build_get_mip_offsets(struct lp_build_sample_context *bld,
                         LLVMValueRef level)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef offsets, offset1;

   if (bld->num_mips == 1) {
      offset1 = lp_sample_load_mip_value(bld->gallivm, bld->mip_offsets_type, bld->mip_offsets, level);
      offsets = lp_build_broadcast_scalar(&bld->int_coord_bld, offset1);
   } else if (bld->num_mips == bld->coord_bld.type.length / 4) {
      offsets = bld->int_coord_bld.undef;
      for (unsigned i = 0; i < bld->num_mips; i++) {
         LLVMValueRef indexi = lp_build_const_int32(bld->gallivm, i);
         offset1 = lp_sample_load_mip_value(bld->gallivm, bld->mip_offsets_type,
                                            bld->mip_offsets,
                                            LLVMBuildExtractElement(builder, level,
                                                                    indexi, ""));
         LLVMValueRef indexo = lp_build_const_int32(bld->gallivm, 4 * i);
         offsets = LLVMBuildInsertElement(builder, offsets, offset1,
                                          indexo, "");
      }
      offsets = lp_build_swizzle_scalar_aos(&bld->int_coord_bld,
                                            offsets, 0, 4);
   } else {
      assert (bld->num_mips == bld->coord_bld.type.length);

      offsets = bld->int_coord_bld.undef;
      for (unsigned i = 0; i < bld->num_mips; i++) {
         LLVMValueRef indexi = lp_build_const_int32(bld->gallivm, i);
         offset1 = lp_sample_load_mip_value(bld->gallivm, bld->mip_offsets_type,
                                            bld->mip_offsets,
                                            LLVMBuildExtractElement(builder, level,
                                                                    indexi, ""));
         offsets = LLVMBuildInsertElement(builder, offsets, offset1,
                                          indexi, "");
      }
   }
   return offsets;
}


/**
 * Codegen equivalent for u_minify().
 * @param lod_scalar  if lod is a (broadcasted) scalar
 * Return max(1, base_size >> level);
 */
LLVMValueRef
lp_build_minify(struct lp_build_context *bld,
                LLVMValueRef base_size,
                LLVMValueRef level,
                bool lod_scalar)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   assert(lp_check_value(bld->type, base_size));
   assert(lp_check_value(bld->type, level));

   if (level == bld->zero) {
      /* if we're using mipmap level zero, no minification is needed */
      return base_size;
   } else {
      LLVMValueRef size;
      assert(bld->type.sign);
      if (lod_scalar ||
         (util_get_cpu_caps()->has_avx2 || !util_get_cpu_caps()->has_sse)) {
         size = LLVMBuildLShr(builder, base_size, level, "minify");
         size = lp_build_max(bld, size, bld->one);
      } else {
         /*
          * emulate shift with float mul, since intel "forgot" shifts with
          * per-element shift count until avx2, which results in terrible
          * scalar extraction (both count and value), scalar shift,
          * vector reinsertion. Should not be an issue on any non-x86 cpu
          * with a vector instruction set.
          * On cpus with AMD's XOP this should also be unnecessary but I'm
          * not sure if llvm would emit this with current flags.
          */
         LLVMValueRef const127, const23, lf;
         struct lp_type ftype;
         struct lp_build_context fbld;
         ftype = lp_type_float_vec(32, bld->type.length * bld->type.width);
         lp_build_context_init(&fbld, bld->gallivm, ftype);
         const127 = lp_build_const_int_vec(bld->gallivm, bld->type, 127);
         const23 = lp_build_const_int_vec(bld->gallivm, bld->type, 23);

         /* calculate 2^(-level) float */
         lf = lp_build_sub(bld, const127, level);
         lf = lp_build_shl(bld, lf, const23);
         lf = LLVMBuildBitCast(builder, lf, fbld.vec_type, "");

         /* finish shift operation by doing float mul */
         base_size = lp_build_int_to_float(&fbld, base_size);
         size = lp_build_mul(&fbld, base_size, lf);
         /*
          * do the max also with floats because
          * a) non-emulated int max requires sse41
          *    (this is actually a lie as we could cast to 16bit values
          *    as 16bit is sufficient and 16bit int max is sse2)
          * b) with avx we can do int max 4-wide but float max 8-wide
          */
         size = lp_build_max(&fbld, size, fbld.one);
         size = lp_build_itrunc(&fbld, size);
      }
      return size;
   }
}


/*
 * Scale image dimensions with block sizes.
 *
 * tex_blocksize is the resource format blocksize
 * view_blocksize is the view format blocksize
 *
 * This must be applied post-minification, but
 * only when blocksizes are different.
 *
 * ret = (size + (tex_blocksize - 1)) >> log2(tex_blocksize);
 * ret *= blocksize;
 */
LLVMValueRef
lp_build_scale_view_dims(struct lp_build_context *bld, LLVMValueRef size,
                         LLVMValueRef tex_blocksize,
                         LLVMValueRef tex_blocksize_log2,
                         LLVMValueRef view_blocksize)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef ret =
      LLVMBuildAdd(builder, size,
                   LLVMBuildSub(builder, tex_blocksize,
                                lp_build_const_int_vec(bld->gallivm,
                                                       bld->type, 1), ""),
                   "");
   ret = LLVMBuildLShr(builder, ret, tex_blocksize_log2, "");
   ret = LLVMBuildMul(builder, ret, view_blocksize, "");
   return ret;
}


/*
 * Scale a single image dimension.
 *
 * Scale one image between resource and view blocksizes.
 * noop if sizes are the same.
 */
LLVMValueRef
lp_build_scale_view_dim(struct gallivm_state *gallivm, LLVMValueRef size,
                        unsigned tex_blocksize, unsigned view_blocksize)
{
   if (tex_blocksize == view_blocksize)
      return size;

   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef ret =
      LLVMBuildAdd(builder, size,
                   lp_build_const_int32(gallivm, tex_blocksize - 1), "");
   ret = LLVMBuildLShr(builder, ret,
                       lp_build_const_int32(gallivm,
                                            util_logbase2(tex_blocksize)), "");
   ret = LLVMBuildMul(builder, ret,
                      lp_build_const_int32(gallivm, view_blocksize), "");
   return ret;
}


/**
 * Dereference stride_array[mipmap_level] array to get a stride.
 * Return stride as a vector.
 */
static LLVMValueRef
lp_build_get_level_stride_vec(struct lp_build_sample_context *bld,
                              LLVMTypeRef stride_type,
                              LLVMValueRef stride_array, LLVMValueRef level)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef stride, stride1;

   if (bld->num_mips == 1) {
      stride1 = lp_sample_load_mip_value(bld->gallivm, stride_type, stride_array, level);
      stride = lp_build_broadcast_scalar(&bld->int_coord_bld, stride1);
   } else if (bld->num_mips == bld->coord_bld.type.length / 4) {
      LLVMValueRef stride1;

      stride = bld->int_coord_bld.undef;
      for (unsigned i = 0; i < bld->num_mips; i++) {
         LLVMValueRef indexi = lp_build_const_int32(bld->gallivm, i);
         stride1 = lp_sample_load_mip_value(bld->gallivm, stride_type, stride_array,
                                            LLVMBuildExtractElement(builder, level,
                                                                    indexi, ""));
         LLVMValueRef indexo = lp_build_const_int32(bld->gallivm, 4 * i);
         stride = LLVMBuildInsertElement(builder, stride, stride1, indexo, "");
      }
      stride = lp_build_swizzle_scalar_aos(&bld->int_coord_bld, stride, 0, 4);
   } else {
      LLVMValueRef stride1;

      assert (bld->num_mips == bld->coord_bld.type.length);

      stride = bld->int_coord_bld.undef;
      for (unsigned i = 0; i < bld->coord_bld.type.length; i++) {
         LLVMValueRef indexi = lp_build_const_int32(bld->gallivm, i);
         stride1 = lp_sample_load_mip_value(bld->gallivm, stride_type, stride_array,
                                            LLVMBuildExtractElement(builder, level,
                                                                    indexi, ""));
         stride = LLVMBuildInsertElement(builder, stride, stride1, indexi, "");
      }
   }
   return stride;
}


/**
 * When sampling a mipmap, we need to compute the width, height, depth
 * of the source levels from the level indexes.  This helper function
 * does that.
 */
void
lp_build_mipmap_level_sizes(struct lp_build_sample_context *bld,
                            LLVMValueRef ilevel,
                            LLVMValueRef *out_size,
                            LLVMValueRef *row_stride_vec,
                            LLVMValueRef *img_stride_vec)
{
   const unsigned dims = bld->dims;
   LLVMValueRef ilevel_vec;

   /*
    * Compute width, height, depth at mipmap level 'ilevel'
    */
   if (bld->num_mips == 1) {
      ilevel_vec = lp_build_broadcast_scalar(&bld->int_size_bld, ilevel);
      *out_size = lp_build_minify(&bld->int_size_bld, bld->int_size,
                                  ilevel_vec, true);
      *out_size = lp_build_scale_view_dims(&bld->int_size_bld, *out_size,
                                           bld->int_tex_blocksize,
                                           bld->int_tex_blocksize_log2,
                                           bld->int_view_blocksize);
   } else {
      LLVMValueRef int_size_vec;
      LLVMValueRef int_tex_blocksize_vec, int_tex_blocksize_log2_vec;
      LLVMValueRef int_view_blocksize_vec;
      LLVMValueRef tmp[LP_MAX_VECTOR_LENGTH];
      const unsigned num_quads = bld->coord_bld.type.length / 4;

      if (bld->num_mips == num_quads) {
         /*
          * XXX: this should be #ifndef SANE_INSTRUCTION_SET.
          * intel "forgot" the variable shift count instruction until avx2.
          * A harmless 8x32 shift gets translated into 32 instructions
          * (16 extracts, 8 scalar shifts, 8 inserts), llvm is apparently
          * unable to recognize if there are really just 2 different shift
          * count values. So do the shift 4-wide before expansion.
          */
         struct lp_build_context bld4;
         struct lp_type type4;

         type4 = bld->int_coord_bld.type;
         type4.length = 4;

         lp_build_context_init(&bld4, bld->gallivm, type4);

         if (bld->dims == 1) {
            assert(bld->int_size_in_bld.type.length == 1);
            int_size_vec = lp_build_broadcast_scalar(&bld4,
                                                     bld->int_size);
            int_tex_blocksize_vec =
               lp_build_broadcast_scalar(&bld4, bld->int_tex_blocksize);
            int_tex_blocksize_log2_vec =
               lp_build_broadcast_scalar(&bld4, bld->int_tex_blocksize_log2);
            int_view_blocksize_vec =
               lp_build_broadcast_scalar(&bld4, bld->int_view_blocksize);
         } else {
            assert(bld->int_size_in_bld.type.length == 4);
            int_size_vec = bld->int_size;
            int_tex_blocksize_vec = bld->int_tex_blocksize;
            int_tex_blocksize_log2_vec = bld->int_tex_blocksize_log2;
            int_view_blocksize_vec = bld->int_view_blocksize;
         }

         for (unsigned i = 0; i < num_quads; i++) {
            LLVMValueRef ileveli;
            LLVMValueRef indexi = lp_build_const_int32(bld->gallivm, i);

            ileveli = lp_build_extract_broadcast(bld->gallivm,
                                                 bld->leveli_bld.type,
                                                 bld4.type,
                                                 ilevel,
                                                 indexi);
            tmp[i] = lp_build_minify(&bld4, int_size_vec, ileveli, true);
            tmp[i] = lp_build_scale_view_dims(&bld4, tmp[i],
                                              int_tex_blocksize_vec,
                                              int_tex_blocksize_log2_vec,
                                              int_view_blocksize_vec);
         }
         /*
          * out_size is [w0, h0, d0, _, w1, h1, d1, _, ...] vector for
          * dims > 1, [w0, w0, w0, w0, w1, w1, w1, w1, ...] otherwise.
          */
         *out_size = lp_build_concat(bld->gallivm,
                                     tmp,
                                     bld4.type,
                                     num_quads);
      } else {
         /* FIXME: this is terrible and results in _huge_ vector
          * (for the dims > 1 case).
          * Should refactor this (together with extract_image_sizes) and do
          * something more useful. Could for instance if we have width,height
          * with 4-wide vector pack all elements into a 8xi16 vector
          * (on which we can still do useful math) instead of using a 16xi32
          * vector.
          * For dims == 1 this will create [w0, w1, w2, w3, ...] vector.
          * For dims > 1 this will create [w0, h0, d0, _, w1, h1, d1, _, ...]
          * vector.
          */
         assert(bld->num_mips == bld->coord_bld.type.length);
         if (bld->dims == 1) {
            assert(bld->int_size_in_bld.type.length == 1);
            int_size_vec = lp_build_broadcast_scalar(&bld->int_coord_bld,
                                                     bld->int_size);
            int_tex_blocksize_vec =
               lp_build_broadcast_scalar(&bld->int_coord_bld,
                                         bld->int_tex_blocksize);
            int_tex_blocksize_log2_vec =
               lp_build_broadcast_scalar(&bld->int_coord_bld,
                                         bld->int_tex_blocksize_log2);
            int_view_blocksize_vec =
               lp_build_broadcast_scalar(&bld->int_coord_bld,
                                         bld->int_view_blocksize);
            *out_size = lp_build_minify(&bld->int_coord_bld, int_size_vec,
                                        ilevel, false);
            *out_size = lp_build_scale_view_dims(&bld->int_coord_bld,
                                                 *out_size,
                                                 int_tex_blocksize_vec,
                                                 int_tex_blocksize_log2_vec,
                                                 int_view_blocksize_vec);
         } else {
            LLVMValueRef ilevel1;
            for (unsigned i = 0; i < bld->num_mips; i++) {
               LLVMValueRef indexi = lp_build_const_int32(bld->gallivm, i);
               ilevel1 = lp_build_extract_broadcast(bld->gallivm,
                                                    bld->int_coord_type,
                                                    bld->int_size_in_bld.type,
                                                    ilevel, indexi);
               tmp[i] = bld->int_size;
               tmp[i] = lp_build_minify(&bld->int_size_in_bld, tmp[i],
                                        ilevel1, true);
               tmp[i] = lp_build_scale_view_dims(&bld->int_size_in_bld,
                                                 tmp[i],
                                                 bld->int_tex_blocksize,
                                                 bld->int_tex_blocksize_log2,
                                                 bld->int_view_blocksize);
            }
            *out_size = lp_build_concat(bld->gallivm, tmp,
                                        bld->int_size_in_bld.type,
                                        bld->num_mips);
         }
      }
   }

   if (dims >= 2) {
      *row_stride_vec = lp_build_get_level_stride_vec(bld,
                                                      bld->row_stride_type,
                                                      bld->row_stride_array,
                                                      ilevel);
   }
   if (dims == 3 || has_layer_coord(bld->static_texture_state->target)) {
      *img_stride_vec = lp_build_get_level_stride_vec(bld,
                                                      bld->img_stride_type,
                                                      bld->img_stride_array,
                                                      ilevel);
   }
}


/**
 * Extract and broadcast texture size.
 *
 * @param size_type   type of the texture size vector (either
 *                    bld->int_size_type or bld->float_size_type)
 * @param coord_type  type of the texture size vector (either
 *                    bld->int_coord_type or bld->coord_type)
 * @param size        vector with the texture size (width, height, depth)
 */
void
lp_build_extract_image_sizes(struct lp_build_sample_context *bld,
                             struct lp_build_context *size_bld,
                             struct lp_type coord_type,
                             LLVMValueRef size,
                             LLVMValueRef *out_width,
                             LLVMValueRef *out_height,
                             LLVMValueRef *out_depth)
{
   const unsigned dims = bld->dims;
   LLVMTypeRef i32t = LLVMInt32TypeInContext(bld->gallivm->context);
   struct lp_type size_type = size_bld->type;

   if (bld->num_mips == 1) {
      *out_width = lp_build_extract_broadcast(bld->gallivm,
                                              size_type,
                                              coord_type,
                                              size,
                                              LLVMConstInt(i32t, 0, 0));
      if (dims >= 2) {
         *out_height = lp_build_extract_broadcast(bld->gallivm,
                                                  size_type,
                                                  coord_type,
                                                  size,
                                                  LLVMConstInt(i32t, 1, 0));
         if (dims == 3) {
            *out_depth = lp_build_extract_broadcast(bld->gallivm,
                                                    size_type,
                                                    coord_type,
                                                    size,
                                                    LLVMConstInt(i32t, 2, 0));
         }
      }
   } else {
      unsigned num_quads = bld->coord_bld.type.length / 4;

      if (dims == 1) {
         *out_width = size;
      } else if (bld->num_mips == num_quads) {
         *out_width = lp_build_swizzle_scalar_aos(size_bld, size, 0, 4);
         if (dims >= 2) {
            *out_height = lp_build_swizzle_scalar_aos(size_bld, size, 1, 4);
            if (dims == 3) {
               *out_depth = lp_build_swizzle_scalar_aos(size_bld, size, 2, 4);
            }
         }
      } else {
         assert(bld->num_mips == bld->coord_type.length);
         *out_width = lp_build_pack_aos_scalars(bld->gallivm, size_type,
                                                coord_type, size, 0);
         if (dims >= 2) {
            *out_height = lp_build_pack_aos_scalars(bld->gallivm, size_type,
                                                    coord_type, size, 1);
            if (dims == 3) {
               *out_depth = lp_build_pack_aos_scalars(bld->gallivm, size_type,
                                                      coord_type, size, 2);
            }
         }
      }
   }
}


/**
 * Unnormalize coords.
 *
 * @param flt_size  vector with the integer texture size (width, height, depth)
 */
void
lp_build_unnormalized_coords(struct lp_build_sample_context *bld,
                             LLVMValueRef flt_size,
                             LLVMValueRef *s,
                             LLVMValueRef *t,
                             LLVMValueRef *r)
{
   const unsigned dims = bld->dims;
   LLVMValueRef width;
   LLVMValueRef height = NULL;
   LLVMValueRef depth = NULL;

   lp_build_extract_image_sizes(bld,
                                &bld->float_size_bld,
                                bld->coord_type,
                                flt_size,
                                &width,
                                &height,
                                &depth);

   /* s = s * width, t = t * height */
   *s = lp_build_mul(&bld->coord_bld, *s, width);
   if (dims >= 2) {
      *t = lp_build_mul(&bld->coord_bld, *t, height);
      if (dims >= 3) {
         *r = lp_build_mul(&bld->coord_bld, *r, depth);
      }
   }
}


/**
 * Generate new coords and faces for cubemap texels falling off the face.
 *
 * @param face   face (center) of the pixel
 * @param x0     lower x coord
 * @param x1     higher x coord (must be x0 + 1)
 * @param y0     lower y coord
 * @param y1     higher y coord (must be x0 + 1)
 * @param max_coord     texture cube (level) size - 1
 * @param next_faces    new face values when falling off
 * @param next_xcoords  new x coord values when falling off
 * @param next_ycoords  new y coord values when falling off
 *
 * The arrays hold the new values when under/overflow of
 * lower x, higher x, lower y, higher y coord would occur (in this order).
 * next_xcoords/next_ycoords have two entries each (for both new lower and
 * higher coord).
 */
void
lp_build_cube_new_coords(struct lp_build_context *ivec_bld,
                        LLVMValueRef face,
                        LLVMValueRef x0,
                        LLVMValueRef x1,
                        LLVMValueRef y0,
                        LLVMValueRef y1,
                        LLVMValueRef max_coord,
                        LLVMValueRef next_faces[4],
                        LLVMValueRef next_xcoords[4][2],
                        LLVMValueRef next_ycoords[4][2])
{
   /*
    * Lookup tables aren't nice for simd code hence try some logic here.
    * (Note that while it would not be necessary to do per-sample (4) lookups
    * when using a LUT as it's impossible that texels fall off of positive
    * and negative edges simultaneously, it would however be necessary to
    * do 2 lookups for corner handling as in this case texels both fall off
    * of x and y axes.)
    */
   /*
    * Next faces (for face 012345):
    * x < 0.0  : 451110
    * x >= 1.0 : 540001
    * y < 0.0  : 225422
    * y >= 1.0 : 334533
    * Hence nfx+ (and nfy+) == nfx- (nfy-) xor 1
    * nfx-: face > 1 ? (face == 5 ? 0 : 1) : (4 + face & 1)
    * nfy+: face & ~4 > 1 ? face + 2 : 3;
    * This could also use pshufb instead, but would need (manually coded)
    * ssse3 intrinsic (llvm won't do non-constant shuffles).
    */
   struct gallivm_state *gallivm = ivec_bld->gallivm;
   LLVMValueRef sel, sel_f2345, sel_f23, sel_f2, tmpsel, tmp;
   LLVMValueRef faceand1, sel_fand1, maxmx0, maxmx1, maxmy0, maxmy1;
   LLVMValueRef c2 = lp_build_const_int_vec(gallivm, ivec_bld->type, 2);
   LLVMValueRef c3 = lp_build_const_int_vec(gallivm, ivec_bld->type, 3);
   LLVMValueRef c4 = lp_build_const_int_vec(gallivm, ivec_bld->type, 4);
   LLVMValueRef c5 = lp_build_const_int_vec(gallivm, ivec_bld->type, 5);

   sel = lp_build_cmp(ivec_bld, PIPE_FUNC_EQUAL, face, c5);
   tmpsel = lp_build_select(ivec_bld, sel, ivec_bld->zero, ivec_bld->one);
   sel_f2345 = lp_build_cmp(ivec_bld, PIPE_FUNC_GREATER, face, ivec_bld->one);
   faceand1 = lp_build_and(ivec_bld, face, ivec_bld->one);
   tmp = lp_build_add(ivec_bld, faceand1, c4);
   next_faces[0] = lp_build_select(ivec_bld, sel_f2345, tmpsel, tmp);
   next_faces[1] = lp_build_xor(ivec_bld, next_faces[0], ivec_bld->one);

   tmp = lp_build_andnot(ivec_bld, face, c4);
   sel_f23 = lp_build_cmp(ivec_bld, PIPE_FUNC_GREATER, tmp, ivec_bld->one);
   tmp = lp_build_add(ivec_bld, face, c2);
   next_faces[3] = lp_build_select(ivec_bld, sel_f23, tmp, c3);
   next_faces[2] = lp_build_xor(ivec_bld, next_faces[3], ivec_bld->one);

   /*
    * new xcoords (for face 012345):
    * x < 0.0  : max   max   t     max-t max  max
    * x >= 1.0 : 0     0     max-t t     0    0
    * y < 0.0  : max   0     max-s s     s    max-s
    * y >= 1.0 : max   0     s     max-s s    max-s
    *
    * ncx[1] = face & ~4 > 1 ? (face == 2 ? max-t : t) : 0
    * ncx[0] = max - ncx[1]
    * ncx[3] = face > 1 ? (face & 1 ? max-s : s) : (face & 1) ? 0 : max
    * ncx[2] = face & ~4 > 1 ? max - ncx[3] : ncx[3]
    */
   sel_f2 = lp_build_cmp(ivec_bld, PIPE_FUNC_EQUAL, face, c2);
   maxmy0 = lp_build_sub(ivec_bld, max_coord, y0);
   tmp = lp_build_select(ivec_bld, sel_f2, maxmy0, y0);
   next_xcoords[1][0] = lp_build_select(ivec_bld, sel_f23, tmp, ivec_bld->zero);
   next_xcoords[0][0] = lp_build_sub(ivec_bld, max_coord, next_xcoords[1][0]);
   maxmy1 = lp_build_sub(ivec_bld, max_coord, y1);
   tmp = lp_build_select(ivec_bld, sel_f2, maxmy1, y1);
   next_xcoords[1][1] = lp_build_select(ivec_bld, sel_f23, tmp, ivec_bld->zero);
   next_xcoords[0][1] = lp_build_sub(ivec_bld, max_coord, next_xcoords[1][1]);

   sel_fand1 = lp_build_cmp(ivec_bld, PIPE_FUNC_EQUAL, faceand1, ivec_bld->one);

   tmpsel = lp_build_select(ivec_bld, sel_fand1, ivec_bld->zero, max_coord);
   maxmx0 = lp_build_sub(ivec_bld, max_coord, x0);
   tmp = lp_build_select(ivec_bld, sel_fand1, maxmx0, x0);
   next_xcoords[3][0] = lp_build_select(ivec_bld, sel_f2345, tmp, tmpsel);
   tmp = lp_build_sub(ivec_bld, max_coord, next_xcoords[3][0]);
   next_xcoords[2][0] = lp_build_select(ivec_bld, sel_f23, tmp, next_xcoords[3][0]);
   maxmx1 = lp_build_sub(ivec_bld, max_coord, x1);
   tmp = lp_build_select(ivec_bld, sel_fand1, maxmx1, x1);
   next_xcoords[3][1] = lp_build_select(ivec_bld, sel_f2345, tmp, tmpsel);
   tmp = lp_build_sub(ivec_bld, max_coord, next_xcoords[3][1]);
   next_xcoords[2][1] = lp_build_select(ivec_bld, sel_f23, tmp, next_xcoords[3][1]);

   /*
    * new ycoords (for face 012345):
    * x < 0.0  : t     t     0     max   t    t
    * x >= 1.0 : t     t     0     max   t    t
    * y < 0.0  : max-s s     0     max   max  0
    * y >= 1.0 : s     max-s 0     max   0    max
    *
    * ncy[0] = face & ~4 > 1 ? (face == 2 ? 0 : max) : t
    * ncy[1] = ncy[0]
    * ncy[3] = face > 1 ? (face & 1 ? max : 0) : (face & 1) ? max-s : max
    * ncx[2] = face & ~4 > 1 ? max - ncx[3] : ncx[3]
    */
   tmp = lp_build_select(ivec_bld, sel_f2, ivec_bld->zero, max_coord);
   next_ycoords[0][0] = lp_build_select(ivec_bld, sel_f23, tmp, y0);
   next_ycoords[1][0] = next_ycoords[0][0];
   next_ycoords[0][1] = lp_build_select(ivec_bld, sel_f23, tmp, y1);
   next_ycoords[1][1] = next_ycoords[0][1];

   tmpsel = lp_build_select(ivec_bld, sel_fand1, maxmx0, x0);
   tmp = lp_build_select(ivec_bld, sel_fand1, max_coord, ivec_bld->zero);
   next_ycoords[3][0] = lp_build_select(ivec_bld, sel_f2345, tmp, tmpsel);
   tmp = lp_build_sub(ivec_bld, max_coord, next_ycoords[3][0]);
   next_ycoords[2][0] = lp_build_select(ivec_bld, sel_f23, next_ycoords[3][0], tmp);
   tmpsel = lp_build_select(ivec_bld, sel_fand1, maxmx1, x1);
   tmp = lp_build_select(ivec_bld, sel_fand1, max_coord, ivec_bld->zero);
   next_ycoords[3][1] = lp_build_select(ivec_bld, sel_f2345, tmp, tmpsel);
   tmp = lp_build_sub(ivec_bld, max_coord, next_ycoords[3][1]);
   next_ycoords[2][1] = lp_build_select(ivec_bld, sel_f23, next_ycoords[3][1], tmp);
}


/** Helper used by lp_build_cube_lookup() */
static LLVMValueRef
lp_build_cube_imapos(struct lp_build_context *coord_bld, LLVMValueRef coord)
{
   /* ima = +0.5 / abs(coord); */
   LLVMValueRef posHalf = lp_build_const_vec(coord_bld->gallivm, coord_bld->type, 0.5);
   LLVMValueRef absCoord = lp_build_abs(coord_bld, coord);
   /* avoid div by zero */
   LLVMValueRef sel = lp_build_cmp(coord_bld, PIPE_FUNC_GREATER, absCoord, coord_bld->zero);
   LLVMValueRef div = lp_build_div(coord_bld, posHalf, absCoord);
   LLVMValueRef ima = lp_build_select(coord_bld, sel, div, coord_bld->zero);
   return ima;
}


/** Helper for doing 3-wise selection.
 * Returns sel1 ? val2 : (sel0 ? val0 : val1).
 */
static LLVMValueRef
lp_build_select3(struct lp_build_context *sel_bld,
                 LLVMValueRef sel0,
                 LLVMValueRef sel1,
                 LLVMValueRef val0,
                 LLVMValueRef val1,
                 LLVMValueRef val2)
{
   LLVMValueRef tmp = lp_build_select(sel_bld, sel0, val0, val1);
   return lp_build_select(sel_bld, sel1, val2, tmp);
}


/**
 * Generate code to do cube face selection and compute per-face texcoords.
 */
void
lp_build_cube_lookup(struct lp_build_sample_context *bld,
                     LLVMValueRef *coords,
                     const struct lp_derivatives *derivs_in, /* optional */
                     struct lp_derivatives *derivs_out, /* optional */
                     bool need_derivs)
{
   struct lp_build_context *coord_bld = &bld->coord_bld;
   LLVMBuilderRef builder = bld->gallivm->builder;
   struct gallivm_state *gallivm = bld->gallivm;
   LLVMValueRef si, ti, ri;

   /*
    * Do per-pixel face selection. We cannot however (as we used to do)
    * simply calculate the derivs afterwards (which is very bogus for
    * explicit derivs btw) because the values would be "random" when
    * not all pixels lie on the same face.
    */
   struct lp_build_context *cint_bld = &bld->int_coord_bld;
   struct lp_type intctype = cint_bld->type;
   LLVMTypeRef coord_vec_type = coord_bld->vec_type;
   LLVMTypeRef cint_vec_type = cint_bld->vec_type;
   LLVMValueRef as, at, ar, face, face_s, face_t;
   LLVMValueRef as_ge_at, maxasat, ar_ge_as_at;
   LLVMValueRef snewx, tnewx, snewy, tnewy, snewz, tnewz;
   LLVMValueRef tnegi, rnegi;
   LLVMValueRef ma, mai, signma, signmabit, imahalfpos;
   LLVMValueRef posHalf = lp_build_const_vec(gallivm, coord_bld->type, 0.5);
   LLVMValueRef signmask = lp_build_const_int_vec(gallivm, intctype,
                                                  1LL << (intctype.width - 1));
   LLVMValueRef signshift = lp_build_const_int_vec(gallivm, intctype,
                                                   intctype.width -1);
   LLVMValueRef facex = lp_build_const_int_vec(gallivm, intctype, PIPE_TEX_FACE_POS_X);
   LLVMValueRef facey = lp_build_const_int_vec(gallivm, intctype, PIPE_TEX_FACE_POS_Y);
   LLVMValueRef facez = lp_build_const_int_vec(gallivm, intctype, PIPE_TEX_FACE_POS_Z);
   LLVMValueRef s = coords[0];
   LLVMValueRef t = coords[1];
   LLVMValueRef r = coords[2];

   assert(PIPE_TEX_FACE_NEG_X == PIPE_TEX_FACE_POS_X + 1);
   assert(PIPE_TEX_FACE_NEG_Y == PIPE_TEX_FACE_POS_Y + 1);
   assert(PIPE_TEX_FACE_NEG_Z == PIPE_TEX_FACE_POS_Z + 1);

   /*
    * get absolute value (for x/y/z face selection) and sign bit
    * (for mirroring minor coords and pos/neg face selection)
    * of the original coords.
    */
   as = lp_build_abs(&bld->coord_bld, s);
   at = lp_build_abs(&bld->coord_bld, t);
   ar = lp_build_abs(&bld->coord_bld, r);

   /*
    * major face determination: select x if x > y else select y
    * select z if z >= max(x,y) else select previous result
    * if some axis are the same we chose z over y, y over x - the
    * dx10 spec seems to ask for it while OpenGL doesn't care (if we
    * wouldn't care could save a select or two if using different
    * compares and doing at_g_as_ar last since tnewx and tnewz are the
    * same).
    */
   as_ge_at = lp_build_cmp(coord_bld, PIPE_FUNC_GREATER, as, at);
   maxasat = lp_build_max(coord_bld, as, at);
   ar_ge_as_at = lp_build_cmp(coord_bld, PIPE_FUNC_GEQUAL, ar, maxasat);

   if (need_derivs) {
      /*
       * XXX: This is really really complex.
       * It is a bit overkill to use this for implicit derivatives as well,
       * no way this is worth the cost in practice, but seems to be the
       * only way for getting accurate and per-pixel lod values.
       */
      LLVMValueRef ima, imahalf, tmp, ddx[3], ddy[3];
      LLVMValueRef madx, mady, madxdivma, madydivma;
      LLVMValueRef sdxi, tdxi, rdxi, sdyi, tdyi, rdyi;
      LLVMValueRef tdxnegi, rdxnegi, tdynegi, rdynegi;
      LLVMValueRef sdxnewx, sdxnewy, sdxnewz, tdxnewx, tdxnewy, tdxnewz;
      LLVMValueRef sdynewx, sdynewy, sdynewz, tdynewx, tdynewy, tdynewz;
      LLVMValueRef face_sdx, face_tdx, face_sdy, face_tdy;
      /*
       * s = 1/2 * (sc / ma + 1)
       * t = 1/2 * (tc / ma + 1)
       *
       * s' = 1/2 * (sc' * ma - sc * ma') / ma^2
       * t' = 1/2 * (tc' * ma - tc * ma') / ma^2
       *
       * dx.s = 0.5 * (dx.sc - sc * dx.ma / ma) / ma
       * dx.t = 0.5 * (dx.tc - tc * dx.ma / ma) / ma
       * dy.s = 0.5 * (dy.sc - sc * dy.ma / ma) / ma
       * dy.t = 0.5 * (dy.tc - tc * dy.ma / ma) / ma
       */

      /* select ma, calculate ima */
      ma = lp_build_select3(coord_bld, as_ge_at, ar_ge_as_at, s, t, r);
      mai = LLVMBuildBitCast(builder, ma, cint_vec_type, "");
      signmabit = LLVMBuildAnd(builder, mai, signmask, "");
      ima = lp_build_div(coord_bld, coord_bld->one, ma);
      imahalf = lp_build_mul(coord_bld, posHalf, ima);
      imahalfpos = lp_build_abs(coord_bld, imahalf);

      if (!derivs_in) {
         ddx[0] = lp_build_ddx(coord_bld, s);
         ddx[1] = lp_build_ddx(coord_bld, t);
         ddx[2] = lp_build_ddx(coord_bld, r);
         ddy[0] = lp_build_ddy(coord_bld, s);
         ddy[1] = lp_build_ddy(coord_bld, t);
         ddy[2] = lp_build_ddy(coord_bld, r);
      } else {
         ddx[0] = derivs_in->ddx[0];
         ddx[1] = derivs_in->ddx[1];
         ddx[2] = derivs_in->ddx[2];
         ddy[0] = derivs_in->ddy[0];
         ddy[1] = derivs_in->ddy[1];
         ddy[2] = derivs_in->ddy[2];
      }

      /* select major derivatives */
      madx = lp_build_select3(coord_bld, as_ge_at, ar_ge_as_at, ddx[0], ddx[1], ddx[2]);
      mady = lp_build_select3(coord_bld, as_ge_at, ar_ge_as_at, ddy[0], ddy[1], ddy[2]);

      si = LLVMBuildBitCast(builder, s, cint_vec_type, "");
      ti = LLVMBuildBitCast(builder, t, cint_vec_type, "");
      ri = LLVMBuildBitCast(builder, r, cint_vec_type, "");

      sdxi = LLVMBuildBitCast(builder, ddx[0], cint_vec_type, "");
      tdxi = LLVMBuildBitCast(builder, ddx[1], cint_vec_type, "");
      rdxi = LLVMBuildBitCast(builder, ddx[2], cint_vec_type, "");

      sdyi = LLVMBuildBitCast(builder, ddy[0], cint_vec_type, "");
      tdyi = LLVMBuildBitCast(builder, ddy[1], cint_vec_type, "");
      rdyi = LLVMBuildBitCast(builder, ddy[2], cint_vec_type, "");

      /*
       * compute all possible new s/t coords, which does the mirroring,
       * and do the same for derivs minor axes.
       * snewx = signma * -r;
       * tnewx = -t;
       * snewy = s;
       * tnewy = signma * r;
       * snewz = signma * s;
       * tnewz = -t;
       */
      tnegi = LLVMBuildXor(builder, ti, signmask, "");
      rnegi = LLVMBuildXor(builder, ri, signmask, "");
      tdxnegi = LLVMBuildXor(builder, tdxi, signmask, "");
      rdxnegi = LLVMBuildXor(builder, rdxi, signmask, "");
      tdynegi = LLVMBuildXor(builder, tdyi, signmask, "");
      rdynegi = LLVMBuildXor(builder, rdyi, signmask, "");

      snewx = LLVMBuildXor(builder, signmabit, rnegi, "");
      tnewx = tnegi;
      sdxnewx = LLVMBuildXor(builder, signmabit, rdxnegi, "");
      tdxnewx = tdxnegi;
      sdynewx = LLVMBuildXor(builder, signmabit, rdynegi, "");
      tdynewx = tdynegi;

      snewy = si;
      tnewy = LLVMBuildXor(builder, signmabit, ri, "");
      sdxnewy = sdxi;
      tdxnewy = LLVMBuildXor(builder, signmabit, rdxi, "");
      sdynewy = sdyi;
      tdynewy = LLVMBuildXor(builder, signmabit, rdyi, "");

      snewz = LLVMBuildXor(builder, signmabit, si, "");
      tnewz = tnegi;
      sdxnewz = LLVMBuildXor(builder, signmabit, sdxi, "");
      tdxnewz = tdxnegi;
      sdynewz = LLVMBuildXor(builder, signmabit, sdyi, "");
      tdynewz = tdynegi;

      /* select the mirrored values */
      face = lp_build_select3(cint_bld, as_ge_at, ar_ge_as_at, facex, facey, facez);
      face_s = lp_build_select3(cint_bld, as_ge_at, ar_ge_as_at, snewx, snewy, snewz);
      face_t = lp_build_select3(cint_bld, as_ge_at, ar_ge_as_at, tnewx, tnewy, tnewz);
      face_sdx = lp_build_select3(cint_bld, as_ge_at, ar_ge_as_at, sdxnewx, sdxnewy, sdxnewz);
      face_tdx = lp_build_select3(cint_bld, as_ge_at, ar_ge_as_at, tdxnewx, tdxnewy, tdxnewz);
      face_sdy = lp_build_select3(cint_bld, as_ge_at, ar_ge_as_at, sdynewx, sdynewy, sdynewz);
      face_tdy = lp_build_select3(cint_bld, as_ge_at, ar_ge_as_at, tdynewx, tdynewy, tdynewz);

      face_s = LLVMBuildBitCast(builder, face_s, coord_vec_type, "");
      face_t = LLVMBuildBitCast(builder, face_t, coord_vec_type, "");
      face_sdx = LLVMBuildBitCast(builder, face_sdx, coord_vec_type, "");
      face_tdx = LLVMBuildBitCast(builder, face_tdx, coord_vec_type, "");
      face_sdy = LLVMBuildBitCast(builder, face_sdy, coord_vec_type, "");
      face_tdy = LLVMBuildBitCast(builder, face_tdy, coord_vec_type, "");

      /* deriv math, dx.s = 0.5 * (dx.sc - sc * dx.ma / ma) / ma */
      madxdivma = lp_build_mul(coord_bld, madx, ima);
      tmp = lp_build_mul(coord_bld, madxdivma, face_s);
      tmp = lp_build_sub(coord_bld, face_sdx, tmp);
      derivs_out->ddx[0] = lp_build_mul(coord_bld, tmp, imahalf);

      /* dx.t = 0.5 * (dx.tc - tc * dx.ma / ma) / ma */
      tmp = lp_build_mul(coord_bld, madxdivma, face_t);
      tmp = lp_build_sub(coord_bld, face_tdx, tmp);
      derivs_out->ddx[1] = lp_build_mul(coord_bld, tmp, imahalf);

      /* dy.s = 0.5 * (dy.sc - sc * dy.ma / ma) / ma */
      madydivma = lp_build_mul(coord_bld, mady, ima);
      tmp = lp_build_mul(coord_bld, madydivma, face_s);
      tmp = lp_build_sub(coord_bld, face_sdy, tmp);
      derivs_out->ddy[0] = lp_build_mul(coord_bld, tmp, imahalf);

      /* dy.t = 0.5 * (dy.tc - tc * dy.ma / ma) / ma */
      tmp = lp_build_mul(coord_bld, madydivma, face_t);
      tmp = lp_build_sub(coord_bld, face_tdy, tmp);
      derivs_out->ddy[1] = lp_build_mul(coord_bld, tmp, imahalf);

      signma = LLVMBuildLShr(builder, mai, signshift, "");
      coords[2] = LLVMBuildOr(builder, face, signma, "face");

      /* project coords */
      face_s = lp_build_mul(coord_bld, face_s, imahalfpos);
      face_t = lp_build_mul(coord_bld, face_t, imahalfpos);

      coords[0] = lp_build_add(coord_bld, face_s, posHalf);
      coords[1] = lp_build_add(coord_bld, face_t, posHalf);

      return;
   }

   ma = lp_build_select3(coord_bld, as_ge_at, ar_ge_as_at, s, t, r);
   mai = LLVMBuildBitCast(builder, ma, cint_vec_type, "");
   signmabit = LLVMBuildAnd(builder, mai, signmask, "");

   si = LLVMBuildBitCast(builder, s, cint_vec_type, "");
   ti = LLVMBuildBitCast(builder, t, cint_vec_type, "");
   ri = LLVMBuildBitCast(builder, r, cint_vec_type, "");

   /*
    * compute all possible new s/t coords, which does the mirroring
    * snewx = signma * -r;
    * tnewx = -t;
    * snewy = s;
    * tnewy = signma * r;
    * snewz = signma * s;
    * tnewz = -t;
    */
   tnegi = LLVMBuildXor(builder, ti, signmask, "");
   rnegi = LLVMBuildXor(builder, ri, signmask, "");

   snewx = LLVMBuildXor(builder, signmabit, rnegi, "");
   tnewx = tnegi;

   snewy = si;
   tnewy = LLVMBuildXor(builder, signmabit, ri, "");

   snewz = LLVMBuildXor(builder, signmabit, si, "");
   tnewz = tnegi;

   /* select the mirrored values */
   face_s = lp_build_select3(cint_bld, as_ge_at, ar_ge_as_at, snewx, snewy, snewz);
   face_t = lp_build_select3(cint_bld, as_ge_at, ar_ge_as_at, tnewx, tnewy, tnewz);
   face = lp_build_select3(cint_bld, as_ge_at, ar_ge_as_at, facex, facey, facez);

   face_s = LLVMBuildBitCast(builder, face_s, coord_vec_type, "");
   face_t = LLVMBuildBitCast(builder, face_t, coord_vec_type, "");

   /* add +1 for neg face */
   /* XXX with AVX probably want to use another select here -
    * as long as we ensure vblendvps gets used we can actually
    * skip the comparison and just use sign as a "mask" directly.
    */
   signma = LLVMBuildLShr(builder, mai, signshift, "");
   coords[2] = LLVMBuildOr(builder, face, signma, "face");

   /* project coords */
   imahalfpos = lp_build_cube_imapos(coord_bld, ma);
   face_s = lp_build_mul(coord_bld, face_s, imahalfpos);
   face_t = lp_build_mul(coord_bld, face_t, imahalfpos);

   coords[0] = lp_build_add(coord_bld, face_s, posHalf);
   coords[1] = lp_build_add(coord_bld, face_t, posHalf);
}


/**
 * Compute the partial offset of a pixel block along an arbitrary axis.
 *
 * @param coord   coordinate in pixels
 * @param stride  number of bytes between rows of successive pixel blocks
 * @param block_length  number of pixels in a pixels block along the coordinate
 *                      axis
 * @param out_offset    resulting relative offset of the pixel block in bytes
 * @param out_subcoord  resulting sub-block pixel coordinate
 */
void
lp_build_sample_partial_offset(struct lp_build_context *bld,
                               unsigned block_length,
                               LLVMValueRef coord,
                               LLVMValueRef stride,
                               LLVMValueRef *out_offset,
                               LLVMValueRef *out_subcoord)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef offset;
   LLVMValueRef subcoord;

   if (block_length == 1) {
      subcoord = bld->zero;
   } else {
      /*
       * Pixel blocks have power of two dimensions. LLVM should convert the
       * rem/div to bit arithmetic.
       * TODO: Verify this.
       * It does indeed BUT it does transform it to scalar (and back) when doing so
       * (using roughly extract, shift/and, mov, unpack) (llvm 2.7).
       * The generated code looks seriously unfunny and is quite expensive.
       */
#if 0
      LLVMValueRef block_width = lp_build_const_int_vec(bld->type, block_length);
      subcoord = LLVMBuildURem(builder, coord, block_width, "");
      coord    = LLVMBuildUDiv(builder, coord, block_width, "");
#else
      unsigned logbase2 = util_logbase2(block_length);
      LLVMValueRef block_shift = lp_build_const_int_vec(bld->gallivm, bld->type, logbase2);
      LLVMValueRef block_mask = lp_build_const_int_vec(bld->gallivm, bld->type, block_length - 1);
      subcoord = LLVMBuildAnd(builder, coord, block_mask, "");
      coord = LLVMBuildLShr(builder, coord, block_shift, "");
#endif
   }

   offset = lp_build_mul(bld, coord, stride);

   assert(out_offset);
   assert(out_subcoord);

   *out_offset = offset;
   *out_subcoord = subcoord;
}


/**
 * Compute the offset of a pixel block.
 *
 * x, y, z, y_stride, z_stride are vectors, and they refer to pixels.
 *
 * Returns the relative offset and i,j sub-block coordinates
 */
void
lp_build_sample_offset(struct lp_build_context *bld,
                       const struct util_format_description *format_desc,
                       LLVMValueRef x,
                       LLVMValueRef y,
                       LLVMValueRef z,
                       LLVMValueRef y_stride,
                       LLVMValueRef z_stride,
                       LLVMValueRef *out_offset,
                       LLVMValueRef *out_i,
                       LLVMValueRef *out_j)
{
   LLVMValueRef x_stride;
   LLVMValueRef offset;

   x_stride = lp_build_const_vec(bld->gallivm, bld->type,
                                 format_desc->block.bits/8);

   lp_build_sample_partial_offset(bld,
                                  format_desc->block.width,
                                  x, x_stride,
                                  &offset, out_i);

   if (y && y_stride) {
      LLVMValueRef y_offset;
      lp_build_sample_partial_offset(bld,
                                     format_desc->block.height,
                                     y, y_stride,
                                     &y_offset, out_j);
      offset = lp_build_add(bld, offset, y_offset);
   } else {
      *out_j = bld->zero;
   }

   if (z && z_stride) {
      LLVMValueRef z_offset;
      LLVMValueRef k;
      lp_build_sample_partial_offset(bld,
                                     1, /* pixel blocks are always 2D */
                                     z, z_stride,
                                     &z_offset, &k);
      offset = lp_build_add(bld, offset, z_offset);
   }

   *out_offset = offset;
}


static LLVMValueRef
lp_build_sample_min(struct lp_build_context *bld,
                    LLVMValueRef x,
                    LLVMValueRef v0,
                    LLVMValueRef v1)
{
   /* if the incoming LERP weight is 0 then the min/max
    * should ignore that value. */
   LLVMValueRef mask = lp_build_compare(bld->gallivm,
                                        bld->type,
                                        PIPE_FUNC_NOTEQUAL,
                                        x, bld->zero);
   LLVMValueRef min = lp_build_min(bld, v0, v1);

   return lp_build_select(bld, mask, min, v0);
}


static LLVMValueRef
lp_build_sample_max(struct lp_build_context *bld,
                    LLVMValueRef x,
                    LLVMValueRef v0,
                    LLVMValueRef v1)
{
   /* if the incoming LERP weight is 0 then the min/max
    * should ignore that value. */
   LLVMValueRef mask = lp_build_compare(bld->gallivm,
                                        bld->type,
                                        PIPE_FUNC_NOTEQUAL,
                                        x, bld->zero);
   LLVMValueRef max = lp_build_max(bld, v0, v1);

   return lp_build_select(bld, mask, max, v0);
}


static LLVMValueRef
lp_build_sample_min_2d(struct lp_build_context *bld,
                       LLVMValueRef x,
                       LLVMValueRef y,
                       LLVMValueRef a,
                       LLVMValueRef b,
                       LLVMValueRef c,
                       LLVMValueRef d)
{
   LLVMValueRef v0 = lp_build_sample_min(bld, x, a, b);
   LLVMValueRef v1 = lp_build_sample_min(bld, x, c, d);
   return lp_build_sample_min(bld, y, v0, v1);
}


static LLVMValueRef
lp_build_sample_max_2d(struct lp_build_context *bld,
                       LLVMValueRef x,
                       LLVMValueRef y,
                       LLVMValueRef a,
                       LLVMValueRef b,
                       LLVMValueRef c,
                       LLVMValueRef d)
{
   LLVMValueRef v0 = lp_build_sample_max(bld, x, a, b);
   LLVMValueRef v1 = lp_build_sample_max(bld, x, c, d);
   return lp_build_sample_max(bld, y, v0, v1);
}


static LLVMValueRef
lp_build_sample_min_3d(struct lp_build_context *bld,
                LLVMValueRef x,
                LLVMValueRef y,
                LLVMValueRef z,
                LLVMValueRef a, LLVMValueRef b,
                LLVMValueRef c, LLVMValueRef d,
                LLVMValueRef e, LLVMValueRef f,
                LLVMValueRef g, LLVMValueRef h)
{
   LLVMValueRef v0 = lp_build_sample_min_2d(bld, x, y, a, b, c, d);
   LLVMValueRef v1 = lp_build_sample_min_2d(bld, x, y, e, f, g, h);
   return lp_build_sample_min(bld, z, v0, v1);
}


static LLVMValueRef
lp_build_sample_max_3d(struct lp_build_context *bld,
                       LLVMValueRef x,
                       LLVMValueRef y,
                       LLVMValueRef z,
                       LLVMValueRef a, LLVMValueRef b,
                       LLVMValueRef c, LLVMValueRef d,
                       LLVMValueRef e, LLVMValueRef f,
                       LLVMValueRef g, LLVMValueRef h)
{
   LLVMValueRef v0 = lp_build_sample_max_2d(bld, x, y, a, b, c, d);
   LLVMValueRef v1 = lp_build_sample_max_2d(bld, x, y, e, f, g, h);
   return lp_build_sample_max(bld, z, v0, v1);
}


void
lp_build_reduce_filter(struct lp_build_context *bld,
                       enum pipe_tex_reduction_mode mode,
                       unsigned flags,
                       unsigned num_chan,
                       LLVMValueRef x,
                       LLVMValueRef *v00,
                       LLVMValueRef *v01,
                       LLVMValueRef *out)
{
   unsigned chan;
   switch (mode) {
   case PIPE_TEX_REDUCTION_MIN:
      for (chan = 0; chan < num_chan; chan++)
         out[chan] = lp_build_sample_min(bld, x, v00[chan], v01[chan]);
      break;
   case PIPE_TEX_REDUCTION_MAX:
      for (chan = 0; chan < num_chan; chan++)
         out[chan] = lp_build_sample_max(bld, x, v00[chan], v01[chan]);
      break;
   case PIPE_TEX_REDUCTION_WEIGHTED_AVERAGE:
   default:
      for (chan = 0; chan < num_chan; chan++)
         out[chan] = lp_build_lerp(bld, x, v00[chan], v01[chan], flags);
      break;
   }
}


void
lp_build_reduce_filter_2d(struct lp_build_context *bld,
                          enum pipe_tex_reduction_mode mode,
                          unsigned flags,
                          unsigned num_chan,
                          LLVMValueRef x,
                          LLVMValueRef y,
                          LLVMValueRef *v00,
                          LLVMValueRef *v01,
                          LLVMValueRef *v10,
                          LLVMValueRef *v11,
                          LLVMValueRef *out)
{
   switch (mode) {
   case PIPE_TEX_REDUCTION_MIN:
      for (unsigned chan = 0; chan < num_chan; chan++)
         out[chan] = lp_build_sample_min_2d(bld, x, y, v00[chan], v01[chan],
                                            v10[chan], v11[chan]);
      break;
   case PIPE_TEX_REDUCTION_MAX:
      for (unsigned chan = 0; chan < num_chan; chan++)
         out[chan] = lp_build_sample_max_2d(bld, x, y, v00[chan], v01[chan],
                                            v10[chan], v11[chan]);
      break;
   case PIPE_TEX_REDUCTION_WEIGHTED_AVERAGE:
   default:
      for (unsigned chan = 0; chan < num_chan; chan++)
         out[chan] = lp_build_lerp_2d(bld, x, y, v00[chan], v01[chan],
                                      v10[chan], v11[chan], flags);
      break;
   }
}


void
lp_build_reduce_filter_3d(struct lp_build_context *bld,
                          enum pipe_tex_reduction_mode mode,
                          unsigned flags,
                          unsigned num_chan,
                          LLVMValueRef x,
                          LLVMValueRef y,
                          LLVMValueRef z,
                          LLVMValueRef *v000,
                          LLVMValueRef *v001,
                          LLVMValueRef *v010,
                          LLVMValueRef *v011,
                          LLVMValueRef *v100,
                          LLVMValueRef *v101,
                          LLVMValueRef *v110,
                          LLVMValueRef *v111,
                          LLVMValueRef *out)
{
   switch (mode) {
   case PIPE_TEX_REDUCTION_MIN:
      for (unsigned chan = 0; chan < num_chan; chan++)
         out[chan] = lp_build_sample_min_3d(bld, x, y, z,
                                     v000[chan], v001[chan], v010[chan], v011[chan],
                                     v100[chan], v101[chan], v110[chan], v111[chan]);
      break;
   case PIPE_TEX_REDUCTION_MAX:
      for (unsigned chan = 0; chan < num_chan; chan++)
         out[chan] = lp_build_sample_max_3d(bld, x, y, z,
                                     v000[chan], v001[chan], v010[chan], v011[chan],
                                     v100[chan], v101[chan], v110[chan], v111[chan]);
      break;
   case PIPE_TEX_REDUCTION_WEIGHTED_AVERAGE:
   default:
      for (unsigned chan = 0; chan < num_chan; chan++)
         out[chan] = lp_build_lerp_3d(bld, x, y, z,
                                      v000[chan], v001[chan], v010[chan], v011[chan],
                                      v100[chan], v101[chan], v110[chan], v111[chan],
                                      flags);
      break;
   }
}


/*
 * generated from
 * const float alpha = 2;
 * for (unsigned i = 0; i < WEIGHT_LUT_SIZE; i++) {
 *    const float r2 = (float) i / (float) (WEIGHT_LUT_SIZE - 1);
 *    const float weight = (float)expf(-alpha * r2);
 */
static const float aniso_filter_table[1024] = {
   1.000000, 0.998047, 0.996098, 0.994152, 0.992210, 0.990272, 0.988338, 0.986408,
   0.984481, 0.982559, 0.980640, 0.978724, 0.976813, 0.974905, 0.973001, 0.971100,
   0.969204, 0.967311, 0.965421, 0.963536, 0.961654, 0.959776, 0.957901, 0.956030,
   0.954163, 0.952299, 0.950439, 0.948583, 0.946730, 0.944881, 0.943036, 0.941194,
   0.939356, 0.937521, 0.935690, 0.933862, 0.932038, 0.930218, 0.928401, 0.926588,
   0.924778, 0.922972, 0.921169, 0.919370, 0.917575, 0.915782, 0.913994, 0.912209,
   0.910427, 0.908649, 0.906874, 0.905103, 0.903335, 0.901571, 0.899810, 0.898052,
   0.896298, 0.894548, 0.892801, 0.891057, 0.889317, 0.887580, 0.885846, 0.884116,
   0.882389, 0.880666, 0.878946, 0.877229, 0.875516, 0.873806, 0.872099, 0.870396,
   0.868696, 0.866999, 0.865306, 0.863616, 0.861929, 0.860245, 0.858565, 0.856888,
   0.855215, 0.853544, 0.851877, 0.850213, 0.848553, 0.846896, 0.845241, 0.843591,
   0.841943, 0.840299, 0.838657, 0.837019, 0.835385, 0.833753, 0.832124, 0.830499,
   0.828877, 0.827258, 0.825643, 0.824030, 0.822421, 0.820814, 0.819211, 0.817611,
   0.816014, 0.814420, 0.812830, 0.811242, 0.809658, 0.808076, 0.806498, 0.804923,
   0.803351, 0.801782, 0.800216, 0.798653, 0.797093, 0.795536, 0.793982, 0.792432,
   0.790884, 0.789339, 0.787798, 0.786259, 0.784723, 0.783191, 0.781661, 0.780134,
   0.778610, 0.777090, 0.775572, 0.774057, 0.772545, 0.771037, 0.769531, 0.768028,
   0.766528, 0.765030, 0.763536, 0.762045, 0.760557, 0.759071, 0.757589, 0.756109,
   0.754632, 0.753158, 0.751687, 0.750219, 0.748754, 0.747291, 0.745832, 0.744375,
   0.742921, 0.741470, 0.740022, 0.738577, 0.737134, 0.735694, 0.734258, 0.732823,
   0.731392, 0.729964, 0.728538, 0.727115, 0.725695, 0.724278, 0.722863, 0.721451,
   0.720042, 0.718636, 0.717232, 0.715831, 0.714433, 0.713038, 0.711645, 0.710255,
   0.708868, 0.707483, 0.706102, 0.704723, 0.703346, 0.701972, 0.700601, 0.699233,
   0.697867, 0.696504, 0.695144, 0.693786, 0.692431, 0.691079, 0.689729, 0.688382,
   0.687037, 0.685696, 0.684356, 0.683020, 0.681686, 0.680354, 0.679025, 0.677699,
   0.676376, 0.675054, 0.673736, 0.672420, 0.671107, 0.669796, 0.668488, 0.667182,
   0.665879, 0.664579, 0.663281, 0.661985, 0.660692, 0.659402, 0.658114, 0.656828,
   0.655546, 0.654265, 0.652987, 0.651712, 0.650439, 0.649169, 0.647901, 0.646635,
   0.645372, 0.644112, 0.642854, 0.641598, 0.640345, 0.639095, 0.637846, 0.636601,
   0.635357, 0.634116, 0.632878, 0.631642, 0.630408, 0.629177, 0.627948, 0.626721,
   0.625497, 0.624276, 0.623056, 0.621839, 0.620625, 0.619413, 0.618203, 0.616996,
   0.615790, 0.614588, 0.613387, 0.612189, 0.610994, 0.609800, 0.608609, 0.607421,
   0.606234, 0.605050, 0.603868, 0.602689, 0.601512, 0.600337, 0.599165, 0.597994,
   0.596826, 0.595661, 0.594497, 0.593336, 0.592177, 0.591021, 0.589866, 0.588714,
   0.587564, 0.586417, 0.585272, 0.584128, 0.582988, 0.581849, 0.580712, 0.579578,
   0.578446, 0.577317, 0.576189, 0.575064, 0.573940, 0.572819, 0.571701, 0.570584,
   0.569470, 0.568357, 0.567247, 0.566139, 0.565034, 0.563930, 0.562829, 0.561729,
   0.560632, 0.559537, 0.558444, 0.557354, 0.556265, 0.555179, 0.554094, 0.553012,
   0.551932, 0.550854, 0.549778, 0.548704, 0.547633, 0.546563, 0.545496, 0.544430,
   0.543367, 0.542306, 0.541246, 0.540189, 0.539134, 0.538081, 0.537030, 0.535981,
   0.534935, 0.533890, 0.532847, 0.531806, 0.530768, 0.529731, 0.528696, 0.527664,
   0.526633, 0.525604, 0.524578, 0.523553, 0.522531, 0.521510, 0.520492, 0.519475,
   0.518460, 0.517448, 0.516437, 0.515429, 0.514422, 0.513417, 0.512414, 0.511414,
   0.510415, 0.509418, 0.508423, 0.507430, 0.506439, 0.505450, 0.504462, 0.503477,
   0.502494, 0.501512, 0.500533, 0.499555, 0.498580, 0.497606, 0.496634, 0.495664,
   0.494696, 0.493730, 0.492765, 0.491803, 0.490842, 0.489884, 0.488927, 0.487972,
   0.487019, 0.486068, 0.485118, 0.484171, 0.483225, 0.482281, 0.481339, 0.480399,
   0.479461, 0.478524, 0.477590, 0.476657, 0.475726, 0.474797, 0.473870, 0.472944,
   0.472020, 0.471098, 0.470178, 0.469260, 0.468343, 0.467429, 0.466516, 0.465605,
   0.464695, 0.463788, 0.462882, 0.461978, 0.461075, 0.460175, 0.459276, 0.458379,
   0.457484, 0.456590, 0.455699, 0.454809, 0.453920, 0.453034, 0.452149, 0.451266,
   0.450384, 0.449505, 0.448627, 0.447751, 0.446876, 0.446003, 0.445132, 0.444263,
   0.443395, 0.442529, 0.441665, 0.440802, 0.439941, 0.439082, 0.438224, 0.437368,
   0.436514, 0.435662, 0.434811, 0.433961, 0.433114, 0.432268, 0.431424, 0.430581,
   0.429740, 0.428901, 0.428063, 0.427227, 0.426393, 0.425560, 0.424729, 0.423899,
   0.423071, 0.422245, 0.421420, 0.420597, 0.419776, 0.418956, 0.418137, 0.417321,
   0.416506, 0.415692, 0.414880, 0.414070, 0.413261, 0.412454, 0.411648, 0.410844,
   0.410042, 0.409241, 0.408442, 0.407644, 0.406848, 0.406053, 0.405260, 0.404469,
   0.403679, 0.402890, 0.402103, 0.401318, 0.400534, 0.399752, 0.398971, 0.398192,
   0.397414, 0.396638, 0.395863, 0.395090, 0.394319, 0.393548, 0.392780, 0.392013,
   0.391247, 0.390483, 0.389720, 0.388959, 0.388199, 0.387441, 0.386684, 0.385929,
   0.385175, 0.384423, 0.383672, 0.382923, 0.382175, 0.381429, 0.380684, 0.379940,
   0.379198, 0.378457, 0.377718, 0.376980, 0.376244, 0.375509, 0.374776, 0.374044,
   0.373313, 0.372584, 0.371856, 0.371130, 0.370405, 0.369682, 0.368960, 0.368239,
   0.367520, 0.366802, 0.366086, 0.365371, 0.364657, 0.363945, 0.363234, 0.362525,
   0.361817, 0.361110, 0.360405, 0.359701, 0.358998, 0.358297, 0.357597, 0.356899,
   0.356202, 0.355506, 0.354812, 0.354119, 0.353427, 0.352737, 0.352048, 0.351360,
   0.350674, 0.349989, 0.349306, 0.348623, 0.347942, 0.347263, 0.346585, 0.345908,
   0.345232, 0.344558, 0.343885, 0.343213, 0.342543, 0.341874, 0.341206, 0.340540,
   0.339874, 0.339211, 0.338548, 0.337887, 0.337227, 0.336568, 0.335911, 0.335255,
   0.334600, 0.333947, 0.333294, 0.332643, 0.331994, 0.331345, 0.330698, 0.330052,
   0.329408, 0.328764, 0.328122, 0.327481, 0.326842, 0.326203, 0.325566, 0.324930,
   0.324296, 0.323662, 0.323030, 0.322399, 0.321770, 0.321141, 0.320514, 0.319888,
   0.319263, 0.318639, 0.318017, 0.317396, 0.316776, 0.316157, 0.315540, 0.314924,
   0.314309, 0.313695, 0.313082, 0.312470, 0.311860, 0.311251, 0.310643, 0.310036,
   0.309431, 0.308827, 0.308223, 0.307621, 0.307021, 0.306421, 0.305822, 0.305225,
   0.304629, 0.304034, 0.303440, 0.302847, 0.302256, 0.301666, 0.301076, 0.300488,
   0.299902, 0.299316, 0.298731, 0.298148, 0.297565, 0.296984, 0.296404, 0.295825,
   0.295247, 0.294671, 0.294095, 0.293521, 0.292948, 0.292375, 0.291804, 0.291234,
   0.290666, 0.290098, 0.289531, 0.288966, 0.288401, 0.287838, 0.287276, 0.286715,
   0.286155, 0.285596, 0.285038, 0.284482, 0.283926, 0.283371, 0.282818, 0.282266,
   0.281714, 0.281164, 0.280615, 0.280067, 0.279520, 0.278974, 0.278429, 0.277885,
   0.277342, 0.276801, 0.276260, 0.275721, 0.275182, 0.274645, 0.274108, 0.273573,
   0.273038, 0.272505, 0.271973, 0.271442, 0.270912, 0.270382, 0.269854, 0.269327,
   0.268801, 0.268276, 0.267752, 0.267229, 0.266707, 0.266186, 0.265667, 0.265148,
   0.264630, 0.264113, 0.263597, 0.263082, 0.262568, 0.262056, 0.261544, 0.261033,
   0.260523, 0.260014, 0.259506, 0.259000, 0.258494, 0.257989, 0.257485, 0.256982,
   0.256480, 0.255979, 0.255479, 0.254980, 0.254482, 0.253985, 0.253489, 0.252994,
   0.252500, 0.252007, 0.251515, 0.251023, 0.250533, 0.250044, 0.249555, 0.249068,
   0.248582, 0.248096, 0.247611, 0.247128, 0.246645, 0.246163, 0.245683, 0.245203,
   0.244724, 0.244246, 0.243769, 0.243293, 0.242818, 0.242343, 0.241870, 0.241398,
   0.240926, 0.240456, 0.239986, 0.239517, 0.239049, 0.238583, 0.238117, 0.237651,
   0.237187, 0.236724, 0.236262, 0.235800, 0.235340, 0.234880, 0.234421, 0.233963,
   0.233506, 0.233050, 0.232595, 0.232141, 0.231688, 0.231235, 0.230783, 0.230333,
   0.229883, 0.229434, 0.228986, 0.228538, 0.228092, 0.227647, 0.227202, 0.226758,
   0.226315, 0.225873, 0.225432, 0.224992, 0.224552, 0.224114, 0.223676, 0.223239,
   0.222803, 0.222368, 0.221934, 0.221500, 0.221068, 0.220636, 0.220205, 0.219775,
   0.219346, 0.218917, 0.218490, 0.218063, 0.217637, 0.217212, 0.216788, 0.216364,
   0.215942, 0.215520, 0.215099, 0.214679, 0.214260, 0.213841, 0.213423, 0.213007,
   0.212591, 0.212175, 0.211761, 0.211347, 0.210935, 0.210523, 0.210111, 0.209701,
   0.209291, 0.208883, 0.208475, 0.208068, 0.207661, 0.207256, 0.206851, 0.206447,
   0.206044, 0.205641, 0.205239, 0.204839, 0.204439, 0.204039, 0.203641, 0.203243,
   0.202846, 0.202450, 0.202054, 0.201660, 0.201266, 0.200873, 0.200481, 0.200089,
   0.199698, 0.199308, 0.198919, 0.198530, 0.198143, 0.197756, 0.197369, 0.196984,
   0.196599, 0.196215, 0.195832, 0.195449, 0.195068, 0.194687, 0.194306, 0.193927,
   0.193548, 0.193170, 0.192793, 0.192416, 0.192041, 0.191665, 0.191291, 0.190917,
   0.190545, 0.190172, 0.189801, 0.189430, 0.189060, 0.188691, 0.188323, 0.187955,
   0.187588, 0.187221, 0.186856, 0.186491, 0.186126, 0.185763, 0.185400, 0.185038,
   0.184676, 0.184316, 0.183956, 0.183597, 0.183238, 0.182880, 0.182523, 0.182166,
   0.181811, 0.181455, 0.181101, 0.180747, 0.180394, 0.180042, 0.179690, 0.179339,
   0.178989, 0.178640, 0.178291, 0.177942, 0.177595, 0.177248, 0.176902, 0.176556,
   0.176211, 0.175867, 0.175524, 0.175181, 0.174839, 0.174497, 0.174157, 0.173816,
   0.173477, 0.173138, 0.172800, 0.172462, 0.172126, 0.171789, 0.171454, 0.171119,
   0.170785, 0.170451, 0.170118, 0.169786, 0.169454, 0.169124, 0.168793, 0.168463,
   0.168134, 0.167806, 0.167478, 0.167151, 0.166825, 0.166499, 0.166174, 0.165849,
   0.165525, 0.165202, 0.164879, 0.164557, 0.164236, 0.163915, 0.163595, 0.163275,
   0.162957, 0.162638, 0.162321, 0.162004, 0.161687, 0.161371, 0.161056, 0.160742,
   0.160428, 0.160114, 0.159802, 0.159489, 0.159178, 0.158867, 0.158557, 0.158247,
   0.157938, 0.157630, 0.157322, 0.157014, 0.156708, 0.156402, 0.156096, 0.155791,
   0.155487, 0.155183, 0.154880, 0.154578, 0.154276, 0.153975, 0.153674, 0.153374,
   0.153074, 0.152775, 0.152477, 0.152179, 0.151882, 0.151585, 0.151289, 0.150994,
   0.150699, 0.150404, 0.150111, 0.149817, 0.149525, 0.149233, 0.148941, 0.148650,
   0.148360, 0.148070, 0.147781, 0.147492, 0.147204, 0.146917, 0.146630, 0.146344,
   0.146058, 0.145772, 0.145488, 0.145204, 0.144920, 0.144637, 0.144354, 0.144072,
   0.143791, 0.143510, 0.143230, 0.142950, 0.142671, 0.142392, 0.142114, 0.141837,
   0.141560, 0.141283, 0.141007, 0.140732, 0.140457, 0.140183, 0.139909, 0.139636,
   0.139363, 0.139091, 0.138819, 0.138548, 0.138277, 0.138007, 0.137738, 0.137469,
   0.137200, 0.136932, 0.136665, 0.136398, 0.136131, 0.135865, 0.135600, 0.135335,
};


const float *
lp_build_sample_aniso_filter_table(void)
{
   return aniso_filter_table;
}
