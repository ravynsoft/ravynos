/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
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

#include <stdarg.h>

#include "i915_context.h"
#include "i915_debug.h"
#include "i915_debug_private.h"
#include "i915_fpc.h"
#include "i915_reg.h"

#include "nir/nir.h"
#include "pipe/p_shader_tokens.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_info.h"
#include "tgsi/tgsi_parse.h"
#include "util/log.h"
#include "util/ralloc.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "draw/draw_vertex.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Simple pass-through fragment shader to use when we don't have
 * a real shader (or it fails to compile for some reason).
 */
static unsigned passthrough_program[] = {
   _3DSTATE_PIXEL_SHADER_PROGRAM | ((1 * 3) - 1),
   /* move to output color:
    */
   (A0_MOV | (REG_TYPE_OC << A0_DEST_TYPE_SHIFT) | A0_DEST_CHANNEL_ALL |
    (REG_TYPE_R << A0_SRC0_TYPE_SHIFT) | (0 << A0_SRC0_NR_SHIFT)),
   ((SRC_ONE << A1_SRC0_CHANNEL_X_SHIFT) |
    (SRC_ZERO << A1_SRC0_CHANNEL_Y_SHIFT) |
    (SRC_ZERO << A1_SRC0_CHANNEL_Z_SHIFT) |
    (SRC_ONE << A1_SRC0_CHANNEL_W_SHIFT)),
   0};

/**
 * component-wise negation of ureg
 */
static inline int
negate(int reg, int x, int y, int z, int w)
{
   /* Another neat thing about the UREG representation */
   return reg ^ (((x & 1) << UREG_CHANNEL_X_NEGATE_SHIFT) |
                 ((y & 1) << UREG_CHANNEL_Y_NEGATE_SHIFT) |
                 ((z & 1) << UREG_CHANNEL_Z_NEGATE_SHIFT) |
                 ((w & 1) << UREG_CHANNEL_W_NEGATE_SHIFT));
}

/**
 * In the event of a translation failure, we'll generate a simple color
 * pass-through program.
 */
static void
i915_use_passthrough_shader(struct i915_fragment_shader *fs)
{
   fs->program = (uint32_t *)MALLOC(sizeof(passthrough_program));
   if (fs->program) {
      memcpy(fs->program, passthrough_program, sizeof(passthrough_program));
      fs->program_len = ARRAY_SIZE(passthrough_program);
   }
   fs->num_constants = 0;
}

void
i915_program_error(struct i915_fp_compile *p, const char *msg, ...)
{
   va_list args;
   va_start(args, msg);
   ralloc_vasprintf_append(&p->error, msg, args);
   va_end(args);
}

static uint32_t
get_mapping(struct i915_fragment_shader *fs, enum tgsi_semantic semantic,
            int index)
{
   int i;
   for (i = 0; i < I915_TEX_UNITS; i++) {
      if (fs->texcoords[i].semantic == -1) {
         fs->texcoords[i].semantic = semantic;
         fs->texcoords[i].index = index;
         return i;
      }
      if (fs->texcoords[i].semantic == semantic &&
          fs->texcoords[i].index == index)
         return i;
   }
   debug_printf("Exceeded max generics\n");
   return 0;
}

/**
 * Construct a ureg for the given source register.  Will emit
 * constants, apply swizzling and negation as needed.
 */
static uint32_t
src_vector(struct i915_fp_compile *p,
           const struct i915_full_src_register *source,
           struct i915_fragment_shader *fs)
{
   uint32_t index = source->Register.Index;
   uint32_t src = 0, sem_name, sem_ind;

