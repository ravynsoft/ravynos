/**************************************************************************
 *
 * Copyright 2011-2012 Advanced Micro Devices, Inc.
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

#ifndef LP_BLD_IR_COMMON_H
#define LP_BLD_IR_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gallivm/lp_bld.h"
#include "gallivm/lp_bld_limits.h"

/* SM 4.0 says that subroutines can nest 32 deep and
 * we need one more for our main function */
#define LP_MAX_NUM_FUNCS 33

enum lp_exec_mask_break_type {
   LP_EXEC_MASK_BREAK_TYPE_LOOP,
   LP_EXEC_MASK_BREAK_TYPE_SWITCH
};

struct lp_exec_mask {
   struct lp_build_context *bld;

   bool has_mask;
   bool ret_in_main;

   LLVMTypeRef int_vec_type;

   LLVMValueRef exec_mask;

   LLVMValueRef ret_mask;
   LLVMValueRef cond_mask;
   LLVMValueRef switch_mask;         /* current switch exec mask */
   LLVMValueRef cont_mask;
   LLVMValueRef break_mask;

   struct function_ctx {
      int pc;
      LLVMValueRef ret_mask;

      LLVMValueRef cond_stack[LP_MAX_TGSI_NESTING];
      int cond_stack_size;

      /* keep track if break belongs to switch or loop */
      enum lp_exec_mask_break_type break_type_stack[LP_MAX_TGSI_NESTING];
      enum lp_exec_mask_break_type break_type;

      struct {
         LLVMValueRef switch_val;
         LLVMValueRef switch_mask;
         LLVMValueRef switch_mask_default;
         bool switch_in_default;
         unsigned switch_pc;
      } switch_stack[LP_MAX_TGSI_NESTING];
      int switch_stack_size;
      LLVMValueRef switch_val;
      LLVMValueRef switch_mask_default; /* reverse of switch mask used for default */
      bool switch_in_default;        /* if switch exec is currently in default */
      unsigned switch_pc;               /* when used points to default or endswitch-1 */

      LLVMValueRef loop_limiter;
      LLVMBasicBlockRef loop_block;
      LLVMValueRef break_var;
      struct {
         LLVMBasicBlockRef loop_block;
         LLVMValueRef cont_mask;
         LLVMValueRef break_mask;
         LLVMValueRef break_var;
      } loop_stack[LP_MAX_TGSI_NESTING];
      int loop_stack_size;
      int bgnloop_stack_size;

   } *function_stack;
   int function_stack_size;
};

struct lp_build_mask_context;

void lp_exec_mask_function_init(struct lp_exec_mask *mask, int function_idx);
void lp_exec_mask_init(struct lp_exec_mask *mask, struct lp_build_context *bld);
void lp_exec_mask_fini(struct lp_exec_mask *mask);
void lp_exec_mask_store(struct lp_exec_mask *mask,
                        struct lp_build_context *bld_store,
                        LLVMValueRef val,
                        LLVMValueRef dst_ptr);
void lp_exec_mask_update(struct lp_exec_mask *mask);
void lp_exec_bgnloop_post_phi(struct lp_exec_mask *mask);
void lp_exec_bgnloop(struct lp_exec_mask *mask, bool load_mask);
void lp_exec_endloop(struct gallivm_state *gallivm,
                     struct lp_exec_mask *exec_mask,
                     struct lp_build_mask_context *mask);
void lp_exec_mask_cond_push(struct lp_exec_mask *mask,
                            LLVMValueRef val);
void lp_exec_mask_cond_invert(struct lp_exec_mask *mask);
void lp_exec_mask_cond_pop(struct lp_exec_mask *mask);
void lp_exec_continue(struct lp_exec_mask *mask);

void lp_exec_break(struct lp_exec_mask *mask, int *pc, bool break_always);

#ifdef __cplusplus
}
#endif

#endif
