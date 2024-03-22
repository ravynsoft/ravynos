/*
 * Copyright © 2019 Intel Corporation
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

#ifndef MI_BUILDER_H
#define MI_BUILDER_H

#include "dev/intel_device_info.h"
#include "genxml/genX_bits.h"
#include "util/bitscan.h"
#include "util/fast_idiv_by_const.h"
#include "util/u_math.h"

#ifndef MI_BUILDER_NUM_ALLOC_GPRS
/** The number of GPRs the MI builder is allowed to allocate
 *
 * This may be set by a user of this API so that it can reserve some GPRs at
 * the top end for its own use.
 */
#define MI_BUILDER_NUM_ALLOC_GPRS 16
#endif

/** These must be defined by the user of the builder
 *
 * void *__gen_get_batch_dwords(__gen_user_data *user_data,
 *                              unsigned num_dwords);
 *
 * __gen_address_type
 * __gen_address_offset(__gen_address_type addr, uint64_t offset);
 *
 *
 * If self-modifying batches are supported, we must be able to pass batch
 * addresses around as void*s so pinning as well as batch chaining or some
 * other mechanism for ensuring batch pointers remain valid during building is
 * required. The following function must also be defined, it returns an
 * address in canonical form:
 *
 * __gen_address_type
 * __gen_get_batch_address(__gen_user_data *user_data, void *location);
 *
 * Also, __gen_combine_address must accept a location value of NULL and return
 * a fully valid 64-bit address.
 */

/*
 * Start of the actual MI builder
 */

#define __genxml_cmd_length(cmd) cmd ## _length
#define __genxml_cmd_header(cmd) cmd ## _header
#define __genxml_cmd_pack(cmd) cmd ## _pack

#define mi_builder_pack(b, cmd, dst, name)                          \
   for (struct cmd name = { __genxml_cmd_header(cmd) },                 \
        *_dst = (struct cmd *)(dst); __builtin_expect(_dst != NULL, 1); \
        __genxml_cmd_pack(cmd)((b)->user_data, (void *)_dst, &name),    \
        _dst = NULL)

#define mi_builder_emit(b, cmd, name)                               \
   mi_builder_pack((b), cmd, __gen_get_batch_dwords((b)->user_data, __genxml_cmd_length(cmd)), name)


enum mi_value_type {
   MI_VALUE_TYPE_IMM,
   MI_VALUE_TYPE_MEM32,
   MI_VALUE_TYPE_MEM64,
   MI_VALUE_TYPE_REG32,
   MI_VALUE_TYPE_REG64,
};

struct mi_value {
   enum mi_value_type type;

   union {
      uint64_t imm;
      __gen_address_type addr;
      uint32_t reg;
   };

#if GFX_VERx10 >= 75
   bool invert;
#endif
};

struct mi_reg_num {
   uint32_t num;
#if GFX_VER >= 11
   bool cs;
#endif
};

static inline struct mi_reg_num
mi_adjust_reg_num(uint32_t reg)
{
#if GFX_VER >= 11
   bool cs = reg >= 0x2000 && reg < 0x4000;
   return (struct mi_reg_num) {
      .num = reg - (cs ? 0x2000 : 0),
      .cs = cs,
   };
#else
   return (struct mi_reg_num) { .num = reg, };
#endif
}

#if GFX_VER >= 9
#define MI_BUILDER_MAX_MATH_DWORDS 256
#else
#define MI_BUILDER_MAX_MATH_DWORDS 64
#endif

struct mi_builder {
   const struct intel_device_info *devinfo;
   __gen_user_data *user_data;

#if GFX_VERx10 >= 75
   uint32_t gprs;
   uint8_t gpr_refs[MI_BUILDER_NUM_ALLOC_GPRS];

   unsigned num_math_dwords;
   uint32_t math_dwords[MI_BUILDER_MAX_MATH_DWORDS];
#endif

#if GFX_VERx10 >= 125
   uint32_t mocs;
#endif
};

static inline void
mi_builder_init(struct mi_builder *b,
                const struct intel_device_info *devinfo,
                __gen_user_data *user_data)
{
   memset(b, 0, sizeof(*b));
   b->devinfo = devinfo;
   b->user_data = user_data;

#if GFX_VERx10 >= 75
   b->gprs = 0;
   b->num_math_dwords = 0;
#endif
}

static inline void
mi_builder_flush_math(struct mi_builder *b)
{
#if GFX_VERx10 >= 75
   if (b->num_math_dwords == 0)
      return;

   uint32_t *dw = (uint32_t *)__gen_get_batch_dwords(b->user_data,
                                                     1 + b->num_math_dwords);
   mi_builder_pack(b, GENX(MI_MATH), dw, math) {
#if GFX_VERx10 >= 125
      math.MOCS = b->mocs;
#endif
      math.DWordLength = 1 + b->num_math_dwords - GENX(MI_MATH_length_bias);
   }
   memcpy(dw + 1, b->math_dwords, b->num_math_dwords * sizeof(uint32_t));
   b->num_math_dwords = 0;
#endif
}

/**
 * Set mocs index to mi_build
 *
 * This is required when a MI_MATH instruction will be emitted and
 * the code is used in GFX 12.5 or newer.
 */
static inline void
mi_builder_set_mocs(UNUSED struct mi_builder *b, UNUSED uint32_t mocs)
{
#if GFX_VERx10 >= 125
   if (b->mocs != 0 && b->mocs != mocs)
      mi_builder_flush_math(b);
   b->mocs = mocs;
#endif
}

#define _MI_BUILDER_GPR_BASE 0x2600
/* The actual hardware limit on GPRs */
#define _MI_BUILDER_NUM_HW_GPRS 16

#if GFX_VERx10 >= 75

static inline bool
mi_value_is_reg(struct mi_value val)
{
   return val.type == MI_VALUE_TYPE_REG32 ||
          val.type == MI_VALUE_TYPE_REG64;
}

static inline bool
mi_value_is_gpr(struct mi_value val)
{
   return mi_value_is_reg(val) &&
          val.reg >= _MI_BUILDER_GPR_BASE &&
          val.reg < _MI_BUILDER_GPR_BASE +
                    _MI_BUILDER_NUM_HW_GPRS * 8;
}

