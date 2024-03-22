/*
 * Copyright 2018 VMware, Inc.
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
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * This utility transforms the shader to support dynamic array indexing
 * for samplers and constant buffers.
 * It calculates dynamic array index first and then compare it with each
 * index and operation will be performed with matching index
 */

#include "util/u_debug.h"
#include "util/u_math.h"
#include "tgsi_info.h"
#include "tgsi_dynamic_indexing.h"
#include "tgsi_transform.h"
#include "tgsi_dump.h"
#include "pipe/p_state.h"


struct dIndexing_transform_context
{
   struct tgsi_transform_context base;
   unsigned orig_num_tmp;
   unsigned orig_num_imm;
   unsigned num_const_bufs;
   unsigned num_samplers;
   unsigned num_iterations;
   unsigned const_buf_range[PIPE_MAX_CONSTANT_BUFFERS];
};


static inline struct dIndexing_transform_context *
dIndexing_transform_context(struct tgsi_transform_context *ctx)
{
   return (struct dIndexing_transform_context *) ctx;
}


/**
 * TGSI declaration transform callback.
 */
static void
dIndexing_decl(struct tgsi_transform_context *ctx,
               struct tgsi_full_declaration *decl)
{
   struct dIndexing_transform_context *dc = dIndexing_transform_context(ctx);

   if (decl->Declaration.File == TGSI_FILE_TEMPORARY) {
      /**
       * Emit some extra temporary register to use in keeping track of
       * dynamic index.
       */
      dc->orig_num_tmp = decl->Range.Last;
      decl->Range.Last = decl->Range.Last + 3;
   }
   else if (decl->Declaration.File == TGSI_FILE_CONSTANT) {
      /* Keep track of number of constants in each buffer */
      dc->const_buf_range[decl->Dim.Index2D] = decl->Range.Last;
   }
   ctx->emit_declaration(ctx, decl);
}


/**
 * TGSI transform prolog callback.
 */
static void
dIndexing_prolog(struct tgsi_transform_context *ctx)
{
   tgsi_transform_immediate_int_decl(ctx, 0, 1, 2, 3);
   tgsi_transform_immediate_int_decl(ctx, 4, 5, 6, 7);
}


/**
 * This function emits some extra instruction to remove dynamic array
 * indexing of constant buffers / samplers from the shader.
 * It calculates dynamic array index first and compare it with each index for
 * declared constants/samplers.
 */
