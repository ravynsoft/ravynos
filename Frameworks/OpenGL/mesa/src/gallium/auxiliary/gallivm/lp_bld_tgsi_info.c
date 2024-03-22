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


#include "util/compiler.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_util.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_strings.h"
#include "lp_bld_debug.h"
#include "lp_bld_tgsi.h"


/**
 * Analysis context.
 *
 * This is where we keep store the value of each channel of the IMM/TEMP/OUT
 * register values, as we walk the shader.
 */
struct analysis_context
{
   struct lp_tgsi_info *info;

   unsigned num_imms;
   float imm[LP_MAX_TGSI_IMMEDIATES][4];
   unsigned sample_target[PIPE_MAX_SHADER_SAMPLER_VIEWS];

   struct lp_tgsi_channel_info temp[32][4];
};


/**
 * Describe the specified channel of the src register.
 */
static void
analyse_src(struct analysis_context *ctx,
            struct lp_tgsi_channel_info *chan_info,
            const struct tgsi_src_register *src,
            unsigned chan)
{
   chan_info->file = TGSI_FILE_NULL;
   if (!src->Indirect && !src->Absolute && !src->Negate) {
      unsigned swizzle = tgsi_util_get_src_register_swizzle(src, chan);
      if (src->File == TGSI_FILE_TEMPORARY) {
         if (src->Index < ARRAY_SIZE(ctx->temp)) {
            *chan_info = ctx->temp[src->Index][swizzle];
         }
      } else {
         chan_info->file = src->File;
         if (src->File == TGSI_FILE_IMMEDIATE) {
            assert(src->Index < ARRAY_SIZE(ctx->imm));
            if (src->Index < ARRAY_SIZE(ctx->imm)) {
               chan_info->u.value = ctx->imm[src->Index][swizzle];
            }
         } else {
            chan_info->u.index = src->Index;
            chan_info->swizzle = swizzle;
         }
      }
   }
}


/**
 * Whether this register channel refers to a specific immediate value.
 */
static bool
is_immediate(const struct lp_tgsi_channel_info *chan_info, float value)
{
   return chan_info->file == TGSI_FILE_IMMEDIATE &&
          chan_info->u.value == value;
}


/**
 * Analyse properties of tex instructions, in particular used
 * to figure out if a texture is considered indirect.
 * Not actually used by much except the tgsi dumping code.
 */
static void
analyse_tex(struct analysis_context *ctx,
            const struct tgsi_full_instruction *inst,
            enum lp_build_tex_modifier modifier)
{
   struct lp_tgsi_info *info = ctx->info;
   unsigned chan;

