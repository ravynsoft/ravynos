/*
 * Copyright © 2019 Google, Inc.
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

#ifndef FD6_PACK_H
#define FD6_PACK_H

#include "a6xx.xml.h"

struct fd_reg_pair {
   uint32_t reg;
   uint64_t value;
   struct fd_bo *bo;
   bool is_address;
   bool bo_write;
   uint32_t bo_offset;
   uint32_t bo_shift;
   uint32_t bo_low;
};

#define __bo_type struct fd_bo *

#include "a6xx-pack.xml.h"
#include "adreno-pm4-pack.xml.h"

#define __assert_eq(a, b)                                                      \
   do {                                                                        \
      if ((a) != (b)) {                                                        \
         fprintf(stderr, "assert failed: " #a " (0x%x) != " #b " (0x%x)\n", a, \
                 b);                                                           \
         assert((a) == (b));                                                   \
      }                                                                        \
   } while (0)

#if !FD_BO_NO_HARDPIN
#  error 'Hardpin unsupported'
#endif

static inline uint64_t
__reg_iova(const struct fd_reg_pair *reg)
{
   uint64_t iova = __reloc_iova((struct fd_bo *)reg->bo,
                                reg->bo_offset, 0,
                                -reg->bo_shift);
   return iova << reg->bo_low;
}

#define __ONE_REG(ring, i, ...)                                                \
   do {                                                                        \
      const struct fd_reg_pair __regs[] = {__VA_ARGS__};                       \
      /* NOTE: allow __regs[0].reg==0, this happens in OUT_PKT() */            \
      if (i < ARRAY_SIZE(__regs) && (i == 0 || __regs[i].reg > 0)) {           \
         __assert_eq(__regs[0].reg + i, __regs[i].reg);                        \
         if (__regs[i].bo) {                                                   \
            uint64_t *__p64 = (uint64_t *)__p;                                 \
            *__p64 = __reg_iova(&__regs[i]) | __regs[i].value;                 \
            __p += 2;                                                          \
            fd_ringbuffer_assert_attached(ring, __regs[i].bo);                 \
         } else {                                                              \
            *__p++ = __regs[i].value;                                          \
            if (__regs[i].is_address)                                          \
               *__p++ = __regs[i].value >> 32;                                 \
         }                                                                     \
      }                                                                        \
   } while (0)

#define OUT_REG(ring, ...)                                                     \
   do {                                                                        \
      const struct fd_reg_pair __regs[] = {__VA_ARGS__};                       \
      unsigned count = ARRAY_SIZE(__regs);                                     \
                                                                               \
      STATIC_ASSERT(ARRAY_SIZE(__regs) > 0);                                   \
      STATIC_ASSERT(ARRAY_SIZE(__regs) <= 16);                                 \
                                                                               \
      BEGIN_RING(ring, count + 1);                                             \
      uint32_t *__p = ring->cur;                                               \
      *__p++ = pm4_pkt4_hdr((uint16_t)__regs[0].reg, (uint16_t)count);         \
                                                                               \
      __ONE_REG(ring, 0, __VA_ARGS__);                                         \
      __ONE_REG(ring, 1, __VA_ARGS__);                                         \
      __ONE_REG(ring, 2, __VA_ARGS__);                                         \
      __ONE_REG(ring, 3, __VA_ARGS__);                                         \
      __ONE_REG(ring, 4, __VA_ARGS__);                                         \
      __ONE_REG(ring, 5, __VA_ARGS__);                                         \
      __ONE_REG(ring, 6, __VA_ARGS__);                                         \
      __ONE_REG(ring, 7, __VA_ARGS__);                                         \
      __ONE_REG(ring, 8, __VA_ARGS__);                                         \
      __ONE_REG(ring, 9, __VA_ARGS__);                                         \
      __ONE_REG(ring, 10, __VA_ARGS__);                                        \
      __ONE_REG(ring, 11, __VA_ARGS__);                                        \
      __ONE_REG(ring, 12, __VA_ARGS__);                                        \
      __ONE_REG(ring, 13, __VA_ARGS__);                                        \
      __ONE_REG(ring, 14, __VA_ARGS__);                                        \
      __ONE_REG(ring, 15, __VA_ARGS__);                                        \
      ring->cur = __p;                                                         \
   } while (0)

