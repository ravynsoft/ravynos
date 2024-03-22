/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
 * Copyright 2007-2008 VMware, Inc.
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
 * Position and shader input interpolation.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#include "pipe/p_shader_tokens.h"
#include "util/compiler.h"
#include "util/u_debug.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "gallivm/lp_bld_debug.h"
#include "gallivm/lp_bld_const.h"
#include "gallivm/lp_bld_arit.h"
#include "gallivm/lp_bld_swizzle.h"
#include "gallivm/lp_bld_flow.h"
#include "gallivm/lp_bld_logic.h"
#include "gallivm/lp_bld_struct.h"
#include "gallivm/lp_bld_gather.h"
#include "lp_bld_interp.h"


/*
 * The shader JIT function operates on blocks of quads.
 * Each block has 2x2 quads and each quad has 2x2 pixels.
 *
 * We iterate over the quads in order 0, 1, 2, 3:
 *
 * #################
 * #   |   #   |   #
 * #---0---#---1---#
 * #   |   #   |   #
 * #################
 * #   |   #   |   #
 * #---2---#---3---#
 * #   |   #   |   #
 * #################
 *
 * If we iterate over multiple quads at once, quads 01 and 23 are processed
 * together.
 *
 * Within each quad, we have four pixels which are represented in SOA
 * order:
 *
 * #########
 * # 0 | 1 #
 * #---+---#
 * # 2 | 3 #
 * #########
 *
 * So the green channel (for example) of the four pixels is stored in
 * a single vector register: {g0, g1, g2, g3}.
 * The order stays the same even with multiple quads:
 * 0 1 4 5
 * 2 3 6 7
 * is stored as g0..g7
 */


/**
 * Do one perspective divide per quad.
 *
 * For perspective interpolation, the final attribute value is given
 *
 *  a' = a/w = a * oow
 *
 * where
 *
 *  a = a0 + dadx*x + dady*y
 *  w = w0 + dwdx*x + dwdy*y
 *  oow = 1/w = 1/(w0 + dwdx*x + dwdy*y)
 *
 * Instead of computing the division per pixel, with this macro we compute the
 * division on the upper left pixel of each quad, and use a linear
 * approximation in the remaining pixels, given by:
 *
 *  da'dx = (dadx - dwdx*a)*oow
 *  da'dy = (dady - dwdy*a)*oow
 *
 * Ironically, this actually makes things slower -- probably because the
 * divide hardware unit is rarely used, whereas the multiply unit is typically
 * already saturated.
 */
#define PERSPECTIVE_DIVIDE_PER_QUAD 0


static const unsigned char quad_offset_x[16] = {0, 1, 0, 1, 2, 3, 2, 3, 0, 1, 0, 1, 2, 3, 2, 3};
static const unsigned char quad_offset_y[16] = {0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 3, 3, 2, 2, 3, 3};


static void
attrib_name(LLVMValueRef val, unsigned attrib, unsigned chan, const char *suffix)
{
   if (attrib == 0)
      lp_build_name(val, "pos.%c%s", "xyzw"[chan], suffix);
   else
      lp_build_name(val, "input%u.%c%s", attrib - 1, "xyzw"[chan], suffix);
}


static void
calc_offsets(struct lp_build_context *coeff_bld,
             unsigned quad_start_index,
             LLVMValueRef *pixoffx,
             LLVMValueRef *pixoffy)
{
   unsigned num_pix = coeff_bld->type.length;
   struct gallivm_state *gallivm = coeff_bld->gallivm;
   LLVMBuilderRef builder = coeff_bld->gallivm->builder;
   LLVMValueRef nr, pixxf, pixyf;

   *pixoffx = coeff_bld->undef;
   *pixoffy = coeff_bld->undef;

   for (unsigned i = 0; i < num_pix; i++) {
      nr = lp_build_const_int32(gallivm, i);
      pixxf = lp_build_const_float(gallivm, quad_offset_x[i % num_pix] +
                                   (quad_start_index & 1) * 2);
      pixyf = lp_build_const_float(gallivm, quad_offset_y[i % num_pix] +
                                   (quad_start_index & 2));
      *pixoffx = LLVMBuildInsertElement(builder, *pixoffx, pixxf, nr, "");
      *pixoffy = LLVMBuildInsertElement(builder, *pixoffy, pixyf, nr, "");
   }
}


