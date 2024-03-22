/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/


#include "pipe/p_shader_tokens.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_parse.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_pstipple.h"

#include "svga_tgsi_emit.h"
#include "svga_context.h"


static bool emit_vs_postamble( struct svga_shader_emitter *emit );
static bool emit_ps_postamble( struct svga_shader_emitter *emit );


static SVGA3dShaderOpCodeType
translate_opcode(enum tgsi_opcode opcode)
{
   switch (opcode) {
   case TGSI_OPCODE_ADD:        return SVGA3DOP_ADD;
   case TGSI_OPCODE_DP3:        return SVGA3DOP_DP3;
   case TGSI_OPCODE_DP4:        return SVGA3DOP_DP4;
   case TGSI_OPCODE_FRC:        return SVGA3DOP_FRC;
   case TGSI_OPCODE_MAD:        return SVGA3DOP_MAD;
   case TGSI_OPCODE_MAX:        return SVGA3DOP_MAX;
   case TGSI_OPCODE_MIN:        return SVGA3DOP_MIN;
   case TGSI_OPCODE_MOV:        return SVGA3DOP_MOV;
   case TGSI_OPCODE_MUL:        return SVGA3DOP_MUL;
   case TGSI_OPCODE_NOP:        return SVGA3DOP_NOP;
   default:
      assert(!"svga: unexpected opcode in translate_opcode()");
      return SVGA3DOP_LAST_INST;
   }
}


static SVGA3dShaderRegType
translate_file(enum tgsi_file_type file)
{
   switch (file) {
   case TGSI_FILE_TEMPORARY: return SVGA3DREG_TEMP;
   case TGSI_FILE_INPUT:     return SVGA3DREG_INPUT;
   case TGSI_FILE_OUTPUT:    return SVGA3DREG_OUTPUT; /* VS3.0+ only */
   case TGSI_FILE_IMMEDIATE: return SVGA3DREG_CONST;
   case TGSI_FILE_CONSTANT:  return SVGA3DREG_CONST;
   case TGSI_FILE_SAMPLER:   return SVGA3DREG_SAMPLER;
   case TGSI_FILE_ADDRESS:   return SVGA3DREG_ADDR;
   default:
      assert(!"svga: unexpected register file in translate_file()");
      return SVGA3DREG_TEMP;
   }
}


/**
 * Translate a TGSI destination register to an SVGA3DShaderDestToken.
 * \param insn  the TGSI instruction
 * \param idx  which TGSI dest register to translate (usually (always?) zero)
 */
static SVGA3dShaderDestToken
translate_dst_register( struct svga_shader_emitter *emit,
                        const struct tgsi_full_instruction *insn,
                        unsigned idx )
{
   const struct tgsi_full_dst_register *reg = &insn->Dst[idx];
   SVGA3dShaderDestToken dest;

   switch (reg->Register.File) {
   case TGSI_FILE_OUTPUT:
      /* Output registers encode semantic information in their name.
       * Need to lookup a table built at decl time:
       */
      dest = emit->output_map[reg->Register.Index];
      emit->num_output_writes++;
      break;

   default:
      {
         unsigned index = reg->Register.Index;
         assert(index < SVGA3D_TEMPREG_MAX);
         index = MIN2(index, SVGA3D_TEMPREG_MAX - 1);
         dest = dst_register(translate_file(reg->Register.File), index);
      }
      break;
   }

   if (reg->Register.Indirect) {
      debug_warning("Indirect indexing of dest registers is not supported!\n");
   }

   dest.mask = reg->Register.WriteMask;
   assert(dest.mask);

   if (insn->Instruction.Saturate)
      dest.dstMod = SVGA3DDSTMOD_SATURATE;

   return dest;
}


/**
 * Apply a swizzle to a src_register, returning a new src_register
 * Ex: swizzle(SRC.ZZYY, SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y)
 * would return SRC.YYZZ
 */
static struct src_register
swizzle(struct src_register src,
        unsigned x, unsigned y, unsigned z, unsigned w)
{
   assert(x < 4);
   assert(y < 4);
   assert(z < 4);
   assert(w < 4);
   x = (src.base.swizzle >> (x * 2)) & 0x3;
   y = (src.base.swizzle >> (y * 2)) & 0x3;
   z = (src.base.swizzle >> (z * 2)) & 0x3;
   w = (src.base.swizzle >> (w * 2)) & 0x3;

   src.base.swizzle = TRANSLATE_SWIZZLE(x, y, z, w);

   return src;
}


/**
 * Apply a "scalar" swizzle to a src_register returning a new
 * src_register where all the swizzle terms are the same.
 * Ex: scalar(SRC.WZYX, SWIZZLE_Y) would return SRC.ZZZZ
 */
static struct src_register
scalar(struct src_register src, unsigned comp)
{
   assert(comp < 4);
   return swizzle( src, comp, comp, comp, comp );
}


static bool
svga_arl_needs_adjustment( const struct svga_shader_emitter *emit )
{
   unsigned i;

   for (i = 0; i < emit->num_arl_consts; ++i) {
      if (emit->arl_consts[i].arl_num == emit->current_arl)
         return true;
   }
   return false;
}


static int
svga_arl_adjustment( const struct svga_shader_emitter *emit )
{
   unsigned i;

   for (i = 0; i < emit->num_arl_consts; ++i) {
      if (emit->arl_consts[i].arl_num == emit->current_arl)
         return emit->arl_consts[i].number;
   }
   return 0;
}


/**
 * Translate a TGSI src register to a src_register.
 */
static struct src_register
translate_src_register( const struct svga_shader_emitter *emit,
                        const struct tgsi_full_src_register *reg )
{
   struct src_register src;

   switch (reg->Register.File) {
   case TGSI_FILE_INPUT:
      /* Input registers are referred to by their semantic name rather
       * than by index.  Use the mapping build up from the decls:
       */
      src = emit->input_map[reg->Register.Index];
      break;

   case TGSI_FILE_IMMEDIATE:
      /* Immediates are appended after TGSI constants in the D3D
       * constant buffer.
       */
      src = src_register( translate_file( reg->Register.File ),
                          reg->Register.Index + emit->imm_start );
      break;

   default:
      src = src_register( translate_file( reg->Register.File ),
                          reg->Register.Index );
      break;
   }

   /* Indirect addressing.
    */
   if (reg->Register.Indirect) {
      if (emit->unit == PIPE_SHADER_FRAGMENT) {
         /* Pixel shaders have only loop registers for relative
          * addressing into inputs. Ignore the redundant address
          * register, the contents of aL should be in sync with it.
          */
         if (reg->Register.File == TGSI_FILE_INPUT) {
            src.base.relAddr = 1;
            src.indirect = src_token(SVGA3DREG_LOOP, 0);
         }
      }
      else {
         /* Constant buffers only.
          */
         if (reg->Register.File == TGSI_FILE_CONSTANT) {
            /* we shift the offset towards the minimum */
            if (svga_arl_needs_adjustment( emit )) {
               src.base.num -= svga_arl_adjustment( emit );
            }
            src.base.relAddr = 1;

            /* Not really sure what should go in the second token:
             */
            src.indirect = src_token( SVGA3DREG_ADDR,
                                      reg->Indirect.Index );

            src.indirect.swizzle = SWIZZLE_XXXX;
         }
      }
   }

   src = swizzle( src,
                  reg->Register.SwizzleX,
                  reg->Register.SwizzleY,
                  reg->Register.SwizzleZ,
                  reg->Register.SwizzleW );

   /* src.mod isn't a bitfield, unfortunately */
   if (reg->Register.Absolute) {
      if (reg->Register.Negate)
         src.base.srcMod = SVGA3DSRCMOD_ABSNEG;
      else
         src.base.srcMod = SVGA3DSRCMOD_ABS;
   }
   else {
      if (reg->Register.Negate)
         src.base.srcMod = SVGA3DSRCMOD_NEG;
      else
         src.base.srcMod = SVGA3DSRCMOD_NONE;
   }

   return src;
}


/*
 * Get a temporary register.
 * Note: if we exceed the temporary register limit we just use
 * register SVGA3D_TEMPREG_MAX - 1.
 */
static SVGA3dShaderDestToken
get_temp( struct svga_shader_emitter *emit )
{
   int i = emit->nr_hw_temp + emit->internal_temp_count++;
   if (i >= SVGA3D_TEMPREG_MAX) {
      debug_warn_once("svga: Too many temporary registers used in shader\n");
      i = SVGA3D_TEMPREG_MAX - 1;
   }
   return dst_register( SVGA3DREG_TEMP, i );
}


/**
 * Release a single temp.  Currently only effective if it was the last
 * allocated temp, otherwise release will be delayed until the next
 * call to reset_temp_regs().
 */
static void
release_temp( struct svga_shader_emitter *emit,
              SVGA3dShaderDestToken temp )
{
   if (temp.num == emit->internal_temp_count - 1)
      emit->internal_temp_count--;
}


/**
 * Release all temps.
 */
static void
reset_temp_regs(struct svga_shader_emitter *emit)
{
   emit->internal_temp_count = 0;
}


/** Emit bytecode for a src_register */
static bool
emit_src(struct svga_shader_emitter *emit, const struct src_register src)
{
   if (src.base.relAddr) {
      assert(src.base.reserved0);
      assert(src.indirect.reserved0);
      return (svga_shader_emit_dword( emit, src.base.value ) &&
              svga_shader_emit_dword( emit, src.indirect.value ));
   }
   else {
      assert(src.base.reserved0);
      return svga_shader_emit_dword( emit, src.base.value );
   }
}


/** Emit bytecode for a dst_register */
static bool
emit_dst(struct svga_shader_emitter *emit, SVGA3dShaderDestToken dest)
{
   assert(dest.reserved0);
   assert(dest.mask);
   return svga_shader_emit_dword( emit, dest.value );
}


/** Emit bytecode for a 1-operand instruction */
static bool
emit_op1(struct svga_shader_emitter *emit,
         SVGA3dShaderInstToken inst,
         SVGA3dShaderDestToken dest,
         struct src_register src0)
{
   return (emit_instruction(emit, inst) &&
           emit_dst(emit, dest) &&
           emit_src(emit, src0));
}


/** Emit bytecode for a 2-operand instruction */
static bool
emit_op2(struct svga_shader_emitter *emit,
         SVGA3dShaderInstToken inst,
         SVGA3dShaderDestToken dest,
         struct src_register src0,
         struct src_register src1)
{
   return (emit_instruction(emit, inst) &&
           emit_dst(emit, dest) &&
           emit_src(emit, src0) &&
           emit_src(emit, src1));
}


/** Emit bytecode for a 3-operand instruction */
static bool
emit_op3(struct svga_shader_emitter *emit,
         SVGA3dShaderInstToken inst,
         SVGA3dShaderDestToken dest,
         struct src_register src0,
         struct src_register src1,
         struct src_register src2)
{
   return (emit_instruction(emit, inst) &&
           emit_dst(emit, dest) &&
           emit_src(emit, src0) &&
           emit_src(emit, src1) &&
           emit_src(emit, src2));
}


/** Emit bytecode for a 4-operand instruction */
static bool
emit_op4(struct svga_shader_emitter *emit,
         SVGA3dShaderInstToken inst,
         SVGA3dShaderDestToken dest,
         struct src_register src0,
         struct src_register src1,
         struct src_register src2,
         struct src_register src3)
{
   return (emit_instruction(emit, inst) &&
           emit_dst(emit, dest) &&
           emit_src(emit, src0) &&
           emit_src(emit, src1) &&
           emit_src(emit, src2) &&
           emit_src(emit, src3));
}


/**
 * Apply the absolute value modifier to the given src_register, returning
 * a new src_register.
 */
static struct src_register 
absolute(struct src_register src)
{
   src.base.srcMod = SVGA3DSRCMOD_ABS;
   return src;
}


/**
 * Apply the negation modifier to the given src_register, returning
 * a new src_register.
 */
static struct src_register 
negate(struct src_register src)
{
   switch (src.base.srcMod) {
   case SVGA3DSRCMOD_ABS:
      src.base.srcMod = SVGA3DSRCMOD_ABSNEG;
      break;
   case SVGA3DSRCMOD_ABSNEG:
      src.base.srcMod = SVGA3DSRCMOD_ABS;
      break;
   case SVGA3DSRCMOD_NEG:
      src.base.srcMod = SVGA3DSRCMOD_NONE;
      break;
   case SVGA3DSRCMOD_NONE:
      src.base.srcMod = SVGA3DSRCMOD_NEG;
      break;
   }
   return src;
}



/* Replace the src with the temporary specified in the dst, but copying
 * only the necessary channels, and preserving the original swizzle (which is
 * important given that several opcodes have constraints in the allowed
 * swizzles).
 */
static bool
emit_repl(struct svga_shader_emitter *emit,
          SVGA3dShaderDestToken dst,
          struct src_register *src0)
{
   unsigned src0_swizzle;
   unsigned chan;

   assert(SVGA3dShaderGetRegType(dst.value) == SVGA3DREG_TEMP);

   src0_swizzle = src0->base.swizzle;

   dst.mask = 0;
   for (chan = 0; chan < 4; ++chan) {
      unsigned swizzle = (src0_swizzle >> (chan *2)) & 0x3;
      dst.mask |= 1 << swizzle;
   }
   assert(dst.mask);

   src0->base.swizzle = SVGA3DSWIZZLE_NONE;

   if (!emit_op1( emit, inst_token( SVGA3DOP_MOV ), dst, *src0 ))
      return false;

   *src0 = src( dst );
   src0->base.swizzle = src0_swizzle;

   return true;
}


/**
 * Submit/emit an instruction with zero operands.
 */
static bool
submit_op0(struct svga_shader_emitter *emit,
           SVGA3dShaderInstToken inst,
           SVGA3dShaderDestToken dest)
{
   return (emit_instruction( emit, inst ) &&
           emit_dst( emit, dest ));
}


/**
 * Submit/emit an instruction with one operand.
 */
static bool
submit_op1(struct svga_shader_emitter *emit,
           SVGA3dShaderInstToken inst,
           SVGA3dShaderDestToken dest,
           struct src_register src0)
{
   return emit_op1( emit, inst, dest, src0 );
}


/**
 * Submit/emit an instruction with two operands.
 *
 * SVGA shaders may not refer to >1 constant register in a single
 * instruction.  This function checks for that usage and inserts a
 * move to temporary if detected.
 *
 * The same applies to input registers -- at most a single input
 * register may be read by any instruction.
 */
