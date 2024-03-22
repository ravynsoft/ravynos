/*
 * Copyright 2014, 2015 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* the virgl hw tgsi vs what the current gallium want will diverge over time.
   so add a transform stage to remove things we don't want to send unless
   the receiver supports it.
*/

#include "tgsi/tgsi_transform.h"
#include "tgsi/tgsi_info.h"
#include "tgsi/tgsi_scan.h"
#include "virgl_context.h"
#include "virgl_screen.h"

struct virgl_input_temp {
   enum tgsi_file_type file;

   /* Index within in the INPUT or SV files, or ~0 if no DCL of this input */
   unsigned index;

   /* TGSI_FILE_TEMPORARY index it will be mapped to. */
   unsigned temp;

   bool sint;
};

enum virgl_input_temps {
   INPUT_TEMP_LAYER,
   INPUT_TEMP_VIEWPORT_INDEX,
   INPUT_TEMP_BLOCK_ID,
   INPUT_TEMP_HELPER_INVOCATION,
   INPUT_TEMP_COUNT,
};

struct virgl_transform_context {
   struct tgsi_transform_context base;
   struct tgsi_shader_info info;

   bool cull_enabled;
   bool has_precise;
   bool fake_fp64;
   bool is_separable;

   unsigned next_temp;

   unsigned src_temp;

   unsigned writemask_fixup_outs[5];
   unsigned writemask_fixup_temps;
   unsigned num_writemask_fixups;

   struct virgl_input_temp input_temp[INPUT_TEMP_COUNT];

   uint32_t *precise_flags;
};

static void
virgl_tgsi_transform_declaration_input_temp(const struct tgsi_full_declaration *decl,
                                            struct virgl_input_temp *input_temp,
                                            enum tgsi_semantic semantic_name)
{
   if (decl->Semantic.Name == semantic_name) {
      input_temp->file = decl->Declaration.File;
      input_temp->index = decl->Range.First;
   }
}

static void
virgl_tgsi_transform_declaration(struct tgsi_transform_context *ctx,
                                 struct tgsi_full_declaration *decl)
{
   struct virgl_transform_context *vtctx = (struct virgl_transform_context *)ctx;

   switch (decl->Declaration.File) {
   case TGSI_FILE_CONSTANT:
      if (decl->Declaration.Dimension) {
         if (decl->Dim.Index2D == 0)
            decl->Declaration.Dimension = 0;
      }
      break;
   case TGSI_FILE_INPUT:
      virgl_tgsi_transform_declaration_input_temp(decl, &vtctx->input_temp[INPUT_TEMP_LAYER],
                                                   TGSI_SEMANTIC_LAYER);
      virgl_tgsi_transform_declaration_input_temp(decl, &vtctx->input_temp[INPUT_TEMP_VIEWPORT_INDEX],
                                                   TGSI_SEMANTIC_VIEWPORT_INDEX);
      break;
   case TGSI_FILE_SYSTEM_VALUE:
      virgl_tgsi_transform_declaration_input_temp(decl, &vtctx->input_temp[INPUT_TEMP_BLOCK_ID],
                                                   TGSI_SEMANTIC_BLOCK_ID);
      virgl_tgsi_transform_declaration_input_temp(decl, &vtctx->input_temp[INPUT_TEMP_HELPER_INVOCATION],
                                                   TGSI_SEMANTIC_HELPER_INVOCATION);
      break;
   case TGSI_FILE_OUTPUT:
      switch (decl->Semantic.Name) {
      case TGSI_SEMANTIC_CLIPDIST:
         vtctx->writemask_fixup_outs[vtctx->num_writemask_fixups++] = decl->Range.First;
         if (decl->Range.Last != decl->Range.First)
            vtctx->writemask_fixup_outs[vtctx->num_writemask_fixups++] = decl->Range.Last;
         break;
      case TGSI_SEMANTIC_CLIPVERTEX:
         vtctx->writemask_fixup_outs[vtctx->num_writemask_fixups++] = decl->Range.First;
         break;
      case TGSI_SEMANTIC_COLOR:
         /* Vertex front/backface color output also has issues with writemasking */
         if (vtctx->base.processor != PIPE_SHADER_FRAGMENT)
            vtctx->writemask_fixup_outs[vtctx->num_writemask_fixups++] = decl->Range.First;
         break;
      }
      break;
   case TGSI_FILE_TEMPORARY:
      vtctx->next_temp = MAX2(vtctx->next_temp, decl->Range.Last + 1);
      break;
   default:
      break;
   }
   assert(vtctx->num_writemask_fixups <= ARRAY_SIZE(vtctx->writemask_fixup_outs));