   switch (source->Register.File) {
   case TGSI_FILE_TEMPORARY:
      if (source->Register.Index >= I915_MAX_TEMPORARY) {
         i915_program_error(p, "Exceeded max temporary reg");
         return 0;
      }
      src = UREG(REG_TYPE_R, index);
      break;
   case TGSI_FILE_INPUT:
      /* XXX: Packing COL1, FOGC into a single attribute works for
       * texenv programs, but will fail for real fragment programs
       * that use these attributes and expect them to be a full 4
       * components wide.  Could use a texcoord to pass these
       * attributes if necessary, but that won't work in the general
       * case.
       *
       * We also use a texture coordinate to pass wpos when possible.
       */

      sem_name = p->shader->info.input_semantic_name[index];
      sem_ind = p->shader->info.input_semantic_index[index];

      switch (sem_name) {
      case TGSI_SEMANTIC_GENERIC:
      case TGSI_SEMANTIC_TEXCOORD:
      case TGSI_SEMANTIC_PCOORD:
      case TGSI_SEMANTIC_POSITION: {
         if (sem_name == TGSI_SEMANTIC_PCOORD)
            fs->reads_pntc = true;

         int real_tex_unit = get_mapping(fs, sem_name, sem_ind);
         src = i915_emit_decl(p, REG_TYPE_T, T_TEX0 + real_tex_unit,
                              D0_CHANNEL_ALL);
         break;
      }
      case TGSI_SEMANTIC_COLOR:
         if (sem_ind == 0) {
            src = i915_emit_decl(p, REG_TYPE_T, T_DIFFUSE, D0_CHANNEL_ALL);
         } else {
            /* secondary color */
            assert(sem_ind == 1);
            src = i915_emit_decl(p, REG_TYPE_T, T_SPECULAR, D0_CHANNEL_XYZ);
            src = swizzle(src, X, Y, Z, ONE);
         }
         break;
      case TGSI_SEMANTIC_FOG:
         src = i915_emit_decl(p, REG_TYPE_T, T_FOG_W, D0_CHANNEL_W);
         src = swizzle(src, W, W, W, W);
         break;
      case TGSI_SEMANTIC_FACE: {
         /* for back/front faces */
         int real_tex_unit = get_mapping(fs, sem_name, sem_ind);
         src =
            i915_emit_decl(p, REG_TYPE_T, T_TEX0 + real_tex_unit, D0_CHANNEL_X);
         break;
      }
      default:
         i915_program_error(p, "Bad source->Index");
         return 0;
      }
      break;

   case TGSI_FILE_IMMEDIATE: {
      assert(index < p->num_immediates);

      uint8_t swiz[4] = {source->Register.SwizzleX, source->Register.SwizzleY,
                         source->Register.SwizzleZ, source->Register.SwizzleW};

      uint8_t neg[4] = {source->Register.Negate, source->Register.Negate,
                        source->Register.Negate, source->Register.Negate};

      unsigned i;

      for (i = 0; i < 4; i++) {
         if (swiz[i] == TGSI_SWIZZLE_ZERO || swiz[i] == TGSI_SWIZZLE_ONE) {
            continue;
         } else if (p->immediates[index][swiz[i]] == 0.0) {
            swiz[i] = TGSI_SWIZZLE_ZERO;
         } else if (p->immediates[index][swiz[i]] == 1.0) {
            swiz[i] = TGSI_SWIZZLE_ONE;
         } else if (p->immediates[index][swiz[i]] == -1.0) {
            swiz[i] = TGSI_SWIZZLE_ONE;
            neg[i] ^= 1;
         } else {
            break;
         }
      }

      if (i == 4) {
         return negate(
            swizzle(UREG(REG_TYPE_R, 0), swiz[0], swiz[1], swiz[2], swiz[3]),
            neg[0], neg[1], neg[2], neg[3]);
      }

      index = p->immediates_map[index];
      FALLTHROUGH;
   }

   case TGSI_FILE_CONSTANT:
      src = UREG(REG_TYPE_CONST, index);
      break;

   default:
      i915_program_error(p, "Bad source->File");
      return 0;
   }

   src = swizzle(src, source->Register.SwizzleX, source->Register.SwizzleY,
                 source->Register.SwizzleZ, source->Register.SwizzleW);

   /* No HW abs flag, so we have to max with the negation. */
   if (source->Register.Absolute) {
      uint32_t tmp = i915_get_utemp(p);
      i915_emit_arith(p, A0_MAX, tmp, A0_DEST_CHANNEL_ALL, 0, src,
                      negate(src, 1, 1, 1, 1), 0);
      src = tmp;
   }

   /* There's both negate-all-components and per-component negation.
    * Try to handle both here.
    */
   {
      int n = source->Register.Negate;
      src = negate(src, n, n, n, n);
   }

   return src;
}

/**
 * Construct a ureg for a destination register.
 */
static uint32_t
get_result_vector(struct i915_fp_compile *p,
                  const struct i915_full_dst_register *dest)
{
   switch (dest->Register.File) {
   case TGSI_FILE_OUTPUT: {
      uint32_t sem_name =
         p->shader->info.output_semantic_name[dest->Register.Index];
      switch (sem_name) {
      case TGSI_SEMANTIC_POSITION:
         return UREG(REG_TYPE_OD, 0);
      case TGSI_SEMANTIC_COLOR:
         return UREG(REG_TYPE_OC, 0);
      default:
         i915_program_error(p, "Bad inst->DstReg.Index/semantics");
         return 0;
      }
   }
   case TGSI_FILE_TEMPORARY:
      return UREG(REG_TYPE_R, dest->Register.Index);
   default:
      i915_program_error(p, "Bad inst->DstReg.File");
      return 0;
   }
}

/**
 * Compute flags for saturation and writemask.
 */
static uint32_t
get_result_flags(const struct i915_full_instruction *inst)
{
   const uint32_t writeMask = inst->Dst[0].Register.WriteMask;
   uint32_t flags = 0x0;

   if (inst->Instruction.Saturate)
      flags |= A0_DEST_SATURATE;

   if (writeMask & TGSI_WRITEMASK_X)
      flags |= A0_DEST_CHANNEL_X;
   if (writeMask & TGSI_WRITEMASK_Y)
      flags |= A0_DEST_CHANNEL_Y;
   if (writeMask & TGSI_WRITEMASK_Z)
      flags |= A0_DEST_CHANNEL_Z;
   if (writeMask & TGSI_WRITEMASK_W)
      flags |= A0_DEST_CHANNEL_W;

   return flags;
}

/**
 * Convert TGSI_TEXTURE_x token to DO_SAMPLE_TYPE_x token
 */