static bool
submit_op2(struct svga_shader_emitter *emit,
           SVGA3dShaderInstToken inst,
           SVGA3dShaderDestToken dest,
           struct src_register src0,
           struct src_register src1)
{
   SVGA3dShaderDestToken temp;
   SVGA3dShaderRegType type0, type1;
   bool need_temp = false;

   temp.value = 0;
   type0 = SVGA3dShaderGetRegType( src0.base.value );
   type1 = SVGA3dShaderGetRegType( src1.base.value );

   if (type0 == SVGA3DREG_CONST &&
       type1 == SVGA3DREG_CONST &&
       src0.base.num != src1.base.num)
      need_temp = true;

   if (type0 == SVGA3DREG_INPUT &&
       type1 == SVGA3DREG_INPUT &&
       src0.base.num != src1.base.num)
      need_temp = true;

   if (need_temp) {
      temp = get_temp( emit );

      if (!emit_repl( emit, temp, &src0 ))
         return false;
   }

   if (!emit_op2( emit, inst, dest, src0, src1 ))
      return false;

   if (need_temp)
      release_temp( emit, temp );

   return true;
}


/**
 * Submit/emit an instruction with three operands.
 *
 * SVGA shaders may not refer to >1 constant register in a single
 * instruction.  This function checks for that usage and inserts a
 * move to temporary if detected.
 */
static bool
submit_op3(struct svga_shader_emitter *emit,
           SVGA3dShaderInstToken inst,
           SVGA3dShaderDestToken dest,
           struct src_register src0,
           struct src_register src1,
           struct src_register src2)
{
   SVGA3dShaderDestToken temp0;
   SVGA3dShaderDestToken temp1;
   bool need_temp0 = false;
   bool need_temp1 = false;
   SVGA3dShaderRegType type0, type1, type2;

   temp0.value = 0;
   temp1.value = 0;
   type0 = SVGA3dShaderGetRegType( src0.base.value );
   type1 = SVGA3dShaderGetRegType( src1.base.value );
   type2 = SVGA3dShaderGetRegType( src2.base.value );

   if (inst.op != SVGA3DOP_SINCOS) {
      if (type0 == SVGA3DREG_CONST &&
          ((type1 == SVGA3DREG_CONST && src0.base.num != src1.base.num) ||
           (type2 == SVGA3DREG_CONST && src0.base.num != src2.base.num)))
         need_temp0 = true;

      if (type1 == SVGA3DREG_CONST &&
          (type2 == SVGA3DREG_CONST && src1.base.num != src2.base.num))
         need_temp1 = true;
   }

   if (type0 == SVGA3DREG_INPUT &&
       ((type1 == SVGA3DREG_INPUT && src0.base.num != src1.base.num) ||
        (type2 == SVGA3DREG_INPUT && src0.base.num != src2.base.num)))
      need_temp0 = true;

   if (type1 == SVGA3DREG_INPUT &&
       (type2 == SVGA3DREG_INPUT && src1.base.num != src2.base.num))
      need_temp1 = true;

   if (need_temp0) {
      temp0 = get_temp( emit );

      if (!emit_repl( emit, temp0, &src0 ))
         return false;
   }

   if (need_temp1) {
      temp1 = get_temp( emit );

      if (!emit_repl( emit, temp1, &src1 ))
         return false;
   }

   if (!emit_op3( emit, inst, dest, src0, src1, src2 ))
      return false;

   if (need_temp1)
      release_temp( emit, temp1 );
   if (need_temp0)
      release_temp( emit, temp0 );
   return true;
}


/**
 * Submit/emit an instruction with four operands.
 *
 * SVGA shaders may not refer to >1 constant register in a single
 * instruction.  This function checks for that usage and inserts a
 * move to temporary if detected.
 */
static bool
submit_op4(struct svga_shader_emitter *emit,
           SVGA3dShaderInstToken inst,
           SVGA3dShaderDestToken dest,
           struct src_register src0,
           struct src_register src1,
           struct src_register src2,
           struct src_register src3)
{
   SVGA3dShaderDestToken temp0;
   SVGA3dShaderDestToken temp3;
   bool need_temp0 = false;
   bool need_temp3 = false;
   SVGA3dShaderRegType type0, type1, type2, type3;

   temp0.value = 0;
   temp3.value = 0;
   type0 = SVGA3dShaderGetRegType( src0.base.value );
   type1 = SVGA3dShaderGetRegType( src1.base.value );
   type2 = SVGA3dShaderGetRegType( src2.base.value );
   type3 = SVGA3dShaderGetRegType( src2.base.value );

   /* Make life a little easier - this is only used by the TXD
    * instruction which is guaranteed not to have a constant/input reg
    * in one slot at least:
    */
   assert(type1 == SVGA3DREG_SAMPLER);
   (void) type1;

   if (type0 == SVGA3DREG_CONST &&
       ((type3 == SVGA3DREG_CONST && src0.base.num != src3.base.num) ||
        (type2 == SVGA3DREG_CONST && src0.base.num != src2.base.num)))
      need_temp0 = true;

   if (type3 == SVGA3DREG_CONST &&
       (type2 == SVGA3DREG_CONST && src3.base.num != src2.base.num))
      need_temp3 = true;

   if (type0 == SVGA3DREG_INPUT &&
       ((type3 == SVGA3DREG_INPUT && src0.base.num != src3.base.num) ||
        (type2 == SVGA3DREG_INPUT && src0.base.num != src2.base.num)))
      need_temp0 = true;

   if (type3 == SVGA3DREG_INPUT &&
       (type2 == SVGA3DREG_INPUT && src3.base.num != src2.base.num))
      need_temp3 = true;

   if (need_temp0) {
      temp0 = get_temp( emit );

      if (!emit_repl( emit, temp0, &src0 ))
         return false;
   }

   if (need_temp3) {
      temp3 = get_temp( emit );

      if (!emit_repl( emit, temp3, &src3 ))
         return false;
   }

   if (!emit_op4( emit, inst, dest, src0, src1, src2, src3 ))
      return false;

   if (need_temp3)
      release_temp( emit, temp3 );
   if (need_temp0)
      release_temp( emit, temp0 );
   return true;
}


/**
 * Do the src and dest registers refer to the same register?
 */
static bool
alias_src_dst(struct src_register src,
              SVGA3dShaderDestToken dst)
{
   if (src.base.num != dst.num)
      return false;

   if (SVGA3dShaderGetRegType(dst.value) !=
       SVGA3dShaderGetRegType(src.base.value))
      return false;

   return true;
}


/**
 * Helper for emitting SVGA immediate values using the SVGA3DOP_DEF[I]
 * instructions.
 */
static bool
emit_def_const(struct svga_shader_emitter *emit,
               SVGA3dShaderConstType type,
               unsigned idx, float a, float b, float c, float d)
{
   SVGA3DOpDefArgs def;
   SVGA3dShaderInstToken opcode;

   switch (type) {
   case SVGA3D_CONST_TYPE_FLOAT:
      opcode = inst_token( SVGA3DOP_DEF );
      def.dst = dst_register( SVGA3DREG_CONST, idx );
      def.constValues[0] = a;
      def.constValues[1] = b;
      def.constValues[2] = c;
      def.constValues[3] = d;
      break;
   case SVGA3D_CONST_TYPE_INT:
      opcode = inst_token( SVGA3DOP_DEFI );
      def.dst = dst_register( SVGA3DREG_CONSTINT, idx );
      def.constIValues[0] = (int)a;
      def.constIValues[1] = (int)b;
      def.constIValues[2] = (int)c;
      def.constIValues[3] = (int)d;
      break;
   default:
      assert(0);
      opcode = inst_token( SVGA3DOP_NOP );
      break;
   }

   if (!emit_instruction(emit, opcode) ||
       !svga_shader_emit_dwords( emit, def.values, ARRAY_SIZE(def.values)))
      return false;

   return true;
}


static bool
create_loop_const( struct svga_shader_emitter *emit )
{
   unsigned idx = emit->nr_hw_int_const++;

   if (!emit_def_const( emit, SVGA3D_CONST_TYPE_INT, idx,
                        255, /* iteration count */
                        0, /* initial value */
                        1, /* step size */
                        0 /* not used, must be 0 */))
      return false;

   emit->loop_const_idx = idx;
   emit->created_loop_const = true;

   return true;
}

static bool
create_arl_consts( struct svga_shader_emitter *emit )
{
   int i;

   for (i = 0; i < emit->num_arl_consts; i += 4) {
      int j;
      unsigned idx = emit->nr_hw_float_const++;
      float vals[4];
      for (j = 0; j < 4 && (j + i) < emit->num_arl_consts; ++j) {
         vals[j] = (float) emit->arl_consts[i + j].number;
         emit->arl_consts[i + j].idx = idx;
         switch (j) {
         case 0:
            emit->arl_consts[i + 0].swizzle = TGSI_SWIZZLE_X;
            break;
         case 1:
            emit->arl_consts[i + 0].swizzle = TGSI_SWIZZLE_Y;
            break;
         case 2:
            emit->arl_consts[i + 0].swizzle = TGSI_SWIZZLE_Z;
            break;
         case 3:
            emit->arl_consts[i + 0].swizzle = TGSI_SWIZZLE_W;
            break;
         }
      }
      while (j < 4)
         vals[j++] = 0;

      if (!emit_def_const( emit, SVGA3D_CONST_TYPE_FLOAT, idx,
                           vals[0], vals[1],
                           vals[2], vals[3]))
         return false;
   }

   return true;
}


/**
 * Return the register which holds the pixel shaders front/back-
 * facing value.
 */
static struct src_register
get_vface( struct svga_shader_emitter *emit )
{
   assert(emit->emitted_vface);
   return src_register(SVGA3DREG_MISCTYPE, SVGA3DMISCREG_FACE);
}


/**
 * Create/emit a "common" constant with values {0, 0.5, -1, 1}.
 * We can swizzle this to produce other useful constants such as
 * {0, 0, 0, 0}, {1, 1, 1, 1}, etc.
 */
static bool
create_common_immediate( struct svga_shader_emitter *emit )
{
   unsigned idx = emit->nr_hw_float_const++;

   /* Emit the constant (0, 0.5, -1, 1) and use swizzling to generate
    * other useful vectors.
    */
   if (!emit_def_const( emit, SVGA3D_CONST_TYPE_FLOAT,
                        idx, 0.0f, 0.5f, -1.0f, 1.0f ))
      return false;
   emit->common_immediate_idx[0] = idx;
   idx++;

   /* Emit constant {2, 0, 0, 0} (only the 2 is used for now) */
   if (emit->key.vs.adjust_attrib_range) {
      if (!emit_def_const( emit, SVGA3D_CONST_TYPE_FLOAT,
                           idx, 2.0f, 0.0f, 0.0f, 0.0f ))
         return false;
      emit->common_immediate_idx[1] = idx;
   }
   else {
      emit->common_immediate_idx[1] = -1;
   }

   emit->created_common_immediate = true;

   return true;
}


/**
 * Return swizzle/position for the given value in the "common" immediate.
 */
static inline unsigned
common_immediate_swizzle(float value)
{
   if (value == 0.0f)
      return TGSI_SWIZZLE_X;
   else if (value == 0.5f)
      return TGSI_SWIZZLE_Y;
   else if (value == -1.0f)
      return TGSI_SWIZZLE_Z;
   else if (value == 1.0f)
      return TGSI_SWIZZLE_W;
   else {
      assert(!"illegal value in common_immediate_swizzle");
      return TGSI_SWIZZLE_X;
   }
}


/**
 * Returns an immediate reg where all the terms are either 0, 1, 2 or 0.5
 */
static struct src_register
get_immediate(struct svga_shader_emitter *emit,
              float x, float y, float z, float w)
{
   unsigned sx = common_immediate_swizzle(x);
   unsigned sy = common_immediate_swizzle(y);
   unsigned sz = common_immediate_swizzle(z);
   unsigned sw = common_immediate_swizzle(w);
   assert(emit->created_common_immediate);
   assert(emit->common_immediate_idx[0] >= 0);
   return swizzle(src_register(SVGA3DREG_CONST, emit->common_immediate_idx[0]),
                  sx, sy, sz, sw);
}


/**
 * returns {0, 0, 0, 0} immediate
 */
static struct src_register
get_zero_immediate( struct svga_shader_emitter *emit )
{
   assert(emit->created_common_immediate);
   assert(emit->common_immediate_idx[0] >= 0);
   return swizzle(src_register( SVGA3DREG_CONST,
                                emit->common_immediate_idx[0]),
                  0, 0, 0, 0);
}


/**
 * returns {1, 1, 1, 1} immediate
 */
static struct src_register
get_one_immediate( struct svga_shader_emitter *emit )
{
   assert(emit->created_common_immediate);
   assert(emit->common_immediate_idx[0] >= 0);
   return swizzle(src_register( SVGA3DREG_CONST,
                                emit->common_immediate_idx[0]),
                  3, 3, 3, 3);
}


/**
 * returns {0.5, 0.5, 0.5, 0.5} immediate
 */
static struct src_register
get_half_immediate( struct svga_shader_emitter *emit )
{
   assert(emit->created_common_immediate);
   assert(emit->common_immediate_idx[0] >= 0);
   return swizzle(src_register(SVGA3DREG_CONST, emit->common_immediate_idx[0]),
                  1, 1, 1, 1);
}


/**
 * returns {2, 2, 2, 2} immediate
 */
static struct src_register
get_two_immediate( struct svga_shader_emitter *emit )
{
   /* Note we use the second common immediate here */
   assert(emit->created_common_immediate);
   assert(emit->common_immediate_idx[1] >= 0);
   return swizzle(src_register( SVGA3DREG_CONST,
                                emit->common_immediate_idx[1]),
                  0, 0, 0, 0);
}


/**
 * returns the loop const
 */
static struct src_register
get_loop_const( struct svga_shader_emitter *emit )
{
   assert(emit->created_loop_const);
   assert(emit->loop_const_idx >= 0);
   return src_register( SVGA3DREG_CONSTINT,
                        emit->loop_const_idx );
}


static struct src_register
get_fake_arl_const( struct svga_shader_emitter *emit )
{
   struct src_register reg;
   int idx = 0, swizzle = 0, i;

   for (i = 0; i < emit->num_arl_consts; ++ i) {
      if (emit->arl_consts[i].arl_num == emit->current_arl) {
         idx = emit->arl_consts[i].idx;
         swizzle = emit->arl_consts[i].swizzle;
      }
   }

   reg = src_register( SVGA3DREG_CONST, idx );
   return scalar(reg, swizzle);
}


