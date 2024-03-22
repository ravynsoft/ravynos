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

#ifndef BRW_FS_BUILDER_H
#define BRW_FS_BUILDER_H

#include "brw_ir_fs.h"
#include "brw_shader.h"
#include "brw_eu.h"
#include "brw_fs.h"

namespace brw {
   /**
    * Toolbox to assemble an FS IR program out of individual instructions.
    *
    * This object is meant to have an interface consistent with
    * brw::vec4_builder.  They cannot be fully interchangeable because
    * brw::fs_builder generates scalar code while brw::vec4_builder generates
    * vector code.
    */
   class fs_builder {
   public:
      /** Type used in this IR to represent a source of an instruction. */
      typedef fs_reg src_reg;

      /** Type used in this IR to represent the destination of an instruction. */
      typedef fs_reg dst_reg;

      /** Type used in this IR to represent an instruction. */
      typedef fs_inst instruction;

      /**
       * Construct an fs_builder that inserts instructions into \p shader.
       * \p dispatch_width gives the native execution width of the program.
       */
      fs_builder(fs_visitor *shader,
                 unsigned dispatch_width) :
         shader(shader), block(NULL), cursor(NULL),
         _dispatch_width(dispatch_width),
         _group(0),
         force_writemask_all(false),
         annotation()
      {
      }

      explicit fs_builder(fs_visitor *s) : fs_builder(s, s->dispatch_width) {}

      /**
       * Construct an fs_builder that inserts instructions into \p shader
       * before instruction \p inst in basic block \p block.  The default
       * execution controls and debug annotation are initialized from the
       * instruction passed as argument.
       */
      fs_builder(fs_visitor *shader, bblock_t *block, fs_inst *inst) :
         shader(shader), block(block), cursor(inst),
         _dispatch_width(inst->exec_size),
         _group(inst->group),
         force_writemask_all(inst->force_writemask_all)
      {
         annotation.str = inst->annotation;
         annotation.ir = inst->ir;
      }

      /**
       * Construct an fs_builder that inserts instructions before \p cursor in
       * basic block \p block, inheriting other code generation parameters
       * from this.
       */
      fs_builder
      at(bblock_t *block, exec_node *cursor) const
      {
         fs_builder bld = *this;
         bld.block = block;
         bld.cursor = cursor;
         return bld;
      }

      /**
       * Construct an fs_builder appending instructions at the end of the
       * instruction list of the shader, inheriting other code generation
       * parameters from this.
       */
      fs_builder
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
      fs_builder
      group(unsigned n, unsigned i) const
      {
         fs_builder bld = *this;

         if (n <= dispatch_width() && i < dispatch_width() / n) {
            bld._group += i * n;
         } else {
            /* The requested channel group isn't a subset of the channel group
             * of this builder, which means that the resulting instructions
             * would use (potentially undefined) channel enable signals not
             * specified by the parent builder.  That's only valid if the
             * instruction doesn't have per-channel semantics, in which case
             * we should clear off the default group index in order to prevent
             * emitting instructions with channel group not aligned to their
             * own execution size.
             */
            assert(force_writemask_all);
            bld._group = 0;
         }

         bld._dispatch_width = n;
         return bld;
      }

      /**
       * Alias for group() with width equal to eight.
       */
      fs_builder
      quarter(unsigned i) const
      {
         return group(8, i);
      }

      /**
       * Construct a builder with per-channel control flow execution masking
       * disabled if \p b is true.  If control flow execution masking is
       * already disabled this has no effect.
       */
      fs_builder
      exec_all(bool b = true) const
      {
         fs_builder bld = *this;
         if (b)
            bld.force_writemask_all = true;
         return bld;
      }

