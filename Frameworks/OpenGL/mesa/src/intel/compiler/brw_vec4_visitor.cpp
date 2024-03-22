/*
 * Copyright © 2011 Intel Corporation
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

#include "brw_vec4.h"
#include "brw_cfg.h"
#include "brw_eu.h"
#include "util/u_math.h"

namespace brw {

vec4_instruction::vec4_instruction(enum opcode opcode, const dst_reg &dst,
                                   const src_reg &src0, const src_reg &src1,
                                   const src_reg &src2)
{
   this->opcode = opcode;
   this->dst = dst;
   this->src[0] = src0;
   this->src[1] = src1;
   this->src[2] = src2;
   this->saturate = false;
   this->force_writemask_all = false;
   this->no_dd_clear = false;
   this->no_dd_check = false;
   this->writes_accumulator = false;
   this->conditional_mod = BRW_CONDITIONAL_NONE;
   this->predicate = BRW_PREDICATE_NONE;
   this->predicate_inverse = false;
   this->target = 0;
   this->shadow_compare = false;
   this->eot = false;
   this->ir = NULL;
   this->urb_write_flags = BRW_URB_WRITE_NO_FLAGS;
   this->header_size = 0;
   this->flag_subreg = 0;
   this->mlen = 0;
   this->base_mrf = 0;
   this->offset = 0;
   this->exec_size = 8;
   this->group = 0;
   this->size_written = (dst.file == BAD_FILE ?
                         0 : this->exec_size * type_sz(dst.type));
   this->annotation = NULL;
}

vec4_instruction *
vec4_visitor::emit(vec4_instruction *inst)
{
   inst->ir = this->base_ir;
   inst->annotation = this->current_annotation;

   this->instructions.push_tail(inst);

   return inst;
}

vec4_instruction *
vec4_visitor::emit_before(bblock_t *block, vec4_instruction *inst,
                          vec4_instruction *new_inst)
{
   new_inst->ir = inst->ir;
   new_inst->annotation = inst->annotation;

   inst->insert_before(block, new_inst);

   return inst;
}

vec4_instruction *
vec4_visitor::emit(enum opcode opcode, const dst_reg &dst, const src_reg &src0,
                   const src_reg &src1, const src_reg &src2)
{
   return emit(new(mem_ctx) vec4_instruction(opcode, dst, src0, src1, src2));
}


vec4_instruction *
vec4_visitor::emit(enum opcode opcode, const dst_reg &dst, const src_reg &src0,
                   const src_reg &src1)
{
   return emit(new(mem_ctx) vec4_instruction(opcode, dst, src0, src1));
}

vec4_instruction *
vec4_visitor::emit(enum opcode opcode, const dst_reg &dst, const src_reg &src0)
{
   return emit(new(mem_ctx) vec4_instruction(opcode, dst, src0));
}

vec4_instruction *
vec4_visitor::emit(enum opcode opcode, const dst_reg &dst)
{
   return emit(new(mem_ctx) vec4_instruction(opcode, dst));
}

vec4_instruction *
vec4_visitor::emit(enum opcode opcode)
{
   return emit(new(mem_ctx) vec4_instruction(opcode, dst_reg()));
}

#define ALU1(op)							\
   vec4_instruction *							\
   vec4_visitor::op(const dst_reg &dst, const src_reg &src0)		\
   {									\
      return new(mem_ctx) vec4_instruction(BRW_OPCODE_##op, dst, src0); \
   }

#define ALU2(op)							\
   vec4_instruction *							\
   vec4_visitor::op(const dst_reg &dst, const src_reg &src0,		\
                    const src_reg &src1)				\
   {									\
      return new(mem_ctx) vec4_instruction(BRW_OPCODE_##op, dst,        \
                                           src0, src1);                 \
   }

#define ALU2_ACC(op)							\
   vec4_instruction *							\
   vec4_visitor::op(const dst_reg &dst, const src_reg &src0,		\
                    const src_reg &src1)				\
   {									\
      vec4_instruction *inst = new(mem_ctx) vec4_instruction(           \
                       BRW_OPCODE_##op, dst, src0, src1);		\
      inst->writes_accumulator = true;                                  \
      return inst;                                                      \
   }

#define ALU3(op)							\
   vec4_instruction *							\
   vec4_visitor::op(const dst_reg &dst, const src_reg &src0,		\
                    const src_reg &src1, const src_reg &src2)		\
   {									\
      assert(devinfo->ver >= 6);						\
      return new(mem_ctx) vec4_instruction(BRW_OPCODE_##op, dst,	\
					   src0, src1, src2);		\
   }

ALU1(NOT)
ALU1(MOV)
ALU1(FRC)
ALU1(RNDD)
ALU1(RNDE)
ALU1(RNDZ)
ALU1(F32TO16)
ALU1(F16TO32)
ALU2(ADD)
ALU2(MUL)
ALU2_ACC(MACH)
ALU2(AND)
ALU2(OR)
ALU2(XOR)
ALU2(DP3)
ALU2(DP4)
ALU2(DPH)
ALU2(SHL)
ALU2(SHR)
ALU2(ASR)
ALU3(LRP)
ALU1(BFREV)
ALU3(BFE)
ALU2(BFI1)
ALU3(BFI2)
ALU1(FBH)
ALU1(FBL)
ALU1(CBIT)
ALU1(LZD)
ALU3(MAD)
ALU2_ACC(ADDC)
ALU2_ACC(SUBB)
ALU2(MAC)
ALU1(DIM)

/** Gfx4 predicated IF. */
vec4_instruction *
vec4_visitor::IF(enum brw_predicate predicate)
{
   vec4_instruction *inst;

   inst = new(mem_ctx) vec4_instruction(BRW_OPCODE_IF);
   inst->predicate = predicate;

   return inst;
}

/** Gfx6 IF with embedded comparison. */
vec4_instruction *
vec4_visitor::IF(src_reg src0, src_reg src1,
                 enum brw_conditional_mod condition)
{
   assert(devinfo->ver == 6);

   vec4_instruction *inst;

   resolve_ud_negate(&src0);
   resolve_ud_negate(&src1);

   inst = new(mem_ctx) vec4_instruction(BRW_OPCODE_IF, dst_null_d(),
					src0, src1);
   inst->conditional_mod = condition;

   return inst;
}

/**
 * CMP: Sets the low bit of the destination channels with the result
 * of the comparison, while the upper bits are undefined, and updates
 * the flag register with the packed 16 bits of the result.
 */