/**
 * Return a register which holds the width and height of the texture
 * currently bound to the given sampler.
 */
static struct src_register
get_tex_dimensions( struct svga_shader_emitter *emit, int sampler_num )
{
   int idx;
   struct src_register reg;

   /* the width/height indexes start right after constants */
   idx = emit->key.tex[sampler_num].width_height_idx +
         emit->info.file_max[TGSI_FILE_CONSTANT] + 1;

   reg = src_register( SVGA3DREG_CONST, idx );
   return reg;
}


static bool
emit_fake_arl(struct svga_shader_emitter *emit,
              const struct tgsi_full_instruction *insn)
{
   const struct src_register src0 =
      translate_src_register(emit, &insn->Src[0] );
   struct src_register src1 = get_fake_arl_const( emit );
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   SVGA3dShaderDestToken tmp = get_temp( emit );

   if (!submit_op1(emit, inst_token( SVGA3DOP_MOV ), tmp, src0))
      return false;

   if (!submit_op2( emit, inst_token( SVGA3DOP_ADD ), tmp, src( tmp ),
                    src1))
      return false;

   /* replicate the original swizzle */
   src1 = src(tmp);
   src1.base.swizzle = src0.base.swizzle;

   return submit_op1( emit, inst_token( SVGA3DOP_MOVA ),
                      dst, src1 );
}


static bool
emit_if(struct svga_shader_emitter *emit,
        const struct tgsi_full_instruction *insn)
{
   struct src_register src0 =
      translate_src_register(emit, &insn->Src[0]);
   struct src_register zero = get_zero_immediate(emit);
   SVGA3dShaderInstToken if_token = inst_token( SVGA3DOP_IFC );

   if_token.control = SVGA3DOPCOMPC_NE;

   if (SVGA3dShaderGetRegType(src0.base.value) == SVGA3DREG_CONST) {
      /*
       * Max different constant registers readable per IFC instruction is 1.
       */
      SVGA3dShaderDestToken tmp = get_temp( emit );

      if (!submit_op1(emit, inst_token( SVGA3DOP_MOV ), tmp, src0))
         return false;

      src0 = scalar(src( tmp ), TGSI_SWIZZLE_X);
   }

   emit->dynamic_branching_level++;

   return (emit_instruction( emit, if_token ) &&
           emit_src( emit, src0 ) &&
           emit_src( emit, zero ) );
}


static bool
emit_else(struct svga_shader_emitter *emit,
          const struct tgsi_full_instruction *insn)
{
   return emit_instruction(emit, inst_token(SVGA3DOP_ELSE));
}


static bool
emit_endif(struct svga_shader_emitter *emit,
           const struct tgsi_full_instruction *insn)
{
   emit->dynamic_branching_level--;

   return emit_instruction(emit, inst_token(SVGA3DOP_ENDIF));
}


/**
 * Translate the following TGSI FLR instruction.
 *    FLR  DST, SRC
 * To the following SVGA3D instruction sequence.
 *    FRC  TMP, SRC
 *    SUB  DST, SRC, TMP
 */
static bool
emit_floor(struct svga_shader_emitter *emit,
           const struct tgsi_full_instruction *insn )
{
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   const struct src_register src0 =
      translate_src_register(emit, &insn->Src[0] );
   SVGA3dShaderDestToken temp = get_temp( emit );

   /* FRC  TMP, SRC */
   if (!submit_op1( emit, inst_token( SVGA3DOP_FRC ), temp, src0 ))
      return false;

   /* SUB  DST, SRC, TMP */
   if (!submit_op2( emit, inst_token( SVGA3DOP_ADD ), dst, src0,
                    negate( src( temp ) ) ))
      return false;

   return true;
}


/**
 * Translate the following TGSI CEIL instruction.
 *    CEIL  DST, SRC
 * To the following SVGA3D instruction sequence.
 *    FRC  TMP, -SRC
 *    ADD  DST, SRC, TMP
 */
static bool
emit_ceil(struct svga_shader_emitter *emit,
          const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderDestToken dst = translate_dst_register(emit, insn, 0);
   const struct src_register src0 =
      translate_src_register(emit, &insn->Src[0]);
   SVGA3dShaderDestToken temp = get_temp(emit);

   /* FRC  TMP, -SRC */
   if (!submit_op1(emit, inst_token(SVGA3DOP_FRC), temp, negate(src0)))
      return false;

   /* ADD DST, SRC, TMP */
   if (!submit_op2(emit, inst_token(SVGA3DOP_ADD), dst, src0, src(temp)))
      return false;

   return true;
}


/**
 * Translate the following TGSI DIV instruction.
 *    DIV  DST.xy, SRC0, SRC1
 * To the following SVGA3D instruction sequence.
 *    RCP  TMP.x, SRC1.xxxx
 *    RCP  TMP.y, SRC1.yyyy
 *    MUL  DST.xy, SRC0, TMP
 */
static bool
emit_div(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn )
{
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   const struct src_register src0 =
      translate_src_register(emit, &insn->Src[0] );
   const struct src_register src1 =
      translate_src_register(emit, &insn->Src[1] );
   SVGA3dShaderDestToken temp = get_temp( emit );
   unsigned i;

   /* For each enabled element, perform a RCP instruction.  Note that
    * RCP is scalar in SVGA3D:
    */
   for (i = 0; i < 4; i++) {
      unsigned channel = 1 << i;
      if (dst.mask & channel) {
         /* RCP  TMP.?, SRC1.???? */
         if (!submit_op1( emit, inst_token( SVGA3DOP_RCP ),
                          writemask(temp, channel),
                          scalar(src1, i) ))
            return false;
      }
   }

   /* Vector mul:
    * MUL  DST, SRC0, TMP
    */
   if (!submit_op2( emit, inst_token( SVGA3DOP_MUL ), dst, src0,
                    src( temp ) ))
      return false;

   return true;
}


/**
 * Translate the following TGSI DP2 instruction.
 *    DP2  DST, SRC1, SRC2
 * To the following SVGA3D instruction sequence.
 *    MUL  TMP, SRC1, SRC2
 *    ADD  DST, TMP.xxxx, TMP.yyyy
 */
static bool
emit_dp2(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn )
{
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   const struct src_register src0 =
      translate_src_register(emit, &insn->Src[0]);
   const struct src_register src1 =
      translate_src_register(emit, &insn->Src[1]);
   SVGA3dShaderDestToken temp = get_temp( emit );
   struct src_register temp_src0, temp_src1;

   /* MUL  TMP, SRC1, SRC2 */
   if (!submit_op2( emit, inst_token( SVGA3DOP_MUL ), temp, src0, src1 ))
      return false;

   temp_src0 = scalar(src( temp ), TGSI_SWIZZLE_X);
   temp_src1 = scalar(src( temp ), TGSI_SWIZZLE_Y);

   /* ADD  DST, TMP.xxxx, TMP.yyyy */
   if (!submit_op2( emit, inst_token( SVGA3DOP_ADD ), dst,
                    temp_src0, temp_src1 ))
      return false;

   return true;
}


/**
 * Sine / Cosine helper function.
 */
static bool
do_emit_sincos(struct svga_shader_emitter *emit,
               SVGA3dShaderDestToken dst,
               struct src_register src0)
{
   src0 = scalar(src0, TGSI_SWIZZLE_X);
   return submit_op1(emit, inst_token(SVGA3DOP_SINCOS), dst, src0);
}


/**
 * Translate TGSI SIN instruction into:
 * SCS TMP SRC
 * MOV DST TMP.yyyy
 */
static bool
emit_sin(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn )
{
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   struct src_register src0 =
      translate_src_register(emit, &insn->Src[0] );
   SVGA3dShaderDestToken temp = get_temp( emit );

   /* SCS TMP SRC */
   if (!do_emit_sincos(emit, writemask(temp, TGSI_WRITEMASK_Y), src0))
      return false;

   src0 = scalar(src( temp ), TGSI_SWIZZLE_Y);

   /* MOV DST TMP.yyyy */
   if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ), dst, src0 ))
      return false;

   return true;
}


/*
 * Translate TGSI COS instruction into:
 * SCS TMP SRC
 * MOV DST TMP.xxxx
 */
static bool
emit_cos(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   struct src_register src0 =
      translate_src_register(emit, &insn->Src[0] );
   SVGA3dShaderDestToken temp = get_temp( emit );

   /* SCS TMP SRC */
   if (!do_emit_sincos( emit, writemask(temp, TGSI_WRITEMASK_X), src0 ))
      return false;

   src0 = scalar(src( temp ), TGSI_SWIZZLE_X);

   /* MOV DST TMP.xxxx */
   if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ), dst, src0 ))
      return false;

   return true;
}


/**
 * Translate/emit TGSI SSG (Set Sign: -1, 0, +1) instruction.
 */
static bool
emit_ssg(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   struct src_register src0 =
      translate_src_register(emit, &insn->Src[0] );
   SVGA3dShaderDestToken temp0 = get_temp( emit );
   SVGA3dShaderDestToken temp1 = get_temp( emit );
   struct src_register zero, one;

   if (emit->unit == PIPE_SHADER_VERTEX) {
      /* SGN  DST, SRC0, TMP0, TMP1 */
      return submit_op3( emit, inst_token( SVGA3DOP_SGN ), dst, src0,
                         src( temp0 ), src( temp1 ) );
   }

   one = get_one_immediate(emit);
   zero = get_zero_immediate(emit);

   /* CMP  TMP0, SRC0, one, zero */
   if (!submit_op3( emit, inst_token( SVGA3DOP_CMP ),
                    writemask( temp0, dst.mask ), src0, one, zero ))
      return false;

   /* CMP  TMP1, negate(SRC0), negate(one), zero */
   if (!submit_op3( emit, inst_token( SVGA3DOP_CMP ),
                    writemask( temp1, dst.mask ), negate( src0 ), negate( one ),
                    zero ))
      return false;

   /* ADD  DST, TMP0, TMP1 */
   return submit_op2( emit, inst_token( SVGA3DOP_ADD ), dst, src( temp0 ),
                      src( temp1 ) );
}


/**
 * Translate/emit the conditional discard instruction (discard if
 * any of X,Y,Z,W are negative).
 */
static bool
emit_cond_discard(struct svga_shader_emitter *emit,
                  const struct tgsi_full_instruction *insn)
{
   const struct tgsi_full_src_register *reg = &insn->Src[0];
   struct src_register src0, srcIn;
   const bool special = (reg->Register.Absolute ||
                         reg->Register.Negate ||
                         reg->Register.Indirect ||
                         reg->Register.SwizzleX != 0 ||
                         reg->Register.SwizzleY != 1 ||
                         reg->Register.SwizzleZ != 2 ||
                         reg->Register.File != TGSI_FILE_TEMPORARY);
   SVGA3dShaderDestToken temp;

   src0 = srcIn = translate_src_register( emit, reg );

   if (special) {
      /* need a temp reg */
      temp = get_temp( emit );
   }

   if (special) {
      /* move the source into a temp register */
      submit_op1(emit, inst_token(SVGA3DOP_MOV), temp, src0);

      src0 = src( temp );
   }

   /* Do the discard by checking if any of the XYZW components are < 0.
    * Note that ps_2_0 and later take XYZW in consideration, while ps_1_x
    * only used XYZ.  The MSDN documentation about this is incorrect.
    */
   if (!submit_op0( emit, inst_token( SVGA3DOP_TEXKILL ), dst(src0) ))
      return false;

   return true;
}


/**
 * Translate/emit the unconditional discard instruction (usually found inside
 * an IF/ELSE/ENDIF block).
 */
static bool
emit_discard(struct svga_shader_emitter *emit,
             const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderDestToken temp;
   struct src_register one = get_one_immediate(emit);
   SVGA3dShaderInstToken inst = inst_token( SVGA3DOP_TEXKILL );

   /* texkill doesn't allow negation on the operand so lets move
    * negation of {1} to a temp register */
   temp = get_temp( emit );
   if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ), temp,
                    negate( one ) ))
      return false;

   return submit_op0( emit, inst, temp );
}


/**
 * Test if r1 and r2 are the same register.
 */
static bool
same_register(struct src_register r1, struct src_register r2)
{
   return (r1.base.num == r2.base.num &&
           r1.base.type_upper == r2.base.type_upper &&
           r1.base.type_lower == r2.base.type_lower);
}



/**
 * Implement conditionals by initializing destination reg to 'fail',
 * then set predicate reg with UFOP_SETP, then move 'pass' to dest
 * based on predicate reg.
 *
 * SETP src0, cmp, src1  -- do this first to avoid aliasing problems.
 * MOV dst, fail
 * MOV dst, pass, p0
 */
static bool
emit_conditional(struct svga_shader_emitter *emit,
                 enum pipe_compare_func compare_func,
                 SVGA3dShaderDestToken dst,
                 struct src_register src0,
                 struct src_register src1,
                 struct src_register pass,
                 struct src_register fail)
{
   SVGA3dShaderDestToken pred_reg = dst_register( SVGA3DREG_PREDICATE, 0 );
   SVGA3dShaderInstToken setp_token;

   switch (compare_func) {
   case PIPE_FUNC_NEVER:
      return submit_op1( emit, inst_token( SVGA3DOP_MOV ),
                         dst, fail );
      break;
   case PIPE_FUNC_LESS:
      setp_token = inst_token_setp(SVGA3DOPCOMP_LT);
      break;
   case PIPE_FUNC_EQUAL:
      setp_token = inst_token_setp(SVGA3DOPCOMP_EQ);
      break;
   case PIPE_FUNC_LEQUAL:
      setp_token = inst_token_setp(SVGA3DOPCOMP_LE);
      break;
   case PIPE_FUNC_GREATER:
      setp_token = inst_token_setp(SVGA3DOPCOMP_GT);
      break;
   case PIPE_FUNC_NOTEQUAL:
      setp_token = inst_token_setp(SVGA3DOPCOMPC_NE);
      break;
   case PIPE_FUNC_GEQUAL:
      setp_token = inst_token_setp(SVGA3DOPCOMP_GE);
      break;
   case PIPE_FUNC_ALWAYS:
      return submit_op1( emit, inst_token( SVGA3DOP_MOV ),
                         dst, pass );
      break;
   }

   if (same_register(src(dst), pass)) {
      /* We'll get bad results if the dst and pass registers are the same
       * so use a temp register containing pass.
       */
      SVGA3dShaderDestToken temp = get_temp(emit);
      if (!submit_op1(emit, inst_token(SVGA3DOP_MOV), temp, pass))
         return false;
      pass = src(temp);
   }

