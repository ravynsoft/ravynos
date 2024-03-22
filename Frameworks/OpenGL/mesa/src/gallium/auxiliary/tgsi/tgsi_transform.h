/**************************************************************************
 * 
 * Copyright 2008 VMware, Inc.
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

#ifndef TGSI_TRANSFORM_H
#define TGSI_TRANSFORM_H


#include "pipe/p_defines.h"
#include "pipe/p_shader_tokens.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_build.h"



/**
 * Subclass this to add caller-specific data
 */
struct tgsi_transform_context
{
/**** PUBLIC ***/

   /**
    * User-defined callbacks invoked per instruction.
    */
   void (*transform_instruction)(struct tgsi_transform_context *ctx,
                                 struct tgsi_full_instruction *inst);

   void (*transform_declaration)(struct tgsi_transform_context *ctx,
                                 struct tgsi_full_declaration *decl);

   void (*transform_immediate)(struct tgsi_transform_context *ctx,
                               struct tgsi_full_immediate *imm);
   void (*transform_property)(struct tgsi_transform_context *ctx,
                              struct tgsi_full_property *prop);

   /**
    * Called after last declaration, before first instruction.  This is
    * where the user might insert new declarations and/or instructions.
    */
   void (*prolog)(struct tgsi_transform_context *ctx);

   /**
    * Called at end of input program to allow caller to append extra
    * instructions.
    */
   void (*epilog)(struct tgsi_transform_context *ctx);

   enum pipe_shader_type processor;

/*** PRIVATE ***/

   /**
    * These are setup by tgsi_transform_shader() and cannot be overridden.
    * Meant to be called from in the above user callback functions.
    */
   void (*emit_instruction)(struct tgsi_transform_context *ctx,
                            const struct tgsi_full_instruction *inst);
   void (*emit_declaration)(struct tgsi_transform_context *ctx,
                            const struct tgsi_full_declaration *decl);
   void (*emit_immediate)(struct tgsi_transform_context *ctx,
                          const struct tgsi_full_immediate *imm);
   void (*emit_property)(struct tgsi_transform_context *ctx,
                         const struct tgsi_full_property *prop);

   struct tgsi_header *header;
   unsigned max_tokens_out;
   struct tgsi_token *tokens_out;
   unsigned ti;
   bool fail;
};


/**
 * Helper for emitting temporary register declarations.
 */
static inline void
tgsi_transform_temps_decl(struct tgsi_transform_context *ctx,
                          unsigned firstIdx, unsigned lastIdx)
{
   struct tgsi_full_declaration decl;

   decl = tgsi_default_full_declaration();
   decl.Declaration.File = TGSI_FILE_TEMPORARY;
   decl.Range.First = firstIdx;
   decl.Range.Last = lastIdx;
   ctx->emit_declaration(ctx, &decl);
}

static inline void
tgsi_transform_temp_decl(struct tgsi_transform_context *ctx,
                         unsigned index)
{
   tgsi_transform_temps_decl(ctx, index, index);
}

static inline void
tgsi_transform_const_decl(struct tgsi_transform_context *ctx,
                          unsigned firstIdx, unsigned lastIdx)
{
   struct tgsi_full_declaration decl;

   decl = tgsi_default_full_declaration();
   decl.Declaration.File = TGSI_FILE_CONSTANT;
   decl.Range.First = firstIdx;
   decl.Range.Last = lastIdx;
   decl.Declaration.Dimension = 1;
   /* Dim.Index2D is already 0 */
   ctx->emit_declaration(ctx, &decl);
}
 
static inline void
tgsi_transform_input_decl(struct tgsi_transform_context *ctx,
                          unsigned index,
                          unsigned sem_name, unsigned sem_index,
                          unsigned interp)
{
   struct tgsi_full_declaration decl;

   decl = tgsi_default_full_declaration();
   decl.Declaration.File = TGSI_FILE_INPUT;
   decl.Declaration.Interpolate = 1;
   decl.Declaration.Semantic = 1;
   decl.Semantic.Name = sem_name;
   decl.Semantic.Index = sem_index;
   decl.Range.First =
   decl.Range.Last = index;
   decl.Interp.Interpolate = interp;

   ctx->emit_declaration(ctx, &decl);
}

static inline void
tgsi_transform_output_decl(struct tgsi_transform_context *ctx,
                          unsigned index,
                          unsigned sem_name, unsigned sem_index,
                          unsigned interp)
{
   struct tgsi_full_declaration decl;

   decl = tgsi_default_full_declaration();
   decl.Declaration.File = TGSI_FILE_OUTPUT;
   decl.Declaration.Interpolate = 1;
   decl.Declaration.Semantic = 1;
   decl.Semantic.Name = sem_name;
   decl.Semantic.Index = sem_index;
   decl.Range.First =
   decl.Range.Last = index;
   decl.Interp.Interpolate = interp;

   ctx->emit_declaration(ctx, &decl);
}

