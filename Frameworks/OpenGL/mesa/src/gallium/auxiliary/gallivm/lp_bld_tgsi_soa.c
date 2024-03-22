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
 * TGSI to LLVM IR translation -- SoA.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 *
 * Based on tgsi_sse2.c code written by Michal Krol, Keith Whitwell,
 * Brian Paul, and others.
 */

#include "util/detect.h"
#include "pipe/p_shader_tokens.h"
#include "util/u_debug.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_exec.h"
#include "tgsi/tgsi_info.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_util.h"
#include "tgsi/tgsi_scan.h"
#include "tgsi/tgsi_strings.h"
#include "lp_bld_tgsi_action.h"
#include "lp_bld_type.h"
#include "lp_bld_const.h"
#include "lp_bld_arit.h"
#include "lp_bld_bitarit.h"
#include "lp_bld_gather.h"
#include "lp_bld_init.h"
#include "lp_bld_logic.h"
#include "lp_bld_misc.h"
#include "lp_bld_swizzle.h"
#include "lp_bld_flow.h"
#include "lp_bld_coro.h"
#include "lp_bld_quad.h"
#include "lp_bld_tgsi.h"
#include "lp_bld_limits.h"
#include "lp_bld_debug.h"
#include "lp_bld_printf.h"
#include "lp_bld_sample.h"
#include "lp_bld_struct.h"
#include "lp_bld_jit_types.h"

#define DUMP_GS_EMITS 0

/*
 * If non-zero, the generated LLVM IR will print intermediate results on every TGSI
 * instruction.
 *
 * TODO:
 * - take execution masks in consideration
 * - debug control-flow instructions
 */
#define DEBUG_EXECUTION 0


/*
 * Emit code to print a register value.
 */
static void
emit_dump_reg(struct gallivm_state *gallivm,
              unsigned file,
              unsigned index,
              unsigned chan,
              LLVMValueRef value)
{
   char buf[32];

   snprintf(buf, sizeof buf, "    %s[%u].%c = ",
            tgsi_file_name(file),
            index, "xyzw"[chan]);

   lp_build_print_value(gallivm, buf, value);
}

static inline struct function_ctx *
func_ctx(struct lp_exec_mask *mask)
{
   assert(mask->function_stack_size > 0);
   assert(mask->function_stack_size <= LP_MAX_NUM_FUNCS);
   return &mask->function_stack[mask->function_stack_size - 1];
}

/*
 * combine the execution mask if there is one with the current mask.
 */
static LLVMValueRef
mask_vec(struct lp_build_tgsi_context *bld_base)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   LLVMBuilderRef builder = bld->bld_base.base.gallivm->builder;
   struct lp_exec_mask *exec_mask = &bld->exec_mask;
   LLVMValueRef bld_mask = bld->mask ? lp_build_mask_value(bld->mask) : NULL;
   if (!exec_mask->has_mask) {
      return bld_mask;
   }
   if (!bld_mask)
      return exec_mask->exec_mask;
   return LLVMBuildAnd(builder, lp_build_mask_value(bld->mask),
                       exec_mask->exec_mask, "");
}

static void lp_exec_tgsi_break(struct lp_exec_mask *mask,
                          struct lp_build_tgsi_context * bld_base)
{
   enum tgsi_opcode opcode =
      bld_base->instructions[bld_base->pc + 1].Instruction.Opcode;
   bool break_always = (opcode == TGSI_OPCODE_ENDSWITCH ||
                        opcode == TGSI_OPCODE_CASE);
   lp_exec_break(mask, &bld_base->pc, break_always);
}

static void lp_exec_switch(struct lp_exec_mask *mask,
                           LLVMValueRef switchval)
{
   struct function_ctx *ctx = func_ctx(mask);

   if (ctx->switch_stack_size >= LP_MAX_TGSI_NESTING ||
       ctx->loop_stack_size > LP_MAX_TGSI_NESTING) {
      ctx->switch_stack_size++;
      return;
   }

   ctx->break_type_stack[ctx->loop_stack_size + ctx->switch_stack_size] =
      ctx->break_type;
   ctx->break_type = LP_EXEC_MASK_BREAK_TYPE_SWITCH;

   ctx->switch_stack[ctx->switch_stack_size].switch_mask = mask->switch_mask;
   ctx->switch_stack[ctx->switch_stack_size].switch_val = ctx->switch_val;
   ctx->switch_stack[ctx->switch_stack_size].switch_mask_default = ctx->switch_mask_default;
   ctx->switch_stack[ctx->switch_stack_size].switch_in_default = ctx->switch_in_default;
   ctx->switch_stack[ctx->switch_stack_size].switch_pc = ctx->switch_pc;
   ctx->switch_stack_size++;

   mask->switch_mask = LLVMConstNull(mask->int_vec_type);
   ctx->switch_val = switchval;
   ctx->switch_mask_default = LLVMConstNull(mask->int_vec_type);
   ctx->switch_in_default = false;
   ctx->switch_pc = 0;

   lp_exec_mask_update(mask);
}

static void lp_exec_endswitch(struct lp_exec_mask *mask,
                              struct lp_build_tgsi_context * bld_base)
{
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   struct function_ctx *ctx = func_ctx(mask);

   if (ctx->switch_stack_size > LP_MAX_TGSI_NESTING) {
      ctx->switch_stack_size--;
      return;
   }

   /* check if there's deferred default if so do it now */
   if (ctx->switch_pc && !ctx->switch_in_default) {
      LLVMValueRef prevmask, defaultmask;
      unsigned tmp_pc;
      prevmask = ctx->switch_stack[ctx->switch_stack_size - 1].switch_mask;
      defaultmask = LLVMBuildNot(builder, ctx->switch_mask_default, "sw_default_mask");
      mask->switch_mask = LLVMBuildAnd(builder, prevmask, defaultmask, "sw_mask");
      ctx->switch_in_default = true;

      lp_exec_mask_update(mask);

      assert(bld_base->instructions[ctx->switch_pc - 1].Instruction.Opcode ==
             TGSI_OPCODE_DEFAULT);

      tmp_pc = bld_base->pc;
      bld_base->pc = ctx->switch_pc;
      /*
       * re-purpose switch_pc to point to here again, since we stop execution of
       * the deferred default after next break.
       */
      ctx->switch_pc = tmp_pc - 1;

      return;
   }

   else if (ctx->switch_pc && ctx->switch_in_default) {
      assert(bld_base->pc == ctx->switch_pc + 1);
   }

   ctx->switch_stack_size--;
   mask->switch_mask = ctx->switch_stack[ctx->switch_stack_size].switch_mask;
   ctx->switch_val = ctx->switch_stack[ctx->switch_stack_size].switch_val;
   ctx->switch_mask_default = ctx->switch_stack[ctx->switch_stack_size].switch_mask_default;
   ctx->switch_in_default = ctx->switch_stack[ctx->switch_stack_size].switch_in_default;
   ctx->switch_pc = ctx->switch_stack[ctx->switch_stack_size].switch_pc;

   ctx->break_type = ctx->break_type_stack[ctx->loop_stack_size + ctx->switch_stack_size];

   lp_exec_mask_update(mask);
}

static void lp_exec_case(struct lp_exec_mask *mask,
                         LLVMValueRef caseval)
{
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   struct function_ctx *ctx = func_ctx(mask);

   LLVMValueRef casemask, prevmask;

   if (ctx->switch_stack_size > LP_MAX_TGSI_NESTING) {
      return;
   }

   /* skipping case mask evaluation here is NOT optional (not in all cases anyway). */
   if (!ctx->switch_in_default) {
      prevmask = ctx->switch_stack[ctx->switch_stack_size - 1].switch_mask;
      casemask = lp_build_cmp(mask->bld, PIPE_FUNC_EQUAL, caseval, ctx->switch_val);
      ctx->switch_mask_default = LLVMBuildOr(builder, casemask,
                                             ctx->switch_mask_default, "sw_default_mask");
      casemask = LLVMBuildOr(builder, casemask, mask->switch_mask, "");
      mask->switch_mask = LLVMBuildAnd(builder, casemask, prevmask, "sw_mask");

      lp_exec_mask_update(mask);
   }
}

/*
 * Analyse default statement in a switch.
 * \return true if default is last statement, false otherwise
 * \param default_pc_start contains pc of instruction to jump to
 *                         if default wasn't last but there's no
 *                         fallthrough into default.
 */
static bool default_analyse_is_last(struct lp_exec_mask *mask,
                                       struct lp_build_tgsi_context * bld_base,
                                       int *default_pc_start)
{
   unsigned pc = bld_base->pc;
   struct function_ctx *ctx = func_ctx(mask);
   int curr_switch_stack = ctx->switch_stack_size;

   if (ctx->switch_stack_size > LP_MAX_TGSI_NESTING) {
      return false;
   }

   /* skip over case statements which are together with default */
   while (bld_base->instructions[pc].Instruction.Opcode == TGSI_OPCODE_CASE) {
      pc++;
   }

   while (pc != ~0u && pc < bld_base->num_instructions) {
      enum tgsi_opcode opcode = bld_base->instructions[pc].Instruction.Opcode;
      switch (opcode) {
      case TGSI_OPCODE_CASE:
         if (curr_switch_stack == ctx->switch_stack_size) {
            *default_pc_start = pc - 1;
            return false;
         }
         break;
      case TGSI_OPCODE_SWITCH:
         curr_switch_stack++;
         break;
      case TGSI_OPCODE_ENDSWITCH:
         if (curr_switch_stack == ctx->switch_stack_size) {
            *default_pc_start = pc - 1;
            return true;
         }
         curr_switch_stack--;
         break;
      default:
         ; /* nothing */
      }
      pc++;
   }
   /* should never arrive here */
   assert(0);
   return true;
}

static void lp_exec_default(struct lp_exec_mask *mask,
                            struct lp_build_tgsi_context * bld_base)
{
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   struct function_ctx *ctx = func_ctx(mask);

   int default_exec_pc = 0;
   bool default_is_last;

   if (ctx->switch_stack_size > LP_MAX_TGSI_NESTING) {
      return;
   }

   /*
    * This is a messy opcode, because it may not be always at the end and
    * there can be fallthrough in and out of it.
    */

   default_is_last = default_analyse_is_last(mask, bld_base, &default_exec_pc);
   /*
    * If it is last statement in switch (note that case statements appearing
    * "at the same time" as default don't change that) everything is just fine,
    * update switch mask and go on. This means we can handle default with
    * fallthrough INTO it without overhead, if it is last.
    */
   if (default_is_last) {
      LLVMValueRef prevmask, defaultmask;
      prevmask = ctx->switch_stack[ctx->switch_stack_size - 1].switch_mask;
      defaultmask = LLVMBuildNot(builder, ctx->switch_mask_default, "sw_default_mask");
      defaultmask = LLVMBuildOr(builder, defaultmask, mask->switch_mask, "");
      mask->switch_mask = LLVMBuildAnd(builder, prevmask, defaultmask, "sw_mask");
      ctx->switch_in_default = true;

      lp_exec_mask_update(mask);
   }
   else {
      /*
       * Technically, "case" immediately before default isn't really a
       * fallthrough, however we still have to count them as such as we
       * already have updated the masks.
       * If that happens in practice could add a switch optimizer pass
       * which just gets rid of all case statements appearing together with
       * default (or could do switch analysis at switch start time instead).
       */
      enum tgsi_opcode opcode =
         bld_base->instructions[bld_base->pc - 1].Instruction.Opcode;
      bool ft_into = (opcode != TGSI_OPCODE_BRK &&
                      opcode != TGSI_OPCODE_SWITCH);
      /*
       * If it is not last statement and there was no fallthrough into it,
       * we record the PC and continue execution at next case (again, those
       * case encountered at the same time don't count). At endswitch
       * time, we update switchmask, and go back executing the code we skipped
       * until the next break (possibly re-executing some code with changed mask
       * if there was a fallthrough out of default).
       * Finally, if it is not last statement and there was a fallthrough into it,
       * do the same as with the former case, except instead of skipping the code
       * just execute it without updating the mask, then go back and re-execute.
       */
      ctx->switch_pc = bld_base->pc;
      if (!ft_into) {
         bld_base->pc = default_exec_pc;
      }
   }
}


static void lp_exec_mask_call(struct lp_exec_mask *mask,
                              int func,
                              int *pc)
{
   if (mask->function_stack_size >= LP_MAX_NUM_FUNCS) {
      return;
   }

   lp_exec_mask_function_init(mask, mask->function_stack_size);
   mask->function_stack[mask->function_stack_size].pc = *pc;
   mask->function_stack[mask->function_stack_size].ret_mask = mask->ret_mask;
   mask->function_stack_size++;
   *pc = func;
}

static void lp_exec_mask_ret(struct lp_exec_mask *mask, int *pc)
{
   LLVMBuilderRef builder = mask->bld->gallivm->builder;
   struct function_ctx *ctx = func_ctx(mask);
   LLVMValueRef exec_mask;

   if (ctx->cond_stack_size == 0 &&
       ctx->loop_stack_size == 0 &&
       ctx->switch_stack_size == 0 &&
       mask->function_stack_size == 1) {
      /* returning from main() */
      *pc = -1;
      return;
   }

   if (mask->function_stack_size == 1) {
      /*
       * This requires special handling since we need to ensure
       * we don't drop the mask even if we have no call stack
       * (e.g. after a ret in a if clause after the endif)
       */
      mask->ret_in_main = true;
   }

   exec_mask = LLVMBuildNot(builder,
                            mask->exec_mask,
                            "ret");

   mask->ret_mask = LLVMBuildAnd(builder,
                                 mask->ret_mask,
                                 exec_mask, "ret_full");

   lp_exec_mask_update(mask);
}

static void lp_exec_mask_bgnsub(struct lp_exec_mask *mask)
{
}

static void lp_exec_mask_endsub(struct lp_exec_mask *mask, int *pc)
{
   struct function_ctx *ctx;

   assert(mask->function_stack_size > 1);
   assert(mask->function_stack_size <= LP_MAX_NUM_FUNCS);

   ctx = func_ctx(mask);
   mask->function_stack_size--;

   *pc = ctx->pc;
   mask->ret_mask = ctx->ret_mask;

   lp_exec_mask_update(mask);
}


static LLVMValueRef
get_file_ptr(struct lp_build_tgsi_soa_context *bld,
             unsigned file,
             int index,
             unsigned chan)
{
   LLVMBuilderRef builder = bld->bld_base.base.gallivm->builder;
   LLVMValueRef (*array_of_vars)[TGSI_NUM_CHANNELS];
   LLVMValueRef var_of_array;
   LLVMTypeRef type_of_array;

   switch (file) {
   case TGSI_FILE_TEMPORARY:
      array_of_vars = bld->temps;
      var_of_array = bld->temps_array;
      type_of_array = bld->temps_array_type;
      break;
   case TGSI_FILE_OUTPUT:
      array_of_vars = bld->outputs;
      var_of_array = bld->outputs_array;
      type_of_array = bld->outputs_array_type;
      break;
   default:
      assert(0);
      return NULL;
   }

   assert(chan < 4);

   if (bld->indirect_files & (1 << file)) {
      LLVMValueRef lindex = lp_build_const_int32(bld->bld_base.base.gallivm, index * 4 + chan);
      /* I'm not sure the other path ever gets hit, but leave until someone figures it out,
         this check doesn't work with opaque pointers. */
      if (1) {//LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(var_of_array))) == LLVMArrayTypeKind) {
         LLVMValueRef gep[2];
         gep[0] = lp_build_const_int32(bld->bld_base.base.gallivm, 0);
         gep[1] = lindex;
         return LLVMBuildGEP2(builder, type_of_array, var_of_array, gep, 2, "");
      } else {
         return LLVMBuildGEP2(builder, type_of_array, var_of_array, &lindex, 1, "");
      }
   }
   else {
      assert(index <= bld->bld_base.info->file_max[file]);
      return array_of_vars[index][chan];
   }
}


/**
 * Return pointer to a temporary register channel (src or dest).
 * Note that indirect addressing cannot be handled here.
 * \param index  which temporary register
 * \param chan  which channel of the temp register.
 */
LLVMValueRef
lp_get_temp_ptr_soa(struct lp_build_tgsi_soa_context *bld,
             unsigned index,
             unsigned chan)
{
   return get_file_ptr(bld, TGSI_FILE_TEMPORARY, index, chan);
}

/**
 * Return pointer to a output register channel (src or dest).
 * Note that indirect addressing cannot be handled here.
 * \param index  which output register
 * \param chan  which channel of the output register.
 */
LLVMValueRef
lp_get_output_ptr(struct lp_build_tgsi_soa_context *bld,
               unsigned index,
               unsigned chan)
{
   return get_file_ptr(bld, TGSI_FILE_OUTPUT, index, chan);
}

/*
 * If we have indirect addressing in outputs copy our alloca array
 * to the outputs slots specified by the caller to make sure
 * our outputs are delivered consistently via the same interface.
 */
static void
gather_outputs(struct lp_build_tgsi_soa_context * bld)
{
   if ((bld->indirect_files & (1 << TGSI_FILE_OUTPUT))) {
      unsigned index, chan;
      assert(bld->bld_base.info->num_outputs <=
             bld->bld_base.info->file_max[TGSI_FILE_OUTPUT] + 1);
      for (index = 0; index < bld->bld_base.info->num_outputs; ++index) {
         for (chan = 0; chan < TGSI_NUM_CHANNELS; ++chan) {
            bld->outputs[index][chan] = lp_get_output_ptr(bld, index, chan);
         }
      }
   }
}

/**
 * Gather vector.
 * XXX the lp_build_gather() function should be capable of doing this
 * with a little work.
 */
static LLVMValueRef
build_gather(struct lp_build_tgsi_context *bld_base,
             LLVMValueRef base_ptr,
             LLVMValueRef indexes,
             LLVMValueRef overflow_mask,
             LLVMValueRef indexes2)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context *uint_bld = &bld_base->uint_bld;
   struct lp_build_context *bld = &bld_base->base;
   LLVMValueRef res;
   unsigned i;

   if (indexes2)
      res = LLVMGetUndef(LLVMVectorType(LLVMFloatTypeInContext(gallivm->context), bld_base->base.type.length * 2));
   else
      res = bld->undef;
   /*
    * overflow_mask is a vector telling us which channels
    * in the vector overflowed. We use the overflow behavior for
    * constant buffers which is defined as:
    * Out of bounds access to constant buffer returns 0 in all
    * components. Out of bounds behavior is always with respect
    * to the size of the buffer bound at that slot.
    */

   if (overflow_mask) {
      /*
       * We avoid per-element control flow here (also due to llvm going crazy,
       * though I suspect it's better anyway since overflow is likely rare).
       * Note that since we still fetch from buffers even if num_elements was
       * zero (in this case we'll fetch from index zero) the jit func callers
       * MUST provide valid fake constant buffers of size 4x32 (the values do
       * not matter), otherwise we'd still need (not per element though)
       * control flow.
       */
      indexes = lp_build_select(uint_bld, overflow_mask, uint_bld->zero, indexes);
      if (indexes2)
         indexes2 = lp_build_select(uint_bld, overflow_mask, uint_bld->zero, indexes2);
   }

   /*
    * Loop over elements of index_vec, load scalar value, insert it into 'res'.
    */
   for (i = 0; i < bld->type.length * (indexes2 ? 2 : 1); i++) {
      LLVMValueRef si, di;
      LLVMValueRef index;
      LLVMValueRef scalar_ptr, scalar;

      di = lp_build_const_int32(bld->gallivm, i);
      if (indexes2)
         si = lp_build_const_int32(bld->gallivm, i >> 1);
      else
         si = di;

      if (indexes2 && (i & 1)) {
         index = LLVMBuildExtractElement(builder,
                                         indexes2, si, "");
      } else {
         index = LLVMBuildExtractElement(builder,
                                         indexes, si, "");
      }
      scalar_ptr = LLVMBuildGEP2(builder, bld->elem_type, base_ptr,
                                 &index, 1, "gather_ptr");
      scalar = LLVMBuildLoad2(builder, bld->elem_type, scalar_ptr, "");

      res = LLVMBuildInsertElement(builder, res, scalar, di, "");
   }

   if (overflow_mask) {
      if (indexes2) {
         res = LLVMBuildBitCast(builder, res, bld_base->dbl_bld.vec_type, "");
         overflow_mask = LLVMBuildSExt(builder, overflow_mask,
                                       bld_base->dbl_bld.int_vec_type, "");
         res = lp_build_select(&bld_base->dbl_bld, overflow_mask,
                               bld_base->dbl_bld.zero, res);
      } else
         res = lp_build_select(bld, overflow_mask, bld->zero, res);
   }

   return res;
}