static void
calc_centroid_offsets(struct lp_build_interp_soa_context *bld,
                      struct gallivm_state *gallivm,
                      LLVMValueRef loop_iter,
                      LLVMTypeRef mask_type,
                      LLVMValueRef mask_store,
                      LLVMValueRef pix_center_offset,
                      LLVMValueRef *centroid_x, LLVMValueRef *centroid_y)
{
   struct lp_build_context *coeff_bld = &bld->coeff_bld;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef s_mask_and = NULL;
   LLVMValueRef centroid_x_offset = pix_center_offset;
   LLVMValueRef centroid_y_offset = pix_center_offset;
   for (int s = bld->coverage_samples - 1; s >= 0; s--) {
      LLVMValueRef sample_cov;
      LLVMValueRef s_mask_idx = LLVMBuildMul(builder, bld->num_loop, lp_build_const_int32(gallivm, s), "");

      s_mask_idx = LLVMBuildAdd(builder, s_mask_idx, loop_iter, "");
      sample_cov = lp_build_pointer_get2(builder, mask_type, mask_store, s_mask_idx);
      if (s == bld->coverage_samples - 1)
         s_mask_and = sample_cov;
      else
         s_mask_and = LLVMBuildAnd(builder, s_mask_and, sample_cov, "");

      LLVMValueRef x_val_idx = lp_build_const_int32(gallivm, s * 2);
      LLVMValueRef y_val_idx = lp_build_const_int32(gallivm, s * 2 + 1);

      x_val_idx = lp_build_array_get2(gallivm, bld->sample_pos_array_type,
                                      bld->sample_pos_array, x_val_idx);
      y_val_idx = lp_build_array_get2(gallivm, bld->sample_pos_array_type,
                                      bld->sample_pos_array, y_val_idx);
      x_val_idx = lp_build_broadcast_scalar(coeff_bld, x_val_idx);
      y_val_idx = lp_build_broadcast_scalar(coeff_bld, y_val_idx);
      centroid_x_offset = lp_build_select(coeff_bld, sample_cov, x_val_idx, centroid_x_offset);
      centroid_y_offset = lp_build_select(coeff_bld, sample_cov, y_val_idx, centroid_y_offset);
   }
   *centroid_x = lp_build_select(coeff_bld, s_mask_and, pix_center_offset, centroid_x_offset);
   *centroid_y = lp_build_select(coeff_bld, s_mask_and, pix_center_offset, centroid_y_offset);
}


/* Note: this assumes the pointer to elem_type is in address space 0 */
static LLVMValueRef
load_casted(LLVMBuilderRef builder, LLVMTypeRef elem_type,
            LLVMValueRef ptr, const char *name) {
   ptr = LLVMBuildBitCast(builder, ptr, LLVMPointerType(elem_type, 0), name);
   return LLVMBuildLoad2(builder, elem_type, ptr, name);
}


static LLVMValueRef
indexed_load(LLVMBuilderRef builder, LLVMTypeRef gep_type,
                  LLVMTypeRef elem_type, LLVMValueRef ptr,
             LLVMValueRef index, const char *name) {
   ptr = LLVMBuildGEP2(builder, gep_type, ptr, &index, 1, name);
   return load_casted(builder, elem_type, ptr, name);
}


/* Much easier, and significantly less instructions in the per-stamp
 * part (less than half) but overall more instructions so a loss if
 * most quads are active. Might be a win though with larger vectors.
 * No ability to do per-quad divide (doable but not implemented)
 * Could be made to work with passed in pixel offsets (i.e. active quad
 * merging).
 */