   if (info->num_texs < ARRAY_SIZE(info->tex)) {
      struct lp_tgsi_texture_info *tex_info = &info->tex[info->num_texs];
      bool indirect = false;
      unsigned readmask = 0;

      tex_info->target = inst->Texture.Texture;
      switch (inst->Texture.Texture) {
      case TGSI_TEXTURE_1D:
         readmask = TGSI_WRITEMASK_X;
         break;
      case TGSI_TEXTURE_1D_ARRAY:
      case TGSI_TEXTURE_2D:
      case TGSI_TEXTURE_RECT:
         readmask = TGSI_WRITEMASK_XY;
         break;
      case TGSI_TEXTURE_SHADOW1D:
      case TGSI_TEXTURE_SHADOW1D_ARRAY:
      case TGSI_TEXTURE_SHADOW2D:
      case TGSI_TEXTURE_SHADOWRECT:
      case TGSI_TEXTURE_2D_ARRAY:
      case TGSI_TEXTURE_2D_MSAA:
      case TGSI_TEXTURE_3D:
      case TGSI_TEXTURE_CUBE:
         readmask = TGSI_WRITEMASK_XYZ;
         break;
      case TGSI_TEXTURE_SHADOW2D_ARRAY:
      case TGSI_TEXTURE_SHADOWCUBE:
      case TGSI_TEXTURE_2D_ARRAY_MSAA:
      case TGSI_TEXTURE_CUBE_ARRAY:
         readmask = TGSI_WRITEMASK_XYZW;
         /* modifier would be in another not analyzed reg so just say indirect */
         if (modifier != LP_BLD_TEX_MODIFIER_NONE) {
            indirect = true;
         }
         break;
      case TGSI_TEXTURE_SHADOWCUBE_ARRAY:
         readmask = TGSI_WRITEMASK_XYZW;
         indirect = true;
         break;
      default:
         assert(0);
         return;
      }

      if (modifier == LP_BLD_TEX_MODIFIER_EXPLICIT_DERIV) {
         /* We don't track explicit derivatives, although we could */
         indirect = true;
         tex_info->sampler_unit = inst->Src[3].Register.Index;
         tex_info->texture_unit = inst->Src[3].Register.Index;
      }  else {
         if (modifier == LP_BLD_TEX_MODIFIER_PROJECTED ||
             modifier == LP_BLD_TEX_MODIFIER_LOD_BIAS ||
             modifier == LP_BLD_TEX_MODIFIER_EXPLICIT_LOD) {
            readmask |= TGSI_WRITEMASK_W;
         }
         tex_info->sampler_unit = inst->Src[1].Register.Index;
         tex_info->texture_unit = inst->Src[1].Register.Index;
      }

      for (chan = 0; chan < 4; ++chan) {
         struct lp_tgsi_channel_info *chan_info = &tex_info->coord[chan];
         if (readmask & (1 << chan)) {
            analyse_src(ctx, chan_info, &inst->Src[0].Register, chan);
            if (chan_info->file != TGSI_FILE_INPUT) {
               indirect = true;
            }
         } else {
            memset(chan_info, 0, sizeof *chan_info);
         }
      }

      if (indirect) {
         info->indirect_textures = true;
      }

      ++info->num_texs;
   } else {
      info->indirect_textures = true;
   }
}


/**
 * Analyse properties of sample instructions, in particular used
 * to figure out if a texture is considered indirect.
 * Not actually used by much except the tgsi dumping code.
 */
static void
analyse_sample(struct analysis_context *ctx,
               const struct tgsi_full_instruction *inst,
               enum lp_build_tex_modifier modifier,
               bool shadow)
{
   struct lp_tgsi_info *info = ctx->info;
   unsigned chan;

   if (info->num_texs < ARRAY_SIZE(info->tex)) {
      struct lp_tgsi_texture_info *tex_info = &info->tex[info->num_texs];
      unsigned target = ctx->sample_target[inst->Src[1].Register.Index];
      bool indirect = false;
      bool shadow = false;
      unsigned readmask;

      switch (target) {
      /* note no shadow targets here */
      case TGSI_TEXTURE_BUFFER:
      case TGSI_TEXTURE_1D:
         readmask = TGSI_WRITEMASK_X;
         break;
      case TGSI_TEXTURE_1D_ARRAY:
      case TGSI_TEXTURE_2D:
      case TGSI_TEXTURE_RECT:
         readmask = TGSI_WRITEMASK_XY;
         break;
      case TGSI_TEXTURE_2D_ARRAY:
      case TGSI_TEXTURE_2D_MSAA:
      case TGSI_TEXTURE_3D:
      case TGSI_TEXTURE_CUBE:
         readmask = TGSI_WRITEMASK_XYZ;
         break;
      case TGSI_TEXTURE_CUBE_ARRAY:
      case TGSI_TEXTURE_2D_ARRAY_MSAA:
         readmask = TGSI_WRITEMASK_XYZW;
         break;
      default:
         assert(0);
         return;
      }

      tex_info->target = target;
      tex_info->texture_unit = inst->Src[1].Register.Index;
      tex_info->sampler_unit = inst->Src[2].Register.Index;

      if (tex_info->texture_unit != tex_info->sampler_unit) {
         info->sampler_texture_units_different = true;
      }

      if (modifier == LP_BLD_TEX_MODIFIER_EXPLICIT_DERIV ||
          modifier == LP_BLD_TEX_MODIFIER_EXPLICIT_LOD ||
          modifier == LP_BLD_TEX_MODIFIER_LOD_BIAS || shadow) {
         /* We don't track insts with additional regs, although we could */
         indirect = true;
      }

      for (chan = 0; chan < 4; ++chan) {
         struct lp_tgsi_channel_info *chan_info = &tex_info->coord[chan];
         if (readmask & (1 << chan)) {
            analyse_src(ctx, chan_info, &inst->Src[0].Register, chan);
            if (chan_info->file != TGSI_FILE_INPUT) {
               indirect = true;
            }
         } else {
            memset(chan_info, 0, sizeof *chan_info);
         }
      }

      if (indirect) {
         info->indirect_textures = true;
      }

      ++info->num_texs;
   } else {
      info->indirect_textures = true;
   }
}