/**
 * Scatter/store vector.
 */
static void
emit_mask_scatter(struct lp_build_tgsi_soa_context *bld,
                  LLVMValueRef base_ptr,
                  LLVMValueRef indexes,
                  LLVMValueRef values,
                  struct lp_exec_mask *mask)
{
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   unsigned i;
   LLVMValueRef pred = mask->has_mask ? mask->exec_mask : NULL;

   /*
    * Loop over elements of index_vec, store scalar value.
    */
   for (i = 0; i < bld->bld_base.base.type.length; i++) {
      LLVMValueRef ii = lp_build_const_int32(gallivm, i);
      LLVMValueRef index = LLVMBuildExtractElement(builder, indexes, ii, "");
      LLVMValueRef scalar_ptr = LLVMBuildGEP2(builder, bld->bld_base.base.elem_type, base_ptr, &index, 1, "scatter_ptr");
      LLVMValueRef val = LLVMBuildExtractElement(builder, values, ii, "scatter_val");
      LLVMValueRef scalar_pred = pred ?
         LLVMBuildExtractElement(builder, pred, ii, "scatter_pred") : NULL;

      if (0)
         lp_build_printf(gallivm, "scatter %d: val %f at %d %p\n",
                         ii, val, index, scalar_ptr);

      if (scalar_pred) {
         LLVMValueRef real_val, dst_val;
         dst_val = LLVMBuildLoad2(builder, bld->bld_base.base.elem_type, scalar_ptr, "");
         real_val = lp_build_select(&bld->elem_bld, scalar_pred, val, dst_val);
         LLVMBuildStore(builder, real_val, scalar_ptr);
      }
      else {
         LLVMBuildStore(builder, val, scalar_ptr);
      }
   }
}


/**
 * Read the current value of the ADDR register, convert the floats to
 * ints, add the base index and return the vector of offsets.
 * The offsets will be used to index into the constant buffer or
 * temporary register file.
 */
static LLVMValueRef
get_indirect_index(struct lp_build_tgsi_soa_context *bld,
                   unsigned reg_file, unsigned reg_index,
                   const struct tgsi_ind_register *indirect_reg,
                   int index_limit)
{
   LLVMBuilderRef builder = bld->bld_base.base.gallivm->builder;
   struct lp_build_context *uint_bld = &bld->bld_base.uint_bld;
   /* always use X component of address register */
   unsigned swizzle = indirect_reg->Swizzle;
   LLVMValueRef base;
   LLVMValueRef rel;
   LLVMValueRef max_index;
   LLVMValueRef index;

   assert(bld->indirect_files & (1 << reg_file));

   base = lp_build_const_int_vec(bld->bld_base.base.gallivm, uint_bld->type, reg_index);

   assert(swizzle < 4);
   switch (indirect_reg->File) {
   case TGSI_FILE_ADDRESS:
      rel = LLVMBuildLoad2(builder,
                           bld->bld_base.base.int_vec_type,
                           bld->addr[indirect_reg->Index][swizzle],
                           "load addr reg");
      /* ADDR LLVM values already have LLVM integer type. */
      break;
   case TGSI_FILE_TEMPORARY:
      rel = lp_get_temp_ptr_soa(bld, indirect_reg->Index, swizzle);
      rel = LLVMBuildLoad2(builder, bld->bld_base.base.vec_type, rel, "load temp reg");
      /* TEMP LLVM values always have LLVM float type, but for indirection, the
       * value actually stored is expected to be an integer */
      rel = LLVMBuildBitCast(builder, rel, uint_bld->vec_type, "");
      break;
   default:
      assert(0);
      rel = uint_bld->zero;
   }

   index = lp_build_add(uint_bld, base, rel);

   /*
    * emit_fetch_constant handles constant buffer overflow so this code
    * is pointless for them.
    * Furthermore the D3D10 spec in section 6.5 says:
    * If the constant buffer bound to a slot is larger than the size
    * declared in the shader for that slot, implementations are allowed
    * to return incorrect data (not necessarily 0) for indices that are
    * larger than the declared size but smaller than the buffer size.
    */
   if (reg_file != TGSI_FILE_CONSTANT) {
      assert(index_limit >= 0);
      max_index = lp_build_const_int_vec(bld->bld_base.base.gallivm,
                                         uint_bld->type, index_limit);

      assert(!uint_bld->type.sign);
      index = lp_build_min(uint_bld, index, max_index);
   }

   return index;
}

static struct lp_build_context *
stype_to_fetch(struct lp_build_tgsi_context * bld_base,
	       enum tgsi_opcode_type stype)
{
   struct lp_build_context *bld_fetch;

   switch (stype) {
   case TGSI_TYPE_FLOAT:
   case TGSI_TYPE_UNTYPED:
      bld_fetch = &bld_base->base;
      break;
   case TGSI_TYPE_UNSIGNED:
      bld_fetch = &bld_base->uint_bld;
      break;
   case TGSI_TYPE_SIGNED:
      bld_fetch = &bld_base->int_bld;
      break;
   case TGSI_TYPE_DOUBLE:
      bld_fetch = &bld_base->dbl_bld;
      break;
   case TGSI_TYPE_UNSIGNED64:
      bld_fetch = &bld_base->uint64_bld;
      break;
   case TGSI_TYPE_SIGNED64:
      bld_fetch = &bld_base->int64_bld;
      break;
   case TGSI_TYPE_VOID:
   default:
      assert(0);
      bld_fetch = NULL;
      break;
   }
   return bld_fetch;
}

static LLVMValueRef
get_soa_array_offsets(struct lp_build_context *uint_bld,
                      LLVMValueRef indirect_index,
                      unsigned chan_index,
                      bool need_perelement_offset)
{
   struct gallivm_state *gallivm = uint_bld->gallivm;
   LLVMValueRef chan_vec =
      lp_build_const_int_vec(uint_bld->gallivm, uint_bld->type, chan_index);
   LLVMValueRef length_vec =
      lp_build_const_int_vec(gallivm, uint_bld->type, uint_bld->type.length);
   LLVMValueRef index_vec;

   /* index_vec = (indirect_index * 4 + chan_index) * length + offsets */
   index_vec = lp_build_shl_imm(uint_bld, indirect_index, 2);
   index_vec = lp_build_add(uint_bld, index_vec, chan_vec);
   index_vec = lp_build_mul(uint_bld, index_vec, length_vec);

   if (need_perelement_offset) {
      LLVMValueRef pixel_offsets;
      unsigned i;
     /* build pixel offset vector: {0, 1, 2, 3, ...} */
      pixel_offsets = uint_bld->undef;
      for (i = 0; i < uint_bld->type.length; i++) {
         LLVMValueRef ii = lp_build_const_int32(gallivm, i);
         pixel_offsets = LLVMBuildInsertElement(gallivm->builder, pixel_offsets,
                                                ii, ii, "");
      }
      index_vec = lp_build_add(uint_bld, index_vec, pixel_offsets);
   }
   return index_vec;
}

static LLVMValueRef
emit_fetch_constant(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_full_src_register * reg,
   enum tgsi_opcode_type stype,
   unsigned swizzle_in)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context *uint_bld = &bld_base->uint_bld;
   unsigned dimension = 0;
   LLVMValueRef consts_ptr;
   LLVMValueRef num_consts;
   LLVMValueRef res;
   unsigned swizzle = swizzle_in & 0xffff;

   /* XXX: Handle fetching xyzw components as a vector */
   assert(swizzle != ~0u);

   if (reg->Register.Dimension) {
      assert(!reg->Dimension.Indirect);
      dimension = reg->Dimension.Index;
      assert(dimension < LP_MAX_TGSI_CONST_BUFFERS);
   }

   consts_ptr = bld->consts[dimension];
   num_consts = bld->consts_sizes[dimension];

   if (reg->Register.Indirect) {
      LLVMValueRef indirect_index;
      LLVMValueRef swizzle_vec =
         lp_build_const_int_vec(gallivm, uint_bld->type, swizzle);
      LLVMValueRef index_vec;  /* index into the const buffer */
      LLVMValueRef overflow_mask;
      LLVMValueRef index_vec2 = NULL;

      indirect_index = get_indirect_index(bld,
                                          reg->Register.File,
                                          reg->Register.Index,
                                          &reg->Indirect,
                                          bld->bld_base.info->file_max[reg->Register.File]);

      /* All fetches are from the same constant buffer, so
       * we need to propagate the size to a vector to do a
       * vector comparison */
      num_consts = lp_build_broadcast_scalar(uint_bld, num_consts);
      /* Construct a boolean vector telling us which channels
       * overflow the bound constant buffer */
      overflow_mask = lp_build_compare(gallivm, uint_bld->type, PIPE_FUNC_GEQUAL,
                                       indirect_index, num_consts);

      /* index_vec = indirect_index * 4 + swizzle */
      index_vec = lp_build_shl_imm(uint_bld, indirect_index, 2);
      index_vec = lp_build_add(uint_bld, index_vec, swizzle_vec);

      if (tgsi_type_is_64bit(stype)) {
         LLVMValueRef swizzle_vec2;
         swizzle_vec2 = lp_build_const_int_vec(gallivm, uint_bld->type, swizzle_in >> 16);
         index_vec2 = lp_build_shl_imm(uint_bld, indirect_index, 2);
         index_vec2 = lp_build_add(uint_bld, index_vec2, swizzle_vec2);
      }
      /* Gather values from the constant buffer */
      res = build_gather(bld_base, consts_ptr, index_vec, overflow_mask, index_vec2);
   }
   else {
      LLVMValueRef index;  /* index into the const buffer */
      LLVMValueRef scalar, scalar_ptr;
      struct lp_build_context *bld_broad = &bld_base->base;
      index = lp_build_const_int32(gallivm, reg->Register.Index * 4 + swizzle);

      scalar_ptr = LLVMBuildGEP2(builder, bld_broad->elem_type, consts_ptr,
                                 &index, 1, "");

      if (tgsi_type_is_64bit(stype) && ((swizzle_in >> 16) != swizzle + 1)) {

         LLVMValueRef scalar2, scalar2_ptr;
         LLVMValueRef shuffles[2];
         index = lp_build_const_int32(gallivm, reg->Register.Index * 4 + (swizzle_in >> 16));

         scalar2_ptr = LLVMBuildGEP2(builder, bld_broad->elem_type, consts_ptr,
                                    &index, 1, "");

         scalar = LLVMBuildLoad2(builder, bld_broad->elem_type, scalar_ptr, "");
         scalar2 = LLVMBuildLoad2(builder, bld_broad->elem_type, scalar2_ptr, "");
         shuffles[0] = lp_build_const_int32(gallivm, 0);
         shuffles[1] = lp_build_const_int32(gallivm, 1);

         res = LLVMGetUndef(LLVMVectorType(bld_broad->elem_type, bld_base->base.type.length * 2));
         res = LLVMBuildInsertElement(builder, res, scalar, shuffles[0], "");
         res = LLVMBuildInsertElement(builder, res, scalar2, shuffles[1], "");
      } else {
        if (stype == TGSI_TYPE_DOUBLE) {
           LLVMTypeRef dptr_type = LLVMPointerType(LLVMDoubleTypeInContext(gallivm->context), 0);
           scalar_ptr = LLVMBuildBitCast(builder, scalar_ptr, dptr_type, "");
           bld_broad = &bld_base->dbl_bld;
        } else if (stype == TGSI_TYPE_UNSIGNED64) {
           LLVMTypeRef u64ptr_type = LLVMPointerType(LLVMInt64TypeInContext(gallivm->context), 0);
           scalar_ptr = LLVMBuildBitCast(builder, scalar_ptr, u64ptr_type, "");
           bld_broad = &bld_base->uint64_bld;
        } else if (stype == TGSI_TYPE_SIGNED64) {
           LLVMTypeRef i64ptr_type = LLVMPointerType(LLVMInt64TypeInContext(gallivm->context), 0);
           scalar_ptr = LLVMBuildBitCast(builder, scalar_ptr, i64ptr_type, "");
           bld_broad = &bld_base->int64_bld;
        }
        scalar = LLVMBuildLoad2(builder, bld_broad->elem_type, scalar_ptr, "");
        res = lp_build_broadcast_scalar(bld_broad, scalar);
      }

   }

   if (stype == TGSI_TYPE_SIGNED || stype == TGSI_TYPE_UNSIGNED || stype == TGSI_TYPE_DOUBLE || stype == TGSI_TYPE_SIGNED64 || stype == TGSI_TYPE_UNSIGNED64) {
      struct lp_build_context *bld_fetch = stype_to_fetch(bld_base, stype);
      res = LLVMBuildBitCast(builder, res, bld_fetch->vec_type, "");
   }

   return res;
}

/**
 * Fetch 64-bit values from two separate channels.
 * 64-bit values are stored split across two channels, like xy and zw.
 * This function creates a set of vec_length*2 floats,
 * extracts the values from the two channels,
 * puts them in the correct place, then casts to vec_length 64-bits.
 */
static LLVMValueRef
emit_fetch_64bit(
   struct lp_build_tgsi_context * bld_base,
   enum tgsi_opcode_type stype,
   LLVMValueRef input,
   LLVMValueRef input2)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef res;
   struct lp_build_context *bld_fetch = stype_to_fetch(bld_base, stype);
   int i;
   LLVMValueRef shuffles[2 * (LP_MAX_VECTOR_WIDTH/32)];
   int len = bld_base->base.type.length * 2;
   assert(len <= (2 * (LP_MAX_VECTOR_WIDTH/32)));

   for (i = 0; i < bld_base->base.type.length * 2; i+=2) {
      shuffles[i] = lp_build_const_int32(gallivm, i / 2);
      shuffles[i + 1] = lp_build_const_int32(gallivm, i / 2 + bld_base->base.type.length);
   }
   res = LLVMBuildShuffleVector(builder, input, input2, LLVMConstVector(shuffles, len), "");

   return LLVMBuildBitCast(builder, res, bld_fetch->vec_type, "");
}

static LLVMValueRef
emit_fetch_immediate(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_full_src_register * reg,
   enum tgsi_opcode_type stype,
   unsigned swizzle_in)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef res = NULL;
   unsigned swizzle = swizzle_in & 0xffff;

   if (bld->use_immediates_array || reg->Register.Indirect) {
      LLVMValueRef imms_array;
      LLVMTypeRef fptr_type;

      /* cast imms_array pointer to float* */
      fptr_type = LLVMPointerType(LLVMFloatTypeInContext(gallivm->context), 0);
      imms_array = LLVMBuildBitCast(builder, bld->imms_array, fptr_type, "");

      if (reg->Register.Indirect) {
         LLVMValueRef indirect_index;
         LLVMValueRef index_vec;  /* index into the immediate register array */
         LLVMValueRef index_vec2 = NULL;
         indirect_index = get_indirect_index(bld,
                                             reg->Register.File,
                                             reg->Register.Index,
                                             &reg->Indirect,
                                             bld->bld_base.info->file_max[reg->Register.File]);
         /*
          * Unlike for other reg classes, adding pixel offsets is unnecessary -
          * immediates are stored as full vectors (FIXME??? - might be better
          * to store them the same as constants) but all elements are the same
          * in any case.
          */
         index_vec = get_soa_array_offsets(&bld_base->uint_bld,
                                           indirect_index,
                                           swizzle,
                                           false);
         if (tgsi_type_is_64bit(stype))
            index_vec2 = get_soa_array_offsets(&bld_base->uint_bld,
                                              indirect_index,
                                              swizzle_in >> 16,
                                              false);
         /* Gather values from the immediate register array */
         res = build_gather(bld_base, imms_array, index_vec, NULL, index_vec2);
      } else {
         LLVMValueRef gep[2];
         gep[0] = lp_build_const_int32(gallivm, 0);
         gep[1] = lp_build_const_int32(gallivm, reg->Register.Index * 4 + swizzle);
         LLVMValueRef imms_ptr = LLVMBuildGEP2(builder,
                                               bld_base->base.vec_type,
                                               bld->imms_array, gep, 2, "");
         res = LLVMBuildLoad2(builder, bld_base->base.vec_type, imms_ptr, "");

         if (tgsi_type_is_64bit(stype)) {
            LLVMValueRef imms_ptr2;
            LLVMValueRef res2;
            gep[1] = lp_build_const_int32(gallivm,
                                          reg->Register.Index * 4 + (swizzle_in >> 16));
            imms_ptr2 = LLVMBuildGEP2(builder, bld_base->base.vec_type,
                                      bld->imms_array, gep, 2, "");
            res2 = LLVMBuildLoad2(builder, bld_base->base.vec_type, imms_ptr2, "");
            res = emit_fetch_64bit(bld_base, stype, res, res2);
         }
      }
   }
   else {
      res = bld->immediates[reg->Register.Index][swizzle];
      if (tgsi_type_is_64bit(stype))
         res = emit_fetch_64bit(bld_base, stype, res, bld->immediates[reg->Register.Index][swizzle_in >> 16]);
   }

   if (stype == TGSI_TYPE_SIGNED || stype == TGSI_TYPE_UNSIGNED || tgsi_type_is_64bit(stype)) {
      struct lp_build_context *bld_fetch = stype_to_fetch(bld_base, stype);
      res = LLVMBuildBitCast(builder, res, bld_fetch->vec_type, "");
   }
   return res;
}

static LLVMValueRef
emit_fetch_input(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_full_src_register * reg,
   enum tgsi_opcode_type stype,
   unsigned swizzle_in)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef res;
   unsigned swizzle = swizzle_in & 0xffff;

   if (reg->Register.Indirect) {
      LLVMValueRef indirect_index;
      LLVMValueRef index_vec;  /* index into the input reg array */
      LLVMValueRef index_vec2 = NULL;
      LLVMValueRef inputs_array;
      LLVMTypeRef fptr_type;

      indirect_index = get_indirect_index(bld,
                                          reg->Register.File,
                                          reg->Register.Index,
                                          &reg->Indirect,
                                          bld->bld_base.info->file_max[reg->Register.File]);

      index_vec = get_soa_array_offsets(&bld_base->uint_bld,
                                        indirect_index,
                                        swizzle,
                                        true);
      if (tgsi_type_is_64bit(stype)) {
         index_vec2 = get_soa_array_offsets(&bld_base->uint_bld,
                                           indirect_index,
                                           swizzle_in >> 16,
                                           true);
      }
      /* cast inputs_array pointer to float* */
      fptr_type = LLVMPointerType(LLVMFloatTypeInContext(gallivm->context), 0);
      inputs_array = LLVMBuildBitCast(builder, bld->inputs_array, fptr_type, "");

      /* Gather values from the input register array */
      res = build_gather(bld_base, inputs_array, index_vec, NULL, index_vec2);
   } else {
      if (bld->indirect_files & (1 << TGSI_FILE_INPUT)) {
         LLVMValueRef lindex = lp_build_const_int32(gallivm,
                                        reg->Register.Index * 4 + swizzle);
         LLVMValueRef input_ptr = LLVMBuildGEP2(builder, bld_base->base.vec_type,
                                                bld->inputs_array, &lindex, 1, "");

         res = LLVMBuildLoad2(builder, bld_base->base.vec_type, input_ptr, "");
         if (tgsi_type_is_64bit(stype)) {
            LLVMValueRef lindex1;
            LLVMValueRef input_ptr2;
            LLVMValueRef res2;

            lindex1 = lp_build_const_int32(gallivm,
                                           reg->Register.Index * 4 + (swizzle_in >> 16));
            input_ptr2 = LLVMBuildGEP2(builder, bld_base->base.vec_type,
                                       bld->inputs_array, &lindex1, 1, "");
            res2 = LLVMBuildLoad2(builder, bld_base->base.vec_type, input_ptr2, "");
            res = emit_fetch_64bit(bld_base, stype, res, res2);
         }
      }
      else {
         res = bld->inputs[reg->Register.Index][swizzle];
         if (tgsi_type_is_64bit(stype))
            res = emit_fetch_64bit(bld_base, stype, res, bld->inputs[reg->Register.Index][swizzle_in >> 16]);
      }
   }

   assert(res);

   if (stype == TGSI_TYPE_SIGNED || stype == TGSI_TYPE_UNSIGNED || tgsi_type_is_64bit(stype)) {
      struct lp_build_context *bld_fetch = stype_to_fetch(bld_base, stype);
      res = LLVMBuildBitCast(builder, res, bld_fetch->vec_type, "");
   }

   return res;
}


