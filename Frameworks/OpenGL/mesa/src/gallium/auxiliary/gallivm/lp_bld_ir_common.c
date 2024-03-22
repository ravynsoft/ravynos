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

#include "util/u_memory.h"
#include "lp_bld_const.h"
#include "lp_bld_type.h"
#include "lp_bld_init.h"
#include "lp_bld_flow.h"
#include "lp_bld_ir_common.h"
#include "lp_bld_logic.h"

/*
 * Return the context for the current function.
 * (always 'main', if shader doesn't do any function calls)
 */
static inline struct function_ctx *
func_ctx(struct lp_exec_mask *mask)
{
   assert(mask->function_stack_size > 0);
   assert(mask->function_stack_size <= LP_MAX_NUM_FUNCS);
   return &mask->function_stack[mask->function_stack_size - 1];
}

/*
 * Returns true if we're in a loop.
 * It's global, meaning that it returns true even if there's
 * no loop inside the current function, but we were inside
 * a loop inside another function, from which this one was called.
 */
static inline bool
mask_has_loop(struct lp_exec_mask *mask)
{
   int i;
   for (i = mask->function_stack_size - 1; i >= 0; --i) {
      const struct function_ctx *ctx = &mask->function_stack[i];
      if (ctx->loop_stack_size > 0)
         return true;
   }
   return false;
}

/*
 * Returns true if we're inside a switch statement.
 * It's global, meaning that it returns true even if there's
 * no switch in the current function, but we were inside
 * a switch inside another function, from which this one was called.
 */
static inline bool
mask_has_switch(struct lp_exec_mask *mask)
{
   int i;
   for (i = mask->function_stack_size - 1; i >= 0; --i) {
      const struct function_ctx *ctx = &mask->function_stack[i];
      if (ctx->switch_stack_size > 0)
         return true;
   }
   return false;
}

/*
 * Returns true if we're inside a conditional.
 * It's global, meaning that it returns true even if there's
 * no conditional in the current function, but we were inside
 * a conditional inside another function, from which this one was called.
 */
static inline bool
mask_has_cond(struct lp_exec_mask *mask)
{
   int i;
   for (i = mask->function_stack_size - 1; i >= 0; --i) {
      const struct function_ctx *ctx = &mask->function_stack[i];
      if (ctx->cond_stack_size > 0)
         return true;
   }
   return false;
}

void lp_exec_mask_update(struct lp_exec_mask *mask)
{
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   bool has_loop_mask = mask_has_loop(mask);
   bool has_cond_mask = mask_has_cond(mask);
   bool has_switch_mask = mask_has_switch(mask);
   bool has_ret_mask = mask->function_stack_size > 1 ||
         mask->ret_in_main;

   if (has_loop_mask) {
      /*for loops we need to update the entire mask at runtime */
      LLVMValueRef tmp;
      assert(mask->break_mask);
      tmp = LLVMBuildAnd(builder,
                         mask->cont_mask,
                         mask->break_mask,
                         "maskcb");
      mask->exec_mask = LLVMBuildAnd(builder,
                                     mask->cond_mask,
                                     tmp,
                                     "maskfull");
   } else
      mask->exec_mask = mask->cond_mask;

   if (has_switch_mask) {
      mask->exec_mask = LLVMBuildAnd(builder,
                                     mask->exec_mask,
                                     mask->switch_mask,
                                     "switchmask");
   }

   if (has_ret_mask) {
      mask->exec_mask = LLVMBuildAnd(builder,
                                     mask->exec_mask,
                                     mask->ret_mask,
                                     "callmask");
   }

   mask->has_mask = (has_cond_mask ||
                     has_loop_mask ||
                     has_switch_mask ||
                     has_ret_mask);
}

/*
 * Initialize a function context at the specified index.
 */
