/**************************************************************************
 *
 * Copyright 2011-2012 Advanced Micro Devices, Inc.
 * Copyright 2010 VMware, Inc.
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

#include "gallivm/lp_bld_tgsi.h"

#include "gallivm/lp_bld_arit.h"
#include "gallivm/lp_bld_gather.h"
#include "gallivm/lp_bld_init.h"
#include "gallivm/lp_bld_intr.h"
#include "tgsi/tgsi_info.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_util.h"
#include "util/u_memory.h"


// grow the instruction buffer by this number of instructions
#define LP_NUM_INSTRUCTIONS 256


/* The user is responsible for freeing list->instructions */
unsigned
lp_bld_tgsi_list_init(struct lp_build_tgsi_context *bld_base)
{
   bld_base->instructions = (struct tgsi_full_instruction *)
         MALLOC(LP_NUM_INSTRUCTIONS * sizeof(struct tgsi_full_instruction));
   if (!bld_base->instructions) {
      return 0;
   }
   bld_base->max_instructions = LP_NUM_INSTRUCTIONS;
   return 1;
}


unsigned
lp_bld_tgsi_add_instruction(struct lp_build_tgsi_context *bld_base,
                            const struct tgsi_full_instruction *inst_to_add)
{
   if (bld_base->num_instructions == bld_base->max_instructions) {
      struct tgsi_full_instruction *instructions =
         REALLOC(bld_base->instructions, bld_base->max_instructions
                 * sizeof(struct tgsi_full_instruction),
                 (bld_base->max_instructions + LP_NUM_INSTRUCTIONS)
                 * sizeof(struct tgsi_full_instruction));
      if (!instructions) {
         return 0;
      }
      bld_base->instructions = instructions;
      bld_base->max_instructions += LP_NUM_INSTRUCTIONS;
   }
   memcpy(bld_base->instructions + bld_base->num_instructions, inst_to_add,
          sizeof(bld_base->instructions[0]));

   bld_base->num_instructions++;

   return 1;
}


/**
 * This function assumes that all the args in emit_data have been set.
 */
static void
lp_build_action_set_dst_type(struct lp_build_emit_data *emit_data,
                             struct lp_build_tgsi_context *bld_base,
                             unsigned tgsi_opcode)
{
   if (emit_data->arg_count == 0) {
      emit_data->dst_type =
         LLVMVoidTypeInContext(bld_base->base.gallivm->context);
   } else {
      /* XXX: Not all opcodes have the same src and dst types. */
      emit_data->dst_type = LLVMTypeOf(emit_data->args[0]);
   }
}


void
lp_build_tgsi_intrinsic(const struct lp_build_tgsi_action *action,
                        struct lp_build_tgsi_context *bld_base,
                        struct lp_build_emit_data *emit_data)
{
   struct lp_build_context * base = &bld_base->base;
   emit_data->output[emit_data->chan] = lp_build_intrinsic(
               base->gallivm->builder, action->intr_name,
               emit_data->dst_type, emit_data->args, emit_data->arg_count, 0);
}


LLVMValueRef
lp_build_emit_llvm(struct lp_build_tgsi_context *bld_base,
                   unsigned tgsi_opcode,
                   struct lp_build_emit_data *emit_data)
{
   struct lp_build_tgsi_action * action = &bld_base->op_actions[tgsi_opcode];
   /* XXX: Assert that this is a componentwise or replicate instruction */

   lp_build_action_set_dst_type(emit_data, bld_base, tgsi_opcode);
   emit_data->chan = 0;
   assert(action->emit);
   action->emit(action, bld_base, emit_data);
   return emit_data->output[0];
}


LLVMValueRef
lp_build_emit_llvm_unary(struct lp_build_tgsi_context *bld_base,
                         unsigned tgsi_opcode,
                         LLVMValueRef arg0)
{
   struct lp_build_emit_data emit_data = {{0}};
   emit_data.info = tgsi_get_opcode_info(tgsi_opcode);
   emit_data.arg_count = 1;
   emit_data.args[0] = arg0;
   return lp_build_emit_llvm(bld_base, tgsi_opcode, &emit_data);
}


LLVMValueRef
lp_build_emit_llvm_binary(struct lp_build_tgsi_context *bld_base,
                          unsigned tgsi_opcode,
                          LLVMValueRef arg0,
                          LLVMValueRef arg1)
{
   struct lp_build_emit_data emit_data = {{0}};
   emit_data.info = tgsi_get_opcode_info(tgsi_opcode);
   emit_data.arg_count = 2;
   emit_data.args[0] = arg0;
   emit_data.args[1] = arg1;
   return lp_build_emit_llvm(bld_base, tgsi_opcode, &emit_data);
}