static LLVMValueRef
emit_fetch_gs_input(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_full_src_register * reg,
   enum tgsi_opcode_type stype,
   unsigned swizzle_in)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   const struct tgsi_shader_info *info = bld->bld_base.info;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef attrib_index = NULL;
   LLVMValueRef vertex_index = NULL;
   unsigned swizzle = swizzle_in & 0xffff;
   LLVMValueRef swizzle_index = lp_build_const_int32(gallivm, swizzle);
   LLVMValueRef res;

   if (info->input_semantic_name[reg->Register.Index] == TGSI_SEMANTIC_PRIMID) {
      /* This is really a system value not a regular input */
      assert(!reg->Register.Indirect);
      assert(!reg->Dimension.Indirect);
      res = bld->system_values.prim_id;
      if (stype != TGSI_TYPE_UNSIGNED && stype != TGSI_TYPE_SIGNED) {
         res = LLVMBuildBitCast(builder, res, bld_base->base.vec_type, "");
      }
      return res;
   }

   if (reg->Register.Indirect) {
      /*
       * XXX: this is possibly not quite the right value, since file_max may be
       * larger than the max attrib index, due to it being the max of declared
       * inputs AND the max vertices per prim (which is 6 for tri adj).
       * It should however be safe to use (since we always allocate
       * PIPE_MAX_SHADER_INPUTS (80) for it, which is overallocated quite a bit).
       */
      int index_limit = info->file_max[reg->Register.File];
      attrib_index = get_indirect_index(bld,
                                        reg->Register.File,
                                        reg->Register.Index,
                                        &reg->Indirect,
                                        index_limit);
   } else {
      attrib_index = lp_build_const_int32(gallivm, reg->Register.Index);
   }

   if (reg->Dimension.Indirect) {
      /*
       * A fixed 6 should do as well (which is what we allocate).
       */
      int index_limit = mesa_vertices_per_prim(info->properties[TGSI_PROPERTY_GS_INPUT_PRIM]);
      vertex_index = get_indirect_index(bld,
                                        reg->Register.File,
                                        reg->Dimension.Index,
                                        &reg->DimIndirect,
                                        index_limit);
   } else {
      vertex_index = lp_build_const_int32(gallivm, reg->Dimension.Index);
   }

   res = bld->gs_iface->fetch_input(bld->gs_iface, &bld_base->base,
                                    reg->Dimension.Indirect,
                                    vertex_index,
                                    reg->Register.Indirect,
                                    attrib_index,
                                    swizzle_index);

   assert(res);
   if (tgsi_type_is_64bit(stype)) {
      LLVMValueRef swizzle_index = lp_build_const_int32(gallivm, swizzle_in >> 16);
      LLVMValueRef res2;
      res2 = bld->gs_iface->fetch_input(bld->gs_iface, &bld_base->base,
                                        reg->Dimension.Indirect,
                                        vertex_index,
                                        reg->Register.Indirect,
                                        attrib_index,
                                        swizzle_index);
      assert(res2);
      res = emit_fetch_64bit(bld_base, stype, res, res2);
   } else if (stype == TGSI_TYPE_UNSIGNED) {
      res = LLVMBuildBitCast(builder, res, bld_base->uint_bld.vec_type, "");
   } else if (stype == TGSI_TYPE_SIGNED) {
      res = LLVMBuildBitCast(builder, res, bld_base->int_bld.vec_type, "");
   }

   return res;
}

static LLVMValueRef
emit_fetch_tcs_input(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_full_src_register * reg,
   enum tgsi_opcode_type stype,
   unsigned swizzle_in)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   const struct tgsi_shader_info *info = bld->bld_base.info;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef attrib_index = NULL;
   LLVMValueRef vertex_index = NULL;
   unsigned swizzle = swizzle_in & 0xffff;
   LLVMValueRef swizzle_index = lp_build_const_int32(gallivm, swizzle);
   LLVMValueRef res;

   if (info->input_semantic_name[reg->Register.Index] == TGSI_SEMANTIC_PRIMID) {
      /* This is really a system value not a regular input */
      assert(!reg->Register.Indirect);
      assert(!reg->Dimension.Indirect);
      res = bld->system_values.prim_id;
      if (stype != TGSI_TYPE_UNSIGNED && stype != TGSI_TYPE_SIGNED) {
         res = LLVMBuildBitCast(builder, res, bld_base->base.vec_type, "");
      }
      return res;
   }

   if (reg->Register.Indirect) {
      int index_limit = info->file_max[reg->Register.File];
      attrib_index = get_indirect_index(bld,
                                        reg->Register.File,
                                        reg->Register.Index,
                                        &reg->Indirect,
                                        index_limit);
   } else {
      attrib_index = lp_build_const_int32(gallivm, reg->Register.Index);
   }

   if (reg->Dimension.Indirect) {
      vertex_index = get_indirect_index(bld,
                                        reg->Register.File,
                                        reg->Dimension.Index,
                                        &reg->DimIndirect,
                                        PIPE_MAX_SHADER_INPUTS);
   } else {
      vertex_index = lp_build_const_int32(gallivm, reg->Dimension.Index);
   }

   // TCS can read from its own outputs
   if (reg->Register.File == TGSI_FILE_OUTPUT) {
      res = bld->tcs_iface->emit_fetch_output(bld->tcs_iface, (struct lp_build_context*)bld_base,
                                              reg->Dimension.Indirect,
                                              vertex_index,
                                              reg->Register.Indirect,
                                              attrib_index,
                                              false,
                                              swizzle_index,
                                              bld_base->info->output_semantic_name[reg->Register.Index]);
   } else {
      res = bld->tcs_iface->emit_fetch_input(bld->tcs_iface, (struct lp_build_context*)bld_base,
                                             reg->Dimension.Indirect,
                                             vertex_index,
                                             reg->Register.Indirect,
                                             attrib_index,
                                             false,
                                             swizzle_index);
   }


   assert(res);
   if (tgsi_type_is_64bit(stype)) {
      LLVMValueRef swizzle_index = lp_build_const_int32(gallivm, swizzle_in >> 16);
      LLVMValueRef res2;
      if (reg->Register.File == TGSI_FILE_OUTPUT) {
         res2 = bld->tcs_iface->emit_fetch_output(bld->tcs_iface, (struct lp_build_context*)bld_base,
                                                  reg->Dimension.Indirect,
                                                  vertex_index,
                                                  reg->Register.Indirect,
                                                  attrib_index,
                                                  false,
                                                  swizzle_index,
                                                  bld_base->info->output_semantic_name[reg->Register.Index]);
      } else {
         res2 = bld->tcs_iface->emit_fetch_input(bld->tcs_iface, (struct lp_build_context*)bld_base,
                                                 reg->Dimension.Indirect,
                                                 vertex_index,
                                                 reg->Register.Indirect,
                                                 attrib_index,
                                                 false,
                                                 swizzle_index);
      }
      assert(res2);
      res = emit_fetch_64bit(bld_base, stype, res, res2);
   } else if (stype == TGSI_TYPE_UNSIGNED) {
      res = LLVMBuildBitCast(builder, res, bld_base->uint_bld.vec_type, "");
   } else if (stype == TGSI_TYPE_SIGNED) {
      res = LLVMBuildBitCast(builder, res, bld_base->int_bld.vec_type, "");
   }

   return res;
}

static LLVMValueRef
emit_fetch_tes_input(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_full_src_register * reg,
   enum tgsi_opcode_type stype,
   unsigned swizzle_in)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   const struct tgsi_shader_info *info = bld->bld_base.info;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef attrib_index = NULL;
   LLVMValueRef vertex_index = NULL;
   unsigned swizzle = swizzle_in & 0xffff;
   LLVMValueRef swizzle_index = lp_build_const_int32(gallivm, swizzle);
   LLVMValueRef res;

   if (info->input_semantic_name[reg->Register.Index] == TGSI_SEMANTIC_PRIMID) {
      /* This is really a system value not a regular input */
      assert(!reg->Register.Indirect);
      assert(!reg->Dimension.Indirect);
      res = bld->system_values.prim_id;
      if (stype != TGSI_TYPE_UNSIGNED && stype != TGSI_TYPE_SIGNED) {
         res = LLVMBuildBitCast(builder, res, bld_base->base.vec_type, "");
      }
      return res;
   }

   if (reg->Register.Indirect) {
      int index_limit = info->file_max[reg->Register.File];
      attrib_index = get_indirect_index(bld,
                                        reg->Register.File,
                                        reg->Register.Index,
                                        &reg->Indirect,
                                        index_limit);
   } else {
      attrib_index = lp_build_const_int32(gallivm, reg->Register.Index);
   }

   if (reg->Dimension.Indirect) {
      vertex_index = get_indirect_index(bld,
                                        reg->Register.File,
                                        reg->Dimension.Index,
                                        &reg->DimIndirect,
                                        PIPE_MAX_SHADER_INPUTS);
   } else {
      vertex_index = lp_build_const_int32(gallivm, reg->Dimension.Index);
   }

   if (info->input_semantic_name[reg->Register.Index] == TGSI_SEMANTIC_PATCH) {
      res = bld->tes_iface->fetch_patch_input(bld->tes_iface, (struct lp_build_context*)bld_base,
                                     reg->Register.Indirect,
                                     attrib_index,
                                     swizzle_index);
   } else {
      res = bld->tes_iface->fetch_vertex_input(bld->tes_iface, (struct lp_build_context*)bld_base,
                                       reg->Dimension.Indirect,
                                       vertex_index,
                                       reg->Register.Indirect,
                                       attrib_index,
                                       false,
                                       swizzle_index);
   }

   assert(res);
   if (tgsi_type_is_64bit(stype)) {
      LLVMValueRef swizzle_index = lp_build_const_int32(gallivm, swizzle_in >> 16);
      LLVMValueRef res2;
      if (info->input_semantic_name[reg->Register.Index] == TGSI_SEMANTIC_PATCH) {
         res2 = bld->tes_iface->fetch_patch_input(bld->tes_iface, (struct lp_build_context*)bld_base,
                                    reg->Register.Indirect,
                                    attrib_index,
                                    swizzle_index);
      }
      else {
         res2 = bld->tes_iface->fetch_vertex_input(bld->tes_iface, (struct lp_build_context*)bld_base,
                                             reg->Dimension.Indirect,
                                             vertex_index,
                                             reg->Register.Indirect,
                                             attrib_index,
                                             false,
                                             swizzle_index);
      }
      assert(res2);
      res = emit_fetch_64bit(bld_base, stype, res, res2);
   } else if (stype == TGSI_TYPE_UNSIGNED) {
      res = LLVMBuildBitCast(builder, res, bld_base->uint_bld.vec_type, "");
   } else if (stype == TGSI_TYPE_SIGNED) {
      res = LLVMBuildBitCast(builder, res, bld_base->int_bld.vec_type, "");
   }

   return res;
}



static LLVMValueRef
emit_fetch_temporary(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_full_src_register * reg,
   enum tgsi_opcode_type stype,
   unsigned swizzle_in)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef res;
   unsigned swizzle = swizzle_in & 0xffff;

   if (reg->Register.Indirect) {
      LLVMValueRef indirect_index;
      LLVMValueRef index_vec, index_vec2 = NULL;  /* index into the temp reg array */
      LLVMValueRef temps_array;
      LLVMTypeRef fptr_type;

      indirect_index = get_indirect_index(bld,
                                          reg->Register.File,
                                          reg->Register.Index,
                                          &reg->Indirect,
                                          bld->bld_base.info->file_max[reg->Register.File]);

      index_vec = get_soa_array_offsets(&bld_base->uint_bld,
                                        indirect_index,
                                        swizzle,
                                        true);
      if (tgsi_type_is_64bit(stype)) {
               index_vec2 = get_soa_array_offsets(&bld_base->uint_bld,
                                                  indirect_index,
                                                  swizzle_in >> 16,
                                                  true);
      }

      /* cast temps_array pointer to float* */
      fptr_type = LLVMPointerType(LLVMFloatTypeInContext(gallivm->context), 0);
      temps_array = LLVMBuildBitCast(builder, bld->temps_array, fptr_type, "");

      /* Gather values from the temporary register array */
      res = build_gather(bld_base, temps_array, index_vec, NULL, index_vec2);
   }
   else {
      LLVMValueRef temp_ptr;
      LLVMTypeRef vec_type = bld->bld_base.base.vec_type;
      temp_ptr = lp_get_temp_ptr_soa(bld, reg->Register.Index, swizzle);
      res = LLVMBuildLoad2(builder, vec_type, temp_ptr, "");

      if (tgsi_type_is_64bit(stype)) {
         LLVMValueRef temp_ptr2, res2;

         temp_ptr2 = lp_get_temp_ptr_soa(bld, reg->Register.Index, swizzle_in >> 16);
         res2 = LLVMBuildLoad2(builder, vec_type, temp_ptr2, "");
         res = emit_fetch_64bit(bld_base, stype, res, res2);
      }
   }

   if (stype == TGSI_TYPE_SIGNED ||
       stype == TGSI_TYPE_UNSIGNED ||
       stype == TGSI_TYPE_DOUBLE ||
       stype == TGSI_TYPE_SIGNED64 ||
       stype == TGSI_TYPE_UNSIGNED64) {
      struct lp_build_context *bld_fetch = stype_to_fetch(bld_base, stype);
      res = LLVMBuildBitCast(builder, res, bld_fetch->vec_type, "");
   }

   return res;
}

static LLVMValueRef
emit_fetch_system_value(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_full_src_register * reg,
   enum tgsi_opcode_type stype,
   unsigned swizzle_in)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   const struct tgsi_shader_info *info = bld->bld_base.info;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef res;
   enum tgsi_opcode_type atype; // Actual type of the value
   unsigned swizzle = swizzle_in & 0xffff;

   assert(!reg->Register.Indirect);

   switch (info->system_value_semantic_name[reg->Register.Index]) {
   case TGSI_SEMANTIC_INSTANCEID:
      res = lp_build_broadcast_scalar(&bld_base->uint_bld, bld->system_values.instance_id);
      atype = TGSI_TYPE_UNSIGNED;
      break;

   case TGSI_SEMANTIC_VERTEXID:
      res = bld->system_values.vertex_id;
      atype = TGSI_TYPE_UNSIGNED;
      break;

   case TGSI_SEMANTIC_VERTEXID_NOBASE:
      res = bld->system_values.vertex_id_nobase;
      atype = TGSI_TYPE_UNSIGNED;
      break;

   case TGSI_SEMANTIC_BASEVERTEX:
      res = bld->system_values.basevertex;
      atype = TGSI_TYPE_UNSIGNED;
      break;

   case TGSI_SEMANTIC_BASEINSTANCE:
      res = lp_build_broadcast_scalar(&bld_base->uint_bld, bld->system_values.base_instance);
      atype = TGSI_TYPE_UNSIGNED;
      break;

   case TGSI_SEMANTIC_PRIMID:
      res = bld->system_values.prim_id;
      atype = TGSI_TYPE_UNSIGNED;
      break;

   case TGSI_SEMANTIC_INVOCATIONID:
      if (info->processor == PIPE_SHADER_TESS_CTRL)
         res = bld->system_values.invocation_id;
      else
         res = lp_build_broadcast_scalar(&bld_base->uint_bld, bld->system_values.invocation_id);
      atype = TGSI_TYPE_UNSIGNED;
      break;

   case TGSI_SEMANTIC_HELPER_INVOCATION:
      res = LLVMBuildNot(gallivm->builder, lp_build_mask_value(bld->mask), "");
      atype = TGSI_TYPE_UNSIGNED;
      break;

   case TGSI_SEMANTIC_THREAD_ID:
      res = bld->system_values.thread_id[swizzle];
      atype = TGSI_TYPE_UNSIGNED;
      break;

   case TGSI_SEMANTIC_BLOCK_ID:
      res = lp_build_broadcast_scalar(&bld_base->uint_bld, bld->system_values.block_id[swizzle]);
      atype = TGSI_TYPE_UNSIGNED;
      break;

   case TGSI_SEMANTIC_GRID_SIZE:
      res = lp_build_broadcast_scalar(&bld_base->uint_bld, bld->system_values.grid_size[swizzle]);
      atype = TGSI_TYPE_UNSIGNED;
      break;

   case TGSI_SEMANTIC_TESSCOORD:
      {
         LLVMValueRef index[] = { lp_build_const_int32(gallivm, 0), lp_build_const_int32(gallivm, swizzle_in) };
         LLVMValueRef array_indexed = LLVMBuildGEP2(gallivm->builder, bld->bld_base.base.vec_type,
                                                    bld->system_values.tess_coord, index, 2, "tess_coord_array_indexed");
         res = LLVMBuildLoad2(builder, bld->bld_base.base.vec_type, array_indexed, "tess_coord");
      }
      atype = TGSI_TYPE_FLOAT;
      break;

   case TGSI_SEMANTIC_FACE:
      res = lp_build_broadcast_scalar(&bld_base->uint_bld, bld->system_values.front_facing);
      atype = TGSI_TYPE_UNSIGNED;
      break;

  case TGSI_SEMANTIC_DRAWID:
      res = lp_build_broadcast_scalar(&bld_base->uint_bld, bld->system_values.draw_id);
      atype = TGSI_TYPE_UNSIGNED;
      break;

  case TGSI_SEMANTIC_SAMPLEID:
      res = lp_build_broadcast_scalar(&bld_base->uint_bld, bld->system_values.sample_id);
      atype = TGSI_TYPE_UNSIGNED;
      break;

   case TGSI_SEMANTIC_TESSOUTER:
      res = lp_build_extract_broadcast(gallivm, lp_type_float_vec(32, 128), bld_base->base.type,
                                       bld->system_values.tess_outer,
                                       lp_build_const_int32(gallivm, swizzle_in));
      atype = TGSI_TYPE_FLOAT;
      break;

   case TGSI_SEMANTIC_TESSINNER:
      res = lp_build_extract_broadcast(gallivm, lp_type_float_vec(32, 128), bld_base->base.type,
                                       bld->system_values.tess_inner,
                                       lp_build_const_int32(gallivm, swizzle_in));
      atype = TGSI_TYPE_FLOAT;
      break;

   case TGSI_SEMANTIC_VERTICESIN:
      res = lp_build_broadcast_scalar(&bld_base->uint_bld, bld->system_values.vertices_in);
      atype = TGSI_TYPE_UNSIGNED;
      break;

   default:
      assert(!"unexpected semantic in emit_fetch_system_value");
      res = bld_base->base.zero;
      atype = TGSI_TYPE_FLOAT;
      break;
   }

   if (atype != stype) {
      if (stype == TGSI_TYPE_FLOAT) {
         res = LLVMBuildBitCast(builder, res, bld_base->base.vec_type, "");
      } else if (stype == TGSI_TYPE_UNSIGNED) {
         res = LLVMBuildBitCast(builder, res, bld_base->uint_bld.vec_type, "");
      } else if (stype == TGSI_TYPE_SIGNED) {
         res = LLVMBuildBitCast(builder, res, bld_base->int_bld.vec_type, "");
      }
   }

   return res;
}

/**
 * Register fetch with derivatives.
 */
static void
emit_fetch_deriv(
   struct lp_build_tgsi_soa_context *bld,
   LLVMValueRef src,
   LLVMValueRef *res,
   LLVMValueRef *ddx,
   LLVMValueRef *ddy)
{
   if (res)
      *res = src;

   /* TODO: use interpolation coeffs for inputs */

   if (ddx)
      *ddx = lp_build_ddx(&bld->bld_base.base, src);

   if (ddy)
      *ddy = lp_build_ddy(&bld->bld_base.base, src);
}

/**
 * store an array of vec-length 64-bit into two arrays of vec_length floats
 * i.e.
 * value is d0, d1, d2, d3 etc.
 * each 64-bit has high and low pieces x, y
 * so gets stored into the separate channels as:
 * chan_ptr = d0.x, d1.x, d2.x, d3.x
 * chan_ptr2 = d0.y, d1.y, d2.y, d3.y
 */
static void
emit_store_64bit_chan(struct lp_build_tgsi_context *bld_base,
                      LLVMValueRef chan_ptr, LLVMValueRef chan_ptr2,
                      LLVMValueRef value)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context *float_bld = &bld_base->base;
   unsigned i;
   LLVMValueRef temp, temp2;
   LLVMValueRef shuffles[LP_MAX_VECTOR_WIDTH/32];
   LLVMValueRef shuffles2[LP_MAX_VECTOR_WIDTH/32];

   for (i = 0; i < bld_base->base.type.length; i++) {
      shuffles[i] = lp_build_const_int32(gallivm, i * 2);
      shuffles2[i] = lp_build_const_int32(gallivm, (i * 2) + 1);
   }

   temp = LLVMBuildShuffleVector(builder, value,
                                 LLVMGetUndef(LLVMTypeOf(value)),
                                 LLVMConstVector(shuffles,
                                                 bld_base->base.type.length),
                                 "");
   temp2 = LLVMBuildShuffleVector(builder, value,
                                  LLVMGetUndef(LLVMTypeOf(value)),
                                  LLVMConstVector(shuffles2,
                                                  bld_base->base.type.length),
                                  "");

   lp_exec_mask_store(&bld->exec_mask, float_bld, temp, chan_ptr);
   lp_exec_mask_store(&bld->exec_mask, float_bld, temp2, chan_ptr2);
}