   /* SETP src0, COMPOP, src1 */
   if (!submit_op2( emit, setp_token, pred_reg,
                    src0, src1 ))
      return false;

   /* MOV dst, fail */
   if (!submit_op1(emit, inst_token(SVGA3DOP_MOV), dst, fail))
      return false;

   /* MOV dst, pass (predicated)
    *
    * Note that the predicate reg (and possible modifiers) is passed
    * as the first source argument.
    */
   if (!submit_op2(emit,
                   inst_token_predicated(SVGA3DOP_MOV), dst,
                   src(pred_reg), pass))
      return false;

   return true;
}


/**
 * Helper for emiting 'selection' commands.  Basically:
 * if (src0 OP src1)
 *    dst = 1.0;
 * else
 *    dst = 0.0;
 */
static bool
emit_select(struct svga_shader_emitter *emit,
            enum pipe_compare_func compare_func,
            SVGA3dShaderDestToken dst,
            struct src_register src0,
            struct src_register src1 )
{
   /* There are some SVGA instructions which implement some selects
    * directly, but they are only available in the vertex shader.
    */
   if (emit->unit == PIPE_SHADER_VERTEX) {
      switch (compare_func) {
      case PIPE_FUNC_GEQUAL:
         return submit_op2( emit, inst_token( SVGA3DOP_SGE ), dst, src0, src1 );
      case PIPE_FUNC_LEQUAL:
         return submit_op2( emit, inst_token( SVGA3DOP_SGE ), dst, src1, src0 );
      case PIPE_FUNC_GREATER:
         return submit_op2( emit, inst_token( SVGA3DOP_SLT ), dst, src1, src0 );
      case PIPE_FUNC_LESS:
         return submit_op2( emit, inst_token( SVGA3DOP_SLT ), dst, src0, src1 );
      default:
         break;
      }
   }

   /* Otherwise, need to use the setp approach:
    */
   {
      struct src_register one, zero;
      /* zero immediate is 0,0,0,1 */
      zero = get_zero_immediate(emit);
      one = get_one_immediate(emit);

      return emit_conditional(emit, compare_func, dst, src0, src1, one, zero);
   }
}


/**
 * Translate/emit a TGSI SEQ, SNE, SLT, SGE, etc. instruction.
 */
static bool
emit_select_op(struct svga_shader_emitter *emit,
               unsigned compare,
               const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   struct src_register src0 = translate_src_register(
      emit, &insn->Src[0] );
   struct src_register src1 = translate_src_register(
      emit, &insn->Src[1] );

   return emit_select( emit, compare, dst, src0, src1 );
}


/**
 * Translate TGSI CMP instruction.  Component-wise:
 * dst = (src0 < 0.0) ? src1 : src2
 */
static bool
emit_cmp(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   const struct src_register src0 =
      translate_src_register(emit, &insn->Src[0] );
   const struct src_register src1 =
      translate_src_register(emit, &insn->Src[1] );
   const struct src_register src2 =
      translate_src_register(emit, &insn->Src[2] );

   if (emit->unit == PIPE_SHADER_VERTEX) {
      struct src_register zero = get_zero_immediate(emit);
      /* We used to simulate CMP with SLT+LRP.  But that didn't work when
       * src1 or src2 was Inf/NaN.  In particular, GLSL sqrt(0) failed
       * because it involves a CMP to handle the 0 case.
       * Use a conditional expression instead.
       */
      return emit_conditional(emit, PIPE_FUNC_LESS, dst,
                              src0, zero, src1, src2);
   }
   else {
      assert(emit->unit == PIPE_SHADER_FRAGMENT);

      /* CMP  DST, SRC0, SRC2, SRC1 */
      return submit_op3( emit, inst_token( SVGA3DOP_CMP ), dst,
                         src0, src2, src1);
   }
}


/**
 * Translate/emit 2-operand (coord, sampler) texture instructions.
 */
static bool
emit_tex2(struct svga_shader_emitter *emit,
          const struct tgsi_full_instruction *insn,
          SVGA3dShaderDestToken dst)
{
   SVGA3dShaderInstToken inst;
   struct src_register texcoord;
   struct src_register sampler;
   SVGA3dShaderDestToken tmp;

   inst.value = 0;

   switch (insn->Instruction.Opcode) {
   case TGSI_OPCODE_TEX:
      inst.op = SVGA3DOP_TEX;
      break;
   case TGSI_OPCODE_TXP:
      inst.op = SVGA3DOP_TEX;
      inst.control = SVGA3DOPCONT_PROJECT;
      break;
   case TGSI_OPCODE_TXB:
      inst.op = SVGA3DOP_TEX;
      inst.control = SVGA3DOPCONT_BIAS;
      break;
   case TGSI_OPCODE_TXL:
      inst.op = SVGA3DOP_TEXLDL;
      break;
   default:
      assert(0);
      return false;
   }

   texcoord = translate_src_register( emit, &insn->Src[0] );
   sampler = translate_src_register( emit, &insn->Src[1] );

   if (emit->key.tex[sampler.base.num].unnormalized ||
       emit->dynamic_branching_level > 0)
      tmp = get_temp( emit );

   /* Can't do mipmapping inside dynamic branch constructs.  Force LOD
    * zero in that case.
    */
   if (emit->dynamic_branching_level > 0 &&
       inst.op == SVGA3DOP_TEX &&
       SVGA3dShaderGetRegType(texcoord.base.value) == SVGA3DREG_TEMP) {
      struct src_register zero = get_zero_immediate(emit);

      /* MOV  tmp, texcoord */
      if (!submit_op1( emit,
                       inst_token( SVGA3DOP_MOV ),
                       tmp,
                       texcoord ))
         return false;

      /* MOV  tmp.w, zero */
      if (!submit_op1( emit,
                       inst_token( SVGA3DOP_MOV ),
                       writemask( tmp, TGSI_WRITEMASK_W ),
                       zero ))
         return false;

      texcoord = src( tmp );
      inst.op = SVGA3DOP_TEXLDL;
   }

   /* Explicit normalization of texcoords:
    */
   if (emit->key.tex[sampler.base.num].unnormalized) {
      struct src_register wh = get_tex_dimensions( emit, sampler.base.num );

      /* MUL  tmp, SRC0, WH */
      if (!submit_op2( emit, inst_token( SVGA3DOP_MUL ),
                       tmp, texcoord, wh ))
         return false;

      texcoord = src( tmp );
   }

   return submit_op2( emit, inst, dst, texcoord, sampler );
}


/**
 * Translate/emit 4-operand (coord, ddx, ddy, sampler) texture instructions.
 */
static bool
emit_tex4(struct svga_shader_emitter *emit,
          const struct tgsi_full_instruction *insn,
          SVGA3dShaderDestToken dst )
{
   SVGA3dShaderInstToken inst;
   struct src_register texcoord;
   struct src_register ddx;
   struct src_register ddy;
   struct src_register sampler;

   texcoord = translate_src_register( emit, &insn->Src[0] );
   ddx      = translate_src_register( emit, &insn->Src[1] );
   ddy      = translate_src_register( emit, &insn->Src[2] );
   sampler  = translate_src_register( emit, &insn->Src[3] );

   inst.value = 0;

   switch (insn->Instruction.Opcode) {
   case TGSI_OPCODE_TXD:
      inst.op = SVGA3DOP_TEXLDD; /* 4 args! */
      break;
   default:
      assert(0);
      return false;
   }

   return submit_op4( emit, inst, dst, texcoord, sampler, ddx, ddy );
}


/**
 * Emit texture swizzle code.  We do this here since SVGA samplers don't
 * directly support swizzles.
 */
static bool
emit_tex_swizzle(struct svga_shader_emitter *emit,
                 SVGA3dShaderDestToken dst,
                 struct src_register src,
                 unsigned swizzle_x,
                 unsigned swizzle_y,
                 unsigned swizzle_z,
                 unsigned swizzle_w)
{
   const unsigned swizzleIn[4] = {swizzle_x, swizzle_y, swizzle_z, swizzle_w};
   unsigned srcSwizzle[4];
   unsigned srcWritemask = 0x0, zeroWritemask = 0x0, oneWritemask = 0x0;
   unsigned i;

   /* build writemasks and srcSwizzle terms */
   for (i = 0; i < 4; i++) {
      if (swizzleIn[i] == PIPE_SWIZZLE_0) {
         srcSwizzle[i] = TGSI_SWIZZLE_X + i;
         zeroWritemask |= (1 << i);
      }
      else if (swizzleIn[i] == PIPE_SWIZZLE_1) {
         srcSwizzle[i] = TGSI_SWIZZLE_X + i;
         oneWritemask |= (1 << i);
      }
      else {
         srcSwizzle[i] = swizzleIn[i];
         srcWritemask |= (1 << i);
      }
   }

   /* write x/y/z/w comps */
   if (dst.mask & srcWritemask) {
      if (!submit_op1(emit,
                      inst_token(SVGA3DOP_MOV),
                      writemask(dst, srcWritemask),
                      swizzle(src,
                              srcSwizzle[0],
                              srcSwizzle[1],
                              srcSwizzle[2],
                              srcSwizzle[3])))
         return false;
   }

   /* write 0 comps */
   if (dst.mask & zeroWritemask) {
      if (!submit_op1(emit,
                      inst_token(SVGA3DOP_MOV),
                      writemask(dst, zeroWritemask),
                      get_zero_immediate(emit)))
         return false;
   }

   /* write 1 comps */
   if (dst.mask & oneWritemask) {
      if (!submit_op1(emit,
                      inst_token(SVGA3DOP_MOV),
                      writemask(dst, oneWritemask),
                      get_one_immediate(emit)))
         return false;
   }

   return true;
}


/**
 * Translate/emit a TGSI texture sample instruction.
 */
static bool
emit_tex(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderDestToken dst =
      translate_dst_register( emit, insn, 0 );
   struct src_register src0 =
      translate_src_register( emit, &insn->Src[0] );
   struct src_register src1 =
      translate_src_register( emit, &insn->Src[1] );

   SVGA3dShaderDestToken tex_result;
   const unsigned unit = src1.base.num;

   /* check for shadow samplers */
   bool compare = (emit->key.tex[unit].compare_mode ==
                   PIPE_TEX_COMPARE_R_TO_TEXTURE);

   /* texture swizzle */
   bool swizzle = (emit->key.tex[unit].swizzle_r != PIPE_SWIZZLE_X ||
                   emit->key.tex[unit].swizzle_g != PIPE_SWIZZLE_Y ||
                   emit->key.tex[unit].swizzle_b != PIPE_SWIZZLE_Z ||
                   emit->key.tex[unit].swizzle_a != PIPE_SWIZZLE_W);

   bool saturate = insn->Instruction.Saturate;

   /* If doing compare processing or tex swizzle or saturation, we need to put
    * the fetched color into a temporary so it can be used as a source later on.
    */
   if (compare || swizzle || saturate) {
      tex_result = get_temp( emit );
   }
   else {
      tex_result = dst;
   }

   switch(insn->Instruction.Opcode) {
   case TGSI_OPCODE_TEX:
   case TGSI_OPCODE_TXB:
   case TGSI_OPCODE_TXP:
   case TGSI_OPCODE_TXL:
      if (!emit_tex2( emit, insn, tex_result ))
         return false;
      break;
   case TGSI_OPCODE_TXD:
      if (!emit_tex4( emit, insn, tex_result ))
         return false;
      break;
   default:
      assert(0);
   }

   if (compare) {
      SVGA3dShaderDestToken dst2;

      if (swizzle || saturate)
         dst2 = tex_result;
      else
         dst2 = dst;

      if (dst.mask & TGSI_WRITEMASK_XYZ) {
         SVGA3dShaderDestToken src0_zdivw = get_temp( emit );
         /* When sampling a depth texture, the result of the comparison is in
          * the Y component.
          */
         struct src_register tex_src_x = scalar(src(tex_result), TGSI_SWIZZLE_Y);
         struct src_register r_coord;

         if (insn->Instruction.Opcode == TGSI_OPCODE_TXP) {
            /* Divide texcoord R by Q */
            if (!submit_op1( emit, inst_token( SVGA3DOP_RCP ),
                             writemask(src0_zdivw, TGSI_WRITEMASK_X),
                             scalar(src0, TGSI_SWIZZLE_W) ))
               return false;

            if (!submit_op2( emit, inst_token( SVGA3DOP_MUL ),
                             writemask(src0_zdivw, TGSI_WRITEMASK_X),
                             scalar(src0, TGSI_SWIZZLE_Z),
                             scalar(src(src0_zdivw), TGSI_SWIZZLE_X) ))
               return false;

            r_coord = scalar(src(src0_zdivw), TGSI_SWIZZLE_X);
         }
         else {
            r_coord = scalar(src0, TGSI_SWIZZLE_Z);
         }

         /* Compare texture sample value against R component of texcoord */
         if (!emit_select(emit,
                          emit->key.tex[unit].compare_func,
                          writemask( dst2, TGSI_WRITEMASK_XYZ ),
                          r_coord,
                          tex_src_x))
            return false;
      }

      if (dst.mask & TGSI_WRITEMASK_W) {
         struct src_register one = get_one_immediate(emit);

        if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ),
                         writemask( dst2, TGSI_WRITEMASK_W ),
                         one ))
           return false;
      }
   }

   if (saturate && !swizzle) {
      /* MOV_SAT real_dst, dst */
      if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ), dst, src(tex_result) ))
         return false;
   }
   else if (swizzle) {
      /* swizzle from tex_result to dst (handles saturation too, if any) */
      emit_tex_swizzle(emit,
                       dst, src(tex_result),
                       emit->key.tex[unit].swizzle_r,
                       emit->key.tex[unit].swizzle_g,
                       emit->key.tex[unit].swizzle_b,
                       emit->key.tex[unit].swizzle_a);
   }

   return true;
}


static bool
emit_bgnloop(struct svga_shader_emitter *emit,
             const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderInstToken inst = inst_token( SVGA3DOP_LOOP );
   struct src_register loop_reg = src_register( SVGA3DREG_LOOP, 0 );
   struct src_register const_int = get_loop_const( emit );

   emit->dynamic_branching_level++;

   return (emit_instruction( emit, inst ) &&
           emit_src( emit, loop_reg ) &&
           emit_src( emit, const_int ) );
}


static bool
emit_endloop(struct svga_shader_emitter *emit,
             const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderInstToken inst = inst_token( SVGA3DOP_ENDLOOP );

   emit->dynamic_branching_level--;

   return emit_instruction( emit, inst );
}


