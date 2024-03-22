/*
 * Copyright 2014 VMware, Inc.
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
 * This utility transforms the geometry shader to emulate point sprite by
 * drawing a quad. It also adds an extra output for the original point position
 * if the point position is to be written to a stream output buffer.
 * Note: It assumes the driver will add a constant for the inverse viewport
 *       after the user defined constants.
 */

#include "util/u_debug.h"
#include "util/u_math.h"
#include "tgsi_info.h"
#include "tgsi_point_sprite.h"
#include "tgsi_transform.h"
#include "pipe/p_state.h"

#define INVALID_INDEX 9999

/* Set swizzle based on the immediates (0, 1, 0, -1) */
static inline unsigned
set_swizzle(int x, int y, int z, int w)
{
   static const unsigned map[3] = {TGSI_SWIZZLE_W, TGSI_SWIZZLE_X,
                                   TGSI_SWIZZLE_Y};
   assert(x >= -1);
   assert(x <= 1);
   assert(y >= -1);
   assert(y <= 1);
   assert(z >= -1);
   assert(z <= 1);
   assert(w >= -1);
   assert(w <= 1);

   return map[x+1] | (map[y+1] << 2) | (map[z+1] << 4) | (map[w+1] << 6);
}

static inline unsigned
get_swizzle(unsigned swizzle, unsigned component)
{
   assert(component < 4);
   return (swizzle >> (component * 2)) & 0x3;
}

struct psprite_transform_context
{
   struct tgsi_transform_context base;
   unsigned num_tmp;
   unsigned num_out;
   unsigned num_orig_out;
   unsigned num_const;
   unsigned num_imm;
   unsigned point_size_in;          // point size input
   unsigned point_size_out;         // point size output
   unsigned point_size_tmp;         // point size temp
   unsigned point_pos_in;           // point pos input
   unsigned point_pos_out;          // point pos output
   unsigned point_pos_sout;         // original point pos for streamout
   unsigned point_pos_tmp;          // point pos temp
   unsigned point_scale_tmp;        // point scale temp
   unsigned point_color_out;        // point color output
   unsigned point_color_tmp;        // point color temp
   unsigned point_imm;              // point immediates
   unsigned point_ivp;              // point inverseViewport constant
   unsigned point_dir_swz[4];       // point direction swizzle
   unsigned point_coord_swz[4];     // point coord swizzle
   unsigned point_coord_enable;     // point coord enable mask
   unsigned point_coord_decl;       // point coord output declared mask
   unsigned point_coord_out;        // point coord output starting index
   unsigned point_coord_aa;         // aa point coord semantic index
   unsigned point_coord_k;          // aa point coord threshold distance
   unsigned stream_out_point_pos:1; // set if to stream out original point pos
   unsigned aa_point:1;             // set if doing aa point
   unsigned need_texcoord_semantic:1;   // set if need texcoord semantic
   unsigned out_tmp_index[PIPE_MAX_SHADER_OUTPUTS];
   int max_generic;                 // max generic semantic index
};

static inline struct psprite_transform_context *
psprite_transform_context(struct tgsi_transform_context *ctx)
{
   return (struct psprite_transform_context *) ctx;
}


/**
 * TGSI declaration transform callback.
 */
static void
psprite_decl(struct tgsi_transform_context *ctx,
             struct tgsi_full_declaration *decl)
{
   struct psprite_transform_context *ts = psprite_transform_context(ctx);
   unsigned range_end = decl->Range.Last + 1;