static inline bool
_mi_value_is_allocated_gpr(struct mi_value val)
{
   return mi_value_is_reg(val) &&
          val.reg >= _MI_BUILDER_GPR_BASE &&
          val.reg < _MI_BUILDER_GPR_BASE +
                    MI_BUILDER_NUM_ALLOC_GPRS * 8;
}

static inline uint32_t
_mi_value_as_gpr(struct mi_value val)
{
   assert(mi_value_is_gpr(val));
   /* Some of the GRL metakernels will generate 64bit value in a GP register,
    * then use only half of that as the last operation on that value. So allow
    * unref on part of a GP register.
    */
   assert(val.reg % 4 == 0);
   return (val.reg - _MI_BUILDER_GPR_BASE) / 8;
}

static inline struct mi_value
mi_new_gpr(struct mi_builder *b)
{
   unsigned gpr = ffs(~b->gprs) - 1;
   assert(gpr < MI_BUILDER_NUM_ALLOC_GPRS);
   assert(b->gpr_refs[gpr] == 0);
   b->gprs |= (1u << gpr);
   b->gpr_refs[gpr] = 1;

   return (struct mi_value) {
      .type = MI_VALUE_TYPE_REG64,
      .reg = _MI_BUILDER_GPR_BASE + gpr * 8,
   };
}

static inline struct mi_value
mi_reserve_gpr(struct mi_builder *b, unsigned gpr)
{
   assert(gpr < MI_BUILDER_NUM_ALLOC_GPRS);
   assert(!(b->gprs & (1 << gpr)));
   assert(b->gpr_refs[gpr] == 0);
   b->gprs |= (1u << gpr);
   b->gpr_refs[gpr] = 128; /* Enough that we won't unref it */

   return (struct mi_value) {
      .type = MI_VALUE_TYPE_REG64,
      .reg = _MI_BUILDER_GPR_BASE + gpr * 8,
   };
}
#endif /* GFX_VERx10 >= 75 */

/** Take a reference to a mi_value
 *
 * The MI builder uses reference counting to automatically free ALU GPRs for
 * re-use in calculations.  All mi_* math functions consume the reference
 * they are handed for each source and return a reference to a value which the
 * caller must consume.  In particular, if you pas the same value into a
 * single mi_* math function twice (say to add a number to itself), you
 * are responsible for calling mi_value_ref() to get a second reference
 * because the mi_* math function will consume it twice.
 */
static inline void
mi_value_add_refs(struct mi_builder *b, struct mi_value val, unsigned num_refs)
{
#if GFX_VERx10 >= 75
   if (_mi_value_is_allocated_gpr(val)) {
      unsigned gpr = _mi_value_as_gpr(val);
      assert(gpr < MI_BUILDER_NUM_ALLOC_GPRS);
      assert(b->gprs & (1u << gpr));
      assert(b->gpr_refs[gpr] < UINT8_MAX);
      b->gpr_refs[gpr] += num_refs;
   }
#endif /* GFX_VERx10 >= 75 */
}

static inline struct mi_value
mi_value_ref(struct mi_builder *b, struct mi_value val)
{
   mi_value_add_refs(b, val, 1);
   return val;
}


/** Drop a reference to a mi_value
 *
 * See also mi_value_ref.
 */
static inline void
mi_value_unref(struct mi_builder *b, struct mi_value val)
{
#if GFX_VERx10 >= 75
   if (_mi_value_is_allocated_gpr(val)) {
      unsigned gpr = _mi_value_as_gpr(val);
      assert(gpr < MI_BUILDER_NUM_ALLOC_GPRS);
      assert(b->gprs & (1u << gpr));
      assert(b->gpr_refs[gpr] > 0);
      if (--b->gpr_refs[gpr] == 0)
         b->gprs &= ~(1u << gpr);
   }
#endif /* GFX_VERx10 >= 75 */
}

static inline struct mi_value
mi_imm(uint64_t imm)
{
   return (struct mi_value) {
      .type = MI_VALUE_TYPE_IMM,
      .imm = imm,
   };
}

static inline struct mi_value
mi_reg32(uint32_t reg)
{
   struct mi_value val = {
      .type = MI_VALUE_TYPE_REG32,
      .reg = reg,
   };
#if GFX_VERx10 >= 75
   assert(!_mi_value_is_allocated_gpr(val));
#endif
   return val;
}

static inline struct mi_value
mi_reg64(uint32_t reg)
{
   struct mi_value val = {
      .type = MI_VALUE_TYPE_REG64,
      .reg = reg,
   };
#if GFX_VERx10 >= 75
   assert(!_mi_value_is_allocated_gpr(val));
#endif
   return val;
}

static inline struct mi_value
mi_mem32(__gen_address_type addr)
{
   return (struct mi_value) {
      .type = MI_VALUE_TYPE_MEM32,
      .addr = addr,
   };
}

static inline struct mi_value
mi_mem64(__gen_address_type addr)
{
   return (struct mi_value) {
      .type = MI_VALUE_TYPE_MEM64,
      .addr = addr,
   };
}

static inline struct mi_value
mi_value_half(struct mi_value value, bool top_32_bits)
{
   switch (value.type) {
   case MI_VALUE_TYPE_IMM:
      if (top_32_bits)
         value.imm >>= 32;
      else
         value.imm &= 0xffffffffu;
      return value;

   case MI_VALUE_TYPE_MEM32:
      assert(!top_32_bits);
      return value;

   case MI_VALUE_TYPE_MEM64:
      if (top_32_bits)
         value.addr = __gen_address_offset(value.addr, 4);
      value.type = MI_VALUE_TYPE_MEM32;
      return value;

   case MI_VALUE_TYPE_REG32:
      assert(!top_32_bits);
      return value;

   case MI_VALUE_TYPE_REG64:
      if (top_32_bits)
         value.reg += 4;
      value.type = MI_VALUE_TYPE_REG32;
      return value;
   }

   unreachable("Invalid mi_value type");
}