/**
 * Translate/emit TGSI BREAK (out of loop) instruction.
 */
static bool
emit_brk(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderInstToken inst = inst_token( SVGA3DOP_BREAK );
   return emit_instruction( emit, inst );
}


/**
 * Emit simple instruction which operates on one scalar value (not
 * a vector).  Ex: LG2, RCP, RSQ.
 */
static bool
emit_scalar_op1(struct svga_shader_emitter *emit,
                SVGA3dShaderOpCodeType opcode,
                const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderInstToken inst;
   SVGA3dShaderDestToken dst;
   struct src_register src;

   inst = inst_token( opcode );
   dst = translate_dst_register( emit, insn, 0 );
   src = translate_src_register( emit, &insn->Src[0] );
   src = scalar( src, TGSI_SWIZZLE_X );

   return submit_op1( emit, inst, dst, src );
}


/**
 * Translate/emit a simple instruction (one which has no special-case
 * code) such as ADD, MUL, MIN, MAX.
 */
static bool
emit_simple_instruction(struct svga_shader_emitter *emit,
                        SVGA3dShaderOpCodeType opcode,
                        const struct tgsi_full_instruction *insn)
{
   const struct tgsi_full_src_register *src = insn->Src;
   SVGA3dShaderInstToken inst;
   SVGA3dShaderDestToken dst;

   inst = inst_token( opcode );
   dst = translate_dst_register( emit, insn, 0 );

   switch (insn->Instruction.NumSrcRegs) {
   case 0:
      return submit_op0( emit, inst, dst );
   case 1:
      return submit_op1( emit, inst, dst,
                         translate_src_register( emit, &src[0] ));
   case 2:
      return submit_op2( emit, inst, dst,
                         translate_src_register( emit, &src[0] ),
                         translate_src_register( emit, &src[1] ) );
   case 3:
      return submit_op3( emit, inst, dst,
                         translate_src_register( emit, &src[0] ),
                         translate_src_register( emit, &src[1] ),
                         translate_src_register( emit, &src[2] ) );
   default:
      assert(0);
      return false;
   }
}


/**
 * TGSI_OPCODE_MOVE is only special-cased here to detect the
 * svga_fragment_shader::constant_color_output case.
 */
static bool
emit_mov(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   const struct tgsi_full_src_register *src = &insn->Src[0];
   const struct tgsi_full_dst_register *dst = &insn->Dst[0];

   if (emit->unit == PIPE_SHADER_FRAGMENT &&
       dst->Register.File == TGSI_FILE_OUTPUT &&
       dst->Register.Index == 0 &&
       src->Register.File == TGSI_FILE_CONSTANT &&
       !src->Register.Indirect) {
      emit->constant_color_output = true;
   }

   return emit_simple_instruction(emit, SVGA3DOP_MOV, insn);
}


/**
 * Translate TGSI SQRT instruction
 * if src1 == 0
 *    mov dst, src1
 * else
 *    rsq temp, src1
 *    rcp dst, temp
 * endif
 */
static bool
emit_sqrt(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   const struct src_register src1 = translate_src_register(emit, &insn->Src[0]);
   const struct src_register zero = get_zero_immediate(emit);
   SVGA3dShaderDestToken dst = translate_dst_register(emit, insn, 0);
   SVGA3dShaderDestToken temp = get_temp(emit);
   SVGA3dShaderInstToken if_token = inst_token(SVGA3DOP_IFC);
   bool ret = true;

   if_token.control = SVGA3DOPCOMP_EQ;

   if (!(emit_instruction(emit, if_token) &&
         emit_src(emit, src1) &&
         emit_src(emit, zero))) {
      ret = false;
      goto cleanup;
   }

   if (!submit_op1(emit,
              inst_token(SVGA3DOP_MOV),
              dst, src1)) {
      ret = false;
      goto cleanup;
   }

   if (!emit_instruction(emit, inst_token(SVGA3DOP_ELSE))) {
      ret = false;
      goto cleanup;
   }

   if (!submit_op1(emit,
              inst_token(SVGA3DOP_RSQ),
              temp, src1)) {
      ret = false;
      goto cleanup;
   }

   if (!submit_op1(emit,
              inst_token(SVGA3DOP_RCP),
              dst, src(temp))) {
      ret = false;
      goto cleanup;
   }

   if (!emit_instruction(emit, inst_token(SVGA3DOP_ENDIF))) {
      ret = false;
      goto cleanup;
   }

cleanup:
   release_temp(emit, temp);

   return ret;
}


/**
 * Translate/emit TGSI DDX, DDY instructions.
 */
static bool
emit_deriv(struct svga_shader_emitter *emit,
           const struct tgsi_full_instruction *insn )
{
   if (emit->dynamic_branching_level > 0 &&
       insn->Src[0].Register.File == TGSI_FILE_TEMPORARY)
   {
      SVGA3dShaderDestToken dst =
         translate_dst_register( emit, insn, 0 );

      /* Deriv opcodes not valid inside dynamic branching, workaround
       * by zeroing out the destination.
       */
      if (!submit_op1(emit,
                      inst_token( SVGA3DOP_MOV ),
                      dst,
                      get_zero_immediate(emit)))
         return false;

      return true;
   }
   else {
      SVGA3dShaderOpCodeType opcode;
      const struct tgsi_full_src_register *reg = &insn->Src[0];
      SVGA3dShaderInstToken inst;
      SVGA3dShaderDestToken dst;
      struct src_register src0;

      switch (insn->Instruction.Opcode) {
      case TGSI_OPCODE_DDX:
         opcode = SVGA3DOP_DSX;
         break;
      case TGSI_OPCODE_DDY:
         opcode = SVGA3DOP_DSY;
         break;
      default:
         return false;
      }

      inst = inst_token( opcode );
      dst = translate_dst_register( emit, insn, 0 );
      src0 = translate_src_register( emit, reg );

      /* We cannot use negate or abs on source to dsx/dsy instruction.
       */
      if (reg->Register.Absolute ||
          reg->Register.Negate) {
         SVGA3dShaderDestToken temp = get_temp( emit );

         if (!emit_repl( emit, temp, &src0 ))
            return false;
      }

      return submit_op1( emit, inst, dst, src0 );
   }
}


/**
 * Translate/emit ARL (Address Register Load) instruction.  Used to
 * move a value into the special 'address' register.  Used to implement
 * indirect/variable indexing into arrays.
 */
static bool
emit_arl(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   ++emit->current_arl;
   if (emit->unit == PIPE_SHADER_FRAGMENT) {
      /* MOVA not present in pixel shader instruction set.
       * Ignore this instruction altogether since it is
       * only used for loop counters -- and for that
       * we reference aL directly.
       */
      return true;
   }
   if (svga_arl_needs_adjustment( emit )) {
      return emit_fake_arl( emit, insn );
   } else {
      /* no need to adjust, just emit straight arl */
      return emit_simple_instruction(emit, SVGA3DOP_MOVA, insn);
   }
}


static bool
emit_pow(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   struct src_register src0 = translate_src_register(
      emit, &insn->Src[0] );
   struct src_register src1 = translate_src_register(
      emit, &insn->Src[1] );
   bool need_tmp = false;

   /* POW can only output to a temporary */
   if (insn->Dst[0].Register.File != TGSI_FILE_TEMPORARY)
      need_tmp = true;

   /* POW src1 must not be the same register as dst */
   if (alias_src_dst( src1, dst ))
      need_tmp = true;

   /* it's a scalar op */
   src0 = scalar( src0, TGSI_SWIZZLE_X );
   src1 = scalar( src1, TGSI_SWIZZLE_X );

   if (need_tmp) {
      SVGA3dShaderDestToken tmp =
         writemask(get_temp( emit ), TGSI_WRITEMASK_X );

      if (!submit_op2(emit, inst_token( SVGA3DOP_POW ), tmp, src0, src1))
         return false;

      return submit_op1(emit, inst_token( SVGA3DOP_MOV ),
                        dst, scalar(src(tmp), 0) );
   }
   else {
      return submit_op2(emit, inst_token( SVGA3DOP_POW ), dst, src0, src1);
   }
}


/**
 * Emit a LRP (linear interpolation) instruction.
 */
static bool
submit_lrp(struct svga_shader_emitter *emit,
           SVGA3dShaderDestToken dst,
           struct src_register src0,
           struct src_register src1,
           struct src_register src2)
{
   SVGA3dShaderDestToken tmp;
   bool need_dst_tmp = false;

   /* The dst reg must be a temporary, and not be the same as src0 or src2 */
   if (SVGA3dShaderGetRegType(dst.value) != SVGA3DREG_TEMP ||
       alias_src_dst(src0, dst) ||
       alias_src_dst(src2, dst))
      need_dst_tmp = true;

   if (need_dst_tmp) {
      tmp = get_temp( emit );
      tmp.mask = dst.mask;
   }
   else {
      tmp = dst;
   }

   if (!submit_op3(emit, inst_token( SVGA3DOP_LRP ), tmp, src0, src1, src2))
      return false;

   if (need_dst_tmp) {
      if (!submit_op1(emit, inst_token( SVGA3DOP_MOV ), dst, src( tmp )))
         return false;
   }

   return true;
}


/**
 * Translate/emit LRP (Linear Interpolation) instruction.
 */
static bool
emit_lrp(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   const struct src_register src0 = translate_src_register(
      emit, &insn->Src[0] );
   const struct src_register src1 = translate_src_register(
      emit, &insn->Src[1] );
   const struct src_register src2 = translate_src_register(
      emit, &insn->Src[2] );

   return submit_lrp(emit, dst, src0, src1, src2);
}

/**
 * Translate/emit DST (Distance function) instruction.
 */
static bool
emit_dst_insn(struct svga_shader_emitter *emit,
              const struct tgsi_full_instruction *insn)
{
   if (emit->unit == PIPE_SHADER_VERTEX) {
      /* SVGA/DX9 has a DST instruction, but only for vertex shaders:
       */
      return emit_simple_instruction(emit, SVGA3DOP_DST, insn);
   }
   else {
      /* result[0] = 1    * 1;
       * result[1] = a[1] * b[1];
       * result[2] = a[2] * 1;
       * result[3] = 1    * b[3];
       */
      SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
      SVGA3dShaderDestToken tmp;
      const struct src_register src0 = translate_src_register(
         emit, &insn->Src[0] );
      const struct src_register src1 = translate_src_register(
         emit, &insn->Src[1] );
      bool need_tmp = false;

      if (SVGA3dShaderGetRegType(dst.value) != SVGA3DREG_TEMP ||
          alias_src_dst(src0, dst) ||
          alias_src_dst(src1, dst))
         need_tmp = true;

      if (need_tmp) {
         tmp = get_temp( emit );
      }
      else {
         tmp = dst;
      }

      /* tmp.xw = 1.0
       */
      if (tmp.mask & TGSI_WRITEMASK_XW) {
         if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ),
                          writemask(tmp, TGSI_WRITEMASK_XW ),
                          get_one_immediate(emit)))
            return false;
      }

      /* tmp.yz = src0
       */
      if (tmp.mask & TGSI_WRITEMASK_YZ) {
         if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ),
                          writemask(tmp, TGSI_WRITEMASK_YZ ),
                          src0))
            return false;
      }

      /* tmp.yw = tmp * src1
       */
      if (tmp.mask & TGSI_WRITEMASK_YW) {
         if (!submit_op2( emit, inst_token( SVGA3DOP_MUL ),
                          writemask(tmp, TGSI_WRITEMASK_YW ),
                          src(tmp),
                          src1))
            return false;
      }

      /* dst = tmp
       */
      if (need_tmp) {
         if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ),
                          dst,
                          src(tmp)))
            return false;
      }
   }

   return true;
}


static bool
emit_exp(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   struct src_register src0 =
      translate_src_register( emit, &insn->Src[0] );
   SVGA3dShaderDestToken fraction;

   if (dst.mask & TGSI_WRITEMASK_Y)
      fraction = dst;
   else if (dst.mask & TGSI_WRITEMASK_X)
      fraction = get_temp( emit );
   else
      fraction.value = 0;

   /* If y is being written, fill it with src0 - floor(src0).
    */
   if (dst.mask & TGSI_WRITEMASK_XY) {
      if (!submit_op1( emit, inst_token( SVGA3DOP_FRC ),
                       writemask( fraction, TGSI_WRITEMASK_Y ),
                       src0 ))
         return false;
   }

   /* If x is being written, fill it with 2 ^ floor(src0).
    */
   if (dst.mask & TGSI_WRITEMASK_X) {
      if (!submit_op2( emit, inst_token( SVGA3DOP_ADD ),
                       writemask( dst, TGSI_WRITEMASK_X ),
                       src0,
                       scalar( negate( src( fraction ) ), TGSI_SWIZZLE_Y ) ) )
         return false;

      if (!submit_op1( emit, inst_token( SVGA3DOP_EXP ),
                       writemask( dst, TGSI_WRITEMASK_X ),
                       scalar( src( dst ), TGSI_SWIZZLE_X ) ) )
         return false;

      if (!(dst.mask & TGSI_WRITEMASK_Y))
         release_temp( emit, fraction );
   }

   /* If z is being written, fill it with 2 ^ src0 (partial precision).
    */
   if (dst.mask & TGSI_WRITEMASK_Z) {
      if (!submit_op1( emit, inst_token( SVGA3DOP_EXPP ),
                       writemask( dst, TGSI_WRITEMASK_Z ),
                       src0 ) )
         return false;
   }

   /* If w is being written, fill it with one.
    */
   if (dst.mask & TGSI_WRITEMASK_W) {
      if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ),
                       writemask(dst, TGSI_WRITEMASK_W),
                       get_one_immediate(emit)))
         return false;
   }

   return true;
}


/**
 * Translate/emit LIT (Lighting helper) instruction.
 */