   if (decl->Declaration.File == TGSI_FILE_INPUT) {
      if (decl->Semantic.Name == TGSI_SEMANTIC_PSIZE) {
         ts->point_size_in = decl->Range.First;
      }
      else if (decl->Semantic.Name == TGSI_SEMANTIC_POSITION) {
         ts->point_pos_in = decl->Range.First;
      }
   }
   else if (decl->Declaration.File == TGSI_FILE_OUTPUT) {
      if (decl->Semantic.Name == TGSI_SEMANTIC_PSIZE) {
         ts->point_size_out = decl->Range.First;
      }
      else if (decl->Semantic.Name == TGSI_SEMANTIC_POSITION) {
         ts->point_pos_out = decl->Range.First;
      }
      else if (!ts->need_texcoord_semantic &&
	       decl->Semantic.Name == TGSI_SEMANTIC_GENERIC &&
               decl->Semantic.Index < 32) {
         ts->point_coord_decl |= 1 << decl->Semantic.Index;
         ts->max_generic = MAX2(ts->max_generic, (int)decl->Semantic.Index);
      }
      else if (ts->need_texcoord_semantic &&
               decl->Semantic.Name == TGSI_SEMANTIC_TEXCOORD) {
         ts->point_coord_decl |= 1 << decl->Semantic.Index;
      }
      ts->num_out = MAX2(ts->num_out, range_end);
   }
   else if (decl->Declaration.File == TGSI_FILE_TEMPORARY) {
      ts->num_tmp = MAX2(ts->num_tmp, range_end);
   }
   else if (decl->Declaration.File == TGSI_FILE_CONSTANT) {
      ts->num_const = MAX2(ts->num_const, range_end);
   }

   ctx->emit_declaration(ctx, decl);
}

/**
 * TGSI immediate declaration transform callback.
 */
static void
psprite_immediate(struct tgsi_transform_context *ctx,
                  struct tgsi_full_immediate *imm)
{
   struct psprite_transform_context *ts = psprite_transform_context(ctx);

   ctx->emit_immediate(ctx, imm);
   ts->num_imm++;
}


/**
 * TGSI transform prolog callback.
 */
static void
psprite_prolog(struct tgsi_transform_context *ctx)
{
   struct psprite_transform_context *ts = psprite_transform_context(ctx);
   unsigned point_coord_enable, en;
   unsigned i;

   /* Replace output registers with temporary registers */
   for (i = 0; i < ts->num_out; i++) {
      ts->out_tmp_index[i] = ts->num_tmp++;
   }
   ts->num_orig_out = ts->num_out;

   /* Declare a tmp register for point scale */
   ts->point_scale_tmp = ts->num_tmp++;

   if (ts->point_size_out != INVALID_INDEX)
      ts->point_size_tmp = ts->out_tmp_index[ts->point_size_out];
   else
      ts->point_size_tmp = ts->num_tmp++;

   assert(ts->point_pos_out != INVALID_INDEX);
   ts->point_pos_tmp = ts->out_tmp_index[ts->point_pos_out];
   ts->out_tmp_index[ts->point_pos_out] = INVALID_INDEX;

   /* Declare one more tmp register for point coord threshold distance
    * if we are generating anti-aliased point.
    */
   if (ts->aa_point)
      ts->point_coord_k = ts->num_tmp++;

   tgsi_transform_temps_decl(ctx, ts->point_size_tmp, ts->num_tmp-1);

   /* Declare an extra output for the original point position for stream out */
   if (ts->stream_out_point_pos) {
      ts->point_pos_sout = ts->num_out++;
      tgsi_transform_output_decl(ctx, ts->point_pos_sout,
                                 TGSI_SEMANTIC_GENERIC, 0, 0);
   }

   /* point coord outputs to be declared */
   point_coord_enable = ts->point_coord_enable & ~ts->point_coord_decl;

   /* Declare outputs for those point coord that are enabled but are not
    * already declared in this shader.
    */
   ts->point_coord_out = ts->num_out;
   if (point_coord_enable) {
      if (ts->need_texcoord_semantic) {
         for (i = 0, en = point_coord_enable; en; en>>=1, i++) {
            if (en & 0x1) {
               tgsi_transform_output_decl(ctx, ts->num_out++,
                                          TGSI_SEMANTIC_TEXCOORD, i, 0);
            }
         }
      } else {
         for (i = 0, en = point_coord_enable; en; en>>=1, i++) {
            if (en & 0x1) {
               tgsi_transform_output_decl(ctx, ts->num_out++,
                                          TGSI_SEMANTIC_GENERIC, i, 0);
               ts->max_generic = MAX2(ts->max_generic, (int)i);
            }
         }
      }
   }

   /* add an extra generic output for aa point texcoord */
   if (ts->aa_point) {
      if (ts->need_texcoord_semantic) {
         ts->point_coord_aa = 0;
      } else {
         ts->point_coord_aa = ts->max_generic + 1;
         assert((ts->point_coord_enable & (1 << ts->point_coord_aa)) == 0);
         ts->point_coord_enable |= 1 << (ts->point_coord_aa);
         tgsi_transform_output_decl(ctx, ts->num_out++, TGSI_SEMANTIC_GENERIC,
                                    ts->point_coord_aa, 0);
      }
   }

   /* Declare extra immediates */
   ts->point_imm = ts->num_imm;
   tgsi_transform_immediate_decl(ctx, 0, 1, 0.5, -1);

   /* Declare point constant -
    * constant.xy -- inverseViewport
    * constant.z -- current point size
    * constant.w -- max point size
    * The driver needs to add this constant to the constant buffer
    */
   ts->point_ivp = ts->num_const++;
   tgsi_transform_const_decl(ctx, ts->point_ivp, ts->point_ivp);

   /* If this geometry shader does not specify point size,
    * get the current point size from the point constant.
    */
   if (ts->point_size_out == INVALID_INDEX) {
      struct tgsi_full_instruction inst;

      inst = tgsi_default_full_instruction();
      inst.Instruction.Opcode = TGSI_OPCODE_MOV;
      inst.Instruction.NumDstRegs = 1;
      tgsi_transform_dst_reg(&inst.Dst[0], TGSI_FILE_TEMPORARY,
                             ts->point_size_tmp, TGSI_WRITEMASK_XYZW);
      inst.Instruction.NumSrcRegs = 1;
      tgsi_transform_src_reg(&inst.Src[0], TGSI_FILE_CONSTANT,
                             ts->point_ivp, TGSI_SWIZZLE_Z,
                             TGSI_SWIZZLE_Z, TGSI_SWIZZLE_Z, TGSI_SWIZZLE_Z);
      ctx->emit_instruction(ctx, &inst);
   }
}