vec4_instruction *
vec4_visitor::CMP(dst_reg dst, src_reg src0, src_reg src1,
                  enum brw_conditional_mod condition)
{
   vec4_instruction *inst;

   /* Take the instruction:
    *
    * CMP null<d> src0<f> src1<f>
    *
    * Original gfx4 does type conversion to the destination type before
    * comparison, producing garbage results for floating point comparisons.
    *
    * The destination type doesn't matter on newer generations, so we set the
    * type to match src0 so we can compact the instruction.
    */
   dst.type = src0.type;

   resolve_ud_negate(&src0);
   resolve_ud_negate(&src1);

   inst = new(mem_ctx) vec4_instruction(BRW_OPCODE_CMP, dst, src0, src1);
   inst->conditional_mod = condition;

   return inst;
}

vec4_instruction *
vec4_visitor::SCRATCH_READ(const dst_reg &dst, const src_reg &index)
{
   vec4_instruction *inst;

   inst = new(mem_ctx) vec4_instruction(SHADER_OPCODE_GFX4_SCRATCH_READ,
					dst, index);
   inst->base_mrf = FIRST_SPILL_MRF(devinfo->ver) + 1;
   inst->mlen = 2;

   return inst;
}

vec4_instruction *
vec4_visitor::SCRATCH_WRITE(const dst_reg &dst, const src_reg &src,
                            const src_reg &index)
{
   vec4_instruction *inst;

   inst = new(mem_ctx) vec4_instruction(SHADER_OPCODE_GFX4_SCRATCH_WRITE,
					dst, src, index);
   inst->base_mrf = FIRST_SPILL_MRF(devinfo->ver);
   inst->mlen = 3;

   return inst;
}

src_reg
vec4_visitor::fix_3src_operand(const src_reg &src)
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

   dst_reg expanded = dst_reg(this, glsl_vec4_type());
   expanded.type = src.type;
   emit(VEC4_OPCODE_UNPACK_UNIFORM, expanded, src);
   return src_reg(expanded);
}

src_reg
vec4_visitor::fix_math_operand(const src_reg &src)
{
   if (devinfo->ver < 6 || src.file == BAD_FILE)
      return src;

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

   if (devinfo->ver == 7 && src.file != IMM)
      return src;

   dst_reg expanded = dst_reg(this, glsl_vec4_type());
   expanded.type = src.type;
   emit(MOV(expanded, src));
   return src_reg(expanded);
}

vec4_instruction *
vec4_visitor::emit_math(enum opcode opcode,
                        const dst_reg &dst,
                        const src_reg &src0, const src_reg &src1)
{
   vec4_instruction *math =
      emit(opcode, dst, fix_math_operand(src0), fix_math_operand(src1));

   if (devinfo->ver == 6 && dst.writemask != WRITEMASK_XYZW) {
      /* MATH on Gfx6 must be align1, so we can't do writemasks. */
      math->dst = dst_reg(this, glsl_vec4_type());
      math->dst.type = dst.type;
      math = emit(MOV(dst, src_reg(math->dst)));
   } else if (devinfo->ver < 6) {
      math->base_mrf = 1;
      math->mlen = src1.file == BAD_FILE ? 1 : 2;
   }

   return math;
}

void
vec4_visitor::emit_pack_half_2x16(dst_reg dst, src_reg src0)
{
   if (devinfo->ver < 7) {
      unreachable("ir_unop_pack_half_2x16 should be lowered");
   }

   assert(dst.type == BRW_REGISTER_TYPE_UD);
   assert(src0.type == BRW_REGISTER_TYPE_F);

   /* From the Ivybridge PRM, Vol4, Part3, Section 6.27 f32to16:
    *
    *   Because this instruction does not have a 16-bit floating-point type,
    *   the destination data type must be Word (W).
    *
    *   The destination must be DWord-aligned and specify a horizontal stride
    *   (HorzStride) of 2. The 16-bit result is stored in the lower word of
    *   each destination channel and the upper word is not modified.
    *
    * The above restriction implies that the f32to16 instruction must use
    * align1 mode, because only in align1 mode is it possible to specify
    * horizontal stride.  We choose here to defy the hardware docs and emit
    * align16 instructions.
    *
    * (I [chadv] did attempt to emit align1 instructions for VS f32to16
    * instructions. I was partially successful in that the code passed all
    * tests.  However, the code was dubiously correct and fragile, and the
    * tests were not harsh enough to probe that frailty. Not trusting the
    * code, I chose instead to remain in align16 mode in defiance of the hw
    * docs).
    *
    * I've [chadv] experimentally confirmed that, on gfx7 hardware and the
    * simulator, emitting a f32to16 in align16 mode with UD as destination
    * data type is safe. The behavior differs from that specified in the PRM
    * in that the upper word of each destination channel is cleared to 0.
    */

   dst_reg tmp_dst(this, glsl_uvec2_type());
   src_reg tmp_src(tmp_dst);

#if 0
   /* Verify the undocumented behavior on which the following instructions
    * rely.  If f32to16 fails to clear the upper word of the X and Y channels,
    * then the result of the bit-or instruction below will be incorrect.
    *
    * You should inspect the disasm output in order to verify that the MOV is
    * not optimized away.
    */
   emit(MOV(tmp_dst, brw_imm_ud(0x12345678u)));
#endif

   /* Give tmp the form below, where "." means untouched.
    *
    *     w z          y          x w z          y          x
    *   |.|.|0x0000hhhh|0x0000llll|.|.|0x0000hhhh|0x0000llll|
    *
    * That the upper word of each write-channel be 0 is required for the
    * following bit-shift and bit-or instructions to work. Note that this
    * relies on the undocumented hardware behavior mentioned above.
    */
   tmp_dst.writemask = WRITEMASK_XY;
   emit(F32TO16(tmp_dst, src0));

   /* Give the write-channels of dst the form:
    *   0xhhhh0000
    */
   tmp_src.swizzle = BRW_SWIZZLE_YYYY;
   emit(SHL(dst, tmp_src, brw_imm_ud(16u)));

   /* Finally, give the write-channels of dst the form of packHalf2x16's
    * output:
    *   0xhhhhllll
    */
   tmp_src.swizzle = BRW_SWIZZLE_XXXX;
   emit(OR(dst, src_reg(dst), tmp_src));
}