static uint32_t
translate_tex_src_target(struct i915_fp_compile *p, uint32_t tex)
{
   switch (tex) {
   case TGSI_TEXTURE_SHADOW1D:
      FALLTHROUGH;
   case TGSI_TEXTURE_1D:
      return D0_SAMPLE_TYPE_2D;

   case TGSI_TEXTURE_SHADOW2D:
      FALLTHROUGH;
   case TGSI_TEXTURE_2D:
      return D0_SAMPLE_TYPE_2D;

   case TGSI_TEXTURE_SHADOWRECT:
      FALLTHROUGH;
   case TGSI_TEXTURE_RECT:
      return D0_SAMPLE_TYPE_2D;

   case TGSI_TEXTURE_3D:
      return D0_SAMPLE_TYPE_VOLUME;

   case TGSI_TEXTURE_CUBE:
      return D0_SAMPLE_TYPE_CUBE;

   default:
      i915_program_error(p, "TexSrc type");
      return 0;
   }
}

/**
 * Return the number of coords needed to access a given TGSI_TEXTURE_*
 */
uint32_t
i915_coord_mask(enum tgsi_opcode opcode, enum tgsi_texture_type tex)
{
   uint32_t coord_mask = 0;

   if (opcode == TGSI_OPCODE_TXP || opcode == TGSI_OPCODE_TXB)
      coord_mask |= TGSI_WRITEMASK_W;

   switch (tex) {
   case TGSI_TEXTURE_1D: /* See the 1D coord swizzle below. */
   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_RECT:
      return coord_mask | TGSI_WRITEMASK_XY;

   case TGSI_TEXTURE_SHADOW1D:
   case TGSI_TEXTURE_SHADOW2D:
   case TGSI_TEXTURE_SHADOWRECT:
   case TGSI_TEXTURE_3D:
   case TGSI_TEXTURE_CUBE:
      return coord_mask | TGSI_WRITEMASK_XYZ;

   default:
      unreachable("bad texture target");
   }
}

/**
 * Generate texel lookup instruction.
 */
static void
emit_tex(struct i915_fp_compile *p, const struct i915_full_instruction *inst,
         uint32_t opcode, struct i915_fragment_shader *fs)
{
   uint32_t texture = inst->Texture.Texture;
   uint32_t unit = inst->Src[1].Register.Index;
   uint32_t tex = translate_tex_src_target(p, texture);
   uint32_t sampler = i915_emit_decl(p, REG_TYPE_S, unit, tex);
   uint32_t coord = src_vector(p, &inst->Src[0], fs);

   /* For 1D textures, set the Y coord to the same as X.  Otherwise, we could
    * select the wrong LOD based on the uninitialized Y coord when we sample our
    * 1D textures as 2D.
    */
   if (texture == TGSI_TEXTURE_1D || texture == TGSI_TEXTURE_SHADOW1D)
      coord = swizzle(coord, X, X, Z, W);

   i915_emit_texld(p, get_result_vector(p, &inst->Dst[0]),
                   get_result_flags(inst), sampler, coord, opcode,
                   i915_coord_mask(inst->Instruction.Opcode, texture));
}

/**
 * Generate a simple arithmetic instruction
 * \param opcode  the i915 opcode
 * \param numArgs  the number of input/src arguments
 */
static void
emit_simple_arith(struct i915_fp_compile *p,
                  const struct i915_full_instruction *inst, uint32_t opcode,
                  uint32_t numArgs, struct i915_fragment_shader *fs)
{
   uint32_t arg1, arg2, arg3;

   assert(numArgs <= 3);

   arg1 = (numArgs < 1) ? 0 : src_vector(p, &inst->Src[0], fs);
   arg2 = (numArgs < 2) ? 0 : src_vector(p, &inst->Src[1], fs);
   arg3 = (numArgs < 3) ? 0 : src_vector(p, &inst->Src[2], fs);

   i915_emit_arith(p, opcode, get_result_vector(p, &inst->Dst[0]),
                   get_result_flags(inst), 0, arg1, arg2, arg3);
}

/** As above, but swap the first two src regs */
static void
emit_simple_arith_swap2(struct i915_fp_compile *p,
                        const struct i915_full_instruction *inst,
                        uint32_t opcode, uint32_t numArgs,
                        struct i915_fragment_shader *fs)
{
   struct i915_full_instruction inst2;

   assert(numArgs == 2);

   /* transpose first two registers */
   inst2 = *inst;
   inst2.Src[0] = inst->Src[1];
   inst2.Src[1] = inst->Src[0];

   emit_simple_arith(p, &inst2, opcode, numArgs, fs);
}

/*
 * Translate TGSI instruction to i915 instruction.
 *
 * Possible concerns:
 *
 * DDX, DDY -- return 0
 * SIN, COS -- could use another taylor step?
 * LIT      -- results seem a little different to sw mesa
 * LOG      -- different to mesa on negative numbers, but this is conformant.
 */
static void
i915_translate_instruction(struct i915_fp_compile *p,
                           const struct i915_full_instruction *inst,
                           struct i915_fragment_shader *fs)
{
   uint32_t src0, src1, src2, flags;
   uint32_t tmp = 0;