static void
coeffs_init_simple(struct lp_build_interp_soa_context *bld,
                   LLVMValueRef a0_ptr,
                   LLVMValueRef dadx_ptr,
                   LLVMValueRef dady_ptr)
{
   struct lp_build_context *coeff_bld = &bld->coeff_bld;
   struct lp_build_context *setup_bld = &bld->setup_bld;
   struct gallivm_state *gallivm = coeff_bld->gallivm;
   LLVMBuilderRef builder = gallivm->builder;

   for (unsigned attrib = 0; attrib < bld->num_attribs; ++attrib) {
      /*
       * always fetch all 4 values for performance/simplicity
       * Note: we do that here because it seems to generate better
       * code. It generates a lot of moves initially but less
       * moves later. As far as I can tell this looks like a
       * llvm issue, instead of simply reloading the values from
       * the passed in pointers it if it runs out of registers
       * it spills/reloads them. Maybe some optimization passes
       * would help.
       * Might want to investigate this again later.
       */
      const enum lp_interp interp = bld->interp[attrib];
      LLVMValueRef index = lp_build_const_int32(gallivm,
                                attrib * TGSI_NUM_CHANNELS);
      LLVMValueRef dadxaos = setup_bld->zero;
      LLVMValueRef dadyaos = setup_bld->zero;
      LLVMValueRef a0aos = setup_bld->zero;

      /* See: lp_state_fs.c / generate_fragment() / fs_elem_type */
      LLVMTypeRef fs_elem_type = LLVMFloatTypeInContext(gallivm->context);

      switch (interp) {
      case LP_INTERP_PERSPECTIVE:
         FALLTHROUGH;

      case LP_INTERP_LINEAR:
         dadxaos = indexed_load(builder, fs_elem_type,
                                setup_bld->vec_type, dadx_ptr, index, "");
         dadyaos = indexed_load(builder, fs_elem_type,
                                setup_bld->vec_type, dady_ptr, index, "");
         attrib_name(dadxaos, attrib, 0, ".dadxaos");
         attrib_name(dadyaos, attrib, 0, ".dadyaos");
         FALLTHROUGH;

      case LP_INTERP_CONSTANT:
      case LP_INTERP_FACING:
         a0aos = indexed_load(builder, fs_elem_type,
                              setup_bld->vec_type, a0_ptr, index, "");
         attrib_name(a0aos, attrib, 0, ".a0aos");
         break;

      case LP_INTERP_POSITION:
         /* Nothing to do as the position coeffs are already setup in slot 0 */
         continue;

      default:
         assert(0);
         break;
      }
      bld->a0aos[attrib] = a0aos;
      bld->dadxaos[attrib] = dadxaos;
      bld->dadyaos[attrib] = dadyaos;
   }
}


/**
 * Interpolate the shader input attribute values.
 * This is called for each (group of) quad(s).
 */