static inline void
tgsi_transform_sampler_decl(struct tgsi_transform_context *ctx,
                            unsigned index)
{
   struct tgsi_full_declaration decl;

   decl = tgsi_default_full_declaration();
   decl.Declaration.File = TGSI_FILE_SAMPLER;
   decl.Range.First =
   decl.Range.Last = index;
   ctx->emit_declaration(ctx, &decl);
}

static inline void
tgsi_transform_sampler_view_decl(struct tgsi_transform_context *ctx,
                                 unsigned index,
                                 unsigned target,
                                 enum tgsi_return_type type)
{
   struct tgsi_full_declaration decl;

   decl = tgsi_default_full_declaration();
   decl.Declaration.File = TGSI_FILE_SAMPLER_VIEW;
   decl.Declaration.UsageMask = TGSI_WRITEMASK_XYZW;
   decl.Range.First =
   decl.Range.Last = index;
   decl.SamplerView.Resource = target;
   decl.SamplerView.ReturnTypeX = type;
   decl.SamplerView.ReturnTypeY = type;
   decl.SamplerView.ReturnTypeZ = type;
   decl.SamplerView.ReturnTypeW = type;

   ctx->emit_declaration(ctx, &decl);
}

static inline void
tgsi_transform_immediate_decl(struct tgsi_transform_context *ctx,
                              float x, float y, float z, float w)
{
   struct tgsi_full_immediate immed;
   unsigned size = 4;

   immed = tgsi_default_full_immediate();
   immed.Immediate.NrTokens = 1 + size; /* one for the token itself */
   immed.u[0].Float = x;
   immed.u[1].Float = y;
   immed.u[2].Float = z;
   immed.u[3].Float = w;

   ctx->emit_immediate(ctx, &immed);
}

static inline void
tgsi_transform_immediate_int_decl(struct tgsi_transform_context *ctx,
                                  int x, int y, int z, int w)
{
   struct tgsi_full_immediate immed;
   unsigned size = 4;

   immed = tgsi_default_full_immediate();
   immed.Immediate.DataType = TGSI_IMM_INT32;
   immed.Immediate.NrTokens = 1 + size; /* one for the token itself */
   immed.u[0].Int = x;
   immed.u[1].Int = y;
   immed.u[2].Int = z;
   immed.u[3].Int = w;

   ctx->emit_immediate(ctx, &immed);
}

static inline void
tgsi_transform_dst_reg(struct tgsi_full_dst_register *reg,
                       unsigned file, unsigned index, unsigned writemask)
{
   reg->Register.File = file;
   reg->Register.Index = index;
   reg->Register.WriteMask = writemask;
}

static inline void
tgsi_transform_src_reg_xyzw(struct tgsi_full_src_register *reg,
                            unsigned file, unsigned index)
{
   reg->Register.File = file;
   reg->Register.Index = index;
   if (file == TGSI_FILE_CONSTANT) {
      reg->Register.Dimension = 1;
      reg->Dimension.Index = 0;
   }
}

static inline void
tgsi_transform_src_reg(struct tgsi_full_src_register *reg,
                       unsigned file, unsigned index, 
                       unsigned swizzleX, unsigned swizzleY,
                       unsigned swizzleZ, unsigned swizzleW)
{
   reg->Register.File = file;
   reg->Register.Index = index;
   if (file == TGSI_FILE_CONSTANT) {
      reg->Register.Dimension = 1;
      reg->Dimension.Index = 0;
   }
   reg->Register.SwizzleX = swizzleX;
   reg->Register.SwizzleY = swizzleY; 
   reg->Register.SwizzleZ = swizzleZ; 
   reg->Register.SwizzleW = swizzleW; 
}

/**
 * Helper for emitting 1-operand instructions.
 */
static inline void
tgsi_transform_op1_inst(struct tgsi_transform_context *ctx,
                        enum tgsi_opcode opcode,
                        unsigned dst_file,
                        unsigned dst_index,
                        unsigned dst_writemask,
                        unsigned src0_file,
                        unsigned src0_index)
{
   struct tgsi_full_instruction inst;

   inst = tgsi_default_full_instruction();
   inst.Instruction.Opcode = opcode;
   inst.Instruction.NumDstRegs = 1;
   inst.Dst[0].Register.File = dst_file,
   inst.Dst[0].Register.Index = dst_index;
   inst.Dst[0].Register.WriteMask = dst_writemask;
   inst.Instruction.NumSrcRegs = 1;
   tgsi_transform_src_reg_xyzw(&inst.Src[0], src0_file, src0_index);

   ctx->emit_instruction(ctx, &inst);
}