static inline void
_mi_copy_no_unref(struct mi_builder *b,
                  struct mi_value dst, struct mi_value src)
{
#if GFX_VERx10 >= 75
   /* TODO: We could handle src.invert by emitting a bit of math if we really
    * wanted to.
    */
   assert(!dst.invert && !src.invert);
#endif
   mi_builder_flush_math(b);

   switch (dst.type) {
   case MI_VALUE_TYPE_IMM:
      unreachable("Cannot copy to an immediate");

   case MI_VALUE_TYPE_MEM64:
   case MI_VALUE_TYPE_REG64:
      switch (src.type) {
      case MI_VALUE_TYPE_IMM:
         if (dst.type == MI_VALUE_TYPE_REG64) {
            uint32_t *dw = (uint32_t *)__gen_get_batch_dwords(b->user_data,
                                                              GENX(MI_LOAD_REGISTER_IMM_length) + 2);
            struct mi_reg_num reg = mi_adjust_reg_num(dst.reg);
            mi_builder_pack(b, GENX(MI_LOAD_REGISTER_IMM), dw, lri) {
               lri.DWordLength = GENX(MI_LOAD_REGISTER_IMM_length) + 2 -
                                 GENX(MI_LOAD_REGISTER_IMM_length_bias);
#if GFX_VER >= 11
               lri.AddCSMMIOStartOffset = reg.cs;
#endif
            }
            dw[1] = reg.num;
            dw[2] = src.imm;
            dw[3] = reg.num + 4;
            dw[4] = src.imm >> 32;
         } else {
#if GFX_VER >= 8
            assert(dst.type == MI_VALUE_TYPE_MEM64);
            uint32_t *dw = (uint32_t *)__gen_get_batch_dwords(b->user_data,
                                                              GENX(MI_STORE_DATA_IMM_length) + 1);
            mi_builder_pack(b, GENX(MI_STORE_DATA_IMM), dw, sdm) {
               sdm.DWordLength = GENX(MI_STORE_DATA_IMM_length) + 1 -
                                 GENX(MI_STORE_DATA_IMM_length_bias);
               sdm.StoreQword = true;
               sdm.Address = dst.addr;
            }
            dw[3] = src.imm;
            dw[4] = src.imm >> 32;
#else
         _mi_copy_no_unref(b, mi_value_half(dst, false),
                              mi_value_half(src, false));
         _mi_copy_no_unref(b, mi_value_half(dst, true),
                              mi_value_half(src, true));
#endif
         }
         break;
      case MI_VALUE_TYPE_REG32:
      case MI_VALUE_TYPE_MEM32:
         _mi_copy_no_unref(b, mi_value_half(dst, false),
                              mi_value_half(src, false));
         _mi_copy_no_unref(b, mi_value_half(dst, true),
                              mi_imm(0));
         break;
      case MI_VALUE_TYPE_REG64:
      case MI_VALUE_TYPE_MEM64:
         _mi_copy_no_unref(b, mi_value_half(dst, false),
                              mi_value_half(src, false));
         _mi_copy_no_unref(b, mi_value_half(dst, true),
                              mi_value_half(src, true));
         break;
      default:
         unreachable("Invalid mi_value type");
      }
      break;

   case MI_VALUE_TYPE_MEM32:
      switch (src.type) {
      case MI_VALUE_TYPE_IMM:
         mi_builder_emit(b, GENX(MI_STORE_DATA_IMM), sdi) {
            sdi.Address = dst.addr;
#if GFX_VER >= 12
            sdi.ForceWriteCompletionCheck = true;
#endif
            sdi.ImmediateData = src.imm;
         }
         break;

      case MI_VALUE_TYPE_MEM32:
      case MI_VALUE_TYPE_MEM64:
#if GFX_VER >= 8
         mi_builder_emit(b, GENX(MI_COPY_MEM_MEM), cmm) {
            cmm.DestinationMemoryAddress = dst.addr;
            cmm.SourceMemoryAddress = src.addr;
         }
#elif GFX_VERx10 == 75
         {
            struct mi_value tmp = mi_new_gpr(b);
            _mi_copy_no_unref(b, tmp, src);
            _mi_copy_no_unref(b, dst, tmp);
            mi_value_unref(b, tmp);
         }
#else
         unreachable("Cannot do mem <-> mem copy on IVB and earlier");
#endif
         break;

      case MI_VALUE_TYPE_REG32:
      case MI_VALUE_TYPE_REG64:
         mi_builder_emit(b, GENX(MI_STORE_REGISTER_MEM), srm) {
            struct mi_reg_num reg = mi_adjust_reg_num(src.reg);
            srm.RegisterAddress = reg.num;
#if GFX_VER >= 11
            srm.AddCSMMIOStartOffset = reg.cs;
#endif
            srm.MemoryAddress = dst.addr;
         }
         break;

      default:
         unreachable("Invalid mi_value type");
      }
      break;

   case MI_VALUE_TYPE_REG32:
      switch (src.type) {
      case MI_VALUE_TYPE_IMM:
         mi_builder_emit(b, GENX(MI_LOAD_REGISTER_IMM), lri) {
            struct mi_reg_num reg = mi_adjust_reg_num(dst.reg);
            lri.RegisterOffset = reg.num;
#if GFX_VER >= 11
            lri.AddCSMMIOStartOffset = reg.cs;
#endif
            lri.DataDWord = src.imm;
         }
         break;

      case MI_VALUE_TYPE_MEM32:
      case MI_VALUE_TYPE_MEM64:
#if GFX_VER >= 7
         mi_builder_emit(b, GENX(MI_LOAD_REGISTER_MEM), lrm) {
            struct mi_reg_num reg = mi_adjust_reg_num(dst.reg);
            lrm.RegisterAddress = reg.num;
#if GFX_VER >= 11
            lrm.AddCSMMIOStartOffset = reg.cs;
#endif
            lrm.MemoryAddress = src.addr;
         }
#else
         unreachable("Cannot load do mem -> reg copy on SNB and earlier");
#endif
         break;

      case MI_VALUE_TYPE_REG32:
      case MI_VALUE_TYPE_REG64:
#if GFX_VERx10 >= 75
         if (src.reg != dst.reg) {
            mi_builder_emit(b, GENX(MI_LOAD_REGISTER_REG), lrr) {
               struct mi_reg_num reg = mi_adjust_reg_num(src.reg);
               lrr.SourceRegisterAddress = reg.num;
#if GFX_VER >= 11
               lrr.AddCSMMIOStartOffsetSource = reg.cs;
#endif
               reg = mi_adjust_reg_num(dst.reg);
               lrr.DestinationRegisterAddress = reg.num;
#if GFX_VER >= 11
               lrr.AddCSMMIOStartOffsetDestination = reg.cs;
#endif
            }
         }
#else
         unreachable("Cannot do reg <-> reg copy on IVB and earlier");
#endif
         break;

      default:
         unreachable("Invalid mi_value type");
      }
      break;

   default:
      unreachable("Invalid mi_value type");
   }
}