void
lp_exec_mask_function_init(struct lp_exec_mask *mask, int function_idx)
{
   LLVMTypeRef int_type = LLVMInt32TypeInContext(mask->bld->gallivm->context);
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   struct function_ctx *ctx =  &mask->function_stack[function_idx];

   ctx->cond_stack_size = 0;
   ctx->loop_stack_size = 0;
   ctx->bgnloop_stack_size = 0;
   ctx->switch_stack_size = 0;

   if (function_idx == 0) {
      ctx->ret_mask = mask->ret_mask;
   }

   ctx->loop_limiter = lp_build_alloca(mask->bld->gallivm,
                                       int_type, "looplimiter");
   LLVMBuildStore(
      builder,
      LLVMConstInt(int_type, LP_MAX_TGSI_LOOP_ITERATIONS, false),
      ctx->loop_limiter);
}

void lp_exec_mask_init(struct lp_exec_mask *mask, struct lp_build_context *bld)
{
   mask->bld = bld;
   mask->has_mask = false;
   mask->ret_in_main = false;
   /* For the main function */
   mask->function_stack_size = 1;

   mask->int_vec_type = lp_build_int_vec_type(bld->gallivm, mask->bld->type);
   mask->exec_mask = mask->ret_mask = mask->break_mask = mask->cont_mask =
         mask->cond_mask = mask->switch_mask =
         LLVMConstAllOnes(mask->int_vec_type);

   mask->function_stack = CALLOC(LP_MAX_NUM_FUNCS,
                                 sizeof(mask->function_stack[0]));
   lp_exec_mask_function_init(mask, 0);
}

void
lp_exec_mask_fini(struct lp_exec_mask *mask)
{
   FREE(mask->function_stack);
}

/* stores val into an address pointed to by dst_ptr.
 * mask->exec_mask is used to figure out which bits of val
 * should be stored into the address
 * (0 means don't store this bit, 1 means do store).
 */
void lp_exec_mask_store(struct lp_exec_mask *mask,
                        struct lp_build_context *bld_store,
                        LLVMValueRef val,
                        LLVMValueRef dst_ptr)
{
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   LLVMValueRef exec_mask = mask->has_mask ? mask->exec_mask : NULL;

   assert(lp_check_value(bld_store->type, val));
   assert(LLVMGetTypeKind(LLVMTypeOf(dst_ptr)) == LLVMPointerTypeKind);
   assert(LLVM_VERSION_MAJOR >= 15
          || (LLVMGetElementType(LLVMTypeOf(dst_ptr)) == LLVMTypeOf(val)
              || LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(dst_ptr))) == LLVMArrayTypeKind));

   if (exec_mask) {
      LLVMValueRef res, dst;

      dst = LLVMBuildLoad2(builder, LLVMTypeOf(val), dst_ptr, "");
      if (bld_store->type.width < 32)
         exec_mask = LLVMBuildTrunc(builder, exec_mask, bld_store->vec_type, "");
      res = lp_build_select(bld_store, exec_mask, val, dst);
      LLVMBuildStore(builder, res, dst_ptr);
   } else
      LLVMBuildStore(builder, val, dst_ptr);
}

void lp_exec_bgnloop_post_phi(struct lp_exec_mask *mask)
{
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   struct function_ctx *ctx = func_ctx(mask);

   if (ctx->loop_stack_size != ctx->bgnloop_stack_size) {
      mask->break_mask = LLVMBuildLoad2(builder, mask->int_vec_type, ctx->break_var, "");
      lp_exec_mask_update(mask);
      ctx->bgnloop_stack_size = ctx->loop_stack_size;
   }
}

void lp_exec_bgnloop(struct lp_exec_mask *mask, bool load)
{
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   struct function_ctx *ctx = func_ctx(mask);

   if (ctx->loop_stack_size >= LP_MAX_TGSI_NESTING) {
      ++ctx->loop_stack_size;
      return;
   }

   ctx->break_type_stack[ctx->loop_stack_size + ctx->switch_stack_size] =
      ctx->break_type;
   ctx->break_type = LP_EXEC_MASK_BREAK_TYPE_LOOP;

   ctx->loop_stack[ctx->loop_stack_size].loop_block = ctx->loop_block;
   ctx->loop_stack[ctx->loop_stack_size].cont_mask = mask->cont_mask;
   ctx->loop_stack[ctx->loop_stack_size].break_mask = mask->break_mask;
   ctx->loop_stack[ctx->loop_stack_size].break_var = ctx->break_var;
   ++ctx->loop_stack_size;

   ctx->break_var = lp_build_alloca(mask->bld->gallivm, mask->int_vec_type, "");
   LLVMBuildStore(builder, mask->break_mask, ctx->break_var);

   ctx->loop_block = lp_build_insert_new_block(mask->bld->gallivm, "bgnloop");

   LLVMBuildBr(builder, ctx->loop_block);
   LLVMPositionBuilderAtEnd(builder, ctx->loop_block);

   if (load) {
      lp_exec_bgnloop_post_phi(mask);
   }
}