   ctx->emit_declaration(ctx, decl);
}

/* for now just strip out the new properties the remote doesn't understand
   yet */
static void
virgl_tgsi_transform_property(struct tgsi_transform_context *ctx,
                              struct tgsi_full_property *prop)
{
   struct virgl_transform_context *vtctx = (struct virgl_transform_context *)ctx;
   switch (prop->Property.PropertyName) {
   case TGSI_PROPERTY_NUM_CLIPDIST_ENABLED:
   case TGSI_PROPERTY_NUM_CULLDIST_ENABLED:
      if (vtctx->cull_enabled)
    ctx->emit_property(ctx, prop);
      break;
   case TGSI_PROPERTY_NEXT_SHADER:
      break;
   default:
      ctx->emit_property(ctx, prop);
      break;
   }
}

static void
virgl_mov_input_temp_sint(struct tgsi_transform_context * ctx,
                          struct virgl_input_temp *temp)
{
   if (temp->index != ~0) {
      tgsi_transform_op2_inst(ctx, TGSI_OPCODE_IMAX,
                              TGSI_FILE_TEMPORARY, temp->temp, TGSI_WRITEMASK_XYZW,
                              temp->file, temp->index,
                              temp->file, temp->index, 0);
   }
}

static void
virgl_mov_input_temp_uint(struct tgsi_transform_context * ctx,
                          struct virgl_input_temp *temp)
{
   if (temp->index != ~0) {
      tgsi_transform_op1_inst(ctx, TGSI_OPCODE_MOV,
                              TGSI_FILE_TEMPORARY, temp->temp, TGSI_WRITEMASK_XYZW,
                              temp->file, temp->index);
   }
}

static void
virgl_tgsi_transform_prolog(struct tgsi_transform_context * ctx)
{
   struct virgl_transform_context *vtctx = (struct virgl_transform_context *)ctx;

   if (vtctx->is_separable) {
      struct tgsi_full_property prop = tgsi_default_full_property();
      prop.Property.PropertyName = TGSI_PROPERTY_SEPARABLE_PROGRAM;
      prop.Property.NrTokens += 1;
      prop.u[0].Data = 1;
      ctx->emit_property(ctx, &prop);
   }

   vtctx->src_temp = vtctx->next_temp;
   vtctx->next_temp += 4;
   tgsi_transform_temps_decl(ctx, vtctx->src_temp, vtctx->src_temp + 3);

   if (vtctx->num_writemask_fixups) {
      vtctx->writemask_fixup_temps = vtctx->next_temp;
      vtctx->next_temp += vtctx->num_writemask_fixups;
      tgsi_transform_temps_decl(ctx,
                                vtctx->writemask_fixup_temps,
                                vtctx->writemask_fixup_temps + vtctx->num_writemask_fixups - 1);
   }

   /* Assign input temps before we emit any instructions, but after we parsed
    * existing temp decls.
    */
   for (int i = 0; i < ARRAY_SIZE(vtctx->input_temp); i++) {
      if (vtctx->input_temp[i].index != ~0) {
         vtctx->input_temp[i].temp = vtctx->next_temp++;
         tgsi_transform_temp_decl(ctx, vtctx->input_temp[i].temp);
      }
   }

   /* virglrenderer makes mistakes in the types of layer/viewport input
    * references from unsigned ops, so we use a temp that we do a no-op signed
    * op to at the top of the shader.
    *
    * https://gitlab.freedesktop.org/virgl/virglrenderer/-/merge_requests/615
    */
   virgl_mov_input_temp_sint(ctx, &vtctx->input_temp[INPUT_TEMP_LAYER]);
   virgl_mov_input_temp_sint(ctx, &vtctx->input_temp[INPUT_TEMP_VIEWPORT_INDEX]);

   /* virglrenderer also makes mistakes in the types of block id input
    * references from signed ops, so we use a temp that we do a plain MOV to at
    * the top of the shader.  Also, it falls over if an unused channel's swizzle
    * uses the .w of the block id.
    */
   if (vtctx->input_temp[INPUT_TEMP_BLOCK_ID].index != ~0) {
      struct tgsi_full_instruction inst = tgsi_default_full_instruction();
      inst.Instruction.Opcode = TGSI_OPCODE_MOV;
      inst.Instruction.NumDstRegs = 1;
      inst.Dst[0].Register.File = TGSI_FILE_TEMPORARY,
      inst.Dst[0].Register.Index = vtctx->input_temp[INPUT_TEMP_BLOCK_ID].temp;
      inst.Dst[0].Register.WriteMask = TGSI_WRITEMASK_XYZ;
      inst.Instruction.NumSrcRegs = 1;
      tgsi_transform_src_reg_xyzw(&inst.Src[0],
                                  vtctx->input_temp[INPUT_TEMP_BLOCK_ID].file,
                                  vtctx->input_temp[INPUT_TEMP_BLOCK_ID].index);
      inst.Src[0].Register.SwizzleX = TGSI_SWIZZLE_X;
      inst.Src[0].Register.SwizzleY = TGSI_SWIZZLE_Y;
      inst.Src[0].Register.SwizzleZ = TGSI_SWIZZLE_Z;
      inst.Src[0].Register.SwizzleW = TGSI_SWIZZLE_Z;
      ctx->emit_instruction(ctx, &inst);
   }

   virgl_mov_input_temp_uint(ctx, &vtctx->input_temp[INPUT_TEMP_HELPER_INVOCATION]);

   vtctx->precise_flags = calloc((vtctx->next_temp + 7)/8, sizeof(uint32_t));
}

