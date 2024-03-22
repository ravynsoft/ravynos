/* -*- c++ -*- */
/*
 * Copyright © 2010-2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef BRW_VEC4_BUILDER_H
#define BRW_VEC4_BUILDER_H

#include "brw_ir_vec4.h"
#include "brw_ir_allocator.h"

namespace brw {
   /**
    * Toolbox to assemble a VEC4 IR program out of individual instructions.
    *
    * This object is meant to have an interface consistent with
    * brw::fs_builder.  They cannot be fully interchangeable because
    * brw::fs_builder generates scalar code while brw::vec4_builder generates
    * vector code.
    */
   class vec4_builder {
   public:
      /** Type used in this IR to represent a source of an instruction. */
      typedef brw::src_reg src_reg;

      /** Type used in this IR to represent the destination of an instruction. */
      typedef brw::dst_reg dst_reg;

      /** Type used in this IR to represent an instruction. */
      typedef vec4_instruction instruction;

      /**
       * Construct a vec4_builder that inserts instructions into \p shader.
       */
      vec4_builder(backend_shader *shader, unsigned dispatch_width = 8) :
         shader(shader), block(NULL), cursor(NULL),
         _dispatch_width(dispatch_width), _group(0),
         force_writemask_all(false),
         annotation()
      {
      }

      /**
       * Construct a vec4_builder that inserts instructions into \p shader
       * before instruction \p inst in basic block \p block.  The default
       * execution controls and debug annotation are initialized from the
       * instruction passed as argument.
       */
      vec4_builder(backend_shader *shader, bblock_t *block, instruction *inst) :
         shader(shader), block(block), cursor(inst),
         _dispatch_width(inst->exec_size), _group(inst->group),
         force_writemask_all(inst->force_writemask_all)
      {
         annotation.str = inst->annotation;
         annotation.ir = inst->ir;
      }

      /**
       * Construct a vec4_builder that inserts instructions before \p cursor
       * in basic block \p block, inheriting other code generation parameters
       * from this.
       */
      vec4_builder
      at(bblock_t *block, exec_node *cursor) const
      {
         vec4_builder bld = *this;
         bld.block = block;
         bld.cursor = cursor;
         return bld;
      }

      /**
       * Construct a vec4_builder appending instructions at the end of the
       * instruction list of the shader, inheriting other code generation
       * parameters from this.
       */
      vec4_builder
      at_end() const
      {
         return at(NULL, (exec_node *)&shader->instructions.tail_sentinel);
      }

      /**
       * Construct a builder specifying the default SIMD width and group of
       * channel enable signals, inheriting other code generation parameters
       * from this.
       *
       * \p n gives the default SIMD width, \p i gives the slot group used for
       * predication and control flow masking in multiples of \p n channels.
       */
      vec4_builder
      group(unsigned n, unsigned i) const
      {
         assert(force_writemask_all ||
                (n <= dispatch_width() && i < dispatch_width() / n));
         vec4_builder bld = *this;
         bld._dispatch_width = n;
         bld._group += i * n;
         return bld;
      }

      /**
       * Construct a builder with per-channel control flow execution masking
       * disabled if \p b is true.  If control flow execution masking is
       * already disabled this has no effect.
       */
      vec4_builder
      exec_all(bool b = true) const
      {
         vec4_builder bld = *this;
         if (b)
            bld.force_writemask_all = true;
         return bld;
      }

      /**
       * Construct a builder with the given debug annotation info.
       */
      vec4_builder
      annotate(const char *str, const void *ir = NULL) const
      {
         vec4_builder bld = *this;
         bld.annotation.str = str;
         bld.annotation.ir = ir;
         return bld;
      }

      /**
       * Get the SIMD width in use.
       */
      unsigned
      dispatch_width() const
      {
         return _dispatch_width;
      }

      /**
       * Get the channel group in use.
       */
      unsigned
      group() const
      {
         return _group;
      }

      /**
       * Allocate a virtual register of natural vector size (four for this IR)
       * and SIMD width.  \p n gives the amount of space to allocate in
       * dispatch_width units (which is just enough space for four logical
       * components in this IR).
       */
      dst_reg
      vgrf(enum brw_reg_type type, unsigned n = 1) const
      {
         assert(dispatch_width() <= 32);

         if (n > 0)
            return retype(dst_reg(VGRF, shader->alloc.allocate(
                                     n * DIV_ROUND_UP(type_sz(type), 4))),
                           type);
         else
            return retype(null_reg_ud(), type);
      }

      /**
       * Create a null register of floating type.
       */
      dst_reg
      null_reg_f() const
      {
         return dst_reg(retype(brw_null_vec(dispatch_width()),
                               BRW_REGISTER_TYPE_F));
      }

      /**
       * Create a null register of signed integer type.
       */
      dst_reg
      null_reg_d() const
      {
         return dst_reg(retype(brw_null_vec(dispatch_width()),
                               BRW_REGISTER_TYPE_D));
      }

      /**
       * Create a null register of unsigned integer type.
       */
      dst_reg
      null_reg_ud() const
      {
         return dst_reg(retype(brw_null_vec(dispatch_width()),
                               BRW_REGISTER_TYPE_UD));
      }

      /**
       * Insert an instruction into the program.
       */
      instruction *
      emit(const instruction &inst) const
      {
         return emit(new(shader->mem_ctx) instruction(inst));
      }

      /**
       * Create and insert a nullary control instruction into the program.
       */
      instruction *
      emit(enum opcode opcode) const
      {
         return emit(instruction(opcode));
      }

      /**
       * Create and insert a nullary instruction into the program.
       */
      instruction *
      emit(enum opcode opcode, const dst_reg &dst) const
      {
         return emit(instruction(opcode, dst));
      }

      /**
       * Create and insert a unary instruction into the program.
       */
      instruction *
      emit(enum opcode opcode, const dst_reg &dst, const src_reg &src0) const
      {
         switch (opcode) {
         case SHADER_OPCODE_RCP:
         case SHADER_OPCODE_RSQ:
         case SHADER_OPCODE_SQRT:
         case SHADER_OPCODE_EXP2:
         case SHADER_OPCODE_LOG2:
         case SHADER_OPCODE_SIN:
         case SHADER_OPCODE_COS:
            return fix_math_instruction(
               emit(instruction(opcode, dst,
                                fix_math_operand(src0))));

         default:
            return emit(instruction(opcode, dst, src0));
         }
      }

      /**
       * Create and insert a binary instruction into the program.
       */
      instruction *
      emit(enum opcode opcode, const dst_reg &dst, const src_reg &src0,
           const src_reg &src1) const
      {
         switch (opcode) {
         case SHADER_OPCODE_POW:
         case SHADER_OPCODE_INT_QUOTIENT:
         case SHADER_OPCODE_INT_REMAINDER:
            return fix_math_instruction(
               emit(instruction(opcode, dst,
                                fix_math_operand(src0),
                                fix_math_operand(src1))));

         default:
            return emit(instruction(opcode, dst, src0, src1));
         }
      }

      /**
       * Create and insert a ternary instruction into the program.
       */
      instruction *
      emit(enum opcode opcode, const dst_reg &dst, const src_reg &src0,
           const src_reg &src1, const src_reg &src2) const
      {
         switch (opcode) {
         case BRW_OPCODE_BFE:
         case BRW_OPCODE_BFI2:
         case BRW_OPCODE_MAD:
         case BRW_OPCODE_LRP:
            return emit(instruction(opcode, dst,
                                    fix_3src_operand(src0),
                                    fix_3src_operand(src1),
                                    fix_3src_operand(src2)));

         default:
            return emit(instruction(opcode, dst, src0, src1, src2));
         }
      }

      /**
       * Insert a preallocated instruction into the program.
       */
      instruction *
      emit(instruction *inst) const
      {
         inst->exec_size = dispatch_width();
         inst->group = group();
         inst->force_writemask_all = force_writemask_all;
         inst->size_written = inst->exec_size * type_sz(inst->dst.type);
         inst->annotation = annotation.str;
         inst->ir = annotation.ir;

         if (block)
            static_cast<instruction *>(cursor)->insert_before(block, inst);
         else
            cursor->insert_before(inst);

         return inst;
      }

      /**
       * Select \p src0 if the comparison of both sources with the given
       * conditional mod evaluates to true, otherwise select \p src1.
       *
       * Generally useful to get the minimum or maximum of two values.
       */
      instruction *
      emit_minmax(const dst_reg &dst, const src_reg &src0,
                  const src_reg &src1, brw_conditional_mod mod) const
      {
         assert(mod == BRW_CONDITIONAL_GE || mod == BRW_CONDITIONAL_L);

         return set_condmod(mod, SEL(dst, fix_unsigned_negate(src0),
                                     fix_unsigned_negate(src1)));
      }

      /**
       * Copy any live channel from \p src to the first channel of the result.
       */
      src_reg
      emit_uniformize(const src_reg &src) const
      {
         const vec4_builder ubld = exec_all();
         const dst_reg chan_index =
            writemask(vgrf(BRW_REGISTER_TYPE_UD), WRITEMASK_X);
         const dst_reg dst = vgrf(src.type);

         ubld.emit(SHADER_OPCODE_FIND_LIVE_CHANNEL, chan_index);
         ubld.emit(SHADER_OPCODE_BROADCAST, dst, src, src_reg(chan_index));

         return src_reg(dst);
      }

      /**
       * Assorted arithmetic ops.
       * @{
       */
#define ALU1(op)                                        \
      instruction *                                     \
      op(const dst_reg &dst, const src_reg &src0) const \
      {                                                 \
         return emit(BRW_OPCODE_##op, dst, src0);       \
      }

#define ALU2(op)                                                        \
      instruction *                                                     \
      op(const dst_reg &dst, const src_reg &src0, const src_reg &src1) const \
      {                                                                 \
         return emit(BRW_OPCODE_##op, dst, src0, src1);                 \
      }

#define ALU2_ACC(op)                                                    \
      instruction *                                                     \
      op(const dst_reg &dst, const src_reg &src0, const src_reg &src1) const \
      {                                                                 \
         instruction *inst = emit(BRW_OPCODE_##op, dst, src0, src1);    \
         inst->writes_accumulator = true;                               \
         return inst;                                                   \
      }

#define ALU3(op)                                                        \
      instruction *                                                     \
      op(const dst_reg &dst, const src_reg &src0, const src_reg &src1,  \
         const src_reg &src2) const                                     \
      {                                                                 \
         return emit(BRW_OPCODE_##op, dst, src0, src1, src2);           \
      }

      ALU2(ADD)
      ALU2_ACC(ADDC)
      ALU2(AND)
      ALU2(ASR)
      ALU2(AVG)
      ALU3(BFE)
      ALU2(BFI1)
      ALU3(BFI2)
      ALU1(BFREV)
      ALU1(CBIT)
      ALU3(CSEL)
      ALU1(DIM)
      ALU2(DP2)
      ALU2(DP3)
      ALU2(DP4)
      ALU2(DPH)
      ALU1(F16TO32)
      ALU1(F32TO16)
      ALU1(FBH)
      ALU1(FBL)
      ALU1(FRC)
      ALU2(LINE)
      ALU1(LZD)
      ALU2(MAC)
      ALU2_ACC(MACH)
      ALU3(MAD)
      ALU1(MOV)
      ALU2(MUL)
      ALU1(NOT)
      ALU2(OR)
      ALU2(PLN)
      ALU1(RNDD)
      ALU1(RNDE)
      ALU1(RNDU)
      ALU1(RNDZ)
      ALU2(SAD2)
      ALU2_ACC(SADA2)
      ALU2(SEL)
      ALU2(SHL)
      ALU2(SHR)
      ALU2_ACC(SUBB)
      ALU2(XOR)

#undef ALU3
#undef ALU2_ACC
#undef ALU2
#undef ALU1
      /** @} */

      /**
       * CMP: Sets the low bit of the destination channels with the result
       * of the comparison, while the upper bits are undefined, and updates
       * the flag register with the packed 16 bits of the result.
       */
      instruction *
      CMP(const dst_reg &dst, const src_reg &src0, const src_reg &src1,
          brw_conditional_mod condition) const
      {
         /* Take the instruction:
          *
          * CMP null<d> src0<f> src1<f>
          *
          * Original gfx4 does type conversion to the destination type
          * before comparison, producing garbage results for floating
          * point comparisons.
          *
          * The destination type doesn't matter on newer generations,
          * so we set the type to match src0 so we can compact the
          * instruction.
          */
         return set_condmod(condition,
                            emit(BRW_OPCODE_CMP, retype(dst, src0.type),
                                 fix_unsigned_negate(src0),
                                 fix_unsigned_negate(src1)));
      }

      /**
       * CMPN: Behaves like CMP, but produces true if src1 is NaN.
       */
      instruction *
      CMPN(const dst_reg &dst, const src_reg &src0, const src_reg &src1,
          brw_conditional_mod condition) const
      {
         /* Take the instruction:
          *
          * CMPN null<d> src0<f> src1<f>
          *
          * Original gfx4 does type conversion to the destination type
          * before comparison, producing garbage results for floating
          * point comparisons.
          *
          * The destination type doesn't matter on newer generations,
          * so we set the type to match src0 so we can compact the
          * instruction.
          */
         return set_condmod(condition,
                            emit(BRW_OPCODE_CMPN, retype(dst, src0.type),
                                 fix_unsigned_negate(src0),
                                 fix_unsigned_negate(src1)));
      }

      /**
       * Gfx4 predicated IF.
       */
      instruction *
      IF(brw_predicate predicate) const
      {
         return set_predicate(predicate, emit(BRW_OPCODE_IF));
      }

      /**
       * Gfx6 IF with embedded comparison.
       */
      instruction *
      IF(const src_reg &src0, const src_reg &src1,
         brw_conditional_mod condition) const
      {
         assert(shader->devinfo->ver == 6);
         return set_condmod(condition,
                            emit(BRW_OPCODE_IF,
                                 null_reg_d(),
                                 fix_unsigned_negate(src0),
                                 fix_unsigned_negate(src1)));
      }

      /**
       * Emit a linear interpolation instruction.
       */
      instruction *
      LRP(const dst_reg &dst, const src_reg &x, const src_reg &y,
          const src_reg &a) const
      {
         /* The LRP instruction actually does op1 * op0 + op2 * (1 - op0), so
          * we need to reorder the operands.
          */
         assert(shader->devinfo->ver >= 6 && shader->devinfo->ver <= 9);
         return emit(BRW_OPCODE_LRP, dst, a, y, x);
      }

      backend_shader *shader;

   protected:
      /**
       * Workaround for negation of UD registers.  See comment in
       * fs_generator::generate_code() for the details.
       */
      src_reg
      fix_unsigned_negate(const src_reg &src) const
      {
         if (src.type == BRW_REGISTER_TYPE_UD && src.negate) {
            dst_reg temp = vgrf(BRW_REGISTER_TYPE_UD);
            MOV(temp, src);
            return src_reg(temp);
         } else {
            return src;
         }
      }

      /**
       * Workaround for register access modes not supported by the ternary
       * instruction encoding.
       */
      src_reg
      fix_3src_operand(const src_reg &src) const
      {
         /* Using vec4 uniforms in SIMD4x2 programs is difficult. You'd like to be
          * able to use vertical stride of zero to replicate the vec4 uniform, like
          *
          *    g3<0;4,1>:f - [0, 4][1, 5][2, 6][3, 7]
          *
          * But you can't, since vertical stride is always four in three-source
          * instructions. Instead, insert a MOV instruction to do the replication so
          * that the three-source instruction can consume it.
          */

         /* The MOV is only needed if the source is a uniform or immediate. */
         if (src.file != UNIFORM && src.file != IMM)
            return src;

         if (src.file == UNIFORM && brw_is_single_value_swizzle(src.swizzle))
            return src;

         const dst_reg expanded = vgrf(src.type);
         emit(VEC4_OPCODE_UNPACK_UNIFORM, expanded, src);
         return src_reg(expanded);
      }

      /**
       * Workaround for register access modes not supported by the math
       * instruction.
       */
      src_reg
      fix_math_operand(const src_reg &src) const
      {
         /* The gfx6 math instruction ignores the source modifiers --
          * swizzle, abs, negate, and at least some parts of the register
          * region description.
          *
          * Rather than trying to enumerate all these cases, *always* expand the
          * operand to a temp GRF for gfx6.
          *
          * For gfx7, keep the operand as-is, except if immediate, which gfx7 still
          * can't use.
          */
         if (shader->devinfo->ver == 6 ||
             (shader->devinfo->ver == 7 && src.file == IMM)) {
            const dst_reg tmp = vgrf(src.type);
            MOV(tmp, src);
            return src_reg(tmp);
         } else {
            return src;
         }
      }

      /**
       * Workaround other weirdness of the math instruction.
       */
      instruction *
      fix_math_instruction(instruction *inst) const
      {
         if (shader->devinfo->ver == 6 &&
             inst->dst.writemask != WRITEMASK_XYZW) {
            const dst_reg tmp = vgrf(inst->dst.type);
            MOV(inst->dst, src_reg(tmp));
            inst->dst = tmp;

         } else if (shader->devinfo->ver < 6) {
            const unsigned sources = (inst->src[1].file == BAD_FILE ? 1 : 2);
            inst->base_mrf = 1;
            inst->mlen = sources;
         }

         return inst;
      }

      bblock_t *block;
      exec_node *cursor;

      unsigned _dispatch_width;
      unsigned _group;
      bool force_writemask_all;

      /** Debug annotation info. */
      struct {
         const char *str;
         const void *ir;
      } annotation;
   };
}

#endif