/**
 * Add the point sprite emulation instructions at the emit vertex instruction
 */
static void
psprite_emit_vertex_inst(struct tgsi_transform_context *ctx,
                         struct tgsi_full_instruction *vert_inst)
{
   struct psprite_transform_context *ts = psprite_transform_context(ctx);
   struct tgsi_full_instruction inst;
   unsigned point_coord_enable, en;
   unsigned i, j, s;

   /* new point coord outputs */
   point_coord_enable = ts->point_coord_enable & ~ts->point_coord_decl;

   /* OUTPUT[pos_sout] = TEMP[pos] */
   if (ts->point_pos_sout != INVALID_INDEX) {
      tgsi_transform_op1_inst(ctx, TGSI_OPCODE_MOV,
                              TGSI_FILE_OUTPUT, ts->point_pos_sout,
                              TGSI_WRITEMASK_XYZW,
                              TGSI_FILE_TEMPORARY, ts->point_pos_tmp);
   }

   /**
    * Set up the point scale vector
    * scale = pointSize * pos.w * inverseViewport
    */

   /* MUL point_scale.x, point_size.x, point_pos.w */
   tgsi_transform_op2_swz_inst(ctx, TGSI_OPCODE_MUL,
                  TGSI_FILE_TEMPORARY, ts->point_scale_tmp, TGSI_WRITEMASK_X,
                  TGSI_FILE_TEMPORARY, ts->point_size_tmp, TGSI_SWIZZLE_X,
                  TGSI_FILE_TEMPORARY, ts->point_pos_tmp, TGSI_SWIZZLE_W, false);

   /* MUL point_scale.xy, point_scale.xx, inverseViewport.xy */
   inst = tgsi_default_full_instruction();
   inst.Instruction.Opcode = TGSI_OPCODE_MUL;
   inst.Instruction.NumDstRegs = 1;
   tgsi_transform_dst_reg(&inst.Dst[0], TGSI_FILE_TEMPORARY,
                          ts->point_scale_tmp, TGSI_WRITEMASK_XY);
   inst.Instruction.NumSrcRegs = 2;
   tgsi_transform_src_reg(&inst.Src[0], TGSI_FILE_TEMPORARY,
                          ts->point_scale_tmp, TGSI_SWIZZLE_X,
                          TGSI_SWIZZLE_X, TGSI_SWIZZLE_X, TGSI_SWIZZLE_X);
   tgsi_transform_src_reg(&inst.Src[1], TGSI_FILE_CONSTANT,
                          ts->point_ivp, TGSI_SWIZZLE_X,
                          TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Z, TGSI_SWIZZLE_Z);
   ctx->emit_instruction(ctx, &inst);