static void
emit_store_output(struct lp_build_tgsi_context *bld_base,
                  enum tgsi_opcode_type dtype,
                  const struct tgsi_full_dst_register *reg,
                  unsigned index,
                  unsigned chan_index,
                  LLVMValueRef indirect_index,
                  LLVMValueRef value)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context *float_bld = &bld_base->base;

   /* Outputs are always stored as floats */
   value = LLVMBuildBitCast(builder, value, float_bld->vec_type, "");

   if (reg->Register.Indirect) {
      LLVMValueRef index_vec;  /* indexes into the output registers */
      LLVMValueRef outputs_array;
      LLVMTypeRef fptr_type;

      index_vec = get_soa_array_offsets(&bld_base->uint_bld,
                                          indirect_index,
                                          chan_index,
                                          true);

      fptr_type = LLVMPointerType(LLVMFloatTypeInContext(gallivm->context), 0);
      outputs_array = LLVMBuildBitCast(builder, bld->outputs_array, fptr_type, "");

      /* Scatter store values into output registers */
      emit_mask_scatter(bld, outputs_array, index_vec, value,
                        &bld->exec_mask);
   }
   else {
      assert(LLVMTypeOf(value) == float_bld->vec_type);
      LLVMValueRef out_ptr = lp_get_output_ptr(bld, reg->Register.Index,
                                                chan_index);

      if (tgsi_type_is_64bit(dtype)) {
         LLVMValueRef out_ptr2 = lp_get_output_ptr(bld, reg->Register.Index,
                                                   chan_index + 1);
         emit_store_64bit_chan(bld_base, out_ptr, out_ptr2,
                                 value);
      } else
         lp_exec_mask_store(&bld->exec_mask, float_bld, value, out_ptr);
   }
}

static void
emit_store_tcs_output(struct lp_build_tgsi_context *bld_base,
                      enum tgsi_opcode_type dtype,
                      const struct tgsi_full_dst_register *reg,
                      unsigned index,
                      unsigned chan_index,
                      LLVMValueRef indirect_index,
                      LLVMValueRef value)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   const struct tgsi_shader_info *info = bld->bld_base.info;
   LLVMValueRef attrib_index = NULL;
   LLVMValueRef vertex_index = NULL;
   LLVMValueRef channel_index = NULL;

   if (reg->Register.Indirect) {
      /*
       * XXX: this is possibly not quite the right value, since file_max may be
       * larger than the max attrib index, due to it being the max of declared
       * inputs AND the max vertices per prim (which is 6 for tri adj).
       * It should however be safe to use (since we always allocate
       * PIPE_MAX_SHADER_INPUTS (80) for it, which is overallocated quite a bit).
       */
      int index_limit = info->file_max[reg->Register.File];
      attrib_index = get_indirect_index(bld,
                                        reg->Register.File,
                                        reg->Register.Index,
                                        &reg->Indirect,
                                        index_limit);
   } else {
      attrib_index = lp_build_const_int32(gallivm, reg->Register.Index);
   }

   if (reg->Dimension.Indirect) {
      vertex_index = get_indirect_index(bld,
                                        reg->Register.File,
                                        reg->Dimension.Index,
                                        &reg->DimIndirect,
                                        PIPE_MAX_SHADER_OUTPUTS);
   } else {
      vertex_index = lp_build_const_int32(gallivm, reg->Dimension.Index);
   }

   channel_index = lp_build_const_int32(gallivm, chan_index);

   assert(bld->tcs_iface->emit_store_output);
   bld->tcs_iface->emit_store_output(bld->tcs_iface, (struct lp_build_context*)bld_base,
                                          bld_base->info->output_semantic_name[reg->Register.Index],
                                          reg->Dimension.Indirect,
                                          vertex_index,
                                          reg->Register.Indirect,
                                          attrib_index,
                                          false,
                                          channel_index,
                                          value,
                                          mask_vec(bld_base));
}

static void
emit_store_temp(struct lp_build_tgsi_context *bld_base,
                  enum tgsi_opcode_type dtype,
                  const struct tgsi_full_dst_register *reg,
                  unsigned index,
                  unsigned chan_index,
                  LLVMValueRef indirect_index,
                  LLVMValueRef value)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context *float_bld = &bld_base->base;

   /* Temporaries are always stored as floats */
   if (!tgsi_type_is_64bit(dtype))
      value = LLVMBuildBitCast(builder, value, float_bld->vec_type, "");
   else
      value = LLVMBuildBitCast(builder, value,  LLVMVectorType(LLVMFloatTypeInContext(gallivm->context), bld_base->base.type.length * 2), "");

   if (reg->Register.Indirect) {
      LLVMValueRef index_vec;  /* indexes into the temp registers */
      LLVMValueRef temps_array;
      LLVMTypeRef fptr_type;

      index_vec = get_soa_array_offsets(&bld_base->uint_bld,
                                          indirect_index,
                                          chan_index,
                                          true);

      fptr_type = LLVMPointerType(LLVMFloatTypeInContext(gallivm->context), 0);
      temps_array = LLVMBuildBitCast(builder, bld->temps_array, fptr_type, "");

      /* Scatter store values into temp registers */
      emit_mask_scatter(bld, temps_array, index_vec, value,
                        &bld->exec_mask);
   }
   else {
      LLVMValueRef temp_ptr;
      temp_ptr = lp_get_temp_ptr_soa(bld, reg->Register.Index, chan_index);

      if (tgsi_type_is_64bit(dtype)) {
         LLVMValueRef temp_ptr2 = lp_get_temp_ptr_soa(bld,
                                                      reg->Register.Index,
                                                      chan_index + 1);
         emit_store_64bit_chan(bld_base, temp_ptr, temp_ptr2,
                                 value);
      }
      else
         lp_exec_mask_store(&bld->exec_mask, float_bld, value, temp_ptr);
   }
}

static void
emit_store_address(struct lp_build_tgsi_context *bld_base,
                   enum tgsi_opcode_type dtype,
                   const struct tgsi_full_dst_register *reg,
                   unsigned index,
                   unsigned chan_index,
                   LLVMValueRef indirect_index,
                   LLVMValueRef value)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context *int_bld = &bld_base->int_bld;

   assert(dtype == TGSI_TYPE_SIGNED);
   assert(LLVMTypeOf(value) == int_bld->vec_type);
   value = LLVMBuildBitCast(builder, value, int_bld->vec_type, "");
   lp_exec_mask_store(&bld->exec_mask, int_bld, value,
                        bld->addr[reg->Register.Index][chan_index]);
}

/**
 * Register store.
 */
static void
emit_store_chan(
   struct lp_build_tgsi_context *bld_base,
   const struct tgsi_full_instruction *inst,
   unsigned index,
   unsigned chan_index,
   LLVMValueRef value)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   const struct tgsi_full_dst_register *reg = &inst->Dst[index];
   struct lp_build_context *float_bld = &bld_base->base;
   LLVMValueRef indirect_index = NULL;
   enum tgsi_opcode_type dtype = tgsi_opcode_infer_dst_type(inst->Instruction.Opcode, index);

   /*
    * Apply saturation.
    *
    * It is always assumed to be float.
    */
   if (inst->Instruction.Saturate) {
      assert(dtype == TGSI_TYPE_FLOAT ||
             dtype == TGSI_TYPE_UNTYPED);
      value = LLVMBuildBitCast(builder, value, float_bld->vec_type, "");
      value = lp_build_clamp_zero_one_nanzero(float_bld, value);
   }

   if (reg->Register.Indirect) {
      /*
       * Currently the mesa/st doesn't generate indirect stores
       * to 64-bit values, it normally uses MOV to do indirect stores.
       */
      assert(!tgsi_type_is_64bit(dtype));
      indirect_index = get_indirect_index(bld,
                                          reg->Register.File,
                                          reg->Register.Index,
                                          &reg->Indirect,
                                          bld->bld_base.info->file_max[reg->Register.File]);
   } else {
      assert(reg->Register.Index <=
                             bld_base->info->file_max[reg->Register.File]);
   }

   if (DEBUG_EXECUTION) {
      emit_dump_reg(gallivm, reg->Register.File, reg->Register.Index, chan_index, value);
   }

   assert(bld_base->emit_store_reg_funcs[reg->Register.File]);
   bld_base->emit_store_reg_funcs[reg->Register.File](bld_base,
                                                      dtype,
                                                      reg,
                                                      index,
                                                      chan_index,
                                                      indirect_index,
                                                      value);

   (void)dtype;
}

/*
 * Called at the beginning of the translation of each TGSI instruction, to
 * emit some debug code.
 */
static void
emit_debug(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_full_instruction * inst,
   const struct tgsi_opcode_info * info)

{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   if (DEBUG_EXECUTION) {
      /*
       * Dump the TGSI instruction.
       */

      struct gallivm_state *gallivm = bld_base->base.gallivm;
      char buf[512];
      buf[0] = '$';
      buf[1] = ' ';
      tgsi_dump_instruction_str(inst, bld_base->pc, &buf[2], sizeof buf - 2);
      lp_build_printf(gallivm, buf);

      /* Dump the execution mask.
       */
      if (bld->exec_mask.has_mask) {
         lp_build_print_value(gallivm, "    mask = ", bld->exec_mask.exec_mask);
      }
   }
}

static void
emit_store(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_full_instruction * inst,
   const struct tgsi_opcode_info * info,
   unsigned index,
   LLVMValueRef dst[4])

{
   enum tgsi_opcode_type dtype = tgsi_opcode_infer_dst_type(inst->Instruction.Opcode, index);

   unsigned writemask = inst->Dst[index].Register.WriteMask;
   while (writemask) {
      unsigned chan_index = u_bit_scan(&writemask);
      if (tgsi_type_is_64bit(dtype) && (chan_index == 1 || chan_index == 3))
          continue;
      emit_store_chan(bld_base, inst, index, chan_index, dst[chan_index]);
   }
}

static unsigned
tgsi_to_pipe_tex_target(enum tgsi_texture_type tgsi_target)
{
   switch (tgsi_target) {
   case TGSI_TEXTURE_BUFFER:
      return PIPE_BUFFER;
   case TGSI_TEXTURE_1D:
   case TGSI_TEXTURE_SHADOW1D:
      return PIPE_TEXTURE_1D;
   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_SHADOW2D:
   case TGSI_TEXTURE_2D_MSAA:
      return PIPE_TEXTURE_2D;
   case TGSI_TEXTURE_3D:
      return PIPE_TEXTURE_3D;
   case TGSI_TEXTURE_CUBE:
   case TGSI_TEXTURE_SHADOWCUBE:
      return PIPE_TEXTURE_CUBE;
   case TGSI_TEXTURE_RECT:
   case TGSI_TEXTURE_SHADOWRECT:
      return PIPE_TEXTURE_RECT;
   case TGSI_TEXTURE_1D_ARRAY:
   case TGSI_TEXTURE_SHADOW1D_ARRAY:
      return PIPE_TEXTURE_1D_ARRAY;
   case TGSI_TEXTURE_2D_ARRAY:
   case TGSI_TEXTURE_SHADOW2D_ARRAY:
   case TGSI_TEXTURE_2D_ARRAY_MSAA:
      return PIPE_TEXTURE_2D_ARRAY;
   case TGSI_TEXTURE_CUBE_ARRAY:
   case TGSI_TEXTURE_SHADOWCUBE_ARRAY:
      return PIPE_TEXTURE_CUBE_ARRAY;
   default:
      assert(0);
      return PIPE_BUFFER;
   }
}


static enum lp_sampler_lod_property
lp_build_lod_property(
   struct lp_build_tgsi_context *bld_base,
   const struct tgsi_full_instruction *inst,
   unsigned src_op)
{
   const struct tgsi_full_src_register *reg = &inst->Src[src_op];
   enum lp_sampler_lod_property lod_property;

   /*
    * Not much we can do here. We could try catching inputs declared
    * with constant interpolation but not sure it's worth it - since for
    * TEX opcodes as well as FETCH/LD the lod comes from same reg as
    * the coords, so it could only work for SAMPLE/TXQ/SVIEWINFO), just
    * like the constant/immediate recognition below.
    * What seems to be of more value would be to recognize temps holding
    * broadcasted scalars but no way we can do it.
    * Tried asking llvm but without any success (using LLVMIsConstant
    * even though this isn't exactly what we'd need), even as simple as
    * IMM[0] UINT32 (0,-1,0,0)
    * MOV TEMP[0] IMM[0].yyyy
    * SVIEWINFO TEMP[1], TEMP[0].xxxx, SVIEWINFO[0]
    * doesn't work.
    * This means there's ZERO chance this will ever catch a scalar lod
    * with traditional tex opcodes as well as texel fetches, since the lod
    * comes from the same reg as coords (except some test shaders using
    * constant coords maybe).
    * There's at least hope for sample opcodes as well as size queries.
    */
   if (inst->Instruction.Opcode == TGSI_OPCODE_TEX_LZ ||
       reg->Register.File == TGSI_FILE_CONSTANT ||
       reg->Register.File == TGSI_FILE_IMMEDIATE) {
      lod_property = LP_SAMPLER_LOD_SCALAR;
   }
   else if (bld_base->info->processor == PIPE_SHADER_FRAGMENT) {
      if (gallivm_perf & GALLIVM_PERF_NO_QUAD_LOD) {
         lod_property = LP_SAMPLER_LOD_PER_ELEMENT;
      }
      else {
         lod_property = LP_SAMPLER_LOD_PER_QUAD;
      }
   }
   else {
      /* never use scalar (per-quad) lod the results are just too wrong. */
      lod_property = LP_SAMPLER_LOD_PER_ELEMENT;
   }
   return lod_property;
}


/**
 * High-level instruction translators.
 */

static void
emit_tex( struct lp_build_tgsi_soa_context *bld,
          const struct tgsi_full_instruction *inst,
          enum lp_build_tex_modifier modifier,
          LLVMValueRef *texel,
          unsigned sampler_reg,
          enum lp_sampler_op_type sampler_op)
{
   unsigned unit = inst->Src[sampler_reg].Register.Index;
   LLVMValueRef oow = NULL;
   LLVMValueRef lod = NULL;
   LLVMValueRef coords[5];
   LLVMValueRef offsets[3] = { NULL };
   struct lp_derivatives derivs;
   struct lp_sampler_params params = { 0 };
   enum lp_sampler_lod_property lod_property = LP_SAMPLER_LOD_SCALAR;
   unsigned num_derivs, num_offsets, i;
   unsigned shadow_coord = 0;
   unsigned layer_coord = 0;
   unsigned sample_key = sampler_op << LP_SAMPLER_OP_TYPE_SHIFT;

   if (!bld->sampler) {
      _debug_printf("warning: found texture instruction but no sampler generator supplied\n");
      for (i = 0; i < 4; i++) {
         texel[i] = bld->bld_base.base.undef;
      }
      return;
   }

   switch (inst->Texture.Texture) {
   case TGSI_TEXTURE_1D_ARRAY:
      layer_coord = 1;
      FALLTHROUGH;
   case TGSI_TEXTURE_1D:
      num_offsets = 1;
      num_derivs = 1;
      break;
   case TGSI_TEXTURE_2D_ARRAY:
      layer_coord = 2;
      FALLTHROUGH;
   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_RECT:
      num_offsets = 2;
      num_derivs = 2;
      break;
   case TGSI_TEXTURE_SHADOW1D_ARRAY:
      layer_coord = 1;
      FALLTHROUGH;
   case TGSI_TEXTURE_SHADOW1D:
      shadow_coord = 2;
      num_offsets = 1;
      num_derivs = 1;
      break;
   case TGSI_TEXTURE_SHADOW2D_ARRAY:
      layer_coord = 2;
      shadow_coord = 3;
      num_offsets = 2;
      num_derivs = 2;
      break;
   case TGSI_TEXTURE_SHADOW2D:
   case TGSI_TEXTURE_SHADOWRECT:
      shadow_coord = 2;
      num_offsets = 2;
      num_derivs = 2;
      break;
   case TGSI_TEXTURE_CUBE:
      num_offsets = 2;
      num_derivs = 3;
      break;
   case TGSI_TEXTURE_3D:
      num_offsets = 3;
      num_derivs = 3;
      break;
   case TGSI_TEXTURE_SHADOWCUBE:
      shadow_coord = 3;
      num_offsets = 2;
      num_derivs = 3;
      break;
   case TGSI_TEXTURE_CUBE_ARRAY:
      num_offsets = 2;
      num_derivs = 3;
      layer_coord = 3;
      break;
   case TGSI_TEXTURE_SHADOWCUBE_ARRAY:
      num_offsets = 2;
      num_derivs = 3;
      layer_coord = 3;
      shadow_coord = 4; /* shadow coord special different reg */
      break;
   case TGSI_TEXTURE_2D_MSAA:
   case TGSI_TEXTURE_2D_ARRAY_MSAA:
   default:
      assert(0);
      return;
   }

   /* Note lod and especially projected are illegal in a LOT of cases */
   if (modifier == LP_BLD_TEX_MODIFIER_LOD_BIAS ||
       modifier == LP_BLD_TEX_MODIFIER_EXPLICIT_LOD) {
      if (inst->Instruction.Opcode == TGSI_OPCODE_TEX_LZ) {
         lod = bld->bld_base.base.zero;
      } else if (inst->Texture.Texture == TGSI_TEXTURE_SHADOWCUBE ||
                 inst->Texture.Texture == TGSI_TEXTURE_CUBE_ARRAY) {
         /* note that shadow cube array with bias/explicit lod does not exist */
         lod = lp_build_emit_fetch(&bld->bld_base, inst, 1, 0);
      }
      else {
         lod = lp_build_emit_fetch(&bld->bld_base, inst, 0, 3);
      }
      if (modifier == LP_BLD_TEX_MODIFIER_LOD_BIAS) {
         sample_key |= LP_SAMPLER_LOD_BIAS << LP_SAMPLER_LOD_CONTROL_SHIFT;
      }
      else if (modifier == LP_BLD_TEX_MODIFIER_EXPLICIT_LOD) {
         sample_key |= LP_SAMPLER_LOD_EXPLICIT << LP_SAMPLER_LOD_CONTROL_SHIFT;
      }
      lod_property = lp_build_lod_property(&bld->bld_base, inst, 0);
   }

   if (sampler_op == LP_SAMPLER_OP_GATHER) {
      uint32_t comp_val = inst->Src[sampler_reg].Register.SwizzleX;
      sample_key |= (comp_val << LP_SAMPLER_GATHER_COMP_SHIFT);
   }
   if (modifier == LP_BLD_TEX_MODIFIER_PROJECTED) {
      oow = lp_build_emit_fetch(&bld->bld_base, inst, 0, 3);
      oow = lp_build_rcp(&bld->bld_base.base, oow);
   }

   for (i = 0; i < num_derivs; i++) {
      coords[i] = lp_build_emit_fetch(&bld->bld_base, inst, 0, i);
      if (modifier == LP_BLD_TEX_MODIFIER_PROJECTED)
         coords[i] = lp_build_mul(&bld->bld_base.base, coords[i], oow);
   }
   for (i = num_derivs; i < 5; i++) {
      coords[i] = bld->bld_base.base.undef;
   }

   /* Layer coord always goes into 3rd slot, except for cube map arrays */
   if (layer_coord) {
      if (layer_coord == 3) {
         coords[3] = lp_build_emit_fetch(&bld->bld_base, inst, 0, layer_coord);
      }
      else {
         coords[2] = lp_build_emit_fetch(&bld->bld_base, inst, 0, layer_coord);
      }
      if (modifier == LP_BLD_TEX_MODIFIER_PROJECTED)
         coords[2] = lp_build_mul(&bld->bld_base.base, coords[2], oow);
   }
   /* Shadow coord occupies always 5th slot. */
   if (shadow_coord) {
      sample_key |= LP_SAMPLER_SHADOW;
      if (shadow_coord == 4) {
         coords[4] = lp_build_emit_fetch(&bld->bld_base, inst, 1, 0);
      }
      else {
         coords[4] = lp_build_emit_fetch(&bld->bld_base, inst, 0, shadow_coord);
      }
      if (modifier == LP_BLD_TEX_MODIFIER_PROJECTED)
         coords[4] = lp_build_mul(&bld->bld_base.base, coords[4], oow);
   }