   switch (inst->Instruction.Opcode) {
   case TGSI_OPCODE_ADD:
      emit_simple_arith(p, inst, A0_ADD, 2, fs);
      break;

   case TGSI_OPCODE_CEIL:
      src0 = src_vector(p, &inst->Src[0], fs);
      tmp = i915_get_utemp(p);
      flags = get_result_flags(inst);
      i915_emit_arith(p, A0_FLR, tmp, flags & A0_DEST_CHANNEL_ALL, 0,
                      negate(src0, 1, 1, 1, 1), 0, 0);
      i915_emit_arith(p, A0_MOV, get_result_vector(p, &inst->Dst[0]), flags, 0,
                      negate(tmp, 1, 1, 1, 1), 0, 0);
      break;

   case TGSI_OPCODE_CMP:
      src0 = src_vector(p, &inst->Src[0], fs);
      src1 = src_vector(p, &inst->Src[1], fs);
      src2 = src_vector(p, &inst->Src[2], fs);
      i915_emit_arith(p, A0_CMP, get_result_vector(p, &inst->Dst[0]),
                      get_result_flags(inst), 0, src0, src2,
                      src1); /* NOTE: order of src2, src1 */
      break;

   case TGSI_OPCODE_DDX:
   case TGSI_OPCODE_DDY:
      /* XXX We just output 0 here */
      debug_printf("Punting DDX/DDY\n");
      src0 = get_result_vector(p, &inst->Dst[0]);
      i915_emit_arith(p, A0_MOV, get_result_vector(p, &inst->Dst[0]),
                      get_result_flags(inst), 0,
                      swizzle(src0, ZERO, ZERO, ZERO, ZERO), 0, 0);
      break;

   case TGSI_OPCODE_DP2:
      src0 = src_vector(p, &inst->Src[0], fs);
      src1 = src_vector(p, &inst->Src[1], fs);

      i915_emit_arith(p, A0_DP3, get_result_vector(p, &inst->Dst[0]),
                      get_result_flags(inst), 0,
                      swizzle(src0, X, Y, ZERO, ZERO), src1, 0);
      break;

   case TGSI_OPCODE_DP3:
      emit_simple_arith(p, inst, A0_DP3, 2, fs);
      break;

   case TGSI_OPCODE_DP4:
      emit_simple_arith(p, inst, A0_DP4, 2, fs);
      break;

   case TGSI_OPCODE_DST:
      src0 = src_vector(p, &inst->Src[0], fs);
      src1 = src_vector(p, &inst->Src[1], fs);

      /* result[0] = 1    * 1;
       * result[1] = a[1] * b[1];
       * result[2] = a[2] * 1;
       * result[3] = 1    * b[3];
       */
      i915_emit_arith(p, A0_MUL, get_result_vector(p, &inst->Dst[0]),
                      get_result_flags(inst), 0, swizzle(src0, ONE, Y, Z, ONE),
                      swizzle(src1, ONE, Y, ONE, W), 0);
      break;

   case TGSI_OPCODE_END:
      /* no-op */
      break;

   case TGSI_OPCODE_EX2:
      src0 = src_vector(p, &inst->Src[0], fs);

      i915_emit_arith(p, A0_EXP, get_result_vector(p, &inst->Dst[0]),
                      get_result_flags(inst), 0, swizzle(src0, X, X, X, X), 0,
                      0);
      break;

   case TGSI_OPCODE_FLR:
      emit_simple_arith(p, inst, A0_FLR, 1, fs);
      break;

   case TGSI_OPCODE_FRC:
      emit_simple_arith(p, inst, A0_FRC, 1, fs);
      break;

   case TGSI_OPCODE_KILL_IF:
      /* kill if src[0].x < 0 || src[0].y < 0 ... */
      src0 = src_vector(p, &inst->Src[0], fs);
      tmp = i915_get_utemp(p);

      i915_emit_texld(p, tmp,               /* dest reg: a dummy reg */
                      A0_DEST_CHANNEL_ALL,  /* dest writemask */
                      0,                    /* sampler */
                      src0,                 /* coord*/
                      T0_TEXKILL,           /* opcode */
                      TGSI_WRITEMASK_XYZW); /* coord_mask */
      break;

   case TGSI_OPCODE_KILL:
      /* unconditional kill */
      tmp = i915_get_utemp(p);

      i915_emit_texld(p, tmp,              /* dest reg: a dummy reg */
                      A0_DEST_CHANNEL_ALL, /* dest writemask */
                      0,                   /* sampler */
                      negate(swizzle(UREG(REG_TYPE_R, 0), ONE, ONE, ONE, ONE),
                             1, 1, 1, 1), /* coord */
                      T0_TEXKILL,         /* opcode */
                      TGSI_WRITEMASK_X);  /* coord_mask */
      break;

   case TGSI_OPCODE_LG2:
      src0 = src_vector(p, &inst->Src[0], fs);

      i915_emit_arith(p, A0_LOG, get_result_vector(p, &inst->Dst[0]),
                      get_result_flags(inst), 0, swizzle(src0, X, X, X, X), 0,
                      0);
      break;

   case TGSI_OPCODE_LIT:
      src0 = src_vector(p, &inst->Src[0], fs);
      tmp = i915_get_utemp(p);

      /* tmp = max( a.xyzw, a.00zw )
       * XXX: Clamp tmp.w to -128..128
       * tmp.y = log(tmp.y)
       * tmp.y = tmp.w * tmp.y
       * tmp.y = exp(tmp.y)
       * result = cmp (a.11-x1, a.1x01, a.1xy1 )
       */
      i915_emit_arith(p, A0_MAX, tmp, A0_DEST_CHANNEL_ALL, 0, src0,
                      swizzle(src0, ZERO, ZERO, Z, W), 0);

      i915_emit_arith(p, A0_LOG, tmp, A0_DEST_CHANNEL_Y, 0,
                      swizzle(tmp, Y, Y, Y, Y), 0, 0);

      i915_emit_arith(p, A0_MUL, tmp, A0_DEST_CHANNEL_Y, 0,
                      swizzle(tmp, ZERO, Y, ZERO, ZERO),
                      swizzle(tmp, ZERO, W, ZERO, ZERO), 0);

      i915_emit_arith(p, A0_EXP, tmp, A0_DEST_CHANNEL_Y, 0,
                      swizzle(tmp, Y, Y, Y, Y), 0, 0);

      i915_emit_arith(
         p, A0_CMP, get_result_vector(p, &inst->Dst[0]), get_result_flags(inst),
         0, negate(swizzle(tmp, ONE, ONE, X, ONE), 0, 0, 1, 0),
         swizzle(tmp, ONE, X, ZERO, ONE), swizzle(tmp, ONE, X, Y, ONE));

      break;

   case TGSI_OPCODE_LRP:
      src0 = src_vector(p, &inst->Src[0], fs);
      src1 = src_vector(p, &inst->Src[1], fs);
      src2 = src_vector(p, &inst->Src[2], fs);
      flags = get_result_flags(inst);
      tmp = i915_get_utemp(p);

      /* b*a + c*(1-a)
       *
       * b*a + c - ca
       *
       * tmp = b*a + c,
       * result = (-c)*a + tmp
       */
      i915_emit_arith(p, A0_MAD, tmp, flags & A0_DEST_CHANNEL_ALL, 0, src1,
                      src0, src2);

      i915_emit_arith(p, A0_MAD, get_result_vector(p, &inst->Dst[0]), flags, 0,
                      negate(src2, 1, 1, 1, 1), src0, tmp);
      break;

   case TGSI_OPCODE_MAD:
      emit_simple_arith(p, inst, A0_MAD, 3, fs);
      break;

   case TGSI_OPCODE_MAX:
      emit_simple_arith(p, inst, A0_MAX, 2, fs);
      break;

   case TGSI_OPCODE_MIN:
      emit_simple_arith(p, inst, A0_MIN, 2, fs);
      break;

   case TGSI_OPCODE_MOV:
      emit_simple_arith(p, inst, A0_MOV, 1, fs);
      break;

   case TGSI_OPCODE_MUL:
      emit_simple_arith(p, inst, A0_MUL, 2, fs);
      break;

   case TGSI_OPCODE_NOP:
      break;

   case TGSI_OPCODE_POW:
      src0 = src_vector(p, &inst->Src[0], fs);
      src1 = src_vector(p, &inst->Src[1], fs);
      tmp = i915_get_utemp(p);
      flags = get_result_flags(inst);

      /* XXX: masking on intermediate values, here and elsewhere.
       */
      i915_emit_arith(p, A0_LOG, tmp, A0_DEST_CHANNEL_X, 0,
                      swizzle(src0, X, X, X, X), 0, 0);

      i915_emit_arith(p, A0_MUL, tmp, A0_DEST_CHANNEL_X, 0, tmp, src1, 0);

      i915_emit_arith(p, A0_EXP, get_result_vector(p, &inst->Dst[0]), flags, 0,
                      swizzle(tmp, X, X, X, X), 0, 0);
      break;

   case TGSI_OPCODE_RET:
      /* XXX: no-op? */
      break;

   case TGSI_OPCODE_RCP:
      src0 = src_vector(p, &inst->Src[0], fs);

      i915_emit_arith(p, A0_RCP, get_result_vector(p, &inst->Dst[0]),
                      get_result_flags(inst), 0, swizzle(src0, X, X, X, X), 0,
                      0);
      break;

   case TGSI_OPCODE_RSQ:
      src0 = src_vector(p, &inst->Src[0], fs);

      i915_emit_arith(p, A0_RSQ, get_result_vector(p, &inst->Dst[0]),
                      get_result_flags(inst), 0, swizzle(src0, X, X, X, X), 0,
                      0);
      break;

   case TGSI_OPCODE_SEQ: {
      const uint32_t zero =
         swizzle(UREG(REG_TYPE_R, 0), SRC_ZERO, SRC_ZERO, SRC_ZERO, SRC_ZERO);

      /* if we're both >= and <= then we're == */
      src0 = src_vector(p, &inst->Src[0], fs);
      src1 = src_vector(p, &inst->Src[1], fs);
      tmp = i915_get_utemp(p);

      if (src0 == zero || src1 == zero) {
         if (src0 == zero)
            src0 = src1;

         /* x == 0 is equivalent to -abs(x) >= 0, but the latter requires only
          * two instructions instead of three.
          */
         i915_emit_arith(p, A0_MAX, tmp, A0_DEST_CHANNEL_ALL, 0, src0,
                         negate(src0, 1, 1, 1, 1), 0);
         i915_emit_arith(p, A0_SGE, get_result_vector(p, &inst->Dst[0]),
                         get_result_flags(inst), 0, negate(tmp, 1, 1, 1, 1),
                         zero, 0);
      } else {
         i915_emit_arith(p, A0_SGE, tmp, A0_DEST_CHANNEL_ALL, 0, src0, src1, 0);

         i915_emit_arith(p, A0_SGE, get_result_vector(p, &inst->Dst[0]),
                         get_result_flags(inst), 0, src1, src0, 0);

         i915_emit_arith(p, A0_MUL, get_result_vector(p, &inst->Dst[0]),
                         get_result_flags(inst), 0,
                         get_result_vector(p, &inst->Dst[0]), tmp, 0);
      }

      break;
   }

   case TGSI_OPCODE_SGE:
      emit_simple_arith(p, inst, A0_SGE, 2, fs);
      break;

   case TGSI_OPCODE_SLE:
      /* like SGE, but swap reg0, reg1 */
      emit_simple_arith_swap2(p, inst, A0_SGE, 2, fs);
      break;

   case TGSI_OPCODE_SLT:
      emit_simple_arith(p, inst, A0_SLT, 2, fs);
      break;

   case TGSI_OPCODE_SGT:
      /* like SLT, but swap reg0, reg1 */
      emit_simple_arith_swap2(p, inst, A0_SLT, 2, fs);
      break;

   case TGSI_OPCODE_SNE: {
      const uint32_t zero =
         swizzle(UREG(REG_TYPE_R, 0), SRC_ZERO, SRC_ZERO, SRC_ZERO, SRC_ZERO);

      /* if we're < or > then we're != */
      src0 = src_vector(p, &inst->Src[0], fs);
      src1 = src_vector(p, &inst->Src[1], fs);
      tmp = i915_get_utemp(p);

      if (src0 == zero || src1 == zero) {
         if (src0 == zero)
            src0 = src1;

         /* x != 0 is equivalent to -abs(x) < 0, but the latter requires only
          * two instructions instead of three.
          */
         i915_emit_arith(p, A0_MAX, tmp, A0_DEST_CHANNEL_ALL, 0, src0,
                         negate(src0, 1, 1, 1, 1), 0);
         i915_emit_arith(p, A0_SLT, get_result_vector(p, &inst->Dst[0]),
                         get_result_flags(inst), 0, negate(tmp, 1, 1, 1, 1),
                         zero, 0);
      } else {
         i915_emit_arith(p, A0_SLT, tmp, A0_DEST_CHANNEL_ALL, 0, src0, src1, 0);

         i915_emit_arith(p, A0_SLT, get_result_vector(p, &inst->Dst[0]),
                         get_result_flags(inst), 0, src1, src0, 0);

         i915_emit_arith(p, A0_ADD, get_result_vector(p, &inst->Dst[0]),
                         get_result_flags(inst), 0,
                         get_result_vector(p, &inst->Dst[0]), tmp, 0);
      }
      break;
   }

   case TGSI_OPCODE_SSG:
      /* compute (src>0) - (src<0) */
      src0 = src_vector(p, &inst->Src[0], fs);
      tmp = i915_get_utemp(p);

      i915_emit_arith(p, A0_SLT, tmp, A0_DEST_CHANNEL_ALL, 0, src0,
                      swizzle(src0, ZERO, ZERO, ZERO, ZERO), 0);

      i915_emit_arith(p, A0_SLT, get_result_vector(p, &inst->Dst[0]),
                      get_result_flags(inst), 0,
                      swizzle(src0, ZERO, ZERO, ZERO, ZERO), src0, 0);

      i915_emit_arith(
         p, A0_ADD, get_result_vector(p, &inst->Dst[0]), get_result_flags(inst),
         0, get_result_vector(p, &inst->Dst[0]), negate(tmp, 1, 1, 1, 1), 0);
      break;

   case TGSI_OPCODE_TEX:
      emit_tex(p, inst, T0_TEXLD, fs);
      break;

   case TGSI_OPCODE_TRUNC:
      emit_simple_arith(p, inst, A0_TRC, 1, fs);
      break;

   case TGSI_OPCODE_TXB:
      emit_tex(p, inst, T0_TEXLDB, fs);
      break;

   case TGSI_OPCODE_TXP:
      emit_tex(p, inst, T0_TEXLDP, fs);
      break;

   default:
      i915_program_error(p, "bad opcode %s (%d)",
                         tgsi_get_opcode_name(inst->Instruction.Opcode),
                         inst->Instruction.Opcode);
      return;
   }