void
vec4_visitor::emit_unpack_half_2x16(dst_reg dst, src_reg src0)
{
   if (devinfo->ver < 7) {
      unreachable("ir_unop_unpack_half_2x16 should be lowered");
   }

   assert(dst.type == BRW_REGISTER_TYPE_F);
   assert(src0.type == BRW_REGISTER_TYPE_UD);

   /* From the Ivybridge PRM, Vol4, Part3, Section 6.26 f16to32:
    *
    *   Because this instruction does not have a 16-bit floating-point type,
    *   the source data type must be Word (W). The destination type must be
    *   F (Float).
    *
    * To use W as the source data type, we must adjust horizontal strides,
    * which is only possible in align1 mode. All my [chadv] attempts at
    * emitting align1 instructions for unpackHalf2x16 failed to pass the
    * Piglit tests, so I gave up.
    *
    * I've verified that, on gfx7 hardware and the simulator, it is safe to
    * emit f16to32 in align16 mode with UD as source data type.
    */

   dst_reg tmp_dst(this, glsl_uvec2_type());
   src_reg tmp_src(tmp_dst);

   tmp_dst.writemask = WRITEMASK_X;
   emit(AND(tmp_dst, src0, brw_imm_ud(0xffffu)));

   tmp_dst.writemask = WRITEMASK_Y;
   emit(SHR(tmp_dst, src0, brw_imm_ud(16u)));

   dst.writemask = WRITEMASK_XY;
   emit(F16TO32(dst, tmp_src));
}

void
vec4_visitor::emit_unpack_unorm_4x8(const dst_reg &dst, src_reg src0)
{
   /* Instead of splitting the 32-bit integer, shifting, and ORing it back
    * together, we can shift it by <0, 8, 16, 24>. The packed integer immediate
    * is not suitable to generate the shift values, but we can use the packed
    * vector float and a type-converting MOV.
    */
   dst_reg shift(this, glsl_uvec4_type());
   emit(MOV(shift, brw_imm_vf4(0x00, 0x60, 0x70, 0x78)));

   dst_reg shifted(this, glsl_uvec4_type());
   src0.swizzle = BRW_SWIZZLE_XXXX;
   emit(SHR(shifted, src0, src_reg(shift)));

   shifted.type = BRW_REGISTER_TYPE_UB;
   dst_reg f(this, glsl_vec4_type());
   emit(VEC4_OPCODE_MOV_BYTES, f, src_reg(shifted));

   emit(MUL(dst, src_reg(f), brw_imm_f(1.0f / 255.0f)));
}

void
vec4_visitor::emit_unpack_snorm_4x8(const dst_reg &dst, src_reg src0)
{
   /* Instead of splitting the 32-bit integer, shifting, and ORing it back
    * together, we can shift it by <0, 8, 16, 24>. The packed integer immediate
    * is not suitable to generate the shift values, but we can use the packed
    * vector float and a type-converting MOV.
    */
   dst_reg shift(this, glsl_uvec4_type());
   emit(MOV(shift, brw_imm_vf4(0x00, 0x60, 0x70, 0x78)));

   dst_reg shifted(this, glsl_uvec4_type());
   src0.swizzle = BRW_SWIZZLE_XXXX;
   emit(SHR(shifted, src0, src_reg(shift)));

   shifted.type = BRW_REGISTER_TYPE_B;
   dst_reg f(this, glsl_vec4_type());
   emit(VEC4_OPCODE_MOV_BYTES, f, src_reg(shifted));

   dst_reg scaled(this, glsl_vec4_type());
   emit(MUL(scaled, src_reg(f), brw_imm_f(1.0f / 127.0f)));

   dst_reg max(this, glsl_vec4_type());
   emit_minmax(BRW_CONDITIONAL_GE, max, src_reg(scaled), brw_imm_f(-1.0f));
   emit_minmax(BRW_CONDITIONAL_L, dst, src_reg(max), brw_imm_f(1.0f));
}

void
vec4_visitor::emit_pack_unorm_4x8(const dst_reg &dst, const src_reg &src0)
{
   dst_reg saturated(this, glsl_vec4_type());
   vec4_instruction *inst = emit(MOV(saturated, src0));
   inst->saturate = true;

   dst_reg scaled(this, glsl_vec4_type());
   emit(MUL(scaled, src_reg(saturated), brw_imm_f(255.0f)));

   dst_reg rounded(this, glsl_vec4_type());
   emit(RNDE(rounded, src_reg(scaled)));

   dst_reg u(this, glsl_uvec4_type());
   emit(MOV(u, src_reg(rounded)));

   src_reg bytes(u);
   emit(VEC4_OPCODE_PACK_BYTES, dst, bytes);
}

void
vec4_visitor::emit_pack_snorm_4x8(const dst_reg &dst, const src_reg &src0)
{
   dst_reg max(this, glsl_vec4_type());
   emit_minmax(BRW_CONDITIONAL_GE, max, src0, brw_imm_f(-1.0f));

   dst_reg min(this, glsl_vec4_type());
   emit_minmax(BRW_CONDITIONAL_L, min, src_reg(max), brw_imm_f(1.0f));

   dst_reg scaled(this, glsl_vec4_type());
   emit(MUL(scaled, src_reg(min), brw_imm_f(127.0f)));

   dst_reg rounded(this, glsl_vec4_type());
   emit(RNDE(rounded, src_reg(scaled)));

   dst_reg i(this, glsl_ivec4_type());
   emit(MOV(i, src_reg(rounded)));

   src_reg bytes(i);
   emit(VEC4_OPCODE_PACK_BYTES, dst, bytes);
}

/*
 * Returns the minimum number of vec4 (as_vec4 == true) or dvec4 (as_vec4 ==
 * false) elements needed to pack a type.
 */