static inline void
tgsi_transform_op2_inst(struct tgsi_transform_context *ctx,
                        enum tgsi_opcode opcode,
                        unsigned dst_file,
                        unsigned dst_index,
                        unsigned dst_writemask,
                        unsigned src0_file,
                        unsigned src0_index,
                        unsigned src1_file,
                        unsigned src1_index,
                        bool src1_negate)
{
   struct tgsi_full_instruction inst;

   inst = tgsi_default_full_instruction();
   inst.Instruction.Opcode = opcode;
   inst.Instruction.NumDstRegs = 1;
   inst.Dst[0].Register.File = dst_file,
   inst.Dst[0].Register.Index = dst_index;
   inst.Dst[0].Register.WriteMask = dst_writemask;
   inst.Instruction.NumSrcRegs = 2;
   tgsi_transform_src_reg_xyzw(&inst.Src[0], src0_file, src0_index);
   tgsi_transform_src_reg_xyzw(&inst.Src[1], src1_file, src1_index);
   inst.Src[1].Register.Negate = src1_negate;

   ctx->emit_instruction(ctx, &inst);
}


static inline void
tgsi_transform_op1_swz_inst(struct tgsi_transform_context *ctx,
                            enum tgsi_opcode opcode,
                            unsigned dst_file,
                            unsigned dst_index,
                            unsigned dst_writemask,
                            unsigned src0_file,
                            unsigned src0_index,
                            unsigned src0_swizzle)
{
   struct tgsi_full_instruction inst;

   inst = tgsi_default_full_instruction();
   inst.Instruction.Opcode = opcode;
   inst.Instruction.NumDstRegs = 1;
   inst.Dst[0].Register.File = dst_file,
   inst.Dst[0].Register.Index = dst_index;
   inst.Dst[0].Register.WriteMask = dst_writemask;
   inst.Instruction.NumSrcRegs = 1;
   tgsi_transform_src_reg_xyzw(&inst.Src[0], src0_file, src0_index);
   switch (dst_writemask) {
   case TGSI_WRITEMASK_X:
      inst.Src[0].Register.SwizzleX = src0_swizzle;
      break;
   case TGSI_WRITEMASK_Y:
      inst.Src[0].Register.SwizzleY = src0_swizzle;
      break;
   case TGSI_WRITEMASK_Z:
      inst.Src[0].Register.SwizzleZ = src0_swizzle;
      break;
   case TGSI_WRITEMASK_W:
      inst.Src[0].Register.SwizzleW = src0_swizzle;
      break;
   default:
      ; /* nothing */
   }

   ctx->emit_instruction(ctx, &inst);
}


static inline void
tgsi_transform_op2_swz_inst(struct tgsi_transform_context *ctx,
                            enum tgsi_opcode opcode,
                            unsigned dst_file,
                            unsigned dst_index,
                            unsigned dst_writemask,
                            unsigned src0_file,
                            unsigned src0_index,
                            unsigned src0_swizzle,
                            unsigned src1_file,
                            unsigned src1_index,
                            unsigned src1_swizzle,
                            bool src1_negate)
{
   struct tgsi_full_instruction inst;

   inst = tgsi_default_full_instruction();
   inst.Instruction.Opcode = opcode;
   inst.Instruction.NumDstRegs = 1;
   inst.Dst[0].Register.File = dst_file,
   inst.Dst[0].Register.Index = dst_index;
   inst.Dst[0].Register.WriteMask = dst_writemask;
   inst.Instruction.NumSrcRegs = 2;
   tgsi_transform_src_reg_xyzw(&inst.Src[0], src0_file, src0_index);
   tgsi_transform_src_reg_xyzw(&inst.Src[1], src1_file, src1_index);
   inst.Src[1].Register.Negate = src1_negate;
   switch (dst_writemask) {
   case TGSI_WRITEMASK_X:
      inst.Src[0].Register.SwizzleX = src0_swizzle;
      inst.Src[1].Register.SwizzleX = src1_swizzle;
      break;
   case TGSI_WRITEMASK_Y:
      inst.Src[0].Register.SwizzleY = src0_swizzle;
      inst.Src[1].Register.SwizzleY = src1_swizzle;
      break;
   case TGSI_WRITEMASK_Z:
      inst.Src[0].Register.SwizzleZ = src0_swizzle;
      inst.Src[1].Register.SwizzleZ = src1_swizzle;
      break;
   case TGSI_WRITEMASK_W:
      inst.Src[0].Register.SwizzleW = src0_swizzle;
      inst.Src[1].Register.SwizzleW = src1_swizzle;
      break;
   default:
      ; /* nothing */
   }

   ctx->emit_instruction(ctx, &inst);
}


