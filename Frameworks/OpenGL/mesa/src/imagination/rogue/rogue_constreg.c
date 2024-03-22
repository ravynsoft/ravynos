/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "rogue.h"
#include "util/macros.h"

/**
 * \file rogue_constreg.c
 *
 * \brief Contains functions to find and allocate constant register values.
 */

/**
 * \brief Mapping of constant register values and their indices.
 */
typedef struct rogue_constreg_map {
   uint32_t value;
   unsigned index;
} rogue_constreg_map;

#define CONSTREG(VALUE, INDEX)            \
   {                                      \
      .value = (VALUE), .index = (INDEX), \
   }

/**
 * \brief Constant register values (sorted for bsearch).
 */
static const rogue_constreg_map const_regs[] = {
   CONSTREG(0x00000000U, 0U), /* 0   (INT32) / 0.0 (Float) */
   CONSTREG(0x00000001U, 1U), /* 1   (INT32) */
   CONSTREG(0x00000002U, 2U), /* 2   (INT32) */
   CONSTREG(0x00000003U, 3U), /* 3   (INT32) */
   CONSTREG(0x00000004U, 4U), /* 4   (INT32) */
   CONSTREG(0x00000005U, 5U), /* 5   (INT32) */
   CONSTREG(0x00000006U, 6U), /* 6   (INT32) */
   CONSTREG(0x00000007U, 7U), /* 7   (INT32) */
   CONSTREG(0x00000008U, 8U), /* 8   (INT32) */
   CONSTREG(0x00000009U, 9U), /* 9   (INT32) */
   CONSTREG(0x0000000aU, 10U), /* 10  (INT32) */
   CONSTREG(0x0000000bU, 11U), /* 11  (INT32) */
   CONSTREG(0x0000000cU, 12U), /* 12  (INT32) */
   CONSTREG(0x0000000dU, 13U), /* 13  (INT32) */
   CONSTREG(0x0000000eU, 14U), /* 14  (INT32) */
   CONSTREG(0x0000000fU, 15U), /* 15  (INT32) */
   CONSTREG(0x00000010U, 16U), /* 16  (INT32) */
   CONSTREG(0x00000011U, 17U), /* 17  (INT32) */
   CONSTREG(0x00000012U, 18U), /* 18  (INT32) */
   CONSTREG(0x00000013U, 19U), /* 19  (INT32) */
   CONSTREG(0x00000014U, 20U), /* 20  (INT32) */
   CONSTREG(0x00000015U, 21U), /* 21  (INT32) */
   CONSTREG(0x00000016U, 22U), /* 22  (INT32) */
   CONSTREG(0x00000017U, 23U), /* 23  (INT32) */
   CONSTREG(0x00000018U, 24U), /* 24  (INT32) */
   CONSTREG(0x00000019U, 25U), /* 25  (INT32) */
   CONSTREG(0x0000001aU, 26U), /* 26  (INT32) */
   CONSTREG(0x0000001bU, 27U), /* 27  (INT32) */
   CONSTREG(0x0000001cU, 28U), /* 28  (INT32) */
   CONSTREG(0x0000001dU, 29U), /* 29  (INT32) */
   CONSTREG(0x0000001eU, 30U), /* 30  (INT32) */
   CONSTREG(0x0000001fU, 31U), /* 31  (INT32) */
   CONSTREG(0x0000007fU, 147U), /* 127 (INT32) */

   CONSTREG(0x37800000U, 134U), /* 1.0f/65536f */
   CONSTREG(0x38000000U, 135U), /* 1.0f/32768f */
   CONSTREG(0x38800000U, 88U), /* float(2^-14) */
   CONSTREG(0x39000000U, 87U), /* float(2^-13) */
   CONSTREG(0x39800000U, 86U), /* float(2^-12) */
   CONSTREG(0x3a000000U, 85U), /* float(2^-11) */
   CONSTREG(0x3a800000U, 84U), /* float(2^-10) */
   CONSTREG(0x3b000000U, 83U), /* float(2^-9) */
   CONSTREG(0x3b4d2e1cU, 136U), /* 0.0031308f */
   CONSTREG(0x3b800000U, 82U), /* float(2^-8) */
   CONSTREG(0x3c000000U, 81U), /* float(2^-7) */
   CONSTREG(0x3c800000U, 80U), /* float(2^-6) */
   CONSTREG(0x3d000000U, 79U), /* float(2^-5) */
   CONSTREG(0x3d25aee6U, 156U), /* 0.04045f */
   CONSTREG(0x3d6147aeU, 140U), /* 0.055f */
   CONSTREG(0x3d800000U, 78U), /* float(2^-4) */
   CONSTREG(0x3d9e8391U, 157U), /* 1.0f/12.92f */
   CONSTREG(0x3e000000U, 77U), /* float(2^-3) */
   CONSTREG(0x3e2aaaabU, 153U), /* 1/6 */
   CONSTREG(0x3e800000U, 76U), /* float(2^-2) */
   CONSTREG(0x3e9a209bU, 145U), /* Log_10(2) */
   CONSTREG(0x3ea2f983U, 128U), /* Float 1/PI */
   CONSTREG(0x3eaaaaabU, 152U), /* 1/3 */
   CONSTREG(0x3ebc5ab2U, 90U), /* 1/e */
   CONSTREG(0x3ed55555U, 138U), /* 1.0f/2.4f */
   CONSTREG(0x3f000000U, 75U), /* float(2^-1) */
   CONSTREG(0x3f22f983U, 129U), /* Float 2/PI */
   CONSTREG(0x3f317218U, 146U), /* Log_e(2) */
   CONSTREG(0x3f3504f3U, 92U), /* Float 1/SQRT(2) */
   CONSTREG(0x3f490fdbU, 93U), /* Float PI/4 */
   CONSTREG(0x3f72a76fU, 158U), /* 1.0f/1.055f */
   CONSTREG(0x3f800000U, 64U), /* 1.0f */
   CONSTREG(0x3f860a92U, 151U), /* Pi/3 */
   CONSTREG(0x3f870a3dU, 139U), /* 1.055f */
   CONSTREG(0x3fa2f983U, 130U), /* Float 4/PI */
   CONSTREG(0x3fb504f3U, 91U), /* Float SQRT(2) */
   CONSTREG(0x3fb8aa3bU, 155U), /* Log_2(e) */
   CONSTREG(0x3fc90fdbU, 94U), /* Float PI/2 */
   CONSTREG(0x40000000U, 65U), /* float(2^1) */
   CONSTREG(0x4019999aU, 159U), /* 2.4f */
   CONSTREG(0x402df854U, 89U), /* e */
   CONSTREG(0x40490fdbU, 95U), /* Float PI */
   CONSTREG(0x40549a78U, 154U), /* Log_2(10) */
   CONSTREG(0x40800000U, 66U), /* float(2^2) */
   CONSTREG(0x40c90fdbU, 131U), /* Float 2*PI */
   CONSTREG(0x41000000U, 67U), /* float(2^3) */
   CONSTREG(0x41490fdbU, 132U), /* Float 4*PI */
   CONSTREG(0x414eb852U, 137U), /* 12.92f */
   CONSTREG(0x41800000U, 68U), /* float(2^4) */
   CONSTREG(0x41c90fdbU, 133U), /* Float 8*PI */
   CONSTREG(0x42000000U, 69U), /* float(2^5) */
   CONSTREG(0x42800000U, 70U), /* float(2^6) */
   CONSTREG(0x43000000U, 71U), /* float(2^7) */
   CONSTREG(0x43800000U, 72U), /* float(2^8) */
   CONSTREG(0x44000000U, 73U), /* float(2^9) */
   CONSTREG(0x44800000U, 74U), /* float(2^10) */
   CONSTREG(0x4b000000U, 149U), /* 2^23 */
   CONSTREG(0x4b800000U, 150U), /* 2^24 */
   CONSTREG(0x7f7fffffU, 148U), /* FLT_MAX */
   CONSTREG(0x7f800000U, 142U), /* Infinity */
   CONSTREG(0x7fff7fffU, 144U), /* ARGB1555 mask */
   CONSTREG(0x80000000U, 141U), /* -0.0f */
   CONSTREG(0xffffffffU, 143U), /* -1 */
};