#if GFX_VERx10 >= 75
static inline struct mi_value
mi_resolve_invert(struct mi_builder *b, struct mi_value src);
#endif

/** Store the value in src to the value represented by dst
 *
 * If the bit size of src and dst mismatch, this function does an unsigned
 * integer cast.  If src has more bits than dst, it takes the bottom bits.  If
 * src has fewer bits then dst, it fills the top bits with zeros.
 *
 * This function consumes one reference for each of src and dst.
 */
static inline void
mi_store(struct mi_builder *b, struct mi_value dst, struct mi_value src)
{
#if GFX_VERx10 >= 75
   src = mi_resolve_invert(b, src);
#endif
   _mi_copy_no_unref(b, dst, src);
   mi_value_unref(b, src);
   mi_value_unref(b, dst);
}

static inline void
mi_memset(struct mi_builder *b, __gen_address_type dst,
          uint32_t value, uint32_t size)
{
#if GFX_VERx10 >= 75
   assert(b->num_math_dwords == 0);
#endif

   /* This memset operates in units of dwords. */
   assert(size % 4 == 0);

   for (uint32_t i = 0; i < size; i += 4) {
      mi_store(b, mi_mem32(__gen_address_offset(dst, i)),
                      mi_imm(value));
   }
}

/* NOTE: On IVB, this function stomps GFX7_3DPRIM_BASE_VERTEX */
static inline void
mi_memcpy(struct mi_builder *b, __gen_address_type dst,
          __gen_address_type src, uint32_t size)
{
#if GFX_VERx10 >= 75
   assert(b->num_math_dwords == 0);
#endif

   /* This memcpy operates in units of dwords. */
   assert(size % 4 == 0);

   for (uint32_t i = 0; i < size; i += 4) {
      struct mi_value dst_val = mi_mem32(__gen_address_offset(dst, i));
      struct mi_value src_val = mi_mem32(__gen_address_offset(src, i));
#if GFX_VERx10 >= 75
      mi_store(b, dst_val, src_val);
#else
      /* IVB does not have a general purpose register for command streamer
       * commands. Therefore, we use an alternate temporary register.
       */
      struct mi_value tmp_reg = mi_reg32(0x2440); /* GFX7_3DPRIM_BASE_VERTEX */
      mi_store(b, tmp_reg, src_val);
      mi_store(b, dst_val, tmp_reg);
#endif
   }
}

/*
 * MI_MATH Section.  Only available on Haswell+
 */

#if GFX_VERx10 >= 75

/**
 * Perform a predicated store (assuming the condition is already loaded
 * in the MI_PREDICATE_RESULT register) of the value in src to the memory
 * location specified by dst.  Non-memory destinations are not supported.
 *
 * This function consumes one reference for each of src and dst.
 */
static inline void
mi_store_if(struct mi_builder *b, struct mi_value dst, struct mi_value src)
{
   assert(!dst.invert && !src.invert);

   mi_builder_flush_math(b);

   /* We can only predicate MI_STORE_REGISTER_MEM, so restrict the
    * destination to be memory, and resolve the source to a temporary
    * register if it isn't in one already.
    */
   assert(dst.type == MI_VALUE_TYPE_MEM64 ||
          dst.type == MI_VALUE_TYPE_MEM32);

   if (src.type != MI_VALUE_TYPE_REG32 &&
       src.type != MI_VALUE_TYPE_REG64) {
      struct mi_value tmp = mi_new_gpr(b);
      _mi_copy_no_unref(b, tmp, src);
      src = tmp;
   }

   if (dst.type == MI_VALUE_TYPE_MEM64) {
      mi_builder_emit(b, GENX(MI_STORE_REGISTER_MEM), srm) {
         struct mi_reg_num reg = mi_adjust_reg_num(src.reg);
         srm.RegisterAddress = reg.num;
#if GFX_VER >= 11
         srm.AddCSMMIOStartOffset = reg.cs;
#endif
         srm.MemoryAddress = dst.addr;
         srm.PredicateEnable = true;
      }
      mi_builder_emit(b, GENX(MI_STORE_REGISTER_MEM), srm) {
         struct mi_reg_num reg = mi_adjust_reg_num(src.reg + 4);
         srm.RegisterAddress = reg.num;
#if GFX_VER >= 11
         srm.AddCSMMIOStartOffset = reg.cs;
#endif
         srm.MemoryAddress = __gen_address_offset(dst.addr, 4);
         srm.PredicateEnable = true;
      }
   } else {
      mi_builder_emit(b, GENX(MI_STORE_REGISTER_MEM), srm) {
         struct mi_reg_num reg = mi_adjust_reg_num(src.reg);
         srm.RegisterAddress = reg.num;
#if GFX_VER >= 11
         srm.AddCSMMIOStartOffset = reg.cs;
#endif
         srm.MemoryAddress = dst.addr;
         srm.PredicateEnable = true;
      }
   }

   mi_value_unref(b, src);
   mi_value_unref(b, dst);
}