#define OUT_PKT(ring, opcode, ...)                                             \
   do {                                                                        \
      const struct fd_reg_pair __regs[] = {__VA_ARGS__};                       \
      unsigned count = ARRAY_SIZE(__regs);                                     \
                                                                               \
      STATIC_ASSERT(ARRAY_SIZE(__regs) <= 16);                                 \
                                                                               \
      BEGIN_RING(ring, count + 1);                                             \
      uint32_t *__p = ring->cur;                                               \
      *__p++ = pm4_pkt7_hdr(opcode, count);                                    \
                                                                               \
      __ONE_REG(ring, 0, __VA_ARGS__);                                         \
      __ONE_REG(ring, 1, __VA_ARGS__);                                         \
      __ONE_REG(ring, 2, __VA_ARGS__);                                         \
      __ONE_REG(ring, 3, __VA_ARGS__);                                         \
      __ONE_REG(ring, 4, __VA_ARGS__);                                         \
      __ONE_REG(ring, 5, __VA_ARGS__);                                         \
      __ONE_REG(ring, 6, __VA_ARGS__);                                         \
      __ONE_REG(ring, 7, __VA_ARGS__);                                         \
      __ONE_REG(ring, 8, __VA_ARGS__);                                         \
      __ONE_REG(ring, 9, __VA_ARGS__);                                         \
      __ONE_REG(ring, 10, __VA_ARGS__);                                        \
      __ONE_REG(ring, 11, __VA_ARGS__);                                        \
      __ONE_REG(ring, 12, __VA_ARGS__);                                        \
      __ONE_REG(ring, 13, __VA_ARGS__);                                        \
      __ONE_REG(ring, 14, __VA_ARGS__);                                        \
      __ONE_REG(ring, 15, __VA_ARGS__);                                        \
      ring->cur = __p;                                                         \
   } while (0)

/* similar to OUT_PKT() but appends specified # of dwords
 * copied for buf to the end of the packet (ie. for use-
 * cases like CP_LOAD_STATE)
 */
#define OUT_PKTBUF(ring, opcode, dwords, sizedwords, ...)                      \
   do {                                                                        \
      const struct fd_reg_pair __regs[] = {__VA_ARGS__};                       \
      unsigned count = ARRAY_SIZE(__regs);                                     \
                                                                               \
      STATIC_ASSERT(ARRAY_SIZE(__regs) <= 16);                                 \
      count += sizedwords;                                                     \
                                                                               \
      BEGIN_RING(ring, count + 1);                                             \
      uint32_t *__p = ring->cur;                                               \
      *__p++ = pm4_pkt7_hdr(opcode, count);                                    \
                                                                               \
      __ONE_REG(ring, 0, __VA_ARGS__);                                         \
      __ONE_REG(ring, 1, __VA_ARGS__);                                         \
      __ONE_REG(ring, 2, __VA_ARGS__);                                         \
      __ONE_REG(ring, 3, __VA_ARGS__);                                         \
      __ONE_REG(ring, 4, __VA_ARGS__);                                         \
      __ONE_REG(ring, 5, __VA_ARGS__);                                         \
      __ONE_REG(ring, 6, __VA_ARGS__);                                         \
      __ONE_REG(ring, 7, __VA_ARGS__);                                         \
      __ONE_REG(ring, 8, __VA_ARGS__);                                         \
      __ONE_REG(ring, 9, __VA_ARGS__);                                         \
      __ONE_REG(ring, 10, __VA_ARGS__);                                        \
      __ONE_REG(ring, 11, __VA_ARGS__);                                        \
      __ONE_REG(ring, 12, __VA_ARGS__);                                        \
      __ONE_REG(ring, 13, __VA_ARGS__);                                        \
      __ONE_REG(ring, 14, __VA_ARGS__);                                        \
      __ONE_REG(ring, 15, __VA_ARGS__);                                        \
      memcpy(__p, dwords, 4 * sizedwords);                                     \
      __p += sizedwords;                                                       \
      ring->cur = __p;                                                         \
   } while (0)

#define OUT_BUF(ring, dwords, sizedwords)                                      \
   do {                                                                        \
      uint32_t *__p = ring->cur;                                               \
      memcpy(__p, dwords, 4 * sizedwords);                                     \
      __p += sizedwords;                                                       \
      ring->cur = __p;                                                         \
   } while (0)

#endif /* FD6_PACK_H */