static void
remove_dynamic_indexes(struct tgsi_transform_context *ctx,
                       struct tgsi_full_instruction *orig_inst,
                       const struct tgsi_full_src_register *reg)
{
   struct dIndexing_transform_context *dc = dIndexing_transform_context(ctx);
   int i, j;
   int tmp_loopIdx = dc->orig_num_tmp + 1;
   int tmp_cond = dc->orig_num_tmp + 2;
   int tmp_arrayIdx = dc->orig_num_tmp + 3;
   int imm_index = dc->orig_num_imm;
   struct tgsi_full_instruction inst;
   unsigned INVALID_INDEX = 99999;
   enum tgsi_file_type file = TGSI_FILE_NULL;
   unsigned index = INVALID_INDEX;
   unsigned imm_swz_index = INVALID_INDEX;

   /* calculate dynamic array index store it in tmp_arrayIdx.x */
   inst = tgsi_default_full_instruction();
   inst.Instruction.Opcode = TGSI_OPCODE_UADD;
   inst.Instruction.NumDstRegs = 1;
   tgsi_transform_dst_reg(&inst.Dst[0], TGSI_FILE_TEMPORARY,
                          tmp_arrayIdx, TGSI_WRITEMASK_X);
   inst.Instruction.NumSrcRegs = 2;
   if (reg->Register.File == TGSI_FILE_CONSTANT) {
      file = reg->DimIndirect.File;
      index = reg->DimIndirect.Index;
      imm_swz_index = reg->Dimension.Index;
   }
   else if (reg->Register.File == TGSI_FILE_SAMPLER) {
      file = reg->Indirect.File;
      index = reg->Indirect.Index;
      imm_swz_index = reg->Register.Index;
   }
   tgsi_transform_src_reg(&inst.Src[0], file,
                          index, TGSI_SWIZZLE_X,
                          TGSI_SWIZZLE_X, TGSI_SWIZZLE_X, TGSI_SWIZZLE_X);
   tgsi_transform_src_reg(&inst.Src[1], TGSI_FILE_IMMEDIATE,
                          imm_index + (imm_swz_index / 4),
                          imm_swz_index % 4,
                          imm_swz_index % 4,
                          imm_swz_index % 4,
                          imm_swz_index % 4);
   ctx->emit_instruction(ctx, &inst);

   /* initialize counter to zero: tmp_loopIdx = 0 */
   inst = tgsi_default_full_instruction();
   inst.Instruction.Opcode = TGSI_OPCODE_MOV;
   inst.Instruction.NumDstRegs = 1;
   tgsi_transform_dst_reg(&inst.Dst[0], TGSI_FILE_TEMPORARY,
                          tmp_loopIdx, TGSI_WRITEMASK_X);
   inst.Instruction.NumSrcRegs = 1;
   tgsi_transform_src_reg(&inst.Src[0], TGSI_FILE_IMMEDIATE,
                          imm_index, TGSI_SWIZZLE_X,
                          TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                          TGSI_SWIZZLE_X);
   ctx->emit_instruction(ctx, &inst);

   for (i = 0; i < dc->num_iterations; i++) {
      bool out_of_bound_index = false;
      /**
       * Make sure we are not exceeding index limit of constant buffer
       *
       * For example, In declaration, We have
       *
       * DCL CONST[0][0..1]
       * DCL CONST[1][0..2]
       * DCL CONST[2][0]
       *
       * and our dynamic index instruction is
       * MOV TEMP[0], CONST[ADDR[0].x][1]
       *
       * We have to make sure to skip unrolling for CONST[2] because
       * it has only one constant in the buffer
       */
      if ((reg->Register.File == TGSI_FILE_CONSTANT) &&
          (!reg->Register.Indirect &&
           (reg->Register.Index > dc->const_buf_range[i]))) {
         out_of_bound_index = true;
      }

      if (!out_of_bound_index) {
         /**
          * If we have an instruction of the format:
          * OPCODE dst, src..., CONST[K][foo], src...
          * where K is dynamic and tmp_loopIdx = i (loopcount),
          * replace it with:
          *
          * if (K == tmp_loopIdx)
          *    OPCODE dst, src... where src is CONST[i][foo] and i is constant
          * }
          *
          * Similarly, If instruction uses dynamic array index for samplers
          * e.g. OPCODE dst, src, SAMPL[k] ..
          * replace it with:
          * if (K == tmp_loopIdx)
          *    OPCODE dst, src, SAMPL[i][foo]... where i is constant.
          * }
          */
         inst = tgsi_default_full_instruction();
         inst.Instruction.Opcode = TGSI_OPCODE_USEQ;
         inst.Instruction.NumDstRegs = 1;
         tgsi_transform_dst_reg(&inst.Dst[0], TGSI_FILE_TEMPORARY,
                                tmp_cond, TGSI_WRITEMASK_X);
         inst.Instruction.NumSrcRegs = 2;
         tgsi_transform_src_reg(&inst.Src[0], TGSI_FILE_TEMPORARY,
                                tmp_arrayIdx, TGSI_SWIZZLE_X,
                                TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                                TGSI_SWIZZLE_X);
         tgsi_transform_src_reg(&inst.Src[1], TGSI_FILE_TEMPORARY,
                                tmp_loopIdx, TGSI_SWIZZLE_X,
                                TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                                TGSI_SWIZZLE_X);
         ctx->emit_instruction(ctx, &inst);

         inst = tgsi_default_full_instruction();
         inst.Instruction.Opcode = TGSI_OPCODE_UIF;
         inst.Instruction.NumDstRegs = 0;
         inst.Instruction.NumSrcRegs = 1;
         tgsi_transform_src_reg(&inst.Src[0], TGSI_FILE_TEMPORARY,
                                tmp_cond, TGSI_SWIZZLE_X,
                                TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                                TGSI_SWIZZLE_X);
         ctx->emit_instruction(ctx, &inst);

         /* emit instruction with new, non-dynamic source registers */
         inst = *orig_inst;
         for (j = 0; j < inst.Instruction.NumSrcRegs; j++) {
            if (inst.Src[j].Dimension.Indirect &&
                inst.Src[j].Register.File == TGSI_FILE_CONSTANT) {
               inst.Src[j].Register.Dimension = 1;
               inst.Src[j].Dimension.Index = i;
               inst.Src[j].Dimension.Indirect = 0;
            }
            else if (inst.Src[j].Register.Indirect &&
                     inst.Src[j].Register.File == TGSI_FILE_SAMPLER) {
               inst.Src[j].Register.Indirect = 0;
               inst.Src[j].Register.Index = i;
            }
         }
         ctx->emit_instruction(ctx, &inst);

         inst = tgsi_default_full_instruction();
         inst.Instruction.Opcode = TGSI_OPCODE_ENDIF;
         inst.Instruction.NumDstRegs = 0;
         inst.Instruction.NumSrcRegs = 0;
         ctx->emit_instruction(ctx, &inst);
      }

      /**
       * Increment counter
       * UADD tmp_loopIdx.x tmp_loopIdx.x imm(1)
       */
      inst = tgsi_default_full_instruction();
      inst.Instruction.Opcode = TGSI_OPCODE_UADD;
      inst.Instruction.NumDstRegs = 1;
      tgsi_transform_dst_reg(&inst.Dst[0], TGSI_FILE_TEMPORARY,
                             tmp_loopIdx, TGSI_WRITEMASK_X);
      inst.Instruction.NumSrcRegs = 2;
      tgsi_transform_src_reg(&inst.Src[0], TGSI_FILE_TEMPORARY,
                              tmp_loopIdx, TGSI_SWIZZLE_X,
                              TGSI_SWIZZLE_X, TGSI_SWIZZLE_X, TGSI_SWIZZLE_X);
      tgsi_transform_src_reg(&inst.Src[1], TGSI_FILE_IMMEDIATE, imm_index,
                             TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Y,
                             TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Y);

      ctx->emit_instruction(ctx, &inst);
   }
}