void lp_exec_endloop(struct gallivm_state *gallivm,
                     struct lp_exec_mask *exec_mask,
                     struct lp_build_mask_context *mask)
{
   LLVMBuilderRef builder = exec_mask->bld->gallivm->builder;
   struct function_ctx *ctx = func_ctx(exec_mask);
   LLVMBasicBlockRef endloop;
   LLVMTypeRef int_type = LLVMInt32TypeInContext(exec_mask->bld->gallivm->context);
   LLVMTypeRef mask_type = LLVMIntTypeInContext(exec_mask->bld->gallivm->context, exec_mask->bld->type.length);
   LLVMValueRef i1cond, i2cond, icond, limiter;

   assert(exec_mask->break_mask);

   assert(ctx->loop_stack_size);
   if (ctx->loop_stack_size > LP_MAX_TGSI_NESTING) {
      --ctx->loop_stack_size;
      --ctx->bgnloop_stack_size;
      return;
   }

   /*
    * Restore the cont_mask, but don't pop
    */
   exec_mask->cont_mask = ctx->loop_stack[ctx->loop_stack_size - 1].cont_mask;
   lp_exec_mask_update(exec_mask);

   /*
    * Unlike the continue mask, the break_mask must be preserved across loop
    * iterations
    */
   LLVMBuildStore(builder, exec_mask->break_mask, ctx->break_var);

   /* Decrement the loop limiter */
   limiter = LLVMBuildLoad2(builder, int_type, ctx->loop_limiter, "");

   limiter = LLVMBuildSub(
      builder,
      limiter,
      LLVMConstInt(int_type, 1, false),
      "");

   LLVMBuildStore(builder, limiter, ctx->loop_limiter);

   LLVMValueRef end_mask = exec_mask->exec_mask;
   if (mask)
      end_mask = LLVMBuildAnd(builder, exec_mask->exec_mask, lp_build_mask_value(mask), "");
   end_mask = LLVMBuildICmp(builder, LLVMIntNE, end_mask, lp_build_zero(gallivm, exec_mask->bld->type), "");
   end_mask = LLVMBuildBitCast(builder, end_mask, mask_type, "");

   /* i1cond = (end_mask != 0) */
   i1cond = LLVMBuildICmp(
      builder,
      LLVMIntNE,
      end_mask,
      LLVMConstNull(mask_type), "i1cond");

   /* i2cond = (looplimiter > 0) */
   i2cond = LLVMBuildICmp(
      builder,
      LLVMIntSGT,
      limiter,
      LLVMConstNull(int_type), "i2cond");

   /* if( i1cond && i2cond ) */
   icond = LLVMBuildAnd(builder, i1cond, i2cond, "");

   endloop = lp_build_insert_new_block(exec_mask->bld->gallivm, "endloop");

   LLVMBuildCondBr(builder,
                   icond, ctx->loop_block, endloop);

   LLVMPositionBuilderAtEnd(builder, endloop);

   assert(ctx->loop_stack_size);
   --ctx->loop_stack_size;
   --ctx->bgnloop_stack_size;
   exec_mask->cont_mask = ctx->loop_stack[ctx->loop_stack_size].cont_mask;
   exec_mask->break_mask = ctx->loop_stack[ctx->loop_stack_size].break_mask;
   ctx->loop_block = ctx->loop_stack[ctx->loop_stack_size].loop_block;
   ctx->break_var = ctx->loop_stack[ctx->loop_stack_size].break_var;
   ctx->break_type = ctx->break_type_stack[ctx->loop_stack_size +
         ctx->switch_stack_size];