   if (modifier == LP_BLD_TEX_MODIFIER_EXPLICIT_DERIV) {
      unsigned dim;
      sample_key |= LP_SAMPLER_LOD_DERIVATIVES << LP_SAMPLER_LOD_CONTROL_SHIFT;
      for (dim = 0; dim < num_derivs; ++dim) {
         derivs.ddx[dim] = lp_build_emit_fetch(&bld->bld_base, inst, 1, dim);
         derivs.ddy[dim] = lp_build_emit_fetch(&bld->bld_base, inst, 2, dim);
      }
      params.derivs = &derivs;
      /*
       * could also check all src regs if constant but I doubt such
       * cases exist in practice.
       */
      if (bld->bld_base.info->processor == PIPE_SHADER_FRAGMENT) {
         if (gallivm_perf & GALLIVM_PERF_NO_QUAD_LOD) {
            lod_property = LP_SAMPLER_LOD_PER_ELEMENT;
         }
         else {
            lod_property = LP_SAMPLER_LOD_PER_QUAD;
         }
      }
      else {
         lod_property = LP_SAMPLER_LOD_PER_ELEMENT;
      }
   }
   sample_key |= lod_property << LP_SAMPLER_LOD_PROPERTY_SHIFT;

   /* we don't handle the 4 offset version of tg4 */
   if (inst->Texture.NumOffsets == 1) {
      unsigned dim;
      sample_key |= LP_SAMPLER_OFFSETS;
      for (dim = 0; dim < num_offsets; dim++) {
         offsets[dim] = lp_build_emit_fetch_texoffset(&bld->bld_base, inst, 0, dim);
      }
   }

   params.type = bld->bld_base.base.type;
   params.sample_key = sample_key;
   params.texture_index = unit;
   params.sampler_index = unit;
   params.resources_type = bld->resources_type;
   params.resources_ptr = bld->resources_ptr;
   params.thread_data_type = bld->thread_data_type;
   params.thread_data_ptr = bld->thread_data_ptr;
   params.coords = coords;
   params.offsets = offsets;
   params.lod = lod;
   params.texel = texel;

   bld->sampler->emit_tex_sample(bld->sampler,
                                 bld->bld_base.base.gallivm,
                                 &params);
}

static void
emit_sample(struct lp_build_tgsi_soa_context *bld,
            const struct tgsi_full_instruction *inst,
            enum lp_build_tex_modifier modifier,
            bool compare,
            enum lp_sampler_op_type sample_type,
            LLVMValueRef *texel)
{
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   unsigned texture_unit, sampler_unit;
   LLVMValueRef lod = NULL;
   LLVMValueRef coords[5];
   LLVMValueRef offsets[3] = { NULL };
   struct lp_derivatives derivs;
   struct lp_sampler_params params = { 0 };
   enum lp_sampler_lod_property lod_property = LP_SAMPLER_LOD_SCALAR;

   unsigned num_offsets, num_derivs, i;
   unsigned layer_coord = 0;
   unsigned sample_key = sample_type << LP_SAMPLER_OP_TYPE_SHIFT;

   if (!bld->sampler) {
      _debug_printf("warning: found texture instruction but no sampler generator supplied\n");
      for (i = 0; i < 4; i++) {
         texel[i] = bld->bld_base.base.undef;
      }
      return;
   }

   /*
    * unlike old-style tex opcodes the texture/sampler indices
    * always come from src1 and src2 respectively.
    */
   texture_unit = inst->Src[1].Register.Index;
   sampler_unit = inst->Src[2].Register.Index;

   /*
    * Note inst->Texture.Texture will contain the number of offsets,
    * however the target information is NOT there and comes from the
    * declared sampler views instead.
    */
   switch (bld->sv[texture_unit].Resource) {
   case TGSI_TEXTURE_1D:
      num_offsets = 1;
      num_derivs = 1;
      break;
   case TGSI_TEXTURE_1D_ARRAY:
      layer_coord = 1;
      num_offsets = 1;
      num_derivs = 1;
      break;
   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_RECT:
      num_offsets = 2;
      num_derivs = 2;
      break;
   case TGSI_TEXTURE_2D_ARRAY:
      layer_coord = 2;
      num_offsets = 2;
      num_derivs = 2;
      break;
   case TGSI_TEXTURE_CUBE:
      num_offsets = 2;
      num_derivs = 3;
      break;
   case TGSI_TEXTURE_3D:
      num_offsets = 3;
      num_derivs = 3;
      break;
   case TGSI_TEXTURE_CUBE_ARRAY:
      layer_coord = 3;
      num_offsets = 2;
      num_derivs = 3;
      break;
   default:
      assert(0);
      return;
   }

   if (modifier == LP_BLD_TEX_MODIFIER_LOD_BIAS ||
       modifier == LP_BLD_TEX_MODIFIER_EXPLICIT_LOD) {
      lod = lp_build_emit_fetch(&bld->bld_base, inst, 3, 0);
      if (modifier == LP_BLD_TEX_MODIFIER_LOD_BIAS) {
         sample_key |= LP_SAMPLER_LOD_BIAS << LP_SAMPLER_LOD_CONTROL_SHIFT;
      }
      else if (modifier == LP_BLD_TEX_MODIFIER_EXPLICIT_LOD) {
         sample_key |= LP_SAMPLER_LOD_EXPLICIT << LP_SAMPLER_LOD_CONTROL_SHIFT;
      }
      lod_property = lp_build_lod_property(&bld->bld_base, inst, 0);
   }
   else if (modifier == LP_BLD_TEX_MODIFIER_LOD_ZERO) {
      /* XXX might be better to explicitly pass the level zero information */
      sample_key |= LP_SAMPLER_LOD_EXPLICIT << LP_SAMPLER_LOD_CONTROL_SHIFT;
      lod = lp_build_const_vec(gallivm, bld->bld_base.base.type, 0.0F);
   }

   for (i = 0; i < num_derivs; i++) {
      coords[i] = lp_build_emit_fetch(&bld->bld_base, inst, 0, i);
   }
   for (i = num_derivs; i < 5; i++) {
      coords[i] = bld->bld_base.base.undef;
   }

   /* Layer coord always goes into 3rd slot, except for cube map arrays */
   if (layer_coord) {
      if (layer_coord == 3)
         coords[3] = lp_build_emit_fetch(&bld->bld_base, inst, 0, layer_coord);
      else
         coords[2] = lp_build_emit_fetch(&bld->bld_base, inst, 0, layer_coord);
   }
   /* Shadow coord occupies always 5th slot. */
   if (compare) {
      sample_key |= LP_SAMPLER_SHADOW;
      coords[4] = lp_build_emit_fetch(&bld->bld_base, inst, 3, 0);
   }

   if (modifier == LP_BLD_TEX_MODIFIER_EXPLICIT_DERIV) {
      unsigned dim;
      sample_key |= LP_SAMPLER_LOD_DERIVATIVES << LP_SAMPLER_LOD_CONTROL_SHIFT;
      for (dim = 0; dim < num_derivs; ++dim) {
         derivs.ddx[dim] = lp_build_emit_fetch(&bld->bld_base, inst, 3, dim);
         derivs.ddy[dim] = lp_build_emit_fetch(&bld->bld_base, inst, 4, dim);
      }
      params.derivs = &derivs;
      /*
       * could also check all src regs if constant but I doubt such
       * cases exist in practice.
       */
      if (bld->bld_base.info->processor == PIPE_SHADER_FRAGMENT) {
         if (gallivm_perf & GALLIVM_PERF_NO_QUAD_LOD) {
            lod_property = LP_SAMPLER_LOD_PER_ELEMENT;
         }
         else {
            lod_property = LP_SAMPLER_LOD_PER_QUAD;
         }
      }
      else {
         lod_property = LP_SAMPLER_LOD_PER_ELEMENT;
      }
   }

   /* some advanced gather instructions (txgo) would require 4 offsets */
   if (inst->Texture.NumOffsets == 1) {
      unsigned dim;
      sample_key |= LP_SAMPLER_OFFSETS;
      for (dim = 0; dim < num_offsets; dim++) {
         offsets[dim] = lp_build_emit_fetch_texoffset(&bld->bld_base, inst, 0, dim);
      }
   }
   sample_key |= lod_property << LP_SAMPLER_LOD_PROPERTY_SHIFT;

   params.type = bld->bld_base.base.type;
   params.sample_key = sample_key;
   params.texture_index = texture_unit;
   params.sampler_index = sampler_unit;
   params.resources_type = bld->resources_type;
   params.resources_ptr = bld->resources_ptr;
   params.thread_data_type = bld->thread_data_type;
   params.thread_data_ptr = bld->thread_data_ptr;
   params.coords = coords;
   params.offsets = offsets;
   params.lod = lod;
   params.texel = texel;

   bld->sampler->emit_tex_sample(bld->sampler,
                                 bld->bld_base.base.gallivm,
                                 &params);

   if (inst->Src[1].Register.SwizzleX != PIPE_SWIZZLE_X ||
       inst->Src[1].Register.SwizzleY != PIPE_SWIZZLE_Y ||
       inst->Src[1].Register.SwizzleZ != PIPE_SWIZZLE_Z ||
       inst->Src[1].Register.SwizzleW != PIPE_SWIZZLE_W) {
      unsigned char swizzles[4];
      swizzles[0] = inst->Src[1].Register.SwizzleX;
      swizzles[1] = inst->Src[1].Register.SwizzleY;
      swizzles[2] = inst->Src[1].Register.SwizzleZ;
      swizzles[3] = inst->Src[1].Register.SwizzleW;

      lp_build_swizzle_soa_inplace(&bld->bld_base.base, texel, swizzles);
   }
}

static void
emit_fetch_texels( struct lp_build_tgsi_soa_context *bld,
                   const struct tgsi_full_instruction *inst,
                   LLVMValueRef *texel,
                   bool is_samplei)
{
   unsigned unit, target;
   LLVMValueRef coord_undef = LLVMGetUndef(bld->bld_base.base.int_vec_type);
   LLVMValueRef explicit_lod = NULL;
   LLVMValueRef coords[5];
   LLVMValueRef offsets[3] = { NULL };
   LLVMValueRef ms_index = NULL;
   struct lp_sampler_params params = { 0 };
   enum lp_sampler_lod_property lod_property = LP_SAMPLER_LOD_SCALAR;
   unsigned dims, i;
   unsigned layer_coord = 0;
   unsigned sample_key = LP_SAMPLER_OP_FETCH << LP_SAMPLER_OP_TYPE_SHIFT;

   if (!bld->sampler) {
      _debug_printf("warning: found texture instruction but no sampler generator supplied\n");
      for (i = 0; i < 4; i++) {
         texel[i] = coord_undef;
      }
      return;
   }

   unit = inst->Src[1].Register.Index;

   if (is_samplei) {
      target = bld->sv[unit].Resource;
   }
   else {
      target = inst->Texture.Texture;
   }

   switch (target) {
   case TGSI_TEXTURE_1D:
   case TGSI_TEXTURE_BUFFER:
      dims = 1;
      break;
   case TGSI_TEXTURE_1D_ARRAY:
      layer_coord = 1;
      dims = 1;
      break;
   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_RECT:
   case TGSI_TEXTURE_2D_MSAA:
      dims = 2;
      break;
   case TGSI_TEXTURE_2D_ARRAY:
   case TGSI_TEXTURE_2D_ARRAY_MSAA:
      layer_coord = 2;
      dims = 2;
      break;
   case TGSI_TEXTURE_3D:
      dims = 3;
      break;
   default:
      assert(0);
      return;
   }

   /* always have lod except for buffers and msaa targets ? */
   if (target != TGSI_TEXTURE_BUFFER &&
       target != TGSI_TEXTURE_2D_MSAA &&
       target != TGSI_TEXTURE_2D_ARRAY_MSAA &&
       inst->Instruction.Opcode != TGSI_OPCODE_TXF_LZ) {
      sample_key |= LP_SAMPLER_LOD_EXPLICIT << LP_SAMPLER_LOD_CONTROL_SHIFT;
      explicit_lod = lp_build_emit_fetch(&bld->bld_base, inst, 0, 3);
      lod_property = lp_build_lod_property(&bld->bld_base, inst, 0);
   }

   if (target == TGSI_TEXTURE_2D_MSAA ||
       target == TGSI_TEXTURE_2D_ARRAY_MSAA) {
      sample_key |= LP_SAMPLER_FETCH_MS;
      ms_index = lp_build_emit_fetch(&bld->bld_base, inst, 0, 3);
   }

   /*
    * XXX: for real msaa support, the w component (or src2.x for sample_i_ms)
    * would be the sample index.
    */

   for (i = 0; i < dims; i++) {
      coords[i] = lp_build_emit_fetch(&bld->bld_base, inst, 0, i);
   }
   /* never use more than 3 coords here but emit_fetch_texel copies all 5 anyway */
   for (i = dims; i < 5; i++) {
      coords[i] = coord_undef;
   }
   if (layer_coord)
      coords[2] = lp_build_emit_fetch(&bld->bld_base, inst, 0, layer_coord);

   if (inst->Texture.NumOffsets == 1) {
      unsigned dim;
      sample_key |= LP_SAMPLER_OFFSETS;
      for (dim = 0; dim < dims; dim++) {
         offsets[dim] = lp_build_emit_fetch_texoffset(&bld->bld_base, inst, 0, dim);
      }
   }
   sample_key |= lod_property << LP_SAMPLER_LOD_PROPERTY_SHIFT;

   params.type = bld->bld_base.base.type;
   params.sample_key = sample_key;
   params.texture_index = unit;
   /*
    * sampler not actually used, set to 0 so it won't exceed PIPE_MAX_SAMPLERS
    * and trigger some assertions with d3d10 where the sampler view number
    * can exceed this.
    */
   params.sampler_index = 0;
   params.resources_type = bld->resources_type;
   params.resources_ptr = bld->resources_ptr;
   params.thread_data_type = bld->thread_data_type;
   params.thread_data_ptr = bld->thread_data_ptr;
   params.coords = coords;
   params.offsets = offsets;
   params.derivs = NULL;
   params.lod = explicit_lod;
   params.texel = texel;
   params.ms_index = ms_index;

   bld->sampler->emit_tex_sample(bld->sampler,
                                 bld->bld_base.base.gallivm,
                                 &params);

   if (is_samplei &&
       (inst->Src[1].Register.SwizzleX != PIPE_SWIZZLE_X ||
        inst->Src[1].Register.SwizzleY != PIPE_SWIZZLE_Y ||
        inst->Src[1].Register.SwizzleZ != PIPE_SWIZZLE_Z ||
        inst->Src[1].Register.SwizzleW != PIPE_SWIZZLE_W)) {
      unsigned char swizzles[4];
      swizzles[0] = inst->Src[1].Register.SwizzleX;
      swizzles[1] = inst->Src[1].Register.SwizzleY;
      swizzles[2] = inst->Src[1].Register.SwizzleZ;
      swizzles[3] = inst->Src[1].Register.SwizzleW;

      lp_build_swizzle_soa_inplace(&bld->bld_base.base, texel, swizzles);
   }
}

static void
emit_size_query( struct lp_build_tgsi_soa_context *bld,
                 const struct tgsi_full_instruction *inst,
                 LLVMValueRef *sizes_out,
                 bool is_sviewinfo)
{
   LLVMValueRef explicit_lod;
   enum lp_sampler_lod_property lod_property;
   unsigned has_lod;
   unsigned i;
   unsigned unit = inst->Src[1].Register.Index;
   enum tgsi_texture_type target;
   enum pipe_texture_target pipe_target;
   struct lp_sampler_size_query_params params = { 0 };

   if (is_sviewinfo) {
      target = bld->sv[unit].Resource;
   }
   else {
      target = inst->Texture.Texture;
   }
   switch (target) {
   case TGSI_TEXTURE_BUFFER:
   case TGSI_TEXTURE_RECT:
   case TGSI_TEXTURE_SHADOWRECT:
   case TGSI_TEXTURE_2D_MSAA:
   case TGSI_TEXTURE_2D_ARRAY_MSAA:
      has_lod = 0;
      break;
   default:
      has_lod = 1;
      break;
   }

   if (!bld->sampler) {
      _debug_printf("warning: found texture query instruction but no sampler generator supplied\n");
      for (i = 0; i < 4; i++)
         sizes_out[i] = bld->bld_base.int_bld.undef;
      return;
   }

   if (has_lod) {
      explicit_lod = lp_build_emit_fetch(&bld->bld_base, inst, 0, 0);
      lod_property = lp_build_lod_property(&bld->bld_base, inst, 0);
   }
   else {
      explicit_lod = NULL;
      lod_property = LP_SAMPLER_LOD_SCALAR;
   }


   pipe_target = tgsi_to_pipe_tex_target(target);

   params.int_type = bld->bld_base.int_bld.type;
   params.texture_unit = unit;
   params.texture_unit_offset = NULL;
   params.target = pipe_target;
   params.resources_type = bld->resources_type;
   params.resources_ptr = bld->resources_ptr;
   params.is_sviewinfo = true;
   params.lod_property = lod_property;
   params.explicit_lod = explicit_lod;
   params.sizes_out = sizes_out;
   params.samples_only = false;

   bld->sampler->emit_size_query(bld->sampler,
                                 bld->bld_base.base.gallivm,
                                 &params);
}

static bool
near_end_of_shader(struct lp_build_tgsi_soa_context *bld,
                   int pc)
{
   unsigned i;

   for (i = 0; i < 5; i++) {
      enum tgsi_opcode opcode;

      if (pc + i >= bld->bld_base.info->num_instructions)
         return true;

      opcode = bld->bld_base.instructions[pc + i].Instruction.Opcode;

      if (opcode == TGSI_OPCODE_END)
         return true;

      if (opcode == TGSI_OPCODE_TEX ||
         opcode == TGSI_OPCODE_TXP ||
         opcode == TGSI_OPCODE_TXD ||
         opcode == TGSI_OPCODE_TXB ||
         opcode == TGSI_OPCODE_TXL ||
         opcode == TGSI_OPCODE_TXF ||
         opcode == TGSI_OPCODE_TXQ ||
         opcode == TGSI_OPCODE_TEX2 ||
         opcode == TGSI_OPCODE_TXB2 ||
         opcode == TGSI_OPCODE_TXL2 ||
         opcode == TGSI_OPCODE_SAMPLE ||
         opcode == TGSI_OPCODE_SAMPLE_B ||
         opcode == TGSI_OPCODE_SAMPLE_C ||
         opcode == TGSI_OPCODE_SAMPLE_C_LZ ||
         opcode == TGSI_OPCODE_SAMPLE_D ||
         opcode == TGSI_OPCODE_SAMPLE_I ||
         opcode == TGSI_OPCODE_SAMPLE_I_MS ||
         opcode == TGSI_OPCODE_SAMPLE_L ||
         opcode == TGSI_OPCODE_SVIEWINFO ||
         opcode == TGSI_OPCODE_CAL ||
         opcode == TGSI_OPCODE_IF ||
         opcode == TGSI_OPCODE_UIF ||
         opcode == TGSI_OPCODE_BGNLOOP ||
         opcode == TGSI_OPCODE_SWITCH)
         return false;
   }

   return true;
}



/**
 * Kill fragment if any of the src register values are negative.
 */
static void
emit_kill_if(
   struct lp_build_tgsi_soa_context *bld,
   const struct tgsi_full_instruction *inst,
   int pc)
{
   LLVMBuilderRef builder = bld->bld_base.base.gallivm->builder;
   const struct tgsi_full_src_register *reg = &inst->Src[0];
   LLVMValueRef terms[TGSI_NUM_CHANNELS];
   LLVMValueRef mask;
   unsigned chan_index;

   memset(&terms, 0, sizeof terms);

   TGSI_FOR_EACH_CHANNEL( chan_index ) {
      unsigned swizzle;

      /* Unswizzle channel */
      swizzle = tgsi_util_get_full_src_register_swizzle( reg, chan_index );

      /* Check if the component has not been already tested. */
      assert(swizzle < TGSI_NUM_CHANNELS);
      if( !terms[swizzle] )
         /* TODO: change the comparison operator instead of setting the sign */
         terms[swizzle] =  lp_build_emit_fetch(&bld->bld_base, inst, 0, chan_index );
   }

   mask = NULL;
   TGSI_FOR_EACH_CHANNEL( chan_index ) {
      if(terms[chan_index]) {
         LLVMValueRef chan_mask;

         /*
          * If term < 0 then mask = 0 else mask = ~0.
          */
         chan_mask = lp_build_cmp(&bld->bld_base.base, PIPE_FUNC_GEQUAL, terms[chan_index], bld->bld_base.base.zero);

         if(mask)
            mask = LLVMBuildAnd(builder, mask, chan_mask, "");
         else
            mask = chan_mask;
      }
   }

   if (bld->exec_mask.has_mask) {
      LLVMValueRef invmask;
      invmask = LLVMBuildNot(builder, bld->exec_mask.exec_mask, "kilp");
      mask = LLVMBuildOr(builder, mask, invmask, "");
   }

   lp_build_mask_update(bld->mask, mask);
   if (!near_end_of_shader(bld, pc))
      lp_build_mask_check(bld->mask);
}