static void
virgl_tgsi_rewrite_src_for_input_temp(struct virgl_input_temp *temp, struct tgsi_full_src_register *src)
{
   if (src->Register.File == temp->file && src->Register.Index == temp->index) {
      src->Register.File = TGSI_FILE_TEMPORARY;
      src->Register.Index = temp->temp;
   }
}

static void
virgl_tgsi_transform_instruction(struct tgsi_transform_context *ctx,
             struct tgsi_full_instruction *inst)
{
   struct virgl_transform_context *vtctx = (struct virgl_transform_context *)ctx;
   if (vtctx->fake_fp64 &&
       (tgsi_opcode_infer_src_type(inst->Instruction.Opcode, 0) == TGSI_TYPE_DOUBLE ||
        tgsi_opcode_infer_dst_type(inst->Instruction.Opcode, 0) == TGSI_TYPE_DOUBLE)) {
      debug_printf("VIRGL: ARB_gpu_shader_fp64 is exposed but not supported.");
      return;
   }

   if (!vtctx->has_precise && inst->Instruction.Precise)
      inst->Instruction.Precise = 0;

   /* For outputs NTT adds a final mov op but NIR doesn't propagate precise with moves,
    * so that we don't see whether the assignment is from a precise instruction, but
    * we need to know this to set the output decoration correctly, so propagate the
    * precise flag with TGSI */
   for (int i = 0; i < inst->Instruction.NumDstRegs; ++i) {
      if (inst->Dst[i].Register.File == TGSI_FILE_TEMPORARY) {
         uint32_t index = inst->Dst[i].Register.Index / 8;
         uint32_t bits = inst->Dst[i].Register.WriteMask << (inst->Dst[i].Register.Index % 8);

         /* Since we re-use temps set and clear the precise flag according to the last use
          * for the register index and written components. Since moves are not marked
          * as precise originally, and we may end up with an if/else clause that assignes
          * a precise result in the if branche, but does a simple move from a constant
          * on the else branche, we don't clear the flag when we hit a mov.
          * We do the conservatiove approach here, because virglrenderer emits different temp
          * ranges, and we don't want to mark all temps as precise only because we have
          * one precise output */
         if (inst->Instruction.Precise)
            vtctx->precise_flags[index] |= bits;
      } else if (inst->Instruction.Opcode == TGSI_OPCODE_MOV) {
         for (int i = 0; i < inst->Instruction.NumSrcRegs; ++i) {
            if (inst->Src[i].Register.File == TGSI_FILE_TEMPORARY) {
               uint32_t index = inst->Src[i].Register.Index / 8;
               uint32_t read_mask = (1 << inst->Src[i].Register.SwizzleX) |
                                    (1 << inst->Src[i].Register.SwizzleY) |
                                    (1 << inst->Src[i].Register.SwizzleZ) |
                                    (1 << inst->Src[i].Register.SwizzleW);
               uint32_t bits = read_mask << (inst->Dst[i].Register.Index % 8);
               if (vtctx->precise_flags[index] & bits) {
                  inst->Instruction.Precise = 1;
                  break;
               }
            }
         }
      }
   }