static inline void
tgsi_transform_op3_swz_inst(struct tgsi_transform_context *ctx,
                            enum tgsi_opcode opcode,
                            unsigned dst_file,
                            unsigned dst_index,
                            unsigned dst_writemask,
                            unsigned src0_file,
                            unsigned src0_index,
                            unsigned src0_swizzle,
                            unsigned src0_negate,
                            unsigned src1_file,
                            unsigned src1_index,
                            unsigned src1_swizzle,
                            unsigned src2_file,
                            unsigned src2_index,
                            unsigned src2_swizzle)
{
   struct tgsi_full_instruction inst;

   inst = tgsi_default_full_instruction();
   inst.Instruction.Opcode = opcode;
   inst.Instruction.NumDstRegs = 1;
   inst.Dst[0].Register.File = dst_file,
   inst.Dst[0].Register.Index = dst_index;
   inst.Dst[0].Register.WriteMask = dst_writemask;
   inst.Instruction.NumSrcRegs = 3;
   tgsi_transform_src_reg_xyzw(&inst.Src[0], src0_file, src0_index);
   inst.Src[0].Register.Negate = src0_negate;
   tgsi_transform_src_reg_xyzw(&inst.Src[1], src1_file, src1_index);
   tgsi_transform_src_reg_xyzw(&inst.Src[2], src2_file, src2_index);
   switch (dst_writemask) {
   case TGSI_WRITEMASK_X:
      inst.Src[0].Register.SwizzleX = src0_swizzle;
      inst.Src[1].Register.SwizzleX = src1_swizzle;
      inst.Src[2].Register.SwizzleX = src2_swizzle;
      break;
   case TGSI_WRITEMASK_Y:
      inst.Src[0].Register.SwizzleY = src0_swizzle;
      inst.Src[1].Register.SwizzleY = src1_swizzle;
      inst.Src[2].Register.SwizzleY = src2_swizzle;
      break;
   case TGSI_WRITEMASK_Z:
      inst.Src[0].Register.SwizzleZ = src0_swizzle;
      inst.Src[1].Register.SwizzleZ = src1_swizzle;
      inst.Src[2].Register.SwizzleZ = src2_swizzle;
      break;
   case TGSI_WRITEMASK_W:
      inst.Src[0].Register.SwizzleW = src0_swizzle;
      inst.Src[1].Register.SwizzleW = src1_swizzle;
      inst.Src[2].Register.SwizzleW = src2_swizzle;
      break;
   default:
      ; /* nothing */
   }

   ctx->emit_instruction(ctx, &inst);
}


static inline void
tgsi_transform_kill_inst(struct tgsi_transform_context *ctx,
                         unsigned src_file,
                         unsigned src_index,
                         unsigned src_swizzle,
                         bool negate)
{
   struct tgsi_full_instruction inst;

   inst = tgsi_default_full_instruction();
   inst.Instruction.Opcode = TGSI_OPCODE_KILL_IF;
   inst.Instruction.NumDstRegs = 0;
   inst.Instruction.NumSrcRegs = 1;
   tgsi_transform_src_reg_xyzw(&inst.Src[0], src_file, src_index);
   inst.Src[0].Register.SwizzleX =
   inst.Src[0].Register.SwizzleY =
   inst.Src[0].Register.SwizzleZ =
   inst.Src[0].Register.SwizzleW = src_swizzle;
   inst.Src[0].Register.Negate = negate;

   ctx->emit_instruction(ctx, &inst);
}


static inline void
tgsi_transform_tex_inst(struct tgsi_transform_context *ctx,
                        unsigned dst_file,
                        unsigned dst_index,
                        unsigned src_file,
                        unsigned src_index,
                        unsigned tex_target,
                        unsigned sampler_index)
{
   struct tgsi_full_instruction inst;

   assert(tex_target < TGSI_TEXTURE_COUNT);

   inst = tgsi_default_full_instruction();
   inst.Instruction.Opcode = TGSI_OPCODE_TEX;
   inst.Instruction.NumDstRegs = 1;
   inst.Dst[0].Register.File = dst_file;
   inst.Dst[0].Register.Index = dst_index;
   inst.Instruction.NumSrcRegs = 2;
   inst.Instruction.Texture = true;
   inst.Texture.Texture = tex_target;
   tgsi_transform_src_reg_xyzw(&inst.Src[0], src_file, src_index);
   tgsi_transform_src_reg_xyzw(&inst.Src[1], TGSI_FILE_SAMPLER, sampler_index);

   ctx->emit_instruction(ctx, &inst);
}


extern struct tgsi_token *
tgsi_transform_shader(const struct tgsi_token *tokens_in,
                      unsigned initial_tokens_len,
                      struct tgsi_transform_context *ctx);


#endif /* TGSI_TRANSFORM_H */