#undef CONSTREG

/**
 * \brief Comparison function for bsearch() to support rogue_constreg_map.
 *
 * \param[in] lhs The left hand side of the comparison.
 * \param[in] rhs The right hand side of the comparison.
 * \return 0 if (lhs == rhs), -1 if (lhs < rhs), 1 if (lhs > rhs).
 */
static int constreg_cmp(const void *lhs, const void *rhs)
{
   const rogue_constreg_map *l = lhs;
   const rogue_constreg_map *r = rhs;

   if (l->value < r->value)
      return -1;
   else if (l->value > r->value)
      return 1;

   return 0;
}

/**
 * \brief Determines whether a given value exists in a constant register.
 *
 * \param[in] imm The immediate value required.
 * \return The index of the constant register containing the value, or
 * ROGUE_NO_CONST_REG if the value is not found.
 */
PUBLIC
unsigned rogue_constreg_lookup(rogue_imm_t imm)
{
   rogue_constreg_map constreg_target = {
      .value = imm.u32,
   };
   const rogue_constreg_map *constreg;

   constreg = bsearch(&constreg_target,
                      const_regs,
                      ARRAY_SIZE(const_regs),
                      sizeof(rogue_constreg_map),
                      constreg_cmp);
   if (!constreg)
      return ROGUE_NO_CONST_REG;

   return constreg->index;
}