static void
attribs_update_simple(struct lp_build_interp_soa_context *bld,
                      struct gallivm_state *gallivm,
                      LLVMValueRef loop_iter,
                      LLVMTypeRef mask_type,
                      LLVMValueRef mask_store,
                      LLVMValueRef sample_id,
                      int start,
                      int end)
{
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context *coeff_bld = &bld->coeff_bld;
   struct lp_build_context *setup_bld = &bld->setup_bld;
   LLVMValueRef oow = NULL;
   LLVMValueRef pixoffx;
   LLVMValueRef pixoffy;
   LLVMValueRef ptr;
   LLVMValueRef pix_center_offset = lp_build_const_vec(gallivm,
                                                       coeff_bld->type, 0.5);

   /* could do this with code-generated passed in pixel offsets too */

   assert(loop_iter);
   ptr = LLVMBuildGEP2(builder, bld->store_elem_type, bld->xoffset_store,
                       &loop_iter, 1, "");
   pixoffx = LLVMBuildLoad2(builder, bld->store_elem_type, ptr, "");
   ptr = LLVMBuildGEP2(builder, bld->store_elem_type, bld->yoffset_store,
                       &loop_iter, 1, "");
   pixoffy = LLVMBuildLoad2(builder, bld->store_elem_type, ptr, "");

   pixoffx = LLVMBuildFAdd(builder, pixoffx,
                           lp_build_broadcast_scalar(coeff_bld, bld->x), "");
   pixoffy = LLVMBuildFAdd(builder, pixoffy,
                           lp_build_broadcast_scalar(coeff_bld, bld->y), "");

   for (unsigned attrib = start; attrib < end; attrib++) {
      const unsigned mask = bld->mask[attrib];
      const enum lp_interp interp = bld->interp[attrib];
      const enum tgsi_interpolate_loc loc = bld->interp_loc[attrib];

      for (unsigned chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
         if (mask & (1 << chan)) {
            LLVMValueRef index = lp_build_const_int32(gallivm, chan);
            LLVMValueRef dadx = coeff_bld->zero;
            LLVMValueRef dady = coeff_bld->zero;
            LLVMValueRef a = coeff_bld->zero;
            LLVMValueRef chan_pixoffx = pixoffx, chan_pixoffy = pixoffy;

            switch (interp) {
            case LP_INTERP_PERSPECTIVE:
               FALLTHROUGH;

            case LP_INTERP_LINEAR:
               if (attrib == 0 && chan == 0) {
                  dadx = coeff_bld->one;
                  if (sample_id) {
                     LLVMValueRef x_val_idx = LLVMBuildMul(gallivm->builder, sample_id, lp_build_const_int32(gallivm, 2), "");
                     x_val_idx = lp_build_array_get2(gallivm, bld->sample_pos_array_type,
                                                     bld->sample_pos_array, x_val_idx);
                     a = lp_build_broadcast_scalar(coeff_bld, x_val_idx);
                  } else {
                     a = lp_build_const_vec(gallivm, coeff_bld->type, bld->pos_offset);
                  }
               }
               else if (attrib == 0 && chan == 1) {
                  dady = coeff_bld->one;
                  if (sample_id) {
                     LLVMValueRef y_val_idx = LLVMBuildMul(gallivm->builder, sample_id, lp_build_const_int32(gallivm, 2), "");
                     y_val_idx = LLVMBuildAdd(gallivm->builder, y_val_idx, lp_build_const_int32(gallivm, 1), "");
                     y_val_idx = lp_build_array_get2(gallivm, bld->sample_pos_array_type,
                                                     bld->sample_pos_array, y_val_idx);
                     a = lp_build_broadcast_scalar(coeff_bld, y_val_idx);
                  } else {
                     a = lp_build_const_vec(gallivm, coeff_bld->type, bld->pos_offset);
                  }
               }
               else {
                  dadx = lp_build_extract_broadcast(gallivm, setup_bld->type,
                                                    coeff_bld->type, bld->dadxaos[attrib],
                                                    index);
                  dady = lp_build_extract_broadcast(gallivm, setup_bld->type,
                                                    coeff_bld->type, bld->dadyaos[attrib],
                                                    index);
                  a = lp_build_extract_broadcast(gallivm, setup_bld->type,
                                                 coeff_bld->type, bld->a0aos[attrib],
                                                 index);

                  if (bld->coverage_samples > 1) {
                     LLVMValueRef xoffset = pix_center_offset;
                     LLVMValueRef yoffset = pix_center_offset;
                     if (loc == TGSI_INTERPOLATE_LOC_SAMPLE ||
                         (attrib == 0 && chan == 2 && sample_id)) {
                        LLVMValueRef x_val_idx = LLVMBuildMul(gallivm->builder, sample_id, lp_build_const_int32(gallivm, 2), "");
                        LLVMValueRef y_val_idx = LLVMBuildAdd(gallivm->builder, x_val_idx, lp_build_const_int32(gallivm, 1), "");

                        x_val_idx = lp_build_array_get2(gallivm, bld->sample_pos_array_type,
                                                       bld->sample_pos_array, x_val_idx);
                        y_val_idx = lp_build_array_get2(gallivm, bld->sample_pos_array_type,
                                                        bld->sample_pos_array, y_val_idx);
                        xoffset = lp_build_broadcast_scalar(coeff_bld, x_val_idx);
                        yoffset = lp_build_broadcast_scalar(coeff_bld, y_val_idx);
                     } else if (loc == TGSI_INTERPOLATE_LOC_CENTROID) {
                        calc_centroid_offsets(bld, gallivm, loop_iter, mask_type, mask_store,
                                              pix_center_offset, &xoffset, &yoffset);
                     }
                     chan_pixoffx = lp_build_add(coeff_bld, chan_pixoffx, xoffset);
                     chan_pixoffy = lp_build_add(coeff_bld, chan_pixoffy, yoffset);
                  }
               }

               /*
                * a = a0 + (x * dadx + y * dady)
                */
               a = lp_build_fmuladd(builder, dadx, chan_pixoffx, a);
               a = lp_build_fmuladd(builder, dady, chan_pixoffy, a);

               if (interp == LP_INTERP_PERSPECTIVE) {
                  if (oow == NULL) {
                     LLVMValueRef w = bld->attribs[0][3];
                     assert(attrib != 0);
                     assert(bld->mask[0] & TGSI_WRITEMASK_W);
                     oow = lp_build_rcp(coeff_bld, w);
                  }
                  a = lp_build_mul(coeff_bld, a, oow);
               }
               break;

            case LP_INTERP_CONSTANT:
            case LP_INTERP_FACING:
               a = lp_build_extract_broadcast(gallivm, setup_bld->type,
                                              coeff_bld->type, bld->a0aos[attrib],
                                              index);
               break;

            case LP_INTERP_POSITION:
               assert(attrib > 0);
               a = bld->attribs[0][chan];
               break;

            default:
               assert(0);
               break;
            }

            if ((attrib == 0) && (chan == 2)) {
               /* add polygon-offset value, stored in the X component of a0 */
               LLVMValueRef offset =
                  lp_build_extract_broadcast(gallivm, setup_bld->type,
                                             coeff_bld->type, bld->a0aos[0],
                                             lp_build_const_int32(gallivm, 0));
               a = LLVMBuildFAdd(builder, a, offset, "");
            }

            bld->attribs[attrib][chan] = a;
         }
      }
   }
}