/**
 * Process an instruction, and update the register values accordingly.
 */
static void
analyse_instruction(struct analysis_context *ctx,
                    struct tgsi_full_instruction *inst)
{
   struct lp_tgsi_info *info = ctx->info;
   struct lp_tgsi_channel_info (*regs)[4];
   unsigned max_regs;
   unsigned i;
   unsigned index;
   unsigned chan;

   for (i = 0; i < inst->Instruction.NumDstRegs; ++i) {
      const struct tgsi_dst_register *dst = &inst->Dst[i].Register;

      /*
       * Get the lp_tgsi_channel_info array corresponding to the destination
       * register file.
       */

      if (dst->File == TGSI_FILE_TEMPORARY) {
         regs = ctx->temp;
         max_regs = ARRAY_SIZE(ctx->temp);
      } else if (dst->File == TGSI_FILE_OUTPUT) {
         regs = info->output;
         max_regs = ARRAY_SIZE(info->output);
      } else if (dst->File == TGSI_FILE_ADDRESS) {
         continue;
      } else if (dst->File == TGSI_FILE_BUFFER) {
         continue;
      } else if (dst->File == TGSI_FILE_IMAGE) {
         continue;
      } else if (dst->File == TGSI_FILE_MEMORY) {
         continue;
      } else {
         assert(0);
         continue;
      }

      /*
       * Detect direct TEX instructions
       */

      switch (inst->Instruction.Opcode) {
      case TGSI_OPCODE_TEX:
         analyse_tex(ctx, inst, LP_BLD_TEX_MODIFIER_NONE);
         break;
      case TGSI_OPCODE_TXD:
         analyse_tex(ctx, inst, LP_BLD_TEX_MODIFIER_EXPLICIT_DERIV);
         break;
      case TGSI_OPCODE_TXB:
         analyse_tex(ctx, inst, LP_BLD_TEX_MODIFIER_LOD_BIAS);
         break;
      case TGSI_OPCODE_TXL:
         analyse_tex(ctx, inst, LP_BLD_TEX_MODIFIER_EXPLICIT_LOD);
         break;
      case TGSI_OPCODE_TXP:
         analyse_tex(ctx, inst, LP_BLD_TEX_MODIFIER_PROJECTED);
         break;
      case TGSI_OPCODE_TEX2:
         analyse_tex(ctx, inst, LP_BLD_TEX_MODIFIER_NONE);
         break;
      case TGSI_OPCODE_TXB2:
         analyse_tex(ctx, inst, LP_BLD_TEX_MODIFIER_LOD_BIAS);
         break;
      case TGSI_OPCODE_TXL2:
         analyse_tex(ctx, inst, LP_BLD_TEX_MODIFIER_EXPLICIT_LOD);
         break;
      case TGSI_OPCODE_SAMPLE:
         analyse_sample(ctx, inst, LP_BLD_TEX_MODIFIER_NONE, false);
         break;
      case TGSI_OPCODE_SAMPLE_C:
         analyse_sample(ctx, inst, LP_BLD_TEX_MODIFIER_NONE, true);
         break;
      case TGSI_OPCODE_SAMPLE_C_LZ:
         analyse_sample(ctx, inst, LP_BLD_TEX_MODIFIER_LOD_ZERO, true);
         break;
      case TGSI_OPCODE_SAMPLE_D:
         analyse_sample(ctx, inst, LP_BLD_TEX_MODIFIER_EXPLICIT_DERIV, false);
         break;
      case TGSI_OPCODE_SAMPLE_B:
         analyse_sample(ctx, inst, LP_BLD_TEX_MODIFIER_LOD_BIAS, false);
         break;
      case TGSI_OPCODE_SAMPLE_L:
         analyse_sample(ctx, inst, LP_BLD_TEX_MODIFIER_EXPLICIT_LOD, false);
         break;
      default:
         break;
      }

      /*
       * Keep track of assignments and writes
       */

      if (dst->Indirect) {
         /*
          * It could be any register index so clear all register indices.
          */

         for (chan = 0; chan < 4; ++chan) {
            if (dst->WriteMask & (1 << chan)) {
               for (index = 0; index < max_regs; ++index) {
                  regs[index][chan].file = TGSI_FILE_NULL;
               }
            }
         }
      } else if (dst->Index < max_regs) {
         /*
          * Update this destination register value.
          */

         struct lp_tgsi_channel_info res[4];

         memset(res, 0, sizeof res);

         if (!inst->Instruction.Saturate) {
            for (chan = 0; chan < 4; ++chan) {
               if (dst->WriteMask & (1 << chan)) {
                  if (inst->Instruction.Opcode == TGSI_OPCODE_MOV) {
                     analyse_src(ctx, &res[chan],
                                 &inst->Src[0].Register, chan);
                  } else if (inst->Instruction.Opcode == TGSI_OPCODE_MUL) {
                     /*
                      * Propagate values across 1.0 and 0.0 multiplications.
                      */

                     struct lp_tgsi_channel_info src0;
                     struct lp_tgsi_channel_info src1;

                     analyse_src(ctx, &src0, &inst->Src[0].Register, chan);
                     analyse_src(ctx, &src1, &inst->Src[1].Register, chan);

                     if (is_immediate(&src0, 0.0f)) {
                        res[chan] = src0;
                     } else if (is_immediate(&src1, 0.0f)) {
                        res[chan] = src1;
                     } else if (is_immediate(&src0, 1.0f)) {
                        res[chan] = src1;
                     } else if (is_immediate(&src1, 1.0f)) {
                        res[chan] = src0;
                     }
                  }
               }
            }
         }

         for (chan = 0; chan < 4; ++chan) {
            if (dst->WriteMask & (1 << chan)) {
               regs[dst->Index][chan] = res[chan];
            }
         }
      }
   }