static inline void
_mi_builder_push_math(struct mi_builder *b,
                      const uint32_t *dwords,
                      unsigned num_dwords)
{
   assert(num_dwords < MI_BUILDER_MAX_MATH_DWORDS);
   if (b->num_math_dwords + num_dwords > MI_BUILDER_MAX_MATH_DWORDS)
      mi_builder_flush_math(b);

   memcpy(&b->math_dwords[b->num_math_dwords],
          dwords, num_dwords * sizeof(*dwords));
   b->num_math_dwords += num_dwords;
}

static inline uint32_t
_mi_pack_alu(uint32_t opcode, uint32_t operand1, uint32_t operand2)
{
   struct GENX(MI_MATH_ALU_INSTRUCTION) instr = {
      .Operand2 = operand2,
      .Operand1 = operand1,
      .ALUOpcode = opcode,
   };

   uint32_t dw;
   GENX(MI_MATH_ALU_INSTRUCTION_pack)(NULL, &dw, &instr);

   return dw;
}

static inline struct mi_value
mi_value_to_gpr(struct mi_builder *b, struct mi_value val)
{
   if (mi_value_is_gpr(val))
      return val;

   /* Save off the invert flag because it makes copy() grumpy */
   bool invert = val.invert;
   val.invert = false;

   struct mi_value tmp = mi_new_gpr(b);
   _mi_copy_no_unref(b, tmp, val);
   tmp.invert = invert;

   return tmp;
}

static inline uint64_t
mi_value_to_u64(struct mi_value val)
{
   assert(val.type == MI_VALUE_TYPE_IMM);
   return val.invert ? ~val.imm : val.imm;
}

static inline uint32_t
_mi_math_load_src(struct mi_builder *b, unsigned src, struct mi_value *val)
{
   if (val->type == MI_VALUE_TYPE_IMM &&
       (val->imm == 0 || val->imm == UINT64_MAX)) {
      uint64_t imm = val->invert ? ~val->imm : val->imm;
      return _mi_pack_alu(imm ? MI_ALU_LOAD1 : MI_ALU_LOAD0, src, 0);
   } else {
      *val = mi_value_to_gpr(b, *val);
      return _mi_pack_alu(val->invert ? MI_ALU_LOADINV : MI_ALU_LOAD,
                          src, _mi_value_as_gpr(*val));
   }
}

static inline struct mi_value
mi_math_binop(struct mi_builder *b, uint32_t opcode,
              struct mi_value src0, struct mi_value src1,
              uint32_t store_op, uint32_t store_src)
{
   struct mi_value dst = mi_new_gpr(b);

   uint32_t dw[4];
   dw[0] = _mi_math_load_src(b, MI_ALU_SRCA, &src0);
   dw[1] = _mi_math_load_src(b, MI_ALU_SRCB, &src1);
   dw[2] = _mi_pack_alu(opcode, 0, 0);
   dw[3] = _mi_pack_alu(store_op, _mi_value_as_gpr(dst), store_src);
   _mi_builder_push_math(b, dw, 4);

   mi_value_unref(b, src0);
   mi_value_unref(b, src1);

   return dst;
}

static inline struct mi_value
mi_inot(struct mi_builder *b, struct mi_value val)
{
   if (val.type == MI_VALUE_TYPE_IMM)
      return mi_imm(~mi_value_to_u64(val));

   val.invert = !val.invert;
   return val;
}

static inline struct mi_value
mi_resolve_invert(struct mi_builder *b, struct mi_value src)
{
   if (!src.invert)
      return src;

   assert(src.type != MI_VALUE_TYPE_IMM);
   return mi_math_binop(b, MI_ALU_ADD, src, mi_imm(0),
                           MI_ALU_STORE, MI_ALU_ACCU);
}

static inline struct mi_value
mi_iadd(struct mi_builder *b, struct mi_value src0, struct mi_value src1)
{
   if (src0.type == MI_VALUE_TYPE_IMM && src1.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src0) + mi_value_to_u64(src1));

   return mi_math_binop(b, MI_ALU_ADD, src0, src1,
                           MI_ALU_STORE, MI_ALU_ACCU);
}

static inline struct mi_value
mi_iadd_imm(struct mi_builder *b,
                struct mi_value src, uint64_t N)
{
   if (N == 0)
      return src;

   return mi_iadd(b, src, mi_imm(N));
}

static inline struct mi_value
mi_isub(struct mi_builder *b, struct mi_value src0, struct mi_value src1)
{
   if (src0.type == MI_VALUE_TYPE_IMM && src1.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src0) - mi_value_to_u64(src1));

   return mi_math_binop(b, MI_ALU_SUB, src0, src1,
                           MI_ALU_STORE, MI_ALU_ACCU);
}

static inline struct mi_value
mi_ieq(struct mi_builder *b, struct mi_value src0, struct mi_value src1)
{
   if (src0.type == MI_VALUE_TYPE_IMM && src1.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src0) == mi_value_to_u64(src1) ? ~0ull : 0);

   /* Compute "equal" by subtracting and storing the zero bit */
   return mi_math_binop(b, MI_ALU_SUB, src0, src1,
                            MI_ALU_STORE, MI_ALU_ZF);
}

static inline struct mi_value
mi_ine(struct mi_builder *b, struct mi_value src0, struct mi_value src1)
{
   if (src0.type == MI_VALUE_TYPE_IMM && src1.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src0) != mi_value_to_u64(src1) ? ~0ull : 0);

   /* Compute "not equal" by subtracting and storing the inverse zero bit */
   return mi_math_binop(b, MI_ALU_SUB, src0, src1,
                            MI_ALU_STOREINV, MI_ALU_ZF);
}

static inline struct mi_value
mi_ult(struct mi_builder *b, struct mi_value src0, struct mi_value src1)
{
   if (src0.type == MI_VALUE_TYPE_IMM && src1.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src0) < mi_value_to_u64(src1) ? ~0ull : 0);

   /* Compute "less than" by subtracting and storing the carry bit */
   return mi_math_binop(b, MI_ALU_SUB, src0, src1,
                           MI_ALU_STORE, MI_ALU_CF);
}