   i915_release_utemps(p);
}

static void
i915_translate_token(struct i915_fp_compile *p,
                     const union i915_full_token *token,
                     struct i915_fragment_shader *fs)
{
   struct i915_fragment_shader *ifs = p->shader;
   switch (token->Token.Type) {
   case TGSI_TOKEN_TYPE_PROPERTY:
      /* Ignore properties where we only support one value. */
      assert(token->FullProperty.Property.PropertyName ==
                TGSI_PROPERTY_FS_COORD_ORIGIN ||
             token->FullProperty.Property.PropertyName ==
                TGSI_PROPERTY_FS_COORD_PIXEL_CENTER ||
             token->FullProperty.Property.PropertyName ==
                TGSI_PROPERTY_FS_COLOR0_WRITES_ALL_CBUFS ||
             token->FullProperty.Property.PropertyName ==
                TGSI_PROPERTY_SEPARABLE_PROGRAM);
      break;

   case TGSI_TOKEN_TYPE_DECLARATION:
      if (token->FullDeclaration.Declaration.File == TGSI_FILE_CONSTANT) {
         if (token->FullDeclaration.Range.Last >= I915_MAX_CONSTANT) {
            i915_program_error(p, "Exceeded %d max uniforms",
                               I915_MAX_CONSTANT);
         } else {
            uint32_t i;
            for (i = token->FullDeclaration.Range.First;
                 i <= token->FullDeclaration.Range.Last; i++) {
               ifs->constant_flags[i] = I915_CONSTFLAG_USER;
               ifs->num_constants = MAX2(ifs->num_constants, i + 1);
            }
         }
      } else if (token->FullDeclaration.Declaration.File ==
                 TGSI_FILE_TEMPORARY) {
         if (token->FullDeclaration.Range.Last >= I915_MAX_TEMPORARY) {
            i915_program_error(p, "Exceeded max TGSI temps (%d/%d)",
                               token->FullDeclaration.Range.Last + 1, I915_MAX_TEMPORARY);
         } else {
            uint32_t i;
            for (i = token->FullDeclaration.Range.First;
                 i <= token->FullDeclaration.Range.Last; i++) {
               /* XXX just use shader->info->file_mask[TGSI_FILE_TEMPORARY] */
               p->temp_flag |= (1 << i); /* mark temp as used */
            }
         }
      }
      break;

   case TGSI_TOKEN_TYPE_IMMEDIATE: {
      const struct tgsi_full_immediate *imm = &token->FullImmediate;
      const uint32_t pos = p->num_immediates++;
      uint32_t j;
      assert(imm->Immediate.NrTokens <= 4 + 1);
      for (j = 0; j < imm->Immediate.NrTokens - 1; j++) {
         p->immediates[pos][j] = imm->u[j].Float;
      }
   } break;

   case TGSI_TOKEN_TYPE_INSTRUCTION:
      if (p->first_instruction) {
         /* resolve location of immediates */
         uint32_t i, j;
         for (i = 0; i < p->num_immediates; i++) {
            /* find constant slot for this immediate */
            for (j = 0; j < I915_MAX_CONSTANT; j++) {
               if (ifs->constant_flags[j] == 0x0) {
                  memcpy(ifs->constants[j], p->immediates[i],
                         4 * sizeof(float));
                  /*printf("immediate %d maps to const %d\n", i, j);*/
                  ifs->constant_flags[j] = 0xf; /* all four comps used */
                  p->immediates_map[i] = j;
                  ifs->num_constants = MAX2(ifs->num_constants, j + 1);
                  break;
               }
            }
            if (j == I915_MAX_CONSTANT) {
               i915_program_error(p, "Exceeded %d max uniforms and immediates.",
                                  I915_MAX_CONSTANT);
            }
         }

         p->first_instruction = false;
      }

      i915_translate_instruction(p, &token->FullInstruction, fs);
      break;

   default:
      assert(0);
   }
}