static LLVMValueRef
lp_build_interp_soa_indirect(struct lp_build_interp_soa_context *bld,
                             struct gallivm_state *gallivm,
                             unsigned attrib, unsigned chan,
                             LLVMValueRef indir_index,
                             LLVMValueRef pixoffx,
                             LLVMValueRef pixoffy)
{
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context *coeff_bld = &bld->coeff_bld;
   const enum lp_interp interp = bld->interp[attrib];
   LLVMValueRef dadx = coeff_bld->zero;
   LLVMValueRef dady = coeff_bld->zero;
   LLVMValueRef a = coeff_bld->zero;
   LLVMTypeRef u8ptr =
      LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0);

   indir_index = LLVMBuildAdd(builder, indir_index,
                              lp_build_const_int_vec(gallivm, coeff_bld->type,
                                                     attrib), "");
   LLVMValueRef index = LLVMBuildMul(builder, indir_index,
                                     lp_build_const_int_vec(gallivm,
                                                            coeff_bld->type,
                                                            4), "");
   index = LLVMBuildAdd(builder, index,
                        lp_build_const_int_vec(gallivm,
                                               coeff_bld->type, chan), "");

   /* size up to byte indices */
   index = LLVMBuildMul(builder, index,
                        lp_build_const_int_vec(gallivm, coeff_bld->type, 4),
                        "");

   struct lp_type dst_type = coeff_bld->type;
   dst_type.length = 1;
   switch (interp) {
   case LP_INTERP_PERSPECTIVE:
      FALLTHROUGH;
   case LP_INTERP_LINEAR:
      dadx = lp_build_gather(gallivm, coeff_bld->type.length,
                             coeff_bld->type.width, dst_type,
                             true, LLVMBuildBitCast(builder, bld->dadx_ptr,
                                                    u8ptr, ""),
                             index, false);

      dady = lp_build_gather(gallivm, coeff_bld->type.length,
                             coeff_bld->type.width, dst_type,
                             true, LLVMBuildBitCast(builder, bld->dady_ptr,
                                                    u8ptr, ""),
                             index, false);

      a = lp_build_gather(gallivm, coeff_bld->type.length,
                          coeff_bld->type.width, dst_type,
                          true, LLVMBuildBitCast(builder, bld->a0_ptr,
                                                 u8ptr, ""),
                          index, false);

      /*
       * a = a0 + (x * dadx + y * dady)
       */
      a = lp_build_fmuladd(builder, dadx, pixoffx, a);
      a = lp_build_fmuladd(builder, dady, pixoffy, a);

      if (interp == LP_INTERP_PERSPECTIVE) {
        LLVMValueRef w = bld->attribs[0][3];
        assert(attrib != 0);
        assert(bld->mask[0] & TGSI_WRITEMASK_W);
        LLVMValueRef oow = lp_build_rcp(coeff_bld, w);
        a = lp_build_mul(coeff_bld, a, oow);
      }

      break;
   case LP_INTERP_CONSTANT:
   case LP_INTERP_FACING:
      a = lp_build_gather(gallivm, coeff_bld->type.length,
                          coeff_bld->type.width, dst_type,
                          true, LLVMBuildBitCast(builder, bld->a0_ptr,
                                                 u8ptr, ""),
                          index, false);
      break;
   default:
      assert(0);
      break;
   }
   return a;
}


