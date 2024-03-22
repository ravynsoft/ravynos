/*
 * Copyright (C) 2021 Collabora Ltd.
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
 *
 * Authors (Collabora):
 *      Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#ifndef __VALHALL_H
#define __VALHALL_H

#include <stdint.h>
#include "bi_opcodes.h"
#include "valhall_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VA_NUM_GENERAL_SLOTS 3

extern const uint32_t valhall_immediates[32];

enum va_size {
   VA_SIZE_8 = 0,
   VA_SIZE_16 = 1,
   VA_SIZE_32 = 2,
   VA_SIZE_64 = 3,
};

enum va_unit {
   /** Fused floating-point multiply-add */
   VA_UNIT_FMA = 0,

   /** Type conversion and basic arithmetic */
   VA_UNIT_CVT = 1,

   /** Special function unit */
   VA_UNIT_SFU = 2,

   /** Varying */
   VA_UNIT_V = 3,

   /** General load/store */
   VA_UNIT_LS = 4,

   /** Texture */
   VA_UNIT_T = 5,

   /** Fused varying and texture */
   VA_UNIT_VT = 6,

   /** Produces a message for a unit not otherwise specified */
   VA_UNIT_NONE = 7
};

struct va_src_info {
   bool absneg       : 1;
   bool swizzle      : 1;
   bool notted       : 1;
   bool lane         : 1;
   bool lanes        : 1;
   bool halfswizzle  : 1;
   bool widen        : 1;
   bool combine      : 1;
   enum va_size size : 2;
} __attribute__((packed));

struct va_opcode_info {
   uint64_t exact;
   struct va_src_info srcs[4];
   uint8_t type_size         : 8;
   enum va_unit unit         : 3;
   unsigned nr_srcs          : 3;
   unsigned nr_staging_srcs  : 2;
   unsigned nr_staging_dests : 2;
   bool has_dest             : 1;
   bool is_signed            : 1;
   bool clamp                : 1;
   bool saturate             : 1;
   bool rhadd                : 1;
   bool round_mode           : 1;
   bool condition            : 1;
   bool result_type          : 1;
   bool vecsize              : 1;
   bool register_format      : 1;
   bool slot                 : 1;
   bool sr_count             : 1;
   bool sr_write_count       : 1;
   unsigned sr_control       : 2;
};

extern const struct va_opcode_info valhall_opcodes[BI_NUM_OPCODES];

/* Bifrost specifies the source of bitwise operations as (A, B, shift), but
 * Valhall specifies (A, shift, B). We follow Bifrost conventions in the
 * compiler, so normalize.
 *
 * Bifrost specifies BLEND as staging + (coverage, blend descriptor), but
 * Valhall specifies staging + (blend descriptor, coverage). Given we put
 * staging sources first, this works out to the same swap as bitwise ops.
 */

static inline bool
va_swap_12(enum bi_opcode op)
{
   switch (op) {
   case BI_OPCODE_BLEND:
   case BI_OPCODE_LSHIFT_AND_I32:
   case BI_OPCODE_LSHIFT_AND_V2I16:
   case BI_OPCODE_LSHIFT_AND_V4I8:
   case BI_OPCODE_LSHIFT_OR_I32:
   case BI_OPCODE_LSHIFT_OR_V2I16:
   case BI_OPCODE_LSHIFT_OR_V4I8:
   case BI_OPCODE_LSHIFT_XOR_I32:
   case BI_OPCODE_LSHIFT_XOR_V2I16:
   case BI_OPCODE_LSHIFT_XOR_V4I8:
   case BI_OPCODE_RSHIFT_AND_I32:
   case BI_OPCODE_RSHIFT_AND_V2I16:
   case BI_OPCODE_RSHIFT_AND_V4I8:
   case BI_OPCODE_RSHIFT_OR_I32:
   case BI_OPCODE_RSHIFT_OR_V2I16:
   case BI_OPCODE_RSHIFT_OR_V4I8:
   case BI_OPCODE_RSHIFT_XOR_I32:
   case BI_OPCODE_RSHIFT_XOR_V2I16:
   case BI_OPCODE_RSHIFT_XOR_V4I8:
      return true;
   default:
      return false;
   }
}

static inline struct va_src_info
va_src_info(enum bi_opcode op, unsigned src)
{
   unsigned idx = (va_swap_12(op) && (src == 1 || src == 2)) ? (3 - src) : src;
   return valhall_opcodes[op].srcs[idx];
}

static inline bool
va_flow_is_wait_or_none(enum va_flow flow)
{
   return (flow <= VA_FLOW_WAIT);
}

#ifdef __cplusplus
} /* extern C */
#endif

#endif