LLVMValueRef
lp_build_emit_llvm_ternary(struct lp_build_tgsi_context *bld_base,
                           unsigned tgsi_opcode,
                           LLVMValueRef arg0,
                           LLVMValueRef arg1,
                           LLVMValueRef arg2)
{
   struct lp_build_emit_data emit_data = {{0}};
   emit_data.info = tgsi_get_opcode_info(tgsi_opcode);
   emit_data.arg_count = 3;
   emit_data.args[0] = arg0;
   emit_data.args[1] = arg1;
   emit_data.args[2] = arg2;
   return lp_build_emit_llvm(bld_base, tgsi_opcode, &emit_data);
}


/**
 * The default fetch implementation.
 */
void
lp_build_fetch_args(struct lp_build_tgsi_context *bld_base,
                    struct lp_build_emit_data *emit_data)
{
   for (unsigned src = 0; src < emit_data->info->num_src; src++) {
      emit_data->args[src] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                                 src, emit_data->src_chan);
   }
   emit_data->arg_count = emit_data->info->num_src;
   lp_build_action_set_dst_type(emit_data, bld_base,
                                emit_data->inst->Instruction.Opcode);
}


/**
 * with 64-bit src and dst channels aren't 1:1.
 * check the src/dst types for the opcode,
 * 1. if neither is 64-bit then src == dst;
 * 2. if dest is 64-bit
 *     - don't store to y or w
 *     - if src is 64-bit then src == dst.
 *     - else for f2d, d.xy = s.x
 *     - else for f2d, d.zw = s.y
 * 3. if dst is single, src is 64-bit
 *    - map dst x,z to src xy;
 *    - map dst y,w to src zw;
 */
static int
get_src_chan_idx(enum tgsi_opcode opcode,
                 int dst_chan_index)
{
   enum tgsi_opcode_type dtype = tgsi_opcode_infer_dst_type(opcode, 0);
   enum tgsi_opcode_type stype = tgsi_opcode_infer_src_type(opcode, 0);

   if (!tgsi_type_is_64bit(dtype) && !tgsi_type_is_64bit(stype))
      return dst_chan_index;
   if (tgsi_type_is_64bit(dtype)) {
      if (dst_chan_index == 1 || dst_chan_index == 3)
         return -1;
      if (tgsi_type_is_64bit(stype))
         return dst_chan_index;
      if (dst_chan_index == 0)
         return 0;
      if (dst_chan_index == 2)
         return 1;
   } else {
      if (dst_chan_index == 0 || dst_chan_index == 2)
         return 0;
      if (dst_chan_index == 1 || dst_chan_index == 3)
         return 2;
   }
   return -1;
}


/* XXX: COMMENT
 * It should be assumed that this function ignores writemasks
 */
bool
lp_build_tgsi_inst_llvm(struct lp_build_tgsi_context *bld_base,
                        const struct tgsi_full_instruction *inst)
{
   enum tgsi_opcode opcode = inst->Instruction.Opcode;
   const struct tgsi_opcode_info *info = tgsi_get_opcode_info(opcode);
   const struct lp_build_tgsi_action *action = &bld_base->op_actions[opcode];
   struct lp_build_emit_data emit_data;
   unsigned chan_index;
   LLVMValueRef val;
   bld_base->pc++;

   if (bld_base->emit_debug) {
      bld_base->emit_debug(bld_base, inst, info);
   }

   /* Ignore deprecated instructions */
   switch (inst->Instruction.Opcode) {
   case TGSI_OPCODE_UP2US:
   case TGSI_OPCODE_UP4B:
   case TGSI_OPCODE_UP4UB:
      /* deprecated? */
      assert(0);
      return false;
      break;
   }

   /* Check if the opcode has been implemented */
   if (!action->emit) {
      return false;
   }

   memset(&emit_data, 0, sizeof(emit_data));

   assert(info->num_dst <= 2);
   if (info->num_dst) {
      TGSI_FOR_EACH_DST0_ENABLED_CHANNEL(inst, chan_index) {
         emit_data.output[chan_index] = bld_base->base.undef;
      }

      if (info->num_dst >= 2) {
         TGSI_FOR_EACH_DST1_ENABLED_CHANNEL(inst, chan_index) {
            emit_data.output1[chan_index] = bld_base->base.undef;
         }
      }
   }

   emit_data.inst = inst;
   emit_data.info = info;

   /* Emit the instructions */
   if (info->output_mode == TGSI_OUTPUT_COMPONENTWISE && bld_base->soa) {
      TGSI_FOR_EACH_DST0_ENABLED_CHANNEL(inst, chan_index) {
         int src_index = get_src_chan_idx(inst->Instruction.Opcode, chan_index);
         /* ignore channels 1/3 in double dst */
         if (src_index == -1)
            continue;
         emit_data.chan = chan_index;
         emit_data.src_chan = src_index;
         if (!action->fetch_args) {
            lp_build_fetch_args(bld_base, &emit_data);
         } else {
             action->fetch_args(bld_base, &emit_data);
         }
         action->emit(action, bld_base, &emit_data);
      }
   } else {
      emit_data.chan = LP_CHAN_ALL;
      if (action->fetch_args) {
         action->fetch_args(bld_base, &emit_data);
      }
      /* Make sure the output value is stored in emit_data.output[0], unless
       * the opcode is channel dependent */
      if (info->output_mode != TGSI_OUTPUT_CHAN_DEPENDENT) {
         emit_data.chan = 0;
      }
      action->emit(action, bld_base, &emit_data);

      /* Replicate the output values */
      if (info->output_mode == TGSI_OUTPUT_REPLICATE && bld_base->soa) {
         val = emit_data.output[0];
         memset(emit_data.output, 0, sizeof(emit_data.output));
         TGSI_FOR_EACH_DST0_ENABLED_CHANNEL(inst, chan_index) {
            emit_data.output[chan_index] = val;
         }

         if (info->num_dst >= 2) {
            val = emit_data.output1[0];
            memset(emit_data.output1, 0, sizeof(emit_data.output1));
            TGSI_FOR_EACH_DST1_ENABLED_CHANNEL(inst, chan_index) {
               emit_data.output1[chan_index] = val;
            }
         }
      }
   }

   if (info->num_dst > 0 && info->opcode != TGSI_OPCODE_STORE) {
      bld_base->emit_store(bld_base, inst, info, 0, emit_data.output);
      if (info->num_dst >= 2)
         bld_base->emit_store(bld_base, inst, info, 1, emit_data.output1);
   }
   return true;
}


