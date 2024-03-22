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

#include "util/u_debug.h"
#include "util/u_memory.h"

#include "lp_bld_init.h"
#include "lp_bld_type.h"
#include "lp_bld_flow.h"


/**
 * Insert a new block, right where builder is pointing to.
 *
 * This is useful important not only for aesthetic reasons, but also for
 * performance reasons, as frequently run blocks should be laid out next to
 * each other and fall-throughs maximized.
 *
 * See also llvm/lib/Transforms/Scalar/BasicBlockPlacement.cpp.
 *
 * Note: this function has no dependencies on the flow code and could
 * be used elsewhere.
 */
LLVMBasicBlockRef
lp_build_insert_new_block(struct gallivm_state *gallivm, const char *name)
{
   LLVMBasicBlockRef current_block;
   LLVMBasicBlockRef next_block;
   LLVMBasicBlockRef new_block;

   /* get current basic block */
   current_block = LLVMGetInsertBlock(gallivm->builder);

   /* check if there's another block after this one */
   next_block = LLVMGetNextBasicBlock(current_block);
   if (next_block) {
      /* insert the new block before the next block */
      new_block = LLVMInsertBasicBlockInContext(gallivm->context, next_block, name);
   }
   else {
      /* append new block after current block */
      LLVMValueRef function = LLVMGetBasicBlockParent(current_block);
      new_block = LLVMAppendBasicBlockInContext(gallivm->context, function, name);
   }

   return new_block;
}


/**
 * Begin a "skip" block.  Inside this block we can test a condition and
 * skip to the end of the block if the condition is false.
 */
void
lp_build_flow_skip_begin(struct lp_build_skip_context *skip,
                         struct gallivm_state *gallivm)
{
   skip->gallivm = gallivm;
   /* create new basic block */
   skip->block = lp_build_insert_new_block(gallivm, "skip");
}


/**
 * Insert code to test a condition and branch to the end of the current
 * skip block if the condition is true.
 */
void
lp_build_flow_skip_cond_break(struct lp_build_skip_context *skip,
                              LLVMValueRef cond)
{
   LLVMBasicBlockRef new_block;

   new_block = lp_build_insert_new_block(skip->gallivm, "");

   /* if cond is true, goto skip->block, else goto new_block */
   LLVMBuildCondBr(skip->gallivm->builder, cond, skip->block, new_block);

   LLVMPositionBuilderAtEnd(skip->gallivm->builder, new_block);
}


void
lp_build_flow_skip_end(struct lp_build_skip_context *skip)
{
   /* goto block */
   LLVMBuildBr(skip->gallivm->builder, skip->block);
   LLVMPositionBuilderAtEnd(skip->gallivm->builder, skip->block);
}


/**
 * Check if the mask predicate is zero.  If so, jump to the end of the block.
 */
void
lp_build_mask_check(struct lp_build_mask_context *mask)
{
   LLVMBuilderRef builder = mask->skip.gallivm->builder;
   LLVMValueRef value;
   LLVMValueRef cond;

   value = lp_build_mask_value(mask);

   /*
    * XXX this doesn't quite generate the most efficient code possible, if
    * the masks are vectors which have all bits set to the same value
    * in each element.
    * movmskps/pmovmskb would be more efficient to get the required value
    * into ordinary reg (certainly with 8 floats).
    * Not sure if llvm could figure that out on its own.
    */

   /* cond = (mask == 0) */
   cond = LLVMBuildICmp(builder,
                        LLVMIntEQ,
                        LLVMBuildBitCast(builder, value, mask->reg_type, ""),
                        LLVMConstNull(mask->reg_type),
                        "");

   /* if cond, goto end of block */
   lp_build_flow_skip_cond_break(&mask->skip, cond);
}


/**
 * Begin a section of code which is predicated on a mask.
 * \param mask  the mask context, initialized here
 * \param flow  the flow context
 * \param type  the type of the mask
 * \param value  storage for the mask
 */
void
lp_build_mask_begin(struct lp_build_mask_context *mask,
                    struct gallivm_state *gallivm,
                    struct lp_type type,
                    LLVMValueRef value)
{
   memset(mask, 0, sizeof *mask);

   mask->reg_type = LLVMIntTypeInContext(gallivm->context, type.width * type.length);
   mask->var_type = lp_build_int_vec_type(gallivm, type);
   mask->var = lp_build_alloca(gallivm,
                               mask->var_type,
                               "execution_mask");

   LLVMBuildStore(gallivm->builder, value, mask->var);

   lp_build_flow_skip_begin(&mask->skip, gallivm);
}