LLVMValueRef
lp_build_interp_soa(struct lp_build_interp_soa_context *bld,
                    struct gallivm_state *gallivm,
                    LLVMValueRef loop_iter,
                    LLVMTypeRef mask_type,
                    LLVMValueRef mask_store,
                    unsigned attrib, unsigned chan,
                    enum tgsi_interpolate_loc loc,
                    LLVMValueRef indir_index,
                    LLVMValueRef offsets[2])
{
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context *coeff_bld = &bld->coeff_bld;
   struct lp_build_context *setup_bld = &bld->setup_bld;
   LLVMValueRef pixoffx;
   LLVMValueRef pixoffy;
   LLVMValueRef ptr;

   /* could do this with code-generated passed in pixel offsets too */

   assert(loop_iter);
   ptr = LLVMBuildGEP2(builder, bld->store_elem_type, bld->xoffset_store,
                       &loop_iter, 1, "");
   pixoffx = LLVMBuildLoad2(builder, bld->store_elem_type, ptr, "");
   ptr = LLVMBuildGEP2(builder, bld->store_elem_type, bld->yoffset_store,
                       &loop_iter, 1, "");
   pixoffy = LLVMBuildLoad2(builder, bld->store_elem_type, ptr, "");

   pixoffx = LLVMBuildFAdd(builder, pixoffx,
                           lp_build_broadcast_scalar(coeff_bld, bld->x), "");
   pixoffy = LLVMBuildFAdd(builder, pixoffy,
                           lp_build_broadcast_scalar(coeff_bld, bld->y), "");

   LLVMValueRef pix_center_offset = lp_build_const_vec(gallivm,
                                                       coeff_bld->type, 0.5);

   if (loc == TGSI_INTERPOLATE_LOC_CENTER) {
      if (bld->coverage_samples > 1) {
         pixoffx = LLVMBuildFAdd(builder, pixoffx, pix_center_offset, "");
         pixoffy = LLVMBuildFAdd(builder, pixoffy, pix_center_offset, "");
      }

      if (offsets[0])
         pixoffx = LLVMBuildFAdd(builder, pixoffx,
                                 offsets[0], "");
      if (offsets[1])
         pixoffy = LLVMBuildFAdd(builder, pixoffy,
                                 offsets[1], "");
   } else if (loc == TGSI_INTERPOLATE_LOC_SAMPLE) {
      LLVMValueRef x_val_idx = LLVMBuildMul(gallivm->builder, offsets[0],
         lp_build_const_int_vec(gallivm, bld->coeff_bld.type, 2 * 4), "");
      LLVMValueRef y_val_idx = LLVMBuildAdd(gallivm->builder, x_val_idx,
         lp_build_const_int_vec(gallivm, bld->coeff_bld.type, 4), "");

      LLVMValueRef base_ptr =
         LLVMBuildBitCast(gallivm->builder,
                          bld->sample_pos_array,
                          LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0), "");
      LLVMValueRef xoffset = lp_build_gather(gallivm,
                                             bld->coeff_bld.type.length,
                                             bld->coeff_bld.type.width,
                                             lp_elem_type(bld->coeff_bld.type),
                                             false,
                                             base_ptr,
                                             x_val_idx, true);
      LLVMValueRef yoffset = lp_build_gather(gallivm,
                                             bld->coeff_bld.type.length,
                                             bld->coeff_bld.type.width,
                                             lp_elem_type(bld->coeff_bld.type),
                                             false,
                                             base_ptr,
                                             y_val_idx, true);

      if (bld->coverage_samples > 1) {
         pixoffx = LLVMBuildFAdd(builder, pixoffx, xoffset, "");
         pixoffy = LLVMBuildFAdd(builder, pixoffy, yoffset, "");
      }
   } else if (loc == TGSI_INTERPOLATE_LOC_CENTROID) {
      LLVMValueRef centroid_x_offset, centroid_y_offset;

      /* for centroid find covered samples for this quad. */
      /* if all samples are covered use pixel centers */
      if (bld->coverage_samples > 1) {
         calc_centroid_offsets(bld, gallivm, loop_iter, mask_type, mask_store,
                               pix_center_offset, &centroid_x_offset,
                               &centroid_y_offset);

         pixoffx = LLVMBuildFAdd(builder, pixoffx, centroid_x_offset, "");
         pixoffy = LLVMBuildFAdd(builder, pixoffy, centroid_y_offset, "");
      }
   }

   // remap attrib properly.
   attrib++;

   if (indir_index)
     return lp_build_interp_soa_indirect(bld, gallivm, attrib, chan,
                                         indir_index, pixoffx, pixoffy);


   const enum lp_interp interp = bld->interp[attrib];
   LLVMValueRef dadx = coeff_bld->zero;
   LLVMValueRef dady = coeff_bld->zero;
   LLVMValueRef a = coeff_bld->zero;

   LLVMValueRef index = lp_build_const_int32(gallivm, chan);

   switch (interp) {
   case LP_INTERP_PERSPECTIVE:
      FALLTHROUGH;
   case LP_INTERP_LINEAR:
      dadx = lp_build_extract_broadcast(gallivm, setup_bld->type,
                                        coeff_bld->type, bld->dadxaos[attrib],
                                        index);

      dady = lp_build_extract_broadcast(gallivm, setup_bld->type,
                                        coeff_bld->type, bld->dadyaos[attrib],
                                        index);

      a = lp_build_extract_broadcast(gallivm, setup_bld->type,
                                     coeff_bld->type, bld->a0aos[attrib],
                                     index);

      /*
       * a = a0 + (x * dadx + y * dady)
       */
      a = lp_build_fmuladd(builder, dadx, pixoffx, a);
      a = lp_build_fmuladd(builder, dady, pixoffy, a);

      if (interp == LP_INTERP_PERSPECTIVE) {
        LLVMValueRef w = bld->attribs[0][3];
        assert(attrib != 0);
        assert(bld->mask[0] & TGSI_WRITEMASK_W);
        LLVMValueRef oow = lp_build_rcp(coeff_bld, w);
        a = lp_build_mul(coeff_bld, a, oow);
      }

      break;
   case LP_INTERP_CONSTANT:
   case LP_INTERP_FACING:
      a = lp_build_extract_broadcast(gallivm, setup_bld->type,
                                     coeff_bld->type, bld->a0aos[attrib],
                                     index);
      break;
   default:
      assert(0);
      break;
   }
   return a;
}