/**
 * Unconditional fragment kill.
 * The only predication is the execution mask which will apply if
 * we're inside a loop or conditional.
 */
static void
emit_kill(struct lp_build_tgsi_soa_context *bld,
          int pc)
{
   LLVMBuilderRef builder = bld->bld_base.base.gallivm->builder;
   LLVMValueRef mask;

   /* For those channels which are "alive", disable fragment shader
    * execution.
    */
   if (bld->exec_mask.has_mask) {
      mask = LLVMBuildNot(builder, bld->exec_mask.exec_mask, "kilp");
   }
   else {
      LLVMValueRef zero = LLVMConstNull(bld->bld_base.base.int_vec_type);
      mask = zero;
   }

   lp_build_mask_update(bld->mask, mask);

   if (!near_end_of_shader(bld, pc))
      lp_build_mask_check(bld->mask);
}


/**
 * Emit code which will dump the value of all the temporary registers
 * to stdout.
 */
static void
emit_dump_file(struct lp_build_tgsi_soa_context *bld,
               unsigned file)
{
   const struct tgsi_shader_info *info = bld->bld_base.info;
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMValueRef reg_ptr;
   int index;
   int max_index = info->file_max[file];

   /*
    * Some register files, particularly constants, can be very large,
    * and dumping everything could make this unusably slow.
    */
   max_index = MIN2(max_index, 32);

   for (index = 0; index <= max_index; index++) {
      LLVMValueRef res;
      unsigned mask;
      int chan;

      if (index < 8 * sizeof(unsigned) &&
          (info->file_mask[file] & (1u << index)) == 0)  {
         /* This was not declared.*/
         continue;
      }

      if (file == TGSI_FILE_INPUT) {
         mask = info->input_usage_mask[index];
      } else {
         mask = TGSI_WRITEMASK_XYZW;
      }

      for (chan = 0; chan < 4; chan++) {
         if ((mask & (1 << chan)) == 0) {
            /* This channel is not used.*/
            continue;
         }

         if (file == TGSI_FILE_CONSTANT) {
            struct tgsi_full_src_register reg;
            memset(&reg, 0, sizeof reg);
            reg.Register.File = file;
            reg.Register.Index = index;
            reg.Register.SwizzleX = 0;
            reg.Register.SwizzleY = 1;
            reg.Register.SwizzleZ = 2;
            reg.Register.SwizzleW = 3;

            res = bld->bld_base.emit_fetch_funcs[file](&bld->bld_base, &reg, TGSI_TYPE_FLOAT, chan);
            if (!res) {
               continue;
            }
         } else if (file == TGSI_FILE_INPUT) {
            res = bld->inputs[index][chan];
            if (!res) {
               continue;
            }
         } else if (file == TGSI_FILE_TEMPORARY) {
            reg_ptr = lp_get_temp_ptr_soa(bld, index, chan);
            assert(reg_ptr);
            res = LLVMBuildLoad2(builder, bld->bld_base.base.vec_type, reg_ptr, "");
         } else if (file == TGSI_FILE_OUTPUT) {
            reg_ptr = lp_get_output_ptr(bld, index, chan);
            assert(reg_ptr);
            res = LLVMBuildLoad2(builder, bld->bld_base.base.vec_type, reg_ptr, "");
         } else {
            assert(0);
            continue;
         }

         emit_dump_reg(gallivm, file, index, chan, res);
      }
   }
}



void
lp_emit_declaration_soa(
   struct lp_build_tgsi_context *bld_base,
   const struct tgsi_full_declaration *decl)
{
   struct lp_build_tgsi_soa_context *bld = lp_soa_context(bld_base);
   struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
   LLVMTypeRef vec_type = bld->bld_base.base.vec_type;
   const unsigned first = decl->Range.First;
   const unsigned last = decl->Range.Last;
   unsigned idx, i;

   assert(last <= bld->bld_base.info->file_max[decl->Declaration.File]);

   switch (decl->Declaration.File) {
   case TGSI_FILE_TEMPORARY:
      if (!(bld->indirect_files & (1 << TGSI_FILE_TEMPORARY))) {
         assert(last < LP_MAX_INLINED_TEMPS);
         for (idx = first; idx <= last; ++idx) {
            for (i = 0; i < TGSI_NUM_CHANNELS; i++)
               bld->temps[idx][i] = lp_build_alloca(gallivm, vec_type, "temp");
         }
      }
      break;

   case TGSI_FILE_OUTPUT:
      if (!(bld->indirect_files & (1 << TGSI_FILE_OUTPUT))) {
         for (idx = first; idx <= last; ++idx) {
            for (i = 0; i < TGSI_NUM_CHANNELS; i++)
               bld->outputs[idx][i] = lp_build_alloca(gallivm,
                                                      vec_type, "output");
         }
      }
      break;

   case TGSI_FILE_ADDRESS:
      /* ADDR registers are only allocated with an integer LLVM IR type,
       * as they are guaranteed to always have integers.
       * XXX: Not sure if this exception is worthwhile (or the whole idea of
       * an ADDR register for that matter).
       */
      assert(last < LP_MAX_TGSI_ADDRS);
      for (idx = first; idx <= last; ++idx) {
         assert(idx < LP_MAX_TGSI_ADDRS);
         for (i = 0; i < TGSI_NUM_CHANNELS; i++)
            bld->addr[idx][i] = lp_build_alloca(gallivm, bld_base->base.int_vec_type, "addr");
      }
      break;

   case TGSI_FILE_SAMPLER_VIEW:
      /*
       * The target stored here MUST match whatever there actually
       * is in the set sampler views (what about return type?).
       */
      assert(last < PIPE_MAX_SHADER_SAMPLER_VIEWS);
      for (idx = first; idx <= last; ++idx) {
         bld->sv[idx] = decl->SamplerView;
      }
      break;

   case TGSI_FILE_CONSTANT:
   {
      /*
       * We could trivially fetch the per-buffer pointer when fetching the
       * constant, relying on llvm to figure out it's always the same pointer
       * anyway. However, doing so results in a huge (more than factor of 10)
       * slowdown in llvm compilation times for some (but not all) shaders
       * (more specifically, the IR optimization spends way more time in
       * DominatorTree::dominates). At least with llvm versions 3.1, 3.3.
       */
      unsigned idx2D = decl->Dim.Index2D;
      LLVMValueRef index2D = lp_build_const_int32(gallivm, idx2D);
      assert(idx2D < LP_MAX_TGSI_CONST_BUFFERS);
      bld->consts[idx2D] = lp_llvm_buffer_base(gallivm, bld->consts_ptr,
                                               index2D, LP_MAX_TGSI_CONST_BUFFERS);
      bld->consts[idx2D] = LLVMBuildBitCast(gallivm->builder, bld->consts[idx2D], LLVMPointerType(LLVMFloatTypeInContext(gallivm->context), 0), "");
      bld->consts_sizes[idx2D] = lp_llvm_buffer_num_elements(gallivm, bld->consts_ptr,
                                                             index2D, LP_MAX_TGSI_CONST_BUFFERS);
   }
   break;
   case TGSI_FILE_BUFFER:
   {
      unsigned idx = decl->Range.First;
      LLVMValueRef index = lp_build_const_int32(gallivm, idx);
      assert(idx < LP_MAX_TGSI_SHADER_BUFFERS);
      bld->ssbos[idx] =
         lp_llvm_buffer_base(gallivm, bld->ssbo_ptr,
                             index, LP_MAX_TGSI_SHADER_BUFFERS);
      bld->ssbo_sizes[idx] =
         lp_llvm_buffer_num_elements(gallivm, bld->ssbo_ptr,
                             index, LP_MAX_TGSI_SHADER_BUFFERS);

   }
   break;
   case TGSI_FILE_MEMORY:
      break;
   default:
      /* don't need to declare other vars */
      break;
   }
}


void lp_emit_immediate_soa(
   struct lp_build_tgsi_context *bld_base,
   const struct tgsi_full_immediate *imm)
{
   struct lp_build_tgsi_soa_context *bld = lp_soa_context(bld_base);
   struct gallivm_state * gallivm = bld_base->base.gallivm;
   LLVMValueRef imms[4];
   unsigned i;
   const unsigned size = imm->Immediate.NrTokens - 1;
   assert(size <= 4);
   switch (imm->Immediate.DataType) {
   case TGSI_IMM_FLOAT32:
      for( i = 0; i < size; ++i )
         imms[i] =
               lp_build_const_vec(gallivm, bld_base->base.type, imm->u[i].Float);

      break;
   case TGSI_IMM_FLOAT64:
   case TGSI_IMM_UINT64:
   case TGSI_IMM_INT64:
   case TGSI_IMM_UINT32:
      for( i = 0; i < size; ++i ) {
         LLVMValueRef tmp = lp_build_const_vec(gallivm, bld_base->uint_bld.type, imm->u[i].Uint);
         imms[i] = LLVMConstBitCast(tmp, bld_base->base.vec_type);
      }

      break;
   case TGSI_IMM_INT32:
      for( i = 0; i < size; ++i ) {
         LLVMValueRef tmp = lp_build_const_vec(gallivm, bld_base->int_bld.type, imm->u[i].Int);
         imms[i] = LLVMConstBitCast(tmp, bld_base->base.vec_type);
      }

      break;
   }
   for( i = size; i < 4; ++i )
      imms[i] = bld_base->base.undef;

   if (bld->use_immediates_array) {
      unsigned index = bld->num_immediates;
      struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
      LLVMBuilderRef builder = gallivm->builder;
      LLVMValueRef gep[2];
      gep[0] = lp_build_const_int32(gallivm, 0);

      assert(bld->indirect_files & (1 << TGSI_FILE_IMMEDIATE));
      for (i = 0; i < 4; ++i ) {
         gep[1] = lp_build_const_int32(gallivm, index * 4 + i);
         LLVMValueRef imm_ptr = LLVMBuildGEP2(builder,
                                              bld->bld_base.base.vec_type,
                                              bld->imms_array, gep, 2, "");
         LLVMBuildStore(builder, imms[i], imm_ptr);
      }
   } else {
      /* simply copy the immediate values into the next immediates[] slot */
      unsigned i;
      assert(imm->Immediate.NrTokens - 1 <= 4);
      assert(bld->num_immediates < LP_MAX_INLINED_IMMEDIATES);

      for(i = 0; i < 4; ++i )
         bld->immediates[bld->num_immediates][i] = imms[i];

      if (bld->indirect_files & (1 << TGSI_FILE_IMMEDIATE)) {
         unsigned index = bld->num_immediates;
         struct gallivm_state *gallivm = bld->bld_base.base.gallivm;
         LLVMBuilderRef builder = gallivm->builder;
         LLVMValueRef gep[2];
         gep[0] = lp_build_const_int32(gallivm, 0);
         for (i = 0; i < 4; ++i ) {
            gep[1] = lp_build_const_int32(gallivm, index * 4 + i);
            LLVMValueRef imm_ptr = LLVMBuildGEP2(builder,
                                                 bld->bld_base.base.vec_type,
                                                 bld->imms_array, gep, 2, "");
            LLVMBuildStore(builder,
                           bld->immediates[index][i],
                           imm_ptr);
         }
      }
   }

   bld->num_immediates++;
}

static void
ddx_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_fetch_deriv(bld, emit_data->args[0], NULL,
                    &emit_data->output[emit_data->chan], NULL);
}

static void
ddy_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_fetch_deriv(bld, emit_data->args[0], NULL, NULL,
                    &emit_data->output[emit_data->chan]);
}

static void
kill_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_kill(bld, bld_base->pc - 1);
}

static void
kill_if_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_kill_if(bld, emit_data->inst, bld_base->pc - 1);
}

static void
tex_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_tex(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_NONE,
            emit_data->output, 1, LP_SAMPLER_OP_TEXTURE);
}

static void
tex2_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_tex(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_NONE,
            emit_data->output, 2, LP_SAMPLER_OP_TEXTURE);
}

static void
txb_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_tex(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_LOD_BIAS,
            emit_data->output, 1, LP_SAMPLER_OP_TEXTURE);
}

static void
txb2_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_tex(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_LOD_BIAS,
            emit_data->output, 2, LP_SAMPLER_OP_TEXTURE);
}

static void
txd_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_tex(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_EXPLICIT_DERIV,
            emit_data->output, 3, LP_SAMPLER_OP_TEXTURE);
}

static void
txl_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_tex(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_EXPLICIT_LOD,
            emit_data->output, 1, LP_SAMPLER_OP_TEXTURE);
}

static void
txl2_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_tex(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_EXPLICIT_LOD,
            emit_data->output, 2, LP_SAMPLER_OP_TEXTURE);
}

static void
txp_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_tex(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_PROJECTED,
            emit_data->output, 1, LP_SAMPLER_OP_TEXTURE);
}

static void
tg4_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_tex(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_NONE,
            emit_data->output, 2, LP_SAMPLER_OP_GATHER);
}

static void
lodq_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_tex(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_NONE,
            emit_data->output, 1, LP_SAMPLER_OP_LODQ);
}

static void
txq_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_size_query(bld, emit_data->inst, emit_data->output, false);
}

static void
txf_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_fetch_texels(bld, emit_data->inst, emit_data->output, false);
}

static void
sample_i_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_fetch_texels(bld, emit_data->inst, emit_data->output, true);
}

static void
sample_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_sample(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_NONE,
               false, LP_SAMPLER_OP_TEXTURE, emit_data->output);
}

static void
sample_b_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_sample(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_LOD_BIAS,
               false, LP_SAMPLER_OP_TEXTURE, emit_data->output);
}

static void
sample_c_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_sample(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_NONE,
               true, LP_SAMPLER_OP_TEXTURE, emit_data->output);
}

static void
sample_c_lz_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_sample(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_LOD_ZERO,
               true, LP_SAMPLER_OP_TEXTURE, emit_data->output);
}

static void
sample_d_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_sample(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_EXPLICIT_DERIV,
               false, LP_SAMPLER_OP_TEXTURE, emit_data->output);
}

static void
sample_l_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_sample(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_EXPLICIT_LOD,
               false, LP_SAMPLER_OP_TEXTURE, emit_data->output);
}

static void
gather4_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_sample(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_NONE,
               false, LP_SAMPLER_OP_GATHER, emit_data->output);
}

static void
sviewinfo_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_size_query(bld, emit_data->inst, emit_data->output, true);
}

static void
lod_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   emit_sample(bld, emit_data->inst, LP_BLD_TEX_MODIFIER_NONE,
               false, LP_SAMPLER_OP_LODQ, emit_data->output);
}

static void
target_to_dims_layer(enum tgsi_texture_type target,
                     unsigned *dims,
                     unsigned *layer_coord)
{
   *layer_coord = 0;
   switch (target) {
   case TGSI_TEXTURE_1D:
   case TGSI_TEXTURE_BUFFER:
      *dims = 1;
      break;
   case TGSI_TEXTURE_1D_ARRAY:
      *layer_coord = 1;
      *dims = 1;
      break;
   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_RECT:
      *dims = 2;
      break;
   case TGSI_TEXTURE_2D_ARRAY:
      *layer_coord = 2;
      *dims = 2;
      break;
   case TGSI_TEXTURE_3D:
   case TGSI_TEXTURE_CUBE:
   case TGSI_TEXTURE_CUBE_ARRAY:
      *dims = 3;
      break;
   default:
      assert(0);
      *dims = 0;
      return;
   }
}

static void
img_load_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context *bld = lp_soa_context(bld_base);
   struct lp_img_params params = { 0 };
   LLVMValueRef coords[5];
   LLVMValueRef coord_undef = LLVMGetUndef(bld->bld_base.base.int_vec_type);
   unsigned dims;
   enum tgsi_texture_type target = emit_data->inst->Memory.Texture;
   unsigned layer_coord;

   target_to_dims_layer(target, &dims, &layer_coord);

   for (unsigned i = 0; i < dims; i++) {
      coords[i] = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 1, i);
   }
   for (unsigned i = dims; i < 5; i++) {
      coords[i] = coord_undef;
   }
   if (layer_coord)
      coords[2] = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 1, layer_coord);

   params.type = bld->bld_base.base.type;
   params.resources_type = bld->resources_type;
   params.resources_ptr = bld->resources_ptr;
   params.thread_data_type = bld->thread_data_type;
   params.thread_data_ptr = bld->thread_data_ptr;
   params.coords = coords;
   params.outdata = emit_data->output;
   params.target = tgsi_to_pipe_tex_target(target);
   params.image_index = emit_data->inst->Src[0].Register.Index;
   params.img_op = LP_IMG_LOAD;
   bld->image->emit_op(bld->image,
                         bld->bld_base.base.gallivm,
                         &params);
}

static void
load_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context *bld = lp_soa_context(bld_base);
   struct gallivm_state * gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = bld->bld_base.base.gallivm->builder;
   const struct tgsi_full_src_register *bufreg = &emit_data->inst->Src[0];
   unsigned buf = bufreg->Register.Index;
   assert(bufreg->Register.File == TGSI_FILE_BUFFER ||
          bufreg->Register.File == TGSI_FILE_IMAGE ||
          bufreg->Register.File == TGSI_FILE_MEMORY ||
          bufreg->Register.File == TGSI_FILE_CONSTBUF);
   bool is_shared = bufreg->Register.File == TGSI_FILE_MEMORY;
   struct lp_build_context *uint_bld = &bld_base->uint_bld;

   if (bufreg->Register.File == TGSI_FILE_IMAGE) {
      img_load_emit(action, bld_base, emit_data);
   } else if (bufreg->Register.File == TGSI_FILE_CONSTBUF) {
      LLVMValueRef consts_ptr = bld->consts[buf];
      LLVMValueRef num_consts = bld->consts_sizes[buf];

      LLVMValueRef indirect_index;
      LLVMValueRef overflow_mask;

      indirect_index = lp_build_emit_fetch(bld_base, emit_data->inst, 1, 0);
      indirect_index = lp_build_shr_imm(uint_bld, indirect_index, 4);

      /* All fetches are from the same constant buffer, so
       * we need to propagate the size to a vector to do a
       * vector comparison */
      num_consts = lp_build_broadcast_scalar(uint_bld, num_consts);

      /* Gather values from the constant buffer */
      unsigned chan_index;
      TGSI_FOR_EACH_DST0_ENABLED_CHANNEL(emit_data->inst, chan_index) {
         /* Construct a boolean vector telling us which channels
          * overflow the bound constant buffer */
         overflow_mask = lp_build_compare(gallivm, uint_bld->type, PIPE_FUNC_GEQUAL,
                                          indirect_index, num_consts);

         /* index_vec = indirect_index * 4 */
         LLVMValueRef index_vec = lp_build_shl_imm(uint_bld, indirect_index, 2);
         index_vec = lp_build_add(uint_bld, index_vec,
                                  lp_build_const_int_vec(gallivm, uint_bld->type, chan_index));

         emit_data->output[chan_index] = build_gather(bld_base, consts_ptr, index_vec, overflow_mask, NULL);
      }
   } else if (0) {
      /* for indirect support with ARB_gpu_shader5 */
   } else {
      LLVMValueRef index;
      LLVMValueRef scalar, scalar_ptr;
      unsigned chan_index;

      index = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 1, 0);
      index = lp_build_shr_imm(uint_bld, index, 2);

      scalar_ptr = is_shared ? bld->shared_ptr : bld->ssbos[buf];

      LLVMValueRef ssbo_limit = NULL;

      if (!is_shared) {
         ssbo_limit = LLVMBuildAShr(gallivm->builder, bld->ssbo_sizes[buf], lp_build_const_int32(gallivm, 2), "");
         ssbo_limit = lp_build_broadcast_scalar(uint_bld, ssbo_limit);
      }

      TGSI_FOR_EACH_DST0_ENABLED_CHANNEL(emit_data->inst, chan_index) {
         LLVMValueRef loop_index = lp_build_add(uint_bld, index, lp_build_const_int_vec(gallivm, uint_bld->type, chan_index));

         LLVMValueRef exec_mask = mask_vec(bld_base);
         if (!is_shared) {
            LLVMValueRef ssbo_oob_cmp = lp_build_cmp(uint_bld, PIPE_FUNC_LESS, loop_index, ssbo_limit);
            exec_mask = LLVMBuildAnd(builder, exec_mask, ssbo_oob_cmp, "");
         }

         LLVMValueRef result = lp_build_alloca(gallivm, uint_bld->vec_type, "");
         struct lp_build_loop_state loop_state;
         lp_build_loop_begin(&loop_state, gallivm, lp_build_const_int32(gallivm, 0));

         struct lp_build_if_state ifthen;
         LLVMValueRef cond, temp_res;

         loop_index = LLVMBuildExtractElement(gallivm->builder, loop_index,
                                              loop_state.counter, "");

         cond = LLVMBuildICmp(gallivm->builder, LLVMIntNE, exec_mask, uint_bld->zero, "");
         cond = LLVMBuildExtractElement(gallivm->builder, cond, loop_state.counter, "");

         lp_build_if(&ifthen, gallivm, cond);
         scalar = lp_build_pointer_get2(builder, uint_bld->elem_type, scalar_ptr, loop_index);

         temp_res = LLVMBuildLoad2(builder, uint_bld->vec_type, result, "");
         temp_res = LLVMBuildInsertElement(builder, temp_res, scalar, loop_state.counter, "");
         LLVMBuildStore(builder, temp_res, result);
         lp_build_else(&ifthen);
         temp_res = LLVMBuildLoad2(builder, uint_bld->vec_type, result, "");
         temp_res = LLVMBuildInsertElement(builder, temp_res, lp_build_const_int32(gallivm, 0), loop_state.counter, "");
         LLVMBuildStore(builder, temp_res, result);
         lp_build_endif(&ifthen);
         lp_build_loop_end_cond(&loop_state, lp_build_const_int32(gallivm, uint_bld->type.length),
                                NULL, LLVMIntUGE);
         emit_data->output[chan_index] = LLVMBuildLoad2(gallivm->builder, uint_bld->vec_type,
                                                        result, "");
      }
   }
}