   lp_exec_mask_update(exec_mask);
}

void lp_exec_mask_cond_push(struct lp_exec_mask *mask,
                            LLVMValueRef val)
{
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   struct function_ctx *ctx = func_ctx(mask);

   if (ctx->cond_stack_size >= LP_MAX_TGSI_NESTING) {
      ctx->cond_stack_size++;
      return;
   }
   if (ctx->cond_stack_size == 0 && mask->function_stack_size == 1) {
      assert(mask->cond_mask == LLVMConstAllOnes(mask->int_vec_type));
   }
   ctx->cond_stack[ctx->cond_stack_size++] = mask->cond_mask;
   assert(LLVMTypeOf(val) == mask->int_vec_type);
   mask->cond_mask = LLVMBuildAnd(builder,
                                  mask->cond_mask,
                                  val,
                                  "");
   lp_exec_mask_update(mask);
}

void lp_exec_mask_cond_invert(struct lp_exec_mask *mask)
{
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   struct function_ctx *ctx = func_ctx(mask);
   LLVMValueRef prev_mask;
   LLVMValueRef inv_mask;

   assert(ctx->cond_stack_size);
   if (ctx->cond_stack_size >= LP_MAX_TGSI_NESTING)
      return;
   prev_mask = ctx->cond_stack[ctx->cond_stack_size - 1];
   if (ctx->cond_stack_size == 1 && mask->function_stack_size == 1) {
      assert(prev_mask == LLVMConstAllOnes(mask->int_vec_type));
   }

   inv_mask = LLVMBuildNot(builder, mask->cond_mask, "");

   mask->cond_mask = LLVMBuildAnd(builder,
                                  inv_mask,
                                  prev_mask, "");
   lp_exec_mask_update(mask);
}

void lp_exec_mask_cond_pop(struct lp_exec_mask *mask)
{
   struct function_ctx *ctx = func_ctx(mask);
   assert(ctx->cond_stack_size);
   --ctx->cond_stack_size;
   if (ctx->cond_stack_size >= LP_MAX_TGSI_NESTING)
      return;
   mask->cond_mask = ctx->cond_stack[ctx->cond_stack_size];
   lp_exec_mask_update(mask);
}


void lp_exec_continue(struct lp_exec_mask *mask)
{
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   LLVMValueRef exec_mask = LLVMBuildNot(builder,
                                         mask->exec_mask,
                                         "");

   mask->cont_mask = LLVMBuildAnd(builder,
                                  mask->cont_mask,
                                  exec_mask, "");

   lp_exec_mask_update(mask);
}

void lp_exec_break(struct lp_exec_mask *mask, int *pc,
                   bool break_always)
{
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   struct function_ctx *ctx = func_ctx(mask);

   if (ctx->break_type == LP_EXEC_MASK_BREAK_TYPE_LOOP) {
      LLVMValueRef exec_mask = LLVMBuildNot(builder,
                                            mask->exec_mask,
                                            "break");

      mask->break_mask = LLVMBuildAnd(builder,
                                      mask->break_mask,
                                      exec_mask, "break_full");
   }
   else {
      if (ctx->switch_in_default) {
         /*
          * stop default execution but only if this is an unconditional switch.
          * (The condition here is not perfect since dead code after break is
          * allowed but should be sufficient since false negatives are just
          * unoptimized - so we don't have to pre-evaluate that).
          */
         if(break_always && ctx->switch_pc) {
            if (pc)
               *pc = ctx->switch_pc;
            return;
         }
      }

      if (break_always) {
         mask->switch_mask = LLVMConstNull(mask->bld->int_vec_type);
      }
      else {
         LLVMValueRef exec_mask = LLVMBuildNot(builder,
                                               mask->exec_mask,
                                               "break");
         mask->switch_mask = LLVMBuildAnd(builder,
                                          mask->switch_mask,
                                          exec_mask, "break_switch");
      }
   }

   lp_exec_mask_update(mask);
}