static bool
emit_lit(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   if (emit->unit == PIPE_SHADER_VERTEX) {
      /* SVGA/DX9 has a LIT instruction, but only for vertex shaders:
       */
      return emit_simple_instruction(emit, SVGA3DOP_LIT, insn);
   }
   else {
      /* D3D vs. GL semantics can be fairly easily accommodated by
       * variations on this sequence.
       *
       * GL:
       *   tmp.y = src.x
       *   tmp.z = pow(src.y,src.w)
       *   p0 = src0.xxxx > 0
       *   result = zero.wxxw
       *   (p0) result.yz = tmp
       *
       * D3D:
       *   tmp.y = src.x
       *   tmp.z = pow(src.y,src.w)
       *   p0 = src0.xxyy > 0
       *   result = zero.wxxw
       *   (p0) result.yz = tmp
       *
       * Will implement the GL version for now.
       */
      SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
      SVGA3dShaderDestToken tmp = get_temp( emit );
      const struct src_register src0 = translate_src_register(
         emit, &insn->Src[0] );

      /* tmp = pow(src.y, src.w)
       */
      if (dst.mask & TGSI_WRITEMASK_Z) {
         if (!submit_op2(emit, inst_token( SVGA3DOP_POW ),
                         tmp,
                         scalar(src0, 1),
                         scalar(src0, 3)))
            return false;
      }

      /* tmp.y = src.x
       */
      if (dst.mask & TGSI_WRITEMASK_Y) {
         if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ),
                          writemask(tmp, TGSI_WRITEMASK_Y ),
                          scalar(src0, 0)))
            return false;
      }

      /* Can't quite do this with emit conditional due to the extra
       * writemask on the predicated mov:
       */
      {
         SVGA3dShaderDestToken pred_reg = dst_register( SVGA3DREG_PREDICATE, 0 );
         struct src_register predsrc;

         /* D3D vs GL semantics:
          */
         if (0)
            predsrc = swizzle(src0, 0, 0, 1, 1); /* D3D */
         else
            predsrc = swizzle(src0, 0, 0, 0, 0); /* GL */

         /* SETP src0.xxyy, GT, {0}.x */
         if (!submit_op2( emit,
                          inst_token_setp(SVGA3DOPCOMP_GT),
                          pred_reg,
                          predsrc,
                          get_zero_immediate(emit)))
            return false;

         /* MOV dst, fail */
         if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ), dst,
                          get_immediate(emit, 1.0f, 0.0f, 0.0f, 1.0f)))
             return false;

         /* MOV dst.yz, tmp (predicated)
          *
          * Note that the predicate reg (and possible modifiers) is passed
          * as the first source argument.
          */
         if (dst.mask & TGSI_WRITEMASK_YZ) {
            if (!submit_op2( emit,
                             inst_token_predicated(SVGA3DOP_MOV),
                             writemask(dst, TGSI_WRITEMASK_YZ),
                             src( pred_reg ), src( tmp ) ))
               return false;
         }
      }
   }

   return true;
}


static bool
emit_ex2(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderInstToken inst;
   SVGA3dShaderDestToken dst;
   struct src_register src0;

   inst = inst_token( SVGA3DOP_EXP );
   dst = translate_dst_register( emit, insn, 0 );
   src0 = translate_src_register( emit, &insn->Src[0] );
   src0 = scalar( src0, TGSI_SWIZZLE_X );

   if (dst.mask != TGSI_WRITEMASK_XYZW) {
      SVGA3dShaderDestToken tmp = get_temp( emit );

      if (!submit_op1( emit, inst, tmp, src0 ))
         return false;

      return submit_op1( emit, inst_token( SVGA3DOP_MOV ),
                         dst,
                         scalar( src( tmp ), TGSI_SWIZZLE_X ) );
   }

   return submit_op1( emit, inst, dst, src0 );
}


static bool
emit_log(struct svga_shader_emitter *emit,
         const struct tgsi_full_instruction *insn)
{
   SVGA3dShaderDestToken dst = translate_dst_register( emit, insn, 0 );
   struct src_register src0 =
      translate_src_register( emit, &insn->Src[0] );
   SVGA3dShaderDestToken abs_tmp;
   struct src_register abs_src0;
   SVGA3dShaderDestToken log2_abs;

   abs_tmp.value = 0;

   if (dst.mask & TGSI_WRITEMASK_Z)
      log2_abs = dst;
   else if (dst.mask & TGSI_WRITEMASK_XY)
      log2_abs = get_temp( emit );
   else
      log2_abs.value = 0;

   /* If z is being written, fill it with log2( abs( src0 ) ).
    */
   if (dst.mask & TGSI_WRITEMASK_XYZ) {
      if (!src0.base.srcMod || src0.base.srcMod == SVGA3DSRCMOD_ABS)
         abs_src0 = src0;
      else {
         abs_tmp = get_temp( emit );

         if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ),
                          abs_tmp,
                          src0 ) )
            return false;

         abs_src0 = src( abs_tmp );
      }

      abs_src0 = absolute( scalar( abs_src0, TGSI_SWIZZLE_X ) );

      if (!submit_op1( emit, inst_token( SVGA3DOP_LOG ),
                       writemask( log2_abs, TGSI_WRITEMASK_Z ),
                       abs_src0 ) )
         return false;
   }

   if (dst.mask & TGSI_WRITEMASK_XY) {
      SVGA3dShaderDestToken floor_log2;

      if (dst.mask & TGSI_WRITEMASK_X)
         floor_log2 = dst;
      else
         floor_log2 = get_temp( emit );

      /* If x is being written, fill it with floor( log2( abs( src0 ) ) ).
       */
      if (!submit_op1( emit, inst_token( SVGA3DOP_FRC ),
                       writemask( floor_log2, TGSI_WRITEMASK_X ),
                       scalar( src( log2_abs ), TGSI_SWIZZLE_Z ) ) )
         return false;

      if (!submit_op2( emit, inst_token( SVGA3DOP_ADD ),
                       writemask( floor_log2, TGSI_WRITEMASK_X ),
                       scalar( src( log2_abs ), TGSI_SWIZZLE_Z ),
                       negate( src( floor_log2 ) ) ) )
         return false;

      /* If y is being written, fill it with
       * abs ( src0 ) / ( 2 ^ floor( log2( abs( src0 ) ) ) ).
       */
      if (dst.mask & TGSI_WRITEMASK_Y) {
         if (!submit_op1( emit, inst_token( SVGA3DOP_EXP ),
                          writemask( dst, TGSI_WRITEMASK_Y ),
                          negate( scalar( src( floor_log2 ),
                                          TGSI_SWIZZLE_X ) ) ) )
            return false;

         if (!submit_op2( emit, inst_token( SVGA3DOP_MUL ),
                          writemask( dst, TGSI_WRITEMASK_Y ),
                          src( dst ),
                          abs_src0 ) )
            return false;
      }

      if (!(dst.mask & TGSI_WRITEMASK_X))
         release_temp( emit, floor_log2 );

      if (!(dst.mask & TGSI_WRITEMASK_Z))
         release_temp( emit, log2_abs );
   }

   if (dst.mask & TGSI_WRITEMASK_XYZ && src0.base.srcMod &&
       src0.base.srcMod != SVGA3DSRCMOD_ABS)
      release_temp( emit, abs_tmp );

   /* If w is being written, fill it with one.
    */
   if (dst.mask & TGSI_WRITEMASK_W) {
      if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ),
                       writemask(dst, TGSI_WRITEMASK_W),
                       get_one_immediate(emit)))
         return false;
   }

   return true;
}


/**
 * Translate TGSI TRUNC or ROUND instruction.
 * We need to truncate toward zero. Ex: trunc(-1.9) = -1
 * Different approaches are needed for VS versus PS.
 */
static bool
emit_trunc_round(struct svga_shader_emitter *emit,
                 const struct tgsi_full_instruction *insn,
                 bool round)
{
   SVGA3dShaderDestToken dst = translate_dst_register(emit, insn, 0);
   const struct src_register src0 =
      translate_src_register(emit, &insn->Src[0] );
   SVGA3dShaderDestToken t1 = get_temp(emit);

   if (round) {
      SVGA3dShaderDestToken t0 = get_temp(emit);
      struct src_register half = get_half_immediate(emit);

      /* t0 = abs(src0) + 0.5 */
      if (!submit_op2(emit, inst_token(SVGA3DOP_ADD), t0,
                      absolute(src0), half))
         return false;

      /* t1 = fract(t0) */
      if (!submit_op1(emit, inst_token(SVGA3DOP_FRC), t1, src(t0)))
         return false;

      /* t1 = t0 - t1 */
      if (!submit_op2(emit, inst_token(SVGA3DOP_ADD), t1, src(t0),
                      negate(src(t1))))
         return false;
   }
   else {
      /* trunc */

      /* t1 = fract(abs(src0)) */
      if (!submit_op1(emit, inst_token(SVGA3DOP_FRC), t1, absolute(src0)))
         return false;

      /* t1 = abs(src0) - t1 */
      if (!submit_op2(emit, inst_token(SVGA3DOP_ADD), t1, absolute(src0),
                      negate(src(t1))))
         return false;
   }

   /*
    * Now we need to multiply t1 by the sign of the original value.
   */
   if (emit->unit == PIPE_SHADER_VERTEX) {
      /* For VS: use SGN instruction */
      /* Need two extra/dummy registers: */
      SVGA3dShaderDestToken t2 = get_temp(emit), t3 = get_temp(emit),
         t4 = get_temp(emit);

      /* t2 = sign(src0) */
      if (!submit_op3(emit, inst_token(SVGA3DOP_SGN), t2, src0,
                      src(t3), src(t4)))
         return false;

      /* dst = t1 * t2 */
      if (!submit_op2(emit, inst_token(SVGA3DOP_MUL), dst, src(t1), src(t2)))
         return false;
   }
   else {
      /* For FS: Use CMP instruction */
      return submit_op3(emit, inst_token( SVGA3DOP_CMP ), dst,
                        src0, src(t1), negate(src(t1)));
   }

   return true;
}


/**
 * Translate/emit "begin subroutine" instruction/marker/label.
 */
static bool
emit_bgnsub(struct svga_shader_emitter *emit,
            unsigned position,
            const struct tgsi_full_instruction *insn)
{
   unsigned i;

   /* Note that we've finished the main function and are now emitting
    * subroutines.  This affects how we terminate the generated
    * shader.
    */
   emit->in_main_func = false;

   for (i = 0; i < emit->nr_labels; i++) {
      if (emit->label[i] == position) {
         return (emit_instruction( emit, inst_token( SVGA3DOP_RET ) ) &&
                 emit_instruction( emit, inst_token( SVGA3DOP_LABEL ) ) &&
                 emit_src( emit, src_register( SVGA3DREG_LABEL, i )));
      }
   }

   assert(0);
   return true;
}


/**
 * Translate/emit subroutine call instruction.
 */
static bool
emit_call(struct svga_shader_emitter *emit,
          const struct tgsi_full_instruction *insn)
{
   unsigned position = insn->Label.Label;
   unsigned i;

   for (i = 0; i < emit->nr_labels; i++) {
      if (emit->label[i] == position)
         break;
   }

   if (emit->nr_labels == ARRAY_SIZE(emit->label))
      return false;

   if (i == emit->nr_labels) {
      emit->label[i] = position;
      emit->nr_labels++;
   }

   return (emit_instruction( emit, inst_token( SVGA3DOP_CALL ) ) &&
           emit_src( emit, src_register( SVGA3DREG_LABEL, i )));
}


/**
 * Called at the end of the shader.  Actually, emit special "fix-up"
 * code for the vertex/fragment shader.
 */
static bool
emit_end(struct svga_shader_emitter *emit)
{
   if (emit->unit == PIPE_SHADER_VERTEX) {
      return emit_vs_postamble( emit );
   }
   else {
      return emit_ps_postamble( emit );
   }
}


/**
 * Translate any TGSI instruction to SVGA.
 */
static bool
svga_emit_instruction(struct svga_shader_emitter *emit,
                      unsigned position,
                      const struct tgsi_full_instruction *insn)
{
   switch (insn->Instruction.Opcode) {

   case TGSI_OPCODE_ARL:
      return emit_arl( emit, insn );

   case TGSI_OPCODE_TEX:
   case TGSI_OPCODE_TXB:
   case TGSI_OPCODE_TXP:
   case TGSI_OPCODE_TXL:
   case TGSI_OPCODE_TXD:
      return emit_tex( emit, insn );

   case TGSI_OPCODE_DDX:
   case TGSI_OPCODE_DDY:
      return emit_deriv( emit, insn );

   case TGSI_OPCODE_BGNSUB:
      return emit_bgnsub( emit, position, insn );

   case TGSI_OPCODE_ENDSUB:
      return true;

   case TGSI_OPCODE_CAL:
      return emit_call( emit, insn );

   case TGSI_OPCODE_FLR:
      return emit_floor( emit, insn );

   case TGSI_OPCODE_TRUNC:
      return emit_trunc_round( emit, insn, false );

   case TGSI_OPCODE_ROUND:
      return emit_trunc_round( emit, insn, true );

   case TGSI_OPCODE_CEIL:
      return emit_ceil( emit, insn );

   case TGSI_OPCODE_CMP:
      return emit_cmp( emit, insn );

   case TGSI_OPCODE_DIV:
      return emit_div( emit, insn );

   case TGSI_OPCODE_DP2:
      return emit_dp2( emit, insn );

   case TGSI_OPCODE_COS:
      return emit_cos( emit, insn );

   case TGSI_OPCODE_SIN:
      return emit_sin( emit, insn );

   case TGSI_OPCODE_END:
      /* TGSI always finishes the main func with an END */
      return emit_end( emit );

   case TGSI_OPCODE_KILL_IF:
      return emit_cond_discard( emit, insn );

      /* Selection opcodes.  The underlying language is fairly
       * non-orthogonal about these.
       */
   case TGSI_OPCODE_SEQ:
      return emit_select_op( emit, PIPE_FUNC_EQUAL, insn );

   case TGSI_OPCODE_SNE:
      return emit_select_op( emit, PIPE_FUNC_NOTEQUAL, insn );

   case TGSI_OPCODE_SGT:
      return emit_select_op( emit, PIPE_FUNC_GREATER, insn );

   case TGSI_OPCODE_SGE:
      return emit_select_op( emit, PIPE_FUNC_GEQUAL, insn );

   case TGSI_OPCODE_SLT:
      return emit_select_op( emit, PIPE_FUNC_LESS, insn );

   case TGSI_OPCODE_SLE:
      return emit_select_op( emit, PIPE_FUNC_LEQUAL, insn );

   case TGSI_OPCODE_POW:
      return emit_pow( emit, insn );

   case TGSI_OPCODE_EX2:
      return emit_ex2( emit, insn );

   case TGSI_OPCODE_EXP:
      return emit_exp( emit, insn );

   case TGSI_OPCODE_LOG:
      return emit_log( emit, insn );

   case TGSI_OPCODE_LG2:
      return emit_scalar_op1( emit, SVGA3DOP_LOG, insn );

   case TGSI_OPCODE_RSQ:
      return emit_scalar_op1( emit, SVGA3DOP_RSQ, insn );

   case TGSI_OPCODE_RCP:
      return emit_scalar_op1( emit, SVGA3DOP_RCP, insn );

   case TGSI_OPCODE_CONT:
      /* not expected (we return PIPE_SHADER_CAP_CONT_SUPPORTED = 0) */
      return false;

   case TGSI_OPCODE_RET:
      /* This is a noop -- we tell mesa that we can't support RET
       * within a function (early return), so this will always be
       * followed by an ENDSUB.
       */
      return true;

      /* These aren't actually used by any of the frontends we care
       * about:
       */
   case TGSI_OPCODE_AND:
   case TGSI_OPCODE_OR:
   case TGSI_OPCODE_I2F:
   case TGSI_OPCODE_NOT:
   case TGSI_OPCODE_SHL:
   case TGSI_OPCODE_ISHR:
   case TGSI_OPCODE_XOR:
      return false;

   case TGSI_OPCODE_IF:
      return emit_if( emit, insn );
   case TGSI_OPCODE_ELSE:
      return emit_else( emit, insn );
   case TGSI_OPCODE_ENDIF:
      return emit_endif( emit, insn );

   case TGSI_OPCODE_BGNLOOP:
      return emit_bgnloop( emit, insn );
   case TGSI_OPCODE_ENDLOOP:
      return emit_endloop( emit, insn );
   case TGSI_OPCODE_BRK:
      return emit_brk( emit, insn );

   case TGSI_OPCODE_KILL:
      return emit_discard( emit, insn );

   case TGSI_OPCODE_DST:
      return emit_dst_insn( emit, insn );

   case TGSI_OPCODE_LIT:
      return emit_lit( emit, insn );

   case TGSI_OPCODE_LRP:
      return emit_lrp( emit, insn );

   case TGSI_OPCODE_SSG:
      return emit_ssg( emit, insn );

   case TGSI_OPCODE_MOV:
      return emit_mov( emit, insn );

   case TGSI_OPCODE_SQRT:
      return emit_sqrt( emit, insn );

   default:
      {
         SVGA3dShaderOpCodeType opcode =
            translate_opcode(insn->Instruction.Opcode);

         if (opcode == SVGA3DOP_LAST_INST)
            return false;

         if (!emit_simple_instruction( emit, opcode, insn ))
            return false;
      }
   }

   return true;
}