static void
img_store_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context *bld = lp_soa_context(bld_base);
   struct lp_img_params params = { 0 };
   LLVMValueRef coords[5];
   LLVMValueRef coord_undef = LLVMGetUndef(bld->bld_base.base.int_vec_type);
   unsigned dims;
   enum tgsi_texture_type target = emit_data->inst->Memory.Texture;
   unsigned layer_coord;

   target_to_dims_layer(target, &dims, &layer_coord);
   for (unsigned i = 0; i < dims; i++) {
      coords[i] = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 0, i);
   }
   for (unsigned i = dims; i < 5; i++) {
      coords[i] = coord_undef;
   }
   if (layer_coord)
      coords[2] = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 0, layer_coord);

   params.type = bld->bld_base.base.type;
   params.resources_type = bld->resources_type;
   params.resources_ptr = bld->resources_ptr;
   params.thread_data_type = bld->thread_data_type;
   params.thread_data_ptr = bld->thread_data_ptr;
   params.coords = coords;
   params.outdata = NULL;
   params.exec_mask = mask_vec(bld_base);
   params.target = tgsi_to_pipe_tex_target(target);
   params.image_index = emit_data->inst->Dst[0].Register.Index;
   params.img_op = LP_IMG_STORE;
   for (unsigned i = 0; i < 4; i++)
      params.indata[i] = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 1, i);

   bld->image->emit_op(bld->image,
                       bld->bld_base.base.gallivm,
                       &params);
}

static void
store_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context *bld = lp_soa_context(bld_base);
   struct gallivm_state * gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = bld->bld_base.base.gallivm->builder;
   struct lp_build_context *uint_bld = &bld_base->uint_bld;
   const struct tgsi_full_dst_register *bufreg = &emit_data->inst->Dst[0];
   unsigned buf = bufreg->Register.Index;
   assert(bufreg->Register.File == TGSI_FILE_BUFFER || bufreg->Register.File == TGSI_FILE_IMAGE || bufreg->Register.File == TGSI_FILE_MEMORY);
   bool is_shared = bufreg->Register.File == TGSI_FILE_MEMORY;

   if (bufreg->Register.File == TGSI_FILE_IMAGE) {
      img_store_emit(action, bld_base, emit_data);
   } else if (0) {

   } else {
      LLVMValueRef index;  /* index into the const buffer */
      LLVMValueRef scalar_ptr;
      LLVMValueRef value;
      unsigned chan_index;

      index = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 0, 0);
      index = lp_build_shr_imm(uint_bld, index, 2);

      scalar_ptr = is_shared ? bld->shared_ptr : bld->ssbos[buf];

      LLVMValueRef ssbo_limit = NULL;

      if (!is_shared) {
         ssbo_limit = LLVMBuildAShr(gallivm->builder, bld->ssbo_sizes[buf], lp_build_const_int32(gallivm, 2), "");
         ssbo_limit = lp_build_broadcast_scalar(uint_bld, ssbo_limit);
      }

      TGSI_FOR_EACH_DST0_ENABLED_CHANNEL(emit_data->inst, chan_index) {
         LLVMValueRef loop_index = lp_build_add(uint_bld, index, lp_build_const_int_vec(gallivm, uint_bld->type, chan_index));

         value = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 1, chan_index);

         LLVMValueRef exec_mask = mask_vec(bld_base);
         if (!is_shared) {
            LLVMValueRef ssbo_oob_cmp = lp_build_cmp(uint_bld, PIPE_FUNC_LESS, loop_index, ssbo_limit);
            exec_mask = LLVMBuildAnd(builder, exec_mask, ssbo_oob_cmp, "");
         }

         struct lp_build_loop_state loop_state;
         lp_build_loop_begin(&loop_state, gallivm, lp_build_const_int32(gallivm, 0));

         LLVMValueRef value_ptr = LLVMBuildExtractElement(gallivm->builder, value,
                                                          loop_state.counter, "");
         value_ptr = LLVMBuildBitCast(gallivm->builder, value_ptr, uint_bld->elem_type, "");

         struct lp_build_if_state ifthen;
         LLVMValueRef cond;

         loop_index = LLVMBuildExtractElement(gallivm->builder, loop_index,
                                              loop_state.counter, "");

         cond = LLVMBuildICmp(gallivm->builder, LLVMIntNE, exec_mask, uint_bld->zero, "");
         cond = LLVMBuildExtractElement(gallivm->builder, cond, loop_state.counter, "");
         lp_build_if(&ifthen, gallivm, cond);

         lp_build_pointer_set(builder, scalar_ptr, loop_index, value_ptr);

         lp_build_endif(&ifthen);
         lp_build_loop_end_cond(&loop_state, lp_build_const_int32(gallivm, uint_bld->type.length),
                                NULL, LLVMIntUGE);
      }
   }
}

static void
resq_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context *bld = lp_soa_context(bld_base);
   struct lp_build_context *uint_bld = &bld_base->uint_bld;
   const struct tgsi_full_src_register *bufreg = &emit_data->inst->Src[0];

   unsigned buf = bufreg->Register.Index;
   assert(bufreg->Register.File == TGSI_FILE_BUFFER || bufreg->Register.File == TGSI_FILE_IMAGE);

   if (bufreg->Register.File == TGSI_FILE_IMAGE) {
      enum tgsi_texture_type target = emit_data->inst->Memory.Texture;
      struct lp_sampler_size_query_params params = { 0 };
      params.int_type = bld->bld_base.int_bld.type;
      params.texture_unit = buf;
      params.target = tgsi_to_pipe_tex_target(target);
      params.resources_type = bld->resources_type;
      params.resources_ptr = bld->resources_ptr;
      params.sizes_out = emit_data->output;

      bld->image->emit_size_query(bld->image,
                                  bld->bld_base.base.gallivm,
                                  &params);
   } else {
      LLVMValueRef num_ssbo = bld->ssbo_sizes[buf];

      emit_data->output[emit_data->chan] = lp_build_broadcast_scalar(uint_bld, num_ssbo);
   }
}

static void
img_atomic_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data,
   LLVMAtomicRMWBinOp op)
{
   struct lp_build_tgsi_soa_context *bld = lp_soa_context(bld_base);
   struct lp_img_params params = { 0 };
   LLVMValueRef coords[5];
   LLVMValueRef coord_undef = LLVMGetUndef(bld->bld_base.base.int_vec_type);
   unsigned dims;
   unsigned layer_coord;
   enum tgsi_texture_type target = emit_data->inst->Memory.Texture;

   target_to_dims_layer(target, &dims, &layer_coord);

   for (unsigned i = 0; i < dims; i++) {
      coords[i] = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 1, i);
   }
   for (unsigned i = dims; i < 5; i++) {
      coords[i] = coord_undef;
   }
   if (layer_coord)
      coords[2] = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 1, layer_coord);

   params.type = bld->bld_base.base.type;
   params.resources_type = bld->resources_type;
   params.resources_ptr = bld->resources_ptr;
   params.thread_data_ptr = bld->thread_data_ptr;
   params.exec_mask = mask_vec(bld_base);
   params.image_index = emit_data->inst->Src[0].Register.Index;
   params.coords = coords;
   params.target = tgsi_to_pipe_tex_target(target);
   params.op = op;
   params.outdata = emit_data->output;
   params.img_op = (emit_data->inst->Instruction.Opcode == TGSI_OPCODE_ATOMCAS) ? LP_IMG_ATOMIC_CAS : LP_IMG_ATOMIC;

   for (unsigned i = 0; i < 4; i++)
      params.indata[i] = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 2, i);
   if (emit_data->inst->Instruction.Opcode == TGSI_OPCODE_ATOMCAS) {
      for (unsigned i = 0; i < 4; i++)
         params.indata2[i] = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 3, i);
   }
   bld->image->emit_op(bld->image,
                       bld->bld_base.base.gallivm,
                       &params);
}

static void
atomic_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context *bld = lp_soa_context(bld_base);
   struct gallivm_state * gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_build_context *uint_bld = &bld_base->uint_bld;
   const struct tgsi_full_src_register *bufreg = &emit_data->inst->Src[0];

   assert(bufreg->Register.File == TGSI_FILE_BUFFER || bufreg->Register.File == TGSI_FILE_IMAGE || bufreg->Register.File == TGSI_FILE_MEMORY);
   unsigned buf = bufreg->Register.Index;
   bool is_shared = bufreg->Register.File == TGSI_FILE_MEMORY;

   LLVMAtomicRMWBinOp op = -1;
   switch (emit_data->inst->Instruction.Opcode) {
   case TGSI_OPCODE_ATOMUADD:
      op = LLVMAtomicRMWBinOpAdd;
      break;
   case TGSI_OPCODE_ATOMXCHG:
      op = LLVMAtomicRMWBinOpXchg;
      break;
   case TGSI_OPCODE_ATOMAND:
      op = LLVMAtomicRMWBinOpAnd;
      break;
   case TGSI_OPCODE_ATOMOR:
      op = LLVMAtomicRMWBinOpOr;
      break;
   case TGSI_OPCODE_ATOMXOR:
      op = LLVMAtomicRMWBinOpXor;
      break;
   case TGSI_OPCODE_ATOMUMIN:
      op = LLVMAtomicRMWBinOpUMin;
      break;
   case TGSI_OPCODE_ATOMUMAX:
      op = LLVMAtomicRMWBinOpUMax;
      break;
   case TGSI_OPCODE_ATOMIMIN:
      op = LLVMAtomicRMWBinOpMin;
      break;
   case TGSI_OPCODE_ATOMIMAX:
      op = LLVMAtomicRMWBinOpMax;
      break;
   case TGSI_OPCODE_ATOMCAS:
      break;
   default:
      assert(0);
      return;
   }

   if (bufreg->Register.File == TGSI_FILE_IMAGE) {
      img_atomic_emit(action, bld_base, emit_data, op);
   } else if (0) {
   } else {
      LLVMValueRef index;  /* index into the const buffer */
      LLVMValueRef scalar, scalar_ptr;
      LLVMValueRef value;

      index = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 1, 0);
      value = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 2, 0);

      index = lp_build_shr_imm(uint_bld, index, 2);

      if (!is_shared) {
         index = lp_build_add(uint_bld, index, lp_build_const_int_vec(gallivm, uint_bld->type, emit_data->chan));
         scalar_ptr = bld->ssbos[buf];
      } else
         scalar_ptr = bld->shared_ptr;

      LLVMValueRef atom_res = lp_build_alloca(gallivm,
                                              uint_bld->vec_type, "");

      LLVMValueRef ssbo_limit;
      if (!is_shared) {
         ssbo_limit = LLVMBuildAShr(gallivm->builder, bld->ssbo_sizes[buf], lp_build_const_int32(gallivm, 2), "");
         ssbo_limit = lp_build_broadcast_scalar(uint_bld, ssbo_limit);
      }

      LLVMValueRef exec_mask = mask_vec(bld_base);

      if (!is_shared) {
         LLVMValueRef ssbo_oob_cmp = lp_build_cmp(uint_bld, PIPE_FUNC_LESS, index, ssbo_limit);
         exec_mask = LLVMBuildAnd(builder, exec_mask, ssbo_oob_cmp, "");
      }

      struct lp_build_loop_state loop_state;
      lp_build_loop_begin(&loop_state, gallivm, lp_build_const_int32(gallivm, 0));

      LLVMValueRef value_ptr = LLVMBuildExtractElement(gallivm->builder, value,
                                                       loop_state.counter, "");
      value_ptr = LLVMBuildBitCast(gallivm->builder, value_ptr, uint_bld->elem_type, "");

      index = LLVMBuildExtractElement(gallivm->builder, index,
                                      loop_state.counter, "");

      scalar_ptr = LLVMBuildGEP2(builder, uint_bld->elem_type, scalar_ptr,
                                 &index, 1, "");

      struct lp_build_if_state ifthen;
      LLVMValueRef cond, temp_res;

      cond = LLVMBuildICmp(gallivm->builder, LLVMIntNE, exec_mask, uint_bld->zero, "");
      cond = LLVMBuildExtractElement(gallivm->builder, cond, loop_state.counter, "");
      lp_build_if(&ifthen, gallivm, cond);

      if (emit_data->inst->Instruction.Opcode == TGSI_OPCODE_ATOMCAS) {
         LLVMValueRef cas_src = lp_build_emit_fetch(&bld->bld_base, emit_data->inst, 3, 0);
         LLVMValueRef cas_src_ptr = LLVMBuildExtractElement(gallivm->builder, cas_src,
                                                            loop_state.counter, "");
         cas_src_ptr = LLVMBuildBitCast(gallivm->builder, cas_src_ptr, uint_bld->elem_type, "");
         scalar = LLVMBuildAtomicCmpXchg(builder, scalar_ptr, value_ptr,
                                         cas_src_ptr,
                                         LLVMAtomicOrderingSequentiallyConsistent,
                                         LLVMAtomicOrderingSequentiallyConsistent,
                                         false);
         scalar = LLVMBuildExtractValue(gallivm->builder, scalar, 0, "");
      } else {
         scalar = LLVMBuildAtomicRMW(builder, op,
                                     scalar_ptr, value_ptr,
                                     LLVMAtomicOrderingSequentiallyConsistent,
                                     false);
      }
      temp_res = LLVMBuildLoad2(builder, uint_bld->vec_type, atom_res, "");
      temp_res = LLVMBuildInsertElement(builder, temp_res, scalar, loop_state.counter, "");
      LLVMBuildStore(builder, temp_res, atom_res);
      lp_build_else(&ifthen);
      temp_res = LLVMBuildLoad2(builder, uint_bld->vec_type, atom_res, "");
      temp_res = LLVMBuildInsertElement(builder, temp_res, lp_build_const_int32(gallivm, 0), loop_state.counter, "");
      LLVMBuildStore(builder, temp_res, atom_res);
      lp_build_endif(&ifthen);

      lp_build_loop_end_cond(&loop_state, lp_build_const_int32(gallivm, uint_bld->type.length),
                             NULL, LLVMIntUGE);
      emit_data->output[emit_data->chan] = LLVMBuildLoad2(gallivm->builder, uint_bld->vec_type, atom_res, "");
   }
}

static void
barrier_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context *bld = lp_soa_context(bld_base);
   struct gallivm_state * gallivm = bld_base->base.gallivm;

   LLVMBasicBlockRef resume = lp_build_insert_new_block(gallivm, "resume");

   lp_build_coro_suspend_switch(gallivm, bld->coro, resume, false);
   LLVMPositionBuilderAtEnd(gallivm->builder, resume);
}

static void
membar_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMBuildFence(builder, LLVMAtomicOrderingSequentiallyConsistent, false, "");
}

static void
increment_vec_ptr_by_mask(struct lp_build_tgsi_context * bld_base,
                          LLVMValueRef ptr,
                          LLVMValueRef mask)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef current_vec = LLVMBuildLoad2(builder, bld_base->uint_bld.vec_type, ptr, "");

   current_vec = LLVMBuildSub(builder, current_vec, mask, "");

   LLVMBuildStore(builder, current_vec, ptr);
}

static void
clear_uint_vec_ptr_from_mask(struct lp_build_tgsi_context * bld_base,
                             LLVMValueRef ptr,
                             LLVMValueRef mask)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef current_vec = LLVMBuildLoad2(builder, bld_base->uint_bld.vec_type, ptr, "");

   current_vec = lp_build_select(&bld_base->uint_bld,
                                 mask,
                                 bld_base->uint_bld.zero,
                                 current_vec);

   LLVMBuildStore(builder, current_vec, ptr);
}

static LLVMValueRef
clamp_mask_to_max_output_vertices(struct lp_build_tgsi_soa_context * bld,
                                  LLVMValueRef current_mask_vec,
                                  LLVMValueRef total_emitted_vertices_vec)
{
   LLVMBuilderRef builder = bld->bld_base.base.gallivm->builder;
   struct lp_build_context *int_bld = &bld->bld_base.int_bld;
   LLVMValueRef max_mask = lp_build_cmp(int_bld, PIPE_FUNC_LESS,
                                        total_emitted_vertices_vec,
                                        bld->max_output_vertices_vec);

   return LLVMBuildAnd(builder, current_mask_vec, max_mask, "");
}

static void
emit_vertex(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   LLVMBuilderRef builder = bld->bld_base.base.gallivm->builder;

   if (bld->gs_iface->emit_vertex) {
      LLVMValueRef stream_id = emit_fetch_immediate(bld_base, &emit_data->inst->Src[0],
                                                    TGSI_TYPE_UNSIGNED,
                                                    emit_data->inst->Src[0].Register.SwizzleX);
      LLVMValueRef mask = mask_vec(bld_base);
      LLVMValueRef total_emitted_vertices_vec =
         LLVMBuildLoad2(builder, bld->bld_base.uint_bld.vec_type, bld->total_emitted_vertices_vec_ptr, "");

      mask = clamp_mask_to_max_output_vertices(bld, mask,
                                               total_emitted_vertices_vec);
      gather_outputs(bld);
      bld->gs_iface->emit_vertex(bld->gs_iface, &bld->bld_base.base,
                                 bld->outputs,
                                 total_emitted_vertices_vec,
                                 mask,
                                 stream_id);
      increment_vec_ptr_by_mask(bld_base, bld->emitted_vertices_vec_ptr,
                                mask);
      increment_vec_ptr_by_mask(bld_base, bld->total_emitted_vertices_vec_ptr,
                                mask);
#if DUMP_GS_EMITS
      lp_build_print_value(bld->bld_base.base.gallivm,
                           " +++ emit vertex masked ones = ",
                           mask);
      lp_build_print_value(bld->bld_base.base.gallivm,
                           " +++ emit vertex emitted = ",
                           total_emitted_vertices_vec);
#endif
   }
}