/**
 * Translate TGSI fragment shader into i915 hardware instructions.
 * \param p  the translation state
 * \param tokens  the TGSI token array
 */
static void
i915_translate_instructions(struct i915_fp_compile *p,
                            const struct i915_token_list *tokens,
                            struct i915_fragment_shader *fs)
{
   int i;
   for (i = 0; i < tokens->NumTokens && !p->error[0]; i++) {
      i915_translate_token(p, &tokens->Tokens[i], fs);
   }
}

static struct i915_fp_compile *
i915_init_compile(struct i915_fragment_shader *ifs)
{
   struct i915_fp_compile *p = CALLOC_STRUCT(i915_fp_compile);
   int i;

   p->shader = ifs;
   p->error = ralloc_strdup(NULL, "");

   /* Put new constants at end of const buffer, growing downward.
    * The problem is we don't know how many user-defined constants might
    * be specified with pipe->set_constant_buffer().
    * Should pre-scan the user's program to determine the highest-numbered
    * constant referenced.
    */
   ifs->num_constants = 0;
   memset(ifs->constant_flags, 0, sizeof(ifs->constant_flags));

   memset(&p->register_phases, 0, sizeof(p->register_phases));

   for (i = 0; i < I915_TEX_UNITS; i++)
      ifs->texcoords[i].semantic = -1;

   p->first_instruction = true;

   p->nr_tex_indirect = 1; /* correct? */
   p->nr_tex_insn = 0;
   p->nr_alu_insn = 0;
   p->nr_decl_insn = 0;

   p->csr = p->program;
   p->decl = p->declarations;
   p->decl_s = 0;
   p->decl_t = 0;
   p->temp_flag = ~0x0U << I915_MAX_TEMPORARY;
   p->utemp_flag = ~0x7;

   /* initialize the first program word */
   *(p->decl++) = _3DSTATE_PIXEL_SHADER_PROGRAM;

   return p;
}