/**
 * Translate/emit a TGSI IMMEDIATE declaration.
 * An immediate vector is a constant that's hard-coded into the shader.
 */
static bool
svga_emit_immediate(struct svga_shader_emitter *emit,
                    const struct tgsi_full_immediate *imm)
{
   static const float id[4] = {0,0,0,1};
   float value[4];
   unsigned i;

   assert(1 <= imm->Immediate.NrTokens && imm->Immediate.NrTokens <= 5);
   for (i = 0; i < 4 && i < imm->Immediate.NrTokens - 1; i++) {
      float f = imm->u[i].Float;
      value[i] = util_is_inf_or_nan(f) ? 0.0f : f;
   }

   /* If the immediate has less than four values, fill in the remaining
    * positions from id={0,0,0,1}.
    */
   for ( ; i < 4; i++ )
      value[i] = id[i];

   return emit_def_const( emit, SVGA3D_CONST_TYPE_FLOAT,
                          emit->imm_start + emit->internal_imm_count++,
                          value[0], value[1], value[2], value[3]);
}


static bool
make_immediate(struct svga_shader_emitter *emit,
               float a, float b, float c, float d,
               struct src_register *out )
{
   unsigned idx = emit->nr_hw_float_const++;

   if (!emit_def_const( emit, SVGA3D_CONST_TYPE_FLOAT,
                        idx, a, b, c, d ))
      return false;

   *out = src_register( SVGA3DREG_CONST, idx );

   return true;
}


/**
 * Emit special VS instructions at top of shader.
 */
static bool
emit_vs_preamble(struct svga_shader_emitter *emit)
{
   if (!emit->key.vs.need_prescale) {
      if (!make_immediate( emit, 0, 0, .5, .5,
                           &emit->imm_0055))
         return false;
   }

   return true;
}


/**
 * Emit special PS instructions at top of shader.
 */
static bool
emit_ps_preamble(struct svga_shader_emitter *emit)
{
   if (emit->ps_reads_pos && emit->info.reads_z) {
      /*
       * Assemble the position from various bits of inputs. Depth and W are
       * passed in a texcoord this is due to D3D's vPos not hold Z or W.
       * Also fixup the perspective interpolation.
       *
       * temp_pos.xy = vPos.xy
       * temp_pos.w = rcp(texcoord1.w);
       * temp_pos.z = texcoord1.z * temp_pos.w;
       */
      if (!submit_op1( emit,
                       inst_token(SVGA3DOP_MOV),
                       writemask( emit->ps_temp_pos, TGSI_WRITEMASK_XY ),
                       emit->ps_true_pos ))
         return false;

      if (!submit_op1( emit,
                       inst_token(SVGA3DOP_RCP),
                       writemask( emit->ps_temp_pos, TGSI_WRITEMASK_W ),
                       scalar( emit->ps_depth_pos, TGSI_SWIZZLE_W ) ))
         return false;

      if (!submit_op2( emit,
                       inst_token(SVGA3DOP_MUL),
                       writemask( emit->ps_temp_pos, TGSI_WRITEMASK_Z ),
                       scalar( emit->ps_depth_pos, TGSI_SWIZZLE_Z ),
                       scalar( src(emit->ps_temp_pos), TGSI_SWIZZLE_W ) ))
         return false;
   }

   return true;
}


/**
 * Emit special PS instructions at end of shader.
 */
static bool
emit_ps_postamble(struct svga_shader_emitter *emit)
{
   unsigned i;

   /* PS oDepth is incredibly fragile and it's very hard to catch the
    * types of usage that break it during shader emit.  Easier just to
    * redirect the main program to a temporary and then only touch
    * oDepth with a hand-crafted MOV below.
    */
   if (SVGA3dShaderGetRegType(emit->true_pos.value) != 0) {
      if (!submit_op1( emit,
                       inst_token(SVGA3DOP_MOV),
                       emit->true_pos,
                       scalar(src(emit->temp_pos), TGSI_SWIZZLE_Z) ))
         return false;
   }

   for (i = 0; i < PIPE_MAX_COLOR_BUFS; i++) {
      if (SVGA3dShaderGetRegType(emit->true_color_output[i].value) != 0) {
         /* Potentially override output colors with white for XOR
          * logicop workaround.
          */
         if (emit->unit == PIPE_SHADER_FRAGMENT &&
             emit->key.fs.white_fragments) {
            struct src_register one = get_one_immediate(emit);

            if (!submit_op1( emit,
                             inst_token(SVGA3DOP_MOV),
                             emit->true_color_output[i],
                             one ))
               return false;
         }
         else if (emit->unit == PIPE_SHADER_FRAGMENT &&
                  i < emit->key.fs.write_color0_to_n_cbufs) {
            /* Write temp color output [0] to true output [i] */
            if (!submit_op1(emit, inst_token(SVGA3DOP_MOV),
                            emit->true_color_output[i],
                            src(emit->temp_color_output[0]))) {
               return false;
            }
         }
         else {
            if (!submit_op1( emit,
                             inst_token(SVGA3DOP_MOV),
                             emit->true_color_output[i],
                             src(emit->temp_color_output[i]) ))
               return false;
         }
      }
   }

   return true;
}


/**
 * Emit special VS instructions at end of shader.
 */
static bool
emit_vs_postamble(struct svga_shader_emitter *emit)
{
   /* PSIZ output is incredibly fragile and it's very hard to catch
    * the types of usage that break it during shader emit.  Easier
    * just to redirect the main program to a temporary and then only
    * touch PSIZ with a hand-crafted MOV below.
    */
   if (SVGA3dShaderGetRegType(emit->true_psiz.value) != 0) {
      if (!submit_op1( emit,
                       inst_token(SVGA3DOP_MOV),
                       emit->true_psiz,
                       scalar(src(emit->temp_psiz), TGSI_SWIZZLE_X) ))
         return false;
   }

   /* Need to perform various manipulations on vertex position to cope
    * with the different GL and D3D clip spaces.
    */
   if (emit->key.vs.need_prescale) {
      SVGA3dShaderDestToken temp_pos = emit->temp_pos;
      SVGA3dShaderDestToken depth = emit->depth_pos;
      SVGA3dShaderDestToken pos = emit->true_pos;
      unsigned offset = emit->info.file_max[TGSI_FILE_CONSTANT] + 1;
      struct src_register prescale_scale = src_register( SVGA3DREG_CONST,
                                                         offset + 0 );
      struct src_register prescale_trans = src_register( SVGA3DREG_CONST,
                                                         offset + 1 );

      if (!submit_op1( emit,
                       inst_token(SVGA3DOP_MOV),
                       writemask(depth, TGSI_WRITEMASK_W),
                       scalar(src(temp_pos), TGSI_SWIZZLE_W) ))
         return false;

      /* MUL temp_pos.xyz,    temp_pos,      prescale.scale
       * MAD result.position, temp_pos.wwww, prescale.trans, temp_pos
       *   --> Note that prescale.trans.w == 0
       */
      if (!submit_op2( emit,
                       inst_token(SVGA3DOP_MUL),
                       writemask(temp_pos, TGSI_WRITEMASK_XYZ),
                       src(temp_pos),
                       prescale_scale ))
         return false;

      if (!submit_op3( emit,
                       inst_token(SVGA3DOP_MAD),
                       pos,
                       swizzle(src(temp_pos), 3, 3, 3, 3),
                       prescale_trans,
                       src(temp_pos)))
         return false;

      /* Also write to depth value */
      if (!submit_op3( emit,
                       inst_token(SVGA3DOP_MAD),
                       writemask(depth, TGSI_WRITEMASK_Z),
                       swizzle(src(temp_pos), 3, 3, 3, 3),
                       prescale_trans,
                       src(temp_pos) ))
         return false;
   }
   else {
      SVGA3dShaderDestToken temp_pos = emit->temp_pos;
      SVGA3dShaderDestToken depth = emit->depth_pos;
      SVGA3dShaderDestToken pos = emit->true_pos;
      struct src_register imm_0055 = emit->imm_0055;

      /* Adjust GL clipping coordinate space to hardware (D3D-style):
       *
       * DP4 temp_pos.z, {0,0,.5,.5}, temp_pos
       * MOV result.position, temp_pos
       */
      if (!submit_op2( emit,
                       inst_token(SVGA3DOP_DP4),
                       writemask(temp_pos, TGSI_WRITEMASK_Z),
                       imm_0055,
                       src(temp_pos) ))
         return false;

      if (!submit_op1( emit,
                       inst_token(SVGA3DOP_MOV),
                       pos,
                       src(temp_pos) ))
         return false;

      /* Move the manipulated depth into the extra texcoord reg */
      if (!submit_op1( emit,
                       inst_token(SVGA3DOP_MOV),
                       writemask(depth, TGSI_WRITEMASK_ZW),
                       src(temp_pos) ))
         return false;
   }

   return true;
}


/**
 * For the pixel shader: emit the code which chooses the front
 * or back face color depending on triangle orientation.
 * This happens at the top of the fragment shader.
 *
 *  0: IF VFACE :4
 *  1:   COLOR = FrontColor;
 *  2: ELSE
 *  3:   COLOR = BackColor;
 *  4: ENDIF
 */
static bool
emit_light_twoside(struct svga_shader_emitter *emit)
{
   struct src_register vface, zero;
   struct src_register front[2];
   struct src_register back[2];
   SVGA3dShaderDestToken color[2];
   int count = emit->internal_color_count;
   unsigned i;
   SVGA3dShaderInstToken if_token;

   if (count == 0)
      return true;

   vface = get_vface( emit );
   zero = get_zero_immediate(emit);

   /* Can't use get_temp() to allocate the color reg as such
    * temporaries will be reclaimed after each instruction by the call
    * to reset_temp_regs().
    */
   for (i = 0; i < count; i++) {
      color[i] = dst_register( SVGA3DREG_TEMP, emit->nr_hw_temp++ );
      front[i] = emit->input_map[emit->internal_color_idx[i]];

      /* Back is always the next input:
       */
      back[i] = front[i];
      back[i].base.num = front[i].base.num + 1;

      /* Reassign the input_map to the actual front-face color:
       */
      emit->input_map[emit->internal_color_idx[i]] = src(color[i]);
   }

   if_token = inst_token( SVGA3DOP_IFC );

   if (emit->key.fs.front_ccw)
      if_token.control = SVGA3DOPCOMP_LT;
   else
      if_token.control = SVGA3DOPCOMP_GT;

   if (!(emit_instruction( emit, if_token ) &&
         emit_src( emit, vface ) &&
         emit_src( emit, zero ) ))
      return false;

   for (i = 0; i < count; i++) {
      if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ), color[i], front[i] ))
         return false;
   }

   if (!(emit_instruction( emit, inst_token( SVGA3DOP_ELSE))))
      return false;

   for (i = 0; i < count; i++) {
      if (!submit_op1( emit, inst_token( SVGA3DOP_MOV ), color[i], back[i] ))
         return false;
   }

   if (!emit_instruction( emit, inst_token( SVGA3DOP_ENDIF ) ))
      return false;

   return true;
}


/**
 * Emit special setup code for the front/back face register in the FS.
 *  0: SETP_GT TEMP, VFACE, 0
 *  where TEMP is a fake frontface register
 */
static bool
emit_frontface(struct svga_shader_emitter *emit)
{
   struct src_register vface;
   SVGA3dShaderDestToken temp;
   struct src_register pass, fail;

   vface = get_vface( emit );

   /* Can't use get_temp() to allocate the fake frontface reg as such
    * temporaries will be reclaimed after each instruction by the call
    * to reset_temp_regs().
    */
   temp = dst_register( SVGA3DREG_TEMP,
                        emit->nr_hw_temp++ );

   if (emit->key.fs.front_ccw) {
      pass = get_zero_immediate(emit);
      fail = get_one_immediate(emit);
   } else {
      pass = get_one_immediate(emit);
      fail = get_zero_immediate(emit);
   }

   if (!emit_conditional(emit, PIPE_FUNC_GREATER,
                         temp, vface, get_zero_immediate(emit),
                         pass, fail))
      return false;

   /* Reassign the input_map to the actual front-face color:
    */
   emit->input_map[emit->internal_frontface_idx] = src(temp);

   return true;
}