LLVMValueRef
lp_build_emit_fetch_src(struct lp_build_tgsi_context *bld_base,
                        const struct tgsi_full_src_register *reg,
                        enum tgsi_opcode_type stype,
                        const unsigned chan_index)
{
   unsigned swizzle;
   LLVMValueRef res;

   if (chan_index == LP_CHAN_ALL) {
      swizzle = ~0u;
   } else {
      swizzle = tgsi_util_get_full_src_register_swizzle(reg, chan_index);
      if (swizzle > 3) {
         assert(0 && "invalid swizzle in emit_fetch()");
         return bld_base->base.undef;
      }
      if (tgsi_type_is_64bit(stype)) {
        unsigned swizzle2;
        swizzle2 = tgsi_util_get_full_src_register_swizzle(reg, chan_index + 1);
        if (swizzle2 > 3) {
           assert(0 && "invalid swizzle in emit_fetch()");
           return bld_base->base.undef;
        }
        swizzle |= (swizzle2 << 16);
      }
   }

   assert(reg->Register.Index <= bld_base->info->file_max[reg->Register.File]);

   if (bld_base->emit_fetch_funcs[reg->Register.File]) {
      res = bld_base->emit_fetch_funcs[reg->Register.File](bld_base, reg, stype,
                                                           swizzle);
   } else {
      assert(0 && "invalid src register in emit_fetch()");
      return bld_base->base.undef;
   }

   if (reg->Register.Absolute) {
      switch (stype) {
      case TGSI_TYPE_FLOAT:
      case TGSI_TYPE_UNTYPED:
          /* modifiers on movs assume data is float */
         res = lp_build_abs(&bld_base->base, res);
         break;
      case TGSI_TYPE_DOUBLE:
      case TGSI_TYPE_UNSIGNED:
      case TGSI_TYPE_SIGNED:
      case TGSI_TYPE_UNSIGNED64:
      case TGSI_TYPE_SIGNED64:
      case TGSI_TYPE_VOID:
      default:
         /* abs modifier is only legal on floating point types */
         assert(0);
         break;
      }
   }

   if (reg->Register.Negate) {
      switch (stype) {
      case TGSI_TYPE_FLOAT:
      case TGSI_TYPE_UNTYPED:
         /* modifiers on movs assume data is float */
         res = lp_build_negate(&bld_base->base, res);
         break;
      case TGSI_TYPE_DOUBLE:
         /* no double build context */
         assert(0);
         break;
      case TGSI_TYPE_SIGNED:
      case TGSI_TYPE_UNSIGNED:
         res = lp_build_negate(&bld_base->int_bld, res);
         break;
      case TGSI_TYPE_SIGNED64:
      case TGSI_TYPE_UNSIGNED64:
         res = lp_build_negate(&bld_base->int64_bld, res);
         break;
      case TGSI_TYPE_VOID:
      default:
         assert(0);
         break;
      }
   }

   /*
    * Swizzle the argument
    */

   if (swizzle == ~0u) {
      res = bld_base->emit_swizzle(bld_base, res,
                     reg->Register.SwizzleX,
                     reg->Register.SwizzleY,
                     reg->Register.SwizzleZ,
                     reg->Register.SwizzleW);
   }

   return res;
}