static inline struct mi_value
mi_uge(struct mi_builder *b, struct mi_value src0, struct mi_value src1)
{
   if (src0.type == MI_VALUE_TYPE_IMM && src1.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src0) >= mi_value_to_u64(src1) ? ~0ull : 0);

   /* Compute "less than" by subtracting and storing the carry bit */
   return mi_math_binop(b, MI_ALU_SUB, src0, src1,
                           MI_ALU_STOREINV, MI_ALU_CF);
}

static inline struct mi_value
mi_iand(struct mi_builder *b, struct mi_value src0, struct mi_value src1)
{
   if (src0.type == MI_VALUE_TYPE_IMM && src1.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src0) & mi_value_to_u64(src1));

   return mi_math_binop(b, MI_ALU_AND, src0, src1,
                           MI_ALU_STORE, MI_ALU_ACCU);
}

static inline struct mi_value
mi_nz(struct mi_builder *b, struct mi_value src)
{
   if (src.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src) != 0 ? ~0ull : 0);

   return mi_math_binop(b, MI_ALU_ADD, src, mi_imm(0),
                           MI_ALU_STOREINV, MI_ALU_ZF);
}

static inline struct mi_value
mi_z(struct mi_builder *b, struct mi_value src)
{
   if (src.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src) == 0 ? ~0ull : 0);

   return mi_math_binop(b, MI_ALU_ADD, src, mi_imm(0),
                           MI_ALU_STORE, MI_ALU_ZF);
}

static inline struct mi_value
mi_ior(struct mi_builder *b,
       struct mi_value src0, struct mi_value src1)
{
   if (src0.type == MI_VALUE_TYPE_IMM && src1.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src0) | mi_value_to_u64(src1));

   return mi_math_binop(b, MI_ALU_OR, src0, src1,
                           MI_ALU_STORE, MI_ALU_ACCU);
}

#if GFX_VERx10 >= 125
static inline struct mi_value
mi_ishl(struct mi_builder *b, struct mi_value src0, struct mi_value src1)
{
   if (src1.type == MI_VALUE_TYPE_IMM) {
      assert(util_is_power_of_two_or_zero(mi_value_to_u64(src1)));
      assert(mi_value_to_u64(src1) <= 32);
   }

   if (src0.type == MI_VALUE_TYPE_IMM && src1.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src0) << mi_value_to_u64(src1));

   return mi_math_binop(b, MI_ALU_SHL, src0, src1,
                           MI_ALU_STORE, MI_ALU_ACCU);
}

static inline struct mi_value
mi_ushr(struct mi_builder *b, struct mi_value src0, struct mi_value src1)
{
   if (src1.type == MI_VALUE_TYPE_IMM) {
      assert(util_is_power_of_two_or_zero(mi_value_to_u64(src1)));
      assert(mi_value_to_u64(src1) <= 32);
   }

   if (src0.type == MI_VALUE_TYPE_IMM && src1.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src0) >> mi_value_to_u64(src1));

   return mi_math_binop(b, MI_ALU_SHR, src0, src1,
                           MI_ALU_STORE, MI_ALU_ACCU);
}

static inline struct mi_value
mi_ushr_imm(struct mi_builder *b, struct mi_value src, uint32_t shift)
{
   if (shift == 0)
      return src;

   if (shift >= 64)
      return mi_imm(0);

   if (src.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src) >> shift);

   struct mi_value res = mi_value_to_gpr(b, src);

   /* Annoyingly, we only have power-of-two shifts */
   while (shift) {
      int bit = u_bit_scan(&shift);
      assert(bit <= 5);
      res = mi_ushr(b, res, mi_imm(1ULL << bit));
   }

   return res;
}

static inline struct mi_value
mi_ishr(struct mi_builder *b, struct mi_value src0, struct mi_value src1)
{
   if (src1.type == MI_VALUE_TYPE_IMM) {
      assert(util_is_power_of_two_or_zero(mi_value_to_u64(src1)));
      assert(mi_value_to_u64(src1) <= 32);
   }

   if (src0.type == MI_VALUE_TYPE_IMM && src1.type == MI_VALUE_TYPE_IMM)
      return mi_imm((int64_t)mi_value_to_u64(src0) >> mi_value_to_u64(src1));

   return mi_math_binop(b, MI_ALU_SAR, src0, src1,
                            MI_ALU_STORE, MI_ALU_ACCU);
}

static inline struct mi_value
mi_ishr_imm(struct mi_builder *b, struct mi_value src, uint32_t shift)
{
   if (shift == 0)
      return src;

   if (shift >= 64)
      return mi_imm(0);

   if (src.type == MI_VALUE_TYPE_IMM)
      return mi_imm((int64_t)mi_value_to_u64(src) >> shift);

   struct mi_value res = mi_value_to_gpr(b, src);

   /* Annoyingly, we only have power-of-two shifts */
   while (shift) {
      int bit = u_bit_scan(&shift);
      assert(bit <= 5);
      res = mi_ishr(b, res, mi_imm(1 << bit));
   }

   return res;
}
#endif /* if GFX_VERx10 >= 125 */

static inline struct mi_value
mi_imul_imm(struct mi_builder *b, struct mi_value src, uint32_t N)
{
   if (src.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src) * N);

   if (N == 0) {
      mi_value_unref(b, src);
      return mi_imm(0);
   }

   if (N == 1)
      return src;

   src = mi_value_to_gpr(b, src);

   struct mi_value res = mi_value_ref(b, src);

   unsigned top_bit = 31 - __builtin_clz(N);
   for (int i = top_bit - 1; i >= 0; i--) {
      res = mi_iadd(b, res, mi_value_ref(b, res));
      if (N & (1 << i))
         res = mi_iadd(b, res, mi_value_ref(b, src));
   }

   mi_value_unref(b, src);

   return res;
}

static inline struct mi_value
mi_ishl_imm(struct mi_builder *b, struct mi_value src, uint32_t shift)
{
   if (shift == 0)
      return src;

   if (shift >= 64)
      return mi_imm(0);

   if (src.type == MI_VALUE_TYPE_IMM)
      return mi_imm(mi_value_to_u64(src) << shift);

   struct mi_value res = mi_value_to_gpr(b, src);

#if GFX_VERx10 >= 125
   /* Annoyingly, we only have power-of-two shifts */
   while (shift) {
      int bit = u_bit_scan(&shift);
      assert(bit <= 5);
      res = mi_ishl(b, res, mi_imm(1 << bit));
   }
#else
   for (unsigned i = 0; i < shift; i++)
      res = mi_iadd(b, res, mi_value_ref(b, res));
#endif

   return res;
}