/* Copy compile results to the fragment program struct and destroy the
 * compilation context.
 */
static void
i915_fini_compile(struct i915_context *i915, struct i915_fp_compile *p)
{
   struct i915_fragment_shader *ifs = p->shader;
   unsigned long program_size = (unsigned long)(p->csr - p->program);
   unsigned long decl_size = (unsigned long)(p->decl - p->declarations);

   if (p->nr_tex_indirect > I915_MAX_TEX_INDIRECT) {
      i915_program_error(p,
                         "Exceeded max nr indirect texture lookups (%d/%d)\n",
                         p->nr_tex_indirect, I915_MAX_TEX_INDIRECT);
   }

   if (p->nr_tex_insn > I915_MAX_TEX_INSN) {
      i915_program_error(p, "Exceeded max TEX instructions (%d/%d)",
                         p->nr_tex_insn, I915_MAX_TEX_INSN);
   }

   if (p->nr_alu_insn > I915_MAX_ALU_INSN) {
      i915_program_error(p, "Exceeded max ALU instructions (%d/%d)",
                         p->nr_alu_insn, I915_MAX_ALU_INSN);
   }

   if (p->nr_decl_insn > I915_MAX_DECL_INSN) {
      i915_program_error(p, "Exceeded max DECL instructions (%d/%d)",
                         p->nr_decl_insn, I915_MAX_DECL_INSN);
   }

   /* hw doesn't seem to like empty frag programs (num_instructions == 1 is just
    * TGSI_END), even when the depth write fixup gets emitted below - maybe that
    * one is fishy, too?
    */
   if (ifs->info.num_instructions == 1)
      i915_program_error(p, "Empty fragment shader");

   if (strlen(p->error) != 0) {
      p->NumNativeInstructions = 0;
      p->NumNativeAluInstructions = 0;
      p->NumNativeTexInstructions = 0;
      p->NumNativeTexIndirections = 0;

      i915_use_passthrough_shader(ifs);
   } else {
      p->NumNativeInstructions =
         p->nr_alu_insn + p->nr_tex_insn + p->nr_decl_insn;
      p->NumNativeAluInstructions = p->nr_alu_insn;
      p->NumNativeTexInstructions = p->nr_tex_insn;
      p->NumNativeTexIndirections = p->nr_tex_indirect;

      /* patch in the program length */
      p->declarations[0] |= program_size + decl_size - 2;

      /* Copy compilation results to fragment program struct:
       */
      assert(!ifs->program);

      ifs->program_len = decl_size + program_size;
      ifs->program = (uint32_t *)MALLOC(ifs->program_len * sizeof(uint32_t));
      memcpy(ifs->program, p->declarations, decl_size * sizeof(uint32_t));
      memcpy(&ifs->program[decl_size], p->program,
             program_size * sizeof(uint32_t));

      if (i915) {
         util_debug_message(
            &i915->debug, SHADER_INFO,
            "%s shader: %d inst, %d tex, %d tex_indirect, %d temps, %d const",
            _mesa_shader_stage_to_abbrev(MESA_SHADER_FRAGMENT),
            (int)program_size, p->nr_tex_insn, p->nr_tex_indirect,
            p->shader->info.file_max[TGSI_FILE_TEMPORARY] + 1,
            ifs->num_constants);
      }
   }

   if (strlen(p->error) != 0)
      ifs->error = p->error;
   else
      ralloc_free(p->error);

   /* Release the compilation struct:
    */
   FREE(p);
}