static int
type_size_xvec4(const struct glsl_type *type, bool as_vec4, bool bindless)
{
   unsigned int i;
   int size;

   switch (type->base_type) {
   case GLSL_TYPE_UINT:
   case GLSL_TYPE_INT:
   case GLSL_TYPE_FLOAT:
   case GLSL_TYPE_FLOAT16:
   case GLSL_TYPE_BOOL:
   case GLSL_TYPE_DOUBLE:
   case GLSL_TYPE_UINT16:
   case GLSL_TYPE_INT16:
   case GLSL_TYPE_UINT8:
   case GLSL_TYPE_INT8:
   case GLSL_TYPE_UINT64:
   case GLSL_TYPE_INT64:
      if (glsl_type_is_matrix(type)) {
         const glsl_type *col_type = glsl_get_column_type(type);
         unsigned col_slots =
            (as_vec4 && glsl_type_is_dual_slot(col_type)) ? 2 : 1;
         return type->matrix_columns * col_slots;
      } else {
         /* Regardless of size of vector, it gets a vec4. This is bad
          * packing for things like floats, but otherwise arrays become a
          * mess.  Hopefully a later pass over the code can pack scalars
          * down if appropriate.
          */
         return (as_vec4 && glsl_type_is_dual_slot(type)) ? 2 : 1;
      }
   case GLSL_TYPE_ARRAY:
      assert(type->length > 0);
      return type_size_xvec4(type->fields.array, as_vec4, bindless) *
             type->length;
   case GLSL_TYPE_STRUCT:
   case GLSL_TYPE_INTERFACE:
      size = 0;
      for (i = 0; i < type->length; i++) {
	 size += type_size_xvec4(type->fields.structure[i].type, as_vec4,
                                 bindless);
      }
      return size;
   case GLSL_TYPE_SUBROUTINE:
      return 1;

   case GLSL_TYPE_SAMPLER:
   case GLSL_TYPE_TEXTURE:
      /* Samplers and textures take up no register space, since they're baked
       * in at link time.
       */
      return bindless ? 1 : 0;
   case GLSL_TYPE_ATOMIC_UINT:
      return 0;
   case GLSL_TYPE_IMAGE:
      return bindless ? 1 : DIV_ROUND_UP(BRW_IMAGE_PARAM_SIZE, 4);
   case GLSL_TYPE_VOID:
   case GLSL_TYPE_ERROR:
   case GLSL_TYPE_COOPERATIVE_MATRIX:
      unreachable("not reached");
   }

   return 0;
}

/**
 * Returns the minimum number of vec4 elements needed to pack a type.
 *
 * For simple types, it will return 1 (a single vec4); for matrices, the
 * number of columns; for array and struct, the sum of the vec4_size of
 * each of its elements; and for sampler and atomic, zero.
 *
 * This method is useful to calculate how much register space is needed to
 * store a particular type.
 */
extern "C" int
type_size_vec4(const struct glsl_type *type, bool bindless)
{
   return type_size_xvec4(type, true, bindless);
}

/**
 * Returns the minimum number of dvec4 elements needed to pack a type.
 *
 * For simple types, it will return 1 (a single dvec4); for matrices, the
 * number of columns; for array and struct, the sum of the dvec4_size of
 * each of its elements; and for sampler and atomic, zero.
 *
 * This method is useful to calculate how much register space is needed to
 * store a particular type.
 *
 * Measuring double-precision vertex inputs as dvec4 is required because
 * ARB_vertex_attrib_64bit states that these uses the same number of locations
 * than the single-precision version. That is, two consecutives dvec4 would be
 * located in location "x" and location "x+1", not "x+2".
 *
 * In order to map vec4/dvec4 vertex inputs in the proper ATTRs,
 * remap_vs_attrs() will take in account both the location and also if the
 * type fits in one or two vec4 slots.
 */
extern "C" int
type_size_dvec4(const struct glsl_type *type, bool bindless)
{
   return type_size_xvec4(type, false, bindless);
}

src_reg::src_reg(class vec4_visitor *v, const struct glsl_type *type)
{
   init();

   this->file = VGRF;
   this->nr = v->alloc.allocate(type_size_vec4(type, false));

   if (glsl_type_is_array(type) || glsl_type_is_struct(type)) {
      this->swizzle = BRW_SWIZZLE_NOOP;
   } else {
      this->swizzle = brw_swizzle_for_size(type->vector_elements);
   }

   this->type = brw_type_for_base_type(type);
}

src_reg::src_reg(class vec4_visitor *v, const struct glsl_type *type, int size)
{
   assert(size > 0);

   init();

   this->file = VGRF;
   this->nr = v->alloc.allocate(type_size_vec4(type, false) * size);

   this->swizzle = BRW_SWIZZLE_NOOP;

   this->type = brw_type_for_base_type(type);
}

dst_reg::dst_reg(class vec4_visitor *v, const struct glsl_type *type)
{
   init();

   this->file = VGRF;
   this->nr = v->alloc.allocate(type_size_vec4(type, false));

   if (glsl_type_is_array(type) || glsl_type_is_struct(type)) {
      this->writemask = WRITEMASK_XYZW;
   } else {
      this->writemask = (1 << type->vector_elements) - 1;
   }

   this->type = brw_type_for_base_type(type);
}

vec4_instruction *
vec4_visitor::emit_minmax(enum brw_conditional_mod conditionalmod, dst_reg dst,
                          src_reg src0, src_reg src1)
{
   vec4_instruction *inst = emit(BRW_OPCODE_SEL, dst, src0, src1);
   inst->conditional_mod = conditionalmod;
   return inst;
}

/**
 * Emits the instructions needed to perform a pull constant load. before_block
 * and before_inst can be NULL in which case the instruction will be appended
 * to the end of the instruction list.
 */
void
vec4_visitor::emit_pull_constant_load_reg(dst_reg dst,
                                          src_reg surf_index,
                                          src_reg offset_reg,
                                          bblock_t *before_block,
                                          vec4_instruction *before_inst)
{
   assert((before_inst == NULL && before_block == NULL) ||
          (before_inst && before_block));

   vec4_instruction *pull;

   if (devinfo->ver >= 7) {
      dst_reg grf_offset = dst_reg(this, glsl_uint_type());

      grf_offset.type = offset_reg.type;

      pull = MOV(grf_offset, offset_reg);

      if (before_inst)
         emit_before(before_block, before_inst, pull);
      else
         emit(pull);

      pull = new(mem_ctx) vec4_instruction(VS_OPCODE_PULL_CONSTANT_LOAD_GFX7,
                                           dst,
                                           surf_index,
                                           src_reg(grf_offset));
      pull->mlen = 1;
   } else {
      pull = new(mem_ctx) vec4_instruction(VS_OPCODE_PULL_CONSTANT_LOAD,
                                           dst,
                                           surf_index,
                                           offset_reg);
      pull->base_mrf = FIRST_PULL_LOAD_MRF(devinfo->ver) + 1;
      pull->mlen = 1;
   }

   if (before_inst)
      emit_before(before_block, before_inst, pull);
   else
      emit(pull);
}