static inline struct mi_value
mi_ushr32_imm(struct mi_builder *b, struct mi_value src, uint32_t shift)
{
   if (shift == 0)
      return src;

   if (shift >= 64)
      return mi_imm(0);

   /* We right-shift by left-shifting by 32 - shift and taking the top 32 bits
    * of the result.
    */
   if (src.type == MI_VALUE_TYPE_IMM)
      return mi_imm((mi_value_to_u64(src) >> shift) & UINT32_MAX);

   if (shift > 32) {
      struct mi_value tmp = mi_new_gpr(b);
      _mi_copy_no_unref(b, mi_value_half(tmp, false),
                               mi_value_half(src, true));
      _mi_copy_no_unref(b, mi_value_half(tmp, true), mi_imm(0));
      mi_value_unref(b, src);
      src = tmp;
      shift -= 32;
   }
   assert(shift <= 32);
   struct mi_value tmp = mi_ishl_imm(b, src, 32 - shift);
   struct mi_value dst = mi_new_gpr(b);
   _mi_copy_no_unref(b, mi_value_half(dst, false),
                            mi_value_half(tmp, true));
   _mi_copy_no_unref(b, mi_value_half(dst, true), mi_imm(0));
   mi_value_unref(b, tmp);
   return dst;
}

static inline struct mi_value
mi_udiv32_imm(struct mi_builder *b, struct mi_value N, uint32_t D)
{
   if (N.type == MI_VALUE_TYPE_IMM) {
      assert(mi_value_to_u64(N) <= UINT32_MAX);
      return mi_imm(mi_value_to_u64(N) / D);
   }

   /* We implicitly assume that N is only a 32-bit value */
   if (D == 0) {
      /* This is invalid but we should do something */
      return mi_imm(0);
   } else if (util_is_power_of_two_or_zero(D)) {
      return mi_ushr32_imm(b, N, util_logbase2(D));
   } else {
      struct util_fast_udiv_info m = util_compute_fast_udiv_info(D, 32, 32);
      assert(m.multiplier <= UINT32_MAX);

      if (m.pre_shift)
         N = mi_ushr32_imm(b, N, m.pre_shift);

      /* Do the 32x32 multiply  into gpr0 */
      N = mi_imul_imm(b, N, m.multiplier);

      if (m.increment)
         N = mi_iadd(b, N, mi_imm(m.multiplier));

      N = mi_ushr32_imm(b, N, 32);

      if (m.post_shift)
         N = mi_ushr32_imm(b, N, m.post_shift);

      return N;
   }
}

#endif /* MI_MATH section */

/* This assumes addresses of strictly more than 32bits (aka. Gfx8+). */
#if MI_BUILDER_CAN_WRITE_BATCH

struct mi_address_token {
   /* Pointers to address memory fields in the batch. */
   uint64_t *ptrs[2];
};

static inline struct mi_address_token
mi_store_address(struct mi_builder *b, struct mi_value addr_reg)
{
   mi_builder_flush_math(b);

   assert(addr_reg.type == MI_VALUE_TYPE_REG64);

   struct mi_address_token token = {};

   for (unsigned i = 0; i < 2; i++) {
      mi_builder_emit(b, GENX(MI_STORE_REGISTER_MEM), srm) {
         srm.RegisterAddress = addr_reg.reg + (i * 4);

         const unsigned addr_dw =
            GENX(MI_STORE_REGISTER_MEM_MemoryAddress_start) / 8;
         token.ptrs[i] = (void *)_dst + addr_dw;
      }
   }

   mi_value_unref(b, addr_reg);
   return token;
}

static inline void
mi_self_mod_barrier(struct mi_builder *b, unsigned cs_prefetch_size)
{
   /* First make sure all the memory writes from previous modifying commands
    * have landed. We want to do this before going through the CS cache,
    * otherwise we could be fetching memory that hasn't been written to yet.
    */
   mi_builder_emit(b, GENX(PIPE_CONTROL), pc) {
      pc.CommandStreamerStallEnable = true;
   }
   /* Documentation says Gfx11+ should be able to invalidate the command cache
    * but experiment show it doesn't work properly, so for now just get over
    * the CS prefetch.
    */
   for (uint32_t i = 0; i < (cs_prefetch_size / 4); i++)
      mi_builder_emit(b, GENX(MI_NOOP), noop);
}

static inline void
_mi_resolve_address_token(struct mi_builder *b,
                          struct mi_address_token token,
                          void *batch_location)
{
   __gen_address_type addr = __gen_get_batch_address(b->user_data,
                                                    batch_location);
   uint64_t addr_addr_u64 = __gen_combine_address(b->user_data, batch_location,
                                                  addr, 0);
   *(token.ptrs[0]) = addr_addr_u64;
   *(token.ptrs[1]) = addr_addr_u64 + 4;
}

#endif /* MI_BUILDER_CAN_WRITE_BATCH */

#if GFX_VERx10 >= 125

/*
 * Indirect load/store.  Only available on XE_HP+
 */

MUST_CHECK static inline struct mi_value
mi_load_mem64_offset(struct mi_builder *b,
                     __gen_address_type addr, struct mi_value offset)
{
   uint64_t addr_u64 = __gen_combine_address(b->user_data, NULL, addr, 0);
   struct mi_value addr_val = mi_imm(addr_u64);

   struct mi_value dst = mi_new_gpr(b);

   uint32_t dw[5];
   dw[0] = _mi_math_load_src(b, MI_ALU_SRCA, &addr_val);
   dw[1] = _mi_math_load_src(b, MI_ALU_SRCB, &offset);
   dw[2] = _mi_pack_alu(MI_ALU_ADD, 0, 0);
   dw[3] = _mi_pack_alu(MI_ALU_LOADIND, _mi_value_as_gpr(dst), MI_ALU_ACCU);
   dw[4] = _mi_pack_alu(MI_ALU_FENCE_RD, 0, 0);
   _mi_builder_push_math(b, dw, 5);

   mi_value_unref(b, addr_val);
   mi_value_unref(b, offset);

   return dst;
}