   /**
    * Set up the point coord threshold distance
    * k = 0.5 - 1 / pointsize
    */
   if (ts->aa_point) {
      tgsi_transform_op2_swz_inst(ctx, TGSI_OPCODE_DIV,
                                  TGSI_FILE_TEMPORARY, ts->point_coord_k,
                                  TGSI_WRITEMASK_X,
                                  TGSI_FILE_IMMEDIATE, ts->point_imm,
                                  TGSI_SWIZZLE_Y,
                                  TGSI_FILE_TEMPORARY, ts->point_size_tmp,
                                  TGSI_SWIZZLE_X, false);

      tgsi_transform_op2_swz_inst(ctx, TGSI_OPCODE_ADD,
                                  TGSI_FILE_TEMPORARY, ts->point_coord_k,
                                  TGSI_WRITEMASK_X,
                                  TGSI_FILE_IMMEDIATE, ts->point_imm,
                                  TGSI_SWIZZLE_Z,
                                  TGSI_FILE_TEMPORARY, ts->point_coord_k,
                                  TGSI_SWIZZLE_X, true);
   }


   for (i = 0; i < 4; i++) {
      unsigned point_dir_swz = ts->point_dir_swz[i];
      unsigned point_coord_swz = ts->point_coord_swz[i];

      /* All outputs need to be emitted for each vertex */
      for (j = 0; j < ts->num_orig_out; j++) {
         if (ts->out_tmp_index[j] != INVALID_INDEX) {
            tgsi_transform_op1_inst(ctx, TGSI_OPCODE_MOV,
                                    TGSI_FILE_OUTPUT, j,
                                    TGSI_WRITEMASK_XYZW,
                                    TGSI_FILE_TEMPORARY, ts->out_tmp_index[j]);
         }
      }

      /* pos = point_scale * point_dir + point_pos */
      inst = tgsi_default_full_instruction();
      inst.Instruction.Opcode = TGSI_OPCODE_MAD;
      inst.Instruction.NumDstRegs = 1;
      tgsi_transform_dst_reg(&inst.Dst[0], TGSI_FILE_OUTPUT, ts->point_pos_out,
                             TGSI_WRITEMASK_XYZW);
      inst.Instruction.NumSrcRegs = 3;
      tgsi_transform_src_reg(&inst.Src[0], TGSI_FILE_TEMPORARY, ts->point_scale_tmp,
                             TGSI_SWIZZLE_X, TGSI_SWIZZLE_Y, TGSI_SWIZZLE_X,
                             TGSI_SWIZZLE_X);
      tgsi_transform_src_reg(&inst.Src[1], TGSI_FILE_IMMEDIATE, ts->point_imm,
                             get_swizzle(point_dir_swz, 0),
                             get_swizzle(point_dir_swz, 1),
                             get_swizzle(point_dir_swz, 2),
                             get_swizzle(point_dir_swz, 3));
      tgsi_transform_src_reg(&inst.Src[2], TGSI_FILE_TEMPORARY, ts->point_pos_tmp,
                             TGSI_SWIZZLE_X, TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Z,
                             TGSI_SWIZZLE_W);
      ctx->emit_instruction(ctx, &inst);

      /* point coord */
      for (j = 0, s = 0, en = point_coord_enable; en; en>>=1, s++) {
         unsigned dstReg;

         if (en & 0x1) {
            dstReg = ts->point_coord_out + j;

            inst = tgsi_default_full_instruction();
            inst.Instruction.Opcode = TGSI_OPCODE_MOV;
            inst.Instruction.NumDstRegs = 1;
            tgsi_transform_dst_reg(&inst.Dst[0], TGSI_FILE_OUTPUT,
                                   dstReg, TGSI_WRITEMASK_XYZW);
            inst.Instruction.NumSrcRegs = 1;
            tgsi_transform_src_reg(&inst.Src[0], TGSI_FILE_IMMEDIATE, ts->point_imm,
                                   get_swizzle(point_coord_swz, 0),
                                   get_swizzle(point_coord_swz, 1),
                                   get_swizzle(point_coord_swz, 2),
                                   get_swizzle(point_coord_swz, 3));
            ctx->emit_instruction(ctx, &inst);

            /* MOV point_coord.z  point_coord_k.x */
            if (s == ts->point_coord_aa) {
               tgsi_transform_op1_swz_inst(ctx, TGSI_OPCODE_MOV,
                                           TGSI_FILE_OUTPUT, dstReg, TGSI_WRITEMASK_Z,
                                           TGSI_FILE_TEMPORARY, ts->point_coord_k,
                                           TGSI_SWIZZLE_X);
            }
            j++;  /* the next point coord output offset */
         }
      }

      /* Emit the EMIT instruction for each vertex of the quad */
      ctx->emit_instruction(ctx, vert_inst);
   }