/**
 * Rather than trying to intercept and jiggle depth writes during
 * emit, just move the value into its correct position at the end of
 * the program:
 */
static void
i915_fixup_depth_write(struct i915_fp_compile *p)
{
   for (int i = 0; i < p->shader->info.num_outputs; i++) {
      if (p->shader->info.output_semantic_name[i] != TGSI_SEMANTIC_POSITION)
         continue;

      const uint32_t depth = UREG(REG_TYPE_OD, 0);

      i915_emit_arith(p, A0_MOV,                  /* opcode */
                      depth,                      /* dest reg */
                      A0_DEST_CHANNEL_W,          /* write mask */
                      0,                          /* saturate? */
                      swizzle(depth, X, Y, Z, Z), /* src0 */
                      0, 0 /* src1, src2 */);
   }
}

void
i915_translate_fragment_program(struct i915_context *i915,
                                struct i915_fragment_shader *fs)
{
   struct i915_fp_compile *p;
   const struct tgsi_token *tokens = fs->state.tokens;
   struct i915_token_list *i_tokens;
   bool debug =
      I915_DBG_ON(DBG_FS) && (!fs->internal || NIR_DEBUG(PRINT_INTERNAL));

   if (debug) {
      mesa_logi("TGSI fragment shader:");
      tgsi_dump(tokens, 0);
   }

   p = i915_init_compile(fs);

   i_tokens = i915_optimize(tokens);
   i915_translate_instructions(p, i_tokens, fs);
   i915_fixup_depth_write(p);

   i915_fini_compile(i915, p);
   i915_optimize_free(i_tokens);

   if (debug) {
      if (fs->error)
         mesa_loge("%s", fs->error);

      mesa_logi("i915 fragment shader with %d constants%s", fs->num_constants,
                fs->num_constants ? ":" : "");

      for (int i = 0; i < I915_MAX_CONSTANT; i++) {
         if (fs->constant_flags[i] &&
             fs->constant_flags[i] != I915_CONSTFLAG_USER) {
            mesa_logi("\t\tC[%d] = { %f, %f, %f, %f }", i, fs->constants[i][0],
                      fs->constants[i][1], fs->constants[i][2],
                      fs->constants[i][3]);
         }
      }
      i915_disassemble_program(fs->program, fs->program_len);
   }
}