   /* virglrenderer can run out of space in internal buffers for immediates as
    * tex operands.  Move the first immediate tex arg to a temp to save space in
    * the buffer.
    *
    * https://gitlab.freedesktop.org/virgl/virglrenderer/-/merge_requests/582
    */
   if (tgsi_get_opcode_info(inst->Instruction.Opcode)->is_tex &&
       inst->Src[0].Register.File == TGSI_FILE_IMMEDIATE) {
      tgsi_transform_op1_inst(ctx, TGSI_OPCODE_MOV,
                              TGSI_FILE_TEMPORARY, vtctx->src_temp,
                              TGSI_WRITEMASK_XYZW,
                              inst->Src[0].Register.File,
                              inst->Src[0].Register.Index);
      inst->Src[0].Register.File = TGSI_FILE_TEMPORARY;
      inst->Src[0].Register.Index = vtctx->src_temp;
   }

   for (unsigned i = 0; i < inst->Instruction.NumDstRegs; i++) {
      /* virglrenderer would fail to compile on clipdist, clipvertex, and some
       * two-sided-related color writes without a full writemask.  So, we write
       * to a temp and store that temp with a full writemask.
       *
       * https://gitlab.freedesktop.org/virgl/virglrenderer/-/merge_requests/616
       */
      if (inst->Dst[i].Register.File == TGSI_FILE_OUTPUT) {
         for (int j = 0; j < vtctx->num_writemask_fixups; j++) {
            if (inst->Dst[i].Register.Index == vtctx->writemask_fixup_outs[j]) {
               inst->Dst[i].Register.File = TGSI_FILE_TEMPORARY;
               inst->Dst[i].Register.Index = vtctx->writemask_fixup_temps + j;
               break;
            }
         }
      }
   }

   for (unsigned i = 0; i < inst->Instruction.NumSrcRegs; i++) {
      if (inst->Src[i].Register.File == TGSI_FILE_CONSTANT &&
          inst->Src[i].Register.Dimension &&
          inst->Src[i].Dimension.Index == 0)
         inst->Src[i].Register.Dimension = 0;

      for (int j = 0; j < ARRAY_SIZE(vtctx->input_temp); j++)
         virgl_tgsi_rewrite_src_for_input_temp(&vtctx->input_temp[j], &inst->Src[i]);

      /* virglrenderer double inputs twice, so move them to temps and drop the
       * swizzle from the double op.
       */
      if (tgsi_opcode_infer_src_type(inst->Instruction.Opcode, i) == TGSI_TYPE_DOUBLE) {
         struct tgsi_full_instruction temp_inst = tgsi_default_full_instruction();
         temp_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
         temp_inst.Instruction.NumDstRegs = 1;
         temp_inst.Dst[0].Register.File = TGSI_FILE_TEMPORARY,
         temp_inst.Dst[0].Register.Index = vtctx->src_temp + i;
         temp_inst.Dst[0].Register.WriteMask = TGSI_WRITEMASK_XY;
         temp_inst.Instruction.NumSrcRegs = 1;
         memcpy(&temp_inst.Src[0], &inst->Src[i], sizeof(temp_inst.Src[0]));
         temp_inst.Src[0].Register.SwizzleX = inst->Src[i].Register.SwizzleX;
         temp_inst.Src[0].Register.SwizzleY = inst->Src[i].Register.SwizzleY;
         temp_inst.Src[0].Register.SwizzleZ = inst->Src[i].Register.SwizzleZ;
         temp_inst.Src[0].Register.SwizzleW = inst->Src[i].Register.SwizzleW;
         ctx->emit_instruction(ctx, &temp_inst);

         memset(&inst->Src[i], 0, sizeof(inst->Src[i]));
         inst->Src[i].Register.File = TGSI_FILE_TEMPORARY;
         inst->Src[i].Register.Index = vtctx->src_temp + i;
         inst->Src[i].Register.SwizzleX = TGSI_SWIZZLE_X;
         inst->Src[i].Register.SwizzleY = TGSI_SWIZZLE_Y;
         inst->Src[i].Register.SwizzleZ = TGSI_SWIZZLE_Z;
         inst->Src[i].Register.SwizzleW = TGSI_SWIZZLE_W;
      }
   }