   /* Emit the ENDPRIM instruction for the quad */
   inst = tgsi_default_full_instruction();
   inst.Instruction.Opcode = TGSI_OPCODE_ENDPRIM;
   inst.Instruction.NumDstRegs = 0;
   inst.Instruction.NumSrcRegs = 1;
   inst.Src[0] = vert_inst->Src[0];
   ctx->emit_instruction(ctx, &inst);
}


/**
 * TGSI instruction transform callback.
 */
static void
psprite_inst(struct tgsi_transform_context *ctx,
             struct tgsi_full_instruction *inst)
{
   struct psprite_transform_context *ts = psprite_transform_context(ctx);

   if (inst->Instruction.Opcode == TGSI_OPCODE_EMIT) {
      psprite_emit_vertex_inst(ctx, inst);
   }
   else if (inst->Dst[0].Register.File == TGSI_FILE_OUTPUT &&
	    inst->Dst[0].Register.Index == (int)ts->point_size_out) {
      /**
       * Replace point size output reg with tmp reg.
       * The tmp reg will be later used as a src reg for computing
       * the point scale factor.
       */
      inst->Dst[0].Register.File = TGSI_FILE_TEMPORARY;
      inst->Dst[0].Register.Index = ts->point_size_tmp;
      ctx->emit_instruction(ctx, inst);

      /* Clamp the point size */
      /* MAX point_size_tmp.x, point_size_tmp.x, point_imm.y */
      tgsi_transform_op2_swz_inst(ctx, TGSI_OPCODE_MAX,
                 TGSI_FILE_TEMPORARY, ts->point_size_tmp, TGSI_WRITEMASK_X,
                 TGSI_FILE_TEMPORARY, ts->point_size_tmp, TGSI_SWIZZLE_X,
                 TGSI_FILE_IMMEDIATE, ts->point_imm, TGSI_SWIZZLE_Y, false);

      /* MIN point_size_tmp.x, point_size_tmp.x, point_ivp.w */
      tgsi_transform_op2_swz_inst(ctx, TGSI_OPCODE_MIN,
                 TGSI_FILE_TEMPORARY, ts->point_size_tmp, TGSI_WRITEMASK_X,
                 TGSI_FILE_TEMPORARY, ts->point_size_tmp, TGSI_SWIZZLE_X,
                 TGSI_FILE_CONSTANT, ts->point_ivp, TGSI_SWIZZLE_W, false);
   }
   else if (inst->Dst[0].Register.File == TGSI_FILE_OUTPUT &&
	    inst->Dst[0].Register.Index == (int)ts->point_pos_out) {
      /**
       * Replace point pos output reg with tmp reg.
       */
      inst->Dst[0].Register.File = TGSI_FILE_TEMPORARY;
      inst->Dst[0].Register.Index = ts->point_pos_tmp;
      ctx->emit_instruction(ctx, inst);
   }
   else if (inst->Dst[0].Register.File == TGSI_FILE_OUTPUT) {
      /**
       * Replace output reg with tmp reg.
       */
      inst->Dst[0].Register.File = TGSI_FILE_TEMPORARY;
      inst->Dst[0].Register.Index = ts->out_tmp_index[inst->Dst[0].Register.Index];
      ctx->emit_instruction(ctx, inst);
   }
   else {
      ctx->emit_instruction(ctx, inst);
   }
}


/**
 * TGSI property instruction transform callback.
 * Transforms a point into a 4-vertex triangle strip.
 */