/**
 * TGSI instruction transform callback.
 */
static void
dIndexing_inst(struct tgsi_transform_context *ctx,
               struct tgsi_full_instruction *inst)
{
   int i;
   bool indexing = false;
   struct dIndexing_transform_context *dc = dIndexing_transform_context(ctx);

   for (i = 0; i < inst->Instruction.NumSrcRegs; i++) {
      struct tgsi_full_src_register *src;
      src = &inst->Src[i];
      /* check if constant buffer/sampler is using dynamic index */
      if ((src->Dimension.Indirect &&
           src->Register.File == TGSI_FILE_CONSTANT) ||
          (src->Register.Indirect &&
           src->Register.File == TGSI_FILE_SAMPLER)) {

         if (indexing)
            assert("More than one src has dynamic indexing");

         if (src->Register.File == TGSI_FILE_CONSTANT)
            dc->num_iterations = dc->num_const_bufs;
         else
            dc->num_iterations = dc->num_samplers;

         remove_dynamic_indexes(ctx, inst, src);
         indexing = true;
      }
   }

   if (!indexing) {
      ctx->emit_instruction(ctx, inst);
   }
}

/**
 * TGSI utility to remove dynamic array indexing for constant buffers and
 * samplers.
 *
 * This utility accepts bitmask of declared constant buffers and samplers,
 * number of immediates used in shader.
 *
 * If dynamic array index is used for constant buffers and samplers, this
 * utility removes those dynamic indexes from shader. It also makes sure
 * that it has same output as per original shader.
 * This is achieved by calculating dynamic array index first and then compare
 * it with each constant buffer/ sampler index and replace that dynamic index
 * with static index.
 */
struct tgsi_token *
tgsi_remove_dynamic_indexing(const struct tgsi_token *tokens_in,
                             unsigned const_buffers_declared_bitmask,
                             unsigned samplers_declared_bitmask,
                             unsigned imm_count)
{
   struct dIndexing_transform_context transform;
   const unsigned num_new_tokens = 1000; /* should be enough */
   const unsigned new_len = tgsi_num_tokens(tokens_in) + num_new_tokens;

   /* setup transformation context */
   memset(&transform, 0, sizeof(transform));
   transform.base.transform_declaration = dIndexing_decl;
   transform.base.transform_instruction = dIndexing_inst;
   transform.base.prolog = dIndexing_prolog;

   transform.orig_num_tmp = 0;
   transform.orig_num_imm = imm_count;
   /* get count of declared const buffers and sampler from their bitmasks*/
   transform.num_const_bufs = log2(const_buffers_declared_bitmask + 1);
   transform.num_samplers = log2(samplers_declared_bitmask + 1);
   transform.num_iterations = 0;

   return tgsi_transform_shader(tokens_in, new_len, &transform.base);
}