src_reg
vec4_visitor::emit_uniformize(const src_reg &src)
{
   const src_reg chan_index(this, glsl_uint_type());
   const dst_reg dst = retype(dst_reg(this, glsl_uint_type()),
                              src.type);

   emit(SHADER_OPCODE_FIND_LIVE_CHANNEL, dst_reg(chan_index))
      ->force_writemask_all = true;
   emit(SHADER_OPCODE_BROADCAST, dst, src, chan_index)
      ->force_writemask_all = true;

   return src_reg(dst);
}

void
vec4_visitor::gs_emit_vertex(int /* stream_id */)
{
   unreachable("not reached");
}

void
vec4_visitor::gs_end_primitive()
{
   unreachable("not reached");
}

void
vec4_visitor::emit_ndc_computation()
{
   if (output_reg[VARYING_SLOT_POS][0].file == BAD_FILE)
      return;

   /* Get the position */
   src_reg pos = src_reg(output_reg[VARYING_SLOT_POS][0]);

   /* Build ndc coords, which are (x/w, y/w, z/w, 1/w) */
   dst_reg ndc = dst_reg(this, glsl_vec4_type());
   output_reg[BRW_VARYING_SLOT_NDC][0] = ndc;
   output_num_components[BRW_VARYING_SLOT_NDC][0] = 4;

   current_annotation = "NDC";
   dst_reg ndc_w = ndc;
   ndc_w.writemask = WRITEMASK_W;
   src_reg pos_w = pos;
   pos_w.swizzle = BRW_SWIZZLE4(BRW_SWIZZLE_W, BRW_SWIZZLE_W, BRW_SWIZZLE_W, BRW_SWIZZLE_W);
   emit_math(SHADER_OPCODE_RCP, ndc_w, pos_w);

   dst_reg ndc_xyz = ndc;
   ndc_xyz.writemask = WRITEMASK_XYZ;

   emit(MUL(ndc_xyz, pos, src_reg(ndc_w)));
}

void
vec4_visitor::emit_psiz_and_flags(dst_reg reg)
{
   if (devinfo->ver < 6 &&
       ((prog_data->vue_map.slots_valid & VARYING_BIT_PSIZ) ||
        output_reg[VARYING_SLOT_CLIP_DIST0][0].file != BAD_FILE ||
        devinfo->has_negative_rhw_bug)) {
      dst_reg header1 = dst_reg(this, glsl_uvec4_type());
      dst_reg header1_w = header1;
      header1_w.writemask = WRITEMASK_W;

      emit(MOV(header1, brw_imm_ud(0u)));

      if (prog_data->vue_map.slots_valid & VARYING_BIT_PSIZ) {
	 src_reg psiz = src_reg(output_reg[VARYING_SLOT_PSIZ][0]);

	 current_annotation = "Point size";
	 emit(MUL(header1_w, psiz, brw_imm_f((float)(1 << 11))));
	 emit(AND(header1_w, src_reg(header1_w), brw_imm_d(0x7ff << 8)));
      }

      if (output_reg[VARYING_SLOT_CLIP_DIST0][0].file != BAD_FILE) {
         current_annotation = "Clipping flags";
         dst_reg flags0 = dst_reg(this, glsl_uint_type());

         emit(CMP(dst_null_f(), src_reg(output_reg[VARYING_SLOT_CLIP_DIST0][0]), brw_imm_f(0.0f), BRW_CONDITIONAL_L));
         emit(VS_OPCODE_UNPACK_FLAGS_SIMD4X2, flags0, brw_imm_d(0));
         emit(OR(header1_w, src_reg(header1_w), src_reg(flags0)));
      }

      if (output_reg[VARYING_SLOT_CLIP_DIST1][0].file != BAD_FILE) {
         dst_reg flags1 = dst_reg(this, glsl_uint_type());
         emit(CMP(dst_null_f(), src_reg(output_reg[VARYING_SLOT_CLIP_DIST1][0]), brw_imm_f(0.0f), BRW_CONDITIONAL_L));
         emit(VS_OPCODE_UNPACK_FLAGS_SIMD4X2, flags1, brw_imm_d(0));
         emit(SHL(flags1, src_reg(flags1), brw_imm_d(4)));
         emit(OR(header1_w, src_reg(header1_w), src_reg(flags1)));
      }

      /* i965 clipping workaround:
       * 1) Test for -ve rhw
       * 2) If set,
       *      set ndc = (0,0,0,0)
       *      set ucp[6] = 1
       *
       * Later, clipping will detect ucp[6] and ensure the primitive is
       * clipped against all fixed planes.
       */
      if (devinfo->has_negative_rhw_bug &&
          output_reg[BRW_VARYING_SLOT_NDC][0].file != BAD_FILE) {
         src_reg ndc_w = src_reg(output_reg[BRW_VARYING_SLOT_NDC][0]);
         ndc_w.swizzle = BRW_SWIZZLE_WWWW;
         emit(CMP(dst_null_f(), ndc_w, brw_imm_f(0.0f), BRW_CONDITIONAL_L));
         vec4_instruction *inst;
         inst = emit(OR(header1_w, src_reg(header1_w), brw_imm_ud(1u << 6)));
         inst->predicate = BRW_PREDICATE_NORMAL;
         output_reg[BRW_VARYING_SLOT_NDC][0].type = BRW_REGISTER_TYPE_F;
         inst = emit(MOV(output_reg[BRW_VARYING_SLOT_NDC][0], brw_imm_f(0.0f)));
         inst->predicate = BRW_PREDICATE_NORMAL;
      }

      emit(MOV(retype(reg, BRW_REGISTER_TYPE_UD), src_reg(header1)));
   } else if (devinfo->ver < 6) {
      emit(MOV(retype(reg, BRW_REGISTER_TYPE_UD), brw_imm_ud(0u)));
   } else {
      emit(MOV(retype(reg, BRW_REGISTER_TYPE_D), brw_imm_d(0)));
      if (output_reg[VARYING_SLOT_PSIZ][0].file != BAD_FILE) {
         dst_reg reg_w = reg;
         reg_w.writemask = WRITEMASK_W;
         src_reg reg_as_src = src_reg(output_reg[VARYING_SLOT_PSIZ][0]);
         reg_as_src.type = reg_w.type;
         reg_as_src.swizzle = brw_swizzle_for_size(1);
         emit(MOV(reg_w, reg_as_src));
      }
      if (output_reg[VARYING_SLOT_LAYER][0].file != BAD_FILE) {
         dst_reg reg_y = reg;
         reg_y.writemask = WRITEMASK_Y;
         reg_y.type = BRW_REGISTER_TYPE_D;
         output_reg[VARYING_SLOT_LAYER][0].type = reg_y.type;
         emit(MOV(reg_y, src_reg(output_reg[VARYING_SLOT_LAYER][0])));
      }
      if (output_reg[VARYING_SLOT_VIEWPORT][0].file != BAD_FILE) {
         dst_reg reg_z = reg;
         reg_z.writemask = WRITEMASK_Z;
         reg_z.type = BRW_REGISTER_TYPE_D;
         output_reg[VARYING_SLOT_VIEWPORT][0].type = reg_z.type;
         emit(MOV(reg_z, src_reg(output_reg[VARYING_SLOT_VIEWPORT][0])));
      }
   }
}

