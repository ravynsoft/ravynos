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
 * Position and shader input interpolation.
 *
 * Special attention is given to the interpolation of side by side quads.
 * Multiplications are made only for the first quad. Interpolation of
 * inputs for posterior quads are done exclusively with additions, and
 * perspective divide if necessary.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#ifndef LP_BLD_INTERP_H
#define LP_BLD_INTERP_H


#include "gallivm/lp_bld.h"
#include "gallivm/lp_bld_type.h"

#include "tgsi/tgsi_exec.h"

/**
 * Describes how to compute the interpolation coefficients (a0, dadx, dady)
 * from the vertices passed into our triangle/line/point functions by the
 * draw module.
 *
 * Vertices are treated as an array of float[4] values, indexed by
 * src_index.
 *
 * LP_INTERP_COLOR is translated to either LP_INTERP_CONSTANT or
 * PERSPECTIVE depending on flatshade state.
 */
enum lp_interp {
   LP_INTERP_CONSTANT,
   LP_INTERP_COLOR,
   LP_INTERP_LINEAR,
   LP_INTERP_PERSPECTIVE,
   LP_INTERP_POSITION,
   LP_INTERP_FACING
};

struct lp_shader_input {
   uint interp:4;       /* enum lp_interp */
   uint usage_mask:4;   /* bitmask of TGSI_WRITEMASK_x flags */
   uint src_index:8;    /* where to find values in incoming vertices */
   uint location:2;     /* TGSI_INTERPOLATE_LOC_* */
   uint padding:14;
};


struct lp_build_interp_soa_context
{
   /* TGSI_QUAD_SIZE x float */
   struct lp_build_context coeff_bld;
   struct lp_build_context setup_bld;

   unsigned num_attribs;
   unsigned mask[1 + PIPE_MAX_SHADER_INPUTS]; /**< TGSI_WRITE_MASK_x */
   enum lp_interp interp[1 + PIPE_MAX_SHADER_INPUTS];
   unsigned interp_loc[1 + PIPE_MAX_SHADER_INPUTS];
   bool depth_clamp;

   double pos_offset;
   unsigned coverage_samples;
   LLVMValueRef num_loop;
   LLVMTypeRef sample_pos_array_type;
   LLVMValueRef sample_pos_array;

   LLVMValueRef x;
   LLVMValueRef y;

   LLVMValueRef a0_ptr;
   LLVMValueRef dadx_ptr;
   LLVMValueRef dady_ptr;

   LLVMValueRef a0aos[1 + PIPE_MAX_SHADER_INPUTS];
   LLVMValueRef dadxaos[1 + PIPE_MAX_SHADER_INPUTS];
   LLVMValueRef dadyaos[1 + PIPE_MAX_SHADER_INPUTS];

   LLVMValueRef attribs[1 + PIPE_MAX_SHADER_INPUTS][TGSI_NUM_CHANNELS];

   LLVMValueRef xoffset_store;
   LLVMValueRef yoffset_store;
   LLVMTypeRef store_elem_type;

   /*
    * Convenience pointers. Callers may access this one.
    */
   const LLVMValueRef *pos;
   const LLVMValueRef (*inputs)[TGSI_NUM_CHANNELS];
};


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
                         LLVMValueRef x,
                         LLVMValueRef y);

void
lp_build_interp_soa_update_inputs_dyn(struct lp_build_interp_soa_context *bld,
                                      struct gallivm_state *gallivm,
                                      LLVMValueRef quad_start_index,
                                      LLVMTypeRef mask_type,
                                      LLVMValueRef mask_store,
                                      LLVMValueRef sample_id);

void
lp_build_interp_soa_update_pos_dyn(struct lp_build_interp_soa_context *bld,
                                   struct gallivm_state *gallivm,
                                   LLVMValueRef quad_start_index,
                                   LLVMValueRef sample_id);

LLVMValueRef
lp_build_interp_soa(struct lp_build_interp_soa_context *bld,
                    struct gallivm_state *gallivm,
                    LLVMValueRef loop_iter,
                    LLVMTypeRef mask_type,
                    LLVMValueRef mask_store,
                    unsigned attrib, unsigned chan,
                    enum tgsi_interpolate_loc loc,
                    LLVMValueRef indir_index,
                    LLVMValueRef offsets[2]);

#endif /* LP_BLD_INTERP_H */
