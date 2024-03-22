/*
 * Copyright (C) 2021 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "bi_builder.h"
#include "va_compiler.h"
#include "valhall.h"
#include "valhall_enums.h"

/* This file contains the final passes of the compiler. Running after
 * scheduling and RA, the IR is now finalized, so we need to emit it to actual
 * bits on the wire (as well as fixup branches)
 */

/*
 * Unreachable for encoding failures, when hitting an invalid instruction.
 * Prints the (first) failing instruction to aid debugging.
 */
NORETURN static void PRINTFLIKE(2, 3)
   invalid_instruction(const bi_instr *I, const char *cause, ...)
{
   fputs("\nInvalid ", stderr);

   va_list ap;
   va_start(ap, cause);
   vfprintf(stderr, cause, ap);
   va_end(ap);

   fputs(":\n\t", stderr);
   bi_print_instr(I, stderr);
   fprintf(stderr, "\n");

   unreachable("Invalid instruction");
}

/*
 * Like assert, but prints the instruction if the assertion fails to aid
 * debugging invalid inputs to the packing module.
 */
#define pack_assert(I, cond)                                                   \
   if (!(cond))                                                                \
      invalid_instruction(I, "invariant " #cond);

/*
 * Validate that two adjacent 32-bit sources form an aligned 64-bit register
 * pair. This is a compiler invariant, required on Valhall but not on Bifrost.
 */
static void
va_validate_register_pair(const bi_instr *I, unsigned s)
{
   ASSERTED bi_index lo = I->src[s], hi = I->src[s + 1];

   pack_assert(I, lo.type == hi.type);

   if (lo.type == BI_INDEX_REGISTER) {
      pack_assert(I, hi.value & 1);
      pack_assert(I, hi.value == lo.value + 1);
   } else if (lo.type == BI_INDEX_FAU && lo.value & BIR_FAU_IMMEDIATE) {
      /* Small constants are zero extended, so the top word encode zero */
      pack_assert(I, hi.value == (BIR_FAU_IMMEDIATE | 0));
   } else {
      pack_assert(I, hi.offset & 1);
      pack_assert(I, hi.offset == lo.offset + 1);
   }
}

static unsigned
va_pack_reg(const bi_instr *I, bi_index idx)
{
   pack_assert(I, idx.type == BI_INDEX_REGISTER);
   pack_assert(I, idx.value < 64);

   return idx.value;
}

static unsigned
va_pack_fau_special(const bi_instr *I, enum bir_fau fau)
{
   switch (fau) {
   case BIR_FAU_ATEST_PARAM:
      return VA_FAU_SPECIAL_PAGE_0_ATEST_DATUM;
   case BIR_FAU_TLS_PTR:
      return VA_FAU_SPECIAL_PAGE_1_THREAD_LOCAL_POINTER;
   case BIR_FAU_WLS_PTR:
      return VA_FAU_SPECIAL_PAGE_1_WORKGROUP_LOCAL_POINTER;
   case BIR_FAU_LANE_ID:
      return VA_FAU_SPECIAL_PAGE_3_LANE_ID;
   case BIR_FAU_PROGRAM_COUNTER:
      return VA_FAU_SPECIAL_PAGE_3_PROGRAM_COUNTER;
   case BIR_FAU_SAMPLE_POS_ARRAY:
      return VA_FAU_SPECIAL_PAGE_0_SAMPLE;

   case BIR_FAU_BLEND_0 ...(BIR_FAU_BLEND_0 + 7):
      return VA_FAU_SPECIAL_PAGE_0_BLEND_DESCRIPTOR_0 + (fau - BIR_FAU_BLEND_0);

   default:
      invalid_instruction(I, "FAU");
   }
}

/*
 * Encode a 64-bit FAU source. The offset is ignored, so this function can be
 * used to encode a 32-bit FAU source by or'ing in the appropriate offset.
 */
static unsigned
va_pack_fau_64(const bi_instr *I, bi_index idx)
{
   pack_assert(I, idx.type == BI_INDEX_FAU);

   unsigned val = (idx.value & BITFIELD_MASK(5));

   if (idx.value & BIR_FAU_IMMEDIATE)
      return (0x3 << 6) | (val << 1);
   else if (idx.value & BIR_FAU_UNIFORM)
      return (0x2 << 6) | (val << 1);
   else
      return (0x7 << 5) | (va_pack_fau_special(I, idx.value) << 1);
}

static unsigned
va_pack_src(const bi_instr *I, unsigned s)
{
   bi_index idx = I->src[s];

   if (idx.type == BI_INDEX_REGISTER) {
      unsigned value = va_pack_reg(I, idx);
      if (idx.discard)
         value |= (1 << 6);
      return value;
   } else if (idx.type == BI_INDEX_FAU) {
      pack_assert(I, idx.offset <= 1);
      return va_pack_fau_64(I, idx) | idx.offset;
   }

   invalid_instruction(I, "type of source %u", s);
}

static unsigned
va_pack_wrmask(const bi_instr *I)
{
   switch (I->dest[0].swizzle) {
   case BI_SWIZZLE_H00:
      return 0x1;
   case BI_SWIZZLE_H11:
      return 0x2;
   case BI_SWIZZLE_H01:
      return 0x3;
   default:
      invalid_instruction(I, "write mask");
   }
}

static enum va_atomic_operation
va_pack_atom_opc(const bi_instr *I)
{
   switch (I->atom_opc) {
   case BI_ATOM_OPC_AADD:
      return VA_ATOMIC_OPERATION_AADD;
   case BI_ATOM_OPC_ASMIN:
      return VA_ATOMIC_OPERATION_ASMIN;
   case BI_ATOM_OPC_ASMAX:
      return VA_ATOMIC_OPERATION_ASMAX;
   case BI_ATOM_OPC_AUMIN:
      return VA_ATOMIC_OPERATION_AUMIN;
   case BI_ATOM_OPC_AUMAX:
      return VA_ATOMIC_OPERATION_AUMAX;
   case BI_ATOM_OPC_AAND:
      return VA_ATOMIC_OPERATION_AAND;
   case BI_ATOM_OPC_AOR:
      return VA_ATOMIC_OPERATION_AOR;
   case BI_ATOM_OPC_AXOR:
      return VA_ATOMIC_OPERATION_AXOR;
   case BI_ATOM_OPC_ACMPXCHG:
   case BI_ATOM_OPC_AXCHG:
      return VA_ATOMIC_OPERATION_AXCHG;
   default:
      invalid_instruction(I, "atomic opcode");
   }
}

static enum va_atomic_operation_with_1
va_pack_atom_opc_1(const bi_instr *I)
{
   switch (I->atom_opc) {
   case BI_ATOM_OPC_AINC:
      return VA_ATOMIC_OPERATION_WITH_1_AINC;
   case BI_ATOM_OPC_ADEC:
      return VA_ATOMIC_OPERATION_WITH_1_ADEC;
   case BI_ATOM_OPC_AUMAX1:
      return VA_ATOMIC_OPERATION_WITH_1_AUMAX1;
   case BI_ATOM_OPC_ASMAX1:
      return VA_ATOMIC_OPERATION_WITH_1_ASMAX1;
   case BI_ATOM_OPC_AOR1:
      return VA_ATOMIC_OPERATION_WITH_1_AOR1;
   default:
      invalid_instruction(I, "atomic opcode with implicit 1");
   }
}

static unsigned
va_pack_dest(const bi_instr *I)
{
   assert(I->nr_dests);
   return va_pack_reg(I, I->dest[0]) | (va_pack_wrmask(I) << 6);
}

static enum va_widen
va_pack_widen_f32(const bi_instr *I, enum bi_swizzle swz)
{
   switch (swz) {
   case BI_SWIZZLE_H01:
      return VA_WIDEN_NONE;
   case BI_SWIZZLE_H00:
      return VA_WIDEN_H0;
   case BI_SWIZZLE_H11:
      return VA_WIDEN_H1;
   default:
      invalid_instruction(I, "widen");
   }
}

static enum va_swizzles_16_bit
va_pack_swizzle_f16(const bi_instr *I, enum bi_swizzle swz)
{
   switch (swz) {
   case BI_SWIZZLE_H00:
      return VA_SWIZZLES_16_BIT_H00;
   case BI_SWIZZLE_H10:
      return VA_SWIZZLES_16_BIT_H10;
   case BI_SWIZZLE_H01:
      return VA_SWIZZLES_16_BIT_H01;
   case BI_SWIZZLE_H11:
      return VA_SWIZZLES_16_BIT_H11;
   default:
      invalid_instruction(I, "16-bit swizzle");
   }
}

static unsigned
va_pack_widen(const bi_instr *I, enum bi_swizzle swz, enum va_size size)
{
   if (size == VA_SIZE_8) {
      switch (swz) {
      case BI_SWIZZLE_H01:
         return VA_SWIZZLES_8_BIT_B0123;
      case BI_SWIZZLE_H00:
         return VA_SWIZZLES_8_BIT_B0101;
      case BI_SWIZZLE_H11:
         return VA_SWIZZLES_8_BIT_B2323;
      case BI_SWIZZLE_B0000:
         return VA_SWIZZLES_8_BIT_B0000;
      case BI_SWIZZLE_B1111:
         return VA_SWIZZLES_8_BIT_B1111;
      case BI_SWIZZLE_B2222:
         return VA_SWIZZLES_8_BIT_B2222;
      case BI_SWIZZLE_B3333:
         return VA_SWIZZLES_8_BIT_B3333;
      default:
         invalid_instruction(I, "8-bit widen");
      }
   } else if (size == VA_SIZE_16) {
      switch (swz) {
      case BI_SWIZZLE_H00:
         return VA_SWIZZLES_16_BIT_H00;
      case BI_SWIZZLE_H10:
         return VA_SWIZZLES_16_BIT_H10;
      case BI_SWIZZLE_H01:
         return VA_SWIZZLES_16_BIT_H01;
      case BI_SWIZZLE_H11:
         return VA_SWIZZLES_16_BIT_H11;
      case BI_SWIZZLE_B0000:
         return VA_SWIZZLES_16_BIT_B00;
      case BI_SWIZZLE_B1111:
         return VA_SWIZZLES_16_BIT_B11;
      case BI_SWIZZLE_B2222:
         return VA_SWIZZLES_16_BIT_B22;
      case BI_SWIZZLE_B3333:
         return VA_SWIZZLES_16_BIT_B33;
      default:
         invalid_instruction(I, "16-bit widen");
      }
   } else if (size == VA_SIZE_32) {
      switch (swz) {
      case BI_SWIZZLE_H01:
         return VA_SWIZZLES_32_BIT_NONE;
      case BI_SWIZZLE_H00:
         return VA_SWIZZLES_32_BIT_H0;
      case BI_SWIZZLE_H11:
         return VA_SWIZZLES_32_BIT_H1;
      case BI_SWIZZLE_B0000:
         return VA_SWIZZLES_32_BIT_B0;
      case BI_SWIZZLE_B1111:
         return VA_SWIZZLES_32_BIT_B1;
      case BI_SWIZZLE_B2222:
         return VA_SWIZZLES_32_BIT_B2;
      case BI_SWIZZLE_B3333:
         return VA_SWIZZLES_32_BIT_B3;
      default:
         invalid_instruction(I, "32-bit widen");
      }
   } else {
      invalid_instruction(I, "type size for widen");
   }
}

static enum va_half_swizzles_8_bit
va_pack_halfswizzle(const bi_instr *I, enum bi_swizzle swz)
{
   switch (swz) {
   case BI_SWIZZLE_B0000:
      return VA_HALF_SWIZZLES_8_BIT_B00;
   case BI_SWIZZLE_B1111:
      return VA_HALF_SWIZZLES_8_BIT_B11;
   case BI_SWIZZLE_B2222:
      return VA_HALF_SWIZZLES_8_BIT_B22;
   case BI_SWIZZLE_B3333:
      return VA_HALF_SWIZZLES_8_BIT_B33;
   case BI_SWIZZLE_B0011:
      return VA_HALF_SWIZZLES_8_BIT_B01;
   case BI_SWIZZLE_B2233:
      return VA_HALF_SWIZZLES_8_BIT_B23;
   case BI_SWIZZLE_B0022:
      return VA_HALF_SWIZZLES_8_BIT_B02;
   default:
      invalid_instruction(I, "v2u8 swizzle");
   }
}

static enum va_lanes_8_bit
va_pack_shift_lanes(const bi_instr *I, enum bi_swizzle swz)
{
   switch (swz) {
   case BI_SWIZZLE_H01:
      return VA_LANES_8_BIT_B02;
   case BI_SWIZZLE_B0000:
      return VA_LANES_8_BIT_B00;
   case BI_SWIZZLE_B1111:
      return VA_LANES_8_BIT_B11;
   case BI_SWIZZLE_B2222:
      return VA_LANES_8_BIT_B22;
   case BI_SWIZZLE_B3333:
      return VA_LANES_8_BIT_B33;
   default:
      invalid_instruction(I, "lane shift");
   }
}

static enum va_combine
va_pack_combine(const bi_instr *I, enum bi_swizzle swz)
{
   switch (swz) {
   case BI_SWIZZLE_H01:
      return VA_COMBINE_NONE;
   case BI_SWIZZLE_H00:
      return VA_COMBINE_H0;
   case BI_SWIZZLE_H11:
      return VA_COMBINE_H1;
   default:
      invalid_instruction(I, "branch lane");
   }
}

static enum va_source_format
va_pack_source_format(const bi_instr *I)
{
   switch (I->source_format) {
   case BI_SOURCE_FORMAT_FLAT32:
      return VA_SOURCE_FORMAT_SRC_FLAT32;
   case BI_SOURCE_FORMAT_FLAT16:
      return VA_SOURCE_FORMAT_SRC_FLAT16;
   case BI_SOURCE_FORMAT_F32:
      return VA_SOURCE_FORMAT_SRC_F32;
   case BI_SOURCE_FORMAT_F16:
      return VA_SOURCE_FORMAT_SRC_F16;
   }

   invalid_instruction(I, "source format");
}

static uint64_t
va_pack_rhadd(const bi_instr *I)
{
   switch (I->round) {
   case BI_ROUND_RTN:
      return 0; /* hadd */
   case BI_ROUND_RTP:
      return BITFIELD_BIT(30); /* rhadd */
   default:
      unreachable("Invalid round for HADD");
   }
}

static uint64_t
va_pack_alu(const bi_instr *I)
{
   struct va_opcode_info info = valhall_opcodes[I->op];
   uint64_t hex = 0;

   switch (I->op) {
   /* Add FREXP flags */
   case BI_OPCODE_FREXPE_F32:
   case BI_OPCODE_FREXPE_V2F16:
   case BI_OPCODE_FREXPM_F32:
   case BI_OPCODE_FREXPM_V2F16:
      if (I->sqrt)
         hex |= 1ull << 24;
      if (I->log)
         hex |= 1ull << 25;
      break;

   /* Add mux type */
   case BI_OPCODE_MUX_I32:
   case BI_OPCODE_MUX_V2I16:
   case BI_OPCODE_MUX_V4I8:
      hex |= (uint64_t)I->mux << 32;
      break;

   /* Add .eq flag */
   case BI_OPCODE_BRANCHZ_I16:
   case BI_OPCODE_BRANCHZI:
      pack_assert(I, I->cmpf == BI_CMPF_EQ || I->cmpf == BI_CMPF_NE);

      if (I->cmpf == BI_CMPF_EQ)
         hex |= (1ull << 36);

      if (I->op == BI_OPCODE_BRANCHZI)
         hex |= (0x1ull << 40); /* Absolute */
      else
         hex |= ((uint64_t)I->branch_offset & BITFIELD_MASK(27)) << 8;

      break;

   /* Add arithmetic flag */
   case BI_OPCODE_RSHIFT_AND_I32:
   case BI_OPCODE_RSHIFT_AND_V2I16:
   case BI_OPCODE_RSHIFT_AND_V4I8:
   case BI_OPCODE_RSHIFT_OR_I32:
   case BI_OPCODE_RSHIFT_OR_V2I16:
   case BI_OPCODE_RSHIFT_OR_V4I8:
   case BI_OPCODE_RSHIFT_XOR_I32:
   case BI_OPCODE_RSHIFT_XOR_V2I16:
   case BI_OPCODE_RSHIFT_XOR_V4I8:
      hex |= (uint64_t)I->arithmetic << 34;
      break;

   case BI_OPCODE_LEA_BUF_IMM:
      /* Buffer table index */
      hex |= 0xD << 8;
      break;

   case BI_OPCODE_LEA_ATTR_IMM:
      hex |= ((uint64_t)I->table) << 16;
      hex |= ((uint64_t)I->attribute_index) << 20;
      break;

   case BI_OPCODE_IADD_IMM_I32:
   case BI_OPCODE_IADD_IMM_V2I16:
   case BI_OPCODE_IADD_IMM_V4I8:
   case BI_OPCODE_FADD_IMM_F32:
   case BI_OPCODE_FADD_IMM_V2F16:
      hex |= ((uint64_t)I->index) << 8;
      break;

   case BI_OPCODE_CLPER_I32:
      hex |= ((uint64_t)I->inactive_result) << 22;
      hex |= ((uint64_t)I->lane_op) << 32;
      hex |= ((uint64_t)I->subgroup) << 36;
      break;

   case BI_OPCODE_LD_VAR:
   case BI_OPCODE_LD_VAR_FLAT:
   case BI_OPCODE_LD_VAR_IMM:
   case BI_OPCODE_LD_VAR_FLAT_IMM:
   case BI_OPCODE_LD_VAR_BUF_F16:
   case BI_OPCODE_LD_VAR_BUF_F32:
   case BI_OPCODE_LD_VAR_BUF_IMM_F16:
   case BI_OPCODE_LD_VAR_BUF_IMM_F32:
   case BI_OPCODE_LD_VAR_SPECIAL:
      if (I->op == BI_OPCODE_LD_VAR_SPECIAL)
         hex |= ((uint64_t)I->varying_name) << 12; /* instead of index */
      else if (I->op == BI_OPCODE_LD_VAR_BUF_IMM_F16 ||
               I->op == BI_OPCODE_LD_VAR_BUF_IMM_F32) {
         hex |= ((uint64_t)I->index) << 16;
      } else if (I->op == BI_OPCODE_LD_VAR_IMM ||
                 I->op == BI_OPCODE_LD_VAR_FLAT_IMM) {
         hex |= ((uint64_t)I->table) << 8;
         hex |= ((uint64_t)I->index) << 12;
      }

      hex |= ((uint64_t)va_pack_source_format(I)) << 24;
      hex |= ((uint64_t)I->update) << 36;
      hex |= ((uint64_t)I->sample) << 38;
      break;

   case BI_OPCODE_LD_ATTR_IMM:
      hex |= ((uint64_t)I->table) << 16;
      hex |= ((uint64_t)I->attribute_index) << 20;
      break;

   case BI_OPCODE_LD_TEX_IMM:
   case BI_OPCODE_LEA_TEX_IMM:
      hex |= ((uint64_t)I->table) << 16;
      hex |= ((uint64_t)I->texture_index) << 20;
      break;

   case BI_OPCODE_ZS_EMIT:
      if (I->stencil)
         hex |= (1 << 24);
      if (I->z)
         hex |= (1 << 25);
      break;

   default:
      break;
   }

   /* FMA_RSCALE.f32 special modes treated as extra opcodes */
   if (I->op == BI_OPCODE_FMA_RSCALE_F32) {
      pack_assert(I, I->special < 4);
      hex |= ((uint64_t)I->special) << 48;
   }

   /* Add the normal destination or a placeholder.  Staging destinations are
    * added elsewhere, as they require special handling for control fields.
    */
   if (info.has_dest && info.nr_staging_dests == 0) {
      hex |= (uint64_t)va_pack_dest(I) << 40;
   } else if (info.nr_staging_dests == 0 && info.nr_staging_srcs == 0) {
      pack_assert(I, I->nr_dests == 0);
      hex |= 0xC0ull << 40; /* Placeholder */
   }

   bool swap12 = va_swap_12(I->op);

   /* First src is staging if we read, skip it when packing sources */
   unsigned src_offset = bi_opcode_props[I->op].sr_read ? 1 : 0;

   for (unsigned i = 0; i < info.nr_srcs; ++i) {
      unsigned logical_i = (swap12 && i == 1) ? 2 : (swap12 && i == 2) ? 1 : i;

      struct va_src_info src_info = info.srcs[i];
      enum va_size size = src_info.size;

      bi_index src = I->src[logical_i + src_offset];
      hex |= (uint64_t)va_pack_src(I, logical_i + src_offset) << (8 * i);

      if (src_info.notted) {
         if (src.neg)
            hex |= (1ull << 35);
      } else if (src_info.absneg) {
         unsigned neg_offs = 32 + 2 + ((2 - i) * 2);
         unsigned abs_offs = 33 + 2 + ((2 - i) * 2);

         if (src.neg)
            hex |= 1ull << neg_offs;
         if (src.abs)
            hex |= 1ull << abs_offs;
      } else {
         if (src.neg)
            invalid_instruction(I, "negate");
         if (src.abs)
            invalid_instruction(I, "absolute value");
      }

      if (src_info.swizzle) {
         unsigned offs = 24 + ((2 - i) * 2);
         unsigned S = src.swizzle;
         pack_assert(I, size == VA_SIZE_16 || size == VA_SIZE_32);

         uint64_t v = (size == VA_SIZE_32 ? va_pack_widen_f32(I, S)
                                          : va_pack_swizzle_f16(I, S));
         hex |= v << offs;
      } else if (src_info.widen) {
         unsigned offs = (i == 1) ? 26 : 36;
         hex |= (uint64_t)va_pack_widen(I, src.swizzle, src_info.size) << offs;
      } else if (src_info.lane) {
         unsigned offs =
            (I->op == BI_OPCODE_MKVEC_V2I8) ? ((i == 0) ? 38 : 36) : 28;

         if (src_info.size == VA_SIZE_16) {
            hex |= (src.swizzle == BI_SWIZZLE_H11 ? 1 : 0) << offs;
         } else if (I->op == BI_OPCODE_BRANCHZ_I16) {
            hex |= ((uint64_t)va_pack_combine(I, src.swizzle) << 37);
         } else {
            pack_assert(I, src_info.size == VA_SIZE_8);
            unsigned comp = src.swizzle - BI_SWIZZLE_B0000;
            pack_assert(I, comp < 4);
            hex |= (uint64_t)comp << offs;
         }
      } else if (src_info.lanes) {
         pack_assert(I, src_info.size == VA_SIZE_8);
         pack_assert(I, i == 1);
         hex |= (uint64_t)va_pack_shift_lanes(I, src.swizzle) << 26;
      } else if (src_info.combine) {
         /* Treat as swizzle, subgroup ops not yet supported */
         pack_assert(I, src_info.size == VA_SIZE_32);
         pack_assert(I, i == 0);
         hex |= (uint64_t)va_pack_widen_f32(I, src.swizzle) << 37;
      } else if (src_info.halfswizzle) {
         pack_assert(I, src_info.size == VA_SIZE_8);
         pack_assert(I, i == 0);
         hex |= (uint64_t)va_pack_halfswizzle(I, src.swizzle) << 36;
      } else if (src.swizzle != BI_SWIZZLE_H01) {
         invalid_instruction(I, "swizzle");
      }
   }

   if (info.saturate)
      hex |= (uint64_t)I->saturate << 30;
   if (info.rhadd)
      hex |= va_pack_rhadd(I);
   if (info.clamp)
      hex |= (uint64_t)I->clamp << 32;
   if (info.round_mode)
      hex |= (uint64_t)I->round << 30;
   if (info.condition)
      hex |= (uint64_t)I->cmpf << 32;
   if (info.result_type)
      hex |= (uint64_t)I->result_type << 30;

   return hex;
}

static uint64_t
va_pack_byte_offset(const bi_instr *I)
{
   int16_t offset = I->byte_offset;
   if (offset != I->byte_offset)
      invalid_instruction(I, "byte offset");

   uint16_t offset_as_u16 = offset;
   return ((uint64_t)offset_as_u16) << 8;
}

static uint64_t
va_pack_byte_offset_8(const bi_instr *I)
{
   uint8_t offset = I->byte_offset;
   if (offset != I->byte_offset)
      invalid_instruction(I, "byte offset");

   return ((uint64_t)offset) << 8;
}

static uint64_t
va_pack_load(const bi_instr *I, bool buffer_descriptor)
{
   const uint8_t load_lane_identity[8] = {
      VA_LOAD_LANE_8_BIT_B0,        VA_LOAD_LANE_16_BIT_H0,
      VA_LOAD_LANE_24_BIT_IDENTITY, VA_LOAD_LANE_32_BIT_W0,
      VA_LOAD_LANE_48_BIT_IDENTITY, VA_LOAD_LANE_64_BIT_IDENTITY,
      VA_LOAD_LANE_96_BIT_IDENTITY, VA_LOAD_LANE_128_BIT_IDENTITY,
   };

   unsigned memory_size = (valhall_opcodes[I->op].exact >> 27) & 0x7;
   uint64_t hex = (uint64_t)load_lane_identity[memory_size] << 36;

   // unsigned
   hex |= (1ull << 39);

   if (!buffer_descriptor)
      hex |= va_pack_byte_offset(I);

   hex |= (uint64_t)va_pack_src(I, 0) << 0;

   if (buffer_descriptor)
      hex |= (uint64_t)va_pack_src(I, 1) << 8;

   return hex;
}

static uint64_t
va_pack_memory_access(const bi_instr *I)
{
   switch (I->seg) {
   case BI_SEG_TL:
      return VA_MEMORY_ACCESS_FORCE;
   case BI_SEG_POS:
      return VA_MEMORY_ACCESS_ISTREAM;
   case BI_SEG_VARY:
      return VA_MEMORY_ACCESS_ESTREAM;
   default:
      return VA_MEMORY_ACCESS_NONE;
   }
}

static uint64_t
va_pack_store(const bi_instr *I)
{
   uint64_t hex = va_pack_memory_access(I) << 24;

   va_validate_register_pair(I, 1);
   hex |= (uint64_t)va_pack_src(I, 1) << 0;

   hex |= va_pack_byte_offset(I);

   return hex;
}

static enum va_lod_mode
va_pack_lod_mode(const bi_instr *I)
{
   switch (I->va_lod_mode) {
   case BI_VA_LOD_MODE_ZERO_LOD:
      return VA_LOD_MODE_ZERO;
   case BI_VA_LOD_MODE_COMPUTED_LOD:
      return VA_LOD_MODE_COMPUTED;
   case BI_VA_LOD_MODE_EXPLICIT:
      return VA_LOD_MODE_EXPLICIT;
   case BI_VA_LOD_MODE_COMPUTED_BIAS:
      return VA_LOD_MODE_COMPUTED_BIAS;
   case BI_VA_LOD_MODE_GRDESC:
      return VA_LOD_MODE_GRDESC;
   }

   invalid_instruction(I, "LOD mode");
}

static enum va_register_type
va_pack_register_type(const bi_instr *I)
{
   switch (I->register_format) {
   case BI_REGISTER_FORMAT_F16:
   case BI_REGISTER_FORMAT_F32:
      return VA_REGISTER_TYPE_F;

   case BI_REGISTER_FORMAT_U16:
   case BI_REGISTER_FORMAT_U32:
      return VA_REGISTER_TYPE_U;

   case BI_REGISTER_FORMAT_S16:
   case BI_REGISTER_FORMAT_S32:
      return VA_REGISTER_TYPE_S;

   default:
      invalid_instruction(I, "register type");
   }
}

static enum va_register_format
va_pack_register_format(const bi_instr *I)
{
   switch (I->register_format) {
   case BI_REGISTER_FORMAT_AUTO:
      return VA_REGISTER_FORMAT_AUTO;
   case BI_REGISTER_FORMAT_F32:
      return VA_REGISTER_FORMAT_F32;
   case BI_REGISTER_FORMAT_F16:
      return VA_REGISTER_FORMAT_F16;
   case BI_REGISTER_FORMAT_S32:
      return VA_REGISTER_FORMAT_S32;
   case BI_REGISTER_FORMAT_S16:
      return VA_REGISTER_FORMAT_S16;
   case BI_REGISTER_FORMAT_U32:
      return VA_REGISTER_FORMAT_U32;
   case BI_REGISTER_FORMAT_U16:
      return VA_REGISTER_FORMAT_U16;
   default:
      invalid_instruction(I, "register format");
   }
}

uint64_t
va_pack_instr(const bi_instr *I)
{
   struct va_opcode_info info = valhall_opcodes[I->op];

   uint64_t hex = info.exact | (((uint64_t)I->flow) << 59);
   hex |= ((uint64_t)va_select_fau_page(I)) << 57;

   if (info.slot)
      hex |= ((uint64_t)I->slot << 30);

   if (info.sr_count) {
      bool read = bi_opcode_props[I->op].sr_read;
      bi_index sr = read ? I->src[0] : I->dest[0];

      unsigned count =
         read ? bi_count_read_registers(I, 0) : bi_count_write_registers(I, 0);

      hex |= ((uint64_t)count << 33);
      hex |= (uint64_t)va_pack_reg(I, sr) << 40;
      hex |= ((uint64_t)info.sr_control << 46);
   }

   if (info.sr_write_count) {
      hex |= ((uint64_t)bi_count_write_registers(I, 0) - 1) << 36;
      hex |= ((uint64_t)va_pack_reg(I, I->dest[0])) << 16;
   }

   if (info.vecsize)
      hex |= ((uint64_t)I->vecsize << 28);

   if (info.register_format)
      hex |= ((uint64_t)va_pack_register_format(I)) << 24;

   switch (I->op) {
   case BI_OPCODE_LOAD_I8:
   case BI_OPCODE_LOAD_I16:
   case BI_OPCODE_LOAD_I24:
   case BI_OPCODE_LOAD_I32:
   case BI_OPCODE_LOAD_I48:
   case BI_OPCODE_LOAD_I64:
   case BI_OPCODE_LOAD_I96:
   case BI_OPCODE_LOAD_I128:
      hex |= va_pack_load(I, false);
      break;

   case BI_OPCODE_LD_BUFFER_I8:
   case BI_OPCODE_LD_BUFFER_I16:
   case BI_OPCODE_LD_BUFFER_I24:
   case BI_OPCODE_LD_BUFFER_I32:
   case BI_OPCODE_LD_BUFFER_I48:
   case BI_OPCODE_LD_BUFFER_I64:
   case BI_OPCODE_LD_BUFFER_I96:
   case BI_OPCODE_LD_BUFFER_I128:
      hex |= va_pack_load(I, true);
      break;

   case BI_OPCODE_STORE_I8:
   case BI_OPCODE_STORE_I16:
   case BI_OPCODE_STORE_I24:
   case BI_OPCODE_STORE_I32:
   case BI_OPCODE_STORE_I48:
   case BI_OPCODE_STORE_I64:
   case BI_OPCODE_STORE_I96:
   case BI_OPCODE_STORE_I128:
      hex |= va_pack_store(I);
      break;

   case BI_OPCODE_ATOM1_RETURN_I32:
      /* Permit omitting the destination for plain ATOM1 */
      if (!bi_count_write_registers(I, 0)) {
         hex |= (0x40ull << 40); // fake read
      }

      /* 64-bit source */
      va_validate_register_pair(I, 0);
      hex |= (uint64_t)va_pack_src(I, 0) << 0;
      hex |= va_pack_byte_offset_8(I);
      hex |= ((uint64_t)va_pack_atom_opc_1(I)) << 22;
      break;

   case BI_OPCODE_ATOM_I32:
   case BI_OPCODE_ATOM_RETURN_I32:
      /* 64-bit source */
      va_validate_register_pair(I, 1);
      hex |= (uint64_t)va_pack_src(I, 1) << 0;
      hex |= va_pack_byte_offset_8(I);
      hex |= ((uint64_t)va_pack_atom_opc(I)) << 22;

      if (I->op == BI_OPCODE_ATOM_RETURN_I32)
         hex |= (0xc0ull << 40); // flags

      if (I->atom_opc == BI_ATOM_OPC_ACMPXCHG)
         hex |= (1 << 26); /* .compare */

      break;

   case BI_OPCODE_ST_CVT:
      /* Staging read */
      hex |= va_pack_store(I);

      /* Conversion descriptor */
      hex |= (uint64_t)va_pack_src(I, 3) << 16;
      break;

   case BI_OPCODE_BLEND: {
      /* Source 0 - Blend descriptor (64-bit) */
      hex |= ((uint64_t)va_pack_src(I, 2)) << 0;
      va_validate_register_pair(I, 2);

      /* Target */
      if (I->branch_offset & 0x7)
         invalid_instruction(I, "unaligned branch");
      hex |= ((I->branch_offset >> 3) << 8);

      /* Source 2 - coverage mask */
      hex |= ((uint64_t)va_pack_reg(I, I->src[1])) << 16;

      /* Vector size */
      unsigned vecsize = 4;
      hex |= ((uint64_t)(vecsize - 1) << 28);

      break;
   }

   case BI_OPCODE_TEX_SINGLE:
   case BI_OPCODE_TEX_FETCH:
   case BI_OPCODE_TEX_GATHER: {
      /* Image to read from */
      hex |= ((uint64_t)va_pack_src(I, 1)) << 0;

      if (I->op == BI_OPCODE_TEX_FETCH && I->shadow)
         invalid_instruction(I, "TEX_FETCH does not support .shadow");

      if (I->array_enable)
         hex |= (1ull << 10);
      if (I->texel_offset)
         hex |= (1ull << 11);
      if (I->shadow)
         hex |= (1ull << 12);
      if (I->skip)
         hex |= (1ull << 39);
      if (!bi_is_regfmt_16(I->register_format))
         hex |= (1ull << 46);

      if (I->op == BI_OPCODE_TEX_SINGLE)
         hex |= ((uint64_t)va_pack_lod_mode(I)) << 13;

      if (I->op == BI_OPCODE_TEX_GATHER) {
         if (I->integer_coordinates)
            hex |= (1 << 13);
         hex |= ((uint64_t)I->fetch_component) << 14;
      }

      hex |= (I->write_mask << 22);
      hex |= ((uint64_t)va_pack_register_type(I)) << 26;
      hex |= ((uint64_t)I->dimension) << 28;

      break;
   }

   default:
      if (!info.exact && I->op != BI_OPCODE_NOP)
         invalid_instruction(I, "opcode");

      hex |= va_pack_alu(I);
      break;
   }

   return hex;
}

static unsigned
va_instructions_in_block(bi_block *block)
{
   unsigned offset = 0;

   bi_foreach_instr_in_block(block, _) {
      offset++;
   }

   return offset;
}

/* Calculate branch_offset from a branch_target for a direct relative branch */

static void
va_lower_branch_target(bi_context *ctx, bi_block *start, bi_instr *I)
{
   /* Precondition: unlowered relative branch */
   bi_block *target = I->branch_target;
   assert(target != NULL);

   /* Signed since we might jump backwards */
   signed offset = 0;

   /* Determine if the target block is strictly greater in source order */
   bool forwards = target->index > start->index;

   if (forwards) {
      /* We have to jump through this block */
      bi_foreach_instr_in_block_from(start, _, I) {
         offset++;
      }

      /* We then need to jump over every following block until the target */
      bi_foreach_block_from(ctx, start, blk) {
         /* End just before the target */
         if (blk == target)
            break;

         /* Count other blocks */
         if (blk != start)
            offset += va_instructions_in_block(blk);
      }
   } else {
      /* Jump through the beginning of this block */
      bi_foreach_instr_in_block_from_rev(start, ins, I) {
         if (ins != I)
            offset--;
      }

      /* Jump over preceding blocks up to and including the target to get to
       * the beginning of the target */
      bi_foreach_block_from_rev(ctx, start, blk) {
         if (blk == start)
            continue;

         offset -= va_instructions_in_block(blk);

         /* End just after the target */
         if (blk == target)
            break;
      }
   }

   /* Offset is relative to the next instruction, so bias */
   offset--;

   /* Update the instruction */
   I->branch_offset = offset;
}

/*
 * Late lowering to insert blend shader calls after BLEND instructions. Required
 * to support blend shaders, so this pass may be omitted if it is known that
 * blend shaders are never used.
 *
 * This lowering runs late because it introduces control flow changes without
 * modifying the control flow graph. It hardcodes registers, meaning running
 * after RA makes sense. Finally, it hardcodes a manually sized instruction
 * sequence, requiring it to run after scheduling.
 *
 * As it is Valhall specific, running it as a pre-pack lowering is sensible.
 */
static void
va_lower_blend(bi_context *ctx)
{
   /* Program counter for *next* instruction */
   bi_index pc = bi_fau(BIR_FAU_PROGRAM_COUNTER, false);

   bi_foreach_instr_global_safe(ctx, I) {
      if (I->op != BI_OPCODE_BLEND)
         continue;

      bi_builder b = bi_init_builder(ctx, bi_after_instr(I));

      unsigned prolog_length = 2 * 8;

      /* By ABI, r48 is the link register shared with blend shaders */
      assert(bi_is_equiv(I->dest[0], bi_register(48)));

      if (I->flow == VA_FLOW_END)
         bi_iadd_imm_i32_to(&b, I->dest[0], va_zero_lut(), 0);
      else
         bi_iadd_imm_i32_to(&b, I->dest[0], pc, prolog_length - 8);

      bi_branchzi(&b, va_zero_lut(), I->src[3], BI_CMPF_EQ);

      /* For fixed function: skip the prologue, or return */
      if (I->flow != VA_FLOW_END)
         I->branch_offset = prolog_length;
   }
}

void
bi_pack_valhall(bi_context *ctx, struct util_dynarray *emission)
{
   unsigned orig_size = emission->size;

   va_validate(stderr, ctx);

   /* Late lowering */
   if (ctx->stage == MESA_SHADER_FRAGMENT && !ctx->inputs->is_blend)
      va_lower_blend(ctx);

   bi_foreach_block(ctx, block) {
      bi_foreach_instr_in_block(block, I) {
         if (I->op == BI_OPCODE_BRANCHZ_I16)
            va_lower_branch_target(ctx, block, I);

         uint64_t hex = va_pack_instr(I);
         util_dynarray_append(emission, uint64_t, hex);
      }
   }

   /* Pad with zeroes, but keep empty programs empty so they may be omitted
    * altogether. Failing to do this would result in a program containing only
    * zeroes, which is invalid and will raise an encoding fault.
    *
    * Pad an extra 16 byte (one instruction) to separate primary and secondary
    * shader disassembles. This is not strictly necessary, but it's a good
    * practice. 128 bytes is the optimal program alignment on Trym, so pad
    * secondary shaders up to 128 bytes. This may help the instruction cache.
    */
   if (orig_size != emission->size) {
      unsigned aligned = ALIGN_POT(emission->size + 16, 128);
      unsigned count = aligned - emission->size;

      memset(util_dynarray_grow(emission, uint8_t, count), 0, count);
   }
}