vec4_instruction *
vec4_visitor::emit_generic_urb_slot(dst_reg reg, int varying, int component)
{
   assert(varying < VARYING_SLOT_MAX);

   unsigned num_comps = output_num_components[varying][component];
   if (num_comps == 0)
      return NULL;

   assert(output_reg[varying][component].type == reg.type);
   current_annotation = output_reg_annotation[varying];
   if (output_reg[varying][component].file != BAD_FILE) {
      src_reg src = src_reg(output_reg[varying][component]);
      src.swizzle = BRW_SWZ_COMP_OUTPUT(component);
      reg.writemask =
         brw_writemask_for_component_packing(num_comps, component);
      return emit(MOV(reg, src));
   }
   return NULL;
}

void
vec4_visitor::emit_urb_slot(dst_reg reg, int varying)
{
   reg.type = BRW_REGISTER_TYPE_F;
   output_reg[varying][0].type = reg.type;

   switch (varying) {
   case VARYING_SLOT_PSIZ:
   {
      /* PSIZ is always in slot 0, and is coupled with other flags. */
      current_annotation = "indices, point width, clip flags";
      emit_psiz_and_flags(reg);
      break;
   }
   case BRW_VARYING_SLOT_NDC:
      current_annotation = "NDC";
      if (output_reg[BRW_VARYING_SLOT_NDC][0].file != BAD_FILE)
         emit(MOV(reg, src_reg(output_reg[BRW_VARYING_SLOT_NDC][0])));
      break;
   case VARYING_SLOT_POS:
      current_annotation = "gl_Position";
      if (output_reg[VARYING_SLOT_POS][0].file != BAD_FILE)
         emit(MOV(reg, src_reg(output_reg[VARYING_SLOT_POS][0])));
      break;
   case BRW_VARYING_SLOT_PAD:
      /* No need to write to this slot */
      break;
   default:
      for (int i = 0; i < 4; i++) {
         emit_generic_urb_slot(reg, varying, i);
      }
      break;
   }
}

static unsigned
align_interleaved_urb_mlen(const struct intel_device_info *devinfo,
                           unsigned mlen)
{
   if (devinfo->ver >= 6) {
      /* URB data written (does not include the message header reg) must
       * be a multiple of 256 bits, or 2 VS registers.  See vol5c.5,
       * section 5.4.3.2.2: URB_INTERLEAVED.
       *
       * URB entries are allocated on a multiple of 1024 bits, so an
       * extra 128 bits written here to make the end align to 256 is
       * no problem.
       */
      if ((mlen % 2) != 1)
	 mlen++;
   }

   return mlen;
}


/**
 * Generates the VUE payload plus the necessary URB write instructions to
 * output it.
 *
 * The VUE layout is documented in Volume 2a.
 */
void
vec4_visitor::emit_vertex()
{
   /* MRF 0 is reserved for the debugger, so start with message header
    * in MRF 1.
    */
   int base_mrf = 1;
   int mrf = base_mrf;
   /* In the process of generating our URB write message contents, we
    * may need to unspill a register or load from an array.  Those
    * reads would use MRFs 14-15.
    */
   int max_usable_mrf = FIRST_SPILL_MRF(devinfo->ver);

   /* The following assertion verifies that max_usable_mrf causes an
    * even-numbered amount of URB write data, which will meet gfx6's
    * requirements for length alignment.
    */
   assert ((max_usable_mrf - base_mrf) % 2 == 0);

   /* First mrf is the g0-based message header containing URB handles and
    * such.
    */
   emit_urb_write_header(mrf++);

   if (devinfo->ver < 6) {
      emit_ndc_computation();
   }

   /* We may need to split this up into several URB writes, so do them in a
    * loop.
    */
   int slot = 0;
   bool complete = false;
   do {
      /* URB offset is in URB row increments, and each of our MRFs is half of
       * one of those, since we're doing interleaved writes.
       */
      int offset = slot / 2;

      mrf = base_mrf + 1;
      for (; slot < prog_data->vue_map.num_slots; ++slot) {
         emit_urb_slot(dst_reg(MRF, mrf++),
                       prog_data->vue_map.slot_to_varying[slot]);

         /* If this was max_usable_mrf, we can't fit anything more into this
          * URB WRITE. Same thing if we reached the maximum length available.
          */
         if (mrf > max_usable_mrf ||
             align_interleaved_urb_mlen(devinfo, mrf - base_mrf + 1) > BRW_MAX_MSG_LENGTH) {
            slot++;
            break;
         }
      }

      complete = slot >= prog_data->vue_map.num_slots;
      current_annotation = "URB write";
      vec4_instruction *inst = emit_urb_write_opcode(complete);
      inst->base_mrf = base_mrf;
      inst->mlen = align_interleaved_urb_mlen(devinfo, mrf - base_mrf);
      inst->offset += offset;
   } while(!complete);
}


src_reg
vec4_visitor::get_scratch_offset(bblock_t *block, vec4_instruction *inst,
				 src_reg *reladdr, int reg_offset)
{
   /* Because we store the values to scratch interleaved like our
    * vertex data, we need to scale the vec4 index by 2.
    */
   int message_header_scale = 2;

   /* Pre-gfx6, the message header uses byte offsets instead of vec4
    * (16-byte) offset units.
    */
   if (devinfo->ver < 6)
      message_header_scale *= 16;

   if (reladdr) {
      /* A vec4 is 16 bytes and a dvec4 is 32 bytes so for doubles we have
       * to multiply the reladdr by 2. Notice that the reg_offset part
       * is in units of 16 bytes and is used to select the low/high 16-byte
       * chunk of a full dvec4, so we don't want to multiply that part.
       */
      src_reg index = src_reg(this, glsl_int_type());
      if (type_sz(inst->dst.type) < 8) {
         emit_before(block, inst, ADD(dst_reg(index), *reladdr,
                                      brw_imm_d(reg_offset)));
         emit_before(block, inst, MUL(dst_reg(index), index,
                                      brw_imm_d(message_header_scale)));
      } else {
         emit_before(block, inst, MUL(dst_reg(index), *reladdr,
                                      brw_imm_d(message_header_scale * 2)));
         emit_before(block, inst, ADD(dst_reg(index), index,
                                      brw_imm_d(reg_offset * message_header_scale)));
      }
      return index;
   } else {
      return brw_imm_d(reg_offset * message_header_scale);
   }
}