   /*
    * Clear all temporaries information in presence of a control flow opcode.
    */

   switch (inst->Instruction.Opcode) {
   case TGSI_OPCODE_IF:
   case TGSI_OPCODE_UIF:
   case TGSI_OPCODE_ELSE:
   case TGSI_OPCODE_ENDIF:
   case TGSI_OPCODE_BGNLOOP:
   case TGSI_OPCODE_BRK:
   case TGSI_OPCODE_CONT:
   case TGSI_OPCODE_ENDLOOP:
   case TGSI_OPCODE_CAL:
   case TGSI_OPCODE_BGNSUB:
   case TGSI_OPCODE_ENDSUB:
   case TGSI_OPCODE_SWITCH:
   case TGSI_OPCODE_CASE:
   case TGSI_OPCODE_DEFAULT:
   case TGSI_OPCODE_ENDSWITCH:
   case TGSI_OPCODE_RET:
   case TGSI_OPCODE_END:
      /* XXX: Are there more cases? */
      memset(&ctx->temp, 0, sizeof ctx->temp);
      memset(&info->output, 0, sizeof info->output);
      FALLTHROUGH;
   default:
      break;
   }
}


static inline void
dump_info(const struct tgsi_token *tokens,
          struct lp_tgsi_info *info)
{
   unsigned index;
   unsigned chan;

   tgsi_dump(tokens, 0);

   for (index = 0; index < info->num_texs; ++index) {
      const struct lp_tgsi_texture_info *tex_info = &info->tex[index];
      debug_printf("TEX[%u] =", index);
      for (chan = 0; chan < 4; ++chan) {
         const struct lp_tgsi_channel_info *chan_info =
               &tex_info->coord[chan];
         if (chan_info->file != TGSI_FILE_NULL) {
            debug_printf(" %s[%u].%c",
                         tgsi_file_name(chan_info->file),
                         chan_info->u.index,
                         "xyzw01"[chan_info->swizzle]);
         } else {
            debug_printf(" _");
         }
      }
      debug_printf(", RES[%u], SAMP[%u], %s\n",
                   tex_info->texture_unit,
                   tex_info->sampler_unit,
                   tgsi_texture_names[tex_info->target]);
   }