      /**
       * Construct a builder with the given debug annotation info.
       */
      fs_builder
      annotate(const char *str, const void *ir = NULL) const
      {
         fs_builder bld = *this;
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
       * Allocate a virtual register of natural vector size (one for this IR)
       * and SIMD width.  \p n gives the amount of space to allocate in
       * dispatch_width units (which is just enough space for one logical
       * component in this IR).
       */
      dst_reg
      vgrf(enum brw_reg_type type, unsigned n = 1) const
      {
         const unsigned unit = reg_unit(shader->devinfo);
         assert(dispatch_width() <= 32);

         if (n > 0)
            return dst_reg(VGRF, shader->alloc.allocate(
                              DIV_ROUND_UP(n * type_sz(type) * dispatch_width(),
                                           unit * REG_SIZE) * unit),
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
         return dst_reg(retype(brw_null_reg(), BRW_REGISTER_TYPE_F));
      }

      dst_reg
      null_reg_df() const
      {
         return dst_reg(retype(brw_null_reg(), BRW_REGISTER_TYPE_DF));
      }

      /**
       * Create a null register of signed integer type.
       */
      dst_reg
      null_reg_d() const
      {
         return dst_reg(retype(brw_null_reg(), BRW_REGISTER_TYPE_D));
      }

      /**
       * Create a null register of unsigned integer type.
       */
      dst_reg
      null_reg_ud() const
      {
         return dst_reg(retype(brw_null_reg(), BRW_REGISTER_TYPE_UD));
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
         return emit(instruction(opcode, dispatch_width()));
      }

      /**
       * Create and insert a nullary instruction into the program.
       */
      instruction *
      emit(enum opcode opcode, const dst_reg &dst) const
      {
         return emit(instruction(opcode, dispatch_width(), dst));
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
            return emit(instruction(opcode, dispatch_width(), dst,
                                    fix_math_operand(src0)));

         default:
            return emit(instruction(opcode, dispatch_width(), dst, src0));
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
            return emit(instruction(opcode, dispatch_width(), dst,
                                    fix_math_operand(src0),
                                    fix_math_operand(src1)));

         default:
            return emit(instruction(opcode, dispatch_width(), dst,
                                    src0, src1));

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
            return emit(instruction(opcode, dispatch_width(), dst,
                                    fix_3src_operand(src0),
                                    fix_3src_operand(src1),
                                    fix_3src_operand(src2)));

         default:
            return emit(instruction(opcode, dispatch_width(), dst,
                                    src0, src1, src2));
         }
      }

      /**
       * Create and insert an instruction with a variable number of sources
       * into the program.
       */
      instruction *
      emit(enum opcode opcode, const dst_reg &dst, const src_reg srcs[],
           unsigned n) const
      {
         /* Use the emit() methods for specific operand counts to ensure that
          * opcode-specific operand fixups occur.
          */
         if (n == 2) {
            return emit(opcode, dst, srcs[0], srcs[1]);
         } else if (n == 3) {
            return emit(opcode, dst, srcs[0], srcs[1], srcs[2]);
         } else {
            return emit(instruction(opcode, dispatch_width(), dst, srcs, n));
         }
      }

      /**
       * Insert a preallocated instruction into the program.
       */
      instruction *
      emit(instruction *inst) const
      {
         assert(inst->exec_size <= 32);
         assert(inst->exec_size == dispatch_width() ||
                force_writemask_all);

         inst->group = _group;
         inst->force_writemask_all = force_writemask_all;
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

         /* In some cases we can't have bytes as operand for src1, so use the
          * same type for both operand.
          */
         return set_condmod(mod, SEL(dst, fix_unsigned_negate(src0),
                                     fix_unsigned_negate(src1)));
      }

      /**
       * Copy any live channel from \p src to the first channel of the result.
       */
      src_reg
      emit_uniformize(const src_reg &src) const
      {
         /* FIXME: We use a vector chan_index and dst to allow constant and
          * copy propagration to move result all the way into the consuming
          * instruction (typically a surface index or sampler index for a
          * send). This uses 1 or 3 extra hw registers in 16 or 32 wide
          * dispatch. Once we teach const/copy propagation about scalars we
          * should go back to scalar destinations here.
          */
         const fs_builder ubld = exec_all();
         const dst_reg chan_index = vgrf(BRW_REGISTER_TYPE_UD);
         const dst_reg dst = vgrf(src.type);

         ubld.emit(SHADER_OPCODE_FIND_LIVE_CHANNEL, chan_index);
         ubld.emit(SHADER_OPCODE_BROADCAST, dst, src, component(chan_index, 0));

         return src_reg(component(dst, 0));
      }

      src_reg
      move_to_vgrf(const src_reg &src, unsigned num_components) const
      {
         src_reg *const src_comps = new src_reg[num_components];
         for (unsigned i = 0; i < num_components; i++)
            src_comps[i] = offset(src, dispatch_width(), i);

         const dst_reg dst = vgrf(src.type, num_components);
         LOAD_PAYLOAD(dst, src_comps, num_components, 0);

         delete[] src_comps;

         return src_reg(dst);
      }

      void
      emit_scan_step(enum opcode opcode, brw_conditional_mod mod,
                     const dst_reg &tmp,
                     unsigned left_offset, unsigned left_stride,
                     unsigned right_offset, unsigned right_stride) const
      {
         dst_reg left, right;
         left = horiz_stride(horiz_offset(tmp, left_offset), left_stride);
         right = horiz_stride(horiz_offset(tmp, right_offset), right_stride);
         if ((tmp.type == BRW_REGISTER_TYPE_Q ||
              tmp.type == BRW_REGISTER_TYPE_UQ) &&
             !shader->devinfo->has_64bit_int) {
            switch (opcode) {
            case BRW_OPCODE_MUL:
               /* This will get lowered by integer MUL lowering */
               set_condmod(mod, emit(opcode, right, left, right));
               break;

            case BRW_OPCODE_SEL: {
               /* In order for the comparisons to work out right, we need our
                * comparisons to be strict.
                */
               assert(mod == BRW_CONDITIONAL_L || mod == BRW_CONDITIONAL_GE);
               if (mod == BRW_CONDITIONAL_GE)
                  mod = BRW_CONDITIONAL_G;

               /* We treat the bottom 32 bits as unsigned regardless of
                * whether or not the integer as a whole is signed.
                */
               dst_reg right_low = subscript(right, BRW_REGISTER_TYPE_UD, 0);
               dst_reg left_low = subscript(left, BRW_REGISTER_TYPE_UD, 0);

               /* The upper bits get the same sign as the 64-bit type */
               brw_reg_type type32 = brw_reg_type_from_bit_size(32, tmp.type);
               dst_reg right_high = subscript(right, type32, 1);
               dst_reg left_high = subscript(left, type32, 1);

               /* Build up our comparison:
                *
                *   l_hi < r_hi || (l_hi == r_hi && l_low < r_low)
                */
               CMP(null_reg_ud(), retype(left_low, BRW_REGISTER_TYPE_UD),
                                  retype(right_low, BRW_REGISTER_TYPE_UD), mod);
               set_predicate(BRW_PREDICATE_NORMAL,
                             CMP(null_reg_ud(), left_high, right_high,
                                 BRW_CONDITIONAL_EQ));
               set_predicate_inv(BRW_PREDICATE_NORMAL, true,
                                 CMP(null_reg_ud(), left_high, right_high, mod));

               /* We could use selects here or we could use predicated MOVs
                * because the destination and second source (if it were a SEL)
                * are the same.
                */
               set_predicate(BRW_PREDICATE_NORMAL, MOV(right_low, left_low));
               set_predicate(BRW_PREDICATE_NORMAL, MOV(right_high, left_high));
               break;
            }

            default:
               unreachable("Unsupported 64-bit scan op");
            }
         } else {
            set_condmod(mod, emit(opcode, right, left, right));
         }
      }

      void
      emit_scan(enum opcode opcode, const dst_reg &tmp,
                unsigned cluster_size, brw_conditional_mod mod) const
      {
         assert(dispatch_width() >= 8);

         /* The instruction splitting code isn't advanced enough to split
          * these so we need to handle that ourselves.
          */
         if (dispatch_width() * type_sz(tmp.type) > 2 * REG_SIZE) {
            const unsigned half_width = dispatch_width() / 2;
            const fs_builder ubld = exec_all().group(half_width, 0);
            dst_reg left = tmp;
            dst_reg right = horiz_offset(tmp, half_width);
            ubld.emit_scan(opcode, left, cluster_size, mod);
            ubld.emit_scan(opcode, right, cluster_size, mod);
            if (cluster_size > half_width) {
               ubld.emit_scan_step(opcode, mod, tmp,
                                   half_width - 1, 0, half_width, 1);
            }
            return;
         }

         if (cluster_size > 1) {
            const fs_builder ubld = exec_all().group(dispatch_width() / 2, 0);
            ubld.emit_scan_step(opcode, mod, tmp, 0, 2, 1, 2);
         }

         if (cluster_size > 2) {
            if (type_sz(tmp.type) <= 4) {
               const fs_builder ubld =
                  exec_all().group(dispatch_width() / 4, 0);
               ubld.emit_scan_step(opcode, mod, tmp, 1, 4, 2, 4);
               ubld.emit_scan_step(opcode, mod, tmp, 1, 4, 3, 4);
            } else {
               /* For 64-bit types, we have to do things differently because
                * the code above would land us with destination strides that
                * the hardware can't handle.  Fortunately, we'll only be
                * 8-wide in that case and it's the same number of
                * instructions.
                */
               const fs_builder ubld = exec_all().group(2, 0);
               for (unsigned i = 0; i < dispatch_width(); i += 4)
                  ubld.emit_scan_step(opcode, mod, tmp, i + 1, 0, i + 2, 1);
            }
         }

         for (unsigned i = 4;
              i < MIN2(cluster_size, dispatch_width());
              i *= 2) {
            const fs_builder ubld = exec_all().group(i, 0);
            ubld.emit_scan_step(opcode, mod, tmp, i - 1, 0, i, 1);

            if (dispatch_width() > i * 2)
               ubld.emit_scan_step(opcode, mod, tmp, i * 3 - 1, 0, i * 3, 1);

            if (dispatch_width() > i * 4) {
               ubld.emit_scan_step(opcode, mod, tmp, i * 5 - 1, 0, i * 5, 1);
               ubld.emit_scan_step(opcode, mod, tmp, i * 7 - 1, 0, i * 7, 1);
            }
         }
      }

      instruction *
      emit_undef_for_dst(const instruction *old_inst) const
      {
         assert(old_inst->dst.file == VGRF);
         instruction *inst = emit(SHADER_OPCODE_UNDEF,
                                  retype(old_inst->dst, BRW_REGISTER_TYPE_UD));
         inst->size_written = old_inst->size_written;

         return inst;
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
      ALU3(ADD3)
      ALU2_ACC(ADDC)
      ALU2(AND)
      ALU2(ASR)
      ALU2(AVG)
      ALU3(BFE)
      ALU2(BFI1)
      ALU3(BFI2)
      ALU1(BFREV)
      ALU1(CBIT)
      ALU1(DIM)
      ALU2(DP2)
      ALU2(DP3)
      ALU2(DP4)
      ALU2(DPH)
      ALU1(FBH)
      ALU1(FBL)
      ALU1(FRC)
      ALU3(DP4A)
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
      ALU2(ROL)
      ALU2(ROR)
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

      instruction *
      F32TO16(const dst_reg &dst, const src_reg &src) const
      {
         assert(dst.type == BRW_REGISTER_TYPE_HF);
         assert(src.type == BRW_REGISTER_TYPE_F);

         if (shader->devinfo->ver >= 8) {
            return MOV(dst, src);
         } else {
            assert(shader->devinfo->ver == 7);
            return emit(BRW_OPCODE_F32TO16,
                        retype(dst, BRW_REGISTER_TYPE_W), src);
         }
      }

      instruction *
      F16TO32(const dst_reg &dst, const src_reg &src) const
      {
         assert(dst.type == BRW_REGISTER_TYPE_F);
         assert(src.type == BRW_REGISTER_TYPE_HF);

         if (shader->devinfo->ver >= 8) {
            return MOV(dst, src);
         } else {
            assert(shader->devinfo->ver == 7);
            return emit(BRW_OPCODE_F16TO32,
                        dst, retype(src, BRW_REGISTER_TYPE_W));
         }
      }
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
       * CSEL: dst = src2 <op> 0.0f ? src0 : src1
       */
      instruction *
      CSEL(const dst_reg &dst, const src_reg &src0, const src_reg &src1,
           const src_reg &src2, brw_conditional_mod condition) const
      {
         /* CSEL only operates on floats, so we can't do integer </<=/>=/>
          * comparisons.  Zero/non-zero (== and !=) comparisons almost work.
          * 0x80000000 fails because it is -0.0, and -0.0 == 0.0.
          */
         assert(src2.type == BRW_REGISTER_TYPE_F);

         return set_condmod(condition,
                            emit(BRW_OPCODE_CSEL,
                                 retype(dst, BRW_REGISTER_TYPE_F),
                                 retype(src0, BRW_REGISTER_TYPE_F),
                                 retype(src1, BRW_REGISTER_TYPE_F),
                                 src2));
      }

      /**
       * Emit a linear interpolation instruction.
       */
      instruction *
      LRP(const dst_reg &dst, const src_reg &x, const src_reg &y,
          const src_reg &a) const
      {
         if (shader->devinfo->ver >= 6 && shader->devinfo->ver <= 10) {
            /* The LRP instruction actually does op1 * op0 + op2 * (1 - op0), so
             * we need to reorder the operands.
             */
            return emit(BRW_OPCODE_LRP, dst, a, y, x);

         } else {
            /* We can't use the LRP instruction.  Emit x*(1-a) + y*a. */
            const dst_reg y_times_a = vgrf(dst.type);
            const dst_reg one_minus_a = vgrf(dst.type);
            const dst_reg x_times_one_minus_a = vgrf(dst.type);

            MUL(y_times_a, y, a);
            ADD(one_minus_a, negate(a), brw_imm_f(1.0f));
            MUL(x_times_one_minus_a, x, src_reg(one_minus_a));
            return ADD(dst, src_reg(x_times_one_minus_a), src_reg(y_times_a));
         }
      }

      /**
       * Collect a number of registers in a contiguous range of registers.
       */
      instruction *
      LOAD_PAYLOAD(const dst_reg &dst, const src_reg *src,
                   unsigned sources, unsigned header_size) const
      {
         instruction *inst = emit(SHADER_OPCODE_LOAD_PAYLOAD, dst, src, sources);
         inst->header_size = header_size;
         inst->size_written = header_size * REG_SIZE;
         for (unsigned i = header_size; i < sources; i++) {
            inst->size_written += dispatch_width() * type_sz(src[i].type) *
                                  dst.stride;
         }

         return inst;
      }

      instruction *
      UNDEF(const dst_reg &dst) const
      {
         assert(dst.file == VGRF);
         assert(dst.offset % REG_SIZE == 0);
         instruction *inst = emit(SHADER_OPCODE_UNDEF,
                                  retype(dst, BRW_REGISTER_TYPE_UD));
         inst->size_written = shader->alloc.sizes[dst.nr] * REG_SIZE - dst.offset;

         return inst;
      }

      instruction *
      DPAS(const dst_reg &dst, const src_reg &src0, const src_reg &src1, const src_reg &src2,
           unsigned sdepth, unsigned rcount) const
      {
         assert(_dispatch_width == 8);
         assert(sdepth == 8);
         assert(rcount == 1 || rcount == 2 || rcount == 4 || rcount == 8);

         instruction *inst = emit(BRW_OPCODE_DPAS, dst, src0, src1, src2);
         inst->sdepth = sdepth;
         inst->rcount = rcount;

         if (dst.type == BRW_REGISTER_TYPE_HF) {
            inst->size_written = rcount * REG_SIZE / 2;
         } else {
            inst->size_written = rcount * REG_SIZE;
         }

         return inst;
      }

      fs_visitor *shader;

      fs_inst *BREAK()    { return emit(BRW_OPCODE_BREAK); }
      fs_inst *DO()       { return emit(BRW_OPCODE_DO); }
      fs_inst *ENDIF()    { return emit(BRW_OPCODE_ENDIF); }
      fs_inst *NOP()      { return emit(BRW_OPCODE_NOP); }
      fs_inst *WHILE()    { return emit(BRW_OPCODE_WHILE); }
      fs_inst *CONTINUE() { return emit(BRW_OPCODE_CONTINUE); }

   private:
      /**
       * Workaround for negation of UD registers.  See comment in
       * fs_generator::generate_code() for more details.
       */
      src_reg
      fix_unsigned_negate(const src_reg &src) const
      {
         if (src.type == BRW_REGISTER_TYPE_UD &&
             src.negate) {
            dst_reg temp = vgrf(BRW_REGISTER_TYPE_UD);
            MOV(temp, src);
            return src_reg(temp);
         } else {
            return src;
         }
      }

      /**
       * Workaround for source register modes not supported by the ternary
       * instruction encoding.
       */
      src_reg
      fix_3src_operand(const src_reg &src) const
      {
         switch (src.file) {
         case FIXED_GRF:
            /* FINISHME: Could handle scalar region, other stride=1 regions */
            if (src.vstride != BRW_VERTICAL_STRIDE_8 ||
                src.width != BRW_WIDTH_8 ||
                src.hstride != BRW_HORIZONTAL_STRIDE_1)
               break;
            FALLTHROUGH;
         case ATTR:
         case VGRF:
         case UNIFORM:
         case IMM:
            return src;
         default:
            break;
         }

         dst_reg expanded = vgrf(src.type);
         MOV(expanded, src);
         return expanded;
      }

      /**
       * Workaround for source register modes not supported by the math
       * instruction.
       */
      src_reg
      fix_math_operand(const src_reg &src) const
      {
         /* Can't do hstride == 0 args on gfx6 math, so expand it out. We
          * might be able to do better by doing execsize = 1 math and then
          * expanding that result out, but we would need to be careful with
          * masking.
          *
          * Gfx6 hardware ignores source modifiers (negate and abs) on math
          * instructions, so we also move to a temp to set those up.
          *
          * Gfx7 relaxes most of the above restrictions, but still can't use IMM
          * operands to math
          */
         if ((shader->devinfo->ver == 6 &&
              (src.file == IMM || src.file == UNIFORM ||
               src.abs || src.negate)) ||
             (shader->devinfo->ver == 7 && src.file == IMM)) {
            const dst_reg tmp = vgrf(src.type);
            MOV(tmp, src);
            return tmp;
         } else {
            return src;
         }
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

static inline fs_reg
offset(const fs_reg &reg, const brw::fs_builder &bld, unsigned delta)
{
   return offset(reg, bld.dispatch_width(), delta);
}

#endif