/**
 * Emits an instruction before @inst to load the value named by @orig_src
 * from scratch space at @base_offset to @temp.
 *
 * @base_offset is measured in 32-byte units (the size of a register).
 */
void
vec4_visitor::emit_scratch_read(bblock_t *block, vec4_instruction *inst,
				dst_reg temp, src_reg orig_src,
				int base_offset)
{
   assert(orig_src.offset % REG_SIZE == 0);
   int reg_offset = base_offset + orig_src.offset / REG_SIZE;
   src_reg index = get_scratch_offset(block, inst, orig_src.reladdr,
                                      reg_offset);

   if (type_sz(orig_src.type) < 8) {
      emit_before(block, inst, SCRATCH_READ(temp, index));
   } else {
      dst_reg shuffled = dst_reg(this, glsl_dvec4_type());
      dst_reg shuffled_float = retype(shuffled, BRW_REGISTER_TYPE_F);
      emit_before(block, inst, SCRATCH_READ(shuffled_float, index));
      index = get_scratch_offset(block, inst, orig_src.reladdr, reg_offset + 1);
      vec4_instruction *last_read =
         SCRATCH_READ(byte_offset(shuffled_float, REG_SIZE), index);
      emit_before(block, inst, last_read);
      shuffle_64bit_data(temp, src_reg(shuffled), false, true, block, last_read);
   }
}

/**
 * Emits an instruction after @inst to store the value to be written
 * to @orig_dst to scratch space at @base_offset, from @temp.
 *
 * @base_offset is measured in 32-byte units (the size of a register).
 */
void
vec4_visitor::emit_scratch_write(bblock_t *block, vec4_instruction *inst,
                                 int base_offset)
{
   assert(inst->dst.offset % REG_SIZE == 0);
   int reg_offset = base_offset + inst->dst.offset / REG_SIZE;
   src_reg index = get_scratch_offset(block, inst, inst->dst.reladdr,
                                      reg_offset);

   /* Create a temporary register to store *inst's result in.
    *
    * We have to be careful in MOVing from our temporary result register in
    * the scratch write.  If we swizzle from channels of the temporary that
    * weren't initialized, it will confuse live interval analysis, which will
    * make spilling fail to make progress.
    */
   bool is_64bit = type_sz(inst->dst.type) == 8;
   const glsl_type *alloc_type =
      is_64bit ? glsl_dvec4_type() : glsl_vec4_type();
   const src_reg temp = swizzle(retype(src_reg(this, alloc_type),
                                       inst->dst.type),
                                brw_swizzle_for_mask(inst->dst.writemask));

   if (!is_64bit) {
      dst_reg dst = dst_reg(brw_writemask(brw_vec8_grf(0, 0),
				          inst->dst.writemask));
      vec4_instruction *write = SCRATCH_WRITE(dst, temp, index);
      if (inst->opcode != BRW_OPCODE_SEL)
         write->predicate = inst->predicate;
      write->ir = inst->ir;
      write->annotation = inst->annotation;
      inst->insert_after(block, write);
   } else {
      dst_reg shuffled = dst_reg(this, alloc_type);
      vec4_instruction *last =
         shuffle_64bit_data(shuffled, temp, true, true, block, inst);
      src_reg shuffled_float = src_reg(retype(shuffled, BRW_REGISTER_TYPE_F));

      uint8_t mask = 0;
      if (inst->dst.writemask & WRITEMASK_X)
         mask |= WRITEMASK_XY;
      if (inst->dst.writemask & WRITEMASK_Y)
         mask |= WRITEMASK_ZW;
      if (mask) {
         dst_reg dst = dst_reg(brw_writemask(brw_vec8_grf(0, 0), mask));

         vec4_instruction *write = SCRATCH_WRITE(dst, shuffled_float, index);
         if (inst->opcode != BRW_OPCODE_SEL)
            write->predicate = inst->predicate;
         write->ir = inst->ir;
         write->annotation = inst->annotation;
         last->insert_after(block, write);
      }

      mask = 0;
      if (inst->dst.writemask & WRITEMASK_Z)
         mask |= WRITEMASK_XY;
      if (inst->dst.writemask & WRITEMASK_W)
         mask |= WRITEMASK_ZW;
      if (mask) {
         dst_reg dst = dst_reg(brw_writemask(brw_vec8_grf(0, 0), mask));

         src_reg index = get_scratch_offset(block, inst, inst->dst.reladdr,
                                            reg_offset + 1);
         vec4_instruction *write =
            SCRATCH_WRITE(dst, byte_offset(shuffled_float, REG_SIZE), index);
         if (inst->opcode != BRW_OPCODE_SEL)
            write->predicate = inst->predicate;
         write->ir = inst->ir;
         write->annotation = inst->annotation;
         last->insert_after(block, write);
      }
   }

   inst->dst.file = temp.file;
   inst->dst.nr = temp.nr;
   inst->dst.offset %= REG_SIZE;
   inst->dst.reladdr = NULL;
}

/**
 * Checks if \p src and/or \p src.reladdr require a scratch read, and if so,
 * adds the scratch read(s) before \p inst. The function also checks for
 * recursive reladdr scratch accesses, issuing the corresponding scratch
 * loads and rewriting reladdr references accordingly.
 *
 * \return \p src if it did not require a scratch load, otherwise, the
 * register holding the result of the scratch load that the caller should
 * use to rewrite src.
 */
src_reg
vec4_visitor::emit_resolve_reladdr(int scratch_loc[], bblock_t *block,
                                   vec4_instruction *inst, src_reg src)
{
   /* Resolve recursive reladdr scratch access by calling ourselves
    * with src.reladdr
    */
   if (src.reladdr)
      *src.reladdr = emit_resolve_reladdr(scratch_loc, block, inst,
                                          *src.reladdr);

   /* Now handle scratch access on src */
   if (src.file == VGRF && scratch_loc[src.nr] != -1) {
      dst_reg temp = dst_reg(this, type_sz(src.type) == 8 ?
         glsl_dvec4_type() : glsl_vec4_type());
      emit_scratch_read(block, inst, temp, src, scratch_loc[src.nr]);
      src.nr = temp.nr;
      src.offset %= REG_SIZE;
      src.reladdr = NULL;
   }

   return src;
}