   for (index = 0; index < PIPE_MAX_SHADER_OUTPUTS; ++index) {
      for (chan = 0; chan < 4; ++chan) {
         const struct lp_tgsi_channel_info *chan_info =
               &info->output[index][chan];
         if (chan_info->file != TGSI_FILE_NULL) {
            debug_printf("OUT[%u].%c = ", index, "xyzw"[chan]);
            if (chan_info->file == TGSI_FILE_IMMEDIATE) {
               debug_printf("%f", chan_info->u.value);
            } else {
               const char *file_name;
               switch (chan_info->file) {
               case TGSI_FILE_CONSTANT:
                  file_name = "CONST";
                  break;
               case TGSI_FILE_INPUT:
                  file_name = "IN";
                  break;
               default:
                  file_name = "???";
                  break;
               }
               debug_printf("%s[%u].%c",
                            file_name,
                            chan_info->u.index,
                            "xyzw01"[chan_info->swizzle]);
            }
            debug_printf("\n");
         }
      }
   }
}


/**
 * Detect any direct relationship between the output color
 */
void
lp_build_tgsi_info(const struct tgsi_token *tokens,
                   struct lp_tgsi_info *info)
{
   struct tgsi_parse_context parse;
   struct analysis_context *ctx;
   unsigned index;
   unsigned chan;

   memset(info, 0, sizeof *info);

   tgsi_scan_shader(tokens, &info->base);

   ctx = CALLOC(1, sizeof(struct analysis_context));
   ctx->info = info;

   tgsi_parse_init(&parse, tokens);

   while (!tgsi_parse_end_of_tokens(&parse)) {
      tgsi_parse_token(&parse);

      switch (parse.FullToken.Token.Type) {
      case TGSI_TOKEN_TYPE_DECLARATION: {
         struct tgsi_full_declaration *decl = &parse.FullToken.FullDeclaration;
         if (decl->Declaration.File == TGSI_FILE_SAMPLER_VIEW) {
            for (index = decl->Range.First; index <= decl->Range.Last; index++) {
               ctx->sample_target[index] = decl->SamplerView.Resource;
            }
         }
      }
         break;

      case TGSI_TOKEN_TYPE_INSTRUCTION:
         {
            struct tgsi_full_instruction *inst =
                  &parse.FullToken.FullInstruction;

            if (inst->Instruction.Opcode == TGSI_OPCODE_END ||
                inst->Instruction.Opcode == TGSI_OPCODE_BGNSUB) {
               /* We reached the end of main function body. */
               goto finished;
            }

            analyse_instruction(ctx, inst);
         }
         break;

      case TGSI_TOKEN_TYPE_IMMEDIATE:
         {
            const unsigned size =
                  parse.FullToken.FullImmediate.Immediate.NrTokens - 1;
            assert(size <= 4);
            if (ctx->num_imms < ARRAY_SIZE(ctx->imm)) {
               for (chan = 0; chan < size; ++chan) {
                  float value = parse.FullToken.FullImmediate.u[chan].Float;
                  ctx->imm[ctx->num_imms][chan] = value;

                  if (value < 0.0f || value > 1.0f) {
                     info->unclamped_immediates = true;
                  }
               }
               ++ctx->num_imms;
            }
         }
         break;

      case TGSI_TOKEN_TYPE_PROPERTY:
         break;

      default:
         assert(0);
      }
   }
finished:

   tgsi_parse_free(&parse);
   FREE(ctx);


   /*
    * Link the output color values.
    */

   for (index = 0; index < PIPE_MAX_COLOR_BUFS; ++index) {
      static const struct lp_tgsi_channel_info null_output[4];
      info->cbuf[index] = null_output;
   }

   for (index = 0; index < info->base.num_outputs; ++index) {
      unsigned semantic_name = info->base.output_semantic_name[index];
      unsigned semantic_index = info->base.output_semantic_index[index];
      if (semantic_name == TGSI_SEMANTIC_COLOR &&
          semantic_index < PIPE_MAX_COLOR_BUFS) {
         info->cbuf[semantic_index] = info->output[index];
      }
   }

   if (gallivm_debug & GALLIVM_DEBUG_TGSI) {
      dump_info(tokens, info);
   }
}