LLVMValueRef
lp_build_mask_value(struct lp_build_mask_context *mask)
{
   return LLVMBuildLoad2(mask->skip.gallivm->builder, mask->var_type, mask->var, "");
}


/**
 * Update boolean mask with given value (bitwise AND).
 * Typically used to update the quad's pixel alive/killed mask
 * after depth testing, alpha testing, TGSI_OPCODE_KILL_IF, etc.
 */
void
lp_build_mask_update(struct lp_build_mask_context *mask,
                     LLVMValueRef value)
{
   value = LLVMBuildAnd(mask->skip.gallivm->builder,
                        lp_build_mask_value(mask),
                        value, "");
   LLVMBuildStore(mask->skip.gallivm->builder, value, mask->var);
}

/*
 * Update boolean mask with given value.
 * Used for per-sample shading to force per-sample execution masks.
 */
void
lp_build_mask_force(struct lp_build_mask_context *mask,
                    LLVMValueRef value)
{
   LLVMBuildStore(mask->skip.gallivm->builder, value, mask->var);
}

/**
 * End section of code which is predicated on a mask.
 */
LLVMValueRef
lp_build_mask_end(struct lp_build_mask_context *mask)
{
   lp_build_flow_skip_end(&mask->skip);
   return lp_build_mask_value(mask);
}



void
lp_build_loop_begin(struct lp_build_loop_state *state,
                    struct gallivm_state *gallivm,
                    LLVMValueRef start)
                    
{
   LLVMBuilderRef builder = gallivm->builder;

   state->block = lp_build_insert_new_block(gallivm, "loop_begin");

   state->counter_type = LLVMTypeOf(start);
   state->counter_var = lp_build_alloca(gallivm, state->counter_type, "loop_counter");
   state->gallivm = gallivm;

   LLVMBuildStore(builder, start, state->counter_var);

   LLVMBuildBr(builder, state->block);

   LLVMPositionBuilderAtEnd(builder, state->block);

   state->counter = LLVMBuildLoad2(builder, state->counter_type, state->counter_var, "");
}


void
lp_build_loop_end_cond(struct lp_build_loop_state *state,
                       LLVMValueRef end,
                       LLVMValueRef step,
                       LLVMIntPredicate llvm_cond)
{
   LLVMBuilderRef builder = state->gallivm->builder;
   LLVMValueRef next;
   LLVMValueRef cond;
   LLVMBasicBlockRef after_block;

   if (!step)
      step = LLVMConstInt(LLVMTypeOf(end), 1, 0);

   next = LLVMBuildAdd(builder, state->counter, step, "");

   LLVMBuildStore(builder, next, state->counter_var);

   cond = LLVMBuildICmp(builder, llvm_cond, next, end, "");

   after_block = lp_build_insert_new_block(state->gallivm, "loop_end");

   LLVMBuildCondBr(builder, cond, after_block, state->block);

   LLVMPositionBuilderAtEnd(builder, after_block);

   state->counter = LLVMBuildLoad2(builder, state->counter_type, state->counter_var, "");
}

void
lp_build_loop_force_set_counter(struct lp_build_loop_state *state,
                          LLVMValueRef end)
{
   LLVMBuilderRef builder = state->gallivm->builder;
   LLVMBuildStore(builder, end, state->counter_var);
}

void
lp_build_loop_force_reload_counter(struct lp_build_loop_state *state)
{
   LLVMBuilderRef builder = state->gallivm->builder;
   state->counter = LLVMBuildLoad2(builder, state->counter_type, state->counter_var, "");
}

void
lp_build_loop_end(struct lp_build_loop_state *state,
                  LLVMValueRef end,
                  LLVMValueRef step)
{
   lp_build_loop_end_cond(state, end, step, LLVMIntNE);
}

/**
 * Creates a c-style for loop,
 * contrasts lp_build_loop as this checks condition on entry
 * e.g. for(i = start; i cmp_op end; i += step)
 * \param state      the for loop state, initialized here
 * \param gallivm    the gallivm state
 * \param start      starting value of iterator
 * \param cmp_op     comparison operator used for comparing current value with end value
 * \param end        value used to compare against iterator
 * \param step       value added to iterator at end of each loop
 */