LLVMValueRef
lp_build_emit_fetch(struct lp_build_tgsi_context *bld_base,
                    const struct tgsi_full_instruction *inst,
                    unsigned src_op,
                    const unsigned chan_index)
{
   const struct tgsi_full_src_register *reg = &inst->Src[src_op];
   enum tgsi_opcode_type stype =
      tgsi_opcode_infer_src_type(inst->Instruction.Opcode, src_op);

   return lp_build_emit_fetch_src(bld_base, reg, stype, chan_index);
}


LLVMValueRef
lp_build_emit_fetch_texoffset(struct lp_build_tgsi_context *bld_base,
                              const struct tgsi_full_instruction *inst,
                              unsigned tex_off_op,
                              const unsigned chan_index)
{
   const struct tgsi_texture_offset *off = &inst->TexOffsets[tex_off_op];
   struct tgsi_full_src_register reg;
   unsigned swizzle;
   LLVMValueRef res;
   enum tgsi_opcode_type stype = TGSI_TYPE_SIGNED;

   /* convert offset "register" to ordinary register so can use normal emit funcs */
   memset(&reg, 0, sizeof(reg));
   reg.Register.File = off->File;
   reg.Register.Index = off->Index;
   reg.Register.SwizzleX = off->SwizzleX;
   reg.Register.SwizzleY = off->SwizzleY;
   reg.Register.SwizzleZ = off->SwizzleZ;

   if (chan_index == LP_CHAN_ALL) {
      swizzle = ~0;
   } else {
      assert(chan_index < TGSI_SWIZZLE_W);
      swizzle = tgsi_util_get_src_register_swizzle(&reg.Register, chan_index);
   }

   assert(off->Index <= bld_base->info->file_max[off->File]);

   if (bld_base->emit_fetch_funcs[off->File]) {
      res = bld_base->emit_fetch_funcs[off->File](bld_base, &reg, stype,
                                                           swizzle);
   } else {
      assert(0 && "invalid src register in emit_fetch_texoffset()");
      return bld_base->base.undef;
   }

   /*
    * Swizzle the argument
    */

   if (swizzle == ~0u) {
      res = bld_base->emit_swizzle(bld_base, res,
                                   off->SwizzleX,
                                   off->SwizzleY,
                                   off->SwizzleZ,
                                   /* there's no 4th channel */
                                   off->SwizzleX);
   }

   return res;
}


bool
lp_build_tgsi_llvm(struct lp_build_tgsi_context *bld_base,
                   const struct tgsi_token *tokens)
{
   if (bld_base->emit_prologue) {
      bld_base->emit_prologue(bld_base);
   }

   if (!lp_bld_tgsi_list_init(bld_base)) {
      return false;
   }

   struct tgsi_parse_context parse;
   tgsi_parse_init(&parse, tokens);

   while(!tgsi_parse_end_of_tokens(&parse)) {
      tgsi_parse_token(&parse);

      switch(parse.FullToken.Token.Type) {
      case TGSI_TOKEN_TYPE_DECLARATION:
         /* Inputs already interpolated */
         bld_base->emit_declaration(bld_base, &parse.FullToken.FullDeclaration);
         break;
      case TGSI_TOKEN_TYPE_INSTRUCTION:
         lp_bld_tgsi_add_instruction(bld_base, &parse.FullToken.FullInstruction);
         break;
      case TGSI_TOKEN_TYPE_IMMEDIATE:
         bld_base->emit_immediate(bld_base, &parse.FullToken.FullImmediate);
         break;
      case TGSI_TOKEN_TYPE_PROPERTY:
         break;
      default:
         assert(0);
      }
   }

   if (bld_base->emit_prologue_post_decl) {
      bld_base->emit_prologue_post_decl(bld_base);
   }

   while (bld_base->pc != -1) {
      const struct tgsi_full_instruction *instr =
         bld_base->instructions + bld_base->pc;
      if (!lp_build_tgsi_inst_llvm(bld_base, instr)) {
         _debug_printf("warning: failed to translate tgsi opcode %s to LLVM\n",
                       tgsi_get_opcode_name(instr->Instruction.Opcode));
         return false;
      }
   }

   tgsi_parse_free(&parse);

   FREE(bld_base->instructions);

   if (bld_base->emit_epilogue) {
      bld_base->emit_epilogue(bld_base);
   }

   return true;
}