   /* virglrenderer doesn't resolve non-float output write properly,
    * so we have to first write to a temporary */
   if (inst->Instruction.Opcode != TGSI_OPCODE_MOV &&
       !tgsi_get_opcode_info(inst->Instruction.Opcode)->is_tex &&
       !tgsi_get_opcode_info(inst->Instruction.Opcode)->is_store &&
       inst->Dst[0].Register.File == TGSI_FILE_OUTPUT &&
       tgsi_opcode_infer_dst_type(inst->Instruction.Opcode, 0) != TGSI_TYPE_FLOAT)  {
      struct tgsi_full_instruction op_to_temp = *inst;
      op_to_temp.Dst[0].Register.File = TGSI_FILE_TEMPORARY;
      op_to_temp.Dst[0].Register.Index = vtctx->src_temp;
      op_to_temp.Dst[0].Dimension.Indirect = 0;
      op_to_temp.Dst[0].Register.Indirect = 0;
      ctx->emit_instruction(ctx, &op_to_temp);

      inst->Instruction.Opcode = TGSI_OPCODE_MOV;
      inst->Instruction.NumSrcRegs = 1;

      memset(&inst->Src[0], 0, sizeof(inst->Src[0]));
      inst->Src[0].Register.File = TGSI_FILE_TEMPORARY;
      inst->Src[0].Register.Index = vtctx->src_temp;
      inst->Src[0].Register.SwizzleY = 1;
      inst->Src[0].Register.SwizzleZ = 2;
      inst->Src[0].Register.SwizzleW = 3;
   }

   ctx->emit_instruction(ctx, inst);

   for (unsigned i = 0; i < inst->Instruction.NumDstRegs; i++) {
      if (vtctx->num_writemask_fixups &&
         inst->Dst[i].Register.File == TGSI_FILE_TEMPORARY &&
         inst->Dst[i].Register.Index >= vtctx->writemask_fixup_temps &&
         inst->Dst[i].Register.Index < vtctx->writemask_fixup_temps + vtctx->num_writemask_fixups) {
         /* Emit the fixup MOV from the clipdist/vert temporary to the real output. */
         unsigned real_out = vtctx->writemask_fixup_outs[inst->Dst[i].Register.Index - vtctx->writemask_fixup_temps];
         tgsi_transform_op1_inst(ctx, TGSI_OPCODE_MOV,
                                 TGSI_FILE_OUTPUT, real_out, TGSI_WRITEMASK_XYZW,
                                 inst->Dst[i].Register.File, inst->Dst[i].Register.Index);
      }
   }
}

struct tgsi_token *virgl_tgsi_transform(struct virgl_screen *vscreen, const struct tgsi_token *tokens_in,
                                        bool is_separable)
{
   struct virgl_transform_context transform;
   const uint newLen = tgsi_num_tokens(tokens_in);

   memset(&transform, 0, sizeof(transform));
   transform.base.transform_declaration = virgl_tgsi_transform_declaration;
   transform.base.transform_property = virgl_tgsi_transform_property;
   transform.base.transform_instruction = virgl_tgsi_transform_instruction;
   transform.base.prolog = virgl_tgsi_transform_prolog;
   transform.cull_enabled = vscreen->caps.caps.v1.bset.has_cull;
   transform.has_precise = vscreen->caps.caps.v2.capability_bits & VIRGL_CAP_TGSI_PRECISE;
   transform.fake_fp64 =
      vscreen->caps.caps.v2.capability_bits & VIRGL_CAP_HOST_IS_GLES;
   transform.is_separable = is_separable && (vscreen->caps.caps.v2.capability_bits_v2 & VIRGL_CAP_V2_SSO);

   for (int i = 0; i < ARRAY_SIZE(transform.input_temp); i++)
      transform.input_temp[i].index = ~0;

   tgsi_scan_shader(tokens_in, &transform.info);

   struct tgsi_token *new_tokens = tgsi_transform_shader(tokens_in, newLen, &transform.base);
   free(transform.precise_flags);
   return new_tokens;

}
