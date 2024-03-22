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
 * LLVM control flow build helpers.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#ifndef LP_BLD_FLOW_H
#define LP_BLD_FLOW_H


#include "gallivm/lp_bld.h"

#ifdef __cplusplus
extern "C" {
#endif

struct lp_type;


/**
 * Early exit. Useful to skip to the end of a function or block when
 * the execution mask becomes zero or when there is an error condition.
 */
struct lp_build_skip_context
{
   struct gallivm_state *gallivm;

   /** Block to skip to */
   LLVMBasicBlockRef block;
};

void
lp_build_flow_skip_begin(struct lp_build_skip_context *ctx,
                         struct gallivm_state *gallivm);

void
lp_build_flow_skip_cond_break(struct lp_build_skip_context *ctx,
                              LLVMValueRef cond);

void
lp_build_flow_skip_end(struct lp_build_skip_context *ctx);


struct lp_build_mask_context
{
   struct lp_build_skip_context skip;

   LLVMTypeRef reg_type;
   LLVMTypeRef var_type;
   /* 'var' is a pointer (alloca) pointing to 'var_type' */
   LLVMValueRef var;
};


void
lp_build_mask_begin(struct lp_build_mask_context *mask,
                    struct gallivm_state *gallivm,
                    struct lp_type type,
                    LLVMValueRef value);

LLVMValueRef
lp_build_mask_value(struct lp_build_mask_context *mask);

/**
 * Bitwise AND the mask with the given value, if a previous mask was set.
 */
void
lp_build_mask_update(struct lp_build_mask_context *mask,
                     LLVMValueRef value);

void
lp_build_mask_force(struct lp_build_mask_context *mask,
                    LLVMValueRef value);

void
lp_build_mask_check(struct lp_build_mask_context *mask);

LLVMValueRef
lp_build_mask_end(struct lp_build_mask_context *mask);


/**
 * LLVM's IR doesn't represent for-loops directly. Furthermore it
 * requires creating code blocks, branches, phi variables, so it
 * requires a fair amount of code.
 *
 * @sa http://www.llvm.org/docs/tutorial/LangImpl5.html#for
 */
struct lp_build_loop_state
{
   LLVMBasicBlockRef block;
   LLVMValueRef counter_var;
   LLVMValueRef counter;
   LLVMTypeRef counter_type;
   struct gallivm_state *gallivm;
};


void
lp_build_loop_begin(struct lp_build_loop_state *state,
                    struct gallivm_state *gallivm,
                    LLVMValueRef start);

void
lp_build_loop_end(struct lp_build_loop_state *state,
                  LLVMValueRef end,
                  LLVMValueRef step);

void
lp_build_loop_force_set_counter(struct lp_build_loop_state *state,
                                LLVMValueRef end);

void
lp_build_loop_force_reload_counter(struct lp_build_loop_state *state);
void
lp_build_loop_end_cond(struct lp_build_loop_state *state,
                       LLVMValueRef end,
                       LLVMValueRef step,
                       LLVMIntPredicate cond);


/**
 * Implementation of simple C-style for loops
 */
struct lp_build_for_loop_state
{
   LLVMBasicBlockRef begin;
   LLVMBasicBlockRef body;
   LLVMBasicBlockRef exit;
   LLVMValueRef counter_var;
   LLVMValueRef counter;
   LLVMTypeRef counter_type;
   LLVMValueRef step;
   LLVMIntPredicate cond;
   LLVMValueRef end;
   struct gallivm_state *gallivm;
};

void
lp_build_for_loop_begin(struct lp_build_for_loop_state *state,
                        struct gallivm_state *gallivm,
                        LLVMValueRef start,
                        LLVMIntPredicate llvm_cond,
                        LLVMValueRef end,
                        LLVMValueRef step);

void
lp_build_for_loop_end(struct lp_build_for_loop_state *state);


/**
 * if/else/endif.
 */
struct lp_build_if_state
{
   struct gallivm_state *gallivm;
   LLVMValueRef condition;
   LLVMBasicBlockRef entry_block;
   LLVMBasicBlockRef true_block;
   LLVMBasicBlockRef false_block;
   LLVMBasicBlockRef merge_block;
};


void
lp_build_if(struct lp_build_if_state *ctx,
            struct gallivm_state *gallivm,
            LLVMValueRef condition);

void
lp_build_else(struct lp_build_if_state *ctx);

void
lp_build_endif(struct lp_build_if_state *ctx);

LLVMBasicBlockRef
lp_build_insert_new_block(struct gallivm_state *gallivm, const char *name);

LLVMValueRef
lp_build_alloca(struct gallivm_state *gallivm,
                LLVMTypeRef type,
                const char *name);

LLVMValueRef
lp_build_alloca_undef(struct gallivm_state *gallivm,
                      LLVMTypeRef type,
                      const char *name);

LLVMValueRef
lp_build_array_alloca(struct gallivm_state *gallivm,
                      LLVMTypeRef type,
                      LLVMValueRef count,
                      const char *name);

#ifdef __cplusplus
}
#endif

#endif /* !LP_BLD_FLOW_H */