void
lp_build_for_loop_begin(struct lp_build_for_loop_state *state,
                        struct gallivm_state *gallivm,
                        LLVMValueRef start,
                        LLVMIntPredicate cmp_op,
                        LLVMValueRef end,
                        LLVMValueRef step)
{
   LLVMBuilderRef builder = gallivm->builder;

   assert(LLVMTypeOf(start) == LLVMTypeOf(end));
   assert(LLVMTypeOf(start) == LLVMTypeOf(step));

   state->begin = lp_build_insert_new_block(gallivm, "loop_begin");
   state->step  = step;
   state->counter_type = LLVMTypeOf(start);
   state->counter_var = lp_build_alloca(gallivm, state->counter_type, "loop_counter");
   state->gallivm = gallivm;
   state->cond = cmp_op;
   state->end = end;

   LLVMBuildStore(builder, start, state->counter_var);
   LLVMBuildBr(builder, state->begin);

   LLVMPositionBuilderAtEnd(builder, state->begin);
   state->counter = LLVMBuildLoad2(builder, state->counter_type, state->counter_var, "");

   state->body = lp_build_insert_new_block(gallivm, "loop_body");
   LLVMPositionBuilderAtEnd(builder, state->body);
}

/**
 * End the for loop.
 */
void
lp_build_for_loop_end(struct lp_build_for_loop_state *state)
{
   LLVMValueRef next, cond;
   LLVMBuilderRef builder = state->gallivm->builder;

   next = LLVMBuildAdd(builder, state->counter, state->step, "");
   LLVMBuildStore(builder, next, state->counter_var);
   LLVMBuildBr(builder, state->begin);

   state->exit = lp_build_insert_new_block(state->gallivm, "loop_exit");

   /*
    * We build the comparison for the begin block here,
    * if we build it earlier the output llvm ir is not human readable
    * as the code produced is not in the standard begin -> body -> end order.
    */
   LLVMPositionBuilderAtEnd(builder, state->begin);
   cond = LLVMBuildICmp(builder, state->cond, state->counter, state->end, "");
   LLVMBuildCondBr(builder, cond, state->body, state->exit);

   LLVMPositionBuilderAtEnd(builder, state->exit);
}


/*
  Example of if/then/else building:

     int x;
     if (cond) {
        x = 1 + 2;
     }
     else {
        x = 2 + 3;
     }

  Is built with:

     // x needs an alloca variable
     x = lp_build_alloca(builder, type, "x");


     lp_build_if(ctx, builder, cond);
        LLVMBuildStore(LLVMBuildAdd(1, 2), x);
     lp_build_else(ctx);
        LLVMBuildStore(LLVMBuildAdd(2, 3). x);
     lp_build_endif(ctx);

 */



/**
 * Begin an if/else/endif construct.
 */
void
lp_build_if(struct lp_build_if_state *ifthen,
            struct gallivm_state *gallivm,
            LLVMValueRef condition)
{
   LLVMBasicBlockRef block = LLVMGetInsertBlock(gallivm->builder);

   memset(ifthen, 0, sizeof *ifthen);
   ifthen->gallivm = gallivm;
   ifthen->condition = condition;
   ifthen->entry_block = block;

   /* create endif/merge basic block for the phi functions */
   ifthen->merge_block = lp_build_insert_new_block(gallivm, "endif-block");

   /* create/insert true_block before merge_block */
   ifthen->true_block =
      LLVMInsertBasicBlockInContext(gallivm->context,
                                    ifthen->merge_block,
                                    "if-true-block");

   /* successive code goes into the true block */
   LLVMPositionBuilderAtEnd(gallivm->builder, ifthen->true_block);
}


/**
 * Begin else-part of a conditional
 */
void
lp_build_else(struct lp_build_if_state *ifthen)
{
   LLVMBuilderRef builder = ifthen->gallivm->builder;

   /* Append an unconditional Br(anch) instruction on the true_block */
   LLVMBuildBr(builder, ifthen->merge_block);

   /* create/insert false_block before the merge block */
   ifthen->false_block =
      LLVMInsertBasicBlockInContext(ifthen->gallivm->context,
                                    ifthen->merge_block,
                                    "if-false-block");

   /* successive code goes into the else block */
   LLVMPositionBuilderAtEnd(builder, ifthen->false_block);
}


