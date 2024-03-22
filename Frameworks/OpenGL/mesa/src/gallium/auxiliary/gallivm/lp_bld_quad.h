/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/


#ifndef LP_BLD_QUAD_H_
#define LP_BLD_QUAD_H_


#include "gallivm/lp_bld.h"


struct lp_build_context;


/*
 * Each quad is composed of four elements.
 *
 * #########
 * # 0 | 1 #
 * #---+---#
 * # 2 | 3 #
 * #########
 */

enum lp_bld_quad {
   LP_BLD_QUAD_TOP_LEFT     = 0,
   LP_BLD_QUAD_TOP_RIGHT    = 1,
   LP_BLD_QUAD_BOTTOM_LEFT  = 2,
   LP_BLD_QUAD_BOTTOM_RIGHT = 3
};


/*
 * (Vector) derivates.
 *
 * More than one quad is supported. The only requirement is that the vector
 * contains a whole number of quads:
 *
 * ######### ######### ...
 * # 0 | 1 # # 4 | 5 #
 * #---+---# #---+---# ...
 * # 2 | 3 # # 6 | 7 #
 * ######### ######### ...
 */

LLVMValueRef
lp_build_ddx(struct lp_build_context *bld,
             LLVMValueRef a);


LLVMValueRef
lp_build_ddy(struct lp_build_context *bld,
             LLVMValueRef a);


/*
 * Packed derivatives (one derivative for each direction per quad)
 */
LLVMValueRef
lp_build_packed_ddx_ddy_twocoord(struct lp_build_context *bld,
                                 LLVMValueRef a, LLVMValueRef b);

LLVMValueRef
lp_build_packed_ddx_ddy_onecoord(struct lp_build_context *bld,
                                 LLVMValueRef a);

/*
 * Twiddle from quad format to row format
 */
void
lp_bld_quad_twiddle(struct gallivm_state *gallivm,
                    struct lp_type lp_dst_type,
                    const LLVMValueRef* src,
                    unsigned src_count,
                    LLVMValueRef* dst);

#endif /* LP_BLD_QUAD_H_ */