/**
 * We can't generally support array access in GRF space, because a
 * single instruction's destination can only span 2 contiguous
 * registers.  So, we send all GRF arrays that get variable index
 * access to scratch space.
 */
void
vec4_visitor::move_grf_array_access_to_scratch()
{
   int scratch_loc[this->alloc.count];
   memset(scratch_loc, -1, sizeof(scratch_loc));

   /* First, calculate the set of virtual GRFs that need to be punted
    * to scratch due to having any array access on them, and where in
    * scratch.
    */
   foreach_block_and_inst(block, vec4_instruction, inst, cfg) {
      if (inst->dst.file == VGRF && inst->dst.reladdr) {
         if (scratch_loc[inst->dst.nr] == -1) {
            scratch_loc[inst->dst.nr] = last_scratch;
            last_scratch += this->alloc.sizes[inst->dst.nr];
         }

         for (src_reg *iter = inst->dst.reladdr;
              iter->reladdr;
              iter = iter->reladdr) {
            if (iter->file == VGRF && scratch_loc[iter->nr] == -1) {
               scratch_loc[iter->nr] = last_scratch;
               last_scratch += this->alloc.sizes[iter->nr];
            }
         }
      }

      for (int i = 0 ; i < 3; i++) {
         for (src_reg *iter = &inst->src[i];
              iter->reladdr;
              iter = iter->reladdr) {
            if (iter->file == VGRF && scratch_loc[iter->nr] == -1) {
               scratch_loc[iter->nr] = last_scratch;
               last_scratch += this->alloc.sizes[iter->nr];
            }
         }
      }
   }

   /* Now, for anything that will be accessed through scratch, rewrite
    * it to load/store.  Note that this is a _safe list walk, because
    * we may generate a new scratch_write instruction after the one
    * we're processing.
    */
   foreach_block_and_inst_safe(block, vec4_instruction, inst, cfg) {
      /* Set up the annotation tracking for new generated instructions. */
      base_ir = inst->ir;
      current_annotation = inst->annotation;

      /* First handle scratch access on the dst. Notice we have to handle
       * the case where the dst's reladdr also points to scratch space.
       */
      if (inst->dst.reladdr)
         *inst->dst.reladdr = emit_resolve_reladdr(scratch_loc, block, inst,
                                                   *inst->dst.reladdr);

      /* Now that we have handled any (possibly recursive) reladdr scratch
       * accesses for dst we can safely do the scratch write for dst itself
       */
      if (inst->dst.file == VGRF && scratch_loc[inst->dst.nr] != -1)
         emit_scratch_write(block, inst, scratch_loc[inst->dst.nr]);

      /* Now handle scratch access on any src. In this case, since inst->src[i]
       * already is a src_reg, we can just call emit_resolve_reladdr with
       * inst->src[i] and it will take care of handling scratch loads for
       * both src and src.reladdr (recursively).
       */
      for (int i = 0 ; i < 3; i++) {
         inst->src[i] = emit_resolve_reladdr(scratch_loc, block, inst,
                                             inst->src[i]);
      }
   }
}

void
vec4_visitor::resolve_ud_negate(src_reg *reg)
{
   if (reg->type != BRW_REGISTER_TYPE_UD ||
       !reg->negate)
      return;

   src_reg temp = src_reg(this, glsl_uvec4_type());
   emit(BRW_OPCODE_MOV, dst_reg(temp), *reg);
   *reg = temp;
}

static brw_rnd_mode
brw_rnd_mode_from_execution_mode(unsigned execution_mode)
{
   if (nir_has_any_rounding_mode_rtne(execution_mode))
      return BRW_RND_MODE_RTNE;
   if (nir_has_any_rounding_mode_rtz(execution_mode))
      return BRW_RND_MODE_RTZ;
   return BRW_RND_MODE_UNSPECIFIED;
}

void
vec4_visitor::emit_shader_float_controls_execution_mode()
{
   unsigned execution_mode = this->nir->info.float_controls_execution_mode;
   if (nir_has_any_rounding_mode_enabled(execution_mode)) {
      brw_rnd_mode rnd = brw_rnd_mode_from_execution_mode(execution_mode);
      const vec4_builder bld = vec4_builder(this).at_end();
      bld.exec_all().emit(SHADER_OPCODE_RND_MODE, dst_null_ud(), brw_imm_d(rnd));
   }
}

vec4_visitor::vec4_visitor(const struct brw_compiler *compiler,
                           const struct brw_compile_params *params,
                           const struct brw_sampler_prog_key_data *key_tex,
                           struct brw_vue_prog_data *prog_data,
                           const nir_shader *shader,
                           bool no_spills,
                           bool debug_enabled)
   : backend_shader(compiler, params, shader, &prog_data->base, debug_enabled),
     key_tex(key_tex),
     prog_data(prog_data),
     fail_msg(NULL),
     first_non_payload_grf(0),
     ubo_push_start(),
     push_length(0),
     live_analysis(this), performance_analysis(this),
     no_spills(no_spills),
     last_scratch(0)
{
   this->failed = false;

   this->base_ir = NULL;
   this->current_annotation = NULL;
   memset(this->output_reg_annotation, 0, sizeof(this->output_reg_annotation));

   memset(this->output_num_components, 0, sizeof(this->output_num_components));

   this->max_grf = devinfo->ver >= 7 ? GFX7_MRF_HACK_START : BRW_MAX_GRF;

   this->uniforms = 0;

   this->nir_ssa_values = NULL;
}


void
vec4_visitor::fail(const char *format, ...)
{
   va_list va;
   char *msg;

   if (failed)
      return;

   failed = true;

   va_start(va, format);
   msg = ralloc_vasprintf(mem_ctx, format, va);
   va_end(va);
   msg = ralloc_asprintf(mem_ctx, "%s compile failed: %s\n",
                         _mesa_shader_stage_to_abbrev(stage), msg);

   this->fail_msg = msg;

   if (unlikely(debug_enabled)) {
      fprintf(stderr, "%s",  msg);
   }
}

} /* namespace brw */