static inline void
mi_store_mem64_offset(struct mi_builder *b,
                          __gen_address_type addr, struct mi_value offset,
                          struct mi_value data)
{
   uint64_t addr_u64 = __gen_combine_address(b->user_data, NULL, addr, 0);
   struct mi_value addr_val = mi_imm(addr_u64);

   data = mi_value_to_gpr(b, mi_resolve_invert(b, data));

   uint32_t dw[5];
   dw[0] = _mi_math_load_src(b, MI_ALU_SRCA, &addr_val);
   dw[1] = _mi_math_load_src(b, MI_ALU_SRCB, &offset);
   dw[2] = _mi_pack_alu(MI_ALU_ADD, 0, 0);
   dw[3] = _mi_pack_alu(MI_ALU_STOREIND, MI_ALU_ACCU, _mi_value_as_gpr(data));
   dw[4] = _mi_pack_alu(MI_ALU_FENCE_WR, 0, 0);
   _mi_builder_push_math(b, dw, 5);

   mi_value_unref(b, addr_val);
   mi_value_unref(b, offset);
   mi_value_unref(b, data);

   /* This is the only math case which has side-effects outside of regular
    * registers to flush math afterwards so we don't confuse anyone.
    */
   mi_builder_flush_math(b);
}

/*
 * Control-flow Section.  Only available on XE_HP+
 */

struct _mi_goto {
   bool predicated;
   void *mi_bbs;
};

struct mi_goto_target {
   bool placed;
   unsigned num_gotos;
   struct _mi_goto gotos[8];
   __gen_address_type addr;
};

#define MI_GOTO_TARGET_INIT ((struct mi_goto_target) {})

#define MI_BUILDER_MI_PREDICATE_RESULT_num  0x2418

static inline void
mi_goto_if(struct mi_builder *b, struct mi_value cond,
           struct mi_goto_target *t)
{
   /* First, set up the predicate, if any */
   bool predicated;
   if (cond.type == MI_VALUE_TYPE_IMM) {
      /* If it's an immediate, the goto either doesn't happen or happens
       * unconditionally.
       */
      if (mi_value_to_u64(cond) == 0)
         return;

      assert(mi_value_to_u64(cond) == ~0ull);
      predicated = false;
   } else if (mi_value_is_reg(cond) &&
              cond.reg == MI_BUILDER_MI_PREDICATE_RESULT_num) {
      /* If it's MI_PREDICATE_RESULT, we use whatever predicate the client
       * provided us with
       */
      assert(cond.type == MI_VALUE_TYPE_REG32);
      predicated = true;
   } else {
      mi_store(b, mi_reg32(MI_BUILDER_MI_PREDICATE_RESULT_num), cond);
      predicated = true;
   }

   if (predicated) {
      mi_builder_emit(b, GENX(MI_SET_PREDICATE), sp) {
         sp.PredicateEnable = NOOPOnResultClear;
      }
   }
   if (t->placed) {
      mi_builder_emit(b, GENX(MI_BATCH_BUFFER_START), bbs) {
         bbs.PredicationEnable         = predicated;
         bbs.AddressSpaceIndicator     = ASI_PPGTT;
         bbs.BatchBufferStartAddress   = t->addr;
      }
   } else {
      assert(t->num_gotos < ARRAY_SIZE(t->gotos));
      struct _mi_goto g = {
         .predicated = predicated,
         .mi_bbs = __gen_get_batch_dwords(b->user_data,
                                          GENX(MI_BATCH_BUFFER_START_length)),
      };
      memset(g.mi_bbs, 0, 4 * GENX(MI_BATCH_BUFFER_START_length));
      t->gotos[t->num_gotos++] = g;
   }
   if (predicated) {
      mi_builder_emit(b, GENX(MI_SET_PREDICATE), sp) {
         sp.PredicateEnable = NOOPNever;
      }
   }
}

static inline void
mi_goto(struct mi_builder *b, struct mi_goto_target *t)
{
   mi_goto_if(b, mi_imm(-1), t);
}

static inline void
mi_goto_target(struct mi_builder *b, struct mi_goto_target *t)
{
   mi_builder_emit(b, GENX(MI_SET_PREDICATE), sp) {
      sp.PredicateEnable = NOOPNever;
      t->addr = __gen_get_batch_address(b->user_data, _dst);
   }
   t->placed = true;

   struct GENX(MI_BATCH_BUFFER_START) bbs = { GENX(MI_BATCH_BUFFER_START_header) };
   bbs.AddressSpaceIndicator     = ASI_PPGTT;
   bbs.BatchBufferStartAddress   = t->addr;

   for (unsigned i = 0; i < t->num_gotos; i++) {
      bbs.PredicationEnable = t->gotos[i].predicated;
      GENX(MI_BATCH_BUFFER_START_pack)(b->user_data, t->gotos[i].mi_bbs, &bbs);
   }
}

static inline struct mi_goto_target
mi_goto_target_init_and_place(struct mi_builder *b)
{
   struct mi_goto_target t = MI_GOTO_TARGET_INIT;
   mi_goto_target(b, &t);
   return t;
}

#define mi_loop(b) \
   for (struct mi_goto_target __break = MI_GOTO_TARGET_INIT, \
        __continue = mi_goto_target_init_and_place(b); !__break.placed; \
        mi_goto(b, &__continue), mi_goto_target(b, &__break))

#define mi_break(b) mi_goto(b, &__break)
#define mi_break_if(b, cond) mi_goto_if(b, cond, &__break)
#define mi_continue(b) mi_goto(b, &__continue)
#define mi_continue_if(b, cond) mi_goto_if(b, cond, &__continue)

#endif /* GFX_VERx10 >= 125 */

#endif /* MI_BUILDER_H */