static void
end_primitive_masked(struct lp_build_tgsi_context * bld_base,
                     LLVMValueRef mask)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   LLVMBuilderRef builder = bld->bld_base.base.gallivm->builder;

   if (bld->gs_iface->end_primitive) {
      struct lp_build_context *uint_bld = &bld_base->uint_bld;
      LLVMValueRef emitted_vertices_vec =
         LLVMBuildLoad2(builder, bld->bld_base.uint_bld.vec_type,
                        bld->emitted_vertices_vec_ptr, "");
      LLVMValueRef emitted_prims_vec =
         LLVMBuildLoad2(builder, bld->bld_base.uint_bld.vec_type,
                        bld->emitted_prims_vec_ptr, "");
      LLVMValueRef total_emitted_vertices_vec =
         LLVMBuildLoad2(builder, bld->bld_base.uint_bld.vec_type,
                        bld->total_emitted_vertices_vec_ptr, "");
      LLVMValueRef emitted_mask = lp_build_cmp(uint_bld, PIPE_FUNC_NOTEQUAL,
                                               emitted_vertices_vec,
                                               uint_bld->zero);
      /* We need to combine the current execution mask with the mask
         telling us which, if any, execution slots actually have
         unemitted primitives, this way we make sure that end_primitives
         executes only on the paths that have unflushed vertices */
      mask = LLVMBuildAnd(builder, mask, emitted_mask, "");

      bld->gs_iface->end_primitive(bld->gs_iface, &bld->bld_base.base,
                                   total_emitted_vertices_vec,
                                   emitted_vertices_vec,
                                   emitted_prims_vec,
                                   mask_vec(bld_base), 0);

#if DUMP_GS_EMITS
      lp_build_print_value(bld->bld_base.base.gallivm,
                           " +++ end prim masked ones = ",
                           mask);
      lp_build_print_value(bld->bld_base.base.gallivm,
                           " +++ end prim emitted verts1 = ",
                           emitted_vertices_vec);
      lp_build_print_value(bld->bld_base.base.gallivm,
                           " +++ end prim emitted prims1 = ",
                           LLVMBuildLoad2(builder, bld->bld_base.uint_bld.vec_type,
                                          bld->emitted_prims_vec_ptr, ""));
#endif
      increment_vec_ptr_by_mask(bld_base, bld->emitted_prims_vec_ptr,
                                mask);
      clear_uint_vec_ptr_from_mask(bld_base, bld->emitted_vertices_vec_ptr,
                                   mask);
#if DUMP_GS_EMITS
      lp_build_print_value(bld->bld_base.base.gallivm,
                           " +++ end prim emitted verts2 = ",
                           LLVMBuildLoad2(builder, bld->bld_base.uint_bld.vec_type,
                                          bld->emitted_vertices_vec_ptr, ""));
#endif
   }

}

static void
end_primitive(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   if (bld->gs_iface->end_primitive) {
      LLVMValueRef mask = mask_vec(bld_base);
      end_primitive_masked(bld_base, mask);
   }
}

static void
barrier_emit_tcs(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   if (bld->tcs_iface->emit_barrier) {
      bld->tcs_iface->emit_barrier((struct lp_build_context*)bld_base);
   }
}


static void
cal_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_mask_call(&bld->exec_mask, emit_data->inst->Label.Label,
                     &bld_base->pc);
}

static void
ret_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_mask_ret(&bld->exec_mask, &bld_base->pc);
}

static void
brk_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_tgsi_break(&bld->exec_mask, bld_base);
}

static void
if_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp;
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   tmp = lp_build_cmp(&bld_base->base, PIPE_FUNC_NOTEQUAL,
                      emit_data->args[0], bld->bld_base.base.zero);
   lp_exec_mask_cond_push(&bld->exec_mask, tmp);
}

static void
uif_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp;
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct lp_build_context *uint_bld = &bld_base->uint_bld;

   tmp = lp_build_cmp(uint_bld, PIPE_FUNC_NOTEQUAL,
                      emit_data->args[0], uint_bld->zero);
   lp_exec_mask_cond_push(&bld->exec_mask, tmp);
}

static void
case_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_case(&bld->exec_mask, emit_data->args[0]);
}

static void
default_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_default(&bld->exec_mask, bld_base);
}

static void
switch_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_switch(&bld->exec_mask, emit_data->args[0]);
}

static void
endswitch_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_endswitch(&bld->exec_mask, bld_base);
}

static void
bgnloop_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_bgnloop(&bld->exec_mask, true);
}

static void
bgnsub_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_mask_bgnsub(&bld->exec_mask);
}

static void
else_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_mask_cond_invert(&bld->exec_mask);
}

static void
endif_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_mask_cond_pop(&bld->exec_mask);
}

static void
endloop_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_endloop(bld_base->base.gallivm, &bld->exec_mask, bld->mask);
}

static void
endsub_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_mask_endsub(&bld->exec_mask, &bld_base->pc);
}

static void
cont_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   lp_exec_continue(&bld->exec_mask);
}

static void emit_prologue(struct lp_build_tgsi_context * bld_base)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   struct gallivm_state * gallivm = bld_base->base.gallivm;

   if (bld->indirect_files & (1 << TGSI_FILE_TEMPORARY)) {
      unsigned array_size = bld_base->info->file_max[TGSI_FILE_TEMPORARY] * 4 + 4;
      bld->temps_array_type = LLVMArrayType(bld_base->base.vec_type, array_size);
      bld->temps_array = lp_build_alloca_undef(gallivm,
                                               LLVMArrayType(bld_base->base.vec_type, array_size),
                                               "temp_array");
   }

   if (bld->indirect_files & (1 << TGSI_FILE_OUTPUT)) {
      LLVMValueRef array_size =
         lp_build_const_int32(gallivm,
                            bld_base->info->file_max[TGSI_FILE_OUTPUT] * 4 + 4);
      bld->outputs_array_type = bld_base->base.vec_type;
      bld->outputs_array = lp_build_array_alloca(gallivm,
                                                bld_base->base.vec_type, array_size,
                                                "output_array");
   }

   if (bld->indirect_files & (1 << TGSI_FILE_IMMEDIATE)) {
      unsigned array_size = bld_base->info->file_max[TGSI_FILE_IMMEDIATE] * 4 + 4;
      bld->imms_array = lp_build_alloca_undef(gallivm,
                                              LLVMArrayType(bld_base->base.vec_type, array_size),
                                              "imms_array");
   }

   /* If we have indirect addressing in inputs we need to copy them into
    * our alloca array to be able to iterate over them */
   if (bld->indirect_files & (1 << TGSI_FILE_INPUT) &&
       !bld->gs_iface && !bld->tes_iface && !bld->tcs_iface) {
      unsigned index, chan;
      LLVMTypeRef vec_type = bld_base->base.vec_type;
      LLVMValueRef array_size = lp_build_const_int32(gallivm,
            bld_base->info->file_max[TGSI_FILE_INPUT]*4 + 4);
      bld->inputs_array = lp_build_array_alloca(gallivm,
                                               vec_type, array_size,
                                               "input_array");

      assert(bld_base->info->num_inputs
                        <= bld_base->info->file_max[TGSI_FILE_INPUT] + 1);

      for (index = 0; index < bld_base->info->num_inputs; ++index) {
         for (chan = 0; chan < TGSI_NUM_CHANNELS; ++chan) {
            LLVMValueRef lindex =
               lp_build_const_int32(gallivm, index * 4 + chan);
            LLVMValueRef input_ptr =
               LLVMBuildGEP2(gallivm->builder,
                             bld->bld_base.base.vec_type,
                             bld->inputs_array,
                             &lindex, 1, "");
            LLVMValueRef value = bld->inputs[index][chan];
            if (value)
               LLVMBuildStore(gallivm->builder, value, input_ptr);
         }
      }
   }

   if (bld->gs_iface) {
      struct lp_build_context *uint_bld = &bld->bld_base.uint_bld;
      bld->emitted_prims_vec_ptr =
         lp_build_alloca(gallivm,
                         uint_bld->vec_type,
                         "emitted_prims_ptr");
      bld->emitted_vertices_vec_ptr =
         lp_build_alloca(gallivm,
                         uint_bld->vec_type,
                         "emitted_vertices_ptr");
      bld->total_emitted_vertices_vec_ptr =
         lp_build_alloca(gallivm,
                         uint_bld->vec_type,
                         "total_emitted_vertices_ptr");

      LLVMBuildStore(gallivm->builder, uint_bld->zero,
                     bld->emitted_prims_vec_ptr);
      LLVMBuildStore(gallivm->builder, uint_bld->zero,
                     bld->emitted_vertices_vec_ptr);
      LLVMBuildStore(gallivm->builder, uint_bld->zero,
                     bld->total_emitted_vertices_vec_ptr);
   }

   if (DEBUG_EXECUTION) {
      lp_build_printf(gallivm, "\n");
      emit_dump_file(bld, TGSI_FILE_CONSTANT);
      if (!bld->gs_iface)
         emit_dump_file(bld, TGSI_FILE_INPUT);
   }
}

static void emit_prologue_post_decl(struct lp_build_tgsi_context * bld_base)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);

   if (bld->tcs_iface && bld->tcs_iface->emit_prologue) {
      bld->tcs_iface->emit_prologue((struct lp_build_context*)bld_base);
   }
}

static void emit_epilogue(struct lp_build_tgsi_context * bld_base)
{
   struct lp_build_tgsi_soa_context * bld = lp_soa_context(bld_base);
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;

   if (DEBUG_EXECUTION) {
      /* for debugging */
      if (0) {
         emit_dump_file(bld, TGSI_FILE_TEMPORARY);
      }
      emit_dump_file(bld, TGSI_FILE_OUTPUT);
      lp_build_printf(bld_base->base.gallivm, "\n");
   }

   if (bld->tcs_iface && bld->tcs_iface->emit_epilogue) {
      bld->tcs_iface->emit_epilogue((struct lp_build_context*)bld_base);
   }

   /* If we have indirect addressing in outputs we need to copy our alloca array
    * to the outputs slots specified by the caller */
   if (bld->gs_iface) {
      LLVMValueRef total_emitted_vertices_vec;
      LLVMValueRef emitted_prims_vec;
      /* implicit end_primitives, needed in case there are any unflushed
         vertices in the cache. Note must not call end_primitive here
         since the exec_mask is not valid at this point. */
      end_primitive_masked(bld_base, lp_build_mask_value(bld->mask));

      total_emitted_vertices_vec =
         LLVMBuildLoad2(builder, bld_base->uint_bld.vec_type,
                        bld->total_emitted_vertices_vec_ptr, "");
      emitted_prims_vec =
         LLVMBuildLoad2(builder, bld_base->uint_bld.vec_type,
                        bld->emitted_prims_vec_ptr, "");

      bld->gs_iface->gs_epilogue(bld->gs_iface,
                                 total_emitted_vertices_vec,
                                 emitted_prims_vec, 0);
   } else {
      gather_outputs(bld);
   }
}

void
lp_build_tgsi_soa(struct gallivm_state *gallivm,
                  const struct tgsi_token *tokens,
                  const struct lp_build_tgsi_params *params,
                  LLVMValueRef (*outputs)[TGSI_NUM_CHANNELS])
{
   struct lp_build_tgsi_soa_context bld;
   struct lp_type type = params->type;
   struct lp_type res_type;

   assert(type.length <= LP_MAX_VECTOR_LENGTH);
   memset(&res_type, 0, sizeof res_type);
   res_type.width = type.width;
   res_type.length = type.length;
   res_type.sign = 1;

   /* Setup build context */
   memset(&bld, 0, sizeof bld);
   lp_build_context_init(&bld.bld_base.base, gallivm, type);
   lp_build_context_init(&bld.bld_base.uint_bld, gallivm, lp_uint_type(type));
   lp_build_context_init(&bld.bld_base.int_bld, gallivm, lp_int_type(type));
   lp_build_context_init(&bld.elem_bld, gallivm, lp_elem_type(type));
   {
      struct lp_type dbl_type;
      dbl_type = type;
      dbl_type.width *= 2;
      lp_build_context_init(&bld.bld_base.dbl_bld, gallivm, dbl_type);
   }
   {
      struct lp_type uint64_type;
      uint64_type = lp_uint_type(type);
      uint64_type.width *= 2;
      lp_build_context_init(&bld.bld_base.uint64_bld, gallivm, uint64_type);
   }
   {
      struct lp_type int64_type;
      int64_type = lp_int_type(type);
      int64_type.width *= 2;
      lp_build_context_init(&bld.bld_base.int64_bld, gallivm, int64_type);
   }
   bld.mask = params->mask;
   bld.inputs = params->inputs;
   bld.outputs = outputs;
   bld.consts_ptr = params->consts_ptr;
   bld.ssbo_ptr = params->ssbo_ptr;
   bld.sampler = params->sampler;
   bld.bld_base.info = params->info;
   bld.indirect_files = params->info->indirect_files;
   bld.context_type = params->context_type;
   bld.context_ptr = params->context_ptr;
   bld.resources_type = params->resources_type;
   bld.resources_ptr = params->resources_ptr;
   bld.thread_data_type =  params->thread_data_type;
   bld.thread_data_ptr = params->thread_data_ptr;
   bld.image = params->image;
   bld.shared_ptr = params->shared_ptr;
   bld.coro = params->coro;

   /*
    * If the number of temporaries is rather large then we just
    * allocate them as an array right from the start and treat
    * like indirect temporaries.
    */
   if (params->info->file_max[TGSI_FILE_TEMPORARY] >= LP_MAX_INLINED_TEMPS) {
      bld.indirect_files |= (1 << TGSI_FILE_TEMPORARY);
   }
   /*
    * For performance reason immediates are always backed in a static
    * array, but if their number is too great, we have to use just
    * a dynamically allocated array.
    */
   bld.use_immediates_array =
         (params->info->file_max[TGSI_FILE_IMMEDIATE] >= LP_MAX_INLINED_IMMEDIATES);
   if (bld.use_immediates_array) {
      bld.indirect_files |= (1 << TGSI_FILE_IMMEDIATE);
   }


   bld.bld_base.soa = true;
   bld.bld_base.emit_debug = emit_debug;
   bld.bld_base.emit_fetch_funcs[TGSI_FILE_CONSTANT] = emit_fetch_constant;
   bld.bld_base.emit_fetch_funcs[TGSI_FILE_IMMEDIATE] = emit_fetch_immediate;
   bld.bld_base.emit_fetch_funcs[TGSI_FILE_INPUT] = emit_fetch_input;
   bld.bld_base.emit_fetch_funcs[TGSI_FILE_TEMPORARY] = emit_fetch_temporary;
   bld.bld_base.emit_fetch_funcs[TGSI_FILE_SYSTEM_VALUE] = emit_fetch_system_value;

   bld.bld_base.emit_store = emit_store;
   bld.bld_base.emit_store_reg_funcs[TGSI_FILE_OUTPUT] = emit_store_output;
   bld.bld_base.emit_store_reg_funcs[TGSI_FILE_TEMPORARY] = emit_store_temp;
   bld.bld_base.emit_store_reg_funcs[TGSI_FILE_ADDRESS] = emit_store_address;

   bld.bld_base.emit_declaration = lp_emit_declaration_soa;
   bld.bld_base.emit_immediate = lp_emit_immediate_soa;

   bld.bld_base.emit_prologue = emit_prologue;
   bld.bld_base.emit_prologue_post_decl = emit_prologue_post_decl;
   bld.bld_base.emit_epilogue = emit_epilogue;

   /* Set opcode actions */
   lp_set_default_actions_cpu(&bld.bld_base);

   bld.bld_base.op_actions[TGSI_OPCODE_BGNLOOP].emit = bgnloop_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_BGNSUB].emit = bgnsub_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_BRK].emit = brk_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_CAL].emit = cal_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_CASE].emit = case_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_CONT].emit = cont_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_DDX].emit = ddx_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_DDY].emit = ddy_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_DEFAULT].emit = default_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ELSE].emit = else_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ENDIF].emit = endif_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ENDLOOP].emit = endloop_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ENDSUB].emit = endsub_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ENDSWITCH].emit = endswitch_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_IF].emit = if_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_UIF].emit = uif_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_KILL_IF].emit = kill_if_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_KILL].emit = kill_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_RET].emit = ret_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_SWITCH].emit = switch_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TEX].emit = tex_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TXB].emit = txb_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TXD].emit = txd_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TXL].emit = txl_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TEX_LZ].emit = txl_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TXP].emit = txp_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TXQ].emit = txq_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TXF].emit = txf_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TXF_LZ].emit = txf_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TEX2].emit = tex2_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TXB2].emit = txb2_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TXL2].emit = txl2_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_TG4].emit = tg4_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_LODQ].emit = lodq_emit;
   /* DX10 sampling ops */
   bld.bld_base.op_actions[TGSI_OPCODE_SAMPLE].emit = sample_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_SAMPLE_B].emit = sample_b_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_SAMPLE_C].emit = sample_c_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_SAMPLE_C_LZ].emit = sample_c_lz_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_SAMPLE_D].emit = sample_d_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_SAMPLE_I].emit = sample_i_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_SAMPLE_I_MS].emit = sample_i_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_SAMPLE_L].emit = sample_l_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_GATHER4].emit = gather4_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_SVIEWINFO].emit = sviewinfo_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_LOD].emit = lod_emit;

   bld.bld_base.op_actions[TGSI_OPCODE_LOAD].emit = load_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_STORE].emit = store_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_RESQ].emit = resq_emit;

   bld.bld_base.op_actions[TGSI_OPCODE_ATOMUADD].emit = atomic_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ATOMXCHG].emit = atomic_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ATOMCAS].emit = atomic_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ATOMAND].emit = atomic_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ATOMOR].emit = atomic_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ATOMXOR].emit = atomic_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ATOMUMIN].emit = atomic_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ATOMUMAX].emit = atomic_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ATOMIMIN].emit = atomic_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_ATOMIMAX].emit = atomic_emit;

   bld.bld_base.op_actions[TGSI_OPCODE_MEMBAR].emit = membar_emit;
   bld.bld_base.op_actions[TGSI_OPCODE_BARRIER].emit = barrier_emit;

   if (params->gs_iface) {
      /* There's no specific value for this because it should always
       * be set, but apps using ext_geometry_shader4 quite often
       * were forgetting so we're using MAX_VERTEX_VARYING from
       * that spec even though we could assert if it's not
       * set, but that's a lot uglier. */
      unsigned max_output_vertices;

      /* inputs are always indirect with gs */
      bld.indirect_files |= (1 << TGSI_FILE_INPUT);
      bld.gs_iface = params->gs_iface;
      bld.bld_base.emit_fetch_funcs[TGSI_FILE_INPUT] = emit_fetch_gs_input;
      bld.bld_base.op_actions[TGSI_OPCODE_EMIT].emit = emit_vertex;
      bld.bld_base.op_actions[TGSI_OPCODE_ENDPRIM].emit = end_primitive;

      max_output_vertices =
         params->info->properties[TGSI_PROPERTY_GS_MAX_OUTPUT_VERTICES];
      if (!max_output_vertices)
         max_output_vertices = 32;

      bld.max_output_vertices_vec =
         lp_build_const_int_vec(gallivm, bld.bld_base.int_bld.type,
                                max_output_vertices);
   }

   if (params->tes_iface) {
      /* inputs are always indirect with tes */
      bld.indirect_files |= (1 << TGSI_FILE_INPUT);
      bld.tes_iface = params->tes_iface;
      bld.bld_base.emit_fetch_funcs[TGSI_FILE_INPUT] = emit_fetch_tes_input;
   }

   if (params->tcs_iface) {
      bld.tcs_iface = params->tcs_iface;
      /* outputs and inputs are always indirect with tcs */
      bld.indirect_files |= (1 << TGSI_FILE_OUTPUT);
      bld.bld_base.emit_store_reg_funcs[TGSI_FILE_OUTPUT] = emit_store_tcs_output;
      bld.indirect_files |= (1 << TGSI_FILE_INPUT);
      bld.bld_base.emit_fetch_funcs[TGSI_FILE_INPUT] = emit_fetch_tcs_input;
      bld.bld_base.emit_fetch_funcs[TGSI_FILE_OUTPUT] = emit_fetch_tcs_input;
      bld.bld_base.op_actions[TGSI_OPCODE_BARRIER].emit = barrier_emit_tcs;
   }

   lp_exec_mask_init(&bld.exec_mask, &bld.bld_base.int_bld);

   bld.system_values = *params->system_values;

   lp_build_tgsi_llvm(&bld.bld_base, tokens);

   if (0) {
      LLVMBasicBlockRef block = LLVMGetInsertBlock(gallivm->builder);
      LLVMValueRef function = LLVMGetBasicBlockParent(block);
      debug_printf("11111111111111111111111111111 \n");
      tgsi_dump(tokens, 0);
      lp_debug_dump_value(function);
      debug_printf("2222222222222222222222222222 \n");
   }

   if (0) {
      LLVMModuleRef module = LLVMGetGlobalParent(
         LLVMGetBasicBlockParent(LLVMGetInsertBlock(gallivm->builder)));
      LLVMDumpModule(module);

   }
   lp_exec_mask_fini(&bld.exec_mask);
}