static void
psprite_property(struct tgsi_transform_context *ctx,
                 struct tgsi_full_property *prop)
{
   switch (prop->Property.PropertyName) {
   case TGSI_PROPERTY_GS_OUTPUT_PRIM:
       prop->u[0].Data = MESA_PRIM_TRIANGLE_STRIP;
       break;
   case TGSI_PROPERTY_GS_MAX_OUTPUT_VERTICES:
       prop->u[0].Data *= 4;
       break;
   default:
       break;
   }
   ctx->emit_property(ctx, prop);
}

/**
 * TGSI utility to transform a geometry shader to support point sprite.
 */
struct tgsi_token *
tgsi_add_point_sprite(const struct tgsi_token *tokens_in,
                      const unsigned point_coord_enable,
                      const bool sprite_origin_lower_left,
                      const bool stream_out_point_pos,
		      const bool need_texcoord_semantic,
                      int *aa_point_coord_index)
{
   struct psprite_transform_context transform;
   const unsigned num_new_tokens = 200; /* should be enough */
   const unsigned new_len = tgsi_num_tokens(tokens_in) + num_new_tokens;
   struct tgsi_token *new_tokens;

   /* setup transformation context */
   memset(&transform, 0, sizeof(transform));
   transform.base.transform_declaration = psprite_decl;
   transform.base.transform_instruction = psprite_inst;
   transform.base.transform_property = psprite_property;
   transform.base.transform_immediate = psprite_immediate;
   transform.base.prolog = psprite_prolog;

   transform.point_size_in = INVALID_INDEX;
   transform.point_size_out = INVALID_INDEX;
   transform.point_size_tmp = INVALID_INDEX;
   transform.point_pos_in = INVALID_INDEX;
   transform.point_pos_out = INVALID_INDEX;
   transform.point_pos_sout = INVALID_INDEX;
   transform.point_pos_tmp = INVALID_INDEX;
   transform.point_scale_tmp = INVALID_INDEX;
   transform.point_imm = INVALID_INDEX;
   transform.point_coord_aa = INVALID_INDEX;
   transform.point_coord_k = INVALID_INDEX;

   transform.stream_out_point_pos = stream_out_point_pos;
   transform.point_coord_enable = point_coord_enable;
   transform.aa_point = aa_point_coord_index != NULL;
   transform.need_texcoord_semantic = need_texcoord_semantic;
   transform.max_generic = -1;

   /* point sprite directions based on the immediates (0, 1, 0.5, -1) */
   /* (-1, -1, 0, 0) */
   transform.point_dir_swz[0] = set_swizzle(-1, -1, 0, 0);
   /* (-1, 1, 0, 0) */
   transform.point_dir_swz[1] = set_swizzle(-1, 1, 0, 0);
   /* (1, -1, 0, 0) */
   transform.point_dir_swz[2] = set_swizzle(1, -1, 0, 0);
   /* (1, 1, 0, 0) */
   transform.point_dir_swz[3] = set_swizzle(1, 1, 0, 0);

   /* point coord based on the immediates (0, 1, 0, -1) */
   if (sprite_origin_lower_left) {
      /* (0, 0, 0, 1) */
      transform.point_coord_swz[0] = set_swizzle(0, 0, 0, 1);
      /* (0, 1, 0, 1) */
      transform.point_coord_swz[1] = set_swizzle(0, 1, 0, 1);
      /* (1, 0, 0, 1) */
      transform.point_coord_swz[2] = set_swizzle(1, 0, 0, 1);
      /* (1, 1, 0, 1) */
      transform.point_coord_swz[3] = set_swizzle(1, 1, 0, 1);
   }
   else {
      /* (0, 1, 0, 1) */
      transform.point_coord_swz[0] = set_swizzle(0, 1, 0, 1);
      /* (0, 0, 0, 1) */
      transform.point_coord_swz[1] = set_swizzle(0, 0, 0, 1);
      /* (1, 1, 0, 1) */
      transform.point_coord_swz[2] = set_swizzle(1, 1, 0, 1);
      /* (1, 0, 0, 1) */
      transform.point_coord_swz[3] = set_swizzle(1, 0, 0, 1);
   }


   new_tokens = tgsi_transform_shader(tokens_in, new_len, &transform.base);

   if (aa_point_coord_index)
      *aa_point_coord_index = transform.point_coord_aa;

   return new_tokens;
}