/**
 * Generate the position vectors.
 *
 * Parameter x0, y0 are the integer values with upper left coordinates.
 */
static void
pos_init(struct lp_build_interp_soa_context *bld,
         LLVMValueRef x0,
         LLVMValueRef y0)
{
   LLVMBuilderRef builder = bld->coeff_bld.gallivm->builder;
   struct lp_build_context *coeff_bld = &bld->coeff_bld;

   bld->x = LLVMBuildSIToFP(builder, x0, coeff_bld->elem_type, "");
   bld->y = LLVMBuildSIToFP(builder, y0, coeff_bld->elem_type, "");
}


/**
 * Initialize fragment shader input attribute info.
 */
void
lp_build_interp_soa_init(struct lp_build_interp_soa_context *bld,
                         struct gallivm_state *gallivm,
                         unsigned num_inputs,
                         const struct lp_shader_input *inputs,
                         bool pixel_center_integer,
                         unsigned coverage_samples,
                         LLVMTypeRef sample_pos_array_type,
                         LLVMValueRef sample_pos_array,
                         LLVMValueRef num_loop,
                         LLVMBuilderRef builder,
                         struct lp_type type,
                         LLVMValueRef a0_ptr,
                         LLVMValueRef dadx_ptr,
                         LLVMValueRef dady_ptr,
                         LLVMValueRef x0,
                         LLVMValueRef y0)
{
   struct lp_type coeff_type;
   struct lp_type setup_type;
   unsigned attrib;
   unsigned chan;

   memset(bld, 0, sizeof *bld);

   memset(&coeff_type, 0, sizeof coeff_type);
   coeff_type.floating = true;
   coeff_type.sign = true;
   coeff_type.width = 32;
   coeff_type.length = type.length;

   memset(&setup_type, 0, sizeof setup_type);
   setup_type.floating = true;
   setup_type.sign = true;
   setup_type.width = 32;
   setup_type.length = TGSI_NUM_CHANNELS;


   /* XXX: we don't support interpolating into any other types */
   assert(memcmp(&coeff_type, &type, sizeof coeff_type) == 0);

   lp_build_context_init(&bld->coeff_bld, gallivm, coeff_type);
   lp_build_context_init(&bld->setup_bld, gallivm, setup_type);

   /* For convenience */
   bld->pos = bld->attribs[0];
   bld->inputs = (const LLVMValueRef (*)[TGSI_NUM_CHANNELS]) bld->attribs[1];

   /* Position */
   bld->mask[0] = TGSI_WRITEMASK_XYZW;
   bld->interp[0] = LP_INTERP_LINEAR;
   bld->interp_loc[0] = 0;

   /* Inputs */
   for (attrib = 0; attrib < num_inputs; ++attrib) {
      bld->mask[1 + attrib] = inputs[attrib].usage_mask;
      bld->interp[1 + attrib] = inputs[attrib].interp;
      bld->interp_loc[1 + attrib] = inputs[attrib].location;
   }
   bld->num_attribs = 1 + num_inputs;

   /* needed for indirect */
   bld->a0_ptr = a0_ptr;
   bld->dadx_ptr = dadx_ptr;
   bld->dady_ptr = dady_ptr;

   /* Ensure all masked out input channels have a valid value */
   for (attrib = 0; attrib < bld->num_attribs; ++attrib) {
      for (chan = 0; chan < TGSI_NUM_CHANNELS; ++chan) {
         bld->attribs[attrib][chan] = bld->coeff_bld.undef;
      }
   }

   if (pixel_center_integer) {
      bld->pos_offset = 0.0;
   } else {
      bld->pos_offset = 0.5;
   }
   bld->coverage_samples = coverage_samples;
   bld->num_loop = num_loop;
   bld->sample_pos_array_type = sample_pos_array_type;
   bld->sample_pos_array = sample_pos_array;

   pos_init(bld, x0, y0);

   /*
    * Simple method (single step interpolation) may be slower if vector length
    * is just 4, but the results are different (generally less accurate) with
    * the other method, so always use more accurate version.
    */
   {
      /* XXX this should use a global static table */
      unsigned i;
      unsigned num_loops = 16 / type.length;
      LLVMValueRef pixoffx, pixoffy, index;
      LLVMValueRef ptr;

      bld->store_elem_type = lp_build_vec_type(gallivm, type);
      bld->xoffset_store =
         lp_build_array_alloca(gallivm, bld->store_elem_type,
                               lp_build_const_int32(gallivm, num_loops), "");
      bld->yoffset_store =
         lp_build_array_alloca(gallivm, bld->store_elem_type,
                               lp_build_const_int32(gallivm, num_loops), "");
      for (i = 0; i < num_loops; i++) {
         index = lp_build_const_int32(gallivm, i);
         calc_offsets(&bld->coeff_bld, i*type.length/4, &pixoffx, &pixoffy);
         ptr = LLVMBuildGEP2(builder, bld->store_elem_type,
                             bld->xoffset_store, &index, 1, "");
         LLVMBuildStore(builder, pixoffx, ptr);
         ptr = LLVMBuildGEP2(builder, bld->store_elem_type,
                             bld->yoffset_store, &index, 1, "");
         LLVMBuildStore(builder, pixoffy, ptr);
      }
   }
   coeffs_init_simple(bld, a0_ptr, dadx_ptr, dady_ptr);
}


/*
 * Advance the position and inputs to the given quad within the block.
 */

void
lp_build_interp_soa_update_inputs_dyn(struct lp_build_interp_soa_context *bld,
                                      struct gallivm_state *gallivm,
                                      LLVMValueRef quad_start_index,
                                      LLVMTypeRef mask_type,
                                      LLVMValueRef mask_store,
                                      LLVMValueRef sample_id)
{
   attribs_update_simple(bld, gallivm, quad_start_index, mask_type,
                         mask_store, sample_id, 1, bld->num_attribs);
}


void
lp_build_interp_soa_update_pos_dyn(struct lp_build_interp_soa_context *bld,
                                   struct gallivm_state *gallivm,
                                   LLVMValueRef quad_start_index,
                                   LLVMValueRef sample_id)
{
   attribs_update_simple(bld, gallivm, quad_start_index,
                         NULL, NULL, sample_id, 0, 1);
}