/**
 * Emit code to invert the T component of the incoming texture coordinate.
 * This is used for drawing point sprites when
 * pipe_rasterizer_state::sprite_coord_mode == PIPE_SPRITE_COORD_LOWER_LEFT.
 */
static bool
emit_inverted_texcoords(struct svga_shader_emitter *emit)
{
   unsigned inverted_texcoords = emit->inverted_texcoords;

   while (inverted_texcoords) {
      const unsigned unit = ffs(inverted_texcoords) - 1;

      assert(emit->inverted_texcoords & (1 << unit));

      assert(unit < ARRAY_SIZE(emit->ps_true_texcoord));

      assert(unit < ARRAY_SIZE(emit->ps_inverted_texcoord_input));

      assert(emit->ps_inverted_texcoord_input[unit]
             < ARRAY_SIZE(emit->input_map));

      /* inverted = coord * (1, -1, 1, 1) + (0, 1, 0, 0) */
      if (!submit_op3(emit,
                      inst_token(SVGA3DOP_MAD),
                      dst(emit->ps_inverted_texcoord[unit]),
                      emit->ps_true_texcoord[unit],
                      get_immediate(emit, 1.0f, -1.0f, 1.0f, 1.0f),
                      get_immediate(emit, 0.0f, 1.0f, 0.0f, 0.0f)))
         return false;

      /* Reassign the input_map entry to the new texcoord register */
      emit->input_map[emit->ps_inverted_texcoord_input[unit]] =
         emit->ps_inverted_texcoord[unit];

      inverted_texcoords &= ~(1 << unit);
   }

   return true;
}


/**
 * Emit code to adjust vertex shader inputs/attributes:
 * - Change range from [0,1] to [-1,1] (for normalized byte/short attribs).
 * - Set attrib W component = 1.
 */
static bool
emit_adjusted_vertex_attribs(struct svga_shader_emitter *emit)
{
   unsigned adjust_mask = (emit->key.vs.adjust_attrib_range |
                           emit->key.vs.adjust_attrib_w_1);
 
   while (adjust_mask) {
      /* Adjust vertex attrib range and/or set W component = 1 */
      const unsigned index = u_bit_scan(&adjust_mask);
      struct src_register tmp;

      /* allocate a temp reg */
      tmp = src_register(SVGA3DREG_TEMP, emit->nr_hw_temp);
      emit->nr_hw_temp++;

      if (emit->key.vs.adjust_attrib_range & (1 << index)) {
         /* The vertex input/attribute is supposed to be a signed value in
          * the range [-1,1] but we actually fetched/converted it to the
          * range [0,1].  This most likely happens when the app specifies a
          * signed byte attribute but we interpreted it as unsigned bytes.
          * See also svga_translate_vertex_format().
          *
          * Here, we emit some extra instructions to adjust
          * the attribute values from [0,1] to [-1,1].
          *
          * The adjustment we implement is:
          *   new_attrib = attrib * 2.0;
          *   if (attrib >= 0.5)
          *      new_attrib = new_attrib - 2.0;
          * This isn't exactly right (it's off by a bit or so) but close enough.
          */
         SVGA3dShaderDestToken pred_reg = dst_register(SVGA3DREG_PREDICATE, 0);

         /* tmp = attrib * 2.0 */
         if (!submit_op2(emit,
                         inst_token(SVGA3DOP_MUL),
                         dst(tmp),
                         emit->input_map[index],
                         get_two_immediate(emit)))
            return false;

         /* pred = (attrib >= 0.5) */
         if (!submit_op2(emit,
                         inst_token_setp(SVGA3DOPCOMP_GE),
                         pred_reg,
                         emit->input_map[index],  /* vert attrib */
                         get_half_immediate(emit)))  /* 0.5 */
            return false;

         /* sub(pred) tmp, tmp, 2.0 */
         if (!submit_op3(emit,
                         inst_token_predicated(SVGA3DOP_SUB),
                         dst(tmp),
                         src(pred_reg),
                         tmp,
                         get_two_immediate(emit)))
            return false;
      }
      else {
         /* just copy the vertex input attrib to the temp register */
         if (!submit_op1(emit,
                         inst_token(SVGA3DOP_MOV),
                         dst(tmp),
                         emit->input_map[index]))
            return false;
      }

      if (emit->key.vs.adjust_attrib_w_1 & (1 << index)) {
         /* move 1 into W position of tmp */
         if (!submit_op1(emit,
                         inst_token(SVGA3DOP_MOV),
                         writemask(dst(tmp), TGSI_WRITEMASK_W),
                         get_one_immediate(emit)))
            return false;
      }

      /* Reassign the input_map entry to the new tmp register */
      emit->input_map[index] = tmp;
   }

   return true;
}


/**
 * Determine if we need to create the "common" immediate value which is
 * used for generating useful vector constants such as {0,0,0,0} and
 * {1,1,1,1}.
 * We could just do this all the time except that we want to conserve
 * registers whenever possible.
 */
static bool
needs_to_create_common_immediate(const struct svga_shader_emitter *emit)
{
   unsigned i;

   if (emit->unit == PIPE_SHADER_FRAGMENT) {
      if (emit->key.fs.light_twoside)
         return true;

      if (emit->key.fs.white_fragments)
         return true;

      if (emit->emit_frontface)
         return true;

      if (emit->info.opcode_count[TGSI_OPCODE_DST] >= 1 ||
          emit->info.opcode_count[TGSI_OPCODE_SSG] >= 1 ||
          emit->info.opcode_count[TGSI_OPCODE_LIT] >= 1)
         return true;

      if (emit->inverted_texcoords)
         return true;

      /* look for any PIPE_SWIZZLE_0/ONE terms */
      for (i = 0; i < emit->key.num_textures; i++) {
         if (emit->key.tex[i].swizzle_r > PIPE_SWIZZLE_W ||
             emit->key.tex[i].swizzle_g > PIPE_SWIZZLE_W ||
             emit->key.tex[i].swizzle_b > PIPE_SWIZZLE_W ||
             emit->key.tex[i].swizzle_a > PIPE_SWIZZLE_W)
            return true;
      }

      for (i = 0; i < emit->key.num_textures; i++) {
         if (emit->key.tex[i].compare_mode
             == PIPE_TEX_COMPARE_R_TO_TEXTURE)
            return true;
      }
   }
   else if (emit->unit == PIPE_SHADER_VERTEX) {
      if (emit->info.opcode_count[TGSI_OPCODE_CMP] >= 1)
         return true;
      if (emit->key.vs.adjust_attrib_range ||
          emit->key.vs.adjust_attrib_w_1)
         return true;
   }

   if (emit->info.opcode_count[TGSI_OPCODE_IF] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_BGNLOOP] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_DDX] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_DDY] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_ROUND] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_SGE] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_SGT] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_SLE] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_SLT] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_SNE] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_SEQ] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_EXP] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_LOG] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_KILL] >= 1 ||
       emit->info.opcode_count[TGSI_OPCODE_SQRT] >= 1)
      return true;

   return false;
}


/**
 * Do we need to create a looping constant?
 */
static bool
needs_to_create_loop_const(const struct svga_shader_emitter *emit)
{
   return (emit->info.opcode_count[TGSI_OPCODE_BGNLOOP] >= 1);
}


static bool
needs_to_create_arl_consts(const struct svga_shader_emitter *emit)
{
   return (emit->num_arl_consts > 0);
}


static bool
pre_parse_add_indirect( struct svga_shader_emitter *emit,
                        int num, int current_arl)
{
   unsigned i;
   assert(num < 0);

   for (i = 0; i < emit->num_arl_consts; ++i) {
      if (emit->arl_consts[i].arl_num == current_arl)
         break;
   }
   /* new entry */
   if (emit->num_arl_consts == i) {
      ++emit->num_arl_consts;
   }
   emit->arl_consts[i].number = (emit->arl_consts[i].number > num) ?
                                num :
                                emit->arl_consts[i].number;
   emit->arl_consts[i].arl_num = current_arl;
   return true;
}


static bool
pre_parse_instruction( struct svga_shader_emitter *emit,
                       const struct tgsi_full_instruction *insn,
                       int current_arl)
{
   if (insn->Src[0].Register.Indirect &&
       insn->Src[0].Indirect.File == TGSI_FILE_ADDRESS) {
      const struct tgsi_full_src_register *reg = &insn->Src[0];
      if (reg->Register.Index < 0) {
         pre_parse_add_indirect(emit, reg->Register.Index, current_arl);
      }
   }

   if (insn->Src[1].Register.Indirect &&
       insn->Src[1].Indirect.File == TGSI_FILE_ADDRESS) {
      const struct tgsi_full_src_register *reg = &insn->Src[1];
      if (reg->Register.Index < 0) {
         pre_parse_add_indirect(emit, reg->Register.Index, current_arl);
      }
   }

   if (insn->Src[2].Register.Indirect &&
       insn->Src[2].Indirect.File == TGSI_FILE_ADDRESS) {
      const struct tgsi_full_src_register *reg = &insn->Src[2];
      if (reg->Register.Index < 0) {
         pre_parse_add_indirect(emit, reg->Register.Index, current_arl);
      }
   }

   return true;
}


static bool
pre_parse_tokens( struct svga_shader_emitter *emit,
                  const struct tgsi_token *tokens )
{
   struct tgsi_parse_context parse;
   int current_arl = 0;

   tgsi_parse_init( &parse, tokens );

   while (!tgsi_parse_end_of_tokens( &parse )) {
      tgsi_parse_token( &parse );
      switch (parse.FullToken.Token.Type) {
      case TGSI_TOKEN_TYPE_IMMEDIATE:
      case TGSI_TOKEN_TYPE_DECLARATION:
         break;
      case TGSI_TOKEN_TYPE_INSTRUCTION:
         if (parse.FullToken.FullInstruction.Instruction.Opcode ==
             TGSI_OPCODE_ARL) {
            ++current_arl;
         }
         if (!pre_parse_instruction( emit, &parse.FullToken.FullInstruction,
                                     current_arl ))
            return false;
         break;
      default:
         break;
      }

   }
   return true;
}


static bool
svga_shader_emit_helpers(struct svga_shader_emitter *emit)
{
   if (needs_to_create_common_immediate( emit )) {
      create_common_immediate( emit );
   }
   if (needs_to_create_loop_const( emit )) {
      create_loop_const( emit );
   }
   if (needs_to_create_arl_consts( emit )) {
      create_arl_consts( emit );
   }

   if (emit->unit == PIPE_SHADER_FRAGMENT) {
      if (!svga_shader_emit_samplers_decl( emit ))
         return false;

      if (!emit_ps_preamble( emit ))
         return false;

      if (emit->key.fs.light_twoside) {
         if (!emit_light_twoside( emit ))
            return false;
      }
      if (emit->emit_frontface) {
         if (!emit_frontface( emit ))
            return false;
      }
      if (emit->inverted_texcoords) {
         if (!emit_inverted_texcoords( emit ))
            return false;
      }
   }
   else {
      assert(emit->unit == PIPE_SHADER_VERTEX);
      if (emit->key.vs.adjust_attrib_range) {
         if (!emit_adjusted_vertex_attribs(emit) ||
             emit->key.vs.adjust_attrib_w_1) {
            return false;
         }
      }
   }

   return true;
}


/**
 * This is the main entrypoint into the TGSI instruction translater.
 * Translate TGSI shader tokens into an SVGA shader.
 */
bool
svga_shader_emit_instructions(struct svga_shader_emitter *emit,
                              const struct tgsi_token *tokens)
{
   struct tgsi_parse_context parse;
   const struct tgsi_token *new_tokens = NULL;
   bool ret = true;
   bool helpers_emitted = false;
   unsigned line_nr = 0;

   if (emit->unit == PIPE_SHADER_FRAGMENT && emit->key.fs.pstipple) {
      unsigned unit;

      new_tokens = util_pstipple_create_fragment_shader(tokens, &unit, 0,
                                                        TGSI_FILE_INPUT);

      if (new_tokens) {
         /* Setup texture state for stipple */
         emit->sampler_target[unit] = TGSI_TEXTURE_2D;
         emit->key.tex[unit].swizzle_r = TGSI_SWIZZLE_X;
         emit->key.tex[unit].swizzle_g = TGSI_SWIZZLE_Y;
         emit->key.tex[unit].swizzle_b = TGSI_SWIZZLE_Z;
         emit->key.tex[unit].swizzle_a = TGSI_SWIZZLE_W;

         emit->pstipple_sampler_unit = unit;

         tokens = new_tokens;
      }
   }

   tgsi_parse_init( &parse, tokens );
   emit->internal_imm_count = 0;

   if (emit->unit == PIPE_SHADER_VERTEX) {
      ret = emit_vs_preamble( emit );
      if (!ret)
         goto done;
   }

   pre_parse_tokens(emit, tokens);

   while (!tgsi_parse_end_of_tokens( &parse )) {
      tgsi_parse_token( &parse );

      switch (parse.FullToken.Token.Type) {
      case TGSI_TOKEN_TYPE_IMMEDIATE:
         ret = svga_emit_immediate( emit, &parse.FullToken.FullImmediate );
         if (!ret)
            goto done;
         break;

      case TGSI_TOKEN_TYPE_DECLARATION:
         ret = svga_translate_decl_sm30( emit, &parse.FullToken.FullDeclaration );
         if (!ret)
            goto done;
         break;

      case TGSI_TOKEN_TYPE_INSTRUCTION:
         if (!helpers_emitted) {
            if (!svga_shader_emit_helpers( emit ))
               goto done;
            helpers_emitted = true;
         }
         ret = svga_emit_instruction( emit,
                                      line_nr++,
                                      &parse.FullToken.FullInstruction );
         if (!ret)
            goto done;
         break;
      default:
         break;
      }

      reset_temp_regs( emit );
   }

   /* Need to terminate the current subroutine.  Note that the
    * hardware doesn't tolerate shaders without sub-routines
    * terminating with RET+END.
    */
   if (!emit->in_main_func) {
      ret = emit_instruction( emit, inst_token( SVGA3DOP_RET ) );
      if (!ret)
         goto done;
   }

   assert(emit->dynamic_branching_level == 0);

   /* Need to terminate the whole shader:
    */
   ret = emit_instruction( emit, inst_token( SVGA3DOP_END ) );
   if (!ret)
      goto done;

done:
   tgsi_parse_free( &parse );
   if (new_tokens) {
      tgsi_free_tokens(new_tokens);
   }

   return ret;
}