/**
 * End a conditional.
 */
void
lp_build_endif(struct lp_build_if_state *ifthen)
{
   LLVMBuilderRef builder = ifthen->gallivm->builder;

   /* Insert branch to the merge block from current block */
   LLVMBuildBr(builder, ifthen->merge_block);

   /*
    * Now patch in the various branch instructions.
    */

   /* Insert the conditional branch instruction at the end of entry_block */
   LLVMPositionBuilderAtEnd(builder, ifthen->entry_block);
   if (ifthen->false_block) {
      /* we have an else clause */
      LLVMBuildCondBr(builder, ifthen->condition,
                      ifthen->true_block, ifthen->false_block);
   }
   else {
      /* no else clause */
      LLVMBuildCondBr(builder, ifthen->condition,
                      ifthen->true_block, ifthen->merge_block);
   }

   /* Resume building code at end of the ifthen->merge_block */
   LLVMPositionBuilderAtEnd(builder, ifthen->merge_block);
}


static LLVMBuilderRef
create_builder_at_entry(struct gallivm_state *gallivm)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
   LLVMValueRef function = LLVMGetBasicBlockParent(current_block);
   LLVMBasicBlockRef first_block = LLVMGetEntryBasicBlock(function);
   LLVMValueRef first_instr = LLVMGetFirstInstruction(first_block);
   LLVMBuilderRef first_builder = LLVMCreateBuilderInContext(gallivm->context);

   if (first_instr) {
      LLVMPositionBuilderBefore(first_builder, first_instr);
   } else {
      LLVMPositionBuilderAtEnd(first_builder, first_block);
   }

   return first_builder;
}


/**
 * Allocate a scalar (or vector) variable.
 *
 * Although not strictly part of control flow, control flow has deep impact in
 * how variables should be allocated.
 *
 * The mem2reg optimization pass is the recommended way to dealing with mutable
 * variables, and SSA. It looks for allocas and if it can handle them, it
 * promotes them, but only looks for alloca instructions in the entry block of
 * the function. Being in the entry block guarantees that the alloca is only
 * executed once, which makes analysis simpler.
 *
 * See also:
 * - http://www.llvm.org/docs/tutorial/OCamlLangImpl7.html#memory
 */
LLVMValueRef
lp_build_alloca(struct gallivm_state *gallivm,
                LLVMTypeRef type,
                const char *name)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMBuilderRef first_builder = create_builder_at_entry(gallivm);
   LLVMValueRef res;

   res = LLVMBuildAlloca(first_builder, type, name);
   LLVMBuildStore(builder, LLVMConstNull(type), res);

   LLVMDisposeBuilder(first_builder);

   return res;
}


/**
 * Like lp_build_alloca, but do not zero-initialize the variable.
 */
LLVMValueRef
lp_build_alloca_undef(struct gallivm_state *gallivm,
                      LLVMTypeRef type,
                      const char *name)
{
   LLVMBuilderRef first_builder = create_builder_at_entry(gallivm);
   LLVMValueRef res;

   res = LLVMBuildAlloca(first_builder, type, name);

   LLVMDisposeBuilder(first_builder);

   return res;
}


/**
 * Allocate an array of scalars/vectors.
 *
 * mem2reg pass is not capable of promoting structs or arrays to registers, but
 * we still put it in the first block anyway as failure to put allocas in the
 * first block may prevent the X86 backend from successfully align the stack as
 * required.
 *
 * Also the scalarrepl pass is supposedly more powerful and can promote
 * arrays in many cases.
 *
 * See also:
 * - http://www.llvm.org/docs/tutorial/OCamlLangImpl7.html#memory
 */
LLVMValueRef
lp_build_array_alloca(struct gallivm_state *gallivm,
                      LLVMTypeRef type,
                      LLVMValueRef count,
                      const char *name)
{
   LLVMBuilderRef first_builder = create_builder_at_entry(gallivm);
   LLVMValueRef res;

   res = LLVMBuildArrayAlloca(first_builder, type, count, name);

   LLVMDisposeBuilder(first_builder);

   return res;
}
